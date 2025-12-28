/*************************************************************************//**
           VeriQC
    Data Quality Check Module
 *- 
@file   CommonBase.h
@author CHC
@date   5 September 2022
@brief  Some quality check algorithm common interfaces
*****************************************************************************/
#ifndef _DATAQC_ALGORITHM_BASE_H_
#define _DATAQC_ALGORITHM_BASE_H_
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "DataQC.h"

/*----------------------------------------------------*/
/* code type defined */
#define QC_CODE_NONE   0                   /* obs code: none or unknown */
#define QC_CODE_L1C    1                   /* obs code: L1C/A,G1C/A,E1C (GPS,GLO,GAL,QZS,SBS) */
#define QC_CODE_L1P    2                   /* obs code: L1P,G1P    (GPS,GLO) */
#define QC_CODE_L1W    3                   /* obs code: L1 Z-track (GPS) */
#define QC_CODE_L1Y    4                   /* obs code: L1Y        (GPS) */
#define QC_CODE_L1M    5                   /* obs code: L1M        (GPS) */
#define QC_CODE_L1N    6                   /* obs code: L1codeless (GPS) */
#define QC_CODE_L1S    7                   /* obs code: L1C(D)     (GPS,QZS) */
#define QC_CODE_L1L    8                   /* obs code: L1C(P)     (GPS,QZS) */
#define QC_CODE_L1E    9                   /* obs code: L1-SAIF    (QZS) */
#define QC_CODE_L1A    10                  /* obs code: E1A        (GAL) */
#define QC_CODE_L1B    11                  /* obs code: E1B        (GAL) */
#define QC_CODE_L1X    12                  /* obs code: E1B+C,L1C(D+P) (GAL,QZS) */
#define QC_CODE_L1Z    13                  /* obs code: E1A+B+C,L1SAIF (GAL,QZS) */
#define QC_CODE_L2C    14                  /* obs code: L2C/A,G1C/A (GPS,GLO) */
#define QC_CODE_L2D    15                  /* obs code: L2 L1C/A-(P2-P1) (GPS) */
#define QC_CODE_L2S    16                  /* obs code: L2C(M)     (GPS,QZS) */
#define QC_CODE_L2L    17                  /* obs code: L2C(L)     (GPS,QZS) */
#define QC_CODE_L2X    18                  /* obs code: L2C(M+L),B1I+Q (GPS,QZS,CMP) */
#define QC_CODE_L2P    19                  /* obs code: L2P,G2P    (GPS,GLO) */
#define QC_CODE_L2W    20                  /* obs code: L2 Z-track (GPS) */
#define QC_CODE_L2Y    21                  /* obs code: L2Y        (GPS) */
#define QC_CODE_L2M    22                  /* obs code: L2M        (GPS) */
#define QC_CODE_L2N    23                  /* obs code: L2codeless (GPS) */
#define QC_CODE_L5I    24                  /* obs code: L5/E5aI    (GPS,GAL,QZS,SBS) */
#define QC_CODE_L5Q    25                  /* obs code: L5/E5aQ    (GPS,GAL,QZS,SBS) */
#define QC_CODE_L5X    26                  /* obs code: L5/E5aI+Q  (GPS,GAL,QZS,SBS) */
#define QC_CODE_L7I    27                  /* obs code: E5bI,B2I   (GAL,CMP) */
#define QC_CODE_L7Q    28                  /* obs code: E5bQ,B2Q   (GAL,CMP) */
#define QC_CODE_L7X    29                  /* obs code: E5bI+Q,B2I+Q (GAL,CMP) */
#define QC_CODE_L6A    30                  /* obs code: E6A        (GAL) */
#define QC_CODE_L6B    31                  /* obs code: E6B        (GAL) */
#define QC_CODE_L6C    32                  /* obs code: E6C        (GAL) */
#define QC_CODE_L6X    33                  /* obs code: E6B+C,LEXS+L,B3I+Q (GAL,QZS,CMP) */
#define QC_CODE_L6Z    34                  /* obs code: E6A+B+C/L6 (GAL,QZS-Block II) */
#define QC_CODE_L6S    35                  /* obs code: LEXS       (QZS) */
#define QC_CODE_L6L    36                  /* obs code: LEXL       (QZS) */
#define QC_CODE_L8I    37                  /* obs code: E5(a+b)I   (GAL) */
#define QC_CODE_L8Q    38                  /* obs code: E5(a+b)Q   (GAL) */
#define QC_CODE_L8X    39                  /* obs code: E5(a+b)I+Q (GAL), BD3-B2a+b */
#define QC_CODE_L2I    40                  /* obs code: B1I        (CMP) */
#define QC_CODE_L2Q    41                  /* obs code: B1Q        (CMP) */
#define QC_CODE_L6I    42                  /* obs code: B3I        (CMP) */
#define QC_CODE_L6Q    43                  /* obs code: B3Q        (CMP) */
#define QC_CODE_L3I    44                  /* obs code: G3I        (GLO) */
#define QC_CODE_L3Q    45                  /* obs code: G3Q        (GLO) */
#define QC_CODE_L3X    46                  /* obs code: G3I+Q      (GLO) */
#define QC_CODE_L1I    47                  /* obs code: B1I        (BDS) */
#define QC_CODE_L1Q    48                  /* obs code: B1Q        (BDS) */
#define QC_CODE_L5D    49                  /* obs code: L5D        (QZS-Block II) */
#define QC_CODE_L5P    50                  /* obs code: L5P        (QZS-Block II) */
#define QC_CODE_L5Z    51                  /* obs code: L5Z        (QZS-Block II) */
#define QC_CODE_L6E    52                  /* obs code: L6E        (QZS-Block II) */
#define QC_CODE_L7D    53                  /* obs code: B2b        (BDS3-B2b) */
#define QC_CODE_L7P    54                  /* obs code: B2b        (BDS3-B2b) */
#define QC_CODE_L7Z    55                  /* obs code: B2b        (BDS3-B2b) */
#define QC_CODE_L1D    56                  /* obs code: B1Q        (BDS3-B1) */
#define QC_CODE_L8D    57                  /* obs code: B2a+b      (BDS3-B2) */
#define QC_CODE_L8P    58                  /* obs code: B2a+b      (BDS3-B2) */
#define QC_CODE_L6D    59                  /* obs code: B3A   D    (BDS3) */
#define QC_CODE_L6P    60                  /* obs code: B3A   P    (BDS3) */


#define QC_FREQ1       1.57542E9           /* L1/E1  frequency (Hz) */
#define QC_FREQ2       1.22760E9           /* L2     frequency (Hz) */
#define QC_FREQ5       1.17645E9           /* L5/E5a frequency (Hz) */
#define QC_FREQ6       1.27875E9           /* E6/LEX frequency (Hz) */
#define QC_FREQ7       1.20714E9           /* E5b    frequency (Hz) */
#define QC_FREQ8       1.191795E9          /* E5a+b  frequency (Hz) */

#define QC_FREQ1_GLO   1.60200E9           /* GLONASS G1 base frequency (Hz) */
#define QC_FREQ1A_GLO  1.600995E9          /* GLONASS G1a CDMA frequency (Hz) */
#define QC_DFRQ1_GLO   0.56250E6           /* GLONASS G1 bias frequency (Hz/n) */
#define QC_FREQ2_GLO   1.24600E9           /* GLONASS G2 base frequency (Hz) */
#define QC_FREQ2A_GLO  1.24806E9           /* GLONASS G2a CDMA frequency (Hz) */
#define QC_DFRQ2_GLO   0.43750E6           /* GLONASS G2 bias frequency (Hz/n) */
#define QC_FREQ3_GLO   1.202025E9          /* GLONASS G3 frequency (Hz) */
#define	QC_DFRQ3_GLO   0.42300E6           /* GLONASS G3 bias frequency (Hz/n) */

#define QC_FREQ2_CMP   1.561098E9          /* BeiDou B1 frequency (Hz) */
#define QC_FREQ7_CMP   1.20714E9           /* BeiDou B2 frequency (Hz) */
#define QC_FREQ6_CMP   1.26852E9           /* BeiDou B3 frequency (Hz) */
#define QC_FREQ1_BD3   QC_FREQ1			   /* BeiDou BD3-B1 frequency (Hz) */
#define QC_FREQ5_BD3   QC_FREQ5			   /* BeiDou BD3-B2a frequency (Hz) */
#define QC_FREQ8_BD3   QC_FREQ8			   /* BeiDou BD3-B2 frequency (Hz) */
#define QC_FREQ7_BD3   1.20714E9           /* BeiDou BD3-B2b frequency (Hz) */
        
#define QC_LAM1_GPS 0.19029367279836487//#define lam1_GPS (CLIGHT/FREQ1)
#define QC_LAM2_GPS 0.24421021342456825//#define lam2_GPS (CLIGHT/FREQ2)
#define QC_LAM3_GPS 0.25482804879085386//#define lam3_GPS (CLIGHT/FREQ5)
        
#define QC_LAM1_CMP 0.19203948631027648 //#define lam1_CMP (CLIGHT/FREQ2_CMP) 
#define QC_LAM2_CMP 0.24834936958430670//#define lam2_CMP  (CLIGHT/FREQ7_CMP) 
#define QC_LAM3_CMP 0.23633246460442089//#define lam3_CMP  (CLIGHT/FREQ6_CMP)
#define QC_LAM4_CMP 0.19029367279836488047631742646405 //#define lam4_CMP (CLIGHT/FREQ1_BD3) 
#define QC_LAM5_CMP 0.2548280487908538399421989884823//#define lam5_CMP  (CLIGHT/FREQ5_BD3) 
#define QC_LAM6_CMP 0.2515470009523449922176213191027//#define lam6_CMP  (CLIGHT/FREQ8_BD3)
        
#define QC_LAM1_GAL 0.19029367279836488047631742646405     //#define lam1_GAL (CLIGHT/CHC_FREQ1/E1) 
#define QC_LAM2_GAL 0.2548280487908538399421989884823      //#define lam2_GAL (CLIGHT/CHC_FREQ5/E5a) 
#define QC_LAM3_GAL 0.24834936958430670841824477691071     //#define lam3_GAL (CLIGHT/CHC_FREQ7/E5b)
#define QC_LAM4_GAL 0.2515470009523449922176213191027      //#define lam3_GAL  (CLIGHT/CHC_FREQ8/E5a+b)
#define QC_LAM5_GAL 0.23444180488758553274682306940371     //#define lam5_GAL  (CLIGHT/CHC_FREQ6/E6)

/** common function */
double QCGetLam(int Sys, int FreID);
double QCCode2Fre(int Sys, int Code);
double QCCode2FreGLO(int Code, int FreGLO);
double QCCode2LamGLO(int Code, int FreGLO);
double GetRawMP(QC_OBSD_T* pObsd, int Freq0, int* pCombi);
int GetCombType(QC_OBSD_T* pObsd, int* pFreq);
BOOL MpProcessorFixed(QC_OBSD_T* pObsd, QC_RESULT_T* pQcRes, int AntID, int SatNo, int* pGfSlip, QC_TIME_T ObsTime);
BOOL MpProcessorFilter(QC_OBSD_T* pObsd, QC_RESULT_T* pQcRes, int AntID, int SatNo, int* pSlip, QC_TIME_T ObsTime);

void SlipProcessEntrance(QC_OBSD_T* pObsdCur, QC_RESULT_T* pQcRes, int AntID, int SatNo, QC_TIME_T* pObsTime);
int QCSlipProcessor(QC_OBSD_T* pObsdPre, QC_OBSD_T* pObsdCur, QC_RESULT_T* pQcRes, int AntID, int SatNo, QC_TIME_T* pObsTime, unsigned char* pFreProcessFlag);

#endif