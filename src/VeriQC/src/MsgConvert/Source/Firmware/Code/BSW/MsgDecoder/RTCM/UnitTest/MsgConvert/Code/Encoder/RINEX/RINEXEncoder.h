#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RINEX Encoder Module
*-
@file   RINEXEncoder.h
@author CHC
@date   2023/04/03
@brief

**************************************************************************/
#ifndef _RINEX_ENCODER_H_
#define _RINEX_ENCODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "RINEXDefines.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"
#include "TimeDefines.h"

typedef enum _GNSS_OBS_DATA_TYPE_E
{
	GNSS_OBS_DATA_TYPE_RANGE = 0,
	GNSS_OBS_DATA_TYPE_GNSSOBS,

	GNSS_OBS_DATA_TYPE_NUM,
} GNSS_OBS_DATA_TYPE_E;

typedef struct _RINEX_OUTPUT_DATA_T
{
	unsigned int    TimeStamp;
	unsigned int    LogOutMaxLen;
	unsigned char*  pLogOut;
	unsigned int*   LogOutDataLen;
} RINEX_OUTPUT_DATA_T;

typedef struct _GNSS_OBS_PER_SAT_T
{
	int ExSystemID;
	int SatID;
	int ObsNum;
	RM_RANGE_DATA_T ObsData[32];				/**< obs data*/
	RM_EX_GNSSOBS_DATA_T GNSSObsData[32];		/**< obs data*/
} GNSS_OBS_PER_SAT_T;

typedef struct _GNSS_OBS_SAT_DATA_T
{
	unsigned int DataType;
	int SatNum;
	UTC_TIME_T ObsTime;										/**< UTC time */
	GNSS_OBS_PER_SAT_T ObsPerData[RINEX_SAT_NUM_MAX];		/**< obs data*/
} GNSS_OBS_SAT_DATA_T;

int GetObsSigCode(int SatSys, int Type, unsigned int* pObsCode);
const char* GetObsSigCharByCode(int CodeType);
const char* GetObsSigChar(int RMSystemID, int Type);
int GenerateRINEX(unsigned int EncodeID, char* pBuff, void* pInData, void* pData);

#ifdef __cplusplus
}
#endif

#endif  /** _RINEX_ENCODER_H_ */

#endif
