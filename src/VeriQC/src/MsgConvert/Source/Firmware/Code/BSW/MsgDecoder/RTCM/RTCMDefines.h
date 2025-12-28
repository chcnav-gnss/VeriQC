/**********************************************************************//**
		VeriQC

		RTCM Encoder Module
*-
@file   RTCMDefine.h
@author CHC
@date   2023/03/02
@brief

**************************************************************************/
#ifndef _RTCM_DEFINES_H_
#define _RTCM_DEFINES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define RTCM2_SEPRATE_TIME_ENABLE		(1) /**< separate time system */

#define RTCM2_PREAMBLE       (0x66)        /**< rtcm ver.2 frame preamble */
#define RTCM3_PREAMBLE       (0xD3)        /**< rtcm ver.3 frame preamble */

#define RTCM2_MSG_LEN_MAX           (128) /**< RTCM2 message length max */

#define RTCM2_MSG_LEN_MIN           (6) /**< RTCM2 message length min */

#define RTCM2_MSG_HEADER_BITS_SIZE           (48)   /**< preamble type station z-count SEQ'NCE DATA WDS health, unit: bit*/

#define RTCM3_MSG_HEADER_BITS_SIZE           (24)   /**< preamble reserved msglen, unit: bit*/
#define RTCM3_MSG_NUMBER_BITS_SIZE           (12)   /**< preamble reserved msglen, unit: bit*/

#define RTCM3_MSG_HEADER_LEN            (3)   /**< preamble reserved msglen, unit: byte*/
#define RTCM3_MSG_CRC24_LEN             (3)   /**< CRC24, unit: byte*/

#define RTCM3_MSG_DATA_LEN_MAX			(1024U) /**< RTCM2 message data length max */
#define RTCM3_MSG_LEN_MAX				(RTCM3_MSG_HEADER_LEN + RTCM3_MSG_CRC24_LEN + RTCM3_MSG_DATA_LEN_MAX)/**< RTCM3 message length max, unit: byte*/

#define RTCM_MSG_PARSED_LEN_MAX             (25 * 1024) /**< RTCM message parsed data struct length max, unit: byte*/

#ifdef RTCM_DEBUG_INFO
#define RTCM_MSG_PARSED_DEBUG_INFO_LEN_MAX             (4 * 1024) /**< RTCM message parsed debug information length max, unit: byte*/
#define RTCM_MSG_PARSED_BASEOBS_SAT_COUNT_LEN_MAX             (1 * 1024) /**< RTCM message parsed baseobs sat count length max, unit: byte*/
#endif

#define RTCM_DECODE_CHANNEL_MAX_NUM         (3) /**< RTCM decode channel max number */

#define RTCM_MAX_SAT_NUM            (64) /**< RTCM satellite number max */
#define RTCM_MAX_SIGN_NUM           (32) /**< RTCM signal number max */
#define RTCM_MAX_CELL_NUM           (64) /**< RTCM cell number max */

#define RTCM_ANTENNA_DESCRIPTOR_LEN_MAX             (32) /**< RTCM antenna descriptor length max */
#define RTCM_ANTENNA_SN_LEN_MAX                     (32) /**< RTCM antenna SN length max */

#define RTCM_RECEIVER_DESCRIPTOR_LEN_MAX            (32) /**< RTCM receiver descriptor length max */
#define RTCM_RECEIVER_FIRMWARE_VER_LEN_MAX          (32) /**< RTCM receiver firmware length max */
#define RTCM_RECEIVER_SN_LEN_MAX                    (32) /**< RTCM receiver SN length max */

#define RTCM_SPECIAL_MSG_LEN_MAX                    (90) /**< RTCM special message length max */

#define RTCM_IN_STATUS_NUM_MAX                      (48) /**< RTCMINSTATUS sub message number max */

#define RTCM_INPUT_PORT_NUM_MAX                     (3) /**< RTCM input port number max */

#define RTCM_CRS_NAME_LEN_MAX                       (32) /**< RTCM1300 Coordinate Reference Systems name length max */

#define RTCM_HELMERT_SOURCE_NAME_LEN_MAX            (32) /**< RTCM1301 Helmert source name length max */
#define RTCM_HELMERT_TARGET_NAME_LEN_MAX            (32) /**< RTCM1301 Helmert target name length max */

#define RTCM_CRS_LINKS_I_NUM_MAX					(8) /**< RTCM1302 Coordinate Reference Systems links number max */
#define RTCM_CRS_DATABASE_LINK_LEN_MAX				(32) /**< RTCM1302 Coordinate Reference Systems database link length max */

#define RTCM_NETWORK_RTK_RESIDUAL_SAT_NUM			(32) /**< RTCM1303 Network RTK Residual satellite number */

#define RTCM_SYSTEM_PARAM_MSG_NUM_MAX				(32) /**< RTCM1304 system param message number max */
#define RTCM_UNICODE_TEXT_STRING_LEN_MAX			(126) /**< RTCM1304 unicode text string length max */

#ifndef PI
#define PI				3.1415926535897932
#endif

#define RANGE_MS    (LIGHT_SPEED * 0.001)      /**< range in 1 ms */

#define RTCM3_MSM_MULTI_BIT_INDEX           (78) /**< RTCM3 MSM multi bit index */
#define RTCM3_GPS_RTK_DATA_SYNC_BIT_INDEX       (78) /**< RTCM3 GPS RTK DATA sync bit index */
#define RTCM3_GLO_RTK_DATA_SYNC_BIT_INDEX       (75) /**< RTCM3 GLO RTK DATA sync bit index */

typedef enum _RTCM_DATA_TYPE_E
{
	RTCM_DATA_TYPE_RTCM3 = 0,
	RTCM_DATA_TYPE_RTCM2 = 1,
} RTCM_DATA_TYPE_E;

typedef enum _REFSTATION_VENDOR_E
{
	REFSTATION_VENDOR_UNKNOWN = 0,		/**< unknown type */
	REFSTATION_VENDOR_CHC_VRS = 1,		/**< Huace SWAS */
	REFSTATION_VENDOR_SWAS_VRS,			/**< Huace SWAS yijianguding */
	REFSTATION_VENDOR_CHC_SAX,			/**< Huace M720 */
	REFSTATION_VENDOR_CHC,				/**< Huace */
	REFSTATION_VENDOR_NOVATEL,			/**< Novatel */
	REFSTATION_VENDOR_UNICORE,			/**< Unicore */
	REFSTATION_VENDOR_TRIMBLE,			/**< Trimble */
	REFSTATION_VENDOR_MAGELLAN,			/**< Magellan */
	REFSTATION_VENDOR_QXWZ_VRS,			/**< QianXunWeiZhi VRS*/
	REFSTATION_VENDOR_SIN,				/**< SINAN SINOGNSS */
	REFSTATION_VENDOR_HEMI,				/**< HEMISPHERE */
	REFSTATION_VENDOR_LEICA,			/**< LEICA */
	REFSTATION_VENDOR_SWIFT,			/**< swift */
	REFSTATION_VENDOR_SEPT,				/**< Septentrio */

	REFSTATION_VENDOR_NUM_MAX,			/**< 14 */
} REFSTATION_VENDOR_E;

#ifdef __cplusplus
}
#endif

#endif

