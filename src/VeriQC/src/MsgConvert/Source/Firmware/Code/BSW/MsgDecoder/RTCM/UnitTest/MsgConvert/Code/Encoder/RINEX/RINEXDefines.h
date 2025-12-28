#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RINEX defines
*-
@file   RINEXDefines.h
@author CHC
@date   2023/04/03
@brief

**************************************************************************/
#ifndef _RINEX_DEFINES_H_
#define _RINEX_DEFINES_H_

#include <stdint.h>
#include "Common/GNSSSignal.h"
#include "TimeDefines.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"

#define RINEX_SAT_NUM_MAX                  128

#define RINEX_HEAD_ONETHIRD_LENGTH         21
#define RINEX_HEAD_HALF_LENGTH             31
#define RINEX_HEAD_TWOTHIRDS_LENGTH        41
#define RINEX_HEAD_LENGTH                  61

#define NORMAL_COORDINATE_VECTOR           3

#define MAXCOMMENT                         100  /**< max number of RINEX comments */

#define SUBFRM_LEN                         150
#define MAX_MSG_LEN                        128

#define MAX_NUM_OBS_TYPE                    64

#define RINEX_ID_OBS                    101     /**< obs data id */
#define RINEX_ID_GPSEPH                 102     /**< GPS eph data id */
#define RINEX_ID_GLOEPH                 103     /**< GLO eph data id */
#define RINEX_ID_QZSSEPH                104     /**< QZS eph data id */
#define RINEX_ID_GALEPH                 105     /**< GAL eph data id */
#define RINEX_ID_BDSEPH                 106     /**< BDS eph data id */
#define RINEX_ID_SBASEPH                107     /**< SBS eph data id */
#define RINEX_ID_GALEPH_FNAV            108     /**< GAL FNAV eph data id */
#define RINEX_ID_GALEPH_INAV            109     /**< GAL INAV eph data id */
#define RINEX_ID_GALEPH_RCED            110     /**< GAL RCED eph data id */
#define RINEX_ID_NICEPH                 111     /**< NAVIC eph data id */
#define RINEX_ID_BDSEPH_CNAV1           112     /**< BDS CNAV1 eph data id */
#define RINEX_ID_BDSEPH_CNAV2           113     /**< BDS CNAV2 eph data id */
#define RINEX_ID_BDSEPH_CNAV3           114     /**< BDS CNAV2 eph data id */
#define RINEX_ID_METE                   115     /**< mete data id */
#define RINEX_ID_MUTIEPH                116     /**< MUTI eph data id */

#define RINEX_ID_OBS_CRX_HEADER         151     /**< obs data crx file header used while endode */
#define RINEX_ID_OBS_HEADER             152     /**< obs data file header while endode */
#define RINEX_ID_MUTIEPH_HEADER         153     /**< muti system in one file(rinex nav type "M") */
#define RINEX_ID_GPSEPH_HEADER          154     /**< GPS eph data file header */
#define RINEX_ID_GLOEPH_HEADER          155     /**< GLO eph data file header */
#define RINEX_ID_GALEPH_HEADER          156     /**< GAL eph data file header */
#define RINEX_ID_BDSEPH_HEADER          157     /**< BDS eph data file header */
#define RINEX_ID_QZSSEPH_HEADER         158     /**< QZS eph data file header */
#define RINEX_ID_SBASEPH_HEADER         159     /**< SBS eph data file header */
#define RINEX_ID_NICEPH_HEADER          160     /**< NAVIC eph data file header */
#define RINEX_ID_METE_HEADER            161     /**< mete data file header */

/* code type defined */
#define OBS_CODE_NONE   0                   /* obs code: none or unknown */
#define OBS_CODE_L1C    1                   /* obs code: L1C/A,G1C/A,E1C (GPS,GLO,GAL,QZS,SBS) */
#define OBS_CODE_L1P    2                   /* obs code: L1P,G1P    (GPS,GLO) */
#define OBS_CODE_L1W    3                   /* obs code: L1 Z-track (GPS) */
#define OBS_CODE_L1Y    4                   /* obs code: L1Y        (GPS) */
#define OBS_CODE_L1M    5                   /* obs code: L1M        (GPS) */
#define OBS_CODE_L1N    6                   /* obs code: L1codeless (GPS) */
#define OBS_CODE_L1S    7                   /* obs code: L1C(D)     (GPS,QZS) */
#define OBS_CODE_L1L    8                   /* obs code: L1C(P)     (GPS,QZS) */
#define OBS_CODE_L1E    9                   /* obs code: L1-SAIF    (QZS) */
#define OBS_CODE_L1A    10                  /* obs code: E1A        (GAL) */
#define OBS_CODE_L1B    11                  /* obs code: E1B        (GAL) */
#define OBS_CODE_L1X    12                  /* obs code: E1B+C,L1C(D+P) (GAL,QZS) */
#define OBS_CODE_L1Z    13                  /* obs code: E1A+B+C,L1SAIF (GAL,QZS) */
#define OBS_CODE_L2C    14                  /* obs code: L2C/A,G1C/A (GPS,GLO) */
#define OBS_CODE_L2D    15                  /* obs code: L2 L1C/A-(P2-P1) (GPS) */
#define OBS_CODE_L2S    16                  /* obs code: L2C(M)     (GPS,QZS) */
#define OBS_CODE_L2L    17                  /* obs code: L2C(L)     (GPS,QZS) */
#define OBS_CODE_L2X    18                  /* obs code: L2C(M+L),B1I+Q (GPS,QZS,CMP) */
#define OBS_CODE_L2P    19                  /* obs code: L2P,G2P    (GPS,GLO) */
#define OBS_CODE_L2W    20                  /* obs code: L2 Z-track (GPS) */
#define OBS_CODE_L2Y    21                  /* obs code: L2Y        (GPS) */
#define OBS_CODE_L2M    22                  /* obs code: L2M        (GPS) */
#define OBS_CODE_L2N    23                  /* obs code: L2codeless (GPS) */
#define OBS_CODE_L5I    24                  /* obs code: L5/E5aI    (GPS,GAL,QZS,SBS) */
#define OBS_CODE_L5Q    25                  /* obs code: L5/E5aQ    (GPS,GAL,QZS,SBS) */
#define OBS_CODE_L5X    26                  /* obs code: L5/E5aI+Q  (GPS,GAL,QZS,SBS) */
#define OBS_CODE_L7I    27                  /* obs code: E5bI,B2I   (GAL,CMP) */
#define OBS_CODE_L7Q    28                  /* obs code: E5bQ,B2Q   (GAL,CMP) */
#define OBS_CODE_L7X    29                  /* obs code: E5bI+Q,B2I+Q (GAL,CMP) */
#define OBS_CODE_L6A    30                  /* obs code: E6A        (GAL) */
#define OBS_CODE_L6B    31                  /* obs code: E6B        (GAL) */
#define OBS_CODE_L6C    32                  /* obs code: E6C        (GAL) */
#define OBS_CODE_L6X    33                  /* obs code: E6B+C,LEXS+L,B3I+Q (GAL,QZS,CMP) */
#define OBS_CODE_L6Z    34                  /* obs code: E6A+B+C/L6 (GAL,QZS-Block II) */
#define OBS_CODE_L6S    35                  /* obs code: LEXS       (QZS) */
#define OBS_CODE_L6L    36                  /* obs code: LEXL       (QZS) */
#define OBS_CODE_L8I    37                  /* obs code: E5(a+b)I   (GAL) */
#define OBS_CODE_L8Q    38                  /* obs code: E5(a+b)Q   (GAL) */
#define OBS_CODE_L8X    39                  /* obs code: E5(a+b)I+Q (GAL), BD3-B2a+b */
#define OBS_CODE_L2I    40                  /* obs code: B1I        (CMP) */
#define OBS_CODE_L2Q    41                  /* obs code: B1Q        (CMP) */
#define OBS_CODE_L6I    42                  /* obs code: B3I        (CMP) */
#define OBS_CODE_L6Q    43                  /* obs code: B3Q        (CMP) */
#define OBS_CODE_L3I    44                  /* obs code: G3I        (GLO) */
#define OBS_CODE_L3Q    45                  /* obs code: G3Q        (GLO) */
#define OBS_CODE_L3X    46                  /* obs code: G3I+Q      (GLO) */
	//#define OBS_CODE_L7C    47                  /*  B2*/
#define OBS_CODE_L1I    47                  /* obs code: B1I        (BDS) */
#define OBS_CODE_L1Q    48                  /* obs code: B1Q        (BDS) */
#define OBS_CODE_L5D    49                  /* obs code: L5D        (QZS-Block II) */
#define OBS_CODE_L5P    50                  /* obs code: L5P        (QZS-Block II) */
#define OBS_CODE_L5Z    51                  /* obs code: L5Z        (QZS-Block II) */
#define OBS_CODE_L6E    52                  /* obs code: L6E        (QZS-Block II) */
#define OBS_CODE_L7D    53                  /* obs code: B2b        (BDS3-B2b) */
#define OBS_CODE_L7P    54                  /* obs code: B2b        (BDS3-B2b) */
#define OBS_CODE_L7Z    55                  /* obs code: B2b        (BDS3-B2b) */
#define OBS_CODE_L1D    56                  /* obs code: B1Q        (BDS3-B1) */
#define OBS_CODE_L8D    57                  /* obs code: B2a+b      (BDS3-B2) */
#define OBS_CODE_L8P    58                  /* obs code: B2a+b      (BDS3-B2) */

#define OBS_CODE_L6D    59                  /* obs code: B3A   D    (BDS3) */
#define OBS_CODE_L6P    60                  /* obs code: B3A   P    (BDS3) */

#define OBS_CODE_L5A    61                  /* obs code: L5 A SPS  (IRNSS) */
#define OBS_CODE_L5B    62                  /* obs code: L5 B RS(D)(IRNSS) */
#define OBS_CODE_L5C    63                  /* obs code: L5 C RS(P)(IRNSS) */
#define OBS_CODE_L9A    64                  /* obs code: S A SPS   (IRNSS) */
#define OBS_CODE_L9B    65                  /* obs code: S B RS(D) (IRNSS) */
#define OBS_CODE_L9C    66                  /* obs code: S C RS(P) (IRNSS) */
#define OBS_CODE_L9X    67                  /* obs code: S B+C     (IRNSS) */

#define OBS_CODE_L4A    68                  /* obs code: GLOL1OCD*/
#define OBS_CODE_L4B    69                  /* obs code: GLOL1OCP */
#define OBS_CODE_L4X    70                  /* obs code: GLOL1OCX */

typedef struct _PER_SIGNAL_PHASE_SHIFT_INFO_T
{
	unsigned int SatNum;
	double PhaseShiftData;
	int SatIDMap[100];
} PER_SIGNAL_PHASE_SHIFT_INFO_T;

typedef struct _PER_SYS_PHASE_SHIFT_INFO_T
{
	PER_SIGNAL_PHASE_SHIFT_INFO_T PerSignalPhaseShiftInfo[32];
} PER_SYS_PHASE_SHIFT_INFO_T;

typedef struct _RINEX_PHASE_SHIFT_INFO_T
{
	PER_SYS_PHASE_SHIFT_INFO_T PerSysPhaseShiftInfo[TOTAL_EX_GNSS_SYSTEM_NUM];
} RINEX_PHASE_SHIFT_INFO_T;

/** GNSS_DATA_ID_OBS_HEADER: rinex header encode obs */
typedef struct _RINEX_OBS_HEADER_T          /**< encode:rinex obs header */
{
	int Ver;                            /**< rinex version */
	int NCom;                           /**< number of comments */
	char Comments[100][RINEX_HEAD_LENGTH]; /**< comment lines*/
	UTC_TIME_T TimeStart, TimeEnd;      /**<  time start/end */
	double TimeInt;                     /**<  time interval (s) */
	int NavSys;                         /**<  navigation system */
	char Pgm[MAX_MSG_LEN];              /**< name of program creating current file */
	char Runby[MAX_MSG_LEN];            /**< name of agency */
	char MarkerName[MAX_MSG_LEN];       /**< name of antena maker */
	char MarkerNo[MAX_MSG_LEN];         /**< number of antena maker */
	char MarkerType[MAX_MSG_LEN];       /**< type of antena maker */
	char Observer[MAX_MSG_LEN];         /**< name of observer */
	char Agency[MAX_MSG_LEN];           /**< name of agency */
	char AntDes[MAX_MSG_LEN];           /**< antena type */
	char AntNo[MAX_MSG_LEN];            /**< antena number */
	char RecType[MAX_MSG_LEN];          /**< reciever type */
	char RecVer[MAX_MSG_LEN];           /**< reciever version */
	char RecNo[MAX_MSG_LEN];                /**< reciever number */
	double Pos[NORMAL_COORDINATE_VECTOR];   /**< approx positon(x/y/z) */
	double Del[NORMAL_COORDINATE_VECTOR];   /**< antena delta(H/E/N) */
	double UserPos[NORMAL_COORDINATE_VECTOR];   /**< approx positon(x/y/z) */
	double UserDel[NORMAL_COORDINATE_VECTOR];   /**< antena delta(H/E/N) */
	int UserPosMode; /**< 0:disable, 1:enable */
	int UserDelMode; /**< 0:disable, 1:enable */
	int Leapsecs;                      /**< leapsecs */
	int ObsTypeCount[TOTAL_EX_GNSS_SYSTEM_NUM]; /**< rinex obs types count */
	char ObsType[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_NUM_OBS_TYPE][4]; /**< rinex obs types */
	INT64 TimeDiff;						/**< time end and time start difference (unit:Millisecond) */
	RINEX_PHASE_SHIFT_INFO_T RinexPhaseShiftInfo; /**< Phase shift information */
}RINEX_OBS_HEADER_T;

/** GNSS_DATA_ID_(X)EPH_HEADER: rinex header eph */
typedef struct _RINEX_EPH_HEADER_T    /**< encode/decode:rinex eph header */
{
	char Pgm[MAX_MSG_LEN];                  /**< name of program creating current file */
	char Runby[MAX_MSG_LEN];                /**< name of agency */
	int Ver;                                /**< rinex version */
	int NCom;                               /**< number of comments */
	char Comments[MAXCOMMENT][RINEX_HEAD_LENGTH];     /**< comment lines */
	double UTC[TOTAL_EX_GNSS_SYSTEM_NUM][9];             /**< utc of every sat of every sys */
	double Ion[TOTAL_EX_GNSS_SYSTEM_NUM][8];             /**< ion of every sat of every sys */
	int Leapsecs;                            /**< leap sec */
	int Sys;                                /**< gnss system */
} RINEX_EPH_HEADER_T;

/** GNSS_DATA_ID_GEPH_HEADER: rinex header glonass eph */
typedef struct _RINEX_GEPH_HEADER_T       /**< encode/decode:rinex glonass eph header */
{
	int Ver;                                /**< rinex version */
	int NCom;                               /**< number of comments */
	int Leapsecs;                           /**< leap sec */
	char Pgm[MAX_MSG_LEN];                  /**< name of program creating current file */
	char Runby[MAX_MSG_LEN];                /**< name of agency */
	char Comments[MAXCOMMENT][RINEX_HEAD_LENGTH];     /**< comment lines */
} RINEX_GEPH_HEADER_T;

/** GNSS_DATA_ID_METE_HEADER: rinex header mete */
typedef struct _RINEX_METE_HEADER_T /**< encode:rinex mete file output option */
{
	int Ver;                                        /**< RINEX version */
	char Pgm[MAX_MSG_LEN];                          /**< name of program creating current file */
	char Runby[MAX_MSG_LEN];                        /**< name of agency */
	char Marker[MAX_MSG_LEN];                       /**< station name */
	char MarkerNo[MAX_MSG_LEN];                     /**< station number */
	char(*SensorInfo)[RINEX_HEAD_ONETHIRD_LENGTH];  /**< description of the met sensor */
	double Pos[NORMAL_COORDINATE_VECTOR];           /**< approximate position of the met sesor-Geocentric coordinates X,Y,Z(ITRF OR WGS84) */
	double H;                                       /**< Ellipsoidal height */
} RINEX_METE_HEADER_T;

typedef struct _RINEX_OBS_OPT_T /**< encode:rinex obs file output option */
{
	UTC_TIME_T TimeStart, TimeEnd;                      /**< time start/end */
	int Ver;                                            /**< RINEX version */
	int NavSys;                                         /**< navigation system */
	int ObsType;                                        /**< observation type */
	int FreqType;                                       /**< frequency type */
	int NObs[TOTAL_EX_GNSS_SYSTEM_NUM];                              /**< number of obs types [GNSS_MAX_NUM_SYS] */
	int Leaps;                                          /**< whether output leapsecs */
	int CrxFlag;                                        /**< for embedded group */
	int Sample;                                         /**< set obs interval */
	unsigned char ExSats[TOTAL_EX_GNSS_SYSTEM_NUM];                  /**< excluded satellites */
	char ObsTypeBuf[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_NUM_OBS_TYPE][4];      /**< obs types [GNSS_MAX_NUM_SYS][GNSS_MAX_NUM_OBS_TYPE][4] */
	char Mask[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_NUM_OBS_TYPE];         /**< code mask */
} RINEX_OBS_OPT_T;

typedef struct _RINEX_EPH_OPT_T /**< encode:rinex peh file output option */
{
	UTC_TIME_T TimeStart, TimeEnd;  /**< time start/end */
	int Ver;                    /**< RINEX version */
	int NavSys;                 /**< navigation system */
	int Leapsecs;               /**< whether output leapsecs */
	int Iono;                   /**< whether output iono */
	int Time;                   /**< whether output time */
	unsigned char ExSats[MAX_SAT_NUM];  /**< excluded satellites */
} RINEX_EPH_OPT_T;

#endif  /** _RINEX_ENCODER_H_ */

#endif
