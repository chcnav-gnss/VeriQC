/**********************************************************************//**
		VeriQC

		Raw Message Encoder Module
*-
@file   RawMsgCommonDefines.h
@author CHC
@date   2022/12/28
@brief define range message

@note RM stands for Raw Message
**************************************************************************/
#ifndef _RAW_MSG_COMMON_DEFINES_H_
#define _RAW_MSG_COMMON_DEFINES_H_


#include "Common/DataTypes.h"

/** Define for File Process*/
#define MAX_READ_SIZE       (100 * 1024)  /**< 100 K */
#define MAX_BUFFER_SIZE      (2 * MAX_READ_SIZE)  /**< 200K */

/** message type */
#define RAW_MSG_TYPE_BIT_SHORT		     4
#define RAW_MSG_TYPE_BIT_MASK_SHORT		(1 << RAW_MSG_TYPE_BIT_SHORT)

#define RAW_MSG_TYPE_UNKNOWN		 	-1
#define RAW_MSG_TYPE_SOURCE_1   		0x01 /**< from antenna 1*/

#define RAW_MSG_TYPE_BINARY    			0x00
#define RAW_MSG_TYPE_BINARY_SHORT		(RAW_MSG_TYPE_BINARY | RAW_MSG_TYPE_BIT_MASK_SHORT)
#define RAW_MSG_TYPE_ASCII     			0x20
#define RAW_MSG_TYPE_ASCII_SHORT    	(RAW_MSG_TYPE_ASCII | RAW_MSG_TYPE_BIT_MASK_SHORT)		/**< patch here, for standard binary log, need only high 4 bit*/
#define RAW_MSG_TYPE_ABBASCII  			0x40
#define RAW_MSG_TYPE_ABBASCII_SHORT    	(RAW_MSG_TYPE_ABBASCII | RAW_MSG_TYPE_BIT_MASK_SHORT)		/**< patch here, for standard binary log, need only high 4 bit  		0x60*/


#define GET_RAWMSG_FORMAT(_MsgType)           ((_MsgType) & 0x60)
#define GET_RAWMSG_SOURCE(_MsgType)           ((_MsgType) & 0x1F)


/** Raw message time status */
#define RM_TIME_STATUS_UNKNOWN 				20 	/**< Time validity is unknown*/
#define RM_TIME_STATUS_APPROXIMATE 			60 	/**< Time is set approximately*/
#define RM_TIME_STATUS_COARSEADJUSTING 		80	/**< Time is approaching coarse precision*/
#define RM_TIME_STATUS_COARSE 				100	/**< This time is valid to coarse precision*/
#define RM_TIME_STATUS_COARSESTEERING 		120	/**< Time is coarse set and is being steered*/
#define RM_TIME_STATUS_FREEWHEELING			130	/**< Position is lost and the range bias cannot be calculated*/
#define RM_TIME_STATUS_FINEADJUSTING 		140	/**< Time is adjusting to fine precision*/
#define RM_TIME_STATUS_FINE 				160	/**< Time has fine precision*/
#define RM_TIME_STATUS_FINEBACKUPSTEERING	170 /**< Time is fine set and is being steered by the backup system*/
#define RM_TIME_STATUS_FINESTEERING 		180	/**< Time is fine set and is being steered*/
#define RM_TIME_STATUS_EXACT		 		190	/**<*/
#define RM_TIME_STATUS_SATTIME				200	/**< Time from satellite. Only used in logs containing satellite data such as ephemeris and almanac*/


/** Raw message BESTSATS */
#define RM_BESTSATS_NUM_MAX					256	/** BESTSATS satellite number max */
#define RM_BESTDOP_SAT_NUM_MAX				256	/** BESTDOP satellite number max */


/** Raw message PSRDOP */
#define RM_PSRDOP_SAT_NUM_MAX				256	/** PSRDOP satellite number max */

/** Raw message PSRSATS */
#define RM_PSRSATS_NUM_MAX					256	/** PSRSATS satellite number max */


/** Raw message RTKDOP */
#define RM_RTKDOP_SAT_NUM_MAX				256	/** RTKDOP satellite number max */

/** Raw message RTKSATS */
#define RM_RTKSATS_NUM_MAX					256	/** RTKSATS satellite number max */


/** Raw message SATVIS2 */
#define RM_SATVIS2_SAT_NUM_MAX				64	/** SATVIS2 satellite number max */

/** Huace Raw message TIMES */
#define RM_TIMES_SYS_NUM_MAX				4	/** TIMES system number max */


#if !defined( __GCC__)
#pragma pack(push,1)
#endif
/** raw message header, previous name: _RAW_MSG_HEADER_T */
typedef struct _RM_HEADER_T
{
	unsigned char	Sync[3]; 			/**< Head Sync */
	unsigned char   HeadLen;
	unsigned short	MsgID;				/**< 4 */
	unsigned char   MsgType;			/**< 6 */
	unsigned char   PortAddr;			/**< 7 */
	unsigned short  MsgLen;				/**< 8 */
	unsigned short  Sequence;			/**< 10 */
	unsigned char   IdleTime;			/**< 12 */
	unsigned char   TimeStatus;			/**< 13 */
	unsigned short  Week;				/**< 14 */
	unsigned int    Milliseconds;		/**< 16 */
	unsigned int    ReceiverStatus;		/**< 20 */
	unsigned short  Reserved;			/**< 24 */
	unsigned short  ReceiverSWVersion;	/**< 26 */
}_PACKED_ RM_HEADER_T;

/** Huace raw message header, previous name: _RAW_MSG_EX_HEADER_T */
typedef struct _RM_EX_HEADER_T
{
	unsigned char	Sync[3]; 			/**< Head Sync */
	unsigned char	IdleTime;			/**< 4 */
	unsigned short	MsgID;				/**< 5 */
	unsigned char	Sequence;			/**< 6 */
	unsigned char	MsgType;			/**< 7 */
	unsigned short	Reserved;			/**< 8 */
	unsigned short	MsgLen;				/**< 9 */
	unsigned char	PortAddr;			/**< 10 */
	unsigned char	TimeStatus;			/**< 11 */
	unsigned short	Week;				/**< 12 */
	unsigned int	Milliseconds;		/**< 13 */
	unsigned int	ReceiverSWVersion;	/**< 14 */
	unsigned int	ReceiverStatus;		/**< 15 */
}_PACKED_ RM_EX_HEADER_T;

#if !defined( __GCC__)
#pragma pack(pop)
#endif


#endif
