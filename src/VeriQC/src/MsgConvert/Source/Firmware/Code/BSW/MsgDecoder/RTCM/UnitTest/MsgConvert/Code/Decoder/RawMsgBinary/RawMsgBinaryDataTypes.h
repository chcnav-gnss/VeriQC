#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RawMsg Binary Decoder Module
*-
@file   RawMsgBinaryDataTypes.h
@author CHC
@date   2023/09/12
@brief

**************************************************************************/
#ifndef _RAWMSG_BINARY_DATA_TYPES_H_
#define _RAWMSG_BINARY_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"
#include "../RawMsgAscii/RawMsgDecodeDataStructure.h"

#define RAWMSG_BINARY_MSG_PARSED_LEN_MAX							(1024 * 1024) /** rangea message parsed data struct length max, unit: byte*/
#define RAWMSG_BINARY_PARSED_BASEOBS_SAT_COUNT_LEN_MAX				(1 * 1024) /** RawMsg message parsed baseobs sat count length max, unit: byte*/

typedef struct _RAWMSG_BINARY_INTERIM_DATA_T
{
	unsigned int BaseObsSatNum[RM_RANGE_SYS_OTHER];
} RAWMSG_BINARY_INTERIM_DATA_T;

typedef struct _RAWMSG_BINARY_PARSE_INFO_T
{
	RAWMSG_BINARY_INTERIM_DATA_T RawMsgBinaryInterimData;    /**< raw msg binary decode interim data */
	MSG_BUF_T* pDecodeBuf;                  /**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;                /**< address stored after data parsed */

	MSG_BUF_T ParsedBaseObsSatCountBuf;     /**< parsed BaseObs satellite count information buffer */
} RAWMSG_BINARY_PARSE_INFO_T;

#ifdef __cplusplus
}
#endif

#endif

#endif
