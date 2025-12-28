#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM2 Filter Module
*-
@file   RTCM2Filter.c
@author CHC
@date   2023/04/14
@brief

**************************************************************************/

#include "OSDependent.h"
#include "RTCM2Filter.h"
#include "BSW/MsgDecoder/RTCM/RTCM2Decoder.h"
#include "RTCMDecodeManage.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "Common/DataTypes.h"
#include "Common/CommonFuncs.h"
#include "Encoder/RINEX/RINEXConvert.h"

#include "../../MsgConvertDataTypes.h"

typedef int (*RTCM2_DECODE_BY_TYPEID_FUNC)(RTCM_PARSE_INFO_T*, void*, RTCM2_DECODE_OVERTIME_FUNC);
typedef int (*CONVERT_FUNC)(int, int, void*, void*, int);

typedef struct _RTCM2_DECODE_FUNC_T
{
	RTCM2_DECODE_BY_TYPEID_FUNC RTCM2DecodeByTypeIDFunc;
	CONVERT_FUNC ConvertFunc;
} RTCM2_DECODE_FUNC_T;

/**********************************************************************//**
@brief  decode navigation data word, check party and decode navigation data word

@param Word	[In]navigation data word (2+30bit), (previous word D29*-30* + current word D1-30)
@param pData   [Out] decoded navigation data without parity(8bitx3)

@retval status (0:ok,<0:parity error)

@author CHC
@date 2023/03/15
@note
**************************************************************************/
static int DecodeWord(unsigned int Word, unsigned char *pData)
{
	const unsigned int hamming[]={
		0xBB1F3480, 0x5D8F9A40, 0xAEC7CD00, 0x5763E680, 0x6BB1F340, 0x8B7A89C0
	};
	unsigned int Parity=0, w;
	int Index;

	if (Word & 0x40000000)
	{
		Word ^= 0x3FFFFFC0;
	}

	for (Index=0; Index<6; Index++)
	{
		Parity <<= 1;
		for (w = (Word & hamming[Index])>>6; w; w>>=1)
		{
			Parity ^= w & 1;
		}
	}

	if (Parity != (Word & 0x3F))
	{
		return -1;
	}

	for (Index = 0; Index < 3; Index++)
	{
		pData[Index] = (unsigned char)(Word >> (22 - (Index << 3)));
	}

	return 0;
}

/**********************************************************************//**
@brief  RTCM2 Filter information clear

@param pDecode	  [In]Physical Port ID index

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int RTCM2FilterClear(MSG_DECODE_T* pDecode)
{
	if (pDecode == NULL)
	{
		return -1;
	}

	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo = pDecode->ProtocolFilterInfo.pRTCM2FilterInfo;

	pRTCM2FilterInfo->MsgLen = 0;
	pRTCM2FilterInfo->Word = pRTCM2FilterInfo->LastWord & 0x03;
	pRTCM2FilterInfo->ByteNum = 0;
	pRTCM2FilterInfo->BitNum = 0;
	pRTCM2FilterInfo->DecodeBuf.Len = 0;

	pRTCM2FilterInfo->DecodeOffset = 0;

	return 0;
}

/**********************************************************************//**
@brief  RTCM2 Filter Single Data

@param pDecode		  [In] Physical Port ID index
@param DecodeData	   [In] need decode data

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static PROTOCOL_FILTER_FRAM_ERR_E RTCM2FilterSingleData(MSG_DECODE_T* pDecode, unsigned char DecodeData)
{
	if (pDecode == NULL)
	{
		return PROTOCOL_FILTER_ERR;
	}

	unsigned int Index = 0;
	unsigned char preamble;
	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo = pDecode->ProtocolFilterInfo.pRTCM2FilterInfo;
	unsigned char* pBuffOut = pRTCM2FilterInfo->DecodeBuf.pBaseAddr;

	if ((DecodeData & 0xC0) != 0x40) /**< ignore if upper 2bit != 01 */
	{
		return PROTOCOL_FILTER_ERR;
	}

	for (Index = 0; Index < 6; Index++, DecodeData >>= 1) /**< decode 6-of-8 form */
	{
		pRTCM2FilterInfo->Word = (pRTCM2FilterInfo->Word << 1) + (DecodeData & 0x01);

		/** synchronize frame */
		if (pRTCM2FilterInfo->ByteNum == 0)
		{
			preamble=(unsigned char)(pRTCM2FilterInfo->Word >> 22);

			if (pRTCM2FilterInfo->Word & 0x40000000)
			{
				preamble ^= 0xFF; /**< decode preamble */
			}

			if (preamble != RTCM2_PREAMBLE)
			{
				continue;
			}

			/** check parity */
			if (0 != DecodeWord(pRTCM2FilterInfo->Word, pBuffOut))
			{
				continue;
			}

			pRTCM2FilterInfo->ByteNum=3;
			pRTCM2FilterInfo->BitNum=0;

			continue;
		}

		if (++pRTCM2FilterInfo->BitNum < 30)
		{
			continue;
		}
		else
		{
			pRTCM2FilterInfo->BitNum = 0;
		}

		/** check parity */
		if (0 != DecodeWord(pRTCM2FilterInfo->Word, pBuffOut + pRTCM2FilterInfo->ByteNum))
		{
			pRTCM2FilterInfo->ByteNum = 0;
			pRTCM2FilterInfo->Word &= 0x03;
			continue;
		}

		pRTCM2FilterInfo->ByteNum += 3;

		if (pRTCM2FilterInfo->ByteNum == 6)
		{
			pRTCM2FilterInfo->MsgLen = ((pBuffOut[5] >> 3) & 0x1f) * 3 + 6;
		}

		if (pRTCM2FilterInfo->ByteNum < pRTCM2FilterInfo->MsgLen)
		{
			continue;
		}

		if ((pRTCM2FilterInfo->ByteNum > RTCM2_MSG_LEN_MAX) || (pRTCM2FilterInfo->ByteNum <= RTCM2_MSG_LEN_MIN))
		{
			RTCM2FilterClear(pDecode);

			return PROTOCOL_FILTER_ERR;
		}

		pRTCM2FilterInfo->DecodeBuf.Len = pRTCM2FilterInfo->ByteNum;

		pRTCM2FilterInfo->ByteNum = 0;
		pRTCM2FilterInfo->LastWord = pRTCM2FilterInfo->Word;
		pRTCM2FilterInfo->Word &= 0x03;

		return PROTOCOL_FILTER_OK;
	}

	return PROTOCOL_WAIT_CONFIRM;
}

/**********************************************************************//**
@brief  RTCM2 Filter

@param pDecode          [In] Physical Port ID index
@param pDecodeData      [In] need decode data buffer
@param DataLen          [In] need decode data length
@param pOffset          [Out] remain decode data offset

@retval see PROTOCOL_FILTER_FRAM_ERR_E

@author CHC
@date 2024/11/15
@note
**************************************************************************/
PROTOCOL_FILTER_FRAM_ERR_E RTCM2Filter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset)
{
	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo;
	unsigned int DataIndex;
	PROTOCOL_FILTER_FRAM_ERR_E Result;
	unsigned int DecodeOffset;

	if (pDecode == NULL)
	{
		return PROTOCOL_FILTER_ERR;
	}

	pRTCM2FilterInfo = pDecode->ProtocolFilterInfo.pRTCM2FilterInfo;
	DecodeOffset = pRTCM2FilterInfo->DecodeOffset;

	for (DataIndex = DecodeOffset; DataIndex < DataLen; DataIndex++)
	{
		Result = RTCM2FilterSingleData(pDecode, pDecodeData[DataIndex]);
		if (Result == PROTOCOL_FILTER_OK)
		{
			*pOffset = DataIndex + 1;
			pRTCM2FilterInfo->DecodeOffset = 0;
			return PROTOCOL_FILTER_OK;
		}
		else if (Result == PROTOCOL_FILTER_ERR)
		{
			pRTCM2FilterInfo->DecodeOffset = 0;
			return PROTOCOL_FILTER_ERR;
		}
	}

	pRTCM2FilterInfo->DecodeOffset = DataLen;
	return PROTOCOL_WAIT_CONFIRM;
}

/**********************************************************************//**
@brief  RTCM2 decode information printf callback

@param pParseInfo		   [In] rtcm parse information
@param pSrcObj			  [In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/11/24
@note
**************************************************************************/
static int RTCM2DecodeInfoPrintfCallback(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	unsigned int DebugInfoLogLen = (unsigned int)strlen(pDebugInfoBuf);
	unsigned int BaseObsSatCountLogLen = (unsigned int)strlen(pBaseObsSatCountBuf);
	char* pEOL = "\r\n";

	if (!pSrcObj)
	{
		return -1;
	}

	if (DebugInfoLogLen > 0)
	{
#ifdef CONVERT_DEBUG_OUTPUT
		printf("%s\r\n", pDebugInfoBuf);
#endif
		if (pMsgConvertData->FpDebugInfoOutP)
		{
			fwrite(pDebugInfoBuf, DebugInfoLogLen, 1, pMsgConvertData->FpDebugInfoOutP);
			fwrite(pEOL, strlen(pEOL), 1, pMsgConvertData->FpDebugInfoOutP);
			fflush(pMsgConvertData->FpDebugInfoOutP);
			pDebugInfoBuf[0] = 0;
		}
	}

	if (BaseObsSatCountLogLen > 0)
	{
		if (pMsgConvertData->FpBaseObsSatCountOutP)
		{
			fwrite(pBaseObsSatCountBuf, BaseObsSatCountLogLen, 1, pMsgConvertData->FpBaseObsSatCountOutP);
			fwrite(pEOL, strlen(pEOL), 1, pMsgConvertData->FpBaseObsSatCountOutP);
			fflush(pMsgConvertData->FpBaseObsSatCountOutP);
			pBaseObsSatCountBuf[0] = 0;
		}
	}

	fflush(NULL);
#endif
	return 0;
}

/**********************************************************************//**
@brief  RTCM2 decode to signal overtime callback

@param TypeID		   [In] RTCM2 type id
@param pParseInfo		   [In] rtcm parse information
@param pSrcObj			  [In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM2DecodeToSignalOverTimeCallback(int TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
	MSG_DECODE_T* pDecode = (MSG_DECODE_T*)pSrcObj;

	return SignalDecodeObsConvert(RAW_DATA_TYPE_RTCM2, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM2);
}

/**********************************************************************//**
@brief  RTCM2 decode to signal

@param TypeID		   [In] RTCM2 type id
@param pCodec		   [In] pointer to codec
@param pParseInfo	   [In] rtcm parse information
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM2DecodeToSignal(unsigned int TypeID, MSG_DECODE_T* pDecode, RTCM_PARSE_INFO_T* pParseInfo, int SourceType)
{
	int Result = -1;

	const RTCM2_DECODE_FUNC_T RTCM2DecodeFuncsMap[] =
	{
		{NULL, NULL},
		{DecodeRTCM2Type1And9, NULL},
		{DecodeRTCM2Type2, SignalDecodeStationPositionConvert},
		{DecodeRTCM2Type3, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2Type1And9, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2Type14, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2Type16, NULL},
		{DecodeRTCM2Type17, SignalDecodeEphConvert},
		{DecodeRTCM2Type18, SignalDecodeObsConvert},
		{DecodeRTCM2Type19, SignalDecodeObsConvert},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2Type22, SignalDecodeStationPositionConvert},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2Type24, SignalDecodeStationPositionConvert},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type31, NULL},
		{DecodeRTCM2Type32, SignalDecodeStationPositionConvert},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{DecodeRTCM2DebugInfo, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type41And42, NULL},
		{DecodeRTCM2Type41And42, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type44, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2DebugInfo, NULL},
	};

	if ((pParseInfo == NULL) || (SourceType != GNSS_STRFMT_RTCM2))
		return -1;

	pParseInfo->RTCMInterimData.LastTypeID = TypeID;

	if ((TypeID <= 59) && (NULL != RTCM2DecodeFuncsMap[TypeID].RTCM2DecodeByTypeIDFunc))
	{
		Result = RTCM2DecodeFuncsMap[TypeID].RTCM2DecodeByTypeIDFunc(pParseInfo, pDecode, RTCM2DecodeToSignalOverTimeCallback);

		RTCM2DecodeInfoPrintfCallback(pParseInfo, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
		if ((Result > 0) && (NULL != RTCM2DecodeFuncsMap[TypeID].ConvertFunc))
		{
			RTCM2DecodeFuncsMap[TypeID].ConvertFunc(RAW_DATA_TYPE_RTCM2, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM2);
		}
	}

	return Result;
}

/**********************************************************************//**
@brief  RTCM2 decode to RINEX overtime callback

@param TypeID		   [In] RTCM2 type id
@param pParseInfo		   [In] rtcm parse information
@param pSrcObj			  [In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM2DecodeToRINEXOverTimeCallback(int TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
	MSG_DECODE_T* pDecode = (MSG_DECODE_T*)pSrcObj;

	return DataDecodeObsConvertRINEX(RAW_DATA_TYPE_RTCM2, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM2);
}

/**********************************************************************//**
@brief  RTCM2 decode to RINEX message

@param TypeID		   [In] RTCM2 type id
@param pCodec		   [In] pointer to codec
@param pParseInfo	   [In] rtcm parse information
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM2DecodeToRINEX(unsigned int TypeID, MSG_DECODE_T* pDecode, RTCM_PARSE_INFO_T* pParseInfo, int SourceType)
{
	int Result = -1;

	const RTCM2_DECODE_FUNC_T RTCM2DecodeFuncsMap[] =
	{
		{NULL, NULL},
		{DecodeRTCM2Type1And9, NULL},
		{DecodeRTCM2Type2, NULL},
		{DecodeRTCM2Type3, DataDecodeStationPositionConvertShow},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type1And9, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type14, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type16, NULL},
		{DecodeRTCM2Type17, DataDecodeUniEphConvertRINEX},
		{DecodeRTCM2Type18, DataDecodeObsConvertRINEX},
		{DecodeRTCM2Type19, DataDecodeObsConvertRINEX},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type22, DataDecodeStationPositionConvertShow},
		{NULL, NULL},
		{DecodeRTCM2Type24, DataDecodeStationPositionConvertShow},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type31, NULL},
		{DecodeRTCM2Type32, DataDecodeStationPositionConvertShow},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type41And42, NULL},
		{DecodeRTCM2Type41And42, NULL},
		{NULL, NULL},
		{DecodeRTCM2Type44, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
	};

	if ((pParseInfo == NULL) || (SourceType != GNSS_STRFMT_RTCM2))
		return -1;

	if ((TypeID <= 59) && (NULL != RTCM2DecodeFuncsMap[TypeID].RTCM2DecodeByTypeIDFunc))
	{
		Result = RTCM2DecodeFuncsMap[TypeID].RTCM2DecodeByTypeIDFunc(pParseInfo, pDecode, RTCM2DecodeToRINEXOverTimeCallback);
		if ((Result > 0) && (NULL != RTCM2DecodeFuncsMap[TypeID].ConvertFunc))
			RTCM2DecodeFuncsMap[TypeID].ConvertFunc(RAW_DATA_TYPE_RTCM2, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM2);
	}

	return Result;
}

/**********************************************************************//**
@brief  RTCM2 decode

@param pCodec		   [In] pointer to codec
@param SourceType	   [In] data source type

@retval <0:error, =0:success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int RTCM2Decode(MSG_DECODE_T* pDecode, int SourceType)
{
	if (pDecode == NULL)
	{
		return -1;
	}

	int Result = -1;
	unsigned int TypeID = 0;
	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo = pDecode->ProtocolFilterInfo.pRTCM2FilterInfo;
	RTCM_PARSE_INFO_T* pParseInfo = pDecode->ProtocolFilterInfo.pRTCMParseInfo;
	unsigned char* pBufIn = (unsigned char*)pRTCM2FilterInfo->DecodeBuf.pBaseAddr;
	double ZCount;

	pParseInfo->DecodeBuf = pRTCM2FilterInfo->DecodeBuf;

	TypeID = GetUnsignedBits(pBufIn, 8, 6);

	if ((ZCount = GetUnsignedBits(pBufIn, 24, 13) * 0.6) >= 3600.0)
	{
		return -1;
	}
	RTCM2AdjHour(pParseInfo, ZCount, TypeID);
	if (RTCM2_SEPRATE_TIME_ENABLE)
	{
		RTCM2SeparateTime(pParseInfo, TypeID);
	}

	if (pDecode->ProtocolFilterInfo.DecodeMode == DECODE_MODE_CONVERT_TO_SIGNAL)
		Result = RTCM2DecodeToSignal(TypeID, pDecode, pParseInfo, SourceType);
	else if (pDecode->ProtocolFilterInfo.DecodeMode == DECODE_MODE_CONVERT_TO_RINEX)
		Result = RTCM2DecodeToRINEX(TypeID, pDecode, pParseInfo, SourceType);

	return Result;
}

/**********************************************************************//**
@brief  RTCM2 filter deinit

@param pRTCM2FilterInfo	 [In] rtcm2 filter info handle

@retval 0:success

@author CHC
@date 2023/06/21
@note
**************************************************************************/
int RTCM2FilterDeinit(RTCM2_FILTER_INFO_T* pRTCM2FilterInfo)
{
	FREE(pRTCM2FilterInfo);
	pRTCM2FilterInfo = NULL;

	return 0;
}

/**********************************************************************//**
@brief  RTCM2 filter init

@retval success: return rtcm2 filter info handle, else return null

@author CHC
@date 2023/03/09
@note
**************************************************************************/
RTCM2_FILTER_INFO_T* RTCM2FilterInit(void)
{
	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo = NULL;

	int InfoSize = MEM_CEILBY8(sizeof(RTCM2_FILTER_INFO_T));
	int DecodeBufLen = MEM_CEILBY8(RTCM2_MSG_LEN_MAX);
	int TotalSize = InfoSize + DecodeBufLen;

	pRTCM2FilterInfo = MALLOC(TotalSize);
	if (pRTCM2FilterInfo == NULL)
	{
		return NULL;
	}

	MEMSET(pRTCM2FilterInfo, 0x00, TotalSize);

	pRTCM2FilterInfo->DecodeBuf.Len = 0;
	pRTCM2FilterInfo->DecodeBuf.pBaseAddr = ((unsigned char*)(pRTCM2FilterInfo)) + InfoSize;

	return pRTCM2FilterInfo;
}

#endif
