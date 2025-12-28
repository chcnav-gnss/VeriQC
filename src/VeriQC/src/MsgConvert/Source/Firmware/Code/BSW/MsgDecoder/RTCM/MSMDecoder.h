/**********************************************************************//**
		VeriQC

		MSM Decoder Module
*-
@file   MSMDecoder.h
@author CHC
@date   2024/07/22
@brief

**************************************************************************/
#ifndef _MSM_DECODER_H_
#define _MSM_DECODER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Common/DataTypes.h"
#include "RTCMDefines.h"
#include "RTCMDataTypes.h"
#include "../MsgDecodeDataTypes.h"

typedef int (*RTCM3_DECODE_OVERTIME_FUNC)(int, RTCM_PARSE_INFO_T*, void*);

int AdjWeek(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, double Tow);
int AdjGPSWeek(int Week);
int AdjGLOTimeDaily(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, double Tod, double Dow);

int LossOfLock(unsigned int* pLockTime, unsigned int LockTime);
int CheckStationID(RTCM_INTERIM_DATA_T* pRTCMInterimData, unsigned int StationID);
int GetMSMLockTimeEx(unsigned int LockTime);

int DecodeObsCallback(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
											OBS_DATA_SAT_T* pSatObsData, unsigned int RawObsDataType, UTC_TIME_T RTCMTime);
int DecodeObsCompleteCallback(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
											OBS_DATA_SAT_T* pSatObsData, unsigned int RawObsDataType);
int DecodeObsOverTimeCallback(int TypeID, GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
	OBS_DATA_SAT_T* pSatObsData, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback,
	unsigned int RawObsDataType);

int MSMIntRoughRangeDecoder(unsigned int Index, unsigned int Nsat, double* pRouRang,
											unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMModRoughRangeDecoder(unsigned int Index, unsigned int Nsat, double* pRouRang,
											unsigned char* pBufIn, unsigned int* pBitNumOut, int CheckType);
int MSMInfoDecoder(unsigned int Index, unsigned int Nsat, unsigned int* pInfo,
							unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMRoughRateDecoder(unsigned int Index, unsigned int Nsat, double* pRouRate,
									unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMPseudoRangeDecoder(unsigned int Index, unsigned int Ncell, double* pPsRang,
										unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMPseudoRangeExDecoder(unsigned int Index, unsigned int Ncell, double* pPsRangEx,
											unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMPhaseRangeDecoder(unsigned int Index, unsigned int Ncell, double* pPhRang,
									unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMPhaseRangeExDecoder(unsigned int Index, unsigned int Ncell, double* pPhRangEx,
										unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMLockTimeDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pLockTime,
									unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMLockTimeExDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pLockTimeEx,
									unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMHalfCycleAmbDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pHalfAmb,
										unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMCnrDecoder(unsigned int Index, unsigned int Ncell, double* pCnr,
							unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMCnrExDecoder(unsigned int Index, unsigned int Ncell, double* pCnrEx,
							unsigned char* pBufIn, unsigned int* pBitNumOut);
int MSMPhaseRangeRateExDecoder(unsigned int Index, unsigned int Ncell, double* pPhRate,
											unsigned char* pBufIn, unsigned int* pBitNumOut);

#ifdef RTCM_DEBUG_INFO
int SatNumbInfoPrintf(RTCM_PARSE_INFO_T* pParseInfo, RTCM_INTERIM_DATA_T* pRTCMInterimData, GNSS_OBS_T* pObsData);
#endif

int RTCM3DecodeTypeMSM(int RTCMSatSys, unsigned TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj,
									RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback);


#ifdef __cplusplus
}
#endif

#endif  /** _MSM_DECODER_H_ */

