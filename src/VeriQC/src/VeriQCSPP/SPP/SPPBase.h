/*************************************************************************//**
           VeriQC
         SPP Module
 *-
@file   SPPBase.h
@author CHC
@date   5 September 2022
@brief  common SPP related structures and functions
*****************************************************************************/

#include "GNSSBase.h"
#include "VeriQCSPP.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SQRT(x)     ((x)<=0.0?0.0:sqrt(x))

#define SQR(x)      ((x)*(x))   /**< @todo recommand to change to SQ, the current macro is easy to confuse with SQRT**/
   
#define EQ_L1 0
#define EQ_L2 1
#define EQ_L3 2
#define EQ_L4 3
#define EQ_L5 4
#define EQ_L6 5
#define EQ_L7 6
#define EQ_P1 7
#define EQ_P2 8
#define EQ_P3 9
#define EQ_P4 10
#define EQ_P5 11
#define EQ_P6 12
#define EQ_P7 13
#define EQ_PURE_NUM 14

#define EQ_IF12    EQ_PURE_NUM
#define EQ_IF13   (EQ_PURE_NUM+1)
#define EQ_IF14   (EQ_PURE_NUM+2)
#define EQ_IF15   (EQ_PURE_NUM+3)
#define EQ_IF34   (EQ_PURE_NUM+4)
#define EQ_IF45   (EQ_PURE_NUM+5)
#define EQ_PLIF12 (EQ_PURE_NUM+6)
#define EQ_PLIF13 (EQ_PURE_NUM+7)
#define EQ_PLIF14 (EQ_PURE_NUM+8)
#define EQ_PLIF15 (EQ_PURE_NUM+9)
#define EQ_PLIF34 (EQ_PURE_NUM+10)
#define EQ_PLIF45 (EQ_PURE_NUM+11)
#define EQ_IF_NUM 12
 
#define LSQMAXPARAM (7+BD23) //x/y/z/4 system clk/ for spp
#define GPSCLKINDEX 0
#define GLOCLKINDEX 1
#define BDSCLKINDEX 2
#define GALCLKINDEX 3
#define BD3CLKINDEX 4
    
typedef struct {
    double *pAPy;
    double *pAP;
    double *pV;
    double *pAx;
    double *pVP;
}LSQ_MEMZONE_T;

typedef struct {
    int VSat[MAXOBS];
    int VSatSort[MAXOBS];
    double AzEl[2 * MAXOBS];
    double AzElSort[2 * MAXOBS];
    MIX_PSEUOBS_T MixPseuObs;
}PNTPOS_ELEMENT_T;

typedef struct {
    double V[SPP_SATNUMLMT_INNER];
    double H[LSQMAXPARAM*SPP_SATNUMLMT_INNER];
    double HTrans[LSQMAXPARAM*SPP_SATNUMLMT_INNER];
    double Var[SPP_SATNUMLMT_INNER];
    double VarAmp[SPP_SATNUMLMT_INNER];
    double Res[SPP_SATNUMLMT_INNER];
    double STDRes[SPP_SATNUMLMT_INNER];
}ESTPOS_EXCAR_ELEMENT_T;

typedef struct {
    double Pxs[MAXSAT];
    double Residual[SPP_SATNUMLMT_INNER];
    double STDRes[SPP_SATNUMLMT_INNER];
    double ATQ[SPP_SATNUMLMT_INNER*LSQMAXPARAM];
    double ATQA[SPP_SATNUMLMT_INNER*SPP_SATNUMLMT_INNER];
    double V[SPP_SATNUMLMT_INNER];
    double H[LSQMAXPARAM*SPP_SATNUMLMT_INNER];
    double Pw[SPP_SATNUMLMT_INNER];
    double HTrans[LSQMAXPARAM*SPP_SATNUMLMT_INNER];
}ESTVEL_ELEMENT_T;

    extern double SatWavelenEx(int Sat, int Freq, const CHC_NAV_T *pNav);
    extern int SortObsEx(CHC_OBSD_T *pEpochData,int ObsNum);
    
	extern double PRangeSF(int Sys, int Sat, int ObsType, double P, U1 Code, double *pVar, const CHC_NAV_T *pNav);
	extern double PRangeIF(char SysIndex, int Sys, int Sat, int ObsType, const double *pP, const U1 *pCode, double *pVar, const CHC_NAV_T *pNav,
		const short* pObsFreqIndex, int* pFirstFreq, int* pSecondFreq);
    int GetErrPseuSatBySPP(const CHC_SSATLIST_T *pSsatlist, const CHC_OBSD_T *pObs, int Num, const double *pRs, const double *pDts,
        const CHC_NAV_T * pNav, const CHC_PRCOPT_T * pOpt, const double *pRr, U1 *pInvalid, double *pGRatio, double ErrLimit);
    extern int ExtractObsToSPP(const CHC_OBSD_T* pObs, int ObsNum, const CHC_PRCOPT_T* pOpt, const double* pRs, const double* pDts, const double* pVare,
        const CHC_NAV_T* pNav, MIX_PSEUOBS_T* pSPPObs, U1 bSpecifySat, const U1* pSpecifySatList, int MaxSatNum);
    extern int PNTPosExNew(const CHC_OBSD_T *pObs, int Num, const CHC_NAV_T * pNav, const CHC_PRCOPT_T *pOpt,
        CHC_SOL_T *pSol, char *pMsg, const double *pRs, const double *pDts, const double *pVar, CHC_SSATLIST_T * pSsatlist);
    extern int ResCodeExcar(const CHC_SSATLIST_T* pSsatlist, const MIX_PSEUOBS_T *pMixPseuObs, const CHC_PRCOPT_T * pOpt, const CHC_NAV_T * pNav, const double *pX, double *pV, double *pH, double *pVar, int *pVSat, double *pVarAmp, U1 *pValidFlag, double *pAzEl, int SpeciSys);
    extern U1 EstVelEx(const CHC_OBSD_T* pObs, int Num, const double* pRs, const double* pDts, const CHC_NAV_T* pNav, const CHC_PRCOPT_T* pOpt, CHC_SOL_T* pSol, const double* pAzEl, const int* pVSat, CHC_SSATLIST_T* pSsatlist);
    extern int EstPosPro(const MIX_PSEUOBS_T * pMixPseuObs, const CHC_NAV_T * pNav,
        const CHC_PRCOPT_T *pOpt, CHC_SOL_T *pSol, int *pVSat, double *pAzEl, char *pMsg, float SPPGap, CHC_SSATLIST_T *pSsatlist);

	extern double VarErrSPPMix(const CHC_PRCOPT_T * pOpt, U1 ObsType, double Ele, int Sys);

    /************WINDOWS***********/

    
    /************inner*************/	
    extern double GetPxsy(double VStd, double K0, double K1, U1 EnMinLmtMark);
    extern void QuickSortD(const double* pValArray, int NumEle, int* pSortedIndex, BOOL bAscendingMark, BOOL bAbsMark);
    
    extern int GetClkBk(CHC_PRCOPT_T *pOpt, CHC_SOL_T *pSol, CHC_SSATLIST_T * pSsatlist, const CHC_OBSD_T *pObs, const double *pRs, const double *pDts, int ObsNum,
        const CHC_NAV_T *pNav, U1 ID);

    extern U1 SettleObs(CHC_OBSD_T *pObs, int Num);
    extern int UpdateSatAzEl(CHC_SSATLIST_T* pSsatList, const CHC_OBSD_T* pObs, int ObsNum, const double* pRs, const double* pRr);
	U1 InitAdjRobustBuffer(CHC_ADJ_ROBUST_BUFFER_T *pAdjRobustBuffer,int Nv,int NPar);
    U1 EndAdjRobustBuffer(CHC_ADJ_ROBUST_BUFFER_T * pAdjRobustBuffer);
    U1 ComputeStandardResSpt(double *pRess,const CHC_ADJ_ROBUST_BUFFER_T * pAdjRobustBuffer,const double *pV,int Nv,double Sigma02,double *pTp);
    U1 ComputeAdjFactorSpt(CHC_ADJ_ROBUST_BUFFER_T * pAdjRobustBuffer, const double *pA, const double *pQ, const double *pTp, int Nv, int NPar);
    /***special for glonass*****/
    //IF
    //pure
    //NW
    /*****end special for glonass****/


    extern double GetRawWaveLength(int SysIndex,int FreqIndex);
    extern double GetCoeffPIF1(int SysIndex, int LCFreqIndex);
    extern double GetCoeffPIF2(int SysIndex, int LCFreqIndex);
    extern int GetRawEquationType(int FreqIndex);
    extern int GetRawFreqByEquationType(int EquType);
    extern int GetIFEquationType(int SysIndex, int LCFreqIndex, int* pFreq1, int* pFreq2);
    extern int GetIFFreqByEquationType(int SysIndex, int ObsType, int* pFreq1, int* pFreq2);

    extern U1 GetSysLam(int Sys, int Sat, double* pLam, const CHC_NAV_T* pNav);

    int UpdateSsatlist(const CHC_NAV_T* pNav, CHC_SSATLIST_T* pSsatlist, CHC_OBSD_T* pObsRover, int RoverObsNum, CHC_OBSD_T* pObsBase, int BaseObsNum);
    U1 InitSsatlist(CHC_SSATLIST_T* pSsatlist, int Num);
    U1 EndSsatlist(CHC_SSATLIST_T* pSsatlist);
    extern double AvgSTD(const double* pA, int Num, double* pSTD);
    extern int GetClkIndex(int Sys);

    extern int GetSatOutInfo(CHC_PRCOPT_T* pOpt, const CHC_OBSD_T* pObs, int SatNum, const double* pSatPosVel,
        CHC_SSATLIST_T* pSsatList, double* pDops);
    
#ifdef __cplusplus
}
#endif
