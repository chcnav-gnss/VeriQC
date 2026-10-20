#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg decode data structure and function
*-
@file    MsgDecodeDefines.h
@author  CHC
@date    2023/03/13
@brief   msg decode data structure and function

**************************************************************************/
#ifndef _MSG_DECODE_DEFINES_EX_H_
#define _MSG_DECODE_DEFINES_EX_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "MsgPort.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "Common/CommonFuncs.h"

#include "RawMsgAscii/RawMsgAsciiDataTypes.h"
#include "RawMsgBinary/RawMsgBinaryDataTypes.h"
#include "Rinex/RinexDataTypes.h"
#include "HRCX/HRCXDataTypes.h"
#include "HRC/HRCDataTypes.h"
#include "CHCRawMsgBinary/CHCRawMsgBinaryDataTypes.h"
#include "UNRawMsgBinary/UNRawMsgBinaryDataTypes.h"

/** s_Receiver data stream format number defined for each manufacturer */
#define GNSS_STRFMT_RTCM2			0		/**< stream format: RTCM 2 */
#define GNSS_STRFMT_RTCM3			1		/**< stream format: RTCM 3 */

#define GNSS_STRFMT_RINEX			11		/**< stream format: RINEX */

#define GNSS_STRFMT_HRCX			37		/**< stream format: HRCX */
#define GNSS_STRFMT_HRC				38		/**< stream format: HRC */

#define GNSS_STRFMT_RAWMSGA			51		/**< RAWMSGA */
#define GNSS_STRFMT_RAWMSGB			52		/**< RAWMSGB */
#define GNSS_STRFMT_BASEOBSA		53		/**< BASEOBSA */
#define GNSS_STRFMT_BASEOBSB		54		/**< BASEOBSB */

#define GNSS_STRFMT_UNRAWMSGB		55		/**< UNRAWMSGB */
#define GNSS_STRFMT_CHCRAWMSGB		56		/**< CHCRAWMSGB */

#define GNSS_STRFMT_KMD_RAWMSGA			101		/**< KMD_RAWMSGA */
#define GNSS_STRFMT_KMD_RAWMSGB			102		/**< KMD_RAWMSGB */

#define MSG_DECODE_TYPE_RTCM2				0x01
#define MSG_DECODE_TYPE_RTCM3				0x02
#define MSG_DECODE_TYPE_RAWMSGA				0x04
#define MSG_DECODE_TYPE_RAWMSGB				0x08
#define MSG_DECODE_TYPE_RINEX				0x10
#define MSG_DECODE_TYPE_HRCX				0x20
#define MSG_DECODE_TYPE_HRC					0x40
#define MSG_DECODE_TYPE_UNRAWMSGB			0x400
#define MSG_DECODE_TYPE_CHCRAWMSGB			0x800

typedef enum _DECODE_MODE_E
{
	DECODE_MODE_CONVERT_TO_SIGNAL = 0,
	DECODE_MODE_CONVERT_TO_RINEX,
	DECODE_MODE_CONVERT_TO_RAWMSGASCII,
	DECODE_MODE_CONVERT_TO_BASEOBSASCII,
	DECODE_MODE_CONVERT_TO_RAWMSGPROCESS,
	DECODE_MODE_CONVERT_TO_RTCM,
	DECODE_MODE_CONVERT_TO_RTKPLOT,
	DECODE_MODE_CONVERT_TO_ASCII,
	DECODE_MODE_CONVERT_TO_BINARY,
} DECODE_MODE_E;

typedef int (*DECODE_CALLBACK)(int DecodeID, void*pSrcObj, int DataID, void* pData);

typedef struct _DECODE_CALLBACK_INFO_T
{
	int DecodeID;
	void* pSrcObj;
	DECODE_CALLBACK DecodeCallback;
} DECODE_CALLBACK_INFO_T;

typedef struct _PROTOCOL_FILTER_INFO_T
{
	void* pRTCMDecodeManageInfo;
	void* pRTCM3FilterInfo;
	void* pRTCM2FilterInfo;
	void* pRawMsgAsciiFilterInfo;
	void* pRawMsgBinaryFilterInfo;
	void* pRinexFilterInfo;
	void* pHRCXFilterInfo;
	void* pHRCFilterInfo;
	void* pUNRawMsgBinaryFilterInfo;
	void* pCHCRawMsgBinaryFilterInfo;
	RTCM_PARSE_INFO_T* pRTCMParseInfo;
	RAWMSG_ASCII_PARSE_INFO_T* pRawMsgAsciiParseInfo;
	RAWMSG_BINARY_PARSE_INFO_T* pRawMsgBinaryParseInfo;
	RINEX_PARSE_INFO_T* pRinexParseInfo;
	HRCX_PARSE_INFO_T* pHRCXParseInfo;
	HRC_PARSE_INFO_T* pHRCParseInfo;
	UN_RAWMSG_BINARY_PARSE_INFO_T* pUNRawMsgBinaryParseInfo;
	CHC_RAWMSG_BINARY_PARSE_INFO_T* pCHCRawMsgBinaryParseInfo;
	int DecodeMode;
	DECODE_CALLBACK_INFO_T DecodeCallbackInfo;
} PROTOCOL_FILTER_INFO_T;

typedef struct _MSG_DECODE_T
{
	void* pBaseAddr;
	PROTOCOL_FILTER_INFO_T ProtocolFilterInfo;
	MSG_PORT_T* pMsgPort;/**< msg port info*/
	unsigned int FilterOffset;

	float SamplePeriod;
	UTC_TIME_T SetStartTime;
	UTC_TIME_T SetEndTime;
} MSG_DECODE_T;

typedef struct _MSG_DECODE_INIT_PARAM_T
{
	unsigned int CmdMode;     /**< 0:binary mode 1:ascii mode*/
	void* pDecodeCore;
} MSG_DECODE_INIT_PARAM_T;

typedef enum _PROTOCOL_FILTER_FRAM_ERR_E
{
	PROTOCOL_FILTER_ERR = -1,       /**< data filter error */
	PROTOCOL_WAIT_CONFIRM = 0,     /**< data not collected complete, need wait confirm */
	PROTOCOL_FILTER_OK,             /**< data analyze success */
} PROTOCOL_FILTER_FRAM_ERR_E;

//typedef PROTOCOL_FILTER_FRAM_ERR_E(* ProtocolFilterFunc)(MSG_DECODE_T*, unsigned char);
typedef PROTOCOL_FILTER_FRAM_ERR_E(* ProtocolFilterFunc)(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset);

typedef int(* ProtocolFilterClearFunc)(MSG_DECODE_T*);

typedef int(*ProtocolDecodeFunc)(MSG_DECODE_T*, int);

typedef struct _PROTOCOL_DECODE_FUNC_T
{
	unsigned int DecodeType;
	ProtocolFilterFunc ProtocolFilter;
	ProtocolFilterClearFunc ProtocolFilterClear;
	ProtocolDecodeFunc ProtocolDecode;
} PROTOCOL_DECODE_FUNC_T;

INLINE int ObsDataPassesRINEXLimits(MSG_DECODE_T* pDecode, GNSS_OBS_T* pObsData)
{
	if ((pDecode == NULL) || (pObsData == NULL))
	{
		return 0;
	}

	if ((!IsAbsoluteDoubleZero(pDecode->SamplePeriod)) &&
		(0 != (pObsData->RangeData.LogHeader.Milliseconds % ((int)(pDecode->SamplePeriod * 1000 + 0.5)))))
	{
		return 0;
	}

	if (((UTCDiff(&pObsData->ObsTime, &pDecode->SetStartTime) < 0) && (pDecode->SetStartTime.Year != 0)) ||
		((UTCDiff(&pDecode->SetEndTime, &pObsData->ObsTime) < 0) && (pDecode->SetEndTime.Year != 0)))
	{
		return 0;
	}

	return 1;
}

#ifdef __cplusplus
}
#endif

#endif  /**_MSG_DECODE_DEFINES_EX_H_*/

#endif
