#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg codec
*-
@file    MsgCodec.c
@author  CHC
@date    2023/06/26
@brief

**************************************************************************/
#include "OSDependent.h"
#include "MsgCodec.h"
#include "Decoder/RTCM/RTCM2Filter.h"
#include "Decoder/RTCM/RTCM3Filter.h"
#include "Decoder/RTCM/RTCMDecodeManage.h"
#include "Decoder/MsgDecode.h"

#include "Decoder/RawMsgAscii/RawMsgAsciiFilter.h"
#include "Decoder/RawMsgBinary/RawMsgBinaryFilter.h"
#include "Decoder/Rinex/RinexFilter.h"
#include "Decoder/HRCX/HRCXFilter.h"
#include "Decoder/CHCRawMsgBinary/CHCRawMsgBinaryFilter.h"
#include "Decoder/UNRawMsgBinary/UNRawMsgBinaryFilter.h"

#define CMD_MAX_BIN_DATA_STRUCT_LEN     (1024*128)

int MsgCodecDeinit(MSG_DECODE_T* pMsgDecode)
{
	if ((pMsgDecode == NULL) || (pMsgDecode->pMsgPort == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRTCM2FilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRTCM3FilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiFilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryFilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRinexParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pRinexFilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pHRCXFilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryFilterInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryParseInfo == NULL) ||
		(pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryFilterInfo == NULL))
	return -1;

	MsgPortDeinit(pMsgDecode->pMsgPort);
	RTCMDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo);
	RTCM3FilterDeinit(pMsgDecode->ProtocolFilterInfo.pRTCM3FilterInfo);
	RTCM2FilterDeinit(pMsgDecode->ProtocolFilterInfo.pRTCM2FilterInfo);
	RawMsgAsciiDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiParseInfo);
	RawMsgAsciiFilterDeinit(pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiFilterInfo);
	RawMsgBinaryDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryParseInfo);
	RawMsgBinaryFilterDeinit(pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryFilterInfo);
	RinexDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pRinexParseInfo);
	RinexFilterDeinit(pMsgDecode->ProtocolFilterInfo.pRinexFilterInfo);
	HRCXDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo);
	HRCXFilterDeinit(pMsgDecode->ProtocolFilterInfo.pHRCXFilterInfo);
	UNRawMsgBinaryDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryParseInfo);
	UNRawMsgBinaryFilterDeinit(pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryFilterInfo);
	CHCRawMsgBinaryDecodeDeinit(pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryParseInfo);
	CHCRawMsgBinaryFilterDeinit(pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryFilterInfo);
	MsgDecodeDeinit(pMsgDecode);

	return 0;
}

MSG_DECODE_T* MsgCodecInit(int DecodeID, void* pSrcObj, int DecodeMode, UTC_TIME_T * pApproximateTime)
{
	MSG_DECODE_T* pMsgDecode = NULL;
	MSG_PORT_T* pMsgPort = NULL;
	RTCM3_FILTER_INFO_T* pRTCM3FilterInfo = NULL;
	RTCM2_FILTER_INFO_T* pRTCM2FilterInfo = NULL;
	RAWMSG_ASCII_FILTER_INFO_T* pRawMsgAsciiFilterInfo = NULL;
	RAWMSG_BINARY_FILTER_INFO_T* pRawMsgBinaryFilterInfo = NULL;
	RINEX_FILTER_INFO_T* pRinexFilterInfo = NULL;
	HRCX_FILTER_INFO_T* pHRCXFilterInfo = NULL;
	UN_RAWMSG_BINARY_FILTER_INFO_T* pUNRawMsgBinaryFilterInfo = NULL;
	CHC_RAWMSG_BINARY_FILTER_INFO_T* pCHCRawMsgBinaryFilterInfo = NULL;
	RTCM_PARSE_INFO_T* pRTCMParseInfo = NULL;
	RAWMSG_ASCII_PARSE_INFO_T* pRawMsgAsciiParseInfo = NULL;
	RAWMSG_BINARY_PARSE_INFO_T* pRawMsgBinaryParseInfo = NULL;
	RINEX_PARSE_INFO_T* pRinexParseInfo = NULL;
	HRCX_PARSE_INFO_T* pHRCXParseInfo = NULL;
	UN_RAWMSG_BINARY_PARSE_INFO_T* pUNRawMsgBinaryParseInfo = NULL;
	CHC_RAWMSG_BINARY_PARSE_INFO_T* pCHCRawMsgBinaryParseInfo = NULL;
	PROTOCOL_FILTER_INFO_T ProtocolFilterInfo = {0};

	MSG_PORT_INIT_PARAM_T MsgPortInitInfo =
	{
		.MsgPortID = 0,
		.CycleBufLen = CMD_MAX_BIN_DATA_STRUCT_LEN,
		.pPortWriteFunc = NULL,
	};

	pMsgPort = MsgPortInit(&MsgPortInitInfo);
	if (!pMsgPort)
	{
		return NULL;
	}

	pRTCMParseInfo = RTCMDecodeInit();
	if (pRTCMParseInfo == NULL)
	{
		return NULL;
	}

	pRawMsgAsciiParseInfo = RawMsgAsciiDecodeInit();
	if (pRawMsgAsciiParseInfo == NULL)
	{
		return NULL;
	}

	pRawMsgBinaryParseInfo = RawMsgBinaryDecodeInit();
	if (pRawMsgBinaryParseInfo == NULL)
	{
		return NULL;
	}

	pRinexParseInfo = RinexDecodeInit();
	if (pRinexParseInfo == NULL)
	{
		return NULL;
	}

	pHRCXParseInfo = HRCXDecodeInit();
	if (pHRCXParseInfo == NULL)
	{
		return NULL;
	}

	pUNRawMsgBinaryParseInfo = UNRawMsgBinaryDecodeInit();
	if (pUNRawMsgBinaryParseInfo == NULL)
	{
		return NULL;
	}

	pCHCRawMsgBinaryParseInfo = CHCRawMsgBinaryDecodeInit();
	if (pCHCRawMsgBinaryParseInfo == NULL)
	{
		return NULL;
	}

	if (pApproximateTime)
	{
		MEMCPY(&pRTCMParseInfo->RTCMInterimData.RTCMTime, pApproximateTime, sizeof(UTC_TIME_T));
		MEMCPY(&pHRCXParseInfo->HRCXInterimData.HRCXTime, pApproximateTime, sizeof(UTC_TIME_T));
	}

	pRTCM3FilterInfo = RTCM3FilterInit();
	if (pRTCM3FilterInfo == NULL)
	{
		return NULL;
	}

	pRTCM2FilterInfo = RTCM2FilterInit();
	if (pRTCM2FilterInfo == NULL)
	{
		return NULL;
	}

	pRawMsgAsciiFilterInfo = RawMsgAsciiFilterInit();
	if (pRawMsgAsciiFilterInfo == NULL)
	{
		return NULL;
	}

	pRawMsgBinaryFilterInfo = RawMsgBinaryFilterInit();
	if (pRawMsgBinaryFilterInfo == NULL)
	{
		return NULL;
	}

	pRinexFilterInfo = RinexFilterInit();
	if (pRinexFilterInfo == NULL)
	{
		return NULL;
	}

	pHRCXFilterInfo = HRCXFilterInit();
	if (pHRCXFilterInfo == NULL)
	{
		return NULL;
	}

	pUNRawMsgBinaryFilterInfo = UNRawMsgBinaryFilterInit();
	if (pUNRawMsgBinaryFilterInfo == NULL)
	{
		return NULL;
	}

	pCHCRawMsgBinaryFilterInfo = CHCRawMsgBinaryFilterInit();
	if (pCHCRawMsgBinaryFilterInfo == NULL)
	{
		return NULL;
	}

	ProtocolFilterInfo.pRTCM3FilterInfo = pRTCM3FilterInfo;
	ProtocolFilterInfo.pRTCM2FilterInfo = pRTCM2FilterInfo;
	ProtocolFilterInfo.pRawMsgAsciiFilterInfo = pRawMsgAsciiFilterInfo;
	ProtocolFilterInfo.pRawMsgBinaryFilterInfo = pRawMsgBinaryFilterInfo;
	ProtocolFilterInfo.pRinexFilterInfo = pRinexFilterInfo;
	ProtocolFilterInfo.pHRCXFilterInfo = pHRCXFilterInfo;
	ProtocolFilterInfo.pUNRawMsgBinaryFilterInfo = pUNRawMsgBinaryFilterInfo;
	ProtocolFilterInfo.pCHCRawMsgBinaryFilterInfo = pCHCRawMsgBinaryFilterInfo;
	ProtocolFilterInfo.pRTCMParseInfo = pRTCMParseInfo;
	ProtocolFilterInfo.pRawMsgAsciiParseInfo = pRawMsgAsciiParseInfo;
	ProtocolFilterInfo.pRawMsgBinaryParseInfo = pRawMsgBinaryParseInfo;
	ProtocolFilterInfo.pRinexParseInfo = pRinexParseInfo;
	ProtocolFilterInfo.pHRCXParseInfo = pHRCXParseInfo;
	ProtocolFilterInfo.pUNRawMsgBinaryParseInfo = pUNRawMsgBinaryParseInfo;
	ProtocolFilterInfo.pCHCRawMsgBinaryParseInfo = pCHCRawMsgBinaryParseInfo;
	ProtocolFilterInfo.DecodeCallbackInfo.DecodeID = DecodeID;
	ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj = pSrcObj;
	ProtocolFilterInfo.DecodeMode = DecodeMode;

	pMsgDecode = MsgDecodeInit(&ProtocolFilterInfo, pMsgPort);
	if (!pMsgDecode)
	{
		return NULL;
	}

	return pMsgDecode;
}

int MsgCodecSetDecodeCallbackInfo(MSG_DECODE_T* pMsgDecode, int DecodeID, void* pSrcObj, DECODE_CALLBACK DecodeCallback)
{
	if ((pMsgDecode == NULL) || (pSrcObj == NULL) || (DecodeCallback == NULL))
	{
		return -1;
	}

	pMsgDecode->ProtocolFilterInfo.DecodeCallbackInfo.DecodeID = DecodeID;
	pMsgDecode->ProtocolFilterInfo.DecodeCallbackInfo.pSrcObj = pSrcObj;
	pMsgDecode->ProtocolFilterInfo.DecodeCallbackInfo.DecodeCallback = DecodeCallback;

	return 0;
}

int MsgCodecSetDecodeMode(MSG_DECODE_T* pMsgDecode, int DecodeMode)
{
	if (pMsgDecode == NULL)
	{
		return -1;
	}

	pMsgDecode->ProtocolFilterInfo.DecodeMode = DecodeMode;

	return 0;
}

#endif
