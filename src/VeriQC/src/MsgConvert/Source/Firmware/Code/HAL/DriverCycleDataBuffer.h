/**********************************************************************//**
			VeriQC

	   		HAL Module
*-
@file    DriverCycleDataBuffer.h
@author  CHC
@date    2022/10/11
@brief   Cycle Data Buffer for input data

**************************************************************************/
#ifndef _DRIVER_CYCLE_DATA_BUFFER_H_
#define _DRIVER_CYCLE_DATA_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "Common/DataTypes.h"


#define CYCLE_BUF_DEBUG_EN			0

#define CB_OVERFLOW_BIT			0
#define CB_UNDERFLOW_BIT		8


#define CB_OVERFLOW_MASK		(1 << CB_OVERFLOW_BIT)
#define CB_UNDERFLOW_MASK		(1 << CB_UNDERFLOW_BIT)

/** call back function type */
typedef int (* DRIVER_OUT_CALLBACK_FUNC_T)(unsigned int PortAddress, void *pOutputData, void *pSourceData, unsigned int DataSize);

/**  cycle buffer ctrl*/
typedef struct _CYCLE_BUFF_CTRL_T
{
	unsigned int	BufferSize;		/**< buffer size*/
	unsigned int	WriteAddr;
	unsigned int	ReadAddr;
	unsigned short	OverflowFlag;
	unsigned short	OverflowCount;	/**< overflow count		*/
			 char *	pcBuffName;		/**< the name of the buffer, for the debug purpose*/
	unsigned char *	pBuff;			/**< Data Buff*/
	unsigned int    FIFOReadyThres; /**< the data size threshold size for FIFO ready, approximately*/

			 int 	FIFOReadyFlag;	/**< if the FIFO is ready for output*/


	unsigned int    PortAddress;	/**< additional port address*/
	DRIVER_OUT_CALLBACK_FUNC_T pOutFunc;	/**< output call back function for driver*/


}CYCLE_BUFF_CTRL_T;


int CycleBufInit(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pcBuffName,unsigned int BufferSize, char * pDataBuff,
	unsigned int    PortAddress,
	DRIVER_OUT_CALLBACK_FUNC_T pOutFunc);
void CycleBufReset(CYCLE_BUFF_CTRL_T * pCycleBufCtrl);

BOOL CycleBufIsEmpty(CYCLE_BUFF_CTRL_T * pCycleBufCtrl);

int CycleBufPutData(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pInputData, unsigned int DataSize);
int CycleBufGetData(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pOutputData, unsigned int ExpectDataSize);
void CycleBufUpdateReadAddr(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, unsigned int AcutalReadSize);
unsigned int CycleBufGetDataSize(CYCLE_BUFF_CTRL_T * pCycleBufCtrl);

#ifdef __cplusplus
}
#endif

#endif
