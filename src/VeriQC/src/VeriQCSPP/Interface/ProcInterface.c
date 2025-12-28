#include "ProcInterface.h"


#include "Logs.h"
#include "SPPBase.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"

#define OUTPUT_GNSSINTEGSOL 1
static FILE* glbFpSPPLog = NULL;
//-------------------------------------------------------------------------------------------------------//
//-------------------------------------------VERIQC2.0 interfaces----------------------------------------//
//-------------------------------------------------------------------------------------------------------//
VERIQC_SPPTC_PROCESSOR_T* VeriQCSPPTcCreate(VERIQC_SPPTC_CREATE_OPT_T* pOpt, struct _VERIQC_ORBIT_PROCESSOR_T* pOrbit);
int32_t VeriQCSPPTcRelease(VERIQC_SPPTC_PROCESSOR_T* pObj);
void VeriQCSPPTcSetOpt(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt, VERIQC_SPPTC_OPT_CTRL_E OptCtrl);
int32_t VeriQCSPPTcGetOpt(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt);
int32_t VeriQCSPPTcAddRangeRov(VERIQC_SPPTC_PROCESSOR_T* pObj, const RM_RANGE_LOG_T* pRangeLog);
int32_t VeriQCSPPTcStart(VERIQC_SPPTC_PROCESSOR_T* pObj);
int32_t VeriQCSPPTcSPP(VERIQC_SPPTC_PROCESSOR_T* pObj);
VERIQC_SOLINFO_T* VeriQCSPPTcGetSol(VERIQC_SPPTC_PROCESSOR_T* pObj);
VERIQC_ORBIT_SOL_T* VeriQCSPPTcGetSatPos(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_ANT_ID_E ID);

static VERIQC_SPPTC_IFX_T s_SPPTcFuncGlb =
{
    VeriQCSPPTcCreate,
    VeriQCSPPTcRelease,
    VeriQCSPPTcSetOpt,
    VeriQCSPPTcGetOpt,
    VeriQCSPPTcAddRangeRov,
    VeriQCSPPTcStart,
    VeriQCSPPTcSPP,
    VeriQCSPPTcGetSol,
    VeriQCSPPTcGetSatPos,
};

VERIQC_SPPTC_IFX_T* VeriQCGetSPPTcIfx()
{
    return &s_SPPTcFuncGlb;
}

int32_t VeriQCSPPTcCopyOption(const VERIQC_SPPTC_OPT_T* pSrc, VERIQC_SPPTC_OPT_T* pDst)
{
    if (!pSrc || !pDst)
    {
        return -1;
    }
    *pDst = *pSrc;    /**< Temporarily direct assignment, later modify to copy member by member as needed. */
    return 0;
}

/**************************************
* create spptc
* @param[in]    popt     Option information related only to creation.
* @return       spptc pointer
* @note         Pay attention to the number of filters and fixers; 
                ensure they are not invalid or exceed the maximum number VERIQC_MAX_FILTER
**************************************/
VERIQC_SPPTC_PROCESSOR_T* VeriQCSPPTcCreate(VERIQC_SPPTC_CREATE_OPT_T* pOpt, struct _VERIQC_ORBIT_PROCESSOR_T* pOrbit)
{
    int i = 0, memused = 0, memfree = 0, enfaster = 0;
    VERIQC_SPPTC_PROCESSOR_T* pSPPtc = NULL;
    VERIQC_SPP_IFX_T* sppifx = NULL;

    if (!pOpt || !pOrbit)
    {
        return NULL;
    }
    sppifx = VeriQCGetSPPIfx();
    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();
    pSPPtc = (VERIQC_SPPTC_PROCESSOR_T*)VERIQC_MALLOC(sizeof(VERIQC_SPPTC_PROCESSOR_T));
    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();
    memset(pSPPtc, 0, sizeof(VERIQC_SPPTC_PROCESSOR_T));
    VeriQCLogSetStreamFormat(LABEL_GNSS, VERIQC_LOG_STFMT_HHMMSS);

    VeriQCInitSPPTcOptDefault(&pSPPtc->Opt);
    pSPPtc->CreateOpt0 = *pOpt;

    /** create spp object */
    if (pSPPtc->CreateOpt0.NumSPP > 0)
    {
        pSPPtc->pSPPProc = sppifx->CreateFunc(0, 0, pSPPtc->CreateOpt0.NumSys, pSPPtc->CreateOpt0.NumFreq);
        memused = (int)VERIQC_GET_USEDMEM();
        memfree = (int)VERIQC_GET_FREEMEM();
    }

    /** Externally passed orbit object. */
    pSPPtc->pOrbitProc = pOrbit;

    /** create observation */
    pSPPtc->pObsRover = (CHC_OBS_T*)VERIQC_MALLOC(sizeof(CHC_OBS_T));
    memset(pSPPtc->pObsRover, 0, sizeof(CHC_OBS_T));
    pSPPtc->pObsBase = (CHC_OBS_T*)VERIQC_MALLOC(sizeof(CHC_OBS_T));
    memset(pSPPtc->pObsBase, 0, sizeof(CHC_OBS_T));

    pSPPtc->pObsuFasterBuff = (CHC_OBS_T*)VERIQC_MALLOC(sizeof(CHC_OBS_T));
    memset(pSPPtc->pObsuFasterBuff, 0, sizeof(CHC_OBS_T));
    pSPPtc->pObsBaseBuff = (CHC_OBS_T*)VERIQC_MALLOC(sizeof(CHC_OBS_T));
    memset(pSPPtc->pObsBaseBuff, 0, sizeof(CHC_OBS_T));
    pSPPtc->ObsuFasterBuffValidFlag = 0;
    pSPPtc->ObsBaseBuffValidFlag = 0;
    pSPPtc->PosRefBuffValidFlag = 0;

    /** Create satellite calculation results. */
    memset(&pSPPtc->OrbitSolRover, 0, sizeof(VERIQC_ORBIT_SOL_T));
    memset(&pSPPtc->OrbitSolBase, 0, sizeof(VERIQC_ORBIT_SOL_T));

    /** init sol */
    memset(&pSPPtc->Sol, 0, sizeof(VERIQC_SOLINFO_T));

    memused = (int)VERIQC_GET_USEDMEM();
    memfree = (int)VERIQC_GET_FREEMEM();
	
    /** Create the result object, move this from start to create to avoid frequent allocation and deallocation. */
    VeriQCMallocSolInfo(&pSPPtc->Sol, MAXSAT);
    /** Create intermediate result variables. */

	/** Set the options pointer for SPP. */
	sppifx->SetOptFunc(pSPPtc->pSPPProc, sppifx->GetSolveOptFunc(pSPPtc->pSPPProc));
	sppifx->SetSolFunc(pSPPtc->pSPPProc, sppifx->GetOneSolFunc(pSPPtc->pSPPProc));

    pSPPtc->TimeStatus = RM_TIME_STATUS_UNKNOWN; /* UNKNOWN */

#ifdef WIN32
    VeriQCLogOpen(LABEL_GNSS, NULL, VeriQCOutSPPLog);
    VeriQCLogSetLevelStdout(LABEL_GNSS, VERIQC_LOG_LEVEL_OFF);
    VeriQCLogSetLevelFile(LABEL_GNSS, VERIQC_LOG_LEVEL_TRACE);
#endif

    return pSPPtc;
}

/**************************************
* release spptc
* @param[in]    pobj     spptc pointer
* @return       0: success, -1: fail
* @note         
**************************************/
int32_t VeriQCSPPTcRelease(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    int i = 0;
    if (!pObj)
    {
        return -1;
    }
    VERIQC_SPP_IFX_T* sppifx = NULL;
    sppifx = VeriQCGetSPPIfx();
    
    sppifx->ReleaseFunc(&(pObj->pSPPProc));

    if (pObj->pObsRover)
    {
        VeriQCFreeObs(pObj->pObsRover);
        VERIQC_FREE(pObj->pObsRover);
    }
    if (pObj->pObsBase)
    {
        VeriQCFreeObs(pObj->pObsBase);
        VERIQC_FREE(pObj->pObsBase);
    }
    if (pObj->pObsuFasterBuff)
    {
        VeriQCFreeObs(pObj->pObsuFasterBuff);
        VERIQC_FREE(pObj->pObsuFasterBuff);
    }
    if (pObj->pObsBaseBuff)
    {
        VeriQCFreeObs(pObj->pObsBaseBuff);
        VERIQC_FREE(pObj->pObsBaseBuff);
    }
    VeriQCFreeSolInfo(&(pObj->Sol));
    VeriQCFreeOrbitSol(&(pObj->OrbitSolFaster));
    VeriQCFreeOrbitSol(&(pObj->OrbitSolRover));
    VeriQCFreeOrbitSol(&(pObj->OrbitSolBase));
	
    VERIQC_FREE(pObj);
    pObj = NULL;
    return 0;
}

static void VeriQCLogOutputPort4(const char* pMsg)
{
    int MsgLen = (int)strlen(pMsg);
    if (MsgLen > 1024)
    {
        char* pNewMsg = "ERROR: msg length large than 1024";
        /** option */
    }
    /** option */
}

/**************************************
* set option
* @param[in]    pObj     spptc pointer
* @param[in]    pOpt     Configuration options for solving
* @param[in]    OptCtrl  set either a specific option or all option
* @return       None
* @note         
**************************************/
void VeriQCSPPTcSetOpt(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt, VERIQC_SPPTC_OPT_CTRL_E OptCtrl)
{
	int i = 0;
    if (!pObj || !pOpt)
    {
        return;
    }
    VeriQCCopySPPTcOptCtrl(pOpt, &pObj->Opt, OptCtrl);

#ifdef WIN32
    VeriQCLogOpen(LABEL_GNSS, NULL, VeriQCOutSPPLog);
    VeriQCLogOpen(LABEL_VERIQC, NULL, VeriQCOutSPPLog);
#else
    VeriQCLogOpen(LABEL_GNSS, VeriQCLogOutputPort4, NULL);
    VeriQCLogOpen(LABEL_VERIQC, VeriQCLogOutputPort4, NULL);
#endif
    if(pOpt->LogLevel[0] != 0)
    {
        int log_level = (pOpt->LogLevel[0] > 0) ? pOpt->LogLevel[0] : VERIQC_LOG_LEVEL_OFF;
        VeriQCLogSetLevelStdout(LABEL_GNSS, log_level);
        VeriQCLogSetLevelStdout(LABEL_VERIQC, log_level);
    }
    if(pOpt->LogLevel[1] != 0)
    {
        int log_level = (pOpt->LogLevel[1] > 0) ? pOpt->LogLevel[1] : VERIQC_LOG_LEVEL_OFF;
        VeriQCLogSetLevelFile(LABEL_GNSS, log_level);
        VeriQCLogSetLevelFile(LABEL_VERIQC, log_level);
    }
}

/**************************************
* get option
* @param[in]    pobj     spptc pointer
* @param[out]   opt      option
* @return       0:success, -1:fail
* @note         
**************************************/
int32_t VeriQCSPPTcGetOpt(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt)
{
    if (!pObj || !pOpt)
    {
        return -1;
    }
    *pOpt = pObj->Opt;
    return 0;
}

VERIQC_ORBIT_SOL_T* VeriQCSPPTcGetSatPos(VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_ANT_ID_E ID)
{
	if (!pObj)
	{
		return NULL;
	}
	if (ID == VERIQC_ANT_ID_ROVER)
	{
		return  &pObj->OrbitSolRover;
	}
	else if (ID == VERIQC_ANT_ID_BASE)
	{
		return  &pObj->OrbitSolBase;
	}
	else
	{
		return NULL;
	}
}

/********************************************************************************************
@brief   add rover range log data to spptc processor

@param   pobj[In]               spptc processor
@param   pRangeLog[In]          range log data

@author  CHC

@return  0:OK, -1:failed
*********************************************************************************************/
int32_t VeriQCSPPTcAddRangeRov(VERIQC_SPPTC_PROCESSOR_T* pObj, const RM_RANGE_LOG_T* pRangeLog)
{
    double deltaTime;
    VERIQC_ORBIT_IFX_T* pOrbit;
    VERIQC_SPP_IFX_T* pSPPIfx;
    CHC_SPP_SOLVE_T* pSPPSolve;
    VERIQC_SPPTC_OPT_T opt = { 0 };
    CHC_NAV_T* pNav;
    QC_TIME_T time;
    int32_t satNum;
#ifdef VERIQC_TICKGET_ENABLE
    uint64_t stick, etick;
    uint32_t scpucycle, ecpucycle;
    stick = tickgetus();
    scpucycle = cpucycle();
#endif

    pOrbit = VeriQCGetOrbitIfx();
    pSPPIfx = VeriQCGetSPPIfx();
    pNav = pOrbit->GetNavFunc(pObj->pOrbitProc);
    pSPPSolve = pSPPIfx->GetSolveFunc(pObj->pSPPProc);

    if (!pObj || !pRangeLog)
    {
        return -1;
    }
    satNum = VeriQCGetNsatFromRange(pRangeLog);
    if (satNum <= 0 || satNum > CHC_MAXOBS)
    {
        return -1;
    }
    time = GPSTime2QCTime(pRangeLog->LogHeader.Week, pRangeLog->LogHeader.Milliseconds * 0.001);
    VeriQCLogSetTime(LABEL_GNSS, &time);
    deltaTime = time.Time - pSPPSolve->OnePseuResult.Sol.Time.Time +
                time.Sec - pSPPSolve->OnePseuResult.Sol.Time.Sec;

    if (pObj->ObsuFasterBuffValidFlag == 0 && fabs(deltaTime) > 0.0099)
    {
        pObj->pObsuFasterBuff->ObsNum = 0;

        if (pObj->pObsuFasterBuff->ObsNumMax < satNum)
        {
            /* clear memory */
            VeriQCFreeObs(pObj->pObsuFasterBuff);
            /* allocate memory */
            VeriQCMallocObs(pObj->pObsuFasterBuff, satNum);
        }

        /* convert to internal struct */
        pObj->TimeStatus = pRangeLog->LogHeader.TimeStatus;
        VeriQCConvertObsRange(&pObj->Opt, pRangeLog, pObj->pObsuFasterBuff, pNav->GLOFreqNum, VERIQC_ANT_ROVER);
#ifdef VERIQC_TICKGET_ENABLE
        etick = tickget();
        gnss_log_debug("convert_obs,nsat=%d,stick=%d,etick=%d,dtick=%d", pRangeLog->ObsNum, stick, etick, etick - stick);
#endif

        VeriQCSPPTcGetOpt(pObj, &opt);
        VeriQCSPPTcSetOpt(pObj, &opt, VERIQC_SPPTC_OPT_CTRL_POSREF);
#ifdef VERIQC_TICKGET_ENABLE
        etick = tickget();
        gnss_log_debug("genobsr,nsat=%d,stick=%d,etick=%d,dtick=%d", pobj->pobsr->n, stick, etick, etick - stick);
#endif

        if (pObj->pObsuFasterBuff->ObsNum > 0)
        {
            pObj->ObsuFasterBuffValidFlag = 1;
        }
    }
#ifdef VERIQC_TICKGET_ENABLE
    etick = tickgetus();
    ecpucycle = cpucycle();
    gnss_log_debug("T,%d,VERIQC_SPPTC_addobs_rov,nsat=%d,vflg=%d,n=%d,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,VERIQC_SPPTC_addobs_rov",
        taskget(), pRangeLog->ObsNum, pobj->pobsu_faster_buff_vflg, pobj->pobsu_faster_buff->n, stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
    gnss_log_debug("T,addobs_rov_time,int=%lld,deci=%.2lf", pRangeLog->LogHeader.Week, pRangeLog->LogHeader.Milliseconds*0.001);
#endif

    return 0;
}

static int32_t UpdatePosBaseInfo(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    VERIQC_ORBIT_IFX_T* porbifx = NULL;
    U1 incrementUpdateMark = 0;
    double tt;
    QC_TIME_T tmpt, time0 = { 0 };

    if (pObj->pObsBase->ObsNum <= 0)
    {
        return 0;
    }

    porbifx = VeriQCGetOrbitIfx();
    
    //check update obs time
    VeriQCCopyTimeFromSPP(&pObj->pObsBase->pData[0].Time, &tmpt);
    tt = QCTimeMinusTime(&tmpt, &pObj->OrbitSolBase.UpdateTime, NULL);
    if (fabs(tt) < 1E-6)
    {
        if (pObj->pObsBase->ObsNum == pObj->OrbitSolBase.ObsNum)
        {
            incrementUpdateMark = 1;
        }
    }
    
    if (!incrementUpdateMark)
    {
        if (pObj->OrbitSolBase.ObsNum >= pObj->pObsBase->ObsNum)
        {
            pObj->OrbitSolBase.UpdateTime = time0;
            VeriQCClearOrbitSol(&pObj->OrbitSolBase); /* only clear contents */
        }
        else
        {
            VeriQCFreeOrbitSol(&pObj->OrbitSolBase);
            VeriQCMallocOrbitSol(&pObj->OrbitSolBase, pObj->pObsBase->ObsNum);
        }
    }

    porbifx->SatPosFunc(pObj->pOrbitProc, pObj->pObsBase, &pObj->OrbitSolBase, incrementUpdateMark);
    
    return 1;
}
static int32_t UpdatePosXInfo(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    VERIQC_ORBIT_IFX_T* porbifx = NULL;

    /** Check if the ephemeris is updated, and recalculate the satellite position for the previous epoch. */
    porbifx = VeriQCGetOrbitIfx();
    /** update base station info */
    UpdatePosBaseInfo(pObj);

    return 1;
}

/**************************************
* Execute the start process to prepare for the actual solving
* @param[in]    pobj     spptc pointer
* @return       0:success, -1:fail
* @note         
**************************************/
int32_t VeriQCSPPTcStart(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    VERIQC_ORBIT_IFX_T* porbifx = NULL;
    int i;
    U1 encodeMark = 0;
    VERIQC_SPPTC_OPT_T pgiopt = { 0 };
    int32_t fastMark = 0;
    CHC_OBS_T *pobsu = NULL;
    VERIQC_ORBIT_SOL_T *orbsolu = NULL;
    CHC_NAV_T* pNav = NULL;
#ifdef VERIQC_TICKGET_ENABLE
    uint64_t stick, etick;
    uint32_t scpucycle, ecpucycle;
    stick = tickgetus();
    scpucycle = cpucycle();
#endif

    if (!pObj)
    {
        return -1;
    }

#ifdef VERIQC_TICKGET_ENABLE
    stick = tickget();
#endif
    VeriQCClearSolInfo(&pObj->Sol);       //new clear operation instead of the free/malloc method
    pObj->Sol.pTime->Time.Time = pObj->pObsuFasterBuff->Time.Time;
    pObj->Sol.pTime->Time.Sec = pObj->pObsuFasterBuff->Time.Sec;

    pobsu = pObj->pObsRover;
    orbsolu = &pObj->OrbitSolRover;

    //getroverobs
    if (pObj->ObsuFasterBuffValidFlag == 1)
    {
        if (pObj->pObsuFasterBuff->ObsNum > 0)
        {
            VeriQCCopyObs(pObj->pObsuFasterBuff, pobsu, 1);

            pObj->ObsuFasterBuffValidFlag = 0;
        }
        else
        {
            pObj->ObsuFasterBuffValidFlag = 0;
#ifdef VERIQC_TICKGET_ENABLE
            etick = tickget();
            gnss_log_warn("VERIQC_SPPTC_start,No Obs,vflg=%d,n=%d,stick=%d,etick=%d,dtick=%d", pobj->pobsu_faster_buff_vflg, pobj->pobsu_faster_buff->n, stick, etick, etick - stick);
#endif
            return -1;
        }
    }
    else
    {
#ifdef VERIQC_TICKGET_ENABLE
        etick = tickget();
        gnss_log_info("VERIQC_SPPTC_start,No Obs,vflg=%d,n=%d,stick=%d,etick=%d,dtick=%d", pobj->pobsu_faster_buff_vflg, pobj->pobsu_faster_buff->n, stick, etick, etick - stick);
#endif
        return -1;
    }

    porbifx = VeriQCGetOrbitIfx();
    /** update faster Obs/Sat position information */
    /** sort Obs */
    VeriQCSortObs(pobsu->pData, pobsu->ObsNum);
    /** calculate sat position */
    if (orbsolu->ObsNum >= pobsu->ObsNum)
    {
        VeriQCClearOrbitSol(orbsolu); /* only clear contents */
    }
    else
    {
        VeriQCFreeOrbitSol(orbsolu);
        VeriQCMallocOrbitSol(orbsolu, pobsu->ObsNum);
    }

    porbifx->SetEphTimeFunc(pObj->pOrbitProc, &pobsu->pData[0].Time);
    porbifx->SatPosFunc(pObj->pOrbitProc, pobsu, orbsolu,  0);
    
    if (pobsu->ObsNum <= 0)
    {
        fastMark = 0;
    }
    else
    {
        fastMark = 1;
    }

    if (1 == fastMark)
    {
        //get refinfo
        if (pObj->ObsBaseBuffValidFlag == 1 && pObj->pObsBaseBuff->ObsNum > 0)
        {
            double age = QCTimeDiff(&pObj->pObsBaseBuff->Time, &pObj->pObsBase->Time);
            if (age >= pObj->Opt.SimulateMaxAge)
            {
                VeriQCCopyObs(pObj->pObsBaseBuff, pObj->pObsBase, 1);
                VeriQCSortObs(pObj->pObsBase->pData, pObj->pObsBase->ObsNum);
            }
            pObj->ObsBaseBuffValidFlag = 0;
        }
        if (pObj->PosRefBuffValidFlag == 1 && pObj->RefMark == 0x03)
        {
            VeriQCSPPTcGetOpt(pObj, &pgiopt);
            for (i = 0; i < 3; i++)
            {
                pgiopt.BasePos[i] = pObj->PosRefBuff[i];
            }
            pgiopt.RefID = pObj->PosRefRefIDBuff;
            pObj->Sol.pSolState->RefID = (unsigned short)pObj->PosRefRefIDBuff;

            VeriQCSPPTcSetOpt(pObj, &pgiopt, VERIQC_SPPTC_OPT_CTRL_POSREF);
            VeriQCSPPTcSetOpt(pObj, &pgiopt, VERIQC_SPPTC_OPT_CTRL_REFID);
            pObj->RefMark = 0;  //reset
            pObj->PosRefBuffValidFlag = 0;//free
        }

    }

#ifdef VERIQC_TICKGET_ENABLE
    etick = tickgetus();
    ecpucycle = cpucycle();
    gnss_log_debug("T,%d,VERIQC_SPPTC_start,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,VERIQC_SPPTC_start",
        taskget(),stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
#endif
    return 0;
}

/**************************************
* go spp
* @param[in]    pobj     spptc pointer
* @return       0:success, -1:fail
* @note         
**************************************/
int32_t VeriQCSPPTcSPP(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    VERIQC_SPP_IFX_T* psppifx = NULL;
    VERIQC_ORBIT_IFX_T* porbifx = NULL;
    CHC_NAV_T* pnav = NULL;
    CHC_SPP_SOLVE_T* pSPPSolve = NULL;
    CHC_SSATLIST_T *pssat = NULL;
	int res=0;
    int msSecond = 0;
	static int indexsolinput = -1;

    int i = 0;
    CHC_SOL_T* psol = NULL, sol0 = { 0 };

#ifdef VERIQC_TICKGET_ENABLE
    uint64_t stick, etick;
    uint32_t scpucycle, ecpucycle;
    stick = tickgetus();
    scpucycle = cpucycle();
    addtaskstate(1, 0);
#endif
    if (!pObj)
    {
        return -1;
    }

    UpdatePosXInfo(pObj);

    psppifx = VeriQCGetSPPIfx();
    porbifx = VeriQCGetOrbitIfx();

    pnav = porbifx->GetNavFunc(pObj->pOrbitProc);
    pSPPSolve = psppifx->GetSolveFunc(pObj->pSPPProc);
    pssat = psppifx->GetSsatlistFunc(pObj->pSPPProc);

    UpdateSsatlist(pnav, &(pSPPSolve->Ssatlist), pObj->pObsRover->pData, pObj->pObsRover->ObsNum, pObj->pObsBase->pData, pObj->pObsBase->ObsNum);

    //spp
#ifdef VERIQC_TICKGET_ENABLE
	stick=tickget();
#endif
    psppifx->ProcessFunc(pObj->pSPPProc, pObj->pObsRover, pnav, &pObj->OrbitSolRover);
#ifdef VERIQC_TICKGET_ENABLE
	etick=tickget();
	gnss_log_debug("psppifx->process use %d ms", etick - stick);
#endif


#ifdef VERIQC_TICKGET_ENABLE
    etick = tickgetus();
    ecpucycle = cpucycle();
    gnss_log_trace("T,%d,VERIQC_SPPTC_spp,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,VERIQC_SPPTC_spprtd",
        taskget(),stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
#endif

	/** final solution */
    psol = psppifx->GetOneSolFunc(pObj->pSPPProc);//result
    
    pssat = psppifx->GetSsatlistFunc(pObj->pSPPProc);
    sol0 = *psol;
    VeriQCConvertSolToSolInfo(&sol0, pssat, &pObj->OrbitSolRover, &pObj->Sol);
    pObj->Sol.pSolState->Cutoff = (float)pSPPSolve->Opt.EleMin;
    pObj->Sol.pTime->TimeStatus = pObj->TimeStatus;

#ifdef VERIQC_TICKGET_ENABLE
    etick = tickgetus();
    ecpucycle = cpucycle();
    VERIQC_printf("T,%d,VERIQC_SPPTC_fast,stat=%d,n=%d,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,VERIQC_SPPTC_fast\r\n",
        taskget(), psol->stat, psol->ns, stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
#endif

    return 0;
}

/**************************************
* get spptc result
* @param[in]    pobj     spptc pointer
* @return       spptc result
* @note         
**************************************/
VERIQC_SOLINFO_T* VeriQCSPPTcGetSol(VERIQC_SPPTC_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return &pObj->Sol;
}

extern void VeriQCOutSPPLog(const char* pMsg)
{
	if (glbFpSPPLog == NULL) return;

	fprintf(glbFpSPPLog, "%s", pMsg);
	//fflush(glbFpSPPLog);
}


/* sort oneepoch observation data -------------------------------------------------------*/
extern int VeriQCSortObs(CHC_OBSD_T* pEpochData, int Num)
{
    //printf("Enter into %s\n", __FUNCTION__);
    return SortObsEx(pEpochData, Num);
}


