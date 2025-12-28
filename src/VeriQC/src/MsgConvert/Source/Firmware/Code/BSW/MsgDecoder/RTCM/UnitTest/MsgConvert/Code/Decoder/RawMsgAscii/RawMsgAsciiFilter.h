#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RawMsgAsciiFilter.h
@author CHC
@date   2023/08/03
@brief

**************************************************************************/
#ifndef _RAWMSG_ASCII_FILTER_H_
#define _RAWMSG_ASCII_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "RawMsgAsciiDataTypes.h"

typedef struct _RAWMSG_ASCII_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	int DecodeID;      /**< RawMsg decode id */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} RAWMSG_ASCII_FILTER_INFO_T;

int RawMsgAsciiFilterClear(MSG_DECODE_T* pDecode);
PROTOCOL_FILTER_FRAM_ERR_E RawMsgAsciiFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int RawMsgAsciiFilterDeinit(RAWMSG_ASCII_FILTER_INFO_T* pRawMsgAsciiFilterInfo);
RAWMSG_ASCII_FILTER_INFO_T* RawMsgAsciiFilterInit(void);

int RawMsgAsciiDecode(MSG_DECODE_T* pDecode, int SourceType);
int RawMsgAsciiDecodeDeinit(RAWMSG_ASCII_PARSE_INFO_T* pRawMsgAsciiParseInfo);
RAWMSG_ASCII_PARSE_INFO_T* RawMsgAsciiDecodeInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _RAWMSG_ASCII_FILTER_H_ */

#endif
