/**********************************************************************//**
		   VeriQC

	CHC Quality Check project
*-
@file   VeriQC.h
@author CHC
@date   2024/05/28
@brief  VeriQC interface

**************************************************************************/
#ifndef _VERI_QC_H_
#define _VERI_QC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>

#if defined(__linux)
#define PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#elif defined(__aarch)

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif /**< NULL */
#define QC_MAX_FILE_NAME_LENGTH       512         /**< max file name length */
#define QC_MAX_SNRGROUP_STR_LENGTH    (9*3+1)     /**< max SNR group STR length */
#define QC_MAX_MSG_LEN                128
#define QC_INVALID_AZEL_VALUE         (-1000.0)   /**< invalid azimuth/elevation sentinel */

#define QC_DIAG_TOP_WIN_NUM           5           /**< top diagnostic time window number */
#define QC_DIAG_TOP_BREAK_NUM         3           /**< top observation break number */
#define QC_CN0_DROP_WINDOW_SEC        300.0       /**< one CN0 drop window time, s */
#define QC_CN0_DROP_EDGE_SEC          60.0        /**< CN0 drop window step, s */
#define QC_SLIP_WINDOW_SEC            300.0       /**< one slip window time, s */
#define QC_SLIP_EDGE_SEC              60.0        /**< slip window step, s */
#define QC_SLIP_EPOCH_RATIO_LIMIT     0.3f        /**< abnormal epoch slip ratio, epoch slip number/epoch sat number */
#define QC_BREAK_TIME_LIMIT           60.0        /**< observation break time limit, s */

typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define QC_WIN_SIZE_MP 50                         /**< Windows size for MP */

#define QC_MAX_NUM_OBS_FILE       2               /**< MaxNum of obsFile: rover and base */
#define MAX_NUM_NAV_FILE          (2*5)           /**< MaxNum of navFile: G\R\E\C four system without J\S and I*/
#define MAX_NUM_INPUT_FILE        \
       (QC_MAX_NUM_OBS_FILE+MAX_NUM_NAV_FILE)     /**< 2 obs file and 8 nav file */

/** some GNSS data define */
/**
* GPS	    L1, L2,  L5,  L1C-P,   L2Z-traching
* BDS	    B1, B2,  B3,  B1C,     B2a, B1A, B3A, B2b, B2a+B2b
* GLONASS	G1, G1a, G2,  G2a,     G3
* Galileo	E1, E5A, E5B, E5(A+B), E6
* QZSS	    L1, L2,  L5,  L5S,     L6,  L1C(P), L1S
* SBAS	    L1, L5
* XW1	    L1L,L1H, L2L, L2H
*/
#define QC_SYSTEM_NUM             7               /**< system number */
#define QC_GPS_FRE_NUM            5               /**< GPS frequency number */
#define QC_GLONASS_FRE_NUM        2               /**< GLO frequency number */
#define QC_GALILEO_FRE_NUM        5               /**< GAL frequency number */
#define QC_BDS_FRE_NUM            6               /**< BDS frequency number */
#define QC_QZSS_FRE_NUM           6               /**< QZS frequency number */
#define QC_SBAS_FRE_NUM           2               /**< SBA frequency number */
#define QC_IRNSS_FRE_NUM          2               /**< NIC frequency number */
#define QC_XW1_FRE_NUM            4               /**< XW1 frequency number */

#define QC_MAXELEVINTERV          9               /**< elevation group number for SNR statistics */
#define QC_NUM_FREQ               QC_BDS_FRE_NUM  /**< number of carrier type */
#define QC_NUM_EXTEND_OBS_CODE    5               /**< extend frequency number */
#define QC_MAX_NUM_CODE           70              /**< max frequency number */
#define QC_MAX_NUM_OBS_TYPE       64              /**< max observation types */
#define QC_MAX_NUM_OBS            256             /**< number of obs in one epoch */
#define QC_MAX_SIG_TYPE           20              /**< number of signal type in one system */
#define QC_MIN_SAMPLE_INTERVAL    0.0001          /**< min value of file sample interval, s */
#define QC_MIN_COMPLETE_FREQ      1               /**< min complete freq obs for use rate */

#define QC_SYS_NONE               0x00            /**< navigation system: none */
#define QC_SYS_GPS                0x01            /**< navigation system: GPS */
#define QC_SYS_SBS                0x02            /**< navigation system: SBAS */
#define QC_SYS_GLO                0x04            /**< navigation system: GLONASS */
#define QC_SYS_GAL                0x08            /**< navigation system: Galileo */
#define QC_SYS_QZS                0x10            /**< navigation system: QZSS */
#define QC_SYS_CMP                0x20            /**< navigation system: BeiDou */
#define QC_SYS_BD3                0x40            /**< navigation system: BeiDou */
#define QC_SYS_XW1                0x80            /**< navigation system: LEO */
#define QC_SYS_ALL                0xFF            /**< navigation system: all */

/** GPS */
#define QC_MIN_PRN_GPS            1               /**< min gps satellite number */
#define QC_MAX_PRN_GPS_TRUE       32
#define QC_MAX_PRN_GPS            42              /**< max gps satellite number, 33~42: QZSS */
#define QC_NUM_SYS_GPS            1
#define QC_NUM_SAT_GPS            (QC_MAX_PRN_GPS-QC_MIN_PRN_GPS+1) /**< gps satellite number */
/** GLONASS */
#define QC_MIN_PRN_GLO            1               /**< min glonass satellite number */
#define QC_MAX_PRN_GLO            27			  /**< max glonass satellite number */
#define QC_NUM_SYS_GLO            1
#define QC_NUM_SAT_GLO            (QC_MAX_PRN_GLO-QC_MIN_PRN_GLO+1) /**< glonass satellite number */
/** Galileo */
#define QC_MIN_PRN_GAL            1               /**< min galileo satellite number */
#define QC_MAX_PRN_GAL            36			  /**< max galileo satellite number */
#define QC_NUM_SYS_GAL            1
#define QC_NUM_SAT_GAL            (QC_MAX_PRN_GAL-QC_MIN_PRN_GAL+1) /**< galileo satellite number */
/** BeiDou */
#define QC_MIN_PRN_CMP            1               /**< min BeiDou satellite number */
#define QC_MAX_PRN_CMP            64			  /**< max BeiDou satellite number */
#define QC_NUM_SYS_CMP            1
#define QC_NUM_SAT_CMP            (QC_MAX_PRN_CMP-QC_MIN_PRN_CMP+1) /**< BeiDou satellite number */
/** QZSS */
#define QC_MIN_PRN_QZS            193             /**< min QZSS satellite number */
#define QC_MAX_PRN_QZS            202			  /**< max QZSS satellite number */
#define QC_NUM_SYS_QZS            1
#define QC_NUM_SAT_QZS            0               /**< (QC_MAX_PRN_QZS-QC_MIN_PRN_QZS+1) QZSS satellite number */
/** SBAS */
#define QC_MIN_PRN_SBS            120             /**< min SBAS satellite number */
#define QC_MAX_PRN_SBS            158			  /**< max SBAS satellite number */
#define QC_NUM_SYS_SBS            1
#define QC_NUM_SAT_SBS            (QC_MAX_PRN_SBS-QC_MIN_PRN_SBS+1) /**< SBAS satellite number */

/** XW1 */
#define QC_MIN_PRN_XW1            22              /**< min XW1 satellite number */
#define QC_MAX_PRN_XW1            620			  /**< max XW1 satellite number */
#define QC_NUM_SYS_XW1            1
#define QC_NUM_SAT_XW1            (QC_MAX_PRN_XW1-QC_MIN_PRN_XW1+1) /**< XW1 satellite number */

/** max number of satellites and navigation systems */
#define QC_MAX_NUM_SAT            (QC_NUM_SAT_GPS+QC_NUM_SAT_GLO+QC_NUM_SAT_GAL+QC_NUM_SAT_CMP+QC_NUM_SAT_QZS+QC_NUM_SAT_SBS+QC_NUM_SAT_XW1) /**< max satellite number */
#define QC_MAX_NUM_SYS            (QC_NUM_SYS_GPS+QC_NUM_SYS_GLO+QC_NUM_SYS_GAL+QC_NUM_SYS_CMP+QC_NUM_SYS_QZS+QC_NUM_SYS_SBS+QC_NUM_SYS_XW1) /**< max system number */

#define QC_LC_NUM                 6               /**< GF/MW combination number */
#define QC_MAXSIGTYPE             32              /**< max signal type */


/** some strtring length define in config file */
#define MAX_FILE_NAME_LENGTH      512
#define MAX_TIME_STR_LENGTH       20
#define MAX_FRE_STR_LENGTH        42 
#define MAX_SYS_STR_LENGTH        (QC_SYSTEM_NUM*2)
#define MAX_OBS_STR_LENGTH        (4*2)
#define MAX_NUM_SAT_BAN           (QC_SYSTEM_NUM*4) /**< BanPRN str length max= satnum * 4 */

/** Multipath mode */
#define MP_METHOD_FIXED           0
#define MP_METHOD_FILTER          1
#define MP_GROUP_PERCENT_SIZE     3 /**< Group sort: PSR MP 0.3,0.6,0.9m */

/** QC result structure */
typedef struct _QC_TIME_T                         /**< gps time <16 Byte>*/
{
	int64_t Time;                                 /**< gps seconds */
	double Sec;                                   /**< fraction of one second */
}QC_TIME_T;

typedef struct _QC_SNR_T
{
	int SnrNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ]; /**< SNR number */
	float Snr[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];  /**< SNR sum for every frequency */
}QC_SNR_T;

typedef enum _QC_EXCEPTION_HANDLING_E
{
	QC_EXCEPTION_OK = 0,                                         /**< success */

	/** Total/File */
	QC_EXCEPTION_UNSUPPORTED_FORMAT,                             /**< file format is unsupported */
	QC_EXCEPTION_OBS_FILE_OPEN_FAILED,                           /**< input obs file open failed */
	QC_EXCEPTION_NAV_FILE_MISSING,                               /**< dependent navigation file is missing */
	QC_EXCEPTION_PARSE_FAILED,                                   /**< file parse failed */
	QC_EXCEPTION_PARSE_HEADER_FAILED,                            /**< header parse failed */
	QC_EXCEPTION_PARSE_EPOCH_FAILED,                             /**< epoch parse failed */
	QC_EXCEPTION_PARTIAL_SYS_FAILED,                             /**< some systems fail to process */
	QC_EXCEPTION_SPP_ALL_FAILED,                                 /**< spp solution failed totally */
	
	/** Single System */
	QC_EXCEPTION_SYS_NAV_UNMATCHED,                              /**< obs exists but no matched nav injected for system */
	QC_EXCEPTION_SYS_SPP_UNSOLVED,                               /**< obs and nav exist but system is still unsolved */

	QC_EXCEPTION_MAX = 0xFFFFFFFF,
}QC_EXCEPTION_HANDLING_E;

/** QC GNSS EPH Mode */
typedef enum _QC_EPH_MODE_E
{
	QC_EPH_MODE_AUTO = 0,                                         /**< auto find same-dir same-name eph/nav file */
	QC_EPH_MODE_NONE,                                             /**< no ephemeris mode */
	QC_EPH_MODE_SPECIFIED,                                        /**< use specified eph/nav file */

	QC_EPH_MODE_MAX = 0xFFFFFFFF,
} QC_EPH_MODE_E;

typedef struct _QC_EXCEPTION_INFO_T
{
	int Code;                                                    /**< see QC_EXCEPTION_HANDLING_E */
	BOOL IsHaveEph;                                              /**< if have Eph */
	BOOL IsHaveObs;                                              /**< If have Obs */
	BOOL IsAllEpochSPPUnsolved;                                  /**< SPP fail */
}QC_EXCEPTION_INFO_T;

typedef struct _QC_SLIP_TIME_WIN_INFO_T
{
	QC_TIME_T WindowStartTime;                                   /**< time window start */
	
	unsigned int WindowEpochNum;                                 /**< epoch number in current window */
	unsigned int OutLimitEpochNum;                               /**< out-limit epoch number in current window */
	float OutLimitEpochRatio;                                    /**< out-limit epoch ratio in current window */
	
	unsigned short SatSlipNum[QC_MAX_NUM_SAT];                   /**< satellite slip number in window */
	unsigned short SatNum[QC_MAX_NUM_SAT];                       /**< satellite number in window */
	float SatSlipRatio[QC_MAX_NUM_SAT];                          /**< satellite slip number/satellite number */
}QC_SLIP_TIME_WIN_INFO_T;

typedef struct _QC_CN0_TIME_WIN_INFO_T
{
	QC_TIME_T WindowStartTime;                                   /**< time window start */

	float WindowStartAvgCN0;                                     /**< average CN0 at window start time */
	float AvgCN0Drop;                                            /**< average CN0 drop in current window */
}QC_CN0_TIME_WIN_INFO_T;

typedef struct _QC_OBS_BREAK_INFO_T
{
	QC_TIME_T BreakStartTime;                                    /**< observation break start time */
	float BreakDurationSec;                                      /**< observation break duration in seconds */
}QC_OBS_BREAK_INFO_T;

/** QC result structure */
typedef struct _QC_RESULT_T
{
	int IntervNum;                                                        /**< elevation group number */
	int FileDataType[QC_MAX_NUM_OBS_FILE];                                /**< File Data type, see GNSS_STRFMT_XX, default STRFMT_NONE */
	QC_EXCEPTION_INFO_T FileException[QC_MAX_NUM_OBS_FILE];               /**< file level exception info */
	QC_EXCEPTION_INFO_T SysException[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS];/**< system level exception info */
	float DegThreshold[QC_MAXELEVINTERV];			                      /**< SNR group elevation threshold[Deg] */
	QC_SNR_T QcSnr[QC_MAXELEVINTERV];                                     /**< SNR */

	int32_t MaxEpochNum[QC_MAX_NUM_OBS_FILE];                             /**< serial number */
	int32_t PosNum[QC_MAX_NUM_OBS_FILE];                                  /**< pos num */
	QC_TIME_T StartTime[QC_MAX_NUM_OBS_FILE];                             /**< start Time */
	QC_TIME_T EndTime[QC_MAX_NUM_OBS_FILE];                               /**< end Time */
	float Interval[QC_MAX_NUM_OBS_FILE];                                  /**< sample interval(s) */
	int32_t SysSigType[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_MAX_SIG_TYPE];/**< system signal type */
	double PosXYZ[QC_MAX_NUM_OBS_FILE][3];                                /**< station ECEF postion from SPP(average) */

	float MPTotel[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ];      /**< MP statistics for every frequency */

	float SNRDeg[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_MAXELEVINTERV][QC_NUM_FREQ];/**< SNR elevation group */
	float SNR[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ];          /**< deg  ave snr */
	double UseRateSys[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS];               /**< use rate for each sys */
	double UseRateTotal[QC_MAX_NUM_OBS_FILE];                             /**< use rate for total */
	
	double OSR[QC_MAX_NUM_OBS_FILE];                                      /**< sat  ave OSR */
	int SlipNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];        /**< cycle slip number for every satellite */
	int DetectNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];      /**< detect epoch number */
	int SlipFlag[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];       /**< satellite slip flag for every epoch */
	int SysSlipNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS];                  /**< cycle slip number for every system */
	int SysDetectNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS];                /**< detect epoch number for every system */
	
	float AvgHealthyArcDurationSec[QC_MAX_NUM_OBS_FILE];                  /**< average healthy arc duration, unit: second */
	float MaxHealthyArcDurationSec[QC_MAX_NUM_OBS_FILE];                  /**< max healthy arc duration, unit: second */
	unsigned int HealthyArcEpochCountTemp[QC_MAX_NUM_OBS_FILE];           /**< current healthy arc epoch count for each file */
	QC_TIME_T HealthyArcStartTimeTemp[QC_MAX_NUM_OBS_FILE];               /**< current healthy arc start time for each file */
	double HealthyArcDurationSumSec[QC_MAX_NUM_OBS_FILE];                 /**< healthy arc duration sum for average calculation */
	unsigned int HealthyArcCount[QC_MAX_NUM_OBS_FILE];                    /**< healthy arc count for average calculation */
	
	int SlipOutlierWindowCount[QC_MAX_NUM_OBS_FILE];                      /**< valid number of slip outlier windows */
	QC_SLIP_TIME_WIN_INFO_T SlipOutlierWindow[QC_MAX_NUM_OBS_FILE][QC_DIAG_TOP_WIN_NUM];/**< top slip outlier windows */
	int SlipOutlierWindowTempCount[QC_MAX_NUM_OBS_FILE];                  /**< valid number of temp slip outlier windows */
	QC_SLIP_TIME_WIN_INFO_T SlipOutlierWindowTemp[QC_MAX_NUM_OBS_FILE][QC_DIAG_TOP_WIN_NUM];/**< temp slip outlier windows */
	
	/** use rate */
	int SatCompleteNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];              /**< complete freq observations number for each sat, file */
	int CompleteNum[QC_MAX_NUM_OBS_FILE];                                 /**< complete freq observations number for each file */
	int SatPossibleNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];              /**< possible freq observations number for each sat, file */
	int PossibleNum[QC_MAX_NUM_OBS_FILE];                                 /**< possible freq observations number for each file */
	double UseRateSat[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];               /**< use rate for each sat, file */
	
	int ObsBreakCount[QC_MAX_NUM_OBS_FILE];                               /**< observation break count, break duration > 1 minute */
	QC_OBS_BREAK_INFO_T ObsBreakTop[QC_MAX_NUM_OBS_FILE][QC_DIAG_TOP_BREAK_NUM];/**< longest observation break info */

	QC_TIME_T MPTime[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];   /**< Current MP time */
	double MP[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];          /**< the sum in the first and the sum of squares in the second */
	int MPNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];          /**< MP number */
	int MpComb[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];         /**< save MP conbination frequency info */
	double MPWins[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ][QC_WIN_SIZE_MP];  /**< MP windows Mean */
	int MPWinsNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];      /**< MP windows number */
	double CurMP[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_NUM_FREQ];       /**< current MP value */
	unsigned short CurMPValidMask[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];
	
	float MPGroupPercent[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE]; /**< mp ratio lower than 0.3m, 0.6m, 0.9m */
	float MPGroupPercentAve[QC_MAX_NUM_OBS_FILE][QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE]; /**< average mp ratio lower than 0.3m, 0.6m, 0.9m */
	int MPGroupNum[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SYS][QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE]; /**< mp observation count for each group threshold(0.3m, 0.6m, 0.9m) */

	QC_TIME_T IonTime[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];               /**< Ion time */
	double Ion[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];                      /**< Ion value */
	int IonTypeComb[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];                 /**< save Ion conbination frequency info */

	QC_TIME_T GFTime[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_LC_NUM];     /**< GF time */
	double GF[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_LC_NUM];            /**< GF value */
	
	QC_TIME_T MWTime[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_LC_NUM];     /**< MW time */
	double MW[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT][QC_LC_NUM];            /**< MW value */
	
	unsigned char CurGFAndMWValidMask[QC_MAX_NUM_OBS_FILE][QC_MAX_NUM_SAT];
	
	unsigned int SatNum[QC_MAX_NUM_OBS_FILE];                             /**< All satellite number */
	unsigned int LowCN0SatNum[QC_MAX_NUM_OBS_FILE];                       /**< low CN0(< 35dBHz) satellite number */
	unsigned int LowCN0EpochNum[QC_MAX_NUM_OBS_FILE];                     /**< low CN0(< 35dBHz) epoch number */
	float LowCN0SatRatio[QC_MAX_NUM_OBS_FILE];                            /**< low CN0(<35dBHz) satellite ratio */
	float LowCN0EpochRatio[QC_MAX_NUM_OBS_FILE];                          /**< low CN0(<35dBHz) epoch ratio */
	
	int CN0DropWindowCount[QC_MAX_NUM_OBS_FILE];                          /**< valid number of CN0 drop windows */
	QC_CN0_TIME_WIN_INFO_T CN0DropWindow[QC_MAX_NUM_OBS_FILE][QC_DIAG_TOP_WIN_NUM];/**< top CN0 drop windows */
	int CN0DropWindowTempCount[QC_MAX_NUM_OBS_FILE];                      /**< valid number of temp CN0 drop windows */
	QC_CN0_TIME_WIN_INFO_T CN0DropWindowTemp[QC_MAX_NUM_OBS_FILE][QC_DIAG_TOP_WIN_NUM];/**< temp CN0 drop windows */
}QC_RESULT_T;

/** QC GNSS config */
typedef struct _CHCQC_GNSSCFG_T
{
	int Mode;                                             /**< GNSS QC mode(0:zero baseline 1:diff sat and epoch 2:short baseline 3:tripleDiff epoch) */
	int Sys;                                              /**< gnss system choose(GPS:G,SBAS:S,GLO:R,GLA:E,QZS:J,BDS:C,IRNSS:I) */
	int RawFileDataType;                                  /**< File Data type, see GNSS_STRFMT_XX, default STRFMT_NONE */
	unsigned char BanPRN[QC_MAX_NUM_SAT];                 /**< gnss system choose(GPS:G,SBAS:S,GLO:R,GLA:E,QZS:J,BDS:C,IRNSS:I) */
	unsigned int EleDeg;                                  /**< ele mask deg */
	float Interval;                                       /**< file sample interval */
	float SNRGroup[QC_MAXELEVINTERV];                     /**< grouping SNR with ele(0,10,30,40,90) */
	int SNRGroupSize;
	BOOL IsThesholdSNR;                                   /**< whether to verify SNR threshold */
	BOOL IsThesholdMP;                                    /**< whether to verify MP threshold */
	double HtoReflector;                                  /**< height from main reflector */
	QC_TIME_T RTCMTime;                                   /**< When decoding RTCM data, need to specify the time of the day */
	QC_TIME_T StartTime;                                  /**< start time to process */
	QC_TIME_T EndTime;                                    /**< end time to process */
	
	BOOL IsInputTrueXYZ;                                  /**< # [bool] whether to input base and rover XYZ */
	double BaseTrueXYZ[3];                                /**< [m] base ecef coordinate */
	double RoverTrueXYZ[3];                               /**< [m] rover ecef coordinate */

	BOOL bIsOutputDataList;                               /**< # [bool] whether to output DataList file */
	BOOL bIsOutputQCLog;                                  /**< # [bool] whether to output .CS/.MP file */
	BOOL bIsOutputQCResBinaryList;                        /**< # [bool] whether to output _QCResList.txt file */
	
	int EphMode;                                          /**< ephemeris mode: auto/no-eph/specified, see QC_EPH_MODE_xx */

	/** # QC threshold */
	float GMpMAX[QC_GPS_FRE_NUM];                         /**< [m]max mp Rms pseudorange for GPS(L1,L2,L5) */
	float RMpMAX[QC_GLONASS_FRE_NUM];                     /**< [m]max mp Rms pseudorange for GLONASS(G1, G1a, G2, G2a, G3)       */
	float EMpMAX[QC_GALILEO_FRE_NUM];                     /**< [m]max mp Rms pseudorange for Galileo(E1, E5A, E5B, E5(A + B), E6)*/
	float CMpMAX[QC_BDS_FRE_NUM];                         /**< [m]max mp Rms pseudorange for BDS(B1,B2,B3,B1C,B2a,B1A,B3A,B2b,B2a+B2b) */
	float JMpMAX[QC_QZSS_FRE_NUM];                        /**< [m]max mp Rms pseudorange for QZSS(L1,L2,L5,L5S,L6)*/
	float SMpMAX[QC_SBAS_FRE_NUM];			              /**< [m]max mp Rms pseudorange for SBAS(L1,L5)*/
	float IMpMAX[QC_IRNSS_FRE_NUM];		                  /**< [m]max mp Rms pseudorange for IRNSS(L5,S)*/
	float LMpMAX[QC_XW1_FRE_NUM];		                  /**< [m]max mp Rms pseudorange for XW1 LEO(L1L,L1H,L2L,L2H)*/
	float MPCarrier;                                      /**< [m]max mp Rms carrier */

	unsigned int GSnrMIN[QC_GPS_FRE_NUM];                 /**< [dBHz] min snr for GPS(L1,L2,L5,L1C/A,L2C/A) */
	unsigned int RSnrMIN[QC_GLONASS_FRE_NUM];             /**< [dBHz] min snr for GLO(G1,G1a,G2,G2a,G3) */
	unsigned int ESnrMIN[QC_GALILEO_FRE_NUM];             /**< [dBHz] min snr for GAL(E1,E5A,E5B,E5(A+B),E6) */
	unsigned int CSnrMIN[QC_BDS_FRE_NUM];                 /**< [dBHz] min snr for BDS(B1,B2,B3,B1C,B2a,B1A,B3A,B2b,B2a+B2b) */
	unsigned int JSnrMIN[QC_QZSS_FRE_NUM];                /**< [dBHz] min snr for QZSS(L1,L2,L5,L5S,L6) */
	unsigned int SSnrMIN[QC_SBAS_FRE_NUM];				  /**< [dBHz] min snr for SBAS(L1,L5)		   */
	unsigned int ISnrMIN[QC_IRNSS_FRE_NUM];				  /**< [dBHz] min snr for IRNSS(L5,S)		   */
	unsigned int LSnrMIN[QC_XW1_FRE_NUM];				  /**< [dBHz] min snr for XW1 LEO(L1L,L1H,L2L,L2H)		   */
	float UseRate;								          /**< [%] data useRate					   */
	unsigned int CJR;									  /**< [int] cycle jump ratio				   */
	/** # SNR min Number of satellite */
	unsigned int NumGSnrMIN[QC_GPS_FRE_NUM];              /**< [int] min GPS SNR sat number(L1,L2,L5)                              */
	unsigned int NumRSnrMIN[QC_GLONASS_FRE_NUM];		  /**< [int] min GLONASS SNR sat number(G1,G1a,G2,G2a,G3)				   */
	unsigned int NumESnrMIN[QC_GALILEO_FRE_NUM];		  /**< [int] min Galileo SNR sat number(E1,E5A,E5B,E5(A+B),E6)			   */
	unsigned int NumCSnrMIN[QC_BDS_FRE_NUM];			  /**< [int] min BDS SNR sat number(B1,B2,B3,B1C,B2a,B1A,B3A,B2b,B2a+B2b)  */
	unsigned int NumJSnrMIN[QC_QZSS_FRE_NUM];			  /**< [int] min QZSS SNR sat number(L1,L2,L5,L5S,L6)					   */
	unsigned int NumSSnrMIN[QC_SBAS_FRE_NUM];			  /**< [int] min SBAS SNR sat number(L1,L5)								   */
	unsigned int NumISnrMIN[QC_IRNSS_FRE_NUM];			  /**< [int] min IRNSS SNR sat number(L5,S)								   */
	unsigned int NumLSnrMIN[QC_XW1_FRE_NUM];			  /**< [int] min XW1 LEO SNR sat number(L1L,L1H,L2L,L2H)								   */
	/** # file settings */
	char OutDir[MAX_FILE_NAME_LENGTH];                    /**< output file dir */
	char RoverFile[MAX_FILE_NAME_LENGTH];                 /**< rover obs file name and path */
	char BaseFile[MAX_FILE_NAME_LENGTH];                  /**< base obs file name and path */
	char RinexEphFile[MAX_FILE_NAME_LENGTH];              /**< specified RINEX eph/nav file path */
	/* # freq_Option form user */
	BOOL AllFreq;                                         /**< gnss observation all frequency (0:from user 1:all freq) */
	int SysSigType[QC_MAX_NUM_SYS][QC_MAXSIGTYPE];
	int NumSigType[QC_MAX_NUM_SYS];                       /**< number of signal types, allFreq != 1 */
	BOOL bIsDoubleDiffCheckLLI;                           /**< whether double difference between station and satellite takes into LLI, default FALSE */
	/** multipath option */
	unsigned int MPMethod;                                /**< 0: use 50-epoch; 1: filter */
}CHCQC_GNSSCFG_T;

/** RinexCfg */
typedef struct _CHCQC_RINEXCFG_T
{
	unsigned int DataType;                     /**< data type */
	unsigned int OutputType;                   /**< data output type */
	unsigned int RinexVer;                     /**< output rinex file version */
	float Interval;                            /**< output rinex file interval(0:from raw file,x>0:from user) */
	unsigned char Sys[MAX_SYS_STR_LENGTH];     /**< gnss system choose(GPS:G,SBAS:S,GLO:R,GLA:E,QZS:J,BDS:C,IRNSS:I) */
	unsigned int Split;                        /**< obsfile split option(0:no,1:station moving,2:time interval,3:time period) */
	unsigned int SplitTimeInt;                 /**< time interval to use when splitting files */
	QC_TIME_T SplitStart;                      /**< start time when splitting files */
	QC_TIME_T SplitEnd;                        /**< end time when splitting files */
	QC_TIME_T RTCMTime;                        /**< rtcm time(null:system time) */
	BOOL CheakSeq;                             /**< cheak the sequence of obs file(0:no,1:yes) */
	unsigned int LeapSecond;                   /**< set leap second(0:from eph,>0:set by user) */
	BOOL AllFreq;                              /**< gnss observation all frequency (0:from user 1:all freq) */
	unsigned char ObsType[MAX_OBS_STR_LENGTH]; /**< obs type choose(C:pseudorange,L:carrier,D:Doppler,S:SNR) */
	unsigned char GPSFreq[MAX_FRE_STR_LENGTH]; /**< GPS freq option from user */
	unsigned char SBAFreq[MAX_FRE_STR_LENGTH]; /**< SBAS freq option from user */
	unsigned char GLOFreq[MAX_FRE_STR_LENGTH]; /**< GLONASS freq option from user */
	unsigned char GALFreq[MAX_FRE_STR_LENGTH]; /**< Galileo freq option from user */
	unsigned char QZSFreq[MAX_FRE_STR_LENGTH]; /**< QZSS freq option from user */
	unsigned char BDSFreq[MAX_FRE_STR_LENGTH]; /**< BDS freq option from user */
	unsigned char NICFreq[MAX_FRE_STR_LENGTH]; /**< IRNASS freq option from user */
	BOOL Pause;                                /**< Whether to pause at the end(0:no,1:yes) */
	char RawFile[MAX_FILE_NAME_LENGTH];        /**< gnss raw file name */
	char OutDir[MAX_FILE_NAME_LENGTH];         /**< rinex output file dir */
}CHCQC_RINEXCFG_T;

typedef CHCQC_GNSSCFG_T QC_OPT_T;

/** Msg decoder structure */
typedef struct _QC_OBSD_T				/* observation data record */
{
	unsigned short Sat;			        /**< satellite/receiver number */
	unsigned short PRN;
	unsigned char Sys;
	unsigned char NumFre;
	int GLOFreq;                        /**< GLONASS frq */

	double SNR[QC_NUM_FREQ];		    /**< signal strength (dbHz) */
	unsigned char LLI[QC_NUM_FREQ];		/**< loss of lock indicator */
	unsigned char Code[QC_NUM_FREQ];    /**< code indicator (CODE_???) */
	unsigned char Visibility;           /**< 1:invisible, 0:visible */
	double L[QC_NUM_FREQ];				/**< observation data carrier-phase (cycle) */
	double P[QC_NUM_FREQ];				/**< observation data pseudorange (m) */
	float  D[QC_NUM_FREQ];				/**< observation data doppler frequency (Hz) */
	double Azel[2];                     /**< azimuth and elevation(rad) */
	double R;                           /**< distance between satellite and station(m) */
	double Dts[2];
	double E[3];                        /**< directional cosine of rover->satellite */
	double SatVelXYZ[3];                /**< Satellite velocity vector m/s */
} QC_OBSD_T;


typedef struct _QC_OBS_T 					/**< observation data */
{
	QC_TIME_T Time;					        /**< receiver sampling time (gpst) */
	int ObsNum, ObsNumMax; 					/**< number of obervation data/allocated */
	double StaPos[3];                       /**< station postion(ECEF-XYZ) */
	QC_OBSD_T Data[QC_MAX_NUM_OBS];			/**< observation data records */
} QC_OBS_T;

typedef struct _QC_SOL_INFO_T 				/**< Epoch SPP solution info */
{
	QC_TIME_T Time;					        /**< SPP solution time (gpst) */
	unsigned int UsedSatNum;                /**< sat number involved in position */
	float Dop[5]; 					        /**< dop[gdop, pdop, hdop, vdop, tdop] */
} QC_SOL_INFO_T;

typedef struct _QC_RINEX_HEADER_INFO_T
{
	char Recfacturer[QC_MAX_MSG_LEN];       /**< name of agency */
	char RecType[QC_MAX_MSG_LEN];           /**< reciever type */
	char RecVer[QC_MAX_MSG_LEN];            /**< reciever version */
	char RecSN[QC_MAX_MSG_LEN];             /**< reciever number */
	char AntType[QC_MAX_MSG_LEN];           /**< antena type */
	char AntSN[QC_MAX_MSG_LEN];             /**< antena number */
	char AntManufacturer[QC_MAX_MSG_LEN];   /**< antena Manufacturer */
	double AntDeltaH;                       /**< antena delta H */
} QC_RINEX_HEADER_T;

typedef struct _CHCQC_READER_T
{
	int IterNo;
	QC_TIME_T LastTime[QC_MAX_NUM_OBS_FILE];
	QC_TIME_T TargetTime;
	int iEpoch[QC_MAX_NUM_OBS_FILE];                              /**< for one station function */
	float Interval;

	FILE* fpOutFileCS[QC_MAX_NUM_OBS_FILE];
	FILE* fpOutFileMP[QC_MAX_NUM_OBS_FILE];                        /**< MP file */
	FILE* fpOutFileDataList[QC_MAX_NUM_OBS_FILE];                  /**< Observation Data List file */

	unsigned long long RawFileSize[QC_MAX_NUM_OBS_FILE];          /**< Raw file size */
	long CurFilePos[QC_MAX_NUM_OBS_FILE];                         /**< Current file pointer position */

	QC_RESULT_T QcRes;

	QC_OBS_T pObs[QC_MAX_NUM_OBS_FILE][4];                        /**< continuous storage of observations of four epochs */
	QC_SOL_INFO_T EpochSolInfo;                                   /**< Epoch SPP solution info */

	QC_OPT_T* pOpt;                                               /**< QC option */

	void* pCodecInfo[MAX_NUM_INPUT_FILE];                         /**< sizeof(QC_CODEC_INFO_T)*QC_MAX_NUM_INPUT_FILE */
												                  /**< 0:rover obs, 1:base obs,2~6:rover nav,7~11:base nav */
	unsigned int AntID[MAX_NUM_INPUT_FILE];
	QC_RINEX_HEADER_T RinexObsHeaderInfo;
}CHCQC_READER_T;

/**************************memory interface type********************************/
typedef void* (*VERIQC_MALLOC_FUNC)(size_t Size);
typedef void* (*VERIQC_REALLOC_FUNC)(void* pTr, size_t Size);
typedef void* (*VERIQC_CALLOC_FUNC)(size_t Count, size_t Size);
typedef void(*VERIQC_FREE_FUNC)(void* pTr);

/**************allocator**************/
typedef struct _VERIQC_ALLOCATOR_T {
	VERIQC_MALLOC_FUNC    LocalMalloc;   /**< malloc funcs  */
	VERIQC_REALLOC_FUNC   LocalRealloc;  /**< realloc funcs */
	VERIQC_CALLOC_FUNC    LocalCalloc;   /**< calloc funcs  */
	VERIQC_FREE_FUNC      LocalFree;     /**< free   funcs  */
} VERIQC_ALLOCATOR_T;

/** function */

#ifdef WIN32
/** common function */
_declspec(dllexport) int PrintfQC(const char* pFormat, ...);
_declspec(dllexport) int GetQCVersion(void);
_declspec(dllexport) char* GetQCVersionStr(void);
_declspec(dllexport) BOOL SetExePath(char* pExePath);
_declspec(dllexport) BOOL MkDirs(char* pPath);
_declspec(dllexport) VERIQC_ALLOCATOR_T* VeriQCGetAllocator(void);

_declspec(dllexport) int QCTime2Epoch(QC_TIME_T Time, double* pEpoch);
_declspec(dllexport) QC_TIME_T Epoch2QCTime(const double* pEpoch);

_declspec(dllexport) int32_t GetQCSatNo(int32_t Sys, int32_t PRN);
_declspec(dllexport) int32_t GetQCSatSys(int32_t Sat, int32_t* pPRN);

/** QC GNSS config */
_declspec(dllexport) void InitGNSSCfg(CHCQC_GNSSCFG_T* pGNSSCfg);
_declspec(dllexport) BOOL ReadGNSSCfg(FILE* fpCfgFile, CHCQC_GNSSCFG_T* pGNSSCfg);


/** interface: GNSS QC function */
_declspec(dllexport) void GNSSProcesser(CHCQC_GNSSCFG_T* pGNSSCfg, QC_RESULT_T* pQCRes);
_declspec(dllexport) void GNSSPreProcess(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader);
_declspec(dllexport) double GetGNSSPreProcessProgress();

/** interface: Rixne function */
_declspec(dllexport) void InitRinexCfg(CHCQC_RINEXCFG_T* pRinexCfg);
_declspec(dllexport) BOOL ReadRinexCfg(FILE* fpCfgFile, CHCQC_RINEXCFG_T* pRinexCfg);
_declspec(dllexport) void RinexProcesser(CHCQC_RINEXCFG_T* pRinexCfg);
_declspec(dllexport) int GetRinexProgress();

/** interface: Quality Check */
_declspec(dllexport) int InitQCReader(CHCQC_READER_T* pQCReader);
_declspec(dllexport) void FreeQCReader(CHCQC_READER_T* pQCReader);
_declspec(dllexport) int8_t QCProcessCore(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader, uint8_t ID);
_declspec(dllexport) BOOL SyncGNSSQCState(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader);/**< Synchronization Configuration */
_declspec(dllexport) void UpdateQCOpt(QC_OPT_T* pOpt, QC_RESULT_T* pQCRes);
_declspec(dllexport) void GetQCAvePos(QC_RESULT_T* pQCRes);
_declspec(dllexport) void UpdateQCRes(QC_RESULT_T* pQCRes, QC_OPT_T* pOpt);

_declspec(dllexport) void WriteQCStatisticsHtml(QC_RESULT_T* pRes, QC_OPT_T* pOpt);


/** interface: RTK function */
_declspec(dllexport) void InitRTKCore(double EleMinDeg);
_declspec(dllexport) void EndRTKCore();
#elif JAVA

#else
/** common function */
int PrintfQC(const char* pFormat, ...);
int GetQCVersion(void);
char* GetQCVersionStr(void);
BOOL SetExePath(char* pExePath);
BOOL MkDirs(char* pPath);
VERIQC_ALLOCATOR_T* VeriQCGetAllocator(void);

int QCTime2Epoch(QC_TIME_T Time, double* pEpoch);
QC_TIME_T Epoch2QCTime(const double* pEpoch);

int32_t GetQCSatNo(int32_t Sys, int32_t PRN);
int32_t GetQCSatSys(int32_t Sat, int32_t* pPRN);

/** QC GNSS config */
void InitGNSSCfg(CHCQC_GNSSCFG_T* pGNSSCfg);
BOOL ReadGNSSCfg(FILE* fpCfgFile, CHCQC_GNSSCFG_T* pGNSSCfg);


/** interface: GNSS QC function */
void GNSSProcesser(CHCQC_GNSSCFG_T* pGNSSCfg, QC_RESULT_T* pQCRes);
void GNSSPreProcess(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader);
double GetGNSSPreProcessProgress();

/** interface: Rixne function */
void InitRinexCfg(CHCQC_RINEXCFG_T* pRinexCfg);
BOOL ReadRinexCfg(FILE* fpCfgFile, CHCQC_RINEXCFG_T* pRinexCfg);
void RinexProcesser(CHCQC_RINEXCFG_T* pRinexCfg);
int GetRinexProgress();

/** interface: Quality Check */
int InitQCReader(CHCQC_READER_T* pQCReader);
void FreeQCReader(CHCQC_READER_T* pQCReader);
int8_t QCProcessCore(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader, uint8_t ID);
BOOL SyncGNSSQCState(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader);/**< Synchronization Configuration */
void UpdateQCOpt(QC_OPT_T* pOpt, QC_RESULT_T* pQCRes);
void GetQCAvePos(QC_RESULT_T* pQCRes);
void UpdateQCRes(QC_RESULT_T* pQCRes, QC_OPT_T* pOpt);

void WriteQCStatisticsHtml(QC_RESULT_T* pRes, QC_OPT_T* pOpt);


/** interface: RTK function */
void InitRTKCore(double EleMinDeg);
void EndRTKCore();

#endif

#define VERIQC_MALLOC   VeriQCGetAllocator()->LocalMalloc
#define VERIQC_FREE     VeriQCGetAllocator()->LocalFree
#define VERIQC_CALLOC   VeriQCGetAllocator()->LocalCalloc
#define VERIQC_REALLOC  VeriQCGetAllocator()->LocalRealloc

#ifdef __cplusplus
}
#endif

#endif/**< _VERI_QC_H_ */
