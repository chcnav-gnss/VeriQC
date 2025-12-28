/**********************************************************************//**
		 VeriQC

	   Common Module
*-
@file GNSSSignal.h
@author CHC
@date 2022/04/15
@brief define GNSS signal

**************************************************************************/
#ifndef _GNSS_SIGNALS_H_
#define _GNSS_SIGNALS_H_


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
#define GNSS_SIGNAL_DEBUG_EN	1
#include <string.h>
#include <memory.h>
#include <stdio.h>
#endif

#include "DataTypes.h"

#define S32_MHZ(_FreqMHz)        ((signed int)((_FreqMHz) * 1000000))
#define U32_MHZ(_FreqMHz)        ((unsigned int)((_FreqMHz) * 1000000))
#define F64_MHZ(_FreqMHz)        ((double)(_FreqMHz) * 1e6)

#ifndef BIT_MASK
#define BIT_MASK(n) ((1u << (n)) - 1)
#endif

/**
 segment bit length of signal ID
MSB                                                                                                           LSB
|   2Bit   |           3bit          |     1bit     |      3Bit       |  1Bit |         8Bit        |   8Bit   |
|  Y Mode  |   Tracking Combine Mode |  Data/Pilot  |  SignalInOneSys |  I/Q  |  FreqIndexInOneSys  |  System  |
|          |  1bit   |  1bit |  1bit |
|          |  A+B+C  |  D+P  |  I+Q  |
 */
#define SID_BIT_LEN_SYS				8		/**< for GNSS signal*/
#define SID_BIT_LEN_FREQ			8		/**< for frequency index in one specific system*/
#define SID_BIT_LEN_IQ				1		/**< branch I or Q */
#define SID_BIT_LEN_SIGNAL			3		/**< PRN signal*/
#define SID_BIT_LEN_DP				1		/**< Data or Pilot */
#define SID_BIT_LEN_COMB_MODE		3		/**< Combine Tracking Mode*/
#define SID_BIT_LEN_Y_MODE			2		/**< GPS Y code tracking mode*/

#define SID_BIT_LEN_FREQID			(SID_BIT_LEN_SYS + SID_BIT_LEN_FREQ)

#define SID_BIT_POS_SYS				0		/**< for GNSS signal*/
#define SID_BIT_POS_FREQ			(SID_BIT_POS_SYS 	+ SID_BIT_LEN_SYS)				/**< for frequency index in one specific system*/
#define SID_BIT_POS_IQ				(SID_BIT_POS_FREQ 	+ SID_BIT_LEN_FREQ)				/**< branch I or Q */
#define SID_BIT_POS_SIGNAL			(SID_BIT_POS_IQ 	+ SID_BIT_LEN_IQ)				/**< signal in one system */
#define SID_BIT_POS_DP				(SID_BIT_POS_SIGNAL + SID_BIT_LEN_SIGNAL)			/**< Data or Pilot */
#define SID_BIT_POS_COMB_MODE		(SID_BIT_POS_DP		+ SID_BIT_LEN_DP)				/**< Data vs pilot mode*/
#define SID_BIT_POS_Y_MODE			(SID_BIT_POS_COMB_MODE + SID_BIT_LEN_COMB_MODE)		/**< GPS Y code tracking mode*/

#define SID_BIT_MASK(Section)            (BIT_MASK(SID_BIT_LEN_##Section) << SID_BIT_POS_##Section)
#define SID_GET_SECTION(Value, Section)  (((Value)>> SID_BIT_POS_##Section) & BIT_MASK(SID_BIT_LEN_##Section) )

/**	Systems (8bit) */
#define GPS				0
#define GLO				1
#define GAL				2
#define BDS				3
#define LAST_SYS		BDS
#define TOTAL_GNSS_SYSTEM_NUM	(LAST_SYS +1)

/** these two are not technically be considered as BB used GNSS system,*/
/** can be used as sub system for GPS in some case */
/** or considered as Extended system*/
#define SBA				4
#define QZS				5
#define NIC				6	/**< NavIC*/
#define BD3				7

#define LAST_EX_SYS		NIC
#define TOTAL_EX_GNSS_SYSTEM_NUM	(LAST_EX_SYS +1)

#define LBA             16    /**< LBand */


/**	Systems MASK*/
#define SYS_MASK_GPS	(1 << GPS)
#define SYS_MASK_BDS	(1 << BDS)
#define SYS_MASK_GAL	(1 << GAL)
#define SYS_MASK_GLO	(1 << GLO)
#define SYS_MASK_SBAS	(1 << SBA)
#define SYS_MASK_QZSS	(1 << QZS)
#define SYS_MASK_NAVIC	(1 << NIC)
#define SYS_MASK_BD3	(1 << BD3)
#define SYS_MASK_ALL	(SYS_MASK_GPS|SYS_MASK_BDS|SYS_MASK_GAL|SYS_MASK_GLO|SYS_MASK_SBAS|SYS_MASK_QZSS|SYS_MASK_NAVIC|SYS_MASK_BD3)

/** FreqIndex in one system (8bit) */
#define L1				0		/**< 1575.42*/
#define L2				1		/**< 1227.60*/
#define L5				2		/**< 1176.45*/
#define L1C				3		/**< only use for L1C signal index */
#define L2P				4		/**< only use for L2P signal index, same as L6 because QZSS without L2P */
#define L6				4		/**< 1278.75*/
#define L6E				5		/**< only use for L6E signal index */

#define G1				0		/**< 1602*/
#define G2				1		/**< 1246*/
#define G3				2		/**< 1202.025 L3*/
#define G4				3		/**< 1600.995 L1 G1a(Rinex)*/
#define G6				4		/**< 1248.06  L2 G2a(Rinex)*/

#define E1				0		/**< 1575.42*/
#define E5A				1		/**< 1176.45*/
#define E5B				2		/**< 1207.14*/
#define E5				3		/**< 1191.795*/
#define E6				4		/**< 1278.75*/

#define B1				0		/**< 1561.098*/
#define B3				1		/**< 1268.52*/
#define B2				2		/**< 1207.14  B2b(Rinex)*/
#define B4				3		/**< 1575.42 L1*/
#define B5				4		/**< 1176.45 L5*/
#define B8				5		/**< 1191.795   B2(Rinex)*/

#define I5				0		/**< 1176.45, NavIC */
#define S9				1		/**< 2492.028,NavIC */

#define RX				3

#define MAX_FREQ_INDEX_NUM	5	/**< for all */

/** FreqIndex in Rinex */
#define RNX_G1				1		/**< 1602*/
#define RNX_L1				1		/**< 1575.42*/
#define RNX_E1				1		/**< 1575.42*/
#define RNX_B1C				1		/**< 1575.42*/ /**< Notice ! Rinex 3.04 B1 => 2, B1C=> 1 before that B1 => 1*/

#define RNX_E6				6		/**< 1278.75*/
#define RNX_L6				6		/**< 1278.75*/
#define RNX_B3				6		/**< 1268.52*/
#define RNX_G2A				6		/**< 1248.06*/

#define RNX_B1				2		/**< 1561.098*/
#define RNX_G2				2		/**< 1246*/
#define RNX_L2				2		/**< 1227.60*/

#define RNX_G3				3		/**< 1202.025*/

#define RNX_E5B				7		/**< 1207.14*/
#define RNX_B2				7		/**< 1207.14*/
#define RNX_B2B				7		/**< 1207.14  B2b*/

#define RNX_E5				8		/**< 1191.795*/
#define RNX_B2AB			8		/**< 1191.795 B2(B2a+B2b)*/

#define RNX_L5				5		/**< 1176.45*/
#define RNX_E5A				5		/**< 1176.45*/
#define RNX_B2A             5		/**< 1176.45*/
#define RNX_I5				5		/**< L5*/

#define RNX_G1A				4		/**< 1600.995*/

#define RNX_S9				9		/**< 2492.028*/

#define MAX_RINEX_FREQ_ID_NUM	9	/**< for all*/



/** IQ (1bit) */
#define SID_I				(0u << SID_BIT_POS_IQ)
#define SID_Q				(1u << SID_BIT_POS_IQ)

/** Data or Pilot (1bit) */
#define SID_DP_DATA			(0u << SID_BIT_POS_DP)
#define SID_DP_PILOT		(1u << SID_BIT_POS_DP)

/** Tracking combine mode (3bit) */
#define SID_COMB_IQ			(1u << (0u + SID_BIT_POS_COMB_MODE))
#define SID_COMB_DP			(1u << (1u + SID_BIT_POS_COMB_MODE))
#define SID_COMB_ABC		(1u << (2u + SID_BIT_POS_COMB_MODE))

/** Y mode */
#define SID_Y_NULL			(0u)
#define SID_Y_FULL			(1u)		/**< Y code*/
#define SID_Y_Z				(2u)		/**< semi code less*/
#define SID_Y_CODELESS		(3u)

/** segment for Y mode, not segment*/
#define SID_Y_SEG_NULL				(SID_Y_NULL 	<< SID_BIT_POS_Y_MODE)
#define SID_Y_SEG_FULL				(SID_Y_FULL 	<< SID_BIT_POS_Y_MODE)
#define SID_Y_SEG_Z					(SID_Y_Z 		<< SID_BIT_POS_Y_MODE)
#define SID_Y_SEG_CODELESS			(SID_Y_CODELESS << SID_BIT_POS_Y_MODE)

/** Signal in one system */
#define GPS_BPSK		0u		/**< C/A*/
#define GPS_P			1u		/**< P Code*/
#define GPS_Y			2u		/**< Y Code*/
#define GPS_1C			3u		/**< new L1C*/
#define GPS_2C			4u		/**< L2C*/
#define GPS_M			5u		/**< Mcode*/
#define QZS_LEX			6u		/**< QZSS lex code */
#define QZS_BOC			7u		/**< QZSS L1C/B */

#define GLO_FDMA		0u		/**< FDMA C/A*/
#define GLO_P			1u		/**< P Code*/
#define GLO_CDMA		2u		/**< G3 CDMA*/
#define GLO_TDM_CDMA	3u		/**< G1 G2 TDM CDMA*/

#define GAL_A			0u		/**< E1A*/
#define GAL_B			1u		/**< E1B*/
#define GAL_C			2u		/**< E1C*/
#define GAL_QPSK		3u		/**< E5*/
#define GAL_P			4u		/**< E6*/

#define BDS_BPSK		0u		/**< BPSK*/
#define BDS_PRM			1u		/**< generate from PRM*/
#define BDS_BOC			2u		/**< B1C*/
#define BDS_QPSK		3u		/**< New signal*/
#define BDS_PRM3		4u		/**< generate from BDS-3 PRM*/
#define BDS_PRM3E		5u		/**< generate from BDS-3 PRM E*/

#define NIC_A			0u		/**< IRNSS L5 A SPS */
#define NIC_B			1u		/**< IRNSS L5 B RS D */
#define NIC_C			2u		/**< IRNSS L5 C RS P */

/** FreqIDs */
#define GPSL1			(GPS	| (L1 << SID_BIT_POS_FREQ))
#define GPSL2			(GPS	| (L2 << SID_BIT_POS_FREQ))
#define GPSL5			(GPS	| (L5 << SID_BIT_POS_FREQ))
#define GPSL6			(GPS	| (L6 << SID_BIT_POS_FREQ))   /**< only use for QZSS L6 */

#define SBAL1			(SBA	| (L1 << SID_BIT_POS_FREQ))
#define SBAL5			(SBA	| (L5 << SID_BIT_POS_FREQ))

#define QZSL1			(QZS	| (L1 << SID_BIT_POS_FREQ))
#define QZSL2			(QZS	| (L2 << SID_BIT_POS_FREQ))
#define QZSL5			(QZS	| (L5 << SID_BIT_POS_FREQ))
#define QZSL6			(QZS	| (L6 << SID_BIT_POS_FREQ))

#define GLOL1			(GLO	| (G1 << SID_BIT_POS_FREQ))
#define GLOL2			(GLO	| (G2 << SID_BIT_POS_FREQ))
#define GLOL3			(GLO	| (G3 << SID_BIT_POS_FREQ))		/**< CDMA L3 */
#define GLOL4			(GLO	| (G4 << SID_BIT_POS_FREQ))		/**< CDMA L1 (G1a)*/
#define GLOL6			(GLO	| (G6 << SID_BIT_POS_FREQ))		/**< CDMA L2 (G2a)*/

#define GALE1			(GAL	| (E1 << SID_BIT_POS_FREQ))
#define GALE5A			(GAL	| (E5A<< SID_BIT_POS_FREQ))
#define GALE5B			(GAL	| (E5B<< SID_BIT_POS_FREQ))
#define GALE5			(GAL	| (E5 << SID_BIT_POS_FREQ))
#define GALE6			(GAL	| (E6 << SID_BIT_POS_FREQ))

#define BDSB1			(BDS	| (B1 << SID_BIT_POS_FREQ))
#define BDSB2			(BDS	| (B2 << SID_BIT_POS_FREQ))
#define BDSB3			(BDS	| (B3 << SID_BIT_POS_FREQ))
#define BDSL1			(BDS	| (B4 << SID_BIT_POS_FREQ)) /**< B1C */
#define BDSB2A			(BDS	| (B5 << SID_BIT_POS_FREQ)) /**< the same with L5*/
#define BDSB2B			(BDS	| (B2 << SID_BIT_POS_FREQ)) /**< same with B2*/
#define BDSB8			(BDS	| (B8 << SID_BIT_POS_FREQ)) /**< B2a + B2b*/

#define NICL5			(NIC	| (I5 << SID_BIT_POS_FREQ))
#define NICS9			(NIC	| (S9 << SID_BIT_POS_FREQ))

/** Signal ID */
#define GPSL1CA			(GPSL1	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))							/**< actually in branch Q */
#define GPSL1P			(GPSL1	| SID_Q |(GPS_P    << SID_BIT_POS_SIGNAL))							/**< actually in branch I */
#define GPSL1Y			(GPSL1	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_FULL)			/**< actually in branch I */
#define GPSL1N			(GPSL1	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_CODELESS)		/**< actually in branch I */
#define GPSL1W			(GPSL1	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_Z)				/**< actually in branch I */
#define GPSL1CD			(GPSL1	| SID_I |(GPS_1C   << SID_BIT_POS_SIGNAL))
#define GPSL1CP			(GPSL1	| SID_Q |(GPS_1C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GPSL1CX			(GPSL1	| SID_Q |(GPS_1C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define GPSL1M			(GPSL1	| SID_Q |(GPS_M    << SID_BIT_POS_SIGNAL))							/**< actually in branch I */
#define GPSL2CA			(GPSL2	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))							/**< actually in branch Q */
#define GPSL2CM			(GPSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL))							/**< actually in branch Q */
#define GPSL2CL			(GPSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)				/**< actually in branch Q */
#define GPSL2CX			(GPSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP) /**< actually in branch Q */
#define GPSL2P			(GPSL2	| SID_Q |(GPS_P    << SID_BIT_POS_SIGNAL))							/**< actually in branch I */
#define GPSL2Y			(GPSL2	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_FULL)			/**< actually in branch I */
#define GPSL2N			(GPSL2	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_CODELESS)		/**< actually in branch I */
#define GPSL2W			(GPSL2	| SID_Q |(GPS_Y    << SID_BIT_POS_SIGNAL)|SID_Y_SEG_Z)				/**< actually in branch I */
#define GPSL2M			(GPSL2	| SID_Q |(GPS_M    << SID_BIT_POS_SIGNAL))							/**< actually in branch I */
#define GPSL5I			(GPSL5	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))
#define GPSL5Q			(GPSL5	| SID_Q |(GPS_BPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GPSL5X			(GPSL5	| SID_Q |(GPS_BPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)

#define SBAL1CA			(SBAL1	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))
#define SBAL5I			(SBAL5	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))
#define SBAL5Q			(SBAL5	| SID_Q |(GPS_BPSK << SID_BIT_POS_SIGNAL))

#define QZSL1CA			(QZSL1	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))							/**< QZS-01(BlockI,PRN=193): in branch Q, others: in branch I */
#define QZSL1CB			(QZSL1	| SID_I |(QZS_BOC  << SID_BIT_POS_SIGNAL))							/**< QZS-01(BlockI,PRN=193): no L1C/B,    others: in branch I */
#define QZSL1CD			(QZSL1	| SID_I |(GPS_1C   << SID_BIT_POS_SIGNAL))							/**< all: in branch Q */
#define QZSL1CP			(QZSL1	| SID_Q |(GPS_1C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)				/**< QZS-01(BlockI,PRN=193): in branch I, others: in branch Q */
#define QZSL1CX			(QZSL1	| SID_Q |(GPS_1C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define QZSL2CM			(QZSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL))
#define QZSL2CL			(QZSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define QZSL2CX			(QZSL2	| SID_I |(GPS_2C   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)
#define QZSL5I			(QZSL5	| SID_I |(GPS_BPSK << SID_BIT_POS_SIGNAL))
#define QZSL5Q			(QZSL5	| SID_Q |(GPS_BPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define QZSL5X			(QZSL5	| SID_Q |(GPS_BPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define QZSL6D			(QZSL6	| SID_I |(QZS_LEX  << SID_BIT_POS_SIGNAL))
#define QZSL6E			(QZSL6	| SID_I |(QZS_LEX  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define QZSL6X			(QZSL6	| SID_I |(QZS_LEX  << SID_BIT_POS_SIGNAL)|SID_COMB_DP)

#define GLOL1CA			(GLOL1	| SID_I |(GLO_FDMA << SID_BIT_POS_SIGNAL))
#define GLOL1P			(GLOL1	| SID_Q	|(GLO_P    << SID_BIT_POS_SIGNAL))
#define GLOL2CA			(GLOL2	| SID_I |(GLO_FDMA << SID_BIT_POS_SIGNAL))
#define GLOL2P			(GLOL2	| SID_Q	|(GLO_P    << SID_BIT_POS_SIGNAL))
#define GLOL3I			(GLOL3	| SID_I |(GLO_CDMA << SID_BIT_POS_SIGNAL))
#define GLOL3Q			(GLOL3	| SID_Q	|(GLO_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GLOL3X			(GLOL3	| SID_Q	|(GLO_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define GLOL1OCD		(GLOL4	| SID_I |(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL))								/**< actually in branch Q */
#define GLOL1OCP		(GLOL4	| SID_I	|(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)					/**< actually in branch Q */
#define GLOL1OCX		(GLOL4	| SID_I	|(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)		/**< actually in branch Q */
#define GLOL2CSI		(GLOL6	| SID_I |(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL))								/**< actually in branch Q */
#define GLOL2OCP		(GLOL6	| SID_I	|(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)					/**< actually in branch Q */
#define GLOL2OCX		(GLOL6	| SID_I	|(GLO_TDM_CDMA << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)		/**< actually in branch Q */

#define GALE1A			(GALE1	| SID_Q |(GAL_A	   << SID_BIT_POS_SIGNAL))
#define GALE1B			(GALE1	| SID_I |(GAL_B	   << SID_BIT_POS_SIGNAL))
#define GALE1C			(GALE1	| SID_I |(GAL_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GALE1X			(GALE1	| SID_I |(GAL_B	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)
#define GALE1Z			(GALE1	| SID_Q |(GAL_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_ABC)
#define GALE5AI			(GALE5A	| SID_I |(GAL_QPSK << SID_BIT_POS_SIGNAL) )
#define GALE5AQ			(GALE5A	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GALE5AX			(GALE5A	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define GALE5BI			(GALE5B	| SID_I |(GAL_QPSK << SID_BIT_POS_SIGNAL) )
#define GALE5BQ			(GALE5B	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT )
#define GALE5BX			(GALE5B	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define GALE5I			(GALE5	| SID_I |(GAL_QPSK << SID_BIT_POS_SIGNAL) )
#define GALE5Q			(GALE5	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GALE5X			(GALE5	| SID_Q |(GAL_QPSK << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define GALE6A			(GALE6	| SID_Q |(GAL_A	   << SID_BIT_POS_SIGNAL))
#define GALE6B			(GALE6	| SID_I |(GAL_B	   << SID_BIT_POS_SIGNAL))
#define GALE6C			(GALE6	| SID_I |(GAL_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define GALE6X			(GALE6	| SID_I |(GAL_B	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)
#define GALE6Z			(GALE6	| SID_Q |(GAL_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_ABC)

#define BDSB1I			(BDSB1	| SID_I |(BDS_BPSK  << SID_BIT_POS_SIGNAL))
#define BDSB1P			(BDSB1	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL))
#define BDSB1X			(BDSB1	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL)|SID_COMB_IQ)
#define BDSB2I			(BDSB2	| SID_I |(BDS_BPSK  << SID_BIT_POS_SIGNAL))
#define BDSB2P			(BDSB2	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL))
#define BDSB2X			(BDSB2	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL)|SID_COMB_IQ)
#define BDSB3I			(BDSB3	| SID_I |(BDS_BPSK  << SID_BIT_POS_SIGNAL))
#define BDSB3P			(BDSB3	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL))
#define BDSB3X			(BDSB3	| SID_Q	|(BDS_PRM   << SID_BIT_POS_SIGNAL)|SID_COMB_IQ)
#define BDSB1CD			(BDSL1	| SID_I |(BDS_BOC   << SID_BIT_POS_SIGNAL))
#define BDSB1CP			(BDSL1	| SID_Q |(BDS_BOC   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< QMBOC(6,1,4/33), 29/33 in Q, 4/33 in I */
#define BDSB1CX			(BDSL1	| SID_Q |(BDS_BOC   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define BDSB2AD			(BDSB2A	| SID_I |(BDS_QPSK  << SID_BIT_POS_SIGNAL))
#define BDSB2AP			(BDSB2A	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define BDSB2AX			(BDSB2A	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)
#define BDSB2BD			(BDSB2B	| SID_I |(BDS_QPSK  << SID_BIT_POS_SIGNAL))
#define BDSB2BP			(BDSB2B	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< @todo Not open yet */
#define BDSB2BZ			(BDSB2B	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)	/**< @todo Not open yet */
#define BDSB8D			(BDSB8	| SID_I |(BDS_QPSK  << SID_BIT_POS_SIGNAL))											/**< B2A + B2B Data*/
#define BDSB8P			(BDSB8	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< B2A + B2B Pilot*/
#define BDSB8X			(BDSB8	| SID_Q |(BDS_QPSK  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)	/**< B2A + B2B D+P*/

#define BDSB1AD			(BDSL1	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL))											/**< BD3 B1A Data,  PRM */
#define BDSB1AP			(BDSL1	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< BD3 B1A Pilot, PRM */
#define BDSB1AX			(BDSL1	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)	/**< BD3 B1A D+P,   PRM */
#define BDSB3AD			(BDSB3	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL))											/**< BD3 B3A Data,  PRM */
#define BDSB3AP			(BDSB3	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< BD3 B3A Pilot, PRM */
#define BDSB3AX			(BDSB3	| SID_Q |(BDS_PRM3  << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)	/**< BD3 B3A D+P,   PRM */
#define BDSB3AED		(BDSB3	| SID_Q |(BDS_PRM3E << SID_BIT_POS_SIGNAL))											/**< BD3 B3AE Data, PRM */
#define BDSB3AEP		(BDSB3	| SID_Q |(BDS_PRM3E << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)							/**< BD3 B3AE Pilot,PRM */
#define BDSB3AEX		(BDSB3	| SID_Q |(BDS_PRM3E << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_IQ|SID_COMB_DP)	/**< BD3 B3AE D+P,  PRM */

#define NICL5A			(NICL5	| SID_I |(NIC_A	   << SID_BIT_POS_SIGNAL))
#define NICL5B			(NICL5	| SID_I |(NIC_B	   << SID_BIT_POS_SIGNAL))
#define NICL5C			(NICL5	| SID_I |(NIC_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define NICL5X			(NICL5	| SID_I |(NIC_B	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)
#define NICS9A			(NICS9	| SID_I |(NIC_A	   << SID_BIT_POS_SIGNAL))
#define NICS9B			(NICS9	| SID_I |(NIC_B	   << SID_BIT_POS_SIGNAL))
#define NICS9C			(NICS9	| SID_I |(NIC_C	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT)
#define NICS9X			(NICS9	| SID_I |(NIC_B	   << SID_BIT_POS_SIGNAL)|SID_DP_PILOT|SID_COMB_DP)


/** Macros */
#define SYSTEM(Signal)         SID_GET_SECTION(Signal, SYS)
#define FREQ_ONE_SYS(Signal)   SID_GET_SECTION(Signal, FREQ)
#define FREQID(Signal)         ((Signal) & BIT_MASK(SID_BIT_LEN_FREQID))
#define BRANCH(Signal)         (((Signal) >> SID_BIT_POS_IQ) & 0x1)
#define GNSSSIGNAL(Signal)         (((Signal) >> SID_BIT_POS_SIGNAL) & BIT_MASK(SID_BIT_LEN_SIGNAL))
#define TRACK_MODE(Signal)     (((Signal) >> SID_BIT_POS_COMB_MODE) & BIT_MASK(SID_BIT_LEN_COMB_MODE + SID_BIT_LEN_Y_MODE))
#define SIGNAL_ONE_SYS(Signal) (SID_GET_SECTION(Signal, GNSSSIGNAL))

/** SignalID or FreqID switch system to GPS, for QZSS or SBAS */
#define SYS_ID_TO_GPS(Signal)     (((Signal) & (~SID_BIT_MASK(SYS))) | (GPS << SID_BIT_POS_SYS))
#define SYS_ID_TO_QZSS(Signal)     (((Signal) & (~SID_BIT_MASK(SYS))) | (QZS << SID_BIT_POS_SYS))
#define SYS_ID_TO_SBAS(Signal)     (((Signal) & (~SID_BIT_MASK(SYS))) | (SBA << SID_BIT_POS_SYS))


/** check if is L2C compatible for GPSL2CM L2CL L2CX */
#define IS_SIGNAL_GPSL2C(Signal) 	(((Signal) & BIT_MASK(SID_BIT_LEN_SYS + SID_BIT_LEN_FREQ + SID_BIT_LEN_IQ + SID_BIT_LEN_SIGNAL)) == GPSL2CM)

/** judge pilot signal */
#define  IS_PILOT_SIGNAL(Signal)    ((Signal) & SID_DP_PILOT)

/** check if is GLONASS FDMA (G1 or G2) */
#define IS_GLO_FDMA(Signal)         (((Signal) & (SID_BIT_MASK(SIGNAL) | SID_BIT_MASK(SYS))) == (GLO | (GLO_FDMA << SID_BIT_POS_SIGNAL)))

#define IS_GPS_L1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GPSL1)
#define IS_GPS_L2(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GPSL2)
#define IS_GPS_L5(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GPSL5)
#define IS_BDS_B1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB1)
#define IS_BDS_B2(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB2)
#define IS_BDS_B3(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB3)
#define IS_BDS_L1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSL1)
#define IS_BDS_B2A(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB2A)
#define IS_BDS_B2B(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB2B)
#define IS_BDS_B2AB(FreqID)    (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == BDSB8)

#define IS_GAL_E1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GALE1)
#define IS_GAL_E5A(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GALE5A)
#define IS_GAL_E5B(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GALE5B)
#define IS_GAL_E5(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GALE5)
#define IS_GAL_E6(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GALE6)
#define IS_GLO_L1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GLOL1)
#define IS_GLO_L2(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GLOL2)
#define IS_GLO_L3(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GLOL3)
#define IS_GLO_L1A(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GLOL4)
#define IS_GLO_L2A(FreqID)     (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == GLOL6)

#define IS_QZS_L1(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == QZSL1)
#define IS_QZS_L2(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == QZSL2)
#define IS_QZS_L5(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == QZSL5)
#define IS_QZS_L6(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == QZSL6)

#define IS_NIC_I5(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == NICL5)
#define IS_NIC_S9(FreqID)      (((FreqID) & BIT_MASK(SID_BIT_LEN_FREQID)) == NICS9)


#define IS_GPS(System)        ((System) == GPS)
#define IS_GLO(System)        ((System) == GLO)
#define IS_BDS(System)        ((System) == BDS)
#define IS_GAL(System)        ((System) == GAL)
#define IS_QZSS(System)       ((System) == QZS)
#define IS_SBAS(System)       ((System) == SBA)
#define IS_NAVIC(System)      ((System) == NIC)

#define IS_GPS_FREQID(FreqID)     (SYSTEM(FreqID) == GPS)
#define IS_GLO_FREQID(FreqID)     (SYSTEM(FreqID) == GLO)
#define IS_BDS_FREQID(FreqID)     (SYSTEM(FreqID) == BDS)
#define IS_GAL_FREQID(FreqID)     (SYSTEM(FreqID) == GAL)
#define IS_QZSS_FREQID(FreqID)    (SYSTEM(FreqID) == QZS)
#define IS_SBAS_FREQID(FreqID)    (SYSTEM(FreqID) == SBA)
#define IS_NIC_FREQID(FreqID)     (SYSTEM(FreqID) == NIC)

#define IS_BDS_P_SIGNAL(Signal)      ( (BDSB1P==(Signal))||(BDSB2P==(Signal))||(BDSB3P==(Signal)) )
#define IS_BDS2_SIGNAL(Signal)   	 ( (BDS == SYSTEM(Signal)) && (SIGNAL(Signal) < BDS_BOC) )
#define IS_BDS3_SIGNAL(Signal)       ( (BDS == SYSTEM(Signal)) && (SIGNAL(Signal) >= BDS_BOC) )


/** signal branch type */
#define BRANCH_I		0
#define BRANCH_Q		1


/** PRN for each GNSS */
#define MIN_GPS_PRN             1
#define MAX_GPS_PRN             32

#define MIN_SBAS_GEO_PRN        120
#define MAX_SBAS_GEO_PRN        158
#define MIN_QZSBAS_PRN          183
#define MAX_QZSBAS_PRN          191

#define MIN_QZSS_PRN            193
#define MAX_QZSS_PRN            202

#define MIN_GLO_SLOT            1      /**< Slot Number */
#define MAX_GLO_SLOT            32
#define MIN_GLO_RF_CH           (-7)   /**< RF channel */
#define MAX_GLO_RF_CH           (6)
#define GLO_FREQ_CH_OFFSET      8      /**< offset RF_CH to 1 based, for Baseband SatID */

#define MIN_GAL_PRN             1
#define MAX_GAL_PRN             36

#define MIN_BDS_PRN             1
#define MAX_BDS_PRN             63

#define MIN_NIC_PRN             1
#define MAX_NIC_PRN             14


/** judge BDS GEO */
#define IS_BDS_GEO_PRN(PRN)          (((PRN) <= 5) || ((PRN) >= 59))

/** judge BD3 MEO + IGEO */
#define IS_BD3_NON_GEO_PRN(PRN)      (((PRN) >= 19) && ((PRN) <= 58))

/** judge QZSS GEO */
#define IS_QZSBAS_GEO_PRN(PRN)       (((PRN) >= 189) && ((PRN) <= 191))
#define IS_QZSBAS_NON_GEO_PRN(PRN)   (((PRN) >= 183) && ((PRN) <= 187))

#define IS_QZSS_GEO_PRN(PRN)         (((PRN) >= 199) && ((PRN) <= 201))
#define IS_QZSS_NON_GEO_PRN(PRN)     (((PRN) >= 193) && ((PRN) <= 197))

/** number of satellite for each GNSS */
#define MAX_GPS_SAT_NUM         (MAX_GPS_PRN - MIN_GPS_PRN + 1)
#define MAX_GLO_SAT_NUM         (MAX_GLO_SLOT - MIN_GLO_SLOT + 1)
#define MAX_GLO_FREQ_CH_NUM     (MAX_GLO_RF_CH - MIN_GLO_RF_CH + 1)
#define MAX_BDS_SAT_NUM         (MAX_BDS_PRN - MIN_BDS_PRN + 1)
#define MAX_GAL_SAT_NUM         (MAX_GAL_PRN - MIN_GAL_PRN + 1)
#define MAX_NIC_SAT_NUM         (MAX_NIC_PRN - MIN_NIC_PRN + 1)

/** Max number of satellite for SBAS*/
#define MAX_SBAS_GEO_SAT_NUM    (MAX_SBAS_GEO_PRN - MIN_SBAS_GEO_PRN + 1)
#define MAX_QZSBAS_SAT_NUM      (MAX_QZSBAS_PRN - MIN_QZSBAS_PRN + 1)
#define MAX_SBAS_ALL_SAT_NUM    (MAX_SBAS_GEO_SAT_NUM  + MAX_QZSBAS_SAT_NUM)
#define MAX_QZSS_SAT_NUM        (MAX_QZSS_PRN - MIN_QZSS_PRN + 1)

#define MAX_SAT_NUM               (MAX_GPS_SAT_NUM + MAX_GLO_SAT_NUM + MAX_BDS_SAT_NUM + MAX_GAL_SAT_NUM + MAX_NIC_SAT_NUM + MAX_SBAS_ALL_SAT_NUM + MAX_QZSS_SAT_NUM)
#define MAX_SAT_NUM_EACH_SYSTEM   V_MAX(MAX_GPS_SAT_NUM, MAX_GLO_SAT_NUM, MAX_BDS_SAT_NUM, MAX_GAL_SAT_NUM, MAX_NIC_SAT_NUM)

/** Max number of Freq for each GNSS */
#define MAX_GPS_FREQ_NUM         (3)		/**< L1 L2 L5 */
#define MAX_GLO_FREQ_NUM         (3)		/**< G1 G2 G3 (for now) */
#define MAX_BDS_FREQ_NUM         (5)		/**< B1 B2 B3 B1C B2a */
#define MAX_GAL_FREQ_NUM         (5)		/**< E1 E5a E5b E5 E6 */
#define MAX_SBA_FREQ_NUM         (2)		/**< L1 L5 */
#define MAX_QZS_FREQ_NUM         (3)		/**< L1 L2 L5 Lex */
#define MAX_NIC_FREQ_NUM         (2)		/**< L5 S */

#define MAX_FREQ_NUM_ONE_SAT     (5)


/** SvID for PVT */
/** GPS */
#define MIN_GPS_SVID             (1)
#define MAX_GPS_SVID             (MIN_GPS_SVID + MAX_GPS_SAT_NUM - 1)

/** QZSS */
#define MIN_QZSS_SVID            (33)
#if MIN_QZSS_SVID <= MAX_GPS_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_QZSS_SVID            (MIN_QZSS_SVID + MAX_QZSS_SAT_NUM - 1) /**<  42*/

/** GLONASS */
#define MIN_GLO_SVID             (65)
#if MIN_GLO_SVID <= MAX_QZSS_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_GLO_SVID             (MIN_GLO_SVID + MAX_GLO_SAT_NUM - 1) /**<  96*/

/** Galileo */
#define MIN_GALILEO_SVID         (101)
#if MIN_GALILEO_SVID <= MAX_GLO_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_GALILEO_SVID         (MIN_GALILEO_SVID + MAX_GAL_SAT_NUM - 1)  /**<  136*/

/** SBAS GEO */
#define MIN_SBAS_GEO_SVID        (140)
#if MIN_SBAS_GEO_SVID <= MAX_GALILEO_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_SBAS_GEO_SVID        (MIN_SBAS_GEO_SVID + MAX_SBAS_GEO_SAT_NUM - 1)
/** SBAS on the QZSS (SAIF) */
#define MIN_QZSBAS_SVID          (MAX_SBAS_GEO_SVID + 1)
#define MAX_QZSBAS_SVID          (MIN_QZSBAS_SVID + MAX_QZSBAS_SAT_NUM - 1) /**<  187*/

/** BDS */
#define MIN_BDS_SVID             (191)
#if MIN_BDS_SVID <= MAX_QZSBAS_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_BDS_SVID             (MIN_BDS_SVID + MAX_BDS_SAT_NUM - 1) /**<  253*/


/** NavIC */
#define MIN_NIC_SVID             (261)
#if MIN_NIC_SVID <= MAX_BDS_SVID
	#error ERROR: SvID overlap
#endif
#define MAX_NIC_SVID             (MIN_NIC_SVID + MAX_NIC_SAT_NUM - 1)


/** Max SvID of all GNSS */
#define MAX_GNSS_SVID            (MAX_NIC_SVID)
#if MAX_GNSS_SVID > 65535
	#error ERROR: SvID overflow (some SvID var type are U16)
#endif



/** SvID for RANGE message */
/** GPS, PRN start from 1*/
#define MIN_GPS_RANGE_PRN        (MIN_GPS_PRN)
#define MAX_GPS_RANGE_PRN        (MAX_GPS_PRN)

/** BDS, PRN start from 1*/
#define MIN_BDS_RANGE_PRN        (MIN_BDS_PRN)
#define MAX_BDS_RANGE_PRN        (MAX_BDS_PRN)
#define MIN_BDS_RANGE_PRN_UC     (161)
#define MAX_BDS_RANGE_PRN_UC     (MIN_BDS_RANGE_PRN_UC + MAX_BDS_SAT_NUM -1)

/** GLONASS, PRN/Slot start from 38*/
#define MIN_GLONASS_RANGE_PRN    (38)
#define MAX_GLONASS_RANGE_PRN    (MIN_GLONASS_RANGE_PRN + MAX_GLO_SAT_NUM - 1) /**<  69*/

/** SBAS PRN start from 120 in Range*/
#define MIN_SBAS_GEO_RANGE_PRN   (MIN_SBAS_GEO_PRN)
#define MAX_SBAS_GEO_RANGE_PRN   (MAX_SBAS_GEO_PRN)

/** QZSS-SBAS(SAIF) PRN start from 183 in Range*/
#define MIN_QZSBAS_RANGE_PRN     (MIN_QZSBAS_PRN)
#define MAX_QZSBAS_RANGE_PRN     (MAX_QZSBAS_PRN)

/** QZSS PRN start from 193 in Range*/
#define MIN_QZSS_RANGE_PRN       (MIN_QZSS_PRN)
#define MAX_QZSS_RANGE_PRN       (MAX_QZSS_PRN)

/** GALILEO, SVID start from 1 - 36 in Range*/
#define MIN_GALILEO_RANGE_PRN    (MIN_GAL_PRN)
#define MAX_GALILEO_RANGE_PRN    (MAX_GAL_PRN)

/** NavIC(IRNSS), SVID start from 1 - 14 in Range*/
#define MIN_NAVIC_RANGE_PRN    	(MIN_NIC_PRN)
#define MAX_NAVIC_RANGE_PRN    	(MAX_NIC_PRN)

/** SvID for NMEA (V3.X) message */
/** GPS*/
#define MIN_GPS_NMEA_SVID        (1)
#define MAX_GPS_NMEA_SVID        (32)

/** SBAS*/
#define MIN_SBAS_GEO_NMEA_SVID   (33)
#define MAX_SBAS_GEO_NMEA_SVID   (64)   /**  without PRN 152-158*/

/** GLONASS*/
#define MIN_GLO_NMEA_SVID        (65)
#define MAX_GLO_NMEA_SVID        (96)

/** QZSS*/
#define MIN_QZSS_NMEA_SVID       (1)
#define MAX_QZSS_NMEA_SVID       (MAX_QZSS_SAT_NUM) /**< 10 */

/** QZSS-SBAS(SAIF)*/
#define MIN_QZSBAS_NMEA_SVID     (MIN_QZSBAS_PRN) /**  not support yet*/
#define MAX_QZSBAS_NMEA_SVID     (MAX_QZSBAS_PRN) /**  not support yet*/

/** GALILEO*/
#define MIN_GAL_NMEA_SVID        (1)
#define MAX_GAL_NMEA_SVID        (MAX_GAL_SAT_NUM) /**< 36 */

/** BDS*/
#define MIN_BDS_NMEA_SVID        (1)
#define MAX_BDS_NMEA_SVID        (MAX_BDS_SAT_NUM) /**< 63 */

/** NavIC*/
#define MIN_NIC_NMEA_SVID        (1)
#define MAX_NIC_NMEA_SVID        (MAX_NIC_SAT_NUM)  /**< 14 */


/** mask of QZSS satellites as PRN, only for constant */
#define QZSS_PRN_TO_INDEX(PRN)                 ((PRN)-MIN_QZSS_PRN)
#define QZSS_SV_MASK(...)                      V_BIT_MASK(V_EACH(QZSS_PRN_TO_INDEX, __VA_ARGS__))
#define QZSS_SV_MASK_RANGE(StartPRN, EndPRN)   SV_MASK_RANGE((StartPRN)-MIN_QZSS_PRN+1, (EndPRN)-MIN_QZSS_PRN+1)


/** physical constant */
#define LIGHT_SPEED         2.99792458e8    /**< m/sec */
#define LIGHT_SPEED_INV  (1 / LIGHT_SPEED)  /**< sec/m */

/** carrier Frequency*/
#define GPS_FREQ_L1       1575420000.0  /**< in unit of Hz*/
#define GPS_FREQ_L2       1227600000.0  /**< in unit of Hz*/
#define GPS_FREQ_L5       1176450000.0  /**< in unit of Hz*/
#define GPS_FREQ_L6       1278750000.0  /**< in unit of Hz*/

#define GLO_FREQ_G1_BASE  1602000000.0  /**< in unit of Hz*/
#define GLO_FREQ_G2_BASE  1246000000.0  /**< in unit of Hz*/

#define GLO_FREQ_G3  	  1202025000.0  /**< in unit of Hz*/
#define GLO_FREQ_G1A  	  1600995000.0  /**< in unit of Hz*/
#define GLO_FREQ_G2A  	  1248060000.0  /**< in unit of Hz*/


#define GAL_FREQ_E1       1575420000.0  /**< in unit of Hz*/
#define GAL_FREQ_E5A      1176450000.0  /**< in unit of Hz*/
#define GAL_FREQ_E5       1191795000.0  /**< in unit of Hz*/
#define GAL_FREQ_E5B      1207140000.0  /**< in unit of Hz*/
#define GAL_FREQ_E6       1278750000.0  /**< in unit of Hz*/

#define QZS_FREQ_L1       1575420000.0  /**< in unit of Hz*/
#define QZS_FREQ_L2       1227600000.0  /**< in unit of Hz*/
#define QZS_FREQ_L5       1176450000.0  /**< in unit of Hz*/
#define QZS_FREQ_LEX      1278750000.0 /**< in unit of Hz*/

#define BDS_FREQ_B1       1561098000.0  /**< in unit of Hz*/
#define BDS_FREQ_B2       1207140000.0  /**< in unit of Hz*/
#define BDS_FREQ_B3       1268520000.0  /**< in unit of Hz*/
#define BDS_FREQ_L1       GPS_FREQ_L1   /**< in unit of Hz*/
#define BDS_FREQ_B2A      GPS_FREQ_L5   /**< in unit of Hz*/
#define BDS_FREQ_B8       1191795000.0  /**< in unit of Hz*/

#define NIC_FREQ_I5       GPS_FREQ_L5  /**< in unit of Hz*/
#define NIC_FREQ_S9       2492028000.0 /**< in unit of Hz*/

#define GPS_WAVELENGTH_L1 (LIGHT_SPEED / GPS_FREQ_L1)       /**< 0.19029367279836488*/
#define GPS_WAVELENGTH_L2 (LIGHT_SPEED / GPS_FREQ_L2)       /**< 0.24421021342456826*/
#define GPS_WAVELENGTH_L5 (LIGHT_SPEED / GPS_FREQ_L5)       /**< 0.25482804879085386*/

#define BDS_WAVELENGTH_B1 (LIGHT_SPEED / BDS_FREQ_B1)       /**< 0.19203948631027648*/
#define BDS_WAVELENGTH_B2 (LIGHT_SPEED / BDS_FREQ_B2)       /**< 0.24834936958430671*/
#define BDS_WAVELENGTH_B3 (LIGHT_SPEED / BDS_FREQ_B3)       /**< 0.23633246460442090 */
#define BDS_WAVELENGTH_B4 (GPS_WAVELENGTH_L1)               /**< 0.19029367279836488 */
#define BDS_WAVELENGTH_B5 (GPS_WAVELENGTH_L5)               /**< 0.25482804879085386*/
#define BDS_WAVELENGTH_B8 (LIGHT_SPEED / BDS_FREQ_B8)       /**< */

#define GAL_WAVELENGTH_E1  (LIGHT_SPEED / GAL_FREQ_E1 )     /**< 0.19029367279836488*/
#define GAL_WAVELENGTH_E5A (LIGHT_SPEED / GAL_FREQ_E5A)     /**< 0.25482804879085386*/
#define GAL_WAVELENGTH_E5  (LIGHT_SPEED / GAL_FREQ_E5 )
#define GAL_WAVELENGTH_E5B (LIGHT_SPEED / GAL_FREQ_E5B)     /**< 0.24834936958430671*/
#define GAL_WAVELENGTH_E6  (LIGHT_SPEED / GAL_FREQ_E6 )

#define NIC_WAVELENGTH_I5   (LIGHT_SPEED / NIC_FREQ_I5)     /**< 0.25482804879085386*/
#define NIC_WAVELENGTH_S9   (LIGHT_SPEED / NIC_FREQ_S9)

#define QZS_WAVELENGTH_LEX   (LIGHT_SPEED / QZS_FREQ_LEX )

#define GLO_G1_FREQ_STEP        562500.0
#define GLO_G2_FREQ_STEP        437500.0

/** FreqCh is unsigned and 0 based*/
#define GLO_FREQ_G1(FreqCh)      (GLO_FREQ_G1_BASE+((FreqCh) - 7)*GLO_G1_FREQ_STEP)
#define GLO_FREQ_G2(FreqCh)      (GLO_FREQ_G2_BASE+((FreqCh) - 7)*GLO_G2_FREQ_STEP)

/** G1 wave length, unit m
	0.187597455043216,0.187531446086481,0.187465483565873,0.187399567432411,0.187333697637180,0.187267874131334,0.187202096866097,
	0.187136365792759,0.187070680862681,0.187005042027290,0.186939449238084,0.186873902446626,0.186808401604549,0.186742946663552,
*/
#define GLO_WAVELENGTH_G1(FreqCh) (LIGHT_SPEED / GLO_FREQ_G1(FreqCh))

/** G2 wave length, unit m
	0.241196727912707,0.241111859254046,0.241027050298980,0.240942300984529,0.240857611247803,0.240772981026001,0.240688410256410,
	0.240603898876405,0.240519446823447,0.240435054035088,0.240350720448965,0.240266446002805,0.240182230634420,0.240098074281710,
*/
#define GLO_WAVELENGTH_G2(FreqCh) (LIGHT_SPEED / GLO_FREQ_G2(FreqCh))

#define GLO_WAVELENGTH_G1_BASE   GLO_WAVELENGTH_G1(7)   /**< 0.18713636579275905*/
#define GLO_WAVELENGTH_G2_BASE   GLO_WAVELENGTH_G2(7)   /**< 0.24060389887640449*/

#define GLO_WAVELENGTH_G1A (LIGHT_SPEED / GLO_FREQ_G1A)         /**< 0.19029367279836488*/
#define GLO_WAVELENGTH_G2A (LIGHT_SPEED / GLO_FREQ_G2A)         /**< 0.24421021342456826*/
#define GLO_WAVELENGTH_G3  (LIGHT_SPEED / GLO_FREQ_G3)          /**< 0.25482804879085386*/

#define GLO_LAMBDA_G1(FreqCh) (1 / GLO_WAVELENGTH_G1(FreqCh))
#define GLO_LAMBDA_G2(FreqCh) (1 / GLO_WAVELENGTH_G2(FreqCh))


#define GPS_L1L2_RATE           0.779220779220779       /**< GPS_FREQ_L2/GPS_FREQ_L1, L2/L1 = 120/154 */
#define GPS_L1L5_RATE           0.746753246753247       /**< GPS_FREQ_L5/GPS_FREQ_L1, L5/L1 = 115/15 4*/
#define BDS_B1B2_RATE           0.773263433813893       /**< BDS_FREQ_B2/BDS_FREQ_B1, B2/B1 = 118/152.6 */
#define BDS_B1B3_RATE           0.812581913499345       /**< BDS_FREQ_B3/BDS_FREQ_B1, B3/B1 = 124/152.6 */
#define BDS_B1B2A_RATE          0.753604193971167       /**< BDS_FREQ_B2A/BDS_FREQ_B1, B2A/B1 = 115/152.6 */
#define BDS_B1L1_RATE           1.00917431192661        /**< BDS_FREQ_L1/BDS_FREQ_B1, L1(B1C)/B1 = 154/152.6 */
#define GLO_L1L2_RATE           0.777777777777778       /**< GLO_FREQ_G2_BASE/GLO_FREQ_G1_BASE, G2/G1 = 7/9 */
#define GAL_E1E5A_RATE          0.746753246753247       /**< GAL_FREQ_E5A/GAL_FREQ_E1, E5a/E1 = 115/154 */
#define GAL_E1E5B_RATE          0.766233766233766       /**< GAL_FREQ_E5B/GAL_FREQ_E1, E5b/E1 = 118/154 */
#define GAL_E1E6_RATE           0.811688311688312       /**< GAL_FREQ_E6/GAL_FREQ_E1, E6/E1 = 125/154 */
#define NIC_I5S9_RATE           2.118260869565217       /**< S/I5 */


/** SecCodeType */
#define SEC_CODE_EXTERN_TABLE     (1 << 0)
#define SEC_CODE_UNIQUE_EACH_SV   (1 << 1)

#define SEC_CODE_NORMAL         (0)
#define SEC_CODE_EXTERN         (SEC_CODE_EXTERN_TABLE)
#define SEC_CODE_UNIQUE         (SEC_CODE_EXTERN_TABLE | SEC_CODE_UNIQUE_EACH_SV)



/** SignalType, 32-bit */
#define ST_BIT_MASK_P               (1 << 0)  /**< P Code, such as L2P */
#define ST_BIT_MASK_BOC             (1 << 1)  /**< BOC(1,1), such as E1B */
#define ST_BIT_MASK_TD_1            (1 << 2)  /**< the 1st in time division code, such as L2CM */
#define ST_BIT_MASK_TD_2            (1 << 3)  /**< the 2nd in time division code, such as L2CL */
#define ST_BIT_MASK_MC              (1 << 4)  /**< memory code, such as E1B */
#define ST_BIT_MASK_1CG             (1 << 5)  /**< one 28bit shift CodeGen, such as L2C */
#define ST_BIT_MASK_WEIL            (1 << 6)  /**< Weil code, such as B1C */
#define ST_BIT_MASK_MBOC_P          (1 << 7)  /**< MBOC(6,1) +, such as E1B */
#define ST_BIT_MASK_MBOC_N          (1 << 8)  /**< MBOC(6,1) -, such as E1C */
#define ST_BIT_MASK_TMBOC_1         (1 << 9)  /**< TMBOC(6,1) type 1, L1Cd */
#define ST_BIT_MASK_TMBOC_2         (1 << 10) /**< TMBOC(6,1) type 2, L1Cp */
#define ST_BIT_MASK_QMBOC           (1 << 11) /**< QMBOC(6,1), such as B1Cp */
#define ST_BIT_MASK_FDMA            (1 << 12) /**< FDMA, such as G1 */
#define ST_BIT_MASK_CSK             (1 << 13) /**< CSK, such as L6D,L6E */
#define ST_BIT_MASK_LFSR32          (1 << 14) /**< need to use 32bit PRNConfig, such as L6D,L6E */


#define ST_BIT_MASK_VIT_CONTINUOUS  (1 << 16) /**< continuous Convolution Code, need continuous mode Viterbi decode, such as SBAS L1 */

#define ST_NORM                    (0)
#define ST_GPS_P                   (ST_BIT_MASK_P)
#define ST_L1CD                    (ST_BIT_MASK_BOC | ST_BIT_MASK_WEIL | ST_BIT_MASK_TMBOC_1)
#define ST_L1CP                    (ST_BIT_MASK_BOC | ST_BIT_MASK_WEIL | ST_BIT_MASK_TMBOC_2)
#define ST_L2CM                    (ST_BIT_MASK_TD_1 | ST_BIT_MASK_1CG)
#define ST_L2CL                    (ST_BIT_MASK_TD_2 | ST_BIT_MASK_1CG)
#define ST_E1B                     (ST_BIT_MASK_BOC | ST_BIT_MASK_MC | ST_BIT_MASK_MBOC_P)
#define ST_E1C                     (ST_BIT_MASK_BOC | ST_BIT_MASK_MC | ST_BIT_MASK_MBOC_N)
#define ST_E6B                     (ST_BIT_MASK_MC)
#define ST_E6C                     (ST_BIT_MASK_MC)
#define ST_B1CD                    (ST_BIT_MASK_BOC | ST_BIT_MASK_WEIL)
#define ST_B1CP                    (ST_BIT_MASK_BOC | ST_BIT_MASK_WEIL | ST_BIT_MASK_QMBOC)
#define ST_G1G2                    (ST_BIT_MASK_FDMA)
#define ST_G1OCD                   (ST_BIT_MASK_TD_1)
#define ST_G1OCP                   (ST_BIT_MASK_TD_2 | ST_BIT_MASK_BOC)
#define ST_G2OCP                   (ST_BIT_MASK_TD_2 | ST_BIT_MASK_BOC)
#define ST_SBAS                    (ST_BIT_MASK_VIT_CONTINUOUS)
#define ST_L5                      (ST_BIT_MASK_VIT_CONTINUOUS)
#define ST_L6D                     (ST_BIT_MASK_LFSR32 | ST_BIT_MASK_CSK | ST_BIT_MASK_TD_1)
#define ST_L6E                     (ST_BIT_MASK_LFSR32 | ST_BIT_MASK_CSK | ST_BIT_MASK_TD_2)


/** signal parameter for each GNSS signal*/
typedef struct _SIGNAL_PARAMETER_T
{
	unsigned char   MsDataNumber;	 /**< number of MsData for BitSync, it is generally number of ms in one navigation bit, max 20 */
	unsigned char   DecodeMsNumber;  /**< Dedicated for decode, formal AccMsNumber, it must be number of ms in one navigation bit if it has */

	unsigned char   OneSecCodeMsLen; /**< ms length of each chip of Secondary Code */
	unsigned char   SecCodeType;     /**< data type of 'SecCodeConfig' */

	/** "SecCodeConfig" explain:
	  if (SecCodeType == SEC_CODE_NORMAL), 'SecCodeConfig' is the coding of Secondary Code (Overlay Code, NH Code), SecCodeConfig's LSB is the first bit of secondary code, filled up zeros in MSB
	  if (SecCodeType & SEC_CODE_EXTERN_TABLE), such as Galileo E5aQ & E5bQ, 'SecCodeConfig' will be a address and point to GNSS_SEC_CODE_PARAM_T structure
	*/
	PTR             SecCodeConfig;
	unsigned short 	SecCodeLength;   /**< bit length of Secondary Code (Overlay Code, NH Code), if no Secondary Code, equal to DecodeMsDataNUmber formal BitLength */

	unsigned short  VitPoly;         /**< Viterbi decode polynomial */

	unsigned int    SignalType;      /**< Signal Type flags */
	unsigned int    CodeLength;
	unsigned int    CodeFreq;
	unsigned short  NavDataRate;     /**< NavData rate, unit symbol/s */
	unsigned short  FreqRange;
	const void *    pPRNConfig;

} SIGNAL_PARAMETER_T;


/** flags for get Signal Name */
typedef unsigned int SIGNAL_NAME_TYPE_T;

/** flags for SIGNAL_NAME_TYPE_T  */
#define SIGNAL_NAME_TYPE_NO_SYS      (1 << 0)  /**< without System Name */
#define SIGNAL_NAME_TYPE_FULL_SYS    (1 << 1)  /**< use entire system name, such as NavIC, Galileo, etc. */
#define SIGNAL_NAME_TYPE_RINEX       (1 << 2)  /**< use Rinex style, such as J1C, R1C, etc. */
#define SIGNAL_NAME_TYPE_SIMPLE      (1 << 3)  /**< use Simple style, such as G1, I5, etc. */
#define SIGNAL_NAME_TYPE_NO_BRANCH   (1 << 4)  /**< ignore Signal branch, such as L5, B1C, etc. */

double GetCarrierWaveLength(unsigned int FreqID, unsigned int GloFreqNum);

const char * GetSystemNameString(unsigned int ExSystemID);

unsigned char* GetGloFreqChMap(void);
unsigned char GetGloFreqCh(unsigned int SatID);

#ifdef __cplusplus
}
#endif

#endif
