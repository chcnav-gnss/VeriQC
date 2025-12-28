#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		Message Convert Module
*-
@file   MsgConvert.h
@author CHC
@date   2023/06/26
@brief

**************************************************************************/
#ifndef _MSG_CONVERT_H_
#define _MSG_CONVERT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "MsgConvertDataTypes.h"
#include "Encoder/RINEX/RINEXDefines.h"
#include "TimeDefines.h"
#include "Common/GNSSSignal.h"
#include "Decoder/MsgDecodeDefinesEx.h"
#include "BSW/MsgDecoder/MsgDecodeDataTypes.h"

typedef enum _OUTPUT_FILE_TYPE_E
{
	OUTPUT_FILE_TYPE_RAWMSGA = 0,
	OUTPUT_FILE_TYPE_BASEOBSA,
	OUTPUT_FILE_TYPE_BASEOBSSATCOUNT,
	OUTPUT_FILE_TYPE_DEBUGINFO,
	OUTPUT_FILE_TYPE_RTCM,
	OUTPUT_FILE_TYPE_RTKPLOT,
	OUTPUT_FILE_TYPE_RAWMSGPROCESS,
	OUTPUT_FILE_TYPE_ASCII,
	OUTPUT_FILE_TYPE_BINARY,

	OUTPUT_FILE_TYPE_NUM,

	OUTPUT_FILE_TYPE_MAX = 0xFFFFFFFF,
} OUTPUT_FILE_TYPE_E;

int InitRinexInputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pRawPath);
int InitRinexMultiInputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pRawPath);
int InitOutputFile(unsigned int FileType, MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir);
int InitRINEXOutputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir);
int InitProcessOutputFile(unsigned int FileType, MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir);

int InitRinexMultiOutputDir(MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir);
int InitRinexMultiOutputFile(MSG_CONVERT_DATA_T* pMsgConvertData);

int ParseHCNHeaderData(MSG_CONVERT_DATA_T* pMsgConvertData);

const char* GetRawObsDataTypeString(unsigned int RawObasDataType);

int SetSysTObs(char* pStr, char TObsSys[][4]);

int AllFreqDecode(MSG_CONVERT_DATA_T* pMsgConvertData, MSG_DECODE_T* pMsgDecode, int Type);
int DataDecodeStart(MSG_CONVERT_DATA_T* pMsgConvertData, MSG_DECODE_T* pMsgDecode, int Type);
int RinexDecodeStart(MSG_CONVERT_DATA_T* pMsgConvertData);

int RinexMultiHeaderInit(MSG_CONVERT_DATA_T* pMsgConvertData);

int GetDataOutType(MSG_CONVERT_DATA_T* pMsgConvertData);

int MsgConvertDeinit(MSG_CONVERT_DATA_T* pMsgConvertData);
MSG_CONVERT_DATA_T* MsgConvertInit(void);

int GetDecodeType(int Type);
int GetOemType(int ver);
int MsgRecv(unsigned int PortAddr, void* pOutputData, void* pSourceData, unsigned int DataSize);

int GetMsgConvertProgress();
void SetMsgConvertProgress(int Value);

#ifdef __cplusplus
}
#endif

#endif  /** _MSG_CONVERT_H_ */

#endif
