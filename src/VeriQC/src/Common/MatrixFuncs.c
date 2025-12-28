/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    MatrixFuncs.c
@author  CHC
@date    2025/10/15
@brief   Matrix related funcs

**************************************************************************/
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "MatrixFuncs.h"
#include "CommonFuncs.h"
#include "Memory.h"
#include <stdarg.h>

/* fatal error ---------------------------------------------------------------*/
extern void FatalErr(const char* pFormat, ...)
{
#ifndef PLATFORM_ARM5
	va_list ap;
	va_start(ap, pFormat); vfprintf(stderr, pFormat, ap); va_end(ap);
#endif 
}

/* matrix routines -----------------------------------------------------------*/

/* new matrix ------------------------------------------------------------------
* allocate memory of matrix
* args   : int    n,m       I   number of rows and columns of matrix
* return : matrix pointer (if n<=0 or m<=0, return NULL)
*-----------------------------------------------------------------------------*/
extern double* Mat(int N, int M)
{
	double* p;
	if (N <= 0 || M <= 0) return NULL;
	if (!(p = (double*)VERIQC_MALLOC(sizeof(double) * N * M))) {
		FatalErr("mat:matrix memory allocation error: n=%d,m=%d\n", N, M);
	}
	return p;
}
extern double* MatFast(int N, int M)
{
	double* p;
	if (N <= 0 || M <= 0) return NULL;
	if (!(p = (double*)VERIQC_MALLOC_FAST(sizeof(double) * N * M))) {

	}

	return p;
}

/* copy matrix -----------------------------------------------------------------
* copy matrix
* args   : double *pA        O   destination matrix A (n x m)
*          double *pB        I   source matrix B (n x m)
*          int    N,M        I   number of rows and columns of matrix
* return : none
*-----------------------------------------------------------------------------*/
extern void MatCpy(double* pA, const double* pB, int N, int M)
{
	memcpy(pA, pB, sizeof(double) * N * M);
}

/* matmul */
extern void MatMulCore(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
	const double* pA, const double* pB, double Beta, double* pC)
{
	double tmp;
	int i, j, x;
	int min, max, jn, xn, jm, xk, im;

	if (Beta != 0.0) for (i = 0; i < N * K; i++) pC[i] = Beta * pC[i];
	else          memset(pC, 0, sizeof(double) * N * K);
	switch (Type)
	{
	case MAT_MUL_TYPE_NN:/* NN */
		for (j = 0; j < K; j++) for (x = 0, jn = j * N, jm = j * M; x < M; x++) {
			tmp = pB[x + jm];
			if (Alpha != 1.0) tmp *= Alpha;
			if (!tmp) continue;
			for (i = 0, xn = x * N; i < N; i++) pC[i + jn] += pA[i + xn] * tmp;
		}
		break;
	case MAT_MUL_TYPE_NT:/* NT */
		for (x = 0; x < M; x++) for (j = 0, xn = x * N, xk = x * K; j < K; j++) {
			tmp = pB[j + xk];
			if (Alpha != 1.0) tmp *= Alpha;
			if (!tmp) continue;
			for (i = 0, jn = j * N; i < N; i++) pC[i + jn] += pA[i + xn] * tmp;
		}
		break;
	case MAT_MUL_TYPE_TN:/* TN */
		for (j = 0; j < K; j++) for (x = 0, jn = j * N, jm = j * M; x < M; x++) {
			tmp = pB[x + jm];
			if (Alpha != 1.0) tmp *= Alpha;
			if (!tmp) continue;
			for (i = 0; i < N; i++) pC[i + jn] += pA[x + i * M] * tmp;
		}
		break;
	case MAT_MUL_TYPE_TT:/* TT */
		max = MAX(MAX(N, K), M); min = MIN(MIN(N, K), M);
		if (max == N) {
			if (min == K) {/* k, m, n */
				for (i = 0; i < N; i++) for (x = 0, im = i * M; x < M; x++) {
					tmp = pA[x + im];
					if (Alpha != 1.0) tmp *= Alpha;
					if (!tmp) continue;
					for (j = 0, xk = x * K; j < K; j++) pC[i + j * N] += tmp * pB[j + xk];
				}
			}
			else {/* m, k, n */
				for (x = 0; x < M; x++) for (j = 0, xk = x * K; j < K; j++) {
					tmp = pB[j + xk];
					if (Alpha != 1.0) tmp *= Alpha;
					if (!tmp) continue;
					for (i = 0, jn = j * N; i < N; i++) pC[i + jn] += pA[x + i * M] * tmp;
				}
			}
		}
		else if (max == K) {
			if (min == N) {/* n, m, k */
				for (j = 0; j < K; j++) for (x = 0, jn = j * N; x < M; x++) {
					tmp = pB[j + x * K];
					if (Alpha != 1.0) tmp *= Alpha;
					if (!tmp) continue;
					for (i = 0; i < N; i++) pC[i + jn] += pA[x + i * M] * tmp;
				}
			}
			else {/* m, n, k */
				for (j = 0; j < K; j++) for (i = 0, jn = j * N; i < N; i++) {
					tmp = 0.0;
					for (x = 0, im = i * M; x < M; x++) tmp += pA[x + im] * pB[j + x * K];
					pC[i + j * N] += Alpha * tmp;
				}
			}
		}
		else {
			if (min == K) {/* k, n, m */
				for (x = 0; x < M; x++) for (i = 0, xk = x * K; i < N; i++) {
					tmp = pA[x + i * M];
					if (Alpha != 1.0) tmp *= Alpha;
					if (!tmp) continue;
					for (j = 0; j < K; j++) pC[i + j * N] += tmp * pB[j + xk];
				}
			}
			else {/* n, k, m */
				for (x = 0; x < M; x++) for (j = 0, xk = x * K; j < K; j++) {
					tmp = pB[j + xk];
					if (Alpha != 1.0) tmp *= Alpha;
					if (!tmp) continue;
					for (i = 0; i < N; i++) pC[i + j * N] += pA[x + i * M] * tmp;
				}
			}
		}
		break;
	default:
		return;
		break;
	}
}

static int LLTUpper(int N, double* pA)
{
	int i, k, m, info = 0;
	double s;
	int k_n, k_k_n, m_k_n, i_n;

	for (k = 0; k < N; k++)
	{
		k_n = k * N;
		k_k_n = k + k_n;
		s = pA[k_k_n];
		if (s <= 0)
		{
			info = 1;
			break;
		}
		for (m = 0; m < k; m++)
		{
			m_k_n = m + k_n;
			s -= pA[m_k_n] * pA[m_k_n];
		}
		if (s <= 0.0)
		{
			info = 1;
			break;
		}
		pA[k_k_n] = sqrt(s);
		for (i = k + 1; i < N; i++)
		{
			i_n = i * N;
			s = pA[k + i_n];
			for (m = 0; m < k; m++)
			{
				s -= pA[m + i_n] * pA[m + k_n];
			}
			s /= pA[k + k_n];
			pA[k + i_n] = s;
		}
	}

	//copy B to A

	return info;
}

/* matminv_sym */
extern int MatInvSymCore(double* pA, int N)
{
	int i, j, k, info;
	double s;
	double* B = NULL;
	int i_n, j_n, j_n_1, i_n_1, i_j_n;
	int memtype = 1;

	B = MatFast(N, N);
	if (!B)
	{
		B = Mat(N, N);
		memtype = 0;
	}

	MatCpy(B, pA, N, N);

	if (!(info = LLTUpper(N, B)))
	{
		for (i = 0; i < N; i++)
		{
			i_n = i * N;
			for (j = 0; j < i; j++)
			{
				B[i + j * N] = B[j + i_n];
			}
		}
		//ope lower matrix
		for (j = 0; j < N; j++)
		{
			j_n = j * N;
			if (B[j + j_n] <= 0.0)//||B[j+j*n]!=B[j+j*n])
			{
				info = 1;
				break;
			}
			j_n_1 = j + j_n;
			B[j_n_1] = 1 / B[j_n_1];

			for (i = j + 1; i < N; i++)
			{
				s = 0.0;
				i_n = i * N;
				for (k = j; k < i; k++)
				{
					s -= B[k + i_n] * B[k + j_n];
				}
				i_n_1 = i + i_n;
				if (B[i_n_1] <= 0.0)//||B[i+i*n]!=B[i+i*n])/*singular*/
				{
					info = 1;
					break;
				}

				s /= B[i_n_1];

				if (s != s)/*singular*/
				{
					info = 1;
					break;
				}
				B[i + j_n] = s;
			}
			if (info)
			{
				break;
			}
		}
		if (!info)
		{
			for (j = 0; j < N; j++)
			{
				j_n = j * N;
				for (i = j; i < N; i++)
				{
					i_j_n = i + j_n;
					i_n = i * N;
					pA[i_j_n] = 0.0;
					for (k = i; k < N; k++)
					{
						pA[i_j_n] += B[k + i_n] * B[k + j_n];
					}
					if (i != j)
					{
						pA[j + i_n] = pA[i_j_n];
					}
				}
			}
		}
	}

	if (B)
	{
		if (memtype == 1)
		{
			VERIQC_FREE_FAST(B);
		}
		else
		{
			VERIQC_FREE(B);
		}
	}

	return info;
}

static int VeriQCMatMulDefault(int Type, int N, int K, int M, double Alpha,
	const double* pA, const double* pB, double Beta, double* pC)
{
	MatMulCore((MAT_MUL_TYPE_E)Type, N, K, M, Alpha, pA, pB, Beta, pC);
	return 0;
}

static int VeriQCSymMatInvDefault(double* pA, int N)
{
	return MatInvSymCore(pA, N);
}

VERIQC_MATMUL_FUNC s_VeriQCMatMulFunc = VeriQCMatMulDefault;
VERIQC_SYM_MATINV_FUNC s_VeriQCSymMatInvFunc = VeriQCSymMatInvDefault;

extern void VeriQCMatMul(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
	const double* pA, const double* pB, double Beta, double* pC)
{
	s_VeriQCMatMulFunc(Type, N, K, M, Alpha, pA, pB, Beta, pC);
}

extern int VeriQCSymMatInv(double* pA, int N)
{
	return s_VeriQCSymMatInvFunc(pA, N);
}

extern void MatMul(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
	const double* pA, const double* pB, double Beta, double* pC)
{
	VeriQCMatMul(Type, N, K, M, Alpha, pA, pB, Beta, pC);
}

extern int MatInvSym(double* pMat, int Row)
{
	return VeriQCSymMatInv(pMat, Row);
}
