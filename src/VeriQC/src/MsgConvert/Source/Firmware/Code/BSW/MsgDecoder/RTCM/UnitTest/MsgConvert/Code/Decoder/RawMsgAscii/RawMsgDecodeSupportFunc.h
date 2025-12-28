#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RawMsgDecodeSupportFunc.h
@author CHC
@date   2023/09/12
@brief

**************************************************************************/
#ifndef _RAWMSG_DECODE_SUPPORT_FUNC_H_
#define _RAWMSG_DECODE_SUPPORT_FUNC_H_

#include "RawMsgDecodeDataStructure.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgRangeDefines.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define GetRawMsgBinaryPortID(PortID)	(((PortID) <= 0x60) ? ((PortID) / 0x20) : (1))

float GetPSRValue(int StdDevPSRValue);
int GetPortID(char* pPortStr, unsigned int MsgLen);
int GetGPSTimeStatus(char* pTimeStatus, unsigned int MsgLen);
int GetSolutionStatus(char* pSolStatus, unsigned int MsgLen);
int GetSolutionStatusEx(char* pSolStatus, unsigned int MsgLen);
int GetObservationStatus(char* pStatus, unsigned int MsgLen);
int GetPositionVelocityType(char* pType, unsigned int MsgLen);
int GetPositionVelocityTypeEx(char* pType, unsigned int MsgLen);
int GetSatelliteSystem(char* pSystem, unsigned int MsgLen);
int GetClockModelStatus(char* pClockStatus, unsigned int MsgLen);
int GetUTCStatus(char* pUTCStatus, unsigned int MsgLen);
int GetDatumID(char* pDatumID, unsigned int MsgLen);
int GetRefStationType(char* pStationType, unsigned int MsgLen);
int GetBaseStationType(char* pStationType, unsigned int MsgLen);
int GetVersionPlatformType(char* pPlatformType, unsigned int MsgLen);
int GetVersionBuildType(char* pBuildType, unsigned int MsgLen);
int GetINSStatus(char* pINSStatus, unsigned int MsgLen);
int GetVersionModifyFlag(char* pModifyFlag, unsigned int MsgLen);
int GetRAWRTCMAUXMsgType(char* pMsgType, unsigned int MsgLen);
int GetSwitchStatus(char* pSwitchStatus, unsigned int MsgLen);
int GetMarkTimeID(char* pMarkTime, unsigned int MsgLen);
int GetMarkPosID(char* pMarkPos, unsigned int MsgLen);
int GetEventTID(char* pEventTime, unsigned int MsgLen);
int GetEventPID(char* pEventPos, unsigned int MsgLen);
int GetVendorType(char* pVendorType, unsigned int MsgLen);
int GetVendorTypeVer2(char* pVendorType, unsigned int MsgLen);
int GetMassStorageDevice(char* pDeviceType, unsigned int MsgLen);
int GetFileStatus(char* pFileStatus, unsigned int MsgLen);
int GetMassStorageStatus(char* pStorageStatus, unsigned int MsgLen);
int GetFileMsgFileType(char* pFileType, unsigned int MsgLen);
int GetGNSSObsSystemID(char* pSystemID, unsigned int MsgLen);
int GetGNSSObsSignalType(unsigned int SystemID, char* pSignalType, unsigned int MsgLen);
int GetBestSatInfoSystemID(char* pSystemID, unsigned int MsgLen);

unsigned int EphemURASqTrans(double UARSq);
unsigned int EphemURATrans(double URA);

int AddObsData(GNSS_OBS_DATA_SAT_T* pGNSSObsData, int RMSystemID, RM_RANGE_DATA_T* pObsData);
int GNSSObsDataToObsData(GNSS_OBS_DATA_SAT_T* pGNSSObsData, GNSS_OBS_T* pObsData);
int GetRMRangeSatIndex(unsigned int RMSystemID, unsigned int PRN);

double GetNVPhaseAlignmentDataCycle(int ExSystemID, int RMSignalID);
double GetRinexPhaseAlignmentDataCycle(int ExSystemID, int RMSignalID);

int UNSignalTypeToRMSignalType(int RMSystem, int UNSignalType);
int UNObsDataConvert(GNSS_OBS_T* pObsData);

unsigned int GNSSObsSystemToRMRangeSystem(int SystemID);
unsigned int GNSSObsSystemToExSystem(int SystemID);
unsigned int GNSSObsSignalToRMRangeSignal(int GNSSObsSystemID, unsigned int SignalType, unsigned int* pFreqIndex);
int RawMsgExHeaderToHeader(RM_EX_HEADER_T* pExHeader, RM_HEADER_T* pHeader);
int RawMsgHeaderToExHeader(RM_HEADER_T* pHeader, RM_EX_HEADER_T* pExHeader);

int RawMsgExHeaderAsciiDecode(char *pBuf, char **pOutBuf, RM_EX_HEADER_T* pLogHeader);
int RawMsgHeaderAsciiDecode(char *pBuf, char **pOutBuf, RM_HEADER_T* pLogHeader);
unsigned int GetGNSSObsFreqIndex(int SystemID, unsigned int SingleType);

void CalculateGALEphTime(int GPSWeek, double GPSSeconds, double t0e, int *EphWeek, double *EphSeconds);

int ExGNSSObsToRangeLog(RM_EX_GNSSOBS_LOG_T* pSource, RM_RANGE_LOG_T* pDist);

int UNObsDataConvert(GNSS_OBS_T* pObsData);

#ifdef __cplusplus
}
#endif

#endif  /** _RAWMSG_DECODE_SUPPORT_FUNC_H_ */

#endif
