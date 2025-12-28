#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RinexDecoder.c
@author CHC
@date   2023/12/19
@brief

**************************************************************************/
#include <stdio.h>
#include <math.h>

#include "OSDependent.h"

#include "RinexDecoder.h"

#include "Common/GNSSConstants.h"
#include "Common/NavDataCommonFuncs.h"

#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"

#include "../../Common/SupportFuncs.h"

#include "../../GNSSPrint.h"

#include "HAL/DriverCycleDataBuffer.h"

/** constants/macros ----------------------------------------------------------*/
#define SQR(x)      ((x)*(x))

#define MAXRNXLEN   (16 * MAX_NUM_OBS_TYPE + 4)   /**< max rinex record length */
#define MAXPOSHEAD  1024                /**< max head line position */
#define MINFREQ_GLO -7                  /**< min frequency number glonass */
#define MAXFREQ_GLO 13                  /**< max frequency number glonass */

#define EXTEND_OBS_CODE_FREQ_NUM		0

#define GPS_EPH_SAT_TYPE_LNAV		"LNAV"
#define GPS_EPH_SAT_TYPE_CNAV		"CNAV"
#define GPS_EPH_SAT_TYPE_CNAV2		"CNV2"

#define GAL_EPH_SAT_TYPE_INAV		"INAV"
#define GAL_EPH_SAT_TYPE_FNAV		"FNAV"

#define GLO_EPH_SAT_TYPE_FDMA		"FDMA"

#define QZS_EPH_SAT_TYPE_LNAV		"LNAV"
#define QZS_EPH_SAT_TYPE_CNAV		"CNAV"
#define QZS_EPH_SAT_TYPE_CNAV2		"CNV2"

#define BDS_EPH_SAT_TYPE_D1			"D1"
#define BDS_EPH_SAT_TYPE_D2			"D2"
#define BDS_EPH_SAT_TYPE_CNAV1		"CNV1"
#define BDS_EPH_SAT_TYPE_CNAV2		"CNV2"
#define BDS_EPH_SAT_TYPE_CNAV3		"CNV3"

#define SBA_EPH_SAT_TYPE_SBAS		"SBAS"

#define NIC_EPH_SAT_TYPE_LNAV		"LNAV"

typedef enum _SYS_TIME_TYPE_E
{
	SYS_TIME_TYPE_GPS = 0,	/**< time SystemIDtem: GPS time */
	SYS_TIME_TYPE_UTC = 1,	/**< time SystemIDtem: UTC */
	SYS_TIME_TYPE_GLO = 2,	/**< time SystemIDtem: GLONASS time */
	SYS_TIME_TYPE_GAL = 3,	/**< time SystemIDtem: Galileo time */
	SYS_TIME_TYPE_QZS = 4,	/**< time SystemIDtem: QZSS time */
	SYS_TIME_TYPE_BDS = 5,	/**< time SystemIDtem: BeiDou time */
	SYS_TIME_TYPE_NIC = 6,	/**< time SystemIDtem: NAVIC time */

	SYS_TIME_TYPE_MAX,
} SYS_TIME_TYPE_E;

typedef struct _OBS_CODE_FREQ_INFO_T
{
	char* CodeStr;
	unsigned int Freq;
	unsigned int SignalType;
} OBS_CODE_FREQ_INFO_T;

static const OBS_CODE_FREQ_INFO_T s_GPSCodeFreqInfoMap[] =
{
	{"1C",	L1,		RM_RANGE_SIGNAL_GPSL1CA},		/**< L1C */
	{"1S",	L1,		RM_RANGE_SIGNAL_GPSL1CD},		/**< L1S */
	{"1L",	L1,		RM_RANGE_SIGNAL_GPSL1CP},		/**< L1L */
	{"1X",	L1,		RM_RANGE_SIGNAL_GPSL1CX},		/**< L1X */
	{"1P",	L1,		RM_RANGE_SIGNAL_GPSL1P},		/**< L1P */
	{"1W",	L1,		RM_RANGE_SIGNAL_GPSL1W},		/**< L1W */
	{"1Y",	L1,		NA8},							/**< L1Y */
	{"1M",	L1,		NA8},							/**< L1M */
	{"1N",	L1,		NA8},							/**< L1N */
	{"2C",	L2,		RM_RANGE_SIGNAL_GPSL2CA},		/**< L2C */
	{"2D",	L2,		NA8},							/**< L2D */
	{"2S",	L2,		RM_RANGE_SIGNAL_GPSL2CM},		/**< L2S */
	{"2L",	L2,		RM_RANGE_SIGNAL_GPSL2CL},		/**< L2L */
	{"2X",	L2,		RM_RANGE_SIGNAL_GPSL2CX},		/**< L2X */
	{"2P",	L2,		RM_RANGE_SIGNAL_GPSL2P},		/**< L2P */
	{"2W",	L2,		RM_RANGE_SIGNAL_GPSL2PCODELESS},/**< L2W */
	{"2Y",	L2,		NA8},							/**< L2Y */
	{"2M",	L2,		NA8},							/**< L2M */
	{"2N",	L2,		NA8},							/**< L2N */
	{"5I",	L5,		RM_RANGE_SIGNAL_GPSL5I},		/**< L5I */
	{"5Q",	L5,		RM_RANGE_SIGNAL_GPSL5Q},		/**< L5Q */
	{"5X",	L5,		RM_RANGE_SIGNAL_GPSL5X},		/**< L5X */
};

static const OBS_CODE_FREQ_INFO_T s_GLOCodeFreqInfoMap[] =
{
	{"1C",	G1,		RM_RANGE_SIGNAL_GLOL1C},		/**< L1C */
	{"1P",	G1,		RM_RANGE_SIGNAL_GLOL1P},		/**< L1P */
	{"4A",	G4,		RM_RANGE_SIGNAL_GLOL1OCD},		/**< L4A */
	{"4B",	G4,		RM_RANGE_SIGNAL_GLOL1OCP},		/**< L4B */
	{"4X",	G4,		RM_RANGE_SIGNAL_GLOL1OCX},		/**< L4X */
	{"2C",	G2,		RM_RANGE_SIGNAL_GLOL2C},		/**< L2C */
	{"2P",	G2,		RM_RANGE_SIGNAL_GLOL2P},		/**< L2P */
	{"6A",	G6,		RM_RANGE_SIGNAL_GLOL2CSI},		/**< L6A */
	{"6B",	G6,		RM_RANGE_SIGNAL_GLOL2OCP},		/**< L6B */
	{"6X",	G6,		RM_RANGE_SIGNAL_GLOL2OCX},		/**< L6X */
	{"3I",	G3,		RM_RANGE_SIGNAL_GLOL3I},		/**< L3I */
	{"3Q",	G3,		RM_RANGE_SIGNAL_GLOL3Q},		/**< L3Q */
	{"3X",	G3,		RM_RANGE_SIGNAL_GLOL3X},		/**< L3X */
};

static const OBS_CODE_FREQ_INFO_T s_GALCodeFreqInfoMap[] =
{
	{"1A",	E1,		NA8},							/**< L1A */
	{"1B",	E1,		RM_RANGE_SIGNAL_GALE1B},		/**< L1B */
	{"1C",	E1,		RM_RANGE_SIGNAL_GALE1C},		/**< L1C */
	{"1X",	E1,		RM_RANGE_SIGNAL_GALE1X},		/**< L1X */
	{"1Z",	E1,		NA8},							/**< L1Z */
	{"5I ",	E5A,	RM_RANGE_SIGNAL_GALE5AI},		/**< L5I */
	{"5Q",	E5A,	RM_RANGE_SIGNAL_GALE5AQ},		/**< L5Q */
	{"5X",	E5A,	RM_RANGE_SIGNAL_GALE5AX},		/**< L5X */
	{"7I",	E5B,	RM_RANGE_SIGNAL_GALE5BI},		/**< L7I */
	{"7Q",	E5B,	RM_RANGE_SIGNAL_GALE5BQ},		/**< L7Q */
	{"7X",	E5B,	RM_RANGE_SIGNAL_GALE5BX},		/**< L7X */
	{"8I",	E5,		RM_RANGE_SIGNAL_GALBOCI},		/**< L8I */
	{"8Q",	E5,		RM_RANGE_SIGNAL_GALBOCQ},		/**< L8Q */
	{"8X",	E5,		RM_RANGE_SIGNAL_GALBOCX},		/**< L8X */
	{"6A",	E6,		NA8},							/**< L6A */
	{"6B",	E6,		RM_RANGE_SIGNAL_GALE6B},		/**< L6B */
	{"6C",	E6,		RM_RANGE_SIGNAL_GALE6C},		/**< L6C */
	{"6X",	E6,		RM_RANGE_SIGNAL_GALE6X},		/**< L6X */
	{"6Z",	E6,		NA8},							/**< L6Z */
};

static const OBS_CODE_FREQ_INFO_T s_QZSCodeFreqInfoMap[] =
{
	{"1C",	L1,		RM_RANGE_SIGNAL_QZSSL1CA},		/**< L1C */
	{"1E",	L1,		NA8},							/**< L1E */
	{"1S",	L1,		RM_RANGE_SIGNAL_QZSSL1CD},		/**< L1S */
	{"1L",	L1,		RM_RANGE_SIGNAL_QZSSL1CP},		/**< L1L */
	{"1X",	L1,		RM_RANGE_SIGNAL_QZSSL1CX},		/**< L1X */
	{"1Z",	L1,		NA8},							/**< L1Z */
	{"1B",	L1,		NA8},							/**< L1B */
	{"2S",	L2,		RM_RANGE_SIGNAL_QZSSL2CM},		/**< L2S */
	{"2L",	L2,		RM_RANGE_SIGNAL_QZSSL2CL},		/**< L2L */
	{"2X",	L2,		RM_RANGE_SIGNAL_QZSSL2CX},		/**< L2X */
	{"5I",	L5,		RM_RANGE_SIGNAL_QZSSL5I},		/**< L5I */
	{"5Q",	L5,		RM_RANGE_SIGNAL_QZSSL5Q},		/**< L5Q */
	{"5X",	L5,		RM_RANGE_SIGNAL_QZSSL5X},		/**< L5X */
	{"5D",	L5,		NA8},							/**< L5D */
	{"5P",	L5,		NA8},							/**< L5P */
	{"5Z",	L5,		NA8},							/**< L5Z */
	{"6S",	L6,		RM_RANGE_SIGNAL_QZSSL6D},		/**< L6S */
	{"6L",	L6,		RM_RANGE_SIGNAL_QZSSL6P},		/**< L6L */
	{"6X",	L6,		RM_RANGE_SIGNAL_QZSSL6X},		/**< L6X */
	{"6E",	L6,		RM_RANGE_SIGNAL_QZSSL6E},		/**< L6E */
	{"6Z",	L6,		RM_RANGE_SIGNAL_QZSSL6Z},		/**< L6Z */
};

static const OBS_CODE_FREQ_INFO_T s_SBACodeFreqInfoMap[] =
{
	{"1C",	L1,		RM_RANGE_SIGNAL_SBASL1C},		/**< L1C */
	{"5I",	L5,		RM_RANGE_SIGNAL_SBASL5I},		/**< L5I */
	{"5Q",	L5,		RM_RANGE_SIGNAL_SBASL5Q},		/**< L5Q */
	{"5X",	L5,		RM_RANGE_SIGNAL_SBASL5X},		/**< L5X */
};

static const OBS_CODE_FREQ_INFO_T s_BDSCodeFreqInfoVer302Map[] =
{
	{"1I",	B1,		RM_RANGE_SIGNAL_BDSB1ID1},		/**< L1I */
	{"1Q",	B1,		RM_RANGE_SIGNAL_BDSB1Q},		/**< L1Q */
	{"1X",	B1,		NA8},							/**< L1X */
	{"1D",	B4,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L1D */
	{"1P",	B4,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L1P */
	{"1X",	B4,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L1X */
	{"5D",	B5,		RM_RANGE_SIGNAL_BDSB2AD},		/**< L5D */
	{"5P",	B5,		RM_RANGE_SIGNAL_BDSB2AP},		/**< L5P */
	{"5X",	B5,		RM_RANGE_SIGNAL_BDSB2AX},		/**< L5X */
	{"7I",	B2,		RM_RANGE_SIGNAL_BDSB2ID1},		/**< L7I */
	{"7Q",	B2,		RM_RANGE_SIGNAL_BDSB2Q},		/**< L7Q */
	{"7X",	B2,		NA8},							/**< L7X */
	{"7D",	B2,		RM_RANGE_SIGNAL_BDSB2BD},		/**< L7D */
	{"7P",	B2,		RM_RANGE_SIGNAL_BDSB2BP},		/**< L7P */
	{"7Z",	B2,		NA8},							/**< L7Z */
	{"8D",	B8,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L8D */
	{"8P",	B8,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L8P */
	{"8X",	B8,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L8X */
	{"6I",	B3,		RM_RANGE_SIGNAL_BDSB3ID1},		/**< L6I */
	{"6Q",	B3,		RM_RANGE_SIGNAL_BDSB3Q},		/**< L6Q */
	{"6X",	B3,		NA8},							/**< L6X */
	{"6A",	B3,		NA8},							/**< L6A */
};

static const OBS_CODE_FREQ_INFO_T s_BDSCodeFreqInfoMap[] =
{
	{"2I",	B1,		RM_RANGE_SIGNAL_BDSB1ID1},		/**< L2I */
	{"2Q",	B1,		RM_RANGE_SIGNAL_BDSB1Q},		/**< L2Q */
	{"2X",	B1,		NA8},							/**< L2X */
	{"1D",	B4,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L1D */
	{"1P",	B4,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L1P */
	{"1X",	B4,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L1X */
	{"5D",	B5,		RM_RANGE_SIGNAL_BDSB2AD},		/**< L5D */
	{"5P",	B5,		RM_RANGE_SIGNAL_BDSB2AP},		/**< L5P */
	{"5X",	B5,		RM_RANGE_SIGNAL_BDSB2AX},		/**< L5X */
	{"7I",	B2,		RM_RANGE_SIGNAL_BDSB2ID1},		/**< L7I */
	{"7Q",	B2,		RM_RANGE_SIGNAL_BDSB2Q},		/**< L7Q */
	{"7X",	B2,		NA8},							/**< L7X */
	{"7D",	B2,		RM_RANGE_SIGNAL_BDSB2BD},		/**< L7D */
	{"7P",	B2,		RM_RANGE_SIGNAL_BDSB2BP},		/**< L7P */
	{"7Z",	B2,		NA8},							/**< L7Z */
	{"8D",	B8,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L8D */
	{"8P",	B8,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L8P */
	{"8X",	B8,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L8X */
	{"6I",	B3,		RM_RANGE_SIGNAL_BDSB3ID1},		/**< L6I */
	{"6Q",	B3,		RM_RANGE_SIGNAL_BDSB3Q},		/**< L6Q */
	{"6X",	B3,		NA8},							/**< L6X */
	{"6A",	B3,		NA8},							/**< L6A */
};

static const OBS_CODE_FREQ_INFO_T s_BDSCodeFreqInfoVer4Map[] =
{
	{"2I",	B1,		RM_RANGE_SIGNAL_BDSB1ID1},		/**< L2I */
	{"2Q",	B1,		RM_RANGE_SIGNAL_BDSB1Q},		/**< L2Q */
	{"2X",	B1,		NA8},							/**< L2X */
	{"1D",	B4,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L1D */
	{"1P",	B4,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L1P */
	{"1X",	B4,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L1X */
	{"1S",	B4,		NA8},							/**< L1S */
	{"1L",	B4,		NA8},							/**< L1L */
	{"1Z",	B4,		NA8},							/**< L1Z */
	{"5D",	B5,		RM_RANGE_SIGNAL_BDSB2AD},		/**< L5D */
	{"5P",	B5,		RM_RANGE_SIGNAL_BDSB2AP},		/**< L5P */
	{"5X",	B5,		RM_RANGE_SIGNAL_BDSB2AX},		/**< L5X */
	{"7I",	B2,		RM_RANGE_SIGNAL_BDSB2ID1},		/**< L7I */
	{"7Q",	B2,		RM_RANGE_SIGNAL_BDSB2Q},		/**< L7Q */
	{"7X",	B2,		NA8},							/**< L7X */
	{"7D",	B2,		RM_RANGE_SIGNAL_BDSB2BD},		/**< L7D */
	{"7P",	B2,		RM_RANGE_SIGNAL_BDSB2BP},		/**< L7P */
	{"7Z",	B2,		NA8},							/**< L7Z */
	{"8D",	B8,		RM_RANGE_SIGNAL_BDSB1CD},		/**< L8D */
	{"8P",	B8,		RM_RANGE_SIGNAL_BDSB1CP},		/**< L8P */
	{"8X",	B8,		RM_RANGE_SIGNAL_BDSB1CX},		/**< L8X */
	{"6I",	B3,		RM_RANGE_SIGNAL_BDSB3ID1},		/**< L6I */
	{"6Q",	B3,		RM_RANGE_SIGNAL_BDSB3Q},		/**< L6Q */
	{"6X",	B3,		NA8},							/**< L6X */
	{"6D",	B3,		NA8},							/**< L6D */
	{"6P",	B3,		NA8},							/**< L6P */
	{"6Z",	B3,		NA8},							/**< L6Z */
};

static const OBS_CODE_FREQ_INFO_T s_NICCodeFreqInfoMap[] =
{
	{"5A",	L5,		RM_RANGE_SIGNAL_NICL5A},		/**< L5A */
	{"5B",	L5,		NA8},							/**< L5B */
	{"5C",	L5,		NA8},							/**< L5C */
	{"5X",	L5,		NA8},							/**< L5X */
	{"9A",	S9,		NA8},							/**< L9A */
	{"9B",	S9,		NA8},							/**< L9B */
	{"9C",	S9,		NA8},							/**< L9C */
	{"9X",	S9,		NA8},							/**< L9X */
};

typedef struct _OBS_CODE_INFO_T
{
	unsigned int ExSystemID;							/**< satellite SystemID */
	const OBS_CODE_FREQ_INFO_T* pObsCodeFreqInfoMap;	/**< obs code freq informatiuon map */
	unsigned int MapSize;								/**< map size */
} OBS_CODE_INFO_T;

static const OBS_CODE_INFO_T s_ObsCodeInfoMap[] =
{
	{GPS,	s_GPSCodeFreqInfoMap, (sizeof(s_GPSCodeFreqInfoMap) / sizeof(s_GPSCodeFreqInfoMap[0]))},
	{GLO,	s_GLOCodeFreqInfoMap, (sizeof(s_GLOCodeFreqInfoMap) / sizeof(s_GLOCodeFreqInfoMap[0]))},
	{GAL,	s_GALCodeFreqInfoMap, (sizeof(s_GALCodeFreqInfoMap) / sizeof(s_GALCodeFreqInfoMap[0]))},
	{BDS,	s_BDSCodeFreqInfoMap, (sizeof(s_BDSCodeFreqInfoMap) / sizeof(s_BDSCodeFreqInfoMap[0]))},
	{SBA,	s_SBACodeFreqInfoMap, (sizeof(s_SBACodeFreqInfoMap) / sizeof(s_SBACodeFreqInfoMap[0]))},
	{QZS,	s_QZSCodeFreqInfoMap, (sizeof(s_QZSCodeFreqInfoMap) / sizeof(s_QZSCodeFreqInfoMap[0]))},
	{NIC,	s_NICCodeFreqInfoMap, (sizeof(s_NICCodeFreqInfoMap) / sizeof(s_NICCodeFreqInfoMap[0]))},
};

static const int s_NavSystem[] = {/**< satellite SystemIDtems */
	RINEX_SYS_GPS, RINEX_SYS_GLO, RINEX_SYS_GAL, RINEX_SYS_BDS, RINEX_SYS_SBA, RINEX_SYS_QZS, RINEX_SYS_NIC,0
};
static const char s_SysCodes[] = "GRECSJI"; /**< satellite SystemIDtem codes */

static const char s_ObsCodes[] = "CLDS"; /**< obs type codes */

static const char s_FreqCodes[] = "125678";  /**< frequency codes */

static const double s_UraEph[] = { /**< ura values (ref [3] 20.3.3.3.1.1) */
	2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
	3072.0,6144.0,0.0
};

static char s_CodePris[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_FREQ_INDEX_NUM+1][16] = {  /* code priority table */
	/** L1			L2				L5									*/
	{"CSLXPWYMN",	"CDSLXPWYMN",	"IQX",		"",			"",			""},	/**< GPS */

	/** G1			G2				G3			G4(G1a)		G6(G2a)		*/
	{"CP",			"CP",			"IQX",		"ABX",		"ABX",		""},	/**< GLO */

	/** E1			E5A				E5B			E5,			E6			*/
	{"ABCXZ",		"IQX",			"IQX",		"IQX",		"ABCXZ",	""},	/**< GAL */

	/** B1			B3				B2			B4			B5			*/
	{"IQX",			"IQXDPZ",		"IQXDPZ",	"DPXSLZ",	"DPX",		"DPX"},	/**< BDS */

	/** L1			L2				L5									*/
	{"C",			"",				"IQX",		"",			"",			""},	/**< SBA */

	/** L1			L2				L5						L6			*/
	{"CESLXZB",		"SLX",			"IQXDPZ",	"",			"SLXEZ",	""},	/**< QZS */

	/** I5			S9													*/
	{"ABCX",		"ABCX",			"",			"",			"",			""}		/**< NAVIC */
};

typedef struct _SIGNAL_INDEX_INFO_T /**< signal index information */
{
	int Num;									/**< number of index */
	int Freq[MAX_NUM_OBS_TYPE];					/**< signal frequency (1:L1,2:L2,...) */
	int SignalType[MAX_NUM_OBS_TYPE];			/**< signal type */
	int Pos[MAX_NUM_OBS_TYPE];					/**< signal index in obs data (-1:no) */
	unsigned char Pri[MAX_NUM_OBS_TYPE];		/**< signal priority (15-0) */
	unsigned char DataType[MAX_NUM_OBS_TYPE];	/**< data type (0:C,1:L,2:D,3:S) */
	double Shift[MAX_NUM_OBS_TYPE];				/**< phase shift (cycle) */
} SIGNAL_INDEX_INFO_T;

/**********************************************************************//**
@brief  set string without tail space

@param pDst				[In] Destination Data
@param pSrc				[In] source data
@param Num				[In] string number

@retval <0 error, =0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static int SetStr(char* pDst, const char *pSrc, int Num)
{
	char *pTempDst = pDst;
	const char *pTempSrc = pSrc;

	strncpy(pDst, pSrc, Num);

	pTempDst += Num;

	*pTempDst-- = '\0';

	while (pTempDst >= pDst && *pTempDst == ' ')
	{
		*pTempDst-- = '\0';
	}

	return 0;
}

/**********************************************************************//**
@brief  convert substring in string to number

@param pStr             [In] pointer to string buffer
@param Pos              [In] substring position
@param Width            [In] substring width

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/08/08
@note
**************************************************************************/
static double Str2Num(const char *pStr, unsigned int Pos, int Width)
{
	double Value;
	char TempStr[256];
	char* pData = TempStr;

	if (((unsigned int)strlen(pStr) < Pos) || ((int)sizeof(TempStr) - 1 < Width))
	{
		return 0.0;
	}

	for (pStr += Pos; *pStr&&--Width >= 0; pStr++)
	{
		*pData++ = ((*pStr == 'd') || (*pStr == 'D')) ? 'E' : *pStr;
	}

	*pData = '\0';

	Value = strtod(TempStr, NULL);

	return Value;
}

/**********************************************************************//**
@brief  get obs code freq

@param Ver					[In] rinex file version
@param ExSystemID			[In] system id
@param pObsType				[In] obs type
@param pFreq				[In] freq
@param pSignalType			[In] signal type

@retval <0 error, =0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static int GetObsCodeFreq(int Ver, int ExSystemID, char* pObsType, unsigned int* pFreq, unsigned int* pSignalType)
{
	unsigned int SysIndex;
	unsigned int DataIndex;
	const OBS_CODE_FREQ_INFO_T* pObsCodeFreqInfoMap;

	if (!pObsType)
	{
		return -1;
	}

	if ((ExSystemID == BDS) && (Ver > 304))
	{
		for (DataIndex = 0; DataIndex< (sizeof(s_BDSCodeFreqInfoVer4Map)/sizeof(s_BDSCodeFreqInfoVer4Map[0])); DataIndex++)
		{
			if (0 != MEMCMP(s_BDSCodeFreqInfoVer4Map[DataIndex].CodeStr, pObsType, 2))
			{
				continue;
			}

			if (pFreq)
			{
				*pFreq = s_BDSCodeFreqInfoVer4Map[DataIndex].Freq;
			}

			if (pSignalType)
			{
				*pSignalType = s_BDSCodeFreqInfoVer4Map[DataIndex].SignalType;
			}

			return 0;
		}
	}
	else if ((ExSystemID == BDS) && (Ver == 302))
	{
		for (DataIndex = 0; DataIndex< (sizeof(s_BDSCodeFreqInfoVer302Map)/sizeof(s_BDSCodeFreqInfoVer302Map[0])); DataIndex++)
		{
			if (0 != MEMCMP(s_BDSCodeFreqInfoVer302Map[DataIndex].CodeStr, pObsType, 2))
			{
				continue;
			}

			if (pFreq)
			{
				*pFreq = s_BDSCodeFreqInfoVer302Map[DataIndex].Freq;
			}

			if (pSignalType)
			{
				*pSignalType = s_BDSCodeFreqInfoVer302Map[DataIndex].SignalType;
			}

			return 0;
		}
	}
	else
	{
		for (SysIndex = 0; SysIndex < (sizeof(s_ObsCodeInfoMap)/sizeof(s_ObsCodeInfoMap[0])); SysIndex++)
		{
			if (s_ObsCodeInfoMap[SysIndex].ExSystemID == ExSystemID)
			{
				pObsCodeFreqInfoMap = s_ObsCodeInfoMap[SysIndex].pObsCodeFreqInfoMap;
				for (DataIndex = 0; DataIndex< s_ObsCodeInfoMap[SysIndex].MapSize; DataIndex++)
				{
					if (0 != MEMCMP(pObsCodeFreqInfoMap[DataIndex].CodeStr, pObsType, 2))
					{
						continue;
					}

					if (pFreq)
					{
						*pFreq = pObsCodeFreqInfoMap[DataIndex].Freq;
					}

					if (pSignalType)
					{

						*pSignalType = pObsCodeFreqInfoMap[DataIndex].SignalType;
					}

					return 0;
				}
			}
		}
	}

	return -1;
}

/**********************************************************************//**
@brief  get obs code type

@param Ver					[In] rinex file version
@param ExSystemID			[In] system id
@param SignalType			[In] signal type

@retval <0 error, =0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static char* GetObsCodeType(int Ver, int ExSystemID, unsigned int SignalType)
{
	unsigned int SysIndex;
	unsigned int DataIndex;
	const OBS_CODE_FREQ_INFO_T* pObsCodeFreqInfoMap;

	if ((ExSystemID == BDS) && (Ver > 304))
	{
		for (DataIndex = 0; DataIndex< (sizeof(s_BDSCodeFreqInfoVer4Map)/sizeof(s_BDSCodeFreqInfoVer4Map[0])); DataIndex++)
		{
			if (SignalType == s_BDSCodeFreqInfoVer4Map[DataIndex].SignalType)
			{
				return s_BDSCodeFreqInfoVer4Map[DataIndex].CodeStr;
			}
		}
	}
	else
	{
		for (SysIndex = 0; SysIndex < (sizeof(s_ObsCodeInfoMap)/sizeof(s_ObsCodeInfoMap[0])); SysIndex++)
		{
			if (s_ObsCodeInfoMap[SysIndex].ExSystemID == ExSystemID)
			{
				pObsCodeFreqInfoMap = s_ObsCodeInfoMap[SysIndex].pObsCodeFreqInfoMap;
				for (DataIndex = 0; DataIndex< s_ObsCodeInfoMap[SysIndex].MapSize; DataIndex++)
				{
					if (SignalType == pObsCodeFreqInfoMap[DataIndex].SignalType)
					{
						return pObsCodeFreqInfoMap[DataIndex].CodeStr;
					}
				}
			}
		}
	}

	return NULL;
}

/**********************************************************************//**
@brief  satellite obs data to gnss obs data

@param pSatObsData    [In] satellite obs data
@param pObsData       [In] obs data

@retval <0 error, =0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static int SatObsDataToObsData(OBS_DATA_SAT_T* pSatObsData, GNSS_OBS_T* pObsData)
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
		for (PserSatObsIndex = 0; PserSatObsIndex < (MAX_FREQ_INDEX_NUM+1); PserSatObsIndex++)
		{
			if (pPerSatObsData->ObsData[PserSatObsIndex].PRN)
			{
				pObsData->RangeData.ObsData[ObsIndex] = pPerSatObsData->ObsData[PserSatObsIndex];
				pObsData->RangeData.ObsNum++;
				ObsIndex++;
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  RTCM3 decode obs callback

@param pObsData             [In] obs data
@param pSatObsData          [In] Satellite obs data

@retval <0 error, =0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static int ObsDataConvert(GNSS_OBS_T* pObsData, OBS_DATA_SAT_T* pSatObsData)
{
	UTC_TIME_T UTCTime = pObsData->ObsTime;

	MEMSET(pObsData, 0x00, sizeof(GNSS_OBS_T));
	SatObsDataToObsData(pSatObsData, pObsData);
	pObsData->ObsTime = UTCTime;
	pObsData->RawDataType = RAW_OBS_DATA_TYPE_RINEX;

	UTCTimeToRangeTime(pObsData, &UTCTime);

	return 0;
}

/**********************************************************************//**
@brief  ura value (m) to ura index

@param value			[In] ura value
@retval ura index

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
extern int GetUraIndex(double value)
{
	unsigned int DataIndex;
	for (DataIndex = 0; DataIndex < 15;DataIndex++)
	{
		if (s_UraEph[DataIndex] >= value)
		{
			break;
		}
	}
	return DataIndex;
}

/**********************************************************************//**
@brief translate rinex system ID mask to internal used system ID

@param RinexSystemIDMask [In] internal used system ID

@return internal used system ID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static unsigned int RinexSystemMaskToExSystem(int RinexSystemIDMask)
{
	unsigned int SystemID = NA8;
	unsigned int SysIndex;

	const static unsigned int s_ExSystemMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		GPS,
		GLO,
		GAL,
		BDS,
		SBA,
		QZS,
		NIC,
	};

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		if (RinexSystemIDMask & (1 << SysIndex))
		{
			SystemID = SysIndex;
			break;
		}
	}

	if (SystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_ExSystemMap[SystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate rinex system ID mask to raw message system

@param RinexSystemIDMask [In] rinex system ID mask

@return RMSystemID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static unsigned int RinexSystemMaskToRMSystem(int RinexSystemIDMask)
{
	unsigned int SystemID = NA8;
	unsigned int SysIndex;

	const static unsigned int s_RMSystemIDMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RM_RANGE_SYS_GPS,
		RM_RANGE_SYS_GLO,
		RM_RANGE_SYS_GAL,
		RM_RANGE_SYS_BDS,
		RM_RANGE_SYS_SBA,
		RM_RANGE_SYS_QZS,
		RM_RANGE_SYS_NIC,
	};

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		if (RinexSystemIDMask & (1 << SysIndex))
		{
			SystemID = SysIndex;
			break;
		}
	}

	if (SystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RMSystemIDMap[SystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate rinex system ID mask to Rinex File Index

@param RinexSystemIDMask [In] rinex system ID mask

@return Rinex File Index

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static unsigned int RinexSystemMaskToRinexFileIndex(int RinexSystemIDMask)
{
	unsigned int SystemID = NA8;
	unsigned int SysIndex;

	const static unsigned int s_RMSystemIDMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RINEX_FILE_INDEX_GPSEPH,
		RINEX_FILE_INDEX_GLOEPH,
		RINEX_FILE_INDEX_GALEPH,
		RINEX_FILE_INDEX_BDSEPH,
		RINEX_FILE_INDEX_SBAEPH,
		RINEX_FILE_INDEX_QZSEPH,
		RINEX_FILE_INDEX_NICEPH,
	};

	if (RinexSystemIDMask == RINEX_SYS_ALL)
	{
		return RINEX_FILE_INDEX_MUTIEPH;
	}

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		if (RinexSystemIDMask & (1 << SysIndex))
		{
			SystemID = SysIndex;
			break;
		}
	}

	if (SystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RMSystemIDMap[SystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate Rinex Header ID to rinex system ID mask

@param RinexHeaderID [In] Rinex Header ID

@return rinex system ID mask

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static int RinexHeaderIDToRinexSystemMask(int RinexHeaderID)
{
	const static unsigned int s_RinexSystemIDMaskMap[] =
	{
		RINEX_SYS_ALL,
		RINEX_SYS_GPS,
		RINEX_SYS_GLO,
		RINEX_SYS_GAL,
		RINEX_SYS_BDS,
		RINEX_SYS_QZS,
		RINEX_SYS_SBA,
		RINEX_SYS_NIC,
	};

	if ((RinexHeaderID >= RINEX_ID_MUTIEPH_HEADER) && (RinexHeaderID <= RINEX_ID_NICEPH_HEADER))
	{
		return s_RinexSystemIDMaskMap[RinexHeaderID - RINEX_ID_MUTIEPH_HEADER]; /**< mixed */
	}

	return -1;
}

/**********************************************************************//**
@brief translate rinex system ID mask to Rinex Header ID

@param RinexSystemIDMask [In] rinex system ID mask

@return RinexHeaderID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static unsigned int RinexSystemMaskToRinexHeaderID(int RinexSystemIDMask)
{
	unsigned int SystemID = NA8;
	unsigned int SysIndex;

	const static unsigned int s_RMSystemIDMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RINEX_ID_GPSEPH_HEADER,
		RINEX_ID_GLOEPH_HEADER,
		RINEX_ID_GALEPH_HEADER,
		RINEX_ID_BDSEPH_HEADER,
		RINEX_ID_SBASEPH_HEADER,
		RINEX_ID_QZSSEPH_HEADER,
		RINEX_ID_NICEPH_HEADER,
	};

	if (RinexSystemIDMask == RINEX_SYS_ALL)
	{
		return RINEX_ID_MUTIEPH_HEADER; /**< mixed */
	}

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		if (RinexSystemIDMask & (1 << SysIndex))
		{
			SystemID = SysIndex;
			break;
		}
	}

	if (SystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RMSystemIDMap[SystemID];
	}

	return NA8;
}

/**********************************************************************//**
@brief translate rinex system ID mask to Rinex Header ID

@param RinexSystemIDMask [In] rinex system ID mask

@return RinexFileID

@author CHC
@date 2022/05/16
@note
**************************************************************************/
static int RinexSystemMaskToRinexFileID(int RinexSystemIDMask)
{
	unsigned int SystemID = NA8;
	unsigned int SysIndex;

	const static int s_RMSystemIDMap[TOTAL_EX_GNSS_SYSTEM_NUM] =
	{
		RINEX_ID_GPSEPH,
		RINEX_ID_GLOEPH,
		RINEX_ID_GALEPH,
		RINEX_ID_BDSEPH,
		RINEX_ID_SBASEPH,
		RINEX_ID_QZSSEPH,
		RINEX_ID_NICEPH,
	};

	if (RinexSystemIDMask == RINEX_SYS_ALL)
	{
		return RINEX_ID_MUTIEPH; /**< mixed */
	}

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		if (RinexSystemIDMask & (1 << SysIndex))
		{
			SystemID = SysIndex;
			break;
		}
	}

	if (SystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_RMSystemIDMap[SystemID];
	}

	return -1;
}

/**********************************************************************//**
@brief  get rinex file line data

@param pFile				[In] rinex file
@param pCycleBufCtrl		[In] rinex cycle buffer ctrl
@param pOutputBuf			[Out] output data buffer
@param ReadSize				[In] read size

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int GetRinexFileLineData(FILE *pFile, CYCLE_BUFF_CTRL_T* pCycleBufCtrl, char* pOutputBuf, int ReadSize)
{
	int DataSize = CycleBufGetDataSize(pCycleBufCtrl);
	int ReadDataSize;
	char ReadDataBuf[RINEX_DATA_BUF_SIZE];
	char* pStr = NULL;

	if (DataSize < ReadSize)
	{
		ReadDataSize = (int)fread(ReadDataBuf, 1, RINEX_DATA_BUF_SIZE, pFile);
		if (ReadDataSize > 0)
		{
			CycleBufPutData(pCycleBufCtrl, ReadDataBuf, ReadDataSize);
		}
	}

	DataSize = CycleBufGetDataSize(pCycleBufCtrl);
	if (DataSize > 0)
	{
		ReadDataSize = (DataSize <= ReadSize) ? DataSize : ReadSize;

		CycleBufGetData(pCycleBufCtrl, (char *)ReadDataBuf, ReadDataSize);
		ReadDataBuf[ReadDataSize] = '\0';

		pStr = strstr(ReadDataBuf, "\n");
		if (pStr != NULL)
		{
			ReadDataSize = (int)(pStr - ReadDataBuf + 1);
			if ((ReadDataSize > 0) && (ReadDataSize <= ReadSize))
			{
				MEMCPY(pOutputBuf, ReadDataBuf, ReadDataSize);
				pOutputBuf[ReadDataSize - 1] = '\0';

				CycleBufUpdateReadAddr(pCycleBufCtrl, ReadDataSize);
				return ReadDataSize;
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  obs string to time

@param pInputStr			[In] input string buffer
@param Pos					[In] substring position
@param Width				[In] substring width
@param pUTCTime				[Out] UTC time

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ObsStr2Time(const char* pInputStr, unsigned int Pos, int Width, UTC_TIME_T* pUTCTime, int SysTimeType)
{
	int TimeBuf[7];
	char StrBuf[128];
	char* pStr = StrBuf;
	UTC_TIME_T TempUTCTime = {0};

	if ((!pUTCTime) || ((unsigned int)strlen(pInputStr) < Pos) || (((int)sizeof(StrBuf) - 1) < Pos))
	{
		return -1;
	}

	for (pInputStr += Pos; *pInputStr && --Width >= 0; )
	{
		*pStr++=*pInputStr++;
	}
	*pStr='\0';

	if (sscanf(StrBuf, "%d %d %d %d %d %d.%d", &TimeBuf[0], &TimeBuf[1], &TimeBuf[2], &TimeBuf[3], &TimeBuf[4], &TimeBuf[5], &TimeBuf[6]) < 7)
	{
		return -1;
	}

	if (TimeBuf[0] < 100)
	{
		TimeBuf[0] += TimeBuf[0] < 80 ? 2000 : 1900;
	}

	TempUTCTime.Year = TimeBuf[0];
	TempUTCTime.Month = TimeBuf[1];
	TempUTCTime.Day = TimeBuf[2];
	TempUTCTime.Hour = TimeBuf[3];
	TempUTCTime.Minute = TimeBuf[4];
	TempUTCTime.Second = TimeBuf[5];
	TempUTCTime.Millisecond = (int)(TimeBuf[6]/10000 + 0.5);

	if ((SysTimeType == SYS_TIME_TYPE_GPS) || (SysTimeType == SYS_TIME_TYPE_QZS) ||
		(SysTimeType == SYS_TIME_TYPE_GAL) || (SysTimeType == SYS_TIME_TYPE_NIC))
	{
		TempUTCTime = UTCAdd(&TempUTCTime, -18000);
	}
	else if (SysTimeType == SYS_TIME_TYPE_BDS)
	{
		TempUTCTime = UTCAdd(&TempUTCTime, -4000);
	}

	MEMCPY(pUTCTime, &TempUTCTime, sizeof(UTC_TIME_T));

	return 0;
}

/**********************************************************************//**
@brief  eph string to time

@param pInputStr			[In] input string buffer
@param Pos					[In] substring position
@param Width				[In] substring width
@param pUTCTime				[Out] UTC time

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int EphStr2Time(const char* pInputStr, unsigned int Pos, int Width, UTC_TIME_T* pUTCTime)
{
	int TimeBuf[6];
	char StrBuf[128];
	char* pStr = StrBuf;

	if ((!pUTCTime) || ((unsigned int)strlen(pInputStr) < Pos) || (((int)sizeof(StrBuf) - 1) < Pos))
	{
		return -1;
	}

	for (pInputStr += Pos; *pInputStr && --Width >= 0; )
	{
		*pStr++=*pInputStr++;
	}
	*pStr='\0';

	if (sscanf(StrBuf, "%d %d %d %d %d %d", &TimeBuf[0], &TimeBuf[1], &TimeBuf[2], &TimeBuf[3], &TimeBuf[4], &TimeBuf[5]) < 6)
	{
		return -1;
	}

	if (TimeBuf[0] < 100)
	{
		TimeBuf[0] += TimeBuf[0] < 80 ? 2000 : 1900;
	}

	pUTCTime->Year = TimeBuf[0];
	pUTCTime->Month = TimeBuf[1];
	pUTCTime->Day = TimeBuf[2];
	pUTCTime->Hour = TimeBuf[3];
	pUTCTime->Minute = TimeBuf[4];
	pUTCTime->Second = TimeBuf[5];
	pUTCTime->Millisecond = 0;

	return 0;
}

/**********************************************************************//**
@brief  satellite id to satellite number

@param pIDBuf			[In] satid string buffer
@param pSystemID		[Out] system id
@param pPRN				[Out] prn

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int SatID2SatNo(const char* pIDBuf, int* pSystemID, int* pPRN)
{
	int SystemID, PRN;
	char Code;

	if (sscanf(pIDBuf, "%d", &PRN) == 1)
	{
		if ((MIN_GPS_PRN <= PRN) && (PRN <= MAX_GPS_PRN))
		{
			SystemID = RINEX_SYS_GPS;
		}
		else if ((MIN_SBAS_GEO_PRN <= PRN) && (PRN <= MAX_SBAS_GEO_PRN))
		{
			SystemID = RINEX_SYS_SBA;
		}
		else if ((MIN_QZSS_PRN <= PRN) && (PRN <= MAX_QZSS_PRN))
		{
			SystemID = RINEX_SYS_QZS;
		}
		else
		{
			return -1;
		}

		*pSystemID = SystemID;
		*pPRN = PRN;

		return 0;
	}

	if (sscanf(pIDBuf, "%c%d", &Code, &PRN) < 2)
	{
		return -1;
	}

	switch (Code)
	{
		case 'G':
			SystemID = RINEX_SYS_GPS;
			PRN += MIN_GPS_RANGE_PRN - 1;
			break;
		case 'R':
			SystemID = RINEX_SYS_GLO;
			PRN += MIN_GLONASS_RANGE_PRN - 1;
			break;
		case 'E':
			SystemID = RINEX_SYS_GAL;
			PRN += MIN_GALILEO_RANGE_PRN - 1;
			break;
		case 'J':
			SystemID = RINEX_SYS_QZS;
			PRN += MIN_QZSS_RANGE_PRN - 1;
			break;
		case 'C':
			SystemID = RINEX_SYS_BDS;
			PRN += MIN_BDS_RANGE_PRN - 1;
			break;
		case 'I':
			SystemID = RINEX_SYS_NIC;
			PRN += MIN_NAVIC_RANGE_PRN - 1;
			break;
		case 'S':
			SystemID = RINEX_SYS_SBA;
			PRN += 100;
			break;
		default:
			return 0;
	}

	*pSystemID = SystemID;
	*pPRN = PRN;

	return 0;
}

/**********************************************************************//**
@brief  conVert rinex obs type

@param Ver				[In] rinex file version
@param ExSystemID		[In] system id
@param pStr				[In] obs code string
@param pType			[In] obs code type

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ConvertCode(double Ver, int ExSystemID, const char* pStr, char* pType)
{
	strcpy(pType, "   ");

	if (!strcmp(pStr, "P1")) /**< Ver.2.11 GPS L1PY,GLO L2P */
	{
		if (ExSystemID == GPS)
		{
			sprintf(pType,"%c1W",'C');
		}
		else if (ExSystemID == GLO)
		{
			sprintf(pType, "%c1P", 'C');
		}
	}
	else if (!strcmp(pStr, "P2")) /**< Ver.2.11 GPS L2PY,GLO L2P */
	{
		if (ExSystemID == GPS)
		{
			sprintf(pType,"%c2W",'C');
		}
		else if (ExSystemID == GLO)
		{
			sprintf(pType, "%c2P", 'C');
		}
	}
	else if (!strcmp(pStr, "C1")) /**< Ver.2.11 GPS L1C,GLO L1C/A */
	{
		if (Ver>=212) /* reject C1 for 2.12 */
		{
			/** do nothing */
		}
		else if ((ExSystemID == GPS) ||(ExSystemID == GLO) ||
			(ExSystemID == QZS) || (ExSystemID == SBA))
		{
			sprintf(pType, "%c1C", 'C');
		}
		else if (ExSystemID == GAL)
		{
			sprintf(pType, "%c1X", 'C');
		}
	}
	else if (!strcmp(pStr, "C2")) /**< Ver.2.11 GPS L2C,GLO L2C/A */
	{
		if (ExSystemID==GPS)
		{
			if (Ver<212)
			{
				sprintf(pType,"%c2W",'C'); /* L2P(Y) */
			}
			else
			{
				sprintf(pType,"%c2X",'C'); /* L2C */
			}
		}
		else if (ExSystemID == GLO)
		{
			sprintf(pType,"%c2C",'C');
		}
		else if (ExSystemID == QZS)
		{
			sprintf(pType,"%c2X",'C');
		}
		else if (ExSystemID == BDS)
		{
			sprintf(pType,"%c2X",'C'); /* ver.2.12 B1_2 */
		}
	}
	else if (Ver >= 212 && pStr[1] == 'A') /**< Ver.2.12 L1C/A */
	{
		if ((ExSystemID == GPS) || (ExSystemID == GLO) ||
			(ExSystemID == QZS) || (ExSystemID == SBA))
		{
			sprintf(pType, "%c1C", pStr[0]);
		}
	}
	else if (Ver >= 212 && pStr[1] == 'B') /**< Ver.2.12 GPS L1C */
	{
		if ((ExSystemID == GPS) || (ExSystemID == QZS))
		{
			sprintf(pType, "%c1X", pStr[0]);
		}
	}
	else if (Ver >= 212 && pStr[1] == 'C') /**< Ver.2.12 GPS L2C */
	{
		if ((ExSystemID == GPS) || (ExSystemID == QZS))
		{
			sprintf(pType, "%c2X", pStr[0]);
		}
	}
	else if (Ver >= 212 && pStr[1] == 'D') /**< Ver.2.12 GLO L2C/A */
	{
		if (ExSystemID == GLO)
		{
			sprintf(pType, "%c2C", pStr[0]);
		}
	}
	else if (Ver >= 212 && pStr[1] == '1') /**< Ver.2.12 GPS L1PY,GLO L1P */
	{
		if (ExSystemID == GPS)
		{
			sprintf(pType, "%c1W", pStr[0]);
		}
		else if (ExSystemID == GLO)
		{
			sprintf(pType, "%c1P", pStr[0]);
		}
		else if (ExSystemID == GAL)
		{
			sprintf(pType, "%c1X", pStr[0]); /**< tentative */
		}
		else if (ExSystemID == BDS)
		{
			sprintf(pType,"%c2X",pStr[0]); /* extension */
		}
	}
	else if (Ver < 212 && pStr[1] == '1')
	{
		if ((ExSystemID == GPS) || (ExSystemID == GLO) ||
			(ExSystemID == QZS) || (ExSystemID == SBA))
		{
			sprintf(pType, "%c1C", pStr[0]);
		}
		else if (ExSystemID == GAL)
		{
			sprintf(pType, "%c1X", pStr[0]); /**< tentative */
		}
	}
	else if (pStr[1] == '2')
	{
		if (ExSystemID == GPS)
		{
			sprintf(pType, "%c2W", pStr[0]);
		}
		else if (ExSystemID == GLO)
		{
			sprintf(pType, "%c2P", pStr[0]);
		}
		else if (ExSystemID == QZS)
		{
			sprintf(pType, "%c2X", pStr[0]);
		}
		else if (ExSystemID == BDS)
		{
			sprintf(pType, "%c2C", pStr[0]); /**< tentative */
		}
	}
	else if (pStr[1] == '5')
	{
		if ((ExSystemID == GPS) || (ExSystemID == GAL) ||
			(ExSystemID == QZS) || (ExSystemID == SBA) ||
			(ExSystemID == BDS)) /**< tentative */
		{
			sprintf(pType, "%c5X", pStr[0]);
		}
	}
	else if (pStr[1] == '6')
	{
		if ((ExSystemID == GAL) || (ExSystemID == QZS) ||
			(ExSystemID == BDS)) /**< tentative */
		{
			sprintf(pType, "%c6X", pStr[0]);
		}
	}
	else if (pStr[1] == '7')
	{
		if ((ExSystemID == GAL) || (ExSystemID == BDS)) /**< tentative */
		{
			sprintf(pType, "%c7X", pStr[0]);
		}
	}
	else if (pStr[1] == '8')
	{
		if (ExSystemID == GAL)
		{
			sprintf(pType, "%c8X", pStr[0]);
		}
	}
	return 0;
}

/**********************************************************************//**
@brief  get rinex code priority

@param ExSystemID		[In] system id
@param Freq				[In] satellite freq
@param ObsCodeType		[In] obs code type

@retval <0 error, >=0 rinex code priority

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int GetRinexCodePri(int ExSystemID, int Freq, char ObsCodeType)
{
	char* pStr;

	if ((ExSystemID >= TOTAL_EX_GNSS_SYSTEM_NUM) || (Freq > MAX_FREQ_INDEX_NUM))
	{
		return -1;
	}

	/** search code priority */
	pStr = strchr(s_CodePris[ExSystemID][Freq], ObsCodeType);

	return (pStr) ? (14 - (int)(pStr - s_CodePris[ExSystemID][Freq])) : 0;
}

/**********************************************************************//**
@brief  get system time type

@param pBuf				[In] rinex header data buffer

@retval <0 error, >=0 system time type

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int GetSysTimeType(char* pBuf)
{
	int SysTimeType = -1;

	static char* s_TimeTypeStrMap[SYS_TIME_TYPE_MAX] = {
		"GPS",
		"UTC",
		"GLO",
		"GAL",
		"QZS", /**< Ver.3.02 */
		"BDT", /**< Ver.3.02 */
		"IRN", /**< Ver.3.02 */
	};

	for (int DataIndex = 0; DataIndex < SYS_TIME_TYPE_MAX; DataIndex++)
	{
		if (!strncmp(pBuf, s_TimeTypeStrMap[DataIndex], 3))
		{
			SysTimeType = DataIndex;
			break;
		}
	}

	return SysTimeType;
}

/**********************************************************************//**
@brief  decode obs rinex file header

@param pFile			[In] input rinex file
@param pBuff			[In] input rinex file data buffer
@param Ver				[In] rinex file version
@param pSysTimeType		[In] system time type
@param ObsType			[In] obs type
@param pObsHeaderInfo	[Out] rinex obs file header information

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeObsHeader(FILE *pFile, char* pBuff, double Ver, int* pSysTimeType,
	char ObsType[][MAX_NUM_OBS_TYPE][4], RINEX_OBS_HEADER_INFO_T* pObsHeaderInfo, CYCLE_BUFF_CTRL_T* pCycleBufCtrl)
{
	/** default codes for unknown code */
	const char *DefaultCodes[] = {
		"CWX   ",   /**< GPS: L125___ */
		"CC    ",   /**< GLO: L12____ */
		"X XXXX",   /**< GAL: L1_5678 */
		"CXXX  ",   /**< QZS: L1256__ */
		"C X   ",   /**< SBS: L1_5___ */
		" C XX "    /**< BDS: L_2_67_ */
		"  A   A"   /**< IRN: L__5___9 */
	};
	int DataIndex, StrIndex;
	unsigned int SysIndex, ObsTypeIndex;
	unsigned int ObsTypeCount, ObsTypeNum;
	unsigned int SatIndex, FreqIndex, SatCount;
	int PRN, FreqChnn, SatID, WideLane[2];
	char* pLabel = pBuff + 60;
	char* pStr, StrBuf[4];
	int Block = 0;
	double Bias;
	int SystemID;
	int Result;
	double Second;
	unsigned int SignalID;
	unsigned int ExSystemID;
	UTC_TIME_T TempUTCTime = {0};
	int SysTimeType = -1;

	if (!pObsHeaderInfo)
	{
		return -1;
	}

	pObsHeaderInfo->ObsHeader.Ver = (int)(Ver * 100 + 0.5);
	if (strstr(pLabel, "PGM / RUN BY / DATE"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.Pgm, pBuff, 20);
		SetStr(pObsHeaderInfo->ObsHeader.Runby, pBuff + 20, 20);
		SetStr(pObsHeaderInfo->FileCreateDate, pBuff + 40, 8);
		SetStr(pObsHeaderInfo->FileCreateTime, pBuff + 49, 8);
	}
	else if (strstr(pLabel, "COMMENT"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.Comments[pObsHeaderInfo->ObsHeader.NCom], pBuff, 60);
		pObsHeaderInfo->ObsHeader.NCom++;
		if (pObsHeaderInfo->ObsHeader.NCom > MAXCOMMENT)
		{
			return-1;
		}
		/** read cnes wl satellite fractional bias */
		if (strstr(pBuff, "WIDELANE SATELLITE FRACTIONAL BIASES") ||
			strstr(pBuff, "WIDELANE SATELLITE FRACTIONNAL BIASES"))
		{
			Block = 1;
		}
		else if (Block)
		{
			/** cnes/cls grg clock */
			Result = SatID2SatNo(pBuff + 3, &SystemID, &PRN);
			if (!strncmp(pBuff, "WL", 2) && (Result == 0) &&
				sscanf(pBuff + 40, "%lf", &Bias) == 1)
			{
				pObsHeaderInfo->WideLaneBias[PRN - 1] = Bias;
			}
			/** cnes ppp-wizard clock */
			else if ((0 == SatID2SatNo(pBuff + 1, &SystemID, &PRN)) && sscanf(pBuff + 6, "%lf", &Bias) == 1)
			{
				pObsHeaderInfo->WideLaneBias[PRN - 1] = Bias;
			}
		}
		pObsHeaderInfo->Block = Block;
	}
	else if (strstr(pLabel, "MARKER NAME"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.MarkerName, pBuff, 60);
	}
	else if (strstr(pLabel, "MARKER NUMBER"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.MarkerNo, pBuff, 20);
	}
	else if (strstr(pLabel, "MARKER TYPE")) /**< Ver.3 */
	{
		SetStr(pObsHeaderInfo->ObsHeader.MarkerType, pBuff, 20);
	}
	else if (strstr(pLabel, "OBSERVER / AGENCY"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.Observer, pBuff, 20);
		SetStr(pObsHeaderInfo->ObsHeader.Agency, pBuff + 20, 40);
	}
	else if (strstr(pLabel, "REC # / TYPE / VERS"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.RecNo, pBuff, 20);
		SetStr(pObsHeaderInfo->ObsHeader.RecType, pBuff + 20, 20);
		SetStr(pObsHeaderInfo->ObsHeader.RecVer, pBuff + 40, 20);
	}
	else if (strstr(pLabel, "ANT # / TYPE"))
	{
		SetStr(pObsHeaderInfo->ObsHeader.AntNo, pBuff, 20);
		SetStr(pObsHeaderInfo->ObsHeader.AntDes, pBuff + 20, 20);
	}
	else if (strstr(pLabel, "APPROX POSITION XYZ"))
	{
		for (DataIndex = 0; DataIndex < 3; DataIndex++)
		{
			pObsHeaderInfo->ObsHeader.Pos[DataIndex] = strtod(pBuff, &pBuff);
		}
	}
	else if (strstr(pLabel, "ANTENNA: DELTA H/E/N"))
	{
		pObsHeaderInfo->ObsHeader.Del[2] = strtod(pBuff, &pBuff); /**< h */
		pObsHeaderInfo->ObsHeader.Del[0] = strtod(pBuff, &pBuff); /**< e */
		pObsHeaderInfo->ObsHeader.Del[1] = strtod(pBuff, &pBuff); /**< n */
	}
	else if (strstr(pLabel, "ANTENNA: DELTA X/Y/Z")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "ANTENNA: PHASECENTER")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "ANTENNA: B.SIGHT XYZ")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "ANTENNA: ZERODIR AZI")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "ANTENNA: ZERODIR XYZ")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "CENTER OF MASS: XYZ")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "SYS / # / OBS TYPES")) /**< Ver.3 */
	{
		pStr = strchr(s_SysCodes, pBuff[0]);
		if (!pStr)
		{
			GNSSPrintf("Rinex observation file 'SYS / # / OBS TYPES' info error.\r\n");
			return -1;
		}
		SysIndex = (unsigned int)(pStr - s_SysCodes);
		ObsTypeNum = (unsigned int)strtoul(&pBuff[3], NULL, 10);

		pObsHeaderInfo->ObsHeader.NavSys |= s_NavSystem[SysIndex];

		for (ObsTypeIndex = ObsTypeCount = 0, StrIndex = 7; ObsTypeIndex < ObsTypeNum; ObsTypeIndex++, StrIndex += 4)
		{
			if (StrIndex > 58)
			{
				//if (!fgets(pBuff, MAXRNXLEN, pFile))
				if (!GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN))
				{
					break;
				}
				StrIndex = 7;
			}
			if (ObsTypeCount < MAX_NUM_OBS_TYPE - 1)
			{
				strncpy(ObsType[SysIndex][ObsTypeCount++], pBuff + StrIndex, 3);
				ObsType[SysIndex][ObsTypeCount][3] = '\0';
			}
		}
		*ObsType[SysIndex][ObsTypeCount] = '\0';

		pObsHeaderInfo->ObsHeader.ObsTypeCount[SysIndex] = ObsTypeCount;

		if (pObsHeaderInfo->ObsHeader.Ver < 303 && SysIndex == 4)
		{
			for (ObsTypeIndex = 0; ObsTypeIndex < ObsTypeCount; ObsTypeIndex++)
			{
				if (ObsType[SysIndex][ObsTypeIndex][1] == 1)
				{
					ObsType[SysIndex][ObsTypeIndex][1] = 2;
				}
			}
		}

		/** if unknown code in Ver.3, set default code */
		for (ObsTypeIndex = 0; ObsTypeIndex < ObsTypeCount; ObsTypeIndex++)
		{
			if (ObsType[SysIndex][ObsTypeIndex][2])
			{
				continue;
			}
			pStr = strchr(s_FreqCodes, ObsType[SysIndex][ObsTypeIndex][1]);
			if (!pStr)
			{
				continue;
			}
			ObsType[SysIndex][ObsTypeIndex][2] = DefaultCodes[SysIndex][(int)(pStr - s_FreqCodes)];
		}
	}
	else if (strstr(pLabel, "WAVELENGTH FACT L1/2")) /**< opt Ver.2 */
	{
		if (pObsHeaderInfo->WaveLenFact[0] == 0)
		{
			pObsHeaderInfo->WaveLenFact[0] = (int)strtoul(pBuff, &pBuff, 10);
			pObsHeaderInfo->WaveLenFact[1] = (int)strtoul(pBuff, &pBuff, 10);
		}
		else
		{
			SatIndex = 0;
			WideLane[0] = (int)strtoul(pBuff, &pBuff, 10);
			WideLane[1] = (int)strtoul(pBuff, &pBuff, 10);
			pObsHeaderInfo->NarWideLaneSatNum = (int)strtoul(pBuff, &pBuff, 10);
			for (DataIndex = 0; DataIndex <= pObsHeaderInfo->NarWideLaneSatNum / 7; DataIndex++)
			{
				if (DataIndex > 0)
				{
					//fgets(pBuff, MAXRNXLEN, pFile);
					GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN);
				}

				for (FreqIndex = 0; FreqIndex < 7; FreqIndex++)
				{
					pBuff += 4;
					SatID = (int)strtoul(pBuff, &pBuff, 10);
					if (SatID == 0)
					{
						break;
					}
					pObsHeaderInfo->WideLaneSat[SatIndex][0] = WideLane[0];
					pObsHeaderInfo->WideLaneSat[SatIndex][1] = WideLane[1];
					pObsHeaderInfo->WideLaneSat[SatIndex][MAX_FREQ_INDEX_NUM] = SatID;
					SatIndex++;
				}
				if (SatIndex == pObsHeaderInfo->NarWideLaneSatNum)
				{
					break;
				}
			}
		}
	}
	else if (strstr(pLabel, "# / TYPES OF OBSERV")) /**< Ver.2 */
	{
		ObsTypeNum = (int)strtoul(pBuff, NULL, 10);
		for (ObsTypeIndex = ObsTypeCount = 0, StrIndex = 10; ObsTypeIndex < ObsTypeNum; ObsTypeIndex++, StrIndex += 6)
		{
			if (StrIndex > 58)
			{
				//if (!fgets(pBuff, MAXRNXLEN, pFile))
				if (!GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN))
				{
					break;
				}
				StrIndex = 10;
			}
			if (ObsTypeCount >= MAX_NUM_OBS_TYPE - 1)
			{
				continue;
			}
			if (pObsHeaderInfo->ObsHeader.Ver <= 299)
			{
				strncpy(StrBuf, pBuff + StrIndex, 2);
				StrBuf[2] = '\0';

				for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
				{
					ConvertCode(Ver, SysIndex, StrBuf, ObsType[SysIndex][ObsTypeCount]);
				}
			}
			ObsTypeCount++;
		}
		*ObsType[0][ObsTypeCount] = '\0';
	}
	else if (strstr(pLabel, "SIGNAL STRENGTH UNIT")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "INTERVAL")) /**< opt */
	{
		pObsHeaderInfo->ObsHeader.TimeInt = strtod(pBuff, &pBuff);
	}
	else if (strstr(pLabel, "TIME OF FIRST OBS"))
	{
		TempUTCTime.Year = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Month = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Day = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Hour = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Minute = (int)strtoul(pBuff, &pBuff, 10);
		Second = strtod(pBuff, &pBuff);
		TempUTCTime.Second = (int)Second;
		TempUTCTime.Millisecond = (int)((Second - TempUTCTime.Second) * 1000 + 0.5);

		SysTimeType = GetSysTimeType(pBuff + 5);
		if (SysTimeType < 0)
		{
			return -1;
		}

		if ((SysTimeType == SYS_TIME_TYPE_GPS) || (SysTimeType == SYS_TIME_TYPE_QZS) ||
		(SysTimeType == SYS_TIME_TYPE_GAL) || (SysTimeType == SYS_TIME_TYPE_NIC))
		{
			TempUTCTime = UTCAdd(&TempUTCTime, -18000);
		}
		else if (SysTimeType == SYS_TIME_TYPE_BDS)
		{
			TempUTCTime = UTCAdd(&TempUTCTime, -4000);
		}

		MEMCPY(&pObsHeaderInfo->ObsHeader.TimeStart, &TempUTCTime, sizeof(UTC_TIME_T));

		*pSysTimeType = SysTimeType;
	}
	else if (strstr(pLabel, "TIME OF LAST OBS")) /**< opt */
	{
		TempUTCTime.Year = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Month = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Day = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Hour = (int)strtoul(pBuff, &pBuff, 10);
		TempUTCTime.Minute = (int)strtoul(pBuff, &pBuff, 10);
		Second = strtod(pBuff, &pBuff);
		TempUTCTime.Second = (int)Second;
		TempUTCTime.Millisecond = (int)((Second - TempUTCTime.Second) * 1000 + 0.5);

		SysTimeType = GetSysTimeType(pBuff + 5);
		if (SysTimeType < 0)
		{
			return -1;
		}

		if ((SysTimeType == SYS_TIME_TYPE_GPS) || (SysTimeType == SYS_TIME_TYPE_QZS) ||
		(SysTimeType == SYS_TIME_TYPE_GAL) || (SysTimeType == SYS_TIME_TYPE_NIC))
		{
			TempUTCTime = UTCAdd(&TempUTCTime, -18000);
		}
		else if (SysTimeType == SYS_TIME_TYPE_BDS)
		{
			TempUTCTime = UTCAdd(&TempUTCTime, -4000);
		}

		MEMCPY(&pObsHeaderInfo->ObsHeader.TimeEnd, &TempUTCTime, sizeof(UTC_TIME_T));

		pObsHeaderInfo->ObsHeader.TimeDiff = UTCDiff(&pObsHeaderInfo->ObsHeader.TimeEnd, &pObsHeaderInfo->ObsHeader.TimeStart);

		if (pObsHeaderInfo->ObsHeader.TimeDiff < 0)
		{
			GNSSPrintf("Obs file \"TIME OF FIRST OBS\" or \"TIME OF LAST OBS\" is error!\r\n");
			/** modify Time Error return */
		}
	}
	else if (strstr(pLabel, "RCV CLOCK OFFS APPL")) /**< opt */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "SYS / DCBS APPLIED")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "SYS / PCVS APPLIED")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "SYS / SCALE FACTOR")) /**< opt Ver.3 */
	{
		/** do nothing */
	}
	else if (strstr(pLabel, "SYS / PHASE SHIFT")) /**< Ver.3.01 */
	{
		pStr = strchr(s_SysCodes, pBuff[0]);
		if (!pStr)
		{
			GNSSPrintf("<GNSSQC error>: Rinex observation file 'SYS / PHASE SHIFT' info error.\r\n");
			return 0;
		}

		RINEX_PHASE_SHIFT_INFO_T* pRinexPhaseShiftInfo = &pObsHeaderInfo->ObsHeader.RinexPhaseShiftInfo;
		PER_SYS_PHASE_SHIFT_INFO_T* pPerSysPhaseShiftInfo;
		PER_SIGNAL_PHASE_SHIFT_INFO_T* pPerSignalPhaseShiftInfo;

		SysIndex = (unsigned int)(pStr - s_SysCodes);
		ExSystemID = RinexSystemToExSystem(SysIndex);

		pPerSysPhaseShiftInfo = &pRinexPhaseShiftInfo->PerSysPhaseShiftInfo[ExSystemID];

		Result = GetObsCodeFreq(pObsHeaderInfo->ObsHeader.Ver, ExSystemID, &pBuff[3], NULL, &SignalID);
		if ((Result < 0) || (SignalID >= 32))
		{
			return 0;
		}

		pPerSignalPhaseShiftInfo = &pPerSysPhaseShiftInfo->PerSignalPhaseShiftInfo[SignalID];

		pBuff += 6;
		pPerSignalPhaseShiftInfo->PhaseShiftData = strtod(pBuff, &pBuff);
		pPerSignalPhaseShiftInfo->SatNum = (unsigned int)strtoul(pBuff, &pBuff, 10);

		for (SatIndex = SatCount = 0, StrIndex = 18; SatIndex < pPerSignalPhaseShiftInfo->SatNum; SatIndex++, StrIndex += 4)
		{
			if (StrIndex >= 58)
			{
				//if (!fgets(pBuff, MAXRNXLEN, pFile))
				if (!GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN))
				{
					break;
				}
				StrIndex = 18;
				pBuff += 18;
			}
			if (SatIndex < 64)
			{
				pBuff += 2;
				pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] = (unsigned int)strtoul(pBuff, &pBuff, 10);
			}
		}
	}
	else if (strstr(pLabel, "GLONASS SLOT / FRQ #")) /**< Ver.3.02 */
	{
		for (DataIndex = 0, pStr = pBuff + 4; DataIndex < 8; DataIndex++, pStr += 8)
		{
			if (sscanf(pStr, "R%2d %2d", &PRN, &FreqChnn) < 2)
			{
				continue;
			}
			if (1 <= PRN && PRN <= MAX_GLO_SLOT)
			{
				pObsHeaderInfo->GLOFreqChnn[PRN - 1] = (char)(FreqChnn + 7);
				pObsHeaderInfo->GLOFreqNum += 1;
			}
		}
	}
	else if (strstr(pLabel, "GLONASS COD/PHS/BIS")) /**< Ver.3.02 */
	{
		for (DataIndex = 0; DataIndex < 4; DataIndex++)
		{
			pBuff++;
			if (0 == strncmp(pBuff, "C1C", 3))
			{
				pBuff += 3;
				pObsHeaderInfo->GLOPhaseBias[0] = strtod(pBuff, &pBuff);
			}
			else if (0 == strncmp(pBuff, "C1P", 3))
			{
				pBuff += 3;
				pObsHeaderInfo->GLOPhaseBias[1] = strtod(pBuff, &pBuff);
			}
			else if (0 == strncmp(pBuff, "C2C", 3))
			{
				pBuff += 3;
				pObsHeaderInfo->GLOPhaseBias[2] = strtod(pBuff, &pBuff);
			}
			else if (0 == strncmp(pBuff, "C2P", 3))
			{
				pBuff += 3;
				pObsHeaderInfo->GLOPhaseBias[3] = strtod(pBuff, &pBuff);
			}
		}
	}
	else if (strstr(pLabel, "LEAP SECONDS")) /**< opt */
	{
		pObsHeaderInfo->ObsHeader.Leapsecs = (int)strtoul(pBuff, &pBuff, 10);
	}
	else if (strstr(pLabel, "# OF SALTELLITES")) /**< opt */
	{
		pObsHeaderInfo->SatNum = (int)strtoul(pBuff, &pBuff, 10);
	}
	else if (strstr(pLabel, "PRN / # OF OBS")) /**< opt */
	{

	}

	return 0;
}

/**********************************************************************//**
@brief  decode navgation rinex file header

@param pBuff			[In] input rinex file data
@param Ver				[In] rinex file version
@param SystemID			[In] system id
@param pHeader			[Out] rinex file header information

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeNavHeader(char *pBuff, double Ver, int SystemID, RINEX_EPH_HEADER_T* pHeader)
{
	int DataIndex;
	char* pLabel = pBuff + 60;

	if (!pHeader)
	{
		return -1;
	}

	pHeader->Ver = (int)(Ver * 100 + 0.5);
	pHeader->Sys = SystemID;

	if (strstr(pLabel, "PGM / RUN BY / DATE"))
	{
		SetStr(pHeader->Pgm, pBuff, 20);
		SetStr(pHeader->Runby, pBuff + 20, 20);
	}
#if 0
	else if (strstr(pLabel, "COMMENT")) /**< opt */
	{
		SetStr(pHeader->Comments[pHeader->NCom], pBuff, 60);
		pHeader->NCom++;
		if (pHeader->NCom > MAXCOMMENT)
		{
			return-1;
		}
	}
#endif
	else if (strstr(pLabel, "ION ALPHA")) /**< opt Ver.2 */
	{
		pBuff += 2;
		for (DataIndex = 0; DataIndex < 4; DataIndex++)
		{
			pHeader->Ion[GPS][DataIndex] = strtod(pBuff, &pBuff);
		}
	}
	else if (strstr(pLabel, "ION BETA")) /**< opt Ver.2 */
	{
		pBuff += 2;
		for (DataIndex = 0; DataIndex < 4; DataIndex++)
		{
			pHeader->Ion[GPS][DataIndex + 4] = strtod(pBuff, &pBuff);
		}
	}
	else if (strstr(pLabel, "DELTA-UTC: A0,A1,T,W")) /**< opt Ver.2 */
	{
		pBuff += 3;
		for (DataIndex = 0; DataIndex < 2; DataIndex++)
		{
			pHeader->UTC[GPS][DataIndex] = strtod(pBuff, &pBuff);
		}
		for (; DataIndex < 4; DataIndex++)
		{
			pHeader->UTC[GPS][DataIndex] = strtod(pBuff, &pBuff);
		}
	}
	else if (strstr(pLabel, "IONOSPHERIC CORR")) /**< opt Ver.3 */
	{
		if (!strncmp(pBuff, "GPSA", 4))
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[GPS][DataIndex] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "GPSB", 4))
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[GPS][DataIndex + 4] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "GAL", 3))
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[GAL][DataIndex] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "QZSA", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[QZS][DataIndex] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "QZSB", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[QZS][DataIndex + 4] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "BDSA", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[BDS][DataIndex] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff, "BDSB", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[BDS][DataIndex + 4] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff,"IRNA",4)) /**< v.3.03 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[NIC][DataIndex] = strtod(pBuff, &pBuff);
			}
		}
		else if (!strncmp(pBuff,"IRNB",4)) /**< v.3.03 */
		{
			pBuff += 5;
			for (DataIndex = 0; DataIndex < 4; DataIndex++)
			{
				pHeader->Ion[NIC][DataIndex+4] = strtod(pBuff, &pBuff);
			}
		}
	}
	else if (strstr(pLabel, "TIME SYSTEM CORR")) /**< opt Ver.3 */
	{
		if (!strncmp(pBuff, "GPUT", 4))
		{
			pBuff += 5;
			pHeader->UTC[GPS][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[GPS][1] = strtod(pBuff, &pBuff);
			pHeader->UTC[GPS][2] = strtod(pBuff, &pBuff);
			pHeader->UTC[GPS][3] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff, "GLUT", 4))
		{
			pBuff += 5;
			pHeader->UTC[GLO][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[GLO][1] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff, "GAUT", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			pHeader->UTC[GAL][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[GAL][1] = strtod(pBuff, &pBuff);
			pHeader->UTC[GAL][2] = strtod(pBuff, &pBuff);
			pHeader->UTC[GAL][3] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff, "QZUT", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			pHeader->UTC[QZS][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[QZS][1] = strtod(pBuff, &pBuff);
			pHeader->UTC[QZS][2] = strtod(pBuff, &pBuff);
			pHeader->UTC[QZS][3] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff, "BDUT", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			pHeader->UTC[BDS][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[BDS][1] = strtod(pBuff, &pBuff);
			pHeader->UTC[BDS][2] = strtod(pBuff, &pBuff);
			pHeader->UTC[BDS][3] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff, "SBUT", 4)) /**< v.3.02 */
		{
			pBuff += 5;
			pHeader->UTC[SBA][0] = strtod(pBuff, &pBuff);
			pHeader->UTC[SBA][1] = strtod(pBuff, &pBuff);
			pHeader->UTC[SBA][2] = strtod(pBuff, &pBuff);
			pHeader->UTC[SBA][3] = strtod(pBuff, &pBuff);
		}
		else if (!strncmp(pBuff,"IRUT",4)) /**< v.3.03 */
		{
			pBuff += 5;
			pHeader->UTC[NIC][0]=strtod(pBuff, &pBuff);
			pHeader->UTC[NIC][1]=strtod(pBuff, &pBuff);
			pHeader->UTC[NIC][2]=strtod(pBuff, &pBuff);
			pHeader->UTC[NIC][3]=strtod(pBuff, &pBuff);
			pHeader->UTC[NIC][8]=0.0; /**< A2 */
		}
	}
	else if (strstr(pLabel, "LEAP SECONDS"))
	{
		pHeader->Leapsecs = (int)strtod(pBuff, &pBuff);
	}
	return 1;
}

/**********************************************************************//**
@brief  decode glonass navgation rinex file header

@param pBuff			[In] input rinex file data
@param Ver				[In] rinex file version
@param pHeader			[Out] rinex file header information

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeGLONavHeader(char* pBuff, double Ver, RINEX_GEPH_HEADER_T* pHeader)
{
	char* pLabel = pBuff + 60;

	pHeader->Ver = (int)(Ver * 100 + 0.5);

	if (strstr(pLabel, "PGM / RUN BY / DATE"))
	{
		SetStr(pHeader->Pgm, pBuff, 20);
		SetStr(pHeader->Runby, pBuff + 20, 20);
	}
#if 0
	else if (strstr(pLabel, "COMMENT"))
	{
		SetStr(pHeader->Comments[pHeader->NCom], pBuff, 60);
		if (++pHeader->NCom > MAXCOMMENT)
		{
			return -1;
		}
	}
#endif
	else if (strstr(pLabel, "CORR TO SYTEM TIME"));
	else if (strstr(pLabel, "LEAP SECONDS"))
	{
		if (pHeader)
		{
			pHeader->Leapsecs = (int)strtod(pBuff, &pBuff);
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  read rinex file header

@param pFile			[In] input rinex file
@param pVer				[Out] rinex file version
@param pFileType		[Out] rinex file type
@param pSystemID		[Out] system id
@param pSysTimeType		[Out] system time type
@param ObsType			[Out] obs data type
@param pRinexInfo		[InOut] rinex information staruct

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ReadRinexHeader(FILE *pFile, double* pVer, char* pFileType, int* pSystemID, int* pSysTimeType,
								char ObsType[][MAX_NUM_OBS_TYPE][4], RINEX_INFO_T* pRinexInfo, unsigned int FileTypeIndex)
{
	char TempBuf[MAXRNXLEN];
	char* pLabel = TempBuf + 60;
	unsigned int DataIndex = 0;
	double Version = 2.10;
	char FileType = ' ';
	int SystemID = RINEX_SYS_GPS;
	int SysTimeType = SYS_TIME_TYPE_GPS;
	unsigned int FileIndex = 0;
	int Result = -1;
	CYCLE_BUFF_CTRL_T* pCycleBufCtrl = &pRinexInfo->CycleBufCtrl[FileTypeIndex];

	//while (fgets(TempBuf, MAXRNXLEN, pFile))
	while (GetRinexFileLineData(pFile, pCycleBufCtrl, TempBuf, MAXRNXLEN))
	{
		if (strlen(TempBuf) <= 60)
		{
			continue;
		}
		else if (strstr(pLabel, "RINEX VERSION / TYPE"))
		{
			Version = strtod(TempBuf, NULL);
			FileType = *(TempBuf + 20);

			/** satellite SystemIDtem */
			switch (*(TempBuf + 40))
			{
				case 'G':
					SystemID = RINEX_SYS_GPS;
					SysTimeType = SYS_TIME_TYPE_GPS;
				break;
				case 'R':
					SystemID = RINEX_SYS_GLO;
					SysTimeType = SYS_TIME_TYPE_UTC;
				break;
				case 'E':
					SystemID = RINEX_SYS_GAL;
					SysTimeType = SYS_TIME_TYPE_GAL;
				break;
				case 'S':
					SystemID = RINEX_SYS_SBA;
					SysTimeType = SYS_TIME_TYPE_GPS;
				break;
				case 'J':
					SystemID = RINEX_SYS_QZS;
					SysTimeType = SYS_TIME_TYPE_QZS;
				break;
				case 'C':
					SystemID = RINEX_SYS_BDS;
					SysTimeType = SYS_TIME_TYPE_BDS;
				break;
				case 'I':
					SystemID = RINEX_SYS_NIC;
					SysTimeType = SYS_TIME_TYPE_NIC;
				case ' ':
				case 'M': /**< mixed */
					SystemID = RINEX_SYS_ALL;
					SysTimeType = SYS_TIME_TYPE_GPS;
				break;
				default:
					break;
			}

			*pFileType = FileType;
			*pSystemID = SystemID;
			*pSysTimeType = SysTimeType;
			*pVer = Version;

			continue;
		}

		/** file type */
		if (Version < 3.0) /**< v.2. */
		{
			switch (FileType)
			{
				case 'O':
					Result = DecodeObsHeader(pFile, TempBuf, Version, &SysTimeType, ObsType, &pRinexInfo->ObsHeaderInfo, pCycleBufCtrl);
					break;
				case 'G':
					Result = DecodeGLONavHeader(TempBuf, Version, &pRinexInfo->GLOEphHeader);
					pRinexInfo->Leapsecs = pRinexInfo->GLOEphHeader.Leapsecs;
					break;
				case 'N':
					FileIndex = RINEX_FILE_INDEX_GPSEPH;
					Result = DecodeNavHeader(TempBuf, Version, SystemID, &pRinexInfo->EphHeader[FileIndex]);
					pRinexInfo->Leapsecs = pRinexInfo->EphHeader[FileIndex].Leapsecs;
					break;
				case 'E':
					FileIndex = RINEX_FILE_INDEX_GALEPH;
					Result = DecodeNavHeader(TempBuf, Version, SystemID, &pRinexInfo->EphHeader[FileIndex]);
					pRinexInfo->Leapsecs = pRinexInfo->EphHeader[FileIndex].Leapsecs;
					break;
				case 'C':
					FileIndex = RINEX_FILE_INDEX_BDSEPH;
					Result = DecodeNavHeader(TempBuf, Version, SystemID, &pRinexInfo->EphHeader[FileIndex]);
					pRinexInfo->Leapsecs = pRinexInfo->EphHeader[FileIndex].Leapsecs;
					break;
				default:
					break;
			}
		}
		else /**< v.3. */
		{
			switch (FileType)
			{
				case 'O':
					Result = DecodeObsHeader(pFile, TempBuf, Version, &SysTimeType, ObsType, &pRinexInfo->ObsHeaderInfo, pCycleBufCtrl);
					break;
				case 'N':
				{
					if (SystemID == RINEX_SYS_GLO)
					{
						Result = DecodeGLONavHeader(TempBuf, Version, &pRinexInfo->GLOEphHeader);
						pRinexInfo->Leapsecs = pRinexInfo->GLOEphHeader.Leapsecs;
						break;
					}
					else
					{
						FileIndex = RinexSystemMaskToRinexFileIndex(SystemID);
						if (NA8 == FileIndex)
						{
							return -1;
						}
						Result = DecodeNavHeader(TempBuf, Version, SystemID, &pRinexInfo->EphHeader[FileIndex]);
						pRinexInfo->Leapsecs = pRinexInfo->EphHeader[FileIndex].Leapsecs;
						break;
					}
				}
					break;
				default:
					break;
			}
		}

		if (Result < 0)
		{
			return Result;
		}

		if (strstr(pLabel, "END OF HEADER"))
		{
			if (Version < 3.0) /**< v.2. */
			{
				if (FileType == 'O')
					return RINEX_ID_OBS_HEADER;
				else if (FileType == 'N')
					return RINEX_ID_GPSEPH_HEADER;
				else if (FileType == 'G')
					return RINEX_ID_GLOEPH_HEADER;
				else if (FileType == 'C')
					return RINEX_ID_BDSEPH_HEADER;
				else if (FileType == 'E')
					return RINEX_ID_GALEPH_HEADER;
			}
			else /**< v.3. */
			{
				if (FileType == 'O')
				{
					return RINEX_ID_OBS_HEADER;
				}
				else if (FileType == 'N')
				{
					return RinexSystemMaskToRinexHeaderID(SystemID);
				}
			}
		}

		if (++DataIndex >= MAXPOSHEAD && FileType == ' ')
		{
			break; /**< no rinex file */
		}
	}

	return -1;
}

/**********************************************************************//**
@brief  decode obs epoch

@param pFile			[In] input rinex file
@param pBuff			[In] rinex file data buffer
@param Ver				[In] rinex file version
@param pUTCTime			[In] UTC Time
@param pFlag			[In] data flag
@param SatBuf			[In] satid buffer
@param pClkOff			[Out] clock off

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeObsEpoch(FILE *pFile, char* pBuff, int Ver, int SysTimeType, UTC_TIME_T* pUTCTime,
	int* pFlag, int* SysBuf, int* SatBuf, double* pClkOff, CYCLE_BUFF_CTRL_T* pCycleBufCtrl)
{
	int SatIndex, StrIndex, SatNum;
	char SatIDBuf[8] = "";
	int SystemID, PRN;
	int Result;

	if (!pUTCTime)
	{
		return -1;
	}

	if (Ver <= 299) /**< Ver.2 */
	{
		if ((SatNum = (int)strtoul(&pBuff[29], NULL, 10)) <= 0)
		{
			return 0;
		}

		*pFlag = (int)strtoul(&pBuff[28], NULL, 10); /**< epoch flag: 3:new site,4:header info,5:external event */
		if (3 <= *pFlag && *pFlag <= 5)
		{
			return SatNum;
		}

		*pClkOff = strtod(&pBuff[68], NULL);

		if (ObsStr2Time(pBuff, 0, 26, pUTCTime, SysTimeType))
		{
			return 0;
		}

		for (SatIndex = 0, StrIndex = 32; SatIndex < SatNum; StrIndex += 3)
		{
			if (StrIndex >= 68)
			{
				//if (!fgets(pBuff, MAXRNXLEN, pFile))
				if (!GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN))
				{
					break;
				}
				StrIndex = 32;
			}
			if (SatIndex < RINEX_SAT_NUM_MAX)
			{
				strncpy(SatIDBuf, pBuff + StrIndex, 3);
				Result = SatID2SatNo(SatIDBuf, &SystemID, &PRN);
				if (Result < 0)
				{
					continue;
				}
				SysBuf[SatIndex] = SystemID;
				SatBuf[SatIndex++] = PRN;
			}
		}
	}
	else /**< Ver.3 */
	{
		if ((SatNum = (int)strtoul(&pBuff[32], NULL, 10)) <= 0)
		{
			return 0;
		}

		*pFlag = (int)strtoul(&pBuff[31], NULL, 10); /** epoch flag: 3:new site,4:header info,5:external event */
		if (3 <= *pFlag && *pFlag <= 5)
		{
			return SatNum;
		}

		if (pBuff[0] != '>' || ObsStr2Time(pBuff, 1, 28, pUTCTime, SysTimeType))
		{
			return 0;
		}
	}

	return SatNum;
}

/**********************************************************************//**
@brief  decode obs data

@param pRinexInfo		[In] rinex information staruct
@param pFile			[In] input rinex file
@param pBuff			[In] rinex file data buffer
@param Ver				[In] rinex file version
@param pSignalIndexInfo	[In] signal index information
@param pSatObsData		[Out] per satellite obs data
@param SysID			[Out] system id
@param PerPRN			[Out] PRN

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeObsData(RINEX_INFO_T* pRinexInfo, FILE* pFile, char* pBuff, int Ver, SIGNAL_INDEX_INFO_T* pSignalIndexInfo,
	OBS_DATA_SAT_T* pSatObsData, CYCLE_BUFF_CTRL_T* pCycleBufCtrl, int SysID, int PerPRN)
{
	SIGNAL_INDEX_INFO_T* pCurSignalIndexInfo;
	double Val[MAX_NUM_OBS_TYPE] = { 0 };
	unsigned char Lock[MAX_NUM_OBS_TYPE] = { 0 };
	unsigned char Sign[MAX_NUM_OBS_TYPE] = { 0 };
	char SatIDBuf[8] = "", SignalMax;
	int PRN = 0;
	int Status = 1, Pos[MAX_NUM_OBS_TYPE], RinexSystemID = 0;
	int StrIndex, ObsTypeIndex;
	int Result;
	unsigned int ObsIndex, DataIndex;
	OBS_DATA_PER_SAT_T* pObsDataPerSat;
	unsigned int C1DataIndex = 0;
	unsigned int C2DataIndex = 0;
	int C1DataBuf[16], C2DataBuf[16];
	unsigned int RMSystemID, ExSystemID;
	unsigned char PhaseLock = 0, CodeLock = 0, HalfCycle = 0;
	unsigned int SatIndex;
	RM_RANGE_DATA_T* pCurRangeData;
	unsigned int ObsNum;
	unsigned int SatID;
	unsigned int LockTimeFlag[MAX_FREQ_INDEX_NUM+1] = {0};
	RINEX_OBS_HEADER_INFO_T* pObsHeaderInfo;
	char TmpBuf[16];

	if ((!pSignalIndexInfo) || (!pSatObsData))
	{
		return -1;
	}

	SatIndex = pSatObsData->SatNum;
	pObsDataPerSat = &pSatObsData->ObsDataPerSat[SatIndex];
	pObsHeaderInfo = &pRinexInfo->ObsHeaderInfo;

	if (Ver > 299) /**< Ver.3 */
	{
		strncpy(SatIDBuf, pBuff, 3);
		Result = SatID2SatNo(SatIDBuf, &RinexSystemID, &PRN);
		if (Result < 0)
		{
			return -1;
		}

		pObsDataPerSat->SatID = (unsigned short)PRN;
	}
	else
	{
		PRN = PerPRN;
		pObsDataPerSat->SatID = (unsigned short)PRN;
		RinexSystemID = SysID;
	}
	if (!pObsDataPerSat->SatID)
	{
		Status = 0;
	}

	/** read obs data fields */
	RMSystemID = RinexSystemMaskToRMSystem(RinexSystemID);
	if (RMSystemID == NA8)
	{
		return -1;
	}

	ExSystemID = RinexSystemMaskToExSystem(RinexSystemID);
	if (ExSystemID == NA8)
	{
		return -1;
	}
	pCurSignalIndexInfo = &pSignalIndexInfo[ExSystemID];

	pObsDataPerSat->SystemID = RMSystemID;
	ObsIndex = pObsDataPerSat->ObsNum;
	pCurRangeData = &pObsDataPerSat->ObsData[0];

	for (ObsTypeIndex = 0, StrIndex = ((Ver <= 299) ? 0 : 3); ObsTypeIndex < pCurSignalIndexInfo->Num; ObsTypeIndex++, StrIndex += 16)
	{
		if (Ver <= 299 && StrIndex >= 80) /**< Ver.2 */
		{
			//if (!fgets(pBuff, MAXRNXLEN, pFile))
			if (!GetRinexFileLineData(pFile, pCycleBufCtrl, pBuff, MAXRNXLEN))
			{
				break;
			}
			StrIndex = 0;
		}
		if (Status)
		{
			strncpy(TmpBuf, &pBuff[StrIndex], 14);
			TmpBuf[14] = '\0';
			Val[ObsTypeIndex] = strtod(TmpBuf, NULL) + pCurSignalIndexInfo->Shift[ObsTypeIndex];

			TmpBuf[0] = pBuff[StrIndex + 14];
			TmpBuf[1] = '\0';
			Lock[ObsTypeIndex] = (unsigned char)strtoul(TmpBuf, NULL, 10);

			TmpBuf[0] = pBuff[StrIndex + 15];
			TmpBuf[1] = '\0';
			Sign[ObsTypeIndex] = (unsigned char)strtoul(TmpBuf, NULL, 10);
		}
	}
	if (!Status)
	{
		return 0;
	}

	/** assign position in obs data */
	C1DataIndex = 0;
	C2DataIndex = 0;
	for (ObsTypeIndex = 0; ObsTypeIndex < pCurSignalIndexInfo->Num; ObsTypeIndex++)
	{
		Pos[ObsTypeIndex] = (Ver <= 211) ? pCurSignalIndexInfo->Freq[ObsTypeIndex] : pCurSignalIndexInfo->Pos[ObsTypeIndex];

		if (pCurSignalIndexInfo->DataType[ObsTypeIndex] == 0 && Pos[ObsTypeIndex] == 0) /**< C1? pSignalIndexInfo */
		{
			C1DataBuf[C1DataIndex++] = ObsTypeIndex;
		}
		if (pCurSignalIndexInfo->DataType[ObsTypeIndex] == 0 && Pos[ObsTypeIndex] == 1) /**< C2? pSignalIndexInfo */
		{
			C2DataBuf[C2DataIndex++] = ObsTypeIndex;
		}
	}

	if (Ver <= 211)
	{
		/** if multiple codes (C1/P1,C2/P2), select higher priority */
		if (C1DataIndex >= 2)
		{
			if (fabs(Val[C1DataBuf[0]]) < 1E-15 && fabs(Val[C1DataBuf[1]]) < 1E-15)
			{
				Pos[C1DataBuf[0]] = -1;
				Pos[C1DataBuf[1]] = -1;
			}
			else if (fabs(Val[C1DataBuf[0]]) > 1E-15 && fabs(Val[C1DataBuf[1]]) < 1E-15)
			{
				Pos[C1DataBuf[0]] = 0;
				Pos[C1DataBuf[1]] = -1;
			}
			else if (fabs(Val[C1DataBuf[0]]) < 1E-15 && fabs(Val[C1DataBuf[1]]) > 1E-15)
			{
				Pos[C1DataBuf[0]] = -1;
				Pos[C1DataBuf[1]] = 0;
			}
			else if (pCurSignalIndexInfo->Pri[C1DataBuf[1]] > pCurSignalIndexInfo->Pri[C1DataBuf[0]])
			{
				Pos[C1DataBuf[1]] = 0;
				Pos[C1DataBuf[0]] = EXTEND_OBS_CODE_FREQ_NUM < 1 ? -1 : MAX_FREQ_INDEX_NUM;
			}
			else
			{
				Pos[C1DataBuf[0]] = 0;
				Pos[C1DataBuf[1]] = EXTEND_OBS_CODE_FREQ_NUM < 1 ? -1 : MAX_FREQ_INDEX_NUM;
			}
		}
		if (C2DataIndex >= 2)
		{
			if (fabs(Val[C2DataBuf[0]]) < 1E-15 && fabs(Val[C2DataBuf[1]]) < 1E-15)
			{
				Pos[C2DataBuf[0]] = -1;
				Pos[C2DataBuf[1]] = -1;
			}
			else if (fabs(Val[C2DataBuf[0]]) > 1E-15 && fabs(Val[C2DataBuf[1]]) < 1E-15)
			{
				Pos[C2DataBuf[0]] = 1;
				Pos[C2DataBuf[1]] = -1;
			}
			else if (fabs(Val[C2DataBuf[0]]) < 1E-15 && fabs(Val[C2DataBuf[1]]) > 1E-15)
			{
				Pos[C2DataBuf[0]] = -1;
				Pos[C2DataBuf[1]] = 1;
			}
			else if (pCurSignalIndexInfo->Pri[C2DataBuf[1]] > pCurSignalIndexInfo->Pri[C2DataBuf[0]])
			{
				Pos[C2DataBuf[1]] = 1;
				Pos[C2DataBuf[0]] = EXTEND_OBS_CODE_FREQ_NUM < 2 ? -1 : MAX_FREQ_INDEX_NUM + 1;
			}
			else
			{
				Pos[C2DataBuf[0]] = 1;
				Pos[C2DataBuf[1]] = EXTEND_OBS_CODE_FREQ_NUM < 2 ? -1 : MAX_FREQ_INDEX_NUM + 1;
			}
		}
	}

	/** save obs data */
	SignalMax = -1;
	for (ObsTypeIndex = 0; ObsTypeIndex < pCurSignalIndexInfo->Num; ObsTypeIndex++)
	{
		if (Pos[ObsTypeIndex] < 0 || fabs(Val[ObsTypeIndex]) < 1E-15)
		{
			continue;
		}

		if (Pos[ObsTypeIndex] > SignalMax)
		{
			SignalMax++;
		}

		SatID = PRNToMSMSatID(RMSystemID, PRN);

		switch (pCurSignalIndexInfo->DataType[ObsTypeIndex])
		{
			case 0:
				pCurRangeData[Pos[ObsTypeIndex]].PRN = (unsigned short)PRN;
				pCurRangeData[Pos[ObsTypeIndex]].PSR = Val[ObsTypeIndex];
				if (RMSystemID == RM_RANGE_SYS_GLO)
				{
					if (pObsHeaderInfo->GLOFreqNum != 0)
					{
						pCurRangeData[Pos[ObsTypeIndex]].FreqNum = (unsigned short)pObsHeaderInfo->GLOFreqChnn[PRN-38];
					}
					else
					{
						pCurRangeData[Pos[ObsTypeIndex]].FreqNum = (unsigned short)GetGloFreqCh(SatID);
					}
				}
				else
				{
					pCurRangeData[Pos[ObsTypeIndex]].FreqNum = 0;
				}
				CodeLock = 1;
				pCurRangeData[Pos[ObsTypeIndex]].ChnTrackStatus |= COMPOSE_CH_TRACK_STATUS_ALL(0, 0, CodeLock, RMSystemID, pCurSignalIndexInfo->SignalType[ObsTypeIndex], 0);
				break;
			case 1:
				pCurRangeData[Pos[ObsTypeIndex]].PRN = (unsigned short)PRN;
				pCurRangeData[Pos[ObsTypeIndex]].ADR = -Val[ObsTypeIndex];
				if (Lock[ObsTypeIndex])
				{
					pCurRangeData[Pos[ObsTypeIndex]].LockTime = 1;
				}
				else
				{
					pCurRangeData[Pos[ObsTypeIndex]].LockTime = 50;
				}
				PhaseLock = (Lock[ObsTypeIndex] & 0x01) ? 0 : 1;
				HalfCycle = (Lock[ObsTypeIndex] & 0x02) ? 0 : 1;
				pCurRangeData[Pos[ObsTypeIndex]].ChnTrackStatus |= COMPOSE_CH_TRACK_STATUS_ALL(PhaseLock, 0, 0, RMSystemID, pCurSignalIndexInfo->SignalType[ObsTypeIndex], HalfCycle);
				break;
			case 2:
				pCurRangeData[Pos[ObsTypeIndex]].Doppler = (float)Val[ObsTypeIndex];
				break;
			case 3:
				pCurRangeData[Pos[ObsTypeIndex]].CN0 = (float)Val[ObsTypeIndex];
				break;
		}
	}

	ObsNum = SignalMax + 1;
	DataIndex = 0;
	for (ObsIndex = 0; ObsIndex < ObsNum; ObsIndex++)
	{
		if (pCurRangeData[ObsIndex].PRN == 0)
		{
			DataIndex++;
		}
	}

	pObsDataPerSat->ObsNum += ObsNum - DataIndex;

	return 0;
}

/**********************************************************************//**
@brief  get phase shift option string

@param ExSystemID		[In] System ID

@retval phase shift option string

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static const char* GetPhaseShiftOptionStr(int ExSystemID)
{
	const static char* s_PhaseShiftOptionStrMap[] = {
		"-GL%2s=%lf", /**< GPS */
		"-RL%2s=%lf", /**< GLO */
		"-EL%2s=%lf", /**< GAL */
		"-CL%2s=%lf", /**< BDS */
		"-SL%2s=%lf", /**< SBA */
		"-JL%2s=%lf", /**< QZS */
		"-IL%2s=%lf", /**< NIC */
	};

	if (ExSystemID < TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return s_PhaseShiftOptionStrMap[ExSystemID];
	}

	return NULL;
}

/**********************************************************************//**
@brief  set signal index

@param Ver				[In] rinex file version
@param ExSystemID		[In] System ID
@param pOpt				[In] pOpt
@param ObsType			[In] Obs Type
@param pSignalIndexInfo	[In] Signal Index Information

@retval <0 error, >=0 success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
static int SetSignalIndex(int Ver, int ExSystemID, const char *pOpt,
	char ObsType[MAX_NUM_OBS_TYPE][4], SIGNAL_INDEX_INFO_T* pSignalIndexInfo)
{
	const char* pStr;
	const char StrBuf[8] = { 0 };
	const char* pOptStr = NULL;
	double Shift;
	int ObsTypeIndex;
	int DataCount = 0;
	int ObsDataIndex;
	int DataIndex;
	int Result;
	int PosCount;

	MEMSET(pSignalIndexInfo, 0xFF, sizeof(SIGNAL_INDEX_INFO_T));

	for (ObsTypeIndex = 0; *ObsType[ObsTypeIndex]; ObsTypeIndex++, DataCount++)
	{
		Result = GetObsCodeFreq(Ver, ExSystemID, ObsType[ObsTypeIndex] + 1, &pSignalIndexInfo->Freq[ObsTypeIndex], &pSignalIndexInfo->SignalType[ObsTypeIndex]);
		if (Result < 0)
		{
			pSignalIndexInfo->Freq[ObsTypeIndex] = NA8;
			pSignalIndexInfo->SignalType[ObsTypeIndex] = NA8;
			continue;
		}

		pStr = strchr(s_ObsCodes, ObsType[ObsTypeIndex][0]);
		pSignalIndexInfo->DataType[ObsTypeIndex] = pStr ? (unsigned char)(pStr - s_ObsCodes) : 0;
		pSignalIndexInfo->Pri[ObsTypeIndex] = (unsigned char)GetRinexCodePri(ExSystemID, pSignalIndexInfo->Freq[ObsTypeIndex], ObsType[ObsTypeIndex][3]);
		pSignalIndexInfo->Pos[ObsTypeIndex] = -1;
	}

	MEMSET(pSignalIndexInfo->Shift, 0x00, sizeof(pSignalIndexInfo->Shift));

	/** parse phase shift options */
	pOptStr = GetPhaseShiftOptionStr(ExSystemID);
	if (NULL == pOptStr)
	{
		return -1;
	}

	for (pStr = pOpt; pStr;)
	{
		pStr = strchr(pStr, '-');
		if (pStr)
		{
			pStr++;
			if (sscanf(pStr, pOptStr, StrBuf, &Shift) < 2)
			{
				continue;
			}

			for (ObsTypeIndex = 0; ObsTypeIndex < DataCount; ObsTypeIndex++)
			{
				if (strcmp(GetObsCodeType(Ver, ExSystemID, pSignalIndexInfo->SignalType[ObsTypeIndex]), StrBuf))
				{
					continue;
				}
				pSignalIndexInfo->Shift[ObsTypeIndex] = Shift;
			}
		}
		else
		{
			break;
		}
	}

	/** assign index for highest priority code */
	for (ObsDataIndex = 0; ObsDataIndex < DataCount; ObsDataIndex++)
	{
		if (pSignalIndexInfo->Pri[ObsTypeIndex] && (pSignalIndexInfo->Pos[ObsDataIndex] < 0))
		{
			if (ObsDataIndex == 0)
			{
				pSignalIndexInfo->Pos[ObsDataIndex] = 0;
				PosCount = 0;
			}
			else
			{
				for (ObsTypeIndex = 0, DataIndex = -1; ObsTypeIndex < ObsDataIndex; ObsTypeIndex++)
				{
					if (pSignalIndexInfo->SignalType[ObsTypeIndex] == pSignalIndexInfo->SignalType[ObsDataIndex])
					{
						pSignalIndexInfo->Pos[ObsDataIndex] = pSignalIndexInfo->Pos[ObsTypeIndex];
						break;
					}
				}

				if (ObsTypeIndex >= ObsDataIndex)
				{
					pSignalIndexInfo->Pos[ObsTypeIndex] = ++PosCount;
				}
			}
		}
	}

	pSignalIndexInfo->Num = DataCount;

	return 0;
}

/**********************************************************************//**
@brief  read rinex obs data body

@param pRinexInfo		[In] rinex information staruct
@param pFile			[In] input rinex file
@param pOpt				[In] pOpt
@param SysTimeType		[In] System time type
@param Ver				[In] rinex file version
@param ObsType			[In] obs data type
@param pFlag			[In] data flag
@param pSatObsData		[Out] per satellite obs data
@param pObsData			[Out] obs data
@param pClkOff			[In] clock off
@param Leapsecs			[In] leap second

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ReadRinexObsBody(RINEX_INFO_T* pRinexInfo, FILE *pFile, const char* pOpt, int Ver, int SysTimeType,
	char ObsType[][MAX_NUM_OBS_TYPE][4], int* pFlag, OBS_DATA_SAT_T* pSatObsData, GNSS_OBS_T* pObsData, double* pClkOff, int Leapsecs)
{
	UTC_TIME_T UTCTime = { 0 };
	SIGNAL_INDEX_INFO_T SignalIndexInfoTab[TOTAL_EX_GNSS_SYSTEM_NUM] = {0};
	char TempBuf[MAXRNXLEN];
	int DataIndex = 0, SatNum = 0, SysBuf[RINEX_SAT_NUM_MAX] = {0}, SatBuf[RINEX_SAT_NUM_MAX] = {0};
	unsigned int SysIndex;
	CYCLE_BUFF_CTRL_T* pCycleBufCtrl = &pRinexInfo->CycleBufCtrl[RINEX_FILE_INDEX_OBS];

	/** set signal index */
	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		SetSignalIndex(Ver, SysIndex, pOpt, ObsType[SysIndex], &SignalIndexInfoTab[SysIndex]);
	}

	MEMSET(pSatObsData, 0, sizeof(OBS_DATA_SAT_T));
	MEMSET(pObsData, 0, sizeof(GNSS_OBS_T));

	/** read record */
	//while (fgets(TempBuf, MAXRNXLEN, pFile))
	while (GetRinexFileLineData(pFile, pCycleBufCtrl, TempBuf, MAXRNXLEN))
	{
		/** decode obs epoch */
		if (DataIndex == 0)
		{
			if ((SatNum = DecodeObsEpoch(pFile, TempBuf, Ver, SysTimeType, &UTCTime, pFlag, SysBuf, SatBuf, pClkOff, pCycleBufCtrl)) <= 0)
			{
				continue;
			}
			pObsData->ObsTime = UTCTime;
#if 0
			TimeDiff = UTCDiff(&pObsData->ObsTime, &pRinexInfo->ObsHeaderInfo.ObsHeader.TimeStart);
			if (TimeDiff < 0)
			{
				printf("Error:Obs file header time information is different from obs data time!\r\n");
				return -1;
			}
#endif
		}
		else if (*pFlag <= 2 || *pFlag == 6)
		{
			/** decode obs data */
			if (pSatObsData->SatNum < OBS_DATA_SAT_NUM_MAX)
			{
				if (DecodeObsData(pRinexInfo, pFile, TempBuf, Ver, SignalIndexInfoTab, pSatObsData, pCycleBufCtrl, SysBuf[pSatObsData->SatNum], SatBuf[pSatObsData->SatNum]) < 0)
				{
					return -1;
				}
				pSatObsData->SatNum++;
			}
		}

		if (DataIndex++ >= SatNum)
		{
			ObsDataConvert(pObsData, pSatObsData);
			return 0;
		}

		MEMSET(TempBuf, 0, MAXRNXLEN);
	}

	return -1;
}

/**********************************************************************//**
@brief  decode ephemeris

@param RinexID			[In] rinex id
@param PRN				[In] gnss prn
@param pUTCTime			[In] UTC time
@param pData			[In] rinex file data
@param pEphData			[Out] eph data
@param pHeader			[Out] rawmsg eph header

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeUniEphVer4BDS(int RinexID,int PRN, UTC_TIME_T* pUTCTime, const double* pData, UNI_EPHEMERIS_T* pEphData, RM_HEADER_T* pHeader)
{
	int TempWeek, WeekMsCount, GPSLeapSecMs;

	if ((!pUTCTime) || (!pData) || (!pEphData))
	{
		return -1;
	}

	pEphData->SystemID = BDS;
	pEphData->SatID = PRN;

	GPSLeapSecMs = GetGPSLeapSecondMs(pUTCTime);
	UTCToGPSTime(pUTCTime, &GPSLeapSecMs, &TempWeek, &WeekMsCount);

	pHeader->Week = TempWeek;
	pHeader->Milliseconds = WeekMsCount;

	pEphData->TOC = (int)(WeekMsCount / 1000);

	pEphData->Af0 = pData[0];
	pEphData->Af1 = pData[1];
	pEphData->Af2 = pData[2];

	pEphData->IODE1 = (unsigned short)pData[3];		/**< AODE */
	pEphData->Crs = pData[4];
	pEphData->DeltaN = pData[5];
	pEphData->M0 = pData[6];

	pEphData->Cuc = pData[7];
	pEphData->Ecc = pData[8];
	pEphData->Cus = pData[9];
	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		pEphData->A = SQR(pData[10]);
	}
	else
	{
		pEphData->SqrtAorDeltaA = pData[10];
	}

	pEphData->TOE = (int)pData[11]; 				/**< toe (s) in gps week */
	pEphData->Cic = pData[12];
	pEphData->Omega0 = pData[13];
	pEphData->Cis = pData[14];

	pEphData->I0 = pData[15];
	pEphData->Crc = pData[16];
	pEphData->Omega = pData[17];
	pEphData->OmegaDot = pData[18];

	pEphData->IDot = pData[19];
	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		pEphData->DeltaN = pData[20];
	}
	else
	{
		pEphData->Week = (unsigned short)pData[21];		/**< bdt week */
	}

	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		/**< todo: need modify SISAI_oe */
		/**< todo: need modify SISAI_ocb */
		/**< todo: need modify SISAI_oc1 */
		/**< todo: need modify SISAI_oc2 */
	}
	else
	{
		pEphData->URA = (unsigned char)GetUraIndex(pData[23]);			/**< ura (m->index) */
		pEphData->Health = (unsigned char)pData[24];	/**< satH1 */
		pEphData->Tgd[0] = pData[25];					/**< TGD1 B1/B3 */
		pEphData->Tgd[1] = pData[26];					/**< TGD2 B1/B3 */
	}

	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		if (RinexID == RINEX_ID_BDSEPH_CNAV1)
		{
			pEphData->Isc[0] = pData[27]; /**< ISC_B1Cd */
		}
		else if (RinexID == RINEX_ID_BDSEPH_CNAV2)
		{
			pEphData->Isc[1] = pData[28]; /**< ISC_B2ad */
		}
		pEphData->Tgd[0] = pData[29]; /**< BDS CNAV: TGD_B1Cp*/
		pEphData->Tgd[1] = pData[30]; /**< BDS CNAV: TGD_B2ap */
	}
	else
	{
		pEphData->IODC = (unsigned short)pData[28];		/**< AODC */
	}

	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		pEphData->URA = (unsigned char)pData[31]; /**< SISMAI */
		pEphData->Health = (unsigned char)pData[32]; /**< Health */

		unsigned short B1C_DIF = 0, B1C_SIF = 0, B1C_AIF = 0;
		unsigned short B2a_DIF = 0, B2a_SIF = 0, B2a_AIF = 0;
		unsigned short B2b_DIF = 0, B2b_SIF = 0, B2b_AIF = 0;
		unsigned short IntegrityFlags = 0;

		IntegrityFlags = (unsigned short)pData[33];

		if (RinexID == RINEX_ID_BDSEPH_CNAV1)
		{
			/** bit 0-2: AIF(B1C),SIF (B1C),DIF(B1C) */
			B1C_AIF = IntegrityFlags & 0x01;
			B1C_SIF = (IntegrityFlags >> 1) & 0x01;
			B1C_DIF = (IntegrityFlags >> 2) & 0x01;
		}
		else if (RinexID == RINEX_ID_BDSEPH_CNAV2)
		{
			/** bit 0-5: AIF(B1C),SIF (B1C),DIF(B1C),AIF(B2a),SIF(B2a),DIF(B2a) */
			B1C_AIF = IntegrityFlags & 0x01;
			B1C_SIF = (IntegrityFlags >> 1) & 0x01;
			B1C_DIF = (IntegrityFlags >> 2) & 0x01;
			B2a_AIF = (IntegrityFlags >> 3) & 0x01;
			B2a_SIF = (IntegrityFlags >> 4) & 0x01;
			B2a_DIF = (IntegrityFlags >> 5) & 0x01;
		}
		else if (RinexID == RINEX_ID_BDSEPH_CNAV3)
		{
			/** bit 0-2: AIF(B2b),SIF (B2b),DIF(B2b) */
			B2b_AIF = IntegrityFlags & 0x01;
			B2b_SIF = (IntegrityFlags >> 1) & 0x01;
			B2b_DIF = (IntegrityFlags >> 2) & 0x01;
		}

		pEphData->Reserved = B1C_DIF + (B1C_SIF << 1) + (B1C_AIF << 2) + (B2a_DIF << 3) + (B2a_SIF << 4) + (B2a_AIF << 5) +
								(B2b_DIF << 6) + (B2b_SIF << 7) + (B2b_AIF << 8); /**< BDS CNAV, bit 0-2, for B1C DIF, SIF, AIF, bit 3-5 for B2a DIF, SIF, AIF, bit 6-8 for B2b DIF, SIF, AIF */

		if ((RinexID == RINEX_ID_BDSEPH_CNAV1) ||(RinexID == RINEX_ID_BDSEPH_CNAV2))
		{
			pEphData->IODC = (unsigned short)pData[34]; /**< IODC */
		}
		else
		{
			pEphData->Tgd[2] = pData[34]; /**< BDS CNAV: TGD_B2bI */
		}

		if ((RinexID == RINEX_ID_BDSEPH_CNAV1) ||(RinexID == RINEX_ID_BDSEPH_CNAV2))
		{
			pEphData->IODE1 = (unsigned short)pData[38]; /**< IODE */
		}
	}

	pEphData->TOC = pEphData->TOC + 14; 			/**<gpst -> bdt*/

	if ((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2) || (RinexID == RINEX_ID_BDSEPH_CNAV3))
	{
		CalcEphInterimParamBD3(pEphData);
		pEphData->EphDataType = EPH_DATA_TYPE_BDS_CNAV;
	}
	else
	{
		if (IS_BDS_GEO_PRN(pEphData->SatID))
		{
			CalcEphInterimParamGEO(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pEphData);
		}
		else
		{
			CalcEphInterimParam(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pEphData);
		}
		pEphData->EphDataType = EPH_DATA_TYPE_BDS_BD2;
	}

	pEphData->EphState = NAV_DATA_STATE_VALID;

	return 0;
}

/**********************************************************************//**
@brief  decode ephemeris

@param Ver				[In] rinex file version
@param SystemID			[In] gnss system id
@param RinexID			[InOut] rinex id
@param PRN				[In] gnss prn
@param pUTCTime			[In] UTC time
@param pData			[In] rinex file data
@param pEphData			[Out] eph data
@param pHeader			[Out] rawmsg eph header

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeUniEph(int Ver, int SystemID, int* pRinexID, int PRN, UTC_TIME_T* pUTCTime, const double* pData, UNI_EPHEMERIS_T* pEphData, RM_HEADER_T* pHeader)
{
	int ExSystemID;
	int TempWeek, WeekMsCount, GPSLeapSecMs;
	int RinexID = *pRinexID;

	if ((!pUTCTime) || (!pData) || (!pEphData))
	{
		return -1;
	}

	if (!(SystemID & (RINEX_SYS_GPS | RINEX_SYS_SBA | RINEX_SYS_GAL | RINEX_SYS_QZS | RINEX_SYS_BDS | RINEX_SYS_NIC)))
	{
		return -1;
	}

	ExSystemID = RinexSystemMaskToExSystem(SystemID);
	if (ExSystemID > TOTAL_EX_GNSS_SYSTEM_NUM)
	{
		return -1;
	}

	pEphData->SystemID = ExSystemID;
	if (ExSystemID == QZS)
	{
		pEphData->SatID = PRN + 1 - MIN_QZSS_RANGE_PRN;
	}
	else
	{
		pEphData->SatID = PRN;
	}

	GPSLeapSecMs = GetGPSLeapSecondMs(pUTCTime);
	UTCToGPSTime(pUTCTime, &GPSLeapSecMs, &TempWeek, &WeekMsCount);

	pHeader->Week = TempWeek;
	pHeader->Milliseconds = WeekMsCount;

	pEphData->TOC = (int)(WeekMsCount / 1000);

	pEphData->Af0 = pData[0];
	pEphData->Af1 = pData[1];
	pEphData->Af2 = pData[2];

	pEphData->A = SQR(pData[10]);
	pEphData->SqrtAorDeltaA = pData[10];
	pEphData->Ecc = pData[8];
	pEphData->I0 = pData[15];
	pEphData->Omega0 = pData[13];
	pEphData->Omega = pData[17];
	pEphData->M0 = pData[6];
	pEphData->DeltaN = pData[5];
	pEphData->OmegaDot = pData[18];
	pEphData->IDot = pData[19];
	pEphData->Crc = pData[16];
	pEphData->Crs = pData[4];
	pEphData->Cuc = pData[7];
	pEphData->Cus = pData[9];
	pEphData->Cic = pData[12];
	pEphData->Cis = pData[14];

	if (SystemID == RINEX_SYS_GPS || SystemID == RINEX_SYS_QZS)
	{
		pEphData->IODE1 = (unsigned short)pData[3];		/**< IODE */
		pEphData->IODC = (unsigned short)pData[26];		/**< IODC */
		pEphData->TOE = (int)pData[11];					/**< toe (s) in gps week */
		pEphData->Week = (unsigned short)pData[21];		/**< gps week */

		pEphData->Health = (unsigned char)pData[24];	/**< sv health */
		pEphData->URA = GetUraIndex(pData[23]);			/**< ura (m->index) */

		pEphData->Tgd[0] = pData[25];					/**< TGD */
		pEphData->FitInterval = (unsigned char)pData[28];	/**< fit interval */

		CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pEphData);

		if (SystemID == RINEX_SYS_GPS)
		{
			pEphData->EphDataType = EPH_DATA_TYPE_GPS;
			*pRinexID = RINEX_ID_GPSEPH;
		}
		else if (SystemID == RINEX_SYS_QZS)
		{
			pEphData->EphDataType = EPH_DATA_TYPE_QZS;
			*pRinexID = RINEX_ID_QZSSEPH;
		}
	}
	else if (SystemID == RINEX_SYS_NIC)
	{
		pEphData->IODE1 = (unsigned short)pData[3];		/**< IODE */
		pEphData->IODC = (unsigned short)pData[26];		/**< IODC */
		pEphData->TOE = (int)pData[11];					/**< toe (s) in gps week */
		pEphData->Week = (unsigned short)pData[21];		/**< gps week */

		pEphData->Health = (unsigned char)pData[24];	/**< sv health */
		pEphData->URA = GetUraIndex(pData[23]);			/**< ura (m->index) */

		pEphData->Tgd[0] = pData[25];					/**< TGD */
		pEphData->FitInterval = (unsigned char)pData[28];	/**< fit interval */

		pEphData->EphDataType = EPH_DATA_TYPE_NIC;
		*pRinexID = RINEX_ID_NICEPH;
	}
	else if (SystemID == RINEX_SYS_GAL) /**< GAL Ver.3 */
	{
		pEphData->IODE1 = (int)pData[3];				/**< IODnav */
		pEphData->TOE = (int)pData[11];					/**< toe (s) in galileo week */
		pEphData->Week = (unsigned short)pData[21] - 1024;		/**< gal week = gps week */
		pEphData->Health = (unsigned char)pData[24];	/**< sv health */
														/**< bit 0: E1B DVS */
														/**< bit 1-2: E1B HS */
														/**< bit  3: E5a DVS */
														/**< bit 4-5: E5a HS */
														/**< bit  6: E5b DVS */
														/**< bit 7-8: E5b HS */
		pEphData->URA = GetUraIndex(pData[23]);			/**< ura (m->index) */

		pEphData->Tgd[0] = pData[25];					/**< BGD E5a/E1 */
		pEphData->Tgd[1] = pData[26];					/**< BGD E5b/E1 */

		CalcEphInterimParam(WGS_SQRT_GM, WGS_OMEGDOTE, pEphData);

		if (Ver > 400)
		{
			if (RinexID ==RINEX_ID_GALEPH_INAV)
			{
				pEphData->EphDataType = EPH_DATA_TYPE_GAL_INAV;
			}
			else
			{
				pEphData->EphDataType = EPH_DATA_TYPE_GAL_FNAV;
			}
		}
		else
		{
			if (fabs(pEphData->Tgd[1]) > 0)
			{
				pEphData->EphDataType = EPH_DATA_TYPE_GAL_INAV;
				*pRinexID = RINEX_ID_GALEPH_INAV;
			}
			else
			{
				pEphData->EphDataType = EPH_DATA_TYPE_GAL_FNAV;
				*pRinexID = RINEX_ID_GALEPH_FNAV;
			}
		}
	}
	else if (SystemID == RINEX_SYS_BDS) /**< BeiDou v.3.02 */
	{
		pEphData->TOC = pEphData->TOC + 14; 			/**<gpst -> bdt*/
		pEphData->IODE1 = (unsigned short)pData[3];		/**< AODE */
		pEphData->IODC = (unsigned short)pData[28];		/**< AODC */
		pEphData->TOE = (int)pData[11];					/**< toe (s) in gps week */
		pEphData->Week = (unsigned short)pData[21];		/**< bdt week */

		pEphData->Health = (unsigned char)pData[24];	/**< satH1 */
		pEphData->URA = GetUraIndex(pData[23]);			/**< ura (m->index) */

		pEphData->Tgd[0] = pData[25];					/**< TGD1 B1/B3 */
		pEphData->Tgd[1] = pData[26];					/**< TGD2 B1/B3 */

		if (IS_BDS_GEO_PRN(pEphData->SatID))
		{
			CalcEphInterimParamGEO(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pEphData);
		}
		else
		{
			CalcEphInterimParam(CGCS2000_SQRT_GM, CGCS2000_OMEGDOTE, pEphData);
		}
		pEphData->EphDataType = EPH_DATA_TYPE_BDS_BD2;
		*pRinexID = RINEX_ID_BDSEPH;
	}

	pEphData->EphState = NAV_DATA_STATE_VALID;

	return 0;
}

/**********************************************************************//**
@brief  decode glonass ephemeris

@param Ver				[In] rinex file version
@param PRN				[In] gnss prn
@param pUTCTime			[In] UTC time
@param pData			[In] rinex file data
@param pGLOEphData		[Out] glonass eph data
@param Leap				[In] glonass leap year

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int DecodeGLOEph(int Ver, int PRN, UTC_TIME_T* pUTCTime, double *pData,
	GLO_EPHEMERIS_T *pGLOEphData, RM_HEADER_T* pHeader, int Leap)
{
	int TempWeek, WeekMsCount;
	double Tow, Tod, Tof;
	int Dow, SystemID = 0;
	int Tk_h, Tk_m, Tk_s;
	UTC_TIME_T TempUTCTime;
	int LeapYears, DayNumber, DayMsCount, GPSLeapSecMs;

	pGLOEphData->SatID = PRNToMSMSatID(RM_RANGE_SYS_GLO, PRN);

	pGLOEphData->TOE = UTC2Epoch(pUTCTime);
	UTCToGLOTime(pUTCTime, &LeapYears, &DayNumber, &DayMsCount);
	pGLOEphData->LeapYear = LeapYears;
	pGLOEphData->Nt = DayNumber;

	/** toc rounded by 15 min in utc */
	GPSLeapSecMs = GetGPSLeapSecondMs(pUTCTime);
	UTCToGPSTime(pUTCTime, &GPSLeapSecMs, &TempWeek, &WeekMsCount);

	pHeader->Week = TempWeek;
	pHeader->Milliseconds = WeekMsCount;

	Tow = WeekMsCount / 1000;
	Dow = (int)floor(Tow / 86400.0);

	Tof=QCTime2GPSTime(UTC2QCTime(pUTCTime), NULL); /**< v.3: tow in utc */
	pGLOEphData->Tb = ((int)(fmod(Tof + 10800.0, 86400.0) / 900.0 + 0.5))*900;

	/** time of frame in utc */
	Tod = Ver <= 299 ? pData[2] : fmod(pData[2], 86400.0); /**< tod (v.2), tow (v.3) in utc */

	GPSTimeToUTC(TempWeek, (int)((Tod + Dow * 86400) * 1000), &GPSLeapSecMs, &TempUTCTime);
	TempUTCTime = UTCAdd(&TempUTCTime, 18000);

	Tk_h = (int)(TempUTCTime.Hour);
	Tk_m = (int)(TempUTCTime.Minute);
	Tk_s = (int)(TempUTCTime.Second / 30);

	pGLOEphData->Tk = (Tk_h << 7) + (Tk_m << 1) + Tk_s;

	pGLOEphData->IODE = (int)(fmod(Tow + 10800.0, 86400.0) / 900.0 + 0.5); /**< iode = tb (7bit), tb =index of UTC+3H within current day */

	pGLOEphData->TauN = -pData[0];						/**< -taun */
	pGLOEphData->GammaN = pData[1];						/**< +gamman */

	pGLOEphData->PosXYZ[0] = pData[3] * 1E3;
	pGLOEphData->PosXYZ[1] = pData[7] * 1E3;
	pGLOEphData->PosXYZ[2] = pData[11] * 1E3;
	pGLOEphData->VelXYZ[0] = pData[4] * 1E3;
	pGLOEphData->VelXYZ[1] = pData[8] * 1E3;
	pGLOEphData->VelXYZ[2] = pData[12] * 1E3;
	pGLOEphData->AccXYZ[0] = pData[5] * 1E3;
	pGLOEphData->AccXYZ[1] = pData[9] * 1E3;
	pGLOEphData->AccXYZ[2] = pData[13] * 1E3;

	pGLOEphData->Bn = ((unsigned char)pData[6] << 2);
	pGLOEphData->Freq = (int)pData[10];
	pGLOEphData->En = (int)pData[14];

	if (Ver >= 305)
	{
		pGLOEphData->StatusFlag = (int)pData[15];
//		pGLOEphData->tgd = pData[16];
//		pGLOEphData->SVA = GetUraIndex(pData[17]);
//		pGLOEphData->svhflags = (int)pData[18];
	}

	if (pGLOEphData->Freq > 128) /**< some receiVer output >128 for minus frequency number */
	{
		pGLOEphData->Freq = pGLOEphData->Freq - 256;
	}

	if (pGLOEphData->Freq < MINFREQ_GLO || MAXFREQ_GLO < pGLOEphData->Freq)
	{
		return -1; /**< invalid freq */
	}

	pGLOEphData->EphDataType = EPH_DATA_TYPE_GLO;
	pGLOEphData->EphState = NAV_DATA_STATE_VALID;

	return 0;
}

/**********************************************************************//**
@brief  get rinex id

@param RinexSysID		[In] rniex system id
@param pSatType			[In] satellite type string

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int GetDecoderRinexID(int RinexSysID, char* pSatType)
{
	int RinexID = -1;

	switch (RinexSysID)
	{
		case RINEX_SYS_GPS:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, GPS_EPH_SAT_TYPE_LNAV, 4))
				{
					RinexID = RINEX_ID_GPSEPH;
				}
			}
			break;
		case RINEX_SYS_GLO:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, GLO_EPH_SAT_TYPE_FDMA, 4))
				{
					RinexID = RINEX_ID_GLOEPH;
				}
			}
			break;
		case RINEX_SYS_GAL:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, GAL_EPH_SAT_TYPE_INAV, 4))
				{
					RinexID = RINEX_ID_GALEPH_INAV;
				}
				else if (0 == MEMCMP(pSatType, GAL_EPH_SAT_TYPE_FNAV, 4))
				{
					RinexID = RINEX_ID_GALEPH_FNAV;
				}
			}
			break;
		case RINEX_SYS_QZS:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, QZS_EPH_SAT_TYPE_LNAV, 4))
				{
					RinexID = RINEX_ID_QZSSEPH;
				}
			}
			break;
		case RINEX_SYS_BDS:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, BDS_EPH_SAT_TYPE_CNAV1, 4))
				{
					RinexID = RINEX_ID_BDSEPH_CNAV1;
				}
				else if (0 == MEMCMP(pSatType, BDS_EPH_SAT_TYPE_CNAV2, 4))
				{
					RinexID = RINEX_ID_BDSEPH_CNAV2;
				}
				else if (0 == MEMCMP(pSatType, BDS_EPH_SAT_TYPE_CNAV3, 4))
				{
					RinexID = RINEX_ID_BDSEPH_CNAV3;
				}
			}
			else if (strlen(pSatType) >= 2)
			{
				if ((0 == MEMCMP(pSatType, BDS_EPH_SAT_TYPE_D1, 2)) ||
					(0 == MEMCMP(pSatType, BDS_EPH_SAT_TYPE_D2, 2)))
				{
					RinexID = RINEX_ID_BDSEPH;
				}
			}
			break;
		case RINEX_SYS_NIC:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, NIC_EPH_SAT_TYPE_LNAV, 4))
				{
					RinexID = RINEX_ID_NICEPH;
				}
			}
			break;
		case RINEX_SYS_SBA:
			if (strlen(pSatType) >= 4)
			{
				if (0 == MEMCMP(pSatType, SBA_EPH_SAT_TYPE_SBAS, 4))
				{
					RinexID = RINEX_ID_SBASEPH;
				}
			}
			break;
		default:
			break;
	}

	return RinexID;
}

/**********************************************************************//**
@brief  read rinex navigation data body

@param pFile			[In] input rniex file
@param FileTypeIndex	[In] file type index
@param Ver				[In] rinex file version
@param SystemID			[In] gnss system id
@param pTypeID			[In] file type id
@param pEphData			[Out] eph data
@param Leap				[In] leapsecs
@param pUTCTime			[In] UTC time
@param pRinexID			[Out] rinex id

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ReadAllRinexNavBody(FILE *pFile, int FileTypeIndex, int Ver, int* pTypeID, RINEX_INFO_T* pRinexInfo, UTC_TIME_T* pUTCTime, int* pRinexID)
{
	UTC_TIME_T UTCTime;
	double DataBuf[MAX_NUM_OBS_TYPE];
	unsigned int DataIndex = 0, StrIndex;
	int PRN, StrPos = 3;
	char TempBuf[MAXRNXLEN];
	char IDBuf[8] = "";
	char* pStr;
	int Result = -1;
	int RinexSysID = 0;
	int Leap;
	int DecodeFlag = 0;
	int DecodeCompleteFlag = 0;
	int RinexID = -1;
	CYCLE_BUFF_CTRL_T* pCycleBufCtrl = &pRinexInfo->CycleBufCtrl[FileTypeIndex];

	//while (fgets(TempBuf, MAXRNXLEN, pFile))
	while (GetRinexFileLineData(pFile, pCycleBufCtrl, TempBuf, MAXRNXLEN))
	{
		if (DataIndex == 0)
		{
			/** decode satellite field */
			if ((Ver >= 400) && (DecodeFlag == 0)) /**< Ver.4*/
			{
				if (0 != MEMCMP(TempBuf, ">EPH ", 5))
				{
					continue;
				}
				StrPos = 5;

				strncpy(IDBuf, TempBuf + StrPos, 3);
				Result = SatID2SatNo(IDBuf, &RinexSysID, &PRN);
				if (Result < 0)
				{
					continue;
				}
				StrPos += 4;

				RinexID = GetDecoderRinexID(RinexSysID, TempBuf + StrPos);
				if (RinexID < 0)
				{
					continue;
				}

				DecodeFlag = 1;
				continue;
			}

			if (Ver >= 300) /**< Ver.3*/
			{
				strncpy(IDBuf, TempBuf, 3);
				Result = SatID2SatNo(IDBuf, &RinexSysID, &PRN);
				if (Result < 0)
				{
					continue;
				}
				StrPos = 4;
			}
			else
			{
				strncpy(IDBuf, TempBuf, 3);
				PRN = (int)strtoul(IDBuf, NULL, 10);

				RinexSysID = RINEX_SYS_GPS;
			}

			/** decode toc field */
			if (EphStr2Time(TempBuf + StrPos, 0, 19, &UTCTime))
			{
				return 0;
			}

			*pUTCTime = UTCTime;

			/** decode DataBuf fields */
			for (StrIndex = 0, pStr = TempBuf + StrPos + 19; StrIndex < 3; StrIndex++, pStr += 19)
			{
				DataBuf[DataIndex++] = Str2Num(pStr, 0, 19);
			}
		}
		else
		{
			/** decode DataBuf fields */
			for (StrIndex = 0, pStr = TempBuf + StrPos; StrIndex < 4; StrIndex++, pStr += 19)
			{
				DataBuf[DataIndex++] = Str2Num(pStr, 0, 19);
			}

			/** decode ephemeris */
			if ((Ver >= 400) && (RinexSysID == RINEX_SYS_BDS))
			{
				if ((((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2)) && (DataIndex >= 39)) ||
					((RinexID == RINEX_ID_BDSEPH_CNAV3) && (DataIndex >= 35)) ||
					((RinexID == RINEX_ID_BDSEPH) && (DataIndex >= 31)))
				{
					Result = DecodeUniEphVer4BDS(RinexID, PRN, pUTCTime, DataBuf, &pRinexInfo->UniEphData[FileTypeIndex], &pRinexInfo->UniEphRMHeader[FileTypeIndex]);
					DecodeCompleteFlag = 1;
				}
			}
			else
			{
				if ((RinexSysID == RINEX_SYS_GLO) && (DataIndex >= 15))
				{
					RinexID = RINEX_ID_GLOEPH;
					Leap = pRinexInfo->GLOEphHeader.Leapsecs;
					Result = DecodeGLOEph(Ver, PRN, &UTCTime, DataBuf, &pRinexInfo->GLOEphData, &pRinexInfo->GLOEphRMHeader, Leap);
					DecodeCompleteFlag = 1;
				}
				else if (DataIndex >= 31)
				{
					Result = DecodeUniEph(Ver, RinexSysID, &RinexID, PRN, &UTCTime, DataBuf, &pRinexInfo->UniEphData[FileTypeIndex], &pRinexInfo->UniEphRMHeader[FileTypeIndex]);
					DecodeCompleteFlag = 1;
				}
			}

			if (DecodeCompleteFlag == 1)
			{
				*pTypeID = RinexID;
				break;
			}
		}
	}

	*pRinexID = RinexID;

	return Result;
}

/**********************************************************************//**
@brief  read rinex navigation data body

@param pFile			[In] input rniex file
@param Ver				[In] rinex file version
@param SystemID			[In] gnss system id
@param pTypeID			[In] file type id
@param pEphData			[Out] eph data
@param pHeader			[InOut] rawmsg eph header
@param Leap				[In] leapsecs
@param pUTCTime			[In] UTC time

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
static int ReadRinexNavBody(FILE *pFile, int Ver, int RinexSysID, int* pTypeID, void *pEphData, RM_HEADER_T* pHeader, int Leap,
									UTC_TIME_T* pUTCTime, int* pRinexID)
{
	UTC_TIME_T UTCTime;
	double DataBuf[MAX_NUM_OBS_TYPE];
	unsigned int DataIndex = 0, StrIndex;
	int PRN, StrPos = 3;
	char TempBuf[MAXRNXLEN];
	char IDBuf[8] = "";
	char* pStr;
	int Result = -1;
	int RinexID = -1;
	int DecodeFlag = 0;
	int DecodeCompleteFlag = 0;

	while (fgets(TempBuf, MAXRNXLEN, pFile))
	{
		if (DataIndex == 0)
		{
			/** decode satellite field */
			if ((Ver >= 400) && (DecodeFlag == 0)) /**< Ver.4*/
			{
				if (0 != MEMCMP(TempBuf, ">EPH ", 5))
				{
					continue;
				}
				StrPos = 5;

				strncpy(IDBuf, TempBuf + StrPos, 3);
				Result = SatID2SatNo(IDBuf, &RinexSysID, &PRN);
				if (Result < 0)
				{
					continue;
				}
				StrPos += 4;

				RinexID = GetDecoderRinexID(RinexSysID, TempBuf + StrPos);
				if (RinexID < 0)
				{
					continue;
				}

				DecodeFlag = 1;
				continue;
			}

			if (Ver >= 300) /**< Ver.3 */
			{
				strncpy(IDBuf, TempBuf, 3);
				Result = SatID2SatNo(IDBuf, &RinexSysID, &PRN);
				if (Result < 0)
				{
					continue;
				}
				StrPos = 4;
			}
			else
			{
				PRN = (int)strtoul(TempBuf, NULL, 10);

				if (RinexSysID == RINEX_SYS_SBA)
				{
					PRN += 100;
				}
				else if (93 <= PRN && PRN <= 97)
				{
					PRN += 100;/** extension */
				}
			}

			/** decode toc field */
			if (EphStr2Time(TempBuf + StrPos, 0, 19, &UTCTime))
			{
				return 0;
			}

			*pUTCTime = UTCTime;

			/** decode DataBuf fields */
			for (StrIndex = 0, pStr = TempBuf + StrPos + 19; StrIndex < 3; StrIndex++, pStr += 19)
			{
				DataBuf[DataIndex++] = Str2Num(pStr, 0, 19);
			}
		}
		else
		{
			/** decode DataBuf fields */
			for (StrIndex = 0, pStr = TempBuf + StrPos; StrIndex < 4; StrIndex++, pStr += 19)
			{
				DataBuf[DataIndex++] = Str2Num(pStr, 0, 19);
			}

			/** decode ephemeris */
			if ((Ver >= 400) && (RinexSysID == RINEX_SYS_BDS))
			{
				if ((((RinexID == RINEX_ID_BDSEPH_CNAV1) || (RinexID == RINEX_ID_BDSEPH_CNAV2)) && (DataIndex >= 39)) ||
					((RinexID == RINEX_ID_BDSEPH_CNAV3) && (DataIndex >= 35)) ||
					((RinexID == RINEX_ID_BDSEPH) && (DataIndex >= 31)))
				{
					Result = DecodeUniEphVer4BDS(RinexID, PRN, pUTCTime, DataBuf, pEphData, pHeader);
					DecodeCompleteFlag = 1;
				}
			}
			else
			{
				if (RinexSysID == RINEX_SYS_GLO && DataIndex >= 15)
				{
					RinexID = RINEX_ID_GLOEPH;
					Result = DecodeGLOEph(Ver, PRN, &UTCTime, DataBuf, pEphData, pHeader, Leap);
					DecodeCompleteFlag = 1;
				}
				else if (DataIndex >= 31)
				{
					Result = DecodeUniEph(Ver, RinexSysID, &RinexID, PRN, &UTCTime, DataBuf, pEphData, pHeader);
					DecodeCompleteFlag = 1;
				}
			}

			if (DecodeCompleteFlag == 1)
			{
				*pTypeID = RinexID;
				break;
			}
		}
	}

	*pRinexID = RinexID;

	return Result;
}

/**********************************************************************//**
@brief  open rinex information data

@param pRinexInfo		[In] rinex information struct
@param pFile			[In] input rniex file
@param FileIndex		[In] rinex file type index

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
int OpenRinexInfo(RINEX_INFO_T* pRinexInfo, FILE* pFile, unsigned int FileTypeIndex)
{
	const char* RinexTypes = "ONGLJHC";
	double Ver;
	char FileType, ObsType[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_NUM_OBS_TYPE][4] = { { "" } };
	unsigned int SysIndex, ObsTypeIndex;
	int SystemID, SysTimeType;
	int Result;

	/** read rinex header from file */
	Result = ReadRinexHeader(pFile, &Ver, &FileType, &SystemID, &SysTimeType, ObsType, pRinexInfo, FileTypeIndex);
	if (Result < 0)
	{
		return Result;
	}

	if (!strchr(RinexTypes, pRinexInfo->FileType[FileTypeIndex]))
	{
		return -1;
	}

	pRinexInfo->Ver = (int)(Ver * 100 + 0.5);
	pRinexInfo->SysTimeType = SysTimeType;

	if (FileType == 'O')
	{
		for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
		{
			for (ObsTypeIndex = 0; ObsTypeIndex < MAX_NUM_OBS_TYPE; ObsTypeIndex++)
			{
				strcpy(pRinexInfo->ObsType[SysIndex][ObsTypeIndex], ObsType[SysIndex][ObsTypeIndex]);
				strcpy(pRinexInfo->ObsHeaderInfo.ObsHeader.ObsType[SysIndex][ObsTypeIndex], ObsType[SysIndex][ObsTypeIndex]);
			}
		}
	}

	pRinexInfo->FileType[FileTypeIndex] = FileType;
	pRinexInfo->FileHeaderType[FileTypeIndex] = Result;
   
	return Result;
}

/**********************************************************************//**
@brief  input rinex information

@param pRinexInfo		[In] rinex information struct
@param pFile			[In] input rniex file
@param FileTypeIndex	[In] rinex file type index
@param pUTCTime			[In] UTC time

@retval <0 error, =0 success

@author CHC
@Date 2023/12/25
@note
**************************************************************************/
int InputRinexInfo(RINEX_INFO_T* pRinexInfo, FILE* pFile, unsigned int FileTypeIndex, UTC_TIME_T* pUTCTime, int* pRinexID)
{
	int SystemID, Flag, TypeID = 0;
	int Result = 0;
	double ClkOff = 0;

	/** read rinex obs data */
	if (pRinexInfo->FileHeaderType[FileTypeIndex] == RINEX_ID_OBS_HEADER)
	{
		if (ReadRinexObsBody(pRinexInfo, pFile, NULL, pRinexInfo->Ver, pRinexInfo->SysTimeType, pRinexInfo->ObsType, &Flag,
					&pRinexInfo->SatObsData, &pRinexInfo->ObsData, &ClkOff, pRinexInfo->ObsHeaderInfo.ObsHeader.Leapsecs) < 0)
		{
			return -1;
		}
		pRinexInfo->UTCTime = pRinexInfo->ObsData.ObsTime;
		*pUTCTime = pRinexInfo->ObsData.ObsTime;
		return RINEX_ID_OBS;
	}

	/** read rinex nav data */
	SystemID = RinexHeaderIDToRinexSystemMask(pRinexInfo->FileHeaderType[FileTypeIndex]);
	if (SystemID < 0)
	{
		return -1;
	}

//	if (SystemID == RINEX_SYS_ALL)
//	{
		if (Result = ReadAllRinexNavBody(pFile, FileTypeIndex, pRinexInfo->Ver, &TypeID, pRinexInfo, pUTCTime, pRinexID))
		{
			return Result < 0 ? -2 : 0;
		}
//	}
//	else if (SystemID == RINEX_SYS_GLO)
//	{
//		if ((Result = ReadRinexNavBody(pFile, pRinexInfo->Ver, SystemID, &TypeID, &pRinexInfo->GLOEphData,
//										&pRinexInfo->GLOEphRMHeader, pRinexInfo->GLOEphHeader.Leapsecs,
//										pUTCTime, pRinexID)) <= 0)
//		{
//			return Result < 0 ? -2 : 0;
//		}
//	}
//	else
//	{
//		if ((Result = ReadRinexNavBody(pFile, pRinexInfo->Ver, SystemID, &TypeID, &pRinexInfo->UniEphData[FileTypeIndex],
//										&pRinexInfo->UniEphRMHeader[FileTypeIndex], pRinexInfo->EphHeader[FileTypeIndex].Leapsecs,
//										pUTCTime, pRinexID)) <= 0)
//		{
//			return Result < 0 ? -2 : 0;
//		}
//	}

	return TypeID;
}

#endif
