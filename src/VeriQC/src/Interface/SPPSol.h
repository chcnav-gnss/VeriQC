/*************************************************************************//**
           VeriQC

	Interface Module
 *-
@file   SPPSol.h
@author CHC
@date   5 September 2022
@brief  Some Algorithm common interfaces
*****************************************************************************/
#ifndef _SPP_SOL_H_
#define _SPP_SOL_H_

#include "VeriQCSPP.h"
#include "DataQC.h"
#include "QCCommon.h"

typedef struct _VERIQC_SIMPLIFY_RESULT_T
{
	QC_TIME_T Time;
	unsigned int PosType;
	unsigned int Num;
	unsigned int NumMax;     /**< QC_MAX_NUM_SAT */
	unsigned int UsedSatNum; /**< sat number involved in position */
	double Pos[3];

	double* pSatCoors;       /**< XYZ */
	double* pDts;
	uint8_t* pSVH;           /**< svh 0:ok 0xFF:error */
	int16_t* pSatNo;

	float Dop[5];            /**< dop[gdop, pdop, hdop, vdop, tdop] */
}VERIQC_SIMPLIFY_RESULT_T;

extern void AddEphSPPCore(const UNI_EPHEMERIS_T* pEPH);
extern void AddGephSPPCore(const GLO_EPHEMERIS_T* pGEPH);
extern void InitRTKRes();
extern void ResetRTKRes(int AntID);
extern int GetEpochResultSPP(RM_RANGE_LOG_T* pObs, int AntID);
extern VERIQC_SIMPLIFY_RESULT_T* GetSPPRes(int AntID);
extern int CalSatPosSpecifyTime(QC_TIME_T Time, int AntID, int SatNO, double SatPosVel[6], double SatClock[2]);

#endif