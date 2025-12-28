#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RawMsgBinaryFilter.h
@author CHC
@date   2023/08/03
@brief

**************************************************************************/
#ifndef _RAWMSG_BINARY_FILTER_H_
#define _RAWMSG_BINARY_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "RawMsgBinaryDataTypes.h"

typedef struct _RAWMSG_BINARY_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	unsigned int MsgLen;		/**< message length */
	int DecodeID;               /**< RawMsg decode id */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} RAWMSG_BINARY_FILTER_INFO_T;

int RawMsgBinaryFilterClear(MSG_DECODE_T* pDecode);
PROTOCOL_FILTER_FRAM_ERR_E RawMsgBinaryFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int RawMsgBinaryFilterDeinit(RAWMSG_BINARY_FILTER_INFO_T* pRawMsgBinaryFilterInfo);
RAWMSG_BINARY_FILTER_INFO_T* RawMsgBinaryFilterInit(void);

int RawMsgBinaryDecode(MSG_DECODE_T* pDecode, int SourceType);
int RawMsgBinaryDecodeDeinit(RAWMSG_BINARY_PARSE_INFO_T* pRawMsgBinaryParseInfo);
RAWMSG_BINARY_PARSE_INFO_T* RawMsgBinaryDecodeInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _RAWMSG_BINARY_FILTER_H_ */

#endif
