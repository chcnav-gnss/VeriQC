#include "GNSSBase.h"
#include "SPPBase.h"
#include "Logs.h"

#define _POSIX_C_SOURCE 199309

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>

#include "Common/DataTypes.h"
#include "Common/GNSSNavDataType.h"
#include "Coordinate.h"

#ifdef WIN32

//#include <windows.h>
//#include <mmSystem.h>
#else
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif



#define PRUNIT_GPS_RTCM3  299792.458  /* rtcm ver.3 unit of gps pseudorange (m) */
#define PRUNIT_GLO_RTCM3  599584.916  /* rtcm ver.3 unit of glonass pseudorange (m) */
#define RANGE_MS_RTCM32    (CLIGHT*0.001)      /* range in 1 ms */
#define SQRR(x)      ((x)*(x))

/* constants -----------------------------------------------------------------*/

const static double s_GPSTime0[] = { 1980,1, 6,0,0,0 }; /* gps time reference */

#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */
#define POLYCRC24Q  0x1864CFBu  /* CRC24Q polynomial */

#define ERR_BRDCI   0.5         /* broadcast iono model error factor */

const static double s_Leaps[][8] = {
    {2017,1,1,0,0,0,-18},
    {2015,7,1,0,0,0,-17},
    {2012,7,1,0,0,0,-16},
    {2009,1,1,0,0,0,-15},
    {2006,1,1,0,0,0,-14},
    {1999,1,1,0,0,0,-13},
    {1997,7,1,0,0,0,-12},
    {1996,1,1,0,0,0,-11},
    {1994,7,1,0,0,0,-10},
    {1993,7,1,0,0,0, -9},
    {1992,7,1,0,0,0, -8},
    {1991,1,1,0,0,0, -7},
    {1990,1,1,0,0,0, -6},
    {1988,1,1,0,0,0, -5},
    {1985,7,1,0,0,0, -4},
    {1983,7,1,0,0,0, -3},
    {1982,7,1,0,0,0, -2},
    {1981,7,1,0,0,0, -1}
};

const double g_Chisqr005[30] = { 3.841, 5.991, 7.815, 9.488, 11.070,12.592,14.067,15.507,16.919,18.307,
                           19.675,21.026,22.362,23.685,24.996,26.296,27.587,28.869,30.144,31.410,
                           32.671,33.924,35.172,36.415,37.652,38.885,40.113,41.337,42.557,43.773
};

CHC_PRCOPT_T g_PrcOptDefault = { 0 };


/* global raw/LC wavelength and coefficient maps */
const static int g_Sys2IndexMap[] = { 0,1,-1,2,1 };/* GPS,CMP,GLO,GAL,BD3, see GetSystemIndexByID */

/* satellite carrier wave length -----------------------------------------------
* get satellite carrier wave lengths
* args   : int    Sat        I   satellite number
*          int    Freq       I   frequency index (0:L1,1:L2,2:L5/3,...)
*          nav_t  *pNav      I   navigation messages
* return : carrier wave length (m) (0.0: error)
*-----------------------------------------------------------------------------*/
extern double SatWavelenEx(int Sat, int Freq, const CHC_NAV_T* pNav)
{
    const double freq_glo[] = { FREQ1_GLO,FREQ2_GLO,FREQ3_GLO };
    const double dfrq_glo[] = { DFRQ1_GLO,DFRQ2_GLO,0.0 };
    int i, sys = SatSys(Sat, NULL);

    if (pNav && sys == SYS_GLO) {
        if (0 <= Freq && Freq <= 2) {
            for (i = 0; i < pNav->NumGLO; i++) {
                if (pNav->pGEph[i].Sat != Sat) continue;
                return CLIGHT / (freq_glo[Freq] + dfrq_glo[Freq] * pNav->pGEph[i].Freq);
            }
        }
    }
    else if (sys == SYS_CMP || sys == SYS_BD3) {
        if (Freq == 0) return CLIGHT / FREQ2_CMP; /* B1 */
        else if (Freq == 2) return CLIGHT / FREQ6_CMP; /* B3 */
        else if (Freq == 1) return CLIGHT / FREQ7_CMP; /* B2 */
        else if (Freq == 3)return CLIGHT / FREQ1_CMP;/*B1C*/
        else if (Freq == 4)return CLIGHT / FREQ5_CMP;/*B2a*/
        else if (Freq == 5)return CLIGHT / FREQ8_CMP;/*B2a+b*/
        else if (Freq == 6)return CLIGHT / FREQ7_CMP;/*B2b*/
    }
    else if (sys == SYS_GAL) { 
        if (Freq == 0) return CLIGHT / FREQ1; /* L1/E1 */
        else if (Freq == 1) return CLIGHT / FREQ7; /* E5b */
        else if (Freq == 2) return CLIGHT / FREQ5; /* L5/E5a */
        else if (Freq == 3) return CLIGHT / FREQ8; /* E5a+b */
        else if (Freq == 4) return CLIGHT / FREQ6; /* E6 */
    }
    else {
        if (Freq == 0) return CLIGHT / FREQ1; /* L1/E1 */
        else if (Freq == 1) return CLIGHT / FREQ2; /* L2 */
        else if (Freq == 2) return CLIGHT / FREQ5; /* L5/E5a */
        else if (Freq == 3) return CLIGHT / FREQ6; /* L6/LEX */
        else if (Freq == 4) return CLIGHT / FREQ7; /* E5b */
        else if (Freq == 5) return CLIGHT / FREQ8; /* E5a+b */
    }
    return 0.0;
}

static int CmpObsEx(const void* pP1, const void* pP2)
{
    CHC_OBSD_T* q1 = (CHC_OBSD_T*)pP1, * q2 = (CHC_OBSD_T*)pP2;
    //double tt=timediff(q1->time,q2->time);
    //if (fabs(tt)>DTTOL) return tt<0?-1:1;
    if (q1->AntID != q2->AntID) return (int)q1->AntID - (int)q2->AntID;
    return (int)q1->Sat - (int)q2->Sat;
}

/* sort oneepoch observation data -------------------------------------------------------*/
extern int SortObsEx(CHC_OBSD_T* pEpochData, int Num)
{
    if (Num <= 0) return 0;
    qsort(pEpochData, Num, sizeof(CHC_OBSD_T), CmpObsEx);
    return 1;
}

/********************************************************************************************
@brief   quick sort double array
@param   pValArray[In]           double array
@param   NumEle[In]              number of element in array
@param   pSortedIndex[Out]       sorted index(0 ~ numEle-1)
@param   bAscendingMark[In]      1: ascending, 0: decending
@param   bAbsMark[In]            1: abs, 0: non-abs
@return  none
*********************************************************************************************/
extern void QuickSortD(const double* pValArray, int NumEle, int* pSortedIndex, BOOL bAscendingMark, BOOL bAbsMark)
{
    double tempv = 0.0;
    int i, j, tempindex = 0;
    double* tv = NULL;
    U1 mark = 0;

    if (NumEle <= 0) return;

    tv = (double*)VERIQC_MALLOC(sizeof(double) * NumEle);

    if (!tv) return;

    for (i = 0; i < NumEle; i++)
    {
        tv[i] = bAbsMark ? fabs(pValArray[i]) : pValArray[i];
        pSortedIndex[i] = i;
    }
    for (i = 0; i < NumEle; i++)
    {
        for (j = i + 1; j < NumEle; j++)
        {
            mark = 0;
            if (bAscendingMark == 0)
            {
                if (tv[j] > tv[i])  mark = 1;//descending
            }
            else //1
            {
                if (tv[j] < tv[i])  mark = 1;//ascending
            }
            if (mark)
            {
                tempv = tv[i];
                tv[i] = tv[j];
                tv[j] = tempv;
                tempindex = pSortedIndex[i];
                pSortedIndex[i] = pSortedIndex[j];
                pSortedIndex[j] = tempindex;
            }
        }
    }

    if (tv) VERIQC_FREE(tv);
}

/* double-differenced measurement error covariance ---------------------------*/

extern double GetPxsy(double VStd, double K0, double K1, U1 EnMinLmtMark)
{
    double ak = 0.0;

    if (fabs(VStd) <= K0)
    {
        return 1.0;
    }
    else if (fabs(VStd) > K0 && fabs(VStd) <= K1)
    {
        ak = 1.0 * K0 / fabs(VStd) * pow((K1 - fabs(VStd)) / (K1 - K0), 2);

        if (EnMinLmtMark)
        {
            if (ak < 0.0011111) ak = 0.0011111;
        }

        return ak;
    }
    else
    {
        if (EnMinLmtMark)
        {
            return 0.0011111;       //1/900.0
        }
        else
        {
            return 0.0;
        }
    }
    return 1.0;
}

U1 ComputeStandardResSpt(double* pRess, const CHC_ADJ_ROBUST_BUFFER_T* pAdjRobustBuffer, const double* pV, int Nv, double Sigma02, double* pTp)
{
    U1 stat = 1;
    int i, index;
    double value;
    for (i = 0; i < Nv; i++)
    {
        index = i + i * Nv;
        value = Sigma02 * pAdjRobustBuffer->pIminusJ[index];
        if (value < ZEROSVALUE)
        {
            stat = 0;
            break;
        }
        pRess[i] = pV[i] * sqrt(pTp[i] / value);
    }
    return stat;
}

U1 ComputeAdjFactorSpt(CHC_ADJ_ROBUST_BUFFER_T* pAdjRobustBuffer, const double* pA, const double* pQ, const double* pTp, int Nv, int NPar)
{
    double* ATQ, * ATQA, * JJ;
    int i, j, in, iindex, ijindex;

    ATQ = pAdjRobustBuffer->pATQ;
    ATQA = pAdjRobustBuffer->pATQA;
    JJ = pAdjRobustBuffer->pIminusJ;

    MatMul(MAT_MUL_TYPE_TN, Nv, NPar, NPar, 1.0, pA, pQ, 0.0, ATQ);//ATQ=A'*Q
    MatMul(MAT_MUL_TYPE_NN, Nv, Nv, NPar, 1.0, ATQ, pA, 0.0, ATQA);//ATQA=ATQ*A

    for (i = 0; i < Nv; i++)
    {
        in = i * Nv;
        for (j = 0; j < Nv; j++)
        {
            JJ[j + in] = ATQA[j + in] * pTp[i];
        }
    }
    //matmul(MAT_MUL_TYPE_NN,nv,nv,nv,1.0,ATQA,P,0.0,JJ);//JJ=ATQA*P

    for (i = 0; i < Nv; i++)
    {
        iindex = i * Nv;
        for (j = 0; j < Nv; j++)
        {
            ijindex = j + iindex;
            if (i != j)
            {
                JJ[ijindex] = 0.0 - JJ[ijindex];
            }
            else
            {
                JJ[ijindex] = 1.0 - JJ[ijindex];
            }
        }
    }

    return 1;
}


U1 InitAdjRobustBuffer(CHC_ADJ_ROBUST_BUFFER_T* pAdjRobustBuffer, int Nv, int NPar)
{
    pAdjRobustBuffer->pATQ = MatFast(Nv, NPar);
    pAdjRobustBuffer->pATQA = MatFast(Nv, Nv);
    pAdjRobustBuffer->pIminusJ = MatFast(Nv, Nv);
    pAdjRobustBuffer->MemTypeATQ = 1;
    pAdjRobustBuffer->MemTypeATQA = 1;
    pAdjRobustBuffer->MemTypeIminusJ = 1;

    if (!pAdjRobustBuffer->pATQ)
    {
        pAdjRobustBuffer->pATQ = Mat(Nv, NPar);
        pAdjRobustBuffer->MemTypeATQ = 0;
    }
    if (!pAdjRobustBuffer->pATQA)
    {
        pAdjRobustBuffer->pATQA = Mat(Nv, Nv);
        pAdjRobustBuffer->MemTypeATQA = 0;
    }
    if (!pAdjRobustBuffer->pIminusJ)
    {
        pAdjRobustBuffer->pIminusJ = Mat(Nv, Nv);
        pAdjRobustBuffer->MemTypeIminusJ = 0;
    }

    pAdjRobustBuffer->NV = Nv;
    pAdjRobustBuffer->ParaNum = NPar;

    return 1;
}

U1 EndAdjRobustBuffer(CHC_ADJ_ROBUST_BUFFER_T* pAdjRobustBuffer)
{
    if (pAdjRobustBuffer->pATQ)
    {
        if (pAdjRobustBuffer->MemTypeATQ)
        {
            VERIQC_FREE_FAST(pAdjRobustBuffer->pATQ);
        }
        else
        {
            VERIQC_FREE(pAdjRobustBuffer->pATQ);
        }

        pAdjRobustBuffer->pATQ = NULL;
    }
    if (pAdjRobustBuffer->pATQA)
    {
        if (pAdjRobustBuffer->MemTypeATQA)
        {
            VERIQC_FREE_FAST(pAdjRobustBuffer->pATQA);
        }
        else
        {
            VERIQC_FREE(pAdjRobustBuffer->pATQA);
        }
        pAdjRobustBuffer->pATQA = NULL;
    }
    if (pAdjRobustBuffer->pIminusJ)
    {
        if (pAdjRobustBuffer->MemTypeIminusJ)
        {
            VERIQC_FREE_FAST(pAdjRobustBuffer->pIminusJ);
        }
        else
        {
            VERIQC_FREE(pAdjRobustBuffer->pIminusJ);
        }

        pAdjRobustBuffer->pIminusJ = NULL;
    }
    pAdjRobustBuffer->NV = 0;
    pAdjRobustBuffer->ParaNum = 0;

    return 1;
}

extern double AvgSTD(const double* pA, int N, double* pStd)
{
    int i;
    double avg = 0.0, sum = 0.0;
    if (N == 0)
    {
        if (pStd) *pStd = 0.0;
        return 99999.9;
    }

    for (i = 0; i < N; i++) sum += pA[i];
    avg = sum / N;
    if (pStd) {
        sum = 0.0;
        for (i = 0; i < N; i++) sum += (pA[i] - avg) * (pA[i] - avg);
        *pStd = N > 1 ? sqrt(sum / (N - 1)) : 0.0;
    }

    return avg;
}

extern int GetClkBk(CHC_PRCOPT_T* pOpt, CHC_SOL_T* pSol, CHC_SSATLIST_T* pSsatlist, const CHC_OBSD_T* pObs, const double* pRs, const double* pDts, int  Nv,
    const CHC_NAV_T* pNav, U1 ID)
{
    int m = 0, sys, k, g, index;
    int i, f, fx = 0;
    double pos_rover[3], r_rover, dion, dtrp, vion, vtrp;
    double e[3], * pLam, dtr[MAXOBS];
    int dsdindex[MAXOBS], satno[MAXOBS];
    double P[MAXOBS], rrDist[MAXOBS], dtsDist[MAXOBS], ionoDist[MAXOBS], tropDist[MAXOBS];
    U1 mark;
    double trph_xs_r = 0.0, trpw_xs_r = 0.0;
    double sea_press_trop = 1013.25, sea_temp_trop = 15.0, humi_trop = 0.7;
    CHC_SSAT_T* pSsat;
    int iSat;
    double weekSecond;
    double* pPosXYZ;

    if (Nv <= 0)
    {
        return 0;
    }
    if (SOLQ_NONE != pSol->Stat)
    {
        pPosXYZ = pSol->Rr;
    }
    else if (SOLQ_NONE != pSol->XSimSol.Stat)
    {
        pPosXYZ = pSol->XSimSol.Rr;
    }
    else
    {
        return 0;
    }

    PosXYZ2LLH(pPosXYZ, pos_rover);
    weekSecond = QCTime2GPSTime(pObs[0].Time, NULL);

    for (m = 0; m < SYSNUM; m++)
    {
        sys = GetSystemIDByIndex(m);
        g = 0;
        k = -1;
        mark = 0;

        if (!(sys & pOpt->NavSys))
        {
            continue;
        }
        k = GetClkIndex(sys);

        if (k == -1 || fabs(pSol->Dtr[k]) > 1E-12)
        {
            continue;
        }

        if (!mark)
        {
            GetTropModelConstant(pos_rover, sea_press_trop, sea_temp_trop, humi_trop, &trph_xs_r, &trpw_xs_r);
            mark = 1;
        }
        g = 0;
        for (i = 0; i < Nv; i++)
        {
            if (i >= MAXOBS) break;

            if (sys != pObs[i].Sys) continue;
            iSat = pObs[i].Sat;
            pSsat = &(pSsatlist->pSsat[pSsatlist->Index[iSat - 1]]);

            r_rover = GeoDist(pRs + i * 6, pPosXYZ, e);

            if (r_rover > 0)
            {
                if (pSsat->AzEl[1] > 0.0)
                {
                    for (f = 0; f < NFREQ; f++)
                    {
                        fx = pObs[i].Index[f];
                        if (pObs[i].ObsValidMask & BIT_MASK_CODE(f))
                        {
                            dion = dtrp = vion = vtrp = 0.0;
                            IonoCorrBroadcast(weekSecond, pNav, sys, pos_rover, pSsat->AzEl, pSsat, &dion, &vion);

                            dtrp = TropModelInputSeaLevelParaAdv(pObs[i].Time, pos_rover, pSsat->AzEl, trph_xs_r, trpw_xs_r);
                            if (f != 0)
                            {
                                pLam = pSsat->Lam;
                                dion *= (pLam[f] > 0.0 ? SQR(pLam[f] / pLam[0]) : 1.0);
                            }
                            satno[g] = iSat;
                            P[g] = pObs[i].P[fx];
                            rrDist[g] = r_rover;
                            dtsDist[g] = CLIGHT * pDts[i * 2];
                            ionoDist[g] = dion;
                            tropDist[g] = dtrp;

                            dtr[g++] = pObs[i].P[fx] - (r_rover - CLIGHT * pDts[i * 2] + dion + dtrp);

                            break;
                        }
                    }
                }
            }
        }

        if (g > 0)
        {
            QuickSortD(dtr, g, dsdindex, 0, 1);
            i = g / 2;
            index = dsdindex[i];
            pSol->Dtr[k] = dtr[index] / CLIGHT;

            GNSS_LOG_DEBUG("id=%d,g=%d,sys=%d,clk-dist=%f,P=%f,dts-dist=%f,iono=%f,trop=%f",
                ID, g, sys, dtr[index], P[index], dtsDist[index], ionoDist[index], tropDist[index]);
        }
    }

    return 1;
}

//if there is L2C,use L2C
U1 SettleObs(CHC_OBSD_T* pObs, int Num)
{
    int i, f1, sys, fx = -1, f1x = -1;
    int f = -1;

    for (i = 0; i < Num; i++)
    {
        sys = pObs[i].Sys;
        if (sys == SYS_GPS)
        {
            f1x = 1;
            f1 = pObs[i].Index[f1x];
            for (fx = NFREQ; fx < NFREQ + NEXOBS; fx++)
            {
                f = pObs[i].Index[fx];
                if (f < 0) continue;
                if (pObs[i].Code[f] == CODE_L2X)
                {
                    if (pObs[i].ObsValidMask & BIT_MASK_PHASE(fx))
                    {
                        if (f1 >= 0)
                        {
                            pObs[i].SNR[f1] = pObs[i].SNR[f];
                            pObs[i].LLI[f1] = pObs[i].LLI[f];
                            pObs[i].Code[f1] = pObs[i].Code[f];
                            pObs[i].L[f1] = pObs[i].L[f];
                            pObs[i].D[f1] = pObs[i].D[f];
                        }
                    }
                    if (pObs[i].ObsValidMask & BIT_MASK_CODE(fx))
                    {
                        if (f1 >= 0)
                        {
                            if (!(pObs[i].ObsValidMask & BIT_MASK_CODE(1)))
                            {
                                pObs[i].P[f1] = pObs[i].P[f];
                                pObs[i].PCSR[f1] = pObs[i].PCSR[f];
                            }
                        }
                    }

                    break;
                }
            }
        }
        else if (sys == SYS_GLO)
        {
            f1x = 1;
            f1 = pObs[i].Index[f1x];
            if (f1 >= 0 && pObs[i].Code[f1] == CODE_NONE)
            {
                for (fx = NFREQ; fx < NFREQ + NEXOBS; fx++)
                {
                    f = pObs[i].Index[fx];
                    if (f < 0) continue;
                    if (pObs[i].Code[f] == CODE_L2C)
                    {
                        if (f1 >= 0)
                        {
                            pObs[i].SNR[f1] = pObs[i].SNR[f];
                            pObs[i].LLI[f1] = pObs[i].LLI[f];
                            pObs[i].Code[f1] = pObs[i].Code[f];
                            pObs[i].L[f1] = pObs[i].L[f];
                            pObs[i].P[f1] = pObs[i].P[f];
                            pObs[i].D[f1] = pObs[i].D[f];
                            pObs[i].PCSR[f1] = pObs[i].PCSR[f];
                            break;
                        }
                    }
                }
            }
        }
    }

    return 1;
}


/********************************************************************************************
@brief   get coefficient of PIF first frequency

@param   SysIndex[In]          system index, see GetSystemIDByIndex
@param   LCFreqIndex[In]       linear combination option(0 ~ NFREQ_IF)

@author  CHC

@return  coefficient of PIF1. 0 if unsupported
@note    LCFreqIndex linear combination
    GPS  0:L1-L2;   1:L1-L5
    CMP  0:B1I-B2I; 1:B1I-B3I; 2:B3I-B1C; 3:B1I-B2a; 4:B1C-B2a
    GAL  0:E1-E5b;  1: E1-E5a; 2:E1-E5(a+b)
*********************************************************************************************/
extern double GetCoeffPIF1(int SysIndex, int LCFreqIndex)
{
    /* coefficient of PIF first frequency */
    const static double s_PIF1Coeff[][CHC_NFREQ_IF] = {/* [sysIndex][LCFreqIndex] */
    {IF12_GPS_1,IF13_GPS_1,0.0       ,0.0       ,0.0       },/* GPS */
    {IF12_CMP_1,IF13_CMP_1,IF34_CMP_3,IF15_CMP_1,IF45_CMP_4},/* CMP */
    {IF12_GAL_1,IF13_GAL_1,IF14_GAL_1,0.0       ,0.0       } /* GAL */
    };

    if (SysIndex < 0)
    {
        return 0.0;
    }
    return s_PIF1Coeff[g_Sys2IndexMap[SysIndex]][LCFreqIndex];
}

/********************************************************************************************
@brief   get coefficient of PIF second frequency

@param   SysIndex[In]          system index, see GetSystemIDByIndex
@param   LCFreqIndex[In]       linear combination option(0 ~ NFREQ_IF)

@author  CHC 

@return  coefficient of PIF1. 0 if unsupported
@note    LCFreqIndex linear combination
    GPS  0:L1-L2;   1:L1-L5
    CMP  0:B1I-B2I; 1:B1I-B3I; 2:B3I-B1C; 3:B1I-B2a; 4:B1C-B2a
    GAL  0:E1-E5b;  1: E1-E5a; 2:E1-E5(a+b)
*********************************************************************************************/
extern double GetCoeffPIF2(int SysIndex, int LCFreqIndex)
{
    /* coefficient of PIF second frequency */
    const static double s_PIF2Coeff[][CHC_NFREQ_IF] = {/* [sysIndex][LCFreqIndex] */
    {IF12_GPS_2,IF13_GPS_3,0.0       ,0.0       ,0.0       },/* GPS */
    {IF12_CMP_2,IF13_CMP_3,IF34_CMP_4,IF15_CMP_5,IF45_CMP_5},/* CMP */
    {IF12_GAL_2,IF13_GAL_3,IF14_GAL_4,0.0       ,0.0       } /* GAL */
    };

    if (SysIndex < 0)
    {
        return 0.0;
    }
    return s_PIF2Coeff[g_Sys2IndexMap[SysIndex]][LCFreqIndex];
}



/********************************************************************************************
@brief   get raw equation type

@param   FreqIndex[In]         frequency index. 0-NFREQ:phase; NFREQ~2*NFREQ: code

@author  CHC

@return  EQ_P?/EQ_L?
@note    see GetFreqIndexByRawEquationType
*********************************************************************************************/
extern int GetRawEquationType(int FreqIndex)
{
    const static int s_RawEquationTypeMap[CHC_NFREQ * 2] = {
        EQ_L1,EQ_L2,EQ_L3,EQ_L4,EQ_L5,
        EQ_P1,EQ_P2,EQ_P3,EQ_P4,EQ_P5
    };

    return s_RawEquationTypeMap[FreqIndex];
}

/********************************************************************************************
@brief   get obs frequency index by raw equation type

@param   EquType[In]           raw equation type. EQ_L???/EQ_P???

@author  CHC

@return  freqIndex: 0-NFREQ:phase; NFREQ~2*NFREQ: code
@note    see GetRawEquationType
*********************************************************************************************/
extern int GetRawFreqByEquationType(int EquType)
{
    if (EquType < EQ_P1)
    {
        /* phase */
        return EquType;
    }
    /* code */
    return EquType - EQ_P1 + NFREQ;
}

/********************************************************************************************
@brief   get frequency index of IF by equation type

@param   SysIndex[In]          system index, see GetSystemIDByIndex
@param   ObsType[In]           MW equation type, EQ_IF??/EQ_PLIF??
@param   pFreq1[Out]           first raw frequency index(0 ~ NFREQ)
@param   pFreq2[Out]           second raw frequency index(0 ~ NFREQ)

@author  CHC

@return  LCFreqIndex, linear combination option(0 ~ NFREQ_CM)
@note    see GetIFEquationType
*********************************************************************************************/
extern int GetIFFreqByEquationType(int SysIndex, int ObsType, int* pFreq1, int* pFreq2)
{
    const static int s_IFFreqMap[][EQ_IF_NUM] = {
        {0, 1, 2, 3,-1, 4, 5, 6, 7, 8,-1, 9},
        {0, 0, 0, 0, 2, 3, 0, 0, 0, 0, 2, 3},
        {1, 2, 3, 4, 3, 4, 1, 2, 3, 4, 3, 4}
    };
    int LCFreqIndex;
    int tempIndex;

    tempIndex = ObsType - EQ_IF12;
    LCFreqIndex = s_IFFreqMap[0][tempIndex];
    *pFreq1 = s_IFFreqMap[1][tempIndex];
    *pFreq2 = s_IFFreqMap[2][tempIndex];

    if (1 == SysIndex || 4 == SysIndex)
    {
        if (EQ_IF34 == ObsType)
        {
            LCFreqIndex = 2;
        }
        else if (EQ_PLIF34 == ObsType)
        {
            LCFreqIndex = 7;
        }
    }

    return LCFreqIndex;
}

extern U1 GetSysLam(int Sys, int Sat, double* pLam, const CHC_NAV_T* pNav)
{
    int f;
    int sysIndex;
    if (Sys == SYS_GPS || Sys == SYS_CMP || Sys == SYS_BD3 || Sys == SYS_GAL)
    {
        sysIndex = GetSystemIndexByID(Sys);
        for (f = 0; f < NFREQ; f++)
        {
            pLam[f] = GetRawWaveLength(sysIndex, f);
        }
    }
    else if (Sys == SYS_GLO)
    {
        for (f = 0; f < NFREQGLO; f++)
        {
            pLam[f] = SatWavelenEx(Sat, f, pNav);
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

extern int GetClkIndex(int Sys)
{
    int k = -1;
    switch (Sys)
    {
    case SYS_GPS:k = GPSCLKINDEX; break;
    case SYS_CMP:k = BDSCLKINDEX; break;
    case SYS_GLO:k = GLOCLKINDEX; break;
    case SYS_GAL:k = GALCLKINDEX; break;
    case SYS_BD3:k = BD3CLKINDEX; break;
    default:break;
    }
    return k;
}

static int ReallocSsatlist(CHC_SSATLIST_T* pSsatlist)
{
    int deltn = 5;
    CHC_SSAT_T* pssat = NULL;
    if (pSsatlist->Num >= pSsatlist->NumMax)
    {
        pssat = VERIQC_MALLOC((pSsatlist->NumMax + deltn) * sizeof(CHC_SSAT_T));
        if (!pssat)
        {
            GNSS_LOG_FATAL("cannot malloc more memory, n=%d, nmax=%d", pSsatlist->Num, pSsatlist->NumMax);
            return -1;
        }
        memcpy(pssat, pSsatlist->pSsat, pSsatlist->NumMax * sizeof(CHC_SSAT_T));
        pSsatlist->NumMax += deltn;
        VERIQC_FREE(pSsatlist->pSsat);
        pSsatlist->pSsat = pssat;
        pssat = NULL;
    }
    return 1;
}

int UpdateSsatlist(const CHC_NAV_T* pNav, CHC_SSATLIST_T* pSsatlist, CHC_OBSD_T* pObsRover, int RoverObsNum, CHC_OBSD_T* pObsBase, int BaseObsNum)
{
    int i = 0, j = 0, k = 0, m = 0, index = -1, f = 0, deltn = 10, del = 0;
    double dt = 0.0, dtmax = 300.0, elemin = 3.0 * D2R, dtmin = 1.0E12;
    QC_TIME_T curtime = { 0 };
    double ep[6] = { 0 };
    int iLoop, fLoop;
    int sysIndex;

    if (!pSsatlist || (!pObsRover && !pObsBase))
    {
        return -1;
    }
    if (RoverObsNum > 0)
    {
        curtime = pObsRover[0].Time;
    }
    else if (BaseObsNum > 0)
    {
        curtime = pObsBase[0].Time;
    }

    //clear the satellites too old, e.g. 2 or 4 hours disappeared, and the elevation is too low
    for (i = 0; i < pSsatlist->Num; i++)
    {
        index = pSsatlist->Index[pSsatlist->pSsat[i].Sat - 1]; del = 0;
        for (f = 0; f < NFREQ; f++)
        {
            dt = fabs(QCTimeDiff(&curtime, &pSsatlist->pSsat[i].LockTime[f]));
            if (dt < dtmax) break;
        }
        if (f >= NFREQ) {
            for (j = 0; j < RoverObsNum; j++) if (pObsRover[j].Sat == pSsatlist->pSsat[i].Sat) break;
            if (j >= RoverObsNum)
            {
                for (k = 0; k < BaseObsNum; k++)
                    if (pObsBase[k].Sat == pSsatlist->pSsat[i].Sat)
                        break;
                if (k >= BaseObsNum)
                {
                    del = 1;
                }
            }
        }
        if (del) {
            pSsatlist->Index[pSsatlist->pSsat[i].Sat - 1] = -1;
            for (j = i + 1; j < pSsatlist->Num; j++) {
                pSsatlist->pSsat[j - 1] = pSsatlist->pSsat[j];
                pSsatlist->Index[pSsatlist->pSsat[j].Sat - 1] = j - 1;
            }
            pSsatlist->Num--; i--;
        }
    }
    for (i = 0; i < RoverObsNum; i++)
    {
        if (pSsatlist->Num >= pSsatlist->NumMax)
        {
            if (-1 == ReallocSsatlist(pSsatlist))
            {
                return -1;
            }
        }
        if (pSsatlist->Num >= pSsatlist->NumMax)
        {
            GNSS_LOG_WARN("rover sat in obs cannot insert into ssatlist, sat=%d, ssatlist->n=%d, ssatlist->nmax=%d",
                pObsRover[i].Sat, pSsatlist->Num, pSsatlist->NumMax);
            break;
        }

        if (pSsatlist->Index[pObsRover[i].Sat - 1] < 0)
        {
            memset(&pSsatlist->pSsat[pSsatlist->Num], 0, sizeof(CHC_SSAT_T));
            pSsatlist->pSsat[pSsatlist->Num].Sat = pObsRover[i].Sat;
            pSsatlist->pSsat[pSsatlist->Num].Sys = pObsRover[i].Sys;
            sysIndex = pObsRover[i].SysIndex;
            for (f = 0; f < NFREQ; f++)
            {
                if (SYS_GLO == pSsatlist->pSsat[pSsatlist->Num].Sys)
                {
                    pSsatlist->pSsat[pSsatlist->Num].Lam[f] = SatWavelenEx(pSsatlist->pSsat[pSsatlist->Num].Sat, f, pNav);
                    pSsatlist->pSsat[pSsatlist->Num].GLOFreq = pNav->GLOFreqNum[pObsRover[i].PRN - 1];
                }
                else
                {
                    pSsatlist->pSsat[pSsatlist->Num].Lam[f] = GetRawWaveLength(sysIndex, f);
                }
            }
            pSsatlist->Index[pObsRover[i].Sat - 1] = pSsatlist->Num;
            pSsatlist->Num++;
        }
        for (f = 0; f < NFREQ; f++)
        {
            if (pObsRover[i].Index[f] < 0) continue;
            pSsatlist->pSsat[pSsatlist->Index[pObsRover[i].Sat - 1]].LockTime[f] = pObsRover[i].Time;
        }
    }
    //update base station
    for (i = 0; i < BaseObsNum; i++)
    {
        if (pSsatlist->Num >= pSsatlist->NumMax)
        {
            if (-1 == ReallocSsatlist(pSsatlist))
            {
                return -1;
            }
        }
        if (pSsatlist->Num >= pSsatlist->NumMax)
        {
            GNSS_LOG_WARN("base sat in obs cannot insert into ssatlist, sat=%d, ssatlist->n=%d, ssatlist->nmax=%d",
                pObsBase[i].Sat, pSsatlist->Num, pSsatlist->NumMax);
            break;
        }
        if (pSsatlist->Index[pObsBase[i].Sat - 1] < 0)
        {
            memset(&pSsatlist->pSsat[pSsatlist->Num], 0, sizeof(CHC_SSAT_T));
            pSsatlist->pSsat[pSsatlist->Num].Sat = pObsBase[i].Sat;
            pSsatlist->pSsat[pSsatlist->Num].Sys = pObsBase[i].Sys;
            sysIndex = pObsBase[i].SysIndex;
            for (f = 0; f < NFREQ; f++)
            {
                if (SYS_GLO == pSsatlist->pSsat[pSsatlist->Num].Sys)
                {
                    pSsatlist->pSsat[pSsatlist->Num].Lam[f] = SatWavelenEx(pSsatlist->pSsat[pSsatlist->Num].Sat, f, pNav);
                    pSsatlist->pSsat[pSsatlist->Num].GLOFreq = pNav->GLOFreqNum[pObsBase[i].PRN - 1];
                }
                else
                {
                    pSsatlist->pSsat[pSsatlist->Num].Lam[f] = GetRawWaveLength(sysIndex, f);
                }
            }
            pSsatlist->Index[pObsBase[i].Sat - 1] = pSsatlist->Num;
            pSsatlist->Num++;
        }
        for (f = 0; f < NFREQ; f++)
        {
            if (pObsBase[i].Index[f] < 0) continue;
            pSsatlist->pSsat[pSsatlist->Index[pObsBase[i].Sat - 1]].LockTime[f] = pObsBase[i].Time;
        }
    }

    /* update glonass wavelength after receive geph */
    for (iLoop = 0; iLoop < pSsatlist->Num; iLoop++)
    {
        if (SYS_GLO != pSsatlist->pSsat[iLoop].Sys)
        {
            continue;
        }
        if (pSsatlist->pSsat[iLoop].Lam[0] < 1E-12)
        {
            for (fLoop = 0; fLoop < NFREQ; fLoop++)
            {
                pSsatlist->pSsat[iLoop].Lam[fLoop] = SatWavelenEx(pSsatlist->pSsat[iLoop].Sat, fLoop, pNav);
            }
        }
    }

    return 1;
}

/********************************************************************************************
@brief   get wave-length of raw gnss frequency

@param   SysIndex[In]          system index, see GetSystemIndexByID
@param   FreqIndex[In]         frequency index

@author  CHC

@return  wave-length(m). 0 if unsupported
@note    frequency index is same as obs frequency index. see convert_obsd()
*********************************************************************************************/
extern double GetRawWaveLength(int SysIndex, int FreqIndex)
{
    /* raw gnss signal wave length map */
    const static double s_RawGNSSWaveLengthMap[][CHC_NFREQ + CHC_NEXOBS] = {/* [sysIndex][freqIndex] */
    {LAM1_GPS,LAM2_GPS,LAM3_GPS,0.0,     0.0,     0.0,     0.0},     /* GPS */
    {LAM1_CMP,LAM2_CMP,LAM3_CMP,LAM4_CMP,LAM5_CMP,LAM6_CMP,LAM7_CMP},/* CMP */
    {LAM1_GAL,LAM2_GAL,LAM3_GAL,LAM4_GAL,0.0,     0.0,     0.0}      /* GAL */
    };

    if (SysIndex < 0)
    {
        return 0.0;
    }
    /**@todo s_Sys2IndexMap[sysIndex] => can be -1 */
    return g_Sys2IndexMap[SysIndex] >= 0 ? s_RawGNSSWaveLengthMap[g_Sys2IndexMap[SysIndex]][FreqIndex] : 0.0;
}


U1 InitSsatlist(CHC_SSATLIST_T* pSsatlist, int Num)
{
    int i = 0;
    if (!pSsatlist) return 0;
    if (!pSsatlist->pSsat)
    {
        pSsatlist->pSsat = (CHC_SSAT_T*)VERIQC_MALLOC(sizeof(CHC_SSAT_T) * Num);
        memset(pSsatlist->pSsat, 0, sizeof(CHC_SSAT_T) * Num);
        pSsatlist->Num = 0;
        pSsatlist->NumMax = Num;
        memset(pSsatlist->Index, -1, sizeof(short) * MAXSAT);
    }
    return 1;
}
U1 EndSsatlist(CHC_SSATLIST_T* pSsatlist)
{
    if (!pSsatlist) return 0;

    if (pSsatlist->pSsat)
    {
        VERIQC_FREE(pSsatlist->pSsat);
        pSsatlist->Num = 0;
        pSsatlist->NumMax = 0;
        pSsatlist->pSsat = NULL;
    }
    return 1;
}

extern int LSQPosFastSpt(const double* pA, const double* pTp, const double* pY, int N, int M, double* pX, double* pQt,
    double* pSigma02, double* pRMS, double* pResidual, double* pPe)
{
    double vPv = 0.0, vtv = 0.0, Qe[9], * Q;
    int info, i, j, k, kn, jn;
    double value;
    LSQ_MEMZONE_T lsqMemZone = { 0 };
    BOOL bLsqMemTypeFast = TRUE;
    VERIQC_MEM_MANAGER_T memManager;
    size_t mTotal = 0;
#ifdef VERIQC_TICKGET_ENABLE
    uint64_t stick, etick;
    uint32_t scpucycle, ecpucycle;
    stick = tickgetus();
    scpucycle = cpucycle();
#endif
    if (M < N) return 1;

    VeriQCMemManagerInit(&memManager);
    mTotal = (N + M * 3 + M * N) * sizeof(double);
    if (!pQt) mTotal += (N * N) * sizeof(double);
    VeriQCAddSize(&memManager, mTotal, 0);
    if (VeriQCAllocateWholeFast(&memManager) < 0)
    {
        if (VeriQCAllocateWhole(&memManager) < 0)
        {
            return 1;
        }
        bLsqMemTypeFast = FALSE;
    }
    lsqMemZone.pAPy = (double*)VeriQCRequestMemory(&memManager, N * sizeof(double), 0);
    lsqMemZone.pAP = (double*)VeriQCRequestMemory(&memManager, N * M * sizeof(double), 0);
    lsqMemZone.pV = (double*)VeriQCRequestMemory(&memManager, M * sizeof(double), 0);
    lsqMemZone.pAx = (double*)VeriQCRequestMemory(&memManager, M * sizeof(double), 0);
    lsqMemZone.pVP = (double*)VeriQCRequestMemory(&memManager, M * sizeof(double), 0);
    if (!pQt) Q = (double*)VeriQCRequestMemory(&memManager, N * N * sizeof(double), 0);
    else Q = pQt;

    if (pTp)
    {
        //AP
        for (k = 0; k < M; k++)
        {
            kn = k * N;
            for (j = 0; j < N; j++)
            {
                lsqMemZone.pAP[j + kn] = pA[j + kn] * pTp[k];
            }
        }
    }
    else
    {
        memcpy(lsqMemZone.pAP, pA, sizeof(double) * N * M);
    }

    MatMul(MAT_MUL_TYPE_NN, N, 1, M, 1.0, lsqMemZone.pAP, pY, 0.0, lsqMemZone.pAPy); /* Ay=A*P*y */

    memset(Q, 0, sizeof(double) * N * N);
    for (j = 0; j < N; j++)
    {
        jn = j * N;
        for (k = 0; k < M; k++)
        {
            kn = k * N;
            value = pA[j + kn];
            for (i = 0; i < N; i++)
            {
                if (i > j) continue;
                Q[i + jn] += lsqMemZone.pAP[i + kn] * value;
                Q[j + i * N] = Q[i + jn];
            }
        }
    }
    if (pPe && pPe[0] > 0.0)
    {
        memcpy(Qe, pPe, 9 * sizeof(double));
        if (!MatInvSym(Qe, 3))
        {
            for (i = 0; i < 3; i++)
            {
                Q[0 + i * N] += Qe[0 + i * 3];
                Q[1 + i * N] += Qe[1 + i * 3];
                Q[2 + i * N] += Qe[2 + i * 3];
            }
        }
    }
    if (!(info = MatInvSym(Q, N)))
    {
        MatMul(MAT_MUL_TYPE_NN, N, 1, N, 1.0, Q, lsqMemZone.pAPy, 0.0, pX); /* x=Q^-1*APy */
        MatMul(MAT_MUL_TYPE_TN, M, 1, N, 1.0, pA, pX, 0.0, lsqMemZone.pAx); /* Ax=A'*x */

        for (i = 0; i < M; i++) lsqMemZone.pV[i] = pY[i] - lsqMemZone.pAx[i];/*v=y-Ax*/

        if (pResidual)
        {
            for (i = 0; i < M; i++) pResidual[i] = lsqMemZone.pV[i];
        }
        if (pSigma02)
        {
            if (pTp)
            {
                for (k = 0; k < M; k++)
                {
                    lsqMemZone.pVP[k] = lsqMemZone.pV[k] * pTp[k];
                }
                MatMul(MAT_MUL_TYPE_NN, 1, 1, M, 1.0, lsqMemZone.pVP, lsqMemZone.pV, 0.0, &vPv);  /* vPv=V'*P*V */
            }
            if (M > N) *pSigma02 = vPv / (M - N);
        }
        if (pRMS)
        {
            MatMul(MAT_MUL_TYPE_TN, 1, 1, M, 1.0, lsqMemZone.pV, lsqMemZone.pV, 0.0, &vtv);  /* vtv=V'*V */
            *pRMS = sqrt(vtv / M);
        }
    }

    if (TRUE == bLsqMemTypeFast)
    {
        VeriQCMemManagerFreeFast(&memManager);
    }
    else
    {
        VeriQCMemManagerFree(&memManager);
    }

#ifdef VERIQC_TICKGET_ENABLE
    etick = tickgetus();
    ecpucycle = cpucycle();
    VERIQC_printf("T,%d,lsq_p_fast_spt,n=%d,m=%d,st=%llu,et=%llu,dt=%lld,sc=%u,ec=%u,dc=%d,lsq_p_fast_spt\r\n",
        taskget(), m, n, stick, etick, etick - stick, scpucycle, ecpucycle, ecpucycle - scpucycle);
#endif

    return info;
}

/********************************************************************************************
@brief   get IF equation type

@param   SysIndex[In]          system index, see GetSystemIDByIndex
@param   LCFreqIndex[In]       linear combination option(0 ~ NFREQ_CM)
@param   pFreq1[Out]           first raw frequency index(0 ~ NFREQ)
@param   pFreq2[Out]           second raw frequency index(0 ~ NFREQ)

@author  CHC

@return  EQ_IF??
@note    see GetIFFreqByEquationType
*********************************************************************************************/
extern int GetIFEquationType(int SysIndex, int LCFreqIndex, int* pFreq1, int* pFreq2)
{
    const static int s_IFEquationType[CHC_NFREQ_IF * 2] = {
        EQ_IF12,EQ_IF13,EQ_IF14,EQ_IF15,EQ_IF45,
        EQ_PLIF12,EQ_PLIF13,EQ_PLIF14,EQ_PLIF15,EQ_PLIF45
    };
    const static int s_IFFreqIndex[][CHC_NFREQ_IF * 2] = {
        {0,0,0,0,3,0,0,0,0,3},
        {1,2,3,4,4,1,2,3,4,4}
    };
    int obsType;

    obsType = s_IFEquationType[LCFreqIndex];
    *pFreq1 = s_IFFreqIndex[0][LCFreqIndex];
    *pFreq2 = s_IFFreqIndex[1][LCFreqIndex];
    if ((1 == SysIndex) || (4 == SysIndex))
    {
        if (2 == LCFreqIndex)
        {
            obsType = EQ_IF34;
            *pFreq1 = 2;
            *pFreq2 = 3;
        }
        else if (6 == LCFreqIndex)
        {
            obsType = EQ_PLIF34;
            *pFreq1 = 2;
            *pFreq2 = 3;
        }
    }

    return obsType;
}

/* constants -----------------------------------------------------------------*/

extern void InitialOptDefault(CHC_PRCOPT_T* pOpt)
{
    int i = 0;
    pOpt->Mode = PMODE_SINGLE;
    pOpt->SolType = 0;
    pOpt->FreqNum = 2;
    pOpt->NavSys = SYS_GPS | SYS_CMP | SYS_GLO | SYS_GAL | SYS_BD3;
    pOpt->EleMin = 8.0 * D2R;
    pOpt->SatEph = EPHOPT_BRDC;
    pOpt->IonoOpt = IONOOPT_EST;
    pOpt->TropOpt = TROPOPT_SAAS;
    pOpt->Dynamics = 0;
    for (i = 0; i < CHC_NFREQ; i++)
    {
        pOpt->ErrorRatio[i] = 100.0;
    }
    pOpt->ErrorFactor[0] = 100.0;
    pOpt->ErrorFactor[1] = 0.01;
    pOpt->ErrorFactor[2] = 0.01;
    pOpt->ErrorFactor[3] = 0.0;
    pOpt->ErrorFactor[4] = 1.0;
    pOpt->InitStateSTD[0] = 30.0;
    pOpt->InitStateSTD[1] = 0.03;
    pOpt->InitStateSTD[2] = 0.3;
    pOpt->ProcessNoiseSTD[0] = 1E-4;
    pOpt->ProcessNoiseSTD[1] = 1E-3;
    pOpt->ProcessNoiseSTD[2] = 1E-4;
    pOpt->ProcessNoiseSTD[3] = 1E-1;
    pOpt->ProcessNoiseSTD[4] = 1E-2;
    pOpt->MaxGDOP = 30.0;
    memset(pOpt->ExSats, 0, CHC_MAXSAT);
    pOpt->SatNumLimit = MAXDDOBS + SYSNUM;
    pOpt->EphInterpolation = 1;
    pOpt->SPPGap = 0.0f;
    pOpt->CroverDopplerVel = 1;

}

/* satellite system+prn/slot number to satellite number ------------------------
* convert satellite system+prn/slot number to satellite number
* args   : int    sys       I   satellite system (SYS_GPS,SYS_GLO,...)
*          int    prn       I   satellite prn/slot number
* return : satellite number (0:error)
*-----------------------------------------------------------------------------*/
extern int SatNo(int Sys, int PRN)
{
    if (PRN <= 0) return 0;
    switch (Sys) {
    case SYS_GPS:
        if (PRN < MINPRNGPS || MAXPRNGPS < PRN) return 0;
        return PRN - MINPRNGPS + 1;
    case SYS_GLO:
        if (PRN < MINPRNGLO || MAXPRNGLO < PRN) return 0;
        return NSATGPS + PRN - MINPRNGLO + 1;
    case SYS_GAL:
        if (PRN < MINPRNGAL || MAXPRNGAL < PRN) return 0;
        return NSATGPS + NSATGLO + PRN - MINPRNGAL + 1;
    case SYS_QZS:
        if (PRN < MINPRNQZS || MAXPRNQZS < PRN) return 0;
        return NSATGPS + NSATGLO + NSATGAL + PRN - MINPRNQZS + 1;
    case SYS_BD3:
    case SYS_CMP:
        if (PRN < MINPRNCMP || MAXPRNCMP < PRN) return 0;
        return NSATGPS + NSATGLO + NSATGAL + NSATQZS + PRN - MINPRNCMP + 1;
    case SYS_SBS:
        if (PRN < MINPRNSBS || MAXPRNSBS < PRN) return 0;
        return NSATGPS + NSATGLO + NSATGAL + NSATQZS + NSATCMP + PRN - MINPRNSBS + 1;
    }
    return 0;
}
/* satellite number to satellite system ----------------------------------------
* convert satellite number to satellite system
* args   : int    Sat        I   satellite number (1-MAXSAT)
*          int    *pPRN      IO  satellite prn/slot number (NULL: no output)
* return : satellite system (SYS_GPS,SYS_GLO,...)
*-----------------------------------------------------------------------------*/
extern int SatSys(int Sat, int* pPRN)
{
    int sys = SYS_NONE;
    if (Sat <= 0 || MAXSAT < Sat) Sat = 0;
    else if (Sat <= NSATGPS) {
        sys = SYS_GPS; Sat += MINPRNGPS - 1;
    }
    else if ((Sat -= NSATGPS) <= NSATGLO) {
        sys = SYS_GLO; Sat += MINPRNGLO - 1;
    }
    else if ((Sat -= NSATGLO) <= NSATGAL) {
        sys = SYS_GAL; Sat += MINPRNGAL - 1;
    }
    else if ((Sat -= NSATGAL) <= NSATQZS) {
        sys = SYS_QZS; Sat += MINPRNQZS - 1;
    }
    else if ((Sat -= NSATQZS) <= NSATCMP) {
        sys = SYS_CMP; Sat += MINPRNCMP - 1;
#ifdef BDS23
        if (sat >= 19) sys = SYS_BD3;
#endif
    }
    else if ((Sat -= NSATCMP) <= NSATSBS) {
        sys = SYS_SBS; Sat += MINPRNSBS - 1;
    }
    else Sat = 0;
    if (pPRN) *pPRN = Sat;
    return sys;
}

extern int IsValidPhase(double Phase)
{
    int stat = 1;
    if (fabs(Phase) <= ZEROSVALUE)
    {
        stat = 0;
    }
    return stat;
}
extern int IsValidDoppler(double Dopler)
{
    int stat = 1;
    if (fabs(Dopler) <= ZEROSVALUE)
    {
        stat = 0;
    }
    return stat;
}

extern int IsValidPseu(double Pseu)
{
    int stat = 1;

    if (Pseu < 1.5E2 || Pseu > 3.55E8)
    {
        stat = 0;
    }

    return stat;
}
extern int IsValidSNR(int SNR)
{
    int stat = 1;

    if (SNR <= 0 || SNR > 100)
    {
        stat = 0;
    }

    return stat;
}



/* new integer matrix ----------------------------------------------------------

extern u1* cmat(int n, int m)
{
    u1* p;
    if (n <= 0 || m <= 0) return NULL;
    if (!(p = (u1*)VERIQC_malloc(sizeof(u1) * n * m)))
    {
        fatalerr("char matrix memory allocation error: n=%d,m=%d\n", n, m);
    }
    return p;
}

/* outer product of 3d vectors -------------------------------------------------


/* LU decomposition ----------------------------------------------------------*/
static int LuDcmp(double* pA, int N, int* pIndx, double* pD)
{
    double big, s, tmp, * vv = Mat(N, 1);
    int i, imax = 0, j, k;
    int jn, ijn, kn;

    *pD = 1.0;
    for (i = 0; i < N; i++)
    {
        big = 0.0; for (j = 0; j < N; j++) if ((tmp = fabs(pA[i + j * N])) > big) big = tmp;
        if (big > 0.0) vv[i] = 1.0 / big; else { VERIQC_FREE(vv); return -1; }
    }
    for (j = 0; j < N; j++)//col
    {
        jn = j * N;
        for (i = 0; i < j; i++) {//row
            ijn = i + j * N;
            s = pA[ijn];
            for (k = 0; k < i; k++) s -= pA[i + k * N] * pA[k + jn]; pA[ijn] = s;
        }
        big = 0.0;
        for (i = j; i < N; i++) {//row
            ijn = i + j * N;
            s = pA[ijn];
            for (k = 0; k < j; k++) s -= pA[i + k * N] * pA[k + jn]; pA[ijn] = s;
            if ((tmp = vv[i] * fabs(s)) >= big) {
                big = tmp;
                imax = i;
            }//tmp:l
        }
        if (j != imax) {
            for (k = 0; k < N; k++) {
                kn = k * N;
                tmp = pA[imax + kn]; pA[imax + kn] = pA[j + kn]; pA[j + kn] = tmp;
            }
            *pD = -(*pD); vv[imax] = vv[j];
        }
        pIndx[j] = imax;
        if (pA[j + jn] == 0.0) { VERIQC_FREE(vv); return -1; }
        if (j != N - 1) {
            tmp = 1.0 / pA[j + jn]; for (i = j + 1; i < N; i++) pA[i + jn] *= tmp;
        }
    }
    VERIQC_FREE(vv);
    return 0;
}
/* LU back-substitution ------------------------------------------------------*/
static void LuBkSb(const double* pA, int N, const int* pIndx, double* pB)
{
    double s;
    int i, ii = -1, ip, j;
    int iin;

    for (i = 0; i < N; i++) {
        ip = pIndx[i]; s = pB[ip]; pB[ip] = pB[i];
        if (ii >= 0) for (j = ii; j < i; j++) s -= pA[i + j * N] * pB[j]; else if (s) ii = i;
        pB[i] = s;
    }
    for (i = N - 1; i >= 0; i--) {
        s = pB[i];
        iin = i + i * N;
        for (j = i + 1; j < N; j++) s -= pA[i + j * N] * pB[j]; pB[i] = s / pA[iin];
    }
}


/* inverse of matrix ---------------------------------------------------------*/
extern int MatInv(double* pA, int N)
{
    double d, * B;
    int i, j, jn, * indx;
    VERIQC_MEM_MANAGER_T memManager;
    int32_t mTotal;

    VeriQCMemManagerInit(&memManager);
    mTotal = N * sizeof(int) + N * N * sizeof(double);
    VeriQCAddSize(&memManager, mTotal, 0);
    if (VeriQCAllocateWhole(&memManager) < 0)
    {
        return -1;
    }
    indx = (int*)VeriQCRequestMemory(&memManager, N * sizeof(int), 0);
    B = (double*)VeriQCRequestMemory(&memManager, N * N * sizeof(double), 0);
    //indx=imat(n,1); B=mat(n,n);
    MatCpy(B, pA, N, N);
    if (LuDcmp(B, N, indx, &d))
    {
        VeriQCMemManagerFree(&memManager);
        return -1;
    }
    for (j = 0; j < N; j++)
    {
        jn = j * N;
        for (i = 0; i < N; i++)
        {
            pA[i + jn] = 0.0;
        }
        pA[j + jn] = 1.0;
        LuBkSb(B, N, indx, pA + jn);
    }

    VeriQCMemManagerFree(&memManager);
    return 0;
}


/* solve linear equation -----------------------------------------------------*/
extern int Solve(MAT_MUL_TYPE_E Type, const double* pA, const double* pY, int N,
    int M, double* pX)
{
    double* B = Mat(N, N);
    int info;

    MatCpy(B, pA, N, N);
    if (!(info = MatInv(B, N))) MatMul(Type, N, M, N, 1.0, B, pY, 0.0, pX);
    VERIQC_FREE(B);
    return info;
}

/* end of matrix routines ----------------------------------------------------*/

/* get tick time ---------------------------------------------------------------
* get current tick in ms
* args   : none
* return : current tick in ms
*-----------------------------------------------------------------------------*/
extern unsigned int TickGet(void)
{
    return VERIQC_TICKGET();
}

/**********************************************************************//**
@brief convert glonass time(leapyear/daynum/sod) to internal time struct

@param LeapYears       [In]  four-year interval number starting from 1996, based on 1
@param DayNum          [In]  Days number of year, based on 1
@param SecOfDay        [In]  second of day

@return gtime
@author CHC
@date 2023/11/09

@note
History:
**************************************************************************/
extern QC_TIME_T GLOEphTime2Time(int LeapYears, int DayNum, double SecOfDay)
{
    static int s_UTCDaysAcc[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int iMonth = 0;
    int Seconds, LeapDay = 0;
    int FirstYear = 0; /**< first year number of 4years - interval */
    int IncludeLeapYear = 0;
    int FirstDoy = 0; /**< total day number of first year */
    int UTCYears = 0;
    double epoch[6];

    SecOfDay -= 10800;
    if (SecOfDay < 0)
    {
        SecOfDay += 86400;
        DayNum--;
    }
    SecOfDay -= s_Leaps[0][6]; /* utc2gpst */
    Seconds = (int)(SecOfDay);
    UTCYears = LeapYears * 4;

    FirstYear = UTCYears + 1992;
    if ((!(FirstYear % 4) && (FirstYear % 100)) || !(FirstYear % 400))
    {
        IncludeLeapYear = 1; /**< first year of four - year interval is a intercalary year */
        FirstDoy = 366;
    }
    else
    {
        FirstDoy = 365;
    }

    DayNum--; /**< change DayNumber's base from 1 to 0 */
    if (DayNum >= (FirstDoy + 365 * 2))
    {
        DayNum -= (FirstDoy + 365 * 2);
        UTCYears += 3;
    }
    else if (DayNum >= (FirstDoy + 365))
    {
        DayNum -= (FirstDoy + 365);
        UTCYears += 2;
    }
    else if (DayNum >= FirstDoy)
    {
        DayNum -= FirstDoy;
        UTCYears++;
    }
    else if (IncludeLeapYear)
    {
        if (DayNum >= 60)
        {
            DayNum--;
        }
        else if (DayNum == 59)
        {
            LeapDay = 1;
        }
    }

    for (iMonth = 1; iMonth < 12; iMonth++)
    {
        if (DayNum < s_UTCDaysAcc[iMonth])
            break;
    }

    if (LeapDay)
    {
        epoch[1] = 2;
        epoch[2] = 29;
    }
    else
    {
        epoch[1] = iMonth;
        epoch[2] = DayNum - (s_UTCDaysAcc[iMonth - 1] - 1);
    }
    epoch[0] = 1992 + UTCYears;
    epoch[3] = (int)(Seconds / 3600);
    Seconds -= (int)(epoch[3] * 3600);
    epoch[4] = (int)(Seconds / 60);
    epoch[5] = Seconds - epoch[4] * 60;

    return Epoch2QCTime(epoch);
}

/**********************************************************************//**
@brief convert internal time struct to glonass time(leapyear/daynum/sod)

@param pTime           [In]   gtime
@param pLeapYears      [Out]  four-year interval number starting from 1996, based on 1
@param pDayNum         [Out]  Days number of year, based on 1
@param pSecOfDay       [Out]  second of day

@return none
@author CHC
@date 2023/11/09

@note
History:
**************************************************************************/
extern void Time2GLOEphTime(QC_TIME_T* pTime, int* pLeapYear, int* pDayNum, double* pSecOfDay)
{
    static int s_UTCDaysAcc[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int FirstYear;
    int DayCount;
    int YearCount;
    double epoch[6];

    QCTime2Epoch(*pTime, epoch);
    /** Get Day's MsCount of GLONASS Time */
    *pSecOfDay = ((epoch[3] * 60) + epoch[4]) * 60 + epoch[5] + 10800;

    /** year number since 1992 */
    YearCount = (int)(epoch[0] - 1992);
    DayCount = s_UTCDaysAcc[(int)(epoch[1] - 1)] + (int)epoch[2] - 1;

    /** Get first year number of this 4 - years interval */
    FirstYear = (YearCount / 4) * 4 + 1992;
    if ((!(FirstYear % 4) && (FirstYear % 100)) || !(FirstYear % 400)) /**< first year is leap year */
    {
        if (((YearCount % 4) != 0) || (epoch[1] > 2))
        {
            DayCount++;
        }
    }

    DayCount += (YearCount % 4) * 365;
    *pDayNum = DayCount + 1;
    *pLeapYear = YearCount / 4;

    if (*pDayNum > 1461)
    {
        *pDayNum -= 1461;
        *pLeapYear += 1;
    }
}

/**********************************************************************//**
@brief convert gps/gal/bds/qzs time(week/sow) to internal time struct

@param Sys           [In]  system
@param Week          [In]  week number of input system
@param SOW           [In]  second of week of input system

@return time
@author CHC
@date 2023/11/09

@note
History:
**************************************************************************/
extern QC_TIME_T UNIEphTime2Time(int Sys, int Week, double SOW)
{
    if (SYS_CMP == Sys)
    {
        return GPSTime2QCTime(Week + 1356, SOW + 14);
    }
    if (SYS_GAL == Sys)
    {
        return GPSTime2QCTime(Week + 1024, SOW);
    }

    return GPSTime2QCTime(Week, SOW);
}

/* transform covariance to local tangental coordinate --------------------------
* transform ecef covariance to local tangental coordinate
* args   : double *pPos      I   geodetic position {lat,lon} (rad)
*          double *pP        I   covariance in ecef coordinate
*          double *pQ        O   covariance in local tangental coordinate
* return : none
*-----------------------------------------------------------------------------*/
extern void CovENU(const double* pPos, const double* pP, double* pQ)
{
    double E[9], EP[9];

    PosXYZ2ENU(pPos, E);
    MatMul(MAT_MUL_TYPE_NN, 3, 3, 3, 1.0, E, pP, 0.0, EP);
    MatMul(MAT_MUL_TYPE_NT, 3, 3, 3, 1.0, EP, E, 0.0, pQ);
}

extern void SolToCov(const CHC_SOL_T* pSol, double* pP)
{
    pP[0] = pSol->Qr[0]; /* xx or ee */
    pP[4] = pSol->Qr[1]; /* yy or nn */
    pP[8] = pSol->Qr[2]; /* zz or uu */
    pP[1] = pP[3] = pSol->Qr[3]; /* xy or en */
    pP[5] = pP[7] = pSol->Qr[4]; /* yz or nu */
    pP[2] = pP[6] = pSol->Qr[5]; /* zx or ue */
}
extern void SolVelToCov(const CHC_SOL_T* pSol, double* pP)
{
    pP[0] = pSol->QVel[0]; /* xx or ee */
    pP[4] = pSol->QVel[1]; /* yy or nn */
    pP[8] = pSol->QVel[2]; /* zz or uu */
    pP[1] = pP[3] = pSol->QVel[3]; /* xy or en */
    pP[5] = pP[7] = pSol->QVel[4]; /* yz or nu */
    pP[2] = pP[6] = pSol->QVel[5]; /* zx or ue */
}

/* debug trace functions -----------------------------------------------------*/

/* satellite azimuth/elevation angle -------------------------------------------
* compute satellite azimuth/elevation angle
* args   : double *pPos      I   geodetic position {lat,lon,h} (rad,m)
*          double *pE        I   receiver-to-satellilte unit vevtor (ecef)
*          double *pAzEl     IO  azimuth/elevation {az,el} (rad) (NULL: no output)
*                               (0.0<=azel[0]<2*pi,-pi/2<=azel[1]<=pi/2)
* return : elevation angle (rad)
*-----------------------------------------------------------------------------*/
extern double SatAzEl(const double* pPos, const double* pE, double* pAzEl)
{
    double az = 0.0, el = PI / 2.0, enu[3];

    if (pPos[2] > -RE_WGS84) {
        ECEF2ENU(pPos, pE, enu);
        az = Dot(enu, enu, 2) < 1E-12 ? 0.0 : atan2(enu[0], enu[1]);
        if (az < 0.0) az += 2 * PI;
        el = asin(enu[2]);
    }
    if (pAzEl) { pAzEl[0] = az; pAzEl[1] = el; }
    return el;
}
/* compute dops ----------------------------------------------------------------
* compute DOP (dilution of precision)
* args   : int    ObsNum     I   number of satellites
*          double *pAzEl     I   satellite azimuth/elevation angle (rad)
*          double EleMin     I   elevation cutoff angle (rad)
*          double *pDop      O   DOPs {GDOP,PDOP,HDOP,VDOP}
* return : none
* notes  : dop[0]-[3] return 0 in case of dop computation error
*-----------------------------------------------------------------------------*/
extern void Dops(int ObsNum, const double* pAzEl, double EleMin, double* pDop)
{
    double H[4 * MAXSAT], Q[16], cosel, sinel;
    int i, n;

    for (i = 0; i < 5; i++) pDop[i] = 0.0;
    for (i = n = 0; i < ObsNum && i < MAXSAT; i++) {
        if (pAzEl[1 + i * 2] < EleMin || pAzEl[i * 2] <= 0.0) continue;//azel[1+i*2]<=0.0) continue
        cosel = cos(pAzEl[1 + i * 2]);
        sinel = sin(pAzEl[1 + i * 2]);
        H[4 * n] = cosel * sin(pAzEl[i * 2]);
        H[1 + 4 * n] = cosel * cos(pAzEl[i * 2]);
        H[2 + 4 * n] = sinel;
        H[3 + 4 * n++] = 1.0;
    }
    if (n < 4) return;

    MatMul(MAT_MUL_TYPE_NT, 4, 4, n, 1.0, H, H, 0.0, Q);
    if (!MatInvSym(Q, 4)) {
        pDop[0] = SQRT(Q[0] + Q[5] + Q[10] + Q[15]); /* GDOP */
        pDop[1] = SQRT(Q[0] + Q[5] + Q[10]);       /* PDOP */
        pDop[2] = SQRT(Q[0] + Q[5]);             /* HDOP */
        pDop[3] = SQRT(Q[10]);                 /* VDOP */
        pDop[4] = SQRT(Q[15]);                 /* TDOP */
    }
}

/**********************************************************************//**
@brief Calculate Ionosphere Delay using

@param pInputIonoParam [In] Ionosphere Parameters
@param pPosBLH         [In] Receiver's Position in BLH, in rad,rad,m
@param GPSSecond       [In] Receiver's Second of System
@param pAzEl           [In] azimuth and elevation  of satellite, in radian

@return Iono Delay (m), 0.0 if Iono Delay Value not reasonable

@author CHC
@date 2022/11/12
@note
History:
**************************************************************************/
extern double IonoDelayGPS(const IONO_PARAM_T* pInputIonoParam, const double* pPosBLH, double GPSSecond, const double* pAzEl)
{
    float SatAzi = (float)pAzEl[0];
    const float El = (float)(pAzEl[1]) / (float)PI;
    float Lat = (float)pPosBLH[0] / (float)PI;
    float Lon = (float)pPosBLH[1] / (float)PI;

    float phi, F, PER, x, AMP, x1;
    float fTemp;
    double ionoDelay = 0.0;
    float T;

    static const IONO_PARAM_T ionoParamDefault = { /**< 2015/7/17 */
        0.7451E-08f,0.2235E-07f,-0.5960E-07f, -0.1192E-06f,
        0.9011E+05f,0.1147E+06f,-0.6554E+05f, -0.5243E+06f ,1 };

    const IONO_PARAM_T* pIonoParam;

    if (El <= 0.0f)
    {
        return 0.0;
    }

    if (!pInputIonoParam->IonoFlag)
    {
        pIonoParam = &ionoParamDefault;
    }
    else
    {
        pIonoParam = pInputIonoParam;
    }

    phi = 0.0137f / (El + 0.11f) - 0.022f;
    Lat += phi * (float)cos(SatAzi);
    if (Lat > 0.416f)
    {
        Lat = 0.416f;
    }
    else if (Lat < -0.416f)
    {
        Lat = -0.416f;
    }

    Lon += phi * (float)sin(SatAzi) / (float)cos(Lat * PI);
    Lat += 0.064f * (float)cos((Lon - 1.617) * (float)PI);
    fTemp = 0.53f - El;
    F = 1.0f + 16.0f * (fTemp * fTemp * fTemp);
    PER = pIonoParam->Beta0 + (pIonoParam->Beta1 + (pIonoParam->Beta2 + pIonoParam->Beta3 * Lat) * Lat) * Lat;
    if (PER < 72000.0f)
    {
        PER = 72000.0f;
    }

    T = (float)((43200 * Lon) + GPSSecond);

    if ((T > 1E8f) || (T < -1E8f))
    {
        return 0.0;
    }
    while (T >= 86400.0f)
    {
        T -= 86400.0f;
    }
    while (T < 0.0)
    {
        T += 86400.0f;
    }
    x = (2.0f * (float)PI) * (T - 50400.0f) / PER;

    if (x >= 1.57f || x <= -1.57f)
    {
        ionoDelay = F * 5e-9;
    }
    else
    {
        AMP = pIonoParam->Alpha0 + (pIonoParam->Alpha1 + (pIonoParam->Alpha2 + pIonoParam->Alpha3 * Lat) * Lat) * Lat;
        if (AMP < 0.0)
        {
            ionoDelay = F * 5e-9;
        }
        else
        {
            x *= x;
            x1 = 1.0f - x / 2.0f;
            x *= x;
            x1 += x / 24.0f;
            ionoDelay = F * (5e-9 + AMP * x1);
        }
    }

    return CLIGHT * ionoDelay;
}

/*********************************************************************************//**
@brief Calculate Ionosphere Delay using

@param pInputIonoParam [In] Ionosphere Parameters
@param pPosBLH         [In] Receiver's Position in BLH, in rad,rad,m
@param BDSSecond       [In] Receiver's Second of System
@param pAzEl           [In] azimuth and elevation  of satellite, in radian

@return Iono Delay (m),  0.0 if Iono Delay Value not reasonable

@author CHC
@date   8 December 2023
*************************************************************************************/
extern double IonoDelayBDS(const IONO_PARAM_T* pInputIonoParam, const double* pPosBLH, double BDSSecond, const double* pAzEl)
{
    float tt, lam, phi, amp, per;
    double r, psi, x;
    float sinPsi;

    static const IONO_PARAM_T ionoParamDefault = { /**<  2023/11/15 */
        4.190952E-08f, -5.960464E-08f, -4.172325E-07f, 7.748604E-07f,
        1.105920E+05f, 1.966080E+05f, -1.507328E+06f, 1.703936E+06f,1 };

    const IONO_PARAM_T* pIonoParam;

    if (pAzEl[1] <= 0.0)
    {
        return 0.0;
    }

    if (!pInputIonoParam->IonoFlag)
    {
        pIonoParam = &ionoParamDefault;
    }
    else
    {
        pIonoParam = pInputIonoParam;
    }

    /** earth centered angle (semi-circle) */
    r = 0.94446912483 * cos(pAzEl[1]);  /**<  6378.0 / (6378.0 + 375.0) */
    psi = PI * 0.5 - pAzEl[1] - asin(r);

    /** subionospheric latitude/longitude (semi-circle) */
    sinPsi = (float)sin(psi);
    phi = (float)asin(sin(pPosBLH[0]) * cos(psi) + cos(pPosBLH[0]) * sinPsi * cos(pAzEl[0]));
    lam = (float)(pPosBLH[1] + asin(sinPsi * sin(pAzEl[0]) / cos(phi)));

    phi = (float)fabs(phi / PI);
    per = pIonoParam->Beta0 + phi * (pIonoParam->Beta1 + phi * (pIonoParam->Beta2 + phi * pIonoParam->Beta3));
    per = per < 72000.0f ? 72000.0f : (per > 172800.0f ? 172800.0f : per);

    /** local time (s) */
    tt = (float)((43200.0 / PI * lam) + BDSSecond);
    if ((tt > 1E8f) || (tt < -1E8f))
    {
        return 0.0;
    }
    while (tt >= 86400.0f)
    {
        tt -= 86400.0f;
    }
    while (tt < 0.0f)
    {
        tt += 86400.0f;
    }

    tt -= 50400.0f;
    x = 5E-9;
    if (fabs(tt) < (per * 0.25))
    {
        amp = pIonoParam->Alpha0 + phi * (pIonoParam->Alpha1 + phi * (pIonoParam->Alpha2 + phi * pIonoParam->Alpha3));
        if (amp > 0.0)
        {
            x += amp * cos(2.0 * PI * tt / per);
        }
    }

    return (CLIGHT * x / (sqrt(1.0 - r * r)));
}

/*********************************************************************************//**
@brief get broadcast iono corr value, convert to this sys first freq iono delay

@param GPSSecond [In] GPS second in week
@param pNav      [In] nav info
@param Sys       [In] sat sys, see SYS_???
@param pPosBLH   [In] Receiver's Position in BLH, in rad,rad,m
@param pAzEl     [In] azimuth and elevation  of satellite, in radian
@param pSsat     [In] sat state info list
@param pIono     [Out] ionospheric delay (L1) (m) of this sys
@param pVar      [Out] ionospheric delay (L1) variance (m^2)

@return 1

@author CHC
@date   12 December 2023
*************************************************************************************/
extern int IonoCorrBroadcast(double GPSSecond, const CHC_NAV_T* pNav, int Sys, const double* pPosBLH,
    const double* pAzEl, const CHC_SSAT_T* pSsat, double* pIono, double* pVar)
{
    double lam;
    BOOL bConvert = FALSE;
    double weekSecond = GPSSecond;
    double temp;
    IONO_PARAM_T* pIonoParam;

    if (SYS_NONE == Sys)
    {
        return 0;
    }

    switch (Sys)
    {
    case SYS_GPS:
        *pIono = IonoDelayGPS(pNav->pIonoParamGPS, pPosBLH, weekSecond, pAzEl);
        break;
    case SYS_CMP:
    case SYS_BD3:
        pIonoParam = pNav->pIonoParamBDS;
        if (pIonoParam->IonoFlag)
        {
            weekSecond -= 14.0;  /**< GPS time second in week -> BDS time second in week*/
            *pIono = IonoDelayBDS(pNav->pIonoParamBDS, pPosBLH, weekSecond, pAzEl);
        }
        else
        {
            bConvert = TRUE;
            *pIono = IonoDelayGPS(pNav->pIonoParamGPS, pPosBLH, weekSecond, pAzEl);
        }
        break;
    case SYS_QZS:
        *pIono = IonoDelayGPS(pNav->pIonoParamQZS, pPosBLH, weekSecond, pAzEl);
    default:
        bConvert = TRUE;
        *pIono = IonoDelayGPS(pNav->pIonoParamGPS, pPosBLH, weekSecond, pAzEl);
        break;
    }
    temp = *pIono * ERR_BRDCI;
    *pVar = temp * temp;

    /** convert to this sys first freq iono delay */
    if (bConvert)
    {
        lam = pSsat->Lam[0];
        temp = lam / LAM1_GPS;
        *pIono *= (lam > 0.0 ? (temp * temp) : 1.0);
    }
    return 1;
}

/* calculate trop total delay by SAAS model with input pressure and temperature */
extern int GetTropModelConstant(const double* pPos, double Pres0, double Temp0, double Humi, double* pTrphXs, double* pTrpwXs)
{
    double hgt, pres, temp, e;
    if (1E4 < pPos[2])
    {
        *pTrphXs = *pTrpwXs = 0.0;
        return 0;
    }
    hgt = pPos[2] < 0.0 ? 0.0 : pPos[2];

    pres = Pres0 * pow(1.0 - 2.2557E-5 * hgt, 5.2568);
    temp = Temp0 - 6.5E-3 * hgt + 273.16;
    e = 6.108 * Humi * exp((17.15 * temp - 4684.0) / (temp - 38.45));
    *pTrphXs = 0.0022768 * pres / (1.0 - 0.00266 * cos(2.0 * pPos[0]) - 0.00028 * hgt * 1E-3);
    *pTrpwXs = 0.002277 * (1255.0 / temp + 0.05) * e;

    return 1;
}
extern double TropModelInputSeaLevelParaAdv(QC_TIME_T Time, const double* pPos, const double* pAzEl, double TrphXs, double TrpwXs)
{
    double  z, trph, trpw;
    double tmpcosz;
    if (1E4 < pPos[2] || pAzEl[1] <= 0) return 0.0;

    z = HALFPI - pAzEl[1];
    tmpcosz = 1.0 / cos(z);

    trph = TrphXs * tmpcosz;
    trpw = TrpwXs * tmpcosz;
    return trph + trpw;
}

/********************************************************************************************
@brief   check sat obs used in solution

@param   pObs          [In] raw obs
@param   pSatXYZ       [In] satellite position
@param   ObsValidMask  [In] obs valid mask(rove or rove+base)
@param   NavSys        [In] navssys option
@param   EleMin        [In] el threshold(rad)
@param   pSsat         [In/Out] satellite information struct

@author  CHC

@return  number of used satellite
*********************************************************************************************/
extern BOOL CheckSatUsed(const CHC_OBSD_T* pObs, const double* pSatXYZ, unsigned int ObsValidMask, int NavSys, double EleMin, CHC_SSAT_T* pSsat)
{
    int fLoop;
    BOOL bUsed = FALSE;

    if (!(pObs->Sys & NavSys))
    {
        pSsat->SatStatus = SAT_STATUS_NOEPH;
        return FALSE;
    }
    if (Norm(pSatXYZ, 3) < RE_WGS84)
    {
        pSsat->SatStatus = SAT_STATUS_NOEPH;
        return FALSE;
    }
    if (pSsat->AzEl[1] < EleMin)
    {
        pSsat->SatStatus = SAT_STATUS_LOWELEV;
        return FALSE;
    }
    if (pObs->SysIndex < 0)
    {
        pSsat->SatStatus = SAT_STATUS_NOTUSED;
        return FALSE;
    }

    pSsat->SatStatus = SAT_STATUS_GOOD;
    for (fLoop = 0; fLoop < NFREQ; fLoop++)
    {
        if (SYS_GLO == NavSys && fLoop >= 2)
        {
            continue;
        }
        if (ObsValidMask & (BIT_MASK_CODE(fLoop) | BIT_MASK_PHASE(fLoop)))/* phase or code is valid */
        {
            pSsat->UsedFrequencyMark |= (1 << fLoop);
            bUsed = TRUE;
        }
    }
    if (TRUE == bUsed)
    {
        pSsat->ValidSatFlagSingle = SATVIS_SOLVING;
    }

    return bUsed;
}

/********************************************************************************************
@brief   update satellite output information

@param   pOpt          [In] process option
@param   pObs          [In] raw obs(rove)
@param   SatNum        [In] number of obs
@param   pSatPos       [In] satellite position
@param   pSsatList     [In/Out] satellite list information
@param   pDops         [Out] dops

@author  CHC

@return  number of used satellite
*********************************************************************************************/
extern int GetSatOutInfo(CHC_PRCOPT_T* pOpt, const CHC_OBSD_T* pObs, int SatNum, const double* pSatPosVel,
    CHC_SSATLIST_T* pSsatList, double* pDops)
{
    int iObs;
    const CHC_OBSD_T* pRoveObs;
    CHC_SSAT_T* pSsat;
    int iSatNo;
    int nSat = 0;
    int fLoop;
    double azel[MAXOBS * 2] = { 0 };

    for (iObs = 0; iObs < SatNum; iObs++)
    {
        pRoveObs = pObs + iObs;
        iSatNo = pRoveObs->Sat;

        pSsat = &pSsatList->pSsat[pSsatList->Index[iSatNo - 1]];
        for (fLoop = 0; fLoop < NFREQ; fLoop++)
        {
            if (pRoveObs->ObsValidMask & BIT_MASK_SNR(fLoop))
            {
                pSsat->SNR[fLoop] = (unsigned char)(pRoveObs->SNR[pRoveObs->Index[fLoop]] * 4 + 0.5);
            }
        }
        if (TRUE == CheckSatUsed(pRoveObs, pSatPosVel + iObs * 6, pRoveObs->ObsValidMask,
            pOpt->NavSys, pOpt->EleMin, pSsat))
        {
            azel[nSat * 2] = pSsat->AzEl[0];
            azel[nSat * 2 + 1] = pSsat->AzEl[1];
            nSat++;
        }
    }
    if (nSat >= 4)
    {
        Dops(nSat, azel, pOpt->EleMin, pDops);
    }

    return nSat;
}
