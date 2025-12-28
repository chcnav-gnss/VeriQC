/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RTCM3Decoder.c
@author CHC
@date   2023/03/09
@brief

**************************************************************************/

#include "RTCM3Decoder.h"
#include "RTCMDefines.h"
#include "RTCMDataTypes.h"
#include "RTCMSupportFunc.h"

#include "Common/DataTypes.h"
#include "Common/CommonConst.h"
#include "Common/GNSSSignal.h"

#include "Common/GNSSConstants.h"
#include "Common/GNSSNavDataType.h"
#include "Common/NavDataCommonFuncs.h"

#ifdef WIN32
#include <windows.h>
#endif

#define RTCM3_TYPE1001H_LEN 52
#define RTCM3_TYPE1001_LEN 58
#define RTCM3_TYPE1002_LEN 74
#define RTCM3_TYPE1003_LEN 101
#define RTCM3_TYPE1004_LEN 125
#define RTCM3_TYPE1104_LEN 52
#define RTCM3_TYPE1005_LEN 140
#define RTCM3_TYPE1006_LEN 156
#define RTCM3_TYPE1007B_LEN 28
#define RTCM3_TYPE1007C_LEN 8
#define RTCM3_TYPE1008B_LEN 36
#define RTCM3_TYPE1008C_LEN 8
#define RTCM3_TYPE1009H_LEN 49
#define RTCM3_TYPE1009_LEN 64
#define RTCM3_TYPE1010_LEN 79
#define RTCM3_TYPE1011_LEN 107
#define RTCM3_TYPE1012_LEN 130
#define RTCM3_TYPE1013_LEN   58
#define RTCM3_TYPE1019_LEN   476
#define RTCM3_TYPE1020_LEN   348
#define RTCM3_TYPE1021_LEN   412
#define RTCM3_TYPE1022_LEN   412
#define RTCM3_TYPE1023_LEN   578
#define RTCM3_TYPE1029_LEN   60
#define RTCM3_TYPE1033B_LEN  60
#define RTCM3_TYPE1033C_LEN  8
#define RTCM3_TYPE1041_LEN   470
#define RTCM3_TYPE1044_LEN   473
#define RTCM3_TYPE1045_LEN   484
#define RTCM3_TYPE1046_LEN   492
#define RTCM3_TYPE1047_LEN   476
#define RTCM3_TYPE1042_LEN   499
#define RTCM3_TYPE1300B_LEN 33
#define RTCM3_TYPE1300C_LEN 8
#define RTCM3_TYPE1301B_LEN 362
#define RTCM3_TYPE1301C_LEN 8
#define RTCM3_TYPE1302B_LEN 33
#define RTCM3_TYPE1302C_LEN 8
#define RTCM3_TYPE1303B_LEN 56
#define RTCM3_TYPE1303C_LEN 49
#define RTCM3_TYPE1304B_LEN 56
#define RTCM3_TYPE1304C_LEN 49

#define PSR_UNIT_GPS  299792.458  /* rtcm ver.3 unit of GPS pseudorange (m) */
#define PSR_UNIT_GLO  599584.916  /* rtcm ver.3 unit of glonass pseudorange (m) */

typedef enum _RTCM3_RTKDATA_INDEX_E
{
	RTCM1001_INDEX = 0,
	RTCM1002_INDEX = 1,
	RTCM1003_INDEX = 2,
	RTCM1004_INDEX = 3,
	RTCM1009_INDEX = 4,
	RTCM1010_INDEX = 5,
	RTCM1011_INDEX = 6,
	RTCM1012_INDEX = 7,
} RTCM3_RTKDATA_INDEX_E;

#define GET_RTKDATA_MASK_FORNT(RTKDataMsgMask, RTKDataIndex)            ((((RTKDataMsgMask) & (~(1 << (RTKDataIndex)))) >> (RTKDataIndex + 1)) ? TRUE : FALSE)
#define GET_RTKDATA_MASK_OTHER(RTKDataMsgMask, RTKDataIndex)            (((RTKDataMsgMask) & (~(1 << (RTKDataIndex)))) ? TRUE : FALSE)
#define GET_RTKDATA_MASK(RTKDataMsgMask, RTKDataIndex)                  (((RTKDataMsgMask) & (1 << (RTKDataIndex))) ? TRUE : FALSE)
#define SET_RTKDATA_MASK(RTKDataMsgMask, RTKDataIndex)                  ((RTKDataMsgMask) |= (1 << (RTKDataIndex)))
#define GET_GPSRTKDATA_MASK_OTHER(RTKDataMsgMask, RTKDataIndex)         ((((RTKDataMsgMask) & 0x0F) & (~(1 << (RTKDataIndex)))) ? TRUE : FALSE)
#define GET_GLORTKDATA_MASK_OTHER(RTKDataMsgMask, RTKDataIndex)         ((((RTKDataMsgMask) & 0xF0) & (~(1 << (RTKDataIndex)))) ? TRUE : FALSE)

#define CHECK_UTC_YEAR(Year)			((((Year) < 2020) || ((Year) > 2099)) ? FALSE : TRUE)

#if CONFIG_RTCMDECODETEST_ENABLE
/*****************************************************************************//**
@brief GPS time to epoch

@param week        [In] GPS week
@param sec         [In] GPS sec

@return epoch time

@author CHC
@date  2023/05/25
@note
*********************************************************************************/
static unsigned int GPSTime2Epoch(int week, int sec)
{
	UTC_TIME_T GPSTime0 = {/**< GPS time reference */
		.Year = 1980,
		.Month = 1,
		.Day = 6,
		.Hour = 0,
		.Minute = 0,
		.Second = 0};

	unsigned int TempTime = UTC2Epoch(&GPSTime0);

	TempTime += 86400 * 7 * week + sec;

	return TempTime;
}
#endif

/*****************************************************************************//**
@brief adjust carrier-phase rollover

@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param ExSystemID           [In] internal used system ID
@param SatID                [In] satellite id
@param FreqIndex            [In] ferq index
@param CarrierPhase         [In] carrier-phase

@return carrier-phase rollover

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
static double AdjCarrierPhase(RTCM_INTERIM_DATA_T* pRTCMInterimData, unsigned int ExSystemID, unsigned int SatID, int FreqIndex, double CarrierPhase)
{
	unsigned int SatIndex = SatID - 1;

	if (fabs(pRTCMInterimData->CarrierPhase[ExSystemID][SatIndex][FreqIndex]) < 1E-15)
	{

	}
	else if (CarrierPhase < pRTCMInterimData->CarrierPhase[ExSystemID][SatIndex][FreqIndex] - 750.0)
	{
		CarrierPhase += 1500.0;
	}
	else if (CarrierPhase > pRTCMInterimData->CarrierPhase[ExSystemID][SatIndex][FreqIndex] + 750.0)
	{
		CarrierPhase -= 1500.0;
	}

	pRTCMInterimData->CarrierPhase[ExSystemID][SatIndex][FreqIndex] = CarrierPhase;

	return CarrierPhase;
}

/*****************************************************************************//**
@brief RTK lock time indicator

@param LockTime         [In] obs lock time(s)

@return MSM lock time indicator

@author CHC
@date  2023/06/28
@note
*********************************************************************************/
static unsigned int GetRTKLockTime(unsigned int LockTime)
{
	if (LockTime < 24) return LockTime;
	if (LockTime < 48) return ((LockTime << 1) - 24);
	if (LockTime < 72) return ((LockTime << 2) - 120);
	if (LockTime < 96) return ((LockTime << 3) - 408);
	if (LockTime < 120) return ((LockTime << 4) - 1176);
	if (LockTime < 127) return ((LockTime << 5) - 3096);

	return 937;
}

/**********************************************************************//**
@brief  Get Int Pseudo Range

@param pRTCMInterimData    [In] pointer to rtcm interim data
@param ExSystemID          [In] GNSS system id
@param SatID               [In] Satellite id
@param pIntPseudoRange     [Out] Int Pseudo Range

@retval <0:error, =0:success

@author CHC
@date 2023/03/09
@note
**************************************************************************/
static int GetIntPseudoRange(RTCM_INTERIM_DATA_T* pRTCMInterimData, unsigned int ExSystemID, unsigned int SatID, int* pIntPseudoRange)
{
	int IntPseudoRange = 0;

#ifndef _NO_CTCLIB_
#ifndef MSG_CONVERT
	int GPSWeek, WeekMsCount, GPSLeapSecMs;
	double PosVel[6]; /**< ECEF-XYZ*/
	double SatClock;
	double Distance;
	BOOL Result = FALSE;
	RTCM_REFSTATION_INFO_T* pRefStationInfo = GetRTCMRefStationInfo();

	if ((!pRTCMInterimData) || (!pIntPseudoRange))
	{
		return -1;
	}

	if (IsAbsoluteDoubleZero(pRefStationInfo->Position[0]) && IsAbsoluteDoubleZero(pRefStationInfo->Position[1]) && IsAbsoluteDoubleZero(pRefStationInfo->Position[2]))
	{
		return -1;
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(&pRTCMInterimData->RTCMTime);
	UTCToGPSTime(&pRTCMInterimData->RTCMTime, &GPSLeapSecMs, &GPSWeek, &WeekMsCount);

	Result = GetSatellitePositionForBSW(GPSWeek, WeekMsCount, ExSystemID, SatID, PosVel, &SatClock);
	if (Result == FALSE)
	{
		return -1;
	}

	Distance = GetGeoDistanceXYZ(pRefStationInfo->Position, PosVel, NULL);

	if (ExSystemID == GPS)
	{
		IntPseudoRange=(int)floor(Distance / PSR_UNIT_GPS);
	}
	else if (ExSystemID == GLO)
	{
		IntPseudoRange=(int)floor(Distance / PSR_UNIT_GLO);
	}

#endif
#endif

	*pIntPseudoRange = IntPseudoRange;
	
	return 0;
}

/*****************************************************************************//**
@brief rtcm GPS RTK message header Log decoder function

@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param pSync                [In] sync
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM GPS RTK message header Log bit number
@param pSatNum              [Out] satellite number
@param pTow                 [Out] Tow

@return <0:error; =0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
static int RTCMGPSRTKMsgHeadDecoder(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, int* pSync,
												unsigned char* pBufIn, unsigned int TotalBitsLen, unsigned int* pBitNumOut,
												unsigned int* pSatNum, unsigned int* pTow)
{
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned int StationID;
	unsigned int TempTow;
	double Tow;

	if ((!pObsData) || (!pRTCMInterimData) || (!pSync) || (!pBufIn) || (!pBitNumOut) || (!pSatNum) || (!pTow))
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1001H_LEN <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn,     Index, 12);         Index += 12;
		TempTow = GetUnsignedBits(pBufIn,       Index, 30);         Index += 30;
		Tow = TempTow * 0.001;
		*pSync = GetUnsignedBits(pBufIn,        Index,  1);         Index +=  1;
		*pSatNum = GetUnsignedBits(pBufIn,        Index,  5);         Index +=  5;
		Tow = floor(Tow * 10.0 + 0.5) * 0.1;
		Index += 4;
#ifdef RTCM_DEBUG_INFO
		*pTow = TempTow;
#endif
	}
	else
	{
		return -3;
	}

	if (CheckStationID(pRTCMInterimData, StationID) || Tow > 604800.0)
	{
		return *pSync ? -1 : -2;
	}

	if (AdjWeek(pObsData, pRTCMInterimData, Tow) < 0)
	{
		return -1;
	}

	*pBitNumOut = Index;

	return 0;
}

/**********************************************************************//**
@brief  decode RTCM1001

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1001(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int ObsIndex = 0, TotalBitsLen;
	INT64 TempTimeDiff;
	double L1PseudoRange, L1CarrierPhase;
	int L1PhPsDiff;
	int L1IntPseudoRange = 0;
	unsigned int SatID, PRN, L1CodeInd, L1LockTime, ExSystemID, SatIndex;
	unsigned int SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0;
	unsigned int GPSSatNum = 0, SBASatNum = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGPSRTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1001, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1001;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1001_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1001_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1001_INDEX);
	}

	if (GET_GPSRTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1001_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GPS];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GPS] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1001_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 24); Index += 24;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;

		if (SatID < 40)
		{
			ExSystemID = GPS;
			PRN = SatID;
#ifdef RTCM_DEBUG_INFO
			GPSSatNum++;
#endif
		}
		else
		{
			ExSystemID = SBA;
			PRN = SatID + 80;
			SatID -= 39;
#ifdef RTCM_DEBUG_INFO
			SBASatNum++;
#endif
		}

		if (ExSystemID == GPS)
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA;
		}
		else
		{
			SignalType = RM_RANGE_SIGNAL_SBASL1C;
		}
		if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
		{
			continue;
		}

		pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
		pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

		pRMRangeData->PRN = PRN;

		if (GetIntPseudoRange(pRTCMInterimData, ExSystemID, SatID, &L1IntPseudoRange) < 0)
		{
			continue;
		}
		L1PseudoRange = L1PseudoRange * 0.02 + L1IntPseudoRange * PSR_UNIT_GPS;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			pRMRangeData->PSR = L1PseudoRange;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L1, L1PhPsDiff * 0.0005 / GPS_WAVELENGTH_L1);
			pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L1 + L1CarrierPhase);
		}
		L1LockTime = GetRTKLockTime(L1LockTime);
		pRMRangeData->LockTime = (float)L1LockTime;
		PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L1], L1LockTime * 1000);
		if (ExSystemID == GPS)
		{
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
		}
		else
		{
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_SBA, SignalType, 0);
		}

#ifdef RTCM_DEBUG_INFO
		if (1 == L1CodeInd) /**< L1 P */
		{
			L1PValid = 1;
		}
		else
		{
			L1CAValid = 1;
		}
#endif
		if (pPerSatObsData->ObsNum == 0)
		{
			SatObsDataSatNumAdd(pSatObsData);
		}

		if (PerSatObsIndex == pPerSatObsData->ObsNum)
		{
			PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
		}

		if (ExSystemID == GPS)
		{
			if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
			{
				break;
			}
		}

		ObsIndex++;
	}

	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1001) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1001_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1001, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GPS] += GPSSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_SBA] += SBASatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GPS);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1002

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1002(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	double L1PseudoRange, L1CarrierPhase;
	INT64 TempTimeDiff;
	int Sync, L1PhPsDiff, Result;
	unsigned int SatNum, SatID, PRN, L1CodeInd, L1LockTime, L1IntPseudoRange, ExSystemID, SatIndex, ObsIndex = 0;
	unsigned int L1Cnr, TotalBitsLen, SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0;
	unsigned int GPSSatNum = 0, SBASatNum = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGPSRTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1002, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1002;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1002_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1002_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1002_INDEX);
	}

	if (GET_GPSRTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1002_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GPS];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GPS] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1002_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 24); Index += 24;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L1IntPseudoRange = GetUnsignedBits(pBufIn,  Index,  8); Index +=  8;
		L1Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;

		if (SatID < 40)
		{
			ExSystemID = GPS;
			PRN = SatID;
#ifdef RTCM_DEBUG_INFO
			GPSSatNum++;
#endif
		}
		else
		{
			ExSystemID = SBA;
			PRN = SatID + 80;
			SatID -= 39;
#ifdef RTCM_DEBUG_INFO
			SBASatNum++;
#endif
		}

		if (ExSystemID == GPS)
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA;
		}
		else
		{
			SignalType = RM_RANGE_SIGNAL_SBASL1C;
		}
		if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
		{
			continue;
		}

		pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
		pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

		pRMRangeData->PRN = PRN;

		L1PseudoRange = L1PseudoRange * 0.02 + L1IntPseudoRange * PSR_UNIT_GPS;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			pRMRangeData->PSR = L1PseudoRange;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L1, L1PhPsDiff * 0.0005 / GPS_WAVELENGTH_L1);
			pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L1 + L1CarrierPhase);
		}
		L1LockTime = GetRTKLockTime(L1LockTime);
		pRMRangeData->LockTime = (float)L1LockTime;
		PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L1], L1LockTime * 1000);
		pRMRangeData->CN0 = (float)DoubleIScale2((int)L1Cnr, 2);
		if (ExSystemID == GPS)
		{
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
		}
		else
		{
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_SBA, SignalType, 0);
		}

#ifdef RTCM_DEBUG_INFO
		if (1 == L1CodeInd) /**< L1 P */
		{
			L1PValid = 1;
		}
		else
		{
			L1CAValid = 1;
		}
#endif
		if (pPerSatObsData->ObsNum == 0)
		{
			SatObsDataSatNumAdd(pSatObsData);
		}

		if (PerSatObsIndex == pPerSatObsData->ObsNum)
		{
			PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
		}

		if (ExSystemID == GPS)
		{
			if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
			{
				break;
			}
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1002) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1002_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1002, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GPS] += GPSSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_SBA] += SBASatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GPS);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1003

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1003(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	const int L2Codes[] = {RM_RANGE_SIGNAL_GPSL2CA, RM_RANGE_SIGNAL_GPSL2P, RM_RANGE_SIGNAL_GPSL2PCODELESS, RM_RANGE_SIGNAL_GPSL2PCODELESS};
	double L1PseudoRange, L1CarrierPhase, L2CarrierPhase;
	INT64 TempTimeDiff;
	int L1IntPseudoRange = 0;
	int PseudoRangeDiff, L1PhPsDiff, L2PhL1PsDiff;
	unsigned int SatIndex, ObsIndex = 0, SatID, PRN, L1CodeInd, L2CodeInd;
	unsigned int L1LockTime, L2LockTime, ExSystemID;
	unsigned int TotalBitsLen, SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0, L2WValid = 0, L2CXValid = 0;
	unsigned int GPSSatNum = 0, SBASatNum = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGPSRTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1003, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1003;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1003_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1003_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1003_INDEX);
	}

	if (GET_GPSRTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1003_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GPS];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GPS] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && SatIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1003_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 24); Index += 24;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L2CodeInd = GetUnsignedBits(pBufIn,         Index,  2); Index +=  2;
		PseudoRangeDiff = GetUnsignedBits(pBufIn,   Index, 14); Index += 14;
		L2PhL1PsDiff = GetSignedBits(pBufIn,        Index, 20); Index += 20;
		L2LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;

		if (SatID < 40)
		{
			ExSystemID = GPS;
			PRN = SatID;
#ifdef RTCM_DEBUG_INFO
			GPSSatNum++;
#endif
		}
		else
		{
			ExSystemID = SBA;
			PRN = SatID + 80;
			SatID -= 39;
#ifdef RTCM_DEBUG_INFO
			SBASatNum++;
#endif
		}

		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		L1PseudoRange = L1PseudoRange * 0.02;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			if (ExSystemID == GPS)
			{
				SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA;
			}
			else
			{
				SignalType = RM_RANGE_SIGNAL_SBASL1C;
			}
			if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			if (GetIntPseudoRange(pRTCMInterimData, ExSystemID, SatID, &L1IntPseudoRange) < 0)
			{
				continue;
			}

			pRMRangeData->PSR = L1PseudoRange + L1IntPseudoRange * PSR_UNIT_GPS;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L1, L1PhPsDiff * 0.0005 / GPS_WAVELENGTH_L1);
			pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L1], L1LockTime * 1000);
			if (ExSystemID == GPS)
			{
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
			}
			else
			{
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_SBA, SignalType, 0);
			}
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
			}
		}

		if (ExSystemID == GPS)
		{
			if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
			{
				break;
			}

			pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

			if ((PseudoRangeDiff != (int)0xFFFFE000) && /**< 0xFFFFE000 : L2-L1 Pseudorange Difference is invalid*/
				(L2PhL1PsDiff != (int)0xFFF80000)) /**< 0xFFF80000 : L2 Phaserange - L1 Pseudorange is invalid*/
			{
				SignalType = (unsigned char)L2Codes[L2CodeInd];
				if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
				{
					continue;
				}

				pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
				pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

				pRMRangeData->PRN = PRN;

				pRMRangeData->PSR = L1PseudoRange + PseudoRangeDiff * 0.02;

				L2CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L2, L2PhL1PsDiff * 0.0005 / GPS_WAVELENGTH_L2);
				pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L2 + L2CarrierPhase);

				L2LockTime = GetRTKLockTime(L2LockTime);
				pRMRangeData->LockTime = (float)L2LockTime;
				PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L2], L2LockTime * 1000);
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
				if (1 == L2CodeInd) /**< L2 P */
				{
					L2WValid = 1;
				}
				else
				{
					L2CXValid = 1;
				}
#endif
				if (pPerSatObsData->ObsNum == 0)
				{
					SatObsDataSatNumAdd(pSatObsData);
				}

				if (PerSatObsIndex == pPerSatObsData->ObsNum)
				{
					PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
				}
			}
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1003) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1003_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid + L2WValid + L2CXValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1003, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GPS] += GPSSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_SBA] += SBASatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GPS);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1004

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1004(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int TotalBitsLen;
	const int L2Codes[] = {RM_RANGE_SIGNAL_GPSL2CA, RM_RANGE_SIGNAL_GPSL2P, RM_RANGE_SIGNAL_GPSL2PCODELESS, RM_RANGE_SIGNAL_GPSL2PCODELESS};
	double L1PseudoRange, L1CarrierPhase, L2CarrierPhase;
	INT64 TempTimeDiff;
	int PseudoRangeDiff, L1PhPsDiff, L2PhL1PsDiff;
	unsigned int SatIndex, ObsIndex = 0, PRN, SatID, L1CodeInd, L2CodeInd;
	unsigned int L1LockTime, L2LockTime, L1IntPsRang, ExSystemID;
	unsigned int L1Cnr, L2Cnr, SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0, L2WValid = 0, L2CXValid = 0;
	unsigned int GPSSatNum = 0, SBASatNum = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGPSRTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1004, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1004;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1004_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1004_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1004_INDEX);
	}

	if (GET_GPSRTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1004_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GPS];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GPS] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1004_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 24); Index += 24;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L1IntPsRang = GetUnsignedBits(pBufIn,       Index,  8); Index +=  8;
		L1Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;
		L2CodeInd = GetUnsignedBits(pBufIn,         Index,  2); Index +=  2;
		PseudoRangeDiff = GetSignedBits(pBufIn,     Index, 14); Index += 14;
		L2PhL1PsDiff = GetSignedBits(pBufIn,        Index, 20); Index += 20;
		L2LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L2Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;

		if (SatID < 40)
		{
			ExSystemID = GPS;
			PRN = SatID;
#ifdef RTCM_DEBUG_INFO
			GPSSatNum++;
#endif
		}
		else
		{
			ExSystemID = SBA;
			PRN = SatID + 80;
			SatID -= 39;
#ifdef RTCM_DEBUG_INFO
			SBASatNum++;
#endif
		}

		L1PseudoRange = L1PseudoRange * 0.02 + L1IntPsRang * PSR_UNIT_GPS;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			if (ExSystemID == GPS)
			{
				SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA;
			}
			else
			{
				SignalType = RM_RANGE_SIGNAL_SBASL1C;
			}
			if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			pRMRangeData->PSR = L1PseudoRange;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L1, L1PhPsDiff * 0.0005 / GPS_WAVELENGTH_L1);
			pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L1], L1LockTime * 1000);
			pRMRangeData->CN0 = (float)DoubleIScale2((int)L1Cnr, 2);
			if (ExSystemID == GPS)
			{
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
			}
			else
			{
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_SBA, SignalType, 0);
			}
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif

			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
			}
		}

		if (ExSystemID == GPS)
		{
			if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
			{
				break;
			}

			if ((PseudoRangeDiff != (int)0xFFFFE000) && /**< 0xFFFFE000 : L2-L1 Pseudorange Difference is invalid*/
				(L2PhL1PsDiff != (int)0xFFF80000)) /**< 0xFFF80000 : L2 Phaserange - L1 Pseudorange is invalid*/
			{
				SignalType = (unsigned char)L2Codes[L2CodeInd];
				if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
				{
					continue;
				}

				pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
				pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

				pRMRangeData->PRN = PRN;

				pRMRangeData->PSR = L1PseudoRange + PseudoRangeDiff * 0.02;

				L2CarrierPhase = AdjCarrierPhase(pRTCMInterimData, ExSystemID, SatID, L2, L2PhL1PsDiff * 0.0005 / GPS_WAVELENGTH_L2);
				pRMRangeData->ADR = -(L1PseudoRange / GPS_WAVELENGTH_L2 + L2CarrierPhase);

				L2LockTime = GetRTKLockTime(L2LockTime);
				pRMRangeData->LockTime = (float)L2LockTime;
				PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][L2], L2LockTime * 1000);
				pRMRangeData->CN0 = (float)DoubleIScale2((int)L2Cnr, 2);
				pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GPS, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
				if (1 == L2CodeInd) /**< L2 P */
				{
					L2WValid = 1;
				}
				else
				{
					L2CXValid = 1;
				}
#endif

				if (pPerSatObsData->ObsNum == 0)
				{
					SatObsDataSatNumAdd(pSatObsData);
				}

				if (PerSatObsIndex == pPerSatObsData->ObsNum)
				{
					PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
				}
			}
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1004) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1004_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid + L2WValid + L2CXValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1004, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GPS] += GPSSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_SBA] += SBASatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GPS);

	return pRTCMInterimData->ObsCompleteFlag;
}

/*****************************************************************************//**
@brief rtcm GLONASS RTK message header Log decoder function

@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param pSync                [In] sync
@param pBufIn               [In] RTCM3 log data buffer
@param TotalBitsLen         [In] RTCM3 log data total bits
@param pBitNumOut           [Out] RTCM GLONASS RTK message header Log bit number
@param pSatNum              [Out] satellite number
@param pTow                 [Out] Tow

@return <0:error; =0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
static int RTCMGLORTKMsgHeadDecoder(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, int* pSync, unsigned char* pBufIn,
													unsigned int TotalBitsLen, unsigned int* pBitNumOut, unsigned int* pSatNum, unsigned int* pTow)
{
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned int StationID;
	unsigned int TempTow;
	double Tod;

	if ((!pObsData) || (!pRTCMInterimData) || (!pSync) || (!pBufIn) || (!pBitNumOut) || (!pSatNum) || (!pTow))
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1009H_LEN <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn,     Index, 12);         Index += 12;
		TempTow = GetUnsignedBits(pBufIn,           Index, 27);     Index += 27;
		Tod = TempTow * 0.001;
		*pSync = GetUnsignedBits(pBufIn,        Index,  1);         Index +=  1;
		*pSatNum = GetUnsignedBits(pBufIn,        Index,  5);         Index +=  5;
		Tod = floor(Tod * 10.0 + 0.5) * 0.1;
		Index += 4;
#ifdef RTCM_DEBUG_INFO
		*pTow = TempTow;
#endif
	}
	else
	{
		return -3;
	}
	/** check station id */
	if (CheckStationID(pRTCMInterimData, StationID) || (Tod > 86400.0))
	{
		return *pSync ? -1 : -2;
	}

	if (AdjGLOTimeDaily(pObsData, pRTCMInterimData, Tod, -1) < 0)
	{
		return -1;
	}

	*pBitNumOut = Index;

	return 0;
}

/**********************************************************************//**
@brief  decode RTCM1009

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1009(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int TotalBitsLen;
	double L1PseudoRange, L1CarrierPhase, Lam1;
	int L1PhPsDiff, L1IntPseudoRange = 0;
	unsigned int SatIndex, ObsIndex = 0, SatID, L1CodeInd, SatFreqChnNum, L1LockTime;
	unsigned int SignalType, PhaseLockFlag;
	INT64 TempTimeDiff;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGLORTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1009, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1009;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1009_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1009_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1009_INDEX);
	}

	if (GET_GLORTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1009_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GLO];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GLO] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1009_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		SatFreqChnNum = GetUnsignedBits(pBufIn,     Index,  5); Index +=  5;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 25); Index += 25;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;

		if (SatID == 0)
		{
			continue;
		}

		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		L1PseudoRange = L1PseudoRange * 0.02;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = SatID + 37;

			if (GetIntPseudoRange(pRTCMInterimData, GLO, SatID, &L1IntPseudoRange) < 0)
			{
				continue;
			}

			pRMRangeData->PSR = L1PseudoRange+ L1IntPseudoRange * PSR_UNIT_GLO;
			Lam1 = GLO_WAVELENGTH_G1((int)SatFreqChnNum);
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G1, L1PhPsDiff * 0.0005 / Lam1);
			pRMRangeData->ADR = -(L1PseudoRange / Lam1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G1], L1LockTime * 1000);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
		{
			break;
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1009) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1009_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GLO Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1009, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GLO] += SatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GLO);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1010

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1010(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int TotalBitsLen;
	double L1PseudoRange, L1CarrierPhase, Lam1;
	INT64 TempTimeDiff;
	int L1PhPsDiff;
	unsigned int SatIndex, ObsIndex = 0, SatID, L1CodeInd, SatFreqChnNum, L1LockTime, L1IntPseudoRange;
	unsigned int L1Cnr, SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGLORTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1010, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1010;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1010_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1010_INDEX))
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1010_INDEX);

	if (GET_GLORTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1010_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GLO];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GLO] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1010_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		SatFreqChnNum = GetUnsignedBits(pBufIn,     Index,  5); Index +=  5;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 25); Index += 25;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L1IntPseudoRange = GetUnsignedBits(pBufIn,  Index,  7); Index +=  7;
		L1Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;

		if (SatID == 0)
		{
			continue;
		}

		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		L1PseudoRange = L1PseudoRange * 0.02 + L1IntPseudoRange * PSR_UNIT_GLO;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = SatID + 37;

			pRMRangeData->PSR = L1PseudoRange;
			Lam1 = GLO_WAVELENGTH_G1((int)SatFreqChnNum);
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G1, L1PhPsDiff * 0.0005 / Lam1);
			pRMRangeData->ADR = -(L1PseudoRange / Lam1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G1], L1LockTime * 1000);
			pRMRangeData->CN0 = (float)DoubleIScale2((int)L1Cnr, 2);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
		{
			break;
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1010) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1010_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GLO Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1010, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GLO] += SatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GLO);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1011

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1011(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int TotalBitsLen, SignalType;
	double L1PseudoRange, L1CarrierPhase, L2CarrierPhase, Lam1, Lam2;
	INT64 TempTimeDiff;
	int L1IntPseudoRange = 0;
	int PseudoRangeDiff, L1PhPsDiff, L2PhL1PsDiff;
	unsigned int SatIndex, ObsIndex = 0, PRN, SatID, SatFreqChnNum, L1CodeInd, L2CodeInd;
	unsigned int L1LockTime, L2LockTime, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0, L2PValid = 0, L2CValid = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGLORTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1011, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1011;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1011_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1011_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1011_INDEX);
	}

	if (GET_GLORTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1011_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GLO];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GLO] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1011_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		SatFreqChnNum = GetUnsignedBits(pBufIn,     Index,  5); Index +=  5;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 25); Index += 25;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L2CodeInd = GetUnsignedBits(pBufIn,         Index,  2); Index +=  2;
		PseudoRangeDiff = GetSignedBits(pBufIn,     Index, 14); Index += 14;
		L2PhL1PsDiff = GetSignedBits(pBufIn,        Index, 20); Index += 20;
		L2LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;

		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		PRN = SatID + 37;

		L1PseudoRange = L1PseudoRange * 0.02;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			Lam1 = GLO_WAVELENGTH_G1((int)SatFreqChnNum);

			if (GetIntPseudoRange(pRTCMInterimData, GLO, SatID, &L1IntPseudoRange) < 0)
			{
				continue;
			}
			pRMRangeData->PSR = L1PseudoRange+ L1IntPseudoRange * PSR_UNIT_GLO;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G1, L1PhPsDiff * 0.0005 / Lam1);
			pRMRangeData->ADR = -(L1PseudoRange / Lam1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G1], L1LockTime * 1000);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
		{
			break;
		}

		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		if ((PseudoRangeDiff != (int)0xFFFFE000) &&/**< 0xFFFFE000 : L2-L1 Pseudorange Difference is invalid*/
			(L2PhL1PsDiff != (int)0xFFF80000)) /**< 0xFFF80000 : L2 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L2CodeInd ? RM_RANGE_SIGNAL_GLOL2P : RM_RANGE_SIGNAL_GLOL2C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			pRMRangeData->PSR = L1PseudoRange + PseudoRangeDiff * 0.02;

			Lam2 = GLO_WAVELENGTH_G2((int)SatFreqChnNum);
			L2CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G2, L2PhL1PsDiff * 0.0005 / Lam2);
			pRMRangeData->ADR = -(L1PseudoRange / Lam2 + L2CarrierPhase);

			L2LockTime = GetRTKLockTime(L2LockTime);
			pRMRangeData->LockTime = (float)L2LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G2], L2LockTime * 1000);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L2CodeInd) /**< L2 P */
			{
				L2PValid = 1;
			}
			else
			{
				L2CValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1011) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1011_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid + L2PValid + L2CValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GLO Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1011, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GLO] += SatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GLO);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1012

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1012(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	int Sync, Result;
	unsigned int SatNum;
	unsigned int TotalBitsLen;
	double L1PseudoRange, L1CarrierPhase, L2CarrierPhase, Lam1, Lam2;
	INT64 TempTimeDiff;
	int PseudoRangeDiff, L1PhPsDiff, L2PhL1PsDiff;
	unsigned int SatIndex, ObsIndex = 0, PRN, SatID, SatFreqChnNum, L1CodeInd, L2CodeInd;
	unsigned int L1LockTime, L2LockTime, L1IntPsRang;
	unsigned int L1Cnr, L2Cnr, SignalType, PhaseLockFlag;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int Tow;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int L1PValid = 0, L1CAValid = 0, L2PValid = 0, L2CValid = 0;
#endif

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pSatObsData = &pRTCMInterimData->SatObsData;

	if ((Result = RTCMGLORTKMsgHeadDecoder(pObsData, pRTCMInterimData, &Sync, pBufIn, TotalBitsLen, &Index, &SatNum, &Tow)) < 0)
	{
		if (Result == -2)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Sync ? 0 : -1;
		}
		else if (Result == -3)
		{
			pRTCMInterimData->ObsCompleteFlag = !Sync;
			return Result;
		}

		return -1;
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			DecodeObsOverTimeCallback(1012, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		pRTCMInterimData->RTKDataMask = 0;
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;

	if (pRTCMInterimData->RTKDataCompleteTypeID == 0)
	{
		pRTCMInterimData->RTKDataCompleteTypeID = pRTCMInterimData->RTKDataCompleteTypeIDLast;
	}

	pRTCMInterimData->RTKDataCompleteTypeIDLast = 1012;

	SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMask, RTCM1012_INDEX);
	if (!GET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1012_INDEX))
	{
		SET_RTKDATA_MASK(pRTCMInterimData->RTKDataMaskLast, RTCM1012_INDEX);
	}

	if (GET_GLORTKDATA_MASK_OTHER(pRTCMInterimData->RTKDataMask, RTCM1012_INDEX))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[GLO];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[GLO] = ObsIndex;
	}

	for (SatIndex = 0; SatIndex < SatNum && ObsIndex < RM_RANGE_MAX_OBS_NUM && Index + RTCM3_TYPE1012_LEN <= TotalBitsLen; SatIndex++)
	{
		SatID = GetUnsignedBits(pBufIn,             Index,  6); Index +=  6;
		L1CodeInd = GetUnsignedBits(pBufIn,         Index,  1); Index +=  1;
		SatFreqChnNum = GetUnsignedBits(pBufIn,     Index,  5); Index +=  5;
		L1PseudoRange = GetUnsignedBits(pBufIn,     Index, 25); Index += 25;
		L1PhPsDiff = GetSignedBits(pBufIn,          Index, 20); Index += 20;
		L1LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L1IntPsRang = GetUnsignedBits(pBufIn,       Index,  7); Index +=  7;
		L1Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;
		L2CodeInd = GetUnsignedBits(pBufIn,         Index,  2); Index +=  2;
		PseudoRangeDiff = GetSignedBits(pBufIn,     Index, 14); Index += 14;
		L2PhL1PsDiff = GetSignedBits(pBufIn,        Index, 20); Index += 20;
		L2LockTime = GetUnsignedBits(pBufIn,        Index,  7); Index +=  7;
		L2Cnr = GetUnsignedBits(pBufIn,             Index,  8); Index +=  8;


		PRN = SatID + 37;

		L1PseudoRange = L1PseudoRange * 0.02 + L1IntPsRang * PSR_UNIT_GLO;
		if (L1PhPsDiff != (int)0xFFF80000) /**< 0xFFF80000 : L1 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L1CodeInd ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			Lam1 = GLO_WAVELENGTH_G1((int)SatFreqChnNum);
			pRMRangeData->PSR = L1PseudoRange;
			L1CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G1, L1PhPsDiff * 0.0005 / Lam1);
			pRMRangeData->ADR = -(L1PseudoRange / Lam1 + L1CarrierPhase);

			L1LockTime = GetRTKLockTime(L1LockTime);
			pRMRangeData->LockTime = (float)L1LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G1], L1LockTime * 1000);
			pRMRangeData->CN0 = (float)DoubleIScale2((int)L1Cnr, 2);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L1CodeInd) /**< L1 P */
			{
				L1PValid = 1;
			}
			else
			{
				L1CAValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		if (++ObsIndex >= RM_RANGE_MAX_OBS_NUM)
		{
			break;
		}


		if ((PseudoRangeDiff != (int)0xFFFFE000) && /**< 0xFFFFE000 : L2-L1 Pseudorange Difference is invalid*/
			(L2PhL1PsDiff != (int)0xFFF80000)) /**< 0xFFF80000 : L2 Phaserange - L1 Pseudorange is invalid*/
		{
			SignalType = L2CodeInd ? RM_RANGE_SIGNAL_GLOL2P : RM_RANGE_SIGNAL_GLOL2C;
			if (GetPerSatObsDataIndex(pSatObsData, GLO, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = PRN;

			pRMRangeData->PSR = L1PseudoRange + PseudoRangeDiff * 0.02;

			Lam2 = GLO_WAVELENGTH_G2((int)SatFreqChnNum);
			L2CarrierPhase = AdjCarrierPhase(pRTCMInterimData, GLO, SatID, G2, L2PhL1PsDiff * 0.0005 / Lam2);
			pRMRangeData->ADR = -(L1PseudoRange / Lam2 + L2CarrierPhase);

			L2LockTime = GetRTKLockTime(L2LockTime);
			pRMRangeData->LockTime = (float)L2LockTime;
			PhaseLockFlag = LossOfLock(&pRTCMInterimData->LockTime[GLO][SatID-1][G2], L2LockTime * 1000);
			pRMRangeData->CN0 = (float)DoubleIScale2((int)L2Cnr, 2);
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, RM_RANGE_SYS_GLO, SignalType, 0);
#ifdef RTCM_DEBUG_INFO
			if (1 == L2CodeInd) /**< L2 P */
			{
				L2PValid = 1;
			}
			else
			{
				L2CValid = 1;
			}
#endif
			if (pPerSatObsData->ObsNum == 0)
			{
				SatObsDataSatNumAdd(pSatObsData);
			}

			if (PerSatObsIndex == pPerSatObsData->ObsNum)
			{
				PerSatObsDataObsNumAdd(pPerSatObsData, GLO, SatID);
			}
		}

		ObsIndex++;
	}


	if (!Sync)
	{
		if ((pRTCMInterimData->RTKDataCompleteTypeID == 1012) ||
			(!GET_RTKDATA_MASK_FORNT(pRTCMInterimData->RTKDataMaskLast, RTCM1012_INDEX)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_RTKDATA);
			pRTCMInterimData->ObsCompleteFlag = 1;
		}
		else
		{
			pRTCMInterimData->ObsCompleteFlag = 0;
		}
	}
	else
	{
		pRTCMInterimData->ObsCompleteFlag = 0;
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CAValid + L2PValid + L2CValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GLO Obs :Epoch %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %d",
		1012, Tow, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCM_SYS_GLO] += SatNum;
	if (pRTCMInterimData->ObsCompleteFlag)
	{
		MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
			pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
			pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
			pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
			pBaseObsSatNumBuf[RTCM_SYS_NIC]);
		pBaseObsSatCountBuf[MsgLen] = '\0';
		pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
		MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);
	}
#endif

	pObsData->ObsSystemMask |= (1 << GLO);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  decode RTCM1005

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1005(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int StationID;
	unsigned int TotalBitsLen;
	RTCM_ANTENNA_INFO_T* pAntennaInfo;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pAntennaInfo = (RTCM_ANTENNA_INFO_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1005_LEN <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn,                                 Index, 12);         Index += 12;
		pAntennaInfo->ITRF = GetUnsignedBits(pBufIn,                        Index,  6);         Index +=  6 ;
		pAntennaInfo->GPSIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->GLOIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->GALIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->StaionIndicator = GetUnsignedBits(pBufIn,             Index,  1);         Index +=  1;
		pAntennaInfo->Position[0] = Get38Bits(pBufIn,                       Index) * 0.0001;    Index += 38;
		pAntennaInfo->ReceiverIndicator = GetUnsignedBits(pBufIn,           Index,  1);         Index +=  1 + 1;
		pAntennaInfo->Position[1] = Get38Bits(pBufIn,                       Index) * 0.0001;    Index += 38;
		pAntennaInfo->QuarterCycleIndicator = GetUnsignedBits(pBufIn,       Index,  2);         Index += 2;
		pAntennaInfo->Position[2] = Get38Bits(pBufIn,                       Index) * 0.0001;
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	pAntennaInfo->StationID = StationID;
	pAntennaInfo->AntennaHeight = 0;

	pRTCMInterimData->RefVendorInfo.QuarterCycleIndicator = pAntennaInfo->QuarterCycleIndicator;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned char SysFlag = (pAntennaInfo->GPSIndicator << GPS) | (pAntennaInfo->GLOIndicator << GLO) | (pAntennaInfo->GALIndicator << GAL);
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u %s F%u Sys %u SingleClock:%u 1/4Cycle:%u %lf %lf %lf %lf",
		1005, pAntennaInfo->StationID, (pAntennaInfo->StaionIndicator == 0) ? "Physical" : "VRS", pAntennaInfo->StaionIndicator,
		SysFlag, pAntennaInfo->ReceiverIndicator, pAntennaInfo->QuarterCycleIndicator,
		pAntennaInfo->Position[0], pAntennaInfo->Position[1], pAntennaInfo->Position[2], pAntennaInfo->AntennaHeight);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1006

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1006(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int StationID;
	unsigned int TotalBitsLen;
	RTCM_ANTENNA_INFO_T* pAntennaInfo;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pAntennaInfo = (RTCM_ANTENNA_INFO_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1006_LEN <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn,                                 Index, 12);         Index += 12;
		pAntennaInfo->ITRF = GetUnsignedBits(pBufIn,                        Index,  6);         Index +=  6 ;
		pAntennaInfo->GPSIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->GLOIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->GALIndicator = GetUnsignedBits(pBufIn,                Index,  1);         Index +=  1;
		pAntennaInfo->StaionIndicator = GetUnsignedBits(pBufIn,             Index,  1);         Index +=  1;
		pAntennaInfo->Position[0] = Get38Bits(pBufIn,                       Index) * 0.0001;    Index += 38;
		pAntennaInfo->ReceiverIndicator = GetUnsignedBits(pBufIn,           Index,  1);         Index +=  1 + 1;
		pAntennaInfo->Position[1] = Get38Bits(pBufIn,                       Index) * 0.0001;    Index += 38;
		pAntennaInfo->QuarterCycleIndicator = GetUnsignedBits(pBufIn,       Index,  2);         Index += 2;
		pAntennaInfo->Position[2] = Get38Bits(pBufIn,                       Index) * 0.0001;    Index += 38;
		pAntennaInfo->AntennaHeight = GetUnsignedBits(pBufIn,               Index, 16) * 0.0001;
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	pAntennaInfo->StationID = StationID;

	pRTCMInterimData->RefVendorInfo.QuarterCycleIndicator = pAntennaInfo->QuarterCycleIndicator;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned char SysFlag = (pAntennaInfo->GPSIndicator << GPS) | (pAntennaInfo->GLOIndicator << GLO) | (pAntennaInfo->GALIndicator << GAL);
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u %s F%u Sys %u SingleClock:%u 1/4Cycle:%u %lf %lf %lf %lf",
		1006, pAntennaInfo->StationID, (pAntennaInfo->StaionIndicator == 0) ? "Physical" : "VRS", pAntennaInfo->StaionIndicator,
		SysFlag, pAntennaInfo->ReceiverIndicator, pAntennaInfo->QuarterCycleIndicator,
		pAntennaInfo->Position[0], pAntennaInfo->Position[1], pAntennaInfo->Position[2], pAntennaInfo->AntennaHeight);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1007

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1007(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	char AntennaDescriptor[RTCM_ANTENNA_DESCRIPTOR_LEN_MAX] = "";
	unsigned int StationID, AntennaDescriptorCntr;
	unsigned int TotalBitsLen;
	RTCM_RECEIVER_DESCRIPTOR_INFO_T* pReceiverInfo;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pReceiverInfo = (RTCM_RECEIVER_DESCRIPTOR_INFO_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	AntennaDescriptorCntr = GetUnsignedBits(pBufIn, Index + 12, 8);

	if (AntennaDescriptorCntr >= RTCM_ANTENNA_DESCRIPTOR_LEN_MAX)
	{
		return -1;
	}

	if ((Index + RTCM3_TYPE1007B_LEN + RTCM3_TYPE1007C_LEN * AntennaDescriptorCntr) <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn, Index, 12);                             Index += 12 + 8;

		MEMCPY(AntennaDescriptor, pBufIn + (Index >> 3), AntennaDescriptorCntr);    Index += AntennaDescriptorCntr << 3;

		pReceiverInfo->AntennaID = GetUnsignedBits(pBufIn, Index, 8);               Index +=  8;
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	strncpy(pReceiverInfo->AntennaDescriptor, AntennaDescriptor, AntennaDescriptorCntr);
	pReceiverInfo->AntennaDescriptor[AntennaDescriptorCntr] = '\0';
	pReceiverInfo->AntennaSN[0] = '\0';
	pReceiverInfo->ReceiverDescriptor[0] = '\0';
	pReceiverInfo->ReceiverFirmVer[0] = '\0';
	pReceiverInfo->ReceiverSN[0] = '\0';

	pReceiverInfo->StationID = StationID;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Receiver:RefS %u SetupID:%3u, |A %u %s",
		1007, pReceiverInfo->StationID, pReceiverInfo->AntennaID, AntennaDescriptorCntr, pReceiverInfo->AntennaDescriptor);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1008

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1008(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int StationID, AntennaDescriptorCntr, AntennaSNCntr, AntennaID;
	unsigned int TotalBitsLen;
	char AntennaDescriptor[RTCM_ANTENNA_DESCRIPTOR_LEN_MAX] = "", AntennaSN[RTCM_ANTENNA_SN_LEN_MAX] = "";
	RTCM_RECEIVER_DESCRIPTOR_INFO_T* pReceiverInfo;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pReceiverInfo = (RTCM_RECEIVER_DESCRIPTOR_INFO_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	AntennaDescriptorCntr = GetUnsignedBits(pBufIn, Index+12, 8);
	if (AntennaDescriptorCntr >= RTCM_ANTENNA_DESCRIPTOR_LEN_MAX)
	{
		return -1;
	}

	AntennaSNCntr = GetUnsignedBits(pBufIn, Index + 28 + 8 * AntennaDescriptorCntr, 8);
	if (AntennaSNCntr >= RTCM_ANTENNA_SN_LEN_MAX)
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1008B_LEN + RTCM3_TYPE1008C_LEN * (AntennaDescriptorCntr + AntennaSNCntr) <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn, Index, 12);                             Index += 12 + 8;

		MEMCPY(AntennaDescriptor, pBufIn + (Index >> 3), AntennaDescriptorCntr);    Index += AntennaDescriptorCntr << 3;

		AntennaID = GetUnsignedBits(pBufIn, Index, 8);                              Index +=  8 + 8;

		MEMCPY(AntennaSN, pBufIn + (Index >> 3), AntennaSNCntr);
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	strncpy(pReceiverInfo->AntennaDescriptor, AntennaDescriptor, AntennaDescriptorCntr);
	pReceiverInfo->AntennaDescriptor[AntennaDescriptorCntr] = '\0';
	pReceiverInfo->AntennaID = AntennaID;
	strncpy(pReceiverInfo->AntennaSN, AntennaSN, AntennaSNCntr);
	pReceiverInfo->AntennaSN[AntennaSNCntr] = '\0';
	pReceiverInfo->ReceiverDescriptor[0] = '\0';
	pReceiverInfo->ReceiverFirmVer[0] = '\0';
	pReceiverInfo->ReceiverSN[0] = '\0';

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Receiver:RefS %u SetupID:%3u, |A %u %s SN %u %s",
		1008, pReceiverInfo->StationID, pReceiverInfo->AntennaID, AntennaDescriptorCntr, pReceiverInfo->AntennaDescriptor,
		AntennaSNCntr, pReceiverInfo->AntennaSN);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1033

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1033(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int StationID, AntennaDescriptorCntr, AntennaSNCntr, ReceiverDescriptorCntr, ReceiverFirmVerCntr, ReceiverSNCntr, AntennaID;
	char AntennaDescriptor[RTCM_ANTENNA_DESCRIPTOR_LEN_MAX] = "", AntennaSN[RTCM_ANTENNA_SN_LEN_MAX] = "";
	char ReceiverDescriptor[RTCM_RECEIVER_DESCRIPTOR_LEN_MAX] = "", ReceiverFirmVer[RTCM_RECEIVER_FIRMWARE_VER_LEN_MAX] = "";
	char ReceiverSN[RTCM_RECEIVER_SN_LEN_MAX] = "";
	unsigned int TotalBitsLen;
	RTCM_RECEIVER_DESCRIPTOR_INFO_T* pReceiverInfo;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pReceiverInfo = (RTCM_RECEIVER_DESCRIPTOR_INFO_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	AntennaDescriptorCntr = GetUnsignedBits(pBufIn, Index + 12, 8);
	if (AntennaDescriptorCntr >= RTCM_ANTENNA_DESCRIPTOR_LEN_MAX)
	{
		return -1;
	}

	AntennaSNCntr = GetUnsignedBits(pBufIn, Index + 28 + 8 * AntennaDescriptorCntr, 8);
	if (AntennaSNCntr >= RTCM_ANTENNA_SN_LEN_MAX)
	{
		return -1;
	}

	ReceiverDescriptorCntr = GetUnsignedBits(pBufIn, Index + 36 + 8 * (AntennaDescriptorCntr + AntennaSNCntr), 8);
	if (ReceiverDescriptorCntr >= RTCM_RECEIVER_DESCRIPTOR_LEN_MAX)
	{
		return -1;
	}

	ReceiverFirmVerCntr = GetUnsignedBits(pBufIn, Index + 44 + 8 * (AntennaDescriptorCntr + AntennaSNCntr + ReceiverDescriptorCntr), 8);
	if (ReceiverFirmVerCntr >= RTCM_RECEIVER_FIRMWARE_VER_LEN_MAX)
	{
		return -1;
	}

	ReceiverSNCntr = GetUnsignedBits(pBufIn, Index + 52 + 8 * (AntennaDescriptorCntr + AntennaSNCntr + ReceiverDescriptorCntr + ReceiverFirmVerCntr), 8);
	if (ReceiverSNCntr >= RTCM_RECEIVER_SN_LEN_MAX)
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1033B_LEN + RTCM3_TYPE1033C_LEN * (AntennaDescriptorCntr + AntennaSNCntr + ReceiverDescriptorCntr + ReceiverFirmVerCntr + ReceiverSNCntr) <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn, Index, 12);                                 Index += 12 + 8;

		MEMCPY(AntennaDescriptor, pBufIn + (Index >> 3), AntennaDescriptorCntr);        Index += AntennaDescriptorCntr << 3;

		AntennaID = GetUnsignedBits(pBufIn, Index, 8);                                  Index +=  8 + 8;

		MEMCPY(AntennaSN, pBufIn + (Index >> 3), AntennaSNCntr);                        Index += (AntennaSNCntr << 3) + 8;

		MEMCPY(ReceiverDescriptor, pBufIn + (Index >> 3), ReceiverDescriptorCntr);      Index += (ReceiverDescriptorCntr << 3) + 8;

		MEMCPY(ReceiverFirmVer, pBufIn + (Index >> 3), ReceiverFirmVerCntr);            Index += (ReceiverFirmVerCntr << 3) + 8;

		MEMCPY(ReceiverSN, pBufIn + (Index >> 3), ReceiverSNCntr);
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	strncpy(pReceiverInfo->AntennaDescriptor, AntennaDescriptor, AntennaDescriptorCntr);
	pReceiverInfo->AntennaDescriptor[AntennaDescriptorCntr] = '\0';
	pReceiverInfo->AntennaID = AntennaID;
	strncpy(pReceiverInfo->AntennaSN, AntennaSN, AntennaSNCntr);
	pReceiverInfo->AntennaSN[AntennaSNCntr] = '\0';
	strncpy(pReceiverInfo->ReceiverDescriptor, ReceiverDescriptor, ReceiverDescriptorCntr);
	pReceiverInfo->ReceiverDescriptor[ReceiverDescriptorCntr] = '\0';
	strncpy(pReceiverInfo->ReceiverFirmVer, ReceiverFirmVer, ReceiverFirmVerCntr);
	pReceiverInfo->ReceiverFirmVer[ReceiverFirmVerCntr] = '\0';
	strncpy(pReceiverInfo->ReceiverSN, ReceiverSN, ReceiverSNCntr);
	pReceiverInfo->ReceiverSN[ReceiverSNCntr] = '\0';

	pRTCMInterimData->RefVendorInfo.RefVendorType = GetRefstationVendorType(ReceiverDescriptor);

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Receiver:RefS %u SetupID:%3u, |A %u %s SN %u %s | R Type %d %s FW %d %s SN %d %s",
		1033, pReceiverInfo->StationID, pReceiverInfo->AntennaID, AntennaDescriptorCntr, pReceiverInfo->AntennaDescriptor,
		AntennaSNCntr, pReceiverInfo->AntennaSN, ReceiverDescriptorCntr, pReceiverInfo->ReceiverDescriptor,
		ReceiverFirmVerCntr, pReceiverInfo->ReceiverFirmVer, ReceiverSNCntr, pReceiverInfo->ReceiverSN);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1019

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1019(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pGPSEphData;
	int SatID, Week, ExSystemID = GPS;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGPSEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1019_LEN <= TotalBitsLen)
	{
		SatID = GetUnsignedBits(pBufIn,                         Index,  6);                 Index +=  6;
		Week = GetUnsignedBits(pBufIn,                          Index, 10);                 Index += 10;
		pGPSEphData->URA = GetUnsignedBits(pBufIn,              Index,  4);                 Index +=  4 + 2;
		pGPSEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,     Index, 14), 43) * PI;   Index += 14;
		pGPSEphData->IODE1 = GetUnsignedBits(pBufIn,                Index,  8);             Index +=  8;
		pGPSEphData->TOC = GetUnsignedBits(pBufIn,                  Index, 16)*16;          Index += 16;
		pGPSEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,      Index,  8), 55);        Index +=  8;
		pGPSEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 43);        Index += 16;
		pGPSEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,      Index, 22), 31);        Index += 22;
		pGPSEphData->IODC = GetUnsignedBits(pBufIn,                 Index, 10);             Index += 10;
		pGPSEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 5);         Index += 16;
		pGPSEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 43) * PI;   Index += 16;
		pGPSEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn,    Index, 32), 33);        Index += 32;
		pGPSEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pGPSEphData->TOE = GetUnsignedBits(pBufIn,                  Index, 16)*16;          Index += 16;
		pGPSEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 5);         Index += 16;
		pGPSEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 24), 43) * PI;   Index += 24;
		pGPSEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn,   Index, 8), 31);         Index +=  8;
		pGPSEphData->Health = GetUnsignedBits(pBufIn,               Index, 6);              Index +=  6 + 1;
		pGPSEphData->FitInterval = GetUnsignedBits(pBufIn,          Index, 1) ? 0 : 4; /**, 0:4hr,1:>4hr */
	}
	else
	{
		return -1;
	}

	if (SatID >= 40)
	{
		ExSystemID = SBA;
		SatID += 80;
	}

	pGPSEphData->SatID = SatID;
	pGPSEphData->SystemID= ExSystemID;
	pGPSEphData->Week = AdjGPSWeek(Week);
	if (CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pGPSEphData))
	{
		return 0;
	}
	pGPSEphData->EphState = NAV_DATA_STATE_VALID;
	pGPSEphData->EphDataType = EPH_DATA_TYPE_GPS;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Ephm:Sat:%2d", 1019, SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1020

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1020(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	GLO_EPHEMERIS_T* pGloEphData;
	int Health;
	int Ln3, P4, P3, P2, P1, P, M, Ln5;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGloEphData = (GLO_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1020_LEN <= TotalBitsLen)
	{
		pGloEphData->SatID = GetUnsignedBits(pBufIn,                        Index, 6);          Index +=  6;
		pGloEphData->Freq = GetUnsignedBits(pBufIn,                         Index, 5) - 7;      Index +=  5 + 2;
		P1 = GetUnsignedBits(pBufIn,                                        Index, 2);          Index +=  2;
		pGloEphData->Tk =GetUnsignedBits(pBufIn,                            Index, 12);             Index += 12;
		Health = GetUnsignedBits(pBufIn,                                    Index, 1);              Index +=  1;
		P2 = GetUnsignedBits(pBufIn,                                        Index,  1);             Index +=  1;
		pGloEphData->Tb = GetUnsignedBits(pBufIn,                           Index,  7) * 900;       Index +=  7;
		pGloEphData->VelXYZ[0] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 24), 20) * 1E3;  Index += 24;
		pGloEphData->PosXYZ[0] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 27), 11) * 1E3;  Index += 27;
		pGloEphData->AccXYZ[0] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index,  5), 30) * 1E3;  Index +=  5;
		pGloEphData->VelXYZ[1] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 24), 20) * 1E3;  Index += 24;
		pGloEphData->PosXYZ[1] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 27), 11) * 1E3;  Index += 27;
		pGloEphData->AccXYZ[1] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index,  5), 30) * 1E3;  Index +=  5;
		pGloEphData->VelXYZ[2] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 24), 20) * 1E3;  Index += 24;
		pGloEphData->PosXYZ[2] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index, 27), 11) * 1E3;  Index += 27;
		pGloEphData->AccXYZ[2] = DoubleIScale2(GetSignedMagBits(pBufIn,     Index,  5), 30) * 1E3;  Index +=  5;
		P3 = GetUnsignedBits(pBufIn,                                        Index,  1);             Index +=  1;
		pGloEphData->GammaN = DoubleIScale2(GetSignedMagBits(pBufIn,        Index, 11), 40);        Index += 11;
		P = GetUnsignedBits(pBufIn,                                         Index,  2);             Index +=  2;
		Ln3 = GetUnsignedBits(pBufIn,                                       Index,  1);             Index +=  1;
		pGloEphData->TauN = DoubleIScale2(GetSignedMagBits(pBufIn,          Index, 22), 30);        Index += 22;
		pGloEphData->DeltaTauN = DoubleIScale2(GetSignedBits(pBufIn,        Index,  5), 30);        Index +=  5;
		pGloEphData->En = GetUnsignedBits(pBufIn,                           Index,  5);             Index +=  5;
		P4 = GetUnsignedBits(pBufIn,                                        Index,  1);             Index +=  1;
		pGloEphData->Ft = GetUnsignedBits(pBufIn,                           Index,  4);             Index +=  4;
		pGloEphData->Nt = GetUnsignedBits(pBufIn,                           Index, 11);             Index += 11;
		M = GetUnsignedBits(pBufIn,                                         Index,  2);             Index +=  2 + 1;
		pGloEphData->Na = GetUnsignedBits(pBufIn,                           Index, 11);             Index += 11;
		pGloEphData->TaoC = DoubleIScale2(GetSignedBits(pBufIn,             Index, 32), 31);        Index += 32;
		pGloEphData->LeapYear = GetUnsignedBits(pBufIn,                     Index,  5);             Index +=  5;
		pGloEphData->TaoGPS = DoubleIScale2(GetSignedBits(pBufIn,           Index, 22), 30);        Index += 22;
		Ln5 = GetUnsignedBits(pBufIn,                                       Index,  1);             Index +=  1 + 7;
	}
	else
	{
		return -1;
	}

	pGloEphData->Bn = Health << 2;
	pGloEphData->StatusFlag = (Ln5 << 10) + (M << 8) + (P << 6) + (Ln3 << 5) + (P4 << 4) + (P3 << 3) + (P2 << 2) + P1;

#if CONFIG_RTCMDECODETEST_ENABLE
	double Tow, Tod, Toe;
	int Week, WeekMsCount, GPSLeapSecMs;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	pRTCMInterimData = &pParseInfo->RTCMInterimData;

#ifdef MSG_CONVERT
	if (FALSE == CHECK_UTC_YEAR(pRTCMInterimData->RTCMTime.Year))
	{
		pRTCMInterimData->RTCMTime = RTCMGetUTCTime();
	}
#else
	pRTCMInterimData->RTCMTime = RTCMGetUTCTime();
#endif

	GPSLeapSecMs = GetGPSLeapSecondMs(&pRTCMInterimData->RTCMTime);
	UTCToGPSTime(&pRTCMInterimData->RTCMTime, &GPSLeapSecMs, &Week, &WeekMsCount);
	Tow = (double)WeekMsCount/1000;
	Tod = fmod(Tow, 86400.0);
	Tow -= Tod;
	Toe = pGloEphData->Tb - 10800.0; /**< lt->utc */
	if (Toe < Tod - 43200.0)
	{
		Toe += 86400.0;
	}
	else if (Toe > Tod + 43200.0)
	{
		Toe -= 86400.0;
	}
	pGloEphData->TOE = GPSTime2Epoch(Week, (int)(Tow + Toe));
#endif

	pGloEphData->EphState = NAV_DATA_STATE_VALID;
	pGloEphData->EphDataType = EPH_DATA_TYPE_GLO;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GLO Ephm:Sat:%2u", 1020, pGloEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1041

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1041(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pNAVICEphData;
	int Week;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int L5Health, SHealth;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pNAVICEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1041_LEN <= TotalBitsLen)
	{
		pNAVICEphData->SatID = GetUnsignedBits(pBufIn,                                 Index,  6);         Index +=  6;
		Week = GetUnsignedBits(pBufIn,                                  Index, 10);         Index += 10;
		pNAVICEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,        Index, 22), 31);    Index += 22;
		pNAVICEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,        Index, 16), 43);    Index += 16;
		pNAVICEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,        Index,  8), 55);    Index +=  8;
		pNAVICEphData->URA = GetUnsignedBits(pBufIn,                    Index,  4);         Index +=  4;
		pNAVICEphData->TOC = GetUnsignedBits(pBufIn,                    Index, 16) * 16;    Index += 16;
		pNAVICEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn,     Index,  8), 31);    Index +=  8;
		pNAVICEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn,     Index, 22), 43) * PI; Index += 22;
		pNAVICEphData->IODE1 = GetUnsignedBits(pBufIn,                  Index,  8);         Index +=  8;
		pNAVICEphData->IODE2 = GetUnsignedBits(pBufIn,                  Index, 10);         Index += 10;
		L5Health = GetUnsignedBits(pBufIn,                              Index, 1);          Index +=  1;
		SHealth = GetUnsignedBits(pBufIn,                               Index, 1);          Index +=  1;
		pNAVICEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15), 29);    Index += 15;
		pNAVICEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15), 29);    Index += 15;
		pNAVICEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15), 29);    Index += 15;
		pNAVICEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15), 29);    Index += 15;
		pNAVICEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15),  5);    Index += 15;
		pNAVICEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,        Index, 15),  5);    Index += 15;
		pNAVICEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,       Index, 14), 43) * PI; Index += 14;
		pNAVICEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,         Index, 32), 31) * PI; Index += 32;
		pNAVICEphData->TOE = GetUnsignedBits(pBufIn,                    Index, 16)*16;      Index += 16;
		pNAVICEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn,      Index, 32), 33);    Index += 32;
		pNAVICEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pNAVICEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn,     Index, 32), 31) * PI; Index += 32;
		pNAVICEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn,      Index, 32), 31) * PI; Index += 32;
		pNAVICEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn,   Index, 22), 43) * PI; Index += 22;
		pNAVICEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,         Index, 32), 31) * PI; Index += 32;
		Index += 2 + 2;
	}
	else
	{
		return -1;
	}

	pNAVICEphData->Health = ((SHealth & 0x01) << 1) + (L5Health & 0x01);
	pNAVICEphData->SystemID= NIC;
	pNAVICEphData->Week = AdjGPSWeek(Week);
	if (CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pNAVICEphData))
	{
		return 0;
	}
	pNAVICEphData->EphState = NAV_DATA_STATE_VALID;
	pNAVICEphData->EphDataType = EPH_DATA_TYPE_NIC;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u NIC Ephm:Sat:%2u", 1041, pNAVICEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1042

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1042(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pBDEphData;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pBDEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1042_LEN <= TotalBitsLen)
	{
		pBDEphData->SatID = GetUnsignedBits(pBufIn,             Index,  6);         Index +=  6;
		pBDEphData->Week = GetUnsignedBits(pBufIn,              Index, 13);         Index += 13;
		pBDEphData->URA = GetUnsignedBits(pBufIn,               Index,  4);         Index +=  4;
		pBDEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,  Index, 14), 43) * PI; Index += 14;
		pBDEphData->IODE1 = GetUnsignedBits(pBufIn,             Index,  5);         Index +=  5;
		pBDEphData->TOC = GetUnsignedBits(pBufIn,               Index, 17)*8;       Index += 17;
		pBDEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 11), 66);    Index += 11;
		pBDEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 22), 50);    Index += 22;
		pBDEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 24), 33);    Index += 24;
		pBDEphData->IODC = GetUnsignedBits(pBufIn,              Index,  5);         Index +=  5;
		pBDEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 6);     Index += 18;
		pBDEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn, Index, 16), 43) * PI; Index += 16;
		pBDEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI; Index += 32;
		pBDEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 31);    Index += 18;
		pBDEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 33);    Index += 32;
		pBDEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 31);    Index += 18;
		pBDEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pBDEphData->TOE = GetUnsignedBits(pBufIn,               Index, 17)*8;       Index += 17;
		pBDEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 31);    Index += 18;
		pBDEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI; Index += 32;
		pBDEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 31);    Index += 18;
		pBDEphData->I0  = DoubleIScale2(GetSignedBits(pBufIn,   Index, 32), 31) * PI; Index += 32;
		pBDEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,   Index, 18), 6);     Index += 18;
		pBDEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI;   Index += 32;
		pBDEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 24), 43) * PI; Index += 24;
		pBDEphData->Tgd[0] = GetSignedBits(pBufIn,              Index, 10)*0.1*1E-9; Index += 10;
		pBDEphData->Tgd[1] = GetSignedBits(pBufIn,              Index, 10)*0.1*1E-9; Index += 10;
		pBDEphData->Health = GetUnsignedBits(pBufIn,            Index, 1);          Index +=  1;
	}
	else
	{
		return -1;
	}

	pBDEphData->SystemID = BDS;
	if (IS_BDS_GEO_PRN(pBDEphData->SatID))
	{
		CalcEphInterimParamGEO(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pBDEphData);
	}
	else
	{
		CalcEphInterimParam(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pBDEphData);
	}
	pBDEphData->EphState = NAV_DATA_STATE_VALID;
	pBDEphData->EphDataType = EPH_DATA_TYPE_BDS_BD2;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u BDS Ephm:Sat:%2u", 1042, pBDEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1044

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1044(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pQzsEphData;
	int Week;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pQzsEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1044_LEN <= TotalBitsLen)
	{
		pQzsEphData->SatID = GetUnsignedBits(pBufIn,            Index,  4);             Index +=  4;
		pQzsEphData->TOC = GetUnsignedBits(pBufIn,              Index, 16) * 16;        Index += 16;
		pQzsEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,  Index,  8), 55);        Index +=  8;
		pQzsEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 43);        Index += 16;
		pQzsEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,  Index, 22), 31);        Index += 22;
		pQzsEphData->IODE1 = GetUnsignedBits(pBufIn,            Index,  8);             Index +=  8;
		pQzsEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 5);         Index += 16;
		pQzsEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn, Index, 16), 43) * PI; Index += 16;
		pQzsEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 32), 31) * PI;   Index += 32;
		pQzsEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 29);        Index += 16;
		pQzsEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 33);       Index += 32;
		pQzsEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 29);        Index += 16;
		pQzsEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pQzsEphData->TOE = GetUnsignedBits(pBufIn,              Index, 16) * 16;        Index += 16;
		pQzsEphData->Cic =DoubleIScale2( GetSignedBits(pBufIn,  Index, 16), 29);        Index += 16;
		pQzsEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI; Index += 32;
		pQzsEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 29);        Index += 16;
		pQzsEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 32), 31) * PI;   Index += 32;
		pQzsEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,  Index, 16), 5);         Index += 16;
		pQzsEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI;  Index += 32;
		pQzsEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 24), 43) * PI; Index += 24;
		pQzsEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 14), 43) * PI;   Index += 14 + 2;
		Week = GetUnsignedBits(pBufIn,                          Index, 10);             Index += 10;
		pQzsEphData->URA = GetUnsignedBits(pBufIn,              Index,  4);             Index +=  4;
		pQzsEphData->Health = GetUnsignedBits(pBufIn,           Index,  6);             Index +=  6;
		pQzsEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn, Index,  8), 31);      Index +=  8;
		pQzsEphData->IODC = GetUnsignedBits(pBufIn,             Index, 10);             Index += 10;
		pQzsEphData->FitInterval = GetUnsignedBits(pBufIn,      Index,  1) ? 0 : 2; /**< 0:2hr,1:>2hr */
	}
	else
	{
		return -1;
	}

	pQzsEphData->SystemID = QZS;
	pQzsEphData->Week = AdjGPSWeek(Week);
	if (CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pQzsEphData))
	{
		return 0;
	}
	pQzsEphData->EphState = NAV_DATA_STATE_VALID;
	pQzsEphData->EphDataType = EPH_DATA_TYPE_QZS;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u QZS Ephm:Sat:%2u", 1044, pQzsEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1045

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1045(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pGalNavEphData;
	int E5aHealth, E5aDataValid;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGalNavEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1045_LEN <= TotalBitsLen)
	{
		pGalNavEphData->SatID = GetUnsignedBits(pBufIn,                 Index,  6);         Index +=  6;
		pGalNavEphData->Week = GetUnsignedBits(pBufIn,                  Index, 12);         Index += 12;
		pGalNavEphData->IODE1 = GetUnsignedBits(pBufIn,                 Index, 10);         Index += 10;
		pGalNavEphData->URA = GetUnsignedBits(pBufIn,                   Index,  8);         Index +=  8;
		pGalNavEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,      Index, 14), 43) * PI; Index += 14;
		pGalNavEphData->TOC = GetUnsignedBits(pBufIn,                   Index, 14) * 60;    Index += 14;
		pGalNavEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,       Index,  6), 59);    Index +=  6;
		pGalNavEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 21) * 1, 46); Index += 21;
		pGalNavEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 31), 34);    Index += 31;
		pGalNavEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 5);     Index += 16;
		pGalNavEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn,    Index, 16), 43) * PI; Index += 16;
		pGalNavEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,        Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 29);    Index += 16;
		pGalNavEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn,     Index, 32), 33);    Index += 32;
		pGalNavEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 29);    Index += 16;
		pGalNavEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pGalNavEphData->TOE = GetUnsignedBits(pBufIn,                   Index, 14) * 60;    Index += 14;
		pGalNavEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 29);    Index += 16;
		pGalNavEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 29);    Index += 16;
		pGalNavEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,        Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,       Index, 16), 5);     Index += 16;
		pGalNavEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn,     Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn,  Index, 24), 43) * PI; Index += 24;
		pGalNavEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn,    Index, 10), 32);    Index += 10;
		E5aHealth = GetUnsignedBits(pBufIn,                             Index,  2);         Index +=  2;
		E5aDataValid = GetUnsignedBits(pBufIn,                          Index,  1);         Index +=  1 + 3;
	}
	else
	{
		return -1;
	}

	pGalNavEphData->SystemID = GAL;
	pGalNavEphData->Health = E5aHealth << 2;
	pGalNavEphData->DataValidStatus = E5aDataValid << 2;
	pGalNavEphData->Tgd[1] = 0;
	if (CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pGalNavEphData))
	{
		return 0;
	}
	pGalNavEphData->EphState = NAV_DATA_STATE_VALID;
	pGalNavEphData->EphDataType = EPH_DATA_TYPE_GAL_INAV;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GAL Ephm:Sat:%2u FNav", 1045, pGalNavEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1046

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1046(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	UNI_EPHEMERIS_T* pGalNavEphData;
	int E5bHealth, E5bDataValid, E1BHealth, E1BDataValid;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGalNavEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1046_LEN <= TotalBitsLen)
	{
		pGalNavEphData->SatID = GetUnsignedBits(pBufIn,             Index,  6);         Index +=  6;
		pGalNavEphData->Week = GetUnsignedBits(pBufIn,              Index, 12);         Index += 12;
		pGalNavEphData->IODE1 = GetUnsignedBits(pBufIn,             Index, 10);         Index += 10;
		pGalNavEphData->URA = GetUnsignedBits(pBufIn,               Index,  8);         Index +=  8;
		pGalNavEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,  Index, 14), 43) * PI; Index += 14;
		pGalNavEphData->TOC = GetUnsignedBits(pBufIn,               Index, 14) * 60;    Index += 14;
		pGalNavEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,   Index,  6), 59);    Index +=  6;
		pGalNavEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 21), 46);    Index += 21;
		pGalNavEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 31), 34);    Index += 31;
		pGalNavEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 5);     Index += 16;
		pGalNavEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn, Index, 16), 43) * PI; Index += 16;
		pGalNavEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 29);    Index += 16;
		pGalNavEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn,  Index, 32), 33);    Index += 32;
		pGalNavEphData->Cus = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 29);    Index += 16;
		pGalNavEphData->SqrtAorDeltaA = DoubleUScale2(GetUnsignedBits(pBufIn, Index, 32), 19); Index += 32;
		pGalNavEphData->TOE = GetUnsignedBits(pBufIn,               Index, 14) * 60;    Index += 14;
		pGalNavEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 29);    Index += 16;
		pGalNavEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 29);    Index += 16;
		pGalNavEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 5);     Index += 16;
		pGalNavEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn, Index, 32), 31) * PI; Index += 32;
		pGalNavEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 24), 43) * PI; Index += 24;
		pGalNavEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn, Index, 10), 32);   Index += 10;
		pGalNavEphData->Tgd[1] = DoubleIScale2(GetSignedBits(pBufIn, Index, 10), 32);   Index += 10;
		E5bHealth = GetUnsignedBits(pBufIn,                         Index,  2);     Index +=  2;
		E5bDataValid = GetUnsignedBits(pBufIn,                      Index,  1);     Index +=  1;
		E1BHealth = GetUnsignedBits(pBufIn,                         Index,  2);     Index +=  2;
		E1BDataValid = GetUnsignedBits(pBufIn,                      Index,  1);     Index +=  1 + 2;
	}
	else
	{
		return -1;
	}

	pGalNavEphData->SystemID = GAL;
	pGalNavEphData->Health = (E5bHealth << 1) + E1BHealth;
	pGalNavEphData->DataValidStatus = (E5bDataValid << 1) + E1BDataValid;
	if (CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pGalNavEphData))
	{
		return 0;
	}
	pGalNavEphData->EphState = NAV_DATA_STATE_VALID;
	pGalNavEphData->EphDataType = EPH_DATA_TYPE_GAL_FNAV;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GAL Ephm:Sat:%2u INav", 1046, pGalNavEphData->SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1230

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1230(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen, BiasIndex;
	signed int Bias;
	RTCM_GLO_CODE_PHASE_BIASES_DATA_T* pGLOCodePhaseBiases;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGLOCodePhaseBiases = (RTCM_GLO_CODE_PHASE_BIASES_DATA_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + 20 >= TotalBitsLen)
	{
		return -1;
	}

	MEMSET(pGLOCodePhaseBiases, 0, sizeof(RTCM_GLO_CODE_PHASE_BIASES_DATA_T));

	pGLOCodePhaseBiases->StationID = GetUnsignedBits(pBufIn,        Index, 12);     Index += 12;
	pGLOCodePhaseBiases->CodeInd = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1 + 3;
	pGLOCodePhaseBiases->SignMask = GetUnsignedBits(pBufIn,         Index,  4);     Index +=  4;

	if (CheckStationID(pRTCMInterimData, pGLOCodePhaseBiases->StationID))
	{
		return -1;
	}

	for (BiasIndex = 0; (BiasIndex < 4) && (Index + 16 <= TotalBitsLen); BiasIndex++)
	{
		if (!(pGLOCodePhaseBiases->SignMask & (1 << (3 - BiasIndex))))
		{
			continue;
		}

		Bias = GetSignedBits(pBufIn,              Index, 16);     Index += 16;

		if (Bias != -32768) /**< -32768: CodePhaseBias is valid */
		{
			pGLOCodePhaseBiases->CodePhaseBias[BiasIndex] = Bias * 0.02;
		}
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Glo Bias:BiasF %u Mask 0X%02X: %.2f, %.2f, %.2f, %.2f",
		1230, pGLOCodePhaseBiases->CodeInd, pGLOCodePhaseBiases->SignMask,
		pGLOCodePhaseBiases->CodePhaseBias[0], pGLOCodePhaseBiases->CodePhaseBias[1],
		pGLOCodePhaseBiases->CodePhaseBias[2], pGLOCodePhaseBiases->CodePhaseBias[3]);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1300

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1300(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned char CRSNameCount;
	unsigned int TotalBitsLen;
	unsigned int MsgIndex;
	RTCM_SERVICE_CRS_MSG_T* pServiceCRSMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pServiceCRSMsg = (RTCM_SERVICE_CRS_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	CRSNameCount = GetUnsignedBits(pBufIn,									Index,  5);				Index +=  5;
	if (CRSNameCount >= RTCM_CRS_NAME_LEN_MAX)
	{
		return -1;
	}

	if ((Index + RTCM3_TYPE1300B_LEN + RTCM3_TYPE1300C_LEN * CRSNameCount) <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < CRSNameCount; MsgIndex++)
		{
			pServiceCRSMsg->CRSName[MsgIndex] = GetUnsignedBits(pBufIn,		Index,  8);				Index +=  8;
		}
		pServiceCRSMsg->CRSNameCount = CRSNameCount;

		pServiceCRSMsg->CoordinateEpoch = GetUnsignedBits(pBufIn,			Index, 16) * 0.01f;		Index += 16;
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %d CRS:Name %u %s, Epoch %.2f",
		1300, pServiceCRSMsg->CRSNameCount, pServiceCRSMsg->CRSName, pServiceCRSMsg->CoordinateEpoch);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}


/**********************************************************************//**
@brief  decode RTCM1301

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1301(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int SourceNameCount, TargetNameCount;
	unsigned int TotalBitsLen;
	unsigned int MsgIndex;
	RTCM_HELMERT_TRANSFORMATION_MSG_T* pHelmertTransMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pHelmertTransMsg = (RTCM_HELMERT_TRANSFORMATION_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	SourceNameCount = GetUnsignedBits(pBufIn, 									Index, 5);			Index +=  5;
	if (SourceNameCount >= RTCM_HELMERT_SOURCE_NAME_LEN_MAX)
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1301C_LEN * SourceNameCount <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < SourceNameCount; MsgIndex++)
		{
			pHelmertTransMsg->SourceName[MsgIndex] = GetUnsignedBits(pBufIn,	Index,  8);			Index +=  8;
		}
		pHelmertTransMsg->SourceNameCount = SourceNameCount;
	}

	TargetNameCount = GetUnsignedBits(pBufIn,									Index, 5);			Index +=  5;
	if (TargetNameCount >= RTCM_HELMERT_TARGET_NAME_LEN_MAX)
	{
		return -1;
	}

	if (Index + RTCM3_TYPE1301C_LEN * TargetNameCount <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < TargetNameCount; MsgIndex++)
		{
			pHelmertTransMsg->TargetName[MsgIndex] = GetUnsignedBits(pBufIn,	Index,  8);			Index +=  8;
		}
		pHelmertTransMsg->TargetNameCount = TargetNameCount;
	}

	if (Index + (RTCM3_TYPE1301B_LEN - 5 - 5) <= TotalBitsLen)
	{
		pHelmertTransMsg->SysIdentifcationNumb = GetUnsignedBits(pBufIn,			Index,  8);		Index +=  8;
		pHelmertTransMsg->UtilizedTransMsg = GetSignedBits(pBufIn,					Index, 10);		Index += 10;
		pHelmertTransMsg->EpochT0 = GetUnsignedBits(pBufIn,							Index, 16);		Index += 16;
		pHelmertTransMsg->dX0 = GetSignedBits(pBufIn,								Index, 23);		Index += 23;
		pHelmertTransMsg->dY0 = GetSignedBits(pBufIn,								Index, 23);		Index += 23;
		pHelmertTransMsg->dZ0 = GetSignedBits(pBufIn,								Index, 23);		Index += 23;
		pHelmertTransMsg->R01 = GetSignedBits(pBufIn,								Index, 32);		Index += 32;
		pHelmertTransMsg->R02 = GetSignedBits(pBufIn,								Index, 32);		Index += 32;
		pHelmertTransMsg->R03 = GetSignedBits(pBufIn,								Index, 32);		Index += 32;
		pHelmertTransMsg->dS0 = GetSignedBits(pBufIn,								Index, 25);		Index += 25;
		pHelmertTransMsg->dXDot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->dYDot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->dZDot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->R1Dot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->R2Dot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->R3Dot = GetSignedBits(pBufIn,								Index, 17);		Index += 17;
		pHelmertTransMsg->dSDot = GetSignedBits(pBufIn,								Index, 14);		Index += 14;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %d SourceName %u %s, TargetName %u %s, Numb %u, Epoch %u",
		1301, pHelmertTransMsg->SourceNameCount, pHelmertTransMsg->SourceName, pHelmertTransMsg->TargetNameCount,
		pHelmertTransMsg->TargetName, pHelmertTransMsg->SysIdentifcationNumb, pHelmertTransMsg->EpochT0);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1302

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1302(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int CRSNameCount, LinkCount;
	unsigned int TotalBitsLen;
	unsigned int MsgIndex, LinkIndex;
	RTCM_CRS_MSG_T* pCRSMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pCRSMsg = (RTCM_CRS_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	CRSNameCount = GetUnsignedBits(pBufIn,							Index,  5);				Index +=  5;
	if (CRSNameCount >= RTCM_CRS_NAME_LEN_MAX)
	{
		return -1;
	}

	if ((Index + RTCM3_TYPE1302B_LEN + RTCM3_TYPE1302C_LEN * CRSNameCount) <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < pCRSMsg->CRSNameCount; MsgIndex++)
		{
			pCRSMsg->CRSName[MsgIndex] = GetUnsignedBits(pBufIn,	Index,  8);				Index +=  8;
		}
		pCRSMsg->CRSNameCount = CRSNameCount;

		pCRSMsg->Anchor = GetUnsignedBits(pBufIn,					Index,  1);				Index +=  1;
		pCRSMsg->PlateNumber = GetUnsignedBits(pBufIn,				Index,  5);				Index +=  5;
		pCRSMsg->LinksINumb = GetUnsignedBits(pBufIn,				Index,  3);				Index +=  3;

		for (LinkIndex = 0; LinkIndex < pCRSMsg->LinksINumb; LinkIndex++)
		{
			LinkCount = GetUnsignedBits(pBufIn,						Index,  5);				Index +=  5;
			for (MsgIndex = 0; MsgIndex < LinkCount; MsgIndex++)
			{
				pCRSMsg->DataBaseLink[LinkIndex].DataBaseLink[MsgIndex] = GetUnsignedBits(pBufIn,	Index,  8);	Index +=  8;
			}
			pCRSMsg->DataBaseLink[LinkIndex].LinkCount = LinkCount;
		}
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %d CRS:Name %u %s, LinksI Numb %u",
		1302, pCRSMsg->CRSNameCount, pCRSMsg->CRSName, pCRSMsg->LinksINumb);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1303

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1303(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int MsgIndex;
	RTCM_NETWORK_RTK_RESIDUAL_MSG_T* pRTKResidualMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pRTKResidualMsg = (RTCM_NETWORK_RTK_RESIDUAL_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pRTKResidualMsg->Tow = GetUnsignedBits(pBufIn,							Index, 20);				Index += 20;
	pRTKResidualMsg->StationID = GetUnsignedBits(pBufIn,					Index, 12);				Index += 12;
	pRTKResidualMsg->RefsN = GetUnsignedBits(pBufIn,					Index,  7);				Index +=  7;
	pRTKResidualMsg->ProcessedSatNumb = GetUnsignedBits(pBufIn,					Index,  5);				Index +=  5;

	if ((Index + RTCM3_TYPE1303B_LEN + RTCM3_TYPE1303C_LEN * pRTKResidualMsg->ProcessedSatNumb) <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < pRTKResidualMsg->ProcessedSatNumb; MsgIndex++)
		{
			pRTKResidualMsg->SatSpecificPart[MsgIndex].SatID = GetUnsignedBits(pBufIn,	Index,  6);				Index +=  6;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Soc = GetUnsignedBits(pBufIn,	Index,  8);				Index +=  8;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sod = GetUnsignedBits(pBufIn,	Index,  9);				Index +=  9;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Soh = GetUnsignedBits(pBufIn,	Index,  6);				Index +=  6;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sic = GetUnsignedBits(pBufIn,	Index, 10);				Index += 10;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sid = GetUnsignedBits(pBufIn,	Index, 10);				Index += 10;
		}
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %d Tow %u, Ref %u, RefsN %u",
		1303, pRTKResidualMsg->Tow, pRTKResidualMsg->StationID, pRTKResidualMsg->RefsN);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1304

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1304(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int MsgIndex;
	RTCM_NETWORK_RTK_RESIDUAL_MSG_T* pRTKResidualMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pRTKResidualMsg = (RTCM_NETWORK_RTK_RESIDUAL_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	pRTKResidualMsg->Tow = GetUnsignedBits(pBufIn,							Index, 20);				Index += 20;
	pRTKResidualMsg->StationID = GetUnsignedBits(pBufIn,					Index, 12);				Index += 12;
	pRTKResidualMsg->RefsN = GetUnsignedBits(pBufIn,					Index,  7);				Index +=  7;
	pRTKResidualMsg->ProcessedSatNumb = GetUnsignedBits(pBufIn,					Index,  5);				Index +=  5;

	if ((Index + RTCM3_TYPE1304B_LEN + RTCM3_TYPE1304C_LEN * pRTKResidualMsg->ProcessedSatNumb) <= TotalBitsLen)
	{
		for (MsgIndex = 0; MsgIndex < pRTKResidualMsg->ProcessedSatNumb; MsgIndex++)
		{
			pRTKResidualMsg->SatSpecificPart[MsgIndex].SatID = GetUnsignedBits(pBufIn,	Index,  6);				Index +=  6;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Soc = GetUnsignedBits(pBufIn,	Index,  8);				Index +=  8;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sod = GetUnsignedBits(pBufIn,	Index,  9);				Index +=  9;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Soh = GetUnsignedBits(pBufIn,	Index,  6);				Index +=  6;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sic = GetUnsignedBits(pBufIn,	Index, 10);				Index += 10;
			pRTKResidualMsg->SatSpecificPart[MsgIndex].Sid = GetUnsignedBits(pBufIn,	Index, 10);				Index += 10;
		}
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %d Tow %u, Ref %u, RefsN %u",
		1304, pRTKResidualMsg->Tow, pRTKResidualMsg->StationID, pRTKResidualMsg->RefsN);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

#ifdef MSG_CONVERT

/**********************************************************************//**
@brief  decode RTCM1013

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1013(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	RTCM_SYSTEM_PARAM_T* pSystemParam;
	unsigned int MsgNum, DataIndex;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pSystemParam = (RTCM_SYSTEM_PARAM_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1013_LEN <= TotalBitsLen)
	{
		pSystemParam->StationID=GetUnsignedBits(pBufIn,	Index, 12);	Index+=12;
		pSystemParam->MJD =GetUnsignedBits(pBufIn,		Index, 16);	Index+=16;
		pSystemParam->Tod =GetUnsignedBits(pBufIn,		Index, 17);	Index+=17;
		MsgNum =GetUnsignedBits(pBufIn,					Index,  5);	Index+=  5;
		pSystemParam->LeapSecs =GetUnsignedBits(pBufIn,	Index,  8);	Index+=  8;
	}
	else
	{
		return -1;
	}

	if ((Index + (MsgNum * 29)) > TotalBitsLen)
	{
		return -1;
	}

	for (DataIndex = 0; DataIndex < MsgNum && DataIndex < RTCM_UNICODE_TEXT_STRING_LEN_MAX; DataIndex++)
	{
		pSystemParam->MsgInfo[DataIndex].MsgID = GetUnsignedBits(pBufIn, Index, 12);	Index +=12;
		pSystemParam->MsgInfo[DataIndex].SyncFlag = GetUnsignedBits(pBufIn, Index,  1);	Index +=  1;
		pSystemParam->MsgInfo[DataIndex].TransInterval = GetUnsignedBits(pBufIn, Index, 16);	Index +=16;
	}

	pSystemParam->MsgNum = MsgNum;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u StationID:%u MJD:%u Tod:%u MsgNum:%u", 1029,
								pSystemParam->StationID, pSystemParam->MJD, pSystemParam->Tod, MsgNum);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM1029

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeType1029(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	RTCM_UNICODE_TEXT_STRING_T* pUnicodeTestStr;
	unsigned int CharsNum, DataIndex;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pUnicodeTestStr = (RTCM_UNICODE_TEXT_STRING_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM3_TYPE1029_LEN <= TotalBitsLen)
	{
		pUnicodeTestStr->StationID=GetUnsignedBits(pBufIn,	Index, 12);	Index+=12;
		pUnicodeTestStr->MJD =GetUnsignedBits(pBufIn,		Index, 16);	Index+=16;
		pUnicodeTestStr->Tod =GetUnsignedBits(pBufIn,		Index, 17);	Index+=17;
		CharsNum =GetUnsignedBits(pBufIn,					Index,  7);	Index+=  7;
		pUnicodeTestStr->CodeNum=GetUnsignedBits(pBufIn,	Index,  8);	Index+=  8;
	}
	else
	{
		return -1;
	}

	if ((Index + (CharsNum << 3)) > TotalBitsLen)
	{
		return -1;
	}

	for (DataIndex = 0; DataIndex < CharsNum && DataIndex < RTCM_UNICODE_TEXT_STRING_LEN_MAX; DataIndex++)
	{
		pUnicodeTestStr->CodeBuf[DataIndex]=GetUnsignedBits(pBufIn, DataIndex, 8); DataIndex+=8;
	}
	pUnicodeTestStr->CodeBuf[DataIndex]='\0';

	pUnicodeTestStr->CharsNum = CharsNum;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u StationID:%u MJD:%u Tod:%u CharsNum:%u Code:%s", 1029,
								pUnicodeTestStr->StationID, pUnicodeTestStr->MJD, pUnicodeTestStr->Tod, CharsNum, pUnicodeTestStr->CodeBuf);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM debug info

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeDebugInfo(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

#ifdef RTCM_DEBUG_INFO

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	unsigned int TypeID = 0;
	unsigned char* pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;

	TypeID = GetUnsignedBits(pBufIn, 24, 12);

	static char* s_RTCM3MsgNameMap[] =
	{
		NULL, /**< RTCM1001 */
		NULL, /**< RTCM1002 */
		NULL, /**< RTCM1003 */
		NULL, /**< RTCM1004 */
		NULL, /**< RTCM1005 */
		NULL, /**< RTCM1006 */
		NULL, /**< RTCM1007 */
		NULL, /**< RTCM1008 */
		NULL, /**< RTCM1009 */
		NULL, /**< RTCM1010 */
		NULL, /**< RTCM1011 */
		NULL, /**< RTCM1012 */
		NULL, /**< RTCM1013 */
		"Network Auxiliary Station Data Message", /**< RTCM1014 */
		"GPS Ionospheric Correction Differences", /**< RTCM1015 */
		"GPS Geometric Correction Differences", /**< RTCM1016 */
		"GPS Combined Geometric and Ionospheric Correction Differences", /**< RTCM1017 */
		NULL, /**< RTCM1018 */
		NULL, /**< RTCM1019 */
		NULL, /**< RTCM1020 */
		"Helmert / Abridged Molodenski Transformation Parameters", /**< RTCM1021 */
		"Molodenski-Badekas Transformation Parameters", /**< RTCM1022 */
		"Residuals, Ellipsoidal Grid Representation", /**< RTCM1023 */
		"Residuals, Plane Grid Representation", /**< RTCM1024 */
		"Projection Parameters, Projection Types other than Lambert Conic Conformal (2 SP) and Oblique Mercator", /**< RTCM1025 */
		"Projection Parameters, Projection Type LCC2SP (Lambert Conic Conformal (2 SP))", /**< RTCM1026 */
		"Projection Parameters, Projection Type OM (Oblique Mercator)", /**< RTCM1027 */
		NULL, /**< RTCM1028 */
		NULL, /**< RTCM1029 */
		"GPS Network RTK Residual Message", /**< RTCM1030 */
		"GLONASS Network RTK Residual Message", /**< RTCM1031 */
		"Physical Reference Station Position Message", /**< RTCM1032 */
		NULL, /**< RTCM1033 */
		"GPS Network FKP Gradient", /**< RTCM1034 */
		"GLONASS Network FKP Gradient", /**< RTCM1035 */
		NULL,
		"GLONASS Ionospheric Correction Differences", /**< RTCM1037 */
		"GLONASS Geometric Correction Differences", /**< RTCM1038 */
		"GLONASS Combined Geometric and Ionospheric Correction Differences", /**< RTCM1039 */
		NULL,
		NULL, /**< RTCM1041 */
		NULL, /**< RTCM1042 */
		NULL,
		NULL, /**< RTCM1044 */
		NULL, /**< RTCM1045 */
		NULL, /**< RTCM1046 */
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"SSR GPS Orbit Correction", /**< RTCM1057 */
		"SSR GPS Clock Correction", /**< RTCM1058 */
		"SSR GPS Code Bias", /**< RTCM1059 */
		"SSR GPS Combined Orbit and Clock Corrections", /**< RTCM1060 */
		"SSR GPS URA", /**< RTCM1061 */
		"SSR GPS High Rate Clock Correction", /**< RTCM1062 */
		"SSR GLONASS Orbit Correction", /**< RTCM1063 */
		"SSR GLONASS Clock Correction", /**< RTCM1064 */
		"SSR GLONASS Code Bias", /**< RTCM1065, */
		"SSR GLONASS Combined Orbit and Clock Corrections", /**< RTCM1066 */
		"SSR GLONASS URA", /**< RTCM1067 */
		"SSR GLONASS High Rate Clock Correction", /**< RTCM1068 */
	};

	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	if (NULL != s_RTCM3MsgNameMap[TypeID - 1001])
	{
		unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Size:%4u %s", TypeID,
								pParseInfo->DecodeBuf.Len, s_RTCM3MsgNameMap[TypeID - 1001]);
		pDebugInfoBuf[MsgLen] = '\0';
		pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
	}
	else
	{
		pParseInfo->ParsedDebugInfoBuf.Len = 0;
	}

	return 1;
#else
	return 0;
#endif
}

#endif /**< MSG_CONVERT */

