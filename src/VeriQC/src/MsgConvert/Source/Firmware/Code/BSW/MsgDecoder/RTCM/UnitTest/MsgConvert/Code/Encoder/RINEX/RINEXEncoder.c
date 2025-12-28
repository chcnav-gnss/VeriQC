#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RINEXEncoder.c
@author CHC
@date   2023/04/03
@brief

**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "RINEXEncoder.h"
#include "Common/DataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "Common/CommonFuncs.h"
#include "Common/GNSSSignal.h"
#include "Common/SupportFuncs.h"
#include "TimeDefines.h"
#include "Common/GNSSConstants.h"
#include "Common/GNSSNavDataType.h"

#include "../../Decoder/RawMsgAscii/RawMsgDecodeSupportFunc.h"

#define OBS_PER_SAT_NUM_MAX			(64)

#define MAX_OBS_NUM					(256)

#define RINEX_MAX_BUF_LEN			(1024*40)
#define RINEX_MAX_HEAD_BUF_LEN		(1024*3)

#define RINEX_CRX_VERSION1 "1.0"	/**< CRINEX version for RINEX 2.x */
#define RINEX_CRX_VERSION2 "3.0"	/**< CRINEX version for RINEX 3.x */
#define RINEX_PROGNAME "RNX2CRX"

static const int s_NavSys[] = {			 /**< satellite systems */
	SYS_MASK_GPS, SYS_MASK_GLO, SYS_MASK_GAL, SYS_MASK_BDS, SYS_MASK_SBAS, SYS_MASK_QZSS, SYS_MASK_NAVIC, 0
};

static const char s_SysCodes[] = "GRECSJI"; /**< satellite SystemIDtem codes */


//static const double s_UraNominal[] = { /**< URA nominal values */
//	2.0,2.8,4.0,5.7,8.0,11.3,16.0,32.0,64.0,128.0,256.0,512.0,1024.0,
//	2048.0,4096.0,8192.0
//};
//
static const double s_UraEph[] = {		 /* ura values (ref [3] 20.3.3.3.1.1) */
	2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
	3072.0,6144.0,0.0
};

typedef struct _SAT_CODE_INFO_T
{
	char* pSatCode;
	int SatOffset;
} SAT_CODE_INFO_T;

typedef struct _HEADER_SYSTEM_INFO_T
{
	char* pSysStr;
	int SatSysMask;
} HEADER_SYSTEM_INFO_T;

/**********************************************************************//**
@brief  satellite index to satellite code

@param SatSys	   [In] satellite system
@param PRN		  [In] satellite PRN
@param Code		 [Out] satellite code

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int Sat2Code(int ExSystemID, int PRN, char *pCode)
{
	static const SAT_CODE_INFO_T s_SatCodeInfoMap[] = {
		{"G%02d",	0},
		{"R%02d",	0},
		{"E%02d",	0},
		{"C%02d",	0},
		{"S%02d",	-100},
		{"J%02d",	(- MIN_QZSS_PRN + 1)},
		{"I%02d",	0},
	};

	if (ExSystemID < (sizeof(s_SatCodeInfoMap)/sizeof(s_SatCodeInfoMap[0])))
	{
		sprintf(pCode, s_SatCodeInfoMap[ExSystemID].pSatCode, PRN + s_SatCodeInfoMap[ExSystemID].SatOffset);
		return 0;
	}

	return -1;
}

/**********************************************************************//**
@brief  get header system information

@param RinexID			[In] rinex id
@param pSatSysMask		[Out] satellite syatem mask

@retval NULL: error, other: header system string

@author CHC
@Date 2024/10/18
@note
**************************************************************************/
static char* GetHeaderSystemInfo(int RinexID, int* pSatSysMask)
{
	static const HEADER_SYSTEM_INFO_T s_HeaderSystemInfoMap[] = {
			{"M: Mixed",		SYS_MASK_ALL},
			{"G: GPS",			SYS_MASK_GPS},
			{"R: GLONASS",		SYS_MASK_GLO},
			{"E: GALILEO",		SYS_MASK_GAL},
			{"C: BDS",			SYS_MASK_BDS},
			{"J: QZSS",			SYS_MASK_QZSS},
			{"S: SBAS Payload",	SYS_MASK_SBAS},
			{"I: NavIC/IRNSS",	SYS_MASK_NAVIC},
	};

	if ((RinexID >= RINEX_ID_MUTIEPH_HEADER) && (RinexID <= RINEX_ID_NICEPH_HEADER))
	{
		int RinexIDIndex = RinexID - RINEX_ID_MUTIEPH_HEADER;
		*pSatSysMask = s_HeaderSystemInfoMap[RinexIDIndex].SatSysMask;

		return s_HeaderSystemInfoMap[RinexIDIndex].pSysStr;
	}

	return NULL;
}

/**********************************************************************//**
@brief  URA index to URA nominal value

@param SVA	  [In] URA index

@retval URA nominal value

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static double UraValue(int SVA)
{
	return (((0 <= SVA) && (SVA < 15)) ? s_UraEph[SVA] : 32767.0);
}

/**********************************************************************//**
@brief  Galileo SISA index to SISA nominal value

@param SISA	  [In] Galileo SISA index

@retval SISA nominal value

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static double SISAValue(int SISA)
{
	if (SISA<= 49)
	{
		return SISA*0.01;
	}
	if (SISA<= 74)
	{
		return 0.5+ ((double)SISA- 50)*0.02;
	}
	if (SISA<= 99)
	{
		return 1.0+((double)SISA- 75)*0.04;
	}
	if (SISA<=125)
	{
		return 2.0+((double)SISA-100)*0.16;
	}

	return -1.0; /**< unknown or NAPA */
}

/**********************************************************************//**
@brief translate rinex system to rawmsg system ID

@param RMRangeSystem [In] raw message system


@return ExSystemID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static unsigned int RinexSystemToRMSystem(int RindexSystem)
{
	const static unsigned int s_RMSystemMap[] =
	{
		RM_RANGE_SYS_GPS,
		RM_RANGE_SYS_GLO,
		RM_RANGE_SYS_GAL,
		RM_RANGE_SYS_BDS,
		RM_RANGE_SYS_SBA,
		RM_RANGE_SYS_QZS,
		RM_RANGE_SYS_NIC,
	};

	if (RindexSystem < (sizeof(s_RMSystemMap) / sizeof(unsigned int)))
	{
		return s_RMSystemMap[RindexSystem];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate raw message system to internal used system ID

@param SatSys	   [In] satitle system
@param Type		 [In] satitle signal type
@param pObsCode	 [Out] obs code

@return <0:error, =0:success

@author CHC
@date 2022/05/16
@note
**************************************************************************/
int GetObsSigCode(int RMSystemID, int Type, unsigned int* pObsCode)
{
	unsigned int ObsCode = NA8;

	const static unsigned char s_ObsCodeMap[RM_RANGE_SYS_OTHER][32] = {
		[RM_RANGE_SYS_GPS] = {
			OBS_CODE_L1C,	OBS_CODE_L2C,	NA8,			NA8, /**< signal id 0-3 */
			OBS_CODE_L1P,	OBS_CODE_L2P,	OBS_CODE_L5I,	NA8, /**< signal id 4-7 */
			OBS_CODE_L1W,	OBS_CODE_L2W,	OBS_CODE_L5X,	NA8, /**< signal id 8-11 */
			OBS_CODE_L1S,	OBS_CODE_L2L,	OBS_CODE_L5Q,	NA8, /**< signal id 12-15 */
			OBS_CODE_L1L,	OBS_CODE_L2S,	NA8,			NA8, /**< signal id 16-19 */
			OBS_CODE_L1X,	OBS_CODE_L2X,	NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_GLO] = {
			OBS_CODE_L1C,	OBS_CODE_L2C,	OBS_CODE_L3I,	NA8, /**< signal id 0-3 */
			OBS_CODE_L1P,	OBS_CODE_L2P,	OBS_CODE_L3Q,	NA8, /**< signal id 4-7 */
			NA8,			NA8,			OBS_CODE_L3X,	NA8, /**< signal id 8-11 */
			OBS_CODE_L4A,	OBS_CODE_L6A,	NA8,			NA8, /**< signal id 12-15 */
			OBS_CODE_L4B,	OBS_CODE_L6B,	NA8,			NA8, /**< signal id 16-19 */
			OBS_CODE_L4X,	OBS_CODE_L6X,	NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_SBA] = {
			OBS_CODE_L1C,	NA8,			NA8,			NA8, /**< signal id 0-3 */
			NA8,			NA8,			OBS_CODE_L5I,	NA8, /**< signal id 4-7 */
			NA8,			NA8,			OBS_CODE_L5Q,	NA8, /**< signal id 8-11 */
			NA8,			NA8,			OBS_CODE_L5X,	NA8, /**< signal id 12-15 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 16-19 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_GAL] = {
			OBS_CODE_L1A,	OBS_CODE_L1B,	OBS_CODE_L1C,	OBS_CODE_L1X, /**< signal id 0-3 */
			OBS_CODE_L1Z,	OBS_CODE_L6A,	OBS_CODE_L6B,	OBS_CODE_L6C, /**< signal id 4-7 */
			OBS_CODE_L6X,	OBS_CODE_L6Z,	NA8,			OBS_CODE_L5I, /**< signal id 8-11 */
			OBS_CODE_L5Q,	OBS_CODE_L5X,	NA8,			NA8, /**< signal id 12-15 */
			OBS_CODE_L7I,	OBS_CODE_L7Q,	OBS_CODE_L7X,	OBS_CODE_L8I, /**< signal id 16-19 */
			OBS_CODE_L8Q,	OBS_CODE_L8X,	NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_BDS] = {
			OBS_CODE_L2I,	OBS_CODE_L7I,	OBS_CODE_L6I,	NA8, /**< signal id 0-3 */
			OBS_CODE_L2I,	OBS_CODE_L7I,	OBS_CODE_L6I,	OBS_CODE_L1P, /**< signal id 4-7 */
			OBS_CODE_L7P,	OBS_CODE_L5P,	OBS_CODE_L1D,	OBS_CODE_L7D, /**< signal id 8-11 */
			OBS_CODE_L5D,	OBS_CODE_L1X,	NA8,			OBS_CODE_L5X, /**< signal id 12-15 */
			NA8,			OBS_CODE_L7I,	NA8,			NA8, /**< signal id 16-19 */ /**<17: B2I for U84B0 */
			OBS_CODE_L2Q,	OBS_CODE_L7Q,	OBS_CODE_L6Q,	NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_QZS] = {
			OBS_CODE_L1C,	NA8,			NA8,			NA8, /**< signal id 0-3 */
			NA8,			NA8,			OBS_CODE_L5I,	NA8, /**< signal id 4-7 */
			NA8,			NA8,			OBS_CODE_L5X,	NA8, /**< signal id 8-11 */
			OBS_CODE_L1S,	OBS_CODE_L2L,	OBS_CODE_L5Q,	NA8, /**< signal id 12-15 */
			OBS_CODE_L1L,	OBS_CODE_L2S,	NA8,			NA8, /**< signal id 16-19 */
			OBS_CODE_L1X,	OBS_CODE_L2X,	NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			OBS_CODE_L6S,	OBS_CODE_L6L, /**< signal id 24-27 */
			OBS_CODE_L6X,	NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_NIC] = {
			OBS_CODE_L5A,	NA8,			NA8,			NA8, /**< signal id 0-3 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 4-7 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 8-11 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 12-15 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 16-19 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 20-23 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 24-27 */
			NA8,			NA8,			NA8,			NA8, /**< signal id 28-31 */
		},
	};

	if ((RMSystemID < RM_RANGE_SYS_OTHER) && (Type < 32))
	{
		ObsCode = s_ObsCodeMap[RMSystemID][Type];
	}

	if (ObsCode == NA8)
	{
		return -1;
	}

	*pObsCode = ObsCode;

	return 0;
}

/**********************************************************************//**
@brief get obs signal charter by code type

@param CodeType		[In] satitle signal type

@return signal charter

@author CHC
@date 2022/05/16
@note
**************************************************************************/
const char* GetObsSigCharByCode(int CodeType)
{
	const static char* s_ObsCodeMap[] = {
		NULL,
		"C1C",
		"C1P",
		"C1W",
		"C1Y",
		"C1M",
		"C1N",
		"C1S",
		"C1L",
		"C1E",
		"C1A",
		"C1B",
		"C1X",
		"C1Z",
		"C2C",
		"C2D",
		"C2S",
		"C2L",
		"C2X",
		"C2P",
		"C2W",
		"C2Y",
		"C2M",
		"C2N",
		"C5I",
		"C5Q",
		"C5X",
		"C7I",
		"C7Q",
		"C7X",
		"C6A",
		"C6B",
		"C6C",
		"C6X",
		"C6Z",
		"C6S",
		"C6L",
		"C8I",
		"C8Q",
		"C8X",
		"C2I",
		"C2Q",
		"C6I",
		"C6Q",
		"C3I",
		"C3Q",
		"C3X",
		"C1I",
		"C1Q",
		"C5D",
		"C5P",
		"C5Z",
		"C6E",
		"C7D",
		"C7P",
		"C7Z",
		"C1D",
		"C8D",
		"C8P",
		"C6D",
		"C6P",
		"C5A",
		"C5B",
		"C5C",
		"C9A",
		"C9B",
		"C9C",
		"C9X",
		"C4A",
		"C4B",
		"C4X",
	};

	if (CodeType < (sizeof(s_ObsCodeMap)/sizeof(s_ObsCodeMap[0])))
	{
		return s_ObsCodeMap[CodeType];
	}

	return NULL;
}

/**********************************************************************//**
@brief get obs signal charter

@param RMSystemID		[In] satitle system
@param Type				[In] satitle signal type

@return signal charter

@author CHC
@date 2022/05/16
@note
**************************************************************************/
const char* GetObsSigChar(int RMSystemID, int Type)
{
	const static char* s_ObsCodeMap[RM_RANGE_SYS_OTHER][32] = {
		[RM_RANGE_SYS_GPS] = {
			"1C",			"2C",			NULL,			NULL, /**< signal id 0-3 */
			"1P",			"2P",			"5I",			NULL, /**< signal id 4-7 */
			"1W",			"2W",			"5X",			NULL, /**< signal id 8-11 */
			"1S",			"2L",			"5Q",			NULL, /**< signal id 12-15 */
			"1L",			"2S",			NULL,			NULL, /**< signal id 16-19 */
			"1X",			"2X",			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_GLO] = {
			"1C",			"2C",			"3I",			NULL, /**< signal id 0-3 */
			"1P",			"2P",			"3Q",			NULL, /**< signal id 4-7 */
			NULL,			NULL,			"3X",			NULL, /**< signal id 8-11 */
			"4A",			"6A",			NULL,			NULL, /**< signal id 12-15 */
			"4B",			"6B",			NULL,			NULL, /**< signal id 16-19 */
			"4X",			"6X",			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_SBA] = {
			"1C",			NULL,			NULL,			NULL, /**< signal id 0-3 */
			NULL,			NULL,			"5I",			NULL, /**< signal id 4-7 */
			NULL,			NULL,			"5Q",			NULL, /**< signal id 8-11 */
			NULL,			NULL,			"5X",			NULL, /**< signal id 12-15 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 16-19 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_GAL] = {
			"1A",			"1B",			"1C",			"1X", /**< signal id 0-3 */
			"1Z",			"6A",			"6B",			"6C", /**< signal id 4-7 */
			"6X",			"6Z",			NULL,			"5I", /**< signal id 8-11 */
			"5Q",			"5X",			NULL,			NULL, /**< signal id 12-15 */
			"7I",			"7Q",			"7X",			"8I", /**< signal id 16-19 */
			"8Q",			"8X",			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_BDS] = {
			"2I",			"7I",			"6I",			NULL, /**< signal id 0-3 */
			"2I",			"7I",			"6I",			"1P", /**< signal id 4-7 */
			NULL,			"5P",			"1D",			"7D", /**< signal id 8-11 */
			"5D",			"1X",			NULL,			"5X", /**< signal id 12-15 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 16-19 */
			"2Q",			"7Q",			"6Q",	NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_QZS] = {
			"1C",			NULL,			NULL,			NULL, /**< signal id 0-3 */
			NULL,			NULL,			"5I",			NULL, /**< signal id 4-7 */
			NULL,			NULL,			"5X",			NULL, /**< signal id 8-11 */
			"1S",			"2L",			"5Q",			NULL, /**< signal id 12-15 */
			"1L",			"2S",			NULL,			NULL, /**< signal id 16-19 */
			"1X",			"2X",			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			"6S",			"6L", /**< signal id 24-27 */
			"6X",			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
		[RM_RANGE_SYS_NIC] = {
			"5A",			NULL,			NULL,			NULL, /**< signal id 0-3 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 4-7 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 8-11 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 12-15 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 16-19 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 20-23 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 24-27 */
			NULL,			NULL,			NULL,			NULL, /**< signal id 28-31 */
		},
	};

	if ((RMSystemID < RM_RANGE_SYS_OTHER) && (Type < 32))
	{
		return s_ObsCodeMap[RMSystemID][Type];
	}

	return NULL;
}

/**********************************************************************//**
@brief  search obs data index

@param Ver			[In] rinex version
@param SatSys		[In] satellite system
@param pData		[In] GNSS_OBS_PER_SAT_T
@param TObs			[In] TObs
@param Mask			[In] satellite Mask
@param DataType		[In] data type

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXObsIndex(int Ver, int ExSystemID, GNSS_OBS_PER_SAT_T* pData, const char * pTObs, char Mask[], unsigned int DataType)
{
	const char* TempID = NULL;
	int Index;/**< freq num */
	int ObsCode = 0;
	int SignalType, RMSystemID;
	unsigned int ChnTrackStatus;

	for (Index = 0; Index < 32; Index++)
	{
		/**< signal Mask */
		if (DataType == GNSS_OBS_DATA_TYPE_RANGE)
		{
			ChnTrackStatus = pData->ObsData[Index].ChnTrackStatus;

			SignalType = GET_CH_TRACK_STATUS_SIGNALTYPE(ChnTrackStatus);
			if (SignalType == NA8)
			{
				continue;
			}

			RMSystemID = GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus);
		}
		else if (DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
		{
			RMSystemID = GNSSObsSystemToRMRangeSystem(pData->GNSSObsData[Index].SystemID);
			if (RMSystemID == NA8)
			{
				continue;
			}

			SignalType = GNSSObsSignalToRMRangeSignal(pData->GNSSObsData[Index].SystemID, pData->GNSSObsData[Index].SignalType, NULL);
			if (SignalType == NA8)
			{
				continue;
			}
		}
		else
		{
			return -1;
		}

		if (Ver < 299) /**< ver.2 */
		{
			if ((!strcmp(pTObs, "C1")) || (!strcmp(pTObs, "L1")) || (!strcmp(pTObs, "S1")) || (!strcmp(pTObs, "D1")))
			{
				if (((RMSystemID == RM_RANGE_SYS_GPS) && (SignalType == RM_RANGE_SIGNAL_GPSL1CA)) ||
					((RMSystemID == RM_RANGE_SYS_GLO) && (SignalType == RM_RANGE_SIGNAL_GLOL1C)) ||
					((RMSystemID == RM_RANGE_SYS_GAL) && (SignalType == RM_RANGE_SIGNAL_GALE1C)) ||
					((RMSystemID == RM_RANGE_SYS_QZS) && (SignalType == RM_RANGE_SIGNAL_QZSSL1CA)) ||
					((RMSystemID == RM_RANGE_SYS_SBA) && (SignalType == RM_RANGE_SIGNAL_SBASL1C)))
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs, "P1"))
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL1P) || (SignalType == RM_RANGE_SIGNAL_GPSL1W))
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs, "C2") && (RMSystemID == RM_RANGE_SYS_GPS || RMSystemID == RM_RANGE_SYS_QZS))
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL2CM) || (SignalType == RM_RANGE_SIGNAL_GPSL2CL) || (SignalType == RM_RANGE_SIGNAL_GPSL2CX) ||
					(SignalType == RM_RANGE_SIGNAL_QZSSL2CM) || (SignalType == RM_RANGE_SIGNAL_QZSSL2CL) || (SignalType == RM_RANGE_SIGNAL_QZSSL2CX))
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs, "C2") && RMSystemID == RM_RANGE_SYS_GLO)
			{
				if (SignalType == RM_RANGE_SIGNAL_GLOL2C)
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs + 1, "1") && RMSystemID == RM_RANGE_SYS_BDS)
			{
				if ((SignalType == RM_RANGE_SIGNAL_BDSB1ID1) || (SignalType == RM_RANGE_SIGNAL_BDSB1ID2) || (SignalType == RM_RANGE_SIGNAL_BDSB1Q))
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs + 1, "2") && RMSystemID == RM_RANGE_SYS_BDS)
			{
				if ((SignalType == RM_RANGE_SIGNAL_BDSB2ID1) || (SignalType == RM_RANGE_SIGNAL_BDSB2ID2) || (SignalType == RM_RANGE_SIGNAL_BDSB2Q))
				{
					return Index;
				}
			}
			else if (!strcmp(pTObs, "P2"))
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL2P) || (SignalType == RM_RANGE_SIGNAL_GPSL2PCODELESS))
				{
					return Index;
				}

			}
			else if (!strcmp(pTObs, "L5") || !strcmp(pTObs, "P5") || !strcmp(pTObs, "S5") || !strcmp(pTObs, "D5") || !strcmp(pTObs, "C5"))
			{
				if ((SignalType == RM_RANGE_SIGNAL_BDSB3ID1) || (SignalType == RM_RANGE_SIGNAL_BDSB3ID2) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL5X) || (SignalType == RM_RANGE_SIGNAL_GLOL2OCX) || (SignalType == RM_RANGE_SIGNAL_QZSSL5X) || (SignalType == RM_RANGE_SIGNAL_SBASL5X) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL5Q) || (SignalType == RM_RANGE_SIGNAL_GLOL1OCX) || (SignalType == RM_RANGE_SIGNAL_QZSSL5Q) || (SignalType == RM_RANGE_SIGNAL_SBASL5Q) ||
					(SignalType == RM_RANGE_SIGNAL_BDSB3Q))
				{
					return Index;
				}
			}
			else if ((!strcmp(pTObs, "L2") || !strcmp(pTObs, "P2") || !strcmp(pTObs, "S2") || !strcmp(pTObs, "D2")) && RMSystemID != RM_RANGE_SYS_GAL)
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL2P) || (SignalType == RM_RANGE_SIGNAL_GLOL2P) || (SignalType == RM_RANGE_SIGNAL_GPSL2PCODELESS) ||
					(SignalType == RM_RANGE_SIGNAL_BDSB2ID1) || (SignalType == RM_RANGE_SIGNAL_BDSB2ID2) || (SignalType == RM_RANGE_SIGNAL_BDSB2Q))
				{
					return Index;
				}

				if (GetObsSigCode(RMSystemID, SignalType, &ObsCode) < 0)
				{
					continue;
				}

				TempID = GetObsSigCharByCode(ObsCode);
				if (TempID == NULL)
				{
					continue;
				}

				if (!strcmp(TempID+1, pTObs+1))
				{
					return Index;
				}
			}
			else if (Ver >= 212 && pTObs[1] == 'A') /**< L1C/A */
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL1CA) || (SignalType == RM_RANGE_SIGNAL_GLOL1C) ||
					(SignalType == RM_RANGE_SIGNAL_QZSSL1CA) || (SignalType == RM_RANGE_SIGNAL_SBASL1C))
				{
					return Index;
				}
			}
			else if (Ver >= 212 && pTObs[1] == 'B') /**< L1C */
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL1CD) || (SignalType == RM_RANGE_SIGNAL_QZSSL1CD) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL1CP) || (SignalType == RM_RANGE_SIGNAL_QZSSL1CP) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL1CX) || (SignalType == RM_RANGE_SIGNAL_QZSSL1CX) || (SignalType == RM_RANGE_SIGNAL_GALE1X))
				{
					return Index;
				}
			}
			else if (Ver >= 212 && pTObs[1] == 'C') /**< L2C */
			{
				if ((SignalType == RM_RANGE_SIGNAL_GPSL2CM) || (SignalType == RM_RANGE_SIGNAL_QZSSL2CM) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL2CL) || (SignalType == RM_RANGE_SIGNAL_QZSSL2CL) ||
					(SignalType == RM_RANGE_SIGNAL_GPSL2CX) || (SignalType == RM_RANGE_SIGNAL_QZSSL2CX))
				{
					return Index;
				}
			}
			else if (Ver >= 212 && pTObs[1] == 'D' && RMSystemID == RM_RANGE_SYS_GLO)
			{ /* GLO L2C/A */
				if (SignalType == RM_RANGE_SIGNAL_GLOL2C)
				{
					return Index;
				}
			}
			else if (RMSystemID == RM_RANGE_SYS_BDS && (!strcmp(pTObs + 1, "6") || !strcmp(pTObs + 1, "7")))
			{
				continue;
			}
			else
			{
				if (GetObsSigCode(RMSystemID, SignalType, &ObsCode) < 0)
				{
					continue;
				}

				TempID = GetObsSigCharByCode(ObsCode);
				if (TempID == NULL)
				{
					continue;
				}

				if (!strcmp(TempID + 1, pTObs + 1))
				{
					return Index;
				}
			}
		}
		else /**< Ver.3 */
		{
			if (GetObsSigCode(RMSystemID, SignalType, &ObsCode) < 0)
			{
				continue;
			}

			TempID = GetObsSigCharByCode(ObsCode);
			if (TempID == NULL)
			{
				continue;
			}

			if (!strcmp(TempID + 1, pTObs + 1))
			{
				return Index;
			}
		}

	}

	return -1;
}

/**********************************************************************//**
@brief  out rinex time string

@param pBuff	  [In] rinex time string buffer

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RinexGetTimeStr(char *pBuff)
{
	UTC_TIME_T TempTime;

	TempTime = RTCMGetUTCTime();
	sprintf(pBuff, "%04d%02d%02d %02d%02d%02dUTC", TempTime.Year, TempTime.Month, TempTime.Day, TempTime.Hour, TempTime.Minute, TempTime.Second);

	return 0;
}

/**********************************************************************//**
@brief  get signal strength

@param Cn0		[In] Cn0

@retval signal strength

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int SignalStrength(double Cn0)
{
	if (Cn0 < 1E-9)
	{
		return 0;
	}
	return MIN(MAX((int)(Cn0/6.0), 1), 9);
}

/**********************************************************************//**
@brief  out rinex nav data string

@param pBuff		[In] rinex nav data string buffer
@param Len		  [Out] rinex nav data string length
@param Value		[In] rinex nav data

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXGetNavDataStr(char *pBuff, int *Len, double Value)
{
	double TempData;

	if (*Len == -1)
	{
		return -1;
	}

	TempData = fabs(Value) < 1E-15 ? 0.0 : floor(log10(fabs(Value)) + 1.0);

	if (fabs(Value) < 1E-15)
	{
		TempData = 1.0;
	}

	if (*Len >= RINEX_MAX_BUF_LEN - 19)
	{
		*Len = -1; return -1;
	}

	*Len += sprintf(pBuff + *Len, "%s%013.12fD%+03.0f", Value < 0.0 ? "-" : " ", fabs(Value) / pow(10.0, TempData - 1.0), TempData - 1.0);

	return 0;
}

/**********************************************************************//**
@brief  output obs types ver.2

@param pBuff		[In] rinex data string buffer
@param pObsOpt	  [In] rniex obs output struct
@param Len		  [Out] rinex nav data string length

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXObsTypeVer2Encode(char *pBuff, const RINEX_OBS_OPT_T* pObsOpt, int *Len)
{
	const char Label[] = "# / TYPES OF OBSERV";
	int Index;

	if (*Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}

	sprintf(pBuff + *Len, "%6d", pObsOpt->NObs[0]);
	*Len += 6;

	for (Index = 0; Index < pObsOpt->NObs[0]; Index++)
	{
		if (Index > 0 && Index % 9 == 0)
		{
			if (*Len >= RINEX_MAX_BUF_LEN - 6)
			{
				return -1;
			}

			sprintf(pBuff + *Len, "      ");
			*Len += 6;
		}

		if (*Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}

		sprintf(pBuff + *Len, "%6s", pObsOpt->ObsTypeBuf[0][Index]);
		*Len += 6;

		if (Index % 9 == 8)
		{
			if (*Len >= RINEX_MAX_BUF_LEN - 22)
			{
				return -1;
			}
			sprintf(pBuff + *Len, "%-20s\r\n", Label); *Len += 22;
		}
	}
	if (pObsOpt->NObs[0] == 0 || Index % 9 > 0)
	{
		if (*Len >= RINEX_MAX_BUF_LEN - (9 - Index % 9) * 6 + 22)
		{
			return -1;
		}

		sprintf(pBuff + *Len, "%*s%-20s\r\n", (9 - Index % 9) * 6, "", Label);
		*Len += (9 - Index % 9) * 6 + 22;
	}

	return 0;
}

/**********************************************************************//**
@brief  output obs types ver.3

@param pBuff		[In] rinex data string buffer
@param pObsOpt	 	[In] rniex obs output struct
@param Len			[Out] rinex nav data string length

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXObsTypeVer3Encode(char* pBuff, const RINEX_OBS_OPT_T* pObsOpt, int* pLen)
{
	const char Label[] = "SYS / # / OBS TYPES";
	int SysIndex, ObsIndex, Index;
	char BDSObsType[MAX_NUM_OBS_TYPE][4];

	for (SysIndex = 0; s_NavSys[SysIndex]; SysIndex++)
	{
		if (!(s_NavSys[SysIndex] & pObsOpt->NavSys) || !pObsOpt->NObs[SysIndex])
		{
			continue;
		}

		if ((pObsOpt->Ver <= 301) && ((s_NavSys[SysIndex] == SYS_MASK_BDS) || (s_NavSys[SysIndex] == SYS_MASK_QZSS) || (s_NavSys[SysIndex] == SYS_MASK_NAVIC)))
		{
			continue;
		}

		if ((pObsOpt->Ver == 302) && (s_NavSys[SysIndex] == SYS_MASK_NAVIC))
		{
			continue;
		}

		if (*pLen >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}

		sprintf(pBuff + *pLen, "%c  %3d", s_SysCodes[SysIndex], pObsOpt->NObs[SysIndex]);
		*pLen += 6;

		/** obstype name revise (C1? <==> C2?), if 3.02->3.03/3.04, vise versa */
		if (s_NavSys[SysIndex] == SYS_MASK_BDS && pObsOpt->Ver < 303)
		{
			for (ObsIndex = 0; ObsIndex < pObsOpt->NObs[SysIndex]; ObsIndex++)
			{
				for (Index = 0; Index < 4; Index++)
				{
					BDSObsType[ObsIndex][Index] = pObsOpt->ObsTypeBuf[SysIndex][ObsIndex][Index];
				}

				if (pObsOpt->Ver < 303)
				{
					if (!strcmp(BDSObsType[ObsIndex] + 1, "2I") || !strcmp(BDSObsType[ObsIndex] + 1, "2Q") || !strcmp(BDSObsType[ObsIndex] + 1, "2X"))
					{
						BDSObsType[ObsIndex][1] = '1';
					}
				}
			}
		}

		for (ObsIndex = 0; ObsIndex < pObsOpt->NObs[SysIndex]; ObsIndex++)
		{
			if (ObsIndex > 0 && ObsIndex % 13 == 0)
			{
				if (*pLen >= RINEX_MAX_BUF_LEN - 6)
				{
					return -1;
				}
				sprintf(pBuff + *pLen, "      "); *pLen += 6;
			}

			if (*pLen >= RINEX_MAX_BUF_LEN - 4)
			{
				return -1;
			}

			if (s_NavSys[SysIndex] == SYS_MASK_BDS && pObsOpt->Ver < 303)
			{
				sprintf(pBuff + *pLen, " %3s", BDSObsType[ObsIndex]);
			}
			else
			{
				sprintf(pBuff + *pLen, " %3s", pObsOpt->ObsTypeBuf[SysIndex][ObsIndex]);
			}

			*pLen += 4;

			if (ObsIndex % 13 == 12)
			{
				if (*pLen > RINEX_MAX_BUF_LEN - 24)
				{
					return -1;
				}
				sprintf(pBuff + *pLen, "  %-20s\r\n", Label);
				*pLen += 24;
			}
		}

		if (ObsIndex % 13 > 0)
		{
			if (*pLen > RINEX_MAX_BUF_LEN - (13 - ObsIndex % 13) * 4 + 22)
			{
				return -1;
			}
			sprintf(pBuff + *pLen, "%*s  %-20s\r\n", (13 - ObsIndex % 13) * 4, "", Label);
			*pLen += (13 - ObsIndex % 13) * 4 + 24;
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  output phase shift ver.3

@param pBuff		[In] rinex data string buffer
@param pObsOpt	 	[In] rinex obs output struct
@param pHeader		[In] rinex obs header struct
@param Len			[Out] rinex nav data string length

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXPhaseShiftVer3Encode(char* pBuff, const RINEX_OBS_OPT_T* pObsOpt, RINEX_OBS_HEADER_T *pHeader, int* pLen)
{
	char Label[20] = {0};
	unsigned int SysIndex, SignalIndex, SatIndex;
	unsigned int ExSystemID, RMSystemID;
	const char* pSigChar = NULL;
	char SigCharBuf[8] = {0};

	RINEX_PHASE_SHIFT_INFO_T* pRinexPhaseShiftInfo = &pHeader->RinexPhaseShiftInfo;
	PER_SYS_PHASE_SHIFT_INFO_T* pPerSysPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pPerSignalPhaseShiftInfo;

	if (pObsOpt->Ver == 301)
	{
		sprintf(Label, "SYS / PHASE SHIFTS");
	}
	else if (pObsOpt->Ver > 301)
	{
		sprintf(Label, "SYS / PHASE SHIFT");
	}

	for (SysIndex = 0; s_NavSys[SysIndex]; SysIndex++)
	{
		if (!(s_NavSys[SysIndex] & pObsOpt->NavSys) || !pObsOpt->NObs[SysIndex])
		{
			continue;
		}
		if (*pLen >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}

		ExSystemID = RinexSystemToExSystem(SysIndex);
		if (ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM)
		{
			continue;
		}

		if ((pObsOpt->Ver == 301) && ((ExSystemID == BDS) || (ExSystemID == QZS) || (ExSystemID == NIC)))
		{
			continue;
		}

		if ((pObsOpt->Ver == 302) && (ExSystemID == NIC))
		{
			continue;
		}

		RMSystemID = RinexSystemToRMSystem(SysIndex);
		pPerSysPhaseShiftInfo = &pRinexPhaseShiftInfo->PerSysPhaseShiftInfo[ExSystemID];

		for (SignalIndex = 0; SignalIndex < 32; SignalIndex++)
		{
			pPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[SignalIndex];
			if (pPerSignalPhaseShiftInfo->SatNum > 0)
			{
				pSigChar = GetObsSigChar(RMSystemID, SignalIndex);
				if (NULL == pSigChar)
				{
					continue;
				}
				MEMCPY(SigCharBuf, pSigChar, 2);
				SigCharBuf[2] = 0;
				if ((pObsOpt->Ver < 303) && (ExSystemID == BDS) && (SigCharBuf[0] == '2'))
				{
					SigCharBuf[0] = '1';/**< PHASE SHIFT revise (C1? <==> C2?), if 3.02->3.03/3.04, vise versa */
				}
				*pLen += sprintf(pBuff + *pLen, "%c L%s % .5f  %02d", s_SysCodes[SysIndex], &SigCharBuf[0], pPerSignalPhaseShiftInfo->PhaseShiftData, pPerSignalPhaseShiftInfo->SatNum);
			}

			for (SatIndex = 0; SatIndex < pPerSignalPhaseShiftInfo->SatNum; SatIndex++)
			{
				if (SatIndex > 0 && SatIndex % 10 == 0)
				{
					if (*pLen >= RINEX_MAX_BUF_LEN - 6)
					{
						return -1;
					}
					*pLen += sprintf(pBuff + *pLen, "                  ");
				}

				if (*pLen >= RINEX_MAX_BUF_LEN - 4)
				{
					return -1;
				}

				if (ExSystemID == QZS)
				{
					*pLen += sprintf(pBuff + *pLen, " %c%02d", s_SysCodes[SysIndex], pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] - MIN_QZSS_PRN + 1);
				}
				else if (ExSystemID == SBA)
				{
					*pLen += sprintf(pBuff + *pLen, " %c%02d", s_SysCodes[SysIndex], pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] - 100);
				}
				else if (ExSystemID == GLO)
				{
					*pLen += sprintf(pBuff + *pLen, " %c%02d", s_SysCodes[SysIndex], pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] - 37);
				}
				else
				{
					*pLen += sprintf(pBuff + *pLen, " %c%02d", s_SysCodes[SysIndex], pPerSignalPhaseShiftInfo->SatIDMap[SatIndex]);
				}

				if (SatIndex % 10 == 9)
				{
					if (*pLen > RINEX_MAX_BUF_LEN - 24)
					{
						return -1;
					}
					*pLen += sprintf(pBuff + *pLen, "  %-20s\r\n", Label);
				}
			}

			if (SatIndex % 10 > 0)
			{
				if ((unsigned int) * pLen > RINEX_MAX_BUF_LEN - (10 - SatIndex % 10) * 4 + 22)
				{
					return -1;
				}
				*pLen += sprintf(pBuff + *pLen, "%*s  %-20s\r\n", (10 - SatIndex % 10) * 4, "", Label);
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  output phase shift ver.3

@param pBuff		[In] rinex data string buffer
@param pObsOpt	 	[In] rinex obs output struct
@param pHeader		[In] rinex obs header struct
@param Len			[Out] rinex nav data string length

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXGLOFreqChnnVer3Encode(char* pBuff, int* pLen)
{
	const char Label[] = "GLONASS SLOT / FRQ #";
	unsigned int SatIndex;
	unsigned char *pGLOFreqChMap =  GetGloFreqChMap();

	if (*pLen >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}

	*pLen += sprintf(pBuff + *pLen, " %02d", 24);

	for (SatIndex = 0; SatIndex < 24; SatIndex++)
	{
		if ((SatIndex > 0) && ((SatIndex & 0x07) == 0))
		{
			if (*pLen >= RINEX_MAX_BUF_LEN - 6)
			{
				return -1;
			}
			*pLen += sprintf(pBuff + *pLen, "   ");
		}

		if (*pLen >= RINEX_MAX_BUF_LEN - 4)
		{
			return -1;
		}

		*pLen += sprintf(pBuff + *pLen, " R%02d", SatIndex + 1);

		*pLen += sprintf(pBuff + *pLen, "% 3d", (int)pGLOFreqChMap[SatIndex] - 7);

		if ((SatIndex & 0x07) == 7)
		{
			if (*pLen > RINEX_MAX_BUF_LEN - 24)
			{
				return -1;
			}
			*pLen += sprintf(pBuff + *pLen, " %-20s\r\n", Label);
		}
	}

	if ((SatIndex & 0x07) > 0)
	{
		if ((unsigned int) * pLen > RINEX_MAX_BUF_LEN - (8 - (SatIndex & 0x07)) * 7 + 22)
		{
			return -1;
		}
		*pLen += sprintf(pBuff + *pLen, "%*s %-20s\r\n", (8 - (SatIndex & 0x07)) * 7, "", Label);
	}

	return 0;
}

/**********************************************************************//**
@brief  output rinex crx obs header field

@param pBuff		[In] rinex data string buffer
@param pObsOpt	  [In] rniex obs output struct
@param pHeader	  [In] rinex crx obs header struct

@retval <0 error, =0 success

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXCrxObsHeaderEncode(char *pBuff, RINEX_OBS_OPT_T* pObsOpt, RINEX_OBS_HEADER_T* pHeader)
{
	const char *GLOCodes[] = { "C1C", "C1P", "C2C", "C2P" };
	double Pos[3] = { 0 }, Del[3] = { 0 };
	int Index;
	int Len = 0;
	char Date[32], *SysString, *TempSysString = "GPS";
	char Line2[41], TimeString[20];
	//TIME_T Time = GetTime();
	time_t TimeSec = 0;//Time.Sec;//time( NULL );
	struct tm *TempTime;

	RinexGetTimeStr(Date);

	if (pObsOpt->Ver < 299) /**< ver.2 */
	{
		SysString = pObsOpt->NavSys == SYS_MASK_GPS ? "G (GPS)" : "M (MIXED)";
	}
	else
	{
		if (pObsOpt->NavSys == SYS_MASK_GPS)
		{
			SysString = "G: GPS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_GLO)
		{
			SysString = "R: GLONASS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_GAL)
		{
			SysString = "E: GALILEO";
		}
		else if (pObsOpt->NavSys == SYS_MASK_QZSS)
		{
			SysString = "J: QZSS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_BDS)
		{
			SysString = "C: BDS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_SBAS)
		{
			SysString = "S: SBAS Payload";
		}
		else if (pObsOpt->NavSys == SYS_MASK_NAVIC)
		{
			SysString = "I: NAVIC";
		}
		else
		{
			SysString = "M: Mixed";
		}
	}

	if(( TempTime = gmtime(&TimeSec)) == NULL )
	{
		TempTime = localtime( &TimeSec );
	}
	strftime(TimeString, 20, "%d-%b-%y %H:%M", TempTime );

	if(pObsOpt->Ver <=299)
	{
		Len += sprintf(pBuff + Len, "%-20.20s", RINEX_CRX_VERSION1 );
	}
	else
	{
		Len += sprintf(pBuff + Len,  "%-20.20s", RINEX_CRX_VERSION2 );
	}

	Len += sprintf(pBuff + Len, "%-40.40s%-20.20s\r\n", "COMPACT RINEX FORMAT", "CRINEX VERS   / TYPE" );

	sprintf(Line2, "%s %s", RINEX_PROGNAME, "ver.4.0.4" );
	Len += sprintf(pBuff + Len, "%-40.40s%-20.20sCRINEX PROG / DATE\r\n", Line2, TimeString );

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%9.2f%-11s%-20s%-20s%-20s\r\n", (double)pObsOpt->Ver / 100, "", "OBSERVATION DATA",
		SysString, "RINEX VERSION / TYPE");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->Pgm, pHeader->Runby, Date,
								"PGM / RUN BY / DATE");

	if (pHeader->NCom < 0 || pHeader->NCom > MAXCOMMENT)
	{
		return -1;
	}

	for (Index = 0; Index < pHeader->NCom; Index++)
	{
		if (!*pHeader->Comments[Index])
		{
			continue;
		}

		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->Comments[Index], "COMMENT");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->MarkerName, "MARKER NAME");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-40.40s%-20s\r\n", pHeader->MarkerNo, "", "MARKER NUMBER");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-40.40s%-20s\r\n", pHeader->Observer, pHeader->Agency,
								"OBSERVER / AGENCY");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->RecNo, pHeader->RecType,
		pHeader->RecVer, "REC # / TYPE / VERS");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->AntDes, pHeader->AntNo,
								"", "ANT # / TYPE");

	for (Index = 0; Index < 3; Index++)
	{
		if (fabs(pHeader->Pos[Index]) < 1E8)
		{
			Pos[Index] = pHeader->Pos[Index];
		}
	}

	for (Index = 0; Index < 3; Index++)
	{
		if (fabs(pHeader->Del[Index]) < 1E8)
		{
			Del[Index] = pHeader->Del[Index];
		}
	}

	if ((fabs(Pos[0]) < 1E-15) || (fabs(Pos[1]) < 1E-15) || (fabs(Pos[2]) < 1E-15))
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		Len += sprintf(pBuff + Len, "%14.4s%14.4s%14.4s%-18s%-20s\r\n", "", "", "", "",
										"APPROX POSITION XYZ");
	}
	else
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%-18s%-20s\r\n", Pos[0], Pos[1], Pos[2], "",
							"APPROX POSITION XYZ");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%-18s%-20s\r\n", Del[0], Del[1], Del[2], "",
								"ANTENNA: DELTA H/E/N");

	if (pObsOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		Len += sprintf(pBuff + Len, "%6d%6d%-48s%-20s\r\n", 1, 1, "", "WAVELENGTH FACT L1/2");
		if (RINEXObsTypeVer2Encode(pBuff, pObsOpt, &Len))
		{
			return Len = -1;
		}
	}
	else/**< ver.3 */
	{
		if (RINEXObsTypeVer3Encode(pBuff, pObsOpt, &Len))
		{
			return Len = -1;
		}
	}

	if (pHeader->TimeInt >= 0.0)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%10.3f%50s%-20s\r\n", pHeader->TimeInt, "", "INTERVAL");
	}
	else
	{
		return -2;
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "  %04d%6d%6d%6d%6d%4d.%03d0000     %-12s%-20s\r\n", pHeader->TimeStart.Year,
					pHeader->TimeStart.Month, pHeader->TimeStart.Day, pHeader->TimeStart.Hour,
					pHeader->TimeStart.Minute, pHeader->TimeStart.Second, pHeader->TimeStart.Millisecond, TempSysString, "TIME OF FIRST OBS");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "  %04d%6d%6d%6d%6d%4d.%03d0000     %-12s%-20s\r\n", pHeader->TimeEnd.Year,
					pHeader->TimeEnd.Month, pHeader->TimeEnd.Day, pHeader->TimeEnd.Hour,
					pHeader->TimeEnd.Minute, pHeader->TimeEnd.Second, pHeader->TimeEnd.Millisecond, TempSysString, "TIME OF LAST OBS");

	if (pObsOpt->Ver >= 301) /**< ver.3.01 */
	{
		RINEXPhaseShiftVer3Encode(pBuff, pObsOpt, pHeader, &Len);
		RINEXGLOFreqChnnVer3Encode(pBuff, &Len);
	}

	if (pObsOpt->Ver >= 302) /**< ver.3.02 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		for (Index = 0; Index < 4; Index++)
		{
			Len += sprintf(pBuff + Len, " %3s %8.3f", GLOCodes[Index], 0.0);
		}
		Len += sprintf(pBuff + Len, "%8s%-20s\r\n", "", "GLONASS COD/PHS/BIS");
	}
	if (pHeader->Leapsecs > 0 || pHeader->Leapsecs < 255)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		pHeader->Leapsecs = (int)(GetGPSLeapSecondMs(&pHeader->TimeStart) / 1000 + 0.5);
		Len += sprintf(pBuff + Len, "%6d%54s%-20s\r\n", pHeader->Leapsecs, "", "LEAP SECONDS");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", "", "END OF HEADER");

	return Len;
}

/**********************************************************************//**
@brief  output rinex obs header field

@param pBuff		[In] rinex data string buffer
@param pObsOpt	  [In] rniex obs output struct
@param pHeader	  [In] rinex crx obs header struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXObsHeaderEncode(char *pBuff, RINEX_OBS_OPT_T * pObsOpt, RINEX_OBS_HEADER_T *pHeader)
{
	const char *GLOCodes[] = { "C1C", "C1P", "C2C", "C2P" };
	double Pos[3] = { 0 }, Del[3] = { 0 };
	int Index;
	int Len = 0;
	char Date[32], *SysString, *TempSysString = "GPS";
	UTC_TIME_T TempTime = {0};

	RinexGetTimeStr(Date);

	if (pObsOpt->Ver < 299) /**< ver.2 */
	{
		SysString = pObsOpt->NavSys == SYS_MASK_GPS ? "G (GPS)" : "M (MIXED)";
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%9.2f%-11s%-20s%-20s%-20s\r\n", (double)pObsOpt->Ver / 100, "", "OBSERVATION DATA",
						SysString, "RINEX VERSION / TYPE");
	}
	else
	{
		if (pObsOpt->NavSys == SYS_MASK_GPS)
		{
			SysString = "G: GPS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_GLO)
		{
			SysString = "R: GLONASS";
		}
		else if (pObsOpt->NavSys == SYS_MASK_GAL)
		{
			SysString = "E: GALILEO";
		}
		else if (pObsOpt->NavSys == SYS_MASK_QZSS)
		{
			SysString = "J: QZSS"; /**< ver.3.02 */
		}
		else if (pObsOpt->NavSys == SYS_MASK_BDS)
		{
			SysString = "C: BDS"; /**< ver.3.02 */
		}
		else if (pObsOpt->NavSys == SYS_MASK_SBAS)
		{
			SysString = "S: SBAS Payload";
		}
		else if (pObsOpt->NavSys == SYS_MASK_NAVIC)
		{
			SysString = "I: NavIC/IRNSS";
		}
		else
		{
			SysString = "M: Mixed";
		}

		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}

		sprintf(pBuff + Len, "%9.2f%-11s%-20s%-20s%-20s\r\n", (double)pObsOpt->Ver / 100, "", "OBSERVATION DATA",
			SysString, "RINEX VERSION / TYPE"); Len += 82;
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->Pgm, pHeader->Runby, Date,
								"PGM / RUN BY / DATE"); //Len += 82;

	if (pHeader->NCom < 0 || pHeader->NCom > MAXCOMMENT)
	{
		return -1;
	}

	for (Index = 0; Index < pHeader->NCom; Index++)
	{
		if (!*pHeader->Comments[Index])
		{
			continue;
		}
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->Comments[Index], "COMMENT");
	}
	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->MarkerName, "MARKER NAME");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-20.20s%-40.40s%-20s\r\n", pHeader->MarkerNo, "", "MARKER NUMBER");

	if (pObsOpt->Ver > 301)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%-20.20s%-40.40s%-20s\r\n", pHeader->MarkerType, "", "MARKER TYPE");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-20.20s%-40.40s%-20s\r\n", pHeader->Observer, pHeader->Agency,
								"OBSERVER / AGENCY");
	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->RecNo, pHeader->RecType,
					pHeader->RecVer, "REC # / TYPE / VERS");
	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->AntDes, pHeader->AntNo,
								"", "ANT # / TYPE");

	if (pHeader->UserPosMode != 0)
	{
		for (Index = 0; Index < 3; Index++)
		{
			if (fabs(pHeader->UserPos[Index]) < 1E8)
			{
				Pos[Index] = pHeader->UserPos[Index];
			}
		}
	}
	else
	{
		for (Index = 0; Index < 3; Index++)
		{
			if (fabs(pHeader->Pos[Index]) < 1E8)
			{
				Pos[Index] = pHeader->Pos[Index];
			}
		}
	}

	if (pHeader->UserDelMode != 0)
	{
		for (Index = 0; Index < 3; Index++)
		{
			if (fabs(pHeader->UserDel[Index]) < 1E8)
			{
				Del[Index] = pHeader->UserDel[Index];
			}
		}
	}
	else
	{
		for (Index = 0; Index < 3; Index++)
		{
			if (fabs(pHeader->Del[Index]) < 1E8)
			{
				Del[Index] = pHeader->Del[Index];
			}
		}
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%-18s%-20s\r\n", Pos[0], Pos[1], Pos[2], "",
									"APPROX POSITION XYZ");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}
	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%-18s%-20s\r\n", Del[0], Del[1], Del[2], "",
								"ANTENNA: DELTA H/E/N");

	if (pObsOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%6d%6d%-48s%-20s\r\n", 1, 1, "", "WAVELENGTH FACT L1/2");
		if (RINEXObsTypeVer2Encode(pBuff, pObsOpt, &Len))
		{
			return Len = -1;
		}
	}
	else	/** ver.3 */
	{
		if (RINEXObsTypeVer3Encode(pBuff, pObsOpt, &Len))
		{
			return Len = -1;
		}
	}

	if (pHeader->TimeInt >= 0.0)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		Len += sprintf(pBuff + Len, "%10.3f%50s%-20s\r\n", pHeader->TimeInt, "", "INTERVAL");
	}
	else
	{
		return -2;
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	TempTime = UTCAdd(&pHeader->TimeStart, pHeader->Leapsecs*1000);
	Len += sprintf(pBuff + Len, "  %04d%6d%6d%6d%6d%5d.%03d0000     %-12s%-20s\r\n", TempTime.Year,
					TempTime.Month, TempTime.Day, TempTime.Hour,
					TempTime.Minute, TempTime.Second, TempTime.Millisecond, TempSysString, "TIME OF FIRST OBS");

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	TempTime = UTCAdd(&pHeader->TimeEnd, pHeader->Leapsecs*1000);
	Len += sprintf(pBuff + Len, "  %04d%6d%6d%6d%6d%5d.%03d0000     %-12s%-20s\r\n", TempTime.Year,
					TempTime.Month, TempTime.Day, TempTime.Hour,
					TempTime.Minute, TempTime.Second, TempTime.Millisecond, TempSysString, "TIME OF LAST OBS");

	if (pObsOpt->Ver >= 301) /**< ver.3.01 */
	{
		RINEXPhaseShiftVer3Encode(pBuff, pObsOpt, pHeader, &Len);
		RINEXGLOFreqChnnVer3Encode(pBuff, &Len);
	}

	if (pObsOpt->Ver >= 302) /**< ver.3.02 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		for (Index = 0; Index < 4; Index++)
		{
			Len += sprintf(pBuff + Len, " %3s %8.3f", GLOCodes[Index], 0.0);
		}
		Len += sprintf(pBuff + Len, "%8s%-20s\r\n", "", "GLONASS COD/PHS/BIS");
	}

	if (pHeader->Leapsecs > 0 || pHeader->Leapsecs < 255)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return Len = -1;
		}
		pHeader->Leapsecs = (int)(GetGPSLeapSecondMs(&pHeader->TimeStart) / 1000 + 0.5);
		Len += sprintf(pBuff + Len, "%6d%54s%-20s\r\n", pHeader->Leapsecs, "", "LEAP SECONDS");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return Len = -1;
	}

	Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", "", "END OF HEADER");

	return Len;
}

/**********************************************************************//**
@brief  output rinex nav header field

@param pBuff		[In] rinex data string buffer
@param pEphOpt	  [In] rniex eph output struct
@param pHeader	  [In] rinex eph header struct
@param MsgID		[In] message id

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXNavHeaderEncode(char* pBuff, const RINEX_EPH_OPT_T* pEphOpt, const RINEX_EPH_HEADER_T* pHeader, int MsgID)
{
	int Index;
	int Len = 0;
	char Date[64], *pSysStr;
	int SatSysMask;

	if (pEphOpt->Ver == 0)
	{
		return -1;
	}

	RinexGetTimeStr(Date);

	if (pEphOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}

		if (MsgID == RINEX_ID_GPSEPH_HEADER)
		{
			Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
						"N: GPS NAV DATA", "G: GPS", "RINEX VERSION / TYPE");
		}
		else if (MsgID == RINEX_ID_GALEPH_HEADER)
		{
			Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
						"N: GAL NAV DATA", "E: GALILEO", "RINEX VERSION / TYPE");
		}
		else if (MsgID == RINEX_ID_BDSEPH_HEADER)
		{
			Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
						"N: COM NAV DATA", "C: BDS", "RINEX VERSION / TYPE");
		}
		else
		{
			Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
						"N: GNSS NAV DATA", "M: Mixed", "RINEX VERSION / TYPE");
		}
	}
	else /**< ver.3 */
	{
		pSysStr = GetHeaderSystemInfo(MsgID, &SatSysMask);
		if (pSysStr == NULL)
		{
			return -1;
		}

		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
				"N: GNSS NAV DATA", pSysStr, "RINEX VERSION / TYPE");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->Pgm, pHeader->Runby, Date,
			"PGM / RUN BY / DATE");

	if (pHeader->NCom < 0 || pHeader->NCom > MAXCOMMENT)
	{
		return -1;
	}

	for (Index = 0; Index < pHeader->NCom; Index++)
	{
		if (!*pHeader->Comments[Index])
		{
			continue;
		}

		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->Comments[Index], "COMMENT");
	}

	if (pEphOpt->Ver < 299) /**< ver.2 */
	{
		if (pEphOpt->Iono)
		{
			if (Len >= RINEX_MAX_BUF_LEN - 164)
			{
				return -1;
			}

			Len += sprintf(pBuff + Len, "  %12.4E%12.4E%12.4E%12.4E%10s%-20s\r\n", pHeader->Ion[0][0],
			pHeader->Ion[0][1], pHeader->Ion[0][2], pHeader->Ion[0][3], "", "ION ALPHA");

			Len += sprintf(pBuff + Len, "  %12.4E%12.4E%12.4E%12.4E%10s%-20s\r\n", pHeader->Ion[0][4],
			pHeader->Ion[0][5], pHeader->Ion[0][6], pHeader->Ion[0][7], "", "ION BETA");
		}

		if (pEphOpt->Time)
		{
			if (Len >= RINEX_MAX_BUF_LEN - 164)
			{
				return -1;
			}

			Len += sprintf(pBuff + Len, "   ");
			RINEXGetNavDataStr(pBuff, &Len, pHeader->UTC[0][0]);
			RINEXGetNavDataStr(pBuff, &Len, pHeader->UTC[0][1]);
			Len += sprintf(pBuff + Len, "%9.0f%9.0f %-20s\r\n", pHeader->UTC[0][2], pHeader->UTC[0][3],
						"DELTA-UTC: A0,A1,T,W");
		}
	}
	else /**< ver.3 */
	{
		if (SatSysMask & SYS_MASK_GPS)
		{
			if (pEphOpt->Iono)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 164)
				{
					return -1;
				}

				if (fabs(pHeader->Ion[0][0]) < 1E-15 || fabs(pHeader->Ion[0][1]) < 1E-15 ||
					fabs(pHeader->Ion[0][2]) < 1E-15 || fabs(pHeader->Ion[0][3]) < 1E-15)
				{
					Len += sprintf(pBuff + Len, "GPSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\r\n",
									pHeader->Ion[0][0], pHeader->Ion[0][1], pHeader->Ion[0][2],
									pHeader->Ion[0][3], "", "IONOSPHERIC CORR");
				}

				if (fabs(pHeader->Ion[0][4]) > 1E-15 || fabs(pHeader->Ion[0][5]) > 1E-15 ||
					fabs(pHeader->Ion[0][6]) > 1E-15 || fabs(pHeader->Ion[0][7]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "GPSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\r\n",
									pHeader->Ion[0][4], pHeader->Ion[0][5], pHeader->Ion[0][6],
									pHeader->Ion[0][7], "", "IONOSPHERIC CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_GAL)
		{
			if (pEphOpt->Iono)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}
				if (fabs(pHeader->Ion[2][0]) > 1E-15 || fabs(pHeader->Ion[2][1]) > 1E-15 ||
					fabs(pHeader->Ion[2][2]) > 1E-15 || fabs(pHeader->Ion[2][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "GAL  %12.4E%12.4E%12.4E%12.4E%7s%-20s\r\n",
									pHeader->Ion[2][0], pHeader->Ion[2][1], pHeader->Ion[2][2],
									0.0, "", "IONOSPHERIC CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_QZSS)
		{
			if (pEphOpt->Iono)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 164) return -1;
				if (fabs(pHeader->Ion[4][0]) > 1E-15 || fabs(pHeader->Ion[4][1]) > 1E-15 ||
					fabs(pHeader->Ion[4][2]) > 1E-15 || fabs(pHeader->Ion[4][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "QZSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\r\n",
									pHeader->Ion[4][0], pHeader->Ion[4][1], pHeader->Ion[4][2],
									pHeader->Ion[4][3], "", "IONOSPHERIC CORR");
				}

				if (fabs(pHeader->Ion[4][4]) > 1E-15 || fabs(pHeader->Ion[4][5]) > 1E-15 ||
					fabs(pHeader->Ion[4][6]) > 1E-15 || fabs(pHeader->Ion[4][7]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "QZSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\r\n",
									pHeader->Ion[4][4], pHeader->Ion[4][5], pHeader->Ion[4][6],
									pHeader->Ion[4][7], "", "IONOSPHERIC CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_GPS)
		{
			if (pEphOpt->Time)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}

				if (fabs(pHeader->UTC[0][0]) > 1E-15 || fabs(pHeader->UTC[0][1]) > 1E-15 ||
					fabs(pHeader->UTC[0][2]) > 1E-15 || fabs(pHeader->UTC[0][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "GPUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\r\n",
									pHeader->UTC[0][0], pHeader->UTC[0][1], pHeader->UTC[0][2],
									pHeader->UTC[0][3], "", "", "TIME SYSTEM CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_GAL)
		{
			if (pEphOpt->Time)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}

				if (fabs(pHeader->UTC[2][0]) > 1E-15 || fabs(pHeader->UTC[2][1]) > 1E-15 ||
					fabs(pHeader->UTC[2][2]) > 1E-15 || fabs(pHeader->UTC[2][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "GAUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\r\n",
									pHeader->UTC[2][0], pHeader->UTC[2][1], pHeader->UTC[2][2],
									pHeader->UTC[2][3], "", "", "TIME SYSTEM CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_QZSS)
		{   /**< ver.3.02 */
			if (pEphOpt->Time)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}

				if (fabs(pHeader->UTC[4][0]) > 1E-15 || fabs(pHeader->UTC[4][1]) > 1E-15 ||
					fabs(pHeader->UTC[4][2]) > 1E-15 || fabs(pHeader->UTC[4][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "QZUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\r\n",
									pHeader->UTC[4][0], pHeader->UTC[4][1], pHeader->UTC[4][2],
									pHeader->UTC[4][3], "", "", "TIME SYSTEM CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_BDS)
		{   /**< ver.3.02 */
			if (pEphOpt->Time)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}
				if (fabs(pHeader->UTC[3][0]) > 1E-15 || fabs(pHeader->UTC[3][1]) > 1E-15 ||
					fabs(pHeader->UTC[3][2]) > 1E-15 || fabs(pHeader->UTC[3][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "BDUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\r\n",
									pHeader->UTC[3][0], pHeader->UTC[3][1], pHeader->UTC[3][2],
									pHeader->UTC[3][3], "", "", "TIME SYSTEM CORR");
				}
			}
		}

		if (SatSysMask & SYS_MASK_NAVIC)
		{   /**< ver.3.02 */
			if (pEphOpt->Time)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 82)
				{
					return -1;
				}
				if (fabs(pHeader->UTC[3][0]) > 1E-15 || fabs(pHeader->UTC[3][1]) > 1E-15 ||
					fabs(pHeader->UTC[3][2]) > 1E-15 || fabs(pHeader->UTC[3][3]) > 1E-15)
				{
					Len += sprintf(pBuff + Len, "IRUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\r\n",
									pHeader->UTC[3][0], pHeader->UTC[3][1], pHeader->UTC[3][2],
									pHeader->UTC[3][3], "", "", "TIME SYSTEM CORR");
				}
			}
		}
	}

//	if (pEphOpt->Leapsecs)
//	{
//		if (pHeader->Leapsecs < 0 || pHeader->Leapsecs>255)
//		{
//			return -2;
//		}
//		if (Len >= RINEX_MAX_BUF_LEN - 82)
//		{
//			return -1;
//		}
//		Len += sprintf(pBuff + Len, "%6d%54s%-20s\r\n", pHeader->Leapsecs, "", "LEAP SECONDS");
//	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return -1;
	}

	Len += sprintf(pBuff + Len, "%60s%-20s\r\n", "", "END OF HEADER");

	return Len;
}

/**********************************************************************//**
@brief  output rinex glonass nav header field

@param pBuff		[In] rinex data string buffer
@param pEphOpt	  [In] rniex eph output struct
@param pHeader	  [In] rinex glonass eph header struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXGNavHeaderEncode(char* pBuff, const RINEX_EPH_OPT_T* pEphOpt, const RINEX_GEPH_HEADER_T *pHeader)
{
	int Index;
	int Len = 0;
	char Date[64];

	if (pEphOpt->Ver == 0)
	{
		return -1;
	}

	RinexGetTimeStr(Date);

	if (pEphOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
										"GLONASS NAV DATA", "", "RINEX VERSION / TYPE");
	}
	else /**< ver.3 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "%9.2f           %-20s%-20s%-20s\r\n", (double)pEphOpt->Ver / 100,
										"N: GNSS NAV DATA", "R: GLONASS", "RINEX VERSION / TYPE");
	}

	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return -1;
	}

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->Pgm, pHeader->Runby, Date,
								"PGM / RUN BY / DATE");

	if (pHeader->NCom < 0 || pHeader->NCom > MAXCOMMENT)
	{
		return -1;
	}

	for (Index = 0; Index < pHeader->NCom; Index++)
	{
		if (!*pHeader->Comments[Index])
		{
			continue;
		}
		if (Len >= RINEX_MAX_BUF_LEN - 82)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", pHeader->Comments[Index], "COMMENT");
	}

//	if (pEphOpt->Leapsecs)
//	{
//		if (pHeader->Leapsecs < 0 || pHeader->Leapsecs > 255)
//		{
//			return -2;
//		}
//
//		if (Len >= RINEX_MAX_BUF_LEN - 82)
//		{
//			return -1;
//		}
//
//		Len += sprintf(pBuff + Len, "%6d%54s%-20s\r\n", pHeader->Leapsecs, "", "LEAP SECONDS");
//	}
	if (Len >= RINEX_MAX_BUF_LEN - 82)
	{
		return -1;
	}

	Len += sprintf(pBuff + Len, "%60s%-20s\r\n", "", "END OF HEADER");

	return Len;
}

/**********************************************************************//**
@brief  output rinex mete header field

@param pBuff		[In] rinex data string buffer
@param pHeader	  [In] rinex mete header struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXMeteHeaderEncode(char *pBuff, const RINEX_METE_HEADER_T *pHeader)
{
	double Pos[3] = { 0 };
	int Index = 0;
	char Date[32];
	int Len = 0;

	if (Len >= RINEX_MAX_BUF_LEN - 13 * 82)
	{
		return -1;
	}

	RinexGetTimeStr(Date);

	Len += sprintf(pBuff + Len, "%9.2f%-11s%-20s%-20s%-20s\r\n", (double)pHeader->Ver / 100, "", "METEOROLOGICAL DATA",
								"", "RINEX VERSION / TYPE");

	Len += sprintf(pBuff + Len, "%-20.20s%-20.20s%-20.20s%-20s\r\n", pHeader->Pgm, pHeader->Runby, Date,
								"PGM / RUN BY / DATE");
	Len += sprintf(pBuff + Len, "%-60sMARKER NAME         \r\n", pHeader->Marker);
	Len += sprintf(pBuff + Len, "%-60sMARKER NUMBER       \r\n", pHeader->MarkerNo);
	Len += sprintf(pBuff + Len, "     3    PR    TD    HR                                    # / TYPES OF OBSERV \r\n");
	Len += sprintf(pBuff + Len, "%-20s%-20s      %7.1f    %-2s SENSOR MOD/TYPE/ACC \r\n", pHeader->SensorInfo[0], pHeader->SensorInfo[1],
					0.0, "PR");
	Len += sprintf(pBuff + Len, "%-20s%-20s      %7.1f    %-2s SENSOR MOD/TYPE/ACC \r\n", pHeader->SensorInfo[0], pHeader->SensorInfo[2],
					0.2, "TD");
	Len += sprintf(pBuff + Len, "%-20s%-20s      %7.1f    %-2s SENSOR MOD/TYPE/ACC \r\n", pHeader->SensorInfo[0], pHeader->SensorInfo[3],
					0.2, "HR");

	for (Index = 0; Index < 3; Index++)
	{
		if (fabs(pHeader->Pos[Index]) < 1E8)
		{
			Pos[Index] = pHeader->Pos[Index];
		}
	}
	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%14.4f %-2s SENSOR POS XYZ\r\n", Pos[0], Pos[1], Pos[2], pHeader->H,
								"PR");
	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%14.4f %-2s SENSOR POS XYZ\r\n", Pos[0], Pos[1], Pos[2], pHeader->H,
								"TD");
	Len += sprintf(pBuff + Len, "%14.4f%14.4f%14.4f%14.4f %-2s SENSOR POS XYZ\r\n", Pos[0], Pos[1], Pos[2], pHeader->H,
								"HR");
	Len += sprintf(pBuff + Len, "%-60.60s%-20s\r\n", "", "END OF HEADER");

	return Len;
}

/**********************************************************************//**
@brief  output rinex obs body field

@param pBuff		[In] rinex data string buffer
@param pObsOpt	  [In] rniex obs output struct
@param pObs		 [In] rinex obs struct
@param SatNumMax	[In] satellite num max
@param Flag		 [In] flag

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXObsBodyEncode(char *pBuff, RINEX_OBS_OPT_T* pObsOpt, GNSS_OBS_SAT_DATA_T* pObsSatData, int Flag)
{
	char Mask[64];
	char SatCode[MAX_OBS_NUM][5] = { "" };
	int ObsIndex, Index, ObsDataIndex, SysIndex = 0, SatNum= 0, SignalIndex = 0, SatIndex, ExSystemID, PRN, LastPRN = 0, IndexTable[MAX_OBS_NUM], SatSysTable[MAX_OBS_NUM] = { 0 };
	int Len = 0;
	UTC_TIME_T TempUTCTime;
	GNSS_OBS_PER_SAT_T* pObsPerSat = NULL;
	unsigned int PhaseLock = 0;
	unsigned int HalfCycle = 0;
	unsigned int RinexSysIndex;
	int SatCount = 0;

	if (pObsOpt->Ver == 0)
	{
		return -1;
	}

	if (pObsSatData->SatNum == 0)
	{
		return -1;
	}

	TempUTCTime = UTCAdd(&pObsSatData->ObsTime, pObsOpt->Leaps*1000);

	if (((UTCDiff(&pObsOpt->TimeStart, &pObsSatData->ObsTime) > 0) ||
		(UTCDiff(&pObsSatData->ObsTime, &pObsOpt->TimeEnd) > 0)) && (pObsOpt->TimeEnd.Year != 0))
	{
		return -2;
	}

	SatNum = pObsSatData->SatNum;
	for (SatIndex = 0; SatIndex < SatNum; SatIndex++)
	{
		pObsPerSat = &pObsSatData->ObsPerData[SatIndex];

		ExSystemID = pObsPerSat->ExSystemID;
		PRN = pObsPerSat->SatID;

		if ((ExSystemID < 0) || !(pObsOpt->NavSys & (1 << ExSystemID)))
		{
			continue;
		}

		if ((pObsOpt->Ver <= 301) && ((ExSystemID == BDS) || (ExSystemID == QZS) || (ExSystemID == NIC) || (ExSystemID == SBA)))
		{
			continue;
		}

		if ((pObsOpt->Ver == 302) && (ExSystemID == NIC))
		{
			continue;
		}

		if (Sat2Code(ExSystemID, PRN, SatCode[SatIndex]))
		{
			continue;
		}

		RinexSysIndex = ExSystemToRinexSysIndex(ExSystemID);
		if (NA8 == RinexSysIndex)
		{
			continue;
		}

		SatSysTable[SatIndex] = RinexSysIndex;

		if (!pObsOpt->NObs[pObsOpt->Ver < 299 ? 0 : SatSysTable[SatIndex]])
		{
			continue;
		}
		IndexTable[SatIndex] = SatIndex;

		SatCount++;
	}

	if (pObsOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 32)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, " %02d %2d %2d %2d %2d %2d.%03d0000  %d%3d",
			(int)TempUTCTime.Year % 100, TempUTCTime.Month, TempUTCTime.Day, TempUTCTime.Hour, TempUTCTime.Minute, TempUTCTime.Second, TempUTCTime.Millisecond, Flag, SatCount);
		for (ObsIndex = 0; ObsIndex < SatCount; ObsIndex++)
		{
			if (ObsIndex > 0 && ObsIndex % 12 == 0)
			{
				if (Len >= RINEX_MAX_BUF_LEN - 34)
				{
					return -1;
				}
				Len += sprintf(pBuff + Len, "\r\n%32s", "");
			}
			if (Len >= RINEX_MAX_BUF_LEN - 3)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "%-3s", SatCode[ObsIndex]);
		}
	}

	if (pObsOpt->Ver >= 299 && pObsOpt->Ver < 303) /**< ver.3.02 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 37)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "> %04d %2d %2d %2d %2d %2d.%03d0000  %d%3d\r\n",
						TempUTCTime.Year, TempUTCTime.Month, TempUTCTime.Day, TempUTCTime.Hour, TempUTCTime.Minute, TempUTCTime.Second, TempUTCTime.Millisecond, Flag, SatCount);
	}

	/** ver.3.03/3.04 */
	if (pObsOpt->Ver >= 303)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 37)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "> %04d %2d %2d %2d %2d %2d.%03d0000  %d%3d\r\n",
						TempUTCTime.Year, TempUTCTime.Month, TempUTCTime.Day, TempUTCTime.Hour, TempUTCTime.Minute, TempUTCTime.Second, TempUTCTime.Millisecond, Flag, SatCount);
	}

	for (SatIndex = 0; SatIndex < SatNum; SatIndex++)
	{
		ExSystemID = pObsSatData->ObsPerData[SatIndex].ExSystemID;

		if ((pObsOpt->Ver <= 301) && ((ExSystemID == BDS) || (ExSystemID == QZS) || (ExSystemID == NIC) || (ExSystemID == SBA)))
		{
			continue;
		}

		if ((pObsOpt->Ver == 302) && (ExSystemID == NIC))
		{
			continue;
		}

		if ((ExSystemID < 0) || !(pObsOpt->NavSys & (1 << ExSystemID)))
		{
			continue;
		}

		/** ver.2 */
		SysIndex = 0;
		for (Index = 0; Index < 64; Index++)
		{
			Mask[Index] = pObsOpt->Mask[SatSysTable[SatIndex]][Index];
		}
		if (pObsOpt->Ver < 299)
		{
			for (Index = 0; Index < pObsOpt->NObs[SysIndex]; Index++)
			{
				if (Index % 5 == 0)
				{
					if (Len >= RINEX_MAX_BUF_LEN - 2)
					{
						return -1;
					}
					Len += sprintf(pBuff + Len, "\r\n");
				}

				/** search pObs data index */
				if ((ObsDataIndex = RINEXObsIndex(pObsOpt->Ver, ExSystemID, &pObsSatData->ObsPerData[IndexTable[SatIndex]], (char*)pObsOpt->ObsTypeBuf[SysIndex][Index], Mask, pObsSatData->DataType)) < 0)
				{
					if (Len >= RINEX_MAX_BUF_LEN - 16)
					{
						return -1;
					}
					Len += sprintf(pBuff + Len, "%14.3f  ", 0.0);
					continue;
				}
				if (Len >= RINEX_MAX_BUF_LEN - 16)
				{
					return -1;
				}
				/** output field */
				switch (pObsOpt->ObsTypeBuf[SysIndex][Index][0])
				{
					case 'C':
					case 'P':
					{
						double PSR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].PSR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].PSR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", PSR + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'L':
					{
						double ADR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ADR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
							PhaseLock = GET_CH_TRACK_STATUS_PHASELOCK(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus);
							HalfCycle = GET_CH_TRACK_STATUS_HALFCYCLE(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus) ? 0 : 1;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ADR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
							PhaseLock = GET_CH_TRACK_STATUS_ADR_VALID_FLAG(pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ChnTrackStatus);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f%u%d", ADR + 1E-9, (!PhaseLock), SignalStrength(CN0));
					}
						break;
					case 'D':
					{
						float Doppler = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].Doppler;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].Doppler;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", Doppler + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'S':
					{
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f  ", CN0 + 1E-9);
					}
						break;
				}
			}
		}

		/** ver.3.02 */
		SysIndex = SatSysTable[SatIndex];

		if (pObsOpt->Ver >= 299 && pObsOpt->Ver < 303)
		{
			if (Len >= RINEX_MAX_BUF_LEN - 3)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "%-3s", SatCode[SatIndex]);
			for (Index = 0; Index < pObsOpt->NObs[SysIndex]; Index++)
			{
				/** search pObs data index */
				if ((ObsDataIndex = RINEXObsIndex(pObsOpt->Ver, ExSystemID, &pObsSatData->ObsPerData[IndexTable[SatIndex]], (char*)pObsOpt->ObsTypeBuf[SysIndex][Index], Mask, pObsSatData->DataType)) < 0)
				{
					if (Len >= RINEX_MAX_BUF_LEN - 16)
					{
						return -1;
					}
					Len += sprintf(pBuff + Len, "%14.3f  ", 0.0);
					continue;
				}
				if (Len >= RINEX_MAX_BUF_LEN - 16)
				{
					return -1;
				}
				/** output field */
				switch (pObsOpt->ObsTypeBuf[SysIndex][Index][0])
				{
					case 'C':
					case 'P':
					{
						double PSR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].PSR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].PSR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", PSR + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'L':
					{
						double ADR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ADR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
							PhaseLock = GET_CH_TRACK_STATUS_PHASELOCK(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus);
							HalfCycle = GET_CH_TRACK_STATUS_HALFCYCLE(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus) ? 0 : 1;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ADR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
							PhaseLock = GET_CH_TRACK_STATUS_ADR_VALID_FLAG(pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ChnTrackStatus);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f%u%d", ADR + 1E-9, (!PhaseLock), SignalStrength(CN0));
					}
						break;
					case 'D':
					{
						float Doppler = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].Doppler;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].Doppler;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", Doppler + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'S':
					{
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f  ", CN0 + 1E-9);
					}
						break;
				}
			}
			if (Len >= RINEX_MAX_BUF_LEN - 2)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "\r\n");
		}

		/** ver.3.03 3.04 */
		if (pObsOpt->Ver >= 303)
		{
			if (Len >= RINEX_MAX_BUF_LEN - 3)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "%-3s", SatCode[SatIndex]);

			for (Index = 0; Index < pObsOpt->NObs[SysIndex]; Index++)
			{
				/** search pObs data index */
				if ((ObsDataIndex = RINEXObsIndex(pObsOpt->Ver, ExSystemID, &pObsSatData->ObsPerData[IndexTable[SatIndex]], (char*)pObsOpt->ObsTypeBuf[SysIndex][Index], Mask, pObsSatData->DataType)) < 0)
				{
					if (Len >= RINEX_MAX_BUF_LEN - 16)
					{
						return -1;
					}
					Len += sprintf(pBuff + Len, "%14.3f  ", 0.0);
					continue;
				}
				if (Len >= RINEX_MAX_BUF_LEN - 16)
				{
					return -1;
				}

				/** output field */
				switch (pObsOpt->ObsTypeBuf[SysIndex][Index][0])
				{
					case 'C':
					case 'P':
					{
						double PSR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].PSR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							PSR = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].PSR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", PSR + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'L':
					{
						double ADR = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ADR;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
							PhaseLock = GET_CH_TRACK_STATUS_PHASELOCK(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus);
							HalfCycle = GET_CH_TRACK_STATUS_HALFCYCLE(pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].ChnTrackStatus) ? 0 : 1;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							ADR = -pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ADR;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
							PhaseLock = GET_CH_TRACK_STATUS_ADR_VALID_FLAG(pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].ChnTrackStatus);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f%u%d", ADR + 1E-9, (!PhaseLock), SignalStrength(CN0));
					}
						break;
					case 'D':
					{
						float Doppler = 0;
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].Doppler;
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							Doppler = pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].Doppler;
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f %d", Doppler + 1E-9, SignalStrength(CN0));
					}
						break;
					case 'S':
					{
						float CN0 = 0;

						if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_RANGE)
						{
							CN0 = pObsSatData->ObsPerData[IndexTable[SatIndex]].ObsData[ObsDataIndex].CN0;
						}
						else if (pObsSatData->DataType == GNSS_OBS_DATA_TYPE_GNSSOBS)
						{
							CN0 = (float)DoubleScale2((double)pObsSatData->ObsPerData[IndexTable[SatIndex]].GNSSObsData[ObsDataIndex].CN0, 7);
						}
						else
						{
							return -1;
						}
						Len += sprintf(pBuff + Len, "%14.3f  ", CN0 + 1E-9);
					}
						break;
				}
			}

			if (Len >= RINEX_MAX_BUF_LEN - 2)
			{
				return -1;
			}

			Len += sprintf(pBuff + Len, "\r\n");
		}
	}

	if (pObsOpt->Ver < 299)
	{
		if (Len >= RINEX_MAX_BUF_LEN - 2)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "\r\n");
	}

	return Len;
}

/*****************************************************************************//**
@brief adjust weekly rollover of bdt time

@param Week			 [In] week
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

/**********************************************************************//**
@brief  output rinex nav body field

@param pBuff		[In] rinex data string buffer
@param pEphOpt	  [In] rniex eph output struct
@param pEphData	 [In] rinex eph struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXNavBodyEncode(unsigned int EncodeID, char *pBuff, RINEX_EPH_OPT_T* pEphOpt, UNI_EPHEMERIS_T* pEphData)
{
	int SatSys, PRN, SatNo;
	char Code[32], *SepString=NULL;
	int Len = 0, Week;
	int CodeFlag, L2Code, L2PDataFlag;
	UTC_TIME_T TempUtcTime;
	int LeapMSec = pEphOpt->Leapsecs * 1000;
	int TOE = 0, Health;
	double SqrtA;
	int GPSLeapSecMs;
	unsigned short EphWeek;
	char* pMsgType = NULL;

	if (pEphOpt->Ver == 0)
	{
		return -1;
	}

	SatSys = pEphData->SystemID;
	if (SatSys == QZS)
	{
		PRN = pEphData->SatID + 192;
	}
	else
	{
		PRN = pEphData->SatID;
	}

	SatNo = GetAllSatIndex(SatSys, PRN);
	if ((SatNo <0 ) || (pEphOpt->NavSys < 0))
	{
		return 0;
	}

	if (pEphOpt->ExSats[SatNo] == 1)
	{
		return 0;
	}

	if ((EncodeID == RINEX_ID_GALEPH) || (EncodeID == RINEX_ID_GALEPH_FNAV) || (EncodeID == RINEX_ID_GALEPH_INAV) || (EncodeID == RINEX_ID_GALEPH_RCED))
	{
		EphWeek = pEphData->Week + 1024;
	}
	else
	{
		EphWeek = pEphData->Week;
	}

	if ((EncodeID == RINEX_ID_BDSEPH) || (EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		Week = AdjBDTimeWeek(EphWeek);
		Week +=  1356;
		TOE = pEphData->TOE + 14;
		GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(Week, (int)pEphData->TOE*1000);
		GPSTimeToUTC(Week, (int)TOE*1000, &GPSLeapSecMs, &TempUtcTime);
	}
	else
	{
		GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(EphWeek, (int)pEphData->TOE * 1000);
		GPSTimeToUTC(EphWeek, (int)pEphData->TOE*1000, &GPSLeapSecMs, &TempUtcTime);
	}

	TempUtcTime = UTCAdd(&TempUtcTime, GPSLeapSecMs);

	if (Sat2Code(SatSys, PRN, Code))
	{
		return 0;
	}

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 22)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "%02d %02d %02d %02d %02d %02d %02d.%01d", PRN,
						(int)TempUtcTime.Year % 100, TempUtcTime.Month, TempUtcTime.Day,
						TempUtcTime.Hour, TempUtcTime.Minute, TempUtcTime.Second, TempUtcTime.Millisecond/100);
		SepString = "   ";
	}
	else
	{
		if (pEphOpt->Ver >= 400)
		{
			if ((EncodeID == RINEX_ID_GPSEPH) || (EncodeID == RINEX_ID_QZSSEPH))
			{
				pMsgType = "LNAV";
			}
			else if ((EncodeID == RINEX_ID_GALEPH) || (EncodeID == RINEX_ID_GALEPH_INAV) || (EncodeID == RINEX_ID_GALEPH_RCED))
			{
				pMsgType = "INAV";
			}
			else if (EncodeID == RINEX_ID_GALEPH_FNAV)
			{
				pMsgType = "FNAV";
			}
			else if (EncodeID == RINEX_ID_SBASEPH)
			{
				pMsgType = "SBAS";
			}
			else if (EncodeID == RINEX_ID_NICEPH)
			{
				pMsgType = "LNAV";
			}
			else
			{
				return -1;
			}

			Len += sprintf(pBuff + Len, ">EPH %-3s %s\r\n", Code, pMsgType);
		}

		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 23)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "%-3s %04d %02d %02d %02d %02d %02d", Code,
						TempUtcTime.Year, TempUtcTime.Month, TempUtcTime.Day,
						TempUtcTime.Hour, TempUtcTime.Minute, TempUtcTime.Second);
		SepString = "    ";
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af1);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af2);

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphData->IODE1); /**< GPS/QZS: IODE, GAL: IODnav, BDS: AODE */
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Crs);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->DeltaN);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->M0);

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cuc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Ecc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cus);
	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		SqrtA = sqrt(pEphData->A);
		RINEXGetNavDataStr(pBuff, &Len, SqrtA);
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->SqrtAorDeltaA);
	}

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphData->TOE);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cic);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Omega0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cis);


	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphData->I0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Crc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Omega);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->OmegaDot);

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}

	CodeFlag = GetCodeFlag(EncodeID);

	L2Code = CodeFlag & 0xFFFF;
	L2PDataFlag = (CodeFlag >> 16) & 0x01;

	RINEXGetNavDataStr(pBuff, &Len, pEphData->IDot);
	if ((EncodeID == RINEX_ID_GPSEPH) || (EncodeID == RINEX_ID_QZSSEPH))
	{
		RINEXGetNavDataStr(pBuff, &Len, L2Code);
	}
	else if ((EncodeID == RINEX_ID_GALEPH) || (EncodeID == RINEX_ID_GALEPH_INAV) || (EncodeID == RINEX_ID_GALEPH_RCED))
	{
		int INAV_E1B, INAV_E5B;
		INAV_E1B = pEphData->DataValidStatus & 0x01;
		INAV_E5B = (pEphData->DataValidStatus >> 1) & 0x01;
		RINEXGetNavDataStr(pBuff, &Len, (INAV_E1B + (INAV_E5B << 2)));
	}
	else if ((EncodeID == RINEX_ID_GALEPH_FNAV))
	{
		int INAV_E5A = (pEphData->DataValidStatus >> 2) & 0x01;
		RINEXGetNavDataStr(pBuff, &Len, (INAV_E5A << 1));
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
	}
	RINEXGetNavDataStr(pBuff, &Len, EphWeek);
	if ((EncodeID == RINEX_ID_GPSEPH) || (EncodeID == RINEX_ID_QZSSEPH))
	{
		RINEXGetNavDataStr(pBuff, &Len, L2PDataFlag);
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
	}

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}

//	if (SatSys == GAL)
//	{
//		RINEXGetNavDataStr(pBuff, &Len, SISAValue(pEphData->URA));
//	}
//	else
	{
		RINEXGetNavDataStr(pBuff, &Len, UraValue(pEphData->URA));
	}

	if (EncodeID == RINEX_ID_GALEPH_FNAV)
	{
		Health = ((pEphData->Health >> 2) << 4) + ((pEphData->DataValidStatus >> 2) << 3);
		RINEXGetNavDataStr(pBuff, &Len, Health);
	}
	else if ((EncodeID == RINEX_ID_GALEPH_INAV) || (EncodeID == RINEX_ID_GALEPH_RCED))
	{
		Health = ((pEphData->Health & 0x1) << 1) + (pEphData->DataValidStatus & 0x1);
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Health);
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Health);
	}

	RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[0]); /**< GPS/QZS:TGD, GAL:BGD E5a/E1, BDS: TGD1 B1/B3 */
	if (SatSys == GAL || SatSys == BDS)
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[1]); /**< GAL:BGD E5b/E1, BDS: TGD2 B2/B3 */
	}
	else if (SatSys == NIC)
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->IODC); /**< GPS/QZS:IODC */
	}

	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}

	if (SatSys == BDS)
	{
		RINEXGetNavDataStr(pBuff, &Len, TOE);
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->TOE);
	}

	if (SatSys == GPS || SatSys == QZS)
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->FitInterval);
	}
	else if (SatSys == BDS)
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->IODC); /**< AODC */
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
	}

//	if ((SatSys != GAL) && (SatSys != GPS))
//	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0);
		RINEXGetNavDataStr(pBuff, &Len, 0.0);
//	}

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n");

	return Len;
}

/**********************************************************************//**
@brief  output rinex nav body field BDS CNAV

@param pBuff		[In] rinex data string buffer
@param pEphOpt	  [In] rniex eph output struct
@param pEphData	 [In] rinex eph struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXNavVer4BodyEncodeBDS(unsigned int EncodeID, char *pBuff, RINEX_EPH_OPT_T* pEphOpt, UNI_EPHEMERIS_T* pEphData)
{
	int SatSys, PRN, SatNo;
	char Code[32], *SepString=NULL;
	int Len = 0, Week;
	UTC_TIME_T TempUtcTime;
	int LeapMSec = pEphOpt->Leapsecs * 1000;
	int TOE = 0;
	double SqrtA;
	int GPSLeapSecMs;
	unsigned short EphWeek;
	char* pMsgType = NULL;

	if (pEphOpt->Ver == 0)
	{
		return -1;
	}

	SatSys = pEphData->SystemID;
	PRN = pEphData->SatID;

	SatNo = GetAllSatIndex(SatSys, PRN);
	if ((SatNo <0 ) || (pEphOpt->NavSys < 0))
	{
		return 0;
	}

	if (pEphOpt->ExSats[SatNo] == 1)
	{
		return 0;
	}

	EphWeek = pEphData->Week;

	Week = AdjBDTimeWeek(EphWeek);
	Week +=  1356;
	TOE = pEphData->TOE + 14;
	GPSLeapSecMs = GetGPSLeapSecondMsByGPSTime(Week, (int)pEphData->TOE*1000);
	GPSTimeToUTC(Week, (int)TOE*1000, &GPSLeapSecMs, &TempUtcTime);

	TempUtcTime = UTCAdd(&TempUtcTime, GPSLeapSecMs);

	if (Sat2Code(SatSys, PRN, Code))
	{
		return 0;
	}

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 23)
	{
		return -1;
	}

	if (EncodeID == RINEX_ID_BDSEPH)
	{
		if (IS_BDS_GEO_PRN(pEphData->SatID))
		{
			pMsgType = "D2";
		}
		else
		{
			pMsgType = "D1";
		}
	}
	else if (EncodeID == RINEX_ID_BDSEPH_CNAV1)
	{
		pMsgType = "CNV1";
	}
	else if (EncodeID == RINEX_ID_BDSEPH_CNAV2)
	{
		pMsgType = "CNV2";
	}
	else if (EncodeID == RINEX_ID_BDSEPH_CNAV3)
	{
		pMsgType = "CNV3";
	}
	else
	{
		return -1;
	}

	Len += sprintf(pBuff + Len, ">EPH %-3s %s\r\n", Code, pMsgType);

	Len += sprintf(pBuff + Len, "%-3s %04d %02d %02d %02d %02d %02d", Code,
					TempUtcTime.Year, TempUtcTime.Month, TempUtcTime.Day,
					TempUtcTime.Hour, TempUtcTime.Minute, TempUtcTime.Second);
	SepString = "    ";

	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af1);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Af2);

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);

	RINEXGetNavDataStr(pBuff, &Len, pEphData->IODE1); /**< GPS/QZS: IODE, GAL: IODnav, BDS: AODE */
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Crs);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->DeltaN);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->M0);

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cuc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Ecc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cus);
	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		SqrtA = sqrt(pEphData->A);
		RINEXGetNavDataStr(pBuff, &Len, SqrtA);
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, pEphData->SqrtAorDeltaA);
	}

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->TOE);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cic);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Omega0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Cis);

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);

	RINEXGetNavDataStr(pBuff, &Len, pEphData->I0);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Crc);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->Omega);
	RINEXGetNavDataStr(pBuff, &Len, pEphData->OmegaDot);

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);

	RINEXGetNavDataStr(pBuff, &Len, pEphData->IDot);
	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		int SatType = 0; /**< reserved */
		RINEXGetNavDataStr(pBuff, &Len, pEphData->DeltaN);
		if (IS_BDS_GEO_PRN(pEphData->SatID))
		{
			SatType = 1; /**< GEO */
		}
		else if (IS_BD3_NON_GEO_PRN(pEphData->SatID))
		{
			SatType = 3; /**< MEO */
		}
		RINEXGetNavDataStr(pBuff, &Len, SatType);
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< todo: need modify t_op */
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
		RINEXGetNavDataStr(pBuff, &Len, EphWeek);
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
	}

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);

	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< todo: need modify SISAI_oe */
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< todo: need modify SISAI_ocb */
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< todo: need modify SISAI_oc1 */
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< todo: need modify SISAI_oc2 */
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, UraValue(pEphData->URA));
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Health);
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[0]); /**< GPS/QZS:TGD, GAL:BGD E5a/E1, BDS: TGD1 B1/B3 */
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[1]); /**< GAL:BGD E5b/E1, BDS: TGD2 B2/B3 */
	}

	if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
	{
		return -1;
	}
	Len += sprintf(pBuff + Len, "\r\n%s", SepString);

	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		
		 if (EncodeID == RINEX_ID_BDSEPH_CNAV1)
		{
			RINEXGetNavDataStr(pBuff, &Len, pEphData->Isc[0]); /**< ISC_B1Cd */
			RINEXGetNavDataStr(pBuff, &Len, 0.0);
		}
		else if (EncodeID == RINEX_ID_BDSEPH_CNAV2)
		{
			RINEXGetNavDataStr(pBuff, &Len, 0.0);
			RINEXGetNavDataStr(pBuff, &Len, pEphData->Isc[1]); /**< ISC_B2ad */
		}
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[0]); /**< BDS CNAV: TGD_B1Cp*/
		RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[1]); /**< BDS CNAV: TGD_B2ap */
	}
	else
	{
		RINEXGetNavDataStr(pBuff, &Len, TOE);
		RINEXGetNavDataStr(pBuff, &Len, pEphData->IODC); /**< AODC */
		RINEXGetNavDataStr(pBuff, &Len, 0.0);
		RINEXGetNavDataStr(pBuff, &Len, 0.0);
	}

	if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) || (EncodeID == RINEX_ID_BDSEPH_CNAV2) || (EncodeID == RINEX_ID_BDSEPH_CNAV3))
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);

		RINEXGetNavDataStr(pBuff, &Len, pEphData->URA); /**< SISMAI */
		RINEXGetNavDataStr(pBuff, &Len, (pEphData->Health & 0x03)); /**< Health */
		int B1C_DIF, B1C_SIF, B1C_AIF;
		int B2a_DIF, B2a_SIF, B2a_AIF;
		int B2b_DIF, B2b_SIF, B2b_AIF;
		int IntegrityFlags = 0;

		B1C_DIF = pEphData->Reserved & 0x01; /**< BDS CNAV, bit 0-2, for B1C DIF, SIF, AIF, bit 3-5 for B2a DIF, SIF, AIF, bit 6-8 for B2b DIF, SIF, AIF */
		B1C_SIF = ((pEphData->Reserved >> 1) & 0x01);
		B1C_AIF = ((pEphData->Reserved >> 2) & 0x01);
		B2a_DIF = ((pEphData->Reserved >> 3) & 0x01);
		B2a_SIF = ((pEphData->Reserved >> 4) & 0x01);
		B2a_AIF = ((pEphData->Reserved >> 5) & 0x01);
		B2b_DIF = ((pEphData->Reserved >> 6) & 0x01);
		B2b_SIF = ((pEphData->Reserved >> 7) & 0x01);
		B2b_AIF = ((pEphData->Reserved >> 8) & 0x01);

		if (EncodeID == RINEX_ID_BDSEPH_CNAV1)
		{
			/** bit 0-2: AIF(B1C),SIF (B1C),DIF(B1C) */
			IntegrityFlags = B1C_AIF + (B1C_SIF << 1) + (B1C_DIF << 2);
		}
		else if (EncodeID == RINEX_ID_BDSEPH_CNAV2)
		{
			/** bit 0-5: AIF(B1C),SIF (B1C),DIF(B1C),AIF(B2a),SIF(B2a),DIF(B2a) */
			IntegrityFlags = B1C_AIF + (B1C_SIF << 1) + (B1C_DIF << 2) + (B2a_AIF << 3) + (B2a_SIF << 4) + (B2a_DIF << 5);
		}
		else if (EncodeID == RINEX_ID_BDSEPH_CNAV3)
		{
			/** bit 0-2: AIF(B2b),SIF (B2b),DIF(B2b) */
			IntegrityFlags = B2b_AIF + (B2b_SIF << 1) + (B2b_DIF << 2);
		}
		RINEXGetNavDataStr(pBuff, &Len, IntegrityFlags);
		if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) ||(EncodeID == RINEX_ID_BDSEPH_CNAV2))
		{
			RINEXGetNavDataStr(pBuff, &Len, pEphData->IODC); /**< IODC */
		}
		else
		{
			RINEXGetNavDataStr(pBuff, &Len, pEphData->Tgd[2]); /**< BDS CNAV: TGD_B2bI */
		}

		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);

		RINEXGetNavDataStr(pBuff, &Len, TOE); /**< t_tm: Transmission time of message (sec of BDT week) */
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
		RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
		if ((EncodeID == RINEX_ID_BDSEPH_CNAV1) ||(EncodeID == RINEX_ID_BDSEPH_CNAV2))
		{
			RINEXGetNavDataStr(pBuff, &Len, pEphData->IODE1); /**< IODE */
		}
		else
		{
			RINEXGetNavDataStr(pBuff, &Len, 0.0); /**< spare */
		}

		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n");
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n");
	}

	return Len;
}

/**********************************************************************//**
@brief  output rinex nav body field

@param pBuff		[In] rinex data string buffer
@param pEphOpt	  [In] rniex eph output struct
@param pEphData	 [In] rinex eph struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXNavBodyEncodeTotal(unsigned int EncodeID, char *pBuff, RINEX_EPH_OPT_T* pEphOpt, UNI_EPHEMERIS_T* pEphData)
{
	if ((pEphOpt->Ver >= 400) && (pEphData->SystemID == BDS))
	{
		return RINEXNavVer4BodyEncodeBDS(EncodeID, pBuff, pEphOpt, pEphData);
	}
	else
	{
		return RINEXNavBodyEncode(EncodeID, pBuff, pEphOpt, pEphData);
	}
}

/**********************************************************************//**
@brief  output rinex glonass nav body field

@param pBuff			[In] rinex data string buffer
@param pEphOpt		  [In] rniex eph output struct
@param pEphGloData	  [In] rinex glonass eph struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
static int RINEXGLONavBodyEncode(char *pBuff, RINEX_EPH_OPT_T* pEphOpt, GLO_EPHEMERIS_T *pEphGloData)
{
	int PRN, SatSys, SatNo;
	char Code[32], *SepString=NULL;
	int Len = 0;
	UTC_TIME_T TempTime = { 0 };
	unsigned int Health;
	double TK_H, TK_M, TK_S;
	double Tow, Tod, Tof;
	unsigned int TempTof;
	int Week, TempLeapMs, WeekMsCount;

	if (pEphOpt->Ver == 0)
	{
		return -1;
	}

	SatSys = GLO;
	PRN = pEphGloData->SatID;
	SatNo = GetAllSatIndex(SatSys, PRN);

	if ((SatSys != GLO) || !( pEphOpt->NavSys & SYS_MASK_GLO))
	{
		return 0;
	}
	if (pEphOpt->ExSats[SatNo] == 1)
	{
		return 0;
	}

	TempLeapMs = pEphOpt->Leapsecs * 1000;

	TK_H = (pEphGloData->Tk >> 7) & 0x1F;
	TK_M = (pEphGloData->Tk >> 1) & 0x3F;
	TK_S = (pEphGloData->Tk & 0x01) * 30.0;

	Epoch2UTC(pEphGloData->TOE, &TempTime);
	TempTime = UTCAdd(&TempTime, -TempLeapMs);
	UTCToGPSTime(&TempTime, &TempLeapMs, &Week, &WeekMsCount);
	Tow = (double)WeekMsCount / 1000;
	Tod = fmod(Tow, 86400.0);
	Tow -= Tod;
	Tof = TK_H * 3600.0 + TK_M * 60.0 + TK_S - 10800.0; /* lt->utc */
	if (Tof < Tod - 43200.0)
	{
		Tof += 86400.0;
	}
	else if (Tof > Tod + 43200.0)
	{
		Tof -= 86400.0;
	}

	TempTof = GPSTime2Epoch(Week, (int)(Tow + Tof));
	Epoch2UTC(TempTof, &TempTime);
	TempTime = UTCAdd(&TempTime, 10800000);
	Tof=QCTime2GPSTime(UTC2QCTime(&TempTime), NULL); /**< v.3: tow in utc */

	if (Tof<0.0)
	{
		Tof += 604800.0;
	}
	if (pEphOpt->Ver < 299)
	{
		Tof = fmod(Tof, 86400.0); /* v.2: tod in utc */
	}
	else
	{
		Tof = fmod(Tof, 604800.0);
	}

	Epoch2UTC(pEphGloData->TOE, &TempTime);

	if (pEphOpt->Ver < 299) /**< ver.2 */
	{
		if (Len >= RINEX_MAX_BUF_LEN - 22)
		{
			return -1;
		}

		Len += sprintf(pBuff + Len, "%02d %02d %02d %02d %02d %02d %02d.%01d", PRN, (int)TempTime.Year % 100,
						TempTime.Month, TempTime.Day, TempTime.Hour, TempTime.Minute, TempTime.Second, TempTime.Millisecond/100);
		SepString = "   ";
	}
	else /**< ver.3 */
	{
		if (Sat2Code(SatSys, PRN, Code))
		{
			return 0;
		}
		if (Len >= RINEX_MAX_BUF_LEN - 23)
		{
			return -1;
		}

		if (pEphOpt->Ver >= 400)
		{
			Len += sprintf(pBuff + Len, ">EPH %-3s FDMA\r\n", Code);
		}

		Len += sprintf(pBuff + Len, "%-3s %04d %02d %02d %02d %02d %02d", Code, TempTime.Year,
						TempTime.Month, TempTime.Day, TempTime.Hour, TempTime.Minute, TempTime.Second);
		SepString = "    ";
	}

	RINEXGetNavDataStr(pBuff, &Len, -pEphGloData->TauN);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->GammaN);
	RINEXGetNavDataStr(pBuff, &Len, Tof);
	if (pEphOpt->Ver < 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	Health = (pEphGloData->Bn >> 2) & 0x01;
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->PosXYZ[0] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->VelXYZ[0] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->AccXYZ[0] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, Health);

	if (pEphOpt->Ver <= 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString); ;
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->PosXYZ[1] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->VelXYZ[1] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->AccXYZ[1] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->Freq);

	if (pEphOpt->Ver <= 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 5)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	else
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n%s", SepString);
	}
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->PosXYZ[2] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->VelXYZ[2] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->AccXYZ[2] / 1E3);
	RINEXGetNavDataStr(pBuff, &Len, pEphGloData->En);

	if (pEphOpt->Ver <= 299)
	{
		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n");
	}
	else
	{
		if (pEphOpt->Ver >= 305)
		{
			if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 6)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "\r\n%s", SepString);
		}
		else
		{
			if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
			{
				return -1;
			}
			Len += sprintf(pBuff + Len, "\r\n");
		}
	}

	if (pEphOpt->Ver >= 305)
	{
		unsigned int P, P1, P2, P3, P4, M;
		unsigned short StatusFlag;
		P = (pEphGloData->StatusFlag >> 6) & 0x03;
		P1 = pEphGloData->StatusFlag & 0x03;
		P2 = (pEphGloData->StatusFlag >> 2) & 0x01;
		P3 = (pEphGloData->StatusFlag >> 3) & 0x01;
		P4 = (pEphGloData->StatusFlag >> 4) & 0x01;
		M = (pEphGloData->StatusFlag >> 8) & 0x03;
		StatusFlag = P + (P1 << 2) + (P2 << 4) + (P3 << 5) + (P4 << 6) + (M << 7);
		RINEXGetNavDataStr(pBuff, &Len, StatusFlag);
		RINEXGetNavDataStr(pBuff, &Len, pEphGloData->DeltaTauN);
		RINEXGetNavDataStr(pBuff, &Len, pEphGloData->Ft);
		RINEXGetNavDataStr(pBuff, &Len, pEphGloData->Bn);

		if (Len == -1 || Len >= RINEX_MAX_BUF_LEN - 2)
		{
			return -1;
		}
		Len += sprintf(pBuff + Len, "\r\n");
	}

	return Len;
}

/**********************************************************************//**
@brief  generate RINEX

@param EncodeID	 [In] RINEX message id
@param pOutBuff	 [Out] RINEX output data struct
@param pInData	  [In] RINEX init data struct
@param pData		[In] need encode data struct

@retval <0 error, >=0 encode data len

@author CHC
@Date 2023/04/05
@note
**************************************************************************/
int GenerateRINEX(unsigned int EncodeID, char* pOutBuff, void* pInData, void* pData)
{
	int Result = -1;

	if ((pOutBuff == NULL) || (pInData == NULL) || (pData == NULL))
	{
		return -1;
	}

	switch (EncodeID)
	{
		case RINEX_ID_OBS_CRX_HEADER:
			Result = RINEXCrxObsHeaderEncode(pOutBuff, (RINEX_OBS_OPT_T*)pInData, (RINEX_OBS_HEADER_T*)pData);
			break;
		case RINEX_ID_OBS_HEADER:
			Result = RINEXObsHeaderEncode(pOutBuff, (RINEX_OBS_OPT_T*)pInData, (RINEX_OBS_HEADER_T*)pData);
			break;
		case RINEX_ID_MUTIEPH_HEADER:
		case RINEX_ID_GPSEPH_HEADER:
		case RINEX_ID_GALEPH_HEADER:
		case RINEX_ID_BDSEPH_HEADER:
		case RINEX_ID_QZSSEPH_HEADER:
		case RINEX_ID_SBASEPH_HEADER:
		case RINEX_ID_NICEPH_HEADER:
			Result = RINEXNavHeaderEncode(pOutBuff, (RINEX_EPH_OPT_T*)pInData, (RINEX_EPH_HEADER_T*)pData, EncodeID);
			break;
		case RINEX_ID_GLOEPH_HEADER:
			Result = RINEXGNavHeaderEncode(pOutBuff, (RINEX_EPH_OPT_T*)pInData, (RINEX_GEPH_HEADER_T*)pData);
			break;
		case RINEX_ID_METE_HEADER:
			Result = RINEXMeteHeaderEncode(pOutBuff, (RINEX_METE_HEADER_T*)pData);
			break;
		case RINEX_ID_OBS:
			Result = RINEXObsBodyEncode(pOutBuff, (RINEX_OBS_OPT_T*)pInData, (GNSS_OBS_SAT_DATA_T* )pData, 0);
			break;
		case RINEX_ID_GPSEPH:
		case RINEX_ID_BDSEPH:
		case RINEX_ID_BDSEPH_CNAV1:
		case RINEX_ID_BDSEPH_CNAV2:
		case RINEX_ID_BDSEPH_CNAV3:
		case RINEX_ID_GALEPH:
		case RINEX_ID_GALEPH_FNAV:
		case RINEX_ID_GALEPH_INAV:
		case RINEX_ID_GALEPH_RCED:
		case RINEX_ID_QZSSEPH:
		case RINEX_ID_SBASEPH:
		case RINEX_ID_NICEPH:
			Result = RINEXNavBodyEncodeTotal(EncodeID, pOutBuff, (RINEX_EPH_OPT_T*)pInData, (UNI_EPHEMERIS_T*)pData);
			break;
		case RINEX_ID_GLOEPH:
			Result = RINEXGLONavBodyEncode(pOutBuff, (RINEX_EPH_OPT_T*)pInData, (GLO_EPHEMERIS_T*)pData);
			break;
		default:
			break;
	}

	return Result;
}

#endif
