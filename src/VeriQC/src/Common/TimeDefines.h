/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    TimeDefines.h
@author  CHC
@date    2025/10/15
@brief   UTC time defination and time-related operation functions

**************************************************************************/
#ifndef _TIME_DEFINES_H_
#define _TIME_DEFINES_H_

#include "DataTypes.h"
#include "VeriQC.h"

#define GPS_LEAP_SECS_MS			 18000			/**< default GPS leap second, in ms */
#define BDS_LEAP_SECS_MS			  4000			/**< default BDS leap second, in ms */

/** UTC time quality*/
typedef enum _UTC_QAULITY_E
{
	UTC_Q_UNKNOWN = 0,
	UTC_Q_EX_COARSE = 1,	/**< external set with low accuracy. e.g. RTC */
	UTC_Q_EX_ACCURATE = 2,	/**< external set with high accuracy. e.g. NTP */
	UTC_Q_COARSE = 3,	    /**< baseband derived with low accuracy. e.g. initialized by GNSS, then GNSS is blocked, PVT not update*/
	UTC_Q_ACCURATE = 4,	    /**< GNSS derived */

	UTC_Q_MAX = 0xFFFFFFFF
}UTC_QAULITY_E;

typedef struct _UTC_TIME_T
{
	UTC_QAULITY_E UTCFlag;	/**< 0- unknown;1-Accurate; 2 External Get (RTC/ NTP) */
	int Year;
	int Month;		        /**< 1 based */
	int Day;
	int Hour;
	int Minute;
	int Second;
	int Millisecond;
} UTC_TIME_T;

/** Common Funcs */

unsigned int UTC2Epoch(UTC_TIME_T* pUTCTime);
unsigned int GPSTime2Epoch(int week, int sec);
int Epoch2UTC(unsigned int Epoch, UTC_TIME_T* pUTCTime);
QC_TIME_T UTC2QCTime(UTC_TIME_T* pUTCTime);

UTC_TIME_T UTCAdd(UTC_TIME_T* pUTCTime, int Millisecond);
INT64 UTCToMillisecond(UTC_TIME_T* pUTCTime);
INT64 UTCDiff(UTC_TIME_T* pTime1, UTC_TIME_T* pTime2);
int GetGPSLeapSecondMs(UTC_TIME_T* pUTCTime);
int GetGPSLeapSecondMsByGPSTime(int GPSWeek, int WeekMsCount);
int GetBDSLeapSecondMs(UTC_TIME_T* pUTCTime);

QC_TIME_T Epoch2QCTime(const double* pEpoch);
extern int QCTime2Epoch(QC_TIME_T Time, double* pEpoch);
void QCTimeToUTCTime(QC_TIME_T* pQC, UTC_TIME_T* pUTC);
double QCTime2GPSTime(QC_TIME_T Time, int* pWeek);
extern QC_TIME_T GPSTime2QCTime(int Week, double Sec);
extern QC_TIME_T QCTimeAdd(const QC_TIME_T* pTime, const double Sec);

extern double QCTimeMinusTime(const QC_TIME_T* pTime1, const QC_TIME_T* pTime2, QC_TIME_T* pDelta);
extern double QCTimeDiff(const QC_TIME_T* pTime1, const QC_TIME_T* pTime2);
extern double QCTime2DOY(QC_TIME_T* pTime, int* pYear);
extern BOOL IsQCTimeSame(QC_TIME_T FirstTime, QC_TIME_T SecondTime);
extern int CopyQCTime(const QC_TIME_T* pSrc, QC_TIME_T* pDst);
extern int QCTime2String(QC_TIME_T* pTime, char* pTimeString, int SecDecimal);

void GLOTimeToUTC(int LeapYears, int DayNumber, int DayMsCount, UTC_TIME_T* pUTCTime);
void UTCToGLOTime(UTC_TIME_T* pUTCTime, int* pLeapYears, int* pDayNumber, int* pDayMsCount);
void GPSTimeToUTC(int GPSWeek, int WeekMsCount, int* pGPSLeapSecMs, UTC_TIME_T* pUTCTime);
void UTCToGPSTime(UTC_TIME_T* pUTCTime, int* pGPSLeapSecMs, int* pGPSWeek, int* pWeekMsCount);
void UTCToBDSTime(UTC_TIME_T* pUTCTime, int* pBDSLeapSecMs, int* pBDSWeek, int* pWeekMsCount);

#endif /**< _TIME_DEFINES_H_ */