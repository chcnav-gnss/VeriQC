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
* convert sum¡¢PFsum to ave¡¢rms
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

	QC_TIME_T pTs = { 0 }; QC_TIME_T pTe = { 0 };

	for ( iSta = 0; iSta < QC_MAX_NUM_OBS_FILE; iSta++)
	{
		if (pQCRes->MaxEpochNum[iSta] <= 0)
		{
			continue;
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
			if (iSys == 4)
			{
				iSys += 0;
			}
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
				}
				else
				{
					pQCRes->MPTotel[iSta][iSys][iFre] = -1;
				}

				
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
	int satno, Type_Comb[2] = { -1 };
	int PreSatIndex = -1;
	int iLoop = 0;
	BOOL bMpSlip = FALSE;
	BOOL bGfSlip = FALSE;
	BOOL bLLISlip = FALSE;
	float Deg0 = 0;
	double EleMask = pOpt->EleDeg * D2R - 1E-4;
	
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
	/** get start time and end time */
	if (pQcRes->MaxEpochNum[AntID] == 1)
	{
		pQcRes->StartTime[AntID] = pObs->Time;
	}
	else
	{
		pQcRes->EndTime[AntID] = pObs->Time;
	}

	
	/** use Rate and snr record */
	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		satno = pObs->Data[iSat].Sat - 1;
		ValidFreqNum = 0;
		for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
		{
			/** Use Rate */
			if ((pObs->Data[iSat].L[iFre] != 0) &&
				(pObs->Data[iSat].P[iFre] != 0) &&
				(pObs->Data[iSat].LLI[iFre] == 0))
			{
				ValidFreqNum++;
			}

			/** Snr very deg num */
			Deg0 = 0;
			for ( iDeg = 0; iDeg < pQcRes->IntervNum; iDeg++)
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

		if ((pObs->Data[iSat].Azel[1] > EleMask) && (pObs->Data[iSat].Visibility == 0))
		{
			pQcRes->PossibleNum[AntID]++;
			pQcRes->SatPossibleNum[AntID][satno]++;

			if (ValidFreqNum >= QC_MIN_COMPLETE_FREQ)
			{
				pQcRes->SatCompleteNum[AntID][satno]++;
				pQcRes->CompleteNum[AntID]++;
			}
		}

		/** Cycle Slip */
		if (GetCombType(&pObs->Data[iSat], Type_Comb))
		{
			/** detect Cycle Slip */
			PreSatIndex = -1;
			for (iLoop = 0; iLoop < pObs[1].ObsNum; iLoop++)
			{
				if (pObs[1].Data[iLoop].Sat == pObs[0].Data[iSat].Sat)
				{
					PreSatIndex = iLoop;
					break;
				}
			}
			
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
	}
}
