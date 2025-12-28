#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		GNSS Print Module
*-
@file   GNSSPrint.h
@author CHC
@date   2023/04/17
@brief

**************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include "GNSSPrint.h"
#include "../../Code/BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "Common/GNSSNavDataType.h"
#include "../../Code/BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"

#include "Common/CommonConst.h"
#include "Common/SupportFuncs.h"
#include "Coordinate.h"

#include "Encoder/RINEX/RINEXDefines.h"

int GNSSPrintf(const char* format, ...)
{
	unsigned char msg[256] = { 0 };
	va_list args;
	va_start(args, format);
	vsnprintf(msg, 256, format, args);
	va_end(args);
#ifndef WIN32
	//
#else
	printf("%s", msg);
#endif
	return 0;
}

/**********************************************************************//**
@brief  show position

@param pPosXYZ       [In] position data

@author CHC
@date 2023/09/12
@note
**************************************************************************/
void ShowPosition(double* pPosXYZ)
{
	double PosLLH[3];

	if ((IsAbsoluteDoubleZero(pPosXYZ[0])) && (IsAbsoluteDoubleZero(pPosXYZ[1])) && (IsAbsoluteDoubleZero(pPosXYZ[2])))
	{
		return;
	}

	PosXYZ2LLH(pPosXYZ, PosLLH);

#ifdef CONVERT_DEBUG_OUTPUT
	GNSSPrintf("Position: XYZ:%lf,%lf,%lf, LLH(rad,rad,m):%lf,%lf,%lf, LLH(degree,degree,m):%lf,%lf,%lf\r\n",
		pPosXYZ[0], pPosXYZ[1], pPosXYZ[2],
		PosLLH[0], PosLLH[1], PosLLH[2], (PosLLH[0] * RTOD), (PosLLH[1] * RTOD), PosLLH[2]);
#endif
}

void ShowReceiverInfo(RTCM_RECEIVER_DESCRIPTOR_INFO_T* pReceiverInfo)
{
#ifdef CONVERT_DEBUG_OUTPUT
	GNSSPrintf("StationID:%d, Antenna:%s, AntennaID:%d, AntennaSN:%s\r\n", pReceiverInfo->StationID, pReceiverInfo->AntennaDescriptor, pReceiverInfo->AntennaID, pReceiverInfo->AntennaSN);
	GNSSPrintf("Receiver:%s, ReceiverFirmVer:%s, ReceiverSN:%s\r\n", pReceiverInfo->ReceiverDescriptor, pReceiverInfo->ReceiverFirmVer, pReceiverInfo->ReceiverSN);
#endif
}

void PrintObsTime(GNSS_OBS_T* pObs)
{
#ifdef CONVERT_DEBUG_OUTPUT
	GNSSPrintf("> %04d %2d %2d %2d %2d %2d.%03d0000\n", pObs->ObsTime.Year, pObs->ObsTime.Month, pObs->ObsTime.Day, pObs->ObsTime.Hour, pObs->ObsTime.Minute, pObs->ObsTime.Second, pObs->ObsTime.Millisecond);
#endif
}

int PrintfProgressBar(int Percentage)
{
	char TempBuf[22] = {0};
	const char* Lable = "|/-\\";

	memset(TempBuf, '#', (Percentage / 5));

	GNSSPrintf("[%-20s][%d%%][%c]\r", TempBuf, Percentage, Lable[Percentage%4]);

	return 0;
}
#endif