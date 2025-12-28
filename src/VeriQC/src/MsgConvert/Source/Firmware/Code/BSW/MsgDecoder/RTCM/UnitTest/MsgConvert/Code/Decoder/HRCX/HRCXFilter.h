#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		HRCX Filter Module
*-
@file   HRCXFilter.h
@author CHC
@date   2024/03/04
@brief

**************************************************************************/
#ifndef _HRCX_FILTER_H_
#define _HRCX_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

typedef struct _HRCX_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	unsigned int MsgLen;        /**< variable length data message */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} HRCX_FILTER_INFO_T;

PROTOCOL_FILTER_FRAM_ERR_E HRCXFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int HRCXFilterClear(MSG_DECODE_T* pDecode);
int HRCXDecode(MSG_DECODE_T* pDecode, int SourceType);
int HRCXParseInfoReset(HRCX_PARSE_INFO_T* pHRCXParseInfo);
int HRCXDecodeDeinit(HRCX_PARSE_INFO_T* pHRCXParseInfo);
HRCX_PARSE_INFO_T* HRCXDecodeInit(void);
int HRCXFilterDeinit(HRCX_FILTER_INFO_T* pHRCXFilterInfo);
HRCX_FILTER_INFO_T* HRCXFilterInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _HRCX_FILTER_H_ */

#endif
