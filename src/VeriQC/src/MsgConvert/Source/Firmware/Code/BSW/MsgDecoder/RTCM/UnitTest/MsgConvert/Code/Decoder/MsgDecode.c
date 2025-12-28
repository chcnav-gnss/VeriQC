#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg decode data structure conversion
*-
@file	MsgDecode.c
@author  CHC
@date	2023/03/13
@brief   msg decode data structure conversion

**************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "MsgDecode.h"
#include "./RTCM/RTCM3Filter.h"
#include "./RTCM/RTCM2Filter.h"
#include "Common/CommonFuncs.h"

#include "./RawMsgAscii/RawMsgAsciiFilter.h"
#include "./RawMsgBinary/RawMsgBinaryFilter.h"
#include "./Rinex/RinexFilter.h"

#include "./HRCX/HRCXFilter.h"

#include "./CHCRawMsgBinary/CHCRawMsgBinaryFilter.h"
#include "./UNRawMsgBinary/UNRawMsgBinaryFilter.h"

PROTOCOL_DECODE_FUNC_T s_ProtocolDecodeFuncTabl[] = {
	{MSG_DECODE_TYPE_RAWMSGA,		RawMsgAsciiFilter,		RawMsgAsciiFilterClear,		RawMsgAsciiDecode},
	{MSG_DECODE_TYPE_RAWMSGB,		RawMsgBinaryFilter,		RawMsgBinaryFilterClear,	RawMsgBinaryDecode},
	{MSG_DECODE_TYPE_RTCM3,			RTCM3Filter,			RTCM3FilterClear,			RTCM3Decode},
	{MSG_DECODE_TYPE_RTCM2,			RTCM2Filter,			RTCM2FilterClear,			RTCM2Decode},
	{MSG_DECODE_TYPE_RINEX,			RinexFilter,			RinexFilterClear,			RinexDecode},
	{MSG_DECODE_TYPE_HRCX,			HRCXFilter,				HRCXFilterClear,			HRCXDecode},
	{MSG_DECODE_TYPE_UNRAWMSGB,	    UNRawMsgBinaryFilter,	UNRawMsgBinaryFilterClear,	UNRawMsgBinaryDecode},
	{MSG_DECODE_TYPE_CHCRAWMSGB,	CHCRawMsgBinaryFilter,	CHCRawMsgBinaryFilterClear,	CHCRawMsgBinaryDecode},
};

/**********************************************************************//**
@brief Msg Frame Match From CycleBuf

@param pDecode		  [In] pointer to decode

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int MsgFilterClearAll(MSG_DECODE_T* pDecode)
{
	unsigned int FuncIndex;

	for (FuncIndex = 0; FuncIndex < ARRAY_SIZE(s_ProtocolDecodeFuncTabl); FuncIndex++)
	{
		s_ProtocolDecodeFuncTabl[FuncIndex].ProtocolFilterClear(pDecode);
	}

	return 0;
}

/**********************************************************************//**
@brief get message filter offset

@param pDecode		  [In] pointer to decode
@param pOffset		  [In] message filter offset

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int GetMsgFilterOffset(MSG_DECODE_T* pDecode, unsigned int* pOffset)
{
	if (!pDecode)
	{
		return -1;
	}

	*pOffset = pDecode->FilterOffset;

	return 0;
}

/**********************************************************************//**
@brief set message filter offset

@param pDecode		  [In] pointer to decode
@param Offset		  [In] message filter offset

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int SetMsgFilterOffset(MSG_DECODE_T* pDecode, unsigned int Offset)
{
	if (!pDecode)
	{
		return -1;
	}

	pDecode->FilterOffset = Offset;

	return 0;
}

/**********************************************************************//**
@brief Msg Frame Match From CycleBuf

@param pDecode		  [In] pointer to decode
@param DecodeType	   [In] decode type
@param SourceType	   [In] data source type

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
static int MsgFrameMatchFromCycleBuf(MSG_DECODE_T* pDecode, int DecodeType, int SourceType)
{
	PROTOCOL_FILTER_FRAM_ERR_E ResFilter = PROTOCOL_FILTER_ERR;
	unsigned int FilterOffset = 0, FuncIndex;
	unsigned int ErrMask = 0, WaitConfirmMask = 0;
	unsigned char DecodeData = 0;
	unsigned int DataSize;
	unsigned char* pBufIn;
	unsigned int DecodeDataSize;
	CYCLE_BUFF_CTRL_T* pCycleBuffCtrl;
	unsigned int ProtocolMask = ((1 << ARRAY_SIZE(s_ProtocolDecodeFuncTabl)) - 1);

	if ((!pDecode) || (GetMsgFilterOffset(pDecode, &FilterOffset) < 0))
	{
		return -1;
	}

	pCycleBuffCtrl = &pDecode->pMsgPort->CycleBuffCtrl;

	DataSize = CycleBufGetDataSize(pCycleBuffCtrl);

	pBufIn = pDecode->pMsgPort->DecodeBuf.pBaseAddr;

	CycleBufGetData(pCycleBuffCtrl, (char *)pBufIn, DataSize);

	DecodeDataSize = DataSize;

	while (DecodeDataSize > 0)
	{
		for (FuncIndex = 0; FuncIndex < ARRAY_SIZE(s_ProtocolDecodeFuncTabl); FuncIndex++)
		{
			if (!(DecodeType & s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType))
			{
				ErrMask |= (1 << FuncIndex);
			}
			else
			{
				ResFilter = s_ProtocolDecodeFuncTabl[FuncIndex].ProtocolFilter(pDecode, pBufIn, DecodeDataSize, &FilterOffset);

				if (PROTOCOL_FILTER_OK == ResFilter)
				{
					s_ProtocolDecodeFuncTabl[FuncIndex].ProtocolDecode(pDecode, SourceType);

					CycleBufUpdateReadAddr(pCycleBuffCtrl, FilterOffset);

					pBufIn += FilterOffset;
					DataSize -= FilterOffset;
					DecodeDataSize = DataSize;
					FilterOffset = 0;
					ErrMask = 0;

					MsgFilterClearAll(pDecode);
					break;
				}
				else if (PROTOCOL_WAIT_CONFIRM == ResFilter)
				{
					WaitConfirmMask |= (1 << FuncIndex);
				}
				else
				{
					ErrMask |= (1 << FuncIndex);
				}
			}
		}

		if ((ErrMask &ProtocolMask) == ProtocolMask)
		{
			CycleBufUpdateReadAddr(pCycleBuffCtrl, 1);

			pBufIn += 1;
			DataSize--;
			DecodeDataSize = DataSize;
			FilterOffset = 0;
			ErrMask = 0;

			MsgFilterClearAll(pDecode);
		}

		if (WaitConfirmMask > 0)
		{
			break;
		}
	}

//	SetMsgFilterOffset(pDecode, FilterOffset);

	return 0;
}

/**********************************************************************//**
@brief msg codec decode

@param pDecode		  [In] pointer to decode
@param pBuf			 [In] pointer to msg data buffer
@param BufSize		  [In] msg data buffer size
@param DecodeType	   [In] decode type
@param SourceType	   [In] data source type

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int MsgDecode(MSG_DECODE_T* pDecode, char* pBuf, unsigned int BufSize, int DecodeType, int SourceType)
{
	CycleBufPutData(&pDecode->pMsgPort->CycleBuffCtrl, pBuf, BufSize);

	MsgFrameMatchFromCycleBuf(pDecode, DecodeType, SourceType);

	return 0;
}

/**********************************************************************//**
@brief msg codec decode reset

@param pDecode		  [In] pointer to decode
@param pBuf			 [In] pointer to msg data buffer
@param BufSize		  [In] msg data buffer size

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int MsgDecodeReset(MSG_DECODE_T* pDecode)
{
	CycleBufReset(&pDecode->pMsgPort->CycleBuffCtrl);
	MsgFilterClearAll(pDecode);
	SetMsgFilterOffset(pDecode, 0);

	return 0;
}

/**********************************************************************//**
@brief deinit msg decode

@param pDecode [In] pointer to decode

@retval <0:error,=0:success

@author CHC
@date 2023/03/13
**************************************************************************/
int MsgDecodeDeinit(void* pDecode)
{
	FREE(pDecode);
	return 0;
}

/**********************************************************************//**
@brief init msg decode

@param pProtocolFilterInfo  [In] protocol filter info
@param pPort				[In] port info

@retval pDecode : pointer to decode

@author CHC
@date 2023/03/13
**************************************************************************/
MSG_DECODE_T* MsgDecodeInit(PROTOCOL_FILTER_INFO_T* pProtocolFilterInfo,
											MSG_PORT_T* pPort)
{
	MSG_DECODE_T* pDecode = NULL;

	if ((!pProtocolFilterInfo) ||(!pPort))
	{
		return NULL;
	}

	int BaseSize = MEM_CEILBY8(sizeof(MSG_DECODE_T));

	pDecode = (MSG_DECODE_T*)MALLOC(BaseSize);
	if (!pDecode)
	{
		return NULL;
	}

	MEMSET(pDecode, 0, BaseSize);

	pDecode->ProtocolFilterInfo.pRTCM2FilterInfo = pProtocolFilterInfo->pRTCM2FilterInfo;
	pDecode->ProtocolFilterInfo.pRTCM3FilterInfo = pProtocolFilterInfo->pRTCM3FilterInfo;
	pDecode->ProtocolFilterInfo.pRawMsgAsciiFilterInfo = pProtocolFilterInfo->pRawMsgAsciiFilterInfo;
	pDecode->ProtocolFilterInfo.pRawMsgBinaryFilterInfo = pProtocolFilterInfo->pRawMsgBinaryFilterInfo;
	pDecode->ProtocolFilterInfo.pRinexFilterInfo = pProtocolFilterInfo->pRinexFilterInfo;
	pDecode->ProtocolFilterInfo.pHRCXFilterInfo = pProtocolFilterInfo->pHRCXFilterInfo;
	pDecode->ProtocolFilterInfo.pUNRawMsgBinaryFilterInfo = pProtocolFilterInfo->pUNRawMsgBinaryFilterInfo;
	pDecode->ProtocolFilterInfo.pCHCRawMsgBinaryFilterInfo = pProtocolFilterInfo->pCHCRawMsgBinaryFilterInfo;
	pDecode->ProtocolFilterInfo.pRTCMParseInfo = pProtocolFilterInfo->pRTCMParseInfo;
	pDecode->ProtocolFilterInfo.pRawMsgAsciiParseInfo = pProtocolFilterInfo->pRawMsgAsciiParseInfo;
	pDecode->ProtocolFilterInfo.pRawMsgBinaryParseInfo = pProtocolFilterInfo->pRawMsgBinaryParseInfo;
	pDecode->ProtocolFilterInfo.pRinexParseInfo = pProtocolFilterInfo->pRinexParseInfo;
	pDecode->ProtocolFilterInfo.pHRCXParseInfo = pProtocolFilterInfo->pHRCXParseInfo;
	pDecode->ProtocolFilterInfo.pUNRawMsgBinaryParseInfo = pProtocolFilterInfo->pUNRawMsgBinaryParseInfo;
	pDecode->ProtocolFilterInfo.pCHCRawMsgBinaryParseInfo = pProtocolFilterInfo->pCHCRawMsgBinaryParseInfo;
	pDecode->ProtocolFilterInfo.DecodeCallbackInfo = pProtocolFilterInfo->DecodeCallbackInfo;
	pDecode->ProtocolFilterInfo.DecodeMode = pProtocolFilterInfo->DecodeMode;
	pDecode->pMsgPort = pPort;

	return pDecode;
}

#endif
