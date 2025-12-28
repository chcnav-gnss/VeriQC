/**********************************************************************//**
		VeriQC

	   BSW
*-
@file    RawMsgDataStructure.h
@author  CHC
@date    2019/12/06     2021/09/18
@brief   chcoem raw data encode function and details

**************************************************************************/
#ifndef _RAW_MSG_DATA_STRUCTURE_H_
#define _RAW_MSG_DATA_STRUCTURE_H_

#include "Common/DataTypes.h"
#include "RawMsgCommonDefines.h"

#include "../../MsgDecoder/RTCM/RTCMDefines.h"

#include "RawMsgRangeDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************command**************************/
/**@todo shall be removed*/
typedef struct _RM_ASCII_TO_BIN_T
{
	RM_HEADER_T    HeadMsg;       /**< message ID */
	void*    pBinPro;             /**< data info address */
	unsigned char * pCurrent;            /**< address increasing from initial value */
	 /** temp data */
	int      tmpexpdata;
	int      tmpdata;
	float    tmpdataf4;
	double   tmpdataf8;
} RM_ASCII_TO_BIN_T;

typedef enum _RM_CLOCK_MODEL_STATUS_E
{
	RM_CLOCK_MODEL_STATUS_VALID = 0,
	RM_CLOCK_MODEL_STATUS_CONVERGING = 1,
	RM_CLOCK_MODEL_STATUS_ITERATING = 2,
	RM_CLOCK_MODEL_STATUS_INVALID = 3,

	RM_CLOCK_MODEL_STATUS_NUM_MAX = 4,
} RM_CLOCK_MODEL_STATUS_E;

typedef enum _RM_UTC_STATUS_E
{
	RM_UTC_STATUS_VALID = 0,
	RM_UTC_STATUS_INVALID = 1,
	RM_UTC_STATUS_WARNING = 2,

	RM_UTC_STATUS_NUM_MAX = 3,
} RM_UTC_STATUS_E;

typedef enum _RM_SATELLITE_SYSTEM_E
{
	RM_SATELLITE_SYSTEM_GPS = 0,
	RM_SATELLITE_SYSTEM_GLO = 1,
	RM_SATELLITE_SYSTEM_SBAS = 2,
	RM_SATELLITE_SYSTEM_GAL = 5,
	RM_SATELLITE_SYSTEM_BDS = 6,
	RM_SATELLITE_SYSTEM_QZS = 7,
	RM_SATELLITE_SYSTEM_NIC = 9,
	RM_SATELLITE_SYSTEM_LBAND = 10,

	RM_SATELLITE_SYSTEM_NUM_MAX = 11,
} RM_SATELLITE_SYSTEM_E;

typedef enum _RM_SOLUTION_STATUS_E
{
	RM_SOLUTION_STATUS_SOL_COMPUTED = 0,
	RM_SOLUTION_STATUS_INSUFFICIENT_OBS = 1,
	RM_SOLUTION_STATUS_NO_CONVERGENCE = 2,
	RM_SOLUTION_STATUS_SINGULARITY = 3,
	RM_SOLUTION_STATUS_COV_TRACE = 4,
	RM_SOLUTION_STATUS_TEST_DIST = 5,
	RM_SOLUTION_STATUS_COLD_START = 6,
	RM_SOLUTION_STATUS_V_H_LIMIT = 7,
	RM_SOLUTION_STATUS_VARIANCE = 8,
	RM_SOLUTION_STATUS_RESIDUALS = 9,
	RM_SOLUTION_STATUS_INTEGRITY_WARNING = 13,
	RM_SOLUTION_STATUS_PENDING = 18,
	RM_SOLUTION_STATUS_INVALID_FIX = 19,
	RM_SOLUTION_STATUS_UNAUTHORIZED = 20,
	RM_SOLUTION_STATUS_INVALID_RATE = 22,

	RM_SOLUTION_STATUS_NUM_MAX = 23,
} RM_SOLUTION_STATUS_E;

typedef enum _RM_POSITION_VELOCITY_TYPE_E
{
	RM_POSITION_VELOCITY_TYPE_NONE = 0,
	RM_POSITION_VELOCITY_TYPE_FIXEDPOS = 1,
	RM_POSITION_VELOCITY_TYPE_FIXEDHEIGHT = 2,
	RM_POSITION_VELOCITY_TYPE_DOPPLER_VELOCITY = 8,
	RM_POSITION_VELOCITY_TYPE_SINGLE = 16,
	RM_POSITION_VELOCITY_TYPE_PSRDIFF = 17,
	RM_POSITION_VELOCITY_TYPE_WAAS = 18,
	RM_POSITION_VELOCITY_TYPE_PROPAGATED = 19,
	RM_POSITION_VELOCITY_TYPE_L1_FLOAT = 32,
	RM_POSITION_VELOCITY_TYPE_NARROW_FLOAT = 34,
	RM_POSITION_VELOCITY_TYPE_L1_INT = 48,
	RM_POSITION_VELOCITY_TYPE_WIDE_INT = 49,
	RM_POSITION_VELOCITY_TYPE_NARROW_INT = 50,
	RM_POSITION_VELOCITY_TYPE_RTK_DIRECT_INS = 51,
	RM_POSITION_VELOCITY_TYPE_INS_SBAS = 52,
	RM_POSITION_VELOCITY_TYPE_INS_PSRSP = 53,
	RM_POSITION_VELOCITY_TYPE_INS_PSRDIFF = 54,
	RM_POSITION_VELOCITY_TYPE_INS_RTKFLOAT = 55,
	RM_POSITION_VELOCITY_TYPE_INS_RTKFIXED = 56,
	RM_POSITION_VELOCITY_TYPE_PPP_CONVERGING = 68,
	RM_POSITION_VELOCITY_TYPE_PPP = 69,
	RM_POSITION_VELOCITY_TYPE_OPERATIONAL = 70,
	RM_POSITION_VELOCITY_TYPE_WARNING = 71,
	RM_POSITION_VELOCITY_TYPE_OUT_OF_BOUNDS = 72,
	RM_POSITION_VELOCITY_TYPE_INS_PPP_CONVERGING = 73,
	RM_POSITION_VELOCITY_TYPE_INS_PPP = 74,
	RM_POSITION_VELOCITY_TYPE_PPP_BASIC_CONVERGING = 77,
	RM_POSITION_VELOCITY_TYPE_PPP_BASIC = 78,
	RM_POSITION_VELOCITY_TYPE_INS_PPP_BASIC_CONVERGING = 79,
	RM_POSITION_VELOCITY_TYPE_INS_PPP_BASIC = 80,

	RM_POSITION_VELOCITY_TYPE_NUM_MAX = 81,
} RM_POSITION_VELOCITY_TYPE_E;

typedef enum _RM_OBSERVATION_STATUS_E
{
	RM_OBSERVATION_STATUS_GOOD = 0,
	RM_OBSERVATION_STATUS_BADHEALTH = 1,
	RM_OBSERVATION_STATUS_OLDEPHEMERIS = 2,
	RM_OBSERVATION_STATUS_ELEVATIONERROR = 6,
	RM_OBSERVATION_STATUS_MISCLOSURE = 7,
	RM_OBSERVATION_STATUS_NODIFFCORR = 8,
	RM_OBSERVATION_STATUS_NOEPHEMERIS = 9,
	RM_OBSERVATION_STATUS_INVALIDIODE = 10,
	RM_OBSERVATION_STATUS_LOCKEDOUT = 11,
	RM_OBSERVATION_STATUS_LOWPOWER = 12,
	RM_OBSERVATION_STATUS_OBSL2 = 13,
	RM_OBSERVATION_STATUS_UNKNOWN = 15,
	RM_OBSERVATION_STATUS_NOIONOCORR = 16,
	RM_OBSERVATION_STATUS_NOTUSED = 17,
	RM_OBSERVATION_STATUS_OBSL1 = 18,
	RM_OBSERVATION_STATUS_OBSE1 = 19,
	RM_OBSERVATION_STATUS_OBSL5 = 20,
	RM_OBSERVATION_STATUS_OBSE5 = 21,
	RM_OBSERVATION_STATUS_OBSB2 = 22,
	RM_OBSERVATION_STATUS_OBSB1 = 23,
	RM_OBSERVATION_STATUS_OBSB3 = 24,
	RM_OBSERVATION_STATUS_NOSIGNALMATCH = 25,
	RM_OBSERVATION_STATUS_SUPPLEMENTARY = 26,
	RM_OBSERVATION_STATUS_NA = 99,
	RM_OBSERVATION_STATUS_BAD_INTEGRITY = 100,
	RM_OBSERVATION_STATUS_LOSSOFLOCK = 101,
	RM_OBSERVATION_STATUS_NOAMBIGUITY = 102,

	RM_OBSERVATION_STATUS_NUM_MAX = 103
} RM_OBSERVATION_STATUS_E;

typedef enum _RM_DATUM_ID_E
{
	RM_DATUM_ID_WGS84 = 61,
	RM_DATUM_ID_USER = 63,
} RM_DATUM_ID_E;

typedef enum _RM_REF_STATION_TYPE_E
{
	REF_STATION_TYPE_NONE = 0,
	REF_STATION_TYPE_RTCMV3 = 4,

	REF_STATION_TYPE_NUM_MAX = 5,
} RM_REF_STATION_TYPE_E;

typedef enum _RM_SBAS_SIGNAL_TYPE_E
{
	SBAS_SIGNAL_TYPE_L1CA = 1,
	SBAS_SIGNAL_TYPE_L5I = 2,

	SBAS_SIGNAL_TYPE_NUM_MAX = 3,
} RM_SBAS_SIGNAL_TYPE_E;

typedef enum _RM_SBAS_PREAMBLE_TYPE_E
{
	SBAS_PREAMBLE_TYPE_8BIT = 0,
	SBAS_PREAMBLE_TYPE_4BIT = 1,

	SBAS_PREAMBLE_TYPE_NUM_MAX = 2,
} RM_SBAS_PREAMBLE_TYPE_E;

typedef enum _RM_RAWRTCMAUX_MSG_TYPE_E
{
	RAWRTCMAUX_MSG_TYPE_HEX = 0,
	RAWRTCMAUX_MSG_TYPE_ASCII = 1,

	RAWRTCMAUX_MSG_TYPE_NUM_MAX = 2,
} RM_RAWRTCMAUX_MSG_TYPE_E;

typedef enum _RM_INS_STATUS_E
{
	INS_INACTIVE = 0,
	INS_ALIGNING = 1,
	INS_HIGH_VARIANCE = 2,
	INS_SOLUTION_GOOD = 3,
	INS_SOLUTION_FREE = 6,
	INS_ALIGNMENT_COMPLETE = 7,
	DETERMINING_ORIENTATION = 8,
	WAITING_INITIALPOS = 9,
	WAITING_AZIMUTH = 10,
	INITIALIZING_BIASES = 11,
	MOTION_DETECT = 12,
	WAITING_ALIGNMENTORIENTATION = 14,

	INS_STATUS_NUM_MAX = 15,
} RM_INS_STATUS_E;
#define INS_STATUS_IMU_ABNORMAL 255	/**< extend INS_STATUS */

/** Huace raw msg enum */
typedef enum _RM_EX_SATELLITE_SYSTEM_E
{
	RM_EX_SATELLITE_SYSTEM_GPS = 0,
	RM_EX_SATELLITE_SYSTEM_GLO = 1,
	RM_EX_SATELLITE_SYSTEM_GAL = 2,
	RM_EX_SATELLITE_SYSTEM_SBAS = 3,
	RM_EX_SATELLITE_SYSTEM_QZS = 4,
	RM_EX_SATELLITE_SYSTEM_BDS = 5,
	RM_EX_SATELLITE_SYSTEM_NIC = 6,
	RM_EX_SATELLITE_SYSTEM_LBAND = 7,

	RM_EX_SATELLITE_SYSTEM_NUM_MAX = 8,
} RM_EX_SATELLITE_SYSTEM_E;
	
typedef enum _RM_EX_SOLUTION_STATUS_E
{
	RM_EX_SOLUTION_STATUS_SOL_COMPUTED = 0,				/**< Solution computed */
	RM_EX_SOLUTION_STATUS_INSUFFICIENT_OBS = 1,			/**< Insufficient observations */
	RM_EX_SOLUTION_STATUS_NO_CONVERGENCE = 2,			/**< No convergence */
	RM_EX_SOLUTION_STATUS_SINGULARITY = 3,				/**< Singularity at parameters matrix */
	RM_EX_SOLUTION_STATUS_COV_TRACE = 4,				/**< Covariance trace exceeds maximum (trace > 1000 m) */
	RM_EX_SOLUTION_STATUS_VARIANCE = 5,					/**< Variance exceeds limits */
	RM_EX_SOLUTION_STATUS_PENDING = 6,					/**< When a SETFIXPOS position command is entered, the receiver 
															computes its own position and determines if the fixed position is valid */
	RM_EX_SOLUTION_STATUS_INVALID_FIX = 7,				/**< The fixed position, entered using the SETFIXPOS position command, is not valid */
	RM_EX_SOLUTION_STATUS_RESIDUALS = 8,				/**< Residuals are too large */
	RM_EX_SOLUTION_STATUS_INVALID_WARNING = 9,			/**< Large residuals make position unreliable */

	RM_EX_SOLUTION_STATUS_NUM_MAX,
} RM_EX_SOLUTION_STATUS_E;

typedef enum _RM_EX_POSITION_VELOCITY_TYPE_E
{
	RM_EX_POSITION_VELOCITY_TYPE_NONE = 0,						/**< No solution */
	RM_EX_POSITION_VELOCITY_TYPE_SINGLE = 1,					/**< Solution calculated using only data supplied by the GNSS satellites */
	RM_EX_POSITION_VELOCITY_TYPE_SPPDIFF = 2,					/**< Solution calculated using pseudorange differential (DGPS, DGNSS) corrections */

	RM_EX_POSITION_VELOCITY_TYPE_NARROW_INT = 4,				/**< Multi-frequency RTK solution with carrier phase ambiguities resolved to narrow-lane integers */
	RM_EX_POSITION_VELOCITY_TYPE_NARROW_FLOAT = 5,				/**< Multi-frequency RTK solution with unresolved, float carrier phase ambiguities */

	RM_EX_POSITION_VELOCITY_TYPE_FIXEDPOS = 7,					/**< Position has been fixed by the SETFIXPOS position command or by position averaging. */

	RM_EX_POSITION_VELOCITY_TYPE_SBAS = 9,						/**< Solution calculated using corrections from an SBAS satellite */
	RM_EX_POSITION_VELOCITY_TYPE_PPP = 10,						/**< Converged B2b or HAS or MADOCA PPP solution */
	RM_EX_POSITION_VELOCITY_TYPE_PPP_FIXED = 11,				/**< Converged PointSky or CLAS PPP solution */
	RM_EX_POSITION_VELOCITY_TYPE_PPP_CONVERGING = 12,			/**< Converging PPP solution */
	RM_EX_POSITION_VELOCITY_TYPE_WIDE_INT = 13,					/**< Multi-frequency RTK solution with carrier phase ambiguities resolved to wide-lane integers */
	RM_EX_POSITION_VELOCITY_TYPE_L1_INT = 14,					/**< Single-frequency RTK solution with carrier phase ambiguities resolved to integers */
	RM_EX_POSITION_VELOCITY_TYPE_L1_FLOAT = 15,					/**< Single-frequency RTK solution with unresolved, float carrier phase ambiguities */
	RM_EX_POSITION_VELOCITY_TYPE_DOPPLER_VELOCITY = 16,			/**< Velocity computed using instantaneous Doppler */

	RM_EX_POSITION_VELOCITY_TYPE_INS = 20,						/**< INS positioning solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_SBAS = 21,					/**< INS position, where the last applied position update used a GNSS solution computed using corrections from an SBAS solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_SPP = 22,					/**< INS position, where the last applied position update used a single point GNSS(SINGLE) solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_SPPDIFF = 23,				/**< INS position, where the last applied position update used a pseudorange differential GNSS (SPPDIFF) solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_RTKFLOAT = 24,				/**< INS position, where the last applied position update used a floating ambiguity RTK (L1_FLOAT or NARROW_FLOAT) solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_RTKFIXED = 25,				/**< INS position, where the last applied position update used a fixed integer ambiguity RTK (L1_INT, WIDE_INT or NARROW_INT) solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_PPP = 26,					/**< INS position, where the last applied position update used B2b or HAS or MADOCA ppp converged solution */
	RM_EX_POSITION_VELOCITY_TYPE_INS_PPPFIXED = 27,				/**< INS position, where the last applied position update used PointSky or CLAS ppp converged solution */

	RM_EX_POSITION_VELOCITY_TYPE_NUM_MAX,
} RM_EX_POSITION_VELOCITY_TYPE_E;

typedef enum _RM_BASE_STATION_TYPE_E
{
	BASE_STATION_TYPE_NONE = 0,
	BASE_STATION_TYPE_RTCMV3 = 2,

	BASE_STATION_TYPE_NUM_MAX,
} RM_BASE_STATION_TYPE_E;

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

/** raw message GPS ephemeris struct, previous name: txxx_EPH_GPS_s */
typedef struct _RM_EPH_GPS_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_GPS_RANGE_PRN(1) */
	double			TOW;		/**< Time stamp of subframe 1 (seconds)*/
	unsigned int	Health;		/**< Health status -a 6 bit health code as defined in ICD-GPS-200a*/
	unsigned int	Iode1;      /**< Issue of ephemeris data 1*/
	unsigned int	Iode2;		/**< Issue of ephemeris data 2*/
	unsigned int	Week;		/**< TOE Week number, GPS Week number  */
	unsigned int	Zweek;		/**< Z count week number, this is week number from subframe 1 of the ephemeris, */
								/**< the toe's week is derived from this to account for roll over*/
	double			TOE;		/**< Reference time for ephemeris, seconds*/
	double			A;			/**< Semi-major axis, meters*/
	double			DN;			/**< Mean motion difference, radians/second*/
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Ecc;		/**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			W;			/**< Argument of perigee, radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters)     */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters) */
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians)*/
	double			Cis;		/**< Inclination (amplitude of sine, radians)*/
	double			Inc0;		/**< Inclination angel at reference time, radians*/
	double			Idot;		/**< Rate if inclination angle, radians/second*/
	double			Omega0;		/**< Right ascension, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second*/
	unsigned int	Iodc;		/**< Issue of data clock*/
	double			TOC;		/**< SV clock correction term, seconds*/
	double			Tgd;		/**< Estimated group delay difference, seconds*/
	double			Af0;		/**< Clock aging parameter, seconds(s)*/
	double			Af1;		/**< Clock aging parameter, (s/s)*/
	double			Af2;		/**< Clock aging parameter, (s/s/s)*/
	unsigned int	ASFlag;     /**< Anti-spooning Flag, 0=false, 1=TRUE*/
	double			N;			/**< Corrected mean motion,  radians/second*/
	double			URA;        /**< User Range Accuracy variance, m'2*/
}_PACKED_ RM_EPH_GPS_DATA_T;

/** QZSS Ephemeris Log Body Structure*/
typedef struct _RM_EPH_QZS_DATA_T
{
	unsigned int	PRN;           /**< satellite PRN number,  based on MIN_QZSS_RANGE_PRN (193)*/
	double			TOW;           /**< Time stamp of subframe 1 (seconds)*/
	unsigned int	Health;        /**< Health status -a 6 bit health code as defined in ICD-GPS-200a*/
	unsigned int	Iode1;         /**< Issue of ephemeris data 1*/
	unsigned int	Iode2;         /**< Issue of ephemeris data 2*/
	unsigned int	Week;          /**< TOE Week number, GPS Week number  */
	unsigned int	Zweek;         /**< Z count week number, this is week number from subframe 1 of the ephemeris, */
								   /**< the toe's week is derived from this to account for roll over*/
	double			TOE;           /**< Reference time for ephemeris, seconds*/
	double			A;             /**< Semi-major axis, meters*/
	double			DN;            /**< Mean motion difference, radians/second*/
	double			M0;            /**< Mean anomaly of reference time , radians*/
	double			Ecc;           /**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			W;             /**< Argument of perigee, radians*/
	double			Cuc;           /**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Cus;           /**< Argument of latitude (amplitude of sine, meters)     */
	double			Crc;           /**< Orbit radius (amplitude of cosine, meters) */
	double			Crs;           /**< Orbit radius (amplitude of sine, radians) */
	double			Cic;           /**< Inclination (amplitude of cosine, radians)*/
	double			Cis;           /**< Inclination (amplitude of sine, radians)*/
	double			Inc0;          /**< Inclination angel at reference time, radians*/
	double			Idot;          /**< Rate if inclination angle, radians/second*/
	double			Omega0;        /**< Right ascension, radians*/
	double			OmegaDot;      /**< Rate of right ascension, radians/second*/
	unsigned int	Iodc;          /**< Issue of data clock*/
	double			TOC;           /**< SV clock correction term, seconds*/
	double			Tgd;           /**< Estimated group delay difference, seconds*/
	double			Af0;           /**< Clock aging parameter, seconds(s)*/
	double			Af1;           /**< Clock aging parameter, (s/s)*/
	double			Af2;           /**< Clock aging parameter, (s/s/s)*/
	unsigned int	ASFlag;        /**< Anti-spooning Flag, 0=false, 1=TRUE*/
	double			N;             /**< Corrected mean motion,  radians/second*/
	double			URA;           /**< User Range Accuracy variance, m'2*/
	unsigned char	FitInterval;   /**< curve fit interval; 0 - eph data are effective for 2 hours; 1 - eph data are effective more than 2 hours*/
	unsigned char	Reseved[3];    /**< reserved*/
}_PACKED_ RM_EPH_QZS_DATA_T;

/** raw message BDS ephemeris struct, previous name: txxx_EPH_BD_s */
typedef struct _RM_EPH_BD2_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week Number, BDS Time Week number  */
	double			URA;		/**< User Range Accuracy variance, m*/
	unsigned int	Health;		/**< Health status -a 6 bit health code as defined in BDS-ICD*/
	double			Tgd;		/**< Estimated group delay difference for B1, seconds*/
	double			Tgd2;		/**< Estimated group delay difference for B2, seconds*/
	unsigned int	Iodc;		/**< Issue of data clock*/
	unsigned int	TOC;		/**< SV clock correction term, seconds*/
	double			Af0;		/**< Clock aging parameter, (s)*/
	double			Af1;		/**< Clock aging parameter, (s/s)*/
	double			Af2;		/**< Clock aging parameter, (s/s/s)*/
	unsigned int	Iode1;		/**< Issue of ephemeris data 1*/
	unsigned int	TOE;		/**< Reference time for ephemeris, seconds*/
	double			RootA;		/**< Square root Semi-major axis, meters*/
	double			Ecc;		/**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			W;			/**< Argument of perigee, radians*/
	double			DN;			/**< Mean motion difference, radians/second*/
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Omega0;		/**< Omega0,Right ascension, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second*/
	double			Inc0;		/**< Inclination angel at reference time, radians*/
	double			Idot;		/**< Rate if inclination angle, radians/second*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians)*/
	double			Cis;		/**< Inclination (amplitude of sine, radians)*/

}_PACKED_ RM_EPH_BD2_DATA_T;

/** raw message BDS CNAV1/CNAV2 ephemeris struct */
typedef struct _RM_EPH_BDS_CNAV12_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week number in BDS time (BDT) */
	unsigned int    SatStatus;  /**< Satellite Status Information
									 bit 0~1: HS() ,bit 2: DIF, bit 3: SIF, bit 4: AIF
									 bit 5 ~ 8, Signal In Space Monitoring Accuracy Index */
	unsigned int	IODE;		/**< Issue of ephemeris data */
	unsigned int	TOE;		/**< Reference time of ephemeris parameters in BeiDou time (BDT). (s) */
	unsigned int    SatType;    /**< 2 bit orbit type */

	double          DeltaA;     /**< Semi-major axis difference at reference time (m) */
	double          ADot;       /**< Change rate in Semi-major axis (m/s) */
	double          DeltaN;     /**< Mean motion difference from computed value (radians/s) */
	double          NDot;       /**< Rate of Mean motion difference from computed value (radians/s/s) */
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Ecc;		/**< Eccentricity, dimensionless */
	double			Omega;	 	/**< Argument of perigee, radians*/
	double			Omega0;		/**< Longitude of ascending node of orbital of planeat weekly epoch, radians */
	double			I0;	        /**< Inclination angel at reference time, radians */
	double			OmegaDot;	/**< Rate of right ascension, radians/second */
	double			I0Dot;		/**< Rate if inclination angle, radians/second */

	double			Cis;		/**< Inclination (amplitude of sine, radians) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians) */
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/

	unsigned int    IODC;		/**< Issue of data, clock */
	unsigned int	TOC;		/**< Reference time of clock parameters in BeiDou time (BDT).*/

	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/
	double          TGDB1Cp;    /**< Group delay differential between the B1C pilot component and the B3I signal (s) */
	double          TGDB2Ap;    /**< Group delay differential between the B2a pilot component and the B3I signal (s) */
	double          ISCd;       /**< Group delay differential between the B1C data and pilot components (s) for CNAV1 msg
									 Group delay differential between the B2a data and pilot components (s) for CNAV2 msg */
	unsigned int    Reserved;
}_PACKED_ RM_EPH_BDS_CNAV12_DATA_T;

/** raw message BDS CNAV3 ephemeris struct */
typedef struct _RM_EPH_BDS_CNAV3_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week number in BDS time (BDT) */
	unsigned int    SatStatus;  /**< Satellite Status Information */
	unsigned int	TOE;		/**< Reference time of ephemeris parameters in BeiDou time (BDT). (s) */
	unsigned int    SatType;    /**< 2 bit orbit type */

	double          DeltaA;     /**< Semi-major axis difference at reference time (m) */
	double          ADot;       /**< Change rate in Semi-major axis (m/s) */
	double          DeltaN;     /**< Mean motion difference from computed value (radians/s) */
	double          NDot;       /**< Rate of Mean motion difference from computed value (radians/s/s) */
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Ecc;		/**< Eccentricity, dimensionless */
	double			Omega;	 	/**< Argument of perigee, radians*/
	double			Omega0;		/**< Longitude of ascending node of orbital of planeat weekly epoch, radians */
	double			I0;	        /**< Inclination angel at reference time, radians */
	double			OmegaDot;	/**< Rate of right ascension, radians/second */
	double			I0Dot;		/**< Rate if inclination angle, radians/second */

	double			Cis;		/**< Inclination (amplitude of sine, radians) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians) */
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/

	unsigned int	TOC;		/**< Reference time of clock parameters in BeiDou time (BDT).*/

	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/
	double          TGDB2Bi;    /**< Group delay differential between the B2bI and the B3I signal (s) */

	unsigned int    Reserved;

}_PACKED_ RM_EPH_BDS_CNAV3_DATA_T;

/** raw message GLONASS ephemeris struct, previous name: txxx_EPH_GLN_s */
typedef struct _RM_EPH_GLO_DATA_T
{
	unsigned short	PRN;		/**< slot information offset - PRN identification (Slot+38). This is also called SLOTO in CDU
									 based on MIN_GLONASS_RANGE_PRN (38) */
	unsigned short  FreqO;		/**< Freq Offset, 0~20 , FreqNum + 7*/
	unsigned char   SatType;	/**< Satellite Type where, 0--GLO_SAT, 1--GLO_SAT_M, 2--GLO_SAT_K*/
	unsigned char   Reserved1;  /**< Reserve d*/
	unsigned short  RefWeek;    /**< Reference Week of ephemeris (GPS Time)*/
	unsigned int    RefSec;     /**< Reference time of ephemeris (GPS Time) in ms*/
	unsigned int    OffsetT;    /**< Integer seconds between GPS and GLONASS time, GLONASST-GPST*/
	unsigned short  Nt;         /**< Current Data number only for M type satellite.*/
	unsigned char   Reserved2;  /**< Reserved*/
	unsigned char   Reserved3;  /**< Reserved*/
	unsigned int    Issue;      /**< 15-minute interval number corresponding to ephemeris reference time*/
	unsigned int    Health;     /**< Ephemeris Health where, 0 = GOOD, 1 = BAD*/
	double			PosX;       /**< X coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			PosY;       /**< Y coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			PosZ;       /**< Z coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			VelX;       /**< X coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			VelY;       /**< Y coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			VelZ;       /**< Z coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			LSAccX;     /**< X coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			LSAccY;     /**< Y coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			LSAccZ;     /**< Z coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			TauN;       /**< Correction to the nth satellite time t_n relative to GLONASS time t_c, in seconds*/
	double			DeltaTauN;  /**< Time difference between navigation RF signal transmitted in L2 sub-band and*/
								/**< navigation RF signal transmitted in L1 sub-band  by nth satellite, in seconds */
	double			Gamma;      /**< Frequency correction, in seconds/second*/
	unsigned int    Tk;         /**< Time of frame start(since start of GLONASS day), in seconds*/
	unsigned int    P;          /**< Technological parameter*/
	unsigned int    Ft;         /**< User range*/
	unsigned int    Age;        /**< Age of data, in days*/
	unsigned int    Flags;      /**< Information flags*/
}_PACKED_ RM_EPH_GLO_DATA_T;

/** raw message Galileo ephemeris struct, previous name: txxx_EPH_GAL_s */
typedef struct _RM_EPH_GAL_DATA_T
{
	unsigned int	PRN;            /**< satellite PRN number, based on MIN_GALILEO_RANGE_PRN(1)*/

	unsigned int 	FNavReceived;   /**< Indicates F/NAV ephemeris data received*/
	unsigned int 	INavReceived;   /**< Indicates I/NAV ephemeris data received*/

	unsigned char 	E1BHealth;      /**< E1B health status bits (only valid if INavReceived is TRUE)*/
	unsigned char 	E5aHealth;      /**< E5a health status bits (only valid if FNavReceived is TRUE)*/
	unsigned char 	E5bHealth;      /**< E5b health status bits (only valid if INavReceived is TRUE)*/

	unsigned char 	E1BDVS;         /**< E1B data validity status (only valid if INavReceived is TRUE)*/
	unsigned char 	E5aDVS;         /**< E5a data validity status (only valid if FNavReceived is TRUE)*/
	unsigned char 	E5bDVS;         /**< E5b data validity status (only valid if INavReceived is TRUE)*/

	unsigned char 	SISA;           /**< Signal in space accuracy*/
	unsigned char 	Received;       /**< Received*/
	unsigned int	IODNav;         /**< Issue of data ephemeris*/

	unsigned int	T0e;            /**< Ephemeris reference time (s)*/
	double			RootA;          /**< Square root of semi-major axis*/
	double			DeltaN;         /**< Mean motion difference (radians/s)*/
	double			M0;             /**< Mean anomaly at ref time (radians)*/
	double			Ecc;            /**< Eccentricity (unitless)*/
	double			Omega;          /**< Argument of perigee (radians)*/

	double			Cuc;            /**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)*/
	double			Cus;            /**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)*/
	double			Crc;            /**< Amplitude of the cosine harmonic correction term to the orbit radius (m)*/
	double			Crs;            /**< Amplitude of the sine harmonic correction term to the orbit radius (m)*/
	double			Cic;            /**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)*/
	double			Cis;            /**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)*/

	double			I0;             /**< Inclination angle at ref time (radians)*/
	double			IDot;           /**< Rate of inclination angle (radians/s)*/
	double			Omega0;         /**< Longitude of ascending node of orbital plane at weekly epoch (radians)*/
	double			OmegaDot;       /**< Rate of right ascension (radians/s)*/
	signed int      FNavT0c;        /**< Clock correction data reference time of week from the F/NAV message (s). Only valid if FNavReceived is TRUE*/
	double			FNavAf0;        /**< SV clock bias correction coefficient from the F/NAV message (s). Only valid if FNavReceived is TRUE*/
	double			FNavAf1;        /**< SV clock drift correction coefficient from the F/NAVmessage (s/s). Only valid if FNavReceived is TRUE*/
	double			FNavAf2;        /**< SV clock drift rate correction coefficient from the F/NAV message (s/s^2). Only valid if FNAVReceivedis TRUE*/
	signed int      INavT0c;        /**< Clock correction data reference time of week from the I/NAV message (s). Only valid if INavReceived is TRUE*/
	double			INavAf0;        /**< SV clock bias correction coefficient from the I/NAV message (s). Only valid if INavReceived is TRUE*/
	double			INavAf1;        /**< SV clock drift correction coefficient from the I/NAVmessage (s/s). Only valid if INavReceived is TRUE*/
	double			INavAf2;        /**< SV clock drift rate correction coefficient from the I/NAV message (s/s^2). Only valid if INAVReceivedis TRUE*/

	double			E1E5aBGD;       /**< E1, E5a broadcast group delay*/
	double			E1E5bBGD;       /**< E1, E5b broadcast group delay. Only valid if INavReceived is TRUE*/

	unsigned int    Week;           /**< Reference week of ephemeris (GPS reference time)*/
}_PACKED_ RM_EPH_GAL_DATA_T;

/** raw message Galileo INAV ephemeris struct */
typedef struct _RM_EPH_GAL_INAV_DATA_T
{
	unsigned int	PRN;            /**< satellite PRN number, based on MIN_GALILEO_RANGE_PRN(1)*/

	unsigned char 	E5bHealth;      /**< E5b health status bits  */
	unsigned char 	E5bDVS;         /**< E5b data validity status */
	unsigned char   Reserved0;
	unsigned char   Reserved1;

	unsigned char 	E1bHealth;      /**< E1b health status bits */
	unsigned char 	E1bDVS;         /**< E1b data validity status */
	unsigned char   Reserved2;
	unsigned char   Reserved3;

	unsigned short  IODNav;         /**< Issue of data ephemeris */
	unsigned char 	SISA;           /**< SISA index, Signal in space accuracy(unitless) */
	unsigned char   INavSource;     /**< Identifies the source signal, 0 -- unknown, 1 -- E1b, 2 -- E5b, 3 -- E1b and E5b */

	unsigned int	T0e;            /**< Ephemeris reference time (s)*/
	unsigned int	T0c;            /**< Clock correction data reference time of week from the I/NAV message (s).*/

	double			M0;             /**< Mean anomaly at ref time (radians)*/
	double			DeltaN;         /**< Mean motion difference (radians/s)*/
	double			Ecc;            /**< Eccentricity (unitless)*/
	double			RootA;          /**< Square root of semi-major axis*/
	double			I0;             /**< Inclination angle at ref time (radians)*/
	double			IDot;           /**< Rate of inclination angle (radians/s)*/
	double			Omega0;         /**< Longitude of ascending node of orbital plane at weekly epoch (radians)*/
	double			Omega;          /**< Argument of perigee (radians)*/
	double			OmegaDot;       /**< Rate of right ascension (radians/s)*/

	double			Cuc;            /**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)*/
	double			Cus;            /**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)*/
	double			Crc;            /**< Amplitude of the cosine harmonic correction term to the orbit radius (m)*/
	double			Crs;            /**< Amplitude of the sine harmonic correction term to the orbit radius (m)*/
	double			Cic;            /**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)*/
	double			Cis;            /**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)*/

	double			Af0;            /**< SV clock bias correction coefficient from the I/NAV message (s) */
	double			Af1;            /**< SV clock drift correction coefficient from the I/NAVmessage (s/s) */
	double			Af2;            /**< SV clock drift rate correction coefficient from the I/NAV message (s/s^2) */

	double			E1E5aBGD;       /**< E1, E5a broadcast group delay */
	double			E1E5bBGD;       /**< E1, E5b broadcast group delay */
}_PACKED_ RM_EPH_GAL_INAV_DATA_T;

/** raw message Galileo FNAV ephemeris struct */
typedef struct _RM_EPH_GAL_FNAV_DATA_T
{
	unsigned int	PRN;            /**< satellite PRN number, based on MIN_GALILEO_RANGE_PRN (1)*/

	unsigned char 	E5aHealth;      /**< E5a health status bits */
	unsigned char 	E5aDVS;         /**< E5a data validity status */
	unsigned char   Reserved0;
	unsigned char   Reserved1;

	unsigned short  IODNav;         /**< Issue of data ephemeris */
	unsigned char 	SISA;           /**< SISA index, Signal in space accuracy(unitless) */
	unsigned char   Reserved2;

	unsigned int	T0e;            /**< Ephemeris reference time (s)*/
	unsigned int	T0c;            /**< Clock correction data reference time of week from the F/NAV message (s).*/

	double			M0;             /**< Mean anomaly at ref time (radians)*/
	double			DeltaN;         /**< Mean motion difference (radians/s)*/
	double			Ecc;            /**< Eccentricity (unitless)*/
	double			RootA;          /**< Square root of semi-major axis*/
	double			I0;             /**< Inclination angle at ref time (radians)*/
	double			IDot;           /**< Rate of inclination angle (radians/s)*/
	double			Omega0;         /**< Longitude of ascending node of orbital plane at weekly epoch (radians)*/
	double			Omega;          /**< Argument of perigee (radians)*/
	double			OmegaDot;       /**< Rate of right ascension (radians/s)*/

	double			Cuc;            /**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)*/
	double			Cus;            /**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)*/
	double			Crc;            /**< Amplitude of the cosine harmonic correction term to the orbit radius (m)*/
	double			Crs;            /**< Amplitude of the sine harmonic correction term to the orbit radius (m)*/
	double			Cic;            /**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)*/
	double			Cis;            /**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)*/

	double			Af0;            /**< SV clock bias correction coefficient from the F/NAV message (s) */
	double			Af1;            /**< SV clock drift correction coefficient from the F/NAVmessage (s/s) */
	double			Af2;            /**< SV clock drift rate correction coefficient from the F/NAV message (s/s^2) */

	double			E1E5aBGD;       /**< E1, E5a broadcast group delay */
}_PACKED_ RM_EPH_GAL_FNAV_DATA_T;

/** raw message NavIC ephemeris struct, previous name: txxx_EPH_GAL_s */
typedef struct _RM_EPH_NIC_DATA_T
{
	unsigned int	PRN;			/**< Satellite Identifier, based on MIN_NAVIC_RANGE_PRN (1) */
	unsigned int	Week;			/**< Week number since the NavIC system time start epoch (August 22nd 1999)*/
	double			Af0;			/**< Clock bias (s)   */
	double			Af1;			/**< Clock drift (s/s) */
	double			Af2;			/**< Clock drift rate (s/s2)   */
	unsigned int    URA;			/**< SV Accuracy  */

	unsigned int	toc;			/**< Reference time for the satellite clock corrections (s)    */
	double			TGD;			/**< Total group delay (s)   */
	double			DN;				/**< Mean motion difference (radian/s)     */
	unsigned int	IODEC;			/**< Issue of data ephemeris and clock*/
	unsigned int	Reserved;
	unsigned int	L5Health;		/**< Health status of navigation data on L5 SPS signal0=OK; 1=bad     */
	unsigned int	SHealth;		/**< Health status of navigation data on S SPS signal0=OK; 1=bad  */

	double			Cuc;			/**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)  */
	double			Cus;			/**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)    */
	double			Cic;			/**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)    */
	double			Cis;			/**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)   */
	double			Crc;			/**< Amplitude of the cosine harmonic correction term to the orbit radius (m)  */
	double			Crs;			/**< Amplitude of the sine harmonic correction term to the orbit radius (m)  */

	double			IDot;			/**< Rate of inclination angle (radians/s)   */
	unsigned int	Spare1;			/**<      */
	double			M0;				/**< Mean anomaly (radians)    */
	unsigned int	Toe;			/**< Time of ephemeris (s)    */
	double			Ecc;			/**< Eccentricity (dimensionless)  */
	double			RootA;			/**< Square root of semi-major axis (sqrt(m))     */
	double			Omega0;			/**< Longitude of ascending node (radians)     */
	double			Omega;			/**< Argument of perigee (radians) */
	double			OmegaDot;		/**< Rate of RAAN (radians/s)     */
	double			I0;				/**< Inclination angle (radians)  */

	unsigned int	Spare2;			/**<      */
	unsigned int	AlertFlag;		/**< The utilization of navigation data shall be at the users own risk.1=Alert; 0=OK    */
	unsigned int	AutoNavFlag;	/**< When set to 1, satellite is in AutoNav mode. Satellite broadcasts primary navigation
									parameters from AutoNav data sets with no uplink from ground for maximum of 7 days   */
}_PACKED_ RM_EPH_NIC_DATA_T;

/** raw message IONUTC data struct, previous name: txxx_IONUTC_s */
typedef struct _RM_IONUTC_DATA_T
{
	double Alpha0;
	double Alpha1;
	double Alpha2;
	double Alpha3;
	double Beta0;
	double Beta1;
	double Beta2;
	double Beta3;
	unsigned int utc_wn;
	unsigned int tot;
	double A0;
	double A1;
	unsigned int wn_lsf;
	unsigned int dn;
	int deltat_ls;
	int deltat_lsf;
	unsigned int Deltat_utc;
}_PACKED_ RM_IONUTC_DATA_T;

/** raw message BESTDOP/PSRDOP/RTKDOP data struct, previous name: txxx_BESTDOP_s, RM_BESTDOP_DATA_T */
typedef struct _RM_DOP_DATA_T
{
	float GDOP;		/**< Geometric dilution of precision */
	float PDOP;		/**< Position dilution of precision */
	float HDOP;		/**< Horizontal dilution of precision */
	float HTDOP;	/**< Horizontal position and time dilution of precision */
	float TDOP;		/**< GPS time dilution of precision */
	float ElevMask;	/**< GPS elevation mask angle */
	unsigned int SatNum;		/**< Number of satellites to follow */
	unsigned int SatTab[RM_BESTDOP_SAT_NUM_MAX];	/**< Satellites in use at time of calculation */
}_PACKED_ RM_DOP_DATA_T;

/** raw message BESTPOS/PSRPOS/RTKPOS/PPPPOS data struct, previous name: txxx_BESTPOS_s,RM_BESTPOS_DATA_T */
typedef struct _RM_POS_DATA_T
{
	int SolStatus;					/**< Solution status */
	int PosType;					/**< Position type */
	double Lat;						/**< Latitude (unit: degrees) */
	double Lon;						/**< Longitude (unit: degrees) */
	double Hgt;						/**< Height above mean sea level (unit: metres) */
	float Undulation;				/**< Undulation - the relationship between the geoid and the ellipsoid (m) of the chosen datum */
	int DatumID;					/**< Datum ID number */
	float LatStd;					/**< Latitude standard deviation (unit: metres) */
	float LonStd;					/**< Longitude standard deviation (unit: metres) */
	float HgtStd;					/**< Height standard deviation (unit: metres) */
	char StationID[4];				/**< Base station ID */
	float DiffAge;					/**< Differential age in seconds */
	float SolAge;					/**< Solution age in seconds */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char SolL1SVs;			/**< Number of satellites with L1/E1/B1 signals used in solution */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned char Reserved;
	unsigned char ExtSolStatus;		/**< Extended solution status
										 bit 0, 1 bits, If an RTK solution: an RTK solution has been verified
													If a PDP solution: solution is GLIDE
													Otherwise: Reserved
										bit 1 - 3, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 4, 1 bits, RTK ASSIST active
										bit 5, 1 bits, 0 = No antenna warning
													1 = Antenna information is missing
										bit 6, 1 bits, Reserved
										bit 7, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections*/
	unsigned char GALBDSSignalMask;	/**< Galileo and BeiDou signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, BeiDou B1 used in Solution (B1I and B1C)
										bit 5, 1 bits, BeiDou B2 used in Solution (B2I, B2a and B2b)
										bit 6, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 7, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
}_PACKED_ RM_POS_DATA_T;

/** raw message BESTVEL/PSRVEL/RTKVEL data struct, previous name: txxx_BESTVEL_s,RM_BESTVEL_DATA_T */
typedef struct _RM_VEL_DATA_T
{
	int SolStatus;	/**< Solution status */
	int VelType;	/**< Velocity type */
	float Latency;	/**< A measure of the latency in the velocity time tag in seconds.
						It should be subtracted from the time to give improved results (s) */
	float DiffAge;	/**< Differential age in seconds */
	double HorSpd;	/**< Horizontal speed over ground, in metres per second */
	double TrkGnd;	/**< Actual direction of motion over ground (track over ground) with respect to True North, in degrees */
	double VertSpd;	/**< Vertical speed, in metres per second, where positive values
						indicate increasing altitude (up) and negative values indicate decreasing altitude (down) */
	float Reserved;
}_PACKED_ RM_VEL_DATA_T;

/** raw message BESTXYZ data struct, previous name: txxx_BESTXYZ_s */
typedef struct _RM_BESTXYZ_DATA_T
{
	int PosSolStatus;				/**< position solution status */
	int PosType;					/**< Position type */
	double PosX;					/**< Position X-coordinate (m) */
	double PosY;					/**< Position Y-coordinate (m) */
	double PosZ;					/**< Position Z-coordinate (m) */
	float PosStdX;					/**< Standard deviation of P-X (m) */
	float PosStdY;					/**< Standard deviation of P-X (m) */
	float PosStdZ;					/**< Standard deviation of P-X (m) */
	int VelSolStatus;				/**< Solution status */
	int VelType;					/**< Velocity type */
	double VelX;					/**< Velocity vector along X-axis (m/s) */
	double VelY;					/**< Velocity vector along Y-axis (m/s) */
	double VelZ;					/**< Velocity vector along Z-axis (m/s) */
	float VelStdX;					/**< Standard deviation of V-X (m/s) */
	float VelStdY;					/**< Standard deviation of V-Y (m/s) */
	float VelStdZ;					/**< Standard deviation of V-Z (m/s) */
	char StationID[4];				/**< Base station identification */
	float Vlatency;					/**< A measure of the latency in the velocity time tag in seconds.
										It should be subtracted from the time to give improved results */
	float DiffAge;					/**< Differential age in seconds */
	float SolAge;					/**< Solution age in seconds */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char SolL1SVs;			/**< Number of satellites with L1/E1/B1 signals used in solution */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	char Reserved;
	unsigned char ExtSolStatus;		/**< Extended solution status
										bit 0, 1 bits, If an RTK solution: an RTK solution has been verified
													If a PDP solution: solution is GLIDE
													Otherwise: Reserved
										bit 1 - 3, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 4, 1 bits, RTK ASSIST active
										bit 5, 1 bits, 0 = No antenna warning
													1 = Antenna information is missing
										bit 6, 1 bits, Reserved
										bit 7, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections*/
	unsigned char GALBDSSignalMask;	/**< Galileo and BeiDou signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, BeiDou B1 used in Solution (B1I and B1C)
										bit 5, 1 bits, BeiDou B2 used in Solution (B2I, B2a and B2b)
										bit 6, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 7, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
}_PACKED_ RM_BESTXYZ_DATA_T;

typedef struct _RM_BESTSATS_CELL_DATA_T
{
	int System;					/**< Satellite System */
	unsigned int SatelliteID;	/**< Satellite ID */
	int Status;					/**< Satellite status */
	unsigned int SignalMask;	/**< Satellite Signal mask */
} RM_BESTSATS_CELL_DATA_T;

/** raw message BESTSATS data struct, previous name: txxx_BESTSATS_s */
typedef struct _RM_BESTSATS_DATA_T
{
	unsigned int SatNum;		/**< Satellite number */
	RM_BESTSATS_CELL_DATA_T BestSats[RM_BESTSATS_NUM_MAX];
}_PACKED_ RM_BESTSATS_DATA_T;

typedef struct _RM_SATVIS2_CELL_DATA_T
{
	unsigned int SatID;			/**< Satellites in use at time of calculation */
	unsigned int Health;		/**< Satellite health as defined in ICD */
	double Elevation;			/**< Elevation (degrees) */
	double Azimuth;				/**< Azimuth (degrees) */
	double TrueDoppler;			/**< Theoretical Doppler of satellite - the expected Doppler frequency based on a satellite's motion relative to the receiver.
								It is computed using the satellite's coordinates and velocity along with the receiver's coordinates and velocity (Hz). */
	double ApparentDoppler;		/**< Apparent Doppler for this receiver - the same as Theoretical Doppler above but with clock drift correction added (Hz). */
} RM_SATVIS2_CELL_DATA_T;

/** raw message SATVIS2 data struct, previous name: txxx_SATVIS2_s */
typedef struct _RM_SATVIS2_DATA_T
{
	int System;					/**< GNSS satellite system identifier */
	int SatVis;					/**< Satellite visibility valid */
	int AlmanacFlag;			/**< Complete almanac used */
	unsigned int SatNum;		/**< complete almanac used */
	RM_SATVIS2_CELL_DATA_T SatVis2CellData[RM_SATVIS2_SAT_NUM_MAX];
}_PACKED_ RM_SATVIS2_DATA_T;

/** raw message TIME data struct, previous name: txxx_TIME_s */
typedef struct _RM_TIME_DATA_T
{
	int ClockStatus;		/**< Clock model status (not including current measurement data) */
	double Offset;			/**< Receiver clock offset in seconds from GPS system time */
	double OffsetStd;		/**< Receiver clock offset standard deviation (s) */
	double UTCOffset;		/**< The offset of GPS system time from UTC time  */
	unsigned int Year;		/**< UTC year */
	unsigned char Month;	/**< UTC month (0-12) */
	unsigned char Day;		/**< UTC day (0-31) */
	unsigned char Hour;		/**< UTC hour (0-23) */
	unsigned char Minu;		/**< UTC minute (0-59) */
	unsigned int MSec;		/**< UTC millisecond (0-60999) */
	int UTCStatus;			/**< UTC status */
}_PACKED_ RM_TIME_DATA_T;

/** raw message REFSTATION data struct, previous name: txxx_REFSTATION_s */
typedef struct _RM_REFSTATION_DATA_T
{
	unsigned int Status;		/**< Status of the base station information */
	double ECEFX;				/**< ECEF X value (m) */
	double ECEFY;				/**< ECEF Y value (m) */
	double ECEFZ;				/**< ECEF Z value (m) */
	unsigned int Health;		/**< Base station health*/
	unsigned int StationType;	/**< Station type */
	char StationID[4];			/**< Base station ID */
}_PACKED_ RM_REFSTATION_DATA_T;

/** raw message REFSTATIONINFO data struct, previous name: txxx_REFSTATIONINFO_s */
typedef struct _RM_REFSTATIONINFO_DATA_T
{
	double Latitude;			/**< Latitude (degrees) */
	double Longitude;			/**< Longitude (degrees) */
	double Height;				/**< Ellipsoidal Height (m) */
	unsigned int DatumID;		/**< Datum ID number */
	float ARPHeight;			/**< Base Antenna ARP (m) */
	unsigned int Health;		/**< Base station health*/
	unsigned int StationType;	/**< Station type */
	char StationID[4];			/**< Base station ID */
	char AntModel[32];			/**< Base Antenna Model Name */
	char AntSerial[32];			/**< Base Antenna Serial Number */
}_PACKED_ RM_REFSTATIONINFO_DATA_T;

typedef struct _RM_RTCMINSTATUS_MSG_INFO_T
{
	unsigned int MsgID;		/**< input message id */
	unsigned int MsgNum;	/**< input message number */
}_PACKED_ RM_RTCMINSTATUS_MSG_INFO_T;

typedef struct _RM_RTCMINSTATUS_PORT_DATA_T
{
	unsigned int RTCMPortID;		/**< RTCM input port id */
	unsigned int MsgTypeNum;		/**< RTCM input port message type number */
	RM_RTCMINSTATUS_MSG_INFO_T MsgInfo[RTCM_IN_STATUS_NUM_MAX];	/**< RTCM input port receiver message information*/
}_PACKED_ RM_RTCMINSTATUS_PORT_DATA_T;

/** raw message RTCMINSTATUS data struct, previous name: txxx_RTCMINSTATUS_s */
typedef struct _RM_RTCMINSTATUS_DATA_T
{
	unsigned int RTCMPortNum;							/**< RTCM input port number */
	RM_RTCMINSTATUS_PORT_DATA_T PortMsgData[RTCM_INPUT_PORT_NUM_MAX];		/**< RTCM input port message data */
}_PACKED_ RM_RTCMINSTATUS_DATA_T;

/** raw message RAWSBASFRAME2 data struct, previous name: txxx_RAWSBASFRAME2_s */
typedef struct _RM_RAWSBASFRAME2_DATA_T
{
	unsigned int   PRN;				/**< SBAS satellite PRN number */
	unsigned int   SignalChannel;	/**< Signal channel number that the frame was decoded on */
	unsigned char  SignalSource;	/**< Identifies the source of the SBAS signal */
	unsigned char  PreambleType;	/**< Identifies what preamble was used when tracking the SBAS signal */
	unsigned short Reserved;		/**< Reserved */
	unsigned int   FrameID;			/**< SBAS frame ID*/
	unsigned char  FrameData[32];	/**< Raw SBAS frame data. There are 226 bits of data and 6 bits of padding */
}_PACKED_ RM_RAWSBASFRAME2_DATA_T;

/** raw message GALCNAVRAWPAGE data struct */
typedef struct _RM_GALCNAVRAWPAGE_DATA_T
{
	unsigned int   SignalChannel;	/**< Signal channel number providing the bits */
	unsigned int   PRN;				/**< GAL satellite PRN number, based on 1 */
	unsigned short MessageID;       /**< ID of the message */
	unsigned short PageID;          /**< ID of the transmitted page of the encoded message */
	unsigned char  FrameData[58];	/**< Raw GAL CNAV frame data. 462 bits, dones not include CRC or Tail bits */
}_PACKED_ RM_GALCNAVRAWPAGE_DATA_T;

/** raw message RAWIMUX data struct, previous name: txxx_RAWIMUX_s */
typedef struct _RM_RAWIMUX_DATA_T
{
	unsigned char	IMUInfo;		/**< IMU Info Bits
										Bit 0: If set, an IMU error was detected. Check the IMU Status field for details.
										Bit 1: If set, the IMU data is encrypted and should not be used.
										Bits 2 to 7: Reserved */
	unsigned char	IMUType;		/**< IMU Type */
	unsigned short	Week;			/**< GPS week */
	double			Seconds;		/**< Seconds from week start */
	unsigned int	IMUStatus;		/**< IMU Status */
	int				ZAccel;			/**< Change in velocity count along z axis*/
	int				YAccel;			/**< - (Change in velocity count along y axis) */
	int				XAccel;			/**< Change in velocity count along x axis*/
	int				ZGyro;			/**< Change in angle count along z axis Right-handed */
	int				YGyro;			/**< -(Change in angle count along y axis Right-handed) */
	int				XGyro;			/**< Change in angle count along x axis Right-handed */
}_PACKED_ RM_RAWIMUX_DATA_T;

/** raw message INSPVA data struct, previous name: txxx_INSPVA_s */
typedef struct _RM_INSPVA_DATA_T
{
	unsigned int	Week;			/**< GPS week */
	double			Seconds;		/**< Seconds from week start */
	double			Latitude;		/**< Latitude(unit: degrees) */
	double			Longitude;		/**< Longitude(unit: degrees) */
	double			Height;			/**< Height(unit: meter) */

	double			VelNorth;		/**< Velocity in a northerly direction (a negative value implies a southerly direction) [m/s] */
	double			VelEast;		/**< Velocity in an easterly direction (a negative value implies a westerly direction) [m/s] */
	double			VelUp;			/**< Velocity in an up direction [m/s] */

	double			Roll;			/**< Right-handed rotation from local level around y-axis in degrees */
	double			Pitch;			/**< Right-handed rotation from local level around x-axis in degrees */
	double			Azimuth;		/**< Left-handed rotation around z-axis in degrees clockwise from North
										This is the inertial azimuth calculated from the IMU gyros and the SPAN filters. */

	unsigned int	INSStatus;		/**< INS Status */
}_PACKED_ RM_INSPVA_DATA_T;

/** raw message INSPVAX data struct */
typedef struct _RM_INSPVAX_DATA_T
{
	unsigned int INSStatus;		/**< solution status */
	unsigned int PosType;		/**< position type */
	double Latitude;			/**< degree */
	double Longitude;			/**< degree */
	double Height;				/**< m */
	float Undulation;			/**< m, undulation */
	double NorthVelocity;		/**< m/s */
	double EastVelocity;		/**< m/s */
	double UpVelocity;			/**< m/s */
	double Roll;				/**< degree */
	double Pitch;				/**< degree */
	double Azimuth;				/**< degree */
	float LatitudeStd;			/**< m, latitude standard deviation */
	float LongitudeStd;			/**< m, longitude standard deviation */
	float HeightStd;			/**< m, height standard deviation */
	float NorthVelStd;			/**< m/s, north velocity standard deviation */
	float EastVelStd;			/**< m/s, east velocity standard deviation */
	float UpVelStd;				/**< m/s, up velocity standard deviation */
	float RollStd;				/**< degree, roll standard deviation */
	float PitchStd;				/**< degree, pitch standard deviation */
	float AzimuthStd;			/**< degree, yaw standard deviation */
	unsigned int ExtSolStat;	/**< extended solution status */
	unsigned short ElapsedTime; /**< s, elapsed time since the last ZUPT or position update */
}_PACKED_ RM_INSPVAX_DATA_T;

typedef struct _RM_DUALANTENNAHEADING_DATA_T
{
	int SolStatus;					/**< Solution status */
	int PosType;					/**< Position type */
	float Length;                   /**< Baseline length in meters */
	float Heading;                  /**< Heading in degrees(0-359.999) */
	float Pitch;                    /**< Pitch(+-90 degrees)*/
	float Reserved;                 /**< Reserved */  
	float HdgStdDev;                /**< Heading standard deviation in degrees */
	float PtchStdDev;               /**< Pitch standard deviation in degrees */
	char StnID[4];                  /**< Station ID string */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char ObsNum;           /**< Number of satellites above the elevation mask angle */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned char SolSource;        /**< Solution source */
	unsigned char ExtSolStatus;		/**< Extended solution status
										 bit 0, 1 bits, If an RTK solution: an RTK solution has been verified
													If a PDP solution: solution is GLIDE
													Otherwise: Reserved
										bit 1 - 3, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 4, 1 bits, RTK ASSIST active
										bit 5, 1 bits, 0 = No antenna warning
													1 = Antenna information is missing
										bit 6, 1 bits, Reserved
										bit 7, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections*/
	unsigned char GALBDSSignalMask;	/**< Galileo and BeiDou signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, BeiDou B1 used in Solution (B1I and B1C)
										bit 5, 1 bits, BeiDou B2 used in Solution (B2I, B2a and B2b)
										bit 6, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 7, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
} _PACKED_ RM_DUALANTENNAHEADING_DATA_T;

#define RAWRTCMAUX_DATA_BUF_MAX_SIZE		(1024+8)

/** raw message RAWRTCMAUX data struct, previous name: txxx_RAWRTCMAUX_s */
typedef struct _RM_RAWRTCMAUX_BODY_HEADER_T
{
	char StationID[4];			/**< Base station ID */
	unsigned int MsgID;			/**< Message ID */
	unsigned int Reserved[4];	/**< Reserved */
	unsigned int MsgType;		/**< Message type */
}_PACKED_ RM_RAWRTCMAUX_BODY_HEADER_T;

typedef struct _RM_RAWRTCMAUX_BODY_HEX_DATA_T
{
	unsigned int DataLen;
	unsigned char DataBuf[RAWRTCMAUX_DATA_BUF_MAX_SIZE];
}_PACKED_ RM_RAWRTCMAUX_BODY_HEX_DATA_T;

typedef struct _RM_RAWRTCMAUX_BODY_ASCII_DATA_T
{
	char DataBuf[RAWRTCMAUX_DATA_BUF_MAX_SIZE];
}_PACKED_ RM_RAWRTCMAUX_BODY_ASCII_DATA_T;

typedef struct _RM_BDSIONO_DATA_T
{
	double Alpha0; /**< Alpha parameter constant term */
	double Alpha1; /**< Alpha parameter 1st order term */
	double Alpha2; /**< Alpha parameter 2nd order term */
	double Alpha3; /**< Alpha parameter 3rd order term */
	double Beta0; /**< Beta parameter constant term */
	double Beta1; /**< Beta parameter 1st order term */
	double Beta2; /**< Beta parameter 2nd order term */
	double Beta3; /**< Beta parameter 3rd order term */
}_PACKED_ RM_BDSIONO_DATA_T;

typedef struct _MARK_MSG_TIME_T
{
	unsigned int Week;   /**< GPS reference week number */
	double Seconds;      /**< Seconds into the week as measured from the receiver clock,
							  coincident with the time of electricalclosure on the Mark Input port */


	double OffSet;       /**< Receiver clock offset, in seconds,
							  A positive offset implies that the receiver clock is ahead of GPS system time.
							  To derive GPS system time, use the following formula:
							  GPS system time = GPS reference time - (offset)
							  Where GPS reference time can be obtained from the log header */
	double OffSetStd;    /**< Standard deviation of receiver clock offset (s) */

	double UTCOffSet;    /**< This field represents the offset of GPS system time from UTC time (s),
							  computed using almanac parameters.
						  	  UTC time is GPS reference time plus the current UTC offset minus the receiver clock offset.
							  UTC time = GPS reference time - offset + UTC offset */

	RM_CLOCK_MODEL_STATUS_E ClockModelStatus; /**< Clock model status */
}_PACKED_ MARK_MSG_TIME_T;

typedef struct _RM_TILTCONFIG_DATA_T
{
	unsigned char TiltMode;	/**< tilt mode */
	unsigned char IMUType;	/**< IMU type */
	unsigned short Reserved1;

	float PoleHeight;		/**< pole height (unit: m) */

	float LeverArmX;		/**< X-direction lever arm (unit: m) */
	float LeverArmY;		/**< Y-direction lever arm (unit: m) */
	float LeverArmZ;		/**< Z-direction lever arm (unit: m) */

	float InstallAngleX;	/**< X-direction install angle (unit: deg) */
	float InstallAngleY;	/**< Y-direction install angle (unit: deg) */
	float InstallAngleZ;	/**< Z-direction install angle (unit: deg) */

	float GyoBiasX;			/**< X-direction gyo bias (unit: deg/s) */
	float GyoBiasY;			/**< Y-direction gyo bias (unit: deg/s) */
	float GyoBiasZ;			/**< Z-direction gyo bias (unit: deg/s) */

	float AccBiasX;			/**< X-direction acc bias (unit: mg) */
	float AccBiasY;			/**< Y-direction acc bias (unit: mg) */
	float AccBiasZ;			/**< Z-direction acc bias (unit: mg) */

	float AlignVel;			/**< alignment velocity */

	unsigned int Reserved[6];
}_PACKED_ RM_TILTCONFIG_DATA_T;

typedef struct _RM_TILTCALIB_DATA_T
{
	signed char CalibProgress;	/**< Calib Progress */
	signed char CalibStatus;	/**< Calib Status */
	unsigned short Reserved1;

	float InstallAngleX;		/**< X-direction install angle (unit: deg) */
	float InstallAngleY;		/**< Y-direction install angle (unit: deg) */
	float InstallAngleZ;		/**< Z-direction install angle (unit: deg) */

	float GyoBiasX;				/**< X-direction gyo bias (unit: deg/s) */
	float GyoBiasY;				/**< Y-direction gyo bias (unit: deg/s) */
	float GyoBiasZ;				/**< Z-direction gyo bias (unit: deg/s) */

	float AccBiasX;				/**< X-direction acc bias (unit: mg) */
	float AccBiasY;				/**< Y-direction acc bias (unit: mg) */
	float AccBiasZ;				/**< Z-direction acc bias (unit: mg) */

	float Azimuth;				/**< Azimuth */
	float AccuracyFactor;		/**< Accuracy Factor */

	unsigned int Reserved[6];
}_PACKED_ RM_TILTCALIB_DATA_T;

typedef struct _RM_ANTISPOOFSTATUS_DATA_T
{
	unsigned int OSNMAStatus;			/**< OSNMA Status */
	unsigned int Reserved1;				/**< Reserved1 */

	unsigned short SpoofExistFlag;		/**< Spoof exist flag */
	unsigned short ConfidenceRatio;		/**< Confidence Ratio */

	unsigned short SpoofDetectMode;		/**< Spoof detect mode */
	unsigned short FirstDetectedWeek;	/**< First detected week */

	unsigned int FirstDetectedTOW;		/**< First detected tow */
	unsigned int DetectSatNum;			/**< detected satellite number */
	unsigned int SpoofSatNum;			/**< Spoof satellite number */

	UINT64 GPSSpoofSatMask;				/**< GPS spoof satellite mask */
	UINT64 GLOSpoofSatMask;				/**< GLO spoof satellite mask */
	UINT64 GALSpoofSatMask;				/**< GAL spoof satellite mask */
	UINT64 BDSSpoofSatMask;				/**< BDS spoof satellite mask */

	unsigned int Reserved2;				/**< Reserved1 */
}_PACKED_ RM_ANTISPOOFSTATUS_DATA_T;

typedef struct _SAT_FILTER_INFO_T
{
	unsigned char SatSystem;	/**< 0:GPS; 1:GLO; 2:GAL; 3:BDS; 4:SBA; 5:QZS; 6:NIC */
	unsigned char PRN;		  /**< 1 based satellite PRN */
	unsigned char AtmConstraintType;/**< bit0~3:ionosphere constraint type; bit4~7:troposphere constraint type */
	unsigned char AmbFixType;   /**< 0:float; 1:EWL fix; 2:WL fix; 3:NL fix */
} SAT_FILTER_INFO_T;

/** raw message FILTERDBG data struct, previous name: txxx_FILTERDBG_s */
typedef struct _RM_FILTERDBG_DATA_T
{
	unsigned char Version;				/**< Message version */
	unsigned char FilterID;				/**< 0/1: RTK filter; 10/11: Heading filter; 20/21: SWASPPP filter; 30/31: MGNSSPPP filter */
	unsigned char SceneType;			/**< 0:survey; 1.mower; 2:agriculture; 3:car; 4:UAV; 5:USV */
	unsigned char NumParam;				/**< number of filter state parameter */
	unsigned int ParamTypeMask;

	float VarXX;						/**< X variance (m^2) */
	float VarYY;						/**< Y variance (m^2) */
	float VarZZ;						/**< Z variance (m^2) */
	float VarXY;						/**< XY co-variance (m^2) */
	float VarYZ;						/**< YZ co-variance (m^2) */
	float VarZX;						/**< ZX co-variance (m^2) */

	unsigned char AtmProcessType;		/**< bit0~3:ionosphere option; bit4~7: troposphere option */
	unsigned char AtmIntegrityIonoStrategy;
	unsigned char EnvLevel;				/**< 0:unknown; 1:good; 2:fail; 3:poor; 4:bad */
	unsigned char FusionType;			/**< bit0~3:phase; bit4~7:code; 0x01:geo; 0x02:GF; 0x04:IF; 0x08:WL */

	unsigned int EventFlag;				/**< bit0:filter reset;
											bit1:ionosphere option changes
											bit2:troposphere option changes
											bit3:filter iteration exceed
											bit4:observation number exceed
											bit5:parameter number exceed
											bit6:WL constraint error
											bit7:numeric error
											bit8:dynamic memory error
											bit9~31:reserved */

	unsigned char EWLFixStrategy;		/**< EWL/WL/NL ambiguity fixed strategy */
	unsigned char WLFixStrategy;
	unsigned char NLFixStrategy;
	unsigned char FixQuality;			/**< fix quality, 0~100 */

	unsigned int AtmIntegrityInfo;		/**< 0bit: set 1 when bAtmIntegrity is true,
											1bit-4bit: integrity version (0: 2.0, 1: 3.0, 2: 4.0),
											5bit-10bit: integrity age,
											11bit-16bit: integrity num,
											17bit-21bit: ionosphere SWAS integrity info (17bit: integrity 3.0 iono info, 18bit: integrity 4.0 model info)
											22bit-26bit: troposphere SWAS integrity info (22bit: integrity 3.0 ZWD info, 23bit: integrity 3.0 trop info)
											27bit-29bit: ionosphere active level, see IONO_ACTIVE_?
											30bit: trop estimate flag */
	unsigned int Reserved[5];

	unsigned char RefSatPRN[4];			/**< 1 based reference satellite PRN, G/R/E/C */

	unsigned char Reserved2[3];
	unsigned char NumSat;

	SAT_FILTER_INFO_T SatInfos[64];	/**< Satellite filter information */
}_PACKED_ RM_FILTERDBG_DATA_T;

#define FILE_NAME_SIZE_MAX			(128u)
#define FILE_ERROR_MSG_SIZE_MAX		(128u)

typedef enum _RM_MASS_STORAGE_DEVICE_TYPE_E
{
	RM_MASS_STORAGE_DEVICE_TYPE_USBSTICK = 1,
	RM_MASS_STORAGE_DEVICE_TYPE_NO_STORAGE = 3,

	RM_MASS_STORAGE_DEVICE_TYPE_NUM_MAX,
} RM_MASS_STORAGE_DEVICE_TYPE_E;

typedef enum _RM_FILELIST_FILE_TYPE_E
{
	FILELIST_FILE_TYPE_NONE = 0,
	FILELIST_FILE_TYPE_FILE = 1,
	FILELIST_FILE_TYPE_DIR = 2,

	FILELIST_FILE_TYPE_NUM_MAX,
} RM_FILELIST_MSG_TYPE_E;

typedef enum _RM_FILE_STATUS_E
{
	FILE_STATUS_OPEN = 1,
	FILE_STATUS_CLOSED = 2,
	FILE_STATUS_BUSY = 3,
	FILE_STATUS_ERROR = 4,
	FILE_STATUS_COPY = 5,
	FILE_STATUS_PENDING = 6,

	FILE_STATUS_NUM_MAX,
} RM_FILE_STATUS_E;

typedef enum _RM_MASS_STORAGE_STATUS_E
{
	MASS_STORAGE_STATUS_UNMOUNTED = 0,
	MASS_STORAGE_STATUS_MOUNTED = 1,
	MASS_STORAGE_STATUS_BUSY = 2,
	MASS_STORAGE_STATUS_ERROR = 3,
	MASS_STORAGE_STATUS_MOUNTING = 4,
	MASS_STORAGE_STATUS_UNMOUNTING = 5,

	MASS_STORAGE_STATUS_NUM_MAX,
} RM_MASS_STORAGE_STATUS_E;

#define FILELIST_FILE_NUM_MAX		(512u)

typedef struct _RM_FILELIST_FILE_INFO_T
{
	unsigned int FileType;				/**< The type of entry for this log */
	unsigned int FileSize;				/**< File Size (in Bytes) */
	unsigned int ChangeDate;			/**< Date of the last change */
	unsigned int ChangeTime;			/**< Time of last change */
	unsigned int NameSize;				/**< File name size (include EOF) */
	char FileName[FILE_NAME_SIZE_MAX];	/**< Name of the file or directory File Name STRING */
}_PACKED_ RM_FILELIST_FILE_INFO_T;

typedef struct _RM_FILELIST_DATA_T
{
	unsigned int MassStorageDevice;		/**< Mass Storage Device */
	unsigned int FileNum;				/**< File number */
	RM_FILELIST_FILE_INFO_T FileInfo[FILELIST_FILE_NUM_MAX];
}_PACKED_ RM_FILELIST_DATA_T;

#define FILESTATUS_FILE_NUM_MAX		(3u)

typedef struct _RM_FILESTATUS_FILE_INFO_T
{
	unsigned int FileStatus;				/**< File status */
	char FileName[FILE_NAME_SIZE_MAX];		/**< Name of the file or directory File Name STRING */
	unsigned int FileSize;					/**< File Size (in Bytes) */
	unsigned int ErrorMsgSize;				/**< Error Message String Size */
	char ErrorMsg[FILE_ERROR_MSG_SIZE_MAX];	/**< Error Message String */
}_PACKED_ RM_FILESTATUS_FILE_INFO_T;

typedef struct _RM_FILESTATUS_DATA_T
{
	unsigned int MassStorageDevice;			/**< Mass Storage Device */
	unsigned int MediaRemainingCapacity;	/**< Remaining capacity on the storage media (kb) */
	unsigned int MediaTotalCapacity;		/**< Total capacity of the storage media (kb) */
	unsigned int FileNum;					/**< File number */
	RM_FILESTATUS_FILE_INFO_T FileInfo[FILESTATUS_FILE_NUM_MAX];
}_PACKED_ RM_FILESTATUS_DATA_T;

typedef struct _RM_FILESYSTEMSTATUS_DATA_T
{
	unsigned int MassStorageDevice;			/**< Mass Storage Device */
	unsigned int MassStorageStatus;			/**< Media Status */
	unsigned int MediaRemainingCapacity;	/**< Remaining capacity on the storage media (kb) */
	unsigned int MediaTotalCapacity;		/**< Total capacity of the storage media (kb) */
	unsigned int ErrorMsgSize;				/**< Error Message String Size */
	char ErrorMsg[FILE_ERROR_MSG_SIZE_MAX];	/**< Error Message String */
}_PACKED_ RM_FILESYSTEMSTATUS_DATA_T;

/** Huace RawMsg data struct */

/** Huace raw message GPS ephemeris struct, previous name: txxx_EPH_GPS_s */
typedef struct _RM_EX_EPH_GPS_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_GPS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week number, GPS Week number  */
	double			URA;        /**< User Range Accuracy variance, m'2*/
	double			Idot;		/**< Rate if inclination angle, radians/second*/
	unsigned int	Iode1;      /**< Issue of ephemeris data 1*/
	double			TOC;		/**< SV clock correction term, seconds*/
	double			Af2;		/**< Clock aging parameter, (s/s/s)*/
	double			Af1;		/**< Clock aging parameter, (s/s)*/
	double			Af0;		/**< Clock aging parameter, seconds(s)*/
	unsigned int	Iodc;		/**< Issue of data clock*/
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			DN;			/**< Mean motion difference, radians/second*/
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Ecc;		/**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters)     */
	double			A;			/**< Semi-major axis, meters*/
	double			TOE;		/**< Reference time for ephemeris, seconds*/
	double			Cic;		/**< Inclination (amplitude of cosine, radians)*/
	double			Omega0;		/**< Right ascension, radians*/
	double			Cis;		/**< Inclination (amplitude of sine, radians)*/
	double			Inc0;		/**< Inclination angel at reference time, radians*/
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters) */
	double			W;			/**< Argument of perigee, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second*/
	double			Tgd;		/**< Estimated group delay difference, seconds*/
	unsigned int	Health;		/**< Health status -a 6 bit health code as defined in ICD-GPS-200a*/
	double			TOW;		/**< Time stamp of subframe 1 (seconds)*/
	unsigned int	Iode2;		/**< Issue of ephemeris data 2*/
	unsigned int	Zweek;		/**< Z count week number, this is week number from subframe 1 of the ephemeris, */
								/**< the toe's week is derived from this to account for roll over*/
	unsigned int	ASFlag;     /**< Anti-spooning Flag, 0=false, 1=TRUE*/
	double			N;			/**< Corrected mean motion,  radians/second*/
}_PACKED_ RM_EX_EPH_GPS_DATA_T;

/** Huace raw message QZSS Ephemeris Log Body Structure*/
typedef struct _RM_EX_EPH_QZS_DATA_T
{
	unsigned int	PRN;           /**< satellite PRN number,  based on MIN_QZSS_RANGE_PRN (193)*/
	double			TOC;           /**< SV clock correction term, seconds*/
	double			Af2;           /**< Clock aging parameter, (s/s/s)*/
	double			Af1;           /**< Clock aging parameter, (s/s)*/
	double			Af0;           /**< Clock aging parameter, seconds(s)*/
	unsigned int	Iode1;         /**< Issue of ephemeris data 1*/
	double			Crs;           /**< Orbit radius (amplitude of sine, radians) */
	double			DN;            /**< Mean motion difference, radians/second*/
	double			M0;            /**< Mean anomaly of reference time , radians*/
	double			Cuc;           /**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Ecc;           /**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			Cus;           /**< Argument of latitude (amplitude of sine, meters)     */
	double			A;             /**< Semi-major axis, meters*/
	double			TOE;           /**< Reference time for ephemeris, seconds*/
	double			Cic;           /**< Inclination (amplitude of cosine, radians)*/
	double			Omega0;        /**< Right ascension, radians*/
	double			Cis;           /**< Inclination (amplitude of sine, radians)*/
	double			Inc0;          /**< Inclination angel at reference time, radians*/
	double			Crc;           /**< Orbit radius (amplitude of cosine, meters) */
	double			W;             /**< Argument of perigee, radians*/
	double			OmegaDot;      /**< Rate of right ascension, radians/second*/
	double			Idot;          /**< Rate if inclination angle, radians/second*/
	unsigned int	Week;          /**< TOE Week number, GPS Week number  */
	double			URA;           /**< User Range Accuracy variance, m'2*/
	unsigned int	Health;        /**< Health status -a 6 bit health code as defined in ICD-GPS-200a*/
	double			Tgd;           /**< Estimated group delay difference, seconds*/
	unsigned int	Iodc;          /**< Issue of data clock*/
	unsigned char	FitInterval;   /**< curve fit interval; 0 - eph data are effective for 2 hours; 1 - eph data are effective more than 2 hours*/
	unsigned char	Reseved[3];    /**< reserved*/
	double			TOW;           /**< Time stamp of subframe 1 (seconds)*/
	unsigned int	Iode2;         /**< Issue of ephemeris data 2*/
	unsigned int	Zweek;         /**< Z count week number, this is week number from subframe 1 of the ephemeris, */
								   /**< the toe's week is derived from this to account for roll over*/
	unsigned int	ASFlag;        /**< Anti-spooning Flag, 0=false, 1=TRUE*/
	double			N;             /**< Corrected mean motion,  radians/second*/
}_PACKED_ RM_EX_EPH_QZS_DATA_T;

/** Huace raw message BDS ephemeris struct, previous name: txxx_EPH_BD_s */
typedef struct _RM_EX_EPH_BD2_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week Number, BDS Time Week number  */
	double			URA;		/**< User Range Accuracy variance, m*/
	double			Idot;		/**< Rate if inclination angle, radians/second*/
	unsigned int	Iode1;		/**< Issue of ephemeris data 1*/
	unsigned int	TOC;		/**< SV clock correction term, seconds*/
	double			Af2;		/**< Clock aging parameter, (s/s/s)*/
	double			Af1;		/**< Clock aging parameter, (s/s)*/
	double			Af0;		/**< Clock aging parameter, (s)*/
	unsigned int	Iodc;		/**< Issue of data clock*/
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			DN;			/**< Mean motion difference, radians/second*/
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Ecc;		/**< Eccentricity, dimensionless - quality defined for a conic section where e = 0 is a circle*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double			RootA;		/**< Square root Semi-major axis, meters*/
	unsigned int	TOE;		/**< Reference time for ephemeris, seconds*/
	double			Cic;		/**< Inclination (amplitude of cosine, radians)*/
	double			Omega0;		/**< Omega0,Right ascension, radians*/
	double			Cis;		/**< Inclination (amplitude of sine, radians)*/
	double			Inc0;		/**< Inclination angel at reference time, radians*/
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			W;			/**< Argument of perigee, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second*/
	double			Tgd;		/**< Estimated group delay difference for B1, seconds*/
	double			Tgd2;		/**< Estimated group delay difference for B2, seconds*/
	unsigned int	Health;		/**< Health status -a 6 bit health code as defined in BDS-ICD*/
}_PACKED_ RM_EX_EPH_BD2_DATA_T;

/** Huace raw message BDS CNAV1/CNAV2 ephemeris struct */
typedef struct _RM_EX_EPH_BDS_CNAV12_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week number in BDS time (BDT) */
	double			I0Dot;		/**< Rate if inclination angle, radians/second */
	unsigned int	IODE;		/**< Issue of ephemeris data */
	unsigned int	TOC;		/**< Reference time of clock parameters in BeiDou time (BDT).*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	unsigned int    IODC;		/**< Issue of data, clock */
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double          DeltaN;     /**< Mean motion difference from computed value (radians/s) */
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Ecc;		/**< Eccentricity, dimensionless */
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double          DeltaA;     /**< Semi-major axis difference at reference time (m) */
	double          ADot;       /**< Change rate in Semi-major axis (m/s) */
	unsigned int	TOE;		/**< Reference time of ephemeris parameters in BeiDou time (BDT). (s) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians) */
	double			Omega0;		/**< Longitude of ascending node of orbital of planeat weekly epoch, radians */
	double			Cis;		/**< Inclination (amplitude of sine, radians) */
	double			I0;	        /**< Inclination angel at reference time, radians */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Omega;	 	/**< Argument of perigee, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second */
	double          TGDB1Cp;    /**< Group delay differential between the B1C pilot component and the B3I signal (s) */
	double          TGDB2Ap;    /**< Group delay differential between the B2a pilot component and the B3I signal (s) */
	unsigned int    SatStatus;  /**< Satellite Status Information
									 bit 0~1: HS() ,bit 2: DIF, bit 3: SIF, bit 4: AIF
									 bit 5 ~ 8, Signal In Space Monitoring Accuracy Index */
	unsigned int    SatType;    /**< 2 bit orbit type */
	double          NDot;       /**< Rate of Mean motion difference from computed value (radians/s/s) */
	double          ISCd;       /**< Group delay differential between the B1C data and pilot components (s) for CNAV1 msg
									 Group delay differential between the B2a data and pilot components (s) for CNAV2 msg */
	unsigned int    Reserved;
}_PACKED_ RM_EX_EPH_BDS_CNAV12_DATA_T;

/** Huace raw message BDS CNAV3 ephemeris struct */
typedef struct _RM_EX_EPH_BDS_CNAV3_DATA_T
{
	unsigned int	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned int	Week;		/**< TOE Week number in BDS time (BDT) */
	double			I0Dot;		/**< Rate if inclination angle, radians/second */
	unsigned int	TOC;		/**< Reference time of clock parameters in BeiDou time (BDT).*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double          DeltaN;     /**< Mean motion difference from computed value (radians/s) */
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Ecc;		/**< Eccentricity, dimensionless */
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double          DeltaA;     /**< Semi-major axis difference at reference time (m) */
	double          ADot;       /**< Change rate in Semi-major axis (m/s) */
	unsigned int	TOE;		/**< Reference time of ephemeris parameters in BeiDou time (BDT). (s) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians) */
	double			Omega0;		/**< Longitude of ascending node of orbital of planeat weekly epoch, radians */
	double			Cis;		/**< Inclination (amplitude of sine, radians) */
	double			I0;	        /**< Inclination angel at reference time, radians */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Omega;	 	/**< Argument of perigee, radians*/
	double			OmegaDot;	/**< Rate of right ascension, radians/second */
	double          TGDB2Bi;    /**< Group delay differential between the B2bI and the B3I signal (s) */
	unsigned int    SatStatus;  /**< Satellite Status Information */
	unsigned int    SatType;    /**< 2 bit orbit type */
	double          NDot;       /**< Rate of Mean motion difference from computed value (radians/s/s) */
	unsigned int    Reserved;
}_PACKED_ RM_EX_EPH_BDS_CNAV3_DATA_T;

/** Huace raw message GLONASS ephemeris struct, previous name: txxx_EPH_GLN_s */
typedef struct _RM_EX_EPH_GLO_DATA_T
{
	unsigned short	PRN;		/**< slot information offset - PRN identification (Slot+38). This is also called SLOTO in CDU
									 based on MIN_GLONASS_RANGE_PRN (38) */
	unsigned short  FreqO;		/**< Freq Offset, 0~20 , FreqNum + 7*/
	unsigned int    Flags;      /**< Information flags*/
	unsigned int    Tk;         /**< Time of frame start(since start of GLONASS day), in seconds*/
	unsigned int    Health;     /**< Ephemeris Health where, 0 = GOOD, 1 = BAD*/
	unsigned int    Issue;      /**< 15-minute interval number corresponding to ephemeris reference time*/
	double			VelX;       /**< X coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			PosX;       /**< X coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			LSAccX;     /**< X coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			VelY;       /**< Y coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			PosY;       /**< Y coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			LSAccY;     /**< Y coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			VelZ;       /**< Z coordinate for satellite velocity at reference time (PZ-90.02), in meters*/
	double			PosZ;       /**< Z coordinate for satellite at reference time (PZ-90.02), in meters*/
	double			LSAccZ;     /**< Z coordinate for lunisolar acceleration at reference time (PZ-90.02), in meters*/
	double			Gamma;      /**< Frequency correction, in seconds/second*/
	unsigned int    P;          /**< Technological parameter*/
	double			TauN;       /**< Correction to the nth satellite time t_n relative to GLONASS time t_c, in seconds*/
	double			DeltaTauN;  /**< Time difference between navigation RF signal transmitted in L2 sub-band and*/
								/**< navigation RF signal transmitted in L1 sub-band  by nth satellite, in seconds */
	unsigned int    Age;        /**< Age of data, in days*/
	unsigned int    Ft;         /**< User range*/
	unsigned short  Nt;         /**< Current Data number only for M type satellite.*/
	unsigned char   Reserved1;  /**< Reserve d*/
	unsigned char   Reserved2;  /**< Reserved*/
	unsigned char   SatType;	/**< Satellite Type where, 0--GLO_SAT, 1--GLO_SAT_M, 2--GLO_SAT_K*/
	unsigned char   Reserved3;  /**< Reserved*/
	unsigned short  RefWeek;    /**< Reference Week of ephemeris (GPS Time)*/
	unsigned int    RefSec;     /**< Reference time of ephemeris (GPS Time) in ms*/
	unsigned int    OffsetT;    /**< Integer seconds between GPS and GLONASS time, GLONASST-GPST*/
}_PACKED_ RM_EX_EPH_GLO_DATA_T;

/** Huace raw message Galileo INAV ephemeris struct */
typedef struct _RM_EX_EPH_GAL_INAV_DATA_T
{
	unsigned int	PRN;            /**< satellite PRN number, based on MIN_GALILEO_RANGE_PRN(1)*/
	unsigned short  IODNav;         /**< Issue of data ephemeris */
	unsigned char 	SISA;           /**< SISA index, Signal in space accuracy(unitless) */
	unsigned char   Reserved0;
	double			IDot;           /**< Rate of inclination angle (radians/s)*/
	unsigned int	T0c;            /**< Clock correction data reference time of week from the I/NAV message (s).*/
	double			Af2;            /**< SV clock drift rate correction coefficient from the I/NAV message (s/s^2) */
	double			Af1;            /**< SV clock drift correction coefficient from the I/NAVmessage (s/s) */
	double			Af0;            /**< SV clock bias correction coefficient from the I/NAV message (s) */
	double			Crs;            /**< Amplitude of the sine harmonic correction term to the orbit radius (m)*/
	double			DeltaN;         /**< Mean motion difference (radians/s)*/
	double			M0;             /**< Mean anomaly at ref time (radians)*/
	double			Cuc;            /**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)*/
	double			Ecc;            /**< Eccentricity (unitless)*/
	double			Cus;            /**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)*/
	double			RootA;          /**< Square root of semi-major axis*/
	unsigned int	T0e;            /**< Ephemeris reference time (s)*/
	double			Cic;            /**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)*/
	double			Omega0;         /**< Longitude of ascending node of orbital plane at weekly epoch (radians)*/
	double			Cis;            /**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)*/
	double			I0;             /**< Inclination angle at ref time (radians)*/
	double			Crc;            /**< Amplitude of the cosine harmonic correction term to the orbit radius (m)*/
	double			Omega;          /**< Argument of perigee (radians)*/
	double			OmegaDot;       /**< Rate of right ascension (radians/s)*/
	double			E1E5aBGD;       /**< E1, E5a broadcast group delay */
	double			E1E5bBGD;       /**< E1, E5b broadcast group delay */
	unsigned char 	E5bHealth;      /**< E5b health status bits  */
	unsigned char 	E5bDVS;         /**< E5b data validity status */
	unsigned char   Reserved1;
	unsigned char   Reserved2;
	unsigned char 	E1bHealth;      /**< E1b health status bits */
	unsigned char 	E1bDVS;         /**< E1b data validity status */
	unsigned char   INavSource;     /**< Identifies the source signal, 0 -- unknown, 1 -- E1b, 2 -- E5b, 3 -- E1b and E5b */
	unsigned char   Reserved3;
}_PACKED_ RM_EX_EPH_GAL_INAV_DATA_T;

/** Huace raw message Galileo FNAV ephemeris struct */
typedef struct _RM_EX_EPH_GAL_FNAV_DATA_T
{
	unsigned int	PRN;            /**< satellite PRN number, based on MIN_GALILEO_RANGE_PRN (1)*/
	unsigned short  IODNav;         /**< Issue of data ephemeris */
	unsigned char 	SISA;           /**< SISA index, Signal in space accuracy(unitless) */
	unsigned char   Reserved0;
	double			IDot;           /**< Rate of inclination angle (radians/s)*/
	unsigned int	T0c;            /**< Clock correction data reference time of week from the F/NAV message (s).*/
	double			Af2;            /**< SV clock drift rate correction coefficient from the F/NAV message (s/s^2) */
	double			Af1;            /**< SV clock drift correction coefficient from the F/NAVmessage (s/s) */
	double			Af0;            /**< SV clock bias correction coefficient from the F/NAV message (s) */
	double			Crs;            /**< Amplitude of the sine harmonic correction term to the orbit radius (m)*/
	double			DeltaN;         /**< Mean motion difference (radians/s)*/
	double			M0;             /**< Mean anomaly at ref time (radians)*/
	double			Cuc;            /**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)*/
	double			Ecc;            /**< Eccentricity (unitless)*/
	double			Cus;            /**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)*/
	double			RootA;          /**< Square root of semi-major axis*/
	unsigned int	T0e;            /**< Ephemeris reference time (s)*/
	double			Cic;            /**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)*/
	double			Omega0;         /**< Longitude of ascending node of orbital plane at weekly epoch (radians)*/
	double			Cis;            /**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)*/
	double			I0;             /**< Inclination angle at ref time (radians)*/
	double			Crc;            /**< Amplitude of the cosine harmonic correction term to the orbit radius (m)*/
	double			Omega;          /**< Argument of perigee (radians)*/
	double			OmegaDot;       /**< Rate of right ascension (radians/s)*/
	double			E1E5aBGD;       /**< E1, E5a broadcast group delay */
	unsigned char 	E5aHealth;      /**< E5a health status bits */
	unsigned char 	E5aDVS;         /**< E5a data validity status */
	unsigned char   Reserved1;
	unsigned char   Reserved2;
}_PACKED_ RM_EX_EPH_GAL_FNAV_DATA_T;

/** Huace raw message NavIC ephemeris struct, previous name: txxx_EPH_GAL_s */
typedef struct _RM_EX_EPH_NIC_DATA_T
{
	unsigned int	PRN;			/**< Satellite Identifier, based on MIN_NAVIC_RANGE_PRN (1) */
	unsigned int	Week;			/**< Week number since the NavIC system time start epoch (August 22nd 1999)*/
	double			Af0;			/**< Clock bias (s)   */
	double			Af1;			/**< Clock drift (s/s) */
	double			Af2;			/**< Clock drift rate (s/s2)   */
	unsigned int    URA;			/**< SV Accuracy  */

	unsigned int	toc;			/**< Reference time for the satellite clock corrections (s)    */
	double			TGD;			/**< Total group delay (s)   */
	double			DN;				/**< Mean motion difference (radian/s)     */
	unsigned int	IODEC;			/**< Issue of data ephemeris and clock*/
	unsigned int	Reserved;
	unsigned int	L5Health;		/**< Health status of navigation data on L5 SPS signal0=OK; 1=bad     */
	unsigned int	SHealth;		/**< Health status of navigation data on S SPS signal0=OK; 1=bad  */

	double			Cuc;			/**< Amplitude of the cosine harmonic correction term to the argument of latitude (radians)  */
	double			Cus;			/**< Amplitude of the sine harmonic correction term to the argument of latitude (radians)    */
	double			Cic;			/**< Amplitude of the cosine harmonic correction term to the angle of inclination (radians)    */
	double			Cis;			/**< Amplitude of the sine harmonic correction term to the angle of inclination (radians)   */
	double			Crc;			/**< Amplitude of the cosine harmonic correction term to the orbit radius (m)  */
	double			Crs;			/**< Amplitude of the sine harmonic correction term to the orbit radius (m)  */

	double			IDot;			/**< Rate of inclination angle (radians/s)   */

	double			M0;				/**< Mean anomaly (radians)    */
	unsigned int	Toe;			/**< Time of ephemeris (s)    */
	double			Ecc;			/**< Eccentricity (dimensionless)  */
	double			RootA;			/**< Square root of semi-major axis (sqrt(m))     */
	double			Omega0;			/**< Longitude of ascending node (radians)     */
	double			Omega;			/**< Argument of perigee (radians) */
	double			OmegaDot;		/**< Rate of RAAN (radians/s)     */
	double			I0;				/**< Inclination angle (radians)  */

	unsigned int	Spare1;			/**<      */
	unsigned int	Spare2;			/**<      */
	unsigned int	AlertFlag;		/**< The utilization of navigation data shall be at the users own risk.1=Alert; 0=OK    */
	unsigned int	AutoNavFlag;	/**< When set to 1, satellite is in AutoNav mode. Satellite broadcasts primary navigation
									parameters from AutoNav data sets with no uplink from ground for maximum of 7 days   */
}_PACKED_ RM_EX_EPH_NIC_DATA_T;

/** Huace raw message GPSIONO data struct, previous name: txxx_GPSIONO_s */
typedef struct _RM_EX_GPSIONO_DATA_T
{
	double Alpha0;
	double Alpha1;
	double Alpha2;
	double Alpha3;
	double Beta0;
	double Beta1;
	double Beta2;
	double Beta3;
	unsigned int Reserved;
}_PACKED_ RM_EX_GPSIONO_DATA_T;

/** Huace raw message GPSUTC data struct, previous name: txxx_GPSUTC_s */
typedef struct _RM_EX_GPSUTC_DATA_T
{
	unsigned int utc_wn;
	unsigned int tot;
	double A0;
	double A1;
	unsigned int wn_lsf;
	unsigned int dn;
	int deltat_ls;
	int deltat_lsf;
	unsigned int Deltat_utc;
}_PACKED_ RM_EX_GPSUTC_DATA_T;

/** Huace raw message BESTDOPS/PSRDOPS/RTKDOPS data struct, previous name: txxx_BESTDOP_s, RM_BESTDOPS_DATA_T */
typedef struct _RM_EX_DOPS_DATA_T
{
	float PDOP;		/**< Position dilution of precision */
	float GDOP;		/**< Geometric dilution of precision */
	float HDOP;		/**< Horizontal dilution of precision */
	float TDOP;		/**< GPS time dilution of precision */
	float HTDOP;	/**< Horizontal position and time dilution of precision */
	float ElevMask;	/**< GPS elevation mask angle */
	unsigned int SatNum;		/**< Number of satellites to follow */
	unsigned short SatTab[RM_BESTDOP_SAT_NUM_MAX];	/**< Satellites in use at time of calculation */
}_PACKED_ RM_EX_DOPS_DATA_T;

/** Huace raw message BESTP/SPPP/RTKP/PPPP data struct, previous name: txxx_BESTP_s,RM_BESTP_DATA_T */
typedef struct _RM_EX_POS_DATA_T
{
	unsigned char SolStatus;		/**< Solution status */
	unsigned char PosType;			/**< Position type */
	unsigned char DatumID;			/**< Datum ID number */
	unsigned char Reserved1;
	double Lat;						/**< Latitude (unit: degrees) */
	double Lon;						/**< Longitude (unit: degrees) */
	double Hgt;						/**< Height above mean sea level (unit: metres) */
	float Undulation;				/**< Undulation - the relationship between the geoid and the ellipsoid (m) of the chosen datum */
	float LatStd;					/**< Latitude standard deviation (unit: metres) */
	float LonStd;					/**< Longitude standard deviation (unit: metres) */
	float HgtStd;					/**< Height standard deviation (unit: metres) */
	float DiffAge;					/**< Differential age in seconds */
	float SolAge;					/**< Solution age in seconds */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned char SolL1SVs;			/**< Number of satellites with L1/E1/B1 signals used in solution */
	unsigned short ExtSolStatus;	/**< Extended solution status
										bit 0 - 2, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 3, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections
										bit4-bit7, 4 bits, VRS Version
													0 = Ver3.0
													1 = Ver4.0
										bit8-bit13, 6 bits, VRS diffage, 0-61sec 
										bit14-bit15, Reserved*/
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
	unsigned char BDSSignalMask;	/**< BeiDou signals used mask
										bit 0, 1 bits, BeiDou B1 used in Solution (B1I)
										bit 1, 1 bits, BeiDou B2 used in Solution (B2I)
										bit 2, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 3, 1 bits, BeiDou B1C used in Solution
										bit 4, 1 bits, BeiDou B2a used in Solution
										bit 5, 1 bits, BeiDou B2b used in Solution */
	unsigned char GALSignalMask;	/**< Galileo signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned char Reserved2;
	unsigned short StationID;		/**< Base station ID */
}_PACKED_ RM_EX_POS_DATA_T;

/** Huace raw message BESTV/SPPV/RTKV data struct, previous name: txxx_BESTV_s,RM_BESTV_DATA_T */
typedef struct _RM_EX_VEL_DATA_T
{
	unsigned char SolStatus;	/**< Solution status */
	unsigned char VelType;		/**< Velocity type */
	unsigned short Reserved1;
	double HorSpd;				/**< Horizontal speed over ground, in metres per second */
	double TrkGnd;				/**< Actual direction of motion over ground (track over ground) with respect to True North, in degrees */
	double VertSpd;				/**< Vertical speed, in metres per second, where positive values
										indicate increasing altitude (up) and negative values indicate decreasing altitude (down) */
	float Latency;				/**< A measure of the latency in the velocity time tag in seconds.
										It should be subtracted from the time to give improved results (s) */
	float DiffAge;				/**< Differential age in seconds */
	float Reserved2;
}_PACKED_ RM_EX_VEL_DATA_T;

/** Huace raw message BESTECEF data struct, previous name: txxx_BESTECEF_s */
typedef struct _RM_EX_BESTECEF_DATA_T
{
	unsigned char PosSolStatus;		/**< position solution status */
	unsigned char PosType;			/**< Position type */
	unsigned short Reserved1;
	double PosX;					/**< Position X-coordinate (m) */
	double PosY;					/**< Position Y-coordinate (m) */
	double PosZ;					/**< Position Z-coordinate (m) */
	float PosStdX;					/**< Standard deviation of P-X (m) */
	float PosStdY;					/**< Standard deviation of P-X (m) */
	float PosStdZ;					/**< Standard deviation of P-X (m) */
	unsigned char VelSolStatus;		/**< Solution status */
	unsigned char VelType;			/**< Velocity type */
	unsigned short Reserved2;
	double VelX;					/**< Velocity vector along X-axis (m/s) */
	double VelY;					/**< Velocity vector along Y-axis (m/s) */
	double VelZ;					/**< Velocity vector along Z-axis (m/s) */
	float VelStdX;					/**< Standard deviation of V-X (m/s) */
	float VelStdY;					/**< Standard deviation of V-Y (m/s) */
	float VelStdZ;					/**< Standard deviation of V-Z (m/s) */
	float Vlatency;					/**< A measure of the latency in the velocity time tag in seconds.
										It should be subtracted from the time to give improved results */
	float DiffAge;					/**< Differential age in seconds */
	float SolAge;					/**< Solution age in seconds */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned char SolL1SVs;			/**< Number of satellites with L1/E1/B1 signals used in solution */
	unsigned short ExtSolStatus;	/**< Extended solution status
										bit 0 - 2, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 3, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections
										bit4-bit7, 4 bits, VRS Version
													0 = Ver3.0
													1 = Ver4.0
										bit8-bit13, 6 bits, VRS diffage, 0-61sec 
										bit14-bit15, Reserved*/
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
	unsigned char BDSSignalMask;	/**< BeiDou signals used mask
										bit 0, 1 bits, BeiDou B1 used in Solution (B1I)
										bit 1, 1 bits, BeiDou B2 used in Solution (B2I)
										bit 2, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 3, 1 bits, BeiDou B1C used in Solution
										bit 4, 1 bits, BeiDou B2a used in Solution
										bit 5, 1 bits, BeiDou B2b used in Solution */
	unsigned char GALSignalMask;	/**< Galileo signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned char Reserved3;
	unsigned short StationID;		/**< Base station identification */
}_PACKED_ RM_EX_BESTECEF_DATA_T;

typedef struct _RM_EX_BESTSATINFO_CELL_DATA_T
{
	int System;					/**< Satellite System */
	unsigned int SatelliteID;	/**< Satellite ID */
	unsigned int SignalMask;	/**< Satellite Signal mask */
	int Status;					/**< Satellite status */
} RM_EX_BESTSATINFO_CELL_DATA_T;

/** Huace raw message BESTSATS data struct, previous name: txxx_BESTSATS_s */
typedef struct _RM_EX_BESTSATINFO_DATA_T
{
	unsigned int SatNum;		/**< Satellite number */
	RM_EX_BESTSATINFO_CELL_DATA_T BestSats[RM_BESTSATS_NUM_MAX];
}_PACKED_ RM_EX_BESTSATINFO_DATA_T;

typedef struct _RM_EX_SATINVIEW_CELL_DATA_T
{
	unsigned int SatID;			/**< Satellites in use at time of calculation */
	unsigned int Health;		/**< Satellite health as defined in ICD */
	double Elevation;			/**< Elevation (degrees) */
	double Azimuth;				/**< Azimuth (degrees) */
	double TrueDoppler;			/**< Theoretical Doppler of satellite - the expected Doppler frequency based on a satellite's motion relative to the receiver.
									It is computed using the satellite's coordinates and velocity along with the receiver's coordinates and velocity (Hz). */
	double ApparentDoppler;		/**< Apparent Doppler for this receiver - the same as Theoretical Doppler above but with clock drift correction added (Hz). */
} RM_EX_SATINVIEW_CELL_DATA_T;

/** Huace raw message SATINVIEW data struct, previous name: txxx_SATINVIEW_s */
typedef struct _RM_EX_SATINVIEW_DATA_T
{
	int System;					/**< GNSS satellite system identifier */
	int SatVis;					/**< Satellite visibility valid */
	int AlmanacFlag;			/**< Complete almanac used */
	unsigned int SatNum;		/**< complete almanac used */
	RM_EX_SATINVIEW_CELL_DATA_T SatInViewCellData[RM_SATVIS2_SAT_NUM_MAX];
}_PACKED_ RM_EX_SATINVIEW_DATA_T;

typedef struct _RM_EX_TIMES_CELL_DATA_T
{
	unsigned int SystemID;	/**< GNSS system id */
	double Offset;			/**< Receiver clock offset in seconds from GPS system time */
	double OffsetStd;		/**< Receiver clock offset standard deviation (s) */
	double UTCOffset;		/**< The offset of GPS system time from UTC time  */
}_PACKED_ RM_EX_TIMES_CELL_DATA_T;

/** Huace raw message TIMES data struct, previous name: txxx_TIMES_s */
typedef struct _RM_EX_TIMES_DATA_T
{
	int ClockStatus;		/**< Clock model status (not including current measurement data) */
	unsigned int Year;		/**< UTC year */
	unsigned char Month;	/**< UTC month (0-12) */
	unsigned char Day;		/**< UTC day (0-31) */
	unsigned char Hour;		/**< UTC hour (0-23) */
	unsigned char Minu;		/**< UTC minute (0-59) */
	unsigned int MSec;		/**< UTC millisecond (0-60999) */
	int UTCStatus;			/**< UTC status */
	unsigned int SysNum;
	RM_EX_TIMES_CELL_DATA_T TimesCellData[RM_TIMES_SYS_NUM_MAX];
}_PACKED_ RM_EX_TIMES_DATA_T;

/** Huace raw message BASESTATION data struct, previous name: txxx_BASESTATION_s */
typedef struct _RM_EX_BASESTATION_DATA_T
{
	unsigned char Status;		/**< Status of the base station information */
	unsigned char StationType;	/**< Station type */
	unsigned short StationID;	/**< Base station ID */
	unsigned int Health;		/**< Base station health*/
	double ECEFX;				/**< ECEF X value (m) */
	double ECEFY;				/**< ECEF Y value (m) */
	double ECEFZ;				/**< ECEF Z value (m) */
}_PACKED_ RM_EX_BASESTATION_DATA_T;

/** Huace raw message BASESTATIONINFO data struct, previous name: txxx_BASESTATIONINFO_s */
typedef struct _RM_EX_BASESTATIONINFO_DATA_T
{
	unsigned char DatumID;		/**< Datum ID number */
	unsigned char StationType;	/**< Station type */
	unsigned short StationID;	/**< Base station ID */
	double Latitude;			/**< Latitude (degrees) */
	double Longitude;			/**< Longitude (degrees) */
	double Height;				/**< Ellipsoidal Height (m) */
	float ARPHeight;			/**< Base Antenna ARP (m) */
	unsigned int Health;		/**< Base station health*/
	char AntModel[32];			/**< Base Antenna Model Name */
	char AntSerial[32];			/**< Base Antenna Serial Number */
}_PACKED_ RM_EX_BASESTATIONINFO_DATA_T;

typedef struct _RM_EX_RTCMINSTATUS_MSG_INFO_T
{
	unsigned int MsgID;		/**< input message id */
	unsigned int MsgNum;	/**< input message number */
}_PACKED_ RM_EX_RTCMINSTATUS_MSG_INFO_T;

typedef struct _RM_EX_RTCMINSTATUS_PORT_DATA_T
{
	unsigned int RTCMPortID;		/**< RTCM input port id */
	unsigned int MsgTypeNum;		/**< RTCM input port message type number */
	RM_EX_RTCMINSTATUS_MSG_INFO_T MsgInfo[RTCM_IN_STATUS_NUM_MAX];	/**< RTCM input port receiver message information*/
}_PACKED_ RM_EX_RTCMINSTATUS_PORT_DATA_T;

/** Huace raw message RTCMINSTATUS data struct, previous name: txxx_RTCMINSTATUS_s */
typedef struct _RM_EX_RTCMINSTATUS_DATA_T
{
	unsigned int RTCMPortNum;							/**< RTCM input port number */
	RM_EX_RTCMINSTATUS_PORT_DATA_T PortMsgData[RTCM_INPUT_PORT_NUM_MAX];		/**< RTCM input port message data */
}_PACKED_ RM_EX_RTCMINSTATUS_DATA_T;

/** Huace raw message SBASRAWFRAME data struct, previous name: txxx_SBASRAWFRAME_s */
typedef struct _RM_EX_SBASRAWFRAME_DATA_T
{
	unsigned int   PRN;				/**< SBAS satellite PRN number */
	unsigned int   SignalChannel;	/**< Signal channel number that the frame was decoded on */
	unsigned char  SignalSource;	/**< Identifies the source of the SBAS signal */
	unsigned char  PreambleType;	/**< Identifies what preamble was used when tracking the SBAS signal */
	unsigned short Reserved;		/**< Reserved */
	unsigned int   FrameID;			/**< SBAS frame ID*/
	unsigned char  FrameData[32];	/**< Raw SBAS frame data. There are 226 bits of data and 6 bits of padding */
}_PACKED_ RM_EX_SBASRAWFRAME_DATA_T;

/** Huace raw message GALCNAVRAWPAGE data struct */
typedef struct _RM_EX_GALCNAVRAWPAGE_DATA_T
{
	unsigned int   SignalChannel;	/**< Signal channel number providing the bits */
	unsigned int   PRN;				/**< GAL satellite PRN number, based on 1 */
	unsigned short MessageID;       /**< ID of the message */
	unsigned short PageID;          /**< ID of the transmitted page of the encoded message */
	unsigned char  FrameData[58];	/**< Raw GAL CNAV frame data. 462 bits, dones not include CRC or Tail bits */
}_PACKED_ RM_EX_GALCNAVRAWPAGE_DATA_T;

/** Huace raw message IMUOBS data struct, previous name: txxx_IMUOBS_s */
typedef struct _RM_EX_IMUOBS_DATA_T
{
	unsigned char	IMUInfo;		/**< IMU Info Bits
										Bit 0: If set, an IMU error was detected. Check the IMU Status field for details.
										Bit 1: If set, the IMU data is encrypted and should not be used.
										Bits 2 to 7: Reserved */
	unsigned char	IMUType;		/**< IMU Type */
	unsigned short	Week;			/**< GPS week */
	double			Seconds;		/**< Seconds from week start */
	int				ZGyro;			/**< Change in angle count along z axis Right-handed */
	int				YGyro;			/**< -(Change in angle count along y axis Right-handed) */
	int				XGyro;			/**< Change in angle count along x axis Right-handed */
	int				ZAccel;			/**< Change in velocity count along z axis*/
	int				YAccel;			/**< - (Change in velocity count along y axis) */
	int				XAccel;			/**< Change in velocity count along x axis*/
	unsigned int	IMUStatus;		/**< IMU Status */
}_PACKED_ RM_EX_IMUOBS_DATA_T;

/** Huace raw message INSSOL data struct, previous name: txxx_INSSOL_s */
typedef struct _RM_EX_INSSOL_DATA_T
{
	unsigned char	INSStatus;		/**< INS Status */
	unsigned char	Reserved[3];
	unsigned int	Week;			/**< GPS week */
	double			Seconds;		/**< Seconds from week start */
	double			Latitude;		/**< Latitude(unit: degrees) */
	double			Longitude;		/**< Longitude(unit: degrees) */
	double			Height;			/**< Height(unit: meter) */

	double			VelNorth;		/**< Velocity in a northerly direction (a negative value implies a southerly direction) [m/s] */
	double			VelEast;		/**< Velocity in an easterly direction (a negative value implies a westerly direction) [m/s] */
	double			VelUp;			/**< Velocity in an up direction [m/s] */

	double			Roll;			/**< Right-handed rotation from local level around y-axis in degrees */
	double			Pitch;			/**< Right-handed rotation from local level around x-axis in degrees */
	double			Azimuth;		/**< Left-handed rotation around z-axis in degrees clockwise from North
										This is the inertial azimuth calculated from the IMU gyros and the SPAN filters. */
}_PACKED_ RM_EX_INSSOL_DATA_T;

/** Huace raw message INSSOLX data struct */
typedef struct _RM_EX_INSSOLX_DATA_T
{
	unsigned char INSStatus;	/**< solution status */
	unsigned char PosType;		/**< position type */
	unsigned short ElapsedTime;	/**< s, elapsed time since the last ZUPT or position update */
	double Latitude;			/**< degree */
	double Longitude;			/**< degree */
	double Height;				/**< m */
	float Undulation;			/**< m, undulation */
	double NorthVelocity;		/**< m/s */
	double EastVelocity;		/**< m/s */
	double UpVelocity;			/**< m/s */
	double Roll;				/**< degree */
	double Pitch;				/**< degree */
	double Azimuth;				/**< degree */
	float LatitudeStd;			/**< m, latitude standard deviation */
	float LongitudeStd;			/**< m, longitude standard deviation */
	float HeightStd;			/**< m, height standard deviation */
	float NorthVelStd;			/**< m/s, north velocity standard deviation */
	float EastVelStd;			/**< m/s, east velocity standard deviation */
	float UpVelStd;				/**< m/s, up velocity standard deviation */
	float RollStd;				/**< degree, roll standard deviation */
	float PitchStd;				/**< degree, pitch standard deviation */
	float AzimuthStd;			/**< degree, yaw standard deviation */
	unsigned int ExtSolStat;	/**< extended solution status */
}_PACKED_ RM_EX_INSSOLX_DATA_T;

/** Huace raw message DANTHEADING data struct */
typedef struct _RM_EX_DANTHEADING_DATA_T
{
	unsigned char SolStatus;		/**< Solution status */
	unsigned char PosType;			/**< Position type */
	unsigned short Reserved1;
	float Length;					/**< Baseline length in meters */
	float Heading;					/**< Heading in degrees(0-359.999) */
	float Pitch;					/**< Pitch(+-90 degrees)*/
	float Reserved2;				/**< Reserved */  
	float HdgStdDev;				/**< Heading standard deviation in degrees */
	float PtchStdDev;				/**< Pitch standard deviation in degrees */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char ObsNum;			/**< Number of satellites above the elevation mask angle */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned short ExtSolStatus;	/**< Extended solution status
										bit 0 - 2, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction
													5 = NovAtel Blended Iono Value
										bit 3, 1 bits, 0 = Terrain Compensation corrections are not used
													1 = Position includes Terrain Compensation corrections
										bit4-bit7, 4 bits, VRS Version
													0 = Ver3.0
													1 = Ver4.0
										bit8-bit13, 6 bits, VRS diffage, 0-61sec 
										bit14-bit15, Reserved*/
	unsigned char SolSource;		/**< Solution source */
	unsigned char GPSGLOSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, Reserved
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, GLONASS L3 used in Solution
										bit 7, 1 bits, Reserved*/
	unsigned char BDSSignalMask;	/**< BeiDou signals used mask
										bit 0, 1 bits, BeiDou B1 used in Solution (B1I)
										bit 1, 1 bits, BeiDou B2 used in Solution (B2I)
										bit 2, 1 bits, BeiDou B3 used in Solution (B3I)
										bit 3, 1 bits, BeiDou B1C used in Solution
										bit 4, 1 bits, BeiDou B2a used in Solution
										bit 5, 1 bits, BeiDou B2b used in Solution */
	unsigned char GALSignalMask;	/**< Galileo signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5a used in Solution
										bit 2, 1 bits, Galileo E5b used in Solution
										bit 3, 1 bits, Galileo ALTBOC used in Solution
										bit 4, 1 bits, Galileo E6 used in Solution (E6B and E6C)*/
	unsigned short StnID;			/**< Station ID string */
} _PACKED_ RM_EX_DANTHEADING_DATA_T;

/** Huace raw message RAWRTCMAUX data struct, previous name: txxx_RAWRTCMAUX_s */
typedef struct _RM_EX_RAWRTCMAUX_BODY_HEADER_T
{
	char StationID[4];			/**< Base station ID */
	unsigned int MsgID;			/**< Message ID */
	unsigned int Reserved[4];	/**< Reserved */
	unsigned int MsgType;		/**< Message type */
}_PACKED_ RM_EX_RAWRTCMAUX_BODY_HEADER_T;

typedef struct _RM_EX_RAWRTCMAUX_BODY_HEX_DATA_T
{
	unsigned int DataLen;
	unsigned char DataBuf[RAWRTCMAUX_DATA_BUF_MAX_SIZE];
}_PACKED_ RM_EX_RAWRTCMAUX_BODY_HEX_DATA_T;

typedef struct _RM_EX_RAWRTCMAUX_BODY_ASCII_DATA_T
{
	char DataBuf[RAWRTCMAUX_DATA_BUF_MAX_SIZE];
}_PACKED_ RM_EX_RAWRTCMAUX_BODY_ASCII_DATA_T;

/** Huace raw message BD2IONO data struct, previous name: txxx_BD2IONO_s */
typedef struct _RM_EX_BD2IONO_DATA_T
{
	double Alpha0; /**< Alpha parameter constant term */
	double Alpha1; /**< Alpha parameter 1st order term */
	double Alpha2; /**< Alpha parameter 2nd order term */
	double Alpha3; /**< Alpha parameter 3rd order term */
	double Beta0; /**< Beta parameter constant term */
	double Beta1; /**< Beta parameter 1st order term */
	double Beta2; /**< Beta parameter 2nd order term */
	double Beta3; /**< Beta parameter 3rd order term */
}_PACKED_ RM_EX_BD2IONO_DATA_T;

/** Huace raw message BD3IONO data struct, previous name: txxx_BD3IONO_s */
typedef struct _RM_EX_BD3IONO_DATA_T
{
	double Alpha1; /**< Alpha parameter 1st order term */
	double Alpha2; /**< Alpha parameter 2nd order term */
	double Alpha3; /**< Alpha parameter 3rd order term */
	double Alpha4; /**< Alpha parameter 4th order term */
	double Alpha5; /**< Alpha parameter 5th orderterm */
	double Alpha6; /**< Alpha parameter 6th order term */
	double Alpha7; /**< Alpha parameter 7th order term */
	double Alpha8; /**< Alpha parameter 8th order term */
	double Alpha9; /**< Alpha parameter 9th order term */
}_PACKED_ RM_EX_BD3IONO_DATA_T;

typedef struct _EVENT_MSG_TIME_T
{
	unsigned char ClockModelStatus;	/**< Clock model status */
	unsigned char Reserved;
	unsigned short Week;			/**< GPS reference week number */
	double Seconds;					/**< Seconds into the week as measured from the receiver clock,
										coincident with the time of electricalclosure on the Mark Input port */
	double OffSet;					/**< Receiver clock offset, in seconds,
										A positive offset implies that the receiver clock is ahead of GPS system time.
										To derive GPS system time, use the following formula:
										GPS system time = GPS reference time - (offset)
										Where GPS reference time can be obtained from the log header */
	double OffSetStd;				/**< Standard deviation of receiver clock offset (s) */
	double UTCOffSet;				/**< This field represents the offset of GPS system time from UTC time (s),
										computed using almanac parameters.
										UTC time is GPS reference time plus the current UTC offset minus the receiver clock offset.
										UTC time = GPS reference time - offset + UTC offset */
}_PACKED_ EVENT_MSG_TIME_T;

/** Huace raw message TILTCONFIG data struct, previous name: txxx_TILTCONFIG_s */
typedef struct _RM_EX_TILTCONFIG_DATA_T
{
	unsigned char TiltMode;	/**< tilt mode */
	unsigned char IMUType;	/**< IMU type */
	unsigned short Reserved1;

	float PoleHeight;		/**< pole height (unit: m) */

	float LeverArmX;		/**< X-direction lever arm (unit: m) */
	float LeverArmY;		/**< Y-direction lever arm (unit: m) */
	float LeverArmZ;		/**< Z-direction lever arm (unit: m) */

	float InstallAngleX;	/**< X-direction install angle (unit: deg) */
	float InstallAngleY;	/**< Y-direction install angle (unit: deg) */
	float InstallAngleZ;	/**< Z-direction install angle (unit: deg) */

	float GyoBiasX;			/**< X-direction gyo bias (unit: deg/s) */
	float GyoBiasY;			/**< Y-direction gyo bias (unit: deg/s) */
	float GyoBiasZ;			/**< Z-direction gyo bias (unit: deg/s) */

	float AccBiasX;			/**< X-direction acc bias (unit: mg) */
	float AccBiasY;			/**< Y-direction acc bias (unit: mg) */
	float AccBiasZ;			/**< Z-direction acc bias (unit: mg) */

	float AlignVel;			/**< alignment velocity */

	unsigned int Reserved[6];
}_PACKED_ RM_EX_TILTCONFIG_DATA_T;

/** Huace raw message ANTISPOOFSTATUS data struct, previous name: txxx_ANTISPOOFSTATUS_s */
typedef struct _RM_EX_ANTISPOOFSTATUS_DATA_T
{
	unsigned int OSNMAStatus;			/**< OSNMA Status */
	unsigned int Reserved1;				/**< Reserved1 */

	unsigned short SpoofExistFlag;		/**< Spoof exist flag */
	unsigned short ConfidenceRatio;		/**< Confidence Ratio */

	unsigned short SpoofDetectMode;		/**< Spoof detect mode */
	unsigned short FirstDetectedWeek;	/**< First detected week */

	unsigned int FirstDetectedTOW;		/**< First detected tow */
	unsigned int DetectSatNum;			/**< detected satellite number */
	unsigned int SpoofSatNum;			/**< Spoof satellite number */

	UINT64 GPSSpoofSatMask;				/**< GPS spoof satellite mask */
	UINT64 GLOSpoofSatMask;				/**< GLO spoof satellite mask */
	UINT64 GALSpoofSatMask;				/**< GAL spoof satellite mask */
	UINT64 BDSSpoofSatMask;				/**< BDS spoof satellite mask */

	unsigned int Reserved2;				/**< Reserved1 */
}_PACKED_ RM_EX_ANTISPOOFSTATUS_DATA_T;

typedef struct _RM_EX_SAT_FILTER_INFO_T
{
	unsigned char SatSystem;	/**< 0:GPS; 1:GLO; 2:GAL; 3:BDS; 4:SBA; 5:QZS; 6:NIC */
	unsigned char PRN;		  /**< 1 based satellite PRN */
	unsigned char AtmConstraintType;/**< bit0~3:ionosphere constraint type; bit4~7:troposphere constraint type */
	unsigned char AmbFixType;   /**< 0:float; 1:EWL fix; 2:WL fix; 3:NL fix */
} RM_EX_SAT_FILTER_INFO_T;

/** Huace raw message FILTERDBG data struct, previous name: txxx_FILTERDBG_s */
typedef struct _RM_EX_FILTERDBG_DATA_T
{
	unsigned char Version;				/**< Message version */
	unsigned char FilterID;				/**< 0/1: RTK filter; 10/11: Heading filter; 20/21: SWASPPP filter; 30/31: MGNSSPPP filter */
	unsigned char SceneType;			/**< 0:survey; 1.mower; 2:agriculture; 3:car; 4:UAV; 5:USV */
	unsigned char NumParam;				/**< number of filter state parameter */
	unsigned int ParamTypeMask;

	float VarXX;						/**< X variance (m^2) */
	float VarYY;						/**< Y variance (m^2) */
	float VarZZ;						/**< Z variance (m^2) */
	float VarXY;						/**< XY co-variance (m^2) */
	float VarYZ;						/**< YZ co-variance (m^2) */
	float VarZX;						/**< ZX co-variance (m^2) */

	unsigned char AtmProcessType;		/**< bit0~3:ionosphere option; bit4~7: troposphere option */
	unsigned char SWASIntegrityMark;
	unsigned char EnvLevel;				/**< 0:unknown; 1:good; 2:fail; 3:poor; 4:bad */
	unsigned char FusionType;			/**< bit0~3:phase; bit4~7:code; 0x01:geo; 0x02:GF; 0x04:IF; 0x08:WL */

	unsigned int EventFlag;				/**< bit0:filter reset;
											bit1:ionosphere option changes
											bit2:troposphere option changes
											bit3:filter iteration exceed
											bit4:observation number exceed
											bit5:parameter number exceed
											bit6:WL constraint error
											bit7:numeric error
											bit8:dynamic memory error
											bit9~31:reserved */

	unsigned char EWLFixStrategy;		/**< EWL/WL/NL ambiguity fixed strategy */
	unsigned char WLFixStrategy;
	unsigned char NLFixStrategy;
	unsigned char FixQuality;			/**< fix quality, 0~100 */

	unsigned int Reserved[6];

	unsigned char RefSatPRN[4];			/**< 1 based reference satellite PRN, G/R/E/C */

	unsigned char Reserved2[3];
	unsigned char NumSat;

	RM_EX_SAT_FILTER_INFO_T SatInfos[64];	/**< Satellite filter information */
}_PACKED_ RM_EX_FILTERDBG_DATA_T;

#define RM_EX_UVNAV_SAT_NUM_MAX			(128u)

/** Huace raw message UVNAV data struct, previous name: txxx_UVNAV_s */
typedef struct _RM_EX_UVNAV_DATA_T
{
	unsigned int DeviceID;
	unsigned int Reserved1;
	double Lat;
	double Lon;
	double Hgt;
	double AltitudeEllipsoid;
	float VelStd;
	float VelHeadingStd;
	unsigned char PosType;
	unsigned char Reserved2;
	unsigned short Reserved3;
	float PosStd;
	float HgtStd;
	float HDop;
	float VDop;
	int Noise;
	unsigned short AGC;
	unsigned char JammingState;
	unsigned char Reserved4;
	int JammingIndicator;
	unsigned char SpoofingState;
	unsigned char Reserved5[3];
	float Vel;
	float VelN;
	float VelE;
	float VelD;
	float TrkGnd;
	unsigned char VelValid;
	unsigned char Reserved6[3];
	int Reserved7;
	UINT64 UTC;
	unsigned char SolnSVs;
	unsigned char Reserved8[3];
	float Heading;
	float HeadingOffset;
	float HdgStdDev;
	float Reserved9;
	unsigned char Reserved10;
	unsigned char RtcmCrcState;
	unsigned char RtcmMsgUsed;
	unsigned int HeadingType;
	float LatStd;
	float LonStd;
	float DiffAge;
	float VerSpdStd;
	float HorSpdStd;
	unsigned short StnID;
	unsigned int VelType;
	unsigned int PosSolStat;
	unsigned short ExtSolStat;
	unsigned char SolnSVsBDS;
	unsigned char SolnSVsGPS;
	unsigned char SolnSVsGAL;
	unsigned char SolnSVsGLO;
	unsigned char SolnSVsQZSS;
	unsigned char SVsNum;
	unsigned short SVsMap[RM_EX_UVNAV_SAT_NUM_MAX];
}_PACKED_ RM_EX_UVNAV_DATA_T;

/** GPS Ephemeris Log Data Structure*/
typedef struct _RM_EPH_LOG_GPS_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_GPS_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_GPS_T;

/** BDS Ephemeris Log Data structure*/
typedef struct _RM_EPH_LOG_BDS_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_BD2_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_BDS_T;

/** BDS CNAV1/CNAV2 Ephemeris Log Data structure*/
typedef struct _RM_EPH_LOG_BDS_CNAV12_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_BDS_CNAV12_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_BDS_CNAV12_T;

/** BDS CNAV3 Ephemeris Log Data structure*/
typedef struct _RM_EPH_LOG_BDS_CNAV3_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_BDS_CNAV3_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_BDS_CNAV3_T;

/** GLO Ephemeris Log Data structure*/
typedef struct _RM_EPH_LOG_GLO_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_GLO_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_GLO_T;

/**QZSS Ephemeris Pkg*/
typedef struct _RM_EPH_LOG_QZS_T
{
	RM_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EPH_QZS_DATA_T EphData;
}_PACKED_ RM_EPH_LOG_QZS_T;

/** GAL Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EPH_LOG_GAL_T
{
	RM_HEADER_T		LogHeader;
	RM_EPH_GAL_DATA_T	EphData;
}_PACKED_ RM_EPH_LOG_GAL_T;

/** GAL INAV Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EPH_LOG_GAL_INAV_T
{
	RM_HEADER_T		LogHeader;
	RM_EPH_GAL_INAV_DATA_T	EphData;
}_PACKED_ RM_EPH_LOG_GAL_INAV_T;

/** GAL FNAV Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EPH_LOG_GAL_FNAV_T
{
	RM_HEADER_T		LogHeader;
	RM_EPH_GAL_FNAV_DATA_T	EphData;
}_PACKED_ RM_EPH_LOG_GAL_FNAV_T;


/** NavIC Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EPH_LOG_NIC_T
{
	RM_HEADER_T		LogHeader;
	RM_EPH_NIC_DATA_T	EphData;
}_PACKED_ RM_EPH_LOG_NIC_T;


typedef struct _RM_IONUTC_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_IONUTC_DATA_T  IonUtcData;
}_PACKED_ RM_IONUTC_LOG_T;

typedef struct _RM_BESTDOP_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_DOP_DATA_T	BestDopData;
}_PACKED_ RM_BESTDOP_LOG_T;

typedef struct _RM_BESTPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	BestPosData;
}_PACKED_ RM_BESTPOS_LOG_T;

typedef struct _RM_BESTVEL_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_VEL_DATA_T	BestVelData;
}_PACKED_ RM_BESTVEL_LOG_T;

typedef struct _RM_BESTXYZ_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_BESTXYZ_DATA_T	BestXYZData;
}_PACKED_ RM_BESTXYZ_LOG_T;

typedef struct _RM_BESTSATS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_BESTSATS_DATA_T	BestSatsData;
}_PACKED_ RM_BESTSATS_LOG_T;

typedef struct _RM_PSRPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	PsrPosData;
}_PACKED_ RM_PSRPOS_LOG_T;

typedef struct _RM_PSRVEL_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_VEL_DATA_T	PsrVelData;
}_PACKED_ RM_PSRVEL_LOG_T;

typedef struct _RM_RTKPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	RTKPosData;
}_PACKED_ RM_RTKPOS_LOG_T;

typedef struct _RM_RTKVEL_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_VEL_DATA_T	RTKVelData;
}_PACKED_ RM_RTKVEL_LOG_T;

typedef struct _RM_PPPPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	PPPPosData;
}_PACKED_ RM_PPPPOS_LOG_T;

typedef struct _RM_MATCHEDPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	MatchedPosData;
}_PACKED_ RM_MATCHEDPOS_LOG_T;

typedef struct _RM_SATVIS2_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_SATVIS2_DATA_T	SatVis2Data;
}_PACKED_ RM_SATVIS2_LOG_T;

typedef struct _RM_TIME_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_TIME_DATA_T	TimeData;
}_PACKED_ RM_TIME_LOG_T;

typedef struct _RM_REFSTATION_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_REFSTATION_DATA_T	RefStationData;
}_PACKED_ RM_REFSTATION_LOG_T;

typedef struct _RM_REFSTATIONINFO_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_REFSTATIONINFO_DATA_T	RefStationInfoData;
}_PACKED_ RM_REFSTATIONINFO_LOG_T;

typedef struct _RM_BASEOBS_LOG_T
{
	RM_EX_HEADER_T LogHeader;			/**< LOG header */
	unsigned short BaseID;			/**< base station id */
	unsigned short VendorTypeID;	/**< Vendor Type ID */
	unsigned int ObsNum;			/**< obs number*/
	RM_RANGE_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_BASEOBS_LOG_T;

typedef struct _RM_RTCMINSTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_RTCMINSTATUS_DATA_T	RTCMInStatusData;
}_PACKED_ RM_RTCMINSTATUS_LOG_T;

typedef struct _RM_RAWSBASFRAME2_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_RAWSBASFRAME2_DATA_T	RawSBASFrame2Data;
}_PACKED_ RM_RAWSBASFRAME2_LOG_T;

typedef struct _RM_RAWIMUX_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_RAWIMUX_DATA_T	RawIMUXData;
}_PACKED_ RM_RAWIMUX_LOG_T;

typedef struct _RM_INSPVA_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_INSPVA_DATA_T	INSPVAData;
}_PACKED_ RM_INSPVA_LOG_T;

typedef struct _RM_INSPVAX_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_INSPVAX_DATA_T	INSPVAXData;
}_PACKED_ RM_INSPVAX_LOG_T;

typedef struct _RM_DUALANTENNAHEADING_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_DUALANTENNAHEADING_DATA_T	DualAntHeadingData;
}_PACKED_ RM_DUALANTENNAHEADING_LOG_T;

typedef struct _RM_GALCNAVRAWPAGE_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_GALCNAVRAWPAGE_DATA_T	GALCNavRawPageData;
}_PACKED_ RM_GALCNAVRAWPAGE_LOG_T;

typedef struct _RM_BDSIONO_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_BDSIONO_DATA_T	BDSIonoData;
}_PACKED_ RM_BDSIONO_LOG_T;

typedef struct _RM_MARKPOS_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_POS_DATA_T	MarkPosData;
}_PACKED_ RM_MARKPOS_LOG_T;

typedef struct _RM_TILTCONFIG_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_TILTCONFIG_DATA_T	TiltConfigData;
}_PACKED_ RM_TILTCONFIG_LOG_T;

typedef struct _RM_TILTCALIB_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_TILTCALIB_DATA_T	TiltCalibData;
}_PACKED_ RM_TILTCALIB_LOG_T;

typedef struct _RM_ANTISPOOFSTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_ANTISPOOFSTATUS_DATA_T	AntiSpoofStatusData;
}_PACKED_ RM_ANTISPOOFSTATUS_LOG_T;

typedef struct _RM_FILTERDBG_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_FILTERDBG_DATA_T FilterDbgData;
}_PACKED_ RM_FILTERDBG_LOG_T;

typedef struct _RM_FILELIST_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_FILELIST_DATA_T FileListData;
}_PACKED_ RM_FILELIST_LOG_T;

typedef struct _RM_FILESTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_FILESTATUS_DATA_T FileStatusData;
}_PACKED_ RM_FILESTATUS_LOG_T;

typedef struct _RM_FILESYSTEMSTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_FILESYSTEMSTATUS_DATA_T FileSystemStatusData;
}_PACKED_ RM_FILESYSTEMSTATUS_LOG_T;

/** Huace Raw message */
/** GPS Ephemeris Log Data Structure*/
typedef struct _RM_EX_EPH_LOG_GPS_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_GPS_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_GPS_T;

/** BDS Ephemeris Log Data structure*/
typedef struct _RM_EX_EPH_LOG_BDS_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_BD2_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_BDS_T;

/** BDS CNAV1/CNAV2 Ephemeris Log Data structure*/
typedef struct _RM_EX_EPH_LOG_BDS_CNAV12_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_BDS_CNAV12_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_BDS_CNAV12_T;

/** BDS CNAV3 Ephemeris Log Data structure*/
typedef struct _RM_EX_EPH_LOG_BDS_CNAV3_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_BDS_CNAV3_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_BDS_CNAV3_T;

/** GLO Ephemeris Log Data structure*/
typedef struct _RM_EX_EPH_LOG_GLO_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_GLO_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_GLO_T;

/**QZSS Ephemeris Pkg*/
typedef struct _RM_EX_EPH_LOG_QZS_T
{
	RM_EX_HEADER_T		LogHeader;  /**< Message Header*/
	RM_EX_EPH_QZS_DATA_T EphData;
}_PACKED_ RM_EX_EPH_LOG_QZS_T;

/** GAL INAV Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EX_EPH_LOG_GAL_INAV_T
{
	RM_EX_HEADER_T		LogHeader;
	RM_EX_EPH_GAL_INAV_DATA_T	EphData;
}_PACKED_ RM_EX_EPH_LOG_GAL_INAV_T;

/** GAL FNAV Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EX_EPH_LOG_GAL_FNAV_T
{
	RM_EX_HEADER_T		LogHeader;
	RM_EX_EPH_GAL_FNAV_DATA_T	EphData;
}_PACKED_ RM_EX_EPH_LOG_GAL_FNAV_T;

/** NavIC Ephemeris Log struct, include Header and Body*/
typedef struct _RM_EX_EPH_LOG_NIC_T
{
	RM_EX_HEADER_T		LogHeader;
	RM_EX_EPH_NIC_DATA_T	EphData;
}_PACKED_ RM_EX_EPH_LOG_NIC_T;

/** GPSIONO Log struct, include Header and Body*/
typedef struct _RM_EX_GPSIONO_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_GPSIONO_DATA_T   GPSIonoData;
}_PACKED_ RM_EX_GPSIONO_LOG_T;

/** GPSUTC Log struct, include Header and Body*/
typedef struct _RM_EX_GPSUTC_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_GPSUTC_DATA_T GPSUtcData;
}_PACKED_ RM_EX_GPSUTC_LOG_T;

/** BESTDOPS Log struct, include Header and Body*/
typedef struct _RM_EX_BESTDOPS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_DOPS_DATA_T	BestDopData;
}_PACKED_ RM_EX_BESTDOPS_LOG_T;

/** BESTP Log struct, include Header and Body*/
typedef struct _RM_EX_BESTP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	BestPosData;
}_PACKED_ RM_EX_BESTP_LOG_T;

/** BESTV Log struct, include Header and Body*/
typedef struct _RM_EX_BESTV_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_VEL_DATA_T	BestVelData;
}_PACKED_ RM_EX_BESTV_LOG_T;

/** BESTECEF Log struct, include Header and Body*/
typedef struct _RM_EX_BESTECEF_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BESTECEF_DATA_T	BestECEFData;
}_PACKED_ RM_EX_BESTECEF_LOG_T;

/** BESTSATINFO Log struct, include Header and Body*/
typedef struct _RM_EX_BESTSATINFO_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BESTSATINFO_DATA_T	BestSatInfoData;
}_PACKED_ RM_EX_BESTSATINFO_LOG_T;

/** SPPP Log struct, include Header and Body*/
typedef struct _RM_EX_SPPP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	PsrPosData;
}_PACKED_ RM_EX_SPPP_LOG_T;

/** SPPV Log struct, include Header and Body*/
typedef struct _RM_EX_SPPV_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_VEL_DATA_T	PsrVelData;
}_PACKED_ RM_EX_SPPV_LOG_T;

/** RTKP Log struct, include Header and Body*/
typedef struct _RM_EX_RTKP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	RTKPosData;
}_PACKED_ RM_EX_RTKP_LOG_T;

/** RTKV Log struct, include Header and Body*/
typedef struct _RM_EX_RTKV_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_VEL_DATA_T	RTKVelData;
}_PACKED_ RM_EX_RTKV_LOG_T;

/** PPPP Log struct, include Header and Body*/
typedef struct _RM_EX_PPPP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	PPPPosData;
}_PACKED_ RM_EX_PPPP_LOG_T;

/** MATCHEDP Log struct, include Header and Body*/
typedef struct _RM_EX_MATCHEDP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	MatchedPosData;
}_PACKED_ RM_EX_MATCHEDP_LOG_T;

/** SATINVIEW Log struct, include Header and Body*/
typedef struct _RM_EX_SATINVIEW_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_SATINVIEW_DATA_T	SatInViewData;
}_PACKED_ RM_EX_SATINVIEW_LOG_T;

/** TIMES Log struct, include Header and Body*/
typedef struct _RM_EX_TIMES_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_TIMES_DATA_T	TimesData;
}_PACKED_ RM_EX_TIMES_LOG_T;

/** BASESTATION Log struct, include Header and Body*/
typedef struct _RM_EX_BASESTATION_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BASESTATION_DATA_T	BaseStationData;
}_PACKED_ RM_EX_BASESTATION_LOG_T;

/** BASESTATIONINFO Log struct, include Header and Body*/
typedef struct _RM_EX_BASESTATIONINFO_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BASESTATIONINFO_DATA_T	BaseStationInfoData;
}_PACKED_ RM_EX_BASESTATIONINFO_LOG_T;

/** BASEOBS Log struct, include Header and Body*/
typedef struct _RM_EX_BASEOBS_LOG_T
{
	RM_EX_HEADER_T LogHeader;			/**< LOG header */
	unsigned short BaseID;			/**< base station id */
	unsigned short VendorTypeID;	/**< Vendor Type ID */
	unsigned int ObsNum;			/**< obs number*/
	RM_RANGE_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_EX_BASEOBS_LOG_T;

/** RTCMINSTATUS Log struct, include Header and Body*/
typedef struct _RM_EX_RTCMINSTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_RTCMINSTATUS_DATA_T	RTCMInStatusData;
}_PACKED_ RM_EX_RTCMINSTATUS_LOG_T;

/** SBASRAWFRAME Log struct, include Header and Body*/
typedef struct _RM_EX_SBASRAWFRAME_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_SBASRAWFRAME_DATA_T	SBASRawFrameData;
}_PACKED_ RM_EX_SBASRAWFRAME_LOG_T;

/** IMUOBS Log struct, include Header and Body*/
typedef struct _RM_EX_IMUOBS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_IMUOBS_DATA_T	IMUObsData;
}_PACKED_ RM_EX_IMUOBS_LOG_T;

/** INSSOL Log struct, include Header and Body*/
typedef struct _RM_EX_INSSOL_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_INSSOL_DATA_T	INSSolData;
}_PACKED_ RM_EX_INSSOL_LOG_T;

/** INSSOLX Log struct, include Header and Body*/
typedef struct _RM_EX_INSSOLX_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_INSSOLX_DATA_T	INSSolXData;
}_PACKED_ RM_EX_INSSOLX_LOG_T;

/** DANTHEADING Log struct, include Header and Body*/
typedef struct _RM_EX_DANTHEADING_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_DANTHEADING_DATA_T	DualAntHeadingData;
}_PACKED_ RM_EX_DANTHEADING_LOG_T;

/** GALCNAVRAWPAGE Log struct, include Header and Body*/
typedef struct _RM_EX_GALCNAVRAWPAGE_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_GALCNAVRAWPAGE_DATA_T	GALCNavRawPageData;
}_PACKED_ RM_EX_GALCNAVRAWPAGE_LOG_T;

/** BD2IONO Log struct, include Header and Body*/
typedef struct _RM_EX_BD2IONO_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BD2IONO_DATA_T	BD2IonoData;
}_PACKED_ RM_EX_BD2IONO_LOG_T;

/** BD3IONO Log struct, include Header and Body*/
typedef struct _RM_EX_BD3IONO_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_BD3IONO_DATA_T	BD3IonoData;
}_PACKED_ RM_EX_BD3IONO_LOG_T;

/** EVENTP Log struct, include Header and Body*/
typedef struct _RM_EX_EVENTP_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_POS_DATA_T	EventPosData;
}_PACKED_ RM_EX_EVENTP_LOG_T;

/** TILTCONFIG Log struct, include Header and Body*/
typedef struct _RM_EX_TILTCONFIG_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_TILTCONFIG_DATA_T	TiltConfigData;
}_PACKED_ RM_EX_TILTCONFIG_LOG_T;

/** ANTISPOOFSTATUS Log struct, include Header and Body*/
typedef struct _RM_EX_ANTISPOOFSTATUS_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_ANTISPOOFSTATUS_DATA_T	AntiSpoofStatusData;
}_PACKED_ RM_EX_ANTISPOOFSTATUS_LOG_T;

/** FILTERDBG Log struct, include Header and Body*/
typedef struct _RM_EX_FILTERDBG_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_FILTERDBG_DATA_T FilterDbgData;
}_PACKED_ RM_EX_FILTERDBG_LOG_T;

/** UVNAV Log struct, include Header and Body*/
typedef struct _RM_EX_UVNAV_LOG_T
{
	RM_EX_HEADER_T	 LogHeader;  /** < Message Header*/
	RM_EX_UVNAV_DATA_T UVNavData;
}_PACKED_ RM_EX_UVNAV_LOG_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif
typedef struct _RAWMSG_ID_INFO_T
{
	unsigned int MsgID;
	char* pMsgIDStr;
} RAWMSG_ID_INFO_T;

#ifdef __cplusplus
}
#endif

#endif /**< _RAW_MSG_DATA_STRUCTURE_H_ */
