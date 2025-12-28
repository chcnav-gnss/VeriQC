/*********************************************************************************//**
               VeriQC
            SPP Module
 *- 
@file   ProcInterface.h
@author CHC
@date   8 March 2024
@brief  SPP interface head file for APP
*************************************************************************************/

#ifndef _PROC_INTERFACE_H_
#define _PROC_INTERFACE_H_

#include "VeriQCSPP.h"

#include "GNSSBase.h"

#ifdef WIN32
#ifndef OUTLOG
#define OUTLOG(msg,...) \
    do { \
        char logbuff[1024]={0}; \
        sprintf(logbuff,msg,__VA_ARGS__); \
        OutputDebugString(logbuff); \
    } while(0);
#endif
#else
#define OUTLOG(msg,...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /** Definition of the overall solution object SPPTC */
    /** Multiple filters can be configured */
    typedef struct _VERIQC_SPPTC_PROCESSOR_T
    {
        struct _CHC_OBS_T* pObsuFasterBuff;                                         //0-free;1-buffered;
        uint8_t ObsuFasterBuffValidFlag;
        VERIQC_ORBIT_SOL_T OrbitSolFaster;                                          //Satellite position information for rover station calculations.-faster
        struct _CHC_OBS_T* pObsBase;                                                //Reference station antenna observation data.
        struct _CHC_OBS_T* pObsRover;                                               //Rover station antenna observation data.
        struct _CHC_OBS_T* pObsBaseBuff;                                            //Reference station antenna observation data_buff
        double PosRefBuff[3];
        int PosRefRefIDBuff;
        uint8_t ObsBaseBuffValidFlag;                                               //0-free;1-buffered;
        uint8_t PosRefBuffValidFlag;                                                //0-free;1-buffered;
        uint8_t RefMark;                                                            //0x01:ref coor, 0x02:ref obs
        VERIQC_ORBIT_SOL_T OrbitSolRover;                                           //Satellite position information for rover station calculations
        VERIQC_ORBIT_SOL_T OrbitSolBase;                                            //Satellite position information for base station calculations
        VERIQC_SPPTC_CREATE_OPT_T CreateOpt0;                                       //Options only for object creation.
        VERIQC_SPPTC_OPT_T Opt;                                                     //VeriQC overall configuration items.
        VERIQC_SOLINFO_T Sol;                                                       //VeriQC solution results. [finnal sol]
        struct _VERIQC_SPP_PROCESSOR_T* pSPPProc;                                   //There is usually only one SPP processor.
        struct _VERIQC_ORBIT_PROCESSOR_T* pOrbitProc;                               //Generally created externally, only one orbit processor is needed, and multiple spptc objects can share it.

        int SceneLevel;   //restore scenelevel in preader->spptc,for gnss integrity statistic
        QC_TIME_T SceneTime;
        QC_TIME_T LastFilterTime;

        unsigned char TimeStatus;  /* rove range obs time status. 0:UNKNOWN, 100:COARSE, 160:FINE, 200:SATTIME */
        unsigned char Reserved[3];
    }VERIQC_SPPTC_PROCESSOR_T;


struct _CHC_PRCOPT_T;
struct _CHC_SOL_T;
struct _CHC_OBS_T;

struct _VERIQC_SPPTC_OPT_T;
struct _CHC_NAV_T;
struct _CHC_SATIPTINFO_T;
struct _QC_TIME_T;
	
//class of spp processor
typedef struct _VERIQC_SPP_PROCESSOR_T
{
    //members:
    unsigned char OptAlloc;					//opt malloc strategy. 1:malloc real memory,0:only set pointer from outside
    unsigned char SolAlloc;					//sol malloc strategy. 1:malloc real memory,0:only set pointer from outside
    struct _CHC_PRCOPT_T* pOpt;
    struct _CHC_SOL_T* pSol;

    CHC_SPP_SOLVE_T* pSPP;
}VERIQC_SPP_PROCESSOR_T;

U1 InitOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult, int Num);
U1 ExpandOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult, int AddNum, U1 KeepFlag);
U1 EndOnePseuResult(CHC_ONE_PSEURESULT_T* pOnePseuResult);

//class of orbit processor 
typedef struct _VERIQC_ORBIT_PROCESSOR_T
{
    //members:
    struct _VERIQC_SPPTC_OPT_T* pOpt;         /*configuration struct*/
    struct _CHC_NAV_T* pNav;                  /*basic ephemeris struct*/
    struct _CHC_SATIPTINFO_T* pSatIptInfo;    /*satellite orbit fitting information*/
    struct _CHC_PCVS_T* pCVS;
    int SatInfoNum;                           /*number of satellite orbit fitting information*/
    unsigned char* pEphValidFlag;             /*ephemeris fitting update status, 0 : not uodate / finish fitting, 1 : update*/
    unsigned char* pEphActiveValidFlag;
    unsigned char EphValidFlagNum;
    struct _QC_TIME_T* pEphTime;              /*gps time of ephemeris*/
    int* pEphIndex;
    int* pEphIndexBD3;

    struct _CHC_SATIPTINFO_T* pSatIptInfoBD3;   //BD3 satellite orbit fitting information
    unsigned char* pEphValidFlagBD3;                 //BD3 ephemeris fitting update status, 0:not uodate/finish fitting, 1:update
    unsigned char* pEphActiveValidFlagBD3;
    unsigned char  EphValidFlagBD3Num;
}VERIQC_ORBIT_PROCESSOR_T;

/** SPP processor interface define */
typedef struct _VERIQC_SPP_PROCESSOR_T* (*VERIQC_CREATE_SPP_PROCESSOR_FUNC)(uint8_t Optalloc, uint8_t SolAlloc, int SysNum, int FreqNum);
typedef void(*VERIQC_RELEASE_SPP_PROCESSOR_FUNC)(struct _VERIQC_SPP_PROCESSOR_T** ppObj);
typedef void(*VERIQC_SPP_SETOPT_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj, struct _CHC_PRCOPT_T* pOpt);
typedef struct _CHC_PRCOPT_T* (*VERIQC_SPP_GETOPT_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);
typedef int(*VERIQC_SPP_PROCESS_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj, struct _CHC_OBS_T* pObs, struct _CHC_NAV_T* pNav,
    struct _VERIQC_ORBIT_SOL_T* pOrbSol);
typedef int(*VERIQC_SPP_SETSOL_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj, struct _CHC_SOL_T* pSol);
typedef struct _CHC_SOL_T* (*VERIQC_SPP_GETSOL_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);
typedef CHC_SPP_SOLVE_T* (*VERIQC_SPP_GETSLOVE_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);
typedef CHC_SOL_T* (*VERIQC_SPP_GETONESOL_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);
typedef CHC_SSATLIST_T*(*VERIQC_SPP_GETSSATLIST_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);
typedef CHC_PRCOPT_T*(*VERIQC_SPP_GETSOLVEOPT_FUNC)(struct _VERIQC_SPP_PROCESSOR_T* pObj);

/** SPP processor interface */
typedef struct VERIQC_spp_ifx_s
{
    VERIQC_CREATE_SPP_PROCESSOR_FUNC CreateFunc;
    VERIQC_RELEASE_SPP_PROCESSOR_FUNC ReleaseFunc;
    VERIQC_SPP_SETOPT_FUNC SetOptFunc;
    VERIQC_SPP_GETOPT_FUNC GetOptFunc;
    VERIQC_SPP_PROCESS_FUNC ProcessFunc;
    VERIQC_SPP_SETSOL_FUNC SetSolFunc;
    VERIQC_SPP_GETSOL_FUNC GetSolFunc;
    VERIQC_SPP_GETSLOVE_FUNC GetSolveFunc;
    VERIQC_SPP_GETONESOL_FUNC GetOneSolFunc;
    VERIQC_SPP_GETSSATLIST_FUNC GetSsatlistFunc;
    VERIQC_SPP_GETSOLVEOPT_FUNC GetSolveOptFunc;
}VERIQC_SPP_IFX_T;


/** Interface func */
VERIQC_SPP_IFX_T* VeriQCGetSPPIfx();

typedef enum _VERIQC_SIZE_STRUCT_E
{
    VERIQC_SIZE_CHC_OBS = 0,          /**< sizeof(CHC_OBS_T) */
    VERIQC_SIZE_PROCESSING_OPT,       /**< sizeof(CHC_PRCOPT_T) */
    VERIQC_SIZE_MAX = 0xFFFFFFFF
}VERIQC_SIZE_STRUCT_E;

extern int VeriQCCopyObs(const struct _CHC_OBS_T* pSrc, struct _CHC_OBS_T* pDst, int IncrementMark);

#ifdef __cplusplus
}
#endif

#endif
