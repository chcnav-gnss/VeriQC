#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   MsgConvert Module
*-
@file    RawMsgDecodeDataStructure.h
@author  CHC
@date    2023/10/26
@brief   chcoem raw data encode function and details

**************************************************************************/
#ifndef _RAW_MSG_DECODE_DATA_STRUCTURE_H_
#define _RAW_MSG_DECODE_DATA_STRUCTURE_H_

#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KMD_EPH_GPS_MIN_PRN		(1)
#define KMD_EPH_GPS_MAX_PRN		(37)

#define KMD_EPH_GLO_MIN_PRN		(38)
#define KMD_EPH_GLO_MAX_PRN		(65)

#define KMD_EPH_GAL_MIN_PRN		(97)
#define KMD_EPH_GAL_MAX_PRN		(136)

#define KMD_EPH_BDS_MIN_PRN		(161)
#define KMD_EPH_BDS_MAX_PRN		(223)

typedef struct _RANGE_DATA_PER_SAT_T
{
	unsigned int ObsNum;
	RM_RANGE_DATA_T ObsData[MAX_FREQ_INDEX_NUM+1]; /**< obs data*/
} RANGE_DATA_PER_SAT_T;

typedef struct _OBS_DATA_PER_SYS_T
{
	unsigned int SatNum;
	RANGE_DATA_PER_SAT_T SatRangeData[64]; /**< obs data*/
} OBS_DATA_PER_SYS_T;

typedef struct _GNSS_OBS_DATA_SAT_T
{
	OBS_DATA_PER_SYS_T SysObsData[RM_RANGE_SYS_OTHER]; /**< obs data*/
} GNSS_OBS_DATA_SAT_T;

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

typedef struct _RM_EPH_UN_BD3_DATA_T
{
	unsigned char	PRN;		/**< satellite PRN number, based on MIN_BDS_RANGE_PRN(1) */
	unsigned char	Health;		/**< Health status -0=healthy, 1=unhealthy*/
	unsigned char	SatType;	/**< 2 bit orbit type */
	unsigned char	SISMAI;

	unsigned short	IODE;		/**< Issue of ephemeris data */
	unsigned short	IODC;		/**< Issue of data, clock */

	unsigned short	Week;		/**< TOE Week number in BDS time (BDT) */
	unsigned short	Zweek;		/**< Z count week number, this is week number from subframe 1 of the ephemeris, */
								/**< the toe's week is derived from this to account for roll over*/

	double			TOW;		/**< Time stamp of subframe 1 (seconds)*/
	double			TOE;		/**< Reference time of ephemeris parameters in BeiDou time (BDT). (s) */

	double			DeltaA;		/**< Semi-major axis difference at reference time (m) */
	double			dDeltaA;	/**< Semi-major axis difference at reference time (m/s) */
	double			DeltaN;		/**< Mean motion difference from computed value (radians/s) */
	double			dDeltaN;	/**< Mean motion difference from computed value (radians/s^2) */
	double			M0;			/**< Mean anomaly of reference time , radians*/
	double			Ecc;		/**< Eccentricity, dimensionless */
	double			W;			/**< Argument of perigee, radians*/
	double			Cuc;		/**< Argument of latitude ( amplitude of cosine, radians)*/
	double			Cus;		/**< Argument of latitude (amplitude of sine, meters) */
	double			Crc;		/**< Orbit radius (amplitude of cosine, meters)*/
	double			Crs;		/**< Orbit radius (amplitude of sine, radians) */
	double			Cic;		/**< Inclination (amplitude of cosine, radians) */
	double			Cis;		/**< Inclination (amplitude of sine, radians) */
	double			I0;			/**< Inclination angel at reference time, radians */
	double			I0Dot;		/**< Rate if inclination angle, radians/second */
	double			Omega0;		/**< Longitude of ascending node of orbital of planeat weekly epoch, radians */
	double			OmegaDot;	/**< Rate of right ascension, radians/second */
	double			TOC;		/**< Reference time of clock parameters in BeiDou time (BDT).*/
	double			TGDB1Cp;	/**< Group delay differential between the B1C pilot component and the B3I signal (s) */
	double			TGDB2Ap;	/**< Group delay differential between the B2a pilot component and the B3I signal (s) */
	double			ISCb2ad;	/**< Group delay differential between the B2a data and pilot components (s) */
	double			ISCb1cd;	/**< Group delay differential between the B1c data and pilot components (s) */
	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/

	int				iTop;
	unsigned char	SISAIoe;
	unsigned char	SISAIocb;
	unsigned char	SISAIoc1;
	unsigned char	SISAIoc2;

	int				Reserved1;
	int				Reserved2;
	unsigned int	FreqType;	/**< Bit 0: B1C, Bit 1: B2A, Bit 2: B2b */
}_PACKED_ RM_EPH_UN_BD3_DATA_T;

/** raw message BDS ephemeris struct, previous name: txxx_EPH_BD_s */
typedef struct _RM_EPH_UN_BD2_DATA_T
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
	double			Tgd1;          /**< Estimated group delay difference, seconds*/
	double			Tgd2;          /**< Estimated group delay difference, seconds*/
	double			Af0;           /**< Clock aging parameter, seconds(s)*/
	double			Af1;           /**< Clock aging parameter, (s/s)*/
	double			Af2;           /**< Clock aging parameter, (s/s/s)*/
	unsigned int	ASFlag;        /**< Anti-spooning Flag, 0=false, 1=TRUE*/
	double			N;             /**< Corrected mean motion,  radians/second*/
	double			URA;           /**< User Range Accuracy variance, m'2*/
}_PACKED_ RM_EPH_UN_BD2_DATA_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif /**< _RAW_MSG_DECODE_DATA_STRUCTURE_H_ */

#endif

