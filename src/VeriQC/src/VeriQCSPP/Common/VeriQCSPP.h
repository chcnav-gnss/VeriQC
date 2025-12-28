/********************************************************************************************
* @file          VeriQCSPP.h
* @brief         Top level interface for SPP
* @author        CHCNAV
* @date          2020-03-17
* @version       v001
* @copyright     CHCNAV
*********************************************************************************************/
#ifndef _VERI_QC_SPP_H_
#define _VERI_QC_SPP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined VERIQC_SURVEY
    /**---------------------------------- SURVEY ----------------------------------*/
#define VERIQC_FLAG    0    /** application scene: 0:SURVEY, 1:AGRI,2:CAR, 3:UAV, 4:USV */
#define RESOURCE_FLAG  1    /** CPU resource: 0:low resource,1: enough resource */
    
#endif

/************************GNSS INTERFACE********************************************/

#define VERIQC_SYS_NUM                              4               /* real system number */
#define VERIQC_NUM_FREQ                             5               /* max frequency number */
#define VERIQC_MAX_DIR                              4096
#define VERIQC_MAX_FILTER                           5               /* max supported filter */
#define VERIQC_ANT_ROVER                            1
#define VERIQC_ANT_BASE                             2
#define VERIQC_NSATGPS                              32
#define VERIQC_NSATGLO                              28
#define VERIQC_NSATGAL                              36
#define VERIQC_NSATQZS                              7  //193-202
#define VERIQC_NSATBDS                              64
#define VERIQC_NSATSBS                              19
#define VERIQC_MAXSAT                               (VERIQC_NSATGPS+VERIQC_NSATGLO+VERIQC_NSATGAL+VERIQC_NSATQZS+VERIQC_NSATBDS+VERIQC_NSATSBS)

#define VERIQC_L1   0x0001
#define VERIQC_L2   0x0002
#define VERIQC_L5   0x0004

#define VERIQC_B1I  0x0001
#define VERIQC_B2I  0x0002
#define VERIQC_B3I  0x0004
#define VERIQC_B1C  0x0008
#define VERIQC_B2A  0x0010
#define VERIQC_B2   0x0020
#define VERIQC_B2B  0x0040

#define VERIQC_G1   0x0001
#define VERIQC_G2   0x0002
#define VERIQC_G3   0x0004

#define VERIQC_E1   0x0001
#define VERIQC_E5B  0x0002
#define VERIQC_E5A  0x0004
#define VERIQC_E5AB 0x0008
#define VERIQC_E6   0x0010


/************************** basic type defined in csppnl*************************************/
/*********************************************************************************************/
#include <stdbool.h> /*true false*/
#include <stddef.h>                 /*size_t NULL*/
#include <stdint.h>                 /*int32_t...*/

#include "TimeDefines.h"

#ifdef _MSC_VER
#pragma warning(disable : 4232)
#endif

typedef unsigned char U1;
typedef signed char C1;
typedef double F8;
typedef unsigned int INT32U;
typedef int INT32S;
typedef unsigned char INT8U;
typedef unsigned short INT16U;
typedef int BOOL;
#ifndef TRUE
#define TRUE (1)
#define FALSE (0)
#endif
	
/********** gnss top define ************/
enum VERIQC_SYS_IDX
{
    VERIQC_SYS_NONE = 0x00,
    VERIQC_SYS_GPS = 0x01,
    VERIQC_SYS_SBS = 0x02,
    VERIQC_SYS_GLN = 0x04,
    VERIQC_SYS_GAL = 0x08,
    VERIQC_SYS_QZS = 0x10,
    VERIQC_SYS_BDS = 0x20,
    VERIQC_SYS_LEO = 0x40,
    VERIQC_SYS_ALL = 0xFF
};

typedef enum _VERIQC_SYSID_E
{
    VERIQC_GNSS_SYSID_GPS = 1, //GPS
    VERIQC_GNSS_SYSID_SBAS,    //SBAS
    VERIQC_GNSS_SYSID_GLN,     //GLONASS
    VERIQC_GNSS_SYSID_GAL,     //Galileo
    VERIQC_GNSS_SYSID_QZS,     //qzss
    VERIQC_GNSS_SYSID_BDS      //BDS
}VERIQC_SYSID_E;

//where obs from
typedef enum _VERIQC_ANT_ID_E
{
    VERIQC_ANT_ID_ROVER = 0,                  /* rover */
    VERIQC_ANT_ID_BASE,                       /* base */
    VERIQC_ANT_ID_VICE1,                      /* aux antenna1 */
    VERIQC_ANT_ID_VICE2                       /* aux antenna2 */
}VERIQC_ANT_ID_E;

/**********top config initparam************/


/**********VERIQC initparam***************/

/**************************************chclib algorithm interface******************************/

/*********************************************************************************************/
#define VERIQC_EXTERN 

typedef size_t(*VERIQC_GET_USEDMEM_FUNC)(void);
typedef size_t(*VERIQC_GET_FREEMEM_FUNC)(void);
typedef uint32_t(*VERIQC_TICKGET_FUNC)(void);
typedef uint32_t(*VERIQC_TASKGET_FUNC)(void);

typedef void(*VERIQC_IO_SEND_BY_PORT_FUNC)(int32_t Port, int32_t Index, const uint8_t* pBuffer, 
    int32_t Length, uint8_t NeedNotify);

/********************GNSS/INS TC INTERFACE******************************/

/* configuration enum, corresponding with VERIQC_SPPTC_opt_t */
typedef enum _VERIQC_SPPTC_OPT_CTRL_E
{
    VERIQC_SPPTC_OPT_CTRL_ALL = 0,      /**< all config */
    VERIQC_SPPTC_OPT_CTRL_SYS,          /**< sys */
    VERIQC_SPPTC_OPT_CTRL_POSREF,       /**< posref */
    VERIQC_SPPTC_OPT_CTRL_SIGSYS,       /**< psig_sys */
    VERIQC_SPPTC_OPT_CTRL_POSMOD,       /**< posmode */
    VERIQC_SPPTC_OPT_CTRL_ELMIN,        /**< elmin */
    VERIQC_SPPTC_OPT_CTRL_SOLTYPE,      /**< soltype */
    VERIQC_SPPTC_OPT_CTRL_NF,           /**< nf */
    VERIQC_SPPTC_OPT_CTRL_SATEPH,       /**< sateph */
    VERIQC_SPPTC_OPT_CTRL_IONOOPT,      /**< ionoopt */
    VERIQC_SPPTC_OPT_CTRL_TROPOPT,      /**< tropopt */
    VERIQC_SPPTC_OPT_CTRL_DYNAMICS,     /**< dynamic */
    VERIQC_SPPTC_OPT_CTRL_ERATIO,       /**< eratio */
    VERIQC_SPPTC_OPT_CTRL_ERR,          /**< err */
    VERIQC_SPPTC_OPT_CTRL_STD,          /**< std */
    VERIQC_SPPTC_OPT_CTRL_PRN,          /**< prn */
    VERIQC_SPPTC_OPT_CTRL_EXCLGPS,      /**< exsats_gps */
    VERIQC_SPPTC_OPT_CTRL_EXCLGLO,      /**< exsats_glo */
    VERIQC_SPPTC_OPT_CTRL_EXCLGAL,      /**< exsats_gal */
    VERIQC_SPPTC_OPT_CTRL_EXCLQZS,      /**< exsats_qzs */
    VERIQC_SPPTC_OPT_CTRL_EXCLBDS,      /**< exsats_bds */
    VERIQC_SPPTC_OPT_CTRL_SATNUMLMT,    /**< satnumlmt */
    VERIQC_SPPTC_OPT_CTRL_REALTIME,     /**< realtime */
    VERIQC_SPPTC_OPT_CTRL_MAXCPUSATPARA,/**< maxcputimems_satpara */
    VERIQC_SPPTC_OPT_CTRL_EPHINTP,      /**< ephinterpolation */
    VERIQC_SPPTC_OPT_CTRL_SPPGAP,       /**< sppgap */
    VERIQC_SPPTC_OPT_CTRL_ENABLE_FASTER,/**< enable faster */
    VERIQC_SPPTC_OPT_CTRL_RESETTODEFAULT,/**< reset to default config */
    VERIQC_SPPTC_OPT_CTRL_REFID,        /**< refid */
    VERIQC_SPPTC_OPT_CTRL_RTMODE,       /**< Realtime mode */
}VERIQC_SPPTC_OPT_CTRL_E;


/*********************internal struct type define*****************************/

/* NMEA gpgga protocol states */
typedef enum
{
    VERIQC_POSTYPE_NONE = 0,            /**< invalid navigation solution */
    VERIQC_POSTYPE_GNSS_SINGLE = 1,     /**< SPP solution */
    VERIQC_POSTYPE_GNSS_DGNSS = 2,      /**< DGNSS solution */
    VERIQC_POSTYPE_GNSS_FIX = 4,        /**< fix solution */
    VERIQC_POSTYPE_GNSS_FLOAT = 5,      /**< float solution */
    VERIQC_POSTYPE_PPP = 6,             /**< PPP solution */
}VERIQC_POSTYPE_E;

typedef union _VERIQC_TRACK_STATUS_UNION
{
    struct
    {
        uint32_t TrackState : 5;     //track status 0: IDLE, 1: FACQ, 2: ACQ, 3: REACQ, 4: OPENLOOP, 5: PULL, 6: TRK, 7: LOCK
        uint32_t ChannelNum : 5;     //channel number, start from zero
        uint32_t PhaseLock : 1;      //Phase lock flag 0: Not locked , 1: Locked
        uint32_t ParityKnown : 1;    //Parity known flag 0: Not known, 1: Known
        uint32_t CodeLock : 1;       //Code locked flag 0: Not locked, 1: Locked
        uint32_t CorrlatorType : 3;  //Correlator type 0: N/A, 1: std correlator: spacing= 1 chip, 2: narrow correlator: spacing < 1 chip, 3: reserved, 4: pulse aperture correlator (PAC)
        uint32_t PseudorangeValid : 1;//Pseudorange phase valid. 0: invalid. 1: valid
        uint32_t CarrierPhaseValid : 1;//Carrier phase valid. 0: invalid. 1: valid
        uint32_t DopplerValid : 1;   //Doppler valid. 0: invalid. 1: valid
        uint32_t FrequencyLock : 1;  //0:not locked,1:locked
        uint32_t Grouping : 1;       //Grouping 0: Not grouped, 1: Grouped
        uint32_t FrameSyncStatus : 2;//Frame sync status. 0: none. 1: bit sync. 2: subframe. 3: exact framesync
        uint32_t Reserved1 : 3;      //signal type. GPS: L1(1) L2P(2) L2C(3) L1C(4) L5(5), BDS: B1I(1) B2I(2) B3I(3) B1C(4) 
                                     // B2a(5) B2b(6) B3Q(7) BDS_B2Q\J code(8) BDS_B1Q\J code(9) GLN: L1(1) L2(2) L3(3) L1P(4), 
                                     // GAL: E1(1) E5a(2) E5b(3) E5(5) E6(6), SBAS: L1(1) L5(5), QZSS: L1CA(1) L1SAIF(2) L2C(3) L5(5)
        uint32_t Fec : 1;            //forward error correction
        uint32_t PrimaryL1Channel : 1;//Primary L1 channel, 0: Not primary, 1: Primary
        uint32_t HalfCycleFlag : 1;  //carrier phase measurement, 0: half cycle not added, 1: half cycle added
        uint32_t Reserved2 : 1;
        uint32_t PRNLockOut : 1;     //flag of whether prn lock, 0: not locked out, 1: locked out
        uint32_t ChannelAssignment : 1;//channel assignment, 0: automatic, 1: forced
    }BITS_FIELD;
    uint32_t All;
} VERIQC_TRACK_STATUS_UNION;

typedef struct _VERIQC_GNSS_OBS_CELL_T    
{
    uint8_t AntID;            //antenna ID, 0: rover, 1: base 2: aux antenna1, 3: aux antenna2
    uint8_t CN0;              //carrier noise ratio[0.25db-Hz]
    uint8_t SigID;            //signal id
    uint8_t LLI;              //loss of lock indicator for phase
    uint32_t LockTime;        //no slip Tick[10ms] count
    double P;                 //P[m]
    double L;                 //L[cycle]
    double D;                 //D[Hz]
    uint32_t Reserved;        //reserved
    VERIQC_TRACK_STATUS_UNION TrackStatus; //track status
}VERIQC_GNSS_OBS_CELL_T;

typedef struct _VERIQC_GNSS_OBS_PER_SAT_T
{
    uint8_t SysID;                   //system id
    uint8_t SatID;                   //sv id
    uint8_t GLOFreq;                 //Glonass frequency number.geph->glnfreq+8[1-14]
    uint8_t SigalMaxNum;             //max signal
    uint16_t SigalValidMask;         //signal valid mask
    uint16_t Reserved;
    VERIQC_GNSS_OBS_CELL_T* pObsCell;//pobs_cell[index]
} VERIQC_GNSS_OBS_PER_SAT_T;

/*GNSS one epoch obs*/
typedef struct _VERIQC_GNSS_OBS_T
{
    QC_TIME_T Time;                  //UNIX time, start from UTC[1970/1/1]
    uint16_t LeapSec;                
    uint8_t TimeStatus;              //time status. 0:uninitialized, 1:real-time clock, 2:message initialize time, 3:unlock, 4:position initialize
    uint8_t Reserved1;               
    uint16_t SatMaxNum;              //max sat num

    VERIQC_GNSS_OBS_PER_SAT_T* pObs; //pobs[index]
} VERIQC_gnss_obs_t;

/***********sat in view struct****************/
typedef struct _VERIQC_SATINFO_PERSAT_T
{
    uint8_t SysID;      //system id
    uint8_t SatID;      //sv id(PRN/Glonass slot) GPS:1-32, GLONASS:1-28, BDS:1-64, GAL:1-36, SBAS: 120-138, QZSS: 193-199
    uint8_t GLOFreq;    //glonass frequency number
    uint8_t SigID;      //signal id GPS: L1(1) L2P(2) L2C(3) L1C(4) L5(5), BDS: B1I(1) B2I(2) B3I(3) B1C(4) B2a(5) B2b(6) B3Q(7) BDS_B2Q\J(8) BDS_B1Q\J(9) GLN: L1(1) L2(2) L3(3) L1P(4),  GAL: E1(1) E5a(2) E5b(3) E5(5) E6(6), SBAS: L1(1) L5(5), QZSS: L1CA(1) L1SAIF(2) L2C(3) L5(5),
    uint8_t SatStatus;  ///* sat status, 0:good, 1:badeph, 2:oldeph, 3:lowele, 4:misclosure, 5:nodiffcorr, 6:noeph, 7:invalid iode, 8:lockedout, 9:lowpower*/
    uint8_t Visibility; //visibility: 0:invisible, 1:visible, 2:tracking, 3:solving
    uint8_t CN0;        //cn0[0.25 dB-Hz]
    uint8_t PSRResLevel;/**< PRS residual level: 1 level == 0.1m, Max Level: 250; Level == 251-254: reserved; Level == 255: SBAS error flag */
    int16_t AzEl[2];    //{azimuth, elevation},(deg)
    float PSR;          //P m
    float PSRRate0;     //P rate
    float PSRAcc;       //P rate rate
    float PSRRes;       //P residual(m)
    float PSRRate0Res;  //P rate residual(m/s)
    double PosVel[6];   /* rove satellite position and velocity in ecef(m, m/s) */
    double SatClockError;/* rove satellite clock error(s) */
    uint8_t UsedInSolve; /* 0 == not used in solving, 1 == used in solving */
    uint8_t UsedFrequencyMark;/* bit0/1/2/3/4; GPS:L1/L2/L5; GLN:G1/G2/G3; GAL:E1/E5b/E5a/E5ab/E6; CMP:B1I/B2I/B3I/B1C/B2a/B2 */
    uint8_t PSRFreqIndex; /**< 0,1,2,3,4,5: GPS:L1/L2/L5; GLN:G1/G2/G3; GAL:E1/E5b/E5a/E5ab/E6; CMP:B1I/B2I/B3I/B1C/B2a/B2 */
    uint8_t Reserved;
} VERIQC_SATINFO_PERSAT_T;

typedef struct _VERIQC_SATINFO_T
{
    QC_TIME_T Time;
    int8_t TimeStatus;
    uint16_t SatNum;
    VERIQC_SATINFO_PERSAT_T* pSatInfo;
} VERIQC_SATINFO_T;

/***********GNSS solve structure****************/
typedef struct _VERIQC_SOLTIME_T
{
    QC_TIME_T Time;             /* solution time(GPST) */
    uint32_t TimeStatus;        /* rove range time status */
    int32_t LeapSec;            /* leap second(s) */
    double ClkBias[6];          /* clock bias(GRCESJ, second) */
    double ClkDrift;            /* clock drift(s/s) */
    QC_TIME_T BaseTime;         /* base obs time(GPST) */
}VERIQC_SOLTIME_T;

typedef struct _VERIQC_SOLSTATE_T
{
    uint8_t  SolStat;           /**< sol state, 0: COMPUTED, 1:INSUFFICIENT_OBS, 2:NO_CONVERGENCE, 24:REPEATED_OBS */
    uint8_t  PosType;           /**< GGA pos type, 0:invalid 1:SPP 2:RTD 4:RTK fix 5:RTK float 6.PPP */
    uint8_t  SolSatNum;         /**< sat number involved in position */
    uint16_t TrackSatNum;       /**< sat number of tracking */
    uint16_t SolSysMask;        /**< position system bit1 : GPS, bit2 : GLN, bit3 : BDS, bit4 : GAL, bit5 : SBAS, bit6 : QZSS */
    uint16_t RefID;             /**< ref ID */
    uint32_t SolCount;          /**< continue sol valid count */
    VERIQC_SATINFO_T* pSatInfo; /**< tracking satellite information lists */
    float Cutoff;               /**< reserved(rad) */
    float DiffAge;              /**< diff age(s) */
    int32_t AgeOk;              /**< ageok */
    double RefPosLLH[3];        /**< base location, lon[rad], lat[rad], height[m] WGS - 84 */
    uint16_t IsStop;            /**< is stopped for a while? */
    uint16_t DynamicMode;       /**< 0:unknown, 1:static, 3:pedestrian, 4:vehicle, 5:high motion, 6: very high motion */
}VERIQC_SOLSTATE_T;

typedef struct _VERIQC_POS_T
{
    double PosLLH[3];          /**< location({lat,lon,alt|[rad, rad, m]) */
    double PosXYZ[3];          /**< position(ecef, {x,y,z}[m]) */
    float  PosLLHStd[3];       /**< location std, {N,E,U}[m] */
    float  PosXYZStd[3];       /**< position std, [m] */
    float  PosRMS;             /**< pos rms[m] */
    float  Dop[5];             /**< dop[gdop, pdop, hdop, vdop, tdop] */
    float  SemiStd[3];         /**< error eclipse{eE,eF,eA}[m,m,rad] */
    float  Undulation;         /**< undulation[m] */
    uint32_t StationID;        /**< station ID */
    int32_t CoordType;         /**< coordinate system, 0: WGS - 84, 1 : PZ90, 2 : CGCS2000 */
    int32_t ProcessObsNum;     /**< number of processed obs */
    int32_t Iter;              /**< number of iteration */
}VERIQC_POS_T;

typedef struct _VERIQC_VEL_T
{
    double VelENU[3];          /*< velocity {N,E,U}[m/s] */
    double VelXYZ[3];          /*< velocity {X,Y,Z}[m/s] */
    float  VelENUStd[3];       /*< velocity std {N,E,U}[m/s] */
    float  VelXYZStd[3];       /*< velocity std {X,Y,Z}[m/s] */
    float  VelRMS;             /*< velocity rms[m/s] */
    double HeadingVel;         /*< velocity heading{0~360},[deg] */
    double HeadingVelStd;      /*< velocity heading std, [deg] */
    uint32_t Reserved;
}VERIQC_VEL_T;

typedef struct _VERIQC_SOLINFO_T
{
    VERIQC_SOLTIME_T* pTime;
    VERIQC_SOLSTATE_T* pSolState;
    VERIQC_POS_T* pPos;
    VERIQC_VEL_T* pVel;
    void* pUserData;
}VERIQC_SOLINFO_T;


/*********************************INTERFACE****************************************/

//VERIQC SPP option only for creation
typedef struct _VERIQC_SPPTC_CREATE_OPT_T
{
    int16_t NumSPP;         //spp processor number[0,1]
    int16_t NumSys;         //number of system
    int16_t NumFreq;        //number of frequency
    int16_t NumSPPSol;      //output spp sol
    int16_t PosMode;      /* 2:RTK, 5:PPP, 6:PPPRTK */
}VERIQC_SPPTC_CREATE_OPT_T;

//VERIQC SPP option
typedef struct _VERIQC_SPPTC_OPT_T
{
    int Sys;                           //VERIQC_SYS_GPS....
    double BasePos[3];                 //base position
    int16_t SysSignal[VERIQC_SYS_NUM]; //signal type used in each system.[0]GPS,[1]:BDS,[2]:GLO,[3]:GAL
    uint8_t PosMode;                   //0:spp,1:rtd,2:rtk,5:ppp,6:ppprtk
    double EleMin;                     //ele cutoff[deg]
    int SolType;                       /**< solution type (0:forward,1:backward) */
    int FreqNum;                       /**< number of frequencies (1:L1,2:L1+L2,3:L1+L2+L5) */
    int SatEph;                        /**< satellite ephemeris/clock 0:brdc, 1:precise, 2:sbas, 3:ssrapc, 4:ssrcom, 5:qzss lex */
    int IonoOpt;                       /**< ionosphere option (IONOOPT_???) */
    int TropOpt;                       /**< troposphere option (TROPOPT_???) */
    int Dynamics;                      /**< dynamics model (0:none,1:velociy,2:static,3:fixed) */
    double ErrorRatio[VERIQC_NUM_FREQ];/**< code/phase error ratio */
    double ErrorFactor[5];             /**< measurement error factor */
    double InitStateSTD[3];            /**< initial-state std [0]bias,[1]iono [2]trop */
    double ProcessNoiseSTD[5];          /**< process-noise std [0]bias,[1]iono [2]trop [3]acch [4]accv */
    uint8_t ExSatsGPS[VERIQC_NSATGPS]; /**< excluded satellites (1:excluded,2:included) */
    uint8_t ExSatsGLO[VERIQC_NSATGLO]; /**< excluded satellites (1:excluded,2:included) */
    uint8_t ExSatsGAL[VERIQC_NSATGAL]; /**< excluded satellites (1:excluded,2:included) */
    uint8_t ExSatsQZS[VERIQC_NSATQZS]; /**< excluded satellites (1:excluded,2:included) */
    uint8_t ExSatsBDS[VERIQC_NSATBDS]; /**< excluded satellites (1:excluded,2:included) */
    int SatNumLimit;                   /**< select satellite num:0,no;>0,yes*/
    int MaxCPUTimeMsSatPara;           //for satpara(ms)
    uint8_t EphInterpolation;          //0-no;1-yes
    float SPPGap;                      //default:1.0
    uint8_t EnableFaster;
    int RefID;
    int RealTimeMode;                  /**< 1=realtime mode, 0=post mode, need all ephs*/
    int8_t LogLevel[2];                /**< gnss log level */
    double SimulateMaxAge;             /**< simulated max differential age by user setting */
}VERIQC_SPPTC_OPT_T;


//orbit
typedef struct _VERIQC_ORBIT_SOL_T
{
    QC_TIME_T UpdateTime;
    int32_t ObsNum;                      //number of obs
    int16_t* pSatNo;                     //[ns]prn
    uint8_t* pSVH;                       //[ns]svh,0:OK
    uint8_t* pType;                      //[ns]sat type,BDS:0-BDS2 1-BDS3 GAL:0-INAV/FNAV 1-CED
    uint8_t* pExcludeSats;               //[ns]exclude sats
    double* pRs;                         //[ns*6]sat position/velocity
    double* pDts;                        //[ns*2]sat clock/drift
    double* pVar;                        //[ns]sat position/clock variance
    int32_t* pIODE;                      //[ns]iode
    int32_t* pToe;                       //toe(sow)
}VERIQC_ORBIT_SOL_T;

//-------------------------------------------------------------------------------------------------------//
//-------------------------------------------VERIQC interfaces----------------------------------------//
//-------------------------------------------------------------------------------------------------------//
struct _VERIQC_ORBIT_PROCESSOR_T;
struct _QC_TIME_T;
struct _CHC_PRCOPT_T;
struct _UNI_EPHEMERIS_T;
struct _GLO_EPHEMERIS_T;
struct _RM_RANGE_LOG_T;

typedef struct _VERIQC_SPPTC_PROCESSOR_T*(*VERIQC_SPPTC_CREATE_FUNC)(VERIQC_SPPTC_CREATE_OPT_T* pOpt, 
    struct _VERIQC_ORBIT_PROCESSOR_T* pOrbit);
typedef int32_t(*VERIQC_SPPTC_RELEASE_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj);
typedef void(*VERIQC_SPPTC_SETOPT_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt,
     VERIQC_SPPTC_OPT_CTRL_E OptCtrl);
typedef int32_t(*VERIQC_SPPTC_GETOPT_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt);
typedef int32_t(*VERIQC_SPPTC_ADDRANGE_ROV_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj, const struct _RM_RANGE_LOG_T* pObs);
typedef int32_t(*VERIQC_SPPTC_START_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj);
typedef int32_t(*VERIQC_SPPTC_SPP_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj);
typedef VERIQC_SOLINFO_T*(*VERIQC_SPPTC_GETSOL_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj);
typedef VERIQC_ORBIT_SOL_T* (*VERIQC_SPPTC_GETSATPOS_FUNC)(struct _VERIQC_SPPTC_PROCESSOR_T* pObj, VERIQC_ANT_ID_E ID);

//spptc external interface
typedef struct _VERIQC_SPPTC_IFX_T
{
    VERIQC_SPPTC_CREATE_FUNC CreateFunc;               //create
    VERIQC_SPPTC_RELEASE_FUNC ReleaseFunc;             //release
    VERIQC_SPPTC_SETOPT_FUNC SetOptFunc;               //setopt
    VERIQC_SPPTC_GETOPT_FUNC GetOptFunc;               //getopt
    VERIQC_SPPTC_ADDRANGE_ROV_FUNC AddRangeRovFunc;    //add obs rover
    VERIQC_SPPTC_START_FUNC StartFunc;                 //sat position, sort obs
    VERIQC_SPPTC_SPP_FUNC SPPFunc;                     //spp
    VERIQC_SPPTC_GETSOL_FUNC GetSolFunc;               //get sol
    VERIQC_SPPTC_GETSATPOS_FUNC GetSatPosFunc;         /**< get sat pos */
}VERIQC_SPPTC_IFX_T;

extern VERIQC_SPPTC_IFX_T* VeriQCGetSPPTcIfx();

struct _CHC_PRCOPT_T;
typedef struct _VERIQC_ORBIT_PROCESSOR_T*(*VERIQC_ORBIT_CREATE_FUNC)();
typedef int32_t(*VERIQC_ORBIT_RELEASE_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj);
typedef void(*VERIQC_ORBIT_SETOPT_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const VERIQC_SPPTC_OPT_T* pOpt, 
    VERIQC_SPPTC_OPT_CTRL_E OptCtrl);
typedef int32_t(*VERIQC_ORBIT_GETOPT_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, VERIQC_SPPTC_OPT_T* pOpt);
typedef struct _CHC_NAV_T*(*VERIQC_GET_NAV_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj);
typedef int32_t(*VERIQC_ORBIT_ADD_UNI_EPHEMERIS_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const struct _UNI_EPHEMERIS_T* pEph);
typedef int32_t(*VERIQC_ORBIT_ADD_GLO_EPHEMERIS_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const struct _GLO_EPHEMERIS_T* pGEph);
typedef int32_t(*VERIQC_ORBIT_FIT_RANGE_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const struct _RM_RANGE_LOG_T* pObs);
typedef int32_t(*VERIQC_ORBIT_SATPOS_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const struct _CHC_OBS_T* pObs, 
    VERIQC_ORBIT_SOL_T* pSol, uint8_t IncrementUpdateMark);
typedef int32_t(*VERIQC_ORBIT_SET_SELEPHTIME_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const struct _QC_TIME_T* pEphTime);
typedef int32_t(*VERIQC_ORBIT_ONE_SAT_CALC_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T Time, VERIQC_SYSID_E SysId, unsigned int SatPrn,
    double* pSatPosVel, double* pSatClock);
typedef int32_t(*VERIQC_ORBIT_SAT_CALC_BY_TOE_FUNC)(struct _VERIQC_ORBIT_PROCESSOR_T* pObj, const QC_TIME_T TransTime, const QC_TIME_T Time, VERIQC_SYSID_E SysId, unsigned int SatPrn, int Toe,
    double* pSatPosVel, double* pSatClock);

//ORBIT processor interface
typedef struct _VERIQC_ORBIT_IFX_T
{
    VERIQC_ORBIT_CREATE_FUNC CreateFunc;               //create
    VERIQC_ORBIT_RELEASE_FUNC ReleaseFunc;             //release
    VERIQC_ORBIT_SETOPT_FUNC SetOptFunc;               //setopt
    VERIQC_ORBIT_GETOPT_FUNC GetOptFunc;               //getopt
    VERIQC_GET_NAV_FUNC GetNavFunc;                    //getnav
    VERIQC_ORBIT_ADD_UNI_EPHEMERIS_FUNC AddUniEphemerisFunc;
    VERIQC_ORBIT_ADD_GLO_EPHEMERIS_FUNC AddGLOEphemerisFunc;
    VERIQC_ORBIT_FIT_RANGE_FUNC OrbfitRangeFunc;
    VERIQC_ORBIT_SATPOS_FUNC SatPosFunc;               //cal sat position
    VERIQC_ORBIT_SET_SELEPHTIME_FUNC SetEphTimeFunc;      //set eph selection time
    VERIQC_ORBIT_ONE_SAT_CALC_FUNC OneSatCalcFunc;
    VERIQC_ORBIT_SAT_CALC_BY_TOE_FUNC SatCalcByToeFunc;
}VERIQC_ORBIT_IFX_T;

extern VERIQC_ORBIT_IFX_T* VeriQCGetOrbitIfx();

#ifdef __cplusplus
}
#endif
/********************* optional data for select **********************/
#endif
