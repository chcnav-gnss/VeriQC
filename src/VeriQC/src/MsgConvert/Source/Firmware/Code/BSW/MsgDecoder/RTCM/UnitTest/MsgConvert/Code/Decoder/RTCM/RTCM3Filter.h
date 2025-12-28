#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM3 Filter Module
*-
@file   RTCM3Filter.h
@author CHC
@date   2023/04/14
@brief

**************************************************************************/
#ifndef _RTCM3_FILTER_H_
#define _RTCM3_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

typedef struct _RTCM3_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	unsigned int MsgLen;        /**< variable length data message */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} RTCM3_FILTER_INFO_T;

PROTOCOL_FILTER_FRAM_ERR_E RTCM3Filter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int RTCM3FilterClear(MSG_DECODE_T* pDecode);
int RTCM3Decode(MSG_DECODE_T* pDecode, int SourceType);
int RTCM3FilterDeinit(RTCM3_FILTER_INFO_T* pRTCM3FilterInfo);
RTCM3_FILTER_INFO_T* RTCM3FilterInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _RTCM3_FILTER_H_ */

#endif
