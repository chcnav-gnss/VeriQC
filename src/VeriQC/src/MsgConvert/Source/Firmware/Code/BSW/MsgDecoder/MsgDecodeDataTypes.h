/**********************************************************************//**
		VeriQC

		Msg Decode Module
*-
@file   MsgDecodeDataTypes.h
@author CHC
@date   2023/06/29
@brief

**************************************************************************/
#ifndef _MSG_DECODE_DATA_TYPES_H_
#define _MSG_DECODE_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "OSDependent.h"

typedef struct _MSG_BUF_T
{
	unsigned char* pBaseAddr;
	unsigned int Len;
} MSG_BUF_T;

#ifdef __cplusplus
}
#endif

#endif

