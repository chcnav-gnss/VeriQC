/*************************************************************************//**
           VeriQC
         SPP Module
 *-
@file   GNSSBase.h
@author CHC
@date   5 September 2022
@brief  common GNSS positioning related structures and functions 
*****************************************************************************/

/*---------------------chc gnssbase structure-------------------------*/

#ifndef GNSSBASE_H__
#define GNSSBASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include <assert.h>

#ifdef WIN32        /**< Windows */
#include <WinSock2.h>
#include <windows.h>
#else               /**< linux */
#include <pthread.h>
#include <sys/types.h>
#include<fcntl.h>
#include<errno.h>
#endif

#include "VeriQCSPP.h"
#include "MemoryManager.h"
#include "MatrixFuncs.h"
#include "TimeDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

    /** Time Cost */

#define  TIMECOST_BEGIN()                               (void)0
#define  TIMECOST_END(CurSpendTime)                     (void)0
#define  TIMECOST_END_PRINT(ModuleName, Format,...)     (void)0
#define  TIMECOST_BEGIN_AS(BeginTick)                   (void)0
#define  TIMECOST_END_AS(BeginTick, CurSpendTime)       (void)0
#define  TIMECOST_END_AS_PRINT(BeginTick, CurSpendTime, ModuleName, Format,...)   (void)0

/** Memory */
    
#define VERIQC_GET_USEDMEM   VeriQCGetMemusedFunc()
#define VERIQC_GET_FREEMEM   VeriQCGetMemfreeFunc()
#define VERIQC_TICKGET	     VeriQCGetTickgetFunc()

/** Deubg Log */

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#define new  new(_CLIENT_BLOCK, __FILE__, __LINE__)  
#endif
#define OUTLOG(Msg,...) \
    do { \
        char LogBuff[1024]={0}; \
        snprintf(LogBuff,1024,Msg,__VA_ARGS__); \
        OutputDebugString(LogBuff); \
    } while(0);
#define LOGI(Msg,...)
#else
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#define OUTLOG(Msg,...)

#define LOGI(Msg,...)
#endif

#define VERIQC_INCREA_EPH_ALM_REALTIME  5   /**< uni/glo eph/alm increment count for real-process */
#define VERIQC_INCREA_EPH_ALM_POSTTIME  256 /**< uni/glo eph/alm increment count for post-process */
    
#define CHC_ZEROSVALUE        1E-12    /* zero value tolerance */

/* enable gnss system */
#define CHC_ENACMP
#define CHC_ENAGPS
#define CHC_ENAGLO
#define CHC_ENAGAL
#define CHC_ENAQZS

#ifdef BDS23
#define BD23 1
#else
#define BD23 0
#endif

#ifdef CHC_ENAQZS
#ifndef USING_QZSS_AS_GPS
#define USING_QZSS_AS_GPS
#endif
#define CHC_SYSNUM (4+BD23)  /*GPS/BDS/GLONASS/GAL*/
#else
#ifdef CHC_ENAGAL
#define CHC_SYSNUM (4+BD23)  /*GPS/BDS/GLONASS/GAL*/
#else
#define CHC_SYSNUM (3+BD23)  /*GPS/BDS/GLONASS*/
#endif
#endif

/** Fast datum when different source */

#define CHC_PMODE_SINGLE     0
#define CHC_PMODE_RTK        2   

#define CHC_PI              3.141592653589793238    /* pi */
#define CHC_D2R             (CHC_PI/180.0)          /* deg to rad */
#define CHC_R2D             (180.0/CHC_PI)          /* rad to deg */
#define CHC_OMGE            7.2921151467E-5         /* earth angular velocity (IS-GPS) (rad/s) */

/* constants -----------------------------------------------------------------*/
#define CHC_CLIGHT      299792458.0         /* speed of light (m/s) */
#define CHC_FREQ1       1.57542E9           /* L1/E1  frequency (Hz) */
#define CHC_FREQ2       1.22760E9           /* L2     frequency (Hz) */
#define CHC_FREQ5       1.17645E9           /* L5/E5a frequency (Hz) */
#define CHC_FREQ6       1.27875E9           /* E6/LEX frequency (Hz) */
#define CHC_FREQ7       1.20714E9           /* E5b    frequency (Hz) */
#define CHC_FREQ8       1.191795E9          /* E5a+b  frequency (Hz) */
#define CHC_FREQ1_GLO   1.60200E9           /* GLONASS G1 base frequency (Hz) */
#define CHC_DFRQ1_GLO   0.56250E6           /* GLONASS G1 bias frequency (Hz/n) */
#define CHC_FREQ2_GLO   1.24600E9           /* GLONASS G2 base frequency (Hz) */
#define CHC_DFRQ2_GLO   0.43750E6           /* GLONASS G2 bias frequency (Hz/n) */
#define CHC_FREQ3_GLO   1.202025E9          /* GLONASS G3 frequency (Hz) */
#define CHC_FREQ1_CMP   1.57542E9           /* BeiDou B1C frequency (Hz) */
#define CHC_FREQ2_CMP   1.561098E9          /* BeiDou B1 frequency (Hz) */
#define CHC_FREQ5_CMP   1.17645E9           /* BeiDou B2a frequency (Hz) */
#define CHC_FREQ7_CMP   1.20714E9           /* BeiDou B2 frequency (Hz) */
#define CHC_FREQ8_CMP   1.191795E9          /* BeiDou B2a+b frequency (Hz) */
#define CHC_FREQ6_CMP   1.26852E9           /* BeiDou B3 frequency (Hz) */

#define CHC_SYS_NONE    0x00                /* navigation system: none */
#define CHC_SYS_GPS     0x01                /* navigation system: GPS */
#define CHC_SYS_SBS     0x02                /* navigation system: SBAS */
#define CHC_SYS_GLO     0x04                /* navigation system: GLONASS */
#define CHC_SYS_GAL     0x08                /* navigation system: Galileo */
#define CHC_SYS_QZS     0x10                /* navigation system: QZSS */
#define CHC_SYS_CMP     0x20                /* navigation system: BeiDou */
#define CHC_SYS_BD3     0x40                /* navigation system: BeiDou 3 */
#define CHC_SYS_ALL     0xFF                /* navigation system: all */

#define CSPP_NFREQ       5
#define CSPP_NEXOBS      2

#ifndef CHC_NFREQ
#define CHC_NFREQ          CSPP_NFREQ           //(3+CHC_EXGAL)  /* number of carrier frequencies */
#endif
#define CHC_NFREQGLO      2
#define CHC_NFREQ_CM_GLO  1
#ifndef CHC_NFREQ_CM
#define CHC_NFREQ_CM      6          /* number of carrier frequencies */
#endif
#ifndef CHC_NFREQ_IF
#define CHC_NFREQ_IF      5          /* number of IF Combination */
#endif

#define CHC_NFREQ_SOLVE    4

#ifndef CHC_NEXOBS
#define CHC_NEXOBS         CSPP_NEXOBS      /* number of extended obs codes */
#endif

#define CHC_MAXPRNGPS_TRUE 32
#define CHC_MINPRNGPS      1                /* min satellite PRN number of GPS */
#ifdef CHC_ENAQZS
#ifdef USING_QZSS_AS_GPS
#define CHC_MAXPRNGPS      39               /* max satellite PRN number of GPS+QZSS */
#else
#define CHC_MAXPRNGPS      32               /* max satellite PRN number of GPS */
#endif
#else
#define CHC_MAXPRNGPS   32                  /* max satellite PRN number of GPS */
#endif
#define CHC_NSATGPS     (CHC_MAXPRNGPS-CHC_MINPRNGPS+1) /* number of GPS satellites */
#define CHC_NSYSGPS     1

#ifdef CHC_ENAGLO
#define CHC_MINPRNGLO   1                   /* min satellite slot number of GLONASS */
#define CHC_MAXPRNGLO   27                  /* max satellite slot number of GLONASS */
#define CHC_NSATGLO     (CHC_MAXPRNGLO-CHC_MINPRNGLO+1) /* number of GLONASS satellites */
#define CHC_NSYSGLO     1
#else
#define CHC_MINPRNGLO   0
#define CHC_MAXPRNGLO   0
#define CHC_NSATGLO     0
#define CHC_NSYSGLO     0
#endif
#ifdef CHC_ENAGAL
#define CHC_MINPRNGAL   1                   /* min satellite PRN number of Galileo */
#define CHC_MAXPRNGAL   36                  /* max satellite PRN number of Galileo */
#define CHC_NSATGAL    (CHC_MAXPRNGAL-CHC_MINPRNGAL+1) /* number of Galileo satellites */
#define CHC_NSYSGAL     1
#else
#define CHC_MINPRNGAL   0
#define CHC_MAXPRNGAL   0
#define CHC_NSATGAL     0
#define CHC_NSYSGAL     0
#endif
#ifdef CHC_ENAQZS
#ifndef USING_QZSS_AS_GPS
#define CHC_MINPRNQZS   193                 /* min satellite PRN number of QZSS */
#define CHC_MAXPRNQZS   200                 /* max satellite PRN number of QZSS */
#define CHC_MINPRNQZS_S 183                 /* min satellite PRN number of QZSS SAIF */
#define CHC_MAXPRNQZS_S 185                 /* max satellite PRN number of QZSS SAIF */
#define CHC_NSATQZS     (CHC_MAXPRNQZS-CHC_MINPRNQZS+1) /* number of QZSS satellites */
#define CHC_NSYSQZS     1
#else
#define CHC_MINPRNQZS   193
#define CHC_MAXPRNQZS   200
#define CHC_NSATQZS     0
#define CHC_NSYSQZS     0
#endif
#else
#define CHC_MINPRNQZS   0
#define CHC_MAXPRNQZS   0
#define CHC_NSATQZS     0
#define CHC_NSYSQZS     0
#endif
#ifdef CHC_ENACMP
#define CHC_MINPRNCMP   1                   /* min satellite sat number of BeiDou */
#define CHC_MAXPRNCMP   64                  /* max satellite sat number of BeiDou */
#define CHC_NSATCMP     (CHC_MAXPRNCMP-CHC_MINPRNCMP+1) /* number of BeiDou satellites */
#define CHC_NSYSCMP     1
#else
#define CHC_MINPRNCMP   0
#define CHC_MAXPRNCMP   0
#define CHC_NSATCMP     0
#define CHC_NSYSCMP     0
#endif
#define CHC_NSYS        (CHC_NSYSGPS+CHC_NSYSGLO+CHC_NSYSGAL+CHC_NSYSQZS+CHC_NSYSCMP+BD23) /* number of systems */
#ifdef CHC_ENASBS
#define CHC_MINPRNSBS   120                 /* min satellite PRN number of SBAS */
#define CHC_MAXPRNSBS   142                 /* max satellite PRN number of SBAS */
#define CHC_NSATSBS     (CHC_MAXPRNSBS-CHC_MINPRNSBS+1) /* number of SBAS satellites */
#else
#define CHC_MINPRNSBS   0
#define CHC_MAXPRNSBS   0
#define CHC_NSATSBS     0
#endif

#define CHC_MAXSAT      (CHC_NSATGPS+CHC_NSATGLO+CHC_NSATGAL+CHC_NSATQZS+CHC_NSATCMP+CHC_NSATSBS)

#ifdef CHC_ENAGPS
#define CHC_GPSSDOBS    16
#else
#define CHC_GPSSDOBS    0
#endif
#ifdef CHC_ENACMP
#define CHC_BDSSDOBS    28
#else
#define CHC_BDSSDOBS    0
#endif
#ifdef CHC_ENAGLO
#define CHC_GLOSDOBS    12
#else
#define CHC_GLOSDOBS    0
#endif
#ifdef CHC_ENAGAL
#define CHC_GALSDOBS    15
#else
#define CHC_GALSDOBS    0
#endif
#ifdef CHC_ENAQZS
#define CHC_QZSSDOBS    7
#else
#define CHC_QZSSDOBS    0
#endif

#define CHC_MAXOBS      (CHC_GPSSDOBS+CHC_BDSSDOBS+CHC_GALSDOBS+(VERIQC_FLAG==2?CHC_QZSSDOBS:CHC_GLOSDOBS))  /* max number of obs in an epoch */               

#define CHC_MAXSDOBS     (VERIQC_FLAG==2&&!RESOURCE_FLAG?28:40)//(CHC_GPSSDOBS+CHC_BDSSDOBS+CHC_GLOSDOBS+CHC_GALSDOBS)
#define CHC_MAXDDOBS     CHC_MAXSDOBS                  /*max number of double difference obs in an epoch*/

#define CHC_SOLQ_NONE   0                   /* solution status: no solution */
#define CHC_SOLQ_FIX    1                   /* solution status: fix */
#define CHC_SOLQ_FLOAT  2                   /* solution status: float */
#define CHC_SOLQ_SBAS   3                   /* solution status: SBAS */
#define CHC_SOLQ_DGPS   4                   /* solution status: DGPS/DGNSS */
#define CHC_SOLQ_SINGLE 5                   /* solution status: single */
#define CHC_SOLQ_PPP    6                   /* solution status: PPP */
#define CHC_SOLQ_DR     7                   /* solution status: dead reconing */
#define CHC_MAXSOLQ     7                   /* max number of solution status */

#define CHC_IONOOPT_OFF  0                  /* ionosphere option: correction off */
#define CHC_IONOOPT_BRDC 1                  /* ionosphere option: broadcast model */
#define CHC_IONOOPT_SBAS 2                  /* ionosphere option: SBAS model */
#define CHC_IONOOPT_IFLC 3                  /* ionosphere option: L1/L2 or L1/L5 iono-free LC */
#define CHC_IONOOPT_EST  4                  /* ionosphere option: estimation */
#define CHC_IONOOPT_TEC  5                  /* ionosphere option: IONEX TEC model */
#define CHC_IONOOPT_QZS  6                  /* ionosphere option: QZSS broadcast model */
#define CHC_IONOOPT_LEX  7                  /* ionosphere option: QZSS LEX ionospehre */
#define CHC_IONOOPT_STEC 8                  /* ionosphere option: SLANT TEC model */
#define CHC_IONOOPT_AUTO 9                  /* ionosphere option: AUTO*/

#define CHC_TROPOPT_OFF  0                  /* troposphere option: correction off */
#define CHC_TROPOPT_SAAS 1                  /* troposphere option: Saastamoinen model */
#define CHC_TROPOPT_SBAS 2                  /* troposphere option: SBAS model */
#define CHC_TROPOPT_EST  3                  /* troposphere option: ZTD estimation */
#define CHC_TROPOPT_ESTG 4                  /* troposphere option: ZTD+grad estimation */
#define CHC_TROPOPT_COR  5                  /* troposphere option: ZTD correction */
#define CHC_TROPOPT_CORG 6                  /* troposphere option: ZTD+grad correction */

#define CHC_EPHOPT_BRDC   0                 /* ephemeris option: broadcast ephemeris */
#define CHC_EPHOPT_PREC   1                 /* ephemeris option: precise ephemeris */
#define CHC_EPHOPT_SBAS   2                 /* ephemeris option: broadcast + SBAS */
#define CHC_EPHOPT_SSRAPC 3                 /* ephemeris option: broadcast + SSR_APC */
#define CHC_EPHOPT_SSRCOM 4                 /* ephemeris option: broadcast + SSR_COM */
#define CHC_EPHOPT_LEX    5                 /* ephemeris option: QZSS LEX ephemeris */
#define CHC_EPHOPT_SSRAPC_L1 6              /* ephemeris option: broadcast + SSR_APC IGS(L1)*/

/** max number of inner dsol */

/* ssat_t::vs */
#define SATVIS_INVISIBLE           0    /* invisible */
#define SATVIS_VISIBLE             1    /* visible */
#define SATVIS_TRACKING            2    /* tracking */
#define SATVIS_SOLVING             3    /* in solve */

/* ssat_t::sat_status */
#define SAT_STATUS_GOOD            0    /* Observation is good */
#define SAT_STATUS_BADHEALTH       1    /* Satellite is flagged as bad health in ephemeris or almanac */
#define SAT_STATUS_OLDEPH          2    /* Ephemeris >3 hours old */
#define SAT_STATUS_LOWELEV         6    /* Satellite was below the elevation cutoff */
#define SAT_STATUS_MISCLOSURE      7    /* Observation was too far from predicted value */
#define SAT_STATUS_NODIFFCORR      8    /* No differential correction available */
#define SAT_STATUS_NOEPH           9    /* No ephemeris available */
#define SAT_STATUS_INVALIDIODE     10   /* IODE used is invalid */
#define SAT_STATUS_LOCKEDOUT       11   /* Satellite has been locked out */
#define SAT_STATUS_LOWPOWER        12   /* Satellite has low signal power */
#define SAT_STATUS_OBSL2           13   /* An L2 observation not directly used in the solution */
#define SAT_STATUS_UNKNOWN         15   /* Observation was not used because it was of an unknown type */
#define SAT_STATUS_NOIONOCORR      16   /* No ionosphere delay correction was available */
#define SAT_STATUS_NOTUSED         17   /* Observation was not used in the solution */
#define SAT_STATUS_OBSL1           18   /* An L1 observation not directly used in the solution */
#define SAT_STATUS_OBSE1           19   /* An E1 observation not directly used in the solution */
#define SAT_STATUS_OBSL5           20   /* An L5 observation not directly used in the solution */
#define SAT_STATUS_OBSE5           21   /* An E5 observation not directly used in the solution */
#define SAT_STATUS_OBSB2           22   /* An B2 observation not directly used in the solution */
#define SAT_STATUS_OBSB1           23   /* An B1 observation not directly used in the solution */
#define SAT_STATUS_OBSB3           24   /* An B3 observation not directly used in the solution */
#define SAT_STATUS_NOSIGNALMATCH   25   /* Signal type does not match */
#define SAT_STATUS_SUPPLEMENTARY   26   /* Observation contributes supplemental information to the solution */
#define SAT_STATUS_NA              99   /* No observation available */
#define SAT_STATUS_BADINTEGRITY    100  /* Observation was an outlier and was eliminated from the solution */
#define SAT_STATUS_LOSSOFLOCK      101  /* Lock was broken on this signal */

/* sol_t::soL_stat */
#define SOL_STATUS_COMPUTED         0   /* solution computed */
#define SOL_STATUS_INSUFFICIENT_OBS 1   /* insufficient obs */
#define SOL_STATUS_NO_CONVERGENCE   2   /* no convergence */
#define SOL_STATUS_SINGULARITY      3
#define SOL_STATUS_COV_TRACE        4
#define SOL_STATUS_REPEATED_OBS     23  /* repeated obs with last epoch */

#define SYSNUM          CHC_SYSNUM   /*GPS/BDS/GLONASS*/
#define ZEROSVALUE      CHC_ZEROSVALUE

#ifndef MIN
#define MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) ((_x) < (_y) ? (_y) : (_x))
#endif
	
	/* constants -----------------------------------------------------------------*/

#ifdef CHC_ENAGAL
#define ENAGAL
#endif
	
#define PI          CHC_PI                          /* pi */
#define HALFPI      (PI*0.5)
#define D2R         CHC_D2R                         /* deg to rad */
#define R2D         CHC_R2D                         /* rad to deg */

#define CLIGHT      CHC_CLIGHT                      /* speed of light: 299792458.0(m/s) */
#define FREQ1       CHC_FREQ1                       /* L1/E1  frequency: 1.57542E9 (Hz) */
#define FREQ2       CHC_FREQ2                       /* L2     frequency: 1.22760E9 (Hz) */
#define FREQ5       CHC_FREQ5                       /* L5/E5a frequency: 1.17645E9 (Hz) */
#define FREQ6       CHC_FREQ6                       /* E6/LEX frequency: 1.27875E9 (Hz) */
#define FREQ7       CHC_FREQ7                       /* E5b    frequency: 1.20714E9 (Hz) */
#define FREQ8       CHC_FREQ8                       /* E5a+b  frequency: 1.191795E9 (Hz) */
#define FREQ1_GLO   CHC_FREQ1_GLO                   /* GLONASS G1 base frequency: 1.60200E9 (Hz) */
#define DFRQ1_GLO   CHC_DFRQ1_GLO                   /* GLONASS G1 bias frequency: 0.56250E6 (Hz/n) */
#define FREQ2_GLO   CHC_FREQ2_GLO                   /* GLONASS G2 base frequency: 1.24600E9 (Hz) */
#define DFRQ2_GLO   CHC_DFRQ2_GLO                   /* GLONASS G2 bias frequency: 0.43750E6 (Hz/n) */
#define FREQ3_GLO   CHC_FREQ3_GLO                   /* GLONASS G3 frequency: 1.202025E9 (Hz) */
#define FREQ1_CMP   CHC_FREQ1_CMP                   /* BeiDou B1C frequency: 1.57542E9 (Hz) */
#define FREQ2_CMP   CHC_FREQ2_CMP                   /* BeiDou B1 frequency: 1.561098E9 (Hz) */
#define FREQ5_CMP   CHC_FREQ5_CMP                   /* BeiDou B2a frequency: 1.17645E9 (Hz) */
#define FREQ7_CMP   CHC_FREQ7_CMP                   /* BeiDou B2 frequency: 1.20714E9 (Hz) */
#define FREQ8_CMP   CHC_FREQ8_CMP                   /* BeiDou B2a+b frequency: 1.191795E9 (Hz) */
#define FREQ6_CMP   CHC_FREQ6_CMP                   /* BeiDou B3 frequency: 1.26852E9 (Hz) */

#define ROUND_D(x)     floor((x)+0.5)

#define PMODE_SINGLE    CHC_PMODE_SINGLE
#define PMODE_RTK       CHC_PMODE_RTK

#define POLYSATPOSSHIFTSEC 10.0

/************************************** LAM LC *************************************************/
/* GPS */
#define LAM1_GPS   0.19029367279836487              /* L1: CLIGHT/FREQ1 */ 
#define LAM2_GPS   0.24421021342456825              /* L2: CLIGHT/FREQ2 */
#define LAM3_GPS   0.25482804879085386              /* L5: CLIGHT/FREQ5 */
#define LAM12_GPS  0.86191840032200564              /* L1-L2: CLIGHT/(FREQ1-FREQ2) */
#define LAM13_GPS  0.75141604130636389              /* L1-L5: CLIGHT/(FREQ1-FREQ5) */
#define LAM23_GPS  5.86104512218963832              /* L2-L5: CLIGHT/(FREQ2-FREQ5) */

/* BDS */
#define LAM1_CMP   0.19203948631027648              /* B1I: CLIGHT/FREQ2_CMP */ 
#define LAM2_CMP   0.24834936958430670              /* B2I: CLIGHT/FREQ7_CMP */ 
#define LAM3_CMP   0.23633246460442089              /* B3I: CLIGHT/FREQ6_CMP */ 
#define LAM4_CMP   0.19029367279836487              /* B1C: CLIGHT/FREQ1_CMP */ 
#define LAM5_CMP   0.25482804879085386              /* B2a: CLIGHT/FREQ5_CMP */ 
#define LAM6_CMP   0.25154700095234499              /* B2a+b: CLIGHT/FREQ8_CMP */ 
#define LAM7_CMP   0.24834936958430671              /* B2b: CLIGHT/FREQ7_CMP */ 
#define LAM12_CMP  0.84697183846671074              /* B11-B2I: CLIGHT/(FREQ2_CMP-FREQ7_CMP) */
#define LAM13_CMP  1.02465823814504166              /* B11-B3I: CLIGHT/(FREQ2_CMP-FREQ6_CMP) */
#define LAM14_CMP -20.9323040078201369              /* B1I-B1C: CLIGHT/(FREQ2_CMP-FREQ1_CMP) */
#define LAM15_CMP  0.77939429816351573              /* B1I-B2a: CLIGHT/(FREQ2_CMP-FREQ5_CMP) */
#define LAM23_CMP -4.88420426849136527              /* B21-B3I: CLIGHT/(FREQ7_CMP-FREQ6_CMP) */
#define LAM34_CMP -0.97684085369827305              /* B3I-B1C: CLIGHT/(FREQ6_CMP-FREQ1_CMP) */
#define LAM35_CMP  3.25613617899424351              /* B3I-B2a: CLIGHT/(FREQ6_CMP-FREQ5_CMP) */
#define LAM45_CMP  0.75141604130636389              /* B1C-B2a: CLIGHT/(FREQ1_CMP-FREQ5_CMP) */

/* GAL */
#define LAM1_GAL   0.19029367279836487              /* E1:  CLIGHT/FREQ1 */ 
#define LAM2_GAL   0.24834936958430671              /* E5b: CLIGHT/FREQ7 */ 
#define LAM3_GAL   0.25482804879085386              /* E5a: CLIGHT/FREQ5 */ 
#define LAM4_GAL   0.25154700095234499              /* E5a+b: CLIGHT/FREQ8 */ 
#define LAM12_GAL  0.81403404474856088              /* E1-E5b: CLIGHT/(FREQ1-FREQ7) */
#define LAM13_GAL  0.75141604130636389              /* E1-E5a: CLIGHT/(FREQ1-FREQ5) */
#define LAM14_GAL  0.78147268295861844              /* E1-E5a+b: CLIGHT/(FREQ1-FREQ8) */
#define LAM23_GAL  9.76840853698273053              /* E5b-E5a: CLIGHT/(FREQ7-FREQ5) */

/************************************** IF LC *************************************************/
/* GPS */
#define IF12_GPS_1       2.54572778016316015        /* L12-1:  FREQ1^2/(FREQ1^2-FREQ2^2) */
#define IF12_GPS_2      -1.54572778016316015        /* L12-2: -FREQ2^2/(FREQ1^2-FREQ2^2) */
#define IF13_GPS_1       2.26060432751882566        /* L15-1:  FREQ1^2/(FREQ1^2-FREQ5^2) */
#define IF13_GPS_3      -1.26060432751882566        /* L15-5: -FREQ5^2/(FREQ1^2-FREQ5^2) */
#define IF23_GPS_2       12.2553191489361702        /* L25-2:  FREQ2^2/(FREQ2^2-FREQ5^2) */
#define IF23_GPS_3      -11.2553191489361702        /* L25-5: -FREQ5^2/(FREQ2^2-FREQ5^2) */
#define GAMMA_GPS_IF12   1.64694444444444444        /* Gamma-L1L2: FREQ1^2/FREQ2^2 */

/* BDS */
#define IF12_CMP_1       2.48716831361692492        /* B12-1:  FREQ2_CMP^2/(FREQ2_CMP^2-FREQ7_CMP^2) */
#define IF12_CMP_2      -1.48716831361692492        /* B12-2: -FREQ7_CMP^2/(FREQ2_CMP^2-FREQ7_CMP^2) */
#define IF13_CMP_1       2.94368177014597839        /* B13-1:  FREQ2_CMP^2/(FREQ2_CMP^2-FREQ6_CMP^2) */
#define IF13_CMP_3      -1.94368177014597839        /* B13-3: -FREQ6_CMP^2/(FREQ2_CMP^2-FREQ6_CMP^2) */
#define IF14_CMP_1      -54.2511415525114155        /* B14-1:  FREQ2_CMP^2/(FREQ2_CMP^2-FREQ1_CMP^2) */
#define IF14_CMP_4       55.2511415525114155        /* B14-4: -FREQ1_CMP^2/(FREQ2_CMP^2-FREQ1_CMP^2) */
#define IF15_CMP_1       2.31438237445536367        /* B15-1:  FREQ2_CMP^2/(FREQ2_CMP^2-FREQ5_CMP^2) */
#define IF15_CMP_5      -1.31438237445536367        /* B15-5:  -FREQ5_CMP^2/(FREQ2_CMP^2-FREQ5_CMP^2) */
#define IF23_CMP_2      -9.58953168044077134        /* B23-2:  FREQ7_CMP^2/(FREQ7_CMP^2-FREQ6_CMP^2) */
#define IF23_CMP_3       10.58953168044077134       /* B23-3: -FREQ6_CMP^2/(FREQ7_CMP^2-FREQ6_CMP^2) */
#define IF34_CMP_3      -1.84364508393285372        /* B34-3:  FREQ6_CMP^2/(FREQ6_CMP^2-FREQ1_CMP^2) */
#define IF34_CMP_4       2.84364508393285372        /* B34-4: -FREQ1_CMP^2/(FREQ6_CMP^2-FREQ1_CMP^2) */
#define IF45_CMP_4       2.26060432751882566        /* B45-4:  FREQ1_CMP^2/(FREQ1_CMP^2-FREQ5_CMP^2) */
#define IF45_CMP_5      -1.26060432751882566        /* B45-5: -FREQ5_CMP^2/(FREQ1_CMP^2-FREQ5_CMP^2) */
#define GAMMA_CMP_IF12   1.67241884515943694        /* Gamma-B1IB2I: FREQ2_CMP^2/FREQ7_CMP^2 */

/* GAL */
#define IF12_GAL_1       2.42197712418300654        /* E12-1:  FREQ1^2/(FREQ1^2-FREQ7^2) */
#define IF12_GAL_2      -1.42197712418300654        /* E12-2: -FREQ7^2/(FREQ1^2-FREQ7^2) */
#define IF13_GAL_1       2.26060432751882566        /* E13-1:  FREQ1^2/(FREQ1^2-FREQ5^2) */
#define IF13_GAL_3      -1.26060432751882566        /* E13-3: -FREQ5^2/(FREQ1^2-FREQ5^2) */
#define IF14_GAL_1       2.33799137399876771        /* E14-1:  FREQ1^2/(FREQ1^2-FREQ8^2) */
#define IF14_GAL_4      -1.33799137399876771        /* E14-4: -FREQ6^2/(FREQ1^2-FREQ8^2) */
#define IF23_GAL_2       19.9198855507868383        /* E23-2:  FREQ7^2/(FREQ7^2-FREQ5^2) */
#define IF23_GAL_3      -18.9198855507868383        /* E23-3: -FREQ5^2/(FREQ7^2-FREQ5^2) */
#define GAMMA_GAL_IFE1E5a 1.79327032136105860       /* Gamma-E1E5a: FREQ1^2/FREQ5^2 */
#define GAMMA_GAL_IFE1E5b 1.70324619362252226       /* Gamma-E1E5b: FREQ1^2/FREQ7^2 */

/* GLO */
#define IF12_GLO_1(f1,f2) (f1*f1/(f1*f1-f2*f2))
#define IF12_GLO_2(f1,f2) (-f2*f2/(f1*f1-f2*f2))
#define IF13_GLO_1(f1,f3) (f1*f1/(f1*f1-f3*f3))
#define IF13_GLO_3(f1,f3) (-f3*f3/(f1*f1-f3*f3))
#define IF23_GLO_2(f2,f3) (f2*f2/(f2*f2-f3*f3))
#define IF23_GLO_3(f2,f3) (-f3*f3/(f2*f2-f3*f3))
#define IF12_GLO_SIM_1    2.53125                   /* (f1*f1/(f1*f1-f2*f2)) */
#define IF12_GLO_SIM_2   -1.53125                   /* (-f2*f2/(f1*f1-f2*f2)) */

#define SYS_NONE    CHC_SYS_NONE        /* navigation system: none */
#define SYS_GPS     CHC_SYS_GPS         /* navigation system: GPS */
#define SYS_SBS     CHC_SYS_SBS         /* navigation system: SBAS */
#define SYS_GLO     CHC_SYS_GLO         /* navigation system: GLONASS */
#define SYS_GAL     CHC_SYS_GAL         /* navigation system: Galileo */
#define SYS_QZS     CHC_SYS_QZS         /* navigation system: QZSS */
#define SYS_CMP     CHC_SYS_CMP         /* navigation system: BeiDou */
#define SYS_BD3     CHC_SYS_BD3         /* navigation system: BeiDou 3 */
#define SYS_ALL     CHC_SYS_ALL         /* navigation system: all */

#ifndef NFREQ
#define NFREQ       CHC_NFREQ          /* number of carrier frequencies */
#endif


#define NFREQGLO     CHC_NFREQGLO           /* number of carrier frequencies of GLONASS */
#ifndef NEXOBS
#define NEXOBS       CHC_NEXOBS             /* number of extended obs codes */
#endif

#define NFREQ_IF CHC_NFREQ_IF

#define MAXPRNGPS_TRUE CHC_MAXPRNGPS_TRUE
#define MINPRNGPS   CHC_MINPRNGPS
#define MAXPRNGPS   CHC_MAXPRNGPS
#define NSATGPS     CHC_NSATGPS
#define NSYSGPS     CHC_NSYSGPS

#define MINPRNGLO   CHC_MINPRNGLO
#define MAXPRNGLO   CHC_MAXPRNGLO
#define NSATGLO     CHC_NSATGLO
#define NSYSGLO     CHC_NSYSGLO

#define MINPRNGAL   CHC_MINPRNGAL
#define MAXPRNGAL   CHC_MAXPRNGAL
#define NSATGAL     CHC_NSATGAL
#define NSYSGAL     CHC_NSYSGAL
#define MINPRNQZS   CHC_MINPRNQZS
#define MAXPRNQZS   CHC_MAXPRNQZS
#define NSATQZS     CHC_NSATQZS
#define NSYSQZS     CHC_NSYSQZS

#define MINPRNCMP   CHC_MINPRNCMP
#define MAXPRNCMP   CHC_MAXPRNCMP
#define NSATCMP     CHC_MAXPRNCMP
#define NSYSCMP     CHC_NSYSCMP

#define MINPRNSBS   CHC_MINPRNSBS                 
#define MAXPRNSBS   CHC_MAXPRNSBS                
#define NSATSBS     CHC_NSATSBS 

//***min

#define MAXSAT      CHC_MAXSAT
	/* max satellite number (1 to MAXSAT) */
#define MAXOBS CHC_MAXOBS

#define SPP_SATNUMLMT_INNER (VERIQC_FLAG>1?28:20)

#define MAXDDOBS    CHC_MAXDDOBS        /*max number of double difference obs in an epoch*/

#define SOLQ_NONE           CHC_SOLQ_NONE                   /* solution status: no solution */
#define SOLQ_FIX            CHC_SOLQ_FIX                    /* solution status: fix */
#define SOLQ_FLOAT          CHC_SOLQ_FLOAT                  /* solution status: float */
#define SOLQ_SBAS           CHC_SOLQ_SBAS                   /* solution status: SBAS */
#define SOLQ_DGPS           CHC_SOLQ_DGPS                   /* solution status: DGPS/DGNSS */
#define SOLQ_SINGLE         CHC_SOLQ_SINGLE                 /* solution status: single */
#define SOLQ_PPP            CHC_SOLQ_PPP                    /* solution status: PPP */
#define SOLQ_DR             CHC_SOLQ_DR                     /* solution status: dead reconing */
#define MAXSOLQ             CHC_MAXSOLQ                     /* max number of solution status */

#define IONOOPT_OFF         CHC_IONOOPT_OFF                 /* ionosphere option: correction off */
#define IONOOPT_BRDC        CHC_IONOOPT_BRDC                /* ionosphere option: broadcast model */
#define IONOOPT_EST         CHC_IONOOPT_EST                 /* ionosphere option: estimation */

#define TROPOPT_OFF         CHC_TROPOPT_OFF                 /* troposphere option: correction off */
#define TROPOPT_SAAS        CHC_TROPOPT_SAAS                /* troposphere option: Saastamoinen model */
#define TROPOPT_EST         CHC_TROPOPT_EST                 /* troposphere option: ZTD estimation */

#define EPHOPT_BRDC         CHC_EPHOPT_BRDC                 /* ephemeris option: broadcast ephemeris */

#define DTTOL       0.0000000005               /* tolerance of time difference (s) */
#define MAXDTOE     7200.0              /* max time difference to ephem Toe (s) for GPS */
#define MAXDTOE_GLO 1800.0              /* max time difference to GLONASS Toe (s) */
#define MAXDTOE_BDS 3600.0

#define CODE_NONE   0                   /* obs code: none or unknown */
#define CODE_L1C    1                   /* obs code: L1C/A,G1C/A,E1C (GPS,GLO,GAL,QZS,SBS) */
#define CODE_L1P    2                   /* obs code: L1P,G1P    (GPS,GLO) */
#define CODE_L1W    3                   /* obs code: L1 Z-track (GPS) */
#define CODE_L1Y    4                   /* obs code: L1Y        (GPS) */
#define CODE_L1M    5                   /* obs code: L1M        (GPS) */
#define CODE_L1N    6                   /* obs code: L1codeless (GPS) */
#define CODE_L1S    7                   /* obs code: L1C(D)     (GPS,QZS) */
#define CODE_L1L    8                   /* obs code: L1C(P)     (GPS,QZS) */
#define CODE_L1E    9                   /* obs code: L1-SAIF    (QZS) */
#define CODE_L1A    10                  /* obs code: E1A        (GAL) */
#define CODE_L1B    11                  /* obs code: E1B        (GAL) */
#define CODE_L1X    12                  /* obs code: E1B+C,L1C(D+P) (GAL,QZS) */
#define CODE_L1Z    13                  /* obs code: E1A+B+C,L1SAIF (GAL,QZS) */
#define CODE_L2C    14                  /* obs code: L2C/A,G1C/A (GPS,GLO) */
#define CODE_L2D    15                  /* obs code: L2 L1C/A-(P2-P1) (GPS) */
#define CODE_L2S    16                  /* obs code: L2C(M)     (GPS,QZS) */
#define CODE_L2L    17                  /* obs code: L2C(L)     (GPS,QZS) */
#define CODE_L2X    18                  /* obs code: L2C(M+L),B1I+Q (GPS,QZS,CMP) */
#define CODE_L2P    19                  /* obs code: L2P,G2P    (GPS,GLO) */
#define CODE_L2W    20                  /* obs code: L2 Z-track (GPS) */
#define CODE_L2Y    21                  /* obs code: L2Y        (GPS) */
#define CODE_L2M    22                  /* obs code: L2M        (GPS) */
#define CODE_L2N    23                  /* obs code: L2codeless (GPS) */
#define CODE_L5I    24                  /* obs code: L5/E5aI    (GPS,GAL,QZS,SBS) */
#define CODE_L5Q    25                  /* obs code: L5/E5aQ    (GPS,GAL,QZS,SBS) */
#define CODE_L5X    26                  /* obs code: L5/E5aI+Q  (GPS,GAL,QZS,SBS) */
#define CODE_L7I    27                  /* obs code: E5bI,B2I   (GAL,CMP) */
#define CODE_L7Q    28                  /* obs code: E5bQ,B2Q   (GAL,CMP) */
#define CODE_L7X    29                  /* obs code: E5bI+Q,B2I+Q (GAL,CMP) */
#define CODE_L6A    30                  /* obs code: E6A        (GAL) */
#define CODE_L6B    31                  /* obs code: E6B        (GAL) */
#define CODE_L6C    32                  /* obs code: E6C        (GAL) */
#define CODE_L6X    33                  /* obs code: E6B+C,LEXS+L,B3I+Q (GAL,QZS,CMP) */
#define CODE_L6Z    34                  /* obs code: E6A+B+C    (GAL) */
#define CODE_L6S    35                  /* obs code: LEXS       (QZS) */
#define CODE_L6L    36                  /* obs code: LEXL       (QZS) */
#define CODE_L8I    37                  /* obs code: E5(a+b)I   (GAL) */
#define CODE_L8Q    38                  /* obs code: E5(a+b)Q   (GAL) */
#define CODE_L8X    39                  /* obs code: E5(a+b)I+Q (GAL) */
#define CODE_L2I    40                  /* obs code: B1I        (CMP) */
#define CODE_L2Q    41                  /* obs code: B1Q        (CMP) */
#define CODE_L6I    42                  /* obs code: B3I        (CMP) */
#define CODE_L6Q    43                  /* obs code: B3Q        (CMP) */
#define CODE_L3I    44                  /* obs code: G3I        (GLO) */
#define CODE_L3Q    45                  /* obs code: G3Q        (GLO) */
#define CODE_L3X    46                  /* obs code: G3I+Q      (GLO) */
#define CODE_L1I    47                  /* obs code: B1I        (BDS) */
#define CODE_L1Q    48                  /* obs code: B1Q        (BDS) */
#define CODE_L5D    49                  /* obs code: L5D        (QZS-Block II) */
#define CODE_L5P    50                  /* obs code: L5P        (QZS-Block II) */
#define CODE_L5Z    51                 /* obs code: L5Z        (QZS-Block II) */
#define CODE_L6E    52                  /* obs code: L6E        (QZS-Block II) */
#define CODE_L7D    53                  /* obs code: B2b        (BDS3-B2b) */
#define CODE_L7P    54                  /* obs code: B2b        (BDS3-B2b) */
#define CODE_L7Z    55                  /* obs code: B2b        (BDS3-B2b) */
#define CODE_L1D    56                  /* obs code: B1Q        (BDS3-B1) */
#define CODE_L8D    57                  /* obs code: B2a+b      (BDS3-B2) */
#define CODE_L8P    58                  /* obs code: B2a+b      (BDS3-B2) */
#define CODE_L6D    59                  /* obs code: B3A   D    (BDS3) */
#define CODE_L6P    60                  /* obs code: B3A   P    (BDS3) */
#define MAXCODE     61                  /* max number of obs code */

/* 0<=fre<7 */
#define BIT_MASK_PHASE(freq) (0x00000001<<(freq)) /* phase bit mask to validate obs */
#define BIT_MASK_CODE(freq)  (0x00000100<<(freq)) /* code bit mask to validate obs */
#define BIT_MASK_DOP(freq)   (0x00010000<<(freq)) /* dop bit mask to validate obs */
#define BIT_MASK_SNR(freq)   (0x01000000<<(freq)) /* snr bit mask to validate obs */
#define BIT_MASK_PHASE_ALL   (0x000000FF)
#define BIT_MASK_CODE_ALL    (0x0000FF00)
#define BIT_MASK_DOP_ALL     (0x00FF0000)
#define BIT_MASK_SNR_ALL     (0xFF000000)
#define BIT_MASK_OBS_ALL     (0xFFFFFFFF)


#ifdef WIN32
#define THREAD_T    HANDLE
#define LOCK_T      CRITICAL_SECTION
#define INITLOCK(f) InitializeCriticalSection(f)
#define LOCK(f)     EnterCriticalSection(f)
#define UNLOCK(f)   LeaveCriticalSection(f)
#define FILEPATHSEP '\\'
#else
#ifdef _PLATFORM_LINUX_
#define THREAD_T    pthread_t
#define LOCK_T      pthread_mutex_t
#define INITLOCK(f) pthread_mutex_init(f,NULL)
#define LOCK(f)     pthread_mutex_lock(f)
#define UNLOCK(f)   pthread_mutex_unlock(f)
#define FILEPATHSEP '/'
#else
#define THREAD_T    int
#define LOCK_T      int
#define INITLOCK(f)
#define LOCK(f)
#define UNLOCK(f)
#define FILEPATHSEP '/'
#endif
#endif   /*WIN32*/
	
/********************** utils **************************/
VERIQC_GET_USEDMEM_FUNC VeriQCGetMemusedFunc(void);
VERIQC_GET_FREEMEM_FUNC VeriQCGetMemfreeFunc(void);
VERIQC_TICKGET_FUNC VeriQCGetTickgetFunc(void);

	/* type definitions ----------------------------------------------------------*/

struct _GAL_IONO_PARAM_T;
struct _IONO_PARAM_T;
	
	typedef struct _CHC_OBSD_T
    {        /* observation data record */
		QC_TIME_T Time;       /* receiver sampling time (GPST) */
        unsigned char Sat;
        unsigned char AntID; /* satellite/receiver number (1:rover;2:base) */
        unsigned char Sys;
        unsigned char PRN; /* satellite system and prn */
        char SysIndex;            /* satellite system index */
        unsigned char FractionMark;//fraction mark
        unsigned char FreqNum;        /* real total frequency number of single epoch single sat obs*/
        unsigned char FreqNumMax;     /* max total frequency number of single epoch single sat obs*/
        unsigned int ObsValidMask;  /* bit0-6:freq; Byte0:phase, Byte1:code, Byte2:doppler, Byte3:SNR */
        short Index[CHC_NFREQ + CHC_NEXOBS];    /* whole freq index*/
        unsigned char SNR[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* signal strength (0.25 dBHz) */
        unsigned char LLI[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* loss of lock indicator */
        unsigned char Code[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* code indicator (CODE_???) */
        double L[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* observation data carrier-phase (cycle) */
        double P[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* observation data pseudorange (m) */
        float  D[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; /* observation data doppler frequency (Hz) */
        float  PCSR[CHC_NFREQ_SOLVE];// [CHC_NFREQ + CHC_NEXOBS]; //pseurange correlator sym ratio
	} CHC_OBSD_T;

	typedef struct _CHC_OBS_T
    {        /* observation data */
        int ObsNum;
        int ObsNumMax;         /* number of obervation data/allocated */
		QC_TIME_T Time;        /* receiver sampling time (GPST) */
		CHC_OBSD_T *pData;//[CHC_MAXOBS];       /* observation data records */
	} CHC_OBS_T;

	typedef struct _CHC_EPH_T {        /* GPS/QZS/GAL broadcast ephemeris type */
		int Sat;            /* satellite number */
        int IODE;
        int IODC;      /* IODE,IODC */
		int SVA;            /* SV accuracy (URA index) */
		int SVH;            /* SV health (0:ok) */
		int Week;           /* GPS/QZS: gps week, GAL: galileo week */
		int Code;           /* GPS/QZS: code on L2, GAL/CMP: data sources */
		int Flag;           /* GPS/QZS: L2 P data flag, CMP: nav type */
        QC_TIME_T Toe;
        QC_TIME_T Toc;
        QC_TIME_T Ttr; /* Toe,Toc,T_trans */
		/* SV orbit parameters */
        double A;
        double E;
        double I0;
        double OMG0;
        double OMG;
        double M0;
        double DeltaN;
        double OMGd;
        double IDot;
		double Crc;
        double Crs;
        double Cuc;
        double Cus;
        double Cic;
        double Cis;
		double ToeSec;        /* Toe (s) in week */
		double FitInterval;         /* fit interval (h) */
		double Af0;
        double Af1;
        double Af2;    /* SV clock parameters (af0,af1,af2) */
		double Tgd[4];      /* group delay parameters */
		/* GPS/QZS:tgd[0]=TGD */
		/* GAL    :tgd[0]=BGD E5a/E1,tgd[1]=BGD E5b/E1 */
		/* CMP    :tgd[0]=BGD1,tgd[1]=BGD2 */
		double ADot;
        double NDot;
	} CHC_EPH_T;

	typedef struct _CHC_GEPH_T
    {        /* GLONASS broadcast ephemeris type */
		int Sat;              /**< satellite number */
		int IODE;             /**< IODE (0-6 bit of tb field) */
		int Freq;             /**< satellite frequency number */
        int SVH;
        int SVA;
        int Age;              /**< satellite health, accuracy, age of operation */
		QC_TIME_T Toe;        /**< epoch of epherides (gpst) */
		QC_TIME_T Tof;        /**< message frame time (gpst) */
		double Pos[3];        /**< satellite position (ecef) (m) */
		double Vel[3];        /**< satellite velocity (ecef) (m/s) */
		double Acc[3];        /**< satellite acceleration (ecef) (m/s^2) */
        double Taun;
        double Gamn;          /**< SV clock bias (s)/relative freq bias */
		double DTaun;         /**< delay between L1 and L2 (s) */
		int LeapSec;          /**< leap seconds*/
	} CHC_GEPH_T;

	typedef struct _CHC_NAV_T {        /**< navigation data type */
        int Num;
        int NumMax;                    /**< number of broadcast ephemeris */
        int NumGLO;
        int NumGLOMax;                 /**< number of glonass ephemeris */
        unsigned int NumBD2;           /**< number of BD2 ephemeris */
        int NumBD3;
        int NumMaxBD3;                 /**< number of BD3 ephemeris */
		CHC_EPH_T *pEph;               /**< GPS/QZS/GAL ephemeris */
		CHC_GEPH_T *pGEph;             /**< GLONASS ephemeris */
		CHC_EPH_T *pEphBD3;            /**< CMP-BD3 ephemeris */
        int LeapSec;                   /**< leap seconds (s) */
		double CBias[CHC_MAXSAT][3];   /**< code bias (0:p1-p2,1:p1-c1,2:p2-c2) (m) */
        struct _IONO_PARAM_T* pIonoParamGPS;     /**< GPS iono model parameters */
        struct _IONO_PARAM_T* pIonoParamBDS;     /**< BDS iono model parameters */
        struct _IONO_PARAM_T* pIonoParamQZS;     /**< QZSS iono model parameters */
		unsigned char GLOFreqNum[CHC_MAXPRNGLO + 1]; /* GLONASS Frequency number, 0 ~ 13 */
	} CHC_NAV_T;

	typedef struct {
		QC_TIME_T SolTime;
		double Rr[6];
		double Qr[6];
		double QVel[6];
		U1 Stat;
		U1 VStat;
        float SPPRatio;
	}CHC_XSIMSOL_T;

    typedef struct 
    {
        U1 MotionMode;//0-unknown; 1 - static; 2 - stable movement; 3 - irregual movement /**< @todo what ?*/
        U1 StaticCnt;
        U1 RandomCnt;
        U1 CVCnt;
        U1 EnvState;//0 - unknown; 1-bad; 2-good; 3->very good; 4->very bad
        U1 EnvChangeMark;   /*  0 : unknown;
                                1 : not change;
                                2 : bad->good;
                                3 : bad->very good;
                                4 : bad->very bad;
                                5 : good->bad;
                                6 : good->very good;
                                7 : good->very bad;
                                8 : very good->bad;
                                9 : very good->good;
                                10: very good->very bad;
                                11: very bad->bad;
                                12: very bad->good;
                                13: very bad->very good */
        QC_TIME_T EnvChangeGt3;
    }CHC_ENVDETECTINFO_T;

	typedef struct _CHC_SOL_T
    {        /* solution type */
		QC_TIME_T Time;       /* time (GPST) */
		double Rr[9];       /* position/velocity/acceleration (m|m/s|m/s^2) */
		/* {x,y,z,vx,vy,vz} or {e,n,u,ve,vn,vu} */
		double  Qr[6];       /* position variance/covariance (m^2) */
		double QVel[6];
		U1 VStat;
		/* {c_xx,c_yy,c_zz,c_xy,c_yz,c_zx} or */
		/* {c_ee,c_nn,c_uu,c_en,c_nu,c_ue} */
		double Dtr[6];      /* receiver clock bias to time systems (s) */
        double DtrDot[6];      /* receiver clock drift to time systems (s) */
		unsigned char Type; /* type (0:xyz-ecef,1:enu-baseline) */
        unsigned char SolStat;/* SOL_STATUS_??? */
		unsigned char Stat; /* solution status (SOLQ_???) */
		unsigned char SatNum;   /* number of valid satellites */
		float Age;          /* age of differential (s) */
		float Ration;
		double RMS;         /* rms */
		double PosRms;      /* posrms */
        double PosRmsLimitPre;
		double VelRms;       //doppler or tdf
		double Dop[5];  //0-GDOP 1-PDOP 2-HDOP 3-VDOP 4-TDOP
		int ARSat[CHC_MAXSAT];
		unsigned char ARSig[CHC_MAXSAT];
		int ARSatNum;      // use to output
		double PseuRms;
		int PseuRmsCnt;
		U1 SPPSat[CHC_MAXSAT];
		QC_TIME_T SPPSatUpdateTime;
		CHC_XSIMSOL_T XSimSol;
        float SemiStd[3];
		int SolCnt;

        unsigned char AgeOk;    /* is diff age ok 0=invalid, 1=valid*/
        CHC_ENVDETECTINFO_T EnvDetectInfo;
        CHC_ENVDETECTINFO_T PreEnvDetectInfo;
		int ProcSatNum;   /* number of processed satellites */
		int Iter;	      /* number of iteration*/
	} CHC_SOL_T;
    
    typedef struct _CHC_SSAT_T
    {
        unsigned char Sat;
		unsigned char Sys;  /* navigation system */
		unsigned char ValidSatFlagSingle;   /* valid satellite flag single */
        unsigned char SatStatus; /* sat status, 0:good, 1:badeph, 2:oldeph, 3:lowele, 4:misclosure, 5:nodiffcorr, 6:noeph, 7:invalid iode, 8:lockedout, 9:lowpower*/
		double AzEl[2];     /* azimuth/elevation angles {az,el} (rad) */
		unsigned char ValidSatFlag[CHC_NFREQ]; /* valid satellite flag */
		unsigned char SNR[CHC_NFREQ]; /* signal strength (0.25 dBHz) */
		QC_TIME_T LockTime[CHC_NFREQ]; /*  lock time, update every epoch*/
        
        float PSR;
        float PSRRate0;
        float PSRAcc;
        float PSRRes;
        float PSRRate0Res;
        unsigned char PSRFreqIndex; /**< 0,1,2,3,4,5: GPS:L1/L2/L5; GLN:G1/G2/G3; GAL:E1/E5b/E5a/E5ab/E6; CMP:B1I/B2I/B3I/B1C/B2a/B2 */
        U1 GLOFreq;
        U1 PSRErrLevel; /**< PRS residual level: 1 level == 0.1m, Max Level: 250; Level == 251-254: reserved; Level == 255: SBAS error flag */
        unsigned char UsedFrequencyMark;/* bit0/1/2/3/4; GPS:L1/L2/L5; GLN:G1/G2/G3; GAL:E1/E5b/E5a/E5ab/E6; CMP:B1I/B2I/B3I/B1C/B2a/B2 */
        double Lam[CHC_NFREQ];   /* satellite wavelength */
	} CHC_SSAT_T;
	
	typedef struct _CHC_SSATLIST_T
    {
		CHC_SSAT_T *pSsat;
		short Index[CHC_MAXSAT];
		U1 NumMax;
		U1 Num;
	}CHC_SSATLIST_T;

    /** @TODO: rebuild CHC_ssatlist_t */

	typedef struct _CHC_PRCOPT_T
    {        /* processing options type */
		int Mode;           /**< positioning mode PMODE_SINGLE, PMODE_DGPS, PMODE_RTK, ....*/
		int SolType;        /**< solution type (0:forward,1:backward) */
		int FreqNum;        /**< number of frequencies (1:L1,2:L1+L2,3:L1+L2+L5) */
		int NavSys;         /**< navigation system */
		double EleMin;      /**< elevation mask angle (rad) */
		int SatEph;         /**< satellite ephemeris/clock (EPHOPT_???) */
		int IonoOpt;        /**< ionosphere option (IONOOPT_???) */
		int TropOpt;        /**< troposphere option (TROPOPT_???) */
		int Dynamics;       /**< dynamics model (0:none,1:velociy,2:static,3:fixed) */
		/* (0:pos in prcopt,  1:average of single pos, */
		/*  2:read from file, 3:rinex header, 4:rtcm pos) */
		double ErrorRatio[CHC_NFREQ]; /* code/phase error ratio */
		double ErrorFactor[5];      /* measurement error factor */
		/* [0]:reserved */
		/* [1-3]:error factor a/b/c of phase (m) */
		/* [4]:doppler frequency (hz) */
		double InitStateSTD[3];      /* initial-state std [0]bias,[1]iono [2]trop */
		double ProcessNoiseSTD[5];      /* process-noise std [0]bias,[1]iono [2]trop [3]acch [4]accv */
		double MaxGDOP;     /* reject threshold of gdop */
		unsigned char ExSats[CHC_MAXSAT]; /* excluded satellites (1:excluded,2:included) */
		int SatNumLimit;     /*select satellite num:0,no;>0,yes*/
		int MaxCPUTimeMsSatPara; //for satpara(ms)
		U1 EphInterpolation;//0-no;1-yes
		float SPPGap;//default:1.0
		U1 CroverDopplerVel;//0-no compute rover doppler vel;1-yes,compute
        
        U1 EnableFaster;
        int RefID;
	} CHC_PRCOPT_T;

	typedef struct _CHC_SATIPTINFO_T 
    {
       U1 Sat;
	   U1 ValidFlag;
	   U1 SVH;
	   U1 Computing;
       U1 XSUsing;
	   U1 Reserved;
	   QC_TIME_T Toc;
	   QC_TIME_T Toe;
	   int IODE;
	   int IODC;
	   QC_TIME_T *pFtPNTTime;
       double *pRs;
	   double *pXs;
	   double Mp;
	   double NDot;
	   double OmgeToeSec;
	   double Se2;
	   double RelCorrConst;//relativity correction constant
	   double Var;
	   double M0;
	   double E;
	   double Af0;//glo taun
	   double Af1;//glo gamn
	   double Af2;
	   U1 ParaNum;
	   U1 ObsNum;    
	}CHC_SATIPTINFO_T;//satellite interpolation information

    typedef enum _STATION_TYPE_E
    {
        STATION_UNKNOWN = 0,
        STATION_ROVER   = 1,
        STATION_BASE    = 2,
        STATION_MAX     = 0xFFFFFFFF
    }STATION_TYPE_E;

	typedef struct {
		double *pIminusJ;
        U1 MemTypeIminusJ;
		double *pATQ;
        U1 MemTypeATQ;
		double *pATQA;
        U1 MemTypeATQA;
		int NV;
		int ParaNum;
	}CHC_ADJ_ROBUST_BUFFER_T;

	typedef struct _CHC_ONE_PSEURESULT_T
    {
		CHC_SOL_T  Sol;        /**< solution */
		CHC_OBSD_T *pObs;
		int AllObsNum;                 /**< total obsveration number{n=nr+nu} */
		double *pRs;            /**< satellite position list */
		double *pDts;           /**< satellite clock list */
		double *pVar;
        int *pToeSec;
        int RoverObsNum;                /**< obsvervation number of rover */
        int CommonSatNum;                /**< common satellite number for rover and base */
        int *pSatIndex;               /**< base index in rover and base satellite list */
        int *pSat;              /**< SatNO list */
        int *pIODE;             /**< EPH IODE list */
        U1  *pEphType;
		U1 Invalid[CHC_MAXSAT];/**< invaild flag for satellite */
		U1 GLOErr;
		int MaxNum;
        double PseuphasedPosRms;
		VERIQC_MEM_MANAGER_T MemManager;

	}CHC_ONE_PSEURESULT_T;

    typedef struct _CHC_SPP_SOLVE_T
    {
        CHC_PRCOPT_T Opt;       /* processing options */

        CHC_ONE_PSEURESULT_T OnePseuResult;
        CHC_SSATLIST_T Ssatlist;

        double PureSPPRms[2];          /**< pure SPP position 0: rms and 1: pos rms */
        double PureSPPVarCoor[6];      /**< pure SPP position variance/covariance (m^2) */
        double PureSPPVarVel[6];       /**< pure SPP vel variance/covariance (m^2) */

        double TimeDiff;
    }CHC_SPP_SOLVE_T;

    struct _RM_RANGE_LOG_T;
    struct _UNI_EPHEMERIS_T;
    struct _GLO_EPHEMERIS_T;
    
    typedef struct 
    {
        int Sat;
        U1 Sys;
        char SysIndex;
        double P;
        U1 ObsType;
        double VarMeasBias;//fix,code bias var
        double VarEphBias;//eph(pos/clk) bias var
        double VarChkBias;//adjust by bias check each other
        double VarSNRBias;//adjust by snr
        double Rs[3];
        double Dts[2];
        U1 SVH;
        U1 SNRf1;
        U1 SNRf2;
    }MIX_PSEUOBSD_T;
	
    typedef struct 
    {
        int ObsNum;
        QC_TIME_T Time;
        MIX_PSEUOBSD_T MixPseuObsd[SPP_SATNUMLMT_INNER];
    }MIX_PSEUOBS_T;

    extern int VeriQCSortObs(CHC_OBSD_T* pEpochData, int ObsNum);
    extern int VeriQCCreateSatInfoArray(CHC_SATIPTINFO_T** ppSatInfoArr, int Num);
    extern int VeriQCReleaseSatInfoArray(CHC_SATIPTINFO_T** ppSatInfoArr, int Num);
    extern int VeriQCOrbitFitting(int NavSys, int SolType, const CHC_OBSD_T* pObs, int Num, CHC_SATIPTINFO_T* pSatIptInfo, const CHC_NAV_T* pNav,
        int MaxCPUTimes, unsigned char* pEphValidFlag, unsigned char* pEphActiveValidFlag, unsigned char* pAlmValidFlag, int BD3Flag);
    extern int VeriQCSatSys(int Sat, int* pPRN);
    extern int VeriQCSatNo(int Sys, int PRN);

    extern int GetSystemIDByIndex(int SysIndex);
    extern int GetSystemIndexByID(int SystemID);
    
    extern QC_TIME_T VeriQCGLOEphTime2Time(int LeapYears, int DayNum, double SecOfDay);
    extern QC_TIME_T VeriQCUNIEphTime2Time(int Sys, int Week, double SOW);

    extern int32_t VeriQCParseRangeSatNo(int Sys, int PRN, int* pSysInner, int* pPrnInner);
    extern int32_t VeriQCParseEphemerisSystem(int Sys);
    extern int32_t VeriQCGetNsatFromRange(const struct _RM_RANGE_LOG_T* pObs);
    /************************copy functions*************************/
    extern int VeriQCCopyTimeFromSPP(const QC_TIME_T* pSrc, QC_TIME_T* pDst);

    //VERIQC2.0
    extern int VeriQCInitSPPTcOptDefault(VERIQC_SPPTC_OPT_T* pOpt);
    extern int VeriQCCopySPPTcOptCtrl(const VERIQC_SPPTC_OPT_T* pSrc, VERIQC_SPPTC_OPT_T* pDst, VERIQC_SPPTC_OPT_CTRL_E OptCtrl);
    extern int VeriQCCopyPrcoptCtrl(const CHC_PRCOPT_T* pSrc, CHC_PRCOPT_T* pDst, VERIQC_SPPTC_OPT_CTRL_E OptCtrl);

    extern int VeriQCConvertObsRange(VERIQC_SPPTC_OPT_T* pOpt, const struct _RM_RANGE_LOG_T* pRangeLog, struct _CHC_OBS_T* pObs,
        unsigned char* pGLOFcn, int StationType);
    extern int VeriQCConvertObsRangeSimple(VERIQC_SPPTC_OPT_T* pOpt, const struct _RM_RANGE_LOG_T* pRangeObs, struct _CHC_OBS_T* pDst);
    extern int VeriQCConvertSysID(const VERIQC_SYSID_E SysID); //TODO:
    extern VERIQC_SYSID_E VeriQCConvertToSysID(const int Sys);
    extern int VeriQCConvertSigID(uint8_t SigID);
    extern unsigned char VeriQCConvertSigIDRange(int Sys, uint32_t SigID, unsigned char* pFreq);
    extern int VeriQCConvertUNIEphemeris(const struct _UNI_EPHEMERIS_T* pSrc, struct _CHC_EPH_T* pDst);
    extern int VeriQCConvertGLOEphemeris(const struct _GLO_EPHEMERIS_T* pSrc, struct _CHC_GEPH_T* pDst);
    extern int VeriQCConvertSolToSolInfo(const struct _CHC_SOL_T* pSrc, const struct _CHC_SSATLIST_T* pSrcSsat,
        const VERIQC_ORBIT_SOL_T* pOrbSol, VERIQC_SOLINFO_T* pDst);

    /** Memory */
    extern int VeriQCMallocObs(CHC_OBS_T* pObs, int Num);
    extern int VeriQCMallocOrbitSol(VERIQC_ORBIT_SOL_T* pSol, int Num);
    extern int VeriQCMallocSolInfo(VERIQC_SOLINFO_T* pSol, int NSat);

    extern int VeriQCClearSolInfo(VERIQC_SOLINFO_T* pSol);
    extern int VeriQCClearOrbitSol(VERIQC_ORBIT_SOL_T* pOrbitSol);

    extern int VeriQCFreeSolInfo(VERIQC_SOLINFO_T* pSol);
    extern int VeriQCFreeOrbitSol(VERIQC_ORBIT_SOL_T* pSol);
    extern int VeriQCFreeObs(CHC_OBS_T* pObs);

    extern void VeriQCOutSPPLog(const char* pMsg);
	
    /* global variables ----------------------------------------------------------*/

    extern const double g_Chisqr005[];

    extern CHC_PRCOPT_T g_PrcOptDefault;   /* default positioning options */

    /* satellites, systems, codes functions --------------------------------------*/
    extern int SatNo(int Sys, int PRN);
    extern int SatSys(int Sat, int* pPRN);
    extern int IsValidPhase(double Phase);
    extern int IsValidDoppler(double Dopler);
    extern int IsValidPseu(double Pseu);
    extern int IsValidSNR(int SNR);

        /*error printf*/
    extern void InitialOptDefault(CHC_PRCOPT_T* pOpt);

    extern int Solve(MAT_MUL_TYPE_E Type, const double* pA, const double* pY, int N,
        int M, double* pX);
    extern int LSQPosFastSpt(const double* pA, const double* pTp, const double* pY, int N, int M, double* pX, double* pQt,
        double* pSigma02, double* pRMS, double* pResidual, double* pPe);

    extern unsigned int TickGet(void);
    extern QC_TIME_T GLOEphTime2Time(int LeapYears, int DayNum, double SecOfDay);
    extern void Time2GLOEphTime(QC_TIME_T* pTime, int* pLeapYear, int* pDayNum, double* pSecOfDay);
    extern QC_TIME_T UNIEphTime2Time(int Sys, int Week, double SOW);

    /* platform dependent functions ----------------------------------------------*/

    /* coordinates transformation ------------------------------------------------*/
    extern void CovENU(const double* pPos, const double* pP, double* pQ);
    extern void SolToCov(const CHC_SOL_T* pSol, double* pP);
    extern void SolVelToCov(const CHC_SOL_T* pSol, double* pP);

    /* positioning models --------------------------------------------------------*/
    extern double SatAzEl(const double* pPos, const double* pE, double* pAzEl);
    extern void Dops(int ObsNum, const double* pAzEl, double EleMin, double* pDop);

    /* atmosphere models ---------------------------------------------------------*/

    extern double IonoDelayGPS(const struct _IONO_PARAM_T* pInputIonoParam, const double* pPosBLH, double GPSSecond, const double* pAzEl);
    extern double IonoDelayBDS(const struct _IONO_PARAM_T* pInputIonoParam, const double* pPosBLH, double BDSSecond, const double* pAzEl);

    extern int GetTropModelConstant(const double* pPos, double Pres0, double Temp0, double Humi, double* pTrphXs, double* pTrpwXs);
    extern double TropModelInputSeaLevelParaAdv(QC_TIME_T Time, const double* pPos, const double* pAzEl, double TrphXs, double TrpwXs);

    extern int IonoCorrBroadcast(double GPSSecond, const CHC_NAV_T* pNav, int Sys, const double* pPosBLH,
        const double* pAzEl, const CHC_SSAT_T* pSsat, double* pIono, double* pVar);

    /* ephemeris and clock functions ---------------------------------------------*/
    extern double Eph2Clk(QC_TIME_T Time, const CHC_EPH_T* pEph);
    extern double GEph2Clk(QC_TIME_T Time, const CHC_GEPH_T* pGEph);
    extern void Eph2Pos(QC_TIME_T Time, const CHC_EPH_T* pEph, double* pRs, double* pDts,
        double* pVar, double* pRelCorr);
    extern void GEph2Pos(QC_TIME_T Time, const CHC_GEPH_T* pGEph, double* pRs, double* pDts,
        double* pVar);
    extern unsigned int SatPoss(CHC_SATIPTINFO_T* pSatIptInfo, QC_TIME_T Time, const CHC_OBSD_T* pObs, int Num, const CHC_NAV_T* pNav,
        int SatEph, double* pRs, double* pDts, double* pVar, U1* pSVH, U1* pType, int* pIODE, int* pToeSec, const U1* pExcludeSats,
        U1 IncrementUpdateMark, int BD3Flag, int16_t* pSatNo, unsigned int* pNumGAL);
    extern int GetEphPolyInfo(int SolType, QC_TIME_T Time, const CHC_EPH_T* pEph, CHC_SATIPTINFO_T* pSatIptInfo);
    extern int GetGEphPolyInfo(int SolType, QC_TIME_T Time, const CHC_GEPH_T* pGEph, CHC_SATIPTINFO_T* pSatIptInfo);
    extern int UpdateInterSatPosInfo(int NavSys, int SolType, const CHC_OBSD_T* pObs, int Num,
        CHC_SATIPTINFO_T* pSatIptInfo, const CHC_NAV_T* pNav, int maxcputimes, unsigned char* pEphValidFlag, unsigned char* pEphActiveValidFlag, unsigned char* pAlmValidFlag);
    extern int UpdateInterSatPosInfoBD3(int SolType, const CHC_OBSD_T* pObs, int Num,
        CHC_SATIPTINFO_T* pSatIptInfo, const CHC_NAV_T* pNav, int MaxCPUTimes, unsigned char* pEphValidFlag, unsigned char* pEphActiveValidFlag);
    extern int InitSatIptInfo(CHC_SATIPTINFO_T* pSatIptInfo, int Num);
    extern void EndSatIptInfo(CHC_SATIPTINFO_T* pSatIptInfo, int Num);
    extern BOOL GetSatApprPosVelClock(CHC_SATIPTINFO_T* pSatIptInfo, QC_TIME_T Time, int Sat, const CHC_NAV_T* pNav, double* pSatRs, double* pSatDts, int BD3Flag);
    extern BOOL GetSatPosVelClockSpecifyToe(CHC_SATIPTINFO_T* pSatIptInfo, QC_TIME_T TransTime, QC_TIME_T Time, int Sat, const CHC_NAV_T* pNav, double* pSatRs, double* pSatDts, int BD3Flag, int Toe);

#ifdef __cplusplus
}
#endif
#endif /* CHCGNSSBASE_H */

