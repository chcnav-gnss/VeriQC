/**********************************************************************//**
		   VeriQC

	Interface Module
*-
@file   PrintFunc.h
@author CHC
@date   2024/06/18
@brief  file output related functions

**************************************************************************/
#ifndef _PRINT_FUNC_H_
#define _PRINT_FUNC_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "DataQC.h"
#include "QCCommon.h"

static unsigned char s_DopplerOut = 0;/**< 0: no output 1: out Ana Result to HTML 2:output file for each epoch(and option 1) */

typedef enum _LOG_HEADTYPE_E          /**< log fomat type */
{
    E_LOG_HEADTYPE_1 = 1,             /**< GpsWeek,GpsSec,Prn,...*/
    E_LOG_HEADTYPE_2 = 2,             /**< TimeStr,Prn,...*/
	E_LOG_HEADTYPE_MAX = 0xFFFFFFFF   /**< TimeStr,Prn,...*/
}LOG_HEADTYPE_E;

void Char2WChar(const char* pChar, wchar_t* pWChar);

void UpdateString2File(FILE* fpOutFile, char* pBuffer, int* pBufferLen);
int PrintSNRDegHtml(char* pBuffer, int* pBufferLen, int AntID, QC_RESULT_T* pQcRes);

void PrintEpochMP(QC_TIME_T* pTime, FILE* fpOutFile, QC_RESULT_T* pQcRes, int AntID, LOG_HEADTYPE_E eLogType);
void PrintEpochCS(QC_TIME_T* pTime, FILE* fpOutFile, QC_RESULT_T* pQcRes, int AntID, LOG_HEADTYPE_E eLogType);
void PrintEpochDataList(QC_TIME_T* pTime, FILE* fpOutFile, QC_OBS_T* pObs, int (*pSlipFlag)[QC_NUM_FREQ], LOG_HEADTYPE_E eLogType, BOOL bCheckLLI, double EleMask);


#endif /**< _PRINT_FUNC_H_ */