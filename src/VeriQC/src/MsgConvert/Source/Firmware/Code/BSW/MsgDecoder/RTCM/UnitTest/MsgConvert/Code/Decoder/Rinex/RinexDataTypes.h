#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		Rinex Decoder Module
*-
@file   RinexDataTypes.h
@author CHC
@date   2023/12/19
@brief

**************************************************************************/
#ifndef _RINEX_DATA_TYPES_H_
#define _RINEX_DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "Common/GNSSNavDataType.h"
#include "TimeDefines.h"

#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

#include "../../Encoder/RINEX/RINEXDefines.h"

#include "HAL/DriverCycleDataBuffer.h"


#define RINEX_GROUP_NUM_MAX			(10u) /**<Supports merging up to ten rinex files */

typedef enum _RINEX_SYS_TYPE_E
{
	RINEX_SYS_NONE = 0x00,		/**< navigation SystemIDtem: none */
	RINEX_SYS_GPS = 0x01,		/**< navigation SystemIDtem: GPS */
	RINEX_SYS_GLO = 0x02,		/**< navigation SystemIDtem: GLONASS */
	RINEX_SYS_GAL = 0x04,		/**< navigation SystemIDtem: Galileo */
	RINEX_SYS_BDS = 0x08,		/**< navigation SystemIDtem: BeiDou */
	RINEX_SYS_SBA = 0x10,		/**< navigation SystemIDtem: SBAS */
	RINEX_SYS_QZS = 0x20,		/**< navigation SystemIDtem: QZSS */
	RINEX_SYS_NIC = 0x40,		/**< navigation SystemIDtem: NAVIC */
	RINEX_SYS_LEO = 0x80,		/**< navigation SystemIDtem: LEO */
	RINEX_SYS_ALL = 0xFF,		/**< navigation SystemIDtem: all */
} RINEX_TIME_TYPE_E;

typedef enum _RINEX_FILE_INDEX_E
{
	RINEX_FILE_INDEX_OBS = 0,
	RINEX_FILE_INDEX_MUTIEPH,	/**< MUTI eph data file */
	RINEX_FILE_INDEX_GPSEPH,	/**< GPS eph data file */
	RINEX_FILE_INDEX_GLOEPH,	/**< GLO eph data file */
	RINEX_FILE_INDEX_GALEPH,	/**< GAL eph data file */
	RINEX_FILE_INDEX_BDSEPH,	/**< BDS eph data file */
	RINEX_FILE_INDEX_SBAEPH,	/**< SBAS eph data file */
	RINEX_FILE_INDEX_QZSEPH,	/**< QZSS eph data file */
	RINEX_FILE_INDEX_NICEPH,	/**< NAVIC eph data file */

	RINEX_FILE_INDEX_NUM,

	RINEX_FILE_INDEX_MAX = 0xFFFFFFFF,
} RINEX_FILE_INDEX_E;

typedef struct _RINEX_OBS_HEADER_INFO_T		  /**< decode:rinex obs header*/
{
	RINEX_OBS_HEADER_T ObsHeader;
	char SysTimeType;				/**< time type of system*/
	char FileCreateDate[9];			/**< date of file creation*/
	char FileCreateTime[9];			/**< time of file creation*/
	char* FileName;
	unsigned int GLOFreqNum;
	char GLOFreqChnn[37];			/**< Glonass frequence channel number, number: 0-14*/
	int Block;
	int SatNum;							/**< satellite numbers which are stored*/
	int WaveLenFact[MAX_FREQ_INDEX_NUM];		/**< widelane fact data of every sat*/
	int NarWideLaneSatNum; 						/**< sat ID which uses narrow-wide-lane*/
	int WideLaneSat[RINEX_SAT_NUM_MAX][MAX_FREQ_INDEX_NUM + 1]; /**< widelane sat and frequence*/
	double GLOPhaseBias[4];						/**< glonass phase bias correction used to align code and phase observations*/
	double WideLaneBias[RINEX_SAT_NUM_MAX]; /**< widelane bias of every sat*/
} RINEX_OBS_HEADER_INFO_T;

#define RINEX_DATA_BUF_SIZE				(128 * 1024ul)
#define RINEX_DATA_CYCLE_BUF_SIZE		(RINEX_DATA_BUF_SIZE + (8 * 1024ul))

typedef struct _RINEX_INFO_T/**< RINEX control struct type */
{
	UTC_TIME_T UTCTime;		/**< message time */
	int Ver; 				/**< RINEX version */
	char   FileType[RINEX_FILE_INDEX_NUM];		/**< RINEX file type ('O','N',...) */
	int    SysTimeType;		/**< time system */
	char   ObsType[TOTAL_EX_GNSS_SYSTEM_NUM][MAX_NUM_OBS_TYPE][4]; /**< rinex obs types */
	int    EphSatNum;		/**< input ephemeris satellite number */
	int    EphSet;			/**< input ephemeris set (0-1) */
	char   Opt[256];		/**< rinex dependent options */

	int Leapsecs;							/**< leap sec */

	int FileHeaderType[RINEX_FILE_INDEX_NUM];		/**< RINEX file type ('O','N',...) */

	RINEX_EPH_HEADER_T EphHeader[RINEX_FILE_INDEX_NUM];			/**< rinex eph header*/
	RINEX_GEPH_HEADER_T GLOEphHeader;		/**< rinex GLO eph header*/
	RINEX_OBS_HEADER_INFO_T ObsHeaderInfo;			/**< rinex obs header*/

	GNSS_OBS_T ObsData;
	OBS_DATA_SAT_T SatObsData;
	GLO_EPHEMERIS_T GLOEphData;
	UNI_EPHEMERIS_T UniEphData[RINEX_FILE_INDEX_NUM];
	RM_HEADER_T GLOEphRMHeader;
	RM_HEADER_T UniEphRMHeader[RINEX_FILE_INDEX_NUM];

	unsigned char RinexDataBuf[RINEX_FILE_INDEX_NUM][RINEX_DATA_CYCLE_BUF_SIZE];
	CYCLE_BUFF_CTRL_T CycleBufCtrl[RINEX_FILE_INDEX_NUM];
} RINEX_INFO_T;

#define RINEX_PARSED_LEN_MAX             (16 * 1024) /** rinex parsed data struct length max, unit: byte*/

typedef struct _RINEX_PARSE_INFO_T
{
	MSG_BUF_T* pDecodeBuf;					/**< address stored after frame match */
	MSG_BUF_T ParsedDataBuf;				/**< address stored after data parsed */
} RINEX_PARSE_INFO_T;

#define RINEX_ID_OBS_HEADER             152     /**< obs data file header while endode */
#define RINEX_ID_NICEPH_HEADER          160     /**< NAVIC eph data file header */

typedef enum _RINEX_SYS_INDEX_E
{
	RINEX_SYS_INDEX_GPS = 0,
	RINEX_SYS_INDEX_GLO,
	RINEX_SYS_INDEX_GAL,
	RINEX_SYS_INDEX_BDS,
	RINEX_SYS_INDEX_SBA,
	RINEX_SYS_INDEX_QZS,
	RINEX_SYS_INDEX_NIC,
} RINEX_SYS_INDEX_E;

#ifdef __cplusplus
}
#endif

#endif /**< _RINEX_DATA_TYPES_H_ */

#endif
