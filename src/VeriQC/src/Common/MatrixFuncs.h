/**********************************************************************//**
        VeriQC

       Common Module
*-
@file    MatrixFuncs.h
@author  CHC
@date    2025/10/15
@brief   Matrix related funcs

**************************************************************************/
#ifndef _MATRIX_FUNCS_H_
#define _MATRIX_FUNCS_H_

typedef enum _MAT_MUL_TYPE_E
{
    MAT_MUL_TYPE_NN = 1,
    MAT_MUL_TYPE_NT = 2,
    MAT_MUL_TYPE_TN = 3,
    MAT_MUL_TYPE_TT = 4,
    MAT_MUL_TYPE_ABA_T = 5,   /* Only for VERIQC_MatrixMul */
    MAT_MUL_TYPE_MAX = 0xFFFFFFFF
}MAT_MUL_TYPE_E;

extern double* Mat(int N, int M);
extern double* MatFast(int N, int M);
extern void MatCpy(double* pA, const double* pB, int N, int M);

extern void MatMul(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
    const double* pA, const double* pB, double Beta, double* pC);
extern void MatMulCore(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
    const double* pA, const double* pB, double Beta, double* pC);
extern void VeriQCMatMul(MAT_MUL_TYPE_E Type, int N, int K, int M, double Alpha,
    const double* pA, const double* pB, double Beta, double* pC);

extern int MatInv(double* pA, int N);
extern int MatInvSym(double* pMat, int Row);
extern int MatInvSymCore(double* pA, int N);
extern int VeriQCSymMatInv(double* pA, int N);

typedef int(*VERIQC_MATMUL_FUNC)(int Type, int N, int K, int M, double Alpha,
    const double* pA, const double* pB, double Beta, double* pC);
typedef int(*VERIQC_SYM_MATINV_FUNC)(double* pA, int N);

#endif