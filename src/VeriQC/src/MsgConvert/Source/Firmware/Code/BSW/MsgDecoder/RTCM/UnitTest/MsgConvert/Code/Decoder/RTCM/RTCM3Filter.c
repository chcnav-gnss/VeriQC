#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM3 Filter Module
*-
@file   RTCM3Filter.c
@author CHC
@date   2023/04/14
@brief

**************************************************************************/
#include "OSDependent.h"
#include "RTCM3Filter.h"
#include "BSW/MsgDecoder/RTCM/RTCM3Decoder.h"
#include "RTCMDecodeManage.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "Common/DataTypes.h"
#include "Common/CheckSum/CRC24Q.h"
#include "Common/CommonFuncs.h"
#include "Encoder/RINEX/RINEXConvert.h"

#include "../../MsgConvertDataTypes.h"

#define RTCM3_MSG_MAX_NUM 64

#define	RTCM3_DECODE_OBS_FILTER_ENABLE			(0)

typedef enum _RTCM3_FILTER_STAGE_E
{
	RTCM3_STAGE_PREAMBLE = 0,
	RTCM3_STAGE_MSGLEN = 1,
	RTCM3_STAGE_CHK = 2,
	RTCM3_STAGE_MAX = 3,
} RTCM3_FILTER_STAGE_E;

typedef int (*RTCM3_DECODE_BY_TYPEID_FUNC)(RTCM_PARSE_INFO_T*, void*, RTCM3_DECODE_OVERTIME_FUNC);
typedef int (*RTCM3_DECODE_INFO_PRINTF_CALLBACK_FUNC)(RTCM_PARSE_INFO_T*, void*);
typedef int (*CONVERT_FUNC)(int, int, void*, void*, int);

typedef struct _RTCM3_DECODE_FUNC_T
{
	RTCM3_DECODE_BY_TYPEID_FUNC RTCM3DecodeByTypeIDFunc;
	RTCM3_DECODE_INFO_PRINTF_CALLBACK_FUNC DecodeInfoPrintfCallbackFunc;
	CONVERT_FUNC ConvertFunc;
} RTCM3_DECODE_FUNC_T;

/** Min SatID of PPP Msg */
static const unsigned char s_RMMinPRNMap[RM_RANGE_SYS_OTHER] =
{
	MIN_GPS_RANGE_PRN,
	MIN_GLONASS_RANGE_PRN,
	MIN_SBAS_GEO_RANGE_PRN,
	MIN_GALILEO_RANGE_PRN,
	MIN_BDS_RANGE_PRN,
	MIN_QZSS_RANGE_PRN,
	MIN_NAVIC_RANGE_PRN,
};

/**********************************************************************//**
@brief  RTCM3 Filter information clear

@param pCodec	   [In] Physical Port ID index

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int RTCM3FilterClear(MSG_DECODE_T* pDecode)
{
	if (pDecode == NULL)
	{
		return -1;
	}

	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo = pDecode->ProtocolFilterInfo.pRTCM3FilterInfo;

	pRTCM3FilterInfo->Stage = RTCM3_STAGE_PREAMBLE;
	pRTCM3FilterInfo->DecodeBuf.Len = 0;
	pRTCM3FilterInfo->MsgLen = 0;

	pRTCM3FilterInfo->DecodeOffset = 0;

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 Filter Single Data

@param pDecode	  [In] Physical Port ID index
@param DecodeData	  [In] need decode data

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static PROTOCOL_FILTER_FRAM_ERR_E RTCM3FilterSingleData(MSG_DECODE_T* pDecode, unsigned char DecodeData)
{
	if (pDecode == NULL)
	{
		return PROTOCOL_FILTER_ERR;
	}

	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo = pDecode->ProtocolFilterInfo.pRTCM3FilterInfo;
	unsigned char* pBuffOut = pRTCM3FilterInfo->DecodeBuf.pBaseAddr;
	unsigned int DataSize = pRTCM3FilterInfo->DecodeBuf.Len;
	unsigned int MsgLen = 0;
	unsigned int MsgCRC = 0;

	if (pRTCM3FilterInfo->Stage == RTCM3_STAGE_PREAMBLE)
	{
		if (DecodeData == RTCM3_PREAMBLE)
		{
			*pBuffOut = DecodeData;

			pRTCM3FilterInfo->DecodeBuf.Len = 1;
			pRTCM3FilterInfo->Stage = RTCM3_STAGE_MSGLEN;

			return PROTOCOL_WAIT_CONFIRM;
		}
		else
		{
			RTCM3FilterClear(pDecode);

			return PROTOCOL_FILTER_ERR;
		}
	}
	else if (pRTCM3FilterInfo->Stage == RTCM3_STAGE_MSGLEN)
	{
		if (DataSize < 3)
		{
			*(pBuffOut + DataSize) = DecodeData;

			if (++DataSize == 3)
			{
				MsgLen = GetUnsignedBits(pBuffOut, 14, 10);

				pRTCM3FilterInfo->Stage = RTCM3_STAGE_CHK;
				pRTCM3FilterInfo->MsgLen = MsgLen;
			}

			pRTCM3FilterInfo->DecodeBuf.Len = DataSize;
		}

		return PROTOCOL_WAIT_CONFIRM;
	}
	else if (pRTCM3FilterInfo->Stage == RTCM3_STAGE_CHK)
	{
		MsgLen = pRTCM3FilterInfo->MsgLen;
		if (DataSize < (MsgLen + 6))
		{
			*(pBuffOut + DataSize) = DecodeData;

			if (++DataSize == (MsgLen + 6))
			{
				MsgCRC = GetUnsignedBits(pBuffOut, (MsgLen+3) << 3, 24);
				if (MsgCRC == CalcCRC24Q(pBuffOut, MsgLen+3))
				{
					pRTCM3FilterInfo->Stage = RTCM3_STAGE_PREAMBLE;
					pRTCM3FilterInfo->MsgLen = 0;
					pRTCM3FilterInfo->DecodeBuf.Len = DataSize;

					return PROTOCOL_FILTER_OK;
				}
				else
				{
					RTCM3FilterClear(pDecode);

					return PROTOCOL_FILTER_ERR;
				}
			}

			if (DataSize > RTCM3_MSG_LEN_MAX)
			{
				RTCM3FilterClear(pDecode);

				return PROTOCOL_FILTER_ERR;
			}

			pRTCM3FilterInfo->DecodeBuf.Len = DataSize;
		}
		else
		{
			RTCM3FilterClear(pDecode);

			return PROTOCOL_FILTER_ERR;
		}
	}

	return PROTOCOL_WAIT_CONFIRM;
}

/**********************************************************************//**
@brief  RTCM3 Filter

@param pDecode          [In] Physical Port ID index
@param pDecodeData      [In] need decode data buffer
@param DataLen          [In] need decode data length
@param pOffset          [Out] remain decode data offset

@retval see PROTOCOL_FILTER_FRAM_ERR_E

@author CHC
@date 2024/11/15
@note
**************************************************************************/
PROTOCOL_FILTER_FRAM_ERR_E RTCM3Filter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset)
{
	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo;
	unsigned int DataIndex;
	PROTOCOL_FILTER_FRAM_ERR_E Result;
	unsigned int DecodeOffset;

	if (pDecode == NULL)
	{
		return PROTOCOL_FILTER_ERR;
	}

	pRTCM3FilterInfo = pDecode->ProtocolFilterInfo.pRTCM3FilterInfo;
	DecodeOffset = pRTCM3FilterInfo->DecodeOffset;

	for (DataIndex = DecodeOffset; DataIndex < DataLen; DataIndex++)
	{
		Result = RTCM3FilterSingleData(pDecode, pDecodeData[DataIndex]);
		if (Result == PROTOCOL_FILTER_OK)
		{
			*pOffset = DataIndex + 1;
			pRTCM3FilterInfo->DecodeOffset = 0;
			return PROTOCOL_FILTER_OK;
		}
		else if (Result == PROTOCOL_FILTER_ERR)
		{
			pRTCM3FilterInfo->DecodeOffset = 0;
			return PROTOCOL_FILTER_ERR;
		}
	}

	pRTCM3FilterInfo->DecodeOffset = DataLen;
	return PROTOCOL_WAIT_CONFIRM;
}

/**********************************************************************//**
@brief  RTCM3 decode obsdata freq or satellite filter

@param pRangeData		[In] range data
@param pSrcObj			[In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/11/24
@note
**************************************************************************/
static int RTCM3DecodeObsDataFilter(RM_RANGE_LOG_T* pRangeData, void* pSrcObj)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RM_RANGE_DATA_T ObsData[RM_RANGE_MAX_OBS_NUM];
	RM_RANGE_DATA_T* pPerObsData;
	unsigned int ObsIndex;
	unsigned int ObsCount = 0;
	unsigned int RMSysID = 0;
	unsigned int SignalType;
	unsigned int* pObsFreqFilter;
	UINT64* pObsSatFilter;
	int NeedObsData = 0;

	if ((pMsgConvertData->ObsFreqFilterEnable) || (pMsgConvertData->ObsSatFilterEnable))
	{
		pObsFreqFilter = &pMsgConvertData->ObsFreqFilter[0];
		pObsSatFilter = &pMsgConvertData->ObsSatFilter[0];
		pPerObsData = pRangeData->ObsData;

		for (ObsIndex = 0; ObsIndex < pRangeData->ObsNum; ObsIndex++)
		{
			RMSysID = GET_CH_TRACK_STATUS_SATSYSTEM(pPerObsData->ChnTrackStatus);
			SignalType = GET_CH_TRACK_STATUS_SIGNALTYPE(pPerObsData->ChnTrackStatus);

			if (RMSysID >= RM_RANGE_SYS_OTHER)
			{
				continue;
			}

			if (pMsgConvertData->ObsSatFilterEnable)
			{
				if ((pObsSatFilter[RMSysID] == 0xFFFFFFFFFFFFFFFF) || (pObsSatFilter[RMSysID] & ((UINT64)1 << (pPerObsData->PRN - s_RMMinPRNMap[RMSysID]))))
				{
					NeedObsData = 1;
				}
				else
				{
					NeedObsData = 0;
				}
			}

			if ((pMsgConvertData->ObsFreqFilterEnable) && (NeedObsData != 0))
			{
				if ((pObsFreqFilter[RMSysID] == 0xFFFFFFFF) || (pObsFreqFilter[RMSysID] & (1 << SignalType)))
				{
					NeedObsData = 2;
				}
				else
				{
					NeedObsData = 0;
				}
			}

			if (NeedObsData != 0)
			{
				MEMCPY(&ObsData[ObsCount], pPerObsData, sizeof(RM_RANGE_DATA_T));
				ObsCount++;
			}

			pPerObsData++;
			NeedObsData = 0;
		}

		if (ObsCount != pRangeData->ObsNum)
		{
			pRangeData->ObsNum = ObsCount;
			MEMCPY(pRangeData->ObsData, ObsData, ObsCount*sizeof(RM_RANGE_DATA_T));
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 decode information printf callback

@param pParseInfo		   [In] rtcm parse information
@param pSrcObj			  [In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/11/24
@note
**************************************************************************/
static int RTCM3DecodeInfoPrintfCallback(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	unsigned int DebugInfoLogLen = (unsigned int)strlen(pDebugInfoBuf);
	unsigned int BaseObsSatCountLogLen = (unsigned int)strlen(pBaseObsSatCountBuf);
	char* pEOL = "\r\n";

	if (pMsgConvertData == NULL)
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
@brief  RTCM3 decode to signal overtime callback

@param TypeID			[In] RTCM type id
@param pParseInfo		[In] rtcm parse information
@param pSrcObj			[In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM3DecodeToSignalOverTimeCallback(int TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
	MSG_DECODE_T* pDecode = (MSG_DECODE_T*)pSrcObj;

	int RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_MSM;
	if (((TypeID >= 1001) && (TypeID <= 1004)) ||((TypeID >= 1009) && (TypeID <= 1012)))
	{
		RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_RTKDATA;
	}

	return SignalDecodeObsConvert(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RawObsDataType);
}

/**********************************************************************//**
@brief  RTCM3 decode VRS overtime callback

@param TypeID			[In] RTCM type id
@param pParseInfo		[In] rtcm parse information
@param pSrcObj			[In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM3DecodeVRSOverTimeCallback(int TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
	MSG_DECODE_T* pDecode = (MSG_DECODE_T*)pSrcObj;

	return -1;
}

/**********************************************************************//**
@brief  RTCM3 decode to signal

@param TypeID		   [In] RTCM3 type id
@param pCodec		   [In] pointer to codec
@param pParseInfo	   [In] rtcm parse information
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM3DecodeToSignal(unsigned TypeID, MSG_DECODE_T* pDecode, RTCM_PARSE_INFO_T* pParseInfo, int SourceType)
{
	int Result = -1;
	unsigned int FuncsIndex;

	const static RTCM3_DECODE_FUNC_T RTCM3DecodeFuncsMap[] =
	{
		{RTCM3DecodeType1001, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1002, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1003, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1004, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1005, RTCM3DecodeInfoPrintfCallback, SignalDecodeAntennaInfoConvert},
		{RTCM3DecodeType1006, RTCM3DecodeInfoPrintfCallback, SignalDecodeAntennaInfoConvert},
		{RTCM3DecodeType1007, RTCM3DecodeInfoPrintfCallback, NULL},
		{RTCM3DecodeType1008, RTCM3DecodeInfoPrintfCallback, NULL},
		{RTCM3DecodeType1009, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1010, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1011, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1012, RTCM3DecodeInfoPrintfCallback, SignalDecodeObsConvert},
		{RTCM3DecodeType1013, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1019, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{RTCM3DecodeType1020, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1029, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeType1033, RTCM3DecodeInfoPrintfCallback, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1041, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{RTCM3DecodeType1042, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1044, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{RTCM3DecodeType1045, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{RTCM3DecodeType1046, RTCM3DecodeInfoPrintfCallback, SignalDecodeEphConvert},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
	};

	const static RTCM3_DECODE_FUNC_T RTCM3CRSDecodeFuncsMap[] =
	{
		{RTCM3DecodeType1300, NULL, NULL},
		{RTCM3DecodeType1301, NULL, NULL},
		{RTCM3DecodeType1302, NULL, NULL},
		{RTCM3DecodeType1303, NULL, NULL},
		{RTCM3DecodeType1304, NULL, NULL},
	};

	if ((pParseInfo == NULL) || (SourceType != GNSS_STRFMT_RTCM3))
	{
		return -1;
	}
	pParseInfo->RTCMInterimData.LastTypeID = TypeID;

	if((TypeID > 1070) && (TypeID <= 1137)) /**< MSM */
	{
		RTCM_SYSTEM_E RTCMSysID = (RTCM_SYSTEM_E)((TypeID - 1070) / 10);
		Result = RTCM3DecodeTypeMSM(RTCMSysID, TypeID, pParseInfo, pDecode, RTCM3DecodeToSignalOverTimeCallback);

		if (pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj != NULL)
		{
			RTCM3DecodeInfoPrintfCallback(pParseInfo, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
		}
		if (Result > 0)
		{
#if (RTCM3_DECODE_OBS_FILTER_ENABLE != 0)
			GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
			RTCM3DecodeObsDataFilter(&pObsData->RangeData, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
#endif
			SignalDecodeObsConvert(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM_MSM);
		}
	}
	else if ((TypeID > 1000) && (TypeID <= 1068)) /**< or map 1005 6 7 8 */
	{
		FuncsIndex = TypeID - 1001;
		if (NULL != RTCM3DecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc)
		{
			int RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_MSM;
			if (((TypeID >= 1001) && (TypeID <= 1004)) ||((TypeID >= 1009) && (TypeID <= 1012)))
			{
				RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_RTKDATA;
			}
			Result = RTCM3DecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc(pParseInfo, pDecode, RTCM3DecodeToSignalOverTimeCallback);

			if ((NULL != RTCM3DecodeFuncsMap[FuncsIndex].DecodeInfoPrintfCallbackFunc) && (NULL != pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj))
			{
				RTCM3DecodeFuncsMap[FuncsIndex].DecodeInfoPrintfCallbackFunc(pParseInfo, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
			}
			if ((Result > 0) && (NULL != RTCM3DecodeFuncsMap[FuncsIndex].ConvertFunc))
			{
				RTCM3DecodeFuncsMap[FuncsIndex].ConvertFunc(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RawObsDataType);
			}
		}
	}
	else if ((TypeID >= 1300) && (TypeID <= 1304))
	{
		FuncsIndex = TypeID - 1300;
		if (NULL != RTCM3CRSDecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc)
		{
			Result = RTCM3CRSDecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc(pParseInfo, pDecode, NULL);

			if ((NULL != RTCM3CRSDecodeFuncsMap[FuncsIndex].DecodeInfoPrintfCallbackFunc) && (NULL != pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj))
			{
				RTCM3CRSDecodeFuncsMap[FuncsIndex].DecodeInfoPrintfCallbackFunc(pParseInfo, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
			}
			if ((Result > 0) && (NULL != RTCM3CRSDecodeFuncsMap[FuncsIndex].ConvertFunc))
			{
				RTCM3CRSDecodeFuncsMap[FuncsIndex].ConvertFunc(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM_MSM);
			}
		}
	}
	else if (1230 == TypeID)
	{
		Result = RTCM3DecodeType1230(pParseInfo, pDecode, NULL);
		if (pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj != NULL)
		{
			RTCM3DecodeInfoPrintfCallback(pParseInfo, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
		}
	}
	else
	{
		/*@todo unknown message warning*/
	}

	return Result;
}

/**********************************************************************//**
@brief  RTCM3 decode to RINEX overtime callback

@param TypeID			[In] RTCM type id
@param pParseInfo		[In] rtcm parse information
@param pSrcObj			[In] pointer to source object

@retval <0 error, =0 success

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM3DecodeToRINEXOverTimeCallback(int TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj)
{
	MSG_DECODE_T* pDecode = (MSG_DECODE_T*)pSrcObj;

	int RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_MSM;
	if (((TypeID >= 1001) && (TypeID <= 1004)) ||((TypeID >= 1009) && (TypeID <= 1012)))
	{
		RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_RTKDATA;
	}

	return DataDecodeObsConvertRINEX(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RawObsDataType);
}

/**********************************************************************//**
@brief  RTCM3 decode to RINEX message

@param TypeID		   [In] RTCM3 type id
@param pCodec		   [In] pointer to codec
@param pParseInfo	   [In] rtcm parse information
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int RTCM3DecodeToRINEX(unsigned TypeID, MSG_DECODE_T* pDecode, RTCM_PARSE_INFO_T* pParseInfo, int SourceType)
{
	int Result = -1;
	unsigned int FuncsIndex;

	const static RTCM3_DECODE_FUNC_T RTCM3DecodeFuncsMap[] =
	{
		{RTCM3DecodeType1001, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1002, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1003, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1004, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1005, NULL, DataDecodeAntennaInfoConvertShow},
		{RTCM3DecodeType1006, NULL, DataDecodeAntennaInfoConvertShow},
		{RTCM3DecodeType1007, NULL, DataDecodeReceiverInfoConvertShow},
		{RTCM3DecodeType1008, NULL, DataDecodeReceiverInfoConvertShow},
		{RTCM3DecodeType1009, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1010, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1011, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1012, NULL, DataDecodeObsConvertRINEX},
		{RTCM3DecodeType1013, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1019, NULL, DataDecodeUniEphConvertRINEX},
		{RTCM3DecodeType1020, NULL, DataDecodeGLOEphConvertRINEX},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1029, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeType1033, NULL, DataDecodeReceiverInfoConvertShow},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1041, NULL, DataDecodeUniEphConvertRINEX},
		{RTCM3DecodeType1042, NULL, DataDecodeUniEphConvertRINEX},
		{NULL, NULL, NULL},
		{RTCM3DecodeType1044, NULL, DataDecodeUniEphConvertRINEX},
		{RTCM3DecodeType1045, NULL, DataDecodeUniEphConvertRINEX},
		{RTCM3DecodeType1046, NULL, DataDecodeUniEphConvertRINEX},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{NULL, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
		{RTCM3DecodeDebugInfo, NULL, NULL},
	};

	const static RTCM3_DECODE_FUNC_T RTCM3CRSDecodeFuncsMap[] =
	{
		{RTCM3DecodeType1300, NULL, NULL},
		{RTCM3DecodeType1301, NULL, NULL},
		{RTCM3DecodeType1302, NULL, NULL},
		{RTCM3DecodeType1303, NULL, NULL},
		{RTCM3DecodeType1304, NULL, NULL},
	};

	if ((pParseInfo == NULL) || (SourceType != GNSS_STRFMT_RTCM3))
	{
		return -1;
	}

	if((TypeID > 1070) && (TypeID <= 1137)) /**< MSM */
	{
		RTCM_SYSTEM_E RTCMSysID = (RTCM_SYSTEM_E)((TypeID - 1070) / 10);
		Result = RTCM3DecodeTypeMSM(RTCMSysID, TypeID, pParseInfo, pDecode, RTCM3DecodeToRINEXOverTimeCallback);
		if (Result > 0)
		{
#if (RTCM3_DECODE_OBS_FILTER_ENABLE != 0)
			GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
			RTCM3DecodeObsDataFilter(&pObsData->RangeData, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj);
#endif
			DataDecodeObsConvertRINEX(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM_MSM);
		}
	}
	else if ((TypeID > 1000) && (TypeID <= 1068)) /**< or map 1005 6 7 8 */
	{
		FuncsIndex = TypeID - 1001;
		if (NULL != RTCM3DecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc)
		{
			int RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_MSM;
			if (((TypeID >= 1001) && (TypeID <= 1004)) ||((TypeID >= 1009) && (TypeID <= 1012)))
			{
				RawObsDataType = RAW_OBS_DATA_TYPE_RTCM_RTKDATA;
			}
			Result = RTCM3DecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc(pParseInfo, pDecode, RTCM3DecodeToRINEXOverTimeCallback);
			if ((Result > 0) && (NULL != RTCM3DecodeFuncsMap[FuncsIndex].ConvertFunc))
			{
				RTCM3DecodeFuncsMap[FuncsIndex].ConvertFunc(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RawObsDataType);
			}
		}
	}
	else if ((TypeID >= 1300) && (TypeID <= 1304))
	{
		FuncsIndex = TypeID - 1300;
		if (NULL != RTCM3CRSDecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc)
		{
			Result = RTCM3CRSDecodeFuncsMap[FuncsIndex].RTCM3DecodeByTypeIDFunc(pParseInfo, pDecode, NULL);
			if ((Result > 0) && (NULL != RTCM3CRSDecodeFuncsMap[FuncsIndex].ConvertFunc))
			{
				RTCM3CRSDecodeFuncsMap[FuncsIndex].ConvertFunc(RAW_DATA_TYPE_RTCM3, TypeID, pDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj, pParseInfo->ParsedDataBuf.pBaseAddr, RAW_OBS_DATA_TYPE_RTCM_MSM);
			}
		}
	}
	else if (1230 == TypeID)
	{
		Result = RTCM3DecodeType1230(pParseInfo, pDecode, NULL);
	}
	else
	{
		/*@todo unknown message warning*/
	}

	return Result;
}

/**********************************************************************//**
@brief  RTCM3 decode

@param pCodec		   [In] pointer to codec
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int RTCM3Decode(MSG_DECODE_T* pDecode, int SourceType)
{
	if (pDecode == NULL)
		return -1;

	int Result = -1;
	unsigned int TypeID = 0;
	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo = pDecode->ProtocolFilterInfo.pRTCM3FilterInfo;
	RTCM_PARSE_INFO_T* pParseInfo = pDecode->ProtocolFilterInfo.pRTCMParseInfo;
	unsigned char* pBufIn = (unsigned char*)pRTCM3FilterInfo->DecodeBuf.pBaseAddr;

	pParseInfo->DecodeBuf = pRTCM3FilterInfo->DecodeBuf;

	TypeID = GetUnsignedBits(pBufIn, 24, 12);

	if (pDecode->ProtocolFilterInfo.DecodeMode == DECODE_MODE_CONVERT_TO_SIGNAL)
		Result = RTCM3DecodeToSignal(TypeID, pDecode, pParseInfo, SourceType);
	else if (pDecode->ProtocolFilterInfo.DecodeMode == DECODE_MODE_CONVERT_TO_RINEX)
		Result = RTCM3DecodeToRINEX(TypeID, pDecode, pParseInfo, SourceType);

	return Result;
}

/**********************************************************************//**
@brief  RTCM3 filter deinit

@param pRTCM3FilterInfo	 [In] rtcm3 filter info handle

@retval 0:success

@author CHC
@date 2023/06/21
@note
**************************************************************************/
int RTCM3FilterDeinit(RTCM3_FILTER_INFO_T* pRTCM3FilterInfo)
{
	FREE(pRTCM3FilterInfo);
	pRTCM3FilterInfo = NULL;

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 filter init

@retval success: return rtcm3 filter info handle, else return null

@author CHC
@date 2023/03/09
@note
**************************************************************************/
RTCM3_FILTER_INFO_T* RTCM3FilterInit(void)
{
	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo = NULL;

	int InfoSize = MEM_CEILBY8(sizeof(RTCM3_FILTER_INFO_T));
	int DecodeBufLen = MEM_CEILBY8(RTCM3_MSG_LEN_MAX);
	int TotalSize = InfoSize + DecodeBufLen;

	pRTCM3FilterInfo = MALLOC(TotalSize);
	if (pRTCM3FilterInfo == NULL)
	{
		return NULL;
	}

	MEMSET(pRTCM3FilterInfo, 0x00, InfoSize);

	pRTCM3FilterInfo->DecodeBuf.Len = 0;
	pRTCM3FilterInfo->DecodeBuf.pBaseAddr = ((unsigned char*)(pRTCM3FilterInfo)) + InfoSize;

	return pRTCM3FilterInfo;
}

#endif
