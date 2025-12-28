#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		HRCX Decoder Module
*-
@file   HRCXDataTypes.h
@author CHC
@date   2024/03/04
@brief

**************************************************************************/
#ifndef _HRCX_DATA_TYPES_H_
#define _HRCX_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "Common/GNSSSignal.h"

#include "TimeDefines.h"

#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

#include "BSW/MsgEncoder/HRCX/HRCXDefines.h"

#define HRCX_MSG_PARSED_LEN_MAX								(30 * 1024) /**< HRCX message parsed data struct length max, unit: byte*/
#define HRCX_PARSED_DEBUG_INFO_LEN_MAX						(4 * 1024) /**< HRCX message parsed debug information length max, unit: byte*/
#define HRCX_PARSED_BASEOBS_SAT_COUNT_LEN_MAX				(1 * 1024) /**< HRCX message parsed baseobs sat count length max, unit: byte*/

typedef enum _HRCX_SYSTEM_E
{
	HRCX_SYS_GPS = 0,
	HRCX_SYS_GLO = 1,
	HRCX_SYS_GAL = 2,
	HRCX_SYS_SBA = 3,
	HRCX_SYS_QZS = 4,
	HRCX_SYS_BDS = 5,
	HRCX_SYS_NIC = 6,
	HRCX_SYS_NUM = 7,
	HRCX_SYS_MAX = 0xFFFFFFFF,
} HRCX_SYSTEM_E;

typedef struct _HRCX_OBSDATA_HEADER_DATA_T
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
	unsigned int    SatMask[HRCX_OBSDATA_MAX_SAT_NUM];            /**< GNSS Satellite Mask*/
	unsigned int    SignMask[HRCX_OBSDATA_MAX_SIGN_NUM];           /**< GNSS Signal Mask*/
	unsigned int    CellMask[HRCX_OBSDATA_MAX_CELL_NUM];           /**< GNSS Cell Mask*/
} HRCX_OBSDATA_HEADER_DATA_T;

typedef struct _HRCX_INTERIM_DATA_T
{
	unsigned int StationID; 							/**< Reference station ID */
	unsigned int LastTypeID;
	UTC_TIME_T LastHRCXTime;							/**< Last UTC time */
	UTC_TIME_T HRCXTime;								/**< UTC time */
	int LeapSec;										/**< leap second */
	int ObsCompleteFlag;								/**< obs data complete flag (1:ok,0:not complete) */
	unsigned int MSMDataCompleteTypeIDLast; 			/**< last RTKData complete Type ID */
	unsigned int MSMDataCompleteTypeID; 				/**< RTKData complete Type ID */
	unsigned int MSMDataMask[HRCX_SYS_NUM]; 			/**< HRCX RTKData message receiver flag:bit0-6:MSM1-MSM7 */
	unsigned int MSMDataMaskLast[HRCX_SYS_NUM]; 		/**< last HRCX RTKData message receiver flag:bit0-6:MSM1-MSM7 */
	unsigned int ObsDataOffset[HRCX_SYS_NUM];			/**< offset of obs data from various systems in the structure RM_RANGE_LOG_T */
	unsigned int LockTime[TOTAL_EX_GNSS_SYSTEM_NUM][HRCX_OBSDATA_MAX_SAT_NUM][HRCX_OBSDATA_MAX_SIGN_NUM]; /**< lock time */
	unsigned int LossCount[TOTAL_EX_GNSS_SYSTEM_NUM][HRCX_OBSDATA_MAX_SAT_NUM][MAX_FREQ_INDEX_NUM]; /**< loss of lock count */
	double CarrierPhase[TOTAL_EX_GNSS_SYSTEM_NUM][HRCX_OBSDATA_MAX_SAT_NUM][MAX_FREQ_INDEX_NUM];
	double Position[3];
	OBS_DATA_SAT_T SatObsData; /**< obs data */
#ifdef HRCX_DEBUG_INFO
	unsigned int BaseObsSatNum[HRCX_SYS_NUM];
#endif
} HRCX_INTERIM_DATA_T;

typedef struct _HRCX_PARSE_INFO_T
{
	HRCX_INTERIM_DATA_T HRCXInterimData;	/**< HRCX decode interim data */
	MSG_BUF_T* pDecodeBuf;					/**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;				/**< address stored after data parsed */
#ifdef HRCX_DEBUG_INFO
	MSG_BUF_T ParsedDebugInfoBuf;			/**< parsed debug information buffer */
	MSG_BUF_T ParsedBaseObsSatCountBuf; 	/**< parsed BaseObs satellite count information buffer */
#endif
} HRCX_PARSE_INFO_T;

#ifdef __cplusplus
}
#endif

#endif /**< _HRCX_DATA_TYPES_H_ */

#endif
