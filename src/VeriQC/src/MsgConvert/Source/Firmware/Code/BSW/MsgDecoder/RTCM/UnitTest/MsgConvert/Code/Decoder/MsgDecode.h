#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg decode data structure and function
*-
@file    MsgCodec.h
@author  CHC
@date    2023/03/13
@brief   msg decode data structure and function

**************************************************************************/
#ifndef _MSG_DECODE_H_
#define _MSG_DECODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "HAL/DriverCycleDataBuffer.h"
#include "MsgPort.h"
#include "MsgDecodeDefinesEx.h"

extern int MsgDecode(MSG_DECODE_T* pDecode, char* pBuf, unsigned int BufSize, int DecodeType, int SourceType);
extern int MsgDecodeDeinit(void* pDecode);
extern MSG_DECODE_T* MsgDecodeInit(PROTOCOL_FILTER_INFO_T* pProtocolFilterInfo,
											MSG_PORT_T* pPort);
extern int MsgDecodeReset(MSG_DECODE_T* pDecode);

int GetMsgFilterOffset(MSG_DECODE_T* pDecode, unsigned int* pOffset);
int MsgFilterClearAll(MSG_DECODE_T* pDecode);
int SetMsgFilterOffset(MSG_DECODE_T* pDecode, unsigned int Offset);

extern PROTOCOL_DECODE_FUNC_T s_ProtocolDecodeFuncTabl[];

#ifdef __cplusplus
}
#endif

#endif	/**<_MSG_DECODER_H_*/

#endif
