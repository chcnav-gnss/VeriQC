#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RINEXConvert.c
@author CHC
@date   2023/06/29
@brief

**************************************************************************/
#include "OSDependent.h"
#include "RINEXConvert.h"
#include "../../MsgConvert.h"
#include "GNSSPrint.h"
#include "Common/SupportFuncs.h"
#include "../../MsgConvertDataTypes.h"

#include "Encoder/RINEX/RINEXDefines.h"
#include "Encoder/RINEX/RINEXEncoder.h"

#include "Decoder/RawMsgAscii/RawMsgDecodeSupportFunc.h"

#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"

#include "BSW/MsgEncoder/RTCM/RTCMEncoderIDDefines.h"
#include "BSW/MsgEncoder/HRCX/HRCXDataType.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgDataStructure.h"

#include "TimeDefines.h"
#include "Coordinate.h"
#include "Common/GNSSNavDataType.h"


#define DTTOL		   5//0.005			/**< tolerance of time difference (s) */

#define MAXDTOE		 7200000				/* max time difference to ephem Toe (s) for GPS */
#define MAXDTOE_GLO	 1800000				/* max time difference to GLONASS Toe (s) */
#define MAXDTOE_SBS	 360.0				  /* max time difference to SBAS Toe (s) */
#define MAXDTOE_S	   86400.0				/* max time difference to ephem toe (s) for other */
#define MAXGDOP		 300.0				  /* max GDOP */

#define DOUBLE_IS_EQUAL(dValue1, dValue2)	(fabs(dValue1 - dValue2) < 1E-9)

typedef struct _TIME_LEAPS_T
{
	UTC_TIME_T Time;
	int Leapsecs;
} TIME_LEAPS_T;

const static TIME_LEAPS_T s_TimeLeapsec[] = {
	{.Time = {0,2017,1,1,0,0,0,0,}, .Leapsecs = -18},
	{.Time = {0,2015,7,1,0,0,0,0,}, .Leapsecs = -17},
	{.Time = {0,2012,7,1,0,0,0,0,}, .Leapsecs = -16},
	{.Time = {0,2009,1,1,0,0,0,0,}, .Leapsecs = -15},
	{.Time = {0,2006,1,1,0,0,0,0,}, .Leapsecs = -14},
	{.Time = {0,1999,1,1,0,0,0,0,}, .Leapsecs = -13},
	{.Time = {0,1997,7,1,0,0,0,0,}, .Leapsecs = -12},
	{.Time = {0,1996,1,1,0,0,0,0,}, .Leapsecs = -11},
	{.Time = {0,1994,7,1,0,0,0,0,}, .Leapsecs = -10},
	{.Time = {0,1993,7,1,0,0,0,0,}, .Leapsecs =  -9 },
	{.Time = {0,1992,7,1,0,0,0,0,}, .Leapsecs =  -8 },
	{.Time = {0,1991,1,1,0,0,0,0,}, .Leapsecs =  -7 },
	{.Time = {0,1990,1,1,0,0,0,0,}, .Leapsecs =  -6 },
	{.Time = {0,1988,1,1,0,0,0,0,}, .Leapsecs =  -5 },
	{.Time = {0,1985,7,1,0,0,0,0,}, .Leapsecs =  -4 },
	{.Time = {0,1983,7,1,0,0,0,0,}, .Leapsecs =  -3 },
	{.Time = {0,1982,7,1,0,0,0,0,}, .Leapsecs =  -2 },
	{.Time = {0,1981,7,1,0,0,0,0,}, .Leapsecs =  -1 },
};

static char* s_ObsCodesMap[] = {	   /* observation code strings */

	""  ,"1C","1P","1W","1Y", "1M","1N","1S","1L","1E", /*  0- 9 */
	"1A","1B","1X","1Z","2C", "2D","2S","2L","2X","2P", /* 10-19 */
	"2W","2Y","2M","2N","5I", "5Q","5X","7I","7Q","7X", /* 20-29 */
	"6A","6B","6C","6X","6Z", "6S","6L","8L","8Q","8X", /* 30-39 */
	"2I","2Q","6I","6Q","3I", "3Q","3X","1I","1Q","5D", /* 40-49 */
	"5P","5Z","6E","7D","7P", "7Z","1D","8D","8P","6D", /* 50-59 */
	"6P"/* 60 */
};

static unsigned char s_ObsFreqs[] = { /* 1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,7:L3 */

	0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  /*  0- 9 */
	1, 1, 1, 1, 2,  2, 2, 2, 2, 2,  /* 10-19 */
	2, 2, 2, 2, 3,  3, 3, 5, 5, 5,  /* 20-29 */
	4, 4, 4, 4, 4,  4, 4, 6, 6, 6,  /* 30-39 */
	2, 2, 4, 4, 3,  3, 3, 1, 1, 3,  /* 40-49 */
	3, 3, 4, 5, 5,  5, 1, 6, 6, 4,  /* 50-59 */
	4 /* 60 */
};

/* msm signal id table -------------------------------------------------------*/
const char *MSMSignalTabGPS[32] = {
/* GPS: ref [13] table 3.5-87, ref [14] table 3.5-91 */
	""  ,"1C","1P","1W","1Y","1M",""  ,"2C","2P","2W","2Y","2M", /*  1-12 */
	""  ,""  ,"2S","2L","2X",""  ,""  ,""  ,""  ,"5I","5Q","5X", /* 13-24 */
	""  ,""  ,""  ,""  ,""  ,"1S","1L","1X"						 /* 25-32 */
};

const char *MSMSignalTabGLO[32] = {
/* GLONASS: ref [13] table 3.5-93, ref [14] table 3.5-97 */
	""  ,"1C","1P",""  ,""  ,""  ,""  ,"2C","2P",""  ,"3I","3Q",
	"3X",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};

const char *MSMSignalTabGAL[32] = {
/* Galileo: ref [13] table 3.5-96, ref [14] table 3.5-100 */
	""  ,"1C","1A","1B","1X","1Z",""  ,"6C","6A","6B","6X","6Z",
	""  ,"7I","7Q","7X",""  ,"8I","8Q","8X",""  ,"5I","5Q","5X",
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};

const char *MSMSignalTabQZS[32] = {
/* QZSS: ref [13] table 3.5-T+003 */
	""  ,"1C",""  ,""  ,""  ,"1Z",""  ,""  ,"6S","6L","6X",""  ,
	""  ,""  ,"2S","2L","2X",""  ,""  ,""  ,""  ,"5I","5Q","5X",
	""  ,""  ,""  ,""  ,""  ,"1S","1L","1X"
};

const char *MSMSignalTabSBA[32] = {
/* SBAS: ref [13] table 3.5-T+005 */
	""  ,"1C",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,"5I","5Q","5X",
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};

const char *MSMSignalTabNIC[32] = {
/* NIC: ref [13] table 3.5-T+005 */
	""  ,"",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,"5C" ,"5A","5B","5X",
	""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};

// RTCM3_HRC
const char *MSMSignalTabBDS[32] = {//hrc beidou3 sigal index

	"" , "2I","2Q","2X","1S","1L","1Z","6I","6Q","6X","6D","6P",// 0 ~11
	"6Z","7I","7Q","7X","6M","6N","6O", "" , "" ,"5D","5P","5X",// 12~23
	"7D","7P","7Z", "" , "" ,"1D","1P","1X" // 24~31
};

/**********************************************************************//**
@brief  get leaps by time

@param Time				[In] TIME_T

@retval leap seconds

@author CHC
@date 2023/04/10
@note
**************************************************************************/
static int GetTimeLeaps(UTC_TIME_T * pTime)
{
	unsigned int Index;
	for (Index = 0; Index < (int)sizeof(s_TimeLeapsec) / (int)sizeof(*s_TimeLeapsec); Index++)
	{
		if (UTCDiff(pTime, (UTC_TIME_T *)&s_TimeLeapsec[Index].Time) >= 0)
		{
			return (int)(-s_TimeLeapsec[Index].Leapsecs);
		}
	}
	return 0;
}

/**********************************************************************//**
@brief  get code flag

@param EncodeID	 [In] encode id

@retval code flag

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int GetCodeFlag(unsigned int EncodeID)
{
	int CodeFlag = 0;

	if (EncodeID == RINEX_ID_GPSEPH)
		CodeFlag = 1;
	else if (EncodeID == RINEX_ID_QZSSEPH)
		CodeFlag = 2;
	else if ((EncodeID == RINEX_ID_BDSEPH) || (EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
		CodeFlag = 0;
	else if (EncodeID == RINEX_ID_GALEPH_FNAV)
		CodeFlag = 2;
		//CodeFlag = (1<<1)+(1<<8);
	else if ((EncodeID == RINEX_ID_GALEPH_INAV) || (EncodeID == RINEX_ID_GALEPH_RCED))
		CodeFlag = 1;
		//CodeFlag = (1<<0)+(1<<2)+(1<<9);

	return CodeFlag;
}

/*****************************************************************************//**
@brief adjust weekly rollover of bdt time

@param Week				[In] week
@param LeapSecMs		[In] leap Second in Ms

@return <0:error,=0:success

@author CHC
@date  2023/03/10
@note
*********************************************************************************/
static int AdjBDTimeWeek(int Week)
{
	int TempWeek, WeekMsCount, BDSLeapSecMs;
	UTC_TIME_T TempUtcTime = RTCMGetUTCTime();

	BDSLeapSecMs = GetBDSLeapSecondMs(&TempUtcTime);
	UTCToBDSTime(&TempUtcTime, &BDSLeapSecMs, &TempWeek, &WeekMsCount);

	if (TempWeek < 1) /**< use 2006/1/1 if time is earlier than 2006/1/1 */
	{
		TempWeek = 1;
	}

	return Week + (TempWeek - Week + 512) / 1024 * 1024;
}

/**********************************************************************//**
@brief  screen by time,screening by time start, time end, and time interval

@param Time				[In] TIME_T
@param TimeStart		[In] time start (ts.time==0:no screening by ts)
@param TimeEnd		  [In] time end   (te.time==0:no screening by te)
@param TimeInt		  [In] time interval (s) (0.0:no screen by tint)

@retval leap seconds

@author CHC
@date 2023/04/10
@note
**************************************************************************/
int ScreenTime(UTC_TIME_T * pTime, UTC_TIME_T * pTimeStart, UTC_TIME_T * pTimeEnd, double TimeInt)
{
	return (TimeInt <= 0.0 || fmod((double)(UTCToMillisecond(pTime) + DTTOL), TimeInt*1000) <= (double)DTTOL * 2) &&
		(pTimeStart->Year == 0 || UTCDiff(pTime, pTimeStart) >= -DTTOL) &&
		(pTimeStart->Year == 0 || UTCDiff(pTime, pTimeEnd) < DTTOL);
}

/**********************************************************************//**
@brief obs to code

@param pObsStr	  [In] obs string
@param pFreq		[In] freq

@return <0:error, =0:success

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static int Obs2Code(const char *pObsStr, int* pFreq)
{
	int CharIndex;

	for (CharIndex = 1; CharIndex < (sizeof(s_ObsCodesMap)/sizeof(s_ObsCodesMap[0])); CharIndex++)
	{
		if (strcmp(s_ObsCodesMap[CharIndex], pObsStr))
		{
			continue;
		}

		if (pFreq)
		{
			*pFreq = s_ObsFreqs[CharIndex];
		}

		return (int)CharIndex;
	}

	return 0;
}

/**********************************************************************//**
@brief  satellite obs reorder

@param ObsData1		 [In] Satellite1 obs data
@param ObsData2		 [In] Satellite2 obs data

@retval difference of Satellite1 and Satellite2

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SatObsReorder(const void* ObsData1, const void* ObsData2)
{
	int ExSystemID1, ExSystemID2, PRN1, PRN2;

	ExSystemID1 = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(((RM_RANGE_DATA_T*)ObsData1)->ChnTrackStatus));
	ExSystemID2 = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(((RM_RANGE_DATA_T*)ObsData2)->ChnTrackStatus));

	if (ExSystemID1 == GLO)
	{
		PRN1 = ((RM_RANGE_DATA_T*)ObsData1)->PRN - 37;
	}
	else
	{
		PRN1 = ((RM_RANGE_DATA_T*)ObsData1)->PRN;
	}

	if (ExSystemID2 == GLO)
	{
		PRN2 = ((RM_RANGE_DATA_T*)ObsData2)->PRN - 37;
	}
	else
	{
		PRN2 = ((RM_RANGE_DATA_T*)ObsData2)->PRN;
	}

	int SatNo1 = GetAllSatIndex(ExSystemID1, PRN1);
	int SatNo2 = GetAllSatIndex(ExSystemID2, PRN2);

	return SatNo1 - SatNo2;
}

/**********************************************************************//**
@brief  satellite gnssobs reorder

@param ObsData1		 [In] Satellite1 obs data
@param ObsData2		 [In] Satellite2 obs data

@retval difference of Satellite1 and Satellite2

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SatGNSSObsReorder(const void* ObsData1, const void* ObsData2)
{
	int ExSystemID1, ExSystemID2, PRN1, PRN2;

	ExSystemID1 = GNSSObsSystemToExSystem(((RM_EX_GNSSOBS_DATA_T*)ObsData1)->SystemID);
	ExSystemID2 = GNSSObsSystemToExSystem(((RM_EX_GNSSOBS_DATA_T*)ObsData2)->SystemID);

	if (ExSystemID1 == GLO)
	{
		PRN1 = ((RM_EX_GNSSOBS_DATA_T*)ObsData1)->PRN - 37;
	}
	else
	{
		PRN1 = ((RM_EX_GNSSOBS_DATA_T*)ObsData1)->PRN;
	}

	if (ExSystemID2 == GLO)
	{
		PRN2 = ((RM_EX_GNSSOBS_DATA_T*)ObsData2)->PRN - 37;
	}
	else
	{
		PRN2 = ((RM_EX_GNSSOBS_DATA_T*)ObsData2)->PRN;
	}

	int SatNo1 = GetAllSatIndex(ExSystemID1, PRN1);
	int SatNo2 = GetAllSatIndex(ExSystemID2, PRN2);

	return SatNo1 - SatNo2;
}

/**********************************************************************//**
@brief  satellite obs reorder

@param ObsData1		 [In] Satellite1 obs data
@param ObsData2		 [In] Satellite2 obs data

@retval difference of Satellite1 and Satellite2

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GetObsFreq(int SatSys, unsigned int Signal, unsigned int* pFreq)
{
	const char *sig;
	unsigned int Freq = 0, Len;

	if (Signal < 1)
	{
		return -1;
	}

	switch (SatSys)
	{
		case GPS: sig = MSMSignalTabGPS[Signal - 1]; break;
		case GLO: sig = MSMSignalTabGLO[Signal - 1]; break;
		case GAL: sig = MSMSignalTabGAL[Signal - 1]; break;
		case QZS: sig = MSMSignalTabQZS[Signal - 1]; break;
		case SBA: sig = MSMSignalTabSBA[Signal - 1]; break;
		case BDS: sig = MSMSignalTabBDS[Signal - 1];break;
		case NIC: sig = MSMSignalTabNIC[Signal - 1];break;
		default: sig = ""; break;
	}
	/* signal to rinex obs type */
	Obs2Code(sig, &Freq);

	if (SatSys == BDS)
	{
		Len = (int)strlen(sig);
		if (Len > 0)
		{
			switch (sig[0])
			{
				case '2': Freq = 1; break;//B1
				case '7': Freq = 2; break;//B2
				case '6': Freq = 3; break;//B3
				case '1': Freq = 4; break;//B1c
				case '5': Freq = 5; break;//B2a
				case '8': Freq = 6; break;//B2new
				default:break;
			}
		}
	}
	else if (SatSys == GAL)
	{
		//E1/E5a/E5b/E5/E6
		Len = (int)strlen(sig);
		if (Len > 0)
		{
			switch (sig[0])
			{
				case '1': Freq = 1; break;
				case '5': Freq = 2; break;//E5a
				case '7': Freq = 3; break;//E5b
				case '8': Freq = 4; break;//E5
				case '6': Freq = 5; break;//E6
				default:break;
			}
		}
	}

	if (pFreq)
	{
		*pFreq = Freq;
	}

	return 0;
}

/**********************************************************************//**
@brief  satellite obs code reorder

@param ObsData1		 [In] Satellite1 obs data
@param ObsData2		 [In] Satellite2 obs data

@retval difference of Satellite1 freq and Satellite2 freq

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SatObsCodeReorder(const void* ObsData1, const void* ObsData2)
{
	int ExSystemID1, ExSystemID2, SigIndex1, SigIndex2, FreqIndex1 = NA8, FreqIndex2 = NA8;
	unsigned int ChnTrackStatus1 = (((RM_RANGE_DATA_T*)ObsData1)->ChnTrackStatus);
	unsigned int ChnTrackStatus2 = (((RM_RANGE_DATA_T*)ObsData2)->ChnTrackStatus);

	ExSystemID1 = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus1));
	ExSystemID2 = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus2));

	SigIndex1 = RMRangeSignalToRTCM3Signal(GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus1), GET_CH_TRACK_STATUS_SIGNALTYPE(ChnTrackStatus1), NULL, 0);
	SigIndex2 = RMRangeSignalToRTCM3Signal(GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus2), GET_CH_TRACK_STATUS_SIGNALTYPE(ChnTrackStatus2), NULL, 0);

	GetObsFreq(ExSystemID1, SigIndex1, &FreqIndex1);
	GetObsFreq(ExSystemID2, SigIndex2, &FreqIndex2);

	return FreqIndex1 - FreqIndex2;
}

/**********************************************************************//**
@brief  satellite gnssobs code reorder

@param ObsData1		 [In] Satellite1 obs data
@param ObsData2		 [In] Satellite2 obs data

@retval difference of Satellite1 freq and Satellite2 freq

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SatGNSSObsCodeReorder(const void* ObsData1, const void* ObsData2)
{
	int ExSystemID1, ExSystemID2, SigIndex1, SigIndex2, FreqIndex1 = NA8, FreqIndex2 = NA8;

	ExSystemID1 = GNSSObsSystemToExSystem(((RM_EX_GNSSOBS_DATA_T*)ObsData1)->SystemID);
	ExSystemID2 = GNSSObsSystemToExSystem(((RM_EX_GNSSOBS_DATA_T*)ObsData2)->SystemID);

	SigIndex1 = (((RM_EX_GNSSOBS_DATA_T*)ObsData1)->SignalType);
	SigIndex2 = (((RM_EX_GNSSOBS_DATA_T*)ObsData2)->SignalType);

	GetObsFreq(ExSystemID1, SigIndex1, &FreqIndex1);
	GetObsFreq(ExSystemID2, SigIndex2, &FreqIndex2);

	return FreqIndex1 - FreqIndex2;
}

/**********************************************************************//**
@brief  set header satellite time

@param pMsgConvertData	 [In] message convert data
@param pObsSatData		 [In] satellite obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SetSingleObsDataHeaderStaTime(SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	if (pSingleObsConvertData->RINEXObsHeader.TimeStart.Year == 0)
	{
		pSingleObsConvertData->RINEXObsHeader.TimeStart = pObsSatData->ObsTime;
		pSingleObsConvertData->RINEXObsHeader.Leapsecs = GetTimeLeaps(&pSingleObsConvertData->RINEXObsHeader.TimeStart);

		if (pSingleObsConvertData->RINEXObsHeader.Leapsecs > 0 && pSingleObsConvertData->RINEXEphHeader.Leapsecs != pSingleObsConvertData->RINEXObsHeader.Leapsecs)
		{
			pSingleObsConvertData->RINEXEphHeader.Leapsecs = pSingleObsConvertData->RINEXObsHeader.Leapsecs;
		}
		pSingleObsConvertData->TimeStartHistory[0] = pSingleObsConvertData->RINEXObsHeader.TimeStart;
		return 0;
	}

	if ((UTCDiff(&pObsSatData->ObsTime, &pSingleObsConvertData->RINEXObsHeader.TimeEnd) > 0) && (pObsSatData->SatNum > 0))
	{
		pSingleObsConvertData->RINEXObsHeader.TimeEnd = pObsSatData->ObsTime;
	}
	return 0;
}

static int SetHeaderStaTime(MSG_CONVERT_DATA_T* pMsgConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData, int RawDataType)
{
	return SetSingleObsDataHeaderStaTime(&pMsgConvertData->SingleObsConvertData[RawDataType], pObsSatData);
}

/**********************************************************************//**
@brief  set satellite signal type

@param pMsgConvertData	 [In] message convert data
@param pObsSatData		 [In] satellite obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SetSingleObsDatsSysSigType(SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	int ObsIndex, SigTypeIndex, ExSystemID, SatIndex, RMSignalID, RMSystemID;
	GNSS_OBS_PER_SAT_T* pObsPerSat;
	RM_RANGE_DATA_T* pRangeData;
	int ObsCode = 0;

	for (SatIndex = 0; SatIndex < pObsSatData->SatNum; SatIndex++)
	{
		pObsPerSat = &pObsSatData->ObsPerData[SatIndex];

		for (ObsIndex = 0; ObsIndex < pObsPerSat->ObsNum; ObsIndex++)
		{
			pRangeData = &pObsPerSat->ObsData[ObsIndex];

			if (pRangeData->PRN == 0)
			{
				continue;
			}

			RMSystemID = GET_CH_TRACK_STATUS_SATSYSTEM(pRangeData->ChnTrackStatus);
			ExSystemID = RMRangeSystemToExSystem(RMSystemID);
			if (ExSystemID < 0 || ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
			{
				continue;
			}

			RMSignalID = GET_CH_TRACK_STATUS_SIGNALTYPE(pRangeData->ChnTrackStatus);
			if (GetObsSigCode(RMSystemID, RMSignalID, &ObsCode) < 0)
			{
				continue;
			}

			for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
			{
				if (pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex] == NA32)
				{
					pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex] = ObsCode;
					break;
				}
				else if (ObsCode == pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex])
				{
					break;
				}
			}
		}
	}

	return 0;
}

static int SetSysSigType(MSG_CONVERT_DATA_T* pMsgConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData, int RawObsDataType)
{
	SetSingleObsDatsSysSigType(&pMsgConvertData->SingleObsConvertData[RawObsDataType], pObsSatData);

	return 0;
}

/**********************************************************************//**
@brief  set satellite signal type

@param pMsgConvertData	 [In] message convert data
@param pObsSatData		 [In] satellite obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int SetSingleGNSSObsDatsSysSigType(SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	int ObsIndex, SigTypeIndex, ExSystemID, SatIndex, RMSignalID, RMSystemID;
	GNSS_OBS_PER_SAT_T* pObsPerSat;
	RM_EX_GNSSOBS_DATA_T* pGNSSObsData;
	int ObsCode = 0;

	for (SatIndex = 0; SatIndex < pObsSatData->SatNum; SatIndex++)
	{
		pObsPerSat = &pObsSatData->ObsPerData[SatIndex];

		for (ObsIndex = 0; ObsIndex < pObsPerSat->ObsNum; ObsIndex++)
		{
			pGNSSObsData = &pObsPerSat->GNSSObsData[ObsIndex];

			if (pGNSSObsData->PRN == 0)
			{
				continue;
			}

			RMSystemID = GNSSObsSystemToRMRangeSystem(pGNSSObsData->SystemID);
			ExSystemID = RMRangeSystemToExSystem(RMSystemID);
			if (ExSystemID < 0 || ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
			{
				continue;
			}

			RMSignalID = GNSSObsSignalToRMRangeSignal(pGNSSObsData->SystemID, pGNSSObsData->SignalType, NULL);
			if (RMSignalID == NA8)
			{
				continue;
			}

			if (GetObsSigCode(RMSystemID, RMSignalID, &ObsCode) < 0)
			{
				continue;
			}

			for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
			{
				if (pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex] == NA32)
				{
					pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex] = ObsCode;
					break;
				}
				else if (ObsCode == pSingleObsConvertData->SysSigType[ExSystemID][SigTypeIndex])
				{
					break;
				}
			}
		}
	}

	return 0;
}

static int SetSysSigTypeEx(MSG_CONVERT_DATA_T* pMsgConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData, int RawObsDataType)
{
	SetSingleGNSSObsDatsSysSigType(&pMsgConvertData->SingleObsConvertData[RawObsDataType], pObsSatData);

	return 0;
}

/**********************************************************************//**
@brief  set header station position

@param pMsgConvertData	  [In] message convert data
@param pStationPosition	 [In] station position

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static void SetHeaderStationPosition(MSG_CONVERT_DATA_T* pMsgConvertData, double* pPosition)
{
	if (pMsgConvertData->RINEXObsHeader.Pos[0] == 0.0)
	{
		pMsgConvertData->RINEXObsHeader.Pos[0] = pPosition[0];
		pMsgConvertData->RINEXObsHeader.Pos[1] = pPosition[1];
		pMsgConvertData->RINEXObsHeader.Pos[2] = pPosition[2];
	}
}

/**********************************************************************//**
@brief  obs data convert rinex

@param pDataBuf				[In] rinex data buf
@param pObsOpt				 [In] obs opt
@param pObsSatData		  [In] satellite obs data
@param pEncodeBuf		   [In] encode buf

@retval <0 error, >=0 encode data len

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int ObsConvertRINEX(RINEX_DATA_BUF_T* pDataBuf, RINEX_OBS_OPT_T* pObsOpt, GNSS_OBS_SAT_DATA_T* pObsSatData, MSG_BUF_T* pEncodeBuf)
{
	int Result = -1;

	if (!pDataBuf || !pObsOpt || !pObsSatData || pObsSatData->SatNum <= 0 || !pEncodeBuf)
	{
		return -1;
	}

	if (!ScreenTime(&pObsSatData->ObsTime, &pObsOpt->TimeStart, &pObsOpt->TimeEnd, 0.0))
	{
		return -1;
	}

	/**< output rinex obs */
	Result = GenerateRINEX(RINEX_ID_OBS, pEncodeBuf->pBaseAddr, pObsOpt, pObsSatData);
	if (Result < 0)
	{
		return Result;
	}

	pEncodeBuf->Len = Result;

	return Result;
}

/**********************************************************************//**
@brief  eph data convert rinex

@param DataID			[In] data id
@param pDataBuf			[In] rinex data buf
@param pObsOpt			[In] obs opt
@param pEphData			[In] satellite eph data
@param pEncodeBuf		[In] encode buf

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int UniEphConvertRINEX(int DataID, RINEX_DATA_BUF_T* pDataBuf, RINEX_EPH_OPT_T* pEphOpt, UNI_EPHEMERIS_T* pEphData, MSG_BUF_T* pEncodeBuf)
{
	UTC_TIME_T TimeStart1, TimeEnd1;
	int ExSystemID, SatIndex, SatID, Week, CodeFlag = 0;
	UTC_TIME_T Time = { 0 };
	QC_TIME_T TempTime = { 0 };
	int LeapMSec = pEphOpt->Leapsecs * 1000;
	int Result = -1;
	int GPSLeapSecMs;
	int SatType = 0;

	ExSystemID = pEphData->SystemID;
	if (ExSystemID == QZS)
	{
		SatID = pEphData->SatID + 192;
	}
	else
	{
		SatID = pEphData->SatID;
	}

	if (ExSystemID == BDS)
	{
		Week = AdjBDTimeWeek(pEphData->Week);
		Week +=  1356;
		GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(Week, (int)pEphData->TOE*1000);
		GPSTimeToUTC(Week, (int)pEphData->TOE*1000, &GPSLeapSecMs, &Time);
	}
	else
	{
		GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(pEphData->Week, (int)pEphData->TOE*1000);
		GPSTimeToUTC(pEphData->Week, (int)pEphData->TOE*1000, &GPSLeapSecMs, &Time);
	}

	TimeStart1 = pEphOpt->TimeStart;
	if (TimeStart1.Year != 0)
	{
		TimeStart1 = UTCAdd(&TimeStart1, -MAXDTOE);
	}
	TimeEnd1 = pEphOpt->TimeEnd;
	if (TimeEnd1.Year != 0)
	{
		TimeEnd1 = UTCAdd(&TimeEnd1, MAXDTOE);
	}

	SatIndex = GetAllSatIndex(ExSystemID, SatID);
	if (SatIndex < 0)
	{
		return -1;
	}

	if (pEphOpt->ExSats[SatIndex] == 1 || !ScreenTime(&Time, &TimeStart1, &TimeEnd1, 0.0))
	{
		return -1;
	}

	if (pDataBuf->ReceiveID >= MAX_RCV_ID_COUNT)
	{
		return -1;
	}
	TempTime = UTC2QCTime(&Time);
	CodeFlag = GetCodeFlag(DataID);
	if (pEphData->SystemID == GAL)
	{
		if (CodeFlag == 0)
		{
			if (TempTime.Time - pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] > 0)
			{
				pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] = (unsigned int)TempTime.Time;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			if (pEphOpt->Ver >= 400)
			{
				if (DataID == RINEX_ID_GALEPH)
				{
					SatType = 0;
				}
				else if (DataID == RINEX_ID_GALEPH_FNAV)
				{
					SatType = 1;
				}
				else
				{
					SatType = 2;
				}

				if (TempTime.Time - pDataBuf->GALTypeTimeFlag[pDataBuf->ReceiveID][SatID- 1][SatType] > 0)
				{
					pDataBuf->GALTypeTimeFlag[pDataBuf->ReceiveID][SatID - 1][SatType] = (unsigned int)TempTime.Time;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				if (TempTime.Time - pDataBuf->GALTimeFlag[pDataBuf->ReceiveID][SatID- 1] > 0)
				{
					pDataBuf->GALTimeFlag[pDataBuf->ReceiveID][SatID - 1] = (unsigned int)TempTime.Time;
				}
				else
				{
					return -1;
				}
			}
		}
	}
	else if (pEphData->SystemID == BDS)
	{
		if (CodeFlag == 0)
		{
			if (pEphOpt->Ver >= 400)
			{
				if (DataID == RINEX_ID_BDSEPH)
				{
					SatType = 0;
				}
				else if (DataID == RINEX_ID_BDSEPH_CNAV1)
				{
					SatType = 1;
				}
				else if (DataID == RINEX_ID_BDSEPH_CNAV2)
				{
					SatType = 2;
				}
				else if (DataID == RINEX_ID_BDSEPH_CNAV3)
				{
					SatType = 3;
				}

				if (TempTime.Time - pDataBuf->BDSTimeFlag[pDataBuf->ReceiveID][SatID- 1][SatType] > 0)
				{
					pDataBuf->BDSTimeFlag[pDataBuf->ReceiveID][SatID - 1][SatType] = (unsigned int)TempTime.Time;
				}
				else
				{
					return -1;
				}
			}
			else
			{
				if (TempTime.Time - pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] > 0)
				{
					pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] = (unsigned int)TempTime.Time;
				}
				else
				{
					return -1;
				}
			}
		}
		else if (CodeFlag <= 2)
		{
			if (TempTime.Time - pDataBuf->BDSJpFlag[pDataBuf->ReceiveID][CodeFlag][pEphData->SatID] > 0)
			{
				pDataBuf->BDSJpFlag[pDataBuf->ReceiveID][CodeFlag][pEphData->SatID] = (unsigned int)TempTime.Time;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (TempTime.Time - pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] > 0)
		{
			pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] = (unsigned int)TempTime.Time;
		}
		else
		{
			return -1;
		}
	}

	Result = GenerateRINEX(DataID, pEncodeBuf->pBaseAddr, pEphOpt, pEphData);
	if (Result < 0)
	{
		return Result;
	}

	pEncodeBuf->Len = Result;

	return Result;
}

/**********************************************************************//**
@brief  GLO eph data convert rinex

@param DataID			[In] data id
@param pDataBuf			[]In] rinex data buf
@param pObsOpt			[In] obs opt
@param pGLOEphData		[In] GLO eph data
@param pEncodeBuf		[In] encode buf

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GLOEphConvertRINEX(RINEX_DATA_BUF_T* pDataBuf, RINEX_EPH_OPT_T* pEphOpt, GLO_EPHEMERIS_T* pGLOEphData, MSG_BUF_T* pEncodeBuf)
{
	UTC_TIME_T TimeStart2, TimeEnd2;
	int SatIndex;
	UTC_TIME_T Time = { 0 };
	QC_TIME_T TempTime = { 0 };
	Epoch2UTC(pGLOEphData->TOE, &Time);
	Time = UTCAdd(&Time, -18000);
	int Result = -1;

	TimeStart2 = pEphOpt->TimeStart;
	if (TimeStart2.Year != 0)
	{
		TimeStart2 = UTCAdd(&TimeStart2, -MAXDTOE_GLO);
	}
	TimeEnd2 = pEphOpt->TimeEnd;
	if (TimeEnd2.Year != 0)
	{
		TimeEnd2 = UTCAdd(&TimeEnd2, MAXDTOE_GLO);
	}

	SatIndex = GetAllSatIndex(GLO, pGLOEphData->SatID);
	if (SatIndex < 0)
	{
		return -1;
	}

	if (pEphOpt->ExSats[SatIndex] == 1 || !ScreenTime(&Time, &TimeStart2, &TimeEnd2, 0.0))
	{
		return -1;
	}

	if (pDataBuf->ReceiveID >= MAX_RCV_ID_COUNT)
	{
		return -1;
	}
	TempTime = UTC2QCTime(&Time);
	if (TempTime.Time - pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] > 0)
	{
		pDataBuf->NavTimeFlag[pDataBuf->ReceiveID][SatIndex] = (unsigned int)TempTime.Time;
	}
	else
	{
		return -1;
	}

	Result =GenerateRINEX(RINEX_ID_GLOEPH, pEncodeBuf->pBaseAddr, pEphOpt, pGLOEphData);
	if (Result < 0)
	{
		return Result;
	}

	pEncodeBuf->Len = Result;

	return Result;
}

/**********************************************************************//**
@brief  range data to satellite obs data

@param pObsData			[In] range obs data
@param pObsSatData		[In] satellite obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int RangeDataToObsSatData(GNSS_OBS_T* pObsData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	unsigned int ObsIndex, SatNum = 0, SignalIndex = 0;
	int ExSystemID, PRN, LastPRN = -1;
	unsigned int Index,signmap[32] = {0};
	RM_RANGE_DATA_T* pRangeData;

	for (ObsIndex = 0; ObsIndex < pObsData->RangeData.ObsNum && SatNum < RINEX_SAT_NUM_MAX; ObsIndex++)
	{
		pRangeData = &pObsData->RangeData.ObsData[ObsIndex];
		if (pRangeData->PRN == 0)
		{
			continue;
		}

		ExSystemID = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(pRangeData->ChnTrackStatus));
		if (ExSystemID == 0xFF)
		{
			continue;
		}

		if (LastPRN != pRangeData->PRN)
		{
			if (LastPRN != -1)
			{
				pObsSatData->ObsPerData[SatNum - 1].ObsNum = SignalIndex;
				for (Index = 0; Index < SignalIndex; Index++)
				{
					signmap[Index] = GET_CH_TRACK_STATUS_SIGNALTYPE(pObsSatData->ObsPerData[SatNum - 1].ObsData[Index].ChnTrackStatus);
				}
				qsort(pObsSatData->ObsPerData[SatNum - 1].ObsData, SignalIndex, sizeof(RM_RANGE_DATA_T), SatObsCodeReorder);
				for (Index = 0; Index < SignalIndex; Index++)
				{
					signmap[Index] = GET_CH_TRACK_STATUS_SIGNALTYPE(pObsSatData->ObsPerData[SatNum - 1].ObsData[Index].ChnTrackStatus);
				}
			}
			LastPRN = pRangeData->PRN;
			SatNum++;
			SignalIndex = 0;
			pObsSatData->ObsPerData[SatNum - 1].ExSystemID = ExSystemID;

			if (ExSystemID == GLO)
			{
				PRN = pRangeData->PRN - 37;
			}
			else
			{
				PRN = pRangeData->PRN;
			}
			pObsSatData->ObsPerData[SatNum - 1].SatID = PRN;
		}

		MEMCPY(&pObsSatData->ObsPerData[SatNum - 1].ObsData[SignalIndex++], pRangeData, sizeof(RM_RANGE_DATA_T));
	}

	pObsSatData->SatNum = SatNum;
	pObsSatData->ObsTime = pObsData->ObsTime;

	return 0;
}

/**********************************************************************//**
@brief  gnssobs data to satellite obs data

@param pObsData			[In] range obs data
@param pObsSatData		[In] satellite obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GNSSObsDataToObsSatData(GNSS_OBS_T* pObsData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	unsigned int ObsIndex, SatNum = 0, SignalIndex = 0;
	int ExSystemID, PRN, LastPRN = -1;
	unsigned int Index,signmap[32] = {0};
	RM_EX_GNSSOBS_DATA_T* pGNSSObsData;

	for (ObsIndex = 0; ObsIndex < pObsData->GNSSObsData.ObsNum && SatNum < RINEX_SAT_NUM_MAX; ObsIndex++)
	{
		pGNSSObsData = &pObsData->GNSSObsData.ObsData[ObsIndex];
		if (pGNSSObsData->PRN == 0)
		{
			continue;
		}

		ExSystemID = GNSSObsSystemToExSystem(pGNSSObsData->SystemID);
		if (ExSystemID == 0xFF)
		{
			continue;
		}

		if (LastPRN != pGNSSObsData->PRN)
		{
			if (LastPRN != -1)
			{
				pObsSatData->ObsPerData[SatNum - 1].ObsNum = SignalIndex;
				for (Index = 0; Index < SignalIndex; Index++)
				{
					signmap[Index] = pObsSatData->ObsPerData[SatNum - 1].GNSSObsData[Index].SignalType;
				}
				qsort(pObsSatData->ObsPerData[SatNum - 1].GNSSObsData, SignalIndex, sizeof(RM_EX_GNSSOBS_DATA_T), SatGNSSObsCodeReorder);
				for (Index = 0; Index < SignalIndex; Index++)
				{
					signmap[Index] = pObsSatData->ObsPerData[SatNum - 1].GNSSObsData[Index].SignalType;
				}
			}
			LastPRN = pGNSSObsData->PRN;
			SatNum++;
			SignalIndex = 0;
			pObsSatData->ObsPerData[SatNum - 1].ExSystemID = ExSystemID;

			if (ExSystemID == GLO)
			{
				PRN = pGNSSObsData->PRN - 37;
			}
			else
			{
				PRN = pGNSSObsData->PRN;
			}
			pObsSatData->ObsPerData[SatNum - 1].SatID = PRN;
		}

		MEMCPY(&pObsSatData->ObsPerData[SatNum - 1].GNSSObsData[SignalIndex++], pGNSSObsData, sizeof(RM_EX_GNSSOBS_DATA_T));
	}

	pObsSatData->SatNum = SatNum;
	pObsSatData->ObsTime = pObsData->ObsTime;

	pObsSatData->DataType = GNSS_OBS_DATA_TYPE_GNSSOBS;

	return 0;
}

/**********************************************************************//**
@brief  output rinex obs header

@param pFile				[In] file handle
@param pObsOpt				[In] obs opt
@param pHeader				[In] rinex obs header struct
@param pEncodeBuf			[In] encode buf

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int OutRINEXObsHeader(FILE* pFile, RINEX_OBS_OPT_T* pObsOpt, RINEX_OBS_HEADER_T* pHeader, MSG_BUF_T* pEncodeBuf)
{
	if (!pFile || !pObsOpt || !pHeader)
	{
		return -1;
	}

	pEncodeBuf->Len = GenerateRINEX(RINEX_ID_OBS_HEADER, pEncodeBuf->pBaseAddr, pObsOpt, pHeader);
	if (pEncodeBuf->Len > 0)
	{
		fprintf(pFile, "%s", pEncodeBuf->pBaseAddr);
	}
	else
	{
		return -1;
	}

	return pEncodeBuf->Len;
}

/**********************************************************************//**
@brief  get rinexheader information of obs data

@param pObsData				[In] obs data
@param pRINEXObsHeader		[Out] pointer to rinex obs header info

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GetSingleObsDataRinexHeaderInfo(GNSS_OBS_T* pObsData, SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData)
{
	unsigned int ObsIndex, SatIndex, ObsNum, ExSystemID, SignalID, PRNLock;
	RM_RANGE_DATA_T* pRMRangeData;
	RINEX_PHASE_SHIFT_INFO_T* pRinexPhaseShiftInfo;
	PER_SYS_PHASE_SHIFT_INFO_T* pPerSysPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pPerSignalPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pTempPerSignalPhaseShiftInfo;
	int RawObsDataType;
	RINEX_OBS_HEADER_T* pRINEXObsHeader = &pSingleObsConvertData->RINEXObsHeader;

	if ((!pObsData) || (!pRINEXObsHeader))
	{
		return -1;
	}

	pRinexPhaseShiftInfo = &pRINEXObsHeader->RinexPhaseShiftInfo;

	ObsNum = pObsData->RangeData.ObsNum;
	RawObsDataType = pObsData->RawDataType;

	for (ObsIndex = 0; ObsIndex < ObsNum; ObsIndex++)
	{
		pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

		ExSystemID = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(pRMRangeData->ChnTrackStatus));
		if (ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
		{
			continue;
		}

		SignalID = GET_CH_TRACK_STATUS_SIGNALTYPE(pRMRangeData->ChnTrackStatus);
		PRNLock = GET_CH_TRACK_STATUS_PRNLOCK(pRMRangeData->ChnTrackStatus);

		pPerSysPhaseShiftInfo = &pRinexPhaseShiftInfo->PerSysPhaseShiftInfo[ExSystemID];
		pPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[SignalID];

		if (ExSystemID == BDS)
		{
			if (SignalID == RM_RANGE_SIGNAL_BDSB1ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB1ID1];
			}
			else if (SignalID == RM_RANGE_SIGNAL_BDSB3ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB3ID1];
			}
			else if (SignalID == RM_RANGE_SIGNAL_BDSB2ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB2ID1];
			}
			else
			{
				pTempPerSignalPhaseShiftInfo = pPerSignalPhaseShiftInfo;
			}
		}
		else
		{
			pTempPerSignalPhaseShiftInfo = pPerSignalPhaseShiftInfo;
		}

		if (pTempPerSignalPhaseShiftInfo->SatNum == 0)
		{
			pTempPerSignalPhaseShiftInfo->SatIDMap[0] = pRMRangeData->PRN;
			pTempPerSignalPhaseShiftInfo->SatNum += 1;
			if ((RawObsDataType >= RAW_OBS_DATA_TYPE_RANGE_ASCII) && (RawObsDataType <= RAW_OBS_DATA_TYPE_RANGECMP_1_BINARY))
			{
				if (PRNLock == 1) /**< new version range */
				{
					pTempPerSignalPhaseShiftInfo->PhaseShiftData = GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
				}
				else
				{
					if (((ExSystemID == GPS) && ((SignalID == RM_RANGE_SIGNAL_GPSL2CL) || (SignalID == RM_RANGE_SIGNAL_GPSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_GPSL2CX) || (SignalID == RM_RANGE_SIGNAL_GPSL1CP))) ||
						((ExSystemID == QZS) && ((SignalID == RM_RANGE_SIGNAL_QZSSL2CL) || (SignalID == RM_RANGE_SIGNAL_QZSSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_QZSSL2CX) || (SignalID == RM_RANGE_SIGNAL_QZSSL1CP))))
					{
						pTempPerSignalPhaseShiftInfo->PhaseShiftData = -GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
					else
					{
						pTempPerSignalPhaseShiftInfo->PhaseShiftData = GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
				}
			}
			else
			{
				pTempPerSignalPhaseShiftInfo->PhaseShiftData = 0;
			}
		}
		else
		{
			for (SatIndex = 0; SatIndex < pTempPerSignalPhaseShiftInfo->SatNum; SatIndex++)
			{
				if (pRMRangeData->PRN == pTempPerSignalPhaseShiftInfo->SatIDMap[SatIndex])
				{
					break;
				}
			}

			if (SatIndex >= pTempPerSignalPhaseShiftInfo->SatNum)
			{
				pTempPerSignalPhaseShiftInfo->SatIDMap[pTempPerSignalPhaseShiftInfo->SatNum] = pRMRangeData->PRN;
				pTempPerSignalPhaseShiftInfo->SatNum += 1;
			}
		}
	}

	return 0;
}

static int GetObsDataRinexHeaderInfo(GNSS_OBS_T* pObsData, MSG_CONVERT_DATA_T* pMsgConvertData, int RawDataType)
{
	return GetSingleObsDataRinexHeaderInfo(pObsData, &pMsgConvertData->SingleObsConvertData[RawDataType]);
}

/**********************************************************************//**
@brief  get rinexheader information of obs data

@param pObsData				[In] obs data
@param pRINEXObsHeader		[Out] pointer to rinex obs header info

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GetSingleGNSSObsDataRinexHeaderInfo(GNSS_OBS_T* pObsData, SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData)
{
	unsigned int ObsIndex, SatIndex, ObsNum, ExSystemID, SignalID, PRNLock= 1; /**< new version range */
	RM_EX_GNSSOBS_DATA_T* pGNSSObsData;
	RINEX_PHASE_SHIFT_INFO_T* pRinexPhaseShiftInfo;
	PER_SYS_PHASE_SHIFT_INFO_T* pPerSysPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pPerSignalPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pTempPerSignalPhaseShiftInfo;
	int RawObsDataType;
	RINEX_OBS_HEADER_T* pRINEXObsHeader = &pSingleObsConvertData->RINEXObsHeader;

	if ((!pObsData) || (!pRINEXObsHeader))
	{
		return -1;
	}

	pRinexPhaseShiftInfo = &pRINEXObsHeader->RinexPhaseShiftInfo;

	ObsNum = pObsData->GNSSObsData.ObsNum;
	RawObsDataType = pObsData->RawDataType;

	for (ObsIndex = 0; ObsIndex < ObsNum; ObsIndex++)
	{
		pGNSSObsData = &pObsData->GNSSObsData.ObsData[ObsIndex];

		ExSystemID = GNSSObsSystemToExSystem(pGNSSObsData->SystemID);
		if (ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
		{
			continue;
		}

		SignalID = GNSSObsSignalToRMRangeSignal(pGNSSObsData->SystemID, pGNSSObsData->SignalType, NULL);
		if (SignalID == NA8)
		{
			continue;
		}

		pPerSysPhaseShiftInfo = &pRinexPhaseShiftInfo->PerSysPhaseShiftInfo[ExSystemID];
		pPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[SignalID];

		if (ExSystemID == BDS)
		{
			if (SignalID == RM_RANGE_SIGNAL_BDSB1ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB1ID1];
			}
			else if (SignalID == RM_RANGE_SIGNAL_BDSB3ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB3ID1];
			}
			else if (SignalID == RM_RANGE_SIGNAL_BDSB2ID2)
			{
				pTempPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[RM_RANGE_SIGNAL_BDSB2ID1];
			}
			else
			{
				pTempPerSignalPhaseShiftInfo = pPerSignalPhaseShiftInfo;
			}
		}
		else
		{
			pTempPerSignalPhaseShiftInfo = pPerSignalPhaseShiftInfo;
		}

		if (pTempPerSignalPhaseShiftInfo->SatNum == 0)
		{
			pTempPerSignalPhaseShiftInfo->SatIDMap[0] = pGNSSObsData->PRN;
			pTempPerSignalPhaseShiftInfo->SatNum += 1;
			if ((RawObsDataType >= RAW_OBS_DATA_TYPE_RANGE_ASCII) && (RawObsDataType <= RAW_OBS_DATA_TYPE_RANGECMP_1_BINARY))
			{
				if (PRNLock == 1) /**< new version range */
				{
					pTempPerSignalPhaseShiftInfo->PhaseShiftData = GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
				}
				else
				{
					if (((ExSystemID == GPS) && ((SignalID == RM_RANGE_SIGNAL_GPSL2CL) || (SignalID == RM_RANGE_SIGNAL_GPSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_GPSL2CX) || (SignalID == RM_RANGE_SIGNAL_GPSL1CP))) ||
						((ExSystemID == QZS) && ((SignalID == RM_RANGE_SIGNAL_QZSSL2CL) || (SignalID == RM_RANGE_SIGNAL_QZSSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_QZSSL2CX) || (SignalID == RM_RANGE_SIGNAL_QZSSL1CP))))
					{
						pTempPerSignalPhaseShiftInfo->PhaseShiftData = -GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
					else
					{
						pTempPerSignalPhaseShiftInfo->PhaseShiftData = GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
				}
			}
			else
			{
				pTempPerSignalPhaseShiftInfo->PhaseShiftData = 0;
			}
		}
		else
		{
			for (SatIndex = 0; SatIndex < pTempPerSignalPhaseShiftInfo->SatNum; SatIndex++)
			{
				if (pGNSSObsData->PRN == pTempPerSignalPhaseShiftInfo->SatIDMap[SatIndex])
				{
					break;
				}
			}

			if (SatIndex >= pTempPerSignalPhaseShiftInfo->SatNum)
			{
				pTempPerSignalPhaseShiftInfo->SatIDMap[pTempPerSignalPhaseShiftInfo->SatNum] = pGNSSObsData->PRN;
				pTempPerSignalPhaseShiftInfo->SatNum += 1;
			}
		}
	}

	return 0;
}

static int GetGNSSObsDataRinexHeaderInfo(GNSS_OBS_T* pObsData, MSG_CONVERT_DATA_T* pMsgConvertData, int RawDataType)
{
	return GetSingleGNSSObsDataRinexHeaderInfo(pObsData, &pMsgConvertData->SingleObsConvertData[RawDataType]);
}

static void SetSingleObsDataTimeInfo(GNSS_OBS_T* pObsData, unsigned int ObsNum, SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData)
{
	double DiffTime = 0;

	if (pSingleObsConvertData->DiffPos)
	{
		if (pSingleObsConvertData->PosHistoryNum < pSingleObsConvertData->MaxPoshis)
		{
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->PosHistoryNum] = pObsData->ObsTime;
		}
		pSingleObsConvertData->DiffPos = 0;
	}
	if (pSingleObsConvertData->TimeInt < 1E-12)
	{
		pSingleObsConvertData->TimeInt = pSingleObsConvertData->RINEXObsHeader.TimeInt;
	}

	if (pSingleObsConvertData->OldTime.Year == 0)
	{
		pSingleObsConvertData->OldTime = pObsData->ObsTime;
	}

	DiffTime = (double)UTCDiff(&pObsData->ObsTime, &pSingleObsConvertData->OldTime) / 1000;
	if (DiffTime > 0)
	{
		pSingleObsConvertData->TimeInt = pSingleObsConvertData->TimeInt > DiffTime ? DiffTime : pSingleObsConvertData->TimeInt;
		pSingleObsConvertData->RINEXObsHeader.TimeInt = pSingleObsConvertData->TimeInt;
	}
	pSingleObsConvertData->OldTime = pObsData->ObsTime;

	if (ObsNum > 0)
	{
		if (pSingleObsConvertData->PosHistoryNum < pSingleObsConvertData->MaxPoshis)
		{
			pSingleObsConvertData->TimeEndHistory[pSingleObsConvertData->PosHistoryNum] = pObsData->ObsTime;
		}
		else
		{
			pSingleObsConvertData->TimeEndHistory[pSingleObsConvertData->MaxPoshis - 1] = pObsData->ObsTime;
		}
	}
}

static void SetObsDataTimeInfo(GNSS_OBS_T* pObsData, MSG_CONVERT_DATA_T* pMsgConvertData, int RawDataType)
{
	SetSingleObsDataTimeInfo(pObsData, pObsData->RangeData.ObsNum, &pMsgConvertData->SingleObsConvertData[RawDataType]);
}

static void SetGNSSObsDataTimeInfo(GNSS_OBS_T* pObsData, MSG_CONVERT_DATA_T* pMsgConvertData, int RawDataType)
{
	SetSingleObsDataTimeInfo(pObsData, pObsData->GNSSObsData.ObsNum, &pMsgConvertData->SingleObsConvertData[RawDataType]);
}

/**********************************************************************//**
@brief  signal decode obs convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeObsConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pData;
	double DiffTime = 0;
	GNSS_OBS_SAT_DATA_T* pObsSatData = &pMsgConvertData->ObsSatData;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	if (pObsData->RangeData.ObsNum == 0)
	{
		return -1;
	}

	pMsgConvertData->RawObsDataType |= (1 << RawObsDataType);

	MEMSET(pObsSatData, 0, sizeof(GNSS_OBS_SAT_DATA_T));

	qsort(pObsData->RangeData.ObsData, pObsData->RangeData.ObsNum, sizeof(RM_RANGE_DATA_T), SatObsReorder);

	RangeDataToObsSatData(pObsData, pObsSatData);

	if (pMsgConvertData->AllFreqFlag > 0)
	{
		SetSysSigType(pMsgConvertData, pObsSatData, RawObsDataType); /**< get signal ID */
	}
	SetHeaderStaTime(pMsgConvertData, pObsSatData, RawObsDataType);

	GetObsDataRinexHeaderInfo(pObsData, pMsgConvertData, RawObsDataType);

	SetObsDataTimeInfo(pObsData, pMsgConvertData, RawObsDataType);

	if (pMsgConvertData->PrintfNumLimit - 150 == 0)
	{
		pMsgConvertData->PrintfNumLimit = 0;
		GNSSPrintf(".");
	}
	else
	{
		pMsgConvertData->PrintfNumLimit++;
	}

	pMsgConvertData->DecodeObsNum++;

	return 0;
}

/**********************************************************************//**
@brief  signal decode gnssobs convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeGNSSObsConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pData;
	double DiffTime = 0;
	GNSS_OBS_SAT_DATA_T* pObsSatData = &pMsgConvertData->ObsSatData;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	if (pObsData->GNSSObsData.ObsNum == 0)
	{
		return -1;
	}

	pMsgConvertData->RawObsDataType |= (1 << RawObsDataType);

	MEMSET(pObsSatData, 0, sizeof(GNSS_OBS_SAT_DATA_T));

	qsort(pObsData->GNSSObsData.ObsData, pObsData->GNSSObsData.ObsNum, sizeof(RM_EX_GNSSOBS_DATA_T), SatGNSSObsReorder);

	GNSSObsDataToObsSatData(pObsData, pObsSatData);

	if (pMsgConvertData->AllFreqFlag > 0)
	{
		SetSysSigTypeEx(pMsgConvertData, pObsSatData, RawObsDataType); /**< get signal ID */
	}
	SetHeaderStaTime(pMsgConvertData, pObsSatData, RawObsDataType);

	GetGNSSObsDataRinexHeaderInfo(pObsData, pMsgConvertData, RawObsDataType);

	SetGNSSObsDataTimeInfo(pObsData, pMsgConvertData, RawObsDataType);

	if (pMsgConvertData->PrintfNumLimit - 150 == 0)
	{
		pMsgConvertData->PrintfNumLimit = 0;
		GNSSPrintf(".");
	}
	else
	{
		pMsgConvertData->PrintfNumLimit++;
	}

	pMsgConvertData->DecodeObsNum++;

	return 0;
}

/**********************************************************************//**
@brief  signal decode eph convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeEphConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData = &pMsgConvertData->GlobalObsConvertData;
	int FileIndex = GetFileIndex(RawDataType, DecodeID);

	if (FileIndex < 0)
	{
		return -1;
	}

	if (pMsgConvertData->RawObsDataType != 0)
	{
		pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[RawObsDataType];
	}

	if (DATA_OUT_TYPE_RINEX == GetDataOutType(pMsgConvertData))
	{
		pSingleObsConvertData->RinexNavFileFpFlag[FileIndex] = 1;
	}
	else
	{
		pMsgConvertData->FpRawMsgOutPFlag = 1;
	}

	return 0;
}

/**********************************************************************//**
@brief  check single position

@param pMsgConvertData		[In] message convert data
@param pPosition			[In] Position data

@retval <0:error, =0:success

@author CHC
@date 2024/07/19
@note
**************************************************************************/
static void CheckPositin(MSG_CONVERT_DATA_T* pMsgConvertData, double* pPosition)
{
	if ((!IsAbsoluteDoubleZero(pPosition[0])) && (!IsAbsoluteDoubleZero(pPosition[1])) && (!IsAbsoluteDoubleZero(pPosition[2])))
	{
		if (!DOUBLE_IS_EQUAL(pMsgConvertData->PosHistoryBuf[0], pPosition[0]) ||
			!DOUBLE_IS_EQUAL(pMsgConvertData->PosHistoryBuf[1], pPosition[1]) ||
			!DOUBLE_IS_EQUAL(pMsgConvertData->PosHistoryBuf[2], pPosition[2]))
		{
			pMsgConvertData->DiffPos = 1;
			if (pMsgConvertData->PosHistoryNum < MAXPOSHIS)
			{
				MEMCPY(&pMsgConvertData->PosHistory[pMsgConvertData->PosHistoryNum][0], &pPosition[0], 3*sizeof(double));
				MEMCPY(&pMsgConvertData->PosHistoryBuf[0], &pPosition[0], 3*sizeof(double));

				pMsgConvertData->PosHistoryNum++;
			}
		}
	}
}

/**********************************************************************//**
@brief  signal decode antenna information convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] antenna information data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeAntennaInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RTCM_ANTENNA_INFO_T* pAntennaInfo = (RTCM_ANTENNA_INFO_T*)pData;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	SetHeaderStationPosition(pMsgConvertData, pAntennaInfo->Position);
	CheckPositin(pMsgConvertData, pAntennaInfo->Position);

	return 0;
}

/**********************************************************************//**
@brief  signal decode station position convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] station position data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeStationPositionConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	STATION_POSITION_DATA_T* pStationPosition = (STATION_POSITION_DATA_T*)pData;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	SetHeaderStationPosition(pMsgConvertData, pStationPosition->Position);
	CheckPositin(pMsgConvertData, pStationPosition->Position);

	return 0;
}

/**********************************************************************//**
@brief  signal decode bestpos information convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] station position data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeBestPosInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	HRCX_BESTPOS_INFO_T* pBestPosInfo = (HRCX_BESTPOS_INFO_T*)pData;
	double PosXYZ[3];

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	PosXYZ[0] = pBestPosInfo->RefX;
	PosXYZ[1] = pBestPosInfo->RefY;
	PosXYZ[2] = pBestPosInfo->RefZ;

	SetHeaderStationPosition(pMsgConvertData, PosXYZ);
	CheckPositin(pMsgConvertData, PosXYZ);

	return 0;
}

/**********************************************************************//**
@brief  signal decode refstation information convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] station position data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeRefStationInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RM_REFSTATION_DATA_T* pRefStation = (RM_REFSTATION_DATA_T*)pData;
	double PosXYZ[3];

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	PosXYZ[0] = pRefStation->ECEFX;
	PosXYZ[1] = pRefStation->ECEFY;
	PosXYZ[2] = pRefStation->ECEFZ;

	SetHeaderStationPosition(pMsgConvertData, PosXYZ);
	CheckPositin(pMsgConvertData, PosXYZ);

	return 0;
}

/**********************************************************************//**
@brief  signal decode refstation convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] station position information data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SignalDecodeRefStationConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RM_REFSTATIONINFO_DATA_T* pRefStationInfo = (RM_REFSTATIONINFO_DATA_T*)pData;
	double PosLLH[3];
	double PosXYZ[3];

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	PosLLH[0] = pRefStationInfo->Latitude;
	PosLLH[1] = pRefStationInfo->Longitude;
	PosLLH[2] = pRefStationInfo->Height;

	PosLLH2XYZ(PosLLH, PosXYZ);

	SetHeaderStationPosition(pMsgConvertData, PosXYZ);
	CheckPositin(pMsgConvertData, PosXYZ);

	return 0;
}

static void SingleObsDataConvertRinex(MSG_CONVERT_DATA_T* pMsgConvertData, unsigned int RawObasDataType, SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	UTC_TIME_T TempTime;
	int Year = 0;
	char FileName[512] = { 0 };
	unsigned int Index;
	size_t Len = strlen(pMsgConvertData->OutDir);
	const char* pGetRawObsDataTypeStr = NULL;

	if ((pSingleObsConvertData->Rece < pSingleObsConvertData->MaxPoshis && pSingleObsConvertData->PosHistoryNum != 0) ||
	 (pSingleObsConvertData->PosHistoryNum == 0 && pSingleObsConvertData->Rece == 0))
	{
		if (pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Year == pObsSatData->ObsTime.Year &&
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Month == pObsSatData->ObsTime.Month &&
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Day == pObsSatData->ObsTime.Day &&
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Hour == pObsSatData->ObsTime.Hour &&
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Minute == pObsSatData->ObsTime.Minute &&
			pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Second == pObsSatData->ObsTime.Second &&
			pSingleObsConvertData->Rece <= pSingleObsConvertData->PosHistoryNum)
		{
			/** Close the old file and open the new one */
			if (pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece].Year != 0)
			{
				TempTime = pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece];
			}
			else
			{
				TempTime = RTCMGetUTCTime();
			}

			Year = (int)TempTime.Year % 100;
			MEMCPY(FileName, pMsgConvertData->OutDir, Len);
			sprintf(FileName + Len, "%s", pMsgConvertData->PureName);

			if (RawObasDataType < RAW_OBS_DATA_TYPE_NUM_MAX)
			{
				pGetRawObsDataTypeStr = GetRawObsDataTypeString(RawObasDataType);
				if (pGetRawObsDataTypeStr == NULL)
				{
					return;
				}
			}

			if (pSingleObsConvertData->Rece > 1)
			{
				pSingleObsConvertData->RINEXObsHeader.TimeStart = pSingleObsConvertData->TimeStartHistory[pSingleObsConvertData->Rece];
				pSingleObsConvertData->RINEXObsHeader.TimeEnd = pSingleObsConvertData->TimeEndHistory[pSingleObsConvertData->Rece];
				for (Index = 0; Index < 3; Index++)
				{
					pSingleObsConvertData->RINEXObsHeader.Pos[Index] = pSingleObsConvertData->PosHistory[pSingleObsConvertData->Rece - 1][Index];
				}
				if (RawObasDataType < RAW_OBS_DATA_TYPE_NUM_MAX)
				{
					sprintf(FileName + (int)strlen(FileName), "%1d%s.%02do", pSingleObsConvertData->Rece - 1, pGetRawObsDataTypeStr, Year);
				}
				else
				{
					sprintf(FileName + (int)strlen(FileName), "%1d.%02do", pSingleObsConvertData->Rece - 1, Year);
				}
				pSingleObsConvertData->RinexObsFileFp = fopen(FileName, "wb+");
				OutRINEXObsHeader(pSingleObsConvertData->RinexObsFileFp, &pSingleObsConvertData->RINEXObsOpt, &pSingleObsConvertData->RINEXObsHeader, &pMsgConvertData->EncodeDataBuf);
			}
			else
			{
				pSingleObsConvertData->RINEXObsHeader.TimeStart = pSingleObsConvertData->TimeStartHistory[0];
				if (pSingleObsConvertData->PosHistoryNum == 0)
				{
					pSingleObsConvertData->RINEXObsHeader.TimeEnd = pSingleObsConvertData->TimeEndHistory[0];
				}
				else
				{
					if (pSingleObsConvertData->TimeEndHistory[1].Year != 0)
					{
						pSingleObsConvertData->RINEXObsHeader.TimeEnd = pSingleObsConvertData->TimeEndHistory[1];
					}
					else
					{
						pSingleObsConvertData->RINEXObsHeader.TimeEnd = pSingleObsConvertData->TimeEndHistory[0];
					}
				}
				if (pSingleObsConvertData->PosHistoryNum != 0)
				{
					for (Index = 0; Index < 3; Index++)
					{
						pSingleObsConvertData->RINEXObsHeader.Pos[Index] = pSingleObsConvertData->PosHistory[0][Index];
					}
				}
				if (pSingleObsConvertData->Rece == 0)
				{
					if (RawObasDataType < RAW_OBS_DATA_TYPE_NUM_MAX)
					{
						sprintf(FileName + (int)strlen(FileName), "%s.%02do", pGetRawObsDataTypeStr, Year);
					}
					else
					{
						sprintf(FileName + (int)strlen(FileName), ".%02do", Year);
					}
					pSingleObsConvertData->RinexObsFileFp = fopen(FileName, "wb+");
					OutRINEXObsHeader(pSingleObsConvertData->RinexObsFileFp, &pSingleObsConvertData->RINEXObsOpt, &pSingleObsConvertData->RINEXObsHeader, &pMsgConvertData->EncodeDataBuf);
				}
			}
			pMsgConvertData->Rece++;
		}
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;
	if (ObsConvertRINEX(&pMsgConvertData->RINEXDataBuf, &pSingleObsConvertData->RINEXObsOpt, pObsSatData, &pMsgConvertData->EncodeDataBuf) < 0)
	{
		return;
	}

	if ((pSingleObsConvertData->RinexObsFileFp) && (pMsgConvertData->EncodeDataBuf.Len > 0))
	{
		fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pSingleObsConvertData->RinexObsFileFp);
	}
}

static void ObsDataConvertRinex(MSG_CONVERT_DATA_T* pMsgConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData, int RawObsDataType)
{
	if (pMsgConvertData->RawObsDataType == 0)
	{
		SingleObsDataConvertRinex(pMsgConvertData, 0xFFFFFFFF, &pMsgConvertData->GlobalObsConvertData, pObsSatData);
	}
	else
	{
		SingleObsDataConvertRinex(pMsgConvertData, RawObsDataType, &pMsgConvertData->SingleObsConvertData[RawObsDataType], pObsSatData);
	}
}

static void RinexMultiObsDataConvertRinex(MSG_CONVERT_DATA_T* pMsgConvertData, RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData, GNSS_OBS_SAT_DATA_T* pObsSatData)
{
	UTC_TIME_T TempTime;
	int Year = 0;
	char FileName[512] = { 0 };
	size_t Len = strlen(pMsgConvertData->OutDir);
	const char* pGetRawObsDataTypeStr = NULL;

	if ((pRinexMultiConvertData->Rece < pRinexMultiConvertData->MaxPoshis) || (pRinexMultiConvertData->Rece == 0))
	{
		/** Close the old file and open the new one */
		if (pRinexMultiConvertData->RINEXObsHeader.TimeStart.Year != 0)
		{
			TempTime = pRinexMultiConvertData->RINEXObsHeader.TimeStart;
		}
		else
		{
			TempTime = RTCMGetUTCTime();
		}

		Year = (int)TempTime.Year % 100;
		MEMCPY(FileName, pMsgConvertData->OutDir, Len);
		sprintf(FileName + Len, "%s", pMsgConvertData->PureName);

		if (pRinexMultiConvertData->Rece == 0)
		{
			sprintf(FileName + (int)strlen(FileName), ".%02do", Year);

			pRinexMultiConvertData->RinexObsFileFp = fopen(FileName, "wb+");
			OutRINEXObsHeader(pRinexMultiConvertData->RinexObsFileFp, &pRinexMultiConvertData->RINEXObsOpt, &pRinexMultiConvertData->RINEXObsHeader, &pMsgConvertData->EncodeDataBuf);
		}

		pRinexMultiConvertData->Rece++;
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;
	if (ObsConvertRINEX(&pMsgConvertData->RINEXDataBuf, &pRinexMultiConvertData->RINEXObsOpt, pObsSatData, &pMsgConvertData->EncodeDataBuf) < 0)
	{
		return;
	}

	if ((pRinexMultiConvertData->RinexObsFileFp) && (pMsgConvertData->EncodeDataBuf.Len > 0))
	{
		fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexObsFileFp);
	}
}

/**********************************************************************//**
@brief  obs data add phase shift

@param pObsData				[In] obs data
@param pMsgConvertData		[In] message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int ObsDataAddPhaseShift(GNSS_OBS_T* pObsData)
{
	unsigned int ObsIndex;
	unsigned int ObsNum = pObsData->RangeData.ObsNum;
	RM_RANGE_DATA_T* pRMRangeData;
	unsigned int ExSystemID;
	unsigned int SignalID, PRNLock;

	if ((pObsData->RawDataType >= RAW_OBS_DATA_TYPE_RANGE_ASCII) && (pObsData->RawDataType <= RAW_OBS_DATA_TYPE_RANGECMP_1_BINARY))
	{
		for (ObsIndex = 0; ObsIndex < ObsNum; ObsIndex++)
		{
			pRMRangeData = &pObsData->RangeData.ObsData[ObsIndex];

			ExSystemID = RMRangeSystemToExSystem(GET_CH_TRACK_STATUS_SATSYSTEM(pRMRangeData->ChnTrackStatus));
			if (ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
			{
				continue;
			}

			SignalID = GET_CH_TRACK_STATUS_SIGNALTYPE(pRMRangeData->ChnTrackStatus);
			PRNLock = GET_CH_TRACK_STATUS_PRNLOCK(pRMRangeData->ChnTrackStatus);

			if (!IsAbsoluteDoubleZero(pRMRangeData->ADR))
			{
				if (PRNLock == 1) /**< new version range */
				{
					pRMRangeData->ADR -= GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
				}
				else
				{
					if (((ExSystemID == GPS) && ((SignalID == RM_RANGE_SIGNAL_GPSL2CL) || (SignalID == RM_RANGE_SIGNAL_GPSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_GPSL2CX) || (SignalID == RM_RANGE_SIGNAL_GPSL1CP))) ||
						((ExSystemID == QZS) && ((SignalID == RM_RANGE_SIGNAL_QZSSL2CL) || (SignalID == RM_RANGE_SIGNAL_QZSSL2CM) ||
												(SignalID == RM_RANGE_SIGNAL_QZSSL2CX) || (SignalID == RM_RANGE_SIGNAL_QZSSL1CP))))
					{
						pRMRangeData->ADR += GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
					else
					{
						pRMRangeData->ADR -= GetRinexPhaseAlignmentDataCycle(ExSystemID, SignalID);
					}
				}
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode obs convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeObsConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pData;
	GNSS_OBS_SAT_DATA_T* pObsSatData = &pMsgConvertData->ObsSatData;
	INT64 TimeDiff;
	int Progress;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MEMSET(pObsSatData, 0, sizeof(GNSS_OBS_SAT_DATA_T));

	ObsDataAddPhaseShift(pObsData);

	qsort(pObsData->RangeData.ObsData, pObsData->RangeData.ObsNum, sizeof(RM_RANGE_DATA_T), SatObsReorder);

	RangeDataToObsSatData(pObsData, pObsSatData);

	if (pMsgConvertData->AllFreqFlag <= 0)
	{
		SetSysSigType(pMsgConvertData, pObsSatData, RawObsDataType);
	}

	ObsDataConvertRinex(pMsgConvertData, pObsSatData, RawObsDataType);

	pMsgConvertData->ObsEpochNum++;

	if ((pMsgConvertData->InputDataType != GNSS_STRFMT_RINEX) && (pMsgConvertData->DecodeObsNum > 0))
	{
		Progress = pMsgConvertData->ObsEpochNum * 100 / pMsgConvertData->DecodeObsNum;
		PrintfProgressBar(Progress);
		SetMsgConvertProgress(Progress);
	}
	else if ((pMsgConvertData->InputDataType == GNSS_STRFMT_RINEX) && (pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeDiff > 0))
	{
		TimeDiff = UTCDiff(&pObsData->ObsTime, &pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeStart);
		Progress = (int)(TimeDiff * 100 / pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeDiff);
		PrintfProgressBar(Progress);
		SetMsgConvertProgress(Progress);
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode gnssobs convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeGNSSObsConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pData;
	GNSS_OBS_SAT_DATA_T* pObsSatData = &pMsgConvertData->ObsSatData;
	INT64 TimeDiff;
	int Progress;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MEMSET(pObsSatData, 0, sizeof(GNSS_OBS_SAT_DATA_T));

	ObsDataAddPhaseShift(pObsData);

	qsort(pObsData->GNSSObsData.ObsData, pObsData->GNSSObsData.ObsNum, sizeof(RM_EX_GNSSOBS_DATA_T), SatGNSSObsReorder);

	GNSSObsDataToObsSatData(pObsData, pObsSatData);

	if (pMsgConvertData->AllFreqFlag <= 0)
	{
		SetSysSigTypeEx(pMsgConvertData, pObsSatData, RawObsDataType);
	}

	ObsDataConvertRinex(pMsgConvertData, pObsSatData, RawObsDataType);

	pMsgConvertData->ObsEpochNum++;

	if ((pMsgConvertData->InputDataType != GNSS_STRFMT_RINEX) && (pMsgConvertData->DecodeObsNum > 0))
	{
		Progress = pMsgConvertData->ObsEpochNum * 100 / pMsgConvertData->DecodeObsNum;
		PrintfProgressBar(Progress);
		SetMsgConvertProgress(Progress);
	}
	else if ((pMsgConvertData->InputDataType == GNSS_STRFMT_RINEX) && (pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeDiff > 0))
	{
		TimeDiff = UTCDiff(&pObsData->ObsTime, &pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeStart);
		Progress = (int)(TimeDiff * 100 / pMsgConvertData->RinexInfo.ObsHeaderInfo.ObsHeader.TimeDiff);
		PrintfProgressBar(Progress);
		SetMsgConvertProgress(Progress);
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode GPS eph convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeUniEphConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData = &pMsgConvertData->GlobalObsConvertData;
	int FileIndex = GetFileIndex(RawDataType, DecodeID);
	int RinexID = GetRinexID(RawDataType, DecodeID);

	if (pMsgConvertData->RawObsDataType != 0)
	{
		pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[RawObsDataType];
	}

	if ((FileIndex < 0) || (RinexID < 0))
	{
		return -1;
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;

	UniEphConvertRINEX(RinexID, &pMsgConvertData->RINEXDataBuf, &pMsgConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);

	if (pMsgConvertData->EncodeDataBuf.Len > 0)
	{
		if (pSingleObsConvertData->RinexNavFileFp[FileIndex])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pSingleObsConvertData->RinexNavFileFp[FileIndex]);
		}

		if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH]);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode GLO eph convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeGLOEphConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData = &pMsgConvertData->GlobalObsConvertData;

	if (pMsgConvertData->RawObsDataType != 0)
	{
		pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[RawObsDataType];
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;

	GLOEphConvertRINEX(&pMsgConvertData->RINEXDataBuf, &pMsgConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);

	if (pMsgConvertData->EncodeDataBuf.Len > 0)
	{
		if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH]);
		}

		if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH]);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode antenna information convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] antenna information data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeAntennaInfoConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	RTCM_ANTENNA_INFO_T* pAntennaInfo = (RTCM_ANTENNA_INFO_T*)pData;

	if (!pData)
	{
		return -1;
	}

	ShowPosition(pAntennaInfo->Position);

	return 0;
}

/**********************************************************************//**
@brief  data decode receiver information convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] receiver information data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeReceiverInfoConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	ShowReceiverInfo(pData);

	return 0;
}

/**********************************************************************//**
@brief  data decode station position convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] station position data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeStationPositionConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType)
{
	STATION_POSITION_DATA_T* pStationPosition = (STATION_POSITION_DATA_T*)pData;

	if (!pData)
	{
		return -1;
	}

	ShowPosition(pStationPosition->Position);

	return 0;
}

/**********************************************************************//**
@brief  data decode obs convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] obs data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RinexDecodeObsConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	GNSS_OBS_T* pObsData = (GNSS_OBS_T*)pData;
	GNSS_OBS_SAT_DATA_T* pObsSatData = &pMsgConvertData->ObsSatData;
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;
	INT64 TimeDiff;
	int Progress;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MEMSET(pObsSatData, 0, sizeof(GNSS_OBS_SAT_DATA_T));

	qsort(pObsData->RangeData.ObsData, pObsData->RangeData.ObsNum, sizeof(RM_RANGE_DATA_T), SatObsReorder);

	RangeDataToObsSatData(pObsData, pObsSatData);

	if (pMsgConvertData->AllFreqFlag <= 0)
	{
//		SetSysSigType(pMsgConvertData, pObsSatData, RawObsDataType);
	}

	RinexMultiObsDataConvertRinex(pMsgConvertData, pRinexMultiConvertData, pObsSatData);

	if ((pMsgConvertData->InputDataType == GNSS_STRFMT_RINEX) && (pRinexMultiConvertData->RINEXObsHeader.TimeDiff > 0))
	{
		TimeDiff = UTCDiff(&pObsData->ObsTime, &pRinexMultiConvertData->RINEXObsHeader.TimeStart);
		Progress = (int)(TimeDiff * 100 / pRinexMultiConvertData->RINEXObsHeader.TimeDiff);
		PrintfProgressBar(Progress);
		SetMsgConvertProgress(Progress);
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode Multi eph convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RinexDecodeMultiEphConvertRinex(int RawDataType, int RinexID, void* pSrcObj, void* pData)
{
	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;
	int FileIndex = GetFileIndex(RawDataType, RinexID);

	if ((FileIndex < 0) || (RinexID < 0))
	{
		return -1;
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;

	if (FileIndex == MSG_CONVERT_FILE_INDEX_GLOEPH)
	{
		GLOEphConvertRINEX(&pMsgConvertData->RINEXDataBuf, &pRinexMultiConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);
	}
	else
	{
		UniEphConvertRINEX(RinexID, &pMsgConvertData->RINEXDataBuf, &pRinexMultiConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);
	}

	if (pMsgConvertData->EncodeDataBuf.Len > 0)
	{
		if (pRinexMultiConvertData->RinexNavFileFp[FileIndex])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[FileIndex]);
		}

		if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH]);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode uni eph convert

@param pSrcObj				[In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RinexDecodeUniEphConvertRinex(int RawDataType, int RinexID, void* pSrcObj, void* pData)
{
	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;
	int FileIndex = GetFileIndex(RawDataType, RinexID);

	if ((FileIndex < 0) || (RinexID < 0))
	{
		return -1;
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;

	UniEphConvertRINEX(RinexID, &pMsgConvertData->RINEXDataBuf, &pRinexMultiConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);

	if (pMsgConvertData->EncodeDataBuf.Len > 0)
	{
		if (pRinexMultiConvertData->RinexNavFileFp[FileIndex])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[FileIndex]);
		}

		if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH]);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  data decode GLO eph convert

@param pSrcObj				 [In] pointer to source object
@param pData				[In] eph data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RinexDecodeGLOEphConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;

	if ((!pSrcObj) || (!pData))
	{
		return -1;
	}

	pMsgConvertData->EncodeDataBuf.Len = 0;

	GLOEphConvertRINEX(&pMsgConvertData->RINEXDataBuf, &pRinexMultiConvertData->RINEXEphOpt, pData, &pMsgConvertData->EncodeDataBuf);

	if (pMsgConvertData->EncodeDataBuf.Len > 0)
	{
		if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH]);
		}

		if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
		{
			fwrite(pMsgConvertData->EncodeDataBuf.pBaseAddr, pMsgConvertData->EncodeDataBuf.Len, 1, pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH]);
		}
	}

	return 0;
}

#endif
