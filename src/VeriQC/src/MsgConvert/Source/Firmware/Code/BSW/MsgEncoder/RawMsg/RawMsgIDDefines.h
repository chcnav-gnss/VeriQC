/**********************************************************************//**
			 VeriQC

			 MsgEncoder Module
 *-
 @file RawMsgDefines.h
 @author CHC
 @date 2022/04/05 14:22:31

@brief definitions for raw messages

**************************************************************************/
#ifndef _RAW_MSG_ID_DEFINES_H_
#define _RAW_MSG_ID_DEFINES_H_

#ifdef __cplusplus
extern "C"
{
#endif


/** message ID for raw message */
/** Section 1: Common used  defines */
#define RAW_MSG_ID_RANGE				43
#define RAW_MSG_ID_RANGECMP          	140
#define RAW_MSG_ID_BASEOBS          	23001

#define RAW_MSG_ID_EPH_GPS          	7
#define RAW_MSG_ID_EPH_GLO          	723
#define RAW_MSG_ID_EPH_GAL          	1122
#define RAW_MSG_ID_EPH_BD2          	1696
#define RAW_MSG_ID_EPH_GAL_INAV         1309
#define RAW_MSG_ID_EPH_GAL_FNAV         1310
#define RAW_MSG_ID_EPH_GAL_RCED			23002 /**< GAL Reduced CED */
#define RAW_MSG_ID_EPH_BD3_CNAV1       	2371
#define RAW_MSG_ID_EPH_BD3_CNAV2       	2372
#define RAW_MSG_ID_EPH_BD3_CNAV3       	2412
#define RAW_MSG_ID_EPH_GPS_CNAV       	/**<@todo fill message ID*/
#define RAW_MSG_ID_EPH_QZSS          	1336/**<@todo fill message ID*/
#define RAW_MSG_ID_EPH_NAVIC          	2123/**<@todo fill message ID*/

/** message ID for Huace raw message */
/** Section 1: Common used  defines */
#define RAW_MSG_ID_GNSSOBS				3000
#define RAW_MSG_ID_GNSSOBSCMP			3058

#define RAW_MSG_ID_GPSEPH				3005
#define RAW_MSG_ID_GLOEPH				3006
#define RAW_MSG_ID_GALEPHNAVI			3009
#define RAW_MSG_ID_GALEPHNAVF			3011
#define RAW_MSG_ID_BD2EPH				3001
#define RAW_MSG_ID_BD3EPHCNAV1			3002
#define RAW_MSG_ID_BD3EPHCNAV2			3003
#define RAW_MSG_ID_BD3EPHCNAV3			3004
#define RAW_MSG_ID_QZSEPH				3013
#define RAW_MSG_ID_NICEPH				3014
#define RAW_MSG_ID_GALEPHRCED			3012
#define RAW_MSG_ID_XWEPHLEO				3102

#define RAW_MSG_ID_GPSEPHXW				4000
#define RAW_MSG_ID_GLOEPHXW				4002
#define RAW_MSG_ID_GALEPHXW				4003
#define RAW_MSG_ID_BDSEPHXW				4004

/** internal MsgIndex */


/** do not changed sequence and value of Eph Index */
#define RAW_MSG_INDEX_EPH_GPS				105
#define RAW_MSG_INDEX_EPH_GLO				106
#define RAW_MSG_INDEX_EPH_GALNAVI			107
#define RAW_MSG_INDEX_EPH_GALNAVF			108
#define RAW_MSG_INDEX_EPH_BD2				109
#define RAW_MSG_INDEX_EPH_BD3CNAV1			110
#define RAW_MSG_INDEX_EPH_BD3CNAV2			111
#define RAW_MSG_INDEX_EPH_BD3CNAV3			112
#define RAW_MSG_INDEX_EPH_SBA				113
#define RAW_MSG_INDEX_EPH_QZS				114
#define RAW_MSG_INDEX_EPH_NIC				115
#define RAW_MSG_INDEX_EPH_GALRCED			116
#define RAW_MSG_INDEX_EPH_XWLEO				117
/**< do not changed sequence and value of Eph Index */


#define RAW_MSG_INDEX_GPSEPHXW				175
#define RAW_MSG_INDEX_GLOEPHXW				176
#define RAW_MSG_INDEX_GALEPHXW				177
#define RAW_MSG_INDEX_BDSEPHXW				178

#ifdef __cplusplus
}
#endif
#endif
