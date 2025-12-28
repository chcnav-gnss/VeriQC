#ifdef MSG_CONVERT
/**********************************************************************//**
		VeriQC

		Message Convert Module
*-
@file   MsgConvert.c
@author CHC
@date   2023/06/26
@brief

**************************************************************************/

#if defined(__linux)
#define PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#elif defined(__aarch)

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "OSDependent.h"
#include "MsgConvert.h"
#include "MsgConvertDataTypes.h"
#include "Encoder/RINEX/RINEXDefines.h"
#include "Encoder/RINEX/RINEXEncoder.h"
#include "Encoder/RINEX/RINEXConvert.h"
#include "Common/SupportFuncs.h"
#include "GNSSPrint.h"
#include "Common/DataTypes.h"
#include "Decoder/MsgPort.h"
#include "Decoder/MsgDecode.h"
#include "Decoder/HRCX/HRCXFilter.h"
#include "Decoder/RTCM/RTCM3Filter.h"
#include "Decoder/RTCM/RTCM2Filter.h"
#include "Decoder/RTCM/RTCMDecodeManage.h"
#include "Decoder/Rinex/RinexDecoder.h"
#include "Decoder/Rinex/RinexFilter.h"
#include "BSW/MsgDecoder/RTCM/RTCMDataTypes.h"
#include "BSW/MsgDecoder/RTCM/RTCMSupportFunc.h"
#include "Common/CommonFuncs.h"
#include "HAL/DriverCycleDataBuffer.h"
#include "TimeDefines.h"
#include "Common/GNSSNavDataType.h"

#define MAX_ENCODE_DATA_BUF_LEN		 (1024*32)

#define READ_BUF_LEN_MAX			(16*1024)

typedef void(*DoCallback)(void* pSrcObj, unsigned int DataID, void* data);

int g_MsgConvertProgress = 0;

/**********************************************************************//**
@brief  get MsgConvert process progress

@return progress

@author CHC
@date 2025/08/18
@note
**************************************************************************/
int GetMsgConvertProgress()
{
	return g_MsgConvertProgress;
}

/**********************************************************************//**
@brief  set MsgConvert process progress

@param Value   [In] progress value

@author CHC
@date 2025/08/18
@note
**************************************************************************/
void SetMsgConvertProgress(int Value)
{
	g_MsgConvertProgress = Value;
}


static int mkdirs(char* Path)
{
	char Str[512] = {0};
	strncpy(Str, Path, 512);
	size_t Len = strlen(Str);
	for (unsigned int Index = 0; Index < Len; Index++)
	{
		if ((Str[Index] == '\\') || (Str[Index] == '/'))
		{
			Str[Index] = '\0';
			if (access(Str, 0) != 0)
			{
				#ifdef WIN32
				_mkdir(Str);
				#elif __linux
				mkdir(Str, S_IRWXU);
				#endif
			}
			Str[Index] = '\\';
		}
	}
	return 1;
}

int MsgRecv(unsigned int PortAddr, void* pOutputData, void* pSourceData, unsigned int DataSize)
{
	MEMCPY(pOutputData, pSourceData, DataSize);
	return 0;
}

/**********************************************************************//**
@brief  int obs header

@param pObsHeader   [In] pointer to obs header

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int InitObsHeader(RINEX_OBS_HEADER_T* pObsHeader)
{
	if (!pObsHeader)
		return -1;

	pObsHeader->Ver = 304;
	pObsHeader->NCom = 0;
	pObsHeader->TimeInt = 1;
	pObsHeader->NavSys = SYS_MASK_ALL;
	memcpy(pObsHeader->Pgm, "CHC", strlen("CHC") + 1);
	memcpy(pObsHeader->Runby, "CHC", strlen("CHC") + 1);
	memcpy(pObsHeader->MarkerName, "MsgConvert", strlen("MsgConvert") + 1);
	memcpy(pObsHeader->MarkerNo, "20230417", strlen("20230417") + 1);
	memcpy(pObsHeader->Observer, "CHC", strlen("CHC") + 1);
	memcpy(pObsHeader->Agency, "CHC", strlen("CHC") + 1);
	memcpy(pObsHeader->AntDes, "Unknown", strlen("Unknown") + 1);
	memcpy(pObsHeader->AntNo, "Unknown", strlen("Unknown") + 1);
	memcpy(pObsHeader->RecType, "Unknown", strlen("Unknown") + 1);
	memcpy(pObsHeader->RecVer, "Unknown", strlen("Unknown") + 1);
	memcpy(pObsHeader->RecNo, "Unknown", strlen("Unknown") + 1);
	pObsHeader->Leapsecs = 0;

	return 0;
}

/**********************************************************************//**
@brief  init eph header

@param pObsHeader   [In] pointer to eph header

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int InitEphHeader(RINEX_EPH_HEADER_T* pEphHeader)
{
	if (!pEphHeader)
		return -1;

	pEphHeader->Ver = 304;
	pEphHeader->NCom = 0;
	pEphHeader->Sys = SYS_MASK_ALL;
	pEphHeader->Leapsecs = 18;
	memcpy(pEphHeader->Runby, "MsgConvert", strlen("MsgConvert") + 1);
	memcpy(pEphHeader->Pgm, "CHC", strlen("CHC") + 1);

	return 0;
}

/**********************************************************************//**
@brief  init GLO eph header

@param pObsHeader   [In] pointer to GLO eph header

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int InitGLOEphHeader(RINEX_GEPH_HEADER_T* pGEphHeader)
{
	pGEphHeader->Leapsecs = 18;
	memcpy(pGEphHeader->Runby, "MsgConvert", strlen("MsgConvert") + 1);
	memcpy(pGEphHeader->Pgm, "CHC", strlen("CHC") + 1);

	return 0;
}

/**********************************************************************//**
@brief  init obs option

@param pObsOpt   [In] pointer to obs option

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int InitObsOpt(RINEX_OBS_OPT_T* pObsOpt)
{
	pObsOpt->Ver = 304;  /**< 211/302/303/304/305 */
	pObsOpt->NavSys = SYS_MASK_ALL;
	pObsOpt->Leaps = 18;
	pObsOpt->CrxFlag = 1;

	return 0;
}

/**********************************************************************//**
@brief  init eph option

@param pObsOpt   [In] pointer to eph option

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int InitEphOpt(RINEX_EPH_OPT_T* pEphOpt)
{
	pEphOpt->Ver = 304;
	pEphOpt->NavSys = SYS_MASK_ALL;
	pEphOpt->Iono = 0;
	pEphOpt->Leapsecs = 18;
	pEphOpt->Time = 1;

	return 0;
}

/**********************************************************************//**
@brief  get oem type

@param ver   [In] version

@retval oem type

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int GetOemType(int ver)
{
	int32_t OemType = -1;

	if (ver == 85) OemType = GNSS_STRFMT_RTCM3;
	else if (ver == 94) OemType = GNSS_STRFMT_RTCM2;
	else if (ver == 106) OemType = GNSS_STRFMT_HRCX;
	return OemType;
}

/**********************************************************************//**
@brief  get raw obs data type string

@param RawObasDataType		[In] raw obs data type

@retval <0 error, >=0 success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
const char* GetRawObsDataTypeString(unsigned int RawObasDataType)
{
	static const char* s_RawObsDataTypeStr[RAW_OBS_DATA_TYPE_NUM_MAX] = {
		"_FromRTCM3_MSM",
		"_FromRTCM3_RTKDATA",
		"_FromRTCM2",
		"",
		"",
		"_FromRANGE_1_ASCII",
		"_FromRANGE_1_BINARY",
		"_FromRANGECMP_ASCII",
		"_FromRANGECMP_BINARY",
		"_FromRANGECMP_1_ASCII",
		"_FromRANGECMP_1_BINARY",
		"_FromBASEOBS_ASCII",
		"_FromBASEOBS_BINARY",
		"",
		"",
		"_FromGNSSOBS_1_ASCII",
		"_FromGNSSOBS_1_BINARY",
		"_FromGNSSOBSCMP_ASCII",
		"_FromGNSSOBSCMP_BINARY",
		"_FromGNSSOBSCMP_1_ASCII",
		"_FromGNSSOBSCMP_1_BINARY",
		"_FromHRCX",
		"_FromRINEX",
	};

	if (RawObasDataType < RAW_OBS_DATA_TYPE_NUM_MAX)
	{
		return s_RawObsDataTypeStr[RawObasDataType];
	}

	return NULL;
}

/**********************************************************************//**
@brief  set system obs type

@param pStr		 [In] string
@param TObsSys	  [In] system obs type

@retval charter num

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SetSysTObs(char* pStr, char TObsSys[][4])
{
	int Num = 0, Index = 0, j = 0;

	Num = atoi(&pStr[0]);

	for (Index = 0; Index < Num; Index++)
	{
		TObsSys[Index][0] = 'C';
		TObsSys[Index + Num][0] = 'L';
		TObsSys[Index + Num * 2][0] = 'S';
		TObsSys[Index + Num * 3][0] = 'D';

		for (j = 0; j < 4; j++)
		{
			TObsSys[Index + Num * j][1] = pStr[3 + Index * 4];
			TObsSys[Index + Num * j][2] = pStr[4 + Index * 4];
		}
	}

	return Num * 4;
}

/**********************************************************************//**
@brief  set system one obs type

@param StartIndex   [In] start index
@param Num		  [In] num
@param pStr		 [In] string
@param TObsSys	  [In] system obs type

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int SetSysOneTObs(int StartIndex, int Num, char* pStr, char TObsSys[][4])
{
	int Index = 0;

	TObsSys[StartIndex][0] = 'C';
	TObsSys[StartIndex + Num][0] = 'L';
	TObsSys[StartIndex + Num * 2][0] = 'S';
	TObsSys[StartIndex + Num * 3][0] = 'D';

	for (Index = 0; Index < 4; Index++)
	{
		TObsSys[StartIndex + Num * Index][1] = pStr[1];
		TObsSys[StartIndex + Num * Index][2] = pStr[2];
	}

	return 0;
}

/**********************************************************************//**
@brief  reset system obs type

@param pMsgConvertData   [In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int ResetSingleObsDataSysTObs(SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData)
{
	int Index = 0, SigTypeIndex = 0, ObsIndex = 0, SysNum = 0, OptIndex = -1;
	char* pSysChar;

	MEMSET(pSingleObsConvertData->RINEXObsOpt.ObsTypeBuf, 0, sizeof(pSingleObsConvertData->RINEXObsOpt.ObsTypeBuf));
	MEMSET(pSingleObsConvertData->RINEXObsOpt.NObs, 0, sizeof(pSingleObsConvertData->RINEXObsOpt.NObs));

	for (Index = 0; Index < TOTAL_EX_GNSS_SYSTEM_NUM; Index++)
	{

		OptIndex = ExSystemToRinexSysIndex(Index);
		if (NA8 == OptIndex)
		{
			continue;
		}

		SysNum = 0;
		for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
		{
			if (pSingleObsConvertData->SysSigType[Index][SigTypeIndex] == NA32)
			{
				break;
			}
			SysNum++;
		}
		/** The number of frequency points (SysNum) of satellite system ID (i+1) is counted */
		if (SysNum > 0)
		{
			for (ObsIndex = 0; ObsIndex < MAX_NUM_OBS_TYPE; ObsIndex++)
			{
				for (SigTypeIndex = 0; SigTypeIndex < 4; SigTypeIndex++)
				{
					pSingleObsConvertData->RINEXObsOpt.ObsTypeBuf[OptIndex][ObsIndex][SigTypeIndex] = 0;
				}
			}

			pSingleObsConvertData->RINEXObsOpt.NObs[OptIndex] = SysNum * 4;
			for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
			{
				if (pSingleObsConvertData->SysSigType[Index][SigTypeIndex] == NA32)
				{
					break;
				}

				pSysChar = (char*)GetObsSigCharByCode(pSingleObsConvertData->SysSigType[Index][SigTypeIndex]);
				if (NULL == pSysChar)
				{
					continue;
				}
				SetSysOneTObs(SigTypeIndex, SysNum, pSysChar, pSingleObsConvertData->RINEXObsOpt.ObsTypeBuf[OptIndex]);
			}
		}
	}

	return 0;
}

int ResetSysTObs(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	int Result = -1;
	unsigned int DataTypeIndex;

	if (pMsgConvertData->RawObsDataType == 0)
	{
		Result = ResetSingleObsDataSysTObs(&pMsgConvertData->GlobalObsConvertData);
	}
	else
	{
		for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
		{
			if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
			{
				Result = ResetSingleObsDataSysTObs(&pMsgConvertData->SingleObsConvertData[DataTypeIndex]);
				if (Result < 0)
				{
					return Result;
				}
			}
		}
	}

	return Result;
}

/**********************************************************************//**
@brief  output system obs type

@param pMsgConvertData   [In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int OutputSysSigType(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	int Index, SigTypeIndex, Len = 0, SysNum = 0;
	char SysChar[TOTAL_EX_GNSS_SYSTEM_NUM][1024] = { 0 };
	const char* pStr;

	GNSSPrintf("All sys sig type:\n");

	for (Index = 0; Index < TOTAL_EX_GNSS_SYSTEM_NUM; Index++)
	{
		switch (Index)
		{
			case GPS: SysChar[Index][0] = 'G'; break;
			case SBA: SysChar[Index][0] = 'S'; break;
			case GLO: SysChar[Index][0] = 'R'; break;
			case GAL: SysChar[Index][0] = 'E'; break;
			case QZS: SysChar[Index][0] = 'J'; break;
			case BDS: SysChar[Index][0] = 'C'; break;
			case NIC: SysChar[Index][0] = 'I'; break;
			default:break;
		}
		SysNum = 0;
		for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
		{
			if (pMsgConvertData->SysSigType[Index][SigTypeIndex] == NA32)
			{
				break;
			}
			SysNum++;
		}
		sprintf(SysChar[Index] + 1, "=%d", SysNum);

		Len = 3;
		for (SigTypeIndex = 0; SigTypeIndex < MAXSIGTYPE; SigTypeIndex++)
		{
			if (pMsgConvertData->SysSigType[Index][SigTypeIndex] == NA32)
			{
				break;
			}

			pStr = GetObsSigCharByCode(pMsgConvertData->SysSigType[Index][SigTypeIndex]);
			if (NULL == pStr)
			{
				continue;
			}
			sprintf(SysChar[Index] + Len, ",%s", pStr);
			Len += 4;
		}
		if (SysNum > 0)
		{
			GNSSPrintf(SysChar[Index]);
			GNSSPrintf("\n");
		}
	}
	GNSSPrintf("\n");

	return 0;
}

/**********************************************************************//**
@brief  init output file

@param FileType				[In] file type
@param pMsgConvertData	 	[In] pointer to message convert data
@param pFileName			[In] file name
@param pOutDir				[In] output dir

@retval <0:error, =0:success

@author CHC
@date 2023/12/15
@note
**************************************************************************/
int InitOutputFile(unsigned int FileType, MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir)
{
	char ObsFileName[512] = { 0 }, pTempFileName[512] = { 0 };
	char PureName[128] = { 0 };
	const char* FileNameBuf[OUTPUT_FILE_TYPE_NUM] = {"_RawMsg.dat", "_BaseObs.dat", "_BaseObsSatCount.dat", "_DebugInfo.dat", "_RTCM3.dat", "_RTKPlot.dat"};
	int Index, Len, Flag, Pos;
	const char* BaseObsSatCountFileHeaderStr = "#GPS Week,\tGPS TOW,\tGPS SatNum,\tGLO SatNum,\tGAL SatNum,\tSBA SatNum,\tQZS SatNum,\tBDS SatNum,\tNIC SatNum\r\n";
	const char* RTKPlotFileHeaderStr = "%\r\n"
									"% (x/y/z-ecef=WGS84,Q=1:fix,2:float,3:sbas,4:dgps,5:single,6:ppp,ns=# of satellites)\r\n"
									"%  GPST                      x-ecef(m)      y-ecef(m)      z-ecef(m)   Q  ns   sdx(m)   sdy(m)   sdz(m)  sdxy(m)  sdyz(m)  sdzx(m) age(s)  ratio\r\n";
	const char* pOutputFileName;

	if (FileType >= OUTPUT_FILE_TYPE_NUM)
	{
		return -1;
	}

	pOutputFileName = FileNameBuf[FileType];

	if ((int)strlen(pOutDir) > 1)
	{
		mkdirs(pOutDir);//create dir
	}
	else
	{
		Len = (int)strlen(pFileName);
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (pFileName[Index] == '\\' || pFileName[Index] == '/')
			{
				strncpy(pOutDir, pFileName, Index + 1);
				break;
			}
		}
	}

	if (pFileName != NULL)
	{
		Len = (int)strlen(pFileName);
		Flag = 0;
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (pFileName[Index] == '.' && !Flag)
			{
				Pos = Index;
				Flag = 1;
			}
			if ((pFileName[Index] == '\\' || pFileName[Index] == '/') && Flag)
			{
				strncpy(PureName, pFileName + Index + 1, Pos - Index - 1);
				break;
			}
			if ((pFileName[Index] == '\\' || pFileName[Index] == '/') && !Flag)
			{
				strncpy(PureName, pFileName + Index + 1, Len - Index - 1);
				break;
			}
		}
		if (('\0' == PureName[0]) && Flag)
		{
			strncpy(PureName, pFileName + Index + 1, Pos - Index - 1);
		}
		strncpy(pMsgConvertData->OutDir, pOutDir, (int)strlen(pOutDir));
		strncpy(pMsgConvertData->PureName, PureName, (int)strlen(PureName));
		Len = (int)strlen(pOutDir);
		strncpy(ObsFileName, pOutDir, Len);
		strncpy(pTempFileName, pOutDir, Len);
		sprintf(ObsFileName + Len, "%s", PureName);
		sprintf(pTempFileName + Len, "%s", PureName);
		Len = (int)strlen(ObsFileName);
		sprintf(pTempFileName + Len, "%s", pOutputFileName);

		if ((FileType == OUTPUT_FILE_TYPE_RAWMSGA) || (FileType == OUTPUT_FILE_TYPE_BASEOBSA))
		{
			pMsgConvertData->FpRawMsgOutP = fopen(pTempFileName, "wb+");
		}
		else if (FileType == OUTPUT_FILE_TYPE_BASEOBSSATCOUNT)
		{
			pMsgConvertData->FpBaseObsSatCountOutP = fopen(pTempFileName, "wb+");

			if (pMsgConvertData->FpBaseObsSatCountOutP != NULL)
			{
				fwrite(BaseObsSatCountFileHeaderStr, strlen(BaseObsSatCountFileHeaderStr), 1, pMsgConvertData->FpBaseObsSatCountOutP);
				fflush(pMsgConvertData->FpBaseObsSatCountOutP);
			}
		}
		else if (FileType == OUTPUT_FILE_TYPE_DEBUGINFO)
		{
			pMsgConvertData->FpDebugInfoOutP = fopen(pTempFileName, "wb+");
		}
		else if (FileType == OUTPUT_FILE_TYPE_RTCM)
		{
			pMsgConvertData->FpRTCMOutP = fopen(pTempFileName, "wb+");
		}
		else if (FileType == OUTPUT_FILE_TYPE_RTKPLOT)
		{
			if (pMsgConvertData->FpRTKPlotOutPFlag)
			{
				pMsgConvertData->FpRTKPlotOutP = fopen(pTempFileName, "wb+");

				if (pMsgConvertData->FpRTKPlotOutP != NULL)
				{
					fwrite(RTKPlotFileHeaderStr, strlen(RTKPlotFileHeaderStr), 1, pMsgConvertData->FpRTKPlotOutP);
					fflush(pMsgConvertData->FpRTKPlotOutP);
				}
			}
		}
	}
	return 1;
}

/**********************************************************************//**
@brief  init rinex input file

@param pMsgConvertData	 	[In] pointer to message convert data
@param pFileName			[In] raw file path

@retval <0:error, =0:success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
int InitRinexInputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pRawPath)
{
	unsigned int TempFileNameLen;
	char TempFilePath[512] = {0};
	char FileTypeBuf[RINEX_FILE_INDEX_NUM * 2] = {'O', 'o', 'P', 'p', 'N', 'n', 'G', 'g', 'L', 'l', 'C', 'c', 'S', 's', 'Q', 'q' ,'I', 'i'};
	unsigned int FileTypeIndex;
	unsigned int FileType = 0xFFFFFFFF;
	char* FileErrStr[RINEX_FILE_INDEX_NUM] = {"Obs", "Muti Eph", "GPS Eph", "GLO Eph","GAL Eph", "BDS Eph", "SBAS Eph", "QZSS Eph", "NAVIC Eph"};

	TempFileNameLen = (unsigned int)strlen(pRawPath) - 1;
	MEMCPY(TempFilePath, pRawPath, TempFileNameLen);

	for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
	{
		if ((pRawPath[TempFileNameLen] == FileTypeBuf[FileTypeIndex * 2]) || (pRawPath[TempFileNameLen] == FileTypeBuf[FileTypeIndex * 2 + 1]))
		{
			FileType = FileTypeIndex;
			break;
		}
	}

	if (FileTypeIndex >= RINEX_FILE_INDEX_NUM)
	{
		GNSSPrintf("Input Rinex file type error!\n");
		return -1;
	}

	for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
	{
		sprintf(TempFilePath + TempFileNameLen, "%1c", FileTypeBuf[FileTypeIndex * 2]);

		pMsgConvertData->RinexFileFp[FileTypeIndex] = fopen(TempFilePath, "rb+");
		if (pMsgConvertData->RinexFileFp[FileTypeIndex] == NULL)
		{
			sprintf(TempFilePath + TempFileNameLen, "%1c", FileTypeBuf[FileTypeIndex * 2 + 1]);

			pMsgConvertData->RinexFileFp[FileTypeIndex] = fopen(TempFilePath, "rb+");
			if (pMsgConvertData->RinexFileFp[FileTypeIndex] == NULL)
			{
				if (FileTypeIndex == RINEX_FILE_INDEX_OBS)
				{
					GNSSPrintf("Rinex obs file open fail!\n");
					return -1;
				}
				else
				{
					GNSSPrintf("Rinex %s file open fail!\n", FileErrStr[FileTypeIndex]);
				}
			}
			else
			{
				pMsgConvertData->RinexFileFpFlag[FileTypeIndex] = 1;

				if (FileTypeIndex == RINEX_FILE_INDEX_MUTIEPH)
				{
					return 0;
				}
			}
		}
		else
		{
			pMsgConvertData->RinexFileFpFlag[FileTypeIndex] = 1;

			if (FileTypeIndex == RINEX_FILE_INDEX_MUTIEPH)
			{
				return 0;
			}
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  init rinex input file

@param pMsgConvertData	 	[In] pointer to message convert data
@param pFileName			[In] raw file path

@retval <0:error, =0:success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
int InitRinexMultiInputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pRawPath)
{
	char TempFilePath[4096] = {0};
	unsigned int TempFileNameLen;
	char FilePath[512];
	unsigned int FileNameLen = 0;
	char* pBuf;
	char* pStr = NULL;
	char FileTypeBuf[RINEX_FILE_INDEX_NUM * 2] = {'O', 'o', 'P', 'p', 'N', 'n', 'G', 'g', 'L', 'l', 'C', 'c', 'S', 's', 'Q', 'q' ,'I', 'i'};
	unsigned int FileTypeIndex;
	unsigned int FileGroupIndex = 0;
	unsigned int FileType = 0xFFFFFFFF;
	char* FileErrStr[RINEX_FILE_INDEX_NUM] = {"Obs", "Muti Eph", "GPS Eph", "GLO Eph","GAL Eph", "BDS Eph", "SBAS Eph", "QZSS Eph", "NAVIC Eph"};
	BOOL InitFlag = TRUE;

	TempFileNameLen = (unsigned int)strlen(pRawPath);
	MEMCPY(TempFilePath, pRawPath, TempFileNameLen);
	pBuf = &TempFilePath[0];

	pStr = strchr(pBuf, ',');
	if (pStr == NULL)
	{
		GNSSPrintf("There is only one rinex file, please check the number of input files!\n");
		return -1;
	}

	while(InitFlag)
	{
		MEMSET(FilePath, 0, sizeof(FilePath));
		pStr = NULL;

		pStr = strchr(pBuf, ',');
		if (pStr != NULL)
		{
			FileNameLen = (int)(pStr - pBuf);
			MEMCPY(FilePath, pBuf, FileNameLen);
		}
		else
		{
			FileNameLen = (int)(&TempFilePath[TempFileNameLen - 1] - pBuf + 1);
			MEMCPY(FilePath, pBuf, FileNameLen); /**< last file */
			InitFlag = FALSE;
		}
		pBuf = pStr;
		pBuf++;

		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			if ((FilePath[FileNameLen-1] == FileTypeBuf[FileTypeIndex * 2]) || (FilePath[FileNameLen-1] == FileTypeBuf[FileTypeIndex * 2 + 1]))
			{
				FileType = FileTypeIndex;
				break;
			}
		}

		if (FileTypeIndex >= RINEX_FILE_INDEX_NUM)
		{
			GNSSPrintf("Input Rinex file type error!\n");
			return -1;
		}

		for (FileTypeIndex = 0; FileTypeIndex < RINEX_FILE_INDEX_NUM; FileTypeIndex++)
		{
			sprintf(FilePath + FileNameLen - 1, "%1c", FileTypeBuf[FileTypeIndex * 2]);

			pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex] = fopen(FilePath, "rb+");
			if (pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex] == NULL)
			{
				sprintf(FilePath + FileNameLen - 1, "%1c", FileTypeBuf[FileTypeIndex * 2 + 1]);

				pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex] = fopen(FilePath, "rb+");
				if (pMsgConvertData->RinexMultiFileFp[FileGroupIndex][FileTypeIndex] == NULL)
				{
					if (FileTypeIndex == RINEX_FILE_INDEX_OBS)
					{
						GNSSPrintf("Rinex obs file[%s] open fail!\n", FilePath);
						return -1;
					}
					else
					{
						GNSSPrintf("Rinex %s file[%s] open fail!\n", FileErrStr[FileTypeIndex], FilePath);
					}
				}
				else
				{
					pMsgConvertData->RinexMultiFileFpFlag[FileGroupIndex][FileTypeIndex] = 1;

					if (FileTypeIndex == RINEX_FILE_INDEX_MUTIEPH)
					{
						break;
					}
				}
			}
			else
			{
				pMsgConvertData->RinexMultiFileFpFlag[FileGroupIndex][FileTypeIndex] = 1;

				if (FileTypeIndex == RINEX_FILE_INDEX_MUTIEPH)
				{
					break;
				}
			}
		}

		FileGroupIndex++;
	}

	pMsgConvertData->RinexMultiFileNum = FileGroupIndex;

	return 0;
}

/**********************************************************************//**
@brief  init rinex output file

@param pMsgConvertData	 	[In] pointer to message convert data
@param pFileName			[In] file name
@param pOutDir				[In] output dir

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int InitSingleObsRINEXOutputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir,
								SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData)
{
	char ObsFileName[512] = { 0 }, pTempFileName[512] = { 0 };
	char PureName[128] = { 0 };
	const char m_dix[] = { 'p','n','g','l' ,'q' ,'c' ,'s','i' };
	int Year = 0, Index, Len, Flag, Pos;
	UTC_TIME_T TempUTCTime;

	if ((int)strlen(pOutDir) > 1)
	{
		mkdirs(pOutDir);//create dir
	}
	else
	{
		Len = (int)strlen(pFileName);
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (pFileName[Index] == '\\' || pFileName[Index] == '/')
			{
				strncpy(pOutDir, pFileName, Index + 1);
				break;
			}
		}
	}

	if (pFileName != NULL)
	{
		Len = (int)strlen(pFileName);
		Flag = 0;
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (pFileName[Index] == '.' && !Flag)
			{
				Pos = Index;
				Flag = 1;
			}
			if ((pFileName[Index] == '\\' || pFileName[Index] == '/') && Flag)
			{
				strncpy(PureName, pFileName + Index + 1, Pos - Index - 1);
				break;
			}
			if ((pFileName[Index] == '\\' || pFileName[Index] == '/') && !Flag)
			{
				strncpy(PureName, pFileName + Index + 1, Len - Index - 1);
				break;
			}
		}
		if (('\0' == PureName[0]) && Flag)
		{
			strncpy(PureName, pFileName + Index + 1, Pos - Index - 1);
		}
		strncpy(pMsgConvertData->OutDir, pOutDir, (int)strlen(pOutDir));
		strncpy(pMsgConvertData->PureName, PureName, (int)strlen(PureName));
		Len = (int)strlen(pOutDir);
		strncpy(ObsFileName, pOutDir, Len);
		strncpy(pTempFileName, pOutDir, Len);
		sprintf(ObsFileName + Len, "%s", PureName);
		sprintf(pTempFileName + Len, "%s", PureName);
		Len = (int)strlen(ObsFileName);

		if (pSingleObsConvertData->RINEXObsHeader.TimeStart.Year != 0)
		{
			TempUTCTime = pSingleObsConvertData->RINEXObsHeader.TimeStart;
		}
		else
		{
			TempUTCTime = RTCMGetUTCTime();
		}

		Year = (int)TempUTCTime.Year % 100;
		if (pSingleObsConvertData->RINEXEphHeader.Ver >= 300)
		{
			sprintf(pTempFileName + Len, ".%02d%1c", Year, m_dix[0]);
			pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH] = fopen(pTempFileName, "wb+");
		}
		for (Index = MSG_CONVERT_FILE_INDEX_GPSEPH; Index <= MSG_CONVERT_FILE_INDEX_NICEPH; Index++)
		{
			/** Select the navigation ephemeris file to be created according to the frequency information */
			if (pSingleObsConvertData->RinexNavFileFpFlag[Index] != 0)
			{
				sprintf(pTempFileName + Len, ".%02d%1c", Year, m_dix[Index]);
				pSingleObsConvertData->RinexNavFileFp[Index] = fopen(pTempFileName, "wb+");
			}
		}
	}
	return 1;
}

/**********************************************************************//**
@brief  init rinex multi output file

@param pMsgConvertData	 	[In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int InitRinexMultiOutputFile(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	char ObsFileName[512] = { 0 }, pTempFileName[512] = { 0 };
	const char m_dix[] = { 'p','n','g','l' ,'q' ,'c' ,'s','i' };
	int Year = 0, Index, Len;
	UTC_TIME_T TempUTCTime;
	char* pOutDir = pMsgConvertData->OutDir;
	char* pFileBaseName = "RinexFileCombined";
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;

	if ((int)strlen(pOutDir) > 1)
	{
		mkdirs(pOutDir);//create dir
	}

	if (pFileBaseName != NULL)
	{
		strncpy(pMsgConvertData->PureName, pFileBaseName, (int)strlen(pFileBaseName));
		Len = (int)strlen(pOutDir);
		strncpy(ObsFileName, pOutDir, Len);
		strncpy(pTempFileName, pOutDir, Len);
		sprintf(ObsFileName + Len, "%s", pFileBaseName);
		sprintf(pTempFileName + Len, "%s", pFileBaseName);
		Len = (int)strlen(ObsFileName);

		if (pRinexMultiConvertData->RINEXObsHeader.TimeStart.Year != 0)
		{
			TempUTCTime = pRinexMultiConvertData->RINEXObsHeader.TimeStart;
		}
		else
		{
			TempUTCTime = RTCMGetUTCTime();
		}

		Year = (int)TempUTCTime.Year % 100;
		if (pRinexMultiConvertData->RINEXEphHeader.Ver >= 300)
		{
			sprintf(pTempFileName + Len, ".%02d%1c", Year, m_dix[0]);
			pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH] = fopen(pTempFileName, "wb+");
		}

		for (Index = MSG_CONVERT_FILE_INDEX_GPSEPH; Index <= MSG_CONVERT_FILE_INDEX_NICEPH; Index++)
		{
			/** Select the navigation ephemeris file to be created according to the frequency information */
			if (pRinexMultiConvertData->RinexNavFileFpFlag[Index] != 0)
			{
				sprintf(pTempFileName + Len, ".%02d%1c", Year, m_dix[Index]);
				pRinexMultiConvertData->RinexNavFileFp[Index] = fopen(pTempFileName, "wb+");
			}
		}
	}

	return 1;
}

/**********************************************************************//**
@brief  init rinex output file

@param pMsgConvertData	 	[In] pointer to message convert data
@param pFileName			[In] file name
@param pOutDir				[In] output dir

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int InitRinexMultiOutputDir(MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir)
{
	char PureName[128] = { 0 };
	int Index, Len, Flag, Pos;
	char* pStr;
	char FilePath[512];
	unsigned int FileNameLen = 0;
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;

	InitEphHeader(&pRinexMultiConvertData->RINEXEphHeader);
	InitGLOEphHeader(&pRinexMultiConvertData->RINEXGEphHeader);
	InitEphOpt(&pRinexMultiConvertData->RINEXEphOpt);

	InitObsHeader(&pRinexMultiConvertData->RINEXObsHeader);
	InitObsOpt(&pRinexMultiConvertData->RINEXObsOpt);

	pStr = strchr(pFileName, ',');
	if (pStr == NULL)
	{
		GNSSPrintf("There is only one rinex file, please check the number of input files!\n");
		return -1;
	}

	FileNameLen = (int)(pStr - pFileName);
	MEMCPY(FilePath, pFileName, FileNameLen);
	FilePath[FileNameLen] = '\0';

	if ((int)strlen(pOutDir) > 1)
	{
		mkdirs(pOutDir);//create dir
	}
	else
	{
		Len = (int)strlen(FilePath);
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (FilePath[Index] == '\\' || FilePath[Index] == '/')
			{
				strncpy(pOutDir, FilePath, Index + 1);
				break;
			}
		}
	}

	if (FilePath != NULL)
	{
		Len = (int)strlen(FilePath);
		Flag = 0;
		for (Index = Len - 1; Index >= 0; Index--)
		{
			if (FilePath[Index] == '.' && !Flag)
			{
				Pos = Index;
				Flag = 1;
			}
			if ((FilePath[Index] == '\\' || FilePath[Index] == '/') && Flag)
			{
				strncpy(PureName, FilePath + Index + 1, Pos - Index - 1);
				break;
			}
			if ((FilePath[Index] == '\\' || FilePath[Index] == '/') && !Flag)
			{
				strncpy(PureName, FilePath + Index + 1, Len - Index - 1);
				break;
			}
		}
		if (('\0' == PureName[0]) && Flag)
		{
			strncpy(PureName, FilePath + Index + 1, Pos - Index - 1);
		}
		strncpy(pMsgConvertData->OutDir, pOutDir, (int)strlen(pOutDir));
	}
	return 1;
}

int InitRINEXOutputFile(MSG_CONVERT_DATA_T* pMsgConvertData, char* pFileName, char* pOutDir)
{
	unsigned int DataTypeIndex;
	
	if (pMsgConvertData->RawObsDataType == 0)
	{
		InitSingleObsRINEXOutputFile(pMsgConvertData, pFileName, pOutDir, &pMsgConvertData->GlobalObsConvertData);
	}
	else
	{
		for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
		{
			if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
			{
				InitSingleObsRINEXOutputFile(pMsgConvertData, pFileName, pOutDir, &pMsgConvertData->SingleObsConvertData[DataTypeIndex]);
			}
		}
	}

	return 1;

}

/**********************************************************************//**
@brief  parse hcn header data

@param pMsgConvertData	  [In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int ParseHCNHeaderData(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	int Type = -1;
	int HCNVer = -1;
	unsigned char Buff[512] = "";

	if (pMsgConvertData == NULL)
		return -1;

	while (1)
	{
		fgets(Buff, 512, pMsgConvertData->RawFp);
		if (strlen(Buff) < 1)
			break;
		if (strstr(Buff, "HUACENAV COLLECTED DATA FILE"))
		{
			fgets(Buff, 512, pMsgConvertData->RawFp);
			HCNVer = (int)atof(Buff + 4);
			continue;
		}
		if (strstr(Buff, "s_ReceiverID"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.RecNo, Buff + 11, strlen(Buff) - 11 - 2);
			continue;
		}
		if (strstr(Buff, "Model"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.MarkerType, Buff + 6, strlen(Buff) - 6 - 2);
			continue;
		}
		if (strstr(Buff, "AntHigh"))
		{
			pMsgConvertData->RINEXObsHeader.Del[0] = atof(Buff + 8);
			continue;
		}
		if (strstr(Buff, "AntType"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.AntDes, Buff + 8, strlen(Buff) - 8 - 2);
			continue;
		}
		if (Buff[0] == 'X' && Buff[1] == ':')
		{
			pMsgConvertData->RINEXObsHeader.Pos[0] = atof(Buff + 2);
			continue;
		}
		if (Buff[0] == 'Y' && Buff[1] == ':')
		{
			pMsgConvertData->RINEXObsHeader.Pos[1] = atof(Buff + 2);
			continue;
		}
		if (Buff[0] == 'Z' && Buff[1] == ':')
		{
			pMsgConvertData->RINEXObsHeader.Pos[2] = atof(Buff + 2);
			continue;
		}
		if (strstr(Buff, "MARKER NAME"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.MarkerName, Buff + 12, strlen(Buff) - 12 - 2);
			continue;
		}
		if (strstr(Buff, "MARKER NUMBER"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.MarkerNo, Buff + 14, strlen(Buff) - 14 - 2);
			continue;
		}
		if (strstr(Buff, "OBSERVER"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.Observer, Buff + 9, strlen(Buff) - 9 - 2);
			continue;
		}
		if (strstr(Buff, "AGENCY"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.Agency, Buff + 7, strlen(Buff) - 7 - 2);
			continue;
		}
		if (strstr(Buff, "REC #:"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.RecNo, Buff + 6, strlen(Buff) - 6 - 2);
			continue;
		}
		if (strstr(Buff, "REC TYPE:"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.RecType, Buff + 9, strlen(Buff) - 9 - 2);
			continue;
		}
		if (strstr(Buff, "REC VERS:"))
		{
			strncpy(pMsgConvertData->RINEXObsHeader.RecVer, Buff + 9, strlen(Buff) - 9 - 2);
			continue;
		}
		if (strstr(Buff, "ANT #:"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.AntNo, Buff + 6, strlen(Buff) - 6 - 2);
			continue;
		}
		if (strstr(Buff, "ANT TYPE:"))
		{
			MEMCPY(pMsgConvertData->RINEXObsHeader.AntDes, Buff + 9, strlen(Buff) - 9 - 2);
			continue;
		}
		if (strstr(Buff, "INTERVAL:"))
		{
			pMsgConvertData->RINEXObsHeader.TimeInt = atof(Buff + 9);
			continue;
		}
	}

	fseek(pMsgConvertData->RawFp, 0, SEEK_SET);
	Type = GetOemType(HCNVer);

	return Type;
}

/**********************************************************************//**
@brief  get decode type

@param Type	  [In] type

@retval decode type

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int GetDecodeType(int Type)
{
	int DecodeType = -1;

	switch (Type)
	{
		case GNSS_STRFMT_RTCM2:
			DecodeType = MSG_DECODE_TYPE_RTCM2;
			break;

		case GNSS_STRFMT_RTCM3:
			DecodeType = MSG_DECODE_TYPE_RTCM3;
			break;

		case GNSS_STRFMT_RINEX:
			DecodeType = MSG_DECODE_TYPE_RINEX;
			break;

		case GNSS_STRFMT_HRCX:
			DecodeType = MSG_DECODE_TYPE_HRCX;
			break;

		case GNSS_STRFMT_RAWMSGA:
		case GNSS_STRFMT_BASEOBSA:
		case GNSS_STRFMT_KMD_RAWMSGA:
			DecodeType = MSG_DECODE_TYPE_RAWMSGA;
			break;

		case GNSS_STRFMT_RAWMSGB:
		case GNSS_STRFMT_BASEOBSB:
		case GNSS_STRFMT_KMD_RAWMSGB:
			DecodeType = MSG_DECODE_TYPE_RAWMSGB | MSG_DECODE_TYPE_CHCRAWMSGB | MSG_DECODE_TYPE_UNRAWMSGB;
			break;

		case GNSS_STRFMT_CHCRAWMSGB:
			DecodeType = MSG_DECODE_TYPE_RAWMSGB | MSG_DECODE_TYPE_CHCRAWMSGB | MSG_DECODE_TYPE_UNRAWMSGB;
			break;

		case GNSS_STRFMT_UNRAWMSGB:
			DecodeType = MSG_DECODE_TYPE_RAWMSGB | MSG_DECODE_TYPE_CHCRAWMSGB | MSG_DECODE_TYPE_UNRAWMSGB;
			break;

		case GNSS_STRFMT_RTKLOGB:
			DecodeType = MSG_DECODE_TYPE_RTKLOGB;
			break;

		default:
			break;
	}

	return DecodeType;
}

/**********************************************************************//**
@brief  judge same eph

@param pMsgConvertData	  [In] pointer to message convert data
@param buf				  [In] time buf

@retval decode type

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int JudgeSameEPH(MSG_CONVERT_DATA_T* pMsgConvertData, unsigned char* buf)
{
	int Index;

	for (Index = 0; Index < 23; Index++)
	{
		pMsgConvertData->SatTime[Index] = buf[Index];
	}
	pMsgConvertData->SatTime[23] = '\0';

	for (Index = 0; Index < pMsgConvertData->EPHSameIndex; Index++)
	{
		if (strcmp(pMsgConvertData->SatTime, pMsgConvertData->EPHSameFlag[Index]) == 0)
		{
			return 1;
		}
	}

	strcpy(pMsgConvertData->EPHSameFlag[pMsgConvertData->EPHSameIndex++], pMsgConvertData->SatTime);

	return 0;
}

/**********************************************************************//**
@brief  get rinex version by rinex id

@param RINEXID	  [In] rinex id
@param EncodeID	 [In] encode id

@retval rinex version

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int GetRINEXVerByRINEXID(unsigned int RINEXID, unsigned int* EncodeID)
{
	int RINEXVer = 0;

	switch (RINEXID % 10)
	{
		case 0: RINEXVer = 211; break;
		case 1: RINEXVer = 302; break;
		case 2: RINEXVer = 304; break;
		case 3: RINEXVer = 305; break;
		default: break;
	}

	*EncodeID = (int)(RINEXID / 10);

	return RINEXVer;
}

/**********************************************************************//**
@brief  rinex encode call back

@param pSrcObj	  [In] pointer to source object
@param DataID	   [In] data id
@param data		 [In] data source

@author CHC
@date 2023/05/17
@note
**************************************************************************/
void RINEXEncodeCallback(void* pSrcObj, unsigned int DataID, void* data)
{
	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData = pSrcObj;
	int RinexVer, EncodeID = 0;
	RinexVer = GetRINEXVerByRINEXID(DataID, &EncodeID);

	switch (EncodeID)
	{
		case RINEX_ID_OBS_CRX_HEADER:
		case RINEX_ID_OBS_HEADER:
		{
			fwrite(((MSG_BUF_T*)data)->pBaseAddr, ((MSG_BUF_T*)data)->Len, 1, pSingleObsConvertData->RinexObsFileFp);
			fflush(pSingleObsConvertData->RinexObsFileFp);
			break;
		}
		case RINEX_ID_MUTIEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GPSEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GPSEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GPSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GLOEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GALEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GALEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GALEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_QZSSEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_QZSSEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_QZSSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_BDSEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_BDSEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_BDSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_SBASEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_SBASEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_SBASEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_NICEPH_HEADER:
		{
			if (pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_NICEPH])
			{
				fprintf(pSingleObsConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_NICEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		default:
			break;
	}
}

/**********************************************************************//**
@brief  rinex encode call back

@param pSrcObj	  [In] pointer to source object
@param DataID	   [In] data id
@param data		 [In] data source

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static void RinexMultiEncodeCallback(void* pSrcObj, unsigned int DataID, void* data)
{
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = pSrcObj;
	int RinexVer, EncodeID = 0;
	RinexVer = GetRINEXVerByRINEXID(DataID, &EncodeID);

	switch (EncodeID)
	{
		case RINEX_ID_OBS_CRX_HEADER:
		case RINEX_ID_OBS_HEADER:
		{
			fwrite(((MSG_BUF_T*)data)->pBaseAddr, ((MSG_BUF_T*)data)->Len, 1, pRinexMultiConvertData->RinexObsFileFp);
			fflush(pRinexMultiConvertData->RinexObsFileFp);
			break;
		}
		case RINEX_ID_MUTIEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_MUTIEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GPSEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GPSEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GPSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GLOEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GLOEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_GALEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GALEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_GALEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_QZSSEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_QZSSEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_QZSSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_BDSEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_BDSEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_BDSEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_SBASEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_SBASEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_SBASEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		case RINEX_ID_NICEPH_HEADER:
		{
			if (pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_NICEPH])
			{
				fprintf(pRinexMultiConvertData->RinexNavFileFp[MSG_CONVERT_FILE_INDEX_NICEPH], "%s", ((MSG_BUF_T*)data)->pBaseAddr);
			}
			break;
		}
		default:
			break;
	}
}

/**********************************************************************//**
@brief  all freq decode

@param pMsgConvertData	  [In] pointer to message convert data
@param pMsgDecode		   [In] pointer to message decode
@param SourceType		   [In] data source type

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int AllFreqDecode(MSG_CONVERT_DATA_T* pMsgConvertData, MSG_DECODE_T* pMsgDecode, int SourceType)
{
	int DecodeType = GetDecodeType(SourceType), Index = 0;
	unsigned char Buffer[READ_BUF_LEN_MAX] = {0};
	double PosHistoryBuf[] = { 0,0,0 };
	size_t BufferSize = 0;
	int DataTypeIndex;
	int TypeIndex;
	unsigned int RawDataTypeCount = 0;

	GNSSPrintf("Analyze all system frequency points of observed values ... Please wait...\n");
	for (Index = 0; Index < 3; Index++)
	{
		PosHistoryBuf[Index] = pMsgConvertData->RINEXObsHeader.Pos[Index];
	}

	if ((DecodeType & MSG_DECODE_TYPE_RTCM2) ||/**< rtcm2 */
		(DecodeType & MSG_DECODE_TYPE_RTCM3) ||/**< rtcm3 */
		(DecodeType & MSG_DECODE_TYPE_RINEX) ||/**< Rinex */
		(DecodeType & MSG_DECODE_TYPE_HRCX) ||/**< HRCX */
		(DecodeType & MSG_DECODE_TYPE_RAWMSGA) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_RAWMSGB) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_CHCRAWMSGB) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_RTKLOGA) ||/**< RTKLOG Binary */
		(DecodeType & MSG_DECODE_TYPE_RTKLOGB))/**< RTKLOG Binary */
	{
		MsgDecodeReset(pMsgDecode);

		while ((BufferSize = fread(Buffer, 1, READ_BUF_LEN_MAX, pMsgConvertData->RawFp)) > 0)
		{
			MsgDecode(pMsgDecode, (char*)Buffer, (unsigned int)BufferSize, DecodeType, SourceType);
		}

		fseek(pMsgConvertData->RawFp, 0, SEEK_SET);

		for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
		{
			if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
			{
				RawDataTypeCount++;
				TypeIndex = DataTypeIndex;
			}
		}

		if (RawDataTypeCount == 1)
		{
			pMsgConvertData->RawObsDataType = 0;

			MEMCPY(&pMsgConvertData->GlobalObsConvertData, &pMsgConvertData->SingleObsConvertData[TypeIndex], sizeof(SINGLE_OBS_CONVERT_DATA_T));
		}

		if (pMsgConvertData->AllFreqFlag > 0)
		{
			ResetSysTObs(pMsgConvertData);
		}

		return 1;
	}
	else
	{
		GNSSPrintf("The decode type is not supported!\n");
	}

	if (pMsgConvertData->FpOutO && fclose(pMsgConvertData->FpOutO) == EOF)
	{
		GNSSPrintf(".o file close error!\n");
	}

	for (Index = MSG_CONVERT_FILE_INDEX_MUTIEPH; Index < MSG_CONVERT_FILE_NUM_MAX; Index++)
	{
		if (pMsgConvertData->FpOutP[Index] && fclose(pMsgConvertData->FpOutP[Index]) == EOF)
		{
			GNSSPrintf("eph file close error!\n");
		}
	}

	return 0;
}

/**********************************************************************//**
@brief  rinex data buf init

@param pDatBuf	  [In] pointer to rinex data buf

@author CHC
@date 2023/05/17
@note
**************************************************************************/
extern void RINEXDataBufInit(RINEX_DATA_BUF_T* pDatBuf)
{
	if (!pDatBuf)
	{
		return;
	}

	pDatBuf->ReceiveID = 0;
	MEMSET(pDatBuf->NavTimeFlag, 0, sizeof(pDatBuf->NavTimeFlag));
	MEMSET(pDatBuf->BDSTimeFlag, 0, sizeof(pDatBuf->BDSTimeFlag));
	MEMSET(pDatBuf->GALTimeFlag, 0, sizeof(pDatBuf->GALTimeFlag));
	MEMSET(pDatBuf->GALTypeTimeFlag, 0, sizeof(pDatBuf->GALTypeTimeFlag));
	MEMSET(pDatBuf->BDSJpFlag, 0, sizeof(pDatBuf->BDSJpFlag));
}

/**********************************************************************//**
@brief  get rinex id

@param EncodeID			 [In] encode id
@param pInData			  [In] pointer to input data buf

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
unsigned int GetRINEXID(unsigned int EncodeID, void* pInData)
{
	switch (EncodeID)
	{
		case RINEX_ID_OBS_CRX_HEADER:
		case RINEX_ID_OBS_HEADER:
		{
			RINEX_OBS_OPT_T* pTempData = (RINEX_OBS_OPT_T*)pInData;
			if (pTempData->Ver == 211)return (EncodeID * 10);
			else if (pTempData->Ver == 302)return (EncodeID * 10 + 1);
			else if (pTempData->Ver == 304)return (EncodeID * 10 + 2);
			else if (pTempData->Ver == 305)return (EncodeID * 10 + 3);
		}
		break;
		case RINEX_ID_MUTIEPH_HEADER:
		case RINEX_ID_GPSEPH_HEADER:
		case RINEX_ID_GALEPH_HEADER:
		case RINEX_ID_BDSEPH_HEADER:
		case RINEX_ID_GLOEPH_HEADER:
		case RINEX_ID_QZSSEPH_HEADER:
		case RINEX_ID_SBASEPH_HEADER:
		case RINEX_ID_NICEPH_HEADER:
		{
			RINEX_EPH_OPT_T* pTempData = (RINEX_EPH_OPT_T*)pInData;
			if (pTempData->Ver == 211)return (EncodeID * 10);
			else if (pTempData->Ver == 302)return (EncodeID * 10 + 1);
			else if (pTempData->Ver == 304)return (EncodeID * 10 + 2);
			else if (pTempData->Ver == 305)return (EncodeID * 10 + 3);
		}
		break;
		case RINEX_ID_METE_HEADER:
		{
			unsigned int* pMeteVer = (unsigned int*)pInData;
			if (*pMeteVer == 211)return (EncodeID * 10);
			else if (*pMeteVer == 302)return (EncodeID * 10 + 1);
			else if (*pMeteVer == 304)return (EncodeID * 10 + 2);
			else if (*pMeteVer == 305)return (EncodeID * 10 + 3);
		}
		break;
		case RINEX_ID_OBS:
		{
			RINEX_OBS_OPT_T* pTempData = (RINEX_OBS_OPT_T*)pInData;
			if (pTempData->Ver == 211)return (EncodeID * 10);
			else if (pTempData->Ver == 302)return (EncodeID * 10 + 1);
			else if (pTempData->Ver == 304)return (EncodeID * 10 + 2);
			else if (pTempData->Ver == 305)return (EncodeID * 10 + 3);
		}
		break;
		case RINEX_ID_GPSEPH:
		case RINEX_ID_BDSEPH:
		case RINEX_ID_BDSEPH_CNAV1:
		case RINEX_ID_BDSEPH_CNAV2:
		case RINEX_ID_GALEPH:
		case RINEX_ID_GALEPH_FNAV:
		case RINEX_ID_GALEPH_INAV:
		case RINEX_ID_GLOEPH:
		case RINEX_ID_QZSSEPH:
		{
			RINEX_EPH_OPT_T* pTempData = (RINEX_EPH_OPT_T*)pInData;
			if (pTempData->Ver == 211)return (EncodeID * 10);
			else if (pTempData->Ver == 302)return (EncodeID * 10 + 1);
			else if (pTempData->Ver == 304)return (EncodeID * 10 + 2);
			else if (pTempData->Ver == 305)return (EncodeID * 10 + 3);
		}
		break;
		case RINEX_ID_METE:
		{
			unsigned int* pMeteVer = (unsigned int*)pInData;
			if (*pMeteVer == 211)return (EncodeID * 10);
			else if (*pMeteVer == 302)return (EncodeID * 10 + 1);
			else if (*pMeteVer == 304)return (EncodeID * 10 + 2);
			else if (*pMeteVer == 305)return (EncodeID * 10 + 3);
		}
		break;
		default:break;
		}

	return 0;
}

/**********************************************************************//**
@brief  rinex encode data

@param EncodeID			 [In] encode id
@param pMsgConvertData	  [In] pointer to message convert data
@param Callback			 [In] callback function

@retval <0:error, >=0:encode data length

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RINEXSingleEncodeData(int EncodeID, MSG_CONVERT_DATA_T* pMsgConvertData, SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData, DoCallback Callback)
{
	int ResultID = -1;
	int Result = -1;

	if ((EncodeID <= 0) || (!pMsgConvertData))
	{
		return -1;
	}

	Result = GenerateRINEX(EncodeID, pMsgConvertData->EncodeDataBuf.pBaseAddr, &pSingleObsConvertData->RINEXEphOpt, &pSingleObsConvertData->RINEXEphHeader);
	if (Result < 0)
	{
		return Result;
	}

	pMsgConvertData->EncodeDataBuf.Len = Result;

	ResultID = GetRINEXID(EncodeID, &pSingleObsConvertData->RINEXEphOpt);

	if ((Callback) && (pMsgConvertData->EncodeDataBuf.Len > 0))
	{
		Callback(pSingleObsConvertData, ResultID, &pMsgConvertData->EncodeDataBuf);
	}

	return pMsgConvertData->EncodeDataBuf.Len;
}

int RINEXEncodeData(int EncodeID, MSG_CONVERT_DATA_T* pMsgConvertData, DoCallback Callback)
{
	int Result = -1;
	unsigned int DataTypeIndex;

	if (pMsgConvertData->RawObsDataType == 0)
	{
		Result = RINEXSingleEncodeData(EncodeID, pMsgConvertData, &pMsgConvertData->GlobalObsConvertData, Callback);
	}
	else
	{
		for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
		{
			if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
			{
				Result = RINEXSingleEncodeData(EncodeID, pMsgConvertData, &pMsgConvertData->SingleObsConvertData[DataTypeIndex], Callback);
				if (Result < 0)
				{
					return Result;
				}
			}
		}
	}

	return Result;
}

/**********************************************************************//**
@brief  rinex encode data

@param EncodeID			 [In] encode id
@param pMsgConvertData	  [In] pointer to message convert data
@param Callback			 [In] callback function

@retval <0:error, >=0:encode data length

@author CHC
@date 2023/05/17
@note
**************************************************************************/
static int RinexMultiEncodeData(int EncodeID, MSG_CONVERT_DATA_T* pMsgConvertData, RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData, DoCallback Callback)
{
	int ResultID = -1;
	int Result = -1;

	if ((EncodeID <= 0) || (!pMsgConvertData))
	{
		return -1;
	}

	Result = GenerateRINEX(EncodeID, pMsgConvertData->EncodeDataBuf.pBaseAddr, &pRinexMultiConvertData->RINEXEphOpt, &pRinexMultiConvertData->RINEXEphHeader);
	if (Result < 0)
	{
		return Result;
	}

	pMsgConvertData->EncodeDataBuf.Len = Result;

	ResultID = GetRINEXID(EncodeID, &pRinexMultiConvertData->RINEXEphOpt);

	if ((Callback) && (pMsgConvertData->EncodeDataBuf.Len > 0))
	{
		Callback(pRinexMultiConvertData, ResultID, &pMsgConvertData->EncodeDataBuf);
	}

	return pMsgConvertData->EncodeDataBuf.Len;
}

/**********************************************************************//**
@brief  rinex multi header init

@param pMsgConvertData	  [In] pointer to message convert data

@retval <0:error, >=0:ok

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int RinexMultiHeaderInit(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	RINEX_MULTI_CONVERT_DATA_T* pRinexMultiConvertData = &pMsgConvertData->RinexMultiConvertData;
	int Index = 0;
	int Header[] = {RINEX_ID_MUTIEPH_HEADER, RINEX_ID_GPSEPH_HEADER,
					RINEX_ID_GLOEPH_HEADER, RINEX_ID_GALEPH_HEADER,
					RINEX_ID_QZSSEPH_HEADER, RINEX_ID_BDSEPH_HEADER,
					RINEX_ID_SBASEPH_HEADER, RINEX_ID_NICEPH_HEADER };

	if (pRinexMultiConvertData->RINEXEphHeader.Ver >= 300)
	{
		RinexMultiEncodeData(Header[0], pMsgConvertData, pRinexMultiConvertData, RinexMultiEncodeCallback);
	}

	for (Index = 1; Index < (sizeof(Header) / sizeof(int)); Index++)
	{
		RinexMultiEncodeData(Header[Index], pMsgConvertData, pRinexMultiConvertData, RinexMultiEncodeCallback);
	}

	return 0;
}

/**********************************************************************//**
@brief  rinex deocde start

@param pMsgConvertData	 	[In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/12/25
@note
**************************************************************************/
int RinexDecodeStart(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	RINEXDataBufInit(&pMsgConvertData->RINEXDataBuf);
	return RinexDecodeConvert(pMsgConvertData);
}

/**********************************************************************//**
@brief  data deocde start

@param pMsgConvertData	  [In] pointer to message convert data
@param pMsgDecode		   [In] pointer to message decode
@param SourceType		   [In] data source type

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int DataDecodeStart(MSG_CONVERT_DATA_T* pMsgConvertData, MSG_DECODE_T* pMsgDecode, int SourceType)
{
	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData;
	int DecodeType = GetDecodeType(SourceType);
	unsigned char Buffer[READ_BUF_LEN_MAX] = "";
	FILE* pFpOut = stdout;
	size_t BufferSize = 0;
	int Index = 0;
	int Header[] = {RINEX_ID_MUTIEPH_HEADER, RINEX_ID_GPSEPH_HEADER,
					RINEX_ID_GLOEPH_HEADER, RINEX_ID_GALEPH_HEADER,
					RINEX_ID_QZSSEPH_HEADER, RINEX_ID_BDSEPH_HEADER,
					RINEX_ID_SBASEPH_HEADER, RINEX_ID_NICEPH_HEADER };

	if ((pMsgConvertData == NULL) || (pMsgDecode == NULL))
	{
		return -1;
	}

	RINEXDataBufInit(&pMsgConvertData->RINEXDataBuf);

	if ((DecodeType & MSG_DECODE_TYPE_RTCM2) ||/**< rtcm2 */
		(DecodeType & MSG_DECODE_TYPE_RTCM3) ||/**< rtcm3 */
		(DecodeType & MSG_DECODE_TYPE_RINEX) ||/**< Rinex */
		(DecodeType & MSG_DECODE_TYPE_HRCX) ||/**< HRCX */
		(DecodeType & MSG_DECODE_TYPE_RAWMSGA) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_RAWMSGB) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_CHCRAWMSGB) ||/**< RawMsg Binary */
		(DecodeType & MSG_DECODE_TYPE_RTKLOGA) ||/**< RTKLOG Binary */
		(DecodeType & MSG_DECODE_TYPE_RTKLOGB))/**< RTKLOG Binary */
	{
		if (pMsgConvertData->RINEXEphHeader.Ver >= 300)
		{
			RINEXEncodeData(Header[0], pMsgConvertData, RINEXEncodeCallback);
		}

		for (Index = 1; Index < (sizeof(Header) / sizeof(int)); Index++)
		{
			RINEXEncodeData(Header[Index], pMsgConvertData, RINEXEncodeCallback);
		}

		if (pMsgConvertData->AllFreqFlag < 1)
		{
			int Year = 0;
			char FileName[256] = {0};
			size_t Len = strlen(pMsgConvertData->OutDir);
			UTC_TIME_T TempTime = RTCMGetUTCTime();
			const char* pGetRawObsDataTypeStr;
			int DataTypeIndex;

			Year = (int)TempTime.Year % 100;
			MEMCPY(FileName, pMsgConvertData->OutDir, Len);
			sprintf(FileName + Len, "%s", pMsgConvertData->PureName);

			if (pMsgConvertData->RawObsDataType == 0)
			{
				pSingleObsConvertData = &pMsgConvertData->GlobalObsConvertData;

				sprintf(FileName + (int)strlen(FileName), ".%2do", Year);
				pSingleObsConvertData->RinexObsFileFp = fopen(FileName, "wb+");
				MEMCPY(pSingleObsConvertData->OutputObsFilePath, FileName, strlen(FileName));

				OutRINEXObsHeader(pSingleObsConvertData->RinexObsFileFp, &pSingleObsConvertData->RINEXObsOpt, &pSingleObsConvertData->RINEXObsHeader, &pMsgConvertData->EncodeDataBuf);
			}
			else
			{
				for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
				{
					if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
					{
						pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[DataTypeIndex];

						pGetRawObsDataTypeStr = GetRawObsDataTypeString(DataTypeIndex);
						if (pGetRawObsDataTypeStr == NULL)
						{
							continue;
						}

						sprintf(FileName + (int)strlen(FileName), "%s.%02do", pGetRawObsDataTypeStr, Year);
						pSingleObsConvertData->RinexObsFileFp = fopen(FileName, "wb+");
						MEMCPY(pSingleObsConvertData->OutputObsFilePath, FileName, strlen(FileName));

						OutRINEXObsHeader(pSingleObsConvertData->RinexObsFileFp, &pSingleObsConvertData->RINEXObsOpt, &pSingleObsConvertData->RINEXObsHeader, &pMsgConvertData->EncodeDataBuf);
					}
				}
			}
		}

		if (pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo)
		{
			RTCMParseInfoReset(pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo);
			pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo->RTCMInterimData.RTCMTime = pMsgConvertData->RTCMTime;
		}

		if (pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo)
		{
			HRCXParseInfoReset(pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo);
			pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo->HRCXInterimData.HRCXTime = pMsgConvertData->HRCXTime;
		}

		MsgDecodeReset(pMsgDecode);

		while ((BufferSize = fread(Buffer, 1, READ_BUF_LEN_MAX, pMsgConvertData->RawFp)) > 0)
		{
			MsgDecode(pMsgDecode, (char*)Buffer, (unsigned int)BufferSize, DecodeType, SourceType);
		}
#ifdef CONVERT_DEBUG_OUTPUT
		OutputSysSigType(pMsgConvertData);

		if (DecodeType == 1)
		{
			GNSSPrintf("Number of base station change records=%d\n", pMsgConvertData->PosHistoryNum);
			for (Index = 0; Index < pMsgConvertData->PosHistoryNum && Index < MAXPOSHIS; Index++)
			{
				GNSSPrintf("%.4f %.4f %.4f\n", pMsgConvertData->PosHistory[Index][0], pMsgConvertData->PosHistory[Index][1], pMsgConvertData->PosHistory[Index][2]);
			}
		}

		GNSSPrintf("s_ObsEpochNum=%d\n", pMsgConvertData->ObsEpochNum);
#endif
	}
	else
	{
		GNSSPrintf("The decode type is not supported!\n");
	}

	if (DATA_OUT_TYPE_RINEX == GetDataOutType(pMsgConvertData))
	{
		if (pMsgConvertData->RawObsDataType == 0)
		{
			pSingleObsConvertData = &pMsgConvertData->GlobalObsConvertData;

			if (!pSingleObsConvertData->RinexObsFileFp)
			{
				GNSSPrintf(".o file pointer error!\n");
			}
			else
			{
				if (fclose(pSingleObsConvertData->RinexObsFileFp) == EOF)
				{
					GNSSPrintf(".o file close error!\n");
				}
			}
		}
		else
		{
			for (int DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
			{
				if (0 != (pMsgConvertData->RawObsDataType & (1 << DataTypeIndex)))
				{
					pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[DataTypeIndex];

					if (!pSingleObsConvertData->RinexObsFileFp)
					{
						GNSSPrintf(".o file pointer error!\n");
					}
					else
					{
						if (fclose(pSingleObsConvertData->RinexObsFileFp) == EOF)
						{
							GNSSPrintf(".o file close error!\n");
						}
					}
				}
			}
		}

		if (pMsgConvertData->RINEXEphHeader.Ver >= 300)
		{
			if (pMsgConvertData->FpOutPFlag[MSG_CONVERT_FILE_INDEX_MUTIEPH] != 0)
			{
				if (fclose(pMsgConvertData->FpOutP[MSG_CONVERT_FILE_INDEX_MUTIEPH]) == EOF)
				{
					GNSSPrintf("eph file close error!\n");
				}
			}
		}
		for (Index = MSG_CONVERT_FILE_INDEX_GPSEPH; Index < MSG_CONVERT_FILE_NUM_MAX; Index++)
		{
			if (pMsgConvertData->FpOutPFlag[Index] != 0)
			{
				if (fclose(pMsgConvertData->FpOutP[Index]) == EOF)
				{
					GNSSPrintf("eph file close error!\n");
				}
			}
		}

		for (Index = 0; Index < 10; Index++)
		{
			if (pMsgConvertData->LbandRawFrameFpOutPFlag[Index] != 0)
			{
				if (fclose(pMsgConvertData->LbandRawFrameFpOutP[Index]) == EOF)
				{
					GNSSPrintf("LbandRawFrame file close error!\n");
				}
			}
		}
	}
	else if (DATA_OUT_TYPE_RAWMSGASCII== GetDataOutType(pMsgConvertData))
	{
		if (!pMsgConvertData->FpRawMsgOutP)
		{
			GNSSPrintf("RANGEA file pointer error!\n");
		}
		else
		{
			if (fclose(pMsgConvertData->FpRawMsgOutP) == EOF)
			{
				GNSSPrintf("RANGEA file close error!\n");
			}
		}
	}
	else if (DATA_OUT_TYPE_BASEOBSASCII== GetDataOutType(pMsgConvertData))
	{
		if (!pMsgConvertData->FpRawMsgOutP)
		{
			GNSSPrintf("BASEOBSA file pointer error!\n");
		}
		else
		{
			if (fclose(pMsgConvertData->FpRawMsgOutP) == EOF)
			{
				GNSSPrintf("BASEOBSA file close error!\n");
			}
		}
	}

	GNSSPrintf("\nMessage convert completed!\n");

	return 0;
}

/**********************************************************************//**
@brief  get data output type

@param pMsgConvertData	  [In] pointer to message convert data

@retval data output type

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int GetDataOutType(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	return pMsgConvertData->DataOutType;
}

/**********************************************************************//**
@brief  message convert deinit

@param pMsgConvertData	  [In] pointer to message convert data

@retval <0:error, =0:success

@author CHC
@date 2023/05/17
@note
**************************************************************************/
int MsgConvertDeinit(MSG_CONVERT_DATA_T* pMsgConvertData)
{
	if ((pMsgConvertData == NULL) || (pMsgConvertData->EncodeDataBuf.pBaseAddr == NULL))
	{
		return -1;
	}

	FREE(pMsgConvertData->EncodeDataBuf.pBaseAddr);
	FREE(pMsgConvertData);

	return 0;
}

/**********************************************************************//**
@brief  message convert init

@retval message convert data pointer

@author CHC
@date 2023/05/17
@note
**************************************************************************/
MSG_CONVERT_DATA_T* MsgConvertInit(void)
{
	MSG_CONVERT_DATA_T* pMsgConvertData = NULL;
	SINGLE_OBS_CONVERT_DATA_T* pSingleObsConvertData;
	int DataTypeIndex;
	int iGroup;

	pMsgConvertData = (MSG_CONVERT_DATA_T*)MALLOC(sizeof(MSG_CONVERT_DATA_T));
	if (pMsgConvertData == NULL)
	{
		return NULL;
	}

	MEMSET(pMsgConvertData, 0, sizeof(MSG_CONVERT_DATA_T));

	pMsgConvertData->EncodeDataBuf.pBaseAddr = MALLOC(MAX_ENCODE_DATA_BUF_LEN);
	if (!pMsgConvertData->EncodeDataBuf.pBaseAddr)
	{
		FREE(pMsgConvertData);
		return NULL;
	}

	InitEphHeader(&pMsgConvertData->GlobalObsConvertData.RINEXEphHeader);
	for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
	{
		InitEphHeader(&pMsgConvertData->SingleObsConvertData[DataTypeIndex].RINEXEphHeader);
	}
	InitEphHeader(&pMsgConvertData->RINEXEphHeader);
	InitGLOEphHeader(&pMsgConvertData->GlobalObsConvertData.RINEXGEphHeader);
	for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
	{
		InitGLOEphHeader(&pMsgConvertData->SingleObsConvertData[DataTypeIndex].RINEXGEphHeader);
	}
	InitGLOEphHeader(&pMsgConvertData->RINEXGEphHeader);
	InitEphOpt(&pMsgConvertData->RINEXEphOpt);
	InitEphOpt(&pMsgConvertData->GlobalObsConvertData.RINEXEphOpt);
	for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
	{
		InitEphOpt(&pMsgConvertData->SingleObsConvertData[DataTypeIndex].RINEXEphOpt);
	}

	pMsgConvertData->GlobalObsConvertData.MaxPoshis = 1;
	InitObsHeader(&pMsgConvertData->GlobalObsConvertData.RINEXObsHeader);
	InitObsOpt(&pMsgConvertData->GlobalObsConvertData.RINEXObsOpt);
	MEMSET(pMsgConvertData->GlobalObsConvertData.SysSigType, NA8, sizeof(pMsgConvertData->GlobalObsConvertData.SysSigType));

	for (int DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
	{
		pSingleObsConvertData = &pMsgConvertData->SingleObsConvertData[DataTypeIndex];

		pSingleObsConvertData->MaxPoshis = 1;
		InitObsHeader(&pSingleObsConvertData->RINEXObsHeader);
		InitObsOpt(&pSingleObsConvertData->RINEXObsOpt);
		MEMSET(pSingleObsConvertData->SysSigType, NA8, sizeof(pSingleObsConvertData->SysSigType));

		
	}

	for (iGroup = 0; iGroup < RINEX_GROUP_NUM_MAX; iGroup++)
	{
		for (DataTypeIndex = 0; DataTypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; DataTypeIndex++)
		{
			CycleBufInit(&pMsgConvertData->RinexMultiInfo[iGroup].CycleBufCtrl[DataTypeIndex], "RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)pMsgConvertData->RinexMultiInfo[iGroup].RinexDataBuf[DataTypeIndex], 0, MsgRecv);
		}
	}

	return pMsgConvertData;
}
#endif
