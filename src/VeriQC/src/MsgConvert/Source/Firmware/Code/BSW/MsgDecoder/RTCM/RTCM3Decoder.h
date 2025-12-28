/**********************************************************************//**
		VeriQC

		RTCM3 Decoder Module
*-
@file   RTCM3Decoder.h
@author CHC
@date   2023/03/02
@brief

**************************************************************************/
#ifndef _RTCM3_DECODER_H_
#define _RTCM3_DECODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "RTCMDefines.h"
#include "RTCMDataTypes.h"
#include "MSMDecoder.h"
#include "../MsgDecodeDataTypes.h"

int RTCM3DecodeType1001(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1002(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1003(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1004(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeType1009(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1010(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1011(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1012(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeType1005(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1006(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1007(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1008(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1033(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeType1019(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1020(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1041(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1042(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1044(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1045(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1046(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeType1230(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeType1300(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1301(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1302(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1303(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1304(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

#ifdef MSG_CONVERT

int RTCM3DecodeType1013(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);
int RTCM3DecodeType1029(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

int RTCM3DecodeDebugInfo(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);

#endif /**< MSG_CONVERT */

#ifdef __cplusplus
}
#endif

#endif  /** _RTCM3_DECODER_H_ */

