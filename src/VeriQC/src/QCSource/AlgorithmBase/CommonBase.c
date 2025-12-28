/*************************************************************************//**
		   VeriQC
	Data Quality Check Module
 *-
@file   CommonBase.c
@author CHC
@date   5 September 2022
@brief  Some quality check algorithm common interfaces
*****************************************************************************/

#include <string.h>
#include <float.h>
#include "CommonBase.h"
#include "MatrixFuncs.h"
#include "TimeDefines.h"
#include "Coordinate.h"

static double g_FreCMP[6] = { QC_FREQ2_CMP,QC_FREQ7,     QC_FREQ6_CMP,QC_FREQ1, QC_FREQ5, QC_FREQ7};
static double g_FreGPS[5] = { QC_FREQ1 ,QC_FREQ2 ,QC_FREQ5,QC_FREQ1 ,QC_FREQ2 };
static double g_FreGAL[5] = { QC_FREQ1 ,QC_FREQ5 ,QC_FREQ7, QC_FREQ8, QC_FREQ6 };
static double g_FreQZS[6] = { QC_FREQ1 ,QC_FREQ2 ,QC_FREQ5, QC_FREQ1, QC_FREQ6,QC_FREQ1 };

FILE* fpLog = NULL;

extern void QCCloseLogfile()
{
	if (fpLog)
	{
		fclose(fpLog);
		fpLog = NULL;
	}
	else
	{
		
	}
}
extern void QCOpenLogfile(char* pFileName)
{
	if (fpLog)
	{
		fclose(fpLog);
		fpLog = NULL;
	}
	else
	{
	}
	fpLog = fopen(pFileName, "w");
}

extern int QCLogTrace(const char *pFormat, ...)
{
#ifdef WIN32
	char buff[1024] = { 0 };
	va_list ap;
	if (fpLog == NULL)
	{
		return 0;
	}
	else
	{
		__crt_va_start(ap, pFormat);
		vsnprintf(buff, 1024, pFormat, ap);
		__crt_va_end(ap);
		fprintf(fpLog, "%s\n", buff);
		return 1;
	}
#else 
	return 1;
#endif
}

extern int QCSatNo2Str(int SatNo, char* pStr)
{
	int prn, sys;
	int len = 0;

	sys = GetQCSatSys(SatNo, &prn);

	switch (sys)
	{
	case QC_SYS_GPS:len = sprintf(pStr, "G%02d", prn); break;
	case QC_SYS_SBS:len = sprintf(pStr, "S%02d", prn); break;
	case QC_SYS_GLO:len = sprintf(pStr, "R%02d", prn); break;
	case QC_SYS_GAL:len = sprintf(pStr, "E%02d", prn); break;
	case QC_SYS_QZS:len = sprintf(pStr, "J%02d", prn); break;
	case QC_SYS_CMP:len = sprintf(pStr, "C%02d", prn); break;
	default:
		break;
	}
	return len;
}

static int8_t* s_ObsCodes[] = {       /* observation code strings */

	""  ,"1C","1P","1W","1Y", "1M","1N","1S","1L","1E", /*  0- 9 */
	"1A","1B","1X","1Z","2C", "2D","2S","2L","2X","2P", /* 10-19 */
	"2W","2Y","2M","2N","5I", "5Q","5X","7I","7Q","7X", /* 20-29 */
	"6A","6B","6C","6X","6Z", "6S","6L","8L","8Q","8X", /* 30-39 */
	"2I","2Q","6I","6Q","3I", "3Q","3X","1I","1Q","5D", /* 40-49 */
	"5P","5Z","6E","7D","7P", "7Z","1D","8D","8P","6D", /* 50-59 */
	"6P","5A","5B","5C","9A", "9B","9C","9X"/* 60 */
};

static uint8_t s_ObsFreqs[] = { /* 1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,7:L3 8:L9 */

	0, 1, 1, 1, 1,  1, 1, 1, 1, 1,  /*  0- 9 */
	1, 1, 1, 1, 2,  2, 2, 2, 2, 2,  /* 10-19 */
	2, 2, 2, 2, 3,  3, 3, 5, 5, 5,  /* 20-29 */
	4, 4, 4, 4, 4,  4, 4, 6, 6, 6,  /* 30-39 */
	2, 2, 4, 4, 3,  3, 3, 1, 1, 3,  /* 40-49 */
	3, 3, 4, 5, 5,  5, 1, 6, 6, 4,  /* 50-59 */
	4, 3, 3, 3, 8,  8, 8, 8 /* 60 */
};

/* obs type string to obs code -------------------------------------------------
* convert obs code type string to obs code
* args   : int8_t   *pObs   I      obs code string ("1C","1P","1Y",...)
*          int32_t  *pFreq  IO     frequency (1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,0:err)
*                               (NULL: no output)
* return : obs code (CODE_???)
* notes  : obs codes are based on reference [6] and qzss extension
*-----------------------------------------------------------------------------*/
extern uint8_t QCObs2Code(const int8_t* pObs, int32_t* pFreq)
{
	int32_t i;
	if (pFreq) *pFreq = 0;
	//for (i=1;*obscodes[i];i++) {

	for (i = 1; i < QC_MAX_NUM_CODE; i++)
	{
		if (strcmp(s_ObsCodes[i], pObs)) continue;
		if (pFreq) *pFreq = s_ObsFreqs[i];
		return (uint8_t)i;
	}
	return QC_CODE_NONE;
}

/* sate1lite system +prn/slot number to satellite number--------------
*  convert satellite system + prn / slot number to satellite number
*  args :  int  Sys I    satellite system(QC_SYS_GPS, QC_SYS_GLO,...)
*          int  PRN I    sate1lite prn/slot number
*  return : sate1lite number(0 :error)
*------------------------------------------------------------------*/
extern int32_t GetQCSatNo(int32_t Sys, int32_t PRN)
{
	if (PRN <= 0)
	{
		return 0;
	}
	switch (Sys)
	{
	case QC_SYS_GPS:
		if (PRN < QC_MIN_PRN_GPS || QC_MAX_PRN_GPS < PRN)
		{
			return 0;
		}
		return PRN - QC_MIN_PRN_GPS + 1;
	case QC_SYS_GLO:
		if (PRN < QC_MIN_PRN_GLO || QC_MAX_PRN_GLO < PRN)
		{
			return 0;
		}
		return QC_NUM_SAT_GPS + PRN - QC_MIN_PRN_GLO + 1;
	case QC_SYS_GAL:
		if (PRN < QC_MIN_PRN_GAL || QC_MAX_PRN_GAL < PRN)
		{
			return 0;
		}
		return QC_NUM_SAT_GPS + QC_NUM_SAT_GLO + PRN - QC_MIN_PRN_GAL + 1;
	case QC_SYS_QZS:
		if (PRN < QC_MIN_PRN_QZS || QC_MAX_PRN_QZS < PRN)
		{
			return 0;
		}
		return QC_NUM_SAT_GPS + QC_NUM_SAT_GLO + QC_NUM_SAT_GAL + PRN - QC_MIN_PRN_QZS + 1;
	case QC_SYS_CMP:
		if (PRN < QC_MIN_PRN_CMP || QC_MAX_PRN_CMP < PRN)
		{
			return 0;
		}
		return QC_NUM_SAT_GPS + QC_NUM_SAT_GLO + QC_NUM_SAT_GAL + QC_NUM_SAT_QZS + PRN - QC_MIN_PRN_CMP + 1;
	case QC_SYS_SBS:
		if (PRN < QC_MIN_PRN_SBS || QC_MAX_PRN_SBS < PRN)
		{
			return 0;
		}
		return QC_NUM_SAT_GPS + QC_NUM_SAT_GLO + QC_NUM_SAT_GAL + QC_NUM_SAT_QZS + QC_NUM_SAT_CMP + PRN - QC_MIN_PRN_SBS + 1;
	default:
		return 0;
	}
}


/* satellite number to satellite system ----------------------------------------
* convert satellite number to satellite system
* args   : int32_t    Sat        I   satellite number (1-CGCODEC_MAX_NUM_SAT)
*          int32_t    *pPRN      IO  satellite prn/slot number (NULL: no output)
* return : satellite system (SYS_GPS,SYS_GLO,...)
*-----------------------------------------------------------------------------*/
extern int32_t GetQCSatSys(int32_t Sat, int32_t* pPRN)
{
	int32_t sys = QC_SYS_NONE;
	if (Sat <= 0 || QC_MAX_NUM_SAT < Sat)
	{
		Sat = 0;
	}
	else if (Sat <= QC_NUM_SAT_GPS)
	{
		if (Sat <= QC_MAX_PRN_GPS_TRUE)
		{
			sys = QC_SYS_GPS; Sat += QC_MIN_PRN_GPS - 1;
		}
		else
		{
			sys = QC_SYS_QZS; Sat -= QC_MAX_PRN_GPS_TRUE;
		}
	}
	else if ((Sat -= QC_NUM_SAT_GPS) <= QC_NUM_SAT_GLO)
	{
		sys = QC_SYS_GLO; Sat += QC_MIN_PRN_GLO - 1;
	}
	else if ((Sat -= QC_NUM_SAT_GLO) <= QC_NUM_SAT_GAL)
	{
		sys = QC_SYS_GAL; Sat += QC_MIN_PRN_GAL - 1;
	}
	else if ((Sat -= QC_NUM_SAT_GAL) <= QC_NUM_SAT_QZS)
	{
		sys = QC_SYS_QZS; Sat += QC_MIN_PRN_QZS - 1;
	}
	else if ((Sat -= QC_NUM_SAT_QZS) <= QC_NUM_SAT_CMP)
	{
		sys = QC_SYS_CMP; Sat += QC_MIN_PRN_CMP - 1;
	}
	else if ((Sat -= QC_NUM_SAT_CMP) <= QC_NUM_SAT_SBS)
	{
		sys = QC_SYS_SBS; Sat += QC_MIN_PRN_SBS - 1;
	}
	else
	{
		Sat = 0;
	}
	if (pPRN)
	{
		*pPRN = Sat;
	}
	return sys;
}

extern double QCSatAzEl(const double* pPos, const double* pE, double* pAzEl)
{
	double az = 0.0, el = PI / 2.0, enu[3];

	if (pPos[2] > -RE_WGS84)
	{
		ECEF2ENU(pPos, pE, enu);
		az = Dot(enu, enu, 2) < 1E-12 ? 0.0 : atan2(enu[0], enu[1]);
		if (az < 0.0) az += 2 * PI;
		el = asin(enu[2]);
	}
	if (pAzEl)
	{
		pAzEl[0] = az;
		pAzEl[1] = el;
	}
	return el;
}

extern int QCSys2Id(int Sys)
{
	int sysId = QC_GNSS_SYSID_NUL;
	switch (Sys)
	{
	case QC_SYS_GPS:sysId = QC_GNSS_SYSID_GPS; break;
	case QC_SYS_SBS:sysId = QC_GNSS_SYSID_SBAS; break;
	case QC_SYS_GLO:sysId = QC_GNSS_SYSID_GLN; break;
	case QC_SYS_GAL:sysId = QC_GNSS_SYSID_GAL; break;
	case QC_SYS_QZS:sysId = QC_GNSS_SYSID_QZS; break;
	case QC_SYS_CMP:sysId = QC_GNSS_SYSID_BDS; break;
	default:
		break;
	}
	return sysId;
}
extern int QCId2Sys(int Id)
{
	int sys = QC_SYS_NONE;
	switch (Id)
	{
	case QC_GNSS_SYSID_GPS :sys = QC_SYS_GPS; break;
	case QC_GNSS_SYSID_SBAS:sys = QC_SYS_SBS; break;
	case QC_GNSS_SYSID_GLN :sys = QC_SYS_GLO; break;
	case QC_GNSS_SYSID_GAL :sys = QC_SYS_GAL; break;
	case QC_GNSS_SYSID_QZS :sys = QC_SYS_QZS; break;
	case QC_GNSS_SYSID_BDS :sys = QC_SYS_CMP; break;
	default:
		break;
	}
	return sys;
}

extern int QCId2SysFreNum(int Id)
{
	int num = 0;
	switch (Id)
	{
	case QC_GNSS_SYSID_GPS :num = QC_GPS_FRE_NUM; break;
	case QC_GNSS_SYSID_SBAS:num = QC_SBAS_FRE_NUM; break;
	case QC_GNSS_SYSID_GLN :num = QC_GLONASS_FRE_NUM; break;
	case QC_GNSS_SYSID_GAL :num = QC_GALILEO_FRE_NUM; break;
	case QC_GNSS_SYSID_QZS :num = QC_QZSS_FRE_NUM; break;
	case QC_GNSS_SYSID_BDS :num = QC_BDS_FRE_NUM; break;
	default:
		break;
	}
	return num;
}

/* string to number ------------------------------------------------------------
* convert substring in string to number
* args   : int8_t     *pStr        I   string ("... nnn.nnn ...")
*          int32_t    Pos,Width       I   substring position and width
* return : converted number (0.0:error)
*-----------------------------------------------------------------------------*/
extern double QCStr2Num(const int8_t* pStr, int32_t Pos, int32_t Width)
{
	double value;
	int8_t str[256], * p = str;

	if (Pos < 0 || (int32_t)strlen(pStr) < Pos || (int32_t)sizeof(str) - 1 < Width) return 0.0;
	for (pStr += Pos; *pStr && --Width >= 0; pStr++) *p++ = *pStr == 'd' || *pStr == 'D' ? 'E' : *pStr; *p = '\0';
	value = atof(str);
	return value;
}

/*********************************************************************//**
@brief QC Library convert CODE_??? to freq value

@param   Code    [In]   signal type CODE_???
@param   FreGLO  [In]   freq number from eph

@return freq value(Hz)
*************************************************************************/
double QCCode2FreGLO(int Code, int FreGLO)
{
	double freq = 0, dfreq = 0;
	switch (Code)
	{
	case QC_CODE_L1C:
	case QC_CODE_L1P:
		/*G1*/
		freq = QC_FREQ1_GLO;
		dfreq = QC_DFRQ1_GLO;
		break;
	/*  G1a
	case QC_CODE_L4A:
	case QC_CODE_L4B:
	case QC_CODE_L4X:
	    break;
	*/
		
	case QC_CODE_L2C:
	case QC_CODE_L2P:
		freq = QC_FREQ2_GLO;
		dfreq = QC_DFRQ2_GLO;
		/*G2*/
		break;
	case QC_CODE_L6A:
	//case QC_CODE_L6B:
	//case QC_CODE_L6X:
		/*G2a*/
		freq = QC_FREQ2A_GLO;
		break;
	case QC_CODE_L3I:
	case QC_CODE_L3Q:
	case QC_CODE_L3X:
		freq = QC_FREQ3_GLO;
		/*G3*/
		break;
	default:
		break;
	}

	return (freq + dfreq * FreGLO);
}


/*********************************************************************//**
@brief QC Library convert CODE_??? to phase lam

@param   Code    [In]   signal type CODE_???
@param   FreGLO [In]   freq number from eph

@return lam(m)
*************************************************************************/
double QCCode2LamGLO(int Code, int FreGLO)
{
	return CLIGHT / QCCode2FreGLO(Code, FreGLO);
}

/*********************************************************************//**
@brief QC Library convert CODE_??? to freId

@param   Sys    [In]   system mask QC_SYS_???
@param   Code   [In]   signal type CODE_???

@return freId
@note
     GPS	    L1, L2C, L5,  L1C, L2P
     BDS	    B1, B2,  B3,  B1C,     B2a, B2b
     GLONASS	G1, G1a, G2,  G2a,     G3
     Galileo	E1, E5A, E5B, E5(A+B), E6
     QZSS	    L1, L2,  L5,  L5S,     L6, L1C(P), L1S
     SBAS	    L1, L5
*************************************************************************/
extern int QCCode2FreID(int Sys, int Code)
{
	int FreID = -1;
	if (Sys == QC_SYS_GPS)
	{
		switch (Code)
		{
		case QC_CODE_L1L:
		case QC_CODE_L1S:
		case QC_CODE_L1X:FreID = 3; break;    /**< "L1" in RINEX define */
		case QC_CODE_L1C:
		case QC_CODE_L1P:
		case QC_CODE_L1W:
		case QC_CODE_L1N:FreID = 0; break;    /**< "L1" in RINEX define */
		case QC_CODE_L2S:
		case QC_CODE_L2L:
		case QC_CODE_L2X:FreID = 1; break;    /**< "L2" in RINEX define */
		case QC_CODE_L2C:
		case QC_CODE_L2D:
		case QC_CODE_L2P:
		case QC_CODE_L2W:
		case QC_CODE_L2N:FreID = 4; break;    /**< "L2" in RINEX define */
		case QC_CODE_L5I:
		case QC_CODE_L5Q:
		case QC_CODE_L5X:FreID = 2; break;    /**< "L5" in RINEX define */
		default:
			break;
		}
	}
	else if (Sys == QC_SYS_CMP)
	{
		switch (Code)
		{
		case QC_CODE_L1Q:
		case QC_CODE_L1I:
		case QC_CODE_L2I:
		case QC_CODE_L2Q:
		case QC_CODE_L2X:FreID = 0; break;/**< "B1" in RINEX define */
		case QC_CODE_L1D:
		case QC_CODE_L1P:
		case QC_CODE_L1X:
		case QC_CODE_L1A:
		case QC_CODE_L1N:FreID = 3; break;/**< "B1C" in RINEX define */
		case QC_CODE_L5D:
		case QC_CODE_L5P:
		case QC_CODE_L5X:
			/*B2a*/
			FreID = 4; break;
		case QC_CODE_L7I:
		case QC_CODE_L7Q:
		case QC_CODE_L7X:
			/*B2*/
			FreID = 1; break;
		case QC_CODE_L7D:
		case QC_CODE_L7P:
		case QC_CODE_L7Z:
			/*B2b*/
			FreID = 5; break;
		case QC_CODE_L6I:
		case QC_CODE_L6Q:
		case QC_CODE_L6X:
		case QC_CODE_L6A:
			/*B3*/
			FreID = 2; break;
		default:
			break;
		}
	}
	else if (Sys == QC_SYS_GAL)
	{
		switch (Code)
		{
		case QC_CODE_L1B:
		case QC_CODE_L1C:
		case QC_CODE_L1X:
			/*E1*/
			FreID = 0; break;
		case QC_CODE_L5I:
		case QC_CODE_L5Q:
		case QC_CODE_L5X:
			/*E5A*/
			FreID = 1; break;
		case QC_CODE_L7I:
		case QC_CODE_L7Q:
		case QC_CODE_L7X:
			/*E5B*/
			FreID = 2; break;
		case QC_CODE_L8I:
		case QC_CODE_L8Q:
		case QC_CODE_L8X:
			/*E5(A+B)*/
			FreID = 3; break;
		case QC_CODE_L6B:
		case QC_CODE_L6C:
		case QC_CODE_L6X:
			/*E6*/
			FreID = 4; break;
		default:
			break;
		}
	}
	else if (Sys == QC_SYS_QZS)
	{
		switch (Code)
		{
		case QC_CODE_L1C:
		case QC_CODE_L1S:
		case QC_CODE_L1X:
			/*L1*/
			FreID = 0; break;
		case QC_CODE_L1L:
			/** L1C(P) */
			FreID = 3; break;
		case QC_CODE_L2S:
		case QC_CODE_L2L:
		case QC_CODE_L2X:
			/*L2*/
			FreID = 1; break;
		case QC_CODE_L5I:
		case QC_CODE_L5Q:
		case QC_CODE_L5X:
			/*L5*/
			FreID = 2; break;
		case QC_CODE_L6S:
		case QC_CODE_L6L:
		case QC_CODE_L6X:
		case QC_CODE_L6E:
		case QC_CODE_L6Z:
			/*L6*/
			FreID = 4; break;
		/** Reserved L1C/B FreID = 5 */
		default:
			break;
		}
	}
	else if (Sys == QC_SYS_GLO)
	{
	switch (Code)
	{
		case QC_CODE_L1C:
		case QC_CODE_L1P:
			/*G1*/
			FreID = 0;
			break;
		case QC_CODE_L2C:
		case QC_CODE_L2P:
			FreID = 1;
			/*G2*/
			break;
		default:
			break;
		}
	}
	return FreID;
}
/** get wavelength from FreID */
double QCGetLam(int Sys, int FreID)
{
	if (FreID < 0)
	{
		return FreID;
	}
	else
	{
		switch (Sys)
		{
		case QC_SYS_GPS:
			return CLIGHT / g_FreGPS[FreID];
			break;
		case QC_SYS_CMP:
			return CLIGHT / g_FreCMP[FreID];
			break;
		case QC_SYS_GAL:
			return CLIGHT / g_FreGAL[FreID];
			break;
		case QC_SYS_QZS:
			return CLIGHT / g_FreQZS[FreID];
			break;
		default:
			return FreID;
			break;
		}
	}
}

/****************************************//**
@brief QcLib functon: Get freq value(Hz) of target freq type

@param Sys       [In]   sys eg, SYS_GPS
@param Code      [In]   code type eg.CODEC_L1C

@return freq value(Hz)

@author CHC
*******************************************/
double QCCode2Fre(int Sys, int Code)
{
	int FreID = -1;
	
	FreID = QCCode2FreID(Sys, Code);
	if(FreID<0)
	{
		return FreID;
	}
	else
	{
		switch (Sys)
		{
		case QC_SYS_GPS:
			return g_FreGPS[FreID];
			break;
		case QC_SYS_CMP:
			return g_FreCMP[FreID];
			break;
		case QC_SYS_GAL:
			return g_FreGAL[FreID];
			break;
		case QC_SYS_QZS:
			return g_FreQZS[FreID];
			break;
		default:
			return FreID;
			break;
		}
	}
}
/****************************************//**
@brief QcLib functon: Get dualfreq combination type

@param pObsd    [In]    input observation struction for QcLib
@param pFreq    [Out]   dualfreq combination{freq0,freq1}

@return 1:ok 0: error

@author CHC
*******************************************/
int GetCombType(QC_OBSD_T* pObsd,int* pFreq)
{
	int iFre;
	int fre0 = -1, fre1 = -1;
	for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
	{
		if ((pObsd->L[iFre] != 0))
		{
			fre0 = iFre;
			break;
		}
	}
	if (fre0 > -1)
	{
		for (iFre = fre0 + 1; iFre < QC_NUM_FREQ; iFre++)
		{
			if ((pObsd->L[iFre] != 0))
			{
				fre1 = iFre;
				break;
			}
		}
	}

	if (fre0 > -1 && fre1 > -1)
	{
		pFreq[0] = fre0;
		pFreq[1] = fre1;
		return 1;
	}
	else
	{
		return 0;
	}
}

/******************************************//**
@brief MW Calculate

@param pObsd       [In] obsveration of one satellite
@param Fre0Index   [In] Freq 0 Index
@param Fre1Index   [In] Freq 1 Index
@param Fre0        [In] Freq 0 frequency, Hz
@param Fre1        [In] Freq 1 frequency, Hz

@return MW

@author CHC
@date 2025/02/11
@note
History:\n
**************************************************/
double CalculateMW(QC_OBSD_T* pObsd, int Fre0Index, int Fre1Index, double Fre0, double Fre1)
{
	double Lam0, Lam1;
	double NW;

	Lam0 = CLIGHT / Fre0;
	Lam1 = CLIGHT / Fre1;

	if (fabs(pObsd->P[Fre0Index] - pObsd->P[Fre1Index]) > 100)
	{
		NW = 0;
	}
	else
	{
		NW = pObsd->L[Fre0Index] - pObsd->L[Fre1Index] -
			(Fre0 - Fre1) / (Fre0 + Fre1) * (pObsd->P[Fre0Index] / Lam0 + pObsd->P[Fre1Index] / Lam1);
	}

	return - NW;
}

/******************************************//**
@brief LWL Calculate

@param pObsd       [In] obsveration of one satellite
@param Fre0Index   [In] Freq 0 Index
@param Fre1Index   [In] Freq 1 Index
@param Fre0        [In] Freq 0 frequency, Hz
@param Fre1        [In] Freq 1 frequency, Hz

@return LWL

@author CHC
@date 2025/02/11
@note
History:\n
**************************************************/
double CalculateLWL(QC_OBSD_T* pObsd, int Fre0Index, int Fre1Index, double Fre0, double Fre1)
{
	double Lam0, Lam1;
	double LWL;

	Lam0 = CLIGHT / Fre0;
	Lam1 = CLIGHT / Fre1;

	LWL = (Fre0 * Lam0 * pObsd->L[Fre0Index] - Fre1 * Lam1 * pObsd->L[Fre1Index]) / (Fre0 - Fre1);

	return LWL;
}

/******************************************//**
@brief NL Calculate

@param pObsd       [In] obsveration of one satellite
@param Fre0Index   [In] Freq 0 Index
@param Fre1Index   [In] Freq 1 Index
@param Fre0        [In] Freq 0 frequency, Hz
@param Fre1        [In] Freq 1 frequency, Hz

@return NL

@author CHC
@date 2025/02/11
@note
History:\n
**************************************************/
double CalculateNL(QC_OBSD_T* pObsd, int Fre0Index, int Fre1Index, double Fre0, double Fre1)
{
	double Lam0, Lam1;
	double LNL;

	Lam0 = CLIGHT / Fre0;
	Lam1 = CLIGHT / Fre1;

	LNL = (2 * Fre0 * Lam0 * pObsd->L[Fre0Index] - Fre1 * Lam1 * pObsd->L[Fre1Index]) / (2 * Fre0 - Fre1);

	return LNL;
}

/******************************************//**
@brief GF Calculate

@param pObsd       [In] obsveration of one satellite
@param Fre0Index   [In] Freq 0 Index
@param Fre1Index   [In] Freq 1 Index
@param Fre0        [In] Freq 0 frequency, Hz
@param Fre1        [In] Freq 1 frequency, Hz

@return GF

@author CHC
@date 2025/02/11
@note
History:\n
**************************************************/
double CalculateGF(QC_OBSD_T* pObsd, int Fre0Index, int Fre1Index, double Fre0, double Fre1)
{
	return (pObsd->L[Fre0Index] / Fre0 - pObsd->L[Fre1Index] / Fre1) * CLIGHT;
}

/* get mp index ---------------------
* QC_OBSD_T* [I]       pObsd
* int        [O]       Freq0
* int*       [IO]      pCombi
* return MP
*------------------------------------*/
double GetRawMP(QC_OBSD_T* pObsd, int Freq0, int* pCombi)
{
	int iFre, sys, frq, f1 = -1;
	double MP = 0.0,fre0, fre1, lam0, lam1;

	sys = pObsd->Sys;
	frq = pObsd->GLOFreq;
	if (sys == QC_SYS_GLO)
	{
		fre0 = QCCode2FreGLO(pObsd->Code[Freq0], frq);
	}
	else
	{
		fre0 = QCCode2Fre(sys, pObsd->Code[Freq0]);
	}
	for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
	{
		if (iFre == Freq0)
		{
			continue;
		}
		if (sys == QC_SYS_GLO)
		{
			fre1 = QCCode2FreGLO(pObsd->Code[iFre], frq);
		}
		else
		{
			fre1 = QCCode2Fre(sys, pObsd->Code[iFre]);
		}

		if (fabs(fre1 - fre0) < 50000000)
		{
			continue;
		}

		if (pObsd->L[iFre] > 0 && pObsd->P[iFre] > 0 && fre0 != fre1)
		{
			f1 = iFre;
			*pCombi = pObsd->Code[Freq0] * 1000 + pObsd->Code[iFre];
			break;
		}
	}
	if (f1 == -1)
	{
		*pCombi = 0;
		return 0;
	}
	
	lam0 = CLIGHT / fre0;
	lam1 = CLIGHT / fre1;
	MP = pObsd->P[Freq0] -lam0 * pObsd->L[Freq0] +
		(2 * QC_SQR(lam0) / (QC_SQR(lam0) - QC_SQR(lam1)))
		* (lam0 * pObsd->L[Freq0] - lam1 * pObsd->L[f1]);
	return MP;
}

/**********************************************//**
@brief Identify Mp Slip

@param MPwins   [In]
@param MpWinSize[In]
@param CurRawMp [In]

@return BOOL
****************************************************/
static BOOL CheckMpSlip(double* pMPwins, int MpWinSize, double CurRawMp)
{
	BOOL bMpSlip = FALSE;
	double meanMp = 0;
	double resAbsMp;
	int iWins;

	if (MpWinSize < 1)
	{
		return bMpSlip;
	}
	else
	{
		for (iWins = 0; iWins < MpWinSize; iWins++)
		{
			meanMp += pMPwins[iWins];
		}
		meanMp = (meanMp + CurRawMp) / (MpWinSize + 1.0);
		resAbsMp = fabs(CurRawMp - meanMp);
		if (resAbsMp > 9.0)
		{
			bMpSlip = TRUE;
		}
	}

	return bMpSlip;
}

/******************************************//**
@brief Process MP related procedures

@param pObsd     obsveration of one satellite
@param pQcRes    QC result structure
@param AntID     ant id same as VERIQC

@return 1:yes slip 0:no slip
@note
**************************************************/
BOOL MpProcessorFixed(QC_OBSD_T* pObsd, QC_RESULT_T* pQcRes, int AntID, int SatNo, int* bGfSlip, QC_TIME_T ObsTime)
{
	int iFre;
	int iWins;
	BOOL bCurFreqMpSlip = 0;
	BOOL bMPSlip = FALSE;
	double rawMP = 0, curMP = 0, sumMP, lastrawMP;
	int MP_Comb = 0;
	char tstr[64] = { 0 };
	char msg[1000] = { 0 };
	double DeltaTime = 0;

	for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
	{
		if ((pObsd->L[iFre] != 0) && (pObsd->P[iFre] != 0))
		{
			DeltaTime = QCTimeDiff(&ObsTime, &pQcRes->MPTime[AntID][SatNo][iFre]);

			rawMP = GetRawMP(pObsd, iFre, &MP_Comb);

			if (MP_Comb == 0)
			{
				continue;
			}

			lastrawMP = pQcRes->MPWins[AntID][SatNo][iFre][0];

			/** if MP combine changes, reset MP window */
			if (pQcRes->MpComb[AntID][SatNo][iFre] != MP_Comb)
			{
				/** Clear MP windows */
				pQcRes->MPWinsNum[AntID][SatNo][iFre] = 0;
			}

			/** Check MP Slip */
			bCurFreqMpSlip = CheckMpSlip(pQcRes->MPWins[AntID][SatNo][iFre], pQcRes->MPWinsNum[AntID][SatNo][iFre], rawMP);
			bMPSlip |= bCurFreqMpSlip;

			/** when MP Slip and GF Slip, reset MP window */
			if ((pQcRes->MaxEpochNum[AntID] > 1) && (pQcRes->MpComb[AntID][SatNo][iFre] > 0)
				&& ((pQcRes->MpComb[AntID][SatNo][iFre] != MP_Comb) || bGfSlip[iFre] || bCurFreqMpSlip || (fabs(DeltaTime - pQcRes->Interval[AntID]) > FLT_EPSILON)))
			{
				/** Clear MP windows */
				memset(pQcRes->MPWins[AntID][SatNo][iFre], 0, sizeof(double) * QC_WIN_SIZE_MP);
				pQcRes->MPWinsNum[AntID][SatNo][iFre] = 0;
			}

			if (pQcRes->MPWinsNum[AntID][SatNo][iFre] < QC_WIN_SIZE_MP)
			{
				for (iWins = pQcRes->MPWinsNum[AntID][SatNo][iFre]; iWins > 0; iWins--)
				{
					if (iWins > 0)
					{
						pQcRes->MPWins[AntID][SatNo][iFre][iWins] = pQcRes->MPWins[AntID][SatNo][iFre][iWins - 1];
					}
				}
				pQcRes->MPWinsNum[AntID][SatNo][iFre]++;
			}
			else
			{
				for (iWins = QC_WIN_SIZE_MP - 1; iWins > 0; iWins--)
				{
					pQcRes->MPWins[AntID][SatNo][iFre][iWins] = pQcRes->MPWins[AntID][SatNo][iFre][iWins - 1];
				}
				pQcRes->MPWinsNum[AntID][SatNo][iFre] = QC_WIN_SIZE_MP;
			}

			pQcRes->MPWins[AntID][SatNo][iFre][0] = rawMP;
			sumMP = 0;
			for (iWins = 0; iWins < pQcRes->MPWinsNum[AntID][SatNo][iFre]; iWins++)
			{
				sumMP += pQcRes->MPWins[AntID][SatNo][iFre][iWins];
			}
			if (pQcRes->MPWinsNum[AntID][SatNo][iFre] != 0)
			{
				curMP = rawMP - sumMP / pQcRes->MPWinsNum[AntID][SatNo][iFre];
			}
			else
			{
				curMP = 0.0;
			}
			pQcRes->MpComb[AntID][SatNo][iFre] = MP_Comb;
			pQcRes->MP[AntID][SatNo][iFre] += QC_SQR(curMP);
			pQcRes->CurMP[AntID][SatNo][iFre] = curMP;
			pQcRes->MPNum[AntID][SatNo][iFre]++;

			pQcRes->MPTime[AntID][SatNo][iFre] = ObsTime;
			pQcRes->CurMPValidMask[AntID][SatNo] |= (1 << iFre);
		}
	}

	return bMPSlip;
}

/******************************************//**
@brief Process MP related procedures, filter

@param pObsd      [In]      obsveration of one satellite
@param pQcRes     [In/Out]  QC result structure
@param AntID      [In]      ant id same as VERIQC
@param SatNo      [In]      satellite index
@param pSlipFlag  [In]      cycle slip flag
@param ObsTime    [In]      obsveration time

@return 1:yes slip 0:no slip
@note
**************************************************/
BOOL MpProcessorFilter(QC_OBSD_T* pObsd, QC_RESULT_T* pQcRes, int AntID, int SatNo, int* pSlipFlag, QC_TIME_T ObsTime)
{
	int iFre;
	BOOL bCurFreqMpSlip = FALSE;
	BOOL bMPSlip = FALSE;
	double RawMP = 0;
	double CurMP = 0;
	double DeltaTime = 0;
	double MPMean;
	double ResAbsMp;
	int MPComb = 0;

	for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
	{
		if (pObsd->L[iFre] != 0 && pObsd->P[iFre] != 0)
		{
			DeltaTime = QCTimeDiff(&ObsTime, &pQcRes->MPTime[AntID][SatNo][iFre]);

			RawMP = GetRawMP(pObsd, iFre, &MPComb);

			if (MPComb == 0)
			{
				continue;
			}

			/** if MP combine changes, reset MP window */
			if (pQcRes->MpComb[AntID][SatNo][iFre] != MPComb)
			{
				/** Clear MP windows */
				pQcRes->MPWinsNum[AntID][SatNo][iFre] = 0;
			}

			/** Check MP Slip */
			bCurFreqMpSlip = FALSE;
			if (pQcRes->MPWinsNum[AntID][SatNo][iFre] < 1)
			{
				MPMean = RawMP;
			}
			else
			{
				MPMean = 0.05 * RawMP + 0.95 * pQcRes->MPWins[AntID][SatNo][iFre][0]; /**< mp(t) = 0.05 * rawMP(t) + 0.95 * mp(t-1) */
				ResAbsMp = fabs(RawMP - MPMean);
				if (ResAbsMp > 9.0)
				{
					bCurFreqMpSlip = TRUE;
				}
			}
			bMPSlip |= bCurFreqMpSlip;

			/** when MP Slip and Cycle Slip, reset MP window */
			if ((pQcRes->MaxEpochNum[AntID] > 1) && (pQcRes->MpComb[AntID][SatNo][iFre] > 0)
				&& ((pQcRes->MpComb[AntID][SatNo][iFre] != MPComb) || pSlipFlag[iFre] || bCurFreqMpSlip || (fabs(DeltaTime - pQcRes->Interval[AntID]) > FLT_EPSILON)))
			{
				/** Clear MP windows */
				pQcRes->MPWins[AntID][SatNo][iFre][0] = RawMP;
				pQcRes->MPWinsNum[AntID][SatNo][iFre] = 1;
				CurMP = 0;
			}
			else
			{
				pQcRes->MPWins[AntID][SatNo][iFre][0] = MPMean;
				pQcRes->MPWinsNum[AntID][SatNo][iFre]++;
				CurMP = RawMP - MPMean;
			}

			pQcRes->MpComb[AntID][SatNo][iFre] = MPComb;
			pQcRes->MP[AntID][SatNo][iFre] += QC_SQR(CurMP);
			pQcRes->CurMP[AntID][SatNo][iFre] = CurMP;
			pQcRes->MPNum[AntID][SatNo][iFre]++;

			pQcRes->MPTime[AntID][SatNo][iFre] = ObsTime;
			pQcRes->CurMPValidMask[AntID][SatNo] |= (1 << iFre);
		}
	}

	return bMPSlip;
}

#define QC_FREQ_MAX_SIG_FOR_SLIP    4

/** structure for slip detect */
typedef struct _SORT_FREQ_GROUP_T
{
	int SortedFreIndex;

	int GroupSigNum;
	int RawSigIndex[QC_FREQ_MAX_SIG_FOR_SLIP];
}SORT_FREQ_GROUP_T;

typedef struct _SLIP_FREQ_COMB_INFO_T
{
	unsigned int CombNum;
	unsigned int BDSNLShiftFlag;
	int Freq1[QC_FREQ_MAX_SIG_FOR_SLIP];
	int Freq2[QC_FREQ_MAX_SIG_FOR_SLIP];
	double SlipValue[QC_FREQ_MAX_SIG_FOR_SLIP];
}SLIP_FREQ_COMB_INFO_T;

/******************************************//**
@brief sort SORT_FREQ_MAP_T

@param pPairA      [In]     SORT_FREQ_MAP_T 1
@param pPairB      [In]     SORT_FREQ_MAP_T 2

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
int SortFreqCompare(const void* pPairA, const void* pPairB)
{
	SORT_FREQ_GROUP_T* pOne = (SORT_FREQ_GROUP_T*)pPairA;
	SORT_FREQ_GROUP_T* pTwo = (SORT_FREQ_GROUP_T*)pPairB;

	return (pOne->SortedFreIndex - pTwo->SortedFreIndex);
}

/******************************************//**
@brief find Freq Combination MW/LWL diff

@param pSaveVSlip  [In]     Combination Slip info
@param Fre1Index   [In]     frequency 1 index
@param Fre2Index   [In]     frequency 2 index

@return slip value

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
double FindSlip(SLIP_FREQ_COMB_INFO_T* pSaveVSlip, int Fre1Index, int Fre2Index)
{
	unsigned int iLoop = 0;

	for (iLoop = 0; iLoop < pSaveVSlip->CombNum; iLoop++)
	{
		if ((pSaveVSlip->Freq1[iLoop] == Fre1Index) && (pSaveVSlip->Freq2[iLoop] == Fre2Index))
		{
			return pSaveVSlip->SlipValue[iLoop];
		}
	}

	return 0;
}

/******************************************//**
@brief add all slip info to SLIP_FREQ_COMB_INFO_T

@param pSaveVSlip  [In]     Combination Slip info
@param Fre1Index   [In]     frequency 1 index
@param Fre2Index   [In]     frequency 2 index
@param WLSlp       [In]     slip value

@return 0

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
int AddAllSlipGroupInfo(SLIP_FREQ_COMB_INFO_T* pSaveVSlip, int Fre1Index, int Fre2Index, double WLSlp)
{
	if (pSaveVSlip->CombNum >= QC_FREQ_MAX_SIG_FOR_SLIP)
	{
		return -1;
	}
	pSaveVSlip->Freq1[pSaveVSlip->CombNum] = Fre1Index;
	pSaveVSlip->Freq2[pSaveVSlip->CombNum] = Fre2Index;
	pSaveVSlip->SlipValue[pSaveVSlip->CombNum] = WLSlp;
	pSaveVSlip->CombNum++;

	return 0;
}

/******************************************//**
@brief record each combination slip info only once

@param pSaveVSlip  [In]     Combination Slip info
@param Fre1Index   [In]     frequency 1 index
@param Fre2Index   [In]     frequency 2 index
@param WLSlp       [In]     slip value

@return 0

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
int AddOnceSlipGroupInfo(SLIP_FREQ_COMB_INFO_T* pSaveVSlip, int Fre1Index, int Fre2Index, double WLSlp)
{
	BOOL bExit = FALSE;
	unsigned int iLoop = 0;

	if (pSaveVSlip->CombNum >= QC_FREQ_MAX_SIG_FOR_SLIP)
	{
		return -1;
	}

	for (iLoop = 0; iLoop < pSaveVSlip->CombNum; iLoop++)
	{
		if ((pSaveVSlip->Freq1[iLoop] == Fre1Index) && (pSaveVSlip->Freq2[iLoop] == Fre2Index))
		{
			pSaveVSlip->SlipValue[iLoop] = WLSlp;
			bExit = TRUE;
			break;
		}
	}

	if (!bExit)
	{
		pSaveVSlip->Freq1[pSaveVSlip->CombNum] = Fre1Index;
		pSaveVSlip->Freq2[pSaveVSlip->CombNum] = Fre2Index;
		pSaveVSlip->SlipValue[pSaveVSlip->CombNum] = WLSlp;
		pSaveVSlip->CombNum++;
	}

	return 0;
}

/******************************************//**
@brief Get Sorted Freq Group for cycle slip

@param pSortedGroup    [Out]    Sorted Freq Group info
@param pObsdPre        [In]     previous observation
@param pObsdCur        [In]     current observation
@param pQcRes          [Out]    Quality check result
@param AntID           [In]     ant id same
@param SatNo           [In]     Sat Number
@param pFreProcessFlag [Out]    if Freq processed, bit set 0

@return previous and current Same Fre Num

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
int GetSortedFreqGroup(SORT_FREQ_GROUP_T* pSortedGroup, QC_OBSD_T* pObsdPre, QC_OBSD_T* pObsdCur, QC_RESULT_T* pQcRes, int AntID, int SatNo, unsigned char* pFreProcessFlag)
{
	int iFre = 0;
	int Result = 0;
	int Sys = 0;
	int SysIndex = 0;
	int iLoop = 0;
	int SortGroupIndex = -1;
	int SameFreNum = 0;

	/** sorted Frequency according to lamda */
	const int(*SortedFreqIndexTable) = NULL;
	const static int GPSSortedFreqIndex[QC_MAX_NUM_SYS + 1][QC_NUM_FREQ] =
	{
		[QC_GNSS_SYSID_NUL]  = {-1, -1, -1, -1, -1, -1},
		[QC_GNSS_SYSID_GPS]  = { 0,  1,  2,  0,  1, -1},
		[QC_GNSS_SYSID_GLN]  = { 0,  1, -1, -1, -1, -1},
		[QC_GNSS_SYSID_GAL]  = { 0,  4,  2,  3,  1, -1},
		[QC_GNSS_SYSID_QZS]  = { 0,  2,  3,  0,  1, -1},
		[QC_GNSS_SYSID_BDS]  = { 1,  3,  2,  0,  4,  3},
		[QC_GNSS_SYSID_SBAS] = {-1, -1, -1, -1, -1, -1},
	};

	Sys = pObsdPre->Sys;
	SysIndex = QCSys2Id(Sys);

	if (SysIndex == QC_GNSS_SYSID_NUL)
	{
		return -1;
	}

	SortedFreqIndexTable = GPSSortedFreqIndex[SysIndex];

	if (SortedFreqIndexTable == NULL)
	{
		return -1;
	}

	/** check same frequency */
	for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
	{
		if ((fabs(pObsdCur->L[iFre]) > DBL_EPSILON) && (fabs(pObsdCur->P[iFre]) > DBL_EPSILON))
		{
			if ((fabs(pObsdPre->L[iFre]) > DBL_EPSILON) && (fabs(pObsdPre->P[iFre]) > DBL_EPSILON))
			{
				if (SortedFreqIndexTable[iFre] == -1)
				{
					continue;
				}

				if (pObsdCur->Code[iFre] != pObsdPre->Code[iFre])
				{
					continue;
				}

				SortGroupIndex = -1;
				for (iLoop = 0; iLoop < SameFreNum; iLoop++)
				{
					if (pSortedGroup[iLoop].SortedFreIndex == SortedFreqIndexTable[iFre])
					{
						SortGroupIndex = iLoop;
						break;
					}
				}

				if (SortGroupIndex == -1)
				{
					pSortedGroup[SameFreNum].RawSigIndex[0] = iFre;
					pSortedGroup[SameFreNum].GroupSigNum++;
					pSortedGroup[SameFreNum].SortedFreIndex = SortedFreqIndexTable[iFre];
					SameFreNum++;
				}
				else
				{
					pSortedGroup[SortGroupIndex].RawSigIndex[pSortedGroup[SortGroupIndex].GroupSigNum++] = iFre;
				}
				pQcRes->DetectNum[AntID][SatNo][iFre]++;
				pQcRes->SysDetectNum[AntID][SysIndex - 1]++;

				*pFreProcessFlag &= ~(1 << iFre);
			}
		}
	}

	return SameFreNum;
}

/******************************************//**
@brief check if target value  exists in Int array

@param pIntArray     [In]     Int Array
@param ArraySize     [In]     Int Array size
@param TargetNumber  [In]     target value

@return Index or -1

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
int FindIntArraySameNumber(int* pIntArray, int ArraySize, int TargetVal)
{
	int iLoop = 0;

	for (iLoop = 0; iLoop < ArraySize; iLoop++)
	{
		if (pIntArray[iLoop] == TargetVal)
		{
			return iLoop;
		}
	}

	return -1;
}

/******************************************//**
@brief transform slip combination info to single frequency

@param pSaveVSlip      [In]     Combination Slip info
@param SaveSlipNum     [In]     Combination Slip Count
@param pSortFreqGroup  [In]     sorted frequency info
@param SameFreNum      [In]     sorted frequency count
@param pQcRes          [Out]    Quality check result
@param AntID         [In]     ant id same as VERIQC
@param SatNo         [In]     Sat Number

@author CHC
@date 2025/06/18
@note
History:\n
**************************************************/
void Transform(SLIP_FREQ_COMB_INFO_T* pSaveVSlip, int SaveSlipNum, SORT_FREQ_GROUP_T* pSortFreqGroup, int SameFreNum, QC_RESULT_T* pQcRes, int AntID, int SatNo)
{
	unsigned int MaxIter = 1;
	double* pMatrix = NULL;
	double* pSlip = NULL;
	double* pO = NULL;
	BOOL bTrans = FALSE;
	int MOriginExit = -1;

	int MOrigin[QC_NUM_FREQ] = { 0 };
	int MOriginNum = 0;
	unsigned int iLoopR = 0;
	unsigned int iLoopC = 0;

	unsigned int WLSlipIndex[QC_NUM_FREQ] = { 0 };
	unsigned int NLSlipIndex = 0;
	unsigned int Temp = 1;
	
	int iLoop = 0;
	int MOriginLoop = 0;
	int RawOrignFreqIndex = -1;
	int iFreq = 0;
	int MapLoop = 0;
	int SysIndex = QCSys2Id(GetQCSatSys((SatNo + 1), NULL)) - 1;

	for (iLoop = 0; iLoop < SaveSlipNum; iLoop++)
	{
		if (pSaveVSlip[iLoop].CombNum > MaxIter)
		{
			MaxIter = pSaveVSlip[iLoop].CombNum;
		}
	}

	if (((SameFreNum + QC_NUM_FREQ) > SaveSlipNum) || (SameFreNum > QC_NUM_FREQ))
	{
		return;/**< error */
	}

	pMatrix = (double*)VERIQC_MALLOC(sizeof(double) * SameFreNum * SameFreNum);
	pSlip = (double*)VERIQC_MALLOC(sizeof(double) * SameFreNum);
	pO = (double*)VERIQC_MALLOC(sizeof(double) * SameFreNum);

	for (Temp = 1; Temp <= MaxIter; Temp++)
	{
		/** Init */
		memset(pSlip, 0, sizeof(double) * SameFreNum);
		memset(pO, 0, sizeof(double) * SameFreNum);
		memset(pMatrix, 0, sizeof(double) * SameFreNum * SameFreNum);

		bTrans = FALSE;
		memset(MOrigin, 0, sizeof(int) * QC_NUM_FREQ);
		MOriginNum = 0;

		for (iLoop = 1; iLoop < SameFreNum; iLoop++)
		{
			/** find Narr-Lane slip */
			if (NLSlipIndex < pSaveVSlip[iLoop + QC_NUM_FREQ].CombNum)
			{
				pSlip[SameFreNum - 1] = -pSaveVSlip[iLoop + QC_NUM_FREQ].SlipValue[NLSlipIndex];
				if (fabs(pSlip[SameFreNum - 1]) > DBL_EPSILON)
				{
					bTrans = TRUE;
				}

				MOriginExit = FindIntArraySameNumber(MOrigin, MOriginNum, pSaveVSlip[iLoop + QC_NUM_FREQ].Freq1[NLSlipIndex]);
				if (MOriginExit == -1)
				{
					MOrigin[MOriginNum++] = pSaveVSlip[iLoop + QC_NUM_FREQ].Freq1[NLSlipIndex];
				}

				MOriginExit = FindIntArraySameNumber(MOrigin, MOriginNum, pSaveVSlip[iLoop + QC_NUM_FREQ].Freq2[NLSlipIndex]);
				if (MOriginExit == -1)
				{
					MOrigin[MOriginNum++] = pSaveVSlip[iLoop + QC_NUM_FREQ].Freq2[NLSlipIndex];
				}

				if ((pSaveVSlip[iLoop + QC_NUM_FREQ].CombNum - 1) > NLSlipIndex)
				{
					NLSlipIndex++;
				}

				pMatrix[(SameFreNum - 1) * SameFreNum + 0] = 2;
				if (pSaveVSlip[iLoop + QC_NUM_FREQ].BDSNLShiftFlag)
				{
					pMatrix[(SameFreNum - 1) * SameFreNum + 2] = -1;
				}
				else
				{
					pMatrix[(SameFreNum - 1) * SameFreNum + 1] = -1;
				}
			}

			/** find Wide-Lane slip */
			if ((WLSlipIndex[iLoop] < pSaveVSlip[iLoop].CombNum) && (WLSlipIndex[iLoop] < QC_FREQ_MAX_SIG_FOR_SLIP))
			{
				pMatrix[(iLoop - 1) * SameFreNum + SameFreNum - 1] = -1;
				pMatrix[(iLoop - 1) * SameFreNum + SameFreNum - iLoop - 1] = 1;
				pSlip[iLoop - 1] = -pSaveVSlip[iLoop].SlipValue[WLSlipIndex[iLoop]];

				MOriginExit = FindIntArraySameNumber(MOrigin, MOriginNum, pSaveVSlip[iLoop].Freq1[WLSlipIndex[iLoop]]);
				if (MOriginExit == -1)
				{
					MOrigin[MOriginNum++] = pSaveVSlip[iLoop].Freq1[WLSlipIndex[iLoop]];
				}

				MOriginExit = FindIntArraySameNumber(MOrigin, MOriginNum, pSaveVSlip[iLoop].Freq2[WLSlipIndex[iLoop]]);
				if (MOriginExit == -1)
				{
					MOrigin[MOriginNum++] = pSaveVSlip[iLoop].Freq2[WLSlipIndex[iLoop]];
				}

				if ((pSaveVSlip[iLoop].CombNum - 1) > WLSlipIndex[iLoop])
				{
					WLSlipIndex[iLoop]++;
				}

				if (fabs(pSlip[SameFreNum - iLoop - 1]) > DBL_EPSILON)
				{
					bTrans = TRUE;
				}
			}
		}

		if (bTrans)
		{
			if (MatInv(pMatrix, SameFreNum) == -1)
			{
				return;
			}
			MatMul(MAT_MUL_TYPE_TT, SameFreNum, 1, SameFreNum, 1.0, pMatrix, pSlip, 0, pO);

			/** save slip info */
			for (iFreq = 0; iFreq < SameFreNum; iFreq++)
			{
				if (pO[iFreq] != 0)
				{
					RawOrignFreqIndex = -1;
					for (MapLoop = 0; MapLoop < pSortFreqGroup[iFreq].GroupSigNum; MapLoop++)
					{
						MOriginExit = FindIntArraySameNumber(MOrigin, MOriginNum, pSortFreqGroup[iFreq].RawSigIndex[MapLoop]);
						if (MOriginExit != -1)
						{
							RawOrignFreqIndex = pSortFreqGroup[iFreq].RawSigIndex[MapLoop];
						}
					}

					if ((RawOrignFreqIndex != -1) && (pQcRes->SlipFlag[AntID][SatNo][RawOrignFreqIndex] != TRUE))
					{
						pQcRes->SlipNum[AntID][SatNo][RawOrignFreqIndex]++;
						pQcRes->SysSlipNum[AntID][SysIndex]++;
						pQcRes->SlipFlag[AntID][SatNo][RawOrignFreqIndex] = TRUE;
					}
				}
			}
		}
	}

	VERIQC_FREE(pMatrix);
	VERIQC_FREE(pSlip);
	VERIQC_FREE(pO);
}

/** struct save sat last epoch observation info */
typedef struct _QC_SLIP_SAT_OBVERSION_INFO_T
{
	QC_TIME_T Time;
	QC_OBSD_T SatPreFreObs;

	char ObsUpdateFlag;
	char Reserved[7];
}QC_SLIP_SAT_OBVERSION_INFO_T;

/** Note: only save one file info now !!! */
static QC_SLIP_SAT_OBVERSION_INFO_T s_SatPreFreObs[QC_MAX_NUM_SAT][QC_NUM_FREQ] = { 0 };

/** reset s_SatPreFreObs after QC */
void ResetQCSlipInfo()
{
	memset(s_SatPreFreObs, 0, sizeof(s_SatPreFreObs));
}

/******************************************//**
@brief detect cycle slip entrance

@param pObsdCur    [In]     current observation
@param pQcRes      [In/Out] QC result structure
@param AntID       [In]     ant id same as VERIQC
@param SatNo       [In]     Sat Number
@param pObsTime    [In]     obsveration time

@return  0   success

@author CHC
@date 2025/08/28
@note
History:\n
**************************************************/
void SlipProcessEntrance(QC_OBSD_T* pObsdCur, QC_RESULT_T* pQcRes, int AntID, int SatNo, QC_TIME_T* pObsTime)
{
	int iLoop;
	int PreSatObsIndex = -1;
	int RemainFreNum = -1;
	unsigned char FreProcessFlag = 0;
	QC_TIME_T LastTime = { 0 };
	QC_SLIP_SAT_OBVERSION_INFO_T* pPreSatObsInfo = &s_SatPreFreObs[SatNo][0];
	QC_OBSD_T* pObsdPre = NULL;

	/** Find newest previous obs */
	for (iLoop = 0; iLoop < QC_NUM_FREQ; iLoop++)
	{
		if ((QCTimeDiff(&pPreSatObsInfo->Time, &LastTime) > 0) && pPreSatObsInfo->ObsUpdateFlag && 
			(QCTimeDiff(pObsTime, &pPreSatObsInfo->Time) > 0))
		{
			memcpy(&LastTime, &pPreSatObsInfo->Time, sizeof(QC_TIME_T));
			PreSatObsIndex = iLoop;
		}

		if ((fabs(pObsdCur->L[iLoop]) > DBL_EPSILON) && (fabs(pObsdCur->P[iLoop]) > DBL_EPSILON))
		{
			FreProcessFlag |= (1 << iLoop);
		}

		pPreSatObsInfo++;
	}

	if (PreSatObsIndex != -1)
	{
		pObsdPre = &s_SatPreFreObs[SatNo][PreSatObsIndex].SatPreFreObs;

		/** Find Current and previous observation same frequency and record frequency previous not have */
		/** First Slip Process */
		QCSlipProcessor(pObsdPre, pObsdCur, pQcRes, AntID, SatNo, pObsTime, &FreProcessFlag);

		/** Loop process */
		for (iLoop = 0; iLoop < QC_NUM_FREQ; iLoop++)
		{
			if (!(FreProcessFlag & (1 << iLoop)))
			{
				continue;
			}
			pPreSatObsInfo = &s_SatPreFreObs[SatNo][iLoop];

			if (!pPreSatObsInfo->ObsUpdateFlag || (pPreSatObsInfo->Time.Time == 0))
			{
				continue;
			}
			pObsdPre = &s_SatPreFreObs[SatNo][iLoop].SatPreFreObs;

			QCSlipProcessor(pObsdPre, pObsdCur, pQcRes, AntID, SatNo, pObsTime, &FreProcessFlag);
		}
	}

	/** update pre observation info */
	for (iLoop = 0; iLoop < QC_NUM_FREQ; iLoop++)
	{
		if ((fabs(pObsdCur->L[iLoop]) > DBL_EPSILON) && (fabs(pObsdCur->P[iLoop]) > DBL_EPSILON))
		{
			memcpy(&s_SatPreFreObs[SatNo][iLoop].SatPreFreObs, pObsdCur, sizeof(QC_OBSD_T));
			memcpy(&s_SatPreFreObs[SatNo][iLoop].Time, pObsTime, sizeof(QC_TIME_T));
			s_SatPreFreObs[SatNo][iLoop].ObsUpdateFlag = 1;
		}
	}
}

/******************************************//**
@brief detect cycle slip

@param pObsdPre         [In]     previous observation
@param pObsdCur         [In]     current observation
@param pQcRes           [In/Out] QC result structure
@param AntID            [In]     ant id
@param SatNo            [In]     Sat Number
@param pObsTime         [In]     obsveration time
@param pFreProcessFlag  [Out]    if Freq processed, bit set 0

@return  0   success

@author CHC
@date 2025/02/11
@note
History:\n
**************************************************/
int QCSlipProcessor(QC_OBSD_T* pObsdPre, QC_OBSD_T* pObsdCur, QC_RESULT_T* pQcRes, int AntID, int SatNo, QC_TIME_T* pObsTime, unsigned char* pFreProcessFlag)
{
	int Sys = 0;
	int GLOFrqNum = 0;
	unsigned int SameFreNum = 0;
	unsigned int iLoop = 1;
	double Diff = 0;
	double Lct1 = 0;
	double Lct2 = 0;
	double DeltaWL = 0;
	double DeltaNL = 0;
	double DeltaEWL = 0;
	double Lam = 0;
	double NLam = 0;
	double ELam = 0;
	double WLSlp = 0;
	double NLSlp = 0;
	double EWLSlp = 0;
	double IsfWl = 0;
	double IsfNl = 0;
	double Disf = 0;
	double K = 0;
	double DeltaIon = 0;

	BOOL bSlip = FALSE;
	unsigned int Fre1GroupIndex = 0;
	unsigned int Fre2GroupIndex = 0;
	unsigned int FreNLGroupIndex = 0;
	unsigned int FreFFGroupIndex = 0;
	double Fre1 = 0;  /**< Hz */
	double Fre2 = 0;  /**< Hz */
	double FreNL = 0; /**< Hz */
	double FreFF = 0; /**< Hz */
	SORT_FREQ_GROUP_T SortFreqGroup[QC_NUM_FREQ] = { 0 };

	int Fre1Index = 0; /**< MW */
	int Fre1Iter = 0;  /**< fre1 iterator */
	int Fre2Index = 0; /**< MW */
	int Fre2Iter = 0;  /**< fre2 iterator */
	int FreNLIndex = 0;/**< NL */
	int FreNLIter = 0; /**< freNL iterator */
	int FreFFIndex = 0;/**< EWL */
	int FreFFIter = 0; /**< freFF iterator */
	int TempFre1Index;

	unsigned int VSlipNum = 0;
	SLIP_FREQ_COMB_INFO_T SaveVSlip[2 * QC_NUM_FREQ + 1] = { 0 }; /**< save all combination info, include repeated combination */
	SLIP_FREQ_COMB_INFO_T OnceVSlip[2 * QC_NUM_FREQ + 1] = { 0 }; /**< record each combination only once, later entries overwriting earlier ones */

	static double s_SatIon[QC_MAX_NUM_SAT] = { 0 };

	BOOL bEndNL = TRUE;
	BOOL bEndFF = TRUE;

	int Week;
	double Sec = QCTime2GPSTime(*pObsTime, &Week);

	Sys = pObsdPre->Sys;
	GLOFrqNum = pObsdPre->GLOFreq;

	/** check same frequency */
	SameFreNum = GetSortedFreqGroup(SortFreqGroup, pObsdPre, pObsdCur, pQcRes, AntID, SatNo, pFreProcessFlag);
	if (SameFreNum < 2)
	{
		return -1;
	}
	qsort(SortFreqGroup, SameFreNum, sizeof(SORT_FREQ_GROUP_T), SortFreqCompare);

	/** Init */
	FreNLGroupIndex = 1;
	/** adjust BDS Freq Combination, avoid B1C-B1I NL */
	if ((Sys == QC_SYS_CMP) && (SortFreqGroup[1].RawSigIndex[0] == 0) && (SortFreqGroup[0].RawSigIndex[0] == 3))
	{
		if (SameFreNum < 3)
		{
			return -1; /**< if only B1C and B1I ? */
		}
		FreNLGroupIndex = 2;
	}

	FreNLIndex = SortFreqGroup[FreNLGroupIndex].RawSigIndex[0];
	Fre2GroupIndex = SameFreNum - 1;

	for (iLoop = 1; iLoop < SameFreNum; iLoop++)
	{
		/** update Fre1 group */
		Fre1GroupIndex = SameFreNum - 1 - iLoop;

		/** find freq for extra-wide-lane */
		if (iLoop > 1)
		{
			FreFFGroupIndex = Fre1GroupIndex + 1;
			FreFFIter = 0;
			FreFFIndex = SortFreqGroup[FreFFGroupIndex].RawSigIndex[FreFFIter];
		}

		FreNLIter = 0;
		Fre2Iter = 0;
		bEndNL = TRUE;
		bEndFF = TRUE;

		for (Fre1Iter = 0; ; )/**< iterator */
		{
			Fre1Index = SortFreqGroup[Fre1GroupIndex].RawSigIndex[Fre1Iter];
			Fre2Index = SortFreqGroup[Fre2GroupIndex].RawSigIndex[Fre2Iter];

			if (Sys == QC_SYS_GLO)
			{
				Fre1 = QCCode2FreGLO(pObsdPre->Code[Fre1Index], GLOFrqNum);
				Fre2 = QCCode2FreGLO(pObsdPre->Code[Fre2Index], GLOFrqNum);
				FreNL = QCCode2FreGLO(pObsdPre->Code[FreNLIndex], GLOFrqNum);
				if (iLoop > 1)
				{
					FreFF = QCCode2FreGLO(pObsdPre->Code[FreFFIndex], GLOFrqNum);
				}
			}
			else
			{
				Fre1 = QCCode2Fre(Sys, pObsdPre->Code[Fre1Index]);
				Fre2 = QCCode2Fre(Sys, pObsdPre->Code[Fre2Index]);
				FreNL = QCCode2Fre(Sys, pObsdPre->Code[FreNLIndex]);
				if (iLoop > 1)
				{
					FreFF = QCCode2Fre(Sys, pObsdPre->Code[FreFFIndex]);
				}
			}

			Diff = 0;
			Lam = CLIGHT / (Fre1 - Fre2);
			WLSlp = 0;

			if (iLoop == 1)
			{
				Lct1 = CalculateMW(pObsdPre, Fre1Index, Fre2Index, Fre1, Fre2);
				Lct2 = CalculateMW(pObsdCur, Fre1Index, Fre2Index, Fre1, Fre2);
				Diff = Lct2 - Lct1;

				WLSlp = 0;
				if (fabs(Diff) > 2)
				{
					bSlip = TRUE;
					WLSlp = round(Diff);
				}

#ifdef _DEBUG
				QCLogTrace("iLoop1,MW,%d,%.2f,%d,%d,%d,%d,%.5f", Week, Sec, Sys, pObsdPre->PRN, Fre1Index, Fre2Index, Diff);
#endif

				AddAllSlipGroupInfo(&SaveVSlip[iLoop], Fre1Index, Fre2Index, WLSlp);
				AddOnceSlipGroupInfo(&OnceVSlip[iLoop], Fre1Index, Fre2Index, WLSlp);
				VSlipNum++;
			}
			else
			{
				Lct1 = CalculateLWL(pObsdPre, Fre1Index, Fre2Index, Fre1, Fre2);
				Lct2 = CalculateLWL(pObsdCur, Fre1Index, Fre2Index, Fre1, Fre2);
				DeltaWL = Lct2 - Lct1;

				Lct1 = CalculateLWL(pObsdPre, FreFFIndex, Fre2Index, FreFF, Fre2);
				Lct2 = CalculateLWL(pObsdCur, FreFFIndex, Fre2Index, FreFF, Fre2);
				DeltaEWL = Lct2 - Lct1;

				EWLSlp = FindSlip(&OnceVSlip[iLoop - 1], FreFFIndex, Fre2Index);
				ELam = CLIGHT / (FreFF - Fre2);
				Diff = (DeltaEWL - DeltaWL + EWLSlp * ELam) / Lam;

				WLSlp = 0;
				if (fabs(Diff) > 2)
				{
					bSlip = TRUE;
					WLSlp = round(Diff);
				}

#ifdef _DEBUG
				QCLogTrace("iLoop%d,LWL,%d,%.2f,%d,%d,%d,%d,%.5f", iLoop, Week, Sec, Sys, pObsdPre->PRN, Fre1Index, Fre2Index, Diff);
#endif

				AddAllSlipGroupInfo(&SaveVSlip[iLoop], Fre1Index, Fre2Index, WLSlp);
				AddOnceSlipGroupInfo(&OnceVSlip[iLoop], Fre1Index, Fre2Index, WLSlp);
				VSlipNum++;
			}

			/** narrow - lane slip */
			if (iLoop == (SameFreNum - 1))
			{
				Lct1 = CalculateLWL(pObsdPre, Fre1Index, Fre2Index, Fre1, Fre2);
				Lct2 = CalculateLWL(pObsdCur, Fre1Index, Fre2Index, Fre1, Fre2);
				DeltaWL = Lct2 - Lct1;

				/** find wide-lane slip from last cascade */
				WLSlp = FindSlip(&OnceVSlip[iLoop], Fre1Index, Fre2Index);

				TempFre1Index = Fre1Index;
				if ((Sys == QC_SYS_CMP) && (SortFreqGroup[1].RawSigIndex[0] == 0) && (SortFreqGroup[0].RawSigIndex[0] == 3))
				{
					if (SameFreNum == 3)
					{
						Fre1Index = 0;
						FreNLIndex = SortFreqGroup[2].RawSigIndex[0];
						OnceVSlip[iLoop + QC_NUM_FREQ].BDSNLShiftFlag = 1;
						SaveVSlip[iLoop + QC_NUM_FREQ].BDSNLShiftFlag = 1;
					}
					
					Fre1 = QCCode2Fre(Sys, pObsdPre->Code[Fre1Index]);
					FreNL = QCCode2Fre(Sys, pObsdPre->Code[FreNLIndex]);
				}

				Lct1 = CalculateNL(pObsdPre, Fre1Index, FreNLIndex, Fre1, FreNL);
				Lct2 = CalculateNL(pObsdCur, Fre1Index, FreNLIndex, Fre1, FreNL);
				DeltaNL = Lct2 - Lct1;
				NLam = CLIGHT / (2 * Fre1 - FreNL);

				/** get Iono */
				IsfWl = QC_SQR(Fre1) * (1.0 / Fre1 - 1.0 / FreNL) / (Fre1 - FreNL);
				IsfNl = QC_SQR(Fre1) * (2.0 / Fre1 - 1.0 / FreNL) / (2 * Fre1 - FreNL);
				Disf = IsfWl - IsfNl;
				K = QC_SQR(Fre1) / QC_SQR(FreNL);
				DeltaIon = 0;
				if (!bSlip)
				{
					DeltaIon = -(pObsdCur->L[Fre1Index] * CLIGHT / Fre1 - pObsdCur->L[FreNLIndex] * CLIGHT / FreNL -
						pObsdPre->L[Fre1Index] * CLIGHT / Fre1 + pObsdPre->L[FreNLIndex] * CLIGHT / FreNL) / (K - 1);
					s_SatIon[SatNo] = DeltaIon;
				}

				Diff = (DeltaWL - DeltaNL - Disf * s_SatIon[SatNo] + WLSlp * Lam) / NLam;

				NLSlp = 0;
				if (fabs(Diff) > 2)
				{
					NLSlp = round(Diff);
				}
#ifdef _DEBUG
				QCLogTrace("iLoop%d,NL,%d,%.2f,%d,%d,%d,%d,%.5f", iLoop, Week, Sec, Sys, pObsdPre->PRN, Fre1Index, FreNLIndex, Diff);
#endif

				Fre1Index = TempFre1Index;
				AddAllSlipGroupInfo(&SaveVSlip[iLoop + QC_NUM_FREQ], Fre1Index, Fre2Index, NLSlp);
				AddOnceSlipGroupInfo(&OnceVSlip[iLoop + QC_NUM_FREQ], Fre1Index, Fre2Index, NLSlp);
				VSlipNum++;
			}

			/** Iter signal */
			if (iLoop > 1) /**< Iter FF */
			{
				if (FreFFIter != SortFreqGroup[FreFFGroupIndex].GroupSigNum)
				{
					FreFFIter++;
				}
				if (FreFFIter == SortFreqGroup[FreFFGroupIndex].GroupSigNum)
				{
					bEndFF = TRUE;
				}
				else
				{
					bEndFF = FALSE;
				}
			}
			if (iLoop == (SameFreNum - 1)) /**< Iter NL */
			{
				if (FreNLIter != SortFreqGroup[FreNLGroupIndex].GroupSigNum)
				{
					FreNLIter++;
				}
				if (FreNLIter == SortFreqGroup[FreNLGroupIndex].GroupSigNum)
				{
					bEndNL = TRUE;
				}
				else
				{
					bEndNL = FALSE;
				}
			}

			if (Fre1Iter != SortFreqGroup[Fre1GroupIndex].GroupSigNum) /**< Iter F1 */
			{
				++Fre1Iter;
			}
			if (Fre2Iter != SortFreqGroup[Fre2GroupIndex].GroupSigNum) /**< Iter F2 */
			{
				++Fre2Iter;
			}
			if ((Fre1Iter == SortFreqGroup[Fre1GroupIndex].GroupSigNum) &&
				(Fre2Iter == SortFreqGroup[Fre2GroupIndex].GroupSigNum) &&
				bEndNL && bEndFF)
			{
				break;
			}
			if (Fre1Iter == SortFreqGroup[Fre1GroupIndex].GroupSigNum)
			{
				--Fre1Iter;
			}
			if (Fre2Iter == SortFreqGroup[Fre2GroupIndex].GroupSigNum)
			{
				--Fre2Iter;
			}
			if (iLoop > 1 && bEndFF)
			{
				--FreFFIter;
			}
			if (iLoop == SameFreNum - 1 && bEndNL)
			{
				--FreNLIter;
			}
			/** update index */
			Fre1Index = SortFreqGroup[Fre1GroupIndex].RawSigIndex[Fre1Iter];
			Fre2Index = SortFreqGroup[Fre2GroupIndex].RawSigIndex[Fre2Iter];
			FreNLIndex = SortFreqGroup[FreNLGroupIndex].RawSigIndex[FreNLIter];
			FreFFIndex = SortFreqGroup[FreFFGroupIndex].RawSigIndex[FreFFIter];
		}
	}

	/** transform and save slip info */
	if (VSlipNum <= 1)
	{
		return -1;
	}

	Transform(SaveVSlip, (2 * QC_NUM_FREQ + 1), SortFreqGroup, SameFreNum, pQcRes, AntID, SatNo);

	return 0;
}
