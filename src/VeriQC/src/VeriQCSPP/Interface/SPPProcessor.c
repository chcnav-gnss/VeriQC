

#include "Logs.h"
#include "ProcInterface.h"
#include "SPPBase.h"

VERIQC_SPP_PROCESSOR_T* VeriQCCreateSPPProcessor(uint8_t OptAlloc, uint8_t SolAlloc, int SysNum, int FreqNum);
void VeriQCReleaseSPPProcessor(VERIQC_SPP_PROCESSOR_T** ppObj);
void VeriQCSPPSetOpt(VERIQC_SPP_PROCESSOR_T* pObj, CHC_PRCOPT_T* pOpt);
CHC_PRCOPT_T* VeriQCSPPGetOpt(VERIQC_SPP_PROCESSOR_T* pObj);
int32_t VeriQCSPPProcess(VERIQC_SPP_PROCESSOR_T* pObj, CHC_OBS_T* pObs, CHC_NAV_T* pNav,  VERIQC_ORBIT_SOL_T* pOrbSol);
int32_t VeriQCSPPSetSol(VERIQC_SPP_PROCESSOR_T* pObj, CHC_SOL_T* pSol);
CHC_SOL_T* VeriQCSPPGetSol(VERIQC_SPP_PROCESSOR_T* pObj);
CHC_SPP_SOLVE_T* VeriQCGetSPPSolve(VERIQC_SPP_PROCESSOR_T* pObj);
CHC_SOL_T* VeriQCGetSPPSolveSol(VERIQC_SPP_PROCESSOR_T* pObj);
CHC_SSATLIST_T* VeriQCSPPGetSsatlist(VERIQC_SPP_PROCESSOR_T* pObj);
CHC_PRCOPT_T* VeriQCGetSPPSolveOpt(VERIQC_SPP_PROCESSOR_T* pObj);

static VERIQC_SPP_IFX_T s_SPPModuleGlb =
{
    VeriQCCreateSPPProcessor,
    VeriQCReleaseSPPProcessor,
    VeriQCSPPSetOpt,
    VeriQCSPPGetOpt,
    VeriQCSPPProcess,
    VeriQCSPPSetSol,
    VeriQCSPPGetSol,
    VeriQCGetSPPSolve,
    VeriQCGetSPPSolveSol,
    VeriQCSPPGetSsatlist,
    VeriQCGetSPPSolveOpt,
};

VERIQC_SPP_IFX_T* VeriQCGetSPPIfx()
{
    return &s_SPPModuleGlb;
}

U1 InitOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult, int Num)
{
    size_t mTotal = 0;
    VERIQC_MEM_MANAGER_T* memOnePseu = &(pOnePseuResult->MemManager);

    if (!pOnePseuResult || Num <= 0)
    {
        return 0;
    }

    memset(pOnePseuResult, 0, sizeof(CHC_ONE_PSEURESULT_T));
    VeriQCMemManagerInit(memOnePseu);
    mTotal = Num * sizeof(CHC_OBSD_T) + 9 * Num * sizeof(double) + 5 * Num * sizeof(int) + Num * sizeof(U1);
    VeriQCAddSize(memOnePseu, mTotal, 0);
    if (VeriQCAllocateWhole(memOnePseu) < 0)
    {
        return 0;
    }

    pOnePseuResult->pObs = (CHC_OBSD_T*)VeriQCRequestMemory(memOnePseu, Num * sizeof(CHC_OBSD_T), 0);
    pOnePseuResult->pRs = (double*)VeriQCRequestMemory(memOnePseu, 6 * Num * sizeof(double), 0);
    pOnePseuResult->pDts = (double*)VeriQCRequestMemory(memOnePseu, 2 * Num * sizeof(double), 0);
    pOnePseuResult->pVar = (double*)VeriQCRequestMemory(memOnePseu, Num * sizeof(double), 0);
    pOnePseuResult->pToeSec = (int*)VeriQCRequestMemory(memOnePseu, Num * sizeof(int), 0);
    pOnePseuResult->pSatIndex = (int*)VeriQCRequestMemory(memOnePseu, Num * sizeof(int), 0);
    pOnePseuResult->pSat = (int*)VeriQCRequestMemory(memOnePseu, Num * sizeof(int), 0);
    pOnePseuResult->pIODE = (int*)VeriQCRequestMemory(memOnePseu, Num * sizeof(int), 0);
    pOnePseuResult->pEphType = (U1*)VeriQCRequestMemory(memOnePseu, Num * sizeof(U1), 0);
    pOnePseuResult->MaxNum = Num;

    return 1;
}
U1 ExpandOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult, int AddNum, U1 KeepFlag)
{
    int n0 = pOnePseuResult->MaxNum, newOnePseuNum = n0 + AddNum;
    size_t mTotal = 0;
    VERIQC_MEM_MANAGER_T memOnePseu = { 0 };
    CHC_OBSD_T* obs;
    double* rs, * dts, * var;
    int* toe_sec, * iu, * sat, * iode;
    U1* ephtype;

    if (AddNum <= 0 || newOnePseuNum > 2 * MAXOBS) return 0;
    mTotal = newOnePseuNum * sizeof(CHC_OBSD_T) + 9 * newOnePseuNum * sizeof(double) + 5 * newOnePseuNum * sizeof(int) + newOnePseuNum * sizeof(U1);
    VeriQCAddSize(&memOnePseu, mTotal, 0);
    if (VeriQCAllocateWhole(&memOnePseu) < 0)
    {
        return 0;
    }
    if (!KeepFlag)
    {
        VeriQCMemManagerFree(&(pOnePseuResult->MemManager));
        pOnePseuResult->pObs = (CHC_OBSD_T*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(CHC_OBSD_T), 0);
        pOnePseuResult->pRs = (double*)VeriQCRequestMemory(&memOnePseu, 6 * newOnePseuNum * sizeof(double), 0);
        pOnePseuResult->pDts = (double*)VeriQCRequestMemory(&memOnePseu, 2 * newOnePseuNum * sizeof(double), 0);
        pOnePseuResult->pVar = (double*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(double), 0);
        pOnePseuResult->pToeSec = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        pOnePseuResult->pSatIndex = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        pOnePseuResult->pSat = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        pOnePseuResult->pIODE = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        pOnePseuResult->pEphType = (U1*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(U1), 0);
    }
    else
    {
        obs = (CHC_OBSD_T*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(CHC_OBSD_T), 0);
        memcpy(obs, pOnePseuResult->pObs, sizeof(CHC_OBSD_T) * n0); pOnePseuResult->pObs = obs;
        rs = (double*)VeriQCRequestMemory(&memOnePseu, 6 * newOnePseuNum * sizeof(double), 0);
        memcpy(rs, pOnePseuResult->pRs, 6 * n0 * sizeof(double)); pOnePseuResult->pRs = rs;
        dts = (double*)VeriQCRequestMemory(&memOnePseu, 2 * newOnePseuNum * sizeof(double), 0);
        memcpy(dts, pOnePseuResult->pDts, 2 * n0 * sizeof(double)); pOnePseuResult->pDts = dts;
        var = (double*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(double), 0);
        memcpy(var, pOnePseuResult->pVar, n0 * sizeof(double)); pOnePseuResult->pVar = var;
        toe_sec = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        memcpy(toe_sec, pOnePseuResult->pToeSec, n0 * sizeof(int)); pOnePseuResult->pToeSec = toe_sec;
        iu = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        memcpy(iu, pOnePseuResult->pSatIndex, n0 * sizeof(int)); pOnePseuResult->pSatIndex = iu;
        sat = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        memcpy(sat, pOnePseuResult->pSat, n0 * sizeof(int)); pOnePseuResult->pSat = sat;
        iode = (int*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(int), 0);
        memcpy(iode, pOnePseuResult->pIODE, n0 * sizeof(int)); pOnePseuResult->pIODE = iode;
        ephtype = (U1*)VeriQCRequestMemory(&memOnePseu, newOnePseuNum * sizeof(U1), 0);
        memcpy(ephtype, pOnePseuResult->pEphType, n0 * sizeof(U1)); pOnePseuResult->pEphType = ephtype;

        VeriQCMemManagerFree(&(pOnePseuResult->MemManager));
    }
    pOnePseuResult->MemManager = memOnePseu;
    pOnePseuResult->MaxNum = newOnePseuNum;

    return 1;
}
U1 EndOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult)
{
    if (!pOnePseuResult) return 0;
    VeriQCMemManagerFree(&pOnePseuResult->MemManager);
    pOnePseuResult->AllObsNum = 0;
    pOnePseuResult->RoverObsNum = 0;
    pOnePseuResult->CommonSatNum = 0;
    pOnePseuResult->MaxNum = 0;
    return 1;
}

U1 InitSPPSolve(CHC_SPP_SOLVE_T* pSPPSolve, int SysNum, int FreqNum)
{
    if (!pSPPSolve) return 0;

    memset(pSPPSolve, 0, sizeof(CHC_SPP_SOLVE_T));

    InitOnePseuResult(&pSPPSolve->OnePseuResult, 70);
    
    InitSsatlist(&pSPPSolve->Ssatlist, (VERIQC_FLAG == 2 ? 35 : 40));

    InitialOptDefault(&g_PrcOptDefault);
    pSPPSolve->Opt = g_PrcOptDefault;

    return 1;
}
U1 EndSPPSolve(CHC_SPP_SOLVE_T* pSPPSolve)
{
    if (!pSPPSolve) return 0;

    EndOnePseuResult(&pSPPSolve->OnePseuResult);

    EndSsatlist(&pSPPSolve->Ssatlist);

    VERIQC_FREE(pSPPSolve);
    pSPPSolve = NULL;

    return 1;
}

void VeriQCSPPConstruct(VERIQC_SPP_PROCESSOR_T* pObj, uint8_t OptAlloc, uint8_t SolAlloc, int SysNum, int FreqNum)
{
    if (!pObj)
    {
        return;
    }
	pObj->OptAlloc = OptAlloc;
	pObj->SolAlloc = SolAlloc;
	pObj->pOpt = NULL;
	pObj->pSol = NULL;
	if (OptAlloc)
	{
		pObj->pOpt = VERIQC_MALLOC(sizeof(CHC_PRCOPT_T));
		memset(pObj->pOpt, 0, sizeof(CHC_PRCOPT_T));
		InitialOptDefault(pObj->pOpt);
	}
	if (SolAlloc)
	{
		pObj->pSol = VERIQC_MALLOC(sizeof(CHC_SOL_T));
		memset(pObj->pSol, 0, sizeof(CHC_SOL_T));
	}

    pObj->pSPP = (CHC_SPP_SOLVE_T*)VERIQC_MALLOC(sizeof(CHC_SPP_SOLVE_T));

    InitSPPSolve(pObj->pSPP, SysNum, FreqNum);
}

void VeriQCSPPDestruct(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return;
    }
	if (pObj->OptAlloc)
	{
		if (pObj->pOpt)
		{
			VERIQC_FREE(pObj->pOpt);
		}
	}
	if (pObj->SolAlloc)
	{
		if (pObj->pSol)
		{
            VERIQC_FREE(pObj->pSol);
		}
	}
	pObj->OptAlloc = 0;
	pObj->SolAlloc = 0;
	pObj->pOpt = NULL;
	pObj->pSol = NULL;

    EndSPPSolve(pObj->pSPP);
}

CHC_SPP_SOLVE_T* VeriQCGetSPPSolve(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return pObj->pSPP;
}

CHC_SOL_T* VeriQCGetSPPSolveSol(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return &pObj->pSPP->OnePseuResult.Sol;
}

CHC_SSATLIST_T* VeriQCSPPGetSsatlist(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return &pObj->pSPP->Ssatlist;
}

CHC_PRCOPT_T* VeriQCGetSPPSolveOpt(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return &pObj->pSPP->Opt;
}

VERIQC_SPP_PROCESSOR_T* VeriQCCreateSPPProcessor(uint8_t OptAlloc, uint8_t SolAlloc, int SysNum, int FreqNum)
{
    VERIQC_SPP_PROCESSOR_T* pobj = NULL;
    pobj = VERIQC_MALLOC(sizeof(VERIQC_SPP_PROCESSOR_T));
    if (!pobj)
        return NULL;
    VeriQCSPPConstruct(pobj, OptAlloc, SolAlloc, SysNum, FreqNum);
    return pobj;
}

void VeriQCReleaseSPPProcessor(VERIQC_SPP_PROCESSOR_T** ppObj)
{
    if (ppObj)
    {
        if (*ppObj)
        {
            VeriQCSPPDestruct(*ppObj);
            VERIQC_FREE(*ppObj);
            *ppObj = NULL;
        }
    }
}

void VeriQCSPPSetOpt(VERIQC_SPP_PROCESSOR_T* pObj, CHC_PRCOPT_T* pOpt)
{
    if (!pObj || !pOpt)
    {
        return;
    }
	if (pObj->OptAlloc)
	{
		memcpy(pObj->pOpt, pOpt, sizeof(CHC_PRCOPT_T));
	}
	else
	{
		pObj->pOpt = pOpt;
	}
}

CHC_PRCOPT_T* VeriQCSPPGetOpt(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return pObj->pOpt;
}


int32_t VeriQCSPPProcess(VERIQC_SPP_PROCESSOR_T* pSPPObj, CHC_OBS_T* pObsPack, CHC_NAV_T* pNav,
    VERIQC_ORBIT_SOL_T* pSatPos)
{
    unsigned int iObs;
    unsigned int iSat;
    unsigned int iPos;
    unsigned int iDt;
    unsigned int iSatPosVel, iSatClk;

    unsigned int obsIndexX6, obsIndexX2;
    unsigned int obsCount;
    unsigned int SatParamOffset;
    unsigned int obsNum;

    unsigned char msg[256];
    CHC_OBSD_T * pOneSatObs = NULL;
	double timeDiff, ttspp, dt = 0.0;
    
    U1 tooShortFlag = 1;
    CHC_SSAT_T* pSatStatus;

    BOOL bRoughEpoch = FALSE;
    double deducePos[3];
    int SatID;
    CHC_SSAT_T* pSsat;
    CHC_SOL_T * pSPPSol;
    CHC_XSIMSOL_T * pSPPSolSimple;
    CHC_ONE_PSEURESULT_T * pPSRResult;
    CHC_SPP_SOLVE_T* pSPPSolve;
    unsigned int fLoop;
    unsigned short index;
    const double shortDatumUpdateLimit = ((0 == VERIQC_FLAG) ? 0 : ((1 == VERIQC_FLAG) ? 1 : 600));

#ifdef _DEBUG
    double ep[6] = { 0 };
#endif
#ifdef VERIQC_TICKGET_ENABLE
    uint32_t SPPTickBegin;
    uint32_t SPPTickCost;
#endif
    TIMECOST_BEGIN_AS(SPPTickBegin);

    if (!pSPPObj || !pObsPack || !pObsPack->pData || !pNav || !pSatPos)
    {
        return -1;
    }

    pSPPSolve = pSPPObj->pSPP;
    obsNum = pObsPack->ObsNum;
    pPSRResult = &pSPPSolve->OnePseuResult;
    pSPPSol = &pPSRResult->Sol;
    pSPPSolSimple = &pSPPSol->XSimSol;

    pSPPSol->Age = 0.0f;
    pSPPSol->AgeOk = 1;
    pSPPSol->SolStat = SOL_STATUS_INSUFFICIENT_OBS;
    if (obsNum <= 0)
    {
        GNSS_LOG_ERROR("No rover observation data!");
        pSPPSol->Time = pObsPack->Time;
        pSPPSol->SolStat = SOL_STATUS_INSUFFICIENT_OBS;
        pSPPSol->Stat = SOLQ_NONE;
        return SOLQ_NONE;
    }
#ifdef _DEBUG
    QCTime2Epoch(pObsPack->Time, ep);
	if (ep[3] == 4 && ep[4] == 37 && fabs(ep[5] - 36.2) < 1.0E-4)
    {
        ep[3] += 0.0;
    }
#endif

    /*to avoid time error*/
    timeDiff = QCTimeDiff(&pObsPack->Time, &pSPPObj->pSol->Time);

    if ((pSPPObj->pSol->Stat != SOLQ_NONE)
        && (IsQCTimeSame(pObsPack->Time, pSPPObj->pSol->Time) || ((pSPPObj->pOpt->SolType == 0) && (timeDiff < 0.0)) || ((pSPPObj->pOpt->SolType == 1) && (timeDiff > 0.0)))
        )
    {
        pSPPSol->Stat = SOLQ_NONE;
        pSPPSol->SolStat = SOL_STATUS_REPEATED_OBS;
        pSPPSol->SatNum = 0;
        pSPPSol->Time = pObsPack->pData[0].Time;
        return -1;
    }
    
    pSPPSol->Stat = SOLQ_NONE;


    pSPPSol->SatNum = 0;
    pSPPSol->ARSatNum = 0; 
    pSatStatus = &pSPPSolve->Ssatlist.pSsat[0];
    for (iSat = 0; iSat < pSPPSolve->Ssatlist.NumMax; iSat++)
    {/* reset satellite usage mark */
        pSatStatus->ValidSatFlagSingle = SATVIS_INVISIBLE;
        pSatStatus->UsedFrequencyMark = 0;
        pSatStatus->SatStatus = SAT_STATUS_NOTUSED;
        pSatStatus->AzEl[0] = 0.0;
        pSatStatus->AzEl[1] = 0.0;
        pSatStatus++;
    }

    memset(pSPPSol->Rr, 0, sizeof(double) * 3);
    memset(pSPPSol->Dtr, 0, sizeof(double) * 6);
    memset(pSPPSol->Dop, 0, sizeof(double) * 5);    /**<@todo use macro*/
    memset(pPSRResult->Invalid, 0, sizeof(unsigned char) * MAXSAT);

    GNSS_LOG_DEBUG("spp rovertime=%f,nsat=%d", pObsPack->Time.Time + pObsPack->Time.Sec, obsNum);

    pSPPObj->pSPP->TimeDiff = timeDiff;
    pPSRResult->AllObsNum = 0;
    pPSRResult->RoverObsNum = 0;
    
    obsCount = 0;
    for (iObs = 0; iObs < obsNum; iObs++)
    {
        unsigned int obsCountX6, obsCountX2;
        SatID = pObsPack->pData[iObs].Sat;
        if (0 == SatID)
        {
            continue;
        }
        pSsat = pSPPSolve->Ssatlist.pSsat + pSPPSolve->Ssatlist.Index[SatID - 1];
        pSsat->ValidSatFlagSingle = SATVIS_TRACKING;

        for (fLoop = 0; fLoop < NFREQ; fLoop++)
        {
            if (pObsPack->pData[iObs].ObsValidMask & BIT_MASK_SNR(fLoop))
            {
                index = (unsigned short)pObsPack->pData[iObs].Index[fLoop];
                pSsat->SNR[fLoop] = (unsigned char)(pObsPack->pData[iObs].SNR[index] * 4 + 0.5);
            }
        }

        if (pSatPos->pSVH[iObs] != 0)
        {
            pSsat->SatStatus = SAT_STATUS_BADHEALTH;
            continue;
        }
        if (pPSRResult->AllObsNum >= pPSRResult->MaxNum)
        {
            if (!ExpandOnePseuResult(pPSRResult, obsNum - pPSRResult->MaxNum + 2, 1))
            {
                break;
            }
        }
        pPSRResult->pObs[obsCount] = pObsPack->pData[iObs];
        obsIndexX6 = iObs * 6;
        obsCountX6 = obsCount * 6;
        for (iSatPosVel = 0; iSatPosVel < 6; iSatPosVel++)
        {
            SatParamOffset = obsIndexX6 + iSatPosVel;
            pPSRResult->pRs[obsCountX6 + iSatPosVel] = pSatPos->pRs[SatParamOffset];
        }
        obsIndexX2 = iObs * 2;
        obsCountX2 = obsCount * 2;
        for (iSatClk = 0; iSatClk < 2; iSatClk++)
        {
            SatParamOffset = obsIndexX2 + iSatClk;
            pPSRResult->pDts[obsCountX2 + iSatClk] = pSatPos->pDts[SatParamOffset];
        }
        pPSRResult->pVar[obsCount] = pSatPos->pVar[iObs];
        pPSRResult->pIODE[obsCount] = pSatPos->pIODE[iObs];
        pPSRResult->pEphType[obsCount] = pSatPos->pType[iObs];
        pPSRResult->pToeSec[obsCount] = pSatPos->pToe[iObs];
        pPSRResult->AllObsNum += 1;
        if (STATION_ROVER == pObsPack->pData[iObs].AntID)
        {
            pPSRResult->pSat[obsCount] = SatID;
            pPSRResult->pSatIndex[obsCount] = obsCount;
            obsCount++;
        }
        /*gnss_log_trace("spp obs[%d], sat=%d, rs=%f,%f,%f, dts=%f,%f, L=%f,%f, P=%f,%f", obsNum,
            pobs->data[i].sat, porbsol->rs[i * 6], porbsol->rs[i * 6 + 1], porbsol->rs[i * 6 + 2],
            porbsol->dts[i * 2], porbsol->dts[i * 2 + 1], pobs->data[i].L[0], pobs->data[i].L[1],
            pobs->data[i].P[0], pobs->data[i].P[1]);*/
    }
    pPSRResult->RoverObsNum = obsCount;

    pOneSatObs = pPSRResult->pObs;
    obsNum = pPSRResult->RoverObsNum;
    SettleObs(pOneSatObs, obsNum); /**@todo large loop?  how to avoid?*/
    ttspp = QCTimeDiff(&pOneSatObs[0].Time, &pSPPSol->SPPSatUpdateTime); /**@todo tooooo many time diff*/
    if (1)/**< fabs(ttspp) > pSPPSolve->opt.sppgap */
    {
        #ifdef VERIQC_TICKGET_ENABLE
        addtaskstate(1, 1); /**<@todo what for ?*/
        #endif

        TIMECOST_BEGIN();
        PNTPosExNew(pOneSatObs, obsNum, pNav, &pSPPSolve->Opt, &pPSRResult->Sol, (char*)msg, pPSRResult->pRs, pPSRResult->pDts,
            pPSRResult->pVar, &pSPPSolve->Ssatlist);
        tooShortFlag = 0;

        pSPPSolve->PureSPPRms[0] = pSPPSol->RMS;
        pSPPSolve->PureSPPRms[1] = pSPPSol->PosRms;

        memcpy(pSPPSolve->PureSPPVarCoor, pSPPSol->Qr,   sizeof(double) * 6);
        memcpy(pSPPSolve->PureSPPVarVel,  pSPPSol->QVel, sizeof(double) * 6);
        TIMECOST_END_PRINT("pntpos_ex", TIME_COST_NULL_EX_PRINT);

    }

    /* save pgistate_tu->pos/dpos in xsimsol first */
    timeDiff = QCTimeDiff(&pOneSatObs[0].Time, &pSPPSolSimple->SolTime);
    for (iPos = 0; iPos < 3; iPos++)
    {
        deducePos[iPos] = pSPPSolSimple->Rr[iPos] + pSPPSolSimple->Rr[3 + iPos] * timeDiff;
    }

    UpdateSatAzEl(&pSPPSolve->Ssatlist, pPSRResult->pObs, obsCount, pPSRResult->pRs, deducePos);

    /** TDP is failed and in SPP gap, do pntpos this epoch */
    /*if ((tooShortFlag))
    {
        TIMECOST_BEGIN();
        pntpos_ex_new(pOneSatObs, obsNum, pNav, &pSPPSolve->opt, &pPSRResult->sol, (char*)msg, pPSRResult->rs, pPSRResult->dts,
            pPSRResult->var, &pSPPSolve->ssatlist);
        tooShortFlag = 0;
        pSPPSolve->pureSPPRms[0] = pSPPSol->rms;
        pSPPSolve->pureSPPRms[1] = pSPPSol->posrms;
        memcpy(pSPPSolve->pureSPPVarCoor, pSPPSol->qr, sizeof(double) * 6);
        memcpy(pSPPSolve->pureSPPVarVel, pSPPSol->qvel, sizeof(double) * 6);
        TIMECOST_END_PRINT("pntpos_ex", TIME_COST_NULL_EX_PRINT);
    }*/

    if (SOLQ_SINGLE == pSPPSol->Stat)
    {
        GNSS_LOG_INFO("pntpos:%.4f %.4f %.4f", pSPPSol->Rr[0],pSPPSol->Rr[1], pSPPSol->Rr[2]);

        //contain infer in pntpos_ex_new,when ns <3 ,set sol stat to SOLQ_NONE
        if (pSPPSol->SatNum < 3)
        {
            pSPPSol->Stat = SOLQ_NONE;
        }
        if (SOLQ_SINGLE == pSPPSol->Stat)
        {
            GetErrPseuSatBySPP(&pSPPSolve->Ssatlist, pOneSatObs, obsNum, pPSRResult->pRs, pPSRResult->pDts, pNav, &(pSPPSolve->Opt),
                pSPPSol->Rr, pPSRResult->Invalid, NULL, 100.0);
        }
    }
	
    if (!pSPPSolve->Opt.EnableFaster)
    {
        GetClkBk(&pSPPSolve->Opt, &pPSRResult->Sol, &(pSPPSolve->Ssatlist), pOneSatObs, pPSRResult->pRs, pPSRResult->pDts, obsNum, pNav, 0);
        GetSatOutInfo(&pSPPSolve->Opt, pOneSatObs, obsNum, pPSRResult->pRs, &(pSPPSolve->Ssatlist), pSPPSol->Dop);
    }

    if (pSPPObj->pSol != &pPSRResult->Sol)
    {
        *pSPPObj->pSol = pPSRResult->Sol;
    }
    for (iDt = 0; iDt < 6; iDt++)
    {
        if (fabs(pSPPSol->Dtr[iDt]) > 1E-12)
        {
            GNSS_LOG_DEBUG("clk=%f,clk_shift=%f,stat=%d", 
                pSPPSol->Dtr[iDt] * CLIGHT, pSPPSol->DtrDot[iDt] * CLIGHT,
                pSPPSol->Stat);
            break;
        }
    }

    TIMECOST_END_AS_PRINT(SPPTickBegin, SPPTickCost, "SPP", "T %d, a %u", taskget(), !tooShortFlag);
    return 0;
}

int32_t VeriQCSPPSetSol(VERIQC_SPP_PROCESSOR_T* pObj, CHC_SOL_T* pSol)
{
	if (!pObj || !pSol)
	{
		return -1;
	}
	if (pObj->SolAlloc)
	{
		*pObj->pSol = *pSol;
	}
	else
	{
		pObj->pSol = pSol;
	}
	return 0;
}

CHC_SOL_T* VeriQCSPPGetSol(VERIQC_SPP_PROCESSOR_T* pObj)
{
    if (!pObj)
    {
        return NULL;
    }
    return pObj->pSol;
}
