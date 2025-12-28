#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RinexAsciiFilter.h
@author CHC
@date   2023/12/20
@brief

**************************************************************************/
#ifndef _RINEX_FILTER_H_
#define _RINEX_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "RinexDataTypes.h"

typedef struct _RINEX_FILTER_INFO_T
{
	unsigned int Stage;		/**< the stage of the currently completed parsing data */
	int DecodeID;			/**< RawMsg decode id */
	MSG_BUF_T DecodeBuf;	/**< address stored after frame match */
} RINEX_FILTER_INFO_T;

int RinexFilterClear(MSG_DECODE_T* pDecode);
PROTOCOL_FILTER_FRAM_ERR_E RinexFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int RinexFilterDeinit(RINEX_FILTER_INFO_T* pRinexFilterInfo);
RINEX_FILTER_INFO_T* RinexFilterInit(void);

int RinexDecode(MSG_DECODE_T* pDecode, int SourceType);
int RinexDecodeDeinit(RINEX_PARSE_INFO_T* pRinexParseInfo);
RINEX_PARSE_INFO_T* RinexDecodeInit(void);

int RinexDecodeConvert(void* pSrcObj);

#ifdef __cplusplus
}
#endif

#endif  /** _RAWMSG_ASCII_FILTER_H_ */

#endif
