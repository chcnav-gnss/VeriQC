#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RawMsg Binary Decoder Module
*-
@file   UNRawMsgBinaryDataTypes.h
@author CHC
@date   2023/09/12
@brief

**************************************************************************/
#ifndef _UN_RAWMSG_BINARY_DATA_TYPES_H_
#define _UN_RAWMSG_BINARY_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"
#include "../RawMsgAscii/RawMsgDecodeDataStructure.h"

#include "Common/TimeDefines.h"

#define UN_RAWMSG_ID_UNOBSVM						(12)
#define UN_RAWMSG_ID_UNOBSVH						(13)
#define UN_RAWMSG_ID_UNOBSVBASE						(284)
#define UN_RAWMSG_ID_UNBASEINFO						(176)
#define UN_RAWMSG_ID_UNGPSEPH						(106)
#define UN_RAWMSG_ID_UNBD3EPH						(2999)
#define UN_RAWMSG_ID_UNBDSEPH						(108)
#define UN_RAWMSG_ID_UNGLOEPH						(107)
#define UN_RAWMSG_ID_UNGALEPH						(109)
#define UN_RAWMSG_ID_UNIHEADING						(972)
#define UN_RAWMSG_ID_GPSION							(8)
#define UN_RAWMSG_ID_BD3ION							(21)
#define UN_RAWMSG_ID_BDSION							(4)
#define UN_RAWMSG_ID_GALION							(9)
#define UN_RAWMSG_ID_GPSUTC							(19)
#define UN_RAWMSG_ID_BD3UTC							(22)
#define UN_RAWMSG_ID_BDSUTC							(2012)
#define UN_RAWMSG_ID_GALUTC							(20)
#define UN_RAWMSG_ID_AGRIC							(11276)
#define UN_RAWMSG_ID_RAWDATA						(8018)
#define UN_RAWMSG_ID_RAWDATAH						(8019)
#define UN_RAWMSG_ID_STADOP							(954)
#define UN_RAWMSG_ID_STADOPH						(2122)
#define UN_RAWMSG_ID_ADRDOP							(953)
#define UN_RAWMSG_ID_ADRDOPH						(2121)
#define UN_RAWMSG_ID_PPPDOP							(1025)
#define UN_RAWMSG_ID_SPPDOP							(173)
#define UN_RAWMSG_ID_SPPDOPH						(2120)
#define UN_RAWMSG_ID_PVTSLN							(1021)
#define UN_RAWMSG_ID_BESTNAV						(2118)
#define UN_RAWMSG_ID_BESTNAVXYZ						(240)
#define UN_RAWMSG_ID_BESTNAVH						(2119)
#define UN_RAWMSG_ID_BESTNAVXYZH					(242)
#define UN_RAWMSG_ID_BESTSAT						(1041)
#define UN_RAWMSG_ID_ADRNAV							(142)
#define UN_RAWMSG_ID_ADRNAVH						(2117)
#define UN_RAWMSG_ID_PPPNAV							(1026)
#define UN_RAWMSG_ID_SPPNAV							(46)
#define UN_RAWMSG_ID_SPPNAVH						(2116)
#define UN_RAWMSG_ID_SATELLITE						(1042)
#define UN_RAWMSG_ID_SATECEF						(2115)
#define UN_RAWMSG_ID_RECTIME						(102)
#define UN_RAWMSG_ID_UNIHEADING2					(1331)
#define UN_RAWMSG_ID_RTKSTATUS						(509)
#define UN_RAWMSG_ID_RTCSTATUS						(510)
#define UN_RAWMSG_ID_JAMSTATUS						(511)
#define UN_RAWMSG_ID_HWSTATUS						(218)
#define UN_RAWMSG_ID_AGC							(220)
#define UN_RAWMSG_ID_INFOPART1						(1019)
#define UN_RAWMSG_ID_INFOPART2						(1020)

#define UN_RAWMSG_BINARY_MSG_PARSED_LEN_MAX							(1024 * 1024) /** rangea message parsed data struct length max, unit: byte*/
#define UN_RAWMSG_BINARY_PARSED_BASEOBS_SAT_COUNT_LEN_MAX				(1 * 1024) /** RawMsg message parsed baseobs sat count length max, unit: byte*/

#if !defined( __GCC__)
#pragma pack(push,1)
#endif
/** raw message header, previous name: _RAW_MSG_HEADER_T */
typedef struct _UN_RM_HEADER_T
{
	unsigned char	Sync[3];
	unsigned char	CPUIDle;
	unsigned short	MsgID;
	unsigned short	MsgLen;
	unsigned char	TimeRef;
	unsigned char	TimeStatus;
	unsigned short	Week;
	unsigned int	Milliseconds;
	unsigned int	Reserved;
	unsigned char	Version;
	unsigned char	LeapSec;
	unsigned short	DelayMs;
}_PACKED_ UN_RM_HEADER_T;

/** raw message range data struct, previous name: txxxRange_s */
typedef struct _UN_RM_OBSV_DATA_T
{
	unsigned short  FreqNum;        /**< GLONASS Frequency number, 0 ~ 13 */
	unsigned short  PRN;            /**< GPS: 1 to 32, SBAS: 120 to 138 or 183-187   GLONASS: 38 to 61  Galileo: 1 to 36, QZSS: 193-197, BDS: 1-37 */
	double    PSR;                  /**< pseudorange measurement (m) */
	double    ADR;                  /**< carrier phase, in cycles */
	unsigned short PSRStd;               /**< pseudorange measurement standard deviation (m) */
	unsigned short ADRStd;               /**< estimated carrier phase standard deviation (cycles) */
	float     Doppler;              /**< instantaneous carrier Doppler frequency (Hz) */
	unsigned short     CN0;                  /**< carrier to noise density ratio (dB-Hz) */
	unsigned short Reserved;
	float     LockTime;             /**< number of seconds of continuous tracking (no cycle slipping) */
	unsigned int ChnTrackStatus;    /**< Channel tracking status
										 bit  0 -  4, 5 bits, Tracking State
										 bit  5 -  9, 5 bits, SV Channel number, 0 = first, n = last
										 bit 10,      1 bit,  Phase lock flag, 0 = not locked, 1 = locked
										 bit 11,      1 bit,  Parity known flag, 0 = not known, 1 = known
										 bit 12,      1 bit,  Code locked flag, 0 = not locked, 1 = locked
										 bit 13 - 15, 3 bits, Correlator type, 0 - 7
										 bit 16 - 18, 3 bits, Satellite system
										 bit 19,      1 bit,  reserved
										 bit 20,      1 bit,  Grouping, 0 = not grouped, 1 = grouped
										 bit 21 - 25, 5 bits, Signal Type
										 bit 26,      1 bit,  Reserved
										 bit 27,      1 bit,  Primary L1 channel, 0 = not primary, 1 = primary
										 bit 28,      1 bit,  Carrier phase measurement, 0 = half cycle not added, 1 = half cycle added
										 bit 29,      1 bit,  Digital filtering on signal, 0 = no digital filter, 1 = digital filter
										 bit 30,      1 bit,  PRN lock flag, 0 = PRN not locked Out, 1 = PRN locked out
										 bit 31,      1 bit,  Channel assignment, 0 = automatic, 1 = forced */
}_PACKED_ UN_RM_OBSV_DATA_T;

typedef struct _UN_RM_BASEINFO_DATA_T
{
	unsigned int Status;		/**< Status of the base station information */
	double ECEFX;				/**< ECEF X value (m) */
	double ECEFY;				/**< ECEF Y value (m) */
	double ECEFZ;				/**< ECEF Z value (m) */
	char StationID[8];			/**< Base station ID */
	unsigned int	Reserved;
} _PACKED_ UN_RM_BASEINFO_DATA_T;

/** raw message GPS ephemeris struct, previous name: txxx_EPH_GPS_s */
typedef struct _UN_RM_EPH_GPS_DATA_T
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
}_PACKED_ UN_RM_EPH_GPS_DATA_T;

/** raw message BDS CNAV3 ephemeris struct */
typedef struct _UN_RM_EPH_BD3_DATA_T
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

	double			TOW;           /**< Time stamp of subframe 1 (seconds)*/
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
	double			TGDB2bi;	/**< Group delay differential between the B1C pilot component and the B2bI signal (s) */
	double			TGDB2bq;	/**< Group delay differential between the B2a pilot component and the B2bQ signal (s) */
	double			Af0;		/**< Constant term of clock correction polynomia, (s)*/
	double			Af1;		/**< Linear term of clock correction polynomia, (s/s)*/
	double			Af2;		/**< Quadratic term of clock correction polynomial, (s/s/s)*/

	int				iTop;
	unsigned char	SISAIoe;
	unsigned char	SISAIocb;
	unsigned char	SISAIoc1;
	unsigned char	SISAIoc2;

	unsigned int	Reserved;
	unsigned int	FreqType;	/**< Bit 0: B1C, Bit 1: B2A, Bit 2: B2b */
}_PACKED_ UN_RM_EPH_BD3_DATA_T;

/** raw message BDS ephemeris struct, previous name: txxx_EPH_BD_s */
typedef struct _UN_RM_EPH_BD2_DATA_T
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
}_PACKED_ UN_RM_EPH_BD2_DATA_T;

/** raw message GLONASS ephemeris struct, previous name: txxx_EPH_GLN_s */
typedef struct _UN_RM_EPH_GLO_DATA_T
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
}_PACKED_ UN_RM_EPH_GLO_DATA_T;

/** raw message Galileo ephemeris struct, previous name: txxx_EPH_GAL_s */
typedef struct _UN_RM_EPH_GAL_DATA_T
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
}_PACKED_ UN_RM_EPH_GAL_DATA_T;

/** raw message UNIHeading struct */
typedef struct _UN_RM_HEADING_DATA_T
{
	int SolStatus;					/**< Solution status */
	int PosType;					/**< Position type */
	float Length;					/**< Baseline length in meters */
	float Heading;					/**< Heading in degrees(0-359.999) */
	float Pitch;					/**< Pitch(+-90 degrees)*/
	float Reserved;					/**< Reserved */  
	float HdgStdDev;				/**< Heading standard deviation in degrees */
	float PtchStdDev;				/**< Pitch standard deviation in degrees */
	char StnID[4];					/**< Station ID string */
	unsigned char TrackSVs;			/**< Number of satellites tracked */
	unsigned char SolSVs;			/**< Number of satellites used in solution */
	unsigned char ObsNum;			/**< Number of satellites above the elevation mask angle */
	unsigned char SolMutiSVs;		/**< Number of satellites with multi-frequency signals used in solution */
	unsigned char Reserved2;		/**< Reserved */
	unsigned char ExtSolStatus;		/**< Extended solution status
										 bit 0, 1 bits, If an RTK solution: an RTK solution has been verified
													If a PDP solution: solution is GLIDE
													Otherwise: Reserved
										bit 1 - 3, 3 bits, Pseudorange Iono Correction
													0 = Unknown or default Klobuchar model
													1 = Klobuchar Broadcast
													2 = SBAS Broadcast
													3 = Multi-frequency Computed
													4 = PSRDiff Correction*/
	unsigned char GALSignalMask;	/**< Galileo and BeiDou signals used mask
										bit 0, 1 bits, Galileo E1 used in Solution
										bit 1, 1 bits, Galileo E5b used in Solution
										bit 2, 1 bits, Galileo E5a used in Solution
										bit 3-7, Reserved*/
	unsigned char GPSGLOBDSSignalMask;	/**< GPS and GLONASS signals used mask
										bit 0, 1 bits, GPS L1 used in Solution
										bit 1, 1 bits, GPS L2 used in Solution
										bit 2, 1 bits, GPS L5 used in Solution
										bit 3, 1 bits, BDS B3 used in Solution
										bit 4, 1 bits, GLONASS L1 used in Solution
										bit 5, 1 bits, GLONASS L2 used in Solution
										bit 6, 1 bits, BDS B1 used in Solution
										bit 7, 1 bits, BDS B2 used in Solution */
}_PACKED_ UN_RM_HEADING_DATA_T;

typedef struct _UN_RM_OBSV_LOG_T
{
	UN_RM_HEADER_T LogHeader;         	/**< LOG header */
	unsigned int ObsNum;			/**< obs number*/
	UN_RM_OBSV_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ UN_RM_OBSV_LOG_T;

typedef struct _UN_RM_BASEINFO_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	UN_RM_BASEINFO_DATA_T	BaseInfoData;
}_PACKED_ UN_RM_BASEINFO_LOG_T;

typedef struct _UN_RM_HEADING_LOG_T
{
	RM_HEADER_T	 LogHeader;  /** < Message Header*/
	UN_RM_HEADING_DATA_T	HeadingData;
}_PACKED_ UN_RM_HEADING_LOG_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif


typedef struct _UN_RAWMSG_BINARY_INTERIM_DATA_T
{
	unsigned int BaseObsSatNum[RM_RANGE_SYS_OTHER];

	unsigned int MsgGPSWeek;
	unsigned int MsgGPSMSecs;
	UTC_TIME_T MsgUTCTime;
} UN_RAWMSG_BINARY_INTERIM_DATA_T;

typedef struct _UN_RAWMSG_BINARY_PARSE_INFO_T
{
	UN_RAWMSG_BINARY_INTERIM_DATA_T UNRawMsgBinaryInterimData;    /**< raw msg binary decode interim data */
	MSG_BUF_T* pDecodeBuf;                  /**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;                /**< address stored after data parsed */

	MSG_BUF_T ParsedBaseObsSatCountBuf;     /**< parsed BaseObs satellite count information buffer */
} UN_RAWMSG_BINARY_PARSE_INFO_T;

#ifdef __cplusplus
}
#endif

#endif

#endif /**< _UN_RAWMSG_BINARY_DATA_TYPES_H_ */

