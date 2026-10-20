/*************************************************************************//**
		   VeriQC
	Data Quality Check Module
*-
@file   GNSSQCProcess.c
@author CHC
@date   1st January 2023
@brief  Some Algorithm common function for QC
*****************************************************************************/
#include <string.h>
#include "DataQC.h"
#include "QCSource/AlgorithmBase/CommonBase.h"
#include "TimeDefines.h"

/*************************************************//**
@brief Get average CN0 value for one epoch

@param pObs     [In]   observations of one epoch
@param EleMask  [In]   elevation mask in rad

@return average CN0 of current epoch
@author CHC

@note 
1.Calculate epoch-average CN0
2.Use current observation data only
3.Return -1 when no valid CN0 exists
******************************************************/
static float GetEpochAvgCN0(const QC_OBS_T* pObs)
{
	int iSat = 0;
	int iFre = 0;
	int Count = 0;
	float SumCN0 = 0.0f;

	if (pObs == NULL)
	{
		return -1.0f;
	}

	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			if (pObs->Data[iSat].SNR[iFre] > 1.0)
			{
				SumCN0 += (float)pObs->Data[iSat].SNR[iFre];
				Count++;
			}
		}
	}

	if (Count <= 0)
	{
		return -1.0f;
	}

	return (SumCN0 / Count);
}

/*************************************************//**
@brief Insert CN0 drop window into top list

@param pQcRes      [In/Out]  QC result structure
@param AntID       [In]      ant id {rover:0 base:1}
@param pWindowInfo [In]      CN0 window info

@return void
@author CHC

@note 
1.Sort by CN0 drop severity
2.Keep descending order in result list
3.Only preserve the top QC_DIAG_TOP_WIN_NUM windows
******************************************************/
static void InsertCN0DropWindow(QC_RESULT_T* pQcRes, int AntID, const QC_CN0_TIME_WIN_INFO_T* pWindowInfo)
{
	int InsertPos = 0;
	int iWin = 0;
	int CurCount = 0;

	if ((pQcRes == NULL) || (pWindowInfo == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	CurCount = pQcRes->CN0DropWindowCount[AntID];
	if (CurCount < 0)
	{
		CurCount = 0;
	}

	if (CurCount > QC_DIAG_TOP_WIN_NUM)
	{
		CurCount = QC_DIAG_TOP_WIN_NUM;
	}

	while ((InsertPos < CurCount) &&
		(pWindowInfo->AvgCN0Drop <= pQcRes->CN0DropWindow[AntID][InsertPos].AvgCN0Drop))
	{
		InsertPos++;
	}
	if (InsertPos >= QC_DIAG_TOP_WIN_NUM)
	{
		return;
	}

	for (iWin = QC_DIAG_TOP_WIN_NUM - 1; iWin > InsertPos; iWin--)
	{
		pQcRes->CN0DropWindow[AntID][iWin] = pQcRes->CN0DropWindow[AntID][iWin - 1];
	}

	pQcRes->CN0DropWindow[AntID][InsertPos] = *pWindowInfo;
	if (pQcRes->CN0DropWindowCount[AntID] < QC_DIAG_TOP_WIN_NUM)
	{
		pQcRes->CN0DropWindowCount[AntID]++;
	}
}

/*************************************************//**
@brief Update CN0 drop sliding window for current epoch

@param pObs    [In]      observations of one epoch
@param AntID   [In]      ant id {rover:0 base:1}
@param pQcRes      [In/Out]  QC result structure
@param pOpt        [In]      QC option
@param bEpochGap   [In]      whether current epoch has interruption gap
@param EpochGapSec [In]      current epoch gap in seconds

@return void
@author CHC

@note 
1.Use 5-minute CN0 window
2.Advance window by configured step
3.Record top CN0 drop windows
******************************************************/
static void UpdateCN0DropWindow(QC_OBS_T* pObs, int AntID, QC_RESULT_T* pQcRes, QC_OPT_T* pOpt, float EpochAvgCN0,
	BOOL bEpochGap, double EpochGapSec)
{
	int iEpoch = 0;
	double DeltaTime[QC_DIAG_TOP_WIN_NUM] = { 0 };
	BOOL bAddNewTempWindow = FALSE;

	if ((pObs == NULL) || (pQcRes == NULL) || (pOpt == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	if (pQcRes->MaxEpochNum[AntID] <= 1)
	{
		/** init CN0 time info */
		memset(pQcRes->CN0DropWindowTemp[AntID], 0, sizeof(pQcRes->CN0DropWindowTemp[AntID]));
		memset(pQcRes->CN0DropWindow[AntID], 0, sizeof(pQcRes->CN0DropWindow[AntID]));
		pQcRes->CN0DropWindowCount[AntID] = 0;
		pQcRes->CN0DropWindowTempCount[AntID] = 0;
	}

	EpochAvgCN0 = GetEpochAvgCN0(pObs);
	if (EpochAvgCN0 <= 0.0f)
	{
		return;
	}

	/** time info */
	for (iEpoch = 0; (iEpoch < QC_DIAG_TOP_WIN_NUM) && (iEpoch < pQcRes->CN0DropWindowTempCount[AntID]); iEpoch++)
	{
		DeltaTime[iEpoch] = QCTimeDiff(&pObs->Time, &pQcRes->CN0DropWindowTemp[AntID][iEpoch].WindowStartTime);
	}

	/** finalize oldest window if it has reached window duration without data gap */
	if ((pQcRes->CN0DropWindowTempCount[AntID] > 0) &&
		(DeltaTime[0] >= (QC_CN0_DROP_WINDOW_SEC - 1e-5)) &&
		(DeltaTime[0] <= (QC_CN0_DROP_WINDOW_SEC + 1e-5)))
	{
		pQcRes->CN0DropWindowTemp[AntID][0].AvgCN0Drop = pQcRes->CN0DropWindowTemp[AntID][0].WindowStartAvgCN0 - EpochAvgCN0;
		if (pQcRes->CN0DropWindowTemp[AntID][0].AvgCN0Drop > 0.0f)
		{
			/** Compare and insert window */
			InsertCN0DropWindow(pQcRes, AntID, &pQcRes->CN0DropWindowTemp[AntID][0]);
		}

		/** update temp window */
		for (iEpoch = 0; iEpoch < (pQcRes->CN0DropWindowTempCount[AntID] - 1); iEpoch++)
		{
			pQcRes->CN0DropWindowTemp[AntID][iEpoch] = pQcRes->CN0DropWindowTemp[AntID][iEpoch + 1];
		}
		pQcRes->CN0DropWindowTempCount[AntID] -= 1;
		bAddNewTempWindow = TRUE;
	}

	/** data gap caused DeltaTime to jump past finalize tolerance, discard */
	if ((pQcRes->CN0DropWindowTempCount[AntID] > 0) && (DeltaTime[0] > (QC_CN0_DROP_WINDOW_SEC + 1e-5)))
	{
		pQcRes->CN0DropWindowTempCount[AntID] = 0;
	}

	if ((pQcRes->CN0DropWindowTempCount[AntID] > 0) && (pQcRes->CN0DropWindowTempCount[AntID] <= QC_DIAG_TOP_WIN_NUM))
	{
		if (DeltaTime[pQcRes->CN0DropWindowTempCount[AntID] - 1] >= QC_CN0_DROP_EDGE_SEC)
		{
			bAddNewTempWindow = TRUE;
		}
	}
	else if (pQcRes->CN0DropWindowTempCount[AntID] == 0)
	{
		bAddNewTempWindow = TRUE;
	}

	if (bAddNewTempWindow)
	{
		if (pQcRes->CN0DropWindowTempCount[AntID] == QC_DIAG_TOP_WIN_NUM)
		{
			for (iEpoch = 0; iEpoch < (pQcRes->CN0DropWindowTempCount[AntID] - 1); iEpoch++)
			{
				pQcRes->CN0DropWindowTemp[AntID][iEpoch] = pQcRes->CN0DropWindowTemp[AntID][iEpoch + 1];
			}
			pQcRes->CN0DropWindowTemp[AntID][pQcRes->CN0DropWindowTempCount[AntID] - 1].WindowStartTime = pObs->Time;
			pQcRes->CN0DropWindowTemp[AntID][pQcRes->CN0DropWindowTempCount[AntID] - 1].WindowStartAvgCN0 = EpochAvgCN0;
		}
		else if (pQcRes->CN0DropWindowTempCount[AntID] < QC_DIAG_TOP_WIN_NUM)
		{
			pQcRes->CN0DropWindowTemp[AntID][pQcRes->CN0DropWindowTempCount[AntID]].WindowStartTime = pObs->Time;
			pQcRes->CN0DropWindowTemp[AntID][pQcRes->CN0DropWindowTempCount[AntID]++].WindowStartAvgCN0 = EpochAvgCN0;
		}
	}
}

/*************************************************//**
@brief Finalize current healthy arc statistics

@param pQcRes    [In/Out]  QC result structure
@param pCurTime  [In]      current epoch time
@param AntID     [In]      ant id {rover:0 base:1}

@return void
@author CHC

@note 
1.Close current healthy arc
2.Update average and maximum duration statistics
3.Reset temporary arc state
******************************************************/
static void FinalizeHealthyArc(QC_RESULT_T* pQcRes, QC_TIME_T* pCurTime, int AntID)
{
	double ArcDurationSec = 0.0;

	if ((pQcRes == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	if (pQcRes->HealthyArcEpochCountTemp[AntID] == 0)
	{
		return;
	}

	/** update average duration */
	ArcDurationSec = QCTimeDiff(pCurTime, &pQcRes->HealthyArcStartTimeTemp[AntID]);
	pQcRes->HealthyArcDurationSumSec[AntID] += ArcDurationSec;
	pQcRes->HealthyArcCount[AntID]++;

	/** update MAX duration */
	if ((float)ArcDurationSec > pQcRes->MaxHealthyArcDurationSec[AntID])
	{
		pQcRes->MaxHealthyArcDurationSec[AntID] = (float)ArcDurationSec;
	}

	pQcRes->HealthyArcEpochCountTemp[AntID] = 0;
	memset(&pQcRes->HealthyArcStartTimeTemp[AntID], 0, sizeof(pQcRes->HealthyArcStartTimeTemp[AntID]));
}

/*************************************************//**
@brief Update healthy arc state for current epoch

@param pQcRes       [In/Out]  QC result structure
@param AntID        [In]      ant id {rover:0 base:1}
@param pObsTime     [In]      current epoch time
@param SlipSatCount [In]      slip satellite count of current epoch

@return void
@author CHC

@note 
1.Use slip satellite count threshold
2.Extend or close current healthy arc
3.Support average and maximum arc statistics
******************************************************/
static void UpdateHealthyArc(QC_RESULT_T* pQcRes, int AntID, const QC_TIME_T* pObsTime, int SlipSatCount)
{
	if ((pQcRes == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	if (SlipSatCount < 3)
	{
		if ((pQcRes->HealthyArcEpochCountTemp[AntID] == 0) && (pObsTime != NULL))
		{
			pQcRes->HealthyArcStartTimeTemp[AntID] = *pObsTime;
		}
		pQcRes->HealthyArcEpochCountTemp[AntID]++;
	}
	else
	{
		FinalizeHealthyArc(pQcRes, pObsTime, AntID);
	}
}

/*************************************************//**
@brief Insert slip outlier window into top list

@param pQcRes      [In/Out]  QC result structure
@param AntID       [In]      ant id {rover:0 base:1}
@param pWindowInfo [In]      slip window info

@return void
@author CHC

@note 
1.Sort by out-limit epoch ratio
2.Use out-limit epoch count as tie breaker
3.Only preserve the top QC_DIAG_TOP_WIN_NUM windows
******************************************************/
static void InsertSlipOutlierWindow(QC_RESULT_T* pQcRes, int AntID, const QC_SLIP_TIME_WIN_INFO_T* pWindowInfo)
{
	int InsertPos = 0;
	int iWin = 0;
	int CurCount = 0;

	if ((pQcRes == NULL) || (pWindowInfo == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	CurCount = pQcRes->SlipOutlierWindowCount[AntID];
	if (CurCount < 0)
	{
		CurCount = 0;
	}
	if (CurCount > QC_DIAG_TOP_WIN_NUM)
	{
		CurCount = QC_DIAG_TOP_WIN_NUM;
	}

	/** Order */
	while ((InsertPos < CurCount) &&
		((pWindowInfo->OutLimitEpochRatio < pQcRes->SlipOutlierWindow[AntID][InsertPos].OutLimitEpochRatio) ||
		((pWindowInfo->OutLimitEpochRatio == pQcRes->SlipOutlierWindow[AntID][InsertPos].OutLimitEpochRatio) &&
		 (pWindowInfo->OutLimitEpochNum <= pQcRes->SlipOutlierWindow[AntID][InsertPos].OutLimitEpochNum))))
	{
		InsertPos++;
	}

	if (InsertPos >= QC_DIAG_TOP_WIN_NUM)
	{
		return;
	}

	/** Update window */
	for (iWin = QC_DIAG_TOP_WIN_NUM - 1; iWin > InsertPos; iWin--)
	{
		pQcRes->SlipOutlierWindow[AntID][iWin] = pQcRes->SlipOutlierWindow[AntID][iWin - 1];
	}
	pQcRes->SlipOutlierWindow[AntID][InsertPos] = *pWindowInfo;
	if (pQcRes->SlipOutlierWindowCount[AntID] < QC_DIAG_TOP_WIN_NUM)
	{
		pQcRes->SlipOutlierWindowCount[AntID]++;
	}
}

/*************************************************//**
@brief Update slip outlier sliding window for current epoch

@param pObs                [In]      observations of one epoch
@param AntID               [In]      ant id {rover:0 base:1}
@param pQcRes              [In/Out]  QC result structure
@param pEpochSatObs        [In]      observed-satellite flag of current epoch
@param pEpochSatSlip       [In]      slip-satellite flag of current epoch
@param EpochValidSatCount  [In]      valid satellite count of current epoch
@param EpochSlipSatCount   [In]      slip satellite count of current epoch
@param bEpochGap           [In]      whether current epoch has interruption gap
@param EpochGapSec         [In]      current epoch gap in seconds

@return void
@author CHC

@note 
1.Use 5-minute slip window
2.Advance window by configured step
3.Record top slip outlier windows and per-satellite ratio
4.Reset temp windows when epoch interruption occurs
******************************************************/
static void UpdateSlipOutlierWindow(QC_OBS_T* pObs, int AntID, QC_RESULT_T* pQcRes,
	const unsigned char* pEpochSatObs, const unsigned char* pEpochSatSlip,
	int EpochValidSatCount, int EpochSlipSatCount, BOOL bEpochGap, double EpochGapSec)
{
	int iEpoch = 0;
	int iSat = 0;
	double DeltaTime[QC_DIAG_TOP_WIN_NUM] = { 0 };
	BOOL bAddNewTempWindow = FALSE;
	BOOL bOutlierEpoch = FALSE;

	if ((pObs == NULL) || (pQcRes == NULL) || (pEpochSatObs == NULL) || (pEpochSatSlip == NULL) ||
		(AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	if (pQcRes->MaxEpochNum[AntID] <= 1)
	{
		memset(pQcRes->SlipOutlierWindowTemp[AntID], 0, sizeof(pQcRes->SlipOutlierWindowTemp[AntID]));
		memset(pQcRes->SlipOutlierWindow[AntID], 0, sizeof(pQcRes->SlipOutlierWindow[AntID]));
		pQcRes->SlipOutlierWindowCount[AntID] = 0;
		pQcRes->SlipOutlierWindowTempCount[AntID] = 0;
	}

	if (bEpochGap)
	{
		memset(pQcRes->SlipOutlierWindowTemp[AntID], 0, sizeof(pQcRes->SlipOutlierWindowTemp[AntID]));
		pQcRes->SlipOutlierWindowTempCount[AntID] = 0;
	}

	/** time info */
	for (iEpoch = 0; (iEpoch < QC_DIAG_TOP_WIN_NUM) && (iEpoch < pQcRes->SlipOutlierWindowTempCount[AntID]); iEpoch++)
	{
		DeltaTime[iEpoch] = QCTimeDiff(&pObs->Time, &pQcRes->SlipOutlierWindowTemp[AntID][iEpoch].WindowStartTime);
	}

	/** finalize oldest window if it has reached window duration without data gap */
	if ((pQcRes->SlipOutlierWindowTempCount[AntID] > 0) &&
		(DeltaTime[0] >= (QC_SLIP_WINDOW_SEC - 1e-5)) &&
		(DeltaTime[0] <= (QC_SLIP_WINDOW_SEC + 1e-5)))
	{
		/** Calculate info */
		if (pQcRes->SlipOutlierWindowTemp[AntID][0].WindowEpochNum > 0)
		{
			pQcRes->SlipOutlierWindowTemp[AntID][0].OutLimitEpochRatio =
				(float)pQcRes->SlipOutlierWindowTemp[AntID][0].OutLimitEpochNum / pQcRes->SlipOutlierWindowTemp[AntID][0].WindowEpochNum;
			for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
			{
				if (pQcRes->SlipOutlierWindowTemp[AntID][0].SatNum[iSat] > 0)
				{
					pQcRes->SlipOutlierWindowTemp[AntID][0].SatSlipRatio[iSat] =
						(float)pQcRes->SlipOutlierWindowTemp[AntID][0].SatSlipNum[iSat] / pQcRes->SlipOutlierWindowTemp[AntID][0].SatNum[iSat];
				}
			}
		}

		/** Compare and insert window */
		InsertSlipOutlierWindow(pQcRes, AntID, &pQcRes->SlipOutlierWindowTemp[AntID][0]);

		/** Update temp window */
		for (iEpoch = 0; iEpoch < (pQcRes->SlipOutlierWindowTempCount[AntID] - 1); iEpoch++)
		{
			pQcRes->SlipOutlierWindowTemp[AntID][iEpoch] = pQcRes->SlipOutlierWindowTemp[AntID][iEpoch + 1];
		}
		pQcRes->SlipOutlierWindowTempCount[AntID] -= 1;
		bAddNewTempWindow = TRUE;
	}

	/** data gap caused DeltaTime to jump past finalize tolerance, discard */
	if ((pQcRes->SlipOutlierWindowTempCount[AntID] > 0) && (DeltaTime[0] > (QC_SLIP_WINDOW_SEC + 1e-5)))
	{
		pQcRes->SlipOutlierWindowTempCount[AntID] = 0;
	}

	if ((pQcRes->SlipOutlierWindowTempCount[AntID] > 0) && (pQcRes->SlipOutlierWindowTempCount[AntID] <= QC_DIAG_TOP_WIN_NUM))
	{
		if (DeltaTime[pQcRes->SlipOutlierWindowTempCount[AntID] - 1] >= QC_SLIP_EDGE_SEC)
		{
			bAddNewTempWindow = TRUE;
		}
	}
	else if (pQcRes->SlipOutlierWindowTempCount[AntID] == 0)
	{
		bAddNewTempWindow = TRUE;
	}

	if (bAddNewTempWindow)
	{
		if (pQcRes->SlipOutlierWindowTempCount[AntID] == QC_DIAG_TOP_WIN_NUM)
		{
			for (iEpoch = 0; iEpoch < (pQcRes->SlipOutlierWindowTempCount[AntID] - 1); iEpoch++)
			{
				pQcRes->SlipOutlierWindowTemp[AntID][iEpoch] = pQcRes->SlipOutlierWindowTemp[AntID][iEpoch + 1];
			}
			pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID] - 1].WindowStartTime = pObs->Time;
			pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID] - 1].WindowEpochNum = 0;
			memset(&pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID] - 1].SatNum, 0, sizeof(unsigned short) * QC_MAX_NUM_SAT);
			memset(&pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID] - 1].SatSlipNum, 0, sizeof(unsigned short) * QC_MAX_NUM_SAT);
		}
		else if (pQcRes->SlipOutlierWindowTempCount[AntID] < QC_DIAG_TOP_WIN_NUM)
		{
			pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID]].WindowStartTime = pObs->Time;
			pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID]].WindowEpochNum = 0;
			memset(&pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID]].SatNum, 0, sizeof(unsigned short) * QC_MAX_NUM_SAT);
			memset(&pQcRes->SlipOutlierWindowTemp[AntID][pQcRes->SlipOutlierWindowTempCount[AntID]++].SatSlipNum, 0, sizeof(unsigned short) * QC_MAX_NUM_SAT);
		}
	}

	if (EpochValidSatCount > 0 &&
		((float)EpochSlipSatCount / (float)EpochValidSatCount) > QC_SLIP_EPOCH_RATIO_LIMIT)
	{
		bOutlierEpoch = TRUE;
	}

	/** Update every window, add new epoch */
	for (iEpoch = 0; iEpoch < pQcRes->SlipOutlierWindowTempCount[AntID]; iEpoch++)
	{
		for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
		{
			if (pEpochSatObs[iSat])
			{
				pQcRes->SlipOutlierWindowTemp[AntID][iEpoch].SatNum[iSat]++;
			}
			if (pEpochSatSlip[iSat])
			{
				pQcRes->SlipOutlierWindowTemp[AntID][iEpoch].SatSlipNum[iSat]++;
			}
		}

		pQcRes->SlipOutlierWindowTemp[AntID][iEpoch].WindowEpochNum++;
		if (bOutlierEpoch)
		{
			pQcRes->SlipOutlierWindowTemp[AntID][iEpoch].OutLimitEpochNum++;
		}
	}
}

extern void UpdateQCOpt(QC_OPT_T* pOpt, QC_RESULT_T* pQCRes)
{
	/** opt processer */
	pQCRes->IntervNum = pOpt->SNRGroupSize;
	memcpy(pQCRes->DegThreshold, pOpt->SNRGroup, sizeof(float) * QC_MAXELEVINTERV);
}

/*----- get average position ------
*
*--------------------*/
extern void GetQCAvePos(QC_RESULT_T* pQCRes)
{
	int iSta, iVer;
	for (iSta = 0; iSta < QC_MAX_NUM_OBS_FILE; iSta++)
	{
		if (pQCRes->MaxEpochNum[iSta] <= 0 || pQCRes->PosNum[iSta] < 0.1)
		{
			continue;
		}
		for (iVer = 0; iVer < 3; iVer++)
		{
			pQCRes->PosXYZ[iSta][iVer] = pQCRes->PosXYZ[iSta][iVer] / pQCRes->PosNum[iSta];
		}
	}

}

/*----------------------------------------------
* After the second time
* convert sum to ave/rms
* 
*------------------------------------------------*/
extern void UpdateQCRes(QC_RESULT_T* pQCRes, QC_OPT_T* pOpt)
{
	int iSta, iSat, iFre, iDeg,iSys;
	int count_snr[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ] = { 0 };
	int count_mp[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ] = { 0 };
	int count_tfL[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ] = { 0 };
	int count_tfP[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ] = { 0 };
	int count_tfD[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ] = { 0 };
	int SumCompleteNumSys[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS] = { 0 };
	int SumPossibleNumSys[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS] = { 0 };

	float sum_SNR_deg[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_MAXELEVINTERV][QC_NUM_FREQ] = { 0 };
	int count_snr_deg[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_MAXELEVINTERV][QC_NUM_FREQ] = { 0 };
	int DDPnum[QC_MAX_NUM_SYS][QC_NUM_FREQ] = {0};
	int DDLnum[QC_MAX_NUM_SYS][QC_NUM_FREQ] = {0};
	int sys, prn;
	int sum_Slip = 0, sum_Detect = 0;

	int CountFreqAllMP[QC_MAX_NUM_OBS_FILE][QC_NUM_FREQ] = { 0 };
	int CountFreqGroupMP[QC_MAX_NUM_OBS_FILE][QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE] = { 0 };

	for ( iSta = 0; iSta < QC_MAX_NUM_OBS_FILE; iSta++)
	{
		if (pQCRes->MaxEpochNum[iSta] <= 0)
		{
			pQCRes->LowCN0EpochRatio[iSta] = -1;
			continue;
		}

		sum_Slip = 0;
		sum_Detect = 0;

		FinalizeHealthyArc(pQCRes, &pQCRes->EndTime[iSta], iSta);
		if (pQCRes->HealthyArcCount[iSta] > 0)
		{
			pQCRes->AvgHealthyArcDurationSec[iSta] = (float)(pQCRes->HealthyArcDurationSumSec[iSta] / pQCRes->HealthyArcCount[iSta]);
		}
		else
		{
			pQCRes->AvgHealthyArcDurationSec[iSta] = 0.0f;
		}

		pQCRes->LowCN0EpochRatio[iSta] = (float)pQCRes->LowCN0EpochNum[iSta] / pQCRes->MaxEpochNum[iSta];

		if (pQCRes->SatNum[iSta] > 0)
		{
			pQCRes->LowCN0SatRatio[iSta] = (float)pQCRes->LowCN0SatNum[iSta] / pQCRes->SatNum[iSta];
		}
		else
		{
			pQCRes->LowCN0SatRatio[iSta] = -1;
		}

		/** SNR and useRate */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
			{
				sys = GetQCSatSys(iSat + 1, &prn);
				iSys = QCSys2Id(sys) - 1;
				if (iSys < 0)
				{
					continue;
				}
				
				for (iDeg = 0; iDeg < pQCRes->IntervNum; iDeg++)
				{
					pQCRes->SNR[iSta][iSys][iFre] += pQCRes->QcSnr[iDeg].Snr[iSta][iSat][iFre];
					count_snr[iSta][iSys][iFre] += pQCRes->QcSnr[iDeg].SnrNum[iSta][iSat][iFre];
					sum_SNR_deg[iSta][iSys][iDeg][iFre]+= pQCRes->QcSnr[iDeg].Snr[iSta][iSat][iFre];
					count_snr_deg[iSta][iSys][iDeg][iFre] += pQCRes->QcSnr[iDeg].SnrNum[iSta][iSat][iFre];
				}
			}

			for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
			{
				if (count_snr[iSta][iSys][iFre] > 0)
				{
					pQCRes->SNR[iSta][iSys][iFre] = 1.0f * pQCRes->SNR[iSta][iSys][iFre] / count_snr[iSta][iSys][iFre];
				}
				else
				{
					pQCRes->SNR[iSta][iSys][iFre] = -1;
				}

				for (iDeg = 0; iDeg < pQCRes->IntervNum; iDeg++)
				{
					if (count_snr_deg[iSta][iSys][iDeg][iFre]>0)
					{
						pQCRes->SNRDeg[iSta][iSys][iDeg][iFre] = sum_SNR_deg[iSta][iSys][iDeg][iFre] / count_snr_deg[iSta][iSys][iDeg][iFre];
					}
					else
					{
						pQCRes->SNRDeg[iSta][iSys][iDeg][iFre] = -1;
					}
				}
			}
		}

		for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
		{
			sys = GetQCSatSys(iSat + 1, &prn);
			iSys = QCSys2Id(sys) - 1;
			if ((iSys < 0) || (pQCRes->SatPossibleNum[iSta][iSat] == 0))
			{
				continue;
			}
			SumPossibleNumSys[iSta][iSys] += pQCRes->SatPossibleNum[iSta][iSat];
			SumCompleteNumSys[iSta][iSys] += pQCRes->SatCompleteNum[iSta][iSat];
			pQCRes->UseRateSat[iSta][iSat] = 1.0 * pQCRes->SatCompleteNum[iSta][iSat] / pQCRes->SatPossibleNum[iSta][iSat];
		}

		/** OSR */
		if (pQCRes->PossibleNum[iSta] > 0)
		{
			pQCRes->UseRateTotal[iSta] = 1.0 * pQCRes->CompleteNum[iSta] / pQCRes->PossibleNum[iSta];
		}
		else
		{
			pQCRes->UseRateTotal[iSta] = -1;
		}

		for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
		{
			if (SumPossibleNumSys[iSta][iSys] > 0)
			{
				pQCRes->UseRateSys[iSta][iSys] = 1.0 * SumCompleteNumSys[iSta][iSys] / SumPossibleNumSys[iSta][iSys];
			}
			else
			{
				pQCRes->UseRateSys[iSta][iSys] = -1;
			}
		}
		for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
		{
			for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
			{
				if (pQCRes->DetectNum[iSta][iSat][iFre] < 0.1)
				{
					continue;
				}
				sum_Slip += pQCRes->SlipNum[iSta][iSat][iFre];
				sum_Detect += pQCRes->DetectNum[iSta][iSat][iFre];
			}
		}
		if (sum_Slip > 0)
		{
			pQCRes->OSR[iSta] = sum_Detect / sum_Slip;
		}
		else
		{
			pQCRes->OSR[iSta] = 99999;
		}

		/** MP */
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
			{
				sys = GetQCSatSys(iSat + 1, &prn);
				iSys = QCSys2Id(sys) - 1;
				if (iSys < 0)
				{
					continue;
				}

				pQCRes->MPTotel[iSta][iSys][iFre] += (float)pQCRes->MP[iSta][iSat][iFre];
				count_mp[iSta][iSys][iFre] += pQCRes->MPNum[iSta][iSat][iFre];
			}

			for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
			{
				if(count_mp[iSta][iSys][iFre]>0)
				{
					pQCRes->MPTotel[iSta][iSys][iFre] = (float)QC_SQRT(pQCRes->MPTotel[iSta][iSys][iFre] / count_mp[iSta][iSys][iFre]);

					pQCRes->MPGroupPercent[iSta][iSys][iFre][0] = (float)pQCRes->MPGroupNum[iSta][iSys][iFre][0] / count_mp[iSta][iSys][iFre];
					pQCRes->MPGroupPercent[iSta][iSys][iFre][1] = (float)pQCRes->MPGroupNum[iSta][iSys][iFre][1] / count_mp[iSta][iSys][iFre];
					pQCRes->MPGroupPercent[iSta][iSys][iFre][2] = (float)pQCRes->MPGroupNum[iSta][iSys][iFre][2] / count_mp[iSta][iSys][iFre];
					CountFreqAllMP[iSta][iFre] += count_mp[iSta][iSys][iFre];
					CountFreqGroupMP[iSta][iFre][0] += pQCRes->MPGroupNum[iSta][iSys][iFre][0];
					CountFreqGroupMP[iSta][iFre][1] += pQCRes->MPGroupNum[iSta][iSys][iFre][1];
					CountFreqGroupMP[iSta][iFre][2] += pQCRes->MPGroupNum[iSta][iSys][iFre][2];
				}
				else
				{
					pQCRes->MPTotel[iSta][iSys][iFre] = -1;

					pQCRes->MPGroupPercent[iSta][iSys][iFre][0] = -1;
					pQCRes->MPGroupPercent[iSta][iSys][iFre][1] = -1;
					pQCRes->MPGroupPercent[iSta][iSys][iFre][2] = -1;
				}
			}

			if (CountFreqAllMP[iSta][iFre] > 0)
			{
				pQCRes->MPGroupPercentAve[iSta][iFre][0] = (float)CountFreqGroupMP[iSta][iFre][0] / CountFreqAllMP[iSta][iFre];
				pQCRes->MPGroupPercentAve[iSta][iFre][1] = (float)CountFreqGroupMP[iSta][iFre][1] / CountFreqAllMP[iSta][iFre];
				pQCRes->MPGroupPercentAve[iSta][iFre][2] = (float)CountFreqGroupMP[iSta][iFre][2] / CountFreqAllMP[iSta][iFre];
			}
			else
			{
				pQCRes->MPGroupPercentAve[iSta][iFre][0] = -1;
				pQCRes->MPGroupPercentAve[iSta][iFre][1] = -1;
				pQCRes->MPGroupPercentAve[iSta][iFre][2] = -1;
			}
		}
	}
}

/*************************************************//**
@brief CHCData functions for one epoch

@param pObs    [In]   observations of one epoch
@param ant_id  [In]   ant id {rover:0 base:1} same as VERIQC_ANT_ID_BASE and VERIQC_ANT_ID_ROVER
@param QcRes   [Out]  QC result sturcture
@param pOpt	   [In]   QC option

@return void
@author CHC

@note 
1.use rate
2.SNR
3.MP and cycle slip
4.Select eligible data
******************************************************/
extern void QCRecordEpoch(QC_OBS_T* pObs, int AntID, QC_RESULT_T* pQcRes, QC_OPT_T* pOpt)
{
	int iSat, iFre, iDeg;
	int ValidFreqNum = 0;
	int EpochValidSatCount = 0;
	int EpochSlipSatCount = 0;
	int satno, Type_Comb[2] = { -1 };
	float Deg0 = 0;
	float EpochAvgCN0;
	double EleMask = pOpt->EleDeg * D2R - 1E-4;
	double EpochGapSec = 0.0;
	double ExpectedEpochInterval = 0.0;
	BOOL bEpochGap = FALSE;
	BOOL bLowCN0Sat = FALSE;
	BOOL bSatHasSlip = FALSE;
	BOOL bNoEphMode = FALSE;
	unsigned char EpochSatObs[QC_MAX_NUM_SAT] = { 0 };
	unsigned char EpochSatSlip[QC_MAX_NUM_SAT] = { 0 };
	bNoEphMode = (pOpt != NULL) && (pOpt->EphMode == QC_EPH_MODE_NONE);
	
	/* init SlipFlag and curMP */
	memset(&pQcRes->CurMPValidMask[AntID][0], 0, sizeof(unsigned short) * QC_MAX_NUM_SAT);
	memset(&pQcRes->CurGFAndMWValidMask[AntID][0], 0, sizeof(unsigned char) * QC_MAX_NUM_SAT);

	for (iSat = 0; iSat < QC_MAX_NUM_SAT; iSat++)
	{
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			pQcRes->CurMP[AntID][iSat][iFre] = -9999;
			pQcRes->SlipFlag[AntID][iSat][iFre] = 0;
		}
		for (iFre = 0; iFre < QC_LC_NUM; iFre++)
		{
			pQcRes->GF[AntID][iSat][iFre] = -9999;
			pQcRes->MW[AntID][iSat][iFre] = -9999;
		}
	}
	
	/* record station postion */
	if (pObs->StaPos[0] != 0)
	{
		pQcRes->PosXYZ[AntID][0] += pObs->StaPos[0];
		pQcRes->PosXYZ[AntID][1] += pObs->StaPos[1];
		pQcRes->PosXYZ[AntID][2] += pObs->StaPos[2];
		pQcRes->PosNum[AntID]++;
	}

	/** get epochNum */
	pQcRes->MaxEpochNum[AntID]++;
	/** get start time and detect epoch gap by previous epoch end time */
	if (pQcRes->MaxEpochNum[AntID] == 1)
	{
		pQcRes->StartTime[AntID] = pObs->Time;
	}
	else
	{
		ExpectedEpochInterval = (pOpt->Interval > QC_MIN_SAMPLE_INTERVAL) ? pOpt->Interval : pQcRes->Interval[AntID];
		EpochGapSec = QCTimeDiff(&pObs->Time, &pQcRes->EndTime[AntID]);
		if ((ExpectedEpochInterval > QC_MIN_SAMPLE_INTERVAL) && (EpochGapSec > (ExpectedEpochInterval + 1e-5)))
		{
			bEpochGap = TRUE;
		}
	}

	/** use Rate and snr record */
	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		satno = pObs->Data[iSat].Sat - 1;
		if ((satno < 0) || (satno >= QC_MAX_NUM_SAT))
		{
			continue;
		}
		ValidFreqNum = 0;
		bLowCN0Sat = FALSE;
		bSatHasSlip = FALSE;
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			/** Use Rate */
			if ((pObs->Data[iSat].L[iFre] != 0) &&
				(pObs->Data[iSat].P[iFre] != 0) &&
				(pObs->Data[iSat].LLI[iFre] == 0))
			{
				ValidFreqNum++;
			}

			if ((pObs->Data[iSat].SNR[iFre] > 1) && (pObs->Data[iSat].SNR[iFre] < 35) && !bLowCN0Sat)
			{
				bLowCN0Sat = TRUE;
			}

			if (!bNoEphMode)
			{
				/** Snr very deg num */
				Deg0 = 0;
				for (iDeg = 0; iDeg < pQcRes->IntervNum; iDeg++)
				{
					if ((pObs->Data[iSat].Azel[1] * R2D > (Deg0 - 1E-4)) &&
						((pObs->Data[iSat].Azel[1] * R2D) < pQcRes->DegThreshold[iDeg]) &&
						(pObs->Data[iSat].SNR[iFre] > 0) && (pObs->Data[iSat].Visibility == 0))
					{
						pQcRes->QcSnr[iDeg].Snr[AntID][satno][iFre] += (float)pObs->Data[iSat].SNR[iFre];
						pQcRes->QcSnr[iDeg].SnrNum[AntID][satno][iFre] ++;
					}
					Deg0 = pQcRes->DegThreshold[iDeg];
				}
			}
			else
			{
				pQcRes->QcSnr[0].Snr[AntID][satno][iFre] += (float)pObs->Data[iSat].SNR[iFre];
				pQcRes->QcSnr[0].SnrNum[AntID][satno][iFre] ++;
			}
		}

		if (!bNoEphMode && (pObs->Data[iSat].Azel[1] > EleMask) && (pObs->Data[iSat].Visibility == 0))
		{
			pQcRes->PossibleNum[AntID]++;
			pQcRes->SatPossibleNum[AntID][satno]++;

			if (ValidFreqNum >= QC_MIN_COMPLETE_FREQ)
			{
				pQcRes->SatCompleteNum[AntID][satno]++;
				pQcRes->CompleteNum[AntID]++;
			}
		}

		if (!EpochSatObs[satno])
		{
			EpochSatObs[satno] = 1;
			EpochValidSatCount++;
		}

		/** Cycle Slip */
		if (GetCombType(&pObs->Data[iSat], Type_Comb))
		{
			/** detect Cycle Slip */
			SlipProcessEntrance(&pObs[0].Data[iSat], pQcRes, AntID, satno, &pObs[0].Time);

			if (pOpt->MPMethod == MP_METHOD_FIXED)
			{
				MpProcessorFixed(&pObs->Data[iSat], pQcRes, AntID, satno, pQcRes->SlipFlag[AntID][satno], pObs->Time);
			}
			else if (pOpt->MPMethod == MP_METHOD_FILTER)
			{
				MpProcessorFilter(&pObs->Data[iSat], pQcRes, AntID, satno, pQcRes->SlipFlag[AntID][satno], pObs->Time);
			}
		}
		else
		{
			/** no DF data */
		}

		if (EpochSatObs[satno])
		{
			for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
			{
				if (pQcRes->SlipFlag[AntID][satno][iFre])
				{
					bSatHasSlip = TRUE;
					break;
				}
			}
			if (bSatHasSlip && !EpochSatSlip[satno])
			{
				EpochSatSlip[satno] = 1;
				EpochSlipSatCount++;
			}
		}

		if (bLowCN0Sat)
		{
			pQcRes->LowCN0SatNum[AntID]++;
		}
		pQcRes->SatNum[AntID]++;
	}

	EpochAvgCN0 = GetEpochAvgCN0(pObs);
	if (EpochAvgCN0 < 35)
	{
		pQcRes->LowCN0EpochNum[AntID]++;
	}

	if (EpochValidSatCount > 0)
	{
		UpdateHealthyArc(pQcRes, AntID, &pObs->Time, EpochSlipSatCount);
		UpdateSlipOutlierWindow(pObs, AntID, pQcRes, EpochSatObs, EpochSatSlip, EpochValidSatCount, EpochSlipSatCount,
			bEpochGap, EpochGapSec);
	}

	UpdateCN0DropWindow(pObs, AntID, pQcRes, pOpt, EpochAvgCN0, bEpochGap, EpochGapSec);

	pQcRes->EndTime[AntID] = pObs->Time;
}
