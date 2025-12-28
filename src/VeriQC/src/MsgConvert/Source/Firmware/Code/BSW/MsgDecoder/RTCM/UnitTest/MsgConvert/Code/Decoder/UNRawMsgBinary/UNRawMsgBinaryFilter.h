#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		BSW
*-
@file   UNUNRawMsgBinaryFilter.h
@author CHC
@date   2023/08/03
@brief

**************************************************************************/
#ifndef _UN_RAWMSG_BINARY_FILTER_H_
#define _UN_RAWMSG_BINARY_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "UNRawMsgBinaryDataTypes.h"

typedef struct _UN_RAWMSG_BINARY_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	unsigned int MsgLen;		/**< message length */
	int DecodeID;               /**< RawMsg decode id */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} UN_RAWMSG_BINARY_FILTER_INFO_T;

int UNRawMsgBinaryFilterClear(MSG_DECODE_T* pDecode);
PROTOCOL_FILTER_FRAM_ERR_E UNRawMsgBinaryFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int UNRawMsgBinaryFilterDeinit(UN_RAWMSG_BINARY_FILTER_INFO_T* pUNRawMsgBinaryFilterInfo);
UN_RAWMSG_BINARY_FILTER_INFO_T* UNRawMsgBinaryFilterInit(void);

int UNRawMsgBinaryDecode(MSG_DECODE_T* pDecode, int SourceType);
int UNRawMsgBinaryDecodeDeinit(UN_RAWMSG_BINARY_PARSE_INFO_T* pUNRawMsgBinaryParseInfo);
UN_RAWMSG_BINARY_PARSE_INFO_T* UNRawMsgBinaryDecodeInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _UN_RAWMSG_BINARY_FILTER_H_ */

#endif
