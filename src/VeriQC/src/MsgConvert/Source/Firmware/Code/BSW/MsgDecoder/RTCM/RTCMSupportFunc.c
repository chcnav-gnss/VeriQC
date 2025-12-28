/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RTCMSupportFunc.c
@author CHC
@date   2023/03/02
@brief

**************************************************************************/

#include "RTCMSupportFunc.h"
#include "Common/GNSSSignal.h"
#include "Common/GNSSConstants.h"


const static unsigned int s_1Mask[] = {0, 1, (1 << 2) - 1, (1 << 3) - 1, (1 << 4) - 1, (1 << 5) - 1, (1 << 6) - 1, (1 << 7) - 1,
						  (1 << 8) - 1, (1 << 9) - 1, (1 << 10) - 1, (1 << 11) - 1, (1 << 12) - 1, (1 << 13) - 1,
						  (1 << 14) - 1, (1 << 15) - 1, (1 << 16) - 1, (1 << 17) - 1, (1 << 18) - 1, (1 << 19) - 1,
						  (1 << 20) - 1, (1 << 21) - 1, (1 << 22) - 1, (1 << 23) - 1, (1 << 24) - 1, (1 << 25) - 1,
						  (1 << 26) - 1, (1 << 27) - 1, (1 << 28) - 1, (1 << 29) - 1, (1 << 30) - 1, (1u << 31) - 1,
						  0xffffffff};

#define GetMultiBit(OutputData, SrcData, SrcStartBit, BitNum)	((OutputData) = ((OutputData) << (BitNum)) + (((SrcData) >> (SrcStartBit)) & s_1Mask[BitNum]))

/**********************************************************************//**
@brief extract unsigned bits

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)
@param Len          [In] bit length (bits) (len<=32)

@retval unsigned bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
unsigned int GetUnsignedBits(const unsigned char *pBuf, int Pos, int Len)
{
	unsigned int Bits = 0;
	unsigned int MsgIndex;
	unsigned int TempLen;
	unsigned char RemainBit;

	MsgIndex = Pos >> 3;
	RemainBit = 8 - (Pos & 0x7);
	TempLen = Len;
	while (TempLen >= RemainBit)
	{
		TempLen -= RemainBit;
		GetMultiBit(Bits, pBuf[MsgIndex], 0, RemainBit);
		RemainBit = 8;
		MsgIndex++;
	}

	if (TempLen)
	{
		GetMultiBit(Bits, pBuf[MsgIndex], (RemainBit - TempLen), TempLen);
	}

	return Bits;
}

/**********************************************************************//**
@brief extract signed bits

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)
@param Len          [In] bit length (bits) (len<=32)

@retval signed bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
int GetSignedBits(const unsigned char *pBuf, int Pos, int Len)
{
	unsigned int Bits = GetUnsignedBits(pBuf, Pos, Len);

	if (Len <= 0 || 32 <= Len || !(Bits & (1u << (Len - 1))))
	{
		return (int)Bits;
	}

	return (int)(Bits | (~0u << Len)); /* extend sign */
}

/**********************************************************************//**
@brief get signed bits to byte data

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)

@retval signed bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
double Get38Bits(const unsigned char *pBuf, int Pos)
{
	return (double)GetSignedBits(pBuf, Pos, 32)*64.0 + GetUnsignedBits(pBuf, Pos + 32, 6);
}

/**********************************************************************//**
@brief get signed bits to byte data

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)

@retval signed bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
UINT64 GetUnsigned40Bits(const unsigned char *pBuf, int Pos)
{
	return (((UINT64)GetUnsignedBits(pBuf, Pos, 32)) << 8) + GetUnsignedBits(pBuf, Pos + 32, 8);
}

/**********************************************************************//**
@brief get signed bits to byte data

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)

@retval signed bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
double Get64Bits(const unsigned char *pBuf, int Pos)
{
	return (double)GetSignedBits(pBuf, Pos, 32)* 4294967296.0 + GetUnsignedBits(pBuf, Pos + 32, 32);
}

/**********************************************************************//**
@brief get sign-magnitude bits

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)
@param Len          [In] bit length (bits) (len<=32)

@retval sign-magnitude bits

@author CHC
@date 2022/05/17
@note
**************************************************************************/
int GetSignedMagBits(const unsigned char *pBuf, int Pos, int Len)
{
	int Value = GetUnsignedBits(pBuf, Pos + 1, Len - 1);
	return GetUnsignedBits(pBuf, Pos, 1) ? -Value : Value;
}

/**********************************************************************//**
@brief get signed bits to byte data

@param pBuf         [In] byte data
@param Pos          [In] bit position from start of data (bits)

@retval sign-magnitude bits

@author CHC
@date 2024/04/24
@note
**************************************************************************/
UINT64 GetUINT64Bits(const unsigned char* pBuf, int Pos)
{
	return (((UINT64)GetSignedBits(pBuf, Pos, 32)) << 32) + (UINT64)GetUnsignedBits(pBuf, Pos + 32, 32);
}

/**********************************************************************//**
@brief  get current time in utc

@retval current time in utc

@author CHC
@date 2023/04/10
@note
**************************************************************************/
UTC_TIME_T RTCMGetUTCTime(void)
{
	UTC_TIME_T TempUTCTime = {0};
#ifdef WIN32
	SYSTEMTIME SysTime;

	GetSystemTime(&SysTime); /* utc */

	TempUTCTime.Year = SysTime.wYear;
	TempUTCTime.Month = SysTime.wMonth;
	TempUTCTime.Day = SysTime.wDay;
	TempUTCTime.Hour = SysTime.wHour;
	TempUTCTime.Minute = SysTime.wMinute;
	TempUTCTime.Second = SysTime.wSecond;
	TempUTCTime.Millisecond = SysTime.wMilliseconds;

#elif defined(__linux)
	struct timeval TimeVal;
	struct tm *TempTime;

	if (!gettimeofday(&TimeVal, NULL) && (TempTime = gmtime(&TimeVal.tv_sec)))
	{
		TempUTCTime.Year = TempTime->tm_year + 1900;
		TempUTCTime.Month = TempTime->tm_mon + 1;
		TempUTCTime.Day = TempTime->tm_mday;
		TempUTCTime.Hour = TempTime->tm_hour;
		TempUTCTime.Minute = TempTime->tm_min;
		TempUTCTime.Second = TempTime->tm_sec;
		TempUTCTime.Millisecond = 0;
	}
#else

	unsigned int Week = 0;
	unsigned int Milliseconds = 0;
	unsigned char TimeStatus = RM_TIME_STATUS_UNKNOWN;

	GetSystemGPSTime(&Week, &Milliseconds, &TimeStatus);

	if ((TimeStatus != RM_TIME_STATUS_UNKNOWN)
		&& (TimeStatus != RM_TIME_STATUS_APPROXIMATE)
		&& (Week > 0))
	{
		int GPSLeapSecMs = GetGPSLeapSecondMs(&TempUTCTime);
		GPSTimeToUTC(Week, Milliseconds, &GPSLeapSecMs, &TempUTCTime);
	}

#endif
	return TempUTCTime;
}

/**********************************************************************//**
@brief  get per satellite obs data index

@param pSatObsData       [In] satellite obs data
@param ExSystemID        [In] system id
@param SatID             [In] satellite id
@param SignalType        [In] signal type
@param pSatIndex         [Out] satellite index
@param pObsIndex         [InOut] obs index

@retval <0 error, =0 success

@author CHC
@date 2023/11/21
@note
**************************************************************************/
int GetPerSatObsDataIndex(OBS_DATA_SAT_T* pSatObsData, unsigned int ExSystemID, unsigned int SatID,
										unsigned int SignalType, unsigned int* pSatIndex, unsigned int* pObsIndex)
{
	unsigned int SatIndex, ObsIndex;
	OBS_DATA_PER_SAT_T* pPerSatObsData;

	if (!pSatObsData)
	{
		return -1;
	}

	if (pSatObsData->SatNum == 0)
	{
		*pSatIndex = 0;
		*pObsIndex = 0;
		return 0;
	}

	for (SatIndex = 0; SatIndex < pSatObsData->SatNum; SatIndex++)
	{
		pPerSatObsData = &pSatObsData->ObsDataPerSat[SatIndex];
		if ((ExSystemID == pPerSatObsData->SystemID) && (SatID == pPerSatObsData->SatID))
		{
			for (ObsIndex = 0; ObsIndex < pPerSatObsData->ObsNum; ObsIndex++)
			{
				if (SignalType == GET_CH_TRACK_STATUS_SIGNALTYPE(pPerSatObsData->ObsData[ObsIndex].ChnTrackStatus))
				{
					*pSatIndex = SatIndex;
					*pObsIndex = ObsIndex;
					return 0;
				}
			}

			if ((ObsIndex == pPerSatObsData->ObsNum) && (ObsIndex < MAX_FREQ_INDEX_NUM))
			{
				*pSatIndex = SatIndex;
				*pObsIndex = ObsIndex;
				return 0;
			}
		}
	}

	if ((SatIndex == pSatObsData->SatNum) && (SatIndex < OBS_DATA_SAT_NUM_MAX))
	{
		*pSatIndex = SatIndex;
		*pObsIndex = 0;
		return 0;
	}

	return -1;
}

/**********************************************************************//**
@brief  satellite obs data satellite number add

@param pSatObsData       [In] satellite obs data

@retval <0 error, =0 success

@author CHC
@date 2023/11/21
@note
**************************************************************************/
int SatObsDataSatNumAdd(OBS_DATA_SAT_T* pSatObsData)
{
	if (!pSatObsData)
	{
		return -1;
	}

	if (pSatObsData->SatNum < OBS_DATA_SAT_NUM_MAX)
	{
		pSatObsData->SatNum++;
	}

	return 0;
}

/**********************************************************************//**
@brief  per satellite obs data obs number add

@param pPerSatObsData    [In] per satellite obs data
@param ExSystemID        [In] system id
@param SatID             [In] satellite id

@retval <0 error, =0 success

@author CHC
@date 2023/11/21
@note
**************************************************************************/
int PerSatObsDataObsNumAdd(OBS_DATA_PER_SAT_T* pPerSatObsData, unsigned int ExSystemID, unsigned int SatID)
{
	if (!pPerSatObsData)
	{
		return -1;
	}

	pPerSatObsData->SystemID = ExSystemID;
	pPerSatObsData->SatID = SatID;
	if (pPerSatObsData->ObsNum < MAX_FREQ_INDEX_NUM)
	{
		pPerSatObsData->ObsNum++;
	}

	return 0;
}

/**********************************************************************//**
@brief  satellite obs data to gnss obs data

@param pSatObsData    [In] satellite obs data
@param pObsData       [In] obs data

@retval <0 error, =0 success

@author CHC
@date 2023/11/21
@note
**************************************************************************/
int SatObsDataToGNSSObsData(OBS_DATA_SAT_T* pSatObsData, GNSS_OBS_T* pObsData)
{
	unsigned int SatIndex, PserSatObsIndex;
	unsigned int ObsIndex = 0;
	OBS_DATA_PER_SAT_T* pPerSatObsData;

	if ((!pSatObsData) || (!pObsData))
	{
		return -1;
	}

	for (SatIndex = 0; SatIndex < pSatObsData->SatNum; SatIndex++)
	{
		pPerSatObsData = &pSatObsData->ObsDataPerSat[SatIndex];
		for (PserSatObsIndex = 0; PserSatObsIndex < pPerSatObsData->ObsNum; PserSatObsIndex++)
		{
			pObsData->RangeData.ObsData[ObsIndex] = pPerSatObsData->ObsData[PserSatObsIndex];
			pObsData->RangeData.ObsNum++;
			ObsIndex++;
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  UTC Time to range time

@param pObsData       [Out] obs data
@param pUTCTime       [In]  UTC Time

@retval <0 error, =0 success

@author CHC
@date 2023/11/21
@note
**************************************************************************/
int UTCTimeToRangeTime(GNSS_OBS_T* pObsData, UTC_TIME_T* pUTCTime)
{
	int Week = 0;
	int WeekMsCount, GPSLeapSecMs;
	RM_HEADER_T* pLogHeader;

	if ((!pObsData) || (!pUTCTime))
	{
		return -1;
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(pUTCTime);
	UTCToGPSTime(pUTCTime, &GPSLeapSecMs, &Week, &WeekMsCount);

	pLogHeader = &pObsData->RangeData.LogHeader;

	pLogHeader->TimeStatus = RM_TIME_STATUS_FINE;
	pLogHeader->Week = Week;
	pLogHeader->Milliseconds = WeekMsCount;

	return 0;
}

/**********************************************************************//**
@brief  satellite number to msm satellite index

@param RMRangeSystem    [In] RawMsg Range Satellite system
@param PRN              [In] satellite number

@retval msm satellite index

@author CHC
@date 2023/05/17
@note
**************************************************************************/
unsigned int PRNToMSMSatID(int RMRangeSystem, unsigned short PRN)
{
	const static unsigned char s_RMMinPRNMap[RM_RANGE_SYS_OTHER] =
	{
		MIN_GPS_RANGE_PRN,
		MIN_GLONASS_RANGE_PRN,
		MIN_SBAS_GEO_RANGE_PRN,
		MIN_GALILEO_RANGE_PRN,
		MIN_BDS_RANGE_PRN,
		MIN_QZSS_RANGE_PRN,
		MIN_NAVIC_RANGE_PRN,
	};

	if ((RMRangeSystem < RM_RANGE_SYS_OTHER) && (RMRangeSystem != RM_RANGE_SYS_SBA))
	{
		return (PRN - s_RMMinPRNMap[RMRangeSystem] + 1);
	}
	else if (RMRangeSystem == RM_RANGE_SYS_SBA)
	{
		if ((PRN >= MIN_SBAS_GEO_RANGE_PRN) && (PRN <= MAX_SBAS_GEO_RANGE_PRN))
		{
			return (PRN - MIN_SBAS_GEO_RANGE_PRN + 1);
		}
		else if ((PRN >= MIN_QZSBAS_RANGE_PRN) && (PRN <= MAX_QZSBAS_RANGE_PRN))
		{
			return (PRN - MIN_QZSBAS_RANGE_PRN + 1 + MAX_SBAS_GEO_SAT_NUM);
		}
		else
		{
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  get RTCM3 MSM msg num base

@param RTCMSatSys           [In] RTCM MSM Satellite system

@retval 0xffffffff:error, other:msm msg num base

@author CHC
@date 2023/05/30
@note
**************************************************************************/
unsigned int GetRTCM3MSMMsgNumBase(int RTCMSatSys)
{
	const static unsigned int s_MSMMsgNumBaseMap[RTCM_SYS_NUM] =
	{
		1071,
		1081,
		1091,
		1101,
		1111,
		1121,
		1131,
	};

	if (RTCMSatSys < RTCM_SYS_NUM)
	{
		return s_MSMMsgNumBaseMap[RTCMSatSys];
	}

	return NA32;
}

/**********************************************************************//**
@brief  raw message signal to RTCM3 signal

@param RMRangeSystem    [In] RawMsg Range Satellite system
@param RMRangeSignal    [In] raw message signal type
@param pFreqIndex       [InOut] freq index
@param BDSExpandFlag    [In] 0: standard BDS signal, other:Expand BDS signal

@retval RTCM3 signal(0:error)

@author CHC
@date 2023/05/17
@note
**************************************************************************/
unsigned int RMRangeSignalToRTCM3Signal(int RMRangeSystem, unsigned int RMRangeSignal, unsigned int* pFreqIndex, unsigned int BDSExpandFlag)
{
	const static unsigned char RTCM3SignalMap[RM_RANGE_SYS_OTHER][32] = {
		[RM_RANGE_SYS_GPS] = {
			RTCM3_SIGNAL_GPS_L1CA,      RTCM3_SIGNAL_GPS_L2CA,      0,                          0,
			RTCM3_SIGNAL_GPS_L1P,       RTCM3_SIGNAL_GPS_L2P,       RTCM3_SIGNAL_GPS_L5I,       0,
			RTCM3_SIGNAL_GPS_L1Z,       RTCM3_SIGNAL_GPS_L2Z,       RTCM3_SIGNAL_GPS_L5X,       0,
			RTCM3_SIGNAL_GPS_L1CD,      RTCM3_SIGNAL_GPS_L2CL,      RTCM3_SIGNAL_GPS_L5Q,       0,
			RTCM3_SIGNAL_GPS_L1CP,      RTCM3_SIGNAL_GPS_L2CM,      0,                          0,
			RTCM3_SIGNAL_GPS_L1CX,      RTCM3_SIGNAL_GPS_L2CX,      0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
		},
		[RM_RANGE_SYS_GLO] = {
			RTCM3_SIGNAL_GLO_G1CA,      RTCM3_SIGNAL_GLO_G2CA,      RTCM3_SIGNAL_GLO_G3I,       0,
			RTCM3_SIGNAL_GLO_G1P,       RTCM3_SIGNAL_GLO_G2P,       RTCM3_SIGNAL_GLO_G3Q,       0,
			0,                          0,                          RTCM3_SIGNAL_GLO_G3X,       0,
			RTCM3_SIGNAL_GLO_G1AD,      RTCM3_SIGNAL_GLO_G2AD,      0,                          0,
			RTCM3_SIGNAL_GLO_G1AP,      RTCM3_SIGNAL_GLO_G2AP,      0,                          0,
			RTCM3_SIGNAL_GLO_G1AX,      RTCM3_SIGNAL_GLO_G2AX,      0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
		},
		[RM_RANGE_SYS_SBA] = {
			RTCM3_SIGNAL_SBA_L1CA,      0,                          0,                          0,
			0,                          0,                          RTCM3_SIGNAL_SBA_L5I,       0,
			0,                          0,                          RTCM3_SIGNAL_SBA_L5X,       0,
			0,                          0,                          RTCM3_SIGNAL_SBA_L5Q,       0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
		},
		[RM_RANGE_SYS_GAL] = {
			RTCM3_SIGNAL_GAL_E1A,       RTCM3_SIGNAL_GAL_E1B,       RTCM3_SIGNAL_GAL_E1C,       RTCM3_SIGNAL_GAL_E1X,
			RTCM3_SIGNAL_GAL_E1Z,       RTCM3_SIGNAL_GAL_E6A,       RTCM3_SIGNAL_GAL_E6B,       RTCM3_SIGNAL_GAL_E6C,
			RTCM3_SIGNAL_GAL_E6X,       RTCM3_SIGNAL_GAL_E6Z,       0,                          RTCM3_SIGNAL_GAL_E5AI,
			RTCM3_SIGNAL_GAL_E5AQ,      RTCM3_SIGNAL_GAL_E5AX,      0,                          0,
			RTCM3_SIGNAL_GAL_E5BI,      RTCM3_SIGNAL_GAL_E5BQ,      RTCM3_SIGNAL_GAL_E5BX,      RTCM3_SIGNAL_GAL_E5I,
			RTCM3_SIGNAL_GAL_E5Q,       RTCM3_SIGNAL_GAL_E5X,       0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
		},
		[RM_RANGE_SYS_BDS] = {
			RTCM3_SIGNAL_BDS_B1I,       RTCM3_SIGNAL_BDS_B2I,       RTCM3_SIGNAL_BDS_B3I,       0,
			RTCM3_SIGNAL_BDS_B1I,       RTCM3_SIGNAL_BDS_B2I,       RTCM3_SIGNAL_BDS_B3I,       RTCM3_SIGNAL_BDS_B1CP,
			RTCM3_SIGNAL_BDS_B2BP,      RTCM3_SIGNAL_BDS_B2AP,      RTCM3_SIGNAL_BDS_B1CD,      RTCM3_SIGNAL_BDS_B2BD,
			RTCM3_SIGNAL_BDS_B2AD,      RTCM3_SIGNAL_BDS_B1CX,      RTCM3_SIGNAL_BDS_B2BX,      RTCM3_SIGNAL_BDS_B2AX,
			0,                          0,                          0,       					0,
			RTCM3_SIGNAL_BDS_B1Q,       RTCM3_SIGNAL_BDS_B2Q,       RTCM3_SIGNAL_BDS_B3Q,    	RTCM3_SIGNAL_BDS_B1AD,
			RTCM3_SIGNAL_BDS_B1AP,      RTCM3_SIGNAL_BDS_B1AX,    	RTCM3_SIGNAL_BDS_B3AD,    	RTCM3_SIGNAL_BDS_B3AP,
			RTCM3_SIGNAL_BDS_B3AX,      RTCM3_SIGNAL_BDS_B3AED,     RTCM3_SIGNAL_BDS_B3AEP,     RTCM3_SIGNAL_BDS_B3AEX,
		},
		[RM_RANGE_SYS_QZS] = {
			RTCM3_SIGNAL_QZS_L1CA,      0,                          0,                          0,
			0,                          0,                          RTCM3_SIGNAL_QZS_L5I,       0,
			0,                          0,                          RTCM3_SIGNAL_QZS_L5X,       0,
			RTCM3_SIGNAL_QZS_L1CD,      RTCM3_SIGNAL_QZS_L2CL,      RTCM3_SIGNAL_QZS_L5Q,       0,
			RTCM3_SIGNAL_QZS_L1CP,      RTCM3_SIGNAL_QZS_L2CM,      0,                          0,
			RTCM3_SIGNAL_QZS_L1CX,      RTCM3_SIGNAL_QZS_L2CX,      0,                          0,
			0,                          0,                          RTCM3_SIGNAL_QZS_L6E,       RTCM3_SIGNAL_QZS_L6D,
			RTCM3_SIGNAL_QZS_L6X,       0,                          0,                          0,
		},
		[RM_RANGE_SYS_NIC] = {
			RTCM3_SIGNAL_NIC_L5A,       0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
			0,                          0,                          0,                          0,
		},
	};

	const static unsigned char RTCM3ExpandSignalBDSMap[32] =
	{
		RTCM3_SIGNAL_BDS_B1I,		RTCM3_SIGNAL_BDS_B2I,		RTCM3_SIGNAL_BDS_B3I,		0,
		RTCM3_SIGNAL_BDS_B1I,		RTCM3_SIGNAL_BDS_B2I,		RTCM3_SIGNAL_BDS_B3I,		RTCM3_SIGNAL_BDS_B1AP,
		RTCM3_SIGNAL_BDS_B2BP,		RTCM3_SIGNAL_BDS_B3AEP,		RTCM3_SIGNAL_BDS_B1AD,		RTCM3_SIGNAL_BDS_B2BD,
		RTCM3_SIGNAL_BDS_B3AED,		RTCM3_SIGNAL_BDS_B1AX,		RTCM3_SIGNAL_BDS_B2BX,		RTCM3_SIGNAL_BDS_B3AEX,
		0,							0,							0,							0,
		RTCM3_SIGNAL_BDS_B1Q,		RTCM3_SIGNAL_BDS_B2Q,		RTCM3_SIGNAL_BDS_B3Q,		RTCM3_SIGNAL_BDS_B1AD,
		RTCM3_SIGNAL_BDS_B1AP,		RTCM3_SIGNAL_BDS_B1AX,		RTCM3_SIGNAL_BDS_B3AD,		RTCM3_SIGNAL_BDS_B3AP,
		RTCM3_SIGNAL_BDS_B3AX,		RTCM3_SIGNAL_BDS_B3AED, 	RTCM3_SIGNAL_BDS_B3AEP, 	RTCM3_SIGNAL_BDS_B3AEX,
	};

	const static unsigned char RTCM3FreqMap[RM_RANGE_SYS_OTHER][32] = {
		[RM_RANGE_SYS_GPS] = {
			L1,     L2,     0xFF,   0xFF,   L1,     L2,     L5,     0xFF,
			L1,     L2,     L5,     0xFF,   L1,     L2,     L5,     0xFF,
			L1,     L2,     0xFF,   0xFF,   L1,     L2,     0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_GLO] = {
			G1,     G2,     G3,     0xFF,   G1,     G2,     G3,     0xFF,
			0xFF,   0xFF,   G3,     0xFF,   G4,     G6,     0xFF,   0xFF,
			G4,     G6,     0xFF,   0xFF,   G4,     G6,     0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_SBA] = {
			L1,     0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   L5,     0xFF,
			0xFF,   0xFF,   L5,     0xFF,   0xFF,   0xFF,   L5,     0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_GAL] = {
			E1,     E1,     E1,     E1,     E1,     E6,     E6,     E6,
			E6,     E6,     0xFF,   E5A,    E5A,    E5A,    0xFF,   0xFF,
			E5B,    E5B,    E5B,    E5,     E5,     E5,     0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_BDS] = {
			B1,     B2,     B3,     0xFF,   B1,     B2,     B3,     B4,
			B2,     B5,     B4,     B2,     B5,     B4,     B2,     B5,
			0xFF,   0xFF,   0xFF,   0xFF,   B1,     B2,     B3,     0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_QZS] = {
			L1,     0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   L5,     0xFF,
			0xFF,   0xFF,   L5,     0xFF,   L1,     L2,     L5,     0xFF,
			L1,     L2,     0xFF,   0xFF,   L1,     L2,     0xFF,   0xFF,
			0xFF,   0xFF,   L6,     L6,     L6,     0xFF,   0xFF,   0xFF,
		},
		[RM_RANGE_SYS_NIC] = {
			I5,     0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
			0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
		},
	};

	const static unsigned char RTCM3ExpandFreqBDSMap[32] =
	{
		B1,     B2,     B3,     0xFF,   B1,     B2,     B3,     B4,
		0xFF,   B5,     B4,     B2,     B5,     B4,     0xFF,   B5,
		0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   B1,     B2,
		B3,     0xFF,   B1,     B2,     B3,     0xFF,   0xFF,   0xFF,
	};

	if ((RMRangeSystem < RM_RANGE_SYS_OTHER) && (RMRangeSignal < 32))
	{
		if ((BDSExpandFlag != 0) && (RMRangeSystem == RM_RANGE_SYS_BDS))
		{
			if (RTCM3ExpandSignalBDSMap[RMRangeSignal] != 0)
			{
				if (pFreqIndex)
				{
					*pFreqIndex = RTCM3ExpandFreqBDSMap[RMRangeSignal];
				}

				return RTCM3ExpandSignalBDSMap[RMRangeSignal];
			}
		}
		else
		{
			if (RTCM3SignalMap[RMRangeSystem][RMRangeSignal] != 0)
			{
				if (pFreqIndex)
				{
					*pFreqIndex = RTCM3FreqMap[RMRangeSystem][RMRangeSignal];
				}

				return RTCM3SignalMap[RMRangeSystem][RMRangeSignal];
			}
		}

	}

	return 0;
}

/**********************************************************************//**
@brief translate rtcm msm message system to raw message system

@param RTCMMSMSystem [In] rtcm msm message system

@return RMRangeSystem

@author CHC
@date 2022/05/30
@note
**************************************************************************/
unsigned int RTCMMSMSystemToRMRangeSystem(int RTCMMSMSystem)
{
	const static unsigned int s_RMRangeSystemMap[RTCM_SYS_NUM] =
	{
		RM_RANGE_SYS_GPS,
		RM_RANGE_SYS_GLO,
		RM_RANGE_SYS_GAL,
		RM_RANGE_SYS_SBA,
		RM_RANGE_SYS_QZS,
		RM_RANGE_SYS_BDS,
		RM_RANGE_SYS_NIC,
	};

	if (RTCMMSMSystem < RTCM_SYS_NUM)
	{
		return s_RMRangeSystemMap[RTCMMSMSystem];
	}

	return NA32;
}

/**********************************************************************//**
@brief translate raw message system to rtcm msm message system

@param RMSystem [In] raw message system

@return RMRangeSystem

@author CHC
@date 2022/11/28
@note
**************************************************************************/
unsigned int RMRangeSystemToRTCMMSMSystem(int RMSystem)
{
	const static unsigned int s_RTCMSystemMap[RM_RANGE_SYS_OTHER] =
	{
		RTCM_SYS_GPS,
		RTCM_SYS_GLO,
		RTCM_SYS_SBA,
		RTCM_SYS_GAL,
		RTCM_SYS_BDS,
		RTCM_SYS_QZS,
		RTCM_SYS_NIC,
	};

	if (RMSystem < RM_RANGE_SYS_OTHER)
	{
		return s_RTCMSystemMap[RMSystem];
	}

	return NA32;
}

/**********************************************************************//**
@brief translate raw message system to internal used system ID

@param RMRangeSystem [In] raw message system

@return ExSystemID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
unsigned int RMRangeSystemToExSystem(int RMRangeSystem)
{
	const static unsigned int s_ExSystemMap[RM_RANGE_SYS_OTHER] =
	{
		GPS,
		GLO,
		SBA,
		GAL,
		BDS,
		QZS,
		NIC,
	};

	if (RMRangeSystem < RM_RANGE_SYS_OTHER)
	{
		return s_ExSystemMap[RMRangeSystem];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate internal used system ID to raw message system

@param ExSystemID [In] internal used system ID


@return RMRangeSystem

@author CHC
@date 2022/05/16
@note
**************************************************************************/
unsigned int ExSystemToRMRangeSystem(int ExSystemID)
{
	const static unsigned int s_RMRangeSystemMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RM_RANGE_SYS_GPS,
		RM_RANGE_SYS_GLO,
		RM_RANGE_SYS_GAL,
		RM_RANGE_SYS_BDS,
		RM_RANGE_SYS_SBA,
		RM_RANGE_SYS_QZS,
		RM_RANGE_SYS_NIC,
	};

	if (ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RMRangeSystemMap[ExSystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief  msm satellite id to PRN

@param ExSystemID       [In] internal used system ID
@param SatID            [In] msm satellite id

@retval PRN

@author CHC
@date 2024/05/17
@note
**************************************************************************/
unsigned int MSMSatIDToPRN(int ExSystemID, unsigned short SatID)
{
	const static unsigned char s_RMMinPRNMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		1,
		MIN_GLONASS_RANGE_PRN,
		1,
		1,
		MIN_SBAS_GEO_RANGE_PRN,
		MIN_QZSS_RANGE_PRN,
		1,
	};

	if ((ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM) && (ExSystemID != SBA))
	{
		return (SatID + s_RMMinPRNMap[ExSystemID] - 1);
	}
	else if (ExSystemID == SBA)
	{
		if (SatID <= MAX_SBAS_GEO_SAT_NUM)
		{
			return (SatID + s_RMMinPRNMap[SBA] - 1);
		}
		else
		{
			return (SatID + MIN_QZSBAS_RANGE_PRN - MAX_SBAS_GEO_SAT_NUM - 1);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 signal to raw message signal

@param ExSystemID       [In] internal used system ID
@param RTCM3SignalIndex [In] RTCM3 signal index
@param pFreqIndex       [InOut] freq index
@param BDSExpandFlag    [In] 0: standard BDS signal, other:Expand BDS signal

@retval raw message signal

@author CHC
@date 2023/06/14
@note
**************************************************************************/
unsigned int RTCM3SignalToRMRangeSignalByExSystem(int ExSystemID, unsigned int RTCM3SignalIndex, unsigned int* pFreqIndex, unsigned int BDSExpandFlag)
{
	const static int SignalMap[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SIGN_NUM] = {
		[GPS] = {
			NA8,                        RM_RANGE_SIGNAL_GPSL1CA,            RM_RANGE_SIGNAL_GPSL1P,     RM_RANGE_SIGNAL_GPSL1W,
			NA8,                        NA8,                                NA8,                        RM_RANGE_SIGNAL_GPSL2CA,
			RM_RANGE_SIGNAL_GPSL2P,     RM_RANGE_SIGNAL_GPSL2PCODELESS,     NA8,                        NA8,
			NA8,                        NA8,                                RM_RANGE_SIGNAL_GPSL2CM,    RM_RANGE_SIGNAL_GPSL2CL,
			RM_RANGE_SIGNAL_GPSL2CX,    NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_GPSL5I,             RM_RANGE_SIGNAL_GPSL5Q,     RM_RANGE_SIGNAL_GPSL5X,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_GPSL1CD,            RM_RANGE_SIGNAL_GPSL1CP,    RM_RANGE_SIGNAL_GPSL1CX,
		},
		[GLO] = {
			NA8,                        RM_RANGE_SIGNAL_GLOL1C,             RM_RANGE_SIGNAL_GLOL1P,     NA8,
			NA8,                        NA8,                                NA8,                        RM_RANGE_SIGNAL_GLOL2C,
			RM_RANGE_SIGNAL_GLOL2P,     NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                RM_RANGE_SIGNAL_GLOL2CSI,   RM_RANGE_SIGNAL_GLOL2OCP,
			RM_RANGE_SIGNAL_GLOL2OCX, NA8,                                  NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_GLOL3I,             RM_RANGE_SIGNAL_GLOL3Q,     RM_RANGE_SIGNAL_GLOL3X,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_GLOL1OCD,           RM_RANGE_SIGNAL_GLOL1OCP, RM_RANGE_SIGNAL_GLOL1OCX,
		},
		[GAL] = {
			NA8,                        RM_RANGE_SIGNAL_GALE1C,             RM_RANGE_SIGNAL_GALE1A,     RM_RANGE_SIGNAL_GALE1B,
			RM_RANGE_SIGNAL_GALE1X,     RM_RANGE_SIGNAL_GALE1Z,             NA8,                        RM_RANGE_SIGNAL_GALE6C,
			RM_RANGE_SIGNAL_GALE6A,     RM_RANGE_SIGNAL_GALE6B,             RM_RANGE_SIGNAL_GALE6X,     RM_RANGE_SIGNAL_GALE6Z,
			NA8,                        RM_RANGE_SIGNAL_GALE5BI,            RM_RANGE_SIGNAL_GALE5BQ,    RM_RANGE_SIGNAL_GALE5BX,
			NA8,                        RM_RANGE_SIGNAL_GALBOCI,            RM_RANGE_SIGNAL_GALBOCQ,    RM_RANGE_SIGNAL_GALBOCX,
			NA8,                        RM_RANGE_SIGNAL_GALE5AI,            RM_RANGE_SIGNAL_GALE5AQ,    RM_RANGE_SIGNAL_GALE5AX,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
		},
		[BDS] = {
			NA8,                        RM_RANGE_SIGNAL_BDSB1ID1,           RM_RANGE_SIGNAL_BDSB1Q,     NA8,
			NA8,                        NA8,                                NA8,                        RM_RANGE_SIGNAL_BDSB3ID1,
			RM_RANGE_SIGNAL_BDSB3Q,     NA8,             					NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_BDSB2ID1,           RM_RANGE_SIGNAL_BDSB2Q,     NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_BDSB2AD,            RM_RANGE_SIGNAL_BDSB2AP,    RM_RANGE_SIGNAL_BDSB2AX,
			RM_RANGE_SIGNAL_BDSB2BD,    RM_RANGE_SIGNAL_BDSB2BP,            RM_RANGE_SIGNAL_BDSB2BX,    NA8,
			NA8,                        RM_RANGE_SIGNAL_BDSB1CD,            RM_RANGE_SIGNAL_BDSB1CP,    RM_RANGE_SIGNAL_BDSB1CX,
		},
		[SBA] = {
			NA8,                        RM_RANGE_SIGNAL_SBASL1C,            NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_SBASL5I,            RM_RANGE_SIGNAL_SBASL5Q,    RM_RANGE_SIGNAL_SBASL5X,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
		},
		[QZS] = {
			NA8,                        RM_RANGE_SIGNAL_QZSSL1CA,           NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			RM_RANGE_SIGNAL_QZSSL6D,    RM_RANGE_SIGNAL_QZSSL6P,            RM_RANGE_SIGNAL_QZSSL6X,    RM_RANGE_SIGNAL_QZSSL6E,
			RM_RANGE_SIGNAL_QZSSL6Z,    NA8,                                RM_RANGE_SIGNAL_QZSSL2CM,   RM_RANGE_SIGNAL_QZSSL2CL,
			RM_RANGE_SIGNAL_QZSSL2CX,   NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_QZSSL5I,            RM_RANGE_SIGNAL_QZSSL5Q,    RM_RANGE_SIGNAL_QZSSL5X,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_QZSSL1CD,           RM_RANGE_SIGNAL_QZSSL1CP,   RM_RANGE_SIGNAL_QZSSL1CX,
		},
		[NIC] = {
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        RM_RANGE_SIGNAL_NICL5A,             NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
			NA8,                        NA8,                                NA8,                        NA8,
		},
	};

	const static int ExpandSignalBDSMap[RTCM_MAX_SIGN_NUM] =
	{
		NA8,                        RM_RANGE_SIGNAL_BDSB1ID1,           RM_RANGE_SIGNAL_BDSB1Q,     NA8,
		RM_RANGE_SIGNAL_BDSB1CD,    RM_RANGE_SIGNAL_BDSB1CP,            RM_RANGE_SIGNAL_BDSB1CX,    RM_RANGE_SIGNAL_BDSB3ID1,
		RM_RANGE_SIGNAL_BDSB3Q,     NA8,             					NA8,                        NA8,
		NA8,                        RM_RANGE_SIGNAL_BDSB2ID1,           RM_RANGE_SIGNAL_BDSB2Q,     NA8,
		RM_RANGE_SIGNAL_BDSB2AD,    RM_RANGE_SIGNAL_BDSB2AP,            RM_RANGE_SIGNAL_BDSB2AX,    NA8,
		NA8,                        NA8,                                NA8,                        NA8,
		RM_RANGE_SIGNAL_BDSB2BD,    RM_RANGE_SIGNAL_BDSB2BP,            RM_RANGE_SIGNAL_BDSB2BX,    NA8,
		NA8,                        NA8,                                NA8,                        NA8,
	};

	const static int FreqMap[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SIGN_NUM] = {
		[GPS] = {
			NA8,    L1,     L1,     L1,     NA8,    NA8,    NA8,    L2,
			L2,     L2,     NA8,    NA8,    NA8,    NA8,    L2,     L2,
			L2,     NA8,    NA8,    NA8,    NA8,    L5,     L5,     L5,
			NA8,    NA8,    NA8,    NA8,    NA8,    L1,     L1,     L1,
		},
		[GLO] = {
			NA8,    G1,     G1,     NA8,    NA8,    NA8,    NA8,    G2,
			G2,     NA8,    NA8,    NA8,    NA8,    NA8,    G6,     G6,
			G6,     NA8,    NA8,    NA8,    NA8,    G3,     G3,     G3,
			NA8,    NA8,    NA8,    NA8,    NA8,    G4,     G4,     G4,
		},
		[GAL] = {
			NA8,    E1,     E1,     E1,     E1,     E1,     NA8,    E6,
			E6,     E6,     E6,     E6,     NA8,    E5B,    E5B,    E5B,
			NA8,    E5,     E5,     E5,     NA8,    E5A,    E5A,    E5A,
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
		},
		[BDS] = {
			NA8,    B1,     B1,     B1,     NA8,    NA8,    NA8,    B3,
			B3,     B3,     NA8,    NA8,    NA8,    B2,     B2,     B2,
			NA8,    NA8,    NA8,    NA8,    NA8,    B5,     B5,     B5,
			B2,     NA8,    NA8,    NA8,    NA8,    B4,     B4,     B4,
		},
		[SBA] = {
			NA8,    L1,     NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
			NA8,    NA8,    NA8,    NA8,    NA8,    L5,     L5,     L5,
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
		},
		[QZS] = {
			NA8,    L1,     NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
			L6,     L6,     L6,     L6,     L6,     NA8,    L2,     L2,
			L2,     NA8,    NA8,    NA8,    NA8,    L5,     L5,     L5,
			NA8,    NA8,    NA8,    NA8,    NA8,    L1,     L1,     L1,
		},
		[NIC] = {
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
			NA8,    NA8,    NA8,    NA8,    NA8,    I5,     NA8,    NA8,
			NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
		},
	};

	const static int ExpandFreqBDSMap[RTCM_MAX_SIGN_NUM] =
	{
		NA8,    B1,     B1,     B1,     B4,     B4,     B4,    B3,
		B3,     B3,     NA8,    NA8,    NA8,    B2,     B2,     B2,
		B5,     B5,     B5,     NA8,    NA8,    NA8,    NA8,    NA8,
		B2,     NA8,    NA8,    NA8,    NA8,    NA8,    NA8,    NA8,
	};

	if ((ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM) && (RTCM3SignalIndex < RTCM_MAX_SIGN_NUM))
	{
		if ((BDSExpandFlag != 0) && (ExSystemID == BDS))
		{
			if (ExpandSignalBDSMap[RTCM3SignalIndex] != NA8)
			{
				if (pFreqIndex)
				{
					*pFreqIndex = ExpandFreqBDSMap[RTCM3SignalIndex];
				}

				return ExpandSignalBDSMap[RTCM3SignalIndex];
			}
		}
		else
		{
			if (SignalMap[ExSystemID][RTCM3SignalIndex] != NA8)
			{
				if (pFreqIndex)
				{
					*pFreqIndex = FreqMap[ExSystemID][RTCM3SignalIndex];
				}

				return SignalMap[ExSystemID][RTCM3SignalIndex];
			}
		}
	}

	return NA8;
}

/**********************************************************************//**
@brief  get Phase Alignment Data

@param ExSystemID       [In] internal used system ID
@param RMSignalID       [In] RawMsg signal ID

@retval Phase Alignment Data

@author CHC
@date 2024/05/27
@note
**************************************************************************/
double GetPhaseAlignmentDataCycle(int ExSystemID, int RMSignalID)
{
	const static double s_PhaseAlignmentDataBuf[TOTAL_EX_GNSS_SYSTEM_NUM][32] =
	{
		[GPS] = {
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		-0.25,	-0.25,	0, /**< Non-zero: L2CL, L5Q */
			0.25,	-0.25,	0,		0,		0,		-0.25,	0,		0, /**< Non-zero: L1CP, L2CM, L2CX */
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[GLO] = {
			0,		0,		0,		0,		0,		0.25,	0.25,	0, /**< Non-zero: L2P, L3Q */
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[GAL] = {
			0,		0,		0.5,	0,		0,		0,		0,		-0.5, /**< Non-zero: E1C, E6C */
			0,		0,		0,		0,		-0.25,	0,		0,		0, /**< Non-zero: E5aQ */
			0,		-0.25,	0,		0,		-0.25,	0,		0,		0, /**< Non-zero: E5bQ, E5AltBOC-Q */
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[BDS] = {
			0,		0,		0,		0,		0,		0,		0,		0.25, /**< Non-zero: B1CP */
			0,		0.25,	0,		0,		0,		0,		0,		0, /**< Non-zero: B2aP */
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[SBA] = {
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[QZS] = {
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		-0.25,	0, /**< Non-zero: L5Q */
			0.25,	0,		0,		0,		0,		0,		0,		0, /**< Non-zero: L1CP */
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[NIC] = {
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
	};

	if ((ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM) && (RMSignalID < 32))
	{
		return s_PhaseAlignmentDataBuf[ExSystemID][RMSignalID];
	}

	return 0;
}

/**********************************************************************//**
@brief  get refstation Phase Alignment Data

@param VendorType      [In] refstation vendor type
@param ExSystemID       [In] internal used system ID
@param RMSignalID       [In] RawMsg signal ID

@retval Phase Alignment Data

@author CHC
@date 2024/05/27
@note
**************************************************************************/
double GetRefstationPhaseAlignmentData(int VendorType, int ExSystemID, int RMSignalID)
{
	const static double s_MagellanPhaseAlignmentDataBuf[2][32] =
	{
		[GPS] = {
			0,		0.25,	0,		0,		0.25,	0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[GLO] = {
			0,		0.25,	0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
	};

	const static double s_TrimblePhaseAlignmentDataBuf[2][32] =
	{
		[GPS] = {
			0,		0.25,	0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
		[GLO] = {
			0,		0.25,	0,		0,		0.25,	0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
			0,		0,		0,		0,		0,		0,		0,		0,
		},
	};

	if ((ExSystemID < 2) && (RMSignalID < 32))
	{
		if (VendorType == REFSTATION_VENDOR_MAGELLAN)
		{
			return s_MagellanPhaseAlignmentDataBuf[ExSystemID][RMSignalID];
		}
		else if (VendorType == REFSTATION_VENDOR_TRIMBLE)
		{
			return s_TrimblePhaseAlignmentDataBuf[ExSystemID][RMSignalID];
		}
	}

	return 0;
}

const static char* s_VendorTypeStrMap[REFSTATION_VENDOR_NUM_MAX] =
{
	"UNKNOWN",
	"CHC VRS",
	"SWAS VRS",
	"CHC SAX",
	"CHC",
	"NOV",
	"UNICORE",
	"TRIMBLE",
	"MAGELLAN",
	"QXWZ VRS",
	"SIN",
	"HEMI",
	"LEICA",
	"SWIFT",
	"SEPT",
};

/**********************************************************************//**
@brief  get vendor type String

@param VendorID      [In] receiver Vendor ID

@retval refstation vendor type

@author CHC
@date 2024/12/02
@note
**************************************************************************/
const char * GetVendorTypeString(unsigned int VendorID)
{
	const static char* s_VendorTypeString[REFSTATION_VENDOR_NUM_MAX] = {
			"UNKNOWN",
			"CHC_VRS",
			"SWAS_VRS",
			"CHC_SAX",
			"CHC",
			"NOVATEL",
			"UNICORE",
			"TRIMBLE",
			"MAGELLAN",
			"QXWZ_VRS",
			"SIN",
			"HEMI",
			"LEICA",
			"SWIFT",
			"SEPT",
	};

	if(VendorID < REFSTATION_VENDOR_NUM_MAX)
	{
		return s_VendorTypeString[VendorID];
	}

	return s_VendorTypeString[0]; /**< return unknown */
}


/**********************************************************************//**
@brief  get refstation vendor type

@param pReceiverDescriptor      [In] receiver descriptor

@retval refstation vendor type

@author CHC
@date 2024/05/27
@note
**************************************************************************/
REFSTATION_VENDOR_E GetRefstationVendorType(char* pReceiverDescriptor)
{
	int DataIndex;
	int ReceiverDescriptorStrLen;

	if (!pReceiverDescriptor)
	{
		return 0;
	}
	ReceiverDescriptorStrLen = (int)strlen(pReceiverDescriptor);
	for (DataIndex = 0; DataIndex < REFSTATION_VENDOR_NUM_MAX; DataIndex++)
	{
		int s_VendorTypeStrLen = (int)strlen(s_VendorTypeStrMap[DataIndex]);
		if (ReceiverDescriptorStrLen < s_VendorTypeStrLen)
		{
			continue;
		}

		if (0 == MEMCMP(s_VendorTypeStrMap[DataIndex], pReceiverDescriptor, s_VendorTypeStrLen))
		{
			return DataIndex;
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  Sec to HMS

@param Secs				[In] Seconds
@param pHour			[Out] Hour
@param pMinu			[Out] Minute
@param pSec				[Out] Seconds

@author CHC
@date 2024/04/24
@note
**************************************************************************/
static void SecToHMS(double Secs, int* pHour, int* pMinu, double* pSec)
{
	int TempHour, TempMinu;
	double TempSec;

	TempHour = (int)(Secs / 3600);

	double Sec0 = Secs - (double)TempHour * 3600;
	TempMinu = (int)(Sec0 / 60);
	TempSec = Sec0 - (double)TempMinu * 60;

	*pHour = TempHour;
	*pMinu = TempMinu;
	*pSec = TempSec;
}

/**********************************************************************//**
@brief  EpochTime To GPS Time

@param EpochTime		[In] EpochTime
@param pGPSWeek			[Out] GPS Time week
@param pWeekMsCount		[Out] GPS Time Week Ms Count

@retval <0:error =0:success

@author CHC
@date 2024/04/24
@note
**************************************************************************/
int EpochTimeToGPSTime(int EpochTime, int * pGPSWeek, int * pWeekMsCount)
{
	int Hour, Minu;
	double Sec;
	UTC_TIME_T UTCTime;
	UTC_TIME_T TempUTCTime;
	INT64 TimeDiff;
	int GPSWeek, WeekMsCount, GPSLeapSecMs;

	//int Delay = _sysPre == SYS_GPS ? 20 : 22;
	int Delay = 20;

	/* tod to hms */
	SecToHMS(EpochTime, &Hour, &Minu, &Sec);

	/** get current time */
	UTCTime = RTCMGetUTCTime();
	TempUTCTime = UTCTime;
	TempUTCTime.Hour = Hour;
	TempUTCTime.Minute = Minu;
	TempUTCTime.Second = (int)Sec;
	TempUTCTime.Millisecond = (int)((Sec - TempUTCTime.Second)*1000+0.5);

	TimeDiff = UTCDiff(&UTCTime, &TempUTCTime) / 1000;

	/** check time */
	if (TimeDiff < ((INT64)-Delay * 3600))
	{
		UTCAdd(&TempUTCTime, -86400000);
	}
	else if (TimeDiff > ((INT64)Delay * 3600))
	{
		UTCAdd(&TempUTCTime, 86400000);
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(&TempUTCTime);
	UTCToGPSTime(&TempUTCTime, &GPSLeapSecMs, &GPSWeek, &WeekMsCount);

	*pGPSWeek = GPSWeek;
	*pWeekMsCount = WeekMsCount;

	return 0;
}

/**********************************************************************//**
@brief get the start index of satellites for each GNSS system

@param  System     [In] GNSS system ID

@return the start index of satellites

@author CHC
@date   2024/07/05
@note
**************************************************************************/
unsigned int GetSatIndexStart(unsigned int System)
{
	const static unsigned int s_SatIndexStartMap[TOTAL_EX_GNSS_SYSTEM_NUM] = {
		[GPS] = 0,
		[BDS] = MAX_GPS_SAT_NUM,
		[GAL] = MAX_GPS_SAT_NUM+MAX_BDS_SAT_NUM,
		[GLO] = MAX_GPS_SAT_NUM+MAX_BDS_SAT_NUM+MAX_GAL_SAT_NUM,
		[QZS] = MAX_GPS_SAT_NUM+MAX_BDS_SAT_NUM+MAX_GAL_SAT_NUM+MAX_GLO_SAT_NUM,
		[SBA] = MAX_GPS_SAT_NUM+MAX_BDS_SAT_NUM+MAX_GAL_SAT_NUM+MAX_GLO_SAT_NUM+MAX_QZSS_SAT_NUM,
		[NIC] = MAX_GPS_SAT_NUM+MAX_BDS_SAT_NUM+MAX_GAL_SAT_NUM+MAX_GLO_SAT_NUM+MAX_QZSS_SAT_NUM+MAX_SBAS_ALL_SAT_NUM,
	};

	if(System >= TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return 0;
	}

	return s_SatIndexStartMap[System];
}


#ifdef RTCM_DEBUG_INFO
/**********************************************************************//**
@brief  get RTCM3 signal name

@param RTCMSys            [In] RTCM3 system id
@param RTCMSignalID       [In] RTCM3 signal id

@retval RTCM3 signal name

@author CHC
@date 2023/11/21
@note
**************************************************************************/
const char* GetRTCM3SignalName(unsigned int RTCMSys, unsigned int RTCMSignalID)
{
	const char* s_RTCM3SystemSignalNameTBL[RTCM_SYS_NUM][RTCM_MAX_SIGN_NUM] =
	{
		{
			"????",
			"GPSL1CA",
			"GPSL1P",
			"GPSL1Z",
			"????",
			"????",
			"????",
			"GPSL2CA",
			"GPSL2P",
			"GPSL2Z",
			"????",
			"????",
			"????",
			"????",
			"GPSL2CM",
			"GPSL2CL",
			"GPSL2CX",
			"????",
			"????",
			"????",
			"????",
			"GPSL5I",
			"GPSL5Q",
			"GPSL5X",
			"????",
			"????",
			"????",
			"????",
			"????",
			"GPSL1CD",
			"GPSL1CP",
			"GPSL1CX"
		},
		{
			"????",
			"GLOG1CA",
			"GLOG1P",
			"????",
			"????",
			"????",
			"????",
			"GLOG2CA",
			"GLOG2P",
			"????",
			"????",
			"????",
			"????",
			"????",
			"GLOG2AD",
			"GLOG2AP",
			"GLOG2AX",
			"????",
			"????",
			"????",
			"????",
			"GLOG3I",
			"GLOG3Q",
			"GLOG3X",
			"????",
			"????",
			"????",
			"????",
			"????",
			"GLOG1AD",
			"GLOG1AP",
			"GLOG1AX",
		},
		{
			"????",
			"GALE1C",
			"GALE1A",
			"GALE1B",
			"GALE1X",
			"GALE1Z",
			"????",
			"GALE6C",
			"GALE6A",
			"GALE6B",
			"GALE6X",
			"GALE6Z",
			"????",
			"GALE5BI",
			"GALE5BQ",
			"GALE5BX",
			"????",
			"GALE5I",
			"GALE5Q",
			"GALE5X",
			"????",
			"GALE5AI",
			"GALE5AQ",
			"GALE5AX",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
		},
		{
			"????",
			"SBAL1CA",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"SBAL5I",
			"SBAL5Q",
			"SBAL5X",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
		},
		{
			"????",
			"QZSL1CA",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"QZSL6D",
			"QZSL6E",
			"QZSL6X",
			"????",
			"????",
			"????",
			"QZSL2CM",
			"QZSL2CL",
			"QZSL2CX",
			"????",
			"????",
			"????",
			"????",
			"QZSL5I",
			"QZSL5Q",
			"QZSL5X",
			"????",
			"????",
			"????",
			"????",
			"????",
			"QZSL1CD",
			"QZSL1CP",
			"QZSL1CX",
		},
		{
			"????",
			"BDSB1I",
			"BDSB1Q",
			"BDSB1X",
			"BDSB1AD",
			"BDSB1AP",
			"BDSB1AX",
			"BDSB3I",
			"BDSB3Q",
			"BDSB3X",
			"BDSB3AD",
			"BDSB3AP",
			"BDSB3AX",
			"BDSB2I",
			"BDSB2Q",
			"BDSB2X",
			"BDSB3AED",
			"BDSB3AEP",
			"BDSB3AEX",
			"????",
			"????",
			"BDSB2AD",
			"BDSB2AP",
			"BDSB2AX",
			"BDSB2BD",
			"BDSB2BP",
			"BDSB2BX",
			"????",
			"????",
			"BDSB1CD",
			"BDSB1CP",
			"BDSB1CX",
		},
		{
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"NICL5A",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
			"????",
		},
	};

	if ((RTCMSys < RTCM_SYS_NUM) && ((RTCMSignalID != 0) && (RTCMSignalID <= RTCM_MAX_SIGN_NUM)))
	{
		return s_RTCM3SystemSignalNameTBL[RTCMSys][RTCMSignalID - 1];
	}
	return "????";
}

/**********************************************************************//**
@brief translate internal used system ID to RTCM system

@param ExSystemID [In] internal used system ID


@return RMRangeSystem

@author CHC
@date 2022/05/16
@note
**************************************************************************/
unsigned int ExSystemToRTCMSystem(int ExSystemID)
{
	const static unsigned int s_RTCMSystemMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RTCM_SYS_GPS,
		RTCM_SYS_GLO,
		RTCM_SYS_GAL,
		RTCM_SYS_BDS,
		RTCM_SYS_SBA,
		RTCM_SYS_QZS,
		RTCM_SYS_NIC,
	};

	if (ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RTCMSystemMap[ExSystemID];
	}

	return NA8;
}

#endif
