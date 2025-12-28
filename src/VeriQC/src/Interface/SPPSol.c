/*************************************************************************//**
		 VeriQC

	Interface Module
 *-
@file   SPPSol.c
@author CHC
@date  2 March 2023
@brief  Some Algorithm common interfaces about SPP
*****************************************************************************/

#include "SPPSol.h"

static VERIQC_SIMPLIFY_RESULT_T* g_pResult[QC_MAX_NUM_OBS_FILE] = { NULL,NULL };
struct _VERIQC_SPPTC_PROCESSOR_T* g_pSPPTC[QC_MAX_NUM_OBS_FILE] = { NULL,NULL };
struct _VERIQC_ORBIT_PROCESSOR_T* g_pOrbit[QC_MAX_NUM_OBS_FILE] = { NULL,NULL };

/** some functions from VERIQC */
void QCSPPTcDefault(VERIQC_SPPTC_CREATE_OPT_T* pOpt0)
{
	pOpt0->NumSPP = 1;
	pOpt0->NumSys = 4;
	pOpt0->NumFreq = 5;
	pOpt0->NumSPPSol = 0;
	pOpt0->PosMode = 0;
}

extern void InitRTKCore(double EleMinDeg)
{
	int iProcess;
	VERIQC_SPPTC_IFX_T* spptc_ifx = VeriQCGetSPPTcIfx();
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	VERIQC_SPPTC_CREATE_OPT_T createOpt = { 0 };
	VERIQC_SPPTC_OPT_T processOpt = { 0 };

	InitRTKRes();

	QCSPPTcDefault(&createOpt);

	for (iProcess = 0; iProcess < QC_MAX_NUM_OBS_FILE; iProcess++)
	{
		g_pOrbit[iProcess] = orbit_ifx->CreateFunc();
		g_pSPPTC[iProcess] = spptc_ifx->CreateFunc(&createOpt, g_pOrbit[iProcess]);
		spptc_ifx->GetOptFunc(g_pSPPTC[iProcess], &processOpt);
		processOpt.SPPGap = 0.0;
		processOpt.SatNumLimit = 0;
		processOpt.MaxCPUTimeMsSatPara = 86400000;
		processOpt.EnableFaster = 0;
		processOpt.RealTimeMode = 0;
		processOpt.SysSignal[1] = VERIQC_B1I | VERIQC_B2I | VERIQC_B3I | VERIQC_B1C | VERIQC_B2A;
		processOpt.EleMin = EleMinDeg;
		processOpt.PosMode = 0;
		spptc_ifx->SetOptFunc(g_pSPPTC[iProcess], &processOpt, VERIQC_SPPTC_OPT_CTRL_ALL);
		orbit_ifx->SetOptFunc(g_pOrbit[iProcess], &processOpt, VERIQC_SPPTC_OPT_CTRL_ALL);
	}
}

extern void EndRTKCore()
{
	VERIQC_SPPTC_IFX_T* spptc_ifx = VeriQCGetSPPTcIfx();
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	int iProcess;

	for (iProcess = 0; iProcess < QC_MAX_NUM_OBS_FILE; iProcess++)
	{
		spptc_ifx->ReleaseFunc(g_pSPPTC[iProcess]); 
		g_pSPPTC[iProcess] = NULL;
		orbit_ifx->ReleaseFunc(g_pOrbit[iProcess]); 
		g_pOrbit[iProcess] = NULL;
		if (g_pResult[iProcess]->pSVH)
		{
			free(g_pResult[iProcess]->pSVH);
			g_pResult[iProcess]->pSVH = NULL;
		}
		if (g_pResult[iProcess]->pDts)
		{
			free(g_pResult[iProcess]->pDts);
			g_pResult[iProcess]->pDts = NULL;
		}
		if (g_pResult[iProcess]->pSatCoors)
		{
			free(g_pResult[iProcess]->pSatCoors); 
			g_pResult[iProcess]->pSatCoors = NULL;
		}
		if (g_pResult[iProcess]->pSatNo)
		{
			free(g_pResult[iProcess]->pSatNo);
			g_pResult[iProcess]->pSatNo = NULL;
		}
		if (g_pResult[iProcess])
		{
			free(g_pResult[iProcess]); 
			g_pResult[iProcess] = NULL;
		}
	}
}

/**********************************************************************//**
@brief add GPS/GAL/BDS/QZS ephemeris to VERIQC

@param pEPH        [In]     GPS/GAL/BDS/QZS ephemeris

@author CHC
@date
@note
History:
1. 2024/07/25 use UNI_EPHEMERIS_T instead
**************************************************************************/
extern void AddEphSPPCore(const UNI_EPHEMERIS_T* pEPH)
{
	int iProcess;
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	if (!pEPH)
	{
		return;
	}

	for (iProcess = 0; iProcess < QC_MAX_NUM_OBS_FILE; iProcess++)
	{
		if (g_pOrbit[iProcess])
		{
			orbit_ifx->AddUniEphemerisFunc(g_pOrbit[iProcess], pEPH);
		}
	}
}

/**********************************************************************//**
@brief add GLO ephemeris to VERIQC

@param pGEPH        [In]     GLO ephemeris

@author CHC
@date
@note
History:
1. 2024/07/25 use GLO_EPHEMERIS_T instead
**************************************************************************/
extern void AddGephSPPCore(const GLO_EPHEMERIS_T* pGEPH)
{
	int iProcess;
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	if (!pGEPH)
	{
		return;
	}

	for (iProcess = 0; iProcess < QC_MAX_NUM_OBS_FILE; iProcess++)
	{
		if (g_pOrbit[iProcess])
		{
			orbit_ifx->AddGLOEphemerisFunc(g_pOrbit[iProcess], pGEPH);
		}
	}
}


extern void InitRTKRes()
{
	int iProcess;
	for (iProcess = 0; iProcess < QC_MAX_NUM_OBS_FILE; iProcess++)
	{
		g_pResult[iProcess] = (VERIQC_SIMPLIFY_RESULT_T*)malloc(sizeof(VERIQC_SIMPLIFY_RESULT_T));
		memset(g_pResult[iProcess], 0, sizeof(VERIQC_SIMPLIFY_RESULT_T));
		g_pResult[iProcess]->NumMax = QC_MAX_NUM_SAT;

		g_pResult[iProcess]->pSatCoors = (double*)malloc(sizeof(double) * QC_MAX_NUM_SAT * 6);
		memset(g_pResult[iProcess]->pSatCoors, 0, sizeof(double) * QC_MAX_NUM_SAT * 6);

		g_pResult[iProcess]->pSatNo = (int16_t*)malloc(sizeof(int16_t) * QC_MAX_NUM_SAT);
		memset(g_pResult[iProcess]->pSatNo, 0, sizeof(int16_t) * QC_MAX_NUM_SAT);

		g_pResult[iProcess]->pDts= (double*)malloc(sizeof(double) * QC_MAX_NUM_SAT *2);
		memset(g_pResult[iProcess]->pDts, 0, sizeof(double) * QC_MAX_NUM_SAT * 2);

		g_pResult[iProcess]->pSVH = (uint8_t*)malloc(sizeof(uint8_t) * QC_MAX_NUM_SAT);
		memset(g_pResult[iProcess]->pSVH, 255, sizeof(uint8_t) * QC_MAX_NUM_SAT);
	}
}

extern void ResetRTKRes(int AntID)
{
	g_pResult[AntID]->Time.Time = 0;
	g_pResult[AntID]->Time.Sec = 0;
	g_pResult[AntID]->PosType = 0;
	g_pResult[AntID]->Num = 0;
	g_pResult[AntID]->Pos[0] = 0;
	g_pResult[AntID]->Pos[1] = 0;
	g_pResult[AntID]->Pos[2] = 0;

	memset(g_pResult[AntID]->pDts, 0, sizeof(double) * QC_MAX_NUM_SAT * 2);
	memset(g_pResult[AntID]->pSatCoors, 0, sizeof(double) * QC_MAX_NUM_SAT * 6);
	memset(g_pResult[AntID]->pSatNo, 0, sizeof(int16_t) * QC_MAX_NUM_SAT);
	memset(g_pResult[AntID]->pSVH, 255, sizeof(uint8_t) * QC_MAX_NUM_SAT);
}

static int FindOrbitSolIndex(const VERIQC_ORBIT_SOL_T* pOrbitSol, unsigned char SysID, unsigned char SatID)
{
	int idex = -1;
	int iSat = 0;
	
	int satNo = GetQCSatNo(QCId2Sys(SysID), SatID);
	for (iSat = 0; iSat < pOrbitSol->ObsNum; iSat++)
	{
		if (pOrbitSol->pSatNo[iSat] == satNo)
		{
			return iSat;
		}
	}
	return idex;
}


/**********************************************************************//**
@brief get epoch SPP result

@param pObs        [In]      observation info
@param AntID       [In]      Antenna ID

@retval  >0  success
@retval  0   fail

@author CHC
@date 
@note
History:
1. 2024/07/25 use RM_RANGE_LOG_T instead
**************************************************************************/
extern int GetEpochResultSPP(RM_RANGE_LOG_T* pObs, int AntID)
{
	int state = 0;
	unsigned int iSat = 0;
	int iOrbitIdex = 0;
	VERIQC_SPPTC_IFX_T* spptc_ifx = VeriQCGetSPPTcIfx();
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	VERIQC_SOLINFO_T* pFinalSol = NULL;
	VERIQC_ORBIT_SOL_T* OrbitSol = NULL;
	VERIQC_SIMPLIFY_RESULT_T* pResult = NULL;
	VERIQC_SATINFO_PERSAT_T* pSatInfo;
	RM_RANGE_DATA_T* pCurRangSatData = NULL;

	pResult = g_pResult[AntID];

	if ((!pObs) || (!pResult))
	{
		return state;
	}
	ResetRTKRes(AntID);

	/** Check if SNR == 0 */
	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		pCurRangSatData = &pObs->ObsData[iSat];

		if (((pCurRangSatData->ADR != 0) || (pCurRangSatData->PSR != 0)) &&
			(pCurRangSatData->CN0 == 0))
		{
			pCurRangSatData->CN0 = 1; /**< avoid VERIQC condition */
		}
	}
	
	spptc_ifx->AddRangeRovFunc(g_pSPPTC[AntID], (RM_RANGE_LOG_T*)pObs);
	orbit_ifx->OrbfitRangeFunc(g_pOrbit[AntID], (RM_RANGE_LOG_T*)pObs);

	spptc_ifx->StartFunc(g_pSPPTC[AntID]);
	OrbitSol = spptc_ifx->GetSatPosFunc(g_pSPPTC[AntID],VERIQC_ANT_ID_ROVER);
	spptc_ifx->SPPFunc(g_pSPPTC[AntID]);
	// spptc_ifx->fast_func(g_pSPPTC[AntID]);

	pFinalSol = spptc_ifx->GetSolFunc(g_pSPPTC[AntID]);

	state = pFinalSol->pSolState->PosType;
	if (state > 0)
	{
		pResult->PosType = pFinalSol->pSolState->PosType;
		pResult->Time.Time = pFinalSol->pTime->Time.Time;
		pResult->Time.Sec = pFinalSol->pTime->Time.Sec;
		memcpy(pResult->Pos, pFinalSol->pPos->PosXYZ, sizeof(double) * 3);
		pResult->UsedSatNum = pFinalSol->pSolState->SolSatNum;
		memcpy(pResult->Dop, pFinalSol->pPos->Dop, sizeof(float) * 5);
	}
	else
	{
		pResult->UsedSatNum = 0;
		memset(pResult->Dop, 0, sizeof(float) * 5);
	}

	pResult->Num = pFinalSol->pSolState->pSatInfo->SatNum;
	for (iSat = 0; iSat < pResult->Num; iSat++)
	{
		pSatInfo = pFinalSol->pSolState->pSatInfo->pSatInfo + iSat;
		pResult->pSatCoors[iSat * 6 + 0] = pSatInfo->PosVel[0];
		pResult->pSatCoors[iSat * 6 + 1] = pSatInfo->PosVel[1];
		pResult->pSatCoors[iSat * 6 + 2] = pSatInfo->PosVel[2];
		pResult->pSatCoors[iSat * 6 + 3] = pSatInfo->PosVel[3];
		pResult->pSatCoors[iSat * 6 + 4] = pSatInfo->PosVel[4];
		pResult->pSatCoors[iSat * 6 + 5] = pSatInfo->PosVel[5];
		pResult->pDts[0 + iSat * 2] = pSatInfo->SatClockError;
		pResult->pDts[1 + iSat * 2] = 0;
		pResult->pSatNo[iSat] = GetQCSatNo(QCId2Sys(pSatInfo->SysID), pSatInfo->SatID);
		pResult->pSVH[iSat] = 0;
	}

	return state;
}

/**********************************************************************//**
@brief detect missed satellite for use rate

@param Time        [In]      Epoch time
@param AntID       [In]      Antenna ID
@param SatNO       [In]      satellite ID number
@param SatPosVel   [Out]     satellite position and velocity
@param SatClock    [Out]     satellite clock

@retval  0   success
@retval -1   fail

@author CHC
@date 2024/04/25
@note
History:\n
**************************************************************************/
extern int CalSatPosSpecifyTime(QC_TIME_T Time, int AntID, int SatNO, double SatPosVel[6], double SatClock[2])
{
	int Result = 0;
	int QCSys, CTCSys;
	unsigned int Prn = 0;
	VERIQC_ORBIT_IFX_T* orbit_ifx = VeriQCGetOrbitIfx();
	QC_TIME_T CTCTime;
	
	CTCTime.Time = Time.Time;
	CTCTime.Sec = Time.Sec;
	
	QCSys = GetQCSatSys(SatNO, &Prn);
	switch (QCSys)
	{
	case QC_SYS_GPS:CTCSys = VERIQC_GNSS_SYSID_GPS;  break;
	case QC_SYS_GLO:CTCSys = VERIQC_GNSS_SYSID_GLN;  break;
	case QC_SYS_GAL:CTCSys = VERIQC_GNSS_SYSID_GAL;  break;
	case QC_SYS_QZS:
		CTCSys = VERIQC_GNSS_SYSID_GPS;  
		Prn += QC_MAX_PRN_GPS_TRUE;
		break;
	case QC_SYS_CMP:CTCSys = VERIQC_GNSS_SYSID_BDS;  break;
	default:
		return -1;
	}

	/** 0: ok -1: error */
	Result = orbit_ifx->OneSatCalcFunc(g_pOrbit[AntID], CTCTime, CTCSys, Prn, SatPosVel, SatClock);

	return Result;
}

extern VERIQC_SIMPLIFY_RESULT_T* GetSPPRes(int AntID)
{
	return g_pResult[AntID];
}
