
#include "GNSSBase.h"
#include <string.h>
#include "Logs.h"
#include "Common/GNSSNavDataType.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"
#include "Coordinate.h"

#ifndef MIN
#define MIN(x,y)    ((x)<=(y)?(x):(y))
#endif
#define VERIQC_MAX_INSSTAT_MAP          6

#define SPP_SQR(x)      ((x) * (x))             /**< square */

static char *s_CHCObsCodes[]={       /* observation code strings */

    ""  ,"1C","1P","1W","1Y","1M","1N","1S","1L","1E", /*  0- 9 */
    "1A","1B","1X","1Z","2C","2D","2S","2L","2X","2P", /* 10-19 */
    "2W","2Y","2M","2N","5I","5Q","5X","7I","7Q","7X", /* 20-29 */
    "6A","6B","6C","6X","6Z","6S","6L","8I","8Q","8X", /* 30-39 */
    "2I","2Q","6I","6Q","3I","3Q","3X","7C","1I","1Q", /* 40-49 */
    "5D","5P","5Z","6E","7D","7P","7Z","1D","8D","8P","" /* 50-60 */
};
static unsigned char s_CHCObsFreqs[]={ /* 1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,7:L3 */

    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  0- 9 */
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, /* 10-19 */
    2, 2, 2, 2, 3, 3, 3, 5, 5, 5, /* 20-29 */
    4, 4, 4, 4, 4, 4, 4, 6, 6, 6, /* 30-39 */
    2, 2, 4, 4, 3, 3, 3, 5, 1, 1, /* 40-49 */
    3, 3, 3, 4, 5, 5, 5, 1, 6, 6, 0  /* 50-60 */
};

static char s_GPSCodePris[2][3][16]={  /* code priority table */

    {"CPYWMNSL","PYWCMNDSLX","IQX"}, /* GPS-code */
    {"CPYWMNSL","PYWCMNDSLX","IQX"}, /* GPS-phase */
};

/** number of range obs signal convert to VERIQC code code and frequency map */
#define RANGE_SIGNAL_MAP_NUM_GPS     22
#define RANGE_SIGNAL_MAP_NUM_GLO     22
#define RANGE_SIGNAL_MAP_NUM_GAL     22
#define RANGE_SIGNAL_MAP_NUM_QZS     29
#define RANGE_SIGNAL_MAP_NUM_SBS     15
#define RANGE_SIGNAL_MAP_NUM_BDS     29
#define RANGE_SYSTEM_MAP_NUM         8
#define EPHEMERIS_SYSTEM_MAP_NUM     8

static char *VeriQCCode2Obs(unsigned char Code, int *pFreq) 
{
    if (pFreq) *pFreq=0;
    if (Code<=-1||60<Code) return "";
    if (pFreq) *pFreq=s_CHCObsFreqs[Code];
    return s_CHCObsCodes[Code];
}
static int VeriQCGetGPSPri(unsigned char Code, unsigned char ObsType) 
{
    const char *p;
    char *obs;
    int i=0;

    obs=VeriQCCode2Obs(Code,&i);
    if (i>3||ObsType>1) return 0;

    /* search code priority */
    return (p=strchr(s_GPSCodePris[ObsType][i-1],obs[1]))?14-(int)(p-s_GPSCodePris[ObsType][i-1]):0;
}

/********************************************************************************************
@brief   parse range obs system and prn to SPP system and prn

@param   Sys[In]               range obs system id
@param   PRN[In]               range obs prn number
@param   pSysInner[Out]        VERIQC system id
@param   pPrnInner[Out]        VERIQC prn number

@author  CHC

@return  VERIQC satellite number
*********************************************************************************************/
extern int32_t VeriQCParseRangeSatNo(int Sys, int PRN, int* pSysInner, int* pPrnInner)
{
    const static unsigned char s_RangeSystemMap[RANGE_SYSTEM_MAP_NUM] = {
    SYS_GPS,SYS_GLO,SYS_SBS,SYS_GAL,SYS_CMP,SYS_QZS,SYS_NONE,SYS_NONE
    };
    int prnBias[8] = { 0,37,CHC_MINPRNSBS - 1 ,0,0,0,0,0 };

    *pSysInner = s_RangeSystemMap[Sys];
    *pPrnInner = PRN - prnBias[Sys];

    return SatNo(*pSysInner, *pPrnInner);
}

/********************************************************************************************
@brief   parse ephemeris system to SPP system

@param   Sys[In]               range obs system id

@author  CHC

@return  VERIQC system id
*********************************************************************************************/
extern int32_t VeriQCParseEphemerisSystem(int Sys)
{
    const static unsigned char s_EphemerisSystemMap[EPHEMERIS_SYSTEM_MAP_NUM] = {
    SYS_GPS,SYS_GLO,SYS_GAL,SYS_CMP,SYS_SBS,SYS_QZS,SYS_NONE,SYS_CMP
    };

    return s_EphemerisSystemMap[Sys];
}

/**********************************************************************//**
@brief  get Phase Alignment Data

@param RMSystemID       [In] RawMsg system ID, see RM_RANGE_SYS_???
@param RMSignalID       [In] RawMsg signal ID

@retval Phase Alignment Data

@author CHC
@date 2024/05/30
@note
**************************************************************************/
double GetPhaseAlignmentData(int RMSystemID, int RMSignalID)
{
    const static double s_GPSPhaseAlignment[32] = {
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      -0.25,  0,
        0.25,   -0.25,  0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
    };
    const static double s_GLOPhaseAlignment[32] = {
        0,      0,      0,      0,      0,      0.25,   0.25,   0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
    };
    const static double s_GALPhaseAlignment[32] = {
        0,      0,      0.5,    0,      0,      0,      0,      -0.5,
        0,      0,      0,      0,      -0.25,  0,      0,      0,
        0,      -0.25,  0,      0,      -0.25,  0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
    };
    const static double s_BDSPhaseAlignment[32] = {
        0,      0,      0,      0,      0,      0,      0,      0.25,
        0,      0.25,   0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
    };
    const static double s_QZSPhaseAlignment[32] = {
        0,      0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      -0.25,  0,
        0.25,   0,      0,      0,      0,      0,      0,      0,
        0,      0,      0,      0,      0,      0,      0,      0,
    };

    if (RM_RANGE_SYS_GPS == RMSystemID)
    {
        return s_GPSPhaseAlignment[RMSignalID];
    }
    else if(RM_RANGE_SYS_GLO == RMSystemID)
    {
        return s_GLOPhaseAlignment[RMSignalID];
    }
    else if (RM_RANGE_SYS_GAL == RMSystemID)
    {
        return s_GALPhaseAlignment[RMSignalID];
    }
    else if (RM_RANGE_SYS_BDS == RMSystemID)
    {
        return s_BDSPhaseAlignment[RMSignalID];
    }
    else if (RM_RANGE_SYS_QZS == RMSystemID)
    {
        return s_QZSPhaseAlignment[RMSignalID];
    }

    return 0;
}


/********************************************************************************************
@brief   get satellite number in range obs

@param   pObs[In]              range obs

@author  CHC

@return  number of satellite
*********************************************************************************************/
extern int32_t VeriQCGetNsatFromRange(const struct _RM_RANGE_LOG_T* pObs)
{
    unsigned int iObs;
    int rangeSys, rangePrn;
    int iSys = 0, iPrn = 0;
    int nSat = 0;
    int satNo, preSatNo = 0;

    if (!pObs)
    {
        return 0;
    }
    for (iObs = 0; iObs < pObs->ObsNum; iObs++)
    {
        rangeSys = GET_CH_TRACK_STATUS_SATSYSTEM(pObs->ObsData[iObs].ChnTrackStatus);
        rangePrn = pObs->ObsData[iObs].PRN;
#ifdef USING_QZSS_AS_GPS
        if (RM_RANGE_SYS_QZS == rangeSys)/* convert QZSS to GPS */
        {
            rangeSys = RM_RANGE_SYS_GPS;
            rangePrn = CHC_MAXPRNGPS_TRUE + rangePrn - CHC_MINPRNQZS + 1;
        }
#endif
        satNo = VeriQCParseRangeSatNo(rangeSys, rangePrn, &iSys, &iPrn);
        if (satNo <= 0)
        {
            continue;
        }
        if (satNo != preSatNo)
        {
            preSatNo = satNo;
            nSat++;
        }
    }
    return nSat;
}

extern int VeriQCInitObsd(CHC_OBSD_T* pObs)
{
    int i = 0;
    if (!pObs)
    {
        return -1;
    }
    
    memset(pObs, 0, sizeof(CHC_OBSD_T));

    for (i = 0; i < (CHC_NFREQ + CHC_NEXOBS); i++)
    {
        pObs->Index[i] = -1;
    }
    pObs->FreqNumMax = CHC_NFREQ_SOLVE;
    return 0;
}

extern int VeriQCCopyTimeSPP(const QC_TIME_T* pSrc, QC_TIME_T* pDst)
{
    if(!pSrc||!pDst)
    {
        return -1;
    }
    pDst->Time = pSrc->Time;
    pDst->Sec = pSrc->Sec;
    return 0;
}

extern int VeriQCCopyTimeToSPP(const QC_TIME_T* pSrc, QC_TIME_T* pDst)
{
    if (!pSrc || !pDst)
    {
        return -1;
    }
    pDst->Time = pSrc->Time;
    pDst->Sec = pSrc->Sec;
    return 0;
}

extern int VeriQCCopyTimeFromSPP(const QC_TIME_T* pSrc, QC_TIME_T* pDst)
{
    if (!pSrc || !pDst)
    {
        return -1;
    }
    pDst->Time = pSrc->Time;
    pDst->Sec = pSrc->Sec;
    return 0;
}

extern int VeriQCCopyObs(const CHC_OBS_T* pSrc, CHC_OBS_T* pDst,int IncrementMark)
{
    if (!pSrc || !pDst) return -1;

    if (IncrementMark)
    {
        if (pDst->ObsNumMax < pSrc->ObsNum)
        {
            //clear old memory
            VeriQCFreeObs(pDst);
            //alloc memory
            VeriQCMallocObs(pDst, pSrc->ObsNum);
        }
    }
    else
    {
        //clear old memory
        VeriQCFreeObs(pDst);
        //alloc memory
        VeriQCMallocObs(pDst, pSrc->ObsNum);
    }

    memcpy(pDst->pData, pSrc->pData, sizeof(CHC_OBSD_T)*pSrc->ObsNum);
    pDst->ObsNum = pSrc->ObsNum;
    pDst->Time = pSrc->Time;
    return 0;
 
}

extern int VeriQCInitSPPTcOptDefault(VERIQC_SPPTC_OPT_T* pOpt)
{
    int i = 0;
    if (!pOpt)
    {
        return -1;
    }
    
    pOpt->Sys = VERIQC_SYS_GPS | VERIQC_SYS_GLN | VERIQC_SYS_BDS | VERIQC_SYS_GAL;
    pOpt->BasePos[0] = 0.0;
    pOpt->BasePos[2] = 0.0;
    pOpt->BasePos[1] = 0.0;
    
    pOpt->SysSignal[0] = VERIQC_L1 | VERIQC_L2 | VERIQC_L5;
    pOpt->SysSignal[1] = VERIQC_B1I | VERIQC_B2I | VERIQC_B3I | VERIQC_B1C | VERIQC_B2A;
    pOpt->SysSignal[2] = VERIQC_G1 | VERIQC_G2;
    pOpt->SysSignal[3] = VERIQC_E1 | VERIQC_E5A | VERIQC_E5B | VERIQC_E5AB;
    pOpt->PosMode = CHC_PMODE_SINGLE;
    pOpt->EleMin = 8.0;
    pOpt->SolType = 0;
    pOpt->FreqNum = CHC_NFREQ;
    pOpt->SatEph = CHC_EPHOPT_BRDC;
    pOpt->IonoOpt = CHC_IONOOPT_EST;
    pOpt->TropOpt = CHC_TROPOPT_SAAS;
    pOpt->Dynamics = 0;
    for (i = 0; i < VERIQC_NUM_FREQ; i++)
    {
        pOpt->ErrorRatio[i] = 100.0;
    }
    pOpt->ErrorFactor[0] = 100.0;
    pOpt->ErrorFactor[1] = 0.01;
    pOpt->ErrorFactor[2] = 0.01;
    pOpt->ErrorFactor[3] = 0.0;
    pOpt->ErrorFactor[4] = 1.0;
    pOpt->InitStateSTD[0] = 30.0;
    pOpt->InitStateSTD[1] = 0.03;
    pOpt->InitStateSTD[2] = 0.3;
    pOpt->ProcessNoiseSTD[0] = 1E-4;
    pOpt->ProcessNoiseSTD[1] = 1E-3;
    pOpt->ProcessNoiseSTD[2] = 1E-4;
    pOpt->ProcessNoiseSTD[3] = 1E-1;
    pOpt->ProcessNoiseSTD[4] = 1E-2;
    memset(pOpt->ExSatsGPS, 0, sizeof(uint8_t)*VERIQC_NSATGPS);
    memset(pOpt->ExSatsGLO, 0, sizeof(uint8_t)*VERIQC_NSATGLO);
    memset(pOpt->ExSatsGAL, 0, sizeof(uint8_t)*VERIQC_NSATGAL);
    memset(pOpt->ExSatsQZS, 0, sizeof(uint8_t)*VERIQC_NSATQZS);
    memset(pOpt->ExSatsBDS, 0, sizeof(uint8_t)*VERIQC_NSATBDS);
    pOpt->SatNumLimit = CHC_MAXDDOBS + CHC_SYSNUM;
   
    pOpt->MaxCPUTimeMsSatPara = 100;
    pOpt->EphInterpolation = 1; 

    pOpt->SPPGap =  0.999f;
    
#if !defined VERIQC_SURVEY
    opt->enable_faster = 0;// 1;
#else
    pOpt->EnableFaster = 1;// 1;
#endif
    pOpt->RefID = 0;
    pOpt->RealTimeMode = 1;
    pOpt->SimulateMaxAge = 0.0;
    pOpt->LogLevel[0] = 3;
    pOpt->LogLevel[1] = 5;

    return 0;
}

extern int VeriQCCopySPPTcOptCtrl(const VERIQC_SPPTC_OPT_T* pSrc, VERIQC_SPPTC_OPT_T* pDst, VERIQC_SPPTC_OPT_CTRL_E OptCtrl)
{
    if (!pSrc || !pDst)
    {
        return -1;
    }
    if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ALL)
    {
        memcpy(pDst, pSrc, sizeof(VERIQC_SPPTC_OPT_T));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SYS)
    {
        pDst->Sys = pSrc->Sys;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_POSREF)
    {
        memcpy(pDst->BasePos, pSrc->BasePos, 3 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SIGSYS)
    {
        memcpy(pDst->SysSignal, pSrc->SysSignal, VERIQC_SYS_NUM * sizeof(int16_t));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_POSMOD)
    {
        pDst->PosMode = pSrc->PosMode;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ELMIN)
    {
        pDst->EleMin = pSrc->EleMin;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SOLTYPE)
    {
        pDst->SolType = pSrc->SolType;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_NF)
    {
        pDst->FreqNum = pSrc->FreqNum;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SATEPH)
    {
        pDst->SatEph = pSrc->SatEph;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_IONOOPT)
    {
        pDst->IonoOpt = pSrc->IonoOpt;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_TROPOPT)
    {
        pDst->TropOpt = pSrc->TropOpt;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_DYNAMICS)
    {
        pDst->Dynamics = pSrc->Dynamics;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ERATIO)
    {
        memcpy(pDst->ErrorRatio, pSrc->ErrorRatio, VERIQC_NUM_FREQ * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ERR)
    {
        memcpy(pDst->ErrorFactor, pSrc->ErrorFactor, 5 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_STD)
    {
        memcpy(pDst->InitStateSTD, pSrc->InitStateSTD, 3 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_PRN)
    {
        memcpy(pDst->ProcessNoiseSTD, pSrc->ProcessNoiseSTD, 5 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGPS)
    {
        memcpy(pDst->ExSatsGPS, pSrc->ExSatsGPS, VERIQC_NSATGPS);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGLO)
    {
        memcpy(pDst->ExSatsGLO, pSrc->ExSatsGLO, VERIQC_NSATGLO);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGAL)
    {
        memcpy(pDst->ExSatsGAL, pSrc->ExSatsGAL, VERIQC_NSATGAL);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLQZS)
    {
        memcpy(pDst->ExSatsQZS, pSrc->ExSatsQZS, VERIQC_NSATQZS);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLBDS)
    {
        memcpy(pDst->ExSatsBDS, pSrc->ExSatsBDS, VERIQC_NSATBDS);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SATNUMLMT)
    {
        pDst->SatNumLimit = pSrc->SatNumLimit;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_MAXCPUSATPARA)
    {
        pDst->MaxCPUTimeMsSatPara = pSrc->MaxCPUTimeMsSatPara;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EPHINTP)
    {
        pDst->EphInterpolation = pSrc->EphInterpolation;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SPPGAP)
    {
        pDst->SPPGap = pSrc->SPPGap;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ENABLE_FASTER)
    {
        pDst->EnableFaster = pSrc->EnableFaster;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_REFID)
    {
        pDst->RefID = pSrc->RefID;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_RESETTODEFAULT)
    {
        VeriQCInitSPPTcOptDefault(pDst);
    }
	else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_RTMODE)
	{
		pDst->RealTimeMode = pSrc->RealTimeMode;
	}

    return 0;
}

extern int VeriQCCopyPrcoptCtrl(const CHC_PRCOPT_T* pSrc, CHC_PRCOPT_T* pDst, VERIQC_SPPTC_OPT_CTRL_E OptCtrl)
{
    int iLoop;

    if (!pSrc || !pDst)
    {
        return -1;
    }
    if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ALL)
    {
        memcpy(pDst, pSrc, sizeof(CHC_PRCOPT_T));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SYS)
    {
        pDst->NavSys = pSrc->NavSys;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SIGSYS)
    {

    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_POSMOD)
    {
        pDst->Mode = pSrc->Mode;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ELMIN)
    {
        pDst->EleMin = pSrc->EleMin;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SOLTYPE)
    {
        pDst->SolType = pSrc->SolType;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_NF)
    {
        pDst->FreqNum = pSrc->FreqNum;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SATEPH)
    {
        pDst->SatEph = pSrc->SatEph;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_IONOOPT)
    {
        pDst->IonoOpt = pSrc->IonoOpt;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_TROPOPT)
    {
        pDst->TropOpt = pSrc->TropOpt;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_DYNAMICS)
    {
        pDst->Dynamics = pSrc->Dynamics;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ERATIO)
    {
        memcpy(pDst->ErrorRatio, pSrc->ErrorRatio, VERIQC_NUM_FREQ * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ERR)
    {
        memcpy(pDst->ErrorFactor, pSrc->ErrorFactor, 5 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_STD)
    {
        memcpy(pDst->InitStateSTD, pSrc->InitStateSTD, 3 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_PRN)
    {
        memcpy(pDst->ProcessNoiseSTD, pSrc->ProcessNoiseSTD, 5 * sizeof(double));
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGPS)
    {
        for (iLoop = 0; iLoop < CHC_MAXSAT; iLoop++)
        {
            if (VeriQCSatSys(iLoop + 1, NULL) == CHC_SYS_GPS)
            {
                pDst->ExSats[iLoop] = pSrc->ExSats[iLoop];
            }
        }
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGLO)
    {
        for (iLoop = 0; iLoop < CHC_MAXSAT; iLoop++)
        {
            if (VeriQCSatSys(iLoop + 1, NULL) == CHC_SYS_GLO)
            {
                pDst->ExSats[iLoop] = pSrc->ExSats[iLoop];
            }
        }
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLGAL)
    {
        for (iLoop = 0; iLoop < CHC_MAXSAT; iLoop++)
        {
            if (VeriQCSatSys(iLoop + 1, NULL) == CHC_SYS_GAL)
            {
                pDst->ExSats[iLoop] = pSrc->ExSats[iLoop];
            }
        }
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLQZS)
    {
        for (iLoop = 0; iLoop < CHC_MAXSAT; iLoop++)
        {
            if (VeriQCSatSys(iLoop + 1, NULL) == CHC_SYS_QZS)
            {
                pDst->ExSats[iLoop] = pSrc->ExSats[iLoop];
            }
        }
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EXCLBDS)
    {
        for (iLoop = 0; iLoop < CHC_MAXSAT; iLoop++)
        {
            if (VeriQCSatSys(iLoop + 1, NULL) & (CHC_SYS_CMP|CHC_SYS_BD3))
            {
                pDst->ExSats[iLoop] = pSrc->ExSats[iLoop];
            }
        }
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SATNUMLMT)
    {
        pDst->SatNumLimit = pSrc->SatNumLimit;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_MAXCPUSATPARA)
    {
        pDst->MaxCPUTimeMsSatPara = pSrc->MaxCPUTimeMsSatPara;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_EPHINTP)
    {
        pDst->EphInterpolation = pSrc->EphInterpolation;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_SPPGAP)
    {
        pDst->SPPGap = pSrc->SPPGap;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_ENABLE_FASTER)
    {
        pDst->EnableFaster = pSrc->EnableFaster;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_REFID)
    {
        pDst->RefID = pSrc->RefID;
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_RESETTODEFAULT)
    {
        InitialOptDefault(pDst);
    }
    else if (OptCtrl == VERIQC_SPPTC_OPT_CTRL_RTMODE)
    {
        
    }
    return 0;
}

static void VeriQCCheckObsValid(struct _CHC_OBSD_T* pObsd)
{
    int fLoop;
    int freqIndex;

    pObsd->ObsValidMask = 0;
    for (fLoop = 0; fLoop < NFREQ; fLoop++)
    {
        freqIndex = pObsd->Index[fLoop];
        if (freqIndex < 0)
        {
            continue;
        }
        if (IsValidPhase(pObsd->L[freqIndex]))
        {
            pObsd->ObsValidMask |= BIT_MASK_PHASE(fLoop);
        }
        if (IsValidPseu(pObsd->P[freqIndex]))
        {
            pObsd->ObsValidMask |= BIT_MASK_CODE(fLoop);
        }
        if (IsValidDoppler(pObsd->D[freqIndex]))
        {
            pObsd->ObsValidMask |= BIT_MASK_DOP(fLoop);
        }
        if (IsValidSNR(pObsd->SNR[freqIndex]))
        {
            pObsd->ObsValidMask |= BIT_MASK_SNR(fLoop);
        }
    }
}

/********************************************************************************************
@brief   convert one range data to VERIQC obsd_t

@param   pOpt[In]              spp options
@param   pRangeData[In]        one range data
@param   pObsd[Out]            VERIQC one satellite obs

@author  CHC

@return  0:OK, -1:error
*********************************************************************************************/
static int VeriQCConvertObsdRange(VERIQC_SPPTC_OPT_T* pOpt, const RM_RANGE_DATA_T* pRangeData, struct _CHC_OBSD_T* pObsd)
{
    int sys;
    int freqIndex = -1;
    unsigned char code, freq;
    unsigned char signalType;
    BOOL bUseHighPriority = FALSE;
    short* pObsIndex;
    int RMSystem;

    if (!pOpt || !pRangeData || !pObsd)
    {
        return -1;
    }
    sys = pObsd->Sys;
    pObsd->FractionMark = 0;

    if (!IsValidPhase(pRangeData->ADR) && !IsValidPseu(pRangeData->PSR))
    {
        return -1;
    }
    signalType = GET_CH_TRACK_STATUS_SIGNALTYPE(pRangeData->ChnTrackStatus);
    RMSystem = GET_CH_TRACK_STATUS_SATSYSTEM(pRangeData->ChnTrackStatus);
    code = VeriQCConvertSigIDRange(sys, signalType, &freq);
    if (CODE_NONE == code || 0 == freq)
    {
        return -1;
    }
    if (sys == CHC_SYS_GPS)
    {
        if (pOpt->SysSignal[0] & freq)
        {
            if (VERIQC_L1 == freq)
            {
                freqIndex = 0;
                bUseHighPriority = TRUE;
            }
            else if (VERIQC_L2 == freq)
            {
                freqIndex = 1;
                bUseHighPriority = TRUE;
            }
            else if (VERIQC_L5 == freq) freqIndex = 2;
        }
    }
    else if (sys == CHC_SYS_GLO)
    {
        if (pOpt->SysSignal[2] & freq)
        {
            if (VERIQC_G1 == freq) freqIndex = 0;
            else if (VERIQC_G2 == freq) freqIndex = 1;
            else if (VERIQC_G3 == freq) freqIndex = 2;
        }
    }
    else if (sys == CHC_SYS_GAL)
    {
        if (pOpt->SysSignal[3] & freq)
        {
            if (VERIQC_E1 == freq) freqIndex = 0;
            else if (VERIQC_E5B == freq) freqIndex = 1;
            else if (VERIQC_E5A == freq) freqIndex = 2;
            else if (VERIQC_E5AB == freq) freqIndex = 3;
            else if (VERIQC_E6 == freq) freqIndex = 4;
        }
    }
    else if ((sys == CHC_SYS_CMP) || (sys == CHC_SYS_BD3))
    {
        if (pOpt->SysSignal[1] & freq)
        {
            if (VERIQC_B1I == freq) freqIndex = 0;
            else if (VERIQC_B2I == freq) freqIndex = 1;
            else if (VERIQC_B3I == freq) freqIndex = 2;
            else if (VERIQC_B1C == freq) freqIndex = 3;
            else if (VERIQC_B2A == freq) freqIndex = 4;
            else if (VERIQC_B2B == freq) freqIndex = 1;
            else if (VERIQC_B2 == freq) freqIndex = 5;
        }
    }
    else
    {
        return -1;
    }

    if (freqIndex >= 0 && pObsd->FreqNumMax > pObsd->FreqNum)
    {
        pObsIndex = pObsd->Index + freqIndex;
        if (*pObsIndex < 0)
        {
            *pObsIndex = pObsd->FreqNum++;
            pObsd->SNR[*pObsIndex ] = (unsigned char)pRangeData->CN0;
            pObsd->LLI[*pObsIndex ] = !GET_CH_TRACK_STATUS_PHASELOCK(pRangeData->ChnTrackStatus) + (GET_CH_TRACK_STATUS_HALFCYCLE(pRangeData->ChnTrackStatus) ? 3 : 0);
            pObsd->Code[*pObsIndex] = code;
            if (GET_CH_TRACK_STATUS_PHASELOCK(pRangeData->ChnTrackStatus) == 0)
            {
                pObsd->L[*pObsIndex] = 0.0;
            }
            else
            {
                pObsd->L[*pObsIndex] = -pRangeData->ADR;
                if (pObsd->AntID != VERIQC_ANT_BASE)
                {
                    pObsd->L[*pObsIndex] -= GetPhaseAlignmentData(RMSystem, signalType);
                }
            }
            pObsd->P[*pObsIndex ] = pRangeData->PSR;
            pObsd->D[*pObsIndex ] = pRangeData->Doppler;
            return 0;
        }
        if (TRUE == bUseHighPriority)/* gps L1/L2 code pri */
        {
            if (!IsAbsoluteDoubleZero(pRangeData->ADR) &&
                VeriQCGetGPSPri(pObsd->Code[*pObsIndex], 1) < VeriQCGetGPSPri(code, 1))
            {
                pObsd->SNR[*pObsIndex] = (unsigned char)pRangeData->CN0;
                pObsd->LLI[*pObsIndex] = !GET_CH_TRACK_STATUS_PHASELOCK(pRangeData->ChnTrackStatus) + (GET_CH_TRACK_STATUS_HALFCYCLE(pRangeData->ChnTrackStatus) ? 3 : 0);
                pObsd->Code[*pObsIndex] = code;
                if (GET_CH_TRACK_STATUS_PHASELOCK(pRangeData->ChnTrackStatus) == 0)
                {
                    pObsd->L[*pObsIndex] = 0.0;
                }
                else
                {
                    pObsd->L[*pObsIndex] = -pRangeData->ADR;
                    if (pObsd->AntID != VERIQC_ANT_BASE)
                    {
                        pObsd->L[*pObsIndex] -= GetPhaseAlignmentData(RMSystem, signalType);
                    }
                }
                pObsd->P[*pObsIndex] = pRangeData->PSR;
                pObsd->D[*pObsIndex] = pRangeData->Doppler;
            }
        }
    }
    return -1;
}

/********************************************************************************************
@brief   convert range log to VERIQC obs

@param   pOpt[In]              spp options
@param   pRangeLog[In]         range log data
@param   pObs[Out]             VERIQC raw obs
@param   StationType[In]       1:rover; 2:base

@author  CHC

@return  0:OK, -1:error
*********************************************************************************************/
extern int VeriQCConvertObsRange(VERIQC_SPPTC_OPT_T* pOpt, const struct _RM_RANGE_LOG_T* pRangeLog,
    CHC_OBS_T* pObs, unsigned char* pGLOFcn, int StationType)
{
    unsigned int iObs;
    int sys, sat, prn;
    int rangeSys, rangePrn;
    int preSat = 0;
    CHC_OBSD_T* pObsd;
    CHC_OBSD_T* pObsdMax;
    BOOL bFirstSignal = TRUE;

    if (!pOpt || !pRangeLog || !pObs)
    {
        return -1;
    }

    pObs->ObsNum = 0;
    pObs->Time = GPSTime2QCTime(pRangeLog->LogHeader.Week, pRangeLog->LogHeader.Milliseconds * 0.001);
    pObsd = pObs->pData;
    pObsdMax = pObsd + pObs->ObsNumMax;
    for (iObs = 0; iObs < pRangeLog->ObsNum; iObs++)
    {
        rangeSys = GET_CH_TRACK_STATUS_SATSYSTEM(pRangeLog->ObsData[iObs].ChnTrackStatus);
        rangePrn = pRangeLog->ObsData[iObs].PRN;
#ifdef USING_QZSS_AS_GPS
        if(RM_RANGE_SYS_QZS == rangeSys)/* convert QZSS to GPS */
        {
            rangeSys = RM_RANGE_SYS_GPS;
            rangePrn = CHC_MAXPRNGPS_TRUE + rangePrn - CHC_MINPRNQZS + 1;
        }
#endif
        sat = VeriQCParseRangeSatNo(rangeSys, rangePrn, &sys, &prn);
        if (!(sys & pOpt->Sys)) continue;
        if (sat <= 0) continue; //screen invalid svid

        if (sat != preSat)
        {
            if (preSat != 0)
            {
                VeriQCCheckObsValid(pObsd);
                pObsd++;
                if (pObsd >= pObsdMax)
                {
                    break;
                }
            }
            VeriQCInitObsd(pObsd);
            preSat = sat;
            pObs->ObsNum++;
            bFirstSignal = TRUE;
        }
        if (TRUE == bFirstSignal)
        {
            pObsd->Sat = (unsigned char)sat;
            pObsd->PRN = (unsigned char)prn;
            pObsd->Sys = (unsigned char)sys;
            pObsd->SysIndex = (char)GetSystemIndexByID(sys);
            pObsd->AntID = (unsigned char)StationType;
            pObsd->Time = pObs->Time;
            bFirstSignal = FALSE;
        }

        VeriQCConvertObsdRange(pOpt, &pRangeLog->ObsData[iObs], pObsd);
        if (NULL != pGLOFcn && SYS_GLO == sys)
        {
            pGLOFcn[prn] = (unsigned char)pRangeLog->ObsData[iObs].FreqNum;
        }
    }
    VeriQCCheckObsValid(pObsd);

    return 0;
}

extern int VeriQCConvertObsRangeSimple(VERIQC_SPPTC_OPT_T* pOpt, const struct _RM_RANGE_LOG_T* pRangeObs, struct _CHC_OBS_T* pDst)
{
    unsigned int iObs, iFreq;
    int iSys = 0, iPrn = 0;
    int satNo, preSatNo = 0;
    QC_TIME_T obsTime;

    if (!pRangeObs || !pDst)
    {
        return -1;
    }

    obsTime = GPSTime2QCTime(pRangeObs->LogHeader.Week, pRangeObs->LogHeader.Milliseconds * 0.001);
    for (iObs = 0; iObs < pRangeObs->ObsNum; iObs++)
    {
        satNo = VeriQCParseRangeSatNo(GET_CH_TRACK_STATUS_SATSYSTEM(pRangeObs->ObsData[iObs].ChnTrackStatus),
            pRangeObs->ObsData[iObs].PRN, &iSys, &iPrn);
        if (satNo <= 0)
        {
            continue;
        }
        if (!(iSys & pOpt->Sys))
        {
            continue;
        }
        if (satNo != preSatNo)
        {
            preSatNo = satNo;

            pDst->pData[pDst->ObsNum].FreqNum = 0;
            pDst->pData[pDst->ObsNum].FreqNumMax = 0;
            pDst->pData[pDst->ObsNum].AntID = STATION_UNKNOWN;
            for (iFreq = 0; iFreq < CHC_NFREQ + CHC_NEXOBS; iFreq++)
            {
                pDst->pData[pDst->ObsNum].Index[iFreq] = -1;
            }
            pDst->pData[pDst->ObsNum].Time = obsTime;
            pDst->pData[pDst->ObsNum].Sat = (unsigned char)satNo;
            pDst->pData[pDst->ObsNum].PRN = (unsigned char)iPrn;
            pDst->pData[pDst->ObsNum].Sys = (unsigned char)iSys;
            pDst->pData[pDst->ObsNum].SysIndex = (char)GetSystemIndexByID(iSys);
            pDst->ObsNum += 1;
        }
    }

    return 0;
}

extern int VeriQCConvertSysID(const VERIQC_SYSID_E SysID)
{
    if (SysID == VERIQC_GNSS_SYSID_GPS)
    {
        return CHC_SYS_GPS;
    }
    if (SysID == VERIQC_GNSS_SYSID_SBAS)
    {
        return CHC_SYS_SBS;
    }
    if (SysID == VERIQC_GNSS_SYSID_GLN)
    {
        return CHC_SYS_GLO;
    }
    if (SysID == VERIQC_GNSS_SYSID_GAL)
    {
        return CHC_SYS_GAL;
    }
    if (SysID == VERIQC_GNSS_SYSID_QZS)
    {
        return CHC_SYS_QZS;
    }
    if (SysID == VERIQC_GNSS_SYSID_BDS)
    {
        return CHC_SYS_CMP;
    }
    return CHC_SYS_NONE;
}

extern VERIQC_SYSID_E VeriQCConvertToSysID(const int Sys)
{
    VERIQC_SYSID_E sysid = 0;
    switch (Sys)
    {
        case SYS_GPS:sysid = VERIQC_GNSS_SYSID_GPS;break;
        case SYS_SBS:sysid = VERIQC_GNSS_SYSID_SBAS;break;
        case SYS_GLO:sysid = VERIQC_GNSS_SYSID_GLN;break;
        case SYS_GAL:sysid = VERIQC_GNSS_SYSID_GAL;break;
        case SYS_QZS:sysid = VERIQC_GNSS_SYSID_QZS;break;
        case SYS_CMP:sysid = VERIQC_GNSS_SYSID_BDS;break;
        default: break;
    }
    return sysid;
}

extern int VeriQCConvertSigID(uint8_t SigID)
{
    return SigID;
}

/********************************************************************************************
@brief   convert range signal type to VERIQC code and frequency

@param   Sys[In]               spp system id
@param   SigID[In]             range signal id
@param   pFreq[Out]            VERIQC frequency

@author  CHC

@return  VERIQC signal code. CODE_NONE if convert failed
*********************************************************************************************/
extern unsigned char VeriQCConvertSigIDRange(int Sys, uint32_t SigID, unsigned char* pFreq)
{
    /* range obs signal convert to VERIQC code code and frequency map */
    const static unsigned char s_RangeSignal2CodeMapGPS[RANGE_SIGNAL_MAP_NUM_GPS] = {
        CODE_L1C,CODE_L2C,CODE_NONE,CODE_NONE,CODE_L1P,CODE_L2P,/* 0-5 */
        CODE_L5I,CODE_NONE,CODE_L1W,CODE_L2W,CODE_L5X,CODE_NONE,/* 6-11 */
        CODE_L1S,CODE_L2L,CODE_L5Q,CODE_NONE,CODE_L1L,CODE_L2S, /* 12-17 */
        CODE_NONE,CODE_NONE,CODE_L1X,CODE_L2X                   /* 18-21 */
    };
    const static unsigned char s_RangeSignal2FreqMapGPS[RANGE_SIGNAL_MAP_NUM_GPS] = {
        VERIQC_L1,VERIQC_L2,0,0,VERIQC_L1,VERIQC_L2,           /* 0-5 */
        VERIQC_L5,0,VERIQC_L1,VERIQC_L2,VERIQC_L5,0,           /* 6-11 */
        VERIQC_L1,VERIQC_L2,VERIQC_L5,0,VERIQC_L1,VERIQC_L2,   /* 12-17 */
        0,0,VERIQC_L1,VERIQC_L2                                /* 18-21 */
    };
    const static unsigned char s_RangeSignal2CodeMapGLO[RANGE_SIGNAL_MAP_NUM_GLO] = {
        CODE_L1C,CODE_L2C,CODE_L3I,CODE_NONE,CODE_L1P,CODE_L2P,   /* 0-5 */
        CODE_L3Q,CODE_NONE,CODE_NONE,CODE_NONE,CODE_L3X,CODE_NONE,/* 6-11 */
        CODE_NONE,CODE_L6A,CODE_NONE,CODE_NONE,CODE_NONE,CODE_L6B,/* 12-17 */
        CODE_NONE,CODE_NONE,CODE_NONE,CODE_L6X                    /* 18-21 */
    };
    const static unsigned char s_RangeSignal2FreqMapGLO[RANGE_SIGNAL_MAP_NUM_GLO] = {
        VERIQC_G1,VERIQC_G2,VERIQC_G3,0,VERIQC_G1,VERIQC_G2,/* 0-5 */
        VERIQC_G3,0,0,0,VERIQC_G3,0,                        /* 6-11 */
        0,0,0,0,0,0,                                        /* 12-17 */
        0,0,0,0                                             /* 18-21 */
    };
    const static unsigned char s_RangeSignal2CodeMapGAL[RANGE_SIGNAL_MAP_NUM_GAL] = {
        CODE_L1A,CODE_L1B,CODE_L1C,CODE_L1X,CODE_L1Z,CODE_L6A,    /* 0-5 */
        CODE_L6B,CODE_L6C,CODE_L6X,CODE_L6Z,CODE_NONE,CODE_L5I,   /* 6-11 */
        CODE_L5Q,CODE_L5X,CODE_NONE,CODE_NONE,CODE_L7I,CODE_L7Q,  /* 12-17 */
        CODE_L7X,CODE_L8I,CODE_L8Q,CODE_L8X                       /* 18-21 */
    };
    const static unsigned char s_RangeSignal2FreqMapGAL[RANGE_SIGNAL_MAP_NUM_GAL] = {
        VERIQC_E1,VERIQC_E1,VERIQC_E1,VERIQC_E1,VERIQC_E1,VERIQC_E6,/* 0-5 */
        VERIQC_E6,VERIQC_E6,VERIQC_E6,VERIQC_E6,0,VERIQC_E5A,       /* 6-11 */
        VERIQC_E5A,VERIQC_E5A,0,0,VERIQC_E5B,VERIQC_E5B,            /* 12-17 */
        VERIQC_E5B,VERIQC_E5AB,VERIQC_E5AB,VERIQC_E5AB              /* 18-21 */
    };
    const static unsigned char s_RangeSignal2CodeMapQZS[RANGE_SIGNAL_MAP_NUM_QZS] = {
        CODE_L1C,CODE_NONE,CODE_NONE,CODE_NONE,CODE_NONE,CODE_NONE,/* 0-5 */
        CODE_L5I,CODE_NONE,CODE_NONE,CODE_NONE,CODE_L5X,CODE_NONE, /* 6-11 */
        CODE_L1S,CODE_NONE,CODE_L5Q,CODE_NONE,CODE_L1L,CODE_L2S,   /* 12-17 */
        CODE_NONE,CODE_NONE,CODE_L1X,CODE_L2X,CODE_NONE,CODE_NONE, /* 18-23 */
        CODE_NONE,CODE_NONE,CODE_L6S,CODE_L6L,CODE_L6X             /* 24-28 */
    };
    const static unsigned char s_RangeSignal2FreqMapQZS[RANGE_SIGNAL_MAP_NUM_QZS] = {
        VERIQC_L1,0,0,0,0,0,                                       /* 0-5 */
        VERIQC_L5,0,0,0,VERIQC_L5,0,                               /* 6-11 */
        VERIQC_L1,0,VERIQC_L5,0,VERIQC_L1,VERIQC_L2,               /* 12-17 */
        0,0,VERIQC_L1,VERIQC_L2,0,0,                               /* 18-23 */
        0,0,0,0,0                                                  /* 24-28 */
    };
    const static unsigned char s_RangeSignal2CodeMapSBS[RANGE_SIGNAL_MAP_NUM_SBS] = {
        CODE_L1C,CODE_NONE,CODE_NONE,CODE_NONE,CODE_NONE,CODE_NONE,/* 0-5 */
        CODE_L5I,CODE_NONE,CODE_NONE,CODE_NONE,CODE_L5X,CODE_NONE, /* 6-11 */
        CODE_NONE,CODE_NONE,CODE_L5Q                               /* 12-14 */
    };
    const static unsigned char s_RangeSignal2FreqMapSBS[RANGE_SIGNAL_MAP_NUM_SBS] = {
        VERIQC_L1,0,0,0,0,0,                                       /* 0-5 */
        VERIQC_L5,0,0,0,VERIQC_L5,0,                               /* 6-11 */
        0,0,VERIQC_L5                                              /* 12-14 */
    };
    const static unsigned char s_RangeSignal2CodeMapBDS[RANGE_SIGNAL_MAP_NUM_BDS] = {
        CODE_L2I,CODE_L7I,CODE_L6I,CODE_NONE,CODE_L2I,CODE_L7I,    /* 0-5 */
        CODE_L6I,CODE_L1P,CODE_L7P,CODE_L5P,CODE_L1D,CODE_L7D,     /* 6-11 */
        CODE_L5D,CODE_L1X,CODE_L7X,CODE_L5X,CODE_NONE,CODE_NONE,   /* 12-17 */
        CODE_NONE,CODE_NONE,CODE_NONE,CODE_NONE,CODE_L2Q,CODE_L7Q, /* 18-23 */
        CODE_L6Q,CODE_NONE,CODE_L2X,CODE_L7X,CODE_L6X              /* 24-28 */
    };
    const static unsigned char s_RangeSignal2FreqMapBDS[RANGE_SIGNAL_MAP_NUM_BDS] = {
        VERIQC_B1I,VERIQC_B2I,VERIQC_B3I,0,VERIQC_B1I,VERIQC_B2I,          /* 0-5 */
        VERIQC_B3I,VERIQC_B1C,VERIQC_B2B,VERIQC_B2A,VERIQC_B1C,VERIQC_B2B, /* 6-11 */
        VERIQC_B2A,VERIQC_B1C,VERIQC_B2B,VERIQC_B2A,0,0,                   /* 12-17 */
        0,0,0,0,VERIQC_B1I,VERIQC_B2I,                                     /* 18-23 */
        VERIQC_B3I,0,VERIQC_B1I,VERIQC_B2I,VERIQC_B3I                      /* 24-28 */
    };

    switch (Sys)
    {
    case SYS_GPS:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_GPS)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapGPS[SigID];
        return s_RangeSignal2CodeMapGPS[SigID];
    case SYS_GLO:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_GLO)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapGLO[SigID];
        return s_RangeSignal2CodeMapGLO[SigID];
    case SYS_GAL:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_GAL)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapGAL[SigID];
        return s_RangeSignal2CodeMapGAL[SigID];
    case SYS_QZS:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_QZS)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapQZS[SigID];
        return s_RangeSignal2CodeMapQZS[SigID];
    case SYS_SBS:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_SBS)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapSBS[SigID];
        return s_RangeSignal2CodeMapSBS[SigID];
    case SYS_CMP:
    case SYS_BD3:
        if (SigID >= RANGE_SIGNAL_MAP_NUM_BDS)
        {
            return CODE_NONE;
        }
        *pFreq = s_RangeSignal2FreqMapBDS[SigID];
        return s_RangeSignal2CodeMapBDS[SigID];
    default: return CODE_NONE;
    }
}

/********************************************************************************************
@brief   convert HC02 unify ephemeris type to VERIQC ephemeris type

@param   pSrc[In]               HC02 unify ephemeris type
@param   pDst[Out]              spp internal ephemeris type

@author  CHC

@return  0:OK, -1:failed
*********************************************************************************************/
extern int VeriQCConvertUNIEphemeris(const struct _UNI_EPHEMERIS_T* pSrc, CHC_EPH_T* pDst)
{
    QC_TIME_T tm = { 0 };
    int uniEphSys, uniEphPrn;
    int sys;
    if (!pSrc || !pDst)
    {
        return -1;
    }
    uniEphSys = pSrc->SystemID;
    uniEphPrn = pSrc->SatID;
#ifdef USING_QZSS_AS_GPS
    if (QZS == uniEphSys)/* convert QZSS to GPS */
    {
        uniEphSys = GPS;
        uniEphPrn = CHC_MAXPRNGPS_TRUE + uniEphPrn;
    }
#endif
    sys = VeriQCParseEphemerisSystem(uniEphSys);
    pDst->Sat = VeriQCSatNo(sys, uniEphPrn);
    pDst->IODE = pSrc->IODE1;
    pDst->IODC = pSrc->IODC;
    pDst->SVA = pSrc->URA;
    pDst->SVH = pSrc->Health;
    pDst->Week = pSrc->Week;

    if (sys == SYS_CMP)
    {
        if (pDst->Week < 1356) pDst->Week += 1356;
    }
    else if(sys == SYS_GAL)
    {
        pDst->Week += 1024;
        
    }
    tm = VeriQCUNIEphTime2Time(sys, pSrc->Week, (double)pSrc->TOE);
    pDst->Toe.Time = tm.Time;
    pDst->Toe.Sec = tm.Sec;
    tm = VeriQCUNIEphTime2Time(sys, pSrc->Week, (double)pSrc->TOC);
    pDst->Toc.Time = tm.Time;
    pDst->Toc.Sec = tm.Sec;
    pDst->Ttr = pDst->Toe;
    pDst->Code = pSrc->NavDataSource;
    if (SYS_CMP == sys && (pSrc->SatID <= 5 || pSrc->SatID >= 60))
    {
        pDst->Flag = 2;
    }
    else
    {
        pDst->Flag = 0;
    }
    pDst->A = pSrc->A;
    pDst->E = pSrc->Ecc;
    pDst->I0 = pSrc->I0;
    pDst->OMG0 = pSrc->Omega0;
    pDst->OMG = pSrc->Omega;
    pDst->M0 = pSrc->M0;
    pDst->DeltaN = pSrc->DeltaN;
    pDst->OMGd = pSrc->OmegaDot;
    pDst->IDot = pSrc->IDot;
    pDst->Crc = pSrc->Crc;
    pDst->Crs = pSrc->Crs;
    pDst->Cuc = pSrc->Cuc;
    pDst->Cus = pSrc->Cus;
    pDst->Cic = pSrc->Cic;
    pDst->Cis = pSrc->Cis;
    pDst->ToeSec = pSrc->TOE;
    if(EPH_DATA_TYPE_BDS_CNAV == pSrc->EphDataType)/* only bds3 cnav */
    {
        pDst->ADot = pSrc->Adot;
        pDst->NDot = pSrc->Ndot;
    }
    else
    {
        pDst->ADot = 0.0;
        pDst->NDot = 0.0;
    }
    pDst->FitInterval = pSrc->FitInterval;
    pDst->Af0 = pSrc->Af0;
    pDst->Af1 = pSrc->Af1;
    pDst->Af2 = pSrc->Af2;
    pDst->Tgd[0] = pSrc->Tgd[0];
    pDst->Tgd[1] = pSrc->Tgd[1];
    pDst->Tgd[2] = pSrc->Tgd[2];
    pDst->Tgd[3] = 0.0;

    return 0;
}

/********************************************************************************************
@brief   convert HC02 glonass ephemeris type to VERIQC glonass ephemeris type

@param   pSrc[In]               HC02 glonass ephemeris type
@param   pDst[Out]              spp internal glonass ephemeris type

@author  CHC

@return  0:OK, -1:failed
*********************************************************************************************/
extern int VeriQCConvertGLOEphemeris(const struct _GLO_EPHEMERIS_T* pSrc, CHC_GEPH_T* pDst)
{
    QC_TIME_T tm = { 0 };
    if (!pSrc || !pDst)
    {
        return -1;
    }
    pDst->Sat = VeriQCSatNo(SYS_GLO, pSrc->SatID);
    pDst->IODE = (pSrc->Tb / 900) & 0x7F;
    pDst->Freq = pSrc->Freq;
    pDst->SVH = (pSrc->Bn >> 1);
    pDst->SVA = 0;
    pDst->Age = 0;
    tm = VeriQCGLOEphTime2Time(pSrc->LeapYear, pSrc->Nt, pSrc->Tb);
    pDst->Toe.Time = tm.Time;
    pDst->Toe.Sec = tm.Sec;
    pDst->Tof = pDst->Toe; /* @TODO:calc by Tk */
    pDst->Pos[0] = pSrc->PosXYZ[0];
    pDst->Pos[1] = pSrc->PosXYZ[1];
    pDst->Pos[2] = pSrc->PosXYZ[2];
    pDst->Vel[0] = pSrc->VelXYZ[0];
    pDst->Vel[1] = pSrc->VelXYZ[1];
    pDst->Vel[2] = pSrc->VelXYZ[2];
    pDst->Acc[0] = pSrc->AccXYZ[0];
    pDst->Acc[1] = pSrc->AccXYZ[1];
    pDst->Acc[2] = pSrc->AccXYZ[2];
    pDst->Taun = pSrc->TauN;
    pDst->Gamn = pSrc->GammaN;
    pDst->DTaun = pSrc->DeltaTauN;
    pDst->LeapSec = 0;

    return 0;

}

/**
* @brief limit angle in degree to 0 ~ 360
* @author CHC
* @date 2020/06/02
* @param[in]    Deg    input angle in degree
* @return limited angle in degree
*/
static double SPPLimitDeg360(double Deg)
{
    if (Deg < 360 && Deg >= 0.0) return Deg;
    return Deg - floor(Deg / 360.0) * 360.0;
}

/**
* @brief getting std of  velocity heading
* @author CHC
* @data 2022/09/09
* @param[in]    double VelENU[3]
* @param[in]    double StdVel[3]
* @return velocity heading std
*/
static double SPPHeadingVelStdFromVelStd(double VelENU[3], double  StdVel[3])
{
    double headingvel_std = 0.0;
    int i;
    /* limit ve and vn values when very small */
    for (i = 0; i < 2; i++)
    {
        if (fabs(VelENU[i]) < 0.001)
        {
            VelENU[i] = 0.001;
        }
    }

    /* limit ve and vn std values when very small */
    for (i = 0; i < 2; i++)
    {
        if (StdVel[i] < 0.005)
        {
            StdVel[i] = 0.005;
        }
    }

    /* getting headingvel std from vel_enu std */
    double ve2 = SPP_SQR(VelENU[0]), vn2 = SPP_SQR(VelENU[1]);
    headingvel_std = sqrt(ve2 * SPP_SQR(StdVel[1]) + vn2 * SPP_SQR(StdVel[0])) / (ve2 + vn2);
    if (headingvel_std > 2 * PI) headingvel_std = 2 * PI;

    return headingvel_std;
}

extern int VeriQCConvertSolToSolInfo(const CHC_SOL_T* pSrc,const CHC_SSATLIST_T *pSrcSsatlist,
    const VERIQC_ORBIT_SOL_T* pOrbSol, VERIQC_SOLINFO_T* pDst)
{
    int i = 0, j = 0, k, solsatnum = 0, satnum, sat, sys, prn = 0, sysid, sigid, solsysmark_shift = 0;
    double pos[3] = { 0 }, P[9] = { 0 }, Q[9] = { 0 }, ele;
    U1 posExisted = 0, snrmax = 0;
    int oldsatnum;
    CHC_SSAT_T *ssat;
    double venu[3], venu_std[3];
    VERIQC_SATINFO_PERSAT_T* pSatInfo;

    if (!pSrc || !pDst)
    {
        return -1;
    }
    if (pDst->pTime)
    {
        VeriQCCopyTimeFromSPP(&pSrc->Time, &pDst->pTime->Time);
        for (i = 0; i < 6; i++)
        {
            pDst->pTime->ClkBias[i] = pSrc->Dtr[i];
        }
        pDst->pTime->ClkDrift = 0.0;
        for (i = 0; i < SYSNUM; i++)
        {
            if (pSrc->DtrDot[i] != 0.0)
            {
                pDst->pTime->ClkDrift = pSrc->DtrDot[i];
                break;
            }
        }
        pDst->pTime->LeapSec=18;
        pDst->pTime->TimeStatus=4;
    }
    if (pDst->pSolState)
    {
        switch (pSrc->Stat)
        {
            case CHC_SOLQ_NONE: pDst->pSolState->PosType = VERIQC_POSTYPE_NONE; break;
            case CHC_SOLQ_SINGLE: pDst->pSolState->PosType = VERIQC_POSTYPE_GNSS_SINGLE; break;
            case CHC_SOLQ_DGPS: pDst->pSolState->PosType = VERIQC_POSTYPE_GNSS_DGNSS; break;
            case CHC_SOLQ_FLOAT: pDst->pSolState->PosType = VERIQC_POSTYPE_GNSS_FLOAT; break;
            case CHC_SOLQ_FIX: pDst->pSolState->PosType = VERIQC_POSTYPE_GNSS_FIX; break;
            case CHC_SOLQ_PPP: pDst->pSolState->PosType = VERIQC_POSTYPE_PPP; break;
			default:pDst->pSolState->PosType = VERIQC_POSTYPE_NONE; break;
        }
        pDst->pSolState->SolStat = pSrc->SolStat;
        pDst->pSolState->SolSatNum = pSrc->SatNum;
        pDst->pSolState->TrackSatNum = pSrc->ARSatNum;
        pDst->pSolState->DiffAge = pSrc->Age;
        pDst->pSolState->AgeOk = pSrc->AgeOk;
        pDst->pSolState->SolCount = pSrc->SolCnt;
        pDst->pSolState->IsStop = pSrc->EnvDetectInfo.MotionMode == 1 && pSrc->EnvDetectInfo.StaticCnt > 30;
        if(1 == pSrc->EnvDetectInfo.MotionMode)
        {
            pDst->pSolState->DynamicMode = 1;
        }
        else
        {
            pDst->pSolState->DynamicMode = 0;
        }
    }
    if (pSrc->Stat == SOLQ_NONE)
    {
        return 0;
    }
    if (pDst->pPos)
    {
        memcpy(pDst->pPos->PosXYZ, pSrc->Rr, 3 * sizeof(double));
        pDst->pPos->PosXYZStd[0] = (float)sqrt(pSrc->Qr[0]);/* missing xyz co-variance */
        pDst->pPos->PosXYZStd[1] = (float)sqrt(pSrc->Qr[1]);
        pDst->pPos->PosXYZStd[2] = (float)sqrt(pSrc->Qr[2]);

        pDst->pPos->PosRMS = (float)pSrc->PosRms;
        pDst->pPos->Dop[0] = (float)pSrc->Dop[0];
        pDst->pPos->Dop[1] = (float)pSrc->Dop[1];
        pDst->pPos->Dop[2] = (float)pSrc->Dop[2];
        pDst->pPos->Dop[3] = (float)pSrc->Dop[3];
        pDst->pPos->Dop[4] = (float)pSrc->Dop[4];
        pDst->pPos->ProcessObsNum = pSrc->ProcSatNum;
        pDst->pPos->Iter = pSrc->Iter;
        PosXYZ2LLH(pSrc->Rr, pos);
        posExisted = 1;
        SolToCov(pSrc, P); /* xyz_std -> llh_std */
        CovENU(pos, P, Q);
        pDst->pPos->PosLLH[0] = pos[0];
        pDst->pPos->PosLLH[1] = pos[1];
        pDst->pPos->PosLLH[2] = pos[2];
        pDst->pPos->PosLLHStd[0] = (float)sqrt(Q[0]);/* missing llh co-variance */
        pDst->pPos->PosLLHStd[1] = (float)sqrt(Q[4]);
        pDst->pPos->PosLLHStd[2] = (float)sqrt(Q[8]);
        pDst->pPos->Undulation = 0;

        if (pSrc->Stat != SOLQ_NONE)
        {
            for (i = 0; i < 3; i++) pDst->pPos->SemiStd[i] = pSrc->SemiStd[i];
        }
        else
        {
            for (i = 0; i < 3; i++) pDst->pPos->SemiStd[i] =0.0f;
            pDst->pPos->Undulation = 0.0f;
        }
    }

    if (pSrc->VStat != SOLQ_NONE && pDst->pVel)
    {
        memcpy(pDst->pVel->VelXYZ, pSrc->Rr + 3, 3 * sizeof(double));
        pDst->pVel->VelXYZStd[0] = (float)sqrt(pSrc->QVel[0]);
        pDst->pVel->VelXYZStd[1] = (float)sqrt(pSrc->QVel[1]);
        pDst->pVel->VelXYZStd[2] = (float)sqrt(pSrc->QVel[2]);
        pDst->pVel->VelRMS = (float)pSrc->VelRms;
        if(!posExisted)
        {
            PosXYZ2LLH(pSrc->Rr, pos);
            posExisted = 1;
        }
        ECEF2ENU(pos, pDst->pVel->VelXYZ, pDst->pVel->VelENU);
        SolVelToCov(pSrc, P);
        CovENU(pos, P, Q);
        pDst->pVel->VelENUStd[0] = (float)sqrt(Q[0]);/* missing llh co-variance */
        pDst->pVel->VelENUStd[1] = (float)sqrt(Q[4]);
        pDst->pVel->VelENUStd[2] = (float)sqrt(Q[8]);

        /* gnss velocity heading and it std values */
        pDst->pVel->HeadingVel =
            atan2(pDst->pVel->VelENU[0], pDst->pVel->VelENU[1]) * 57.2957795130823;
        pDst->pVel->HeadingVel = SPPLimitDeg360(pDst->pVel->HeadingVel);
        venu[0] = pDst->pVel->VelENU[0];
        venu[1] = pDst->pVel->VelENU[1];
        venu[2] = pDst->pVel->VelENU[2];
        venu_std[0] = pDst->pVel->VelENUStd[0];
        venu_std[1] = pDst->pVel->VelENUStd[1];
        venu_std[2] = pDst->pVel->VelENUStd[2];
        pDst->pVel->HeadingVelStd =
            SPPHeadingVelStdFromVelStd(venu, venu_std) * 57.2957795130823;
    }
    if (pDst->pSolState&&pSrcSsatlist)
    {
        if (pDst->pSolState->pSatInfo)
        {
            if (pDst->pTime) pDst->pSolState->pSatInfo->Time = pDst->pTime->Time;
            pDst->pSolState->pSatInfo->TimeStatus = 0;
            oldsatnum = pDst->pSolState->pSatInfo->SatNum;
            pDst->pSolState->pSatInfo->SatNum = 0;
            pDst->pSolState->SolSysMask = 0;
            if (pDst->pSolState->pSatInfo->pSatInfo)
            {
                if (pSrc->Stat != SOLQ_NONE)
                {
                    pDst->pSolState->pSatInfo->TimeStatus = 1;
                    satnum = 0;
                    solsatnum = 0;
                    for (i = 0; i < pSrcSsatlist->Num; i++)
                    {
                        ssat = &pSrcSsatlist->pSsat[i];
                        if (!ssat->ValidSatFlagSingle)
                        {
                            continue;/* invisible */
                        }
                        sat = ssat->Sat;
                        sys = SatSys(sat, &prn);
                        sysid = sigid = 0;

                        switch (sys)
                        {
                            case SYS_GPS:sysid = VERIQC_GNSS_SYSID_GPS; solsysmark_shift = 0; break;
                            case SYS_SBS:sysid = VERIQC_GNSS_SYSID_SBAS; solsysmark_shift = 4; break;
                            case SYS_GLO:sysid = VERIQC_GNSS_SYSID_GLN; solsysmark_shift = 1; break;
                            case SYS_GAL:sysid = VERIQC_GNSS_SYSID_GAL; solsysmark_shift = 3; break;
                            case SYS_QZS:sysid = VERIQC_GNSS_SYSID_QZS; solsysmark_shift = 5; break;
                            case SYS_BD3:
                            case SYS_CMP:sysid = VERIQC_GNSS_SYSID_BDS; solsysmark_shift = 2; break;
                            default: break;
                        }

                        if (ssat->ValidSatFlagSingle == 3)
                        {
                            pDst->pSolState->SolSysMask |= 1u << solsysmark_shift;
                        }

                        for (k = 0; k < pSrc->ARSatNum; k++)
                        {
                            if (sat == pSrc->ARSat[k])
                            {
                                sigid = pSrc->ARSig[k];
                                break;
                            }
                        }
                        if (satnum >= oldsatnum)
                        {
                            satnum += 0;
                        }
                        pSatInfo = pDst->pSolState->pSatInfo->pSatInfo + satnum;
                        pSatInfo->SysID = sysid;
                        pSatInfo->SatID = prn;
                        pSatInfo->GLOFreq = ssat->GLOFreq;
                        pSatInfo->SigID = sigid;
                        pSatInfo->SatStatus = ssat->SatStatus;
                        pSatInfo->Visibility = ssat->ValidSatFlagSingle;
                        pSatInfo->CN0 = ssat->SNR[0];
                        pSatInfo->PSRResLevel = ssat->PSRErrLevel;
                        pSatInfo->AzEl[0] = (int)ROUND_D(ssat->AzEl[0] * R2D);
                        pSatInfo->UsedInSolve = 0;
                        pSatInfo->UsedFrequencyMark = 0;
                        if (ssat->AzEl[1] > 0.0)
                        {
                            ele = ssat->AzEl[1] * R2D;
                            if (ele >0.999999)
                            {
                                pSatInfo->AzEl[1] = (int)ROUND_D(ele);
                            }
                            else
                            {
                                pSatInfo->AzEl[1] = 1;
                            }
                        }
                        else
                        {
                            pSatInfo->AzEl[1] = 0;
                        }
                        pSatInfo->PSR = ssat->PSR;
                        pSatInfo->PSRRate0 = ssat->PSRRate0;
                        pSatInfo->PSRAcc = ssat->PSRAcc;
                        pSatInfo->PSRRes = ssat->PSRRes;
                        pSatInfo->PSRRate0Res = ssat->PSRRate0Res;
                        pSatInfo->PSRFreqIndex = ssat->PSRFreqIndex;

                        snrmax = pSatInfo->CN0;
                        //to avoid the snr[0]=0, select the max snr to output
                        if (snrmax == 0)
                        {
                            for (j = 0; j < CHC_NFREQ; j++)
                            {
                                if(snrmax < ssat->SNR[j])
                                {
                                    snrmax = ssat->SNR[j];
                                }
                            }
                            if (snrmax == 0)
                            {
                                continue;
                            }
                            pSatInfo->CN0 = snrmax;
                        }

                        if(NULL != pOrbSol)
                        {
                            for (j = 0; j < pOrbSol->ObsNum; j++)
                            {
                                if (sat == pOrbSol->pSatNo[j])
                                {
                                    for (k = 0; k < 6; k++)
                                    {
                                        pSatInfo->PosVel[k] = pOrbSol->pRs[k + j * 6];
                                    }
                                    pSatInfo->SatClockError = pOrbSol->pDts[j * 2];
                                    break;
                                }
                            }
                        }
                        /** used in solve */
                        if (ssat->ValidSatFlagSingle == SATVIS_SOLVING)
                        {
                            pSatInfo->UsedInSolve = 1;
                            pSatInfo->UsedFrequencyMark = ssat->UsedFrequencyMark;
                            solsatnum += 1;
                        }
                        satnum++;
                    }
                    pDst->pSolState->pSatInfo->SatNum = satnum;
                    if (pDst->pSolState->TrackSatNum != satnum)
                    {
                        pDst->pSolState->TrackSatNum = satnum;
                    }
                    if (pDst->pSolState->SolSatNum != solsatnum)
                    {
                        pDst->pSolState->SolSatNum = solsatnum;
                    }
                }
            }
        }
    }
    return 0;
}

extern int VeriQCMallocObs(CHC_OBS_T* pObs, int Num)
{
    int i;
    if (!pObs||Num<=0)
    {
        return -1;
    }
    pObs->pData = VERIQC_MALLOC(Num * sizeof(CHC_OBSD_T));
    memset(pObs->pData, 0, Num * sizeof(CHC_OBSD_T));
    pObs->ObsNum = 0;
    pObs->ObsNumMax = Num;

    for (i = 0; i < Num; i++)
    {
        VeriQCInitObsd(&(pObs->pData[i]));
    }
    return 0;
}

extern int VeriQCMallocOrbitSol(VERIQC_ORBIT_SOL_T* pSol, int Num)
{
    if (!pSol)
    {
        return -1;
    }
    if (Num <= 0)
    {
        return -1;
    }
    pSol->ObsNum = Num;
    pSol->pSatNo = VERIQC_MALLOC(pSol->ObsNum * sizeof(int16_t));
    pSol->pSVH = VERIQC_MALLOC(pSol->ObsNum);
    pSol->pType = VERIQC_MALLOC(pSol->ObsNum);
    pSol->pExcludeSats = VERIQC_MALLOC(CHC_MAXSAT);
    pSol->pRs = VERIQC_MALLOC(pSol->ObsNum * 6 * sizeof(double));
    pSol->pDts = VERIQC_MALLOC(pSol->ObsNum * 2 * sizeof(double));
    pSol->pVar = VERIQC_MALLOC(pSol->ObsNum * sizeof(double));
    pSol->pIODE = VERIQC_MALLOC(pSol->ObsNum * sizeof(int32_t));
    pSol->pToe = VERIQC_MALLOC(pSol->ObsNum * sizeof(int32_t));

    VeriQCClearOrbitSol(pSol);

    return 0;
}

extern int VeriQCMallocSolInfo(VERIQC_SOLINFO_T* pSol, int SatNum)
{
    if (!pSol)
    {
        return -1;
    }
    pSol->pTime = VERIQC_MALLOC(sizeof(VERIQC_SOLTIME_T));
    pSol->pSolState = VERIQC_MALLOC(sizeof(VERIQC_SOLSTATE_T));
    pSol->pPos = VERIQC_MALLOC(sizeof(VERIQC_POS_T));
    pSol->pVel = VERIQC_MALLOC(sizeof(VERIQC_VEL_T));
    pSol->pUserData = NULL;
    memset(pSol->pTime, 0, sizeof(VERIQC_SOLTIME_T));
    memset(pSol->pSolState, 0, sizeof(VERIQC_SOLSTATE_T));
    memset(pSol->pPos, 0, sizeof(VERIQC_POS_T));
    memset(pSol->pVel, 0, sizeof(VERIQC_VEL_T));
    pSol->pSolState->pSatInfo = VERIQC_MALLOC(sizeof(VERIQC_SATINFO_T));
    memset(pSol->pSolState->pSatInfo, 0, sizeof(VERIQC_SATINFO_T));
    if (SatNum > 0)
    {
        pSol->pSolState->pSatInfo->SatNum = SatNum;
        pSol->pSolState->pSatInfo->pSatInfo = VERIQC_MALLOC(sizeof(VERIQC_SATINFO_PERSAT_T)*SatNum);
        memset(pSol->pSolState->pSatInfo->pSatInfo, 0, sizeof(VERIQC_SATINFO_PERSAT_T)*SatNum);
    }
    return 0;
}

extern int VeriQCClearSolInfo(VERIQC_SOLINFO_T* pSol)
{
    int i = 0;
    if (!pSol)
    {
        return -1;
    }
    if (pSol->pTime)
    {
        memset(pSol->pTime, 0, sizeof(VERIQC_SOLTIME_T));
    }
    if (pSol->pSolState)
    {
        if (pSol->pSolState->pSatInfo)
        {
            if (pSol->pSolState->pSatInfo->SatNum&&pSol->pSolState->pSatInfo->pSatInfo)
            {
                memset(pSol->pSolState->pSatInfo->pSatInfo, 0, pSol->pSolState->pSatInfo->SatNum * sizeof(VERIQC_SATINFO_PERSAT_T));
                pSol->pSolState->pSatInfo->Time.Time = 0;
                pSol->pSolState->pSatInfo->Time.Sec = 0;
                pSol->pSolState->pSatInfo->TimeStatus = 0;
                pSol->pSolState->pSatInfo->SatNum = 0;
            }
            pSol->pSolState->SolStat = SOL_STATUS_INSUFFICIENT_OBS;
            pSol->pSolState->PosType = 0;
            pSol->pSolState->SolSatNum = 0;
            pSol->pSolState->TrackSatNum = 0;
            pSol->pSolState->SolSysMask = 0;
            pSol->pSolState->RefID = 0;
            pSol->pSolState->SolCount = 0;
            pSol->pSolState->Cutoff = 0;
            pSol->pSolState->DiffAge = 0;
            pSol->pSolState->AgeOk = 0;
            memset(pSol->pSolState->RefPosLLH, 0, sizeof(double) * 3);
        }
    }
    if (pSol->pPos)
    {
        memset(pSol->pPos, 0, sizeof(VERIQC_POS_T));
    }
    if (pSol->pVel)
    {
        memset(pSol->pVel, 0, sizeof(VERIQC_VEL_T));
    }
    if (pSol->pUserData)
    {
        //Nothing to do
    }
    return 0;
}

extern int VeriQCClearOrbitSol(VERIQC_ORBIT_SOL_T* pOrbitSol)
{
    if (!pOrbitSol)
    {
        return -1;
    }
    
    memset(pOrbitSol->pSatNo, 0, sizeof(int16_t)*pOrbitSol->ObsNum);
    //memset(pOrbitSol->svh, 0, pOrbitSol->ns);
    memset(pOrbitSol->pType, 0, pOrbitSol->ObsNum);
    memset(pOrbitSol->pExcludeSats, 0, CHC_MAXSAT);
    //memset(pOrbitSol->rs, 0, sizeof(double)*pOrbitSol->ns * 6);
    //memset(pOrbitSol->dts, 0, sizeof(double)*pOrbitSol->ns * 2);
    //memset(pOrbitSol->var, 0, sizeof(double)*pOrbitSol->ns);
    //memset(pOrbitSol->iode, 0, sizeof(int32_t)*pOrbitSol->ns);
    memset(pOrbitSol->pToe, -1, sizeof(int32_t)*pOrbitSol->ObsNum);
    
    return 0;
}

extern int VeriQCFreeSolInfo(VERIQC_SOLINFO_T* pSol)
{
    if (!pSol)
    {
        return -1;
    }
    if (pSol->pTime)
    {
        VERIQC_FREE(pSol->pTime);
        pSol->pTime = NULL;
    }
    if (pSol->pSolState)
    {
        if (pSol->pSolState->pSatInfo)
        {
            if (pSol->pSolState->pSatInfo->pSatInfo)
            {
                VERIQC_FREE(pSol->pSolState->pSatInfo->pSatInfo);
                pSol->pSolState->pSatInfo->pSatInfo = NULL;
            }
            VERIQC_FREE(pSol->pSolState->pSatInfo);
            pSol->pSolState->pSatInfo = NULL;
        }
        VERIQC_FREE(pSol->pSolState);
        pSol->pSolState = NULL;
    }
    if (pSol->pPos)
    {
        VERIQC_FREE(pSol->pPos);
        pSol->pPos = NULL;
    }
    if (pSol->pVel)
    {
        VERIQC_FREE(pSol->pVel);
        pSol->pVel = NULL;
    }
    if (pSol->pUserData)
    {
        VERIQC_FREE(pSol->pUserData);
        pSol->pUserData = NULL;
    }
    return 0;
}

extern int VeriQCFreeOrbitSol(VERIQC_ORBIT_SOL_T* pSol)
{
    if (!pSol)
    {
        return -1;
    }
    pSol->ObsNum = 0;
    if (pSol->pSatNo)
    {
        VERIQC_FREE(pSol->pSatNo);
        pSol->pSatNo = NULL;
    }
    if (pSol->pSVH)
    {
        VERIQC_FREE(pSol->pSVH);
        pSol->pSVH = NULL;
    }
    if (pSol->pType)
    {
        VERIQC_FREE(pSol->pType);
        pSol->pType = NULL;
    }
    if (pSol->pExcludeSats)
    {
        VERIQC_FREE(pSol->pExcludeSats);
        pSol->pExcludeSats = NULL;
    }
    if (pSol->pRs)
    {
        VERIQC_FREE(pSol->pRs);
        pSol->pRs = NULL;
    }
    if (pSol->pDts)
    {
        VERIQC_FREE(pSol->pDts);
        pSol->pDts = NULL;
    }
    if (pSol->pVar)
    {
        VERIQC_FREE(pSol->pVar);
        pSol->pVar = NULL;
    }
    if (pSol->pIODE)
    {
        VERIQC_FREE(pSol->pIODE);
        pSol->pIODE = NULL;
    }
    if (pSol->pToe)
    {
        VERIQC_FREE(pSol->pToe);
        pSol->pToe = NULL;
    }
    return 0;
}

extern int VeriQCFreeObs(CHC_OBS_T* pObs)
{
    int i = 0;
    if(!pObs)
    {
        return -1;
    }
    if (pObs->pData)
    {
        VERIQC_FREE(pObs->pData);
        pObs->pData = NULL;
    }
    else
    {
        i += 0;
    }
    pObs->ObsNum = 0;
    pObs->ObsNumMax = 0;

    return 0;
}

extern int VeriQCSatSys(int Sat, int* pPRN)
{
    return SatSys(Sat, pPRN);
}

extern int VeriQCSatNo(int Sys, int PRN)
{
    return SatNo(Sys, PRN);
}


/********************************************************************************************
@brief   get system index by system id

@param   SystemID[In]          system id. SYS_???

@author  CHC

@return  system index, 0 based
@note    see GetSystemIDByIndex
*********************************************************************************************/
extern int GetSystemIndexByID(int SystemID)
{
    switch (SystemID)
    {
    case SYS_GPS:return 0;
    case SYS_CMP:return 1;
    case SYS_GLO:return 2;
    case SYS_GAL:return 3;
    case SYS_BD3:return 4;
    default:return -1;
    }
}

/********************************************************************************************
@brief   get system id by system index

@param   SysIndex[In]          system index, 0 based

@author  CHC

@return  SYS_???
@note    see GetSystemIndexByID
*********************************************************************************************/
extern int GetSystemIDByIndex(int SysIndex)
{
    const static unsigned int s_SystemIDMap[] = { SYS_GPS,SYS_CMP,SYS_GLO,SYS_GAL,SYS_BD3 };

    if (SysIndex >= SYSNUM)
    {
        return SYS_NONE;
    }

    return s_SystemIDMap[SysIndex];
}
