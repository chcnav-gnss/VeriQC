/**********************************************************************//**
			VeriQC

			HAL Module
*-
@file    DriverCycleDataBuffer.c
@author  CHC
@date    2022/10/11
@brief   Cycle Data Buffer for input data

**************************************************************************/
#include "DriverCycleDataBuffer.h"
#include <stdio.h>
#include <string.h>

#include "../OSDependent.h"


#if CYCLE_BUF_DEBUG_EN
#if DEBUG_LOG_2_FILE
#define CB_TRACE(Format,...) DebugFileLog(&s_CycleBufferLogCtrl, 3, Format, __VA_ARGS__)
#else
#define CB_TRACE printf
#endif
#else
#define CB_TRACE(Format,...) (void)0
#endif


#ifdef _RELEASE
#define CB_ERR_MSG(Format,...) (void)0
#else
#define CB_ERR_MSG LowLevelPrint
#endif


/**********************************************************************//**
@brief cycle buffer initialization

@param pCycleBufCtrl	[In] cycle buffer control
@param pcBuffName 		[In] buffer name for debug
@param BufferSize		[In] maximum buffer size
@param pDataBuff 		[In] internal data buffer point
@param PortAddress		[In] additional port address
@param  pOutFunc;		[In] output call back function for driver

@author CHC
@date 2022/10/10 14:22:50
@note
History:
- 2022.10.10 Startup
**************************************************************************/
int CycleBufInit(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pcBuffName,unsigned int BufferSize, char * pDataBuff,
	unsigned int    PortAddress,
	DRIVER_OUT_CALLBACK_FUNC_T pOutFunc)
{
	pCycleBufCtrl->BufferSize = BufferSize;
	pCycleBufCtrl->ReadAddr = 0;
	pCycleBufCtrl->WriteAddr = 0;
	pCycleBufCtrl->OverflowFlag = 0;
	pCycleBufCtrl->OverflowCount = 0;
	pCycleBufCtrl->pcBuffName = pcBuffName;
	pCycleBufCtrl->pBuff = (unsigned char *)pDataBuff;
	pCycleBufCtrl->FIFOReadyThres = 500;
	pCycleBufCtrl->FIFOReadyFlag = 0;

	pCycleBufCtrl->PortAddress = PortAddress;
	pCycleBufCtrl->pOutFunc = pOutFunc;

#if 0
	PortSpeedReset(&pCycleBufCtrl->SpeedData);
	pCycleBufCtrl->SpeedData.bStatisticsEn = FALSE;
#endif
	return 0;
}
/**********************************************************************//**
@brief cycle buffer reset

@param pCycleBufCtrl	[In] cycle buffer control

@author CHC
@date 2022/10/11 14:22:50
@note
History:
- 2022.10.11 Startup
**************************************************************************/
void CycleBufReset(CYCLE_BUFF_CTRL_T * pCycleBufCtrl)
{
	pCycleBufCtrl->ReadAddr = 0;
	pCycleBufCtrl->WriteAddr = 0;
	pCycleBufCtrl->OverflowFlag = 0;
	pCycleBufCtrl->OverflowCount = 0;
	pCycleBufCtrl->FIFOReadyFlag = 0;

}
/**********************************************************************//**
@brief judge if there is valid data in the buffer

@param pCycleBufCtrl	[In] cycle buffer control

@return if the buffer is empty

@author CHC
@date 2022/10/10 14:22:50
@note
History:
- 2022.10.10 Startup
**************************************************************************/
BOOL CycleBufIsEmpty(CYCLE_BUFF_CTRL_T * pCycleBufCtrl)
{
	if(pCycleBufCtrl->ReadAddr == pCycleBufCtrl->WriteAddr)
		return TRUE;
	else
		return FALSE;
}

/**********************************************************************//**
@brief put data into cycle buffer

@param pCycleBufCtrl	[In] cycle buffer control
@param pInputData	[In] input data pointer
@param DataSize	 	[In] input data size

in case the internal buffer is not big enough
1. in case in a task, that is assuming not time critical, do try to delay to avoid lose data
2. in case in int, can not wait, clear the buffer

@return overflow flag & Error flags
@retval -1 absolute overflow
@retval -2  overflow

@author CHC
@date 2022/10/10 14:22:50
@note
History:

- 2022.10.10 Startup
**************************************************************************/
int CycleBufPutData(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pInputData, unsigned int DataSize)
{
	unsigned int TotalFIFOSize;
	unsigned int ReadAddrLatch;
	unsigned int OverflowFlag = 0;
	unsigned int BufSize = pCycleBufCtrl->BufferSize;
	unsigned int DataInBufSize;
	BOOL bInTask = TRUE;
	BOOL bOverflow;

/* Section 1: Absolute protection */
	if(0 == DataSize)
	{
		return 0;
	}
	if(DataSize > BufSize - 1)
	{
		CB_TRACE("#E CBuff Abs OVF %u %u" EOL,DataSize,BufSize);
		return CB_OVERFLOW_MASK;
	}

/* Section 2 check the buffer availability & wait if needed */
	do
	{
		TotalFIFOSize = DataSize + pCycleBufCtrl->WriteAddr;
		bOverflow = FALSE;

		ReadAddrLatch = pCycleBufCtrl->ReadAddr;

		if(ReadAddrLatch > pCycleBufCtrl->WriteAddr)
		{
			DataInBufSize = BufSize - ReadAddrLatch + pCycleBufCtrl->WriteAddr;
		}
		else
		{
			DataInBufSize = (pCycleBufCtrl->WriteAddr - ReadAddrLatch);
		}

		if(TotalFIFOSize >= BufSize)
		{
			unsigned int LeftoverSize = TotalFIFOSize - BufSize;
			int TailSize = BufSize - pCycleBufCtrl->WriteAddr;

			if(ReadAddrLatch > pCycleBufCtrl->WriteAddr)
			{
				CB_TRACE("#E CB %s OVF 1 %u %u DataSize %u" EOL,
					pCycleBufCtrl->pcBuffName,ReadAddrLatch, pCycleBufCtrl->WriteAddr, DataSize);
				bOverflow = TRUE;
			}
			else if(LeftoverSize >= ReadAddrLatch)
			{
				CB_TRACE("#E CB %s OVF 2 %u Leftover %u DataSize %u" EOL,
					pCycleBufCtrl->pcBuffName,ReadAddrLatch, pCycleBufCtrl->WriteAddr, DataSize);
				bOverflow = TRUE;
			}

			if(bOverflow)
			{
				OverflowFlag = CB_OVERFLOW_MASK;

				/* if in Task, clear buffer */
				if(bInTask)
				{
					/**@note do not delay here as it's in critical section now, otherwise it might cause unstable situation */
					pCycleBufCtrl->WriteAddr = pCycleBufCtrl->ReadAddr;
					CB_ERR_MSG(EOL "#E %s BufOvf dump data" EOL, pCycleBufCtrl->pcBuffName);
					bOverflow = FALSE;
					pCycleBufCtrl->OverflowFlag = 0;
					pCycleBufCtrl->OverflowCount = 0;
					pCycleBufCtrl->FIFOReadyFlag = 0;
					DataInBufSize = 0;
					break;

				}
				else /* if in int, clear buffer */
				{
					pCycleBufCtrl->WriteAddr = pCycleBufCtrl->ReadAddr;
					CB_ERR_MSG(EOL "#E %s BufOvf" EOL, pCycleBufCtrl->pcBuffName);
					bOverflow = FALSE;
					pCycleBufCtrl->OverflowFlag |= CB_OVERFLOW_MASK;
					break;
				}
			}

			if(!bOverflow)
			{
				MEMCPY(pCycleBufCtrl->pBuff + pCycleBufCtrl->WriteAddr, pInputData, TailSize);
				MEMCPY(pCycleBufCtrl->pBuff, pInputData + BufSize - pCycleBufCtrl->WriteAddr, LeftoverSize);
				pCycleBufCtrl->WriteAddr = LeftoverSize;
				break;
			}
		}
		else
		{
			if(ReadAddrLatch > pCycleBufCtrl->WriteAddr)
			{
				if (pCycleBufCtrl->WriteAddr + DataSize >= ReadAddrLatch)
				{
					bOverflow = TRUE;

				}
			}

			if(bOverflow)
			{
				OverflowFlag = CB_OVERFLOW_MASK;

				/* if in Task, clear buffer */
				if(bInTask)
				{
					/**@note do not delay here as it's in critical section now, otherwise it might cause unstable situation */
					pCycleBufCtrl->WriteAddr = pCycleBufCtrl->ReadAddr;
					CB_ERR_MSG(EOL "#E %s BufOvf dump data2" EOL, pCycleBufCtrl->pcBuffName);
					bOverflow = FALSE;
					pCycleBufCtrl->OverflowFlag = 0;
					pCycleBufCtrl->OverflowCount = 0;
					pCycleBufCtrl->FIFOReadyFlag = 0;
					DataInBufSize = 0;
					break;
				}
				else /* if in int, clear buffer */
				{
					pCycleBufCtrl->WriteAddr = pCycleBufCtrl->ReadAddr;
					CB_ERR_MSG(EOL "#E %s BufOvf2" EOL, pCycleBufCtrl->pcBuffName);
					bOverflow = FALSE;
					pCycleBufCtrl->OverflowFlag |= CB_OVERFLOW_MASK;
					break; /* drop pInputData in interrupt when overflow. [BBFM-101] */
				}
			}

			if(!bOverflow)
			{
				MEMCPY(pCycleBufCtrl->pBuff + pCycleBufCtrl->WriteAddr, pInputData, DataSize);
				pCycleBufCtrl->WriteAddr += DataSize;
				break;
			}
		}

	}while(bInTask);

/* Section 4: alert the fifo ready if needed	*/
	DataInBufSize += DataSize;
	if(DataInBufSize > pCycleBufCtrl->FIFOReadyThres)
	{
		pCycleBufCtrl->FIFOReadyFlag++;
	}

	return OverflowFlag;
}

/**********************************************************************//**
@brief get data from cycle buffer

notice, one shall call CycleBufUpdateReadAddr to update ReadAddr explicitly
after actual read

if it is a trail read, the read address shall not be updated

@param pCycleBufCtrl	[In]  cycle buffer control
@param pOutputData	[Out] output data pointer
@param ExpectDataSize [In] expect get data size

@return size of actual read data size

@author CHC
@date 2022/10/10 14:22:50
@note
History:
- 2022.10.10 Startup
**************************************************************************/
int CycleBufGetData(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, char * pOutputData, unsigned int ExpectDataSize)
{
	unsigned int WriteAddrLatch = pCycleBufCtrl->WriteAddr;
	unsigned int BufSize = pCycleBufCtrl->BufferSize;
	unsigned int DataInBufSize;
	unsigned int AcutalReadSize;
	unsigned int TotalReadSize;

	if(pCycleBufCtrl->ReadAddr > WriteAddrLatch)
	{
		DataInBufSize = BufSize - pCycleBufCtrl->ReadAddr + WriteAddrLatch ;
	}
	else
	{
		DataInBufSize = WriteAddrLatch - pCycleBufCtrl->ReadAddr;
	}

	/* small read */
	if(DataInBufSize > ExpectDataSize)
	{
		AcutalReadSize = ExpectDataSize;
	}
	else
	{
		AcutalReadSize = DataInBufSize;
	}

	TotalReadSize = pCycleBufCtrl->ReadAddr + AcutalReadSize;
	if(TotalReadSize > BufSize)
	{
		pCycleBufCtrl->pOutFunc(pCycleBufCtrl->PortAddress,
			pOutputData,pCycleBufCtrl->pBuff + pCycleBufCtrl->ReadAddr, BufSize - pCycleBufCtrl->ReadAddr);
		pCycleBufCtrl->pOutFunc(pCycleBufCtrl->PortAddress,
			pOutputData + BufSize - pCycleBufCtrl->ReadAddr,pCycleBufCtrl->pBuff, TotalReadSize - BufSize);
	}
	else
	{
		pCycleBufCtrl->pOutFunc(pCycleBufCtrl->PortAddress,
			pOutputData,pCycleBufCtrl->pBuff + pCycleBufCtrl->ReadAddr,AcutalReadSize);
	}

	if((DataInBufSize - AcutalReadSize) < pCycleBufCtrl->FIFOReadyThres)
	{
		if(pCycleBufCtrl->FIFOReadyFlag)
		{
			pCycleBufCtrl->FIFOReadyFlag --;
		}
	}

	return AcutalReadSize;
}

/**********************************************************************//**
@brief update read address to propagate the buffer

shall be called when actually need to read the data

@param pCycleBufCtrl	[In]  cycle buffer control
@param AcutalReadSize [In] expect get data size

@return size of actual read data size

@author CHC
@date 2022/10/10 14:22:50
@note
History:
- 2022.10.10 Startup
**************************************************************************/
void CycleBufUpdateReadAddr(CYCLE_BUFF_CTRL_T * pCycleBufCtrl, unsigned int AcutalReadSize)
{
	unsigned int TotalReadSize;

	TotalReadSize = pCycleBufCtrl->ReadAddr + AcutalReadSize;
	if(TotalReadSize >= pCycleBufCtrl->BufferSize)
	{
		pCycleBufCtrl->ReadAddr = TotalReadSize - pCycleBufCtrl->BufferSize;
	}
	else
	{
		pCycleBufCtrl->ReadAddr += AcutalReadSize;
	}
}

/**********************************************************************//**
@brief get data size from cycle buffer

@param pCycleBufCtrl    [In]  cycle buffer control

@return size of actual data

@author CHC
@date 2023/03/13
**************************************************************************/
unsigned int CycleBufGetDataSize(CYCLE_BUFF_CTRL_T * pCycleBufCtrl)
{
	unsigned int WriteAddrLatch = pCycleBufCtrl->WriteAddr;
	unsigned int BufSize = pCycleBufCtrl->BufferSize;
	unsigned int DataInBufSize;

	if(pCycleBufCtrl->ReadAddr > WriteAddrLatch)
	{
		DataInBufSize = BufSize - pCycleBufCtrl->ReadAddr + WriteAddrLatch ;
	}
	else
	{
		DataInBufSize = WriteAddrLatch - pCycleBufCtrl->ReadAddr;
	}

	return DataInBufSize;
}

