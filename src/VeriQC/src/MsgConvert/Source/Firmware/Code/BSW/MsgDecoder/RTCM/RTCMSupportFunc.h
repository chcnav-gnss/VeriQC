/**********************************************************************//**
		VeriQC

		RTCM support functions
*-
@file   RTCMSupportFunc.h
@author CHC
@date   2023/03/02
@brief

**************************************************************************/
#ifndef _RTCM_SUPPORT_FUNC_H_
#define _RTCM_SUPPORT_FUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__linux)
#define PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#elif defined(__aarch)

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "Common/DataTypes.h"
#include "TimeDefines.h"
#include "RTCMDataTypes.h"

#include "BSW/MsgEncoder/RTCM/RTCMEncoderIDDefines.h"

unsigned int GetUnsignedBits(const unsigned char *pBuf, int Pos, int Len);
int GetSignedBits(const unsigned char *pBuf, int Pos, int Len);
double Get38Bits(const unsigned char *pBuf, int Pos);
UINT64 GetUnsigned40Bits(const unsigned char *pBuf, int Pos);
double Get64Bits(const unsigned char *pBuf, int Pos);
int GetSignedMagBits(const unsigned char *pBuf, int Pos, int Len);
UINT64 GetUINT64Bits(const unsigned char* pBuf, int Pos);

UTC_TIME_T RTCMGetUTCTime(void);
int GetPerSatObsDataIndex(OBS_DATA_SAT_T* pSatObsData, unsigned int ExSystemID, unsigned int SatID,
										unsigned int SignalType, unsigned int* pSatIndex, unsigned int* pObsIndex);
int SatObsDataSatNumAdd(OBS_DATA_SAT_T* pSatObsData);
int PerSatObsDataObsNumAdd(OBS_DATA_PER_SAT_T* pPerSatObsData, unsigned int ExSystemID, unsigned int SatID);
int SatObsDataToGNSSObsData(OBS_DATA_SAT_T* pSatObsData, GNSS_OBS_T* pObsData);
int UTCTimeToRangeTime(GNSS_OBS_T* pObsData, UTC_TIME_T* pUTCTime);

unsigned int PRNToMSMSatID(int RMRangeSystem, unsigned short PRN);
unsigned int GetRTCM3MSMMsgNumBase(int RTCMSatSys);
unsigned int RMRangeSignalToRTCM3Signal(int RMRangeSystem, unsigned int RMRangeSignal, unsigned int* pFreqIndex, unsigned int BDSExpandFlag);
unsigned int RTCMMSMSystemToRMRangeSystem(int RTCMMSMSystem);
unsigned int RMRangeSystemToRTCMMSMSystem(int RMSystem);
unsigned int RMRangeSystemToExSystem(int RMRangeSystem);
unsigned int ExSystemToRMRangeSystem(int ExSystemID);
unsigned int MSMSatIDToPRN(int ExSystemID, unsigned short SatID);
unsigned int RTCM3SignalToRMRangeSignalByExSystem(int ExSystemID, unsigned int RTCM3SignalIndex, unsigned int* pFreqIndex, unsigned int BDSExpandFlag);
double GetPhaseAlignmentDataCycle(int ExSystemID, int RMSignalID);
double GetRefstationPhaseAlignmentData(int VendorType, int ExSystemID, int RMSignalID);
REFSTATION_VENDOR_E GetRefstationVendorType(char* pReceiverDescriptor);

int EpochTimeToGPSTime(int EpochTime, int * pGPSWeek, int * pWeekMsCount);
unsigned int GetSatIndexStart(unsigned int System);

const char * GetVendorTypeString(unsigned int VendorID);

#ifdef RTCM_DEBUG_INFO
const char* GetRTCM3SignalName(unsigned int RTCMSys, unsigned int RTCMSignalID);
unsigned int ExSystemToRTCMSystem(int ExSystemID);
#endif

#ifdef __cplusplus
}
#endif

#endif

