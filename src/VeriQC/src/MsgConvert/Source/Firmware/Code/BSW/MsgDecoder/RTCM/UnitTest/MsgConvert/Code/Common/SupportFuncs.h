#ifdef MSG_CONVERT

/**********************************************************************//**
			 VeriQC

			 Common Module
 *-
 @file SupportFuncs.h
 @author CHC
 @date 2022/06/29

@brief common used operations and functions for all the module

**************************************************************************/
#ifndef _SUPPORT_FUNCS_H_
#define _SUPPORT_FUNCS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "TimeDefines.h"

unsigned int RinexSystemToExSystem(int RindexSystem);
unsigned int ExSystemToRinexSystem(int ExSystemID);
unsigned int ExSystemToRinexSysIndex(int ExSystemID);

int GetFileIndex(int RawDataType, int DecodeID);
int GetRinexID(int RawDataType, int DecodeID);
int GetAllSatIndex(unsigned int ExSystemID, unsigned int PRN);

#ifdef __cplusplus
}
#endif

#endif /**< _SUPPORT_FUNCS_H_ */

#endif /**< MSG_CONVERT */
