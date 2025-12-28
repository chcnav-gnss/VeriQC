/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    Coordinate.c
@author  CHC
@date    2025/10/15
@brief   coordinate transformation related fuctions

**************************************************************************/
#include <math.h>
#include "Coordinate.h"
#include "MatrixFuncs.h"

/************************************************************************//**
@brief Convert Position from ECEF-LLH to ECEF-XYZ, use WGS84 frame

@param PosLLH  [In]  ECEF Position in LLH format [rad,rad, m]
@param PosXYZ  [Out] ECEF Position in XYZ format

@author  CHC
@date    2022/12/26 15:41:22
@note
****************************************************************************/
void PosLLH2XYZ(const double PosLLH[], double PosXYZ[])
{
	double N;
	double SinLat = 0.0;
	double SinLon = 0.0;
	double CosLat = 0.0;
	double CosLon = 0.0;

	SinLat = sin(PosLLH[0]);
	SinLon = sin(PosLLH[1]);
	CosLat = cos(PosLLH[0]);
	CosLon = cos(PosLLH[1]);

	N = WGS_AXIS_A / sqrt(1.0 - WGS_E1_SQR * SinLat * SinLat);

	PosXYZ[0] = (N + PosLLH[2]) * CosLat * CosLon;
	PosXYZ[1] = (N + PosLLH[2]) * CosLat * SinLon;
	PosXYZ[2] = (N * (1.0 - WGS_E1_SQR) + PosLLH[2]) * SinLat;
}

/* transform ecef to geodetic postion ------------------------------------------
* transform ecef position to geodetic position
* args   : double *pR        I   ecef position {x,y,z} (m)
*          double *pPos      O   geodetic position {lat,lon,h} (rad,m)
* return : none
* notes  : WGS84, ellipsoidal height
*-----------------------------------------------------------------------------*/
extern void PosXYZ2LLH(const double* pR, double* pPos)
{
	double e2 = FE_WGS84 * (2.0 - FE_WGS84), r2 = Dot(pR, pR, 2), z, zk, v = RE_WGS84, sinp;
	double r3 = Dot(pR, pR, 3);

	if (r2 < 1E-12 || r3 < 1E-12)
	{
		pPos[0] = pPos[1] = pPos[2] = 0.0;//add 20161109
	}
	//
	for (z = pR[2], zk = 0.0; fabs(z - zk) >= 1E-4;) 
	{
		zk = z;
		sinp = z / sqrt(r2 + z * z);
		v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);
		z = pR[2] + v * e2 * sinp;
	}
	pPos[0] = r2 > 1E-12 ? atan(z / sqrt(r2)) : (pR[2] > 0.0 ? PI / 2.0 : -PI / 2.0);
	pPos[1] = r2 > 1E-12 ? atan2(pR[1], pR[0]) : 0.0;
	pPos[2] = sqrt(r2 + z * z) - v;
}

/* ecef to local coordinate transfromation matrix ------------------------------
* compute ecef to local coordinate transfromation matrix
* args   : double *pPos      I   geodetic position {lat,lon} (rad)
*          double *pE        O   ecef to local coord transformation matrix (3x3)
* return : none
* notes  : matirix stored by column-major order (fortran convention)
*-----------------------------------------------------------------------------*/
extern void PosXYZ2ENU(const double* pPos, double* pE)
{
	double dspsinp = sin(pPos[0]), dspcosp = cos(pPos[0]), dspsinl = sin(pPos[1]), dspcosl = cos(pPos[1]);

	pE[0] = -dspsinl;      pE[3] = dspcosl;       pE[6] = 0.0;
	pE[1] = -dspsinp * dspcosl; pE[4] = -dspsinp * dspsinl; pE[7] = dspcosp;
	pE[2] = dspcosp * dspcosl;  pE[5] = dspcosp * dspsinl;  pE[8] = dspsinp;
}

/* transform ecef vector to local tangental coordinate -------------------------
* transform ecef vector to local tangental coordinate
* args   : double *pPos      I   geodetic position {lat,lon} (rad)
*          double *pR        I   vector in ecef coordinate {x,y,z}
*          double *pE        O   vector in local tangental coordinate {e,n,u}
* return : none
*-----------------------------------------------------------------------------*/
extern void ECEF2ENU(const double* pPos, const double* pR, double* pE)
{
	double E[9];

	PosXYZ2ENU(pPos, E);
	MatMul(MAT_MUL_TYPE_NN, 3, 1, 3, 1.0, E, pR, 0.0, pE);
}

/* transform local vector to ecef coordinate -----------------------------------
* transform local tangental coordinate vector to ecef
* args   : double *pPos      I   geodetic position {lat,lon} (rad)
*          double *pE        I   vector in local tangental coordinate {e,n,u}
*          double *pR        O   vector in ecef coordinate {x,y,z}
* return : none
*-----------------------------------------------------------------------------*/
extern void ENU2ECEF(const double* pPos, const double* pE, double* pR)
{
	double E[9];

	PosXYZ2ENU(pPos, E);
	MatMul(MAT_MUL_TYPE_TN, 3, 1, 3, 1.0, E, pE, 0.0, pR);
}

/* inner product ---------------------------------------------------------------
* inner product of vectors
* args   : double *pA,*pB   I   vector a,b (n x 1)
*          int    N         I   size of vector a,b
* return : a'*b
*-----------------------------------------------------------------------------*/
extern double Dot(const double* pA, const double* pB, int N)
{
	double c = 0.0;

	while (--N >= 0) c += pA[N] * pB[N];
	return c;
}

/* euclid norm -----------------------------------------------------------------
* euclid norm of vector
* args   : double *pa       I   vector a (n x 1)
*          int    N         I   size of vector a
* return : || a ||
*-----------------------------------------------------------------------------*/
extern double Norm(const double* pA, int N)
{
	return sqrt(Dot(pA, pA, N));
}

/* geometric distance ----------------------------------------------------------
* compute geometric distance and receiver-to-satellite unit vector
* args   : double *pRs       I   satellilte position (ecef at transmission) (m)
*          double *pRr       I   receiver position (ecef at reception) (m)
*          double *pE        O   line-of-sight vector (ecef)
* return : geometric distance (m) (0>:error/no satellite position)
* notes  : distance includes sagnac effect correction
*-----------------------------------------------------------------------------*/
extern double GeoDist(const double* pRs, const double* pRr, double* pE)
{
	double r, sagnac, rd;
	int i;

	pE[0] = 0.0;
	pE[1] = 0.0;
	pE[2] = 0.0;

	if (Norm(pRs, 3) < RE_WGS84)
	{
		return -1.0;
	}

	for (i = 0; i < 3; i++)
	{
		pE[i] = pRs[i] - pRr[i];
	}

	r = Norm(pE, 3);
	rd = 1.0 / r;
	for (i = 0; i < 3; i++)
	{
		pE[i] *= rd;
	}

	sagnac = OMGE * (pRs[0] * pRr[1] - pRs[1] * pRr[0]) * CLIGHT_DS;
	return r + sagnac;
}