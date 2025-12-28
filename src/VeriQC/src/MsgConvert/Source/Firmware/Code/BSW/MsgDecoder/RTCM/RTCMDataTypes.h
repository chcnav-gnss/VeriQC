/**********************************************************************//**
		VeriQC

		RTCM Encoder Module
*-
@file   RTCMDataTypes.h
@author CHC
@date   2023/05/23
@brief

**************************************************************************/
#ifndef _RTCM_DATA_TYPES_H_
#define _RTCM_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "RTCMDefines.h"
#include "../MsgDecodeDataTypes.h"
#include "TimeDefines.h"
#include "Common/GNSSNavDataType.h"

#include "../../MsgEncoder/RawMsg/RawMsgRangeDefines.h"


typedef enum _RTCM_SYSTEM_E
{
	RTCM_SYS_GPS = 0,
	RTCM_SYS_GLO = 1,
	RTCM_SYS_GAL = 2,
	RTCM_SYS_SBA = 3,
	RTCM_SYS_QZS = 4,
	RTCM_SYS_BDS = 5,
	RTCM_SYS_NIC = 6,
	RTCM_SYS_NUM = 7,
	RTCM_SYS_MAX = 0xFFFFFFFF,
} RTCM_SYSTEM_E;

typedef enum _RTCM3_MSM_E
{
	RTCM3_MSM1 = 0,
	RTCM3_MSM2 = 1,
	RTCM3_MSM3 = 2,
	RTCM3_MSM4 = 3,
	RTCM3_MSM5 = 4,
	RTCM3_MSM6 = 5,
	RTCM3_MSM7 = 6,
	RTCM3_MSM_NUM = 7,
	RTCM3_MSM_MAX = 0xFFFFFFFF,
} RTCM3_MSM_E;

/** Stationary Antenna Reference Point, with Height Information */
typedef struct _RTCM_ANTENNA_INFO_T
{
	unsigned int    StationID;          /**< Reference Station ID */
	unsigned int    ITRF;               /**< Reserved for ITRF Realization Year */
	unsigned int    GPSIndicator;       /**< GPS Indicator */
	unsigned int    GLOIndicator;       /**< GLONASS Indicator */
	unsigned int    GALIndicator;       /**< Reserved for Galileo Indicator */
	unsigned int    StaionIndicator;    /**< Reference-Station Indicator */
	unsigned int    ReceiverIndicator;  /**< Single Receiver Oscillator Indicator */
	unsigned int    QuarterCycleIndicator;/**< Quarter Cycle Indicator */
	double          Position[3];        /**< Antenna Reference Point ECEF-X,Y,Z (unit : m) */
	double          AntennaHeight;      /**< Antenna Height (unit : m) */
} RTCM_ANTENNA_INFO_T;

/** Antenna & Receiver Descriptor @todo add AntennaDescriptor strlen*/
typedef struct _RTCM_RECEIVER_DESCRIPTOR_INFO_T
{
	unsigned int    StationID;                                              /**< Reference Station ID */
	char            AntennaDescriptor[RTCM_ANTENNA_DESCRIPTOR_LEN_MAX];     /**< Antenna Descriptor */
	unsigned int    AntennaID;                                              /**< Antenna Setup ID */
	char            AntennaSN[RTCM_ANTENNA_SN_LEN_MAX];                     /**< Antenna Serial Number */
	char            ReceiverDescriptor[RTCM_RECEIVER_DESCRIPTOR_LEN_MAX];   /**< Receiver Type Descriptor */
	char            ReceiverFirmVer[RTCM_RECEIVER_FIRMWARE_VER_LEN_MAX];    /**< Receiver Firmware Version */
	char            ReceiverSN[RTCM_RECEIVER_SN_LEN_MAX];                   /**< Receiver Serial Number */
} RTCM_RECEIVER_DESCRIPTOR_INFO_T;

/** Contents of the Message Header, Types 1001, 1002, 1003, 1004: GPS RTK Messages */
typedef struct _RTCM_GPS_RTK_MSG_HEAD_DATA_T
{
	unsigned int    StationID;              /**< Reference station ID*/
	unsigned int    TOW;                    /**< GPS Epoch Time*/
	unsigned int    Sync;                   /**< Synchronous GNSS Flag*/
	unsigned int    SatNum;                 /**< No. of GPS Satellite Signals Processed*/
	unsigned int    DivSmoothingIndicator;  /**< GPS Divergence-free Smoothing Indicator*/
	unsigned int    SmoothingInterval;      /**< GPS Smoothing Interval*/
} RTCM_GPS_RTK_MSG_HEAD_DATA_T;

/** Contents of the Satellite-Specific Portion of a Type 1004 Message, Each Satellite - GPS Extended RTK, L1 & L2 */
typedef struct _RTCM_GPS_EXT_RTK_DATA_T
{
	unsigned int    PRN;                /**< GPS Satellite ID*/
	unsigned int    L1Code;             /**< GPS L1 Code Indicator*/
	unsigned int    L1PseudoRange;      /**< GPS L1 Pseudorange*/
	unsigned int    L1PhasePseudoDiff;  /**< GPS L1 Phaserange - L1 Pseudorange*/
	int             L1LockTime;         /**< GPS L1 Lock time Indicator*/
	unsigned int    L1IntPseudoRange;   /**< GPS Integer L1 Pseudorange Modulus Ambiguity*/
	unsigned int    L1Cnr;              /**< GPS L1 CNR*/
	unsigned int    L2Code;             /**< GPS L2 Code Indicator*/
	int             PseudoRangeDiff;    /**< GPS L2-L1 Pseudorange Difference*/
	int             L2PhL1PsDiff;       /**< GPS L2 Phaserange - L1 Pseudorange*/
	unsigned int    L2LockTime;         /**< GPS L2 Lock time Indicator*/
	unsigned int    L2Cnr;              /**< GPS L2 CNR*/
} RTCM_GPS_EXT_RTK_DATA_T;

typedef struct _RTCM_GPS_RTK_DATA_T
{
	RTCM_GPS_RTK_MSG_HEAD_DATA_T Head;
	RTCM_GPS_EXT_RTK_DATA_T RTKData;
} RTCM_GPS_RTK_DATA_T;

/** Contents of the Message Header, Types 1009 through 1012: GLONASS RTK Messages */
typedef struct _RTCM_GLO_RTK_MSG_HEAD_DATA_T
{
	unsigned int    StationID;                  /**< Reference station ID*/
	unsigned int    TK;                         /**< GLONASS Epoch Time*/
	unsigned int    Sync;                       /**< Synchronous GNSS Flag*/
	unsigned int    SatNum;                     /**< No. of GLONASS Satellite Signals Processed*/
	unsigned int    DivSmoothingIndicator;      /**< GLONASS Divergence-free Smoothing Indicator*/
	unsigned int    SmoothingInterval;          /**< GLONASS Smoothing Interval*/
} RTCM_GLO_RTK_MSG_HEAD_DATA_T;

/** Contents of the Satellite-Specific Portion of a Type 1012 Message, Each Satellite - GLONASS Extended RTK, L1 & L2 */
typedef struct _RTCM_GLO_EXT_RTK_DATA_T
{
	unsigned int    PRN;                /**< GLONASS Satellite ID (Satellite Slot Number)*/
	unsigned int    L1Code;             /**< GLONASS L1 Code Indicator*/
	unsigned int    SatFreqChnNum;      /**< GLONASS Satellite Frequency Channel Number */
	unsigned int    L1PseudoRange;      /**< GLONASS L1 Pseudorange*/
	int             L1PhasePseudoDiff;  /**< GLONASS L1 Phaserange - L1 Pseudorange*/
	unsigned int    L1LockTime;         /**< GLONASS L1 Lock time Indicator*/
	unsigned int    L1IntPseudoRange;   /**< GLONASS Integer L1 Pseudorange Modulus Ambiguity*/
	unsigned int    L1Cnr;              /**< GLONASS L1 CNR*/
	unsigned int    L2Code;             /**< GLONASS L2 Code Indicator*/
	int             PseudoRangeDiff;    /**< GLONASS L2-L1 Pseudorange Difference*/
	int             L2PhL1PsDiff;       /**< GLONASS L2 Phaserange - L1 Pseudorange*/
	unsigned int    L2LockTime;         /**< GLONASS L2 Lock time Indicator*/
	unsigned int    L2Cnr;              /**< GLONASS L2 CNR*/
} RTCM_GLO_EXT_RTK_DATA_T;

typedef struct _RTCM_GLO_RTK_DATA_T
{
	RTCM_GLO_RTK_MSG_HEAD_DATA_T Head;
	RTCM_GLO_EXT_RTK_DATA_T RTKData;
} RTCM_GLO_RTK_DATA_T;

/** Contents of GLONASS L1 and L2 Code-Phase Biases Message 1230 */
typedef struct _RTCM_GLO_CODE_PHASE_BIASES_DATA_T
{
	unsigned int    StationID;          /**< Reference Station ID*/
	unsigned int    CodeInd;            /**< GLONASS Code-Phase bias indicator*/
	unsigned int    Reserved;           /**< Reserved */
	unsigned int    SignMask;           /**< GLONASS FDMA signals mask*/
	double          CodePhaseBias[4];   /**< GLONASS L1C/A L1P L2C/A L2P Code-Phase Bias*/
} RTCM_GLO_CODE_PHASE_BIASES_DATA_T;

/** information of station battery */
typedef struct _STATION_BAT_INFO_T
{
	unsigned int SubID;                 /**< sub message id */
	unsigned int BaseStationOffset;     /**< Base Station Offset */
	unsigned int RemainElectricity;     /**< Remaining Electricity */
	unsigned int PowerInfo;             /**< Base station power information */
} STATION_BAT_INFO_T;

/** monitor sensor observations */
typedef struct _MONITOR_SENSOR_OBS_T
{
	unsigned int SubID;                 /**< sub message id */
	unsigned int BaseStationOffset;     /**< Base Station Offset */
	unsigned int RemainElectricity;     /**< Remaining Electricity */
	unsigned int PowerInfo;             /**< Base station power information */
	int Reserved1;                      /**< Reserved */
	unsigned int TriggerFlag;           /**< Trigger Flag */
	float AccX;                         /**< AccX */
	float AccY;                         /**< AccY */
	float AccZ;                         /**< AccZ */
	float AttitudeX;                    /**< AttitudeX */
	float AttitudeY;                    /**< AttitudeY */
	float AttitudeZ;                    /**< AttitudeZ */
	int Reserved2;                      /**< Reserved */
} MONITOR_SENSOR_OBS_T;

/** Content of Message Header for MSM1, MSM2, MSM3, MSM4, MSM5, MSM6 and MSM7 */
typedef struct _RTCM_MSM_HEADER_DATA_T
{
	unsigned int    Nsat;                   /**< the size of Nsat */
	unsigned int    Nsig;                   /**< the size of Nsig*/
	unsigned int    StationID;              /**< Reference station ID*/
	unsigned int    EpochTime;              /**< GNSS Epoch Time*/
	unsigned int    MultBit;                /**< Multiple Message Bit*/
	unsigned int    IODS;                   /**< Issue of Data Station*/
	unsigned int    TimeTrans;              /**< cumulative session transmitting time */
	unsigned int    ClockSteeringInd;       /**< Clock Steering Indicator*/
	unsigned int    ExClockInd;             /**< External Clock Indicator*/
	unsigned int    DivSmoothingInterval;   /**< GNSS Divergence-free Smoothing Indicator*/
	unsigned int    SmoothingInterval;      /**< GNSS Smoothing Interval*/
	unsigned int    SatMask[RTCM_MAX_SAT_NUM];            /**< GNSS Satellite Mask*/
	unsigned int    SignMask[RTCM_MAX_SIGN_NUM];           /**< GNSS Signal Mask*/
	unsigned int    CellMask[RTCM_MAX_CELL_NUM];           /**< GNSS Cell Mask*/
} RTCM_MSM_HEADER_DATA_T;

/** Content of Satellite Data for MSM4, MSM5, MSM6 and MSM7 */
typedef struct _RTCM_MSM_SAT_DATA_T
{
	unsigned int    NSat;               /**< satellite number*/
	double*         pRoughRange;        /**< GNSS Satellite rough ranges*/
	unsigned int*   pInfo;              /**< GNSS Extended Satellite Information*/
	double*         pRoughRate;         /**< GNSS Satellite rough PhaseRangeRates*/
} RTCM_MSM_SAT_DATA_T;

/** Content of Signal Data for MSM4, MSM5, MSM6 and MSM7 */
typedef struct _RTCM_MSM_SIGNAL_DATA_T
{
	unsigned int    NCell;              /**< The size of cell mask is Nsat*Nsig*/
	double*         pPseudoRange;       /**< GNSS signal fine Pseudoranges*/
	double*         pPhaseRange;        /**< GNSS signal fine Phaserange data*/
	unsigned int*   pLockTime;          /**< GNSS Phaserange Lock Time Indicator*/
	unsigned int*   pHalfCycleAmb;      /**< Half-cycle ambiguity indicator*/
	double*         pCnr;               /**< GNSS signal CNRs*/
	double*         pPhaseRangeRate;    /**< GNSS signal fine PhaseRangeRates*/
} RTCM_MSM_SIGNAL_DATA_T;

typedef struct _RTCM_SPECIAL_MSG_T
{
	char SpecialMsg[RTCM_SPECIAL_MSG_LEN_MAX];  /**< special message */
} RTCM_SPECIAL_MSG_T;

typedef struct _STATION_POSITION_DATA_T     /**< decode/encode: station parameter type */
{
	unsigned int StationID;     /**< Station id */
	int DeltaType;              /**< antenna delta type (0:ENU, 1:XYZ) */
	double Position[3];         /**< station position (ecef) (unit : m)*/
	double PositionDelta[3];    /**< antenna position delta (e/n/u or x/y/z) (unit : m) */
	double AntennaHeight;       /**< Antenna Height (unit : m) */
} STATION_POSITION_DATA_T;

typedef struct _DIFF_CORRECTION_DATA_T    /**< decode/encode:DGPS/GNSS correction type */
{
	unsigned char ChangeBit;    /**< for GLO */
	unsigned char Reserved[3];

	unsigned int SatID;         /**< all system based on 1, GPS(1-32), GAL(1-36), BDS(1-64), QZS(1-7) */
	int IOD;                    /**< issue of data (IOD) */

	UTC_TIME_T Time;            /**< correction time */

	double PRC;                 /**< pseudorange correction (PRC) (m) */
	double RRC;                 /**< range rate correction (RRC) (m/s) */
	double UDRE;                /**< UDRE */
} DIFF_CORRECTION_DATA_T;

/** Differential GPS Corrections */
typedef struct _DIFF_CORRECTION_T    /**< decode/encode:DGPS/GNSS correction type */
{
	unsigned int SystemID;      /**< SystemID, as defined in GNSSSignal.h  */
	unsigned int SatNum;        /**< number of satellites */
	DIFF_CORRECTION_DATA_T DiffCorrectionData[RTCM_MAX_SAT_NUM];
} DIFF_CORRECTION_T;

typedef struct _DELTA_DIFF_CORRECTION_DATA_T
{
	unsigned int SatID;         /**< all system based on 1, GPS(1-32), GAL(1-36), BDS(1-64), QZS(1-7) */
	int IOD;
	UTC_TIME_T Time;            /**< old PRC time */
	double DeltaPRC;
	double DeltaRRC;
	double UDRE;                /**< base on old sat eph UDRE */
} DELTA_DIFF_CORRECTION_DATA_T;

/** Delta Differential GPS Corrections */
typedef struct _DELTA_DIFF_CORRECTION_T
{ /** for msg2 or msg44 */
	unsigned int SystemID;      /**< SystemID, as defined in GNSSSignal.h  */
	unsigned int SatNum;        /**< number of satellites */
	DELTA_DIFF_CORRECTION_DATA_T DeltaDiffCorrectionData[RTCM_MAX_SAT_NUM];
} DELTA_DIFF_CORRECTION_T;

typedef struct _RTCM_REFSTATION_INFO_T
{
	UTC_TIME_T UTCTime;
	int GPSWeek;
	int GPSWeekMsCount;
	unsigned int DataValid;
	unsigned int Health;
	unsigned int StationType;
	unsigned int StationID;
	double Position[3];/** ECEF XYZ(m) */
	double ARPHeight;
	char AntModel[32];
	char AntSerial[32];
} RTCM_REFSTATION_INFO_T;

typedef struct _RTCM_HELMERT_TRANSFORMATION_MSG_T
{
	unsigned int SourceNameCount;							/**< Source-Name Counter N */
	char SourceName[RTCM_HELMERT_SOURCE_NAME_LEN_MAX];		/**< Source-Name */
	unsigned int TargetNameCount;							/**< Target-Name Counter M */
	char TargetName[RTCM_HELMERT_TARGET_NAME_LEN_MAX];		/**< Target-Name */
	unsigned int SysIdentifcationNumb;						/**< System Identification Number */
	int UtilizedTransMsg;									/**< Utilized Transformation Message */
	unsigned int EpochT0;									/**< Reference Epoch t0 */
	int dX0;												/**< Translation in X for Reference Epoch t0 */
	int dY0;												/**< Translation in Y for Reference Epoch t0 */
	int dZ0;												/**< Translation in Z for Reference Epoch t0 */
	int R01;												/**< Rotation around the X-axis for Reference Epoch t0 */
	int R02;												/**< Rotation around the Y-axis for Reference Epoch t0 */
	int R03;												/**< Rotation around the Z-axis for Reference Epoch t0 */
	int dS0;												/**< Scale correction for Reference Epoch t0 */
	int dXDot;												/**< Rate of change of translation in X */
	int dYDot;												/**< Rate of change of translation in Y */
	int dZDot;												/**< Rate of change of translation in Z */
	int R1Dot;												/**< Rate of change of rotation around the X-axis */
	int R2Dot;												/**< Rate of change of rotation around the Y-axis */
	int R3Dot;												/**< Rate of change of rotation around the Z-axis */
	int dSDot;												/**< Rate of change of scale correction */
} RTCM_HELMERT_TRANSFORMATION_MSG_T;

typedef struct _RTCM_CRS_DATABASE_LINK_T
{
	unsigned int LinkCount;								/**< Database-Links Counter */
	char DataBaseLink[RTCM_CRS_DATABASE_LINK_LEN_MAX];	/**< Database-Links */
} RTCM_CRS_DATABASE_LINK_T;

typedef struct _RTCM_CRS_MSG_T
{
	unsigned int CRSNameCount;					/**< RTCM-CRS-Name Counter */
	char CRSName[RTCM_CRS_NAME_LEN_MAX];		/**< RTCM-CRS-Name */
	int Anchor;									/**< Anchor */
	unsigned int PlateNumber;					/**< Plate Number */
	unsigned int LinksINumb;					/**< Number of Database-Links */
	RTCM_CRS_DATABASE_LINK_T DataBaseLink[RTCM_CRS_LINKS_I_NUM_MAX];
} RTCM_CRS_MSG_T;

typedef struct _RTCM_SAT_SPECIFIC_PART_T
{
	unsigned int SatID;	/**< Satellite ID */
	unsigned int Soc;	/**< Constant term of standard deviation(1 sigma) for non-dispersive interpolation residuals */
	unsigned int Sod;	/**< Distance dependent term of standard deviation(1 sigma) for non-dispersive interpolation residuals */
	unsigned int Soh;	/**< Height-dependent term of standard deviation(1 sigma) for non-dispersive interpolation residuals */
	unsigned int Sic;	/**< Constant term of standard deviation(1 sigma) for non-dispersive interpolation residuals */
	unsigned int Sid;	/**< Distance dependent term of standard deviation(1 sigma) for dispersive interpolation residuals */
} RTCM_SAT_SPECIFIC_PART_T;

typedef struct _RTCM_NETWORK_RTK_RESIDUAL_MSG_T
{
	unsigned int Tow;							/**< Residuals Epoch Time */
	unsigned int StationID;						/**< Reference Station ID */
	unsigned int RefsN;							/**< N-Refs */
	unsigned int ProcessedSatNumb;				/**< Number of Satellite Signals Processed */
	RTCM_SAT_SPECIFIC_PART_T SatSpecificPart[RTCM_NETWORK_RTK_RESIDUAL_SAT_NUM];
} RTCM_NETWORK_RTK_RESIDUAL_MSG_T;

typedef struct _OBS_DATA_PER_SAT_T
{
	unsigned int SystemID;
	unsigned int SatID;
	unsigned int ObsNum;
	RM_RANGE_DATA_T ObsData[MAX_FREQ_INDEX_NUM+1]; /**< obs data*/
} OBS_DATA_PER_SAT_T;

#define OBS_DATA_SAT_NUM_MAX			(96)	/**< obs data sat num max */

typedef struct _OBS_DATA_SAT_T
{
	unsigned int SatNum;
	OBS_DATA_PER_SAT_T ObsDataPerSat[OBS_DATA_SAT_NUM_MAX]; /**< obs data*/
} OBS_DATA_SAT_T;

typedef struct _RTCM_REFSTATION_VENDOR_INFO_T
{
	int RefVendorType;
	unsigned int QuarterCycleIndicator;
} RTCM_REFSTATION_VENDOR_INFO_T;

typedef enum _ROUGHRANGE_DATA_VAILD_E
{
	ROUGHRANGE_DATA_INVAILD = 0,
	ROUGHRANGE_DATA_VAILD = 1,
} ROUGHRANGE_DATA_VAILD_E;

typedef struct _RTCM_ROUGHRANGE_INFO_T
{
	int DataVaild;
	int IntRoughRange;
	double ModRoughRange;
} RTCM_ROUGHRANGE_INFO_T;

typedef struct _RTCM_INTERIM_DATA_T
{
	unsigned int StationID;                             /**< Reference station ID */
	unsigned int LastTypeID;
	UTC_TIME_T LastRTCMTime;                            /**< Last UTC time */
	UTC_TIME_T RTCMTime;                                /**< UTC time */
	int LeapSec;                                        /**< leap second */
	int ObsCompleteFlag;                                /**< obs data complete flag (1:ok,0:not complete) */
	unsigned int RTKDataCompleteTypeIDLast;             /**< last RTKData complete Type ID */
	unsigned int RTKDataCompleteTypeID;                 /**< RTKData complete Type ID */
	unsigned int RTKDataMask;                           /**< RTCM RTKData message receiver flag:bit0-3:1001-1004, bit4-7:1009-1012 */
	unsigned int RTKDataMaskLast;                       /**< last RTCM RTKData message receiver flag:bit0-3:1001-1004, bit4-7:1009-1012 */
	unsigned int MSMDataCompleteTypeIDLast;             /**< last RTKData complete Type ID */
	unsigned int MSMDataCompleteTypeID;                 /**< RTKData complete Type ID */
	unsigned int MSMDataMask[RTCM_SYS_NUM];             /**< RTCM RTKData message receiver flag:bit0-6:MSM1-MSM7 */
	unsigned int MSMDataMaskLast[RTCM_SYS_NUM];         /**< last RTCM RTKData message receiver flag:bit0-6:MSM1-MSM7 */
	unsigned int ObsDataOffset[RTCM_SYS_NUM];           /**< offset of obs data from various systems in the structure RM_RANGE_LOG_T */
	unsigned int LockTime[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SAT_NUM][RTCM_MAX_SIGN_NUM]; /**< lock time */
	unsigned int LossCount[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SAT_NUM][MAX_FREQ_INDEX_NUM]; /**< loss of lock count */
	double CarrierPhase[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SAT_NUM][MAX_FREQ_INDEX_NUM];
	RTCM_REFSTATION_INFO_T RefStationInfo;
	double Position[3];
	OBS_DATA_SAT_T SatObsData; /**< obs data */
	RTCM_REFSTATION_VENDOR_INFO_T RefVendorInfo;
#ifdef RTCM_DEBUG_INFO
	unsigned int BaseObsSatNum[RTCM_SYS_NUM];
#endif

	UTC_TIME_T LastVRSTime;                            /**< Last VRS UTC time */
	int VRSCompleteFlag;                               /**< VRS data complete flag (1:ok,0:not complete) */

	RTCM_ROUGHRANGE_INFO_T RoughRangeInfo[TOTAL_EX_GNSS_SYSTEM_NUM][RTCM_MAX_SAT_NUM];   /**< for MSM1-MSM3 calculate int rough range */
} RTCM_INTERIM_DATA_T;

typedef enum _RAW_OBS_DATA_TYPE_E
{
	RAW_OBS_DATA_TYPE_RTCM_MSM = 0,
	RAW_OBS_DATA_TYPE_RTCM_RTKDATA,
	RAW_OBS_DATA_TYPE_RTCM2,
	RAW_OBS_DATA_TYPE_RANGE_ASCII,
	RAW_OBS_DATA_TYPE_RANGE_BINARY,
	RAW_OBS_DATA_TYPE_RANGE_1_ASCII,
	RAW_OBS_DATA_TYPE_RANGE_1_BINARY,
	RAW_OBS_DATA_TYPE_RANGECMP_ASCII,
	RAW_OBS_DATA_TYPE_RANGECMP_BINARY,
	RAW_OBS_DATA_TYPE_RANGECMP_1_ASCII,
	RAW_OBS_DATA_TYPE_RANGECMP_1_BINARY,
	RAW_OBS_DATA_TYPE_BASEOBS_ASCII,
	RAW_OBS_DATA_TYPE_BASEOBS_BINARY,
	RAW_OBS_DATA_TYPE_GNSSOBS_ASCII,
	RAW_OBS_DATA_TYPE_GNSSOBS_BINARY,
	RAW_OBS_DATA_TYPE_GNSSOBS_1_ASCII,
	RAW_OBS_DATA_TYPE_GNSSOBS_1_BINARY,
	RAW_OBS_DATA_TYPE_GNSSOBSCMP_ASCII,
	RAW_OBS_DATA_TYPE_GNSSOBSCMP_BINARY,
	RAW_OBS_DATA_TYPE_GNSSOBSCMP_1_ASCII,
	RAW_OBS_DATA_TYPE_GNSSOBSCMP_1_BINARY,
	RAW_OBS_DATA_TYPE_HRCX,
	RAW_OBS_DATA_TYPE_RINEX,

	RAW_OBS_DATA_TYPE_NUM_MAX,
} RAW_OBS_DATA_TYPE_E;

typedef struct _GNSS_OBS_T
{
	UTC_TIME_T ObsTime;           /**< UTC time */
	unsigned int RawDataType;
	unsigned int BaseID;
	unsigned int VendorTypeID;
	unsigned int BDSSignalExpand; /**< 0: standard BDS signal, other:Expand BDS signal */
	unsigned int ObsSystemMask;
	RM_EX_HEADER_T LogExHeader;
	RM_RANGE_LOG_T RangeData;                           /**< obs data */
	RM_EX_GNSSOBS_LOG_T GNSSObsData;                    /**< obs data */
} GNSS_OBS_T;

typedef struct _RTCM_PARSE_INFO_T
{
	RTCM_INTERIM_DATA_T RTCMInterimData;    /**< rtcm decode interim data */
	MSG_BUF_T DecodeBuf;                  /**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;                /**< address stored after data parsed */
	MSG_BUF_T* pRawDataBuf;                 /**< raw data buffer */
#ifdef RTCM_DEBUG_INFO
	MSG_BUF_T ParsedDebugInfoBuf;           /**< parsed debug information buffer */
	MSG_BUF_T ParsedBaseObsSatCountBuf;     /**< parsed BaseObs satellite count information buffer */
#endif
} RTCM_PARSE_INFO_T;

typedef struct _MSG_DIRECT_OUT_PORT_T
{
	unsigned int InputPortID;
	unsigned int OutputPortID;
} MSG_DIRECT_OUT_PORT_T;

typedef enum _RTCM_STATION_TYPE_E
{
	RTCM_STATION_TYPE_NONE = 0,
	RTCM_STATION_TYPE_RTCMV2 = 1,
	RTCM_STATION_TYPE_RTCMV3 = 2,
} RTCM_STATION_TYPE_E;

typedef enum _RTCM_ANTENNA_E
{
	RTCM_MAIN_ANT = 0,
	RTCM_SLAVE_ANT = 1,
} _RTCM_ANTENNA_E;

typedef enum _RTCM_DATA_SOURCE_E
{
	RTCM_DATA_SOURCE_MAIN_ANT = 0,
	RTCM_DATA_SOURCE_SLAVE_ANT = 1,
} RTCM_DATA_SOURCE_E;

typedef enum _RTCM_INPUT_STATUS_E
{
	RTCM_INPUT_DATA_NO = 0, /**< Serial port do not receive data */
	RTCM_INPUT_DATA_YES = 1, /**< Serial port receive data */
} RTCM_INPUT_STATUS_E;

typedef struct _RTCM_INPUT_DATA_STATUS_T
{
	unsigned int Status; /**< RTCM input data status */
	double TimeMs; /**< system tick time(unit: ms) */
} RTCM_INPUT_DATA_STATUS_T;

typedef struct _RTCM_SYSTEM_PARAM_MSG_INFO_T
{
	unsigned int	MsgID;											/**< Message ID */
	unsigned int	SyncFlag;										/**< Message Sync Flag */
	unsigned int	TransInterval;									/**< Message #1 Transmission Interval */
} RTCM_SYSTEM_PARAM_MSG_INFO_T;

typedef struct _RTCM_SYSTEM_PARAM_T
{
	unsigned int	StationID;										/**< Reference Station ID */
	unsigned int	MJD;											/**< Modified Julian Day (MJD) Number */
	unsigned int	Tod;											/**< Seconds of Day (UTC) */
	unsigned int	MsgNum;											/**< No. of Message ID Announcements to Follow (Nm) */
	unsigned int	LeapSecs;										/**< Leap Seconds, GPS-UTC */
	RTCM_SYSTEM_PARAM_MSG_INFO_T 	MsgInfo[RTCM_SYSTEM_PARAM_MSG_NUM_MAX];		/**< Message Announcements to Follow */
} RTCM_SYSTEM_PARAM_T;

typedef struct _RTCM_UNICODE_TEXT_STRING_T
{
	unsigned int	StationID;										/**< Reference Station ID */
	unsigned int	MJD;											/**< Modified Julian Day (MJD) Number */
	unsigned int	Tod;											/**< Seconds of Day (UTC) */
	unsigned int	CharsNum;										/**< Number of Characters to Follow */
	unsigned int	CodeNum;										/**< Number of UTF-8 Code Units (N) */
	char			CodeBuf[RTCM_UNICODE_TEXT_STRING_LEN_MAX];		/**< UTF-8 Character Code Units */
} RTCM_UNICODE_TEXT_STRING_T;

typedef struct _RTCM_RAWRTCMAUX_DATA_T
{
	int GPSWeek;
	int GPSWeekMsCount;

	unsigned short BaseID;
	unsigned short MsgID;

	unsigned char DataType;
	unsigned char Reserved;
	unsigned short HexDataLen;

	unsigned char DataBuf[RTCM3_MSG_LEN_MAX];
} RTCM_RAWRTCMAUX_DATA_T;

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

typedef struct _RTCM_SERVICE_CRS_MSG_T
{
	unsigned char CRSNameCount;					/**< Service-CRS-Name Counter */
	char CRSName[RTCM_CRS_NAME_LEN_MAX];		/**< Service-CRS-Name */
	float CoordinateEpoch;						/**< Coordinate Epoch */
} _PACKED_ RTCM_SERVICE_CRS_MSG_T;

typedef struct _GLO_CODE_PHASE_BIAS_BODY_T
{
	unsigned char   CodeInd;            /**< GLONASS Code-Phase bias indicator*/
	unsigned char   Reserved;           /**< Reserved */
	unsigned char   SignMask;           /**< GLONASS FDMA signals mask*/
	double          CodePhaseBias[4];   /**< GLONASS L1C/A L1P L2C/A L2P Code-Phase Bias*/
} _PACKED_ GLO_CODE_PHASE_BIAS_BODY_T;

typedef struct _RECEIVER_DESCRIPTOR_BODY_T
{
	unsigned char	AntennaDescriptorCount;									/**< Antenna Descriptor Counter N(N<=31) */
	char			AntennaDescriptor[RTCM_ANTENNA_DESCRIPTOR_LEN_MAX]; 	/**< Antenna Descriptor */
	unsigned int	AntennaID;												/**< Antenna Setup ID */
	unsigned char	AntennaSNCount; 										/**< Antenna Serial Number Counter M(M<=31)*/
	char			AntennaSN[RTCM_ANTENNA_SN_LEN_MAX]; 					/**< Antenna Serial Number */
	unsigned char	ReceiverDescriptorCount; 								/**< Receiver Type Descriptor Counter I(I<=31)*/
	char			ReceiverDescriptor[RTCM_RECEIVER_DESCRIPTOR_LEN_MAX];	/**< Receiver Type Descriptor */
	unsigned char	ReceiverFirmVerCount; 									/**<  Receiver Firmware Version Counter J(J<=31)*/
	char			ReceiverFirmVer[RTCM_RECEIVER_FIRMWARE_VER_LEN_MAX];	/**< Receiver Firmware Version */
	unsigned char	ReceiverSNCount; 										/**< Receiver Serial Number Counter K(K<=31)*/
	char			ReceiverSN[RTCM_RECEIVER_SN_LEN_MAX];					/**< Receiver Serial Number */
} _PACKED_ RECEIVER_DESCRIPTOR_BODY_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif


#ifdef __cplusplus
}
#endif

#endif

