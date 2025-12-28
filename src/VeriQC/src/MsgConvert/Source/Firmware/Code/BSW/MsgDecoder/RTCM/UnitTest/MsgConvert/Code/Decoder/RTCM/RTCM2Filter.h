#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM2 Filter Module
*-
@file   RTCM2Filter.h
@author CHC
@date   2023/04/23
@brief

**************************************************************************/
#ifndef _RTCM2_FILTER_H_
#define _RTCM2_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

typedef struct _RTCM2_FILTER_INFO_T
{
	unsigned int MsgLen;        /**< variable length data message */
	unsigned int LastWord;      /**< last word data*/
	unsigned int Word;          /**< word data*/
	unsigned int ByteNum;       /**< data number after deode*/
	unsigned int BitNum;        /**< bit number after deode*/
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} RTCM2_FILTER_INFO_T;

PROTOCOL_FILTER_FRAM_ERR_E RTCM2Filter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int RTCM2FilterClear(MSG_DECODE_T* pDecode);
int RTCM2Decode(MSG_DECODE_T* pDecode, int SourceType);
int RTCM2FilterDeinit(RTCM2_FILTER_INFO_T* pRTCM2FilterInfo);
RTCM2_FILTER_INFO_T* RTCM2FilterInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _RTCM2_FILTER_H_ */

#endif
