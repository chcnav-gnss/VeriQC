/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    Coordinate.h
@author  CHC
@date    2025/10/15
@brief   coordinate transformation related fuctions

**************************************************************************/
#ifndef _COORDINATE_H_
#define _COORDINATE_H_

#include "GNSSConstants.h"

/** ECEF-LLH to ECEF-XYZ */
void PosLLH2XYZ(const double PosLLH[], double PosECEF[]);
/** ECEF-XYZ to ECEF-LLH */
extern void PosXYZ2LLH(const double* pPosXYZ, double* pPosLLH);
extern void PosXYZ2ENU(const double* pPos, double* pE);

extern void ECEF2ENU(const double* pPos, const double* pR, double* pE);
extern void ENU2ECEF(const double* pPos, const double* pE, double* pR);

extern double Dot(const double* pA, const double* pB, int N);
extern double Norm(const double* pA, int N);
extern double GeoDist(const double* pRS, const double* pR, double* pE);

#endif /**< _COORDINATE_H_ */