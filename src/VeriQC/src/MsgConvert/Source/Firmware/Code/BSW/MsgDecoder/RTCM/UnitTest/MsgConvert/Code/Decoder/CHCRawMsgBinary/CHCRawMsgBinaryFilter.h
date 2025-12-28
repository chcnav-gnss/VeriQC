#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   CHCCHCRawMsgBinaryFilter.h
@author CHC
@date   2023/08/03
@brief

**************************************************************************/
#ifndef _CHC_RAWMSG_BINARY_FILTER_H_
#define _CHC_RAWMSG_BINARY_FILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "../MsgDecodeDefinesEx.h"
#include "CHCRawMsgBinaryDataTypes.h"

typedef struct _CHC_RAWMSG_BINARY_FILTER_INFO_T
{
	unsigned int Stage;         /**< the stage of the currently completed parsing data */
	unsigned int MsgLen;		/**< message length */
	int DecodeID;               /**< CHCRawMsg decode id */
	unsigned int DecodeOffset;
	MSG_BUF_T DecodeBuf;        /**< address stored after frame match */
} CHC_RAWMSG_BINARY_FILTER_INFO_T;

int CHCRawMsgBinaryFilterClear(MSG_DECODE_T* pDecode);
PROTOCOL_FILTER_FRAM_ERR_E CHCRawMsgBinaryFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);
int CHCRawMsgBinaryFilterDeinit(CHC_RAWMSG_BINARY_FILTER_INFO_T* pCHCRawMsgBinaryFilterInfo);
CHC_RAWMSG_BINARY_FILTER_INFO_T* CHCRawMsgBinaryFilterInit(void);

int CHCRawMsgBinaryDecode(MSG_DECODE_T* pDecode, int SourceType);
int CHCRawMsgBinaryDecodeDeinit(CHC_RAWMSG_BINARY_PARSE_INFO_T* pCHCRawMsgBinaryParseInfo);
CHC_RAWMSG_BINARY_PARSE_INFO_T* CHCRawMsgBinaryDecodeInit(void);

#ifdef __cplusplus
}
#endif

#endif  /** _CHC_RAWMSG_BINARY_FILTER_H_ */

#endif
