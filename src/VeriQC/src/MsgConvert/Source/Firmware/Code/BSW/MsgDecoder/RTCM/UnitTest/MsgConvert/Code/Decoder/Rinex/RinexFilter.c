#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   RinexFilter.c
@author CHC
@date   2023/12/30
@brief

**************************************************************************/
#include "OSDependent.h"
#include "RinexFilter.h"
#include "RinexDecoder.h"
#include "RinexDataTypes.h"

#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "BSW/MsgEncoder/RawMsg/RawMsgCommonDefines.h"

#include "../../MsgConvertDataTypes.h"
#include "../../MsgConvert.h"
#include "../../GNSSPrint.h"

#include "Encoder/RINEX/RINEXConvert.h"

#define RINEX_LEN_MAX			(16 * 1024)

typedef int (*CONVERT_RAWMSG_FUNC)(void*, void*, RM_HEADER_T*, int);
typedef int (*CONVERT_RTCM_FUNC)(int , void*, void*, int);
typedef int (*CONVERT_RINEX_FUNC)(int, int, void*, void*);

typedef struct _RINEX_TO_RTCM_SYSTEM_T
{
	int RinexSystem;
	int RTCMSystem;
} RINEX_TO_RTCM_SYSTEM_T;

/**********************************************************************//**
@brief  Rinex Filter information clear

@param pCodec	   [In] Physical Port ID index

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/08/03
@note
**************************************************************************/
int RinexFilterClear(MSG_DECODE_T* pDecode)
{
	if (pDecode == NULL)
		return -1;

	return 0;
}

/**********************************************************************//**
@brief  Rinex Filter

@param pDecode			[In] Physical Port ID index
@param DecodeData		[In] need decode data

@retval see PROTOCOL_ANALYZE_FRAM_ERR_E

@author CHC
@date 2023/08/03
@note
**************************************************************************/
PROTOCOL_FILTER_FRAM_ERR_E RinexFilter(MSG_DECODE_T* pDecode, unsigned char* pDecodeData, unsigned int DataLen, unsigned int* pOffset)
{
	if (pDecode == NULL)
		return PROTOCOL_FILTER_ERR;

	return PROTOCOL_WAIT_CONFIRM;
}

/**********************************************************************//**
@brief  Rinex filter deinit

@param pRinexFilterInfo	 [In] rinex filter info handle

@retval 0:success

@author CHC
@date 2023/06/21
@note
**************************************************************************/
int RinexFilterDeinit(RINEX_FILTER_INFO_T* pRinexFilterInfo)
{
	FREE(pRinexFilterInfo);
	pRinexFilterInfo = NULL;

	return 0;
}

/**********************************************************************//**
@brief  Rinex filter init

@retval success: return rtcm3 filter info handle, else return null

@author CHC
@date 2023/03/09
@note
**************************************************************************/
RINEX_FILTER_INFO_T* RinexFilterInit(void)
{
	RINEX_FILTER_INFO_T* pRinexFilterInfo = NULL;

	int InfoSize = MEM_CEILBY8(sizeof(RINEX_FILTER_INFO_T));
	int DecodeBufLen = MEM_CEILBY8(RINEX_LEN_MAX);
	int TotalSize = InfoSize + DecodeBufLen;

	pRinexFilterInfo = MALLOC(TotalSize);
	if (pRinexFilterInfo == NULL)
	{
		return NULL;
	}

	MEMSET(pRinexFilterInfo, 0x00, InfoSize);

	pRinexFilterInfo->DecodeBuf.Len = 0;
	pRinexFilterInfo->DecodeBuf.pBaseAddr = ((unsigned char*)(pRinexFilterInfo)) + InfoSize;

	return pRinexFilterInfo;
}

/**********************************************************************//**
@brief  Rinex decode

@param pCodec		   [In] pointer to codec
@param SourceType	   [In] data source type

@retval <0 error, =0 Not complete data, >0: RINEX id

@author CHC
@date 2023/12/30
@note
**************************************************************************/
int RinexDecode(MSG_DECODE_T* pDecode, int SourceType)
{
	if (pDecode == NULL)
		return -1;

	int Result = -1;


	return Result;
}

/**********************************************************************//**
@brief  Rinex decode deinit

@param pParseInfo	   [In] rtcm parse information

@retval <0:error, =0:success

@author CHC
@date 2023/12/20
@note
**************************************************************************/
int RinexDecodeDeinit(RINEX_PARSE_INFO_T* pRinexParseInfo)
{
	if (!pRinexParseInfo)
		return -1;

	FREE(pRinexParseInfo);

	return 0;
}

/**********************************************************************//**
@brief  Rinex decode init

@retval struct RINEX_PARSE_INFO_T pointer

@author CHC
@date 2023/03/09
@note
**************************************************************************/
RINEX_PARSE_INFO_T* RinexDecodeInit(void)
{
	RINEX_PARSE_INFO_T* pRinexParseInfo = NULL;
	int RinexSize = MEM_CEILBY8(sizeof(RINEX_PARSE_INFO_T));
	int RinexParseBufSize = MEM_CEILBY8(RINEX_PARSED_LEN_MAX);
	int TotalSize = RinexSize + RinexParseBufSize;

	pRinexParseInfo = MALLOC(TotalSize);
	if (pRinexParseInfo == NULL)
	{
		return NULL;
	}

	MEMSET(pRinexParseInfo, 0x00, sizeof(RINEX_PARSE_INFO_T));

	pRinexParseInfo->ParsedDataBuf.pBaseAddr = (unsigned char*)pRinexParseInfo + RinexSize;

	MEMSET(pRinexParseInfo->ParsedDataBuf.pBaseAddr, 0x00, RinexParseBufSize);

	return pRinexParseInfo;
}

/**********************************************************************//**
@brief  get least time file index

@param pUTCTime		[In] UTC Time

@retval <0:error, =0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int GetLeastTimeFileIndex(UTC_TIME_T* pUTCTime, int* pResultBuf)
{
	unsigned int FileTypeIndex;
	UTC_TIME_T TempUTCTime = {0};
	int FileIndex = -1;

	for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
	{
		if (pResultBuf[FileTypeIndex] >= 0)
		{
			if ((FileIndex < 0) && (pUTCTime[FileTypeIndex].Year > 0))
			{
				TempUTCTime = pUTCTime[FileTypeIndex];
				FileIndex = FileTypeIndex;
			}
			else if ((FileIndex >= 0) && (TempUTCTime.Year > 0) && (pUTCTime[FileTypeIndex].Year > 0))
			{
				if (UTCDiff(&pUTCTime[FileTypeIndex], &TempUTCTime) < 0)
				{
					TempUTCTime = pUTCTime[FileTypeIndex];
					FileIndex = FileTypeIndex;
				}
			}
		}
	}

	return FileIndex;
}

/**********************************************************************//**
@brief  get least time file index

@param pUTCTime			[In] UTC Time
@param ResultBuf		[In] file decode result
@param FileGroupNum		[In] file group number

@retval <0:error, =0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int GetLeastTimeMultiFileIndex(UTC_TIME_T UTCTime[][RINEX_FILE_INDEX_NUM], int ResultBuf[][RINEX_FILE_INDEX_NUM], unsigned int FileGroupNum, int* pFileGroupIndex)
{
	unsigned int FileGroupIndex;
	unsigned int FileTypeIndex;
	UTC_TIME_T TempUTCTime = {0};
	int FileIndex = -1;
	int GroupIndex = -1;

	for (FileGroupIndex = 0; FileGroupIndex < FileGroupNum; FileGroupIndex++)
	{
		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			if (ResultBuf[FileGroupIndex][FileTypeIndex] >= 0)
			{
				if ((FileIndex < 0) && (UTCTime[FileGroupIndex][FileTypeIndex].Year > 0))
				{
					TempUTCTime = UTCTime[FileGroupIndex][FileTypeIndex];
					FileIndex = FileTypeIndex;
					GroupIndex = FileGroupIndex;
				}
				else if ((FileIndex >= 0) && (TempUTCTime.Year > 0) && (UTCTime[FileGroupIndex][FileTypeIndex].Year > 0))
				{
					if (UTCDiff(&UTCTime[FileGroupIndex][FileTypeIndex], &TempUTCTime) < 0)
					{
						TempUTCTime = UTCTime[FileGroupIndex][FileTypeIndex];
						FileIndex = FileTypeIndex;
						GroupIndex = FileGroupIndex;
					}
				}
			}
		}
	}

	*pFileGroupIndex = GroupIndex;

	return FileIndex;
}

/**********************************************************************//**
@brief  check rinex decode over

@param pResultBuf		[In] rinex decode result buffer

@retval <0:decode over, =0:decoding

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int CheckRinexDecodeOver(int* pResultBuf)
{
	unsigned int FileTypeIndex;

	if (!pResultBuf)
	{
		return -1;
	}

	for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
	{
		if (pResultBuf[FileTypeIndex] >= 0)
		{
			return 0;
		}
	}

	return -1;
}

/**********************************************************************//**
@brief  check rinex multi decode over

@param pResultBuf		[In] rinex decode result buffer
@param FileGroupNum		[In] file group number

@retval <0:decode over, =0:decoding

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int CheckRinexMultiDecodeOver(int ResultBuf[][RINEX_FILE_INDEX_NUM], unsigned int FileGroupNum)
{
	unsigned int FileGroupIndex;
	unsigned int FileTypeIndex;

	for (FileGroupIndex = 0; FileGroupIndex < FileGroupNum; FileGroupIndex++)
	{
		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			if (ResultBuf[FileGroupIndex][FileTypeIndex] >= 0)
			{
				return 0;
			}
		}
	}

	return -1;
}

/**********************************************************************//**
@brief  get rtcm system mask

@param RinexNavSys		[In] rinex nav system

@retval <0:error, =0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int GetRTCMSystemMask(int RinexNavSys)
{
	unsigned int DataIndex;
	int SystemMask = 0;

	static const RINEX_TO_RTCM_SYSTEM_T s_RinexToRTCMSystemMap[] =
	{
		{RINEX_SYS_GPS, RTCM_SYS_GPS},
		{RINEX_SYS_GLO, RTCM_SYS_GLO},
		{RINEX_SYS_GAL, RTCM_SYS_GAL},
		{RINEX_SYS_BDS, RTCM_SYS_BDS},
		{RINEX_SYS_SBA, RTCM_SYS_SBA},
		{RINEX_SYS_QZS, RTCM_SYS_QZS},
		{RINEX_SYS_NIC, RTCM_SYS_NIC},
	};

	for (DataIndex = 0; DataIndex < (sizeof(s_RinexToRTCMSystemMap)/sizeof(s_RinexToRTCMSystemMap[0])); DataIndex++)
	{
		if (RinexNavSys & s_RinexToRTCMSystemMap[DataIndex].RinexSystem)
		{
			SystemMask |= 1 << s_RinexToRTCMSystemMap[DataIndex].RTCMSystem;
		}
	}

	return SystemMask;
}

/**********************************************************************//**
@brief  rinex multi file check time

@param pRinexInfo		[In] rinex information
@param RinexGroupNum	[In] rinex group number

@retval <0:error,Two file time intervals overlap
		=0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int RinexMultiFileCheckTime(RINEX_INFO_T* pRinexInfo, unsigned int RinexGroupNum)
{
	unsigned int FileGroupIndex1, FileGroupIndex2;
	RINEX_OBS_HEADER_T* pObsHeader1;
	RINEX_OBS_HEADER_T* pObsHeader2;
	INT64 TimeDiff1 = 0;
	INT64 TimeDiff2 = 0;

	if (RinexGroupNum <= 1)
	{
		return -1;
	}

	for (FileGroupIndex1 = 0; FileGroupIndex1 < RinexGroupNum; FileGroupIndex1++)
	{
		pObsHeader1 = &pRinexInfo[FileGroupIndex1].ObsHeaderInfo.ObsHeader;
		for (FileGroupIndex2 = FileGroupIndex1+1; FileGroupIndex2 < RinexGroupNum; FileGroupIndex2++)
		{
			pObsHeader2 = &pRinexInfo[FileGroupIndex2].ObsHeaderInfo.ObsHeader;

			TimeDiff1 = UTCDiff(&pObsHeader2->TimeStart, & pObsHeader1->TimeStart);
			TimeDiff2 = UTCDiff(&pObsHeader2->TimeStart, &pObsHeader1->TimeEnd);
			if ((TimeDiff1 > 0) && (TimeDiff2 < 0))
			{
				return -1;
			}

			TimeDiff1 = UTCDiff(&pObsHeader2->TimeEnd, &pObsHeader1->TimeStart);
			TimeDiff2 = UTCDiff(&pObsHeader2->TimeEnd, &pObsHeader1->TimeEnd);
			if ((TimeDiff1 > 0) && (TimeDiff2 < 0))
			{
				return -1;
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  Get rinex multi convert data

@param pRinexInfo				[In] rinex information
@param RinexGroupNum			[In] rinex group number
@param pRinexMultiConvertData	[Out] Rinex Multi Convert Data

@retval <0:error, -2:Leapsecs data error;
				-3:PhaseShift Data error;
		=0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int GetRinexMultiConvertData(RINEX_INFO_T* pRinexInfo, unsigned int RinexGroupNum, RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData)
{
	unsigned int FileGroupIndex;
	unsigned int SysIndex;
	unsigned int SignalIndex;
	unsigned int SatIndex;
	int DataIndex;
	int ObsTypeIndex;
	RINEX_OBS_HEADER_T* pMultiObsHeader = &pRinexMultiConvertData->RINEXObsHeader;
	RINEX_OBS_HEADER_T* pObsHeader;
	RINEX_PHASE_SHIFT_INFO_T* pRinexMultiPhaseShiftInfo;
	RINEX_PHASE_SHIFT_INFO_T* pRinexPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pMultiPerSignalPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pPerSignalPhaseShiftInfo;
	PER_SIGNAL_PHASE_SHIFT_INFO_T* pNowPerSignalPhaseShiftInfo;
	RINEX_PHASE_SHIFT_INFO_T RinexPhaseShiftInfo = {0};
	INT64 TimeDiff;
	unsigned int ObsTypeCount = 0;
	char* pObsTypeStr;

	if (RinexGroupNum <= 1)
	{
		return -1;
	}

	MEMCPY(pMultiObsHeader, &pRinexInfo[0].ObsHeaderInfo.ObsHeader, sizeof(RINEX_OBS_HEADER_T));
	pRinexMultiPhaseShiftInfo = &pMultiObsHeader->RinexPhaseShiftInfo;

	for (FileGroupIndex = 1; FileGroupIndex < RinexGroupNum; FileGroupIndex++)
	{
		pObsHeader = &pRinexInfo[FileGroupIndex].ObsHeaderInfo.ObsHeader;
		pRinexPhaseShiftInfo = &pObsHeader->RinexPhaseShiftInfo;

		if (pObsHeader->Leapsecs != pMultiObsHeader->Leapsecs)
		{
			return -2;
		}

		TimeDiff = UTCDiff(&pObsHeader->TimeStart, &pMultiObsHeader->TimeStart);
		if (TimeDiff < 0)
		{
			MEMCPY(&pMultiObsHeader->TimeStart, &pObsHeader->TimeStart, sizeof(UTC_TIME_T));
		}

		TimeDiff = UTCDiff(&pObsHeader->TimeEnd, &pMultiObsHeader->TimeEnd);
		if (TimeDiff > 0)
		{
			MEMCPY(&pMultiObsHeader->TimeEnd, &pObsHeader->TimeEnd, sizeof(UTC_TIME_T));
		}

		if (pMultiObsHeader->TimeInt < pObsHeader->TimeInt)
		{
			pMultiObsHeader->TimeInt = pObsHeader->TimeInt;
		}

		pMultiObsHeader->NavSys |= pObsHeader->NavSys;

		for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
		{
			ObsTypeCount = pObsHeader->ObsTypeCount[SysIndex];
			for (DataIndex = 0; DataIndex < pObsHeader->ObsTypeCount[SysIndex]; DataIndex++)
			{
				pObsTypeStr = pObsHeader->ObsType[SysIndex][DataIndex];
				for (ObsTypeIndex = 0; ObsTypeIndex < pMultiObsHeader->ObsTypeCount[SysIndex]; ObsTypeIndex++)
				{
					if (0 == MEMCMP(pObsTypeStr, pMultiObsHeader->ObsType[SysIndex][ObsTypeIndex], 3))
					{
						break;
					}
				}

				if (ObsTypeIndex >= pMultiObsHeader->ObsTypeCount[SysIndex])
				{
					MEMCPY(pMultiObsHeader->ObsType[SysIndex][pMultiObsHeader->ObsTypeCount[SysIndex]], pObsTypeStr, 3);
					pMultiObsHeader->ObsTypeCount[SysIndex]++;
				}
			}
		}
	}

	for (FileGroupIndex = 0; FileGroupIndex < RinexGroupNum; FileGroupIndex++)
	{
		pRinexPhaseShiftInfo = &pRinexInfo[FileGroupIndex].ObsHeaderInfo.ObsHeader.RinexPhaseShiftInfo;

		for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
		{
			for (SignalIndex = 0; SignalIndex < 32; SignalIndex++)
			{
				pNowPerSignalPhaseShiftInfo = &RinexPhaseShiftInfo.PerSysPhaseShiftInfo[SysIndex].PerSignalPhaseShiftInfo[SignalIndex];
				pPerSignalPhaseShiftInfo = &pRinexPhaseShiftInfo->PerSysPhaseShiftInfo[SysIndex].PerSignalPhaseShiftInfo[SignalIndex];

				if ((pNowPerSignalPhaseShiftInfo->SatNum != 0) || (pPerSignalPhaseShiftInfo->SatNum != 0))
				{
					if ((!IsAbsoluteDoubleZero(pNowPerSignalPhaseShiftInfo->PhaseShiftData)) &&(pNowPerSignalPhaseShiftInfo->PhaseShiftData != pPerSignalPhaseShiftInfo->PhaseShiftData))
					{
						return -3; /**< PhaseShiftData*/
					}

					pNowPerSignalPhaseShiftInfo->PhaseShiftData = pPerSignalPhaseShiftInfo->PhaseShiftData;

					for (SatIndex = 0; SatIndex < 100; SatIndex++)
					{
						if (pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] != 0)
						{
							pNowPerSignalPhaseShiftInfo->SatIDMap[pPerSignalPhaseShiftInfo->SatIDMap[SatIndex] - 1] = 1;
						}
					}
				}
			}
		}
	}

	for (SysIndex = 0; SysIndex < TOTAL_EX_GNSS_SYSTEM_NUM; SysIndex++)
	{
		for (SignalIndex = 0; SignalIndex < 32; SignalIndex++)
		{
			pNowPerSignalPhaseShiftInfo = &RinexPhaseShiftInfo.PerSysPhaseShiftInfo[SysIndex].PerSignalPhaseShiftInfo[SignalIndex];
			pMultiPerSignalPhaseShiftInfo = &pRinexMultiPhaseShiftInfo->PerSysPhaseShiftInfo[SysIndex].PerSignalPhaseShiftInfo[SignalIndex];

			MEMSET(pMultiPerSignalPhaseShiftInfo, 0, sizeof(PER_SIGNAL_PHASE_SHIFT_INFO_T));

			for (SatIndex = 0; SatIndex < 100; SatIndex++)
			{
				if (pNowPerSignalPhaseShiftInfo->SatIDMap[SatIndex] != 0)
				{
					if (GLO == SysIndex)
					{
						pMultiPerSignalPhaseShiftInfo->SatIDMap[pMultiPerSignalPhaseShiftInfo->SatNum] = SatIndex + 1 + 37;
					}
					else if (QZS == SysIndex)
					{
						pMultiPerSignalPhaseShiftInfo->SatIDMap[pMultiPerSignalPhaseShiftInfo->SatNum] = SatIndex + 1 + (MIN_QZSS_PRN - 1);
					}
					else if (SBA == SysIndex)
					{
						pMultiPerSignalPhaseShiftInfo->SatIDMap[pMultiPerSignalPhaseShiftInfo->SatNum] = SatIndex + 1 + 100;
					}
					else
					{
						pMultiPerSignalPhaseShiftInfo->SatIDMap[pMultiPerSignalPhaseShiftInfo->SatNum] = SatIndex + 1;
					}
					pMultiPerSignalPhaseShiftInfo->SatNum++;
				}
			}

			pMultiPerSignalPhaseShiftInfo->PhaseShiftData = pNowPerSignalPhaseShiftInfo->PhaseShiftData;
		}
	}

	pRinexMultiConvertData->RINEXObsHeader.TimeDiff = UTCDiff(&pRinexMultiConvertData->RINEXObsHeader.TimeEnd, &pRinexMultiConvertData->RINEXObsHeader.TimeStart);

	MEMCPY(&pRinexMultiConvertData->RINEXObsOpt.TimeStart, &pRinexMultiConvertData->RINEXObsHeader.TimeStart, sizeof(UTC_TIME_T));
	MEMCPY(&pRinexMultiConvertData->RINEXObsOpt.TimeEnd, &pRinexMultiConvertData->RINEXObsHeader.TimeEnd, sizeof(UTC_TIME_T));
	MEMCPY(pRinexMultiConvertData->RINEXObsOpt.NObs, pRinexMultiConvertData->RINEXObsHeader.ObsTypeCount, sizeof(pRinexMultiConvertData->RINEXObsHeader.ObsTypeCount));
	MEMCPY(pRinexMultiConvertData->RINEXObsOpt.ObsTypeBuf, pRinexMultiConvertData->RINEXObsHeader.ObsType, sizeof(pRinexMultiConvertData->RINEXObsHeader.ObsType));
	pRinexMultiConvertData->RINEXObsOpt.NavSys = pRinexMultiConvertData->RINEXObsHeader.NavSys;

	pRinexMultiConvertData->RINEXObsHeader.Ver = 304;
	pRinexMultiConvertData->RINEXEphHeader.Ver = 304;
	pRinexMultiConvertData->RINEXGEphHeader.Ver = 304;
	MEMSET(pRinexMultiConvertData->RinexNavFileFpFlag, 1, sizeof(pRinexMultiConvertData->RinexNavFileFpFlag));

	pRinexMultiConvertData->Rece = 0;
	pRinexMultiConvertData->MaxPoshis = 1;

	return 0;
}

/**********************************************************************//**
@brief  rinex deocde convert rinex

@param pSrcObj		[In] pointer to source object

@retval <0:error, =0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
static int RinexDecodeConvertRinex(void* pSrcObj)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;

	unsigned int FileTypeIndex, FileGroupIndex;
	int HeaderFlag[RINEX_GROUP_NUM_MAX][RINEX_FILE_INDEX_NUM];
	UTC_TIME_T UTCTimeBuf[RINEX_GROUP_NUM_MAX][RINEX_FILE_INDEX_NUM] = {0};
	int RinexIDBuf[RINEX_GROUP_NUM_MAX][RINEX_FILE_INDEX_NUM] = {0};
	int ResultBuf[RINEX_GROUP_NUM_MAX][RINEX_FILE_INDEX_NUM];
	int LeastFileIndex;
	int Result;
	int LeastFileGroupIndex;

	const static CONVERT_RINEX_FUNC RinexConvertRinexFuncsMap[] =
	{
		RinexDecodeObsConvertRinex,
		RinexDecodeMultiEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
		RinexDecodeGLOEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
		RinexDecodeUniEphConvertRinex,
	};

	MEMSET(HeaderFlag, -1, sizeof(HeaderFlag));
	MEMSET(ResultBuf, -1, sizeof(ResultBuf));

	for (FileGroupIndex = 0; FileGroupIndex < pMsgConvertData->RinexMultiFileNum; FileGroupIndex++)
	{
		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			if (pMsgConvertData->RinexMultiFileFpFlag[FileGroupIndex][FileTypeIndex])
			{
				HeaderFlag[FileGroupIndex][FileTypeIndex] = OpenRinexInfo(&pMsgConvertData->RinexMultiInfo[FileGroupIndex], pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex], FileTypeIndex);
				if (HeaderFlag[FileGroupIndex][FileTypeIndex] < 0)
				{
					return -1;
				}
			}
		}
	}

	if (RinexMultiFileCheckTime(pMsgConvertData->RinexMultiInfo, pMsgConvertData->RinexMultiFileNum) < 0)
	{
		GNSSPrintf("Two file time intervals overlap, please check file!\r\n");
		return -1;
	}

	Result = GetRinexMultiConvertData(pMsgConvertData->RinexMultiInfo, pMsgConvertData->RinexMultiFileNum, &pMsgConvertData->RinexMultiConvertData);
	if (Result < 0)
	{
		if (Result == -2)
		{
			GNSSPrintf("Leapsecs data error, please check file!\r\n");
		}
		else if (Result == -3)
		{
			GNSSPrintf("PhaseShift Data error, please check file!\r\n");
		}
		else
		{
			GNSSPrintf("File header merge error, please check file!\r\n");
		}

		return -1;
	}

	InitRinexMultiOutputFile(pMsgConvertData);

	RinexMultiHeaderInit(pMsgConvertData);

	for (FileGroupIndex = 0; FileGroupIndex < pMsgConvertData->RinexMultiFileNum; FileGroupIndex++)
	{
		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			if ((pMsgConvertData->RinexMultiFileFpFlag[FileTypeIndex]) && (HeaderFlag[FileGroupIndex][FileTypeIndex] > 0))
			{
				ResultBuf[FileGroupIndex][FileTypeIndex] = InputRinexInfo(&pMsgConvertData->RinexMultiInfo[FileGroupIndex], pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex], FileTypeIndex, &UTCTimeBuf[FileGroupIndex][FileTypeIndex], &RinexIDBuf[FileGroupIndex][FileTypeIndex]);
			}
		}
	}

	if (CheckRinexMultiDecodeOver(ResultBuf, pMsgConvertData->RinexMultiFileNum) < 0)
	{
		return -1;
	}

	LeastFileIndex = GetLeastTimeMultiFileIndex(UTCTimeBuf, ResultBuf, pMsgConvertData->RinexMultiFileNum, &LeastFileGroupIndex);
	if (LeastFileIndex < 0)
	{
		return -1;
	}

	if (RinexConvertRinexFuncsMap[LeastFileIndex])
	{
		if (LeastFileIndex == RINEX_FILE_INDEX_OBS)
		{
			RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RINEX_ID_OBS, pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].ObsData);
		}
		else if (LeastFileIndex == RINEX_FILE_INDEX_MUTIEPH)
		{
			if (RINEX_ID_GLOEPH == RinexIDBuf[LeastFileGroupIndex][LeastFileIndex])
			{
				RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].GLOEphData);
			}
			else
			{
				RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].UniEphData[LeastFileIndex]);
			}
		}
		else if (LeastFileIndex == RINEX_FILE_INDEX_GLOEPH)
		{
			RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].GLOEphData);
		}
		else
		{
			RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].UniEphData[LeastFileIndex]);
		}
	}

	while ((LeastFileIndex >= 0) && (LeastFileIndex < RINEX_FILE_INDEX_NUM))
	{
		ResultBuf[LeastFileGroupIndex][LeastFileIndex] = InputRinexInfo(&pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex], pMsgConvertData->RinexMultiFileFp[LeastFileGroupIndex][LeastFileIndex], LeastFileIndex, &UTCTimeBuf[LeastFileGroupIndex][LeastFileIndex], &RinexIDBuf[LeastFileGroupIndex][LeastFileIndex]);
		if (CheckRinexMultiDecodeOver(ResultBuf, pMsgConvertData->RinexMultiFileNum) < 0)
		{
			return 0;
		}

		LeastFileIndex = GetLeastTimeMultiFileIndex(UTCTimeBuf, ResultBuf, pMsgConvertData->RinexMultiFileNum, &LeastFileGroupIndex);
		if ((LeastFileIndex >= 0) && (RinexConvertRinexFuncsMap[LeastFileIndex]))
		{
			if (LeastFileIndex == RINEX_FILE_INDEX_OBS)
			{
				RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RINEX_ID_OBS, pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].ObsData);
			}
			else if (LeastFileIndex == RINEX_FILE_INDEX_MUTIEPH)
			{
				if (RINEX_ID_GLOEPH == RinexIDBuf[LeastFileGroupIndex][LeastFileIndex])
				{
					RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].GLOEphData);
				}
				else
				{
					RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].UniEphData[LeastFileIndex]);
				}
			}
			else if (LeastFileIndex == RINEX_FILE_INDEX_GLOEPH)
			{
				RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].GLOEphData);
			}
			else
			{
				RinexConvertRinexFuncsMap[LeastFileIndex](RAW_DATA_TYPE_RINEX, RinexIDBuf[LeastFileGroupIndex][LeastFileIndex], pMsgConvertData, &pMsgConvertData->RinexMultiInfo[LeastFileGroupIndex].UniEphData[LeastFileIndex]);
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  rinex deocde convert

@param pSrcObj		[In] pointer to source object

@retval <0:error, =0:success

@author CHC
@date 2023/12/27
@note
**************************************************************************/
int RinexDecodeConvert(void* pSrcObj)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = (MSG_CONVERT_DATA_T*)pSrcObj;

	if (pMsgConvertData->DataOutType == DATA_OUT_TYPE_RINEX)
	{
		return RinexDecodeConvertRinex(pSrcObj);
	}
	else
	{
		return -1;
	}
}

#endif
