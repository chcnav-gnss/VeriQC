#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RawMsg ascii Decoder Module
*-
@file   RawMsgAsciiDataTypes.h
@author CHC
@date   2023/05/23
@brief

**************************************************************************/
#ifndef _RAWMSG_ASCII_DATA_TYPES_H_
#define _RAWMSG_ASCII_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"
#include "RawMsgDecodeDataStructure.h"

/** ONES(n) is macro to get n continuous 1s, (2 ^ n - 1), low n bits is 1 */
#define ONES(n) ((1 << (n)) - 1)

/** GET_UBITS gets number of len bits starting from bit pos of data(bit pos is LSB) */
#define GET_UBITS(data, pos, len) ((unsigned int)((data) >> (pos)) & ONES(len))

/** GET_BITS is the same as GET_UBITS but sign extended(assuming data is a signed int) */
#define GET_BITS(_data, _pos, _len) (((((signed int)(_data)) << (32 - (_pos) - (_len)))) >> (32 - (_len)))

#define RAWMSG_ASCII_MSG_PARSED_LEN_MAX						(1024 * 1024) /** RawMsg message parsed data struct length max, unit: byte*/
#define RAWMSG_ASCII_PARSED_BASEOBS_SAT_COUNT_LEN_MAX				(1 * 1024) /** RawMsg message parsed baseobs sat count length max, unit: byte*/

typedef struct _RAWMSG_ASCII_INTERIM_DATA_T
{
	unsigned int BaseObsSatNum[RM_RANGE_SYS_OTHER+1];
} RAWMSG_ASCII_INTERIM_DATA_T;

typedef struct _RAWMSG_ASCII_PARSE_INFO_T
{
	RAWMSG_ASCII_INTERIM_DATA_T RawMsgAsciiInterimData;    /**< raw msg ascii decode interim data */
	MSG_BUF_T* pDecodeBuf;                  /**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;                /**< address stored after data parsed */

	MSG_BUF_T ParsedBaseObsSatCountBuf;     /**< parsed BaseObs satellite count information buffer */
} RAWMSG_ASCII_PARSE_INFO_T;

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

#if !defined( __GCC__)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif
