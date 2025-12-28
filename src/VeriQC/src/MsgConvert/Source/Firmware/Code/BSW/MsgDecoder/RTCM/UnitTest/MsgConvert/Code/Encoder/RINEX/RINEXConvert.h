#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		RINEX Convert Module
*-
@file   RINEXConvert.h
@author CHC
@date   2023/06/29
@brief

**************************************************************************/
#ifndef _RINEX_CONVERT_H_
#define _RINEX_CONVERT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>
#include "RINEXDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

int OutRINEXObsHeader(FILE* fp, RINEX_OBS_OPT_T* pObsOpt, RINEX_OBS_HEADER_T* pHeader, MSG_BUF_T* pEncodeBuf);

int SignalDecodeObsConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeGNSSObsConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeEphConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);

int SignalDecodeAntennaInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeStationPositionConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeRefStationInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeRefStationConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int SignalDecodeBestPosInfoConvert(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);

int DataDecodeObsConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int DataDecodeGNSSObsConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);

int DataDecodeUniEphConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int DataDecodeGLOEphConvertRINEX(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);

int DataDecodeAntennaInfoConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int DataDecodeReceiverInfoConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);
int DataDecodeStationPositionConvertShow(int RawDataType, int DecodeID, void* pSrcObj, void* pData, int RawObsDataType);

int RinexDecodeObsConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData);
int RinexDecodeMultiEphConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData);
int RinexDecodeUniEphConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData);
int RinexDecodeGLOEphConvertRinex(int RawDataType, int DecodeID, void* pSrcObj, void* pData);
     
#ifdef __cplusplus
}
#endif

#endif  /** _RINEX_CONVERT_H_ */

#endif
