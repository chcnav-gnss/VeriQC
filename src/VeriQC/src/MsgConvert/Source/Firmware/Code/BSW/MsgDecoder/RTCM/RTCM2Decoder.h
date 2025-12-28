/**********************************************************************//**
		VeriQC

		RTCM2 Decoder Module
*-
@file   RTCM2Decoder.h
@author CHC
@date   2023/03/13
@brief

**************************************************************************/
#ifndef _RTCM2_DECODER_H_
#define _RTCM2_DECODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "RTCMDefines.h"
#include "RTCMDataTypes.h"
#include "../MsgDecodeDataTypes.h"

typedef int (*RTCM2_DECODE_OVERTIME_FUNC)(int, RTCM_PARSE_INFO_T*, void*);

int RTCM2AdjHour(RTCM_PARSE_INFO_T* pParseInfo, double ZCount, unsigned int TypeID);
int RTCM2SeparateTime(RTCM_PARSE_INFO_T* pParseInfo, unsigned int TypeID);

int DecodeRTCM2Type1And9(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type2(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type3(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type14(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type16(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type17(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type18(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type19(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type22(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type24(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type31(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type32(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type41And42(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);
int DecodeRTCM2Type44(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);

#ifdef MSG_CONVERT

int DecodeRTCM2DebugInfo(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback);

#endif /**< MSG_CONVERT */

#ifdef __cplusplus
}
#endif

#endif  /** _RTCM2_DECODER_H_ */

