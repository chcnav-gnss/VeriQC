/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RTCM3Decoder.c
@author CHC
@date   2023/03/09
@brief

**************************************************************************/

#include "RTCM2Decoder.h"
#include "RTCMDefines.h"
#include "RTCMDataTypes.h"
#include "RTCMSupportFunc.h"


#include "HAL/DriverCycleDataBuffer.h"

#include "Common/CommonFuncs.h"
#include "Common/GNSSSignal.h"
#include "Common/GNSSNavDataType.h"

#define RTCM2_MSG_MAX_NUM 16

#define RTCM2_TYPE1_LEN         40
#define RTCM2_TYPE2_LEN         40
#define RTCM2_TYPE3_LEN         96
#define RTCM2_TYPE14_LEN        24
#define RTCM2_TYPE16P_LEN       8
#define RTCM2_TYPE17_LEN        480
#define RTCM2_TYPE18H_LEN       (24 + 3)
#define RTCM2_TYPE18B_LEN       48
#define RTCM2_TYPE19H_LEN       (24 + 3)
#define RTCM2_TYPE19B_LEN       48
#define RTCM2_TYPE22P_LEN       24
#define RTCM2_TYPE24H_LEN       120
#define RTCM2_TYPE24B_LEN       24
#define RTCM2_TYPE31_LEN        40
#define RTCM2_TYPE32_LEN        96
#define RTCM2_TYPE44_LEN        40

#define CHECK_UTC_YEAR(Year)			((((Year) < 2020) || ((Year) > 2099)) ? FALSE : TRUE)

/*****************************************************************************//**
@brief loss-of-lock indicator

@param pLossCount           [In] RTCM decode loss count buffer
@param LockTime             [In] lock time(unit: ms)

@return Phase lock flag, 0 = not locked, 1 = locked

@author CHC
@date  2023/06/25
@note
*********************************************************************************/
static int LossOfLock(unsigned int* pLossCount, unsigned int LockTime)
{
	int LockFlag = !(LockTime != *pLossCount);

	*pLossCount = LockTime;

	return LockFlag;
}

/**********************************************************************//**
@brief  adjust hourly rollover of rtcm 2 time

@param pParseInfo       [In] rtcm parse information
@param ZCount           [In] ZCount
@param TypeID           [In] rtcm2 type id

@retval <0 error, =0 Not complete data, >0: parsed data

@author CHC
@date 2023/07/12
@note
**************************************************************************/
int RTCM2AdjHour(RTCM_PARSE_INFO_T* pParseInfo, double ZCount, unsigned int TypeID)
{
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	double Tow, Hour, Sec;
	int Week, WeekMsCount, GPSLeapSecMs;

	if (!pParseInfo)
	{
		return -1;
	}

	if ((TypeID != 3) && (TypeID != 18) && (TypeID != 19) && (TypeID != 24) && (TypeID != 32))
	{
		return 0;
	}

	pRTCMInterimData = &pParseInfo->RTCMInterimData;

	/** if no time, get cpu time */
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
	Hour = floor(Tow / 3600.0);
	Sec = Tow - Hour * 3600.0;
	if (ZCount < Sec - 1800.0)
	{
		ZCount += 3600.0;
	}
	else if (ZCount > Sec + 1800.0)
	{
		ZCount -= 3600.0;
	}

	GPSTimeToUTC(Week, (int)((Hour * 3600 + ZCount + 0.0005) * 1000), &GPSLeapSecMs, &pRTCMInterimData->RTCMTime);

	return 0;
}

/**********************************************************************//**
@brief  rtcm2 separate time

@param pParseInfo       [In] rtcm parse information
@param TypeID           [In] rtcm2 type id

@retval <0 error, =0 success

@author CHC
@date 2023/09/26
@note
**************************************************************************/
int RTCM2SeparateTime(RTCM_PARSE_INFO_T* pParseInfo, unsigned int TypeID)
{
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if (!pParseInfo)
	{
		return -1;
	}

	pRTCMInterimData = &pParseInfo->RTCMInterimData;

	if (TypeID == 41 || TypeID == 43 || TypeID == 44 || TypeID == 42)
	{
		pRTCMInterimData->RTCMTime = UTCAdd(&pRTCMInterimData->RTCMTime, 14000);
	}
	else if (TypeID == 31 || TypeID == 33 || TypeID == 34)
	{
		pRTCMInterimData->RTCMTime = UTCAdd(&pRTCMInterimData->RTCMTime, pRTCMInterimData->LeapSec * 1000);
	}

	return 0;
}

/**********************************************************************//**
@brief  RTCM2 decode obs over time callback

@param TypeID               [In] RTCM type id
@param pObsData             [In] obs data
@param pRTCMInterimData     [In] rtcm decode interim data
@param pSatObsData          [In] Satellite obs data
@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 success

@author CHC
@date 2023/11/23
@note
**************************************************************************/
static int RTCM2DecodeObsOverTimeCallback(int TypeID, GNSS_OBS_T* pObsData, RTCM_INTERIM_DATA_T* pRTCMInterimData,
	OBS_DATA_SAT_T* pSatObsData, RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	MEMSET(pObsData, 0x00, sizeof(GNSS_OBS_T));
	SatObsDataToGNSSObsData(pSatObsData, pObsData);
	UTCTimeToRangeTime(pObsData, &pRTCMInterimData->LastRTCMTime);
	pObsData->ObsTime = pRTCMInterimData->LastRTCMTime;

	if ((pOverTimeCallback) && (pParseInfo) && (pSrcObj))
	{
		pOverTimeCallback(TypeID, pParseInfo, pSrcObj);
	}

	return 0;
}

/**********************************************************************//**
@brief  decode RTCM1 and RTCM9

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: parsed data

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type1And9(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int Fact, UDRE, SatID, SatIndex = 0;
	signed int PRC, RRC, IOD;
	DIFF_CORRECTION_T* pDiffCorrection;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pDiffCorrection = (DIFF_CORRECTION_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

#ifdef RTCM_DEBUG_INFO
	int TypeID = GetUnsignedBits(pBufIn, 8, 6);
#endif

	while (Index + RTCM2_TYPE1_LEN <= TotalBitsLen)
	{
		Fact = GetUnsignedBits(pBufIn,      Index,  1);     Index +=  1;
		UDRE = GetUnsignedBits(pBufIn,      Index,  2);     Index +=  2;
		SatID = GetUnsignedBits(pBufIn,     Index,  5);     Index +=  5;
		PRC = GetSignedBits(pBufIn,         Index, 16);     Index += 16;
		RRC = GetSignedBits(pBufIn,         Index,  8);     Index +=  8;
		IOD = GetSignedBits(pBufIn,         Index,  8);     Index +=  8;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if ((PRC == 0x80000000) || (RRC == 0xFFFF8000)) /**< 0x80000000: PRC is invalid, 0xFFFF8000: RRC is invalid */
		{
			continue;
		}

		pDiffCorrection->DiffCorrectionData[SatIndex].SatID = SatID;
		pDiffCorrection->DiffCorrectionData[SatIndex].Time = pRTCMInterimData->RTCMTime;
		pDiffCorrection->DiffCorrectionData[SatIndex].PRC = PRC * (Fact ? 0.32 : 0.02);
		pDiffCorrection->DiffCorrectionData[SatIndex].RRC = RRC * (Fact ? 0.032 : 0.002);
		pDiffCorrection->DiffCorrectionData[SatIndex].IOD = IOD;
		pDiffCorrection->DiffCorrectionData[SatIndex].UDRE = UDRE;

		if (++SatIndex >= RTCM_MAX_SAT_NUM)
		{
			break;
		}
	}

	pDiffCorrection->SystemID = GPS;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = 0;
	if (TypeID == 1)
	{
		MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Differential GPS Corrections", 1);
	}
	else if (TypeID == 9)
	{
		MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Partial Correction Set", 9);
	}
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM2

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type2(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int Fact, UDRE, SatID, SatIndex = 0;
	signed int DeltaPRC, DeltaRRC, IOD;
	DELTA_DIFF_CORRECTION_T* pDeltaDiffCorrection;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pDeltaDiffCorrection = (DELTA_DIFF_CORRECTION_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	while (Index + RTCM2_TYPE2_LEN <= TotalBitsLen)
	{
		Fact = GetUnsignedBits(pBufIn,      Index,  1);     Index +=  1;
		UDRE = GetUnsignedBits(pBufIn,      Index,  2);     Index +=  2;
		SatID = GetUnsignedBits(pBufIn,     Index,  5);     Index +=  5;
		DeltaPRC = GetSignedBits(pBufIn,    Index, 16);     Index += 16;
		DeltaRRC = GetSignedBits(pBufIn,    Index,  8);     Index +=  8;
		IOD = GetSignedBits(pBufIn,         Index,  8);     Index +=  8;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if ((DeltaPRC == 0x80000000) || (DeltaRRC == 0xFFFF8000)) /**< 0x80000000: DeltaPRC is invalid, 0xFFFF8000: DeltaRRC is invalid */
		{
			continue;
		}

		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].SatID = SatID;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].Time = pRTCMInterimData->RTCMTime;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].DeltaPRC = DeltaPRC * (Fact ? 0.32 : 0.02);
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].DeltaRRC = DeltaRRC * (Fact ? 0.032 : 0.002);
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].IOD = IOD;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].UDRE = UDRE;

		if (++SatIndex >= RTCM_MAX_SAT_NUM)
		{
			break;
		}
	}

	pDeltaDiffCorrection->SystemID = GPS;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen= sprintf(pDebugInfoBuf, "MsgID %4u Delta Differential GPS Corrections", 2);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM3

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type3(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	STATION_POSITION_DATA_T* pStationPosition;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pStationPosition = (STATION_POSITION_DATA_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM2_TYPE3_LEN <= TotalBitsLen)
	{
		pStationPosition->StationID = GetUnsignedBits(pBufIn, 14, 10);

		pStationPosition->Position[0] = GetSignedBits(pBufIn, Index, 32) * 0.01; Index += 32;
		pStationPosition->Position[1] = GetSignedBits(pBufIn, Index, 32) * 0.01; Index += 32;
		pStationPosition->Position[2] = GetSignedBits(pBufIn, Index, 32) * 0.01;

		pStationPosition->DeltaType = 0;
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u ENU: %lf %lf %lf", 3,
		pStationPosition->StationID, pStationPosition->Position[0], pStationPosition->Position[1], pStationPosition->Position[2]);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM14

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type14(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	int Week, Hour, GPSLeapSecMs;
	double ZCount;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	ZCount = GetUnsignedBits(pBufIn, 24, 13);
	if (Index + RTCM2_TYPE14_LEN <= TotalBitsLen)
	{
		Week = GetUnsignedBits(pBufIn,  Index, 10);     Index += 10;
		Hour = GetUnsignedBits(pBufIn,  Index,  8);     Index +=  8;
	}
	else
	{
		return -1;
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(&pRTCMInterimData->RTCMTime);
	GPSTimeToUTC(Week, (int)((Hour * 3600.0 + ZCount * 0.6) * 1000), &GPSLeapSecMs, &pRTCMInterimData->RTCMTime);

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen= sprintf(pDebugInfoBuf, "MsgID %4u Week:%d Hour:%d ZCount:%.3f", 14, Week, Hour, ZCount);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM16

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type16(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned int MsgIndex = 0;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	RTCM_SPECIAL_MSG_T* pSpecialMsg;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pSpecialMsg = (RTCM_SPECIAL_MSG_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	while (Index + RTCM2_TYPE16P_LEN <= TotalBitsLen && MsgIndex < RTCM_SPECIAL_MSG_LEN_MAX)
	{
		pSpecialMsg->SpecialMsg[MsgIndex++] = (char)GetUnsignedBits(pBufIn, Index, 8); Index += 8;
	}

	pSpecialMsg->SpecialMsg[MsgIndex] = '\0';

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen= sprintf(pDebugInfoBuf, "MsgID %4u Msg:%s", 16, pSpecialMsg->SpecialMsg);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM17

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type17(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	UNI_EPHEMERIS_T* pGPSEphData;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	double SqrtA;
	int Week, SatID;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pGPSEphData = (UNI_EPHEMERIS_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM2_TYPE17_LEN <= TotalBitsLen)
	{
		Week = GetUnsignedBits(pBufIn,                              Index, 10);             Index += 10;
		pGPSEphData->IDot = DoubleIScale2(GetSignedBits(pBufIn,     Index, 14), 43) * PI;   Index += 14;
		pGPSEphData->IODE1 = GetUnsignedBits(pBufIn,                Index,  8);             Index += 8;
		pGPSEphData->TOC = GetUnsignedBits(pBufIn,                  Index, 16) * 16;        Index += 16;
		pGPSEphData->Af1 = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 43);        Index += 16;
		pGPSEphData->Af2 = DoubleIScale2(GetSignedBits(pBufIn,      Index,  8), 55);        Index +=  8;
		pGPSEphData->Crs = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 5);         Index += 16;
		pGPSEphData->DeltaN = DoubleIScale2(GetSignedBits(pBufIn,   Index, 16), 43) * PI;   Index += 16;
		pGPSEphData->Cuc = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->Ecc = DoubleUScale2(GetUnsignedBits(pBufIn,    Index, 32), 33);        Index += 32;
		pGPSEphData->Cus = GetSignedBits(pBufIn,                    Index, 16);             Index += 16;
		SqrtA = DoubleUScale2(GetUnsignedBits(pBufIn,               Index, 32), 19);        Index += 32;
		pGPSEphData->TOE = GetUnsignedBits(pBufIn,                  Index, 16);             Index += 16;
		pGPSEphData->Omega0 = DoubleIScale2(GetSignedBits(pBufIn,   Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Cic = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->I0 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Cis = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 29);        Index += 16;
		pGPSEphData->Omega = DoubleIScale2(GetSignedBits(pBufIn,    Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->Crc = DoubleIScale2(GetSignedBits(pBufIn,      Index, 16), 5);         Index += 16;
		pGPSEphData->OmegaDot = DoubleIScale2(GetSignedBits(pBufIn, Index, 24), 43) * PI;   Index += 24;
		pGPSEphData->M0 = DoubleIScale2(GetSignedBits(pBufIn,       Index, 32), 31) * PI;   Index += 32;
		pGPSEphData->IODC = GetUnsignedBits(pBufIn,                 Index, 10);             Index += 10;
		pGPSEphData->Af0 = DoubleIScale2(GetSignedBits(pBufIn,      Index, 22), 31);        Index += 22;
		SatID = GetUnsignedBits(pBufIn,                             Index,  5);             Index +=  5 + 3;
		pGPSEphData->Tgd[0] = DoubleIScale2(GetSignedBits(pBufIn,   Index,  8), 31);        Index +=  8 + 2;
		pGPSEphData->URA = GetUnsignedBits(pBufIn,                  Index,  4);             Index +=  4;
		pGPSEphData->Health = GetUnsignedBits(pBufIn,               Index,  6);             Index +=  6 + 1;
	}
	else
	{
		return -1;
	}

	if (SatID == 0)
	{
		SatID = 32;
	}

	pGPSEphData->SatID = SatID;
	pGPSEphData->SystemID= GPS;
	pGPSEphData->Week = Week;
	pGPSEphData->SqrtAorDeltaA = SqrtA;
	pGPSEphData->A = SqrtA * SqrtA;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u GPS Ephm:Sat:%2d", 17, SatID);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM18

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type18(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen, SatelliteSystem, SignalType;
	unsigned int USec;
	signed int CarrierPhase;
	unsigned int ObsIndex = 0, Freq = 0, Sync = 1, Code, SystemID = 0, SatID;
	unsigned int PRN, Loss, PhaseLockFlag, ExSystemID = 0;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned short FreqChnn = 0;
	int Week, WeekMsCount, GPSLeapSecMs;
	INT64 TempTimeDiff;
	UTC_TIME_T TempUTCTime;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int SatNum = 0;
	unsigned int L1PValid = 0, L1CValid = 0, L2PValid = 0, L2CValid = 0;
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

	if (Index + RTCM2_TYPE18H_LEN <= TotalBitsLen)
	{
		pObsData->BaseID = GetUnsignedBits(pBufIn, 14, 10);

		Freq = GetUnsignedBits(pBufIn,      Index,  2);     Index +=  2 + 2;
		USec = GetUnsignedBits(pBufIn,      Index, 20);     Index += 20;
		SystemID = GetUnsignedBits(pBufIn,  Index+2,  1);
	}
	else
	{
		return -1;
	}

	if (Freq & 0x1)
	{
		return -1;
	}
	Freq >>= 1;

	if (SystemID)
	{
		TempUTCTime = UTCAdd(&pRTCMInterimData->RTCMTime, pRTCMInterimData->LeapSec*1000);
	}
	else
	{
		TempUTCTime = pRTCMInterimData->RTCMTime;
	}

	TempUTCTime = UTCAdd(&TempUTCTime, (int)((USec + 500) / 1000));

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &TempUTCTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			RTCM2DecodeObsOverTimeCallback(18, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback);
		}
		ObsIndex = 0;
		pObsData->RangeData.ObsNum = 0;
		pRTCMInterimData->ObsCompleteFlag = 0;
		MEMSET(pObsData, 0x00, sizeof(GNSS_OBS_T));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
	}
	pObsData->ObsTime = TempUTCTime;
	pRTCMInterimData->LastRTCMTime = TempUTCTime;

	GPSLeapSecMs = GetGPSLeapSecondMs(&pObsData->ObsTime);
	UTCToGPSTime(&pObsData->ObsTime, &GPSLeapSecMs, &Week, &WeekMsCount);

	pObsData->RangeData.LogHeader.TimeStatus = RM_TIME_STATUS_FINE;
	pObsData->RangeData.LogHeader.Week = Week;
	pObsData->RangeData.LogHeader.Milliseconds = WeekMsCount;

	while (Index + RTCM2_TYPE18B_LEN <= TotalBitsLen && ObsIndex < RM_RANGE_MAX_OBS_NUM)
	{
		Sync = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1;
		Code = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1;
		SystemID = GetUnsignedBits(pBufIn,      Index,  1);     Index +=  1;
		SatID = GetUnsignedBits(pBufIn,         Index,  5);     Index +=  5 + 3;
		Loss = GetUnsignedBits(pBufIn,          Index,  5);     Index +=  5;
		CarrierPhase = GetSignedBits(pBufIn,    Index, 32);     Index += 32;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if (SystemID)
		{
			SatelliteSystem = RM_RANGE_SYS_GLO;
			ExSystemID = GLO;
			SignalType = !Freq ? (Code ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C) : (Code ? RM_RANGE_SIGNAL_GLOL2P : RM_RANGE_SIGNAL_GLOL2C);
			PRN = SatID + 37;
			FreqChnn = (unsigned short)GetGloFreqCh(SatID);
		}
		else
		{
			SatelliteSystem = RM_RANGE_SYS_GPS;
			ExSystemID = GPS;
			SignalType = !Freq ? (Code ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA) : (Code ? RM_RANGE_SIGNAL_GPSL2P : RM_RANGE_SIGNAL_GPSL2CA);
			PRN = SatID;
			FreqChnn = 0;
		}
#ifdef RTCM_DEBUG_INFO
		if (!Freq)
		{
			if (Code)
			{
				L1PValid = 1;
			}
			else
			{
				L1CValid = 1;
			}
		}
		else
		{
			if (Code)
			{
				L2PValid = 1;
			}
			else
			{
				L2CValid = 1;
			}
		}
#endif

		if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
		{
			continue;
		}

		pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
		pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

		pRMRangeData->PRN = PRN;
		pRMRangeData->FreqNum = FreqChnn;

		pRMRangeData->ADR = CarrierPhase / 256.0;

		pRMRangeData->ADRStd = 0;
		pRMRangeData->CN0 = 0;
		pRMRangeData->Doppler = 0;
		pRMRangeData->LockTime = 0;

		PhaseLockFlag = LossOfLock(&pRTCMInterimData->LossCount[ExSystemID][SatID-1][Freq], Loss);
		pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(PhaseLockFlag, 1, 1, SatelliteSystem, SignalType, 0);

		if (pPerSatObsData->ObsNum == 0)
		{
			SatObsDataSatNumAdd(pSatObsData);
#ifdef RTCM_DEBUG_INFO
			SatNum++;
#endif
		}

		if (PerSatObsIndex == pPerSatObsData->ObsNum)
		{
			PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
		}
	}

	pRTCMInterimData->ObsCompleteFlag = !Sync;

	if (!Sync)
	{
		SatObsDataToGNSSObsData(pSatObsData, pObsData);
		UTCTimeToRangeTime(pObsData, &pObsData->ObsTime);
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CValid + L2PValid + L2CValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = 0;
	MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Obs :USec %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %u",
		18, USec, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	unsigned int RTCMSystem = ExSystemToRTCMSystem(ExSystemID);
	if (RTCMSystem < RTCM_SYS_NUM)
	{
		pBaseObsSatNumBuf[RTCMSystem] += SatNum;
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
	}
#endif

	return Sync ? 0 : 1;
}

/**********************************************************************//**
@brief  decode RTCM19

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type19(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen, SatelliteSystem, SignalType;
	unsigned int USec;
	signed int PseudoRange;
	unsigned int ObsIndex = 0, Freq = 0, Sync = 1, Code, SystemID = 0, SatID;
	unsigned int PRN, ExSystemID = 0;
	RM_RANGE_DATA_T* pRMRangeData;
	GNSS_OBS_T* pObsData;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;
	OBS_DATA_SAT_T* pSatObsData;
	OBS_DATA_PER_SAT_T* pPerSatObsData;
	unsigned int PerSatIndex = 0;
	unsigned int PerSatObsIndex = 0;
	unsigned short FreqChnn = 0;
	int Week, WeekMsCount, GPSLeapSecMs;
	INT64 TempTimeDiff;
	UTC_TIME_T TempUTCTime;
	unsigned int OverTimeFlag = 0;
#ifdef RTCM_DEBUG_INFO
	unsigned int SatNum = 0;
	unsigned int L1PValid = 0, L1CValid = 0, L2PValid = 0, L2CValid = 0;
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

	if (Index + RTCM2_TYPE19H_LEN <= TotalBitsLen)
	{
		pObsData->BaseID = GetUnsignedBits(pBufIn, 14, 10);

		Freq = GetUnsignedBits(pBufIn,      Index,  2);     Index +=  2 + 2;
		USec = GetUnsignedBits(pBufIn,      Index, 20);     Index += 20;
		SystemID = GetUnsignedBits(pBufIn,  Index+2,  1);
	}
	else
	{
		return -1;
	}

	if (Freq & 0x1)
	{
		return -1;
	}
	Freq >>= 1;

	if (SystemID)
	{
		TempUTCTime = UTCAdd(&pRTCMInterimData->RTCMTime, pRTCMInterimData->LeapSec*1000);
	}
	else
	{
		TempUTCTime = pRTCMInterimData->RTCMTime;
	}

	TempUTCTime = UTCAdd(&TempUTCTime, (int)((USec + 500) / 1000));

	TempTimeDiff = UTCDiff(&pRTCMInterimData->LastRTCMTime, &TempUTCTime);
	if (llabs(TempTimeDiff) > 35)/**< 35:35ms for 25Hz:40ms*/
	{
		OverTimeFlag = 1;
	}
	if (pRTCMInterimData->ObsCompleteFlag || OverTimeFlag)
	{
		if ((OverTimeFlag) && (!pRTCMInterimData->ObsCompleteFlag))
		{
			RTCM2DecodeObsOverTimeCallback(19, pObsData, pRTCMInterimData, pSatObsData, pParseInfo, pSrcObj, pOverTimeCallback);
		}
		ObsIndex = 0;
		pObsData->RangeData.ObsNum = 0;
		pRTCMInterimData->ObsCompleteFlag = 0;
		MEMSET(pObsData, 0x00, sizeof(GNSS_OBS_T));
		MEMSET(&pRTCMInterimData->SatObsData, 0x00, sizeof(OBS_DATA_SAT_T));
	}
	pObsData->ObsTime = TempUTCTime;
	pRTCMInterimData->LastRTCMTime = TempUTCTime;

	GPSLeapSecMs = GetGPSLeapSecondMs(&pObsData->ObsTime);
	UTCToGPSTime(&pObsData->ObsTime, &GPSLeapSecMs, &Week, &WeekMsCount);

	pObsData->RangeData.LogHeader.TimeStatus = RM_TIME_STATUS_FINE;
	pObsData->RangeData.LogHeader.Week = Week;
	pObsData->RangeData.LogHeader.Milliseconds = WeekMsCount;

	while (Index + RTCM2_TYPE19B_LEN <= TotalBitsLen && ObsIndex < RM_RANGE_MAX_OBS_NUM)
	{
		Sync = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1;
		Code = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1;
		SystemID = GetUnsignedBits(pBufIn,      Index,  1);     Index +=  1;
		SatID = GetUnsignedBits(pBufIn,         Index,  5);     Index +=  5 + 8;
		PseudoRange = GetSignedBits(pBufIn,     Index, 32);     Index += 32;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if (SystemID)
		{
			SatelliteSystem = RM_RANGE_SYS_GLO;
			ExSystemID = GLO;
			SignalType = !Freq ? (Code ? RM_RANGE_SIGNAL_GLOL1P : RM_RANGE_SIGNAL_GLOL1C) : (Code ? RM_RANGE_SIGNAL_GLOL2P : RM_RANGE_SIGNAL_GLOL2C);
			PRN = SatID + 37;
			FreqChnn = (unsigned short)GetGloFreqCh(SatID);
		}
		else
		{
			SatelliteSystem = RM_RANGE_SYS_GPS;
			ExSystemID = GPS;
			SignalType = !Freq ? (Code ? RM_RANGE_SIGNAL_GPSL1P : RM_RANGE_SIGNAL_GPSL1CA) : (Code ? RM_RANGE_SIGNAL_GPSL2P : RM_RANGE_SIGNAL_GPSL2CA);
			PRN = SatID;
			FreqChnn = 0;
		}
#ifdef RTCM_DEBUG_INFO
		if (!Freq)
		{
			if (Code)
			{
				L1PValid = 1;
			}
			else
			{
				L1CValid = 1;
			}
		}
		else
		{
			if (Code)
			{
				L2PValid = 1;
			}
			else
			{
				L2CValid = 1;
			}
		}
#endif

		if (GetPerSatObsDataIndex(pSatObsData, ExSystemID, SatID, SignalType, &PerSatIndex, &PerSatObsIndex) < 0)
		{
			continue;
		}

		pPerSatObsData = &pSatObsData->ObsDataPerSat[PerSatIndex];
		pRMRangeData = &pPerSatObsData->ObsData[PerSatObsIndex];

		pRMRangeData->PRN = PRN;
		pRMRangeData->FreqNum = FreqChnn;

		pRMRangeData->PSR   = PseudoRange * 0.02;

		pRMRangeData->PSRStd = 0;
		pRMRangeData->CN0 = 0;
		pRMRangeData->Doppler = 0;
		pRMRangeData->LockTime = 0;

		if (PerSatObsIndex < pPerSatObsData->ObsNum)
		{
			pRMRangeData->ChnTrackStatus |= COMPOSE_CH_TRACK_STATUS_ALL(0, 1, 1, SatelliteSystem, SignalType, 0);
		}
		else
		{
			pRMRangeData->ChnTrackStatus = COMPOSE_CH_TRACK_STATUS_ALL(1, 1, 1, SatelliteSystem, SignalType, 0);
		}

		if (pPerSatObsData->ObsNum == 0)
		{
			SatObsDataSatNumAdd(pSatObsData);
#ifdef RTCM_DEBUG_INFO
			SatNum++;
#endif
		}

		if (PerSatObsIndex == pPerSatObsData->ObsNum)
		{
			PerSatObsDataObsNumAdd(pPerSatObsData, ExSystemID, SatID);
		}
	}

	pRTCMInterimData->ObsCompleteFlag = !Sync;

	if (!Sync)
	{
		SatObsDataToGNSSObsData(pSatObsData, pObsData);
		UTCTimeToRangeTime(pObsData, &pObsData->ObsTime);
	}

#ifdef RTCM_DEBUG_INFO
	unsigned int SignalNum = L1PValid + L1CValid + L2PValid + L2CValid;
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = 0;
	MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Obs :USec %10u,SatNum:%2u, Signal:%u, BaseID %04u, Sync %u",
		19, USec, SatNum, SignalNum, pRTCMInterimData->StationID, Sync);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;

	char* pBaseObsSatCountBuf = (char*)pParseInfo->ParsedBaseObsSatCountBuf.pBaseAddr;
	unsigned int* pBaseObsSatNumBuf = pRTCMInterimData->BaseObsSatNum;
	unsigned int RTCMSystem = ExSystemToRTCMSystem(ExSystemID);
	if (RTCMSystem < RTCM_SYS_NUM)
	{
		pBaseObsSatNumBuf[RTCMSystem] += SatNum;
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
	}
#endif

	return Sync ? 0 : 1;
}

/**********************************************************************//**
@brief  decode RTCM22

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type22(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen, DelIndex;
	double Del[2][3] = {{0}}, L1PhaCenterHigh = 0.0;
	int NH;
	STATION_POSITION_DATA_T* pStationPosition;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pStationPosition = (STATION_POSITION_DATA_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM2_TYPE22P_LEN <= TotalBitsLen)
	{
		pStationPosition->StationID = GetUnsignedBits(pBufIn, 14, 10);

		Del[0][0] = GetSignedBits(pBufIn,   Index, 8) / 25600.0;                Index += 8;
		Del[0][1] = GetSignedBits(pBufIn,   Index, 8) / 25600.0;                Index += 8;
		Del[0][2] = GetSignedBits(pBufIn,   Index, 8) / 25600.0;                Index += 8;
	}
	else
	{
		return -1;
	}

	if (Index + RTCM2_TYPE22P_LEN <= TotalBitsLen)
	{
		Index += 2 + 1 + 1 + 1;
		NH = GetUnsignedBits(pBufIn,        Index, 1);                          Index += 1;
		L1PhaCenterHigh = NH ? 0 : GetUnsignedBits(pBufIn, Index, 18) / 25600.0; Index += 18;
	}

	if (Index + RTCM2_TYPE22P_LEN <= TotalBitsLen)
	{
		Del[1][0] = GetSignedBits(pBufIn,   Index, 8) / 1600.0;                 Index += 8;
		Del[1][1] = GetSignedBits(pBufIn,   Index, 8) / 1600.0;                 Index += 8;
		Del[1][2] = GetSignedBits(pBufIn,   Index, 8) / 1600.0;
	}

	pStationPosition->DeltaType = 1; /**< xyz */
	for (DelIndex = 0; DelIndex < 3; DelIndex++)
	{
		pStationPosition->PositionDelta[DelIndex] = Del[0][DelIndex];
	}

	pStationPosition->AntennaHeight = L1PhaCenterHigh;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u XYZ: %lf %lf %lf AntennaHeight: %lf",
		22, pStationPosition->StationID, pStationPosition->PositionDelta[0], pStationPosition->PositionDelta[1],
		pStationPosition->PositionDelta[2], pStationPosition->AntennaHeight);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM3

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type24(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen, AH;
	STATION_POSITION_DATA_T* pStationPosition;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pStationPosition = (STATION_POSITION_DATA_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM2_TYPE24H_LEN <= TotalBitsLen)
	{
		pStationPosition->StationID = GetUnsignedBits(pBufIn, 14, 10);

		pStationPosition->Position[0] = Get38Bits(pBufIn, Index) * 0.0001; Index += 38 + 2;
		pStationPosition->Position[1] = Get38Bits(pBufIn, Index) * 0.0001; Index += 38 + 2;
		pStationPosition->Position[2] = Get38Bits(pBufIn, Index) * 0.0001; Index += 38 + 1;
		AH = GetUnsignedBits(pBufIn,                Index, 1);   Index +=  1;

		pStationPosition->DeltaType = 0;
	}
	else
	{
		return -1;
	}

	if ((AH) && (Index + RTCM2_TYPE24B_LEN <= TotalBitsLen))
	{
		pStationPosition->AntennaHeight = GetUnsignedBits(pBufIn, Index, 18) * 0.0001; Index += 18 + 6;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u ENU: %lf %lf %lf AntennaHeight: %lf",
		24, pStationPosition->StationID, pStationPosition->Position[0], pStationPosition->Position[1], pStationPosition->Position[2],
		pStationPosition->AntennaHeight);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM31

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type31(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int Fact, UDRE, SatID, SatIndex = 0, ChangeBit;
	signed int PRC, RRC, IOD;
	DIFF_CORRECTION_T* pDiffCorrection;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pDiffCorrection = (DIFF_CORRECTION_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	while (Index + RTCM2_TYPE31_LEN <= TotalBitsLen)
	{
		Fact = GetUnsignedBits(pBufIn,          Index,  1);     Index +=  1;
		UDRE = GetUnsignedBits(pBufIn,          Index,  2);     Index +=  2;
		SatID = GetUnsignedBits(pBufIn,         Index,  5);     Index +=  5;
		PRC = GetSignedBits(pBufIn,             Index, 16);     Index += 16;
		RRC = GetSignedBits(pBufIn,             Index,  8);     Index +=  8;
		ChangeBit = GetUnsignedBits(pBufIn,     Index,  1);     Index +=  1;
		IOD = GetSignedBits(pBufIn,             Index,  7);     Index +=  7;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if ((PRC == 0x80000000) || (RRC == 0xFFFF8000)) /**< 0x80000000: PRC is invalid, 0xFFFF8000: RRC is invalid */
		{
			continue;
		}

		pDiffCorrection->DiffCorrectionData[SatIndex].SatID = SatID;
		pDiffCorrection->DiffCorrectionData[SatIndex].Time = pRTCMInterimData->RTCMTime;
		pDiffCorrection->DiffCorrectionData[SatIndex].PRC = PRC * (Fact ? 0.32 : 0.02);
		pDiffCorrection->DiffCorrectionData[SatIndex].RRC = RRC * (Fact ? 0.032 : 0.002);
		pDiffCorrection->DiffCorrectionData[SatIndex].IOD = IOD;
		pDiffCorrection->DiffCorrectionData[SatIndex].UDRE = UDRE;
		pDiffCorrection->DiffCorrectionData[SatIndex].ChangeBit = ChangeBit;

		if (++SatIndex >= RTCM_MAX_SAT_NUM)
		{
			break;
		}
	}

	pDiffCorrection->SystemID = GLO;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen= sprintf(pDebugInfoBuf, "MsgID %4u Differential GLONASS Corrections ", 31);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM32

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type32(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	STATION_POSITION_DATA_T* pStationPosition;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pStationPosition = (STATION_POSITION_DATA_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	if (Index + RTCM2_TYPE3_LEN <= TotalBitsLen)
	{
		pStationPosition->StationID = GetUnsignedBits(pBufIn, 14, 10);

		pStationPosition->Position[0] = GetSignedBits(pBufIn, Index, 32) * 0.01; Index += 32;
		pStationPosition->Position[1] = GetSignedBits(pBufIn, Index, 32) * 0.01; Index += 32;
		pStationPosition->Position[2] = GetSignedBits(pBufIn, Index, 32) * 0.01;

		pStationPosition->DeltaType = 0;
	}
	else
	{
		return -1;
	}

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u ARP :RefS %u ENU: %lf %lf %lf",
		32, pStationPosition->StationID, pStationPosition->Position[0], pStationPosition->Position[1], pStationPosition->Position[2]);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM41/RTCM42

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type41And42(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned int BitsLen = 32;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int UDRE, IonPreSig, SatID, SatIndex = 0;
	unsigned int System, ExSystemID = GPS;
	signed int PRC, IOD;
	DIFF_CORRECTION_T* pDiffCorrection;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pDiffCorrection = (DIFF_CORRECTION_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

#ifdef RTCM_DEBUG_INFO
	int TypeID = GetUnsignedBits(pBufIn, 8, 6);
#endif

	System = GetUnsignedBits(pBufIn,      Index,  4); Index +=  4;
	switch (System)
	{
		case  1: ExSystemID = GPS; break;
		case  2: ExSystemID = GLO; break;
		case  6: ExSystemID = BDS; break;
		default: return 0;
	}

	Index += 4 + 2 + 2;
	IonPreSig = GetUnsignedBits(pBufIn,     Index,  1); Index +=  1;

	if (IonPreSig == 1)
	{
		BitsLen = 44;
	}

	while(Index + BitsLen < TotalBitsLen)
	{
		SatID = GetUnsignedBits(pBufIn,     Index,  6); Index +=  6;
		UDRE = GetUnsignedBits(pBufIn,      Index,  4); Index +=  4;
		IOD = GetSignedBits(pBufIn,         Index,  8); Index +=  8;
		PRC = GetSignedBits(pBufIn,         Index, 14); Index += 14;

		if (IonPreSig == 1)
		{
			Index += 12;
		}

		if (PRC == 0x80000000)/**< 0x80000000: PRC is invalid*/
		{
			continue;
		}

		pDiffCorrection->DiffCorrectionData[SatIndex].SatID = SatID;
		pDiffCorrection->DiffCorrectionData[SatIndex].Time = pRTCMInterimData->RTCMTime;
		pDiffCorrection->DiffCorrectionData[SatIndex].PRC = PRC * 0.02;
		pDiffCorrection->DiffCorrectionData[SatIndex].RRC = 0;
		pDiffCorrection->DiffCorrectionData[SatIndex].IOD = IOD;
		pDiffCorrection->DiffCorrectionData[SatIndex].UDRE = UDRE;

		if (++SatIndex >= RTCM_MAX_SAT_NUM)
		{
			break;
		}
	}

	pDiffCorrection->SystemID = ExSystemID;

#ifdef RTCM_DEBUG_INFO
		char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
		unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4d Differential Corrections", TypeID);
		pDebugInfoBuf[MsgLen] = '\0';
		pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

/**********************************************************************//**
@brief  decode RTCM44

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2Type44(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;
	(void)pOverTimeCallback;

	unsigned int Index = RTCM2_MSG_HEADER_BITS_SIZE;
	unsigned char* pBufIn;
	unsigned int TotalBitsLen;
	unsigned int Fact, UDRE, SatID, SatIndex = 0;
	signed int DeltaPRC, DeltaRRC, IOD;
	DELTA_DIFF_CORRECTION_T* pDeltaDiffCorrection;
	RTCM_INTERIM_DATA_T* pRTCMInterimData;

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	pDeltaDiffCorrection = (DELTA_DIFF_CORRECTION_T*)pParseInfo->ParsedDataBuf.pBaseAddr;
	pRTCMInterimData = &pParseInfo->RTCMInterimData;
	pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	TotalBitsLen = pParseInfo->DecodeBuf.Len << 3;

	while (Index + RTCM2_TYPE44_LEN <= TotalBitsLen)
	{
		Fact = GetUnsignedBits(pBufIn,      Index,  1); Index +=  1;
		UDRE = GetUnsignedBits(pBufIn,      Index,  2); Index +=  2;
		SatID = GetUnsignedBits(pBufIn,     Index,  5); Index +=  5;
		DeltaPRC = GetSignedBits(pBufIn,    Index, 16); Index += 16;
		DeltaRRC = GetSignedBits(pBufIn,    Index,  8); Index +=  8;
		IOD = GetSignedBits(pBufIn,         Index,  8); Index +=  8;

		if (SatID == 0)
		{
			SatID = 32;
		}

		if (DeltaPRC == 0x80000000 || DeltaRRC == 0xFFFF8000) /**< 0x80000000: DeltaPRC is invalid, 0xFFFF8000: DeltaRRC is invalid */
		{
			continue;
		}

		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].SatID = SatID;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].Time = pRTCMInterimData->RTCMTime;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].DeltaPRC = DeltaPRC * (Fact ? 0.32 : 0.02);
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].DeltaRRC = DeltaRRC * (Fact ? 0.032 : 0.002);
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].IOD = IOD;
		pDeltaDiffCorrection->DeltaDiffCorrectionData[SatIndex].UDRE = UDRE;

		if (++SatIndex >= RTCM_MAX_SAT_NUM)
		{
			break;
		}
	}

	pDeltaDiffCorrection->SystemID = BDS;

#ifdef RTCM_DEBUG_INFO
	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	unsigned int MsgLen = sprintf(pDebugInfoBuf, "MsgID %4u Delta Differential Corrections", 44);
	pDebugInfoBuf[MsgLen] = '\0';
	pParseInfo->ParsedDebugInfoBuf.Len = MsgLen + 1;
#endif

	return 1;
}

#ifdef MSG_CONVERT

/**********************************************************************//**
@brief  decode RTCM4

@param pParseInfo           [In] rtcm parse information
@param pSrcObj              [In] pointer to source object
@param pOverTimeCallback    [In] decode obs overtime callback function

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int DecodeRTCM2DebugInfo(RTCM_PARSE_INFO_T* pParseInfo, void* pSrcObj, RTCM2_DECODE_OVERTIME_FUNC pOverTimeCallback)
{
	(void)pSrcObj;

	(void)pOverTimeCallback;
#ifdef RTCM_DEBUG_INFO

	if ((!pParseInfo) || (!pParseInfo->DecodeBuf.pBaseAddr) || (!pParseInfo->ParsedDataBuf.pBaseAddr))
	{
		return -1;
	}

	unsigned char* pBufIn = (unsigned char*)pParseInfo->DecodeBuf.pBaseAddr;
	int TypeID = GetUnsignedBits(pBufIn, 8, 6);

	static char* s_RTCM2MsgNameMap[] =
	{
		NULL, /**< RTCM1 */
		NULL, /**< RTCM2 */
		NULL, /**< RTCM3 */
		"Reference Station Datum Message", /**< RTCM4 */
		"GPS Constellation Health", /**< RTCM5 */
		"GPS Null Frame", /**< RTCM6 */
		"DGPS Radiobeacon Almanac", /**< RTCM7 */
		"Pseudolite Almanac", /**< RTCM8 */
		NULL, /**< RTCM9 */
		"P-Code Differential Corrections", /**< RTCM10 */
		"C/A Code L2 Corrections", /**< RTCM11 */
		"Pseudolite Station Parameters", /**< RTCM12 */
		"Ground Transmitter Parameters", /**< RTCM13 */
		NULL, /**< RTCM14 */
		"Ionospheric Delay Message", /**< RTCM15 */
		NULL, /**< RTCM16 */
		NULL, /**< RTCM17 */
		NULL, /**< RTCM18 */
		NULL, /**< RTCM19 */
		"RTK Carrier Phase Corrections", /**< RTCM20 */
		"High-Accuracy Pseudorange Corrections", /**< RTCM21 */
		NULL, /**< RTCM22 */
		"Antenna Type Definition Record", /**< RTCM23 */
		NULL, /**< RTCM24 */
		NULL, /**< RTCM25 */
		NULL,
		"Extended Radiobeacon Almanac", /**< RTCM27 */
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"GLONASS Constellation Health", /**< RTCM33 */
		"GLONASS Partial Correction Set", /**< RTCM34 */
		"GLONASS Radiobeacon Almanac", /**< RTCM35 */
		"GLONASS Special Message", /**< RTCM36 */
		"GNSS System Time Offset", /**< RTCM37 */
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
		NULL,
		"Proprietary Message", /**< RTCM59 */
	};

	char* pDebugInfoBuf = (char*)pParseInfo->ParsedDebugInfoBuf.pBaseAddr;
	if (NULL != s_RTCM2MsgNameMap[TypeID - 1])
	{
		unsigned int MsgLen= sprintf(pDebugInfoBuf, "MsgID %4d Size:%4u %s", TypeID,
									pParseInfo->DecodeBuf.Len, s_RTCM2MsgNameMap[TypeID - 1]);
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

