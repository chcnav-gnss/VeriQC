#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RTCMDecodeManage.c
@author CHC
@date   2023/04/24
@brief

**************************************************************************/
#include "RTCMDecodeManage.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "Common/DataTypes.h"
#include "Common/CommonFuncs.h"
#include "Common/GNSSSignal.h"

#include "TimeDefines.h"
#include "Common/GNSSNavDataType.h"

#define RTCM_DECODE_INFO_STATUS_IDLE			0x00
#define RTCM_DECODE_INFO_STATUS_USING			0xA5

#define RTCM_SUB_MSG_PARSED_LEN_MAX				(15*1024 * 1024)

static RTCM_DECODE_INFO_T s_RTCMDecodeInfoMap[RTCM_DECODE_CHANNEL_MAX_NUM];

/**********************************************************************//**
@brief  rtcm data structure initial

@param pParseInfo	   [In] rtcm parse information

@retval <0:error, =0:success

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCMParseInfoReset(RTCM_PARSE_INFO_T* pRTCMParseInfo)
{
	unsigned char* pBaseAddr;
#ifdef RTCM_DEBUG_INFO
	unsigned char* pParsedDebugInfoBuf;
	unsigned char* pParsedBaseObsSatCountBuf;
#endif

	if (!pRTCMParseInfo)
	{
		return -1;
	}

	pBaseAddr = pRTCMParseInfo->ParsedDataBuf.pBaseAddr;
#ifdef RTCM_DEBUG_INFO
	pParsedDebugInfoBuf = pRTCMParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	pParsedBaseObsSatCountBuf = pRTCMParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
#endif

	MEMSET(pRTCMParseInfo, 0x00, sizeof(RTCM_PARSE_INFO_T));

	pRTCMParseInfo->ParsedDataBuf.pBaseAddr = (unsigned char*)pBaseAddr;
	MEMSET(pRTCMParseInfo->ParsedDataBuf.pBaseAddr, 0x00, RTCM_SUB_MSG_PARSED_LEN_MAX);
#ifdef RTCM_DEBUG_INFO
	pRTCMParseInfo->ParsedDebugInfoBuf.pBaseAddr = (unsigned char*)pParsedDebugInfoBuf;
	MEMSET(pRTCMParseInfo->ParsedDebugInfoBuf.pBaseAddr, 0x00, RTCM_MSG_PARSED_DEBUG_INFO_LEN_MAX);

	pRTCMParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr = (unsigned char*)pParsedBaseObsSatCountBuf;
	MEMSET(pRTCMParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr, 0x00, RTCM_MSG_PARSED_BASEOBS_SAT_COUNT_LEN_MAX);
#endif

	pRTCMParseInfo->RTCMInterimData.ObsCompleteFlag = 1;
	pRTCMParseInfo->RTCMInterimData.LeapSec = 18;

	return 0;
}

/**********************************************************************//**
@brief  RTCM decode deinit

@param pParseInfo	   [In] rtcm parse information

@retval <0:error, =0:success

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCMDecodeDeinit(RTCM_PARSE_INFO_T* pRTCMParseInfo)
{
	if (!pRTCMParseInfo)
	{
		return -1;
	}

	FREE(pRTCMParseInfo);

	return 0;
}

/**********************************************************************//**
@brief  RTCM decode init

@retval struct RTCM_PARSE_INFO_T pointer

@author CHC
@date 2023/03/09
@note
**************************************************************************/
RTCM_PARSE_INFO_T* RTCMDecodeInit(void)
{
	RTCM_PARSE_INFO_T* pRTCMParseInfo = NULL;
	int RTCMSize = MEM_CEILBY8(sizeof(RTCM_PARSE_INFO_T));
	int RTCMParseBufSize = MEM_CEILBY8(RTCM_SUB_MSG_PARSED_LEN_MAX);
	int TotalSize = RTCMSize + RTCMParseBufSize;
#ifdef RTCM_DEBUG_INFO
	int RTCMParseDebugInfoBufSize = MEM_CEILBY8(RTCM_MSG_PARSED_DEBUG_INFO_LEN_MAX);
	TotalSize += RTCMParseDebugInfoBufSize;
	int RTCMParseBaseObsSatCountBufSize = MEM_CEILBY8(RTCM_MSG_PARSED_BASEOBS_SAT_COUNT_LEN_MAX);
	TotalSize += RTCMParseBaseObsSatCountBufSize;
#endif

	pRTCMParseInfo = MALLOC(TotalSize);
	if (pRTCMParseInfo == NULL)
	{
		return NULL;
	}

	MEMSET(pRTCMParseInfo, 0x00, sizeof(RTCM_PARSE_INFO_T));

	pRTCMParseInfo->ParsedDataBuf.pBaseAddr = (unsigned char*)pRTCMParseInfo + RTCMSize;

	MEMSET(pRTCMParseInfo->ParsedDataBuf.pBaseAddr, 0x00, RTCMParseBufSize);

#ifdef RTCM_DEBUG_INFO
	pRTCMParseInfo->ParsedDebugInfoBuf.pBaseAddr = (unsigned char*)pRTCMParseInfo + RTCMSize + RTCMParseBufSize;

	MEMSET(pRTCMParseInfo->ParsedDebugInfoBuf.pBaseAddr, 0x00, RTCMParseDebugInfoBufSize);

	pRTCMParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr = (unsigned char*)pRTCMParseInfo + RTCMSize + RTCMParseBufSize + RTCMParseDebugInfoBufSize;

	MEMSET(pRTCMParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr, 0x00, RTCMParseBaseObsSatCountBufSize);
#endif

	pRTCMParseInfo->RTCMInterimData.ObsCompleteFlag = 1;
	pRTCMParseInfo->RTCMInterimData.LeapSec = 18;

	return pRTCMParseInfo;
}
#endif
