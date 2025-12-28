/*************************************************************************//**
           VeriQC

	Interface Module
 *-
@file   QCProcessor.h
@author CHC
@date   2 March 2023
@brief  Some applications of QC
*****************************************************************************/
#ifndef _QC_PROCESSER_H_
#define _QC_PROCESSER_H_
#include "QCCommon.h"
#include "DataQC.h"
#include "SPPSol.h"

#define OBS_CODEC_BUF_SIZE 50*1024

void InitObsCodec(char* pFileName, CHCQC_READER_T* pQcReader, int AntID, int Index, QC_TIME_T RTCMTime);
BOOL InitNavCodec(char* pFileName, CHCQC_READER_T* pQcReader, uint8_t OrderNumber);
void InitQCRes(QC_RESULT_T* pQCRes);
void ResetQCReader(CHCQC_READER_T* pQcReader);
void ResetQCfp(CHCQC_READER_T* pQcReader);

extern int IsInvailQCTimeRange(const CHCQC_READER_T* pQCReader, const QC_TIME_T* pCurrentTime, int ID);
void QCSPPTcDefault(VERIQC_SPPTC_CREATE_OPT_T* pOpt0);
void ConvertObsEpoch(QC_OBS_T* pObs, const GNSS_OBS_T* pObsSrc, VERIQC_SIMPLIFY_RESULT_T* pRes, CHCQC_READER_T* pQCReader,int AntID, int Week, double Second);

int DetectFileFormat(const char* pFileName);

#endif