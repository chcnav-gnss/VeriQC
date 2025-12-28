/**********************************************************************//**
		VeriQC

		PVT Module
*-
@file   GNSSNavDataType.h
@author CHC
@date   2023/05/24 11:57:27
@brief  common functions of navigation data type

**************************************************************************/

#ifndef _GNSS_NAV_DATA_TYPE_H_
#define _GNSS_NAV_DATA_TYPE_H_

#include "Common/DataTypes.h"                    /* INT64 */

/** Navigation Data State */
#define NAV_DATA_STATE_INVALID			0		/**< not valid */
#define NAV_DATA_STATE_UNKNOWN			1		/**< not judged yet, e.g. read from flash before time check or never decoded */
#define NAV_DATA_STATE_VALID			2		/**< valid for use, include unhealth state */

/** Eph data type */
#define EPH_DATA_TYPE_UNKNOWN           0
#define EPH_DATA_TYPE_GPS               1
#define EPH_DATA_TYPE_GPS_CNAV          2
#define EPH_DATA_TYPE_GLO               3
#define EPH_DATA_TYPE_GLO_CDMA          4
#define EPH_DATA_TYPE_GAL_INAV          5
#define EPH_DATA_TYPE_GAL_FNAV          6
#define EPH_DATA_TYPE_GAL_CNAV          7
#define EPH_DATA_TYPE_BDS_BD2           8
#define EPH_DATA_TYPE_BDS_CNAV          9
#define EPH_DATA_TYPE_QZS               10
#define EPH_DATA_TYPE_NIC               11
#define EPH_DATA_TYPE_SBA               12
#define EPH_DATA_TYPE_GAL_REDUCED_CED   13
#define ALM_DATA_TYPE_SBA               24


/** satellite type defined in the BDS 3 nav data*/
#define BD3_SAT_TYPE_NONE             0
#define BD3_SAT_TYPE_GEO	          1
#define BD3_SAT_TYPE_IGSO	          2
#define BD3_SAT_TYPE_MEO	          3


/** IONO Parameters structure */
typedef struct _IONO_PARAM_T
{
	float	Alpha0;  /**< 2**-30 */
	float	Alpha1;  /**< 2**-27 */
	float	Alpha2;  /**< 2**-24 */
	float	Alpha3;  /**< 2**-24 */
	float	Beta0;   /**< 2**11 */
	float	Beta1;   /**< 2**14 */
	float	Beta2;   /**< 2**16 */
	float	Beta3;   /**< 2**16 */
	unsigned int	 IonoFlag; /**< 0 - invalid, changed unsigned long to u32
									1 - decoder from navigation
									2 -  PVT set default valid */
} IONO_PARAM_T;

typedef struct _GLO_EPHEMERIS_T   /**< ephemeris type for glonass satellite */
{
	unsigned char     EphState;   /**< NAV_DATA_STATUS_X */
	unsigned char     EphDataType;/**< data type, EPH_DATA_TYPE_X */
	unsigned char     SatID;      /**< slot number that transmit signal,get by string 4, start from 1-27 */
	  signed char     Freq;       /**< frequency number of satellite,[-7,6] */


	unsigned short    StatusFlag; /**< 11 bits, P1 bit0-1, P2 bit 2, P3 bit3, P4 bit4, ln3 bit5, P bit6-7, M:bit8-9, ln5 bit10 */
								  /**< P1  -- 2bits, flag of the immediate data updating
									   P2  -- 1bit,  flag of oddness(1) or evenness(0) of the value of tb
									   P3  -- 1bits, flag indicating a number of satellites for which almanac is transmitted
									   P4  -- 1bits, flag to show that eph param are present
									   ln3 -- 1bits, decoded from 3th string
									   P   -- 2bits, technological parameters
									   M   -- 2bits, type of satellite transmitting navigation signal
									   ln5 -- 1bits, decoded form 5th string */
	unsigned char     Reserved[3];/**< reserved for 4 bytes align */

	unsigned char     RawTb;      /**< original tb in nav data */

	unsigned char     Bn;         /**< healthy Flag, 3bits, bit0 & bit1 is useless, bit2 is health Flag,1-unhealthy,0-OK */
	unsigned char     Ft;		  /**< indicator of accuracy of measurements */
	unsigned char     En;		  /**< 5bits, age of the immediate information, [0-31] days */
	unsigned char     LeapYear;   /**< four-year interval number starting form 1996, get by N4 in string4 */

	unsigned short    Tk;		  /**< 12bits, time referenced to the beginning of the frame within the current day
									   hour:bit76-bit72, min:bit71-bit66, sec:bit65*30 */

	unsigned short    Nt;         /**< calendar number of day within four-year interval starting from a leap year, [1 ~ 1461]
									   decoded from string4, for eph data related */
	unsigned short    Na;         /**< calendar number of day within four-year interval starting from a leap year, which TaoC referenced [1 ~ 1461]
									   decoded from string5, for Tc and other almanac data related */
	unsigned int      Tb;         /**< index of interval within current day(toe),S */

	double            TauN;       /**< SV clock bias(s) */
	double            DeltaTauN;  /**< time difference between L1 and L2 */
	double            GammaN;     /**< SV relative frequency bias */
	double            TaoC;       /**< difference between glonass system time and UTC(SU), range +- 1 sec */
	double            TaoGPS;     /**< Correction to GPS time relative to GLonass time, from string 5 */

	double            PosXYZ[3];  /**< position in PZ-90 at instant tb,m */
	double            VelXYZ[3];  /**< velocity in PZ-90 at instant tb,m/s */
	double            AccXYZ[3];  /**< acceleration in PZ-90 at instant tb,m/s^2 */

/** interim variables from basic ephemeris parameter, avoid unnecessary calculation every time */
	unsigned int      IODE;       /**< IODE (0-6 bit of tb field) */
	unsigned int      TOE;        /**< epoch of ephemeris (gpst) */
}GLO_EPHEMERIS_T;

typedef struct _UNI_EPHEMERIS_T       /**< ephemeris type for non-glonass satellite */
{
	unsigned char     EphState;         /**< NAV_DATA_STATUS_X */
	unsigned char     EphDataType;      /**< data type, not used at present, EPH_DATA_TYPE_X */
	unsigned char     SystemID;         /**< SystemID, as defined in GNSSSignal.h  */
	unsigned char     SatID;            /**< all system based on 1, GPS(1-32), GAL(1-36), BDS(1-64), QZS(1-10) */

	unsigned char     SatType;          /**< Special for BDS CNAV,, BD3_SAT_TYPE_XXX,  0 -- GEO, 1 -- IGSO, 3 -- METO */
	unsigned char     Health;           /**< GPS/QZS          :bit0, 1-unhealthy,0-OK
											 GAL              :bit0-1 for E1b, bit2-3 for E5b, bit4-5 for E5a
											 BDS              :bit0, satH1(1-unhealthy,0-OK)
											 BDS CNAV         :bit0-1, 0 for OK, 1 for unhealthy or test, 2 - reserved, 3 - reserved
											 NavIC            :bit0 for L5 SPS, bit1 for S SPS;  (1-unhealthy,0-OK)
											 GPS CNAV         :bit0:L1; bit1:L2; bit2:L5 */
	unsigned char     DataValidStatus;  /**< special for GAL, Data Valid Status, 0 -- Nav data valid, 1 -- working without guarantee
											 bit 0 for E1B, bit 1 for E5b, bit 2 for E5a */
	unsigned char     NavDataSource;    /**< Special for GAL INav, bit0 for E1B, bit1 for E5b; 0 -- unknown, 1 -- E1B, 2 -- E5b, 3 -- E1B and E5b
											 Special for BDS CNAV, bit 0 for CNVA1(B1C), bit 1 for CNAV2(B2a), bit 2 for CNAV3(B2b) */

	unsigned char     FitInterval;      /**< special for GPS/QZSS, Curve fit interval,
											 0 -- eph data are effective for 2hours, 1 -- eph data are effective for > 2hours */
	unsigned char     URA;              /**< GPS/QZS/BDS: URA, GAL:SISA Index, NIC:URA, BDS CNAV: SISMAI */
	unsigned short    Reserved;         /**< special for NAVIC, 10 bit reserved in subframe1
											 special for BDS CNAV, bit 0-2, for B1C DIF, SIF, AIF, bit 3-5 for B2a DIF, SIF, AIF, bit 6-8 for B2b DIF, SIF, AIF */

	unsigned char     NICSpare1;        /**< special for NAVIC, 2 bit spare in subframe1 */
	unsigned char     NICSpare2;        /**< special for NAVIC, 2 bit spare in subframe2 */
	unsigned char  	  AlertFlag;        /**< special for NAVIC, the utilization of navigation data shall be at the user's own risk, 1 -- alert, 0 -- OK */
	unsigned char 	  AutoNav;          /**< special for NAVIC */

	int               TOW;              /**< Special for GPS/QZS, Time Stamp of subframe1 (s) */

	unsigned short    Week;             /**< Week Number of TOE, GPS/QZS: GPS week; BDS: BDS week; GAL: GAL week, NIC: NIC Week */
	unsigned short    ZWeek;            /**< week count, decoded from navdata, GPS/QZS: GPS week; BDS: BDS week; GAL: GAL week, NIC: NIC Week */

	unsigned short    Reserved0;        /**< for 8 byte align */
	unsigned short    IODC;             /**< GPS/QZS IODC, BDS AODC, GAL/NIC -- */
	int               TOC;              /**< Reference time of clock parameters, second in week, GAL T0C */
	double            Af0;              /**< SV clock bias, seconds */
	double            Af1;              /**< SV clock drift, seconds/seconds */
	double            Af2;              /**< SV clock drift rate, seconds/seconds^2 */


	double            Tgd[3];           /**< GPS/QZS/GPS CNAV :Tgd[0]=TGD
											 GAL              :Tgd[0]=BGD E5a/E1,Tgd[1]=BGD E5b/E1
											 BDS              :Tgd[0]=BGD1,Tgd[1]=BGD2
											 BDS CNAV         :Tgd[0]=TgdB1cP,Tgd[1]=TgdB2aP,Tgd[2]=TgdB2bI */
	double            Isc[4];           /**< GPS CNAV         :Isc[0]=IscL1CA,Isc[1]=IscL2C,Isc[2]=IscL5I5,Isc[3]=IscL5Q5
											 BDS CNAV         :Isc[0]=ISCB1cd,Isc[1]=ISCB2ad */

	unsigned short    IODE1;            /**< GPS IODE1, QZS IODE, GAL IODnav, BDS AODE, BDS CNAV IODE, NIC IODEC */
	unsigned short    IODE2;            /**< GPS IODE2 */
	int               TOE;              /**< Reference time for ephemeris, second in week, GAL T0E */
	double            SqrtAorDeltaA;    /**< SqrtA for non-CNAV Eph, Square Root of the Semi-Major Axis, sqrt(meters)
											 DeltaA for CNAV Eph, semo-major axis difference at reference time, for BDS CNAV, GPS CNAV */
	double            Adot;             /**< correction for A (GPS/BDS CNAV) */
	double            DeltaN;           /**< Mean Motion Difference from Computed Value, radiant/second */
	double            Ndot;             /**< correction for deltN (GPS/BDS CNAV) */
	double            M0;               /**< Mean Anomaly at Reference Time, radiant */
	double            Ecc;              /**< Eccentricity */
	double            Omega;            /**< Argument of Perigee, radiant */
	double            Cuc;              /**< Amplitude of the Cosine Harmonic Correction Term to the Argument of Latitude, radiant */
	double            Cus;              /**< Amplitude of the Sine Harmonic Correction Term to the Argument of Latitude, radiant */
	double            Crc;              /**< Amplitude of the Cosine Harmonic Correction Term to the Orbit Radius, meters */
	double            Crs;              /**< Amplitude of the Sine Harmonic Correction Term to the Orbit Radius, meters */
	double            Cic;              /**< Amplitude of the Cosine Harmonic Correction Term to the Angle of Inclination, radiant */
	double            Cis;              /**< Amplitude of the Sine Harmonic Correction Term to the Angle of Inclination, radiant */
	double            I0;               /**< Inclination Angle at Reference Time, radiant */
	double            IDot;             /**< Rate of Inclination Angle, radiant/seconds */
	double            Omega0;           /**< Longitude of Ascending Node of Orbit Plane at Weekly Epoch, radiant */
	double            OmegaDot;         /**< Rate of Right Ascension, radiant/second */

/** interim variables from basic ephemeris parameter, avoid unnecessary calculation every time */
	double            A;                /**< the Semi-Major Axis, meters */
	double            RootEcc;          /**< Square Root of One Minus Ecc Square, sqrt(1-Ecc^2)*/
	double            N;                /**< Corrected Mean Angular Rate, (sqrt(GM/A^3) + DeltaN), radiant/seconds*/
	double            OmegaT;           /**< Longitude of Ascending Node of Orbit Plane at toe, (Omega0 + OMEGDOTE*toe), radiant */
	double            OmegaDelta;       /**< Delta Between OmegaDot and OMEGDOTE, (OmegaDot - OMEGDOTE), radiant/seconds */
	double            Ek;               /**< Eccentric Anomaly, will be changed every epoch, radiant */
}UNI_EPHEMERIS_T;

#endif
