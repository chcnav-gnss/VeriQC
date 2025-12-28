#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg Port Module
*-
@file    MsgPort.h
@author  CHC
@date    2023/03/13
@brief   Common method of msg port

**************************************************************************/
#ifndef _MSG_PORT_H_
#define _MSG_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "HAL/DriverCycleDataBuffer.h"
#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"

typedef int (*MSG_RESPONSE_PORT_WRITE_FUNC)(unsigned int PortID, void* pBuff, unsigned int Len);

typedef struct _MSG_PORT_T
{
	unsigned int MsgPortID;             /**< msg port ID, example COM1/USB1/CAN1*/
	unsigned int OutputPortID;          /**< port ID of user cmd, example "com1" of ">outmsg,com1,gga,1*ff"*/
	MSG_RESPONSE_PORT_WRITE_FUNC pPortWriteFunc; /**< Write message response to physical port */
	MSG_BUF_T DecodeBuf;            /**< Temporarily store the command after single byte parsed */
	CYCLE_BUFF_CTRL_T CycleBuffCtrl;    /**< Physical port buffer, such as com port */
} MSG_PORT_T;

typedef struct _MSG_PORT_INIT_PARAM_T
{
	unsigned int MsgPortID;         /**< msg port ID, example COM1/USB1/CAN1*/
	unsigned int CycleBufLen;       /**< cycle buffer length */
	MSG_RESPONSE_PORT_WRITE_FUNC pPortWriteFunc; /**< Write command response to physical port */
} MSG_PORT_INIT_PARAM_T;

extern int MsgPortDeinit(void* pPort);
extern MSG_PORT_T* MsgPortInit(const MSG_PORT_INIT_PARAM_T* pInitInfo);
extern int MsgPortWrite(MSG_PORT_T* pMsgPort, void* pBuff, unsigned int Len);

#ifdef __cplusplus
}
#endif

#endif

#endif
