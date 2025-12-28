#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg codec
*-
@file    MsgCodec.h
@author  CHC
@date    2023/06/26
@brief

**************************************************************************/
#ifndef _MSG_CODEC_H_
#define _MSG_CODEC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "Encoder/RINEX/RINEXDefines.h"
#include "TimeDefines.h"
#include "Common/GNSSSignal.h"
#include "Decoder/MsgDecodeDefinesEx.h"

int MsgCodecDeinit(MSG_DECODE_T* pMsgDecode);
MSG_DECODE_T* MsgCodecInit(int DecodeID, void* pSrcObj, int DecodeMode, UTC_TIME_T * pApproximateTime);
int MsgCodecSetDecodeCallbackInfo(MSG_DECODE_T* pMsgDecode, int DecodeID, void* pSrcObj, DECODE_CALLBACK DecodeCallback);
int MsgCodecSetDecodeMode(MSG_DECODE_T* pMsgDecode, int DecodeMode);

#ifdef __cplusplus
}
#endif

#endif  /* _MSG_CODEC_H_ */

#endif
