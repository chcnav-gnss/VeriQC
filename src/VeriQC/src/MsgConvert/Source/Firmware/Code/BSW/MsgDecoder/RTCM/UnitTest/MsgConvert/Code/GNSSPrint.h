#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RTCM3 Decoder Module
*-
@file   GNSSPrint.h
@author CHC
@date   2023/03/02
@brief

**************************************************************************/
#ifndef _GNSS_PRINT_H_
#define _GNSS_PRINT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "Common/GNSSNavDataType.h"

int GNSSPrintf(const char* format, ...);

void ShowReceiverInfo(RTCM_RECEIVER_DESCRIPTOR_INFO_T* pReceiverInfo);
void ShowPosition(double* pPosXYZ);

void PrintObsTime(GNSS_OBS_T* pObs);
int PrintfProgressBar(int Percentage);

#ifdef __cplusplus
}
#endif

#endif  /** _GNSS_PRINT_H_ */

#endif
