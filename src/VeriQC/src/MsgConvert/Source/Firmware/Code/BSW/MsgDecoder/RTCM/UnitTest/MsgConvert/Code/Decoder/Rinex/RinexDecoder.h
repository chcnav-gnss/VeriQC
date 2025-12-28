#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RinexDecoder.h
@author CHC
@date   2023/12/19
@brief

**************************************************************************/
#ifndef _RINEX_DECODER_H_
#define _RINEX_DECODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>
#include "Common/DataTypes.h"
#include "RinexDataTypes.h"

int OpenRinexInfo(RINEX_INFO_T* pRinexInfo, FILE* pFile, unsigned int FileTypeIndex);
int InputRinexInfo(RINEX_INFO_T* pRinexInfo, FILE* pFile, unsigned int FileTypeIndex, UTC_TIME_T* pUTCTime, int* pRinexID);

#ifdef __cplusplus
}
#endif

#endif  /** _RINEX_DECODER_H_ */

#endif
