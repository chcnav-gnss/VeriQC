#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM Decode Module
*-
@file   RTCMDecodeManage.h
@author CHC
@date   2023/04/24
@brief

**************************************************************************/
#ifndef _RTCM_DECODE_MANAGE_H_
#define _RTCM_DECODE_MANAGE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"
#include "TimeDefines.h"

typedef struct _RTCM_DECODE_DATA_QUALITY_T
{
	unsigned int ThroughputRate;    /**< rtcm data throughput rate (unit: B/s) */
	long long ReceiverBytes;        /**< receiver rtcm data (unit: Byte) */
	unsigned int SatelliteNum;      /**< number of satellites */
	unsigned int Distance;          /**< distance from reference station */
	unsigned int SignalNum;         /**< how many stars and frequencies, for example: 5 Star 3 Freq */
	unsigned int StationID;         /**< designated reference station */
} RTCM_DECODE_DATA_QUALITY_T;

typedef struct _RTCM_DECODE_INFO_T
{
	unsigned int UseStatus;                     /**< use status 0: idle, 0xFF:error, other:using */
	UTC_TIME_T FirstMsgTime;                    /**< time of receive first rtcm msg */
	UTC_TIME_T LastMsgTime;                     /**< time of receive last rtcm msg */
	RTCM_PARSE_INFO_T* pRTCMParseInfo;          /**< rtcm parse information*/
	RTCM_DECODE_DATA_QUALITY_T DataQuality;     /**< rtcm data quality */
} RTCM_DECODE_INFO_T;

typedef struct _RTCM_DECODE_MANAGE_INFO_T
{
	int RTCMDecodeIndex;        /**< 0xFF:idle, 0 ~ (RTCM_DECODE_CHANNEL_MAX_NUM-1):Valid */
	unsigned int UseStatus;     /**< use status 0: idle, 0xFF:error, other:using */
	RTCM_DECODE_INFO_T* pRTCMDecodeInfo;
} RTCM_DECODE_MANAGE_INFO_T;

typedef enum _RTCM_OUTPUT_MODE_T
{
	RTCM_OUTPUT_MODE_ONE = 0,
	RTCM_OUTPUT_MODE_TWO = 1,
	RTCM_OUTPUT_MODE_THREE = 2,
	RTCM_OUTPUT_MODE_OTHER   = 0xFFFFFFFF,
} RTCM_OUTPUT_MODE_T;

int RTCMParseInfoReset(RTCM_PARSE_INFO_T* pRTCMParseInfo);

int RTCMDecodeDeinit(RTCM_PARSE_INFO_T* pRTCMParseInfo);
RTCM_PARSE_INFO_T* RTCMDecodeInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _RTCM3_DECODER_H_ */

#endif
