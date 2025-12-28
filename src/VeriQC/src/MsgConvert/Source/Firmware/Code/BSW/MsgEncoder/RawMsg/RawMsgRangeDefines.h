/**********************************************************************//**
		VeriQC

		Raw Message Encoder Module
*-
@file   RawMsgRangeDefine.h
@author CHC
@date   2022/12/28
@brief define range message

@note RM stands for Raw Message
**************************************************************************/
#ifndef _RAW_MSG_RANGE_DEFINES_H_
#define _RAW_MSG_RANGE_DEFINES_H_

#include "Common/GNSSSignal.h"
#include "Common/CommonFuncs.h"
#include "Common/DataTypes.h"

#include "RawMsgCommonDefines.h"


/**Max observation number for one range log*/
#define RM_RANGE_MAX_OBS_NUM		(256)

/** Max satellite number of one Range log */
#define RM_RANGE_MAX_SAT_NUM    (64)




#define MSG_IS_SHORT_FORMAT(_MsgType)	((_MsgType) & RAW_MSG_TYPE_BIT_MASK_SHORT)


/** Definitions of judge system by RANGE PRN*/
#define IS_GPS_RANGE_PRN(_Prn)    (((_Prn) >= MIN_GPS_RANGE_PRN)     && ((_Prn) <= MAX_GPS_RANGE_PRN))
#define IS_GLO_RANGE_PRN(_Prn)    (((_Prn) >= MIN_GLONASS_RANGE_PRN) && ((_Prn) <= MAX_GLONASS_RANGE_PRN))
#define IS_BDS_RANGE_PRN(_Prn)    (((_Prn) >= MIN_BDS_RANGE_PRN)     && ((_Prn) <= MAX_BDS_RANGE_PRN))
#define IS_GAL_RANGE_PRN(_Prn)    (((_Prn) >= MIN_GALILEO_RANGE_PRN) && ((_Prn) <= MAX_GALILEO_RANGE_PRN))
#define IS_QZSS_RANGE_PRN(_Prn)   (((_Prn) >= MIN_QZSS_RANGE_PRN)    && ((_Prn) <= MAX_QZSS_RANGE_PRN))
#define IS_NAVIC_RANGE_PRN(_Prn)  (((_Prn) >= MIN_NAVIC_RANGE_PRN)   && ((_Prn) <= MAX_NAVIC_RANGE_PRN))



/** Definitions for Rangecmp*/
#define MAX_VALUE_RANGECMP      (8388608)               /**< for calculate Rangecmp ADR  */
#define MAX_LOCKTIME_RANGECMP 	(2097151)           	/**< for LockTime of Rangecmp*/

/** used in range*/
typedef enum _RM_RANGE_SYSTEM_T
{
	RM_RANGE_SYS_GPS	= 0,
	RM_RANGE_SYS_GLO	= 1,
	RM_RANGE_SYS_SBA	= 2,
	RM_RANGE_SYS_GAL	= 3,
	RM_RANGE_SYS_BDS	= 4,
	RM_RANGE_SYS_QZS	= 5,
	RM_RANGE_SYS_NIC 	= 6,
	RM_RANGE_SYS_OTHER = 7,
	RM_RANGE_SYS_MAX   = 0xFFFFFFFF,

}RM_RANGE_SYSTEM_T;

/** used in CHC GNSSOBS, define fallow with RTCM */
typedef enum _RM_GNSSOBS_SYSTEM_T
{
	RM_GNSSOBS_SYS_GPS = 0,
	RM_GNSSOBS_SYS_GLO = 1,
	RM_GNSSOBS_SYS_GAL = 2,
	RM_GNSSOBS_SYS_SBA = 3,
	RM_GNSSOBS_SYS_QZS = 4,
	RM_GNSSOBS_SYS_BDS = 5,
	RM_GNSSOBS_SYS_NIC = 6,
	RM_GNSSOBS_SYS_OTHER = 7,
	RM_GNSSOBS_SYS_MAX = 0xFFFFFFFF,

}RM_GNSSOBS_SYSTEM_T;

/** used in RANGECMP4 SATVIS RTK/POS_SAT*/
typedef enum _RM_NV_SYSTEM_T
{
	RM_NV_SYS_GPS	= 0,
	RM_NV_SYS_GLO	= 1,
	RM_NV_SYS_SBA	= 2,

	RM_NV_SYS_GAL	= 5,
	RM_NV_SYS_BDS	= 6,
	RM_NV_SYS_QZS	= 7,
	RM_NV_SYS_NIC	= 9,

	RM_NV_SYS_MAX   = 0xFFFFFFFF,

}RM_NV_SYSTEM_T;



/** signal type for range */
#define RM_RANGE_SIGNAL_GPSL1CA				0   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL2CA				1   /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL1P				4   /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL2P				5   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL5I				6   /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL1W				8   /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL2PCODELESS	    9   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL5X				10  /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL1CD				12  /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL2CL				13  /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL5Q				14  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL1CP				16  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL2CM				17  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GPSL1CX				20  /**< range signal type definitions,extension*/
#define RM_RANGE_SIGNAL_GPSL2CX				21  /**< range signal type definitions,extension*/


#define RM_RANGE_SIGNAL_GLOL1C				0   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GLOL2C				1   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GLOL3I				2   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL1P				4   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL2P				5   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GLOL3Q				6   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GLOL3X				10  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL1OCD            12  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL2CSI            13  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL1OCP            16  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL2OCP            17  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL1OCX            20  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GLOL2OCX            21  /**< range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_GALE1A				0   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE1B				1   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE1C				2   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE1X				3   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE1Z				4   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE6A				5   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE6B				6   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE6C				7   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE6X				8   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE6Z				9   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE5AI				11  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE5AQ				12  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE5AX				13  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE5BI				16  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALE5BQ				17  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALE5BX				18  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALBOCI				19  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_GALBOCQ				20  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_GALBOCX				21  /**< range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_QZSSL1CA			0   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_QZSSL5I				6   /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL5X				10  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL1CD			12  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL2CL			13  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL5Q				14  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_QZSSL1CP			16  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_QZSSL2CM			17  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_QZSSL1CX			20  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL2CX			21  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL6D				26  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL6P				27  /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_QZSSL6X				28  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL6E				29  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_QZSSL6Z				30  /**< range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_SBASL1C				0   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_SBASL5I				6   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_SBASL5Q				10  /**< range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_SBASL5X				14  /**< range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_BDSB1ID1			0   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB2ID1			1   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB3ID1			2   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB1ID2			4   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB2ID2			5   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB3ID2			6   /**< range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB1CP				7   /**< B1c P, range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB2BP				8   /**< B2b P,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB2AP				9   /**< B2a P, range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB1CD				10  /**< B1c D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB2BD				11  /**< B2b D,range signal type definitions*/
#define RM_RANGE_SIGNAL_BDSB2AD				12  /**< B2a D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB1CX				13  /**< B1c X,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB2BX				14  /**< B2b X,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB2AX				15  /**< B2a X,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB8D				16  /**< AceBOC B2 D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB8P				17  /**< AceBOC B2 P,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB8X				18  /**< AceBOC B2 X,range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_BDSB1Q				20  /**< BD2 J B1Q, range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB2Q				21  /**< BD2 J B2Q, range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3Q				22  /**< BD2 J B3Q, range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB1AD				23  /**< BD3 J B1A  D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB1AP				24  /**< BD3 J B1A  P,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB1AX				25  /**< BD3 J B1A  X,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AD				26  /**< BD3 J B3A  D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AP				27  /**< BD3 J B3A  P,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AX				28  /**< BD3 J B3A  X,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AED			29  /**< BD3 J B3AE D,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AEP			30  /**< BD3 J B3AE P,range signal type definitions, extension*/
#define RM_RANGE_SIGNAL_BDSB3AEX			31  /**< BD3 J B3AE X,range signal type definitions, extension*/


#define RM_RANGE_SIGNAL_NICL5A				0  /**< range signal type definitions*/


/** signal type for CHC GNSSOBS, define follow RTCM */
/** GPS Signals */
#define RM_GNSSOBS_SIGNAL_GPSL1CA				2   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL1P				3   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL1W				4   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GPSL2CA				8   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL2P				9   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL2PY				10	/**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GPSL2CM				15  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL2CL				16  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL2CX				17  /**< GNSSOBS signal type definitions,*/

#define RM_GNSSOBS_SIGNAL_GPSL5I				22  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL5Q				23  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL5X				24  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GPSL1CD				30  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL1CP				31  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GPSL1CX				32  /**< GNSSOBS signal type definitions,*/

/** GLO Signals */
#define RM_GNSSOBS_SIGNAL_GLOL1C				2   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL1P				3   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GLOL2C				8   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL2P				9   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GLOL2CSI				15  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL2OCP				16  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL2OCX				17  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GLOL3I				22  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL3Q				23  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL3X				24  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GLOL1OCD				30  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL1OCP				31  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GLOL1OCX				32  /**< GNSSOBS signal type definitions*/

/** GAL Signals */
#define RM_GNSSOBS_SIGNAL_GALE1C				2   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE1A				3   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE1B				4   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE1X				5   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE1Z				6   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GALE6C				8   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE6A				9   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE6B				10  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE6X				11  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE6Z				12   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GALE5BI				14  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE5BQ				15  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE5BX				16  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GALBOCI				18  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALBOCQ				19  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALBOCX				20  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_GALE5AI				22  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE5AQ				23  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_GALE5AX				24  /**< GNSSOBS signal type definitions*/

/** QZS Signals */
#define RM_GNSSOBS_SIGNAL_QZSSL1CA				2   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_QZSSL6D				9   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL6P				10  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL6X				11  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL6E				12  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL6Z				13  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_QZSSL2CM				15  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL2CL				16  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL2CX				17  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_QZSSL5I				22  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL5Q				23  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL5X				24  /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_QZSSL1CD				30  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL1CP				31  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_QZSSL1CX				32  /**< GNSSOBS signal type definitions*/

/** SBA Signals */
#define RM_GNSSOBS_SIGNAL_SBASL1C				2   /**< GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_SBASL5I				22  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_SBASL5X				23  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_SBASL5Q				24  /**< GNSSOBS signal type definitions*/

/** BDS Signals */
#define RM_GNSSOBS_SIGNAL_BDSB1I				2   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB1Q				3   /**< BD2 J B1Q, GNSSOBS signal type definions*/

#define RM_GNSSOBS_SIGNAL_BDSB3I				8   /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB3Q				9   /**< BD2 J B3Q, GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_BDSB2I 				14  /**< GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB2Q				15  /**< BD2 J B2Q, GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_BDSB2AD				22  /**< B2a D, GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB2AP				23  /**< B2a P, GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB2AX				24  /**< B2a X, GNSSOBS signal type definitions*/

#define RM_GNSSOBS_SIGNAL_BDSB2BD				25  /**< B2b D/I, GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB2BP				26  /**< B2b P, GNSSOBS signal type definitions,extension*/
#define RM_GNSSOBS_SIGNAL_BDSB2BX				27  /**< B2b X, GNSSOBS signal type definitions,extension*/

#define RM_GNSSOBS_SIGNAL_BDSB1CD				30  /**< B1c D, GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB1CP				31  /**< B1c P, GNSSOBS signal type definitions*/
#define RM_GNSSOBS_SIGNAL_BDSB1CX				32  /**< B1c X, GNSSOBS signal type definitions*/

/** NIC Signals */
#define RM_GNSSOBS_SIGNAL_NICL5A				22  /**< GNSSOBS signal type definitions*/

typedef enum _GPS_AS_FLAG_T
{
	AS_FALSE = 0,
	AS_TRUE = 1,
	AS_MAX = 0xFFFFFFFF
} GPS_AS_FLAG_T;   /**< Anti - spoofing Flag */



INLINE RM_RANGE_SYSTEM_T  GetRangeSystem(unsigned int RangeStatus)
{
	return (RM_RANGE_SYSTEM_T)((RangeStatus >>16 ) & 0x7);
};

INLINE unsigned int  GetRangeSignalType(unsigned int RangeStatus)
{
	return (unsigned int)((RangeStatus>>21) & 0x1F);
};

#if !defined( __GCC__)
#pragma pack(push,1)
#endif

/** raw message range data struct, previous name: txxxRange_s */
typedef struct _RM_RANGE_DATA_T
{
	unsigned short  PRN;            /**< GPS: 1 to 32, SBAS: 120 to 138 or 183-187   GLONASS: 38 to 61  Galileo: 1 to 36, QZSS: 193-197, BDS: 1-37 */
	unsigned short  FreqNum;        /**< GLONASS Frequency number, 0 ~ 13 */
	double    PSR;                  /**< pseudorange measurement (m) */
	float     PSRStd;               /**< pseudorange measurement standard deviation (m) */
	double    ADR;                  /**< carrier phase, in cycles */
	float     ADRStd;               /**< estimated carrier phase standard deviation (cycles) */
	float     Doppler;              /**< instantaneous carrier Doppler frequency (Hz) */
	float     CN0;                  /**< carrier to noise density ratio (dB-Hz) */
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
}_PACKED_ RM_RANGE_DATA_T;

/** Get specified section of Channel Tracking Status */
#define GET_CH_TRACK_STATUS_CHNUMBER(ChnTrackStatus)      (((ChnTrackStatus) >> 5) & 0x1f)    /**< channel number */
#define GET_CH_TRACK_STATUS_PHASELOCK(ChnTrackStatus)     (((ChnTrackStatus) >> 10) & 0x01)   /**< channel number */
#define GET_CH_TRACK_STATUS_PARITYKNOWN(ChnTrackStatus)   (((ChnTrackStatus) >> 11) & 0x01)   /**< channel number */
#define GET_CH_TRACK_STATUS_CODELOCK(ChnTrackStatus)      (((ChnTrackStatus) >> 12) & 0x01)   /**< channel number */
#define GET_CH_TRACK_STATUS_SATSYSTEM(ChnTrackStatus)     (((ChnTrackStatus) >> 16) & 0x07)   /**< sat system */
#define GET_CH_TRACK_STATUS_SIGNALTYPE(ChnTrackStatus)    (((ChnTrackStatus) >> 21) & 0x1f)   /**< Signal type */
#define GET_CH_TRACK_STATUS_HALFCYCLE(ChnTrackStatus)     (((ChnTrackStatus) >> 28) & 0x01)   /**< phase measurement Half-Cycle Flag */
#define GET_CH_TRACK_STATUS_PRNLOCK(ChnTrackStatus)       (((ChnTrackStatus) >> 30) & 0x01)   /**< PRN lock flag */
#define GET_CH_TRACK_STATUS_CHASSIGN(ChnTrackStatus)      (((ChnTrackStatus) >> 31) & 0x01)   /**< channel assignment */

/** compose System and Signal sections of Channel Tracking Status */
#define COMPOSE_CH_TRACK_STATUS_SYS_SIGNAL(System, SignalType)  ((((System) & 0x07) << 16) | (((SignalType) & 0x1f) << 21))

/** compose phaselock, parityknown, codelock, system, signaltype, halfcycle sections fo channel tracking status */
#define COMPOSE_CH_TRACK_STATUS_ALL(PhaseLock, ParityKnown, CodeLock, System, SignalType, HalfCycle)  \
		((((PhaseLock) & 0x1) << 10) | (((ParityKnown) & 0x1) << 11) | (((CodeLock) & 0x1) << 12) | (((System) & 0x07) << 16) | (((SignalType) & 0x1f) << 21) | (((HalfCycle) & 0x1) << 28))

typedef struct _RM_RANGE_LOG_T
{
	RM_HEADER_T LogHeader;         	/**< LOG header */
	unsigned int ObsNum;			/**< obs number*/
	RM_RANGE_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_RANGE_LOG_T;

/** raw message rangecmp data struct, previous name: txxxRangeCMP_s */
typedef struct _RM_RANGECMP_DATA_T
{
	unsigned int  ChnTrackStatus;  /**< channel tracking status */
	unsigned  doppler : 28;        /**< instantaneous carrier Doppler frequency (Hz) */
	unsigned  psrLow : 4;          /**< pseudorange measurement, low 4-bit */
	unsigned  psrHigh : 32;        /**< pseudorange measurement, high 32-bit */
	unsigned  adr : 32;            /**< carrier phase */
	unsigned  psr_std : 4;         /**< pseudorange measurement standard deviation */
	unsigned  adr_std : 4;         /**< ADR measurement standard deviation */
	unsigned  satID : 8;           /**< GPS: 1 to 32, SBAS: 120 to 138 or 183-187   GLONASS: 38 to 61  Galileo: 1 to 36, QZSS: 193-197, BDS: 1-37 */
	unsigned  locktimeLow : 16;    /**< number of seconds of continuous tracking (no cycle slipping), low 16-bit */
	unsigned  locktimeHigh : 5;    /**< number of seconds of continuous tracking (no cycle slipping), high 5-bit */
	unsigned  cn0 : 5;             /**< carrier to noise density ratio */
	unsigned  freqNum : 6;         /**< GLONASS Frequency number (n+7) */
	unsigned  Reversed : 16;
}_PACKED_ RM_RANGECMP_DATA_T;

typedef struct _RM_RANGECMP_LOG_T
{
	RM_HEADER_T LogHeader;         	/**< LOG header */
	unsigned int ObsNum;			/**< obs number*/
	RM_RANGECMP_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_RANGECMP_LOG_T;

/** Huace raw message GNSSOBS data struct, previous name: txxxGNSSOBS_s */
typedef struct _RM_EX_GNSSOBS_DATA_T
{
	unsigned short	PRN;			/**< GPS: 1 to 32, SBAS: 120 to 138 or 183-187   GLONASS: 38 to 61  Galileo: 1 to 36, QZSS: 193-197, BDS: 1-37 */
	unsigned char	SystemID;		/**< Satellite system */
	unsigned char	SignalType;		/**< Satellite signal type */
	signed char		FreqNum;		/**< GLONASS Frequency number, -7~+6 */
	unsigned char	Reserved;
	unsigned short	CN0;			/**< Carrier to noise density ratio C/No = 10[log10(S/N0)] (dB-Hz), C/No *128 */
	double			PSR;			/**< pseudorange measurement (m) */
	double			ADR;			/**< carrier phase, in cycles */
	float			Doppler;		/**< instantaneous carrier Doppler frequency (Hz) */
	unsigned short	PSRStd;			/**< pseudorange measurement standard deviation (m), psr std * 128*/
	unsigned short	ADRStd;			/**< estimated carrier phase standard deviation (cycles), adt std * 8192 */
	float			LockTime;		/**< number of seconds of continuous tracking (no cycle slipping) */
	unsigned int	ChnTrackStatus;	/**< Channel tracking status
										 bit  0 -  2, 3 bits, Tracking State
										 bit  3,     1 bit, Reserved
										 bit  4 -  10, 7 bits, PLD
										 bit 11,     1 bit, Reserved
										 bit 12,      1 bit,  PSR Smooth Flag, 0 = Not smoothed, 1 = smoothed
										 bit 13,      1 bit,  ADR Valid Flag, 0 = invalid, 1 = valid
										 bit 14,      1 bit,  PhaseShiftVer, 0 = version0, 1 = version1, always set to 1 since 202411
										 bit 15,     1 bit, Reserved
										 bit 16 - 20, 5 bits, SV channel number
										 bit 21 - 31, 11 bits, Reserved*/
}_PACKED_ RM_EX_GNSSOBS_DATA_T;

#define GET_CH_TRACK_STATUS_PSR_SMOOTH_FLAG(ChnTrackStatus)    (((ChnTrackStatus) >> 12) & 0x01)   /**< PSR Smooth Flag */
#define GET_CH_TRACK_STATUS_ADR_VALID_FLAG(ChnTrackStatus)    (((ChnTrackStatus) >> 13) & 0x01)   /**< ADR Valid Flag */
#define GET_CH_TRACK_STATUS_PHASE_SHIFT_VER(ChnTrackStatus)    (((ChnTrackStatus) >> 14) & 0x01)   /**< PSR Smooth Flag */

typedef struct _RM_EX_GNSSOBS_LOG_T
{
	RM_EX_HEADER_T LogHeader;         	/**< LOG header */
	unsigned int ObsNum;			/**< obs number*/
	RM_EX_GNSSOBS_DATA_T		ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_EX_GNSSOBS_LOG_T;

/** raw message gnssobscmp data struct, previous name: txxxGNSSOBSCMP_s */
typedef struct _RM_EX_GNSSOBSCMP_DATA_T
{
	unsigned int  ChnTrackStatus;  /**< channel tracking status */
	unsigned  doppler : 28;        /**< instantaneous carrier Doppler frequency (Hz) */
	unsigned  psrLow : 4;          /**< pseudorange measurement, low 4-bit */
	unsigned  psrHigh : 32;        /**< pseudorange measurement, high 32-bit */
	unsigned  adr : 32;            /**< carrier phase */
	unsigned  psr_std : 4;         /**< pseudorange measurement standard deviation */
	unsigned  adr_std : 4;         /**< ADR measurement standard deviation */
	unsigned  satID : 8;           /**< GPS: 1 to 32, SBAS: 120 to 138 or 183-187   GLONASS: 38 to 61  Galileo: 1 to 36, QZSS: 193-197, BDS: 1-37 */
	unsigned  locktimeLow : 16;    /**< number of seconds of continuous tracking (no cycle slipping), low 16-bit */
	unsigned  locktimeHigh : 5;    /**< number of seconds of continuous tracking (no cycle slipping), high 5-bit */
	unsigned  cn0 : 5;             /**< carrier to noise density ratio */
	unsigned  freqNum : 6;         /**< GLONASS Frequency number (n+7) */
	unsigned  SystemID : 4;        /**< Satellite system id */
	unsigned  SignalType : 6;      /**< Signal type (n-1) */
	unsigned  Reversed : 6;
}_PACKED_ RM_EX_GNSSOBSCMP_DATA_T;

typedef struct _RM_EX_GNSSOBSCMP_LOG_T
{
	RM_EX_HEADER_T LogHeader;		/**< LOG header */
	unsigned int ObsNum;			/**< obs number*/
	RM_EX_GNSSOBSCMP_DATA_T ObsData[RM_RANGE_MAX_OBS_NUM];	/**< obs data*/
}_PACKED_ RM_EX_GNSSOBSCMP_LOG_T;


#if !defined( __GCC__)
#pragma pack(pop)
#endif


/** raw message one channel obs data, previous name: cgcodec_rangeb_sat_cell_s */
typedef struct _RM_RANGE_SAT_CELL_T        /*encode:one channel obs data*/
{
	int antID;                                  /*Antenna ID*/
	unsigned char SNR;                            	/*signal-to-noise ratio*/
	signed short glnfrq;                      		/*glonass frequency*/
	double L, P, D, L_std, P_std, locktime;     /*observation value*/
	unsigned int  ChnTrackStatus;               /**< track status*/
}RM_RANGE_SAT_CELL_T;

/** raw message satellite info struct, previous name: cgcodec_rangeb_sat_s */
typedef struct _RM_RANGE_SAT_DATA_T       	/* encode:observation information in one epoch for one satellite */
{
	unsigned char sat;                            /* satellite ID*/
	unsigned char nfreq;                          /* number of frequency*/
	RM_RANGE_SAT_CELL_T* range_cell;  /* one channel observation cell*/
}RM_RANGE_SAT_DATA_T;
#endif
