/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    TimeFuncs.h
@author  CHC
@date    2025/10/15
@brief   time-related operation functions

**************************************************************************/
#include <time.h>
#include <math.h>
#include "TimeDefines.h"

static int s_UTCDaysAcc[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
const static double s_GPSTime0[] = { 1980, 1, 6, 0, 0, 0 }; /**< gps time reference */
const static QC_TIME_T s_QCTimeFormatGPSTime0 = { 315964800,0 };/**< QC_TIME_T format of gpst0 */

typedef struct _LEAP_SEC_INFO_T
{
	int LeapSecMs;
	UTC_TIME_T UTCTime;
} LEAP_SEC_INFO_T;

#ifdef MSG_CONVERT

static const LEAP_SEC_INFO_T s_GPSLeapSecsMap[] = {
	{
		.LeapSecMs = 18000,
		{
			.UTCFlag = 0,
			.Year = 2017,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 17000,
		{
			.UTCFlag = 0,
			.Year = 2015,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 16000,
		{
			.UTCFlag = 0,
			.Year = 2012,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 15000,
		{
			.UTCFlag = 0,
			.Year = 2009,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 14000,
		{
			.UTCFlag = 0,
			.Year = 2006,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 13000,
		{
			.UTCFlag = 0,
			.Year = 1999,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 12000,
		{
			.UTCFlag = 0,
			.Year = 1997,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 11000,
		{
			.UTCFlag = 0,
			.Year = 1996,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 10000,
		{
			.UTCFlag = 0,
			.Year = 1994,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 9000,
		{
			.UTCFlag = 0,
			.Year = 1993,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 8000,
		{
			.UTCFlag = 0,
			.Year = 1992,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 7000,
		{
			.UTCFlag = 0,
			.Year = 1991,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 6000,
		{
			.UTCFlag = 0,
			.Year = 1990,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 5000,
		{
			.UTCFlag = 0,
			.Year = 1988,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 4000,
		{
			.UTCFlag = 0,
			.Year = 1985,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 3000,
		{
			.UTCFlag = 0,
			.Year = 1983,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 2000,
		{
			.UTCFlag = 0,
			.Year = 1982,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 1000,
		{
			.UTCFlag = 0,
			.Year = 1981,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
};
#endif

static const LEAP_SEC_INFO_T s_BDSLeapSecsMap[] = {
	{
		.LeapSecMs = 4000,
		{
			.UTCFlag = 0,
			.Year = 2017,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 3000,
		{
			.UTCFlag = 0,
			.Year = 2015,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 2000,
		{
			.UTCFlag = 0,
			.Year = 2012,
			.Month = 7,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
	{
		.LeapSecMs = 1000,
		{
			.UTCFlag = 0,
			.Year = 2009,
			.Month = 1,
			.Day = 1,
			.Hour = 0,
			.Minute = 0,
			.Second = 0,
			.Millisecond = 0,
		},
	},
};

/**********************************************************************//**
@brief transfer GLONASST to UTC

@param LeapYears   [In]  four-year interval number staring from 1992
@param DayNumber   [In]  Days number of year, based on 1
@param DayMsCount  [In]  Ms Count of day
@param pUtcTime    [Out] UTC time

@author CHC
@date 2022/11/23
@note
**************************************************************************/
void GLOTimeToUTC(int LeapYears, int DayNumber, int DayMsCount, UTC_TIME_T* pUtcTime)
{
	int iMonth = 0;
	int Seconds, LeapDay = 0;
	int FirstYear = 0; /**< first year number of 4years - interval */
	int IncludeLeapYear = 0;
	int FirstDoy = 0; /**< total day number of first year */
	int UTCYears = 0;

	DayMsCount -= 10800000;
	if (DayMsCount < 0)
	{
		DayMsCount += 86400000;
		DayNumber--;
	}
	Seconds = DayMsCount / 1000;
	pUtcTime->Millisecond = DayMsCount - Seconds * 1000;
	UTCYears = LeapYears * 4;

	FirstYear = UTCYears + 1992;
	if ((!(FirstYear % 4) && (FirstYear % 100)) || !(FirstYear % 400))
	{
		IncludeLeapYear = 1; /**< first year of four - year interval is a intercalary year */
		FirstDoy = 366;
	}
	else
	{
		FirstDoy = 365;
	}

	DayNumber--; /**< change DayNumber's base from 1 to 0 */
	if (DayNumber >= (FirstDoy + 365 * 2))
	{
		DayNumber -= (FirstDoy + 365 * 2);
		UTCYears += 3;
	}
	else if (DayNumber >= (FirstDoy + 365))
	{
		DayNumber -= (FirstDoy + 365);
		UTCYears += 2;
	}
	else if (DayNumber >= FirstDoy)
	{
		DayNumber -= FirstDoy;
		UTCYears++;
	}
	else if (IncludeLeapYear)
	{
		if (DayNumber >= 60)
		{
			DayNumber--;
		}
		else if (DayNumber == 59)
		{
			LeapDay = 1;
		}
	}

	for (iMonth = 1; iMonth < 12; iMonth++)
	{
		if (DayNumber < s_UTCDaysAcc[iMonth])
			break;
	}

	if (LeapDay)
	{
		pUtcTime->Month = 2;
		pUtcTime->Day = 29;
	}
	else
	{
		pUtcTime->Month = iMonth;
		pUtcTime->Day = DayNumber - (s_UTCDaysAcc[iMonth - 1] - 1);
	}
	pUtcTime->Year = 1992 + UTCYears;
	pUtcTime->Hour = Seconds / 3600;
	Seconds -= pUtcTime->Hour * 3600;
	pUtcTime->Minute = Seconds / 60;
	pUtcTime->Second = Seconds - pUtcTime->Minute * 60;
}

/**********************************************************************//**
@brief transfer UTC to GLONASST

@param pUtcTime   [In]  UTC time
@param LeapYears  [Out] four-year interval number starting from 1996, based on 1
@param DayNumber  [Out] Days number of year, based on 1
@param DayMsCount [Out] Ms Count of day

@author CHC
@date 2022/11/23
@note
History:
**************************************************************************/
void UTCToGLOTime(UTC_TIME_T* pUtcTime, int* pLeapYears, int* pDayNumber, int* pDayMsCount)
{
	int FirstYear = 0;
	int MsCountOfDay = 0;
	int DayCount = 0;
	int YearCount = 0;

	/** Get Day's MsCount of GLONASS Time */
	MsCountOfDay = (((pUtcTime->Hour * 60) + pUtcTime->Minute) * 60 + pUtcTime->Second) * 1000 + pUtcTime->Millisecond + 10800000;
	*pDayMsCount = MsCountOfDay % 86400000;

	/** year number since 1992 */
	YearCount = pUtcTime->Year - 1992;
	DayCount = s_UTCDaysAcc[pUtcTime->Month - 1] + pUtcTime->Day - 1 + (MsCountOfDay / 86400000);

	/** Get first year number of this 4 - years interval */
	FirstYear = (YearCount / 4) * 4 + 1992;
	if ((!(FirstYear % 4) && (FirstYear % 100)) || !(FirstYear % 400)) /**< first year is leap year */
	{
		if (((YearCount % 4) != 0) || (pUtcTime->Month > 2))
		{
			DayCount++;
		}
	}

	DayCount += (YearCount % 4) * 365;
	*pDayNumber = DayCount + 1;
	*pLeapYears = YearCount / 4;

	if (*pDayNumber > 1461)
	{
		*pDayNumber -= 1461;
		*pLeapYears += 1;
	}
}

/**********************************************************************//**
@brief Convert GPS Time to UTC Time
	This program handles the date from Jan. 1, 1980 00:00:00.00 UTC
	till year 2099 !!! (do not treat year 2100 as common year)

@param GPSWeek        [In]  GPS Week number
@param WeekMsCount    [In]  GPS MsCount
@param pGPSLeapSecMs  [In]  GPS Leap Second in Ms
@param pUTCTime       [Out] UTC Time converted from GPS Time

@author CHC
@date 2022/11/23
@note
History:
-1. 2024/05/07, modify start time from 1984.01.01 00:00:00 to 1980.01.01 00:00:00
**************************************************************************/
void GPSTimeToUTC(int GPSWeek, int WeekMsCount, int* pGPSLeapSecMs, UTC_TIME_T* pUtcTime)
{
	int LeapYears, TotalDays, MsSeconds;
	int LeapSecondMs = 0;

	TotalDays = (GPSWeek - 1) * 7;
	MsSeconds = WeekMsCount + 604800000;

	if (pGPSLeapSecMs)
	{
		LeapSecondMs = *pGPSLeapSecMs;
		if (LeapSecondMs < 0)
		{
			LeapSecondMs = GPS_LEAP_SECS_MS;
		}
	}
	else
	{
		LeapSecondMs = GPS_LEAP_SECS_MS;
	}

	MsSeconds -= LeapSecondMs;
	TotalDays += MsSeconds / 86400000;
	MsSeconds %= 86400000;

	TotalDays += 5; /**< move StartTime from 1980.01.06 00:00:00 --> 1980.01.01 00:00:00 */
	LeapYears = TotalDays / (366 + 365 * 3);
	TotalDays -= LeapYears * (366 + 365 * 3);

	GLOTimeToUTC(LeapYears - 3, TotalDays + 1, MsSeconds + 10800000, pUtcTime);
}

/**********************************************************************//**
@brief Convert UTC Time to GPS Time

@param pUtcTime       [In]  UTC Time structure
@param pGpsLeapSecMs  [In]  pointer to GPS Second in Ms
@param pGpsWeek       [Out] GPS Week number
@param pWeekMsCount   [Out] GPS Time MsCount

@author CHC
@date 2022/11/23
@note
History:
**************************************************************************/
void UTCToGPSTime(UTC_TIME_T* pUtcTime, int* pGPSLeapSecMs, int* pGPSWeek, int* pWeekMsCount)
{
	int LeapYears, TotalDays, MsSeconds;
	int LeapSecondMs = 0;

	/** convert UTC to GLONASS Time */
	UTCToGLOTime(pUtcTime, &LeapYears, &TotalDays, &MsSeconds);

	/** convert GLOT to GPST */
	MsSeconds -= 10800000;
	TotalDays--;	/**< convert to 1 based day count */

	/** Get GPS LeapSecond */
	if (pGPSLeapSecMs)
	{
		LeapSecondMs = *pGPSLeapSecMs;
		if (LeapSecondMs < 0)
		{
			LeapSecondMs = GPS_LEAP_SECS_MS;
		}
	}
	else
	{
		LeapSecondMs = GPS_LEAP_SECS_MS;
	}

	MsSeconds += LeapSecondMs;
	if (MsSeconds >= 86400000)
	{
		MsSeconds -= 86400000;
		TotalDays++;
	}
	else if (MsSeconds < 0)
	{
		MsSeconds += 86400000;
		TotalDays--;
	}
	TotalDays += (LeapYears + 2) * (366 + 365 * 3);
	*pGPSWeek = TotalDays / 7 + 208;
	*pWeekMsCount = (TotalDays % 7) * 86400000 + MsSeconds;
}

/**********************************************************************//**
@brief Convert UTC Time to BDS Time

@param pUtcTime      [In]  UTC Time structure
@param pBDSLeapSecMs [In]  pointer to BDS LeapSecond in Ms
@param pBDSWeek      [Out] BDS Week number
@param pWeekMsCount  [Out] BDS Time MsCount

@author CHC
@date 2022/11/23
@note
History:
**************************************************************************/
void UTCToBDSTime(UTC_TIME_T* pUtcTime, int* pBDSLeapSecMs, int* pBDSWeek, int* pWeekMsCount)
{
	int LeapYears, TotalDays, MsSeconds;
	int LeapSecondMs = 0;

	/** Convert UTC to GLOT */
	UTCToGLOTime(pUtcTime, &LeapYears, &TotalDays, &MsSeconds);
	MsSeconds -= 10800000;
	TotalDays--;	/**< convert to 1 based day count*/

	/** Get BDS LeapSecond in ms */
	if (pBDSLeapSecMs)
	{
		LeapSecondMs = *pBDSLeapSecMs;
		if (LeapSecondMs == 0)
		{
			LeapSecondMs = BDS_LEAP_SECS_MS;
		}
	}
	else
	{
		LeapSecondMs = BDS_LEAP_SECS_MS;
	}

	MsSeconds += LeapSecondMs;
	if (MsSeconds >= 86400000)
	{
		MsSeconds -= 86400000;
		TotalDays++;
	}
	else if (MsSeconds < 0)
	{
		MsSeconds += 86400000;
		TotalDays--;
	}

	TotalDays += (LeapYears - 3) * (366 + 365 * 3) - 731;
	*pBDSWeek = TotalDays / 7;
	*pWeekMsCount = (TotalDays % 7) * 86400000 + MsSeconds;
}

const static double s_VeriQCLeaps[64][7] = {     /* leap seconds {y,m,d,h,m,s,utc-gpst,...} */
	{2017,1,1,0,0,0,-18},
	{2015,7,1,0,0,0,-17},
	{2012,7,1,0,0,0,-16},
	{2009,1,1,0,0,0,-15},
	{2006,1,1,0,0,0,-14},
	{1999,1,1,0,0,0,-13},
	{1997,7,1,0,0,0,-12},
	{1996,1,1,0,0,0,-11},
	{1994,7,1,0,0,0,-10},
	{1993,7,1,0,0,0, -9},
	{1992,7,1,0,0,0, -8},
	{1991,1,1,0,0,0, -7},
	{1990,1,1,0,0,0, -6},
	{1988,1,1,0,0,0, -5},
	{1985,7,1,0,0,0, -4},
	{1983,7,1,0,0,0, -3},
	{1982,7,1,0,0,0, -2},
	{1981,7,1,0,0,0, -1},
	{0}
};

int CopyQCTime(const QC_TIME_T* pSrc, QC_TIME_T* pDst)
{
	if (!pSrc || !pDst)
	{
		return -1;
	}
	pDst->Time = pSrc->Time;
	pDst->Sec = pSrc->Sec;
	return 0;
}

extern QC_TIME_T QCTimeAdd(const QC_TIME_T* pTime1, const double Sec)
{
	QC_TIME_T targetTime = { 0 };
	int int_sec = 0;
	float deci_sec = 0.0;

	int_sec = (int)(Sec / 1);
	deci_sec = (float)(Sec - int_sec);
	targetTime.Time = pTime1->Time + int_sec;
	targetTime.Sec = pTime1->Sec + deci_sec;
	return targetTime;
}

double QCTimeMinusTime(const QC_TIME_T* pTime1, const QC_TIME_T* pTime2, QC_TIME_T* pDelta)
{
	QC_TIME_T tmp;
	double tt;
	tmp.Sec = pTime1->Sec - pTime2->Sec;
	tt = floor(tmp.Sec);
	tmp.Time = pTime1->Time - pTime2->Time + (long long)tt;
	tmp.Sec -= tt;
	if (pDelta) *pDelta = tmp;
	return tmp.Time + tmp.Sec;
}

extern double QCTimeDiff(const QC_TIME_T* pTime1, const QC_TIME_T* pTime2)
{
	return (double)(pTime1->Time - pTime2->Time) + pTime1->Sec - pTime2->Sec;
}

/* time to calendar day/time ---------------------------------------------------
* convert gtime_t struct to calendar day/time
* args   : QC_TIME_T t      I   QC_TIME_T struct
*          double *ep       O   day/time {year,month,day,hour,min,sec}
* return : none
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
int QCTime2Epoch(QC_TIME_T Time, double* pEpoch)
{
	const int32_t mday[] = { /* # of days in a month */
		31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
		31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
	};
	int32_t days, sec, mon, day;

	/* leap year if year%4==0 in 1901-2099 */
	days = (int32_t)(Time.Time / 86400);
	sec = (int32_t)(Time.Time - (int64_t)days * 86400);
	for (day = days % 1461, mon = 0; mon < 48; mon++)
	{
		if (day >= mday[mon]) day -= mday[mon]; else break;
	}
	pEpoch[0] = 1970 + (double)(days / 1461) * 4 + mon / 12; pEpoch[1] = (double)(mon % 12) + 1; pEpoch[2] = (double)day + 1;
	pEpoch[3] = sec / 3600; pEpoch[4] = sec % 3600 / 60; pEpoch[5] = sec % 60 + Time.Sec;
	return 0;
}

/**
 *  @breif  gpstime convert to time
 *  @return QC_TIME_T
 */
extern QC_TIME_T GPSTime2QCTime(int Week, double Sec)
{
	QC_TIME_T t0;
	QC_TIME_T t;
	/* for the time start with 0 */
	if (Week == 0)
	{
		t0.Time = 0;
		t0.Sec = Sec;
		return t0;
	}

	t = Epoch2QCTime(s_GPSTime0);

	if (Sec < -1E9 || 1E9 < Sec) Sec = 0.0;
	t.Time += (time_t)86400 * 7 * Week + (int)Sec;
	t.Sec = Sec - (int)Sec;
	return t;
}

extern double QCTime2DOY(QC_TIME_T* pTime, int* pYear)
{
	double ep[6] = { 0 };
	QC_TIME_T t0 = { 0 };

	QCTime2Epoch(*pTime, ep);
	ep[1] = ep[2] = 1.0; ep[3] = ep[4] = ep[5] = 0.0;
	t0 = Epoch2QCTime(ep);
	return QCTimeDiff(pTime, &t0) / 86400.0 + 1.0;
}

/*****************************************************************************//**
@brief utc to epoch

@param pUTCTime        [In] UTC TIME

@return epoch time

@author CHC
@date  2023/05/25
@note
*********************************************************************************/
unsigned int UTC2Epoch(UTC_TIME_T* pUTCTime)
{
	const unsigned int DayOfYear[] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
	unsigned int TempTime = 0;
	unsigned int Days, Year = pUTCTime->Year, Month = pUTCTime->Month, Day = pUTCTime->Day;

	if (Year < 1970 || 2099 < Year || Month < 1 || 12 < Month)
		return TempTime;

	/** leap year if year%4==0 in 1901-2099 */
	Days = (Year - 1970) * 365 + (Year - 1969) / 4 + DayOfYear[Month - 1] + Day - 2 + (Year % 4 == 0 && Month >= 3 ? 1 : 0);
	TempTime = (long long)Days * 86400 + pUTCTime->Hour * 3600 + pUTCTime->Minute * 60 + pUTCTime->Second;

	return TempTime;
}

/*****************************************************************************//**
@brief gps time to epoch

@param week        [In] gps week
@param sec         [In] gps sec

@return epoch time

@author CHC
@date  2023/05/25
@note
*********************************************************************************/
unsigned int GPSTime2Epoch(int Week, int Sec)
{
	UTC_TIME_T GPSTime0 = {/**< gps time reference */
		.Year = 1980,
		.Month = 1,
		.Day = 6,
		.Hour = 0,
		.Minute = 0,
		.Second = 0 };

	unsigned int TempTime = UTC2Epoch(&GPSTime0);

	TempTime += 86400 * 7 * Week + Sec;

	return TempTime;
}


QC_TIME_T UTC2QCTime(UTC_TIME_T* pUTCTime)
{
	const int Doy[] = { 1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
	QC_TIME_T TempTime = { 0 };
	int Days, Sec, Year = pUTCTime->Year, Mon = pUTCTime->Month, Day = pUTCTime->Day;

	if (Year < 1970 || 2099 < Year || Mon < 1 || 12 < Mon)
	{
		return TempTime;
	}

	/* leap year if year%4==0 in 1901-2099 */
	Days = (Year - 1970) * 365 + ((Year - 1969) >> 2) + Doy[Mon - 1] + Day - 2 + ((Year & 0x03) == 0 && Mon >= 3 ? 1 : 0);
	Sec = pUTCTime->Second;
	TempTime.Time = (long long)Days * 86400 + (int)pUTCTime->Hour * 3600 + (int)pUTCTime->Minute * 60 + Sec;
	TempTime.Sec = (double)pUTCTime->Millisecond / 1000;

	return TempTime;
}

/**********************************************************************//**
@brief epoch of ephemeris to utc time struct

@param Epoch		 [In]  epoch of ephemeris (gpst)
@param pUTCTime		 [Out] UTC Time struct

@return 0:success

@author CHC
@date 2022/05/16
@note
**************************************************************************/
int Epoch2UTC(unsigned int Epoch, UTC_TIME_T* pUTCTime)
{
	const int MonthDay[] = { /**< # of days in a month */
		31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
		31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
	};
	int Days, Sec, Month, Day;

	/** leap year if year%4==0 in 1901-2099 */
	Days = (int)(Epoch / 86400);
	Sec = (int)(Epoch - Days * 86400);
	for (Day = Days % 1461, Month = 0; Month < 48; Month++)
	{
		if (Day >= MonthDay[Month])
		{
			Day -= MonthDay[Month];
		}
		else
		{
			break;
		}
	}

	pUTCTime->Year = 1970 + Days / 1461 * 4 + Month / 12;
	pUTCTime->Month = Month % 12 + 1;
	pUTCTime->Day = Day + 1;
	pUTCTime->Hour = Sec / 3600;
	pUTCTime->Minute = Sec % 3600 / 60;
	pUTCTime->Second = Sec % 60;
	pUTCTime->Millisecond = 0;

	return 0;
}

/**********************************************************************//**
@brief  convert TIME_T struct to week and tow in gps time

@param Time		 [In] QC_TIME_T struct
@param pWeek	 [In&Out] week number in gps time (NULL: no output)

@retval time of week in gps time (s)

@author CHC
@date 2023/04/10
@note
**************************************************************************/
double QCTime2GPSTime(QC_TIME_T Time, int* pWeek)
{
	QC_TIME_T TempTime = Epoch2QCTime(s_GPSTime0);
	long long Sec = Time.Time - TempTime.Time;
	int TempWeek = (int)(Sec / (86400 * 7));

	if (Time.Time + Time.Sec < 604800.0)
	{
		if (pWeek)
		{
			*pWeek = 0;
		}
		return Time.Time + Time.Sec;
	}

	if (pWeek)
	{
		*pWeek = TempWeek;
	}

	return (double)(Sec - (double)TempWeek * 86400 * 7) + Time.Sec;
}

/* convert calendar day/time to time -------------------------------------------
* convert calendar day/time to gtime_t struct
* args   : double *pEpoch       I   day/time {year,month,day,hour,min,sec}
* return : gtime_t struct
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
extern QC_TIME_T Epoch2QCTime(const double* pEpoch)
{
	const int doy[] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
	QC_TIME_T time = { 0 };
	int days, sec, year = (int)pEpoch[0], mon = (int)pEpoch[1], day = (int)pEpoch[2];

	if (year < 1970 || 2099 < year || mon < 1 || 12 < mon) return time;

	/* leap year if year%4==0 in 1901-2099 */
	days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);
	sec = (int)floor(pEpoch[5]);
	time.Time = (time_t)days * 86400 + (int)pEpoch[3] * 3600 + (int)pEpoch[4] * 60 + sec;
	time.Sec = pEpoch[5] - sec;
	return time;
}

/********************************************************************************************
@brief   the two times are same (the times must both have the same source)

@param   FirstTime[In]      first time
@param   SecondTime[In]     second time

@author  CHC

@return  TRUE:time same; FALSE:different
*********************************************************************************************/
extern BOOL IsQCTimeSame(QC_TIME_T FirstTime, QC_TIME_T SecondTime)
{
	const unsigned int* pIntFirst = (const unsigned int*)&(FirstTime.Sec);
	const unsigned int* pIntSecond = (const unsigned int*)&(SecondTime.Sec);
	return (FirstTime.Time == SecondTime.Time &&
		(*(pIntFirst++) == *(pIntSecond++) && *(pIntFirst) == *(pIntSecond)));
}

/*********************************************************************
@brief convert GPS time to string
@param  QC_TIME_T *pTime          [I]                 GPS time
@param  char *pTimeString         [IO]                string
@param  int  SecDecimal           [I]                 N decimal places if sec
@return NULL
@author CHC
@date   28 February 2023
**********************************************************************/
extern int QCTime2String(QC_TIME_T* pTime, char* pTimeString, int SecDecimal)
{
	double ep[6];

	QC_TIME_T t = { 0 };

	if (!pTime || !pTimeString)
	{
		return -1;
	}
	else
	{
		t.Time = pTime->Time;
		t.Sec = pTime->Sec;
		if (SecDecimal < 0)SecDecimal = 0; else if (SecDecimal > 12) SecDecimal = 12;
		if (1.0 - t.Sec < 0.5 / pow(10.0, SecDecimal)) { t.Time++; t.Sec = 0.0; };
		QCTime2Epoch(t, ep);
		sprintf(pTimeString, "%04.0f/%02.0f/%02.0f %02.0f:%02.0f:%0*.*f", ep[0], ep[1], ep[2],
			ep[3], ep[4], SecDecimal <= 0 ? 2 : SecDecimal + 3, SecDecimal <= 0 ? 0 : SecDecimal, ep[5]);

		return 0;
	}
}

/**********************************************************************//**
@brief UTC Time add LeapSecMs

@todo shall be removed !!! use PVT common RTCM is all about GPStime why need UTC?

@param pUTCTime     [In]  UTC Time structure
@param SecondMs     [In]  add to ms

@author CHC
@date 2023/05/08
@note
History:
**************************************************************************/
UTC_TIME_T UTCAdd(UTC_TIME_T* pUTCTime, int Millisecond)
{
	int LeapYears, TotalDays, MsSeconds;
	UTC_TIME_T TempUtcTime = { 0 };

	if (pUTCTime->Year <= 0)
	{
		return TempUtcTime;
	}

	/** convert UTC to GLONASS Time */
	UTCToGLOTime(pUTCTime, &LeapYears, &TotalDays, &MsSeconds);

	MsSeconds += Millisecond;
	if (MsSeconds >= 86400000)
	{
		MsSeconds -= 86400000;
		TotalDays++;
	}
	else if (MsSeconds < 0)
	{
		MsSeconds += 86400000;
		TotalDays--;
	}

	GLOTimeToUTC(LeapYears, TotalDays, MsSeconds, &TempUtcTime);

	return TempUtcTime;
}

/**********************************************************************//**
@brief UTC Time to millisecond

@param pUTCTime     [In] UTC Time structure

@author CHC
@date 2023/05/08
@note start time: 1970-1-1 00:00:00
History:
**************************************************************************/
INT64 UTCToMillisecond(UTC_TIME_T* pUTCTime)
{
	const unsigned int DayOfYear[] = { 1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
	unsigned int Year = pUTCTime->Year;
	unsigned int Month = pUTCTime->Month;
	unsigned int Day = pUTCTime->Day;
	unsigned int Days;

	if ((Year < 1970) || (2099 < Year) || (Month < 1) || (12 < Month) || (31 < Day))
	{
		return 0;
	}

	Days = (Year - 1970) * 365 + ((Year - 1969) >> 2) + DayOfYear[Month - 1] + Day - 2 + ((((Year & 0x03) == 0) && (Month >= 3)) ? 1 : 0);

	return ((INT64)Days * 86400 + (INT64)pUTCTime->Hour * 3600 + (INT64)pUTCTime->Minute * 60 + pUTCTime->Second) * 1000 + pUTCTime->Millisecond;
}

/**********************************************************************//**
@brief  difference between UTC_TIME_T structs

@param pTime1         [In] UTC_TIME_T porint
@param pTime2         [In] UTC_TIME_T porint

@retval Millisecond

@author CHC
@date 2023/04/10
@note
**************************************************************************/
INT64 UTCDiff(UTC_TIME_T* pTime1, UTC_TIME_T* pTime2)
{
	return UTCToMillisecond(pTime1) - UTCToMillisecond(pTime2);
}

/**********************************************************************//**
@brief  get GPS leap second Milliseconds

@param pUTCTime       [In] UTC time

@retval leap seconds

@author CHC
@date 2023/04/10
@note
**************************************************************************/
int GetGPSLeapSecondMs(UTC_TIME_T* pUTCTime)
{
#ifdef MSG_CONVERT
	int Index;

	if (!pUTCTime)
	{
		return -1;
	}

	for (Index = 0; Index < (sizeof(s_GPSLeapSecsMap) / sizeof(s_GPSLeapSecsMap[0])); Index++)
	{
		if (UTCDiff(pUTCTime, (UTC_TIME_T*)&s_GPSLeapSecsMap[Index].UTCTime) >= 0)
		{
			return (s_GPSLeapSecsMap[Index].LeapSecMs);
		}
	}

	return 0;
#elif (defined _HW_S2C_) || (defined _HW_XJ2_) || (defined _HW_ZYNQ_)
	RECEIVER_INFO_T* pReceiverInfo = GetReceiverInfo(PVT_MAJOR_ID);

	return pReceiverInfo->GPSLeapSecondMs;
#else
	return GPS_LEAP_SECS_MS;
#endif
}

/**********************************************************************//**
@brief  get GPS leap second Milliseconds

@param pUTCTime       [In] UTC time

@retval leap seconds

@author CHC
@date 2023/04/10
@note
**************************************************************************/
int GetGPSLeapSecondMsByGPSTime(int GPSWeek, int WeekMsCount)
{
	int GPSLeapSecMs = 0;
	UTC_TIME_T UTCTime;

	GPSTimeToUTC(GPSWeek, WeekMsCount, &GPSLeapSecMs, &UTCTime);

	return GetGPSLeapSecondMs(&UTCTime);
}

/**********************************************************************//**
@brief  get BDS leap second Milliseconds

@param pUTCTime       [In] UTC time

@retval leap seconds

@author CHC
@date 2023/04/10
@note
**************************************************************************/
int GetBDSLeapSecondMs(UTC_TIME_T* pUTCTime)
{
	int Index;

	if (!pUTCTime)
	{
		return -1;
	}

	for (Index = 0; Index < (sizeof(s_BDSLeapSecsMap) / sizeof(s_BDSLeapSecsMap[0])); Index++)
	{
		if (UTCDiff(pUTCTime, (UTC_TIME_T*)&s_BDSLeapSecsMap[Index].UTCTime) >= 0)
		{
			return (s_BDSLeapSecsMap[Index].LeapSecMs);
		}
	}

	return 0;
}


/**********************************************************************//**
@brief convert QC_TIME_T to UTC_TIME_T

@param pQC       [In]     QC time
@param pUTC      [In]     UTC time

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
void QCTimeToUTCTime(QC_TIME_T* pQC, UTC_TIME_T* pUTC)
{
	int Week = 0;
	int GPSLeapSecMs = 0;
	double Sec = 0;

	Sec = QCTime2GPSTime(*pQC, &Week);
	GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(Week, (int)(Sec * 1000.0));

	GPSTimeToUTC(Week, (int)(Sec * 1000.0), &GPSLeapSecMs, pUTC);
}