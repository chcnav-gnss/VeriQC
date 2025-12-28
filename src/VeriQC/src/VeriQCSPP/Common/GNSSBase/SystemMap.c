/*
*********************************************************************************************************
*                                                VeriQC
*                                        the common applacation task for user
*
*
* File    : SystemMap.c
* By      : CHC
* Version : V0.01
* Date    : 2019-08-09
* --------------------
*  read file data and deal data and send data in a new file
*********************************************************************************************************
*/
#include <stdlib.h> /*malloc*/
#include <stdio.h> /*fprintf stdout*/
#include <stdarg.h>/*va_start va_end*/
#include "VeriQCSPP.h"
#include "GNSSBase.h"

static void VeriQCIoSendByPort(int32_t Port, int32_t Index, const uint8_t *pBuffer, int32_t Length, uint8_t NeedNotify)
{
    if (Port == 0)
    {
        //printf("%s", buffer);
        //OUTLOG("%s", buffer);
    }
}

static size_t VeriQCGetUsedMemDefault(void) { return 0; }
static size_t VeriQCGetFreeMemDefault(void) { return 0; }
static uint32_t VeriQCGetTickGetDefault(void) { return 0; }
static uint32_t VeriQCGetTaskGetDefault(void) { return 0; }

/****************************allocator****************************/

VERIQC_GET_USEDMEM_FUNC g_VeriQCGetUsedMemFunc = VeriQCGetUsedMemDefault;
VERIQC_GET_FREEMEM_FUNC g_VeriQCGetFreeMemFunc = VeriQCGetFreeMemDefault;
VERIQC_IO_SEND_BY_PORT_FUNC m_VeriQCGetSendByPortFunc = VeriQCIoSendByPort;
VERIQC_TICKGET_FUNC g_VeriQCTickGetFunc = VeriQCGetTickGetDefault;
VERIQC_TASKGET_FUNC g_VeriQCTaskGetFunc = VeriQCGetTaskGetDefault;

VERIQC_GET_USEDMEM_FUNC VeriQCGetMemusedFunc(void)
{
    return g_VeriQCGetUsedMemFunc;
}
VERIQC_GET_FREEMEM_FUNC VeriQCGetMemfreeFunc(void)
{
    return g_VeriQCGetFreeMemFunc;
}
VERIQC_TICKGET_FUNC VeriQCGetTickgetFunc(void)
{
	return g_VeriQCTickGetFunc;
}


