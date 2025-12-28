/**********************************************************************//**
		VeriQC

		HRCX Encoder Module
*-
@file   HRCXDefine.h
@author CHC
@date   2024/02/27
@brief

**************************************************************************/
#ifndef _HRCX_DEFINES_H_
#define _HRCX_DEFINES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define HRCX_PREAMBLE0					0x55	/**< HRCX frame preamble0 */
#define HRCX_PREAMBLE1					0x2A	/**< HRCX frame preamble1 */
#define HRCX_PREAMBLE2					0xD3	/**< HRCX frame preamble2 */

#define HRCX_MSG_LEN_SIZE				(2)		/**< HRCX message length size */

#define HRCX_MSG_HEADER_LEN				(6)		/**< preamble(3 byte) + length(2 byte) + board type(1 byte) */
#define HRCX_MSG_CRC24_LEN				(3)		/**< CRC24, unit: byte*/

#define HRCX_MSG_LEN_MAX				(HRCX_MSG_HEADER_LEN + HRCX_MSG_CRC24_LEN + 2048) /**< HRCX message length max, unit: byte*/

#define HRCX_MSG_HEADER_BITS_SIZE			(48)	/**< preamble reserved msglen, unit: bit*/
#define HRCX_MSG_NUMBER_BITS_SIZE			(16)	/**< preamble reserved msglen, unit: bit*/

#define HRCX_OBSDATA_MAX_SAT_NUM			(64U)
#define HRCX_OBSDATA_MAX_SIGN_NUM			(32U)
#define HRCX_OBSDATA_MAX_CELL_NUM			(64U)

#define HRCX_OBSDATA_SYNC_FLAG_INDEX		(110U)

#ifdef __cplusplus
}
#endif

#endif

