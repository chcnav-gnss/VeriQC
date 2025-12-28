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

#include "Common/GNSSNavDataType.h"
#include "Common/NavDataCommonFuncs.h"


#ifdef WIN32
#include <windows.h>
#endif

#define RTCM3_MSMH_LEN       157
#define RTCM3_MSM1S_LEN      10
#define RTCM3_MSM1C_LEN      15
#define RTCM3_MSM2S_LEN      10
#define RTCM3_MSM2C_LEN      27
#define RTCM3_MSM3S_LEN      10
#define RTCM3_MSM3C_LEN      42
#define RTCM3_MSM4S_LEN      18
#define RTCM3_MSM4C_LEN      48
#define RTCM3_MSM5S_LEN      36
#define RTCM3_MSM5C_LEN      63
#define RTCM3_MSM6S_LEN      18
#define RTCM3_MSM6C_LEN      65
#define RTCM3_MSM7S_LEN      36
#define RTCM3_MSM7C_LEN      80

#define BDS_SIGNAL_EXPAND_MASK_B1C			(0X01)
#define BDS_SIGNAL_EXPAND_MASK_B2A			(0X02)
#define BDS_SIGNAL_EXPAND_MASK				(BDS_SIGNAL_EXPAND_MASK_B1C | BDS_SIGNAL_EXPAND_MASK_B2A)

#define BDS_SIGNAL_STANDARD_MASK_B1C		(0X04)
#define BDS_SIGNAL_STANDARD_MASK_B2A		(0X08)
#define BDS_SIGNAL_STANDARD_MASK			(BDS_SIGNAL_STANDARD_MASK_B1C | BDS_SIGNAL_STANDARD_MASK_B2A)

#define GET_MSM_MASK_FORNT(MSMMsgMask, MSMIndex)                        ((((MSMMsgMask) & (~(1 << (MSMIndex)))) >> ((MSMIndex) + 1)) ? TRUE : FALSE)
#define GET_MSM_MASK_OTHER(MSMMsgMask, MSMIndex)                        (((MSMMsgMask) & (~(1 << (MSMIndex)))) ? TRUE : FALSE)
#define GET_MSM_MASK(MSMMsgMask, MSMIndex)                              (((MSMMsgMask) & (1 << (MSMIndex))) ? TRUE : FALSE)
#define SET_MSM_MASK(MSMMsgMask, MSMIndex)                              ((MSMMsgMask) |= (1 << (MSMIndex)))

#define CHECK_UTC_YEAR(Year)			((((Year) < 2020) || ((Year) > 2099)) ? FALSE : TRUE)

#define RANGE_MS_CHECK_VALUE			(2.9e5)

/*****************************************************************************//**
@brief adjust weekly rollover of GPS time

@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param Tow                  [In] Tow, GPS Second, [0,604799]

@return <0:error,=0:success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int AdjWeek(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, double Tow)
{
	double TowP;
	int Week = 0;
	int WeekMsCount, GPSLeapSecMs;

	if ((!pObsData) || (!pRTCMInterimData))
	{
		return -1;
	}

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

	if ((Week <= 0) || (WeekMsCount < 0))
	{
		return -1;
	}

	TowP = (double)WeekMsCount/1000;

	if (Tow < (TowP - 302400.0))
	{
		Tow += 604800.0;
	}
	else if (Tow > (TowP + 302400.0))
	{
		Tow -= 604800.0;
	}

	WeekMsCount = (int)(Tow*1000 + 0.5);

	GPSTimeToUTC(Week, WeekMsCount, &GPSLeapSecMs, &pRTCMInterimData->RTCMTime);

	return 0;
}

/**********************************************************************//**
@brief  adjust GPS week number using cpu time

@param Week             [In] not-adjusted GPS week number

@retval adjusted GPS week number

@author CHC
@date 2023/04/10
@note
**************************************************************************/
int AdjGPSWeek(int Week)
{
	int TempWeek, WeekMsCount, GPSLeapSecMs;
	UTC_TIME_T TempUtcTime = RTCMGetUTCTime();

	GPSLeapSecMs = GetGPSLeapSecondMs(&TempUtcTime);
	UTCToGPSTime(&TempUtcTime, &GPSLeapSecMs, &TempWeek, &WeekMsCount);

	if (TempWeek < 1560)
	{
		TempWeek = 1560; /**< use 2009/12/1 if time is earlier than 2009/12/1 */
	}

	return Week + (TempWeek - Week + 512) / 1024 * 1024;
}

/*****************************************************************************//**
@brief adjust daily rollover of glonass time

@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param Tod                  [In] Tod
@param Dow                  [In] Dow

@return <0:error,=0:success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int AdjGLOTimeDaily(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData, double Tod, double Dow)
{
	UTC_TIME_T Time;
	double TowP, TodP;
	double Tow;
	int Week;
	int TempLeapSecMs, WeekMsCount;

	if ((!pObsData) || (!pRTCMInterimData))
	{
		return -1;
	}

#ifdef MSG_CONVERT
	if (FALSE == CHECK_UTC_YEAR(pRTCMInterimData->RTCMTime.Year))
	{
		pRTCMInterimData->RTCMTime = RTCMGetUTCTime();
	}
#else
	pRTCMInterimData->RTCMTime = RTCMGetUTCTime();
#endif

	Time = UTCAdd(&pRTCMInterimData->RTCMTime, 10800000); /**<  glonass Time in UTC format */

	TempLeapSecMs = 0; /**< GLO Time 2 GLO Time, Set LeapSec = 0 */

	UTCToGPSTime(&Time, &TempLeapSecMs, &Week, &WeekMsCount); /**< GLO Time in UTC format to Week+MsCount format */

	if ((Week <= 0) || (WeekMsCount < 0))
	{
		return -1;
	}

	TowP = (double)WeekMsCount/1000; /**< Pre second of Week */
	TodP = fmod(TowP, 86400.0);      /**< Pre second of day */

	if (Dow < 0)
	{
		Dow = (int)(TowP/86400.0);  /**< Pre day of Week */

		if (Tod < (TodP - 43200.0))
		{
			Tod += 86400.0;
		}
		else if (Tod > (TodP + 43200.0))
		{
			Tod -= 86400.0;
		}

		GPSTimeToUTC(Week, (int)((Dow*86400 + Tod)*1000 + 0.5), &TempLeapSecMs, &Time); /**< GLO Time in Week+Sec format --> UTC format */
	}
	else
	{
		Tow = Tod + Dow * 86400;
		if (Tow < (TowP - 302400.0))
		{
			Tow += 604800.0;
		}
		else if (Tow > (TowP + 302400.0))
		{
			Tow -= 604800.0;
		}

		GPSTimeToUTC(Week, (int)(Tow*1000 + 0.5), &TempLeapSecMs, &Time); /**< GLO Time in Week+Sec format --> UTC format */
	}

	pRTCMInterimData->RTCMTime = UTCAdd(&Time, -10800000); /**< GLO Time --> UTC Time */

	return 0;
}

/*****************************************************************************//**
@brief loss-of-lock indicator

@param pLockTime            [In] RTCM decode lock time
@param LockTime             [In] lock time(unit: ms)

@return Phase lock flag, 0 = not locked, 1 = locked

@author CHC
@date  2023/06/25
@note
*********************************************************************************/
int LossOfLock(unsigned int* pLockTime, unsigned int LockTime)
{
	unsigned int TempLockTime = *pLockTime;
	int LockFlag = !((!LockTime && !TempLockTime) || (LockTime < TempLockTime));

	*pLockTime = LockTime;

	return LockFlag;
}

/*****************************************************************************//**
@brief check station ID consistency

@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param StationID            [In] satellite number

@return <0:error,=0:success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int CheckStationID(RTCM_INTERIM_DATA_T* pRTCMInterimData, unsigned int StationID)
{
	/** save station id */
	if (pRTCMInterimData->StationID==0||pRTCMInterimData->ObsCompleteFlag)
	{
		pRTCMInterimData->StationID = StationID;
	}
	else if (StationID!=pRTCMInterimData->StationID)
	{
		pRTCMInterimData->StationID = 0;

		return -1;
	}

	return 0;
}

/*****************************************************************************//**
@brief msm lock time indicator to obs lock time

@param LockTime         [In] msm lock time indicator

@return obs lock time(ms)

@author CHC
@date  2023/05/18
@note
*********************************************************************************/
int GetMSMLockTimeEx(unsigned int LockTime)
{
	unsigned int Index;

	for (Index = 2; Index < 23; Index++)
	{
		if (LockTime < (Index << 5))
		{
			if (Index != 2)
			{
				return ((LockTime - ((Index-1) << 5)) << (Index - 2)) + (1 << (Index + 3));
			}
			else
			{
				return LockTime;
			}
		}
	}
	return 67108864;
}

/**********************************************************************//**
@brief  RTCM3 decode obs callback

@param pObsData             [In] obs data
@param pRTCMInterimData     [In] rtcm decode interim data
@param pSatObsData          [In] Satellite obs data
@param RawObsDataType       [In] raw obs data type

@retval <0 error, =0 success

@author CHC
@date 2023/11/23
@note
**************************************************************************/
int DecodeObsCallback(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
											OBS_DATA_SAT_T* pSatObsData, unsigned int RawObsDataType, UTC_TIME_T RTCMTime)
{
	unsigned int BDSSignalExpand = pObsData->BDSSignalExpand;
	unsigned int ObsSystemMask = pObsData->ObsSystemMask;
	MEMSET(pObsData, 0x00, sizeof(GNSS_OBS_T));
	pObsData->BDSSignalExpand = BDSSignalExpand;
	pObsData->ObsSystemMask = ObsSystemMask;
	SatObsDataToGNSSObsData(pSatObsData, pObsData);
	pObsData->RawDataType = RawObsDataType;
	pObsData->ObsTime = RTCMTime;
	pObsData->BaseID = pRTCMInterimData->StationID;
	pObsData->VendorTypeID = pRTCMInterimData->RefVendorInfo.RefVendorType;
	UTCTimeToRangeTime(pObsData, &RTCMTime);

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 decode obs complete callback

@param pObsData             [In] obs data
@param pRTCMInterimData     [In] rtcm decode interim data
@param pSatObsData          [In] Satellite obs data
@param RawObsDataType       [In] raw obs data type

@retval <0 error, =0 success

@author CHC
@date 2023/11/23
@note
**************************************************************************/
int DecodeObsCompleteCallback(GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
											OBS_DATA_SAT_T* pSatObsData, unsigned int RawObsDataType)
{
	return DecodeObsCallback(pObsData, pRTCMInterimData, pSatObsData, RawObsDataType, pRTCMInterimData->RTCMTime);
}

/**********************************************************************//**
@brief  RTCM3 decode obs over time callback

@param TypeID				[In] RTCM type id
@param pObsData             [In] obs data
@param pRTCMInterimData     [In] rtcm decode interim data
@param pSatObsData          [In] Satellite obs data
@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function
@param RawObsDataType       [In] raw obs data type

@retval <0 error, =0 success

@author CHC
@date 2023/11/23
@note
**************************************************************************/
int DecodeObsOverTimeCallback(int TypeID, GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
	OBS_DATA_SAT_T* pSatObsData, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback,
	unsigned int RawObsDataType)
{
	DecodeObsCallback(pObsData,   pRTCMInterimData, pSatObsData, RawObsDataType, pRTCMInterimData->LastRTCMTime);

	if ((pOverTimeCallback) && (pParseInfo) && (pSrcObj))
	{
		pOverTimeCallback(TypeID, pParseInfo, pSrcObj);
	}

	return 0;
}

/*****************************************************************************//**
@brief rtcm MSM header Log decoder function

@param RTCMSatSys           [In] RTCM satellite system
@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param pMSMHead             [Out] RTCM MSM header Data after decode
@param pBufIn               [In] RTCM3 log data buffer
@param TotalBitsLen         [In] RTCM3 log data total bits
@param pBitNumOut           [Out] RTCM MSM header Log bit number
@param pNcell               [Out] cell number
@param pTow                 [Out] TOW
@param pSignalMask          [Out] Signal mask

@return <0: error; =0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
static int RTCM3MSMHeadDecoder(int RTCMSatSys, GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
											RTCM_MSM_HEADER_DATA_T* pMSMHead, unsigned char* pBufIn,
											unsigned int TotalBitsLen, unsigned int* pBitNumOut, unsigned int* pNcell,
											unsigned int* pTow, unsigned int* pSignalMask)
{
	unsigned int Index = RTCM3_MSG_HEADER_BITS_SIZE + RTCM3_MSG_NUMBER_BITS_SIZE;
	unsigned int MaskIndex;
	unsigned int Dow, Mask, StationID, Ncell = 0;
	double Tow, Tod;
	unsigned int TempTow;

	if ((!pObsData) || (!pRTCMInterimData) || (!pMSMHead) || (!pBufIn) || (!pBitNumOut) || (!pTow) || (!pSignalMask))
	{
		return -1;
	}

	if (Index + RTCM3_MSMH_LEN <= TotalBitsLen)
	{
		StationID = GetUnsignedBits(pBufIn,         Index, 12);             Index += 12;
		if (RTCMSatSys == RTCM_SYS_GLO)
		{
			Dow = GetUnsignedBits(pBufIn,           Index,  3);             Index += 3;
			TempTow = GetUnsignedBits(pBufIn,       Index, 27);             Index += 27;
			Tod = TempTow * 0.001;
			if (AdjGLOTimeDaily(pObsData, pRTCMInterimData, Tod, Dow) < 0)
			{
				return -1;
			}
		}
		else if ((RTCMSatSys == RTCM_SYS_GPS) || (RTCMSatSys == RTCM_SYS_QZS))
		{
			TempTow = GetUnsignedBits(pBufIn,           Index, 30);         Index += 30;
			Tow = TempTow * 0.001;
			if (AdjWeek(pObsData, pRTCMInterimData, Tow) < 0)
			{
				return -1;
			}
		}
		else if (RTCMSatSys == RTCM_SYS_BDS)
		{
			TempTow = GetUnsignedBits(pBufIn,           Index, 30);         Index += 30;
			Tow = TempTow * 0.001 + 14.0;
			if(Tow >= 604800)
			{
				Tow -= 604800;
			}
			if (AdjWeek(pObsData, pRTCMInterimData, Tow) < 0)
			{
				return -1;
			}
		}
		else
		{
			TempTow = GetUnsignedBits(pBufIn,           Index, 30);         Index += 30;
			Tow = TempTow * 0.001;
			if (AdjWeek(pObsData, pRTCMInterimData, Tow) < 0)
			{
				return -1;
			}
		}

#ifdef RTCM_DEBUG_INFO
		*pTow = TempTow;
#endif

		pMSMHead->MultBit = GetUnsignedBits(pBufIn,                 Index, 1); Index += 1;
		pMSMHead->IODS = GetUnsignedBits(pBufIn,                    Index, 3); Index += 3;
		pMSMHead->TimeTrans = GetUnsignedBits(pBufIn,               Index, 7); Index += 7;
		pMSMHead->ClockSteeringInd = GetUnsignedBits(pBufIn,        Index, 2); Index += 2;
		pMSMHead->ExClockInd = GetUnsignedBits(pBufIn,              Index, 2); Index += 2;
		pMSMHead->DivSmoothingInterval = GetUnsignedBits(pBufIn,    Index, 1); Index += 1;
		pMSMHead->SmoothingInterval = GetUnsignedBits(pBufIn,       Index, 3); Index += 3;

		/** GNSS satellite mask */
		for (MaskIndex = 1; MaskIndex <= 64; MaskIndex++)
		{
			Mask = GetUnsignedBits(pBufIn,          Index, 1);              Index += 1;
			if (Mask)
			{
				pMSMHead->SatMask[pMSMHead->Nsat++] = (unsigned char)MaskIndex;
			}
		}

#ifdef RTCM_DEBUG_INFO
		*pSignalMask = GetUnsignedBits(pBufIn, Index, 32);
#endif

		/** GNSS signal mask */
		for (MaskIndex = 1; MaskIndex <= 32; MaskIndex++)
		{
			Mask = GetUnsignedBits(pBufIn,          Index, 1);              Index += 1;
			if (Mask)
			{
				pMSMHead->SignMask[pMSMHead->Nsig++] = (unsigned char)MaskIndex;
			}
		}
	}
	else
	{
		return -1;
	}

	if (CheckStationID(pRTCMInterimData, StationID))
	{
		return -1;
	}

	if (pMSMHead->Nsat * pMSMHead->Nsig > 64)
	{
		return -1;
	}

	if (Index + pMSMHead->Nsat * pMSMHead->Nsig > TotalBitsLen)
	{
		return -1;
	}

	for (MaskIndex = 0; MaskIndex < pMSMHead->Nsat * pMSMHead->Nsig; MaskIndex++)
	{
		pMSMHead->CellMask[MaskIndex] = (unsigned char)GetUnsignedBits(pBufIn, Index, 1); Index += 1;
		if (pMSMHead->CellMask[MaskIndex])
		{
			Ncell++;
		}
	}

	*pNcell = Ncell;
	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM rough range integer ms

@param Index                [In] bit pointer position of the current buffer
@param Nsat                 [In] satellite number
@param pMSMRouRang          [In] RTCM MSM rough range integer ms Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM rough range integer ms Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMIntRoughRangeDecoder(unsigned int Index, unsigned int Nsat, double* pRouRang,
											unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RangeIndex;
	unsigned int TempRange;

	if ((pRouRang == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Nsat; RangeIndex++)
	{
		TempRange = GetUnsignedBits(pBufIn, Index, 8); Index += 8;

		if (TempRange != 255)
		{
			pRouRang[RangeIndex] = TempRange * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM rough range modulo 1 ms

@param Index                [In] bit pointer position of the current buffer
@param Nsat                 [In] satellite number
@param pRouRang             [In] RTCM MSM rough range modulo 1 ms Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM rough range modulo 1 ms Log bit number
@param CheckType            [In] 0:no check, 1: check

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMModRoughRangeDecoder(unsigned int Index, unsigned int Nsat, double* pRouRang,
											unsigned char* pBufIn, unsigned int* pBitNumOut, int CheckType)
{
	unsigned int RangeIndex;
	unsigned int TempRange;

	if ((pRouRang == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Nsat; RangeIndex++)
	{
		TempRange = GetUnsignedBits(pBufIn, Index, 10); Index += 10;

		if (CheckType)
		{
			if (fabs(pRouRang[RangeIndex]) > 1E-12)
			{
				pRouRang[RangeIndex] += DoubleUScale2(TempRange, 10) * RANGE_MS;
			}
		}
		else
		{
			pRouRang[RangeIndex] += DoubleUScale2(TempRange, 10) * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM extended satellite info

@param Index                [In] bit pointer position of the current buffer
@param Nsat                 [In] satellite number
@param pInfo                [In] RTCM MSM extended satellite info Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM extended satellite info Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMInfoDecoder(unsigned int Index, unsigned int Nsat, unsigned int* pInfo,
							unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int InfoIndex;

	if ((pInfo == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (InfoIndex = 0; InfoIndex < Nsat; InfoIndex++)
	{
		pInfo[InfoIndex] = GetUnsignedBits(pBufIn, Index, 4);
		Index += 4;
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM rough phase-range-rate

@param Index                [In] bit pointer position of the current buffer
@param Nsat                 [In] satellite number
@param pRouRate             [In] RTCM MSM rough phase-range-rate Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM rough phase-range-rate Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMRoughRateDecoder(unsigned int Index, unsigned int Nsat, double* pRouRate,
									unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RateIndex;
	int TempRate;

	if ((pRouRate == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RateIndex = 0; RateIndex < Nsat; RateIndex++)
	{
		TempRate = GetSignedBits(pBufIn, Index, 14);
		Index += 14;

		if (TempRate != -8192)
		{
			pRouRate[RateIndex] = TempRate * 1.0;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM fine pseudorang

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pPsRang              [In] RTCM MSM fine pseudorang Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM fine pseudorang Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMPseudoRangeDecoder(unsigned int Index, unsigned int Ncell, double* pPsRang,
										unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RangeIndex;
	int TempPseudoRange;

	if ((pPsRang == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Ncell; RangeIndex++)
	{
		TempPseudoRange = GetSignedBits(pBufIn, Index, 15);
		Index += 15;

		if (TempPseudoRange != -16384)
		{
			pPsRang[RangeIndex] = DoubleIScale2(TempPseudoRange, 24) * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM fine pseudorang with extended resolution

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pPsRangEx            [In] RTCM MSM fine pseudorang with extended resolution Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM fine pseudorang with extended resolution Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMPseudoRangeExDecoder(unsigned int Index, unsigned int Ncell, double* pPsRangEx,
											unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RangeIndex;
	int TempPseudoRangeEx;

	if ((pPsRangEx == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Ncell; RangeIndex++)
	{
		TempPseudoRangeEx = GetSignedBits(pBufIn, Index, 20);
		Index += 20;

		if (TempPseudoRangeEx != -524288)
		{
			pPsRangEx[RangeIndex] = DoubleIScale2(TempPseudoRangeEx, 29) * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM fine phase-range

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pPhRang              [In] RTCM MSM fine phase-range Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM fine phase-range Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMPhaseRangeDecoder(unsigned int Index, unsigned int Ncell, double* pPhRang,
									unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RangeIndex;
	int TempPhaseRange;

	if ((pPhRang == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Ncell; RangeIndex++)
	{
		TempPhaseRange = GetSignedBits(pBufIn, Index, 22);
		Index += 22;

		if (TempPhaseRange != -2097152)
		{
			pPhRang[RangeIndex] = DoubleIScale2(TempPhaseRange, 29) * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM fine phase-range with extended resolution

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pPhRangEx            [In] RTCM MSM fine phase-range with extended resolution Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM fine phase-range with extended resolution Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMPhaseRangeExDecoder(unsigned int Index, unsigned int Ncell, double* pPhRangEx,
										unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RangeIndex;
	int TempPhaseRangeEx;

	if ((pPhRangEx == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RangeIndex = 0; RangeIndex < Ncell; RangeIndex++)
	{
		TempPhaseRangeEx = GetSignedBits(pBufIn, Index, 24);
		Index += 24;

		if (TempPhaseRangeEx != -8388608)
		{
			pPhRangEx[RangeIndex] = DoubleIScale2(TempPhaseRangeEx, 31) * RANGE_MS;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM lock-time indicator

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pLockTime            [In] RTCM MSM lock-time indicator Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM lock-time indicator Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMLockTimeDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pLockTime,
									unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int LockTimeIndex, TempLockTime;

	if ((pLockTime == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (LockTimeIndex = 0; LockTimeIndex < Ncell; LockTimeIndex++)
	{
		TempLockTime = GetUnsignedBits(pBufIn, Index, 4); Index += 4;
		if (TempLockTime > 0)
		{
			pLockTime[LockTimeIndex] = (1 << (TempLockTime + 4));
		}
		else
		{
			pLockTime[LockTimeIndex] = 0;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM lock-time indicator with extended range and resolution

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pLockTimeEx          [In] RTCM MSM lock-time indicator with extended range and resolution Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM lock-time indicator with extended range and resolution Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMLockTimeExDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pLockTimeEx,
									unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int LockTimeIndex;

	if ((pLockTimeEx == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (LockTimeIndex = 0; LockTimeIndex < Ncell; LockTimeIndex++)
	{
		pLockTimeEx[LockTimeIndex] = GetMSMLockTimeEx(GetUnsignedBits(pBufIn, Index, 10));
		Index += 10;
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM half-cycle-ambiguity indicator

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pHalfAmb             [In] RTCM MSM half-cycle-ambiguity indicator Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM half-cycle-ambiguity indicator Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMHalfCycleAmbDecoder(unsigned int Index, unsigned int Ncell, unsigned int* pHalfAmb,
										unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int HalfAmbIndex;

	if ((pHalfAmb == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (HalfAmbIndex = 0; HalfAmbIndex < Ncell; HalfAmbIndex++)
	{
		pHalfAmb[HalfAmbIndex] = GetUnsignedBits(pBufIn, Index, 1);
		Index += 1;
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM signal cnr

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pCnr                 [In] RTCM MSM signal cnr Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM signal cnr Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMCnrDecoder(unsigned int Index, unsigned int Ncell, double* pCnr,
							unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int CnrIndex;

	if ((pCnr == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (CnrIndex = 0; CnrIndex < Ncell; CnrIndex++)
	{
		pCnr[CnrIndex] = GetUnsignedBits(pBufIn, Index, 6) * 1.0;
		Index += 6;
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM signal cnr with extended resolution

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pCnrEx               [In] RTCM MSM signal cnr with extended resolution Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM signal cnr with extended resolution Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMCnrExDecoder(unsigned int Index, unsigned int Ncell, double* pCnrEx,
							unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int CnrIndex;

	if ((pCnrEx == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (CnrIndex = 0; CnrIndex < Ncell; CnrIndex++)
	{
		pCnrEx[CnrIndex] = GetUnsignedBits(pBufIn, Index, 10) * 0.0625;
		Index += 10;
	}

	*pBitNumOut = Index;

	return 0;
}

/*****************************************************************************//**
@brief decode rtcm MSM fine phase-range-rate

@param Index                [In] bit pointer position of the current buffer
@param Ncell                [In] Nsat * Nsig
@param pPhRate              [In] RTCM MSM phase-range-rate Data Buffer
@param pBufIn               [In] RTCM3 log data buffer
@param pBitNumOut           [Out] RTCM MSM phase-range-rate Log bit number

@return < 0:error, 0: success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
int MSMPhaseRangeRateExDecoder(unsigned int Index, unsigned int Ncell, double* pPhRate,
											unsigned char* pBufIn, unsigned int* pBitNumOut)
{
	unsigned int RateIndex;
	int TempRate;

	if ((pPhRate == NULL) || (pBufIn == NULL) || (pBitNumOut == NULL))
	{
		return -1;
	}

	for (RateIndex = 0; RateIndex < Ncell; RateIndex++)
	{
		TempRate = GetSignedBits(pBufIn, Index, 15);
		Index += 15;

		if (TempRate != -16384)
		{
			pPhRate[RateIndex] = TempRate * 0.0001;
		}
	}

	*pBitNumOut = Index;

	return 0;
}

#ifdef RTCM_DEBUG_INFO

/**********************************************************************//**
@brief  printf satellite number information

@param pParseInfo          [In] rtcm parse information
@param pRTCMInterimData    [In] RTCM interim data
@param pObsData            [In] obs data

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int SatNumbInfoPrintf(RTCM_PARSE_INFO_T* pParseInfo, RTCM_INTERIM_DATA_T* pRTCMInterimData, GNSS_OBS_T* pObsData)
{
	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;

	unsigned int MsgLen = sprintf(pBaseObsSatCountBuf, "%u,\t\t%.3lf,\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u,\t\t\t%02u",
		pObsData->RangeData.LogHeader.Week, (double)(pObsData->RangeData.LogHeader.Milliseconds * 0.001),
		pBaseObsSatNumBuf[RTCM_SYS_GPS], pBaseObsSatNumBuf[RTCM_SYS_GLO], pBaseObsSatNumBuf[RTCM_SYS_GAL],
		pBaseObsSatNumBuf[RTCM_SYS_SBA], pBaseObsSatNumBuf[RTCM_SYS_QZS], pBaseObsSatNumBuf[RTCM_SYS_BDS],
		pBaseObsSatNumBuf[RTCM_SYS_NIC]);
	pBaseObsSatCountBuf[MsgLen] = '\0';
	pParseInfo->ParsedBaseObsSatCountBuf.Len = MsgLen + 1;
	MEMSET(pRTCMInterimData->BaseObsSatNum, 0x00, sizeof(unsigned int) * RTCM_SYS_NUM);

	return 0;
}

#endif

/**********************************************************************//**
@brief  save obs data in msm message

@param ExSystemID           [In] internal used system ID
@param MSMID                [In] MSM ID
@param TypeID               [In] RTCM3 type id
@param pObsData             [In] obs data struct pointer
@param pRTCMInterimData     [In] RTCM decode interim data pointer
@param pRouRang             [In] GNSS Satellite rough ranges
@param pPsRang              [In] GNSS signal fine Pseudoranges
@param pPhRang              [In] GNSS signal fine Phaserange
@param pRouRate             [In] GNSS Satellite rough PhaseRangeRates
@param pPhRate              [In] GNSS signal fine PhaseRangeRates
@param pCnr                 [In] GNSS signal CNRs
@param pLockTime            [In] GNSS Phaserange Lock Time Indicator
@param pInfo                [In] Extended Satellite Information
@param pHalfAmb             [In] Half-cycle ambiguity indicator
@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function
@param RTCMSatSys           [In] RTCM MSM Satellite system
@param SysSatNum            [In] Satellite number

@return <0: fail
		=0/1: ObsCompleteFlag

@author CHC
@date 2023/03/13
@note
**************************************************************************/
static int SaveMSMObs(int ExSystemID, int MSMID, int TypeID, GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
							RTCM_MSM_HEADER_DATA_T* pHeaderData, double* pRoughRange, double* pPseudoRange,
							double *pPhaseRange, double *pRoughRate, double *pPhaseRangeRate, double *pCnr,
							unsigned int* pLockTime, unsigned int* pInfo, unsigned int* pHalfCycleAmb, RTCM_PARSE_INFO_T* pParseInfo,
							void* pSrcObj, RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback, int RTCMSatSys)
{
	double WaveLen;
	INT64 TempTimeDiff;
	unsigned int SatIndex, SigIndex, DataIndex, ObsIndex = 0, SatID, FreqChnn = 0;
	unsigned char CellMask[RTCM_MAX_CELL_NUM];
	unsigned int SignalMap[RTCM_MAX_SIGN_NUM], FreqIndexTab[RTCM_MAX_SIGN_NUM];
	unsigned int SatelliteSystem, SignalType, CarrPhaMeasurement, PhaseLockFlag, SigTypeIndex;
	RM_RANGE_DATA_T* pRMRangeData;
	unsigned int FreqID, FreqIndex;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned int OverTimeFlag = 0;
	unsigned int SignalID;
	unsigned int BDSExpandFlag = 0;
	unsigned int BDSSignalMask = 0;
	unsigned int LastSysSatNum = 0;

	if ((!pObsData) || (!pRTCMInterimData))
	{
		return -1;
	}

	pSatObsData = &pRTCMInterimData->SatObsData;

	if (ExSystemID == BDS)
	{
		for (SigIndex = 0; SigIndex < pHeaderData->Nsig; SigIndex++)
		{
			SignalID = pHeaderData->SignMask[SigIndex];
			if ((SignalID >= RTCM3_SIGNAL_BDS_B1AD) && (SignalID <= RTCM3_SIGNAL_BDS_B1AX))
			{
				BDSSignalMask |= BDS_SIGNAL_EXPAND_MASK_B1C;
			}
			else if ((SignalID >= RTCM3_SIGNAL_BDS_B3AED) && (SignalID <= RTCM3_SIGNAL_BDS_B3AEX))
			{
				BDSSignalMask |= BDS_SIGNAL_EXPAND_MASK_B2A;
			}
			else if ((SignalID >= RTCM3_SIGNAL_BDS_B1CD) && (SignalID <= RTCM3_SIGNAL_BDS_B1CX))
			{
				BDSSignalMask |= BDS_SIGNAL_STANDARD_MASK_B1C;
			}
			else if ((SignalID >= RTCM3_SIGNAL_BDS_B2AD) && (SignalID <= RTCM3_SIGNAL_BDS_B2AX))
			{
				BDSSignalMask |= BDS_SIGNAL_STANDARD_MASK_B2A;
			}
		}

		if ((0 == (BDSSignalMask & BDS_SIGNAL_STANDARD_MASK)) && ((BDSSignalMask & BDS_SIGNAL_EXPAND_MASK) > 0))
		{
			BDSExpandFlag = 1;
		}
	}

	for (SigIndex = 0; SigIndex < pHeaderData->Nsig; SigIndex++)
	{
		SignalMap[SigIndex] = RTCM3SignalToRMRangeSignalByExSystem(ExSystemID, pHeaderData->SignMask[SigIndex] - 1, &FreqIndexTab[SigIndex], BDSExpandFlag);
	}

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &pRTCMInterimData->RTCMTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag) && (pSatObsData->SatNum > 0))
		{
			DecodeObsOverTimeCallback(TypeID, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback, RAW_OBS_DATA_TYPE_RTCM_MSM);
#ifdef RTCM_DEBUG_INFO
			SatNumbInfoPrintf(pParseInfo, pRTCMInterimData, pObsData);
#endif
		}
		ObsIndex = pRTCMInterimData->ObsCompleteFlag = 0;
		pObsData->BDSSignalExpand = 0;
		MEMSET(pRTCMInterimData->MSMDataMask, 0x00, sizeof(pRTCMInterimData->MSMDataMask));
		MEMSET(pRTCMInterimData->ObsDataOffset, 0x00, sizeof(pRTCMInterimData->ObsDataOffset));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
		pRTCMInterimData->MSMDataCompleteTypeID = pRTCMInterimData->MSMDataCompleteTypeIDLast;
		pObsData->RangeData.ObsNum = 0;
		pObsData->ObsSystemMask = 0;
	}
	pRTCMInterimData->LastRTCMTime = pRTCMInterimData->RTCMTime;
	LastSysSatNum = pRTCMInterimData->SatObsData.SatNum;

	if (BDSExpandFlag)
	{
		pObsData->BDSSignalExpand = BDSExpandFlag;
	}

	if (pRTCMInterimData->MSMDataCompleteTypeID == 0)
	{
		pRTCMInterimData->MSMDataCompleteTypeID = pRTCMInterimData->MSMDataCompleteTypeIDLast;
	}

	pRTCMInterimData->MSMDataCompleteTypeIDLast = TypeID;

	SET_MSM_MASK(pRTCMInterimData->MSMDataMask[ExSystemID], MSMID);

	if (!GET_MSM_MASK(pRTCMInterimData->MSMDataMaskLast[ExSystemID], MSMID))
	{
		SET_MSM_MASK(pRTCMInterimData->MSMDataMaskLast[ExSystemID], MSMID);
	}

	if (GET_MSM_MASK_OTHER(pRTCMInterimData->MSMDataMask[ExSystemID], MSMID))
	{
		ObsIndex = pRTCMInterimData->ObsDataOffset[ExSystemID];
	}
	else
	{
		ObsIndex = pObsData->RangeData.ObsNum;
		pRTCMInterimData->ObsDataOffset[ExSystemID] = ObsIndex;
	}

	for (SatIndex = DataIndex = 0; SatIndex < pHeaderData->Nsat; SatIndex++)
	{
		MEMSET(CellMask, 0, sizeof(unsigned char) * RTCM_MAX_CELL_NUM);
		for (SigIndex = 0; SigIndex < pHeaderData->Nsig; SigIndex++)
		{
			CellMask[SigIndex] = pHeaderData->CellMask[SigIndex + SatIndex * pHeaderData->Nsig];
		}

		SatID = pHeaderData->SatMask[SatIndex];

		for (SigIndex = 0; SigIndex < pHeaderData->Nsig; SigIndex++)
		{
			if (!pHeaderData->CellMask[SigIndex + SatIndex * pHeaderData->Nsig])
			{
				continue;
			}

			if (SignalMap[SigIndex] == NA8)
			{
				DataIndex++;
				continue;
			}

			SignalType = SignalMap[SigIndex];
			FreqIndex = FreqIndexTab[SigIndex];
			SigTypeIndex = pHeaderData->SignMask[SigIndex] - 1;

			if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
			{
				DataIndex++;
				continue;
			}

			if (IsNaN(pRoughRange[SatIndex])) /**< for MSM1-3 satellite RoughRange is invaild */
			{
				DataIndex++;
				continue;
			}

			pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
			pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

			pRMRangeData->PRN = MSMSatIDToPRN(ExSystemID, SatID);
			SatelliteSystem = ExSystemToRMRangeSystem(ExSystemID);

			/** glonass wave length by extended info */
			if (ExSystemID == GLO)
			{
				if (pInfo && pInfo[SatIndex] <= 13)
				{
					FreqChnn = pInfo[SatIndex];
				}
				else
				{
					FreqChnn = (unsigned int)GetGloFreqCh(SatID);
				}
			}
			FreqID = ((FreqIndex & BIT_MASK(SID_BIT_LEN_FREQ)) << SID_BIT_LEN_FREQ) | (ExSystemID & BIT_MASK(SID_BIT_LEN_SYS));
			WaveLen = GetCarrierWaveLength(FreqID, FreqChnn);

			/** pseudorange (m) */
			if (!IsAbsoluteDoubleZero(pRoughRange[SatIndex]) && (pPseudoRange[DataIndex] > -1E12))
			{
				pRMRangeData->PSR = pRoughRange[SatIndex] + pPseudoRange[DataIndex];
			}

			/** carrier-phase (cycle) */
			if (!IsAbsoluteDoubleZero(pRoughRange[SatIndex]) && (pPhaseRange[DataIndex] > -1E12) && (WaveLen > 0))
			{
				pRMRangeData->ADR = -(pRoughRange[SatIndex] + pPhaseRange[DataIndex]) / WaveLen;
			}

			/** doppler (hz) */
			if (pRoughRate && pPhaseRangeRate && !IsAbsoluteDoubleZero(pRoughRate[SatIndex]) && (pPhaseRangeRate[DataIndex] > -1E12) && (WaveLen > 0))
			{
				pRMRangeData->Doppler = (float)(-(pRoughRate[SatIndex] + pPhaseRangeRate[DataIndex]) / WaveLen);
			}

			pRMRangeData->LockTime = (float)(pLockTime[DataIndex] / 1000.0);
			CarrPhaMeasurement = (pHalfCycleAmb[DataIndex] ? 1 : 0);
			pRMRangeData->CN0 = (float)pCnr[DataIndex];
			pRMRangeData->FreqNum = (ExSystemID == GLO) ? FreqChnn : 0;
			PhaseLockFlag = (SigTypeIndex < RTCM_MAX_SIGN_NUM) ? LossOfLock(&pRTCMInterimData->LockTime[ExSystemID][SatID-1][SigTypeIndex], pLockTime[DataIndex]) : 0;

			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, SatelliteSystem, SignalType, CarrPhaMeasurement);

			ObsIndex++;

			DataIndex++;

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

	if (!pHeaderData->MultBit)
	{
		if ((pRTCMInterimData->MSMDataCompleteTypeID == TypeID) ||
			(!GET_MSM_MASK_FORNT(pRTCMInterimData->MSMDataMaskLast[ExSystemID], MSMID)))
		{
			DecodeObsCompleteCallback(pObsData, pRTCMInterimData, pSatObsData, RAW_OBS_DATA_TYPE_RTCM_MSM);
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
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	pBaseObsSatNumBuf[RTCMSatSys] += (pRTCMInterimData->SatObsData.SatNum - LastSysSatNum);
#endif

	pObsData->ObsSystemMask |= (1 << ExSystemID);

	return pRTCMInterimData->ObsCompleteFlag;
}

/**********************************************************************//**
@brief  get RTCM3 MSM NSat Data Bits

@param MSMID           [In] RTCM MSM id

@retval 0xffffffff:error, other:NSat Data Bits

@author CHC
@date 2023/05/30
@note
**************************************************************************/
static unsigned int GetRTCM3MSMNSatDataBits(int MSMID)
{
	const static unsigned int s_MSMNSatDataBitMap[] =
	{
		RTCM3_MSM1S_LEN,
		RTCM3_MSM2S_LEN,
		RTCM3_MSM3S_LEN,
		RTCM3_MSM4S_LEN,
		RTCM3_MSM5S_LEN,
		RTCM3_MSM6S_LEN,
		RTCM3_MSM7S_LEN,
	};

	if (MSMID <= RTCM3_MSM7)
	{
		return s_MSMNSatDataBitMap[MSMID];
	}

	return NA32;
}

/**********************************************************************//**
@brief  get RTCM3 MSM NCell Data Bits

@param MSMID           [In] RTCM MSM id

@retval 0xffffffff:error, other:NCell Data Bits

@author CHC
@date 2023/05/30
@note
**************************************************************************/
static unsigned int GetRTCM3MSMNCellDataBits(int MSMID)
{
	const static unsigned int s_MSMNCellDataBitMap[] =
	{
		RTCM3_MSM1C_LEN,
		RTCM3_MSM2C_LEN,
		RTCM3_MSM3C_LEN,
		RTCM3_MSM4C_LEN,
		RTCM3_MSM5C_LEN,
		RTCM3_MSM6C_LEN,
		RTCM3_MSM7C_LEN,
	};

	if (MSMID <= RTCM3_MSM7)
	{
		return s_MSMNCellDataBitMap[MSMID];
	}

	return NA32;
}

/**********************************************************************//**
@brief translate rtcm msm message system to raw message system

@param RTCMMSMSystem [In] rtcm msm message system

@return RMRangeSystem

@author CHC
@date 2022/05/30
@note
**************************************************************************/
static unsigned int RTCMMSMSystemToExSystem(int RTCMMSMSystem)
{
	const static unsigned int s_ExSystemMap[RTCM_SYS_NUM] =
	{
		GPS,
		GLO,
		GAL,
		SBA,
		QZS,
		BDS,
		NIC,
	};

	if (RTCMMSMSystem < RTCM_SYS_NUM)
	{
		return s_ExSystemMap[RTCMMSMSystem];
	}

	return NA32;
}

/**********************************************************************//**
@brief  Calibrate Int Rough Range

@param ExSystemID          [In] GNSS system id
@param SatID               [In] Satellite id
@param pRTCMInterimData    [In] pointer to rtcm interim data
@param ModRoughRange       [In] Mod Rough Range
@param pIntRoughRange      [InOut] Int Rough Range

@retval <0 error, =0 success

@author CHC
@date 2023/03/09
@note
**************************************************************************/
static int CalibrateIntRoughRange(unsigned int ExSystemID, unsigned int SatID, RTCM_INTERIM_DATA_T* pRTCMInterimData,
										double ModRoughRange, int* pIntRoughRange)
{
	RTCM_ROUGHRANGE_INFO_T* pRoughRangeInfo = &pRTCMInterimData->RoughRangeInfo[ExSystemID][SatID - 1];
	int IntRoughRange = *pIntRoughRange;

	if (pRoughRangeInfo->DataVaild == ROUGHRANGE_DATA_INVAILD)
	{
		if (IsAbsoluteDoubleZero(ModRoughRange)) /**< first use, data maybe invaild */
		{
			return -1;
		}

		pRoughRangeInfo->IntRoughRange = IntRoughRange;
		pRoughRangeInfo->ModRoughRange = ModRoughRange;
		pRoughRangeInfo->DataVaild = ROUGHRANGE_DATA_VAILD;

		return 0;
	}

	if (fabs(ModRoughRange - pRoughRangeInfo->ModRoughRange) > RANGE_MS_CHECK_VALUE)
	{
		if (IntRoughRange == pRoughRangeInfo->IntRoughRange)
		{
			if (pRoughRangeInfo->ModRoughRange > ModRoughRange)
			{
				IntRoughRange++;
			}
			else
			{
				if (IntRoughRange > 0)
				{
					IntRoughRange--;
				}
			}

			*pIntRoughRange = IntRoughRange;

			pRoughRangeInfo->IntRoughRange = IntRoughRange;
			pRoughRangeInfo->ModRoughRange = ModRoughRange;
		}
		else
		{
			pRoughRangeInfo->IntRoughRange = IntRoughRange;
			pRoughRangeInfo->ModRoughRange = ModRoughRange;
		}
	}
	else
	{
		if (abs(IntRoughRange - pRoughRangeInfo->IntRoughRange) > 1) /**< over 1ms, search again */
		{
			pRoughRangeInfo->IntRoughRange = IntRoughRange;
			pRoughRangeInfo->ModRoughRange = ModRoughRange;
		}
		else
		{
			IntRoughRange = pRoughRangeInfo->IntRoughRange;

			*pIntRoughRange = IntRoughRange;
			pRoughRangeInfo->ModRoughRange = ModRoughRange;
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  Calculate Int Rough Range

@param ExSystemID          [In] GNSS system id
@param NSat                [In] Satellite number
@param pRTCMInterimData    [In] pointer to rtcm interim data
@param pSatMasK            [In] Satellite index table
@param pRoughRange         [Out] Rough Range

@retval <0 error, =0 success

@author CHC
@date 2023/03/09
@note
**************************************************************************/
static int CalculateIntRoughRange(unsigned int ExSystemID, unsigned int NSat, RTCM_INTERIM_DATA_T* pRTCMInterimData, unsigned int* pSatIndexTab, double* pRoughRange)
{
#ifndef _NO_CTCLIB_
#ifndef MSG_CONVERT
	int GPSWeek, WeekMsCount, GPSLeapSecMs;
	unsigned int SatIndex;
	double PosVel[6]; /**< ECEF-XYZ*/
	double SatClock;
	BOOL Result = FALSE;
	double Distance;
	RTCM_REFSTATION_INFO_T* pRefStationInfo = GetRTCMRefStationInfo();
	double RoughRanges;
	int IntRoughRange;

	if ((!pRTCMInterimData) || (!pRoughRange) || (!pSatIndexTab))
	{
		return -1;
	}

	if (IsAbsoluteDoubleZero(pRefStationInfo->Position[0]) && IsAbsoluteDoubleZero(pRefStationInfo->Position[1]) && IsAbsoluteDoubleZero(pRefStationInfo->Position[2]))
	{
		return -1;
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(&pRTCMInterimData->RTCMTime);
	UTCToGPSTime(&pRTCMInterimData->RTCMTime, &GPSLeapSecMs, &GPSWeek, &WeekMsCount);

	for (SatIndex = 0; SatIndex < NSat; SatIndex++)
	{
		if (pSatIndexTab[SatIndex] > 0)
		{
			Result = GetSatellitePositionForBSW(GPSWeek, WeekMsCount, ExSystemID, pSatIndexTab[SatIndex], PosVel, &SatClock);
			if (Result == FALSE)
			{
				MEMSET(&pRoughRange[SatIndex], 0xFF, sizeof(double)); /**< set satellite RoughRanges is invaild */
				continue;
			}

			Distance = GetGeoDistanceXYZ(pRefStationInfo->Position, PosVel, NULL) - SatClock * LIGHT_SPEED;

			IntRoughRange = (int)(Distance / RANGE_MS);

			if (CalibrateIntRoughRange(ExSystemID, pSatIndexTab[SatIndex], pRTCMInterimData, pRoughRange[SatIndex], &IntRoughRange) < 0)
			{
				MEMSET(&pRoughRange[SatIndex], 0xFF, sizeof(double)); /**< set satellite RoughRanges is invaild */
				continue;
			}

			RoughRanges=(double)IntRoughRange * RANGE_MS;

			pRoughRange[SatIndex] += RoughRanges;
		}
	}
#endif
#endif
	return 0;
}

/**********************************************************************//**
@brief  decode RTCM3MSM

@param RTCMSatSys          [In] RTCM MSM Satellite system
@param TypeID              [In] RTCM3 type id
@param pParseInfo          [In] rtcm parse information
@param pSrcObj             [In] pointer to source object
@param pOverTimeCallback   [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: GNSS data id

@author CHC
@date 2023/03/09
@note
**************************************************************************/
int RTCM3DecodeTypeMSM(int RTCMSatSys, unsigned TypeID, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj,
									RTCM3_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned char* pBufIn;
	unsigned int Index = 0;
	RTCM_MSM_HEADER_DATA_T MSMHeaderData = {0};
	unsigned int DataIndex, TotalBitsLen;
	unsigned int Ncell;
	double RoughRange[64] = {0}, RoughRate[64] = {0}, PseudoRange[64] = {0}, PhaseRange[64] = {0}, PhaseRangeRate[64] = {0}, Cnr[64] = {0};
	unsigned int Info[64], LockTime[64], HalfCycleAmb[64];
	unsigned int MsgNumBase, MSMID, ExSystemID;
	unsigned int NSatDataBits, NCellDataBits;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	int Result;
	unsigned int Tow, SignalMask;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pObsData = (GNSS_OBS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	/** decode msm header */
	if (RTCM3MSMHeadDecoder(RTCMSatSys, pObsData, pRTCMInterimData, &MSMHeaderData, pBufIn, TotalBitsLen, &Index, &Ncell, &Tow, &SignalMask) < 0)
	{
		return -1;
	}

	MsgNumBase = GetRTCM3MSMMsgNumBase(RTCMSatSys);
	if (MsgNumBase == NA32)
	{
		return -1;
	}

	ExSystemID = RTCMMSMSystemToExSystem(RTCMSatSys);
	if (ExSystemID == NA32)
	{
		return -1;
	}

	MSMID = TypeID - MsgNumBase;
	if (MSMID > RTCM3_MSM7)
	{
		return -1;
	}

	NSatDataBits = GetRTCM3MSMNSatDataBits(MSMID);
	NCellDataBits = GetRTCM3MSMNCellDataBits(MSMID);

	if ((NSatDataBits == NA32) || (NCellDataBits == NA32))
	{
		return -1;
	}

	if (Index + MSMHeaderData.Nsat * NSatDataBits + Ncell * NCellDataBits > TotalBitsLen)
	{
		return -1;
	}

	for (DataIndex = 0; DataIndex < MSMHeaderData.Nsat; DataIndex++)
	{
		Info[DataIndex] = 15;
	}

	for (DataIndex = 0; DataIndex < Ncell; DataIndex++)
	{
		PseudoRange[DataIndex] = PhaseRange[DataIndex] = PhaseRangeRate[DataIndex] = -1E16;
	}

	/** decode msm satellite data */
	switch (MSMID)
	{
		case RTCM3_MSM1:
			{
				/** decode msm satellite data */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 0); /**< rough range modulo 1 ms */

				/** decode msm signal data */
				MSMPseudoRangeDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
			}
			break;
		case RTCM3_MSM2:
			{
				/** decode msm satellite data */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 0); /**< rough range modulo 1 ms */

				/** decode msm signal data */
				MSMPhaseRangeDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
			}
			break;
		case RTCM3_MSM3:
			{
				/** decode msm satellite data */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 0); /**< rough range modulo 1 ms */

				/** decode msm signal data */
				MSMPseudoRangeDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
				MSMPhaseRangeDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
			}
			break;
		case RTCM3_MSM4:
			{
				/** decode msm satellite data */
				MSMIntRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index); /**< rough range integer ms */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 1); /**< rough range modulo 1 ms */

				/** decode msm signal data */
				MSMPseudoRangeDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
				MSMPhaseRangeDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
				MSMCnrDecoder(Index, Ncell, Cnr, pBufIn, &Index); /**< signal cnr */
			}
			break;
		case RTCM3_MSM5:
			{
				/** decode msm satellite data */
				MSMIntRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index); /**< rough range integer ms */
				MSMInfoDecoder(Index, MSMHeaderData.Nsat, Info, pBufIn, &Index); /**< extended satellite info */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 1); /**< rough range modulo 1 ms */
				MSMRoughRateDecoder(Index, MSMHeaderData.Nsat, RoughRate, pBufIn, &Index); /**< rough phase-range-rate */

				/** decode msm signal data */
				MSMPseudoRangeDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
				MSMPhaseRangeDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
				MSMCnrDecoder(Index, Ncell, Cnr, pBufIn, &Index); /**< signal cnr */
				MSMPhaseRangeRateExDecoder(Index, Ncell, PhaseRangeRate, pBufIn, &Index); /**< fine phase-range-rate */
			}
			break;
		case RTCM3_MSM6:
			{
				/** decode msm satellite data */
				MSMIntRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index); /**< rough range integer ms */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 1); /**< rough range modulo 1 ms */

				/** decode msm signal data */
				MSMPseudoRangeExDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
				MSMPhaseRangeExDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeExDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
				MSMCnrExDecoder(Index, Ncell, Cnr, pBufIn, &Index); /**< signal cnr */
			}
			break;
		case RTCM3_MSM7:
			{
				/** decode msm satellite data */
				MSMIntRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index); /**< rough range integer ms */
				MSMInfoDecoder(Index, MSMHeaderData.Nsat, Info, pBufIn, &Index); /**< extended satellite info */
				MSMModRoughRangeDecoder(Index, MSMHeaderData.Nsat, RoughRange, pBufIn, &Index, 1); /**< rough range modulo 1 ms */
				MSMRoughRateDecoder(Index, MSMHeaderData.Nsat, RoughRate, pBufIn, &Index); /**< rough phase-range-rate */

				/** decode msm signal data */
				MSMPseudoRangeExDecoder(Index, Ncell, PseudoRange, pBufIn, &Index); /**< fine pseudorange */
				MSMPhaseRangeExDecoder(Index, Ncell, PhaseRange, pBufIn, &Index); /**< fine phase-range */
				MSMLockTimeExDecoder(Index, Ncell, LockTime, pBufIn, &Index); /**< lock-time indicator */
				MSMHalfCycleAmbDecoder(Index, Ncell, HalfCycleAmb, pBufIn, &Index); /**< half-cycle-amb indicator */
				MSMCnrExDecoder(Index, Ncell, Cnr, pBufIn, &Index); /**< signal cnr */
				MSMPhaseRangeRateExDecoder(Index, Ncell, PhaseRangeRate, pBufIn, &Index); /**< fine phase-range-rate */
			}
			break;
	}

	if ((MSMID == RTCM3_MSM1) || (MSMID == RTCM3_MSM2) || (MSMID == RTCM3_MSM3))
	{
		CalculateIntRoughRange(ExSystemID, MSMHeaderData.Nsat, pRTCMInterimData, MSMHeaderData.SatMask, RoughRange);
	}

	Result = SaveMSMObs(ExSystemID, MSMID, TypeID, pObsData, pRTCMInterimData, &MSMHeaderData, RoughRange,
							PseudoRange, PhaseRange, RoughRate, PhaseRangeRate, Cnr, LockTime, Info, HalfCycleAmb,
							pParseInfo, pSrcObj, pOverTimeCallback, RTCMSatSys);

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalIndex;
	const char* pSiganlName;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u MSM%d %4s:Epoch %10u,Sat:%2u, Signal:%u, BaseID %04u, Sync %d,",
		TypeID, (MSMID+1), GetSystemNameString(ExSystemID), Tow, MSMHeaderData.Nsat, MSMHeaderData.Nsig,
		pRTCMInterimData->StationID, Result);

	MsgLen += sprintf(pDebugInfoBuf + MsgLen, " Signal Mask: %08X", SignalMask);

	for (SignalIndex = 0; SignalIndex < MSMHeaderData.Nsig; SignalIndex++)
	{
		pSiganlName = GetRTCM3SignalName(RTCMSatSys, MSMHeaderData.SignMask[SignalIndex]);
		MsgLen += sprintf(pDebugInfoBuf + MsgLen, "   %02u %8s", MSMHeaderData.SignMask[SignalIndex], pSiganlName);
	}

	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	if (Result)
	{
		SatNumbInfoPrintf(pParseInfo, pRTCMInterimData, pObsData);
	}
#endif

	return Result;
}

