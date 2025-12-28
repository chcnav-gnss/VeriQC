#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

	   msg Port Module
*-
@file	MsgPort.c
@author  CHC
@date	2023/1/05
@brief   Common method of msg port
@brief   recode comport table
**************************************************************************/

#include "OSDependent.h"
#include "MsgPort.h"
#include "Common/CommonFuncs.h"

static int MsgRecv(unsigned int PortAddr, void *pOutputData, void *pSourceData, unsigned int DataSize)
{
	MEMCPY(pOutputData,pSourceData,DataSize);
	return 0;
}

/**********************************************************************//**
@brief Write response message to physical port

@param pMsgPort [In] Msg port handle
@param pBuff	[In] Buffer pointer
@param Len	  [In] Buffer length

@retval 0:success, <0, fail

@author CHC
@date 2023/03/29
**************************************************************************/
int MsgPortWrite(MSG_PORT_T* pMsgPort, void* pBuff, unsigned int Len)
{
	if ((pMsgPort == NULL) || (pBuff == NULL))
		return -1;

	pMsgPort->pPortWriteFunc(pMsgPort->MsgPortID, pBuff, Len);

	return 0;
}

/**********************************************************************//**
@brief	free msg port

@author CHC
@date 2023/03/13
@note
**************************************************************************/
int MsgPortDeinit(void* pPort)
{
	FREE(pPort);
	return 0;
}

/**********************************************************************//**
@brief init cmd port

@param pInitInfo [In] Port init param

@retval pPort : pointer to msg port

@author CHC
@date 2023/03/13
**************************************************************************/
MSG_PORT_T* MsgPortInit(const MSG_PORT_INIT_PARAM_T* pInitInfo)
{
	MSG_PORT_T* pPort = NULL;
	char CycleBufNameBuf[16] = {0};

	int BaseSize		 = MEM_CEILBY8(sizeof(MSG_PORT_T));
	int DecodeBufLen;
	int CycleBufSize;
	int TotalSize;

	if (!pInitInfo)
	{
		return NULL;
	}
	DecodeBufLen	 = MEM_CEILBY8(pInitInfo->CycleBufLen);
	CycleBufSize	 = MEM_CEILBY8(pInitInfo->CycleBufLen);
	TotalSize		= BaseSize + DecodeBufLen + CycleBufSize;

	pPort = MALLOC(TotalSize);
	if (!pPort)
	{
		return NULL;
	}
	MEMSET(pPort, 0, TotalSize);

	pPort->MsgPortID = pInitInfo->MsgPortID;
	pPort->pPortWriteFunc = pInitInfo->pPortWriteFunc;
	pPort->DecodeBuf.Len = 0;
	pPort->DecodeBuf.pBaseAddr = ((unsigned char*)(pPort)) + BaseSize;

	//GetPhysicalPortNameByID(pInitInfo->MsgPortID, CycleBufNameBuf);
	CycleBufInit(&pPort->CycleBuffCtrl, CycleBufNameBuf, CycleBufSize, (char *)pPort + BaseSize + DecodeBufLen, 0, MsgRecv);

	return pPort;
}

#endif
