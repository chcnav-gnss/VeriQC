/*************************************************************************
		   VeriQC

	Interface Module
*-
@file   QCProcessor.h
@author CHC
@date   1st January 2023
@brief  Some Algorithm common function for QC
*****************************************************************************/

#include <ctype.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "QCProcessor.h"
#include "MsgConvert.h"
#include "Rinex/RinexDecoder.h"
#include "MsgConvertTransQC.h"
#include "MsgDecode.h"
#include "MsgCodec.h"
#include "RawMsgAscii/RawMsgAsciiFilter.h"
#include "RawMsgBinary/RawMsgBinaryFilter.h"
#include "CHCRawMsgBinary/CHCRawMsgBinaryFilter.h"
#include "UNRawMsgBinary/UNRawMsgBinaryFilter.h"
#include "HAL/DriverCycleDataBuffer.h"
#include "RTCM/RTCM3Filter.h"
#include "RTCM/RTCM2Filter.h"
#include "HRCX/HRCXFilter.h"
#include "CommonBase.h"
#include "QCCommon.h"
#include "Coordinate.h"
#include "TimeDefines.h"

#define QC_READ_MAX_BUFF     QC_MIN(64, OBS_CODEC_BUF_SIZE)

static int s_FreqGLO[QC_NUM_SAT_GLO] = { 0 };
static double s_GNSSPreProcessProgress = 0.0;

static BOOL IsNoEphModeEnabled(const QC_OPT_T* pOpt)
{
	return (pOpt != NULL) && (pOpt->EphMode == QC_EPH_MODE_NONE);
}

static void InitOpenedNavCodec(CHCQC_READER_T* pQcReader, int CodecOrder, int RinexFileIndex, FILE* fp, uint8_t OrderNumber)
{
	QC_CODEC_INFO_T* pCodecInfo = (QC_CODEC_INFO_T*)pQcReader->pCodecInfo[CodecOrder];

	CycleBufInit(&pCodecInfo->RinexInfo.CycleBufCtrl[RinexFileIndex],
		"RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)pCodecInfo->RinexInfo.RinexDataBuf[RinexFileIndex], 0, MsgRecv);
	OpenRinexInfo(&pCodecInfo->RinexInfo, fp, RinexFileIndex);
	pCodecInfo->fpInputFile = fp;
	pCodecInfo->MsgConvertRinexIndex = RinexFileIndex;
	pCodecInfo->StreamFormat = GNSS_STRFMT_RINEX;
	pCodecInfo->EpochWait = 1;
	pQcReader->AntID[CodecOrder] = (OrderNumber < 7) ? VERIQC_ANT_ID_ROVER : VERIQC_ANT_ID_BASE;
}

static FILE* OpenNavVariant(char* pFilePath, char NavSuffix)
{
	size_t FilePathLen;

	if (pFilePath == NULL)
	{
		return NULL;
	}

	FilePathLen = strlen(pFilePath);
	if (FilePathLen < 1)
	{
		return NULL;
	}

	pFilePath[FilePathLen - 1] = NavSuffix;
	return fopen(pFilePath, "rb");
}

static int GetNavCodecOrderOffset(int RinexFileIndex)
{
	switch (RinexFileIndex)
	{
		case RINEX_FILE_INDEX_MUTIEPH:
			return 0;
		case RINEX_FILE_INDEX_GPSEPH:
			return 1;
		case RINEX_FILE_INDEX_GLOEPH:
			return 2;
		case RINEX_FILE_INDEX_GALEPH:
			return 3;
		case RINEX_FILE_INDEX_BDSEPH:
			return 4;
		default:
			return -1;
	}
}

static int DetectSpecifiedNavRinexIndex(FILE* fp)
{
	char LineBuf[256] = { 0 };
	char FileType;
	char SystemChar = ' ';
	double Version;

	if (fp == NULL)
	{
		return -1;
	}

	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		return -1;
	}

	while (fgets(LineBuf, sizeof(LineBuf), fp) != NULL)
	{
		if (strstr(LineBuf, "RINEX VERSION / TYPE") == NULL)
		{
			continue;
		}

		if (strlen(LineBuf) <= 20)
		{
			break;
		}

		Version = strtod(LineBuf, NULL);
		FileType = LineBuf[20];
		if (strlen(LineBuf) > 40)
		{
			SystemChar = LineBuf[40];
		}

		if (Version < 3.0)
		{
			switch (FileType)
			{
				case 'N':
					return RINEX_FILE_INDEX_GPSEPH;
				case 'G':
					return RINEX_FILE_INDEX_GLOEPH;
				case 'E':
					return RINEX_FILE_INDEX_GALEPH;
				case 'C':
					return RINEX_FILE_INDEX_BDSEPH;
				default:
					return -1;
			}
		}

		if (FileType != 'N')
		{
			return -1;
		}

		switch (SystemChar)
		{
			case 'G':
				return RINEX_FILE_INDEX_GPSEPH;
			case 'R':
				return RINEX_FILE_INDEX_GLOEPH;
			case 'E':
				return RINEX_FILE_INDEX_GALEPH;
			case 'C':
				return RINEX_FILE_INDEX_BDSEPH;
			case ' ':
			case 'M':
				return RINEX_FILE_INDEX_MUTIEPH;
			default:
				return -1;
		}
	}

	return -1;
}

/*************************************************//**
@brief Initialize QC exception info with default state

@param pInfo   [Out]  exception info structure

@return void
@author CHC

@note 
1.Initialize default exception code
2.Clear observation and ephemeris flags
3.Set all-epoch SPP unsolved flag to TRUE
******************************************************/
static void InitQCExceptionInfo(QC_EXCEPTION_INFO_T* pInfo)
{
	if (pInfo == NULL)
	{
		return;
	}
	pInfo->Code = QC_EXCEPTION_OK;
	pInfo->IsHaveEph = FALSE;
	pInfo->IsHaveObs = FALSE;
	pInfo->IsAllEpochSPPUnsolved = TRUE;
}

/*************************************************//**
@brief Set QC exception code

@param pInfo   [In/Out]  exception info structure
@param Code    [In]      exception code

@return void
@author CHC

@note 
1.Only update exception code
2.Keep other state fields unchanged
******************************************************/
static void SetQCExceptionCode(QC_EXCEPTION_INFO_T* pInfo, int Code)
{
	if (pInfo == NULL)
	{
		return;
	}
	pInfo->Code = Code;
}

/*************************************************//**
@brief Finalize system/file exception code by final system state

@param pQcReader [In/Out]  QC reader object
@param AntID     [In]      ant id {rover:0 base:1}

@return void
@author CHC

@note 
1.Summarize final observation and SPP state of all systems
2.Generate system-level nav-unmatched or spp-unsolved code
3.Generate file-level spp-all-failed or partial-system-failed code
******************************************************/
static void FinalizeFileExceptionBySysState(CHCQC_READER_T* pQcReader, int AntID)
{
	unsigned int iSys = 0;
	int FailedSysCount = 0;
	int ObservedSysCount = 0;
	BOOL IsAnyObs = FALSE;
	BOOL IsAnyEph = FALSE;
	BOOL IsAllObservedSysUnsolved = TRUE;
	BOOL bNoEphMode = FALSE;
	QC_EXCEPTION_INFO_T* pFileException = NULL;

	if ((pQcReader == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	pFileException = &pQcReader->QcRes.FileException[AntID];
	bNoEphMode = IsNoEphModeEnabled(pQcReader->pOpt);
	for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
	{
		QC_EXCEPTION_INFO_T* pSysExc = &pQcReader->QcRes.SysException[AntID][iSys];
		int OldSysCode = pSysExc->Code;

		if (pSysExc->IsHaveEph)
		{
			IsAnyEph = TRUE;
		}

		if (!pSysExc->IsHaveObs)
		{
			continue;
		}

		ObservedSysCount++;
		IsAnyObs = TRUE;
		if (pSysExc->IsAllEpochSPPUnsolved == FALSE)
		{
			IsAllObservedSysUnsolved = FALSE;
		}

		if (!bNoEphMode && !pSysExc->IsHaveEph && (pSysExc->Code == QC_EXCEPTION_OK))
		{
			/** no eph */
			SetQCExceptionCode(pSysExc, QC_EXCEPTION_SYS_NAV_UNMATCHED);
		}
		else if (!bNoEphMode && pSysExc->IsHaveEph && (pSysExc->IsAllEpochSPPUnsolved) && (pSysExc->Code == QC_EXCEPTION_OK))
		{
			SetQCExceptionCode(pSysExc, QC_EXCEPTION_SYS_SPP_UNSOLVED);
		}

		if (pSysExc->Code != QC_EXCEPTION_OK)
		{
			FailedSysCount++;
		}
	}

	pFileException->IsHaveObs = IsAnyObs;
	pFileException->IsHaveEph = IsAnyEph;
	pFileException->IsAllEpochSPPUnsolved = (IsAnyObs == TRUE) ? IsAllObservedSysUnsolved : TRUE;

	if (!bNoEphMode && (IsAnyObs == TRUE) && (pFileException->IsAllEpochSPPUnsolved == TRUE) && (pFileException->Code == QC_EXCEPTION_OK))
	{
		SetQCExceptionCode(pFileException, QC_EXCEPTION_SPP_ALL_FAILED);
	}
	else if (!bNoEphMode && (FailedSysCount > 0) && (FailedSysCount < ObservedSysCount) && (pFileException->Code == QC_EXCEPTION_OK))
	{
		SetQCExceptionCode(pFileException, QC_EXCEPTION_PARTIAL_SYS_FAILED);
	}
}

/*************************************************//**
@brief Insert observation break info into top list

@param pQcRes          [In/Out]  QC result structure
@param AntID           [In]      ant id {rover:0 base:1}
@param pBreakStartTime [In]      break start time
@param BreakDurationSec[In]      break duration in seconds

@return void
@author CHC

@note 
1.Record every break count
2.Keep top break list in descending order
3.Only preserve the longest QC_DIAG_TOP_BREAK_NUM breaks
******************************************************/
static void InsertObsBreakTop(QC_RESULT_T* pQcRes, int AntID, const QC_TIME_T* pBreakStartTime, float BreakDurationSec)
{
	int iLoop = 0;
	int InsertPos = 0;
	int ValidCount = 0;

	if ((pQcRes == NULL) || (pBreakStartTime == NULL) || (AntID < 0) || (AntID >= QC_MAX_NUM_OBS_FILE))
	{
		return;
	}

	if (BreakDurationSec <= 0.0f)
	{
		return;
	}

	pQcRes->ObsBreakCount[AntID]++;
	ValidCount = pQcRes->ObsBreakCount[AntID];
	if (ValidCount > QC_DIAG_TOP_BREAK_NUM)
	{
		ValidCount = QC_DIAG_TOP_BREAK_NUM;
	}

	while ((InsertPos < ValidCount) && 
		(BreakDurationSec <= pQcRes->ObsBreakTop[AntID][InsertPos].BreakDurationSec))
	{
		InsertPos++;
	}

	if (pQcRes->ObsBreakCount[AntID] <= QC_DIAG_TOP_BREAK_NUM)
	{
		for (iLoop = ValidCount - 1; iLoop > InsertPos; iLoop--)
		{
			pQcRes->ObsBreakTop[AntID][iLoop] = pQcRes->ObsBreakTop[AntID][iLoop - 1];
		}
		pQcRes->ObsBreakTop[AntID][InsertPos].BreakStartTime = *pBreakStartTime;
		pQcRes->ObsBreakTop[AntID][InsertPos].BreakDurationSec = BreakDurationSec;

		return;
	}

	if (InsertPos >= QC_DIAG_TOP_BREAK_NUM)
	{
		return;
	}

	for (iLoop = QC_DIAG_TOP_BREAK_NUM - 1; iLoop > InsertPos; iLoop--)
	{
		pQcRes->ObsBreakTop[AntID][iLoop] = pQcRes->ObsBreakTop[AntID][iLoop - 1];
	}
	pQcRes->ObsBreakTop[AntID][InsertPos].BreakStartTime = *pBreakStartTime;
	pQcRes->ObsBreakTop[AntID][InsertPos].BreakDurationSec = BreakDurationSec;
}

/**********************************************************************//**
@brief get system signal type from Range Log

@param pObs         [In]     Observation info
@param pQcReader    [In/Out] Quality check reader
@param AntID        [In]     Antenna ID

@return 0

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
int QCSetSysSigTypeFromRange(GNSS_OBS_T* pObs, QC_RESULT_T* pQcRes, int AntID)
{
	unsigned int iObs = 0;
	int iType, sysindex;
	int QCCode = 0;
	int RangeSysID = 0;
	int RangeSigID = 0;

	for (iObs = 0; iObs < pObs->RangeData.ObsNum; iObs++)
	{
		RangeSysID = GetRangeSystem(pObs->RangeData.ObsData[iObs].ChnTrackStatus);
		sysindex = g_RangeToQCSysID[RangeSysID] - 1;

		if (sysindex < 0 || sysindex >= 6) continue;

		RangeSigID = GetRangeSignalType(pObs->RangeData.ObsData[iObs].ChnTrackStatus);
		QCCode = RangeToQCSigID(RangeSysID, RangeSigID);

		if (QCCode == -1) continue;
		for (iType = 0; iType < QC_MAXSIGTYPE; iType++)
		{
			if (pQcRes->SysSigType[AntID][sysindex][iType] == 0)
			{
				pQcRes->SysSigType[AntID][sysindex][iType] = QCCode;
				break;
			}
			else if (QCCode == pQcRes->SysSigType[AntID][sysindex][iType])
			{
				break;
			}
		}
	}

	return 0;
}

void SaveRinexObsHeaderInfo(RINEX_OBS_HEADER_T* pSrc, QC_RINEX_HEADER_T* pDst)
{
	MEMCPY(pDst->RecVer, pSrc->RecVer, strlen(pSrc->RecVer));
	MEMCPY(pDst->RecSN, pSrc->RecNo, strlen(pSrc->RecNo));
	MEMCPY(pDst->RecType, pSrc->RecType, strlen(pSrc->RecType));
	MEMCPY(pDst->AntType, pSrc->AntDes, strlen(pSrc->AntDes));
	MEMCPY(pDst->AntSN, pSrc->AntNo, strlen(pSrc->AntNo));
	pDst->AntDeltaH = pSrc->Del[0];
}

/**********************************************************************//**
@brief Init Obs Codec info

@param pFileName    [In]         File path
@param pQcReader    [In]         Quality check reader
@param AntID        [In]         Antenna ID
@param FileIndex    [In]         File index
@param RTCMTime     [In]         RTCM time

@author CHC
@date
@note
History:
1. 2024/07/25 use MsgConvert funcs instead
**************************************************************************/
void InitObsCodec(char* pFileName, CHCQC_READER_T* pQcReader, int AntID, int FileIndex, QC_TIME_T RTCMTime)
{
	int type = STRFMT_NONE;
	int Result = -1;
	long CurFileOffSet = 0;
	UTC_TIME_T UTCTime = { 0 };
	QC_CODEC_INFO_T* pCodecInfo = NULL;

	pCodecInfo = pQcReader->pCodecInfo[FileIndex];
	InitQCExceptionInfo(&pQcReader->QcRes.FileException[FileIndex]);

	if (pQcReader->pOpt->RawFileDataType == STRFMT_NONE)
	{
		type = DetectFileFormatBySuffix(pFileName);
	}
	else
	{
		type = pQcReader->pOpt->RawFileDataType; 
		pQcReader->QcRes.FileDataType[FileIndex] = pQcReader->pOpt->RawFileDataType;
	}

	if (type == STRFMT_NONE)
	{
		if (strlen(pFileName) > 0)
		{
			PrintfQC("<GNSSQC error>: Unknown data format.\n");
			SetQCExceptionCode(&pQcReader->QcRes.FileException[FileIndex], QC_EXCEPTION_UNSUPPORTED_FORMAT);
		}
		else
		{
			//filename=="";
		}
		return;
	}
	else
	{
		if (type == GNSS_STRFMT_RINEX)
		{
			pCodecInfo->fpInputFile = fopen(pFileName, "r");
		}
		else
		{
			pCodecInfo->fpInputFile = fopen(pFileName, "rb");
		}
		if (pCodecInfo->fpInputFile == NULL)
		{
			PrintfQC("<GNSSQC error>: Open file error:%s\n", pFileName);
			SetQCExceptionCode(&pQcReader->QcRes.FileException[FileIndex], QC_EXCEPTION_OBS_FILE_OPEN_FAILED);
			return;
		}

		switch (type)
		{
		case GNSS_STRFMT_RTCM2:
			pQcReader->QcRes.FileDataType[FileIndex] = GNSS_STRFMT_RTCM2;
			if (RTCMTime.Time == 0)
			{
				PrintfQC("<GNSSQC warning>: no approx time for rtcm decoding\n");
			}
			break;
		case GNSS_STRFMT_RTCM3:
			pQcReader->QcRes.FileDataType[FileIndex] = GNSS_STRFMT_RTCM3;
			if (RTCMTime.Time == 0)
			{
				PrintfQC("<GNSSQC warning>: no approx time for rtcm decoding\n");
			}
			break;
		case GNSS_STRFMT_HRC:
			pQcReader->QcRes.FileDataType[FileIndex] = GNSS_STRFMT_HRC;
			if (RTCMTime.Time == 0)
			{
				PrintfQC("<GNSSQC warning>: no approx time for hrc decoding\n");
			}
			break;
		case STRFMT_TEMP:
		case GNSS_STRFMT_HRCX:
		case GNSS_STRFMT_RAWMSGA:
		case GNSS_STRFMT_RAWMSGB:
		case GNSS_STRFMT_CHCRAWMSGB:
		case GNSS_STRFMT_UNRAWMSGB:
			break;
		case GNSS_STRFMT_RINEX:
			pQcReader->QcRes.FileDataType[FileIndex] = GNSS_STRFMT_RINEX;
			CycleBufInit(&pCodecInfo->RinexInfo.CycleBufCtrl[RINEX_FILE_INDEX_OBS], "RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)pCodecInfo->RinexInfo.RinexDataBuf[RINEX_FILE_INDEX_OBS], 0, MsgRecv);
			pCodecInfo->MsgConvertRinexIndex = RINEX_FILE_INDEX_OBS;
			Result = OpenRinexInfo(&pCodecInfo->RinexInfo, pCodecInfo->fpInputFile, RINEX_FILE_INDEX_OBS);
			SaveRinexObsHeaderInfo(&pCodecInfo->RinexInfo.ObsHeaderInfo.ObsHeader, &pQcReader->RinexObsHeaderInfo);
			if (Result == -1)
			{
				fclose(pCodecInfo->fpInputFile);
				pCodecInfo->fpInputFile = NULL;
				SetQCExceptionCode(&pQcReader->QcRes.FileException[FileIndex], QC_EXCEPTION_PARSE_HEADER_FAILED);
			}
			
			break;
		default:
			PrintfQC("<GNSSQC error>: Unknown data format.\n");
			SetQCExceptionCode(&pQcReader->QcRes.FileException[FileIndex], QC_EXCEPTION_UNSUPPORTED_FORMAT);
			return;
			break;
		}
		
		pQcReader->AntID[FileIndex] = AntID;
		
		QCTimeToUTCTime(&RTCMTime, &UTCTime);
		pCodecInfo->pMsgDecode = MsgCodecInit(0, NULL, DECODE_MODE_CONVERT_TO_SIGNAL, &UTCTime);
		pCodecInfo->StreamFormat = type;
		pCodecInfo->EpochWait = 1;

		if (pCodecInfo->fpInputFile)
		{
			/** get file size */
			CurFileOffSet = ftell(pCodecInfo->fpInputFile);
			fseek(pCodecInfo->fpInputFile, 0, SEEK_END);
			pQcReader->RawFileSize[FileIndex] = ftell(pCodecInfo->fpInputFile);
			fseek(pCodecInfo->fpInputFile, CurFileOffSet, SEEK_SET);
		}
	}
}

/**********************************************************************//**
@brief Init Nav Codec info

@param pFileName    [In]         File path
@param pQcReader    [In]         Quality check reader
@param OrderNumber  [In]         File order

@author CHC
@date
@note
History:
1. 2024/07/25 use MsgConvert funcs instead
**************************************************************************/
BOOL InitNavCodec(char* pFileName, CHCQC_READER_T* pQcReader, uint8_t OrderNumber, char* pSpecifiedNavFile)
{
	char fileNav[QC_MAX_FILE_NAME_LENGTH] = { 0 };
	int i;
	static const char s_NavSuffix[] = { 'p','P', 'n','N', 'G','g', 'L','l', 'C','c' };
	static const int  s_NavFileIndex[] = {
		RINEX_FILE_INDEX_MUTIEPH, RINEX_FILE_INDEX_MUTIEPH,
		RINEX_FILE_INDEX_GPSEPH,  RINEX_FILE_INDEX_GPSEPH,
		RINEX_FILE_INDEX_GLOEPH,  RINEX_FILE_INDEX_GLOEPH,
		RINEX_FILE_INDEX_GALEPH,  RINEX_FILE_INDEX_GALEPH,
		RINEX_FILE_INDEX_BDSEPH,  RINEX_FILE_INDEX_BDSEPH
	};
	int NavTypeCount = (int)(sizeof(s_NavSuffix) / sizeof(s_NavSuffix[0]));
	FILE* fp = NULL;
	BOOL NavFound = FALSE;
	BOOL Specified = (pSpecifiedNavFile != NULL) && (pSpecifiedNavFile[0] != '\0');

	/** Current contract: 2=rover (slots 2..6), 7=base (slots 7..11). */
	if (OrderNumber > 7)
	{
		return FALSE;
	}

	/** --- Branch 1: caller specified a nav file --- */
	if (Specified)
	{
		int RinexFileIndex, CodecOrderOffset;

		if (strlen(pSpecifiedNavFile) >= QC_MAX_FILE_NAME_LENGTH)
		{
			return FALSE;
		}
		strncpy(fileNav, pSpecifiedNavFile, sizeof(fileNav) - 1);

		fp = fopen(fileNav, "rb");
		if (fp == NULL)
		{
			return FALSE;
		}

		RinexFileIndex = DetectSpecifiedNavRinexIndex(fp);
		if ((RinexFileIndex >= RINEX_FILE_INDEX_MUTIEPH) && (RinexFileIndex <= RINEX_FILE_INDEX_BDSEPH))
		{
			CodecOrderOffset = GetNavCodecOrderOffset(RinexFileIndex);
			if (CodecOrderOffset >= 0)
			{
				fseek(fp, 0, SEEK_SET);
				InitOpenedNavCodec(pQcReader, OrderNumber + CodecOrderOffset, RinexFileIndex, fp, OrderNumber);
				return TRUE;
			}
		}

		fclose(fp);
		return FALSE;
	}

	/** --- Branch 2: auto-search by base file name --- */
	if (pFileName == NULL)
	{
		return FALSE;
	}
	if (strlen(pFileName) >= sizeof(fileNav))
	{
		return FALSE;
	}
	strncpy(fileNav, pFileName, sizeof(fileNav) - 1);

	/** Try mixed nav (p/P) first; if found, covers all systems and we are done. */
	for (i = 0; i < 2; i++)
	{
		if ((fp = OpenNavVariant(fileNav, s_NavSuffix[i])) != NULL)
		{
			InitOpenedNavCodec(pQcReader, OrderNumber, RINEX_FILE_INDEX_MUTIEPH, fp, OrderNumber);
			return TRUE;
		}
	}

	/** Otherwise probe each single-system suffix; Step by 2: avoid double-opening the same file on case-insensitive filesystems. */
	for (; i < NavTypeCount; i += 2)
	{
		fp = OpenNavVariant(fileNav, s_NavSuffix[i]);
		if (fp == NULL)
		{
			fp = OpenNavVariant(fileNav, s_NavSuffix[i + 1]);
		}
		if (fp != NULL)
		{
			InitOpenedNavCodec(pQcReader, OrderNumber + (i / 2), s_NavFileIndex[i], fp, OrderNumber);
			NavFound = TRUE;
		}
	}

	return NavFound;
}

/*********************************************************************
@ init pQcReader using pGnssCfg
@param CHCQC_GNSSCFG_T*[I]  pGNSSCfg
@param QC_READER_T*    [I0] pQCReader
@return none
@author CHC
@date   16 January 2023
**********************************************************************/
BOOL SyncGNSSQCState(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader)/**< Synchronization Configuration */
{
	BOOL invail_nav_r, invail_nav_b;
	char outfilename_r[MAX_FILE_NAME_LENGTH] = { 0 };
	char outfilename_b[MAX_FILE_NAME_LENGTH] = { 0 };
	char pureFilename[MAX_FILE_NAME_LENGTH] = { 0 };
	char pOFileName[MAX_FILE_NAME_LENGTH] = { 0 };
	QC_TIME_T Time = { 0 };
	/** pGnssCfg option -> pQcReader */
	memcpy(pQCReader->pOpt, pGNSSCfg, sizeof(CHCQC_GNSSCFG_T));
	GetOutFileStr(pGNSSCfg->OutDir, pGNSSCfg->RoverFile, outfilename_r);
	GetOutFileStr(pGNSSCfg->OutDir, pGNSSCfg->BaseFile, outfilename_b);
#ifdef _DEBUG
	PrintfQC("<GNSSQC info>: pGnssCfg->roverFile=[%s]\n", pGNSSCfg->RoverFile);
	PrintfQC("<GNSSQC info>: pGnssCfg->baseFile=[%s]\n", pGNSSCfg->BaseFile);
	PrintfQC("<GNSSQC info>: outDir=[%s] roverFile=[%s],baseFile=[%s]\n", pGNSSCfg->OutDir, outfilename_r, outfilename_b);
#endif

	if (strlen(outfilename_r) > 1)
	{
		/* rover file yes */
		/** MP and CS file */
		if (pGNSSCfg->bIsOutputQCLog == TRUE)
		{
			sprintf(pOFileName, "%s%s", outfilename_r, ".MP");
			pQCReader->fpOutFileMP[0] = fopen(pOFileName, "w");
			sprintf(pOFileName, "%s%s", outfilename_r, ".CS");
			pQCReader->fpOutFileCS[0] = fopen(pOFileName, "w");
			sprintf(pOFileName, "%s%s", outfilename_r, ".GF");

			if (!pQCReader->fpOutFileMP[0] || !pQCReader->fpOutFileCS[0])
			{
				PrintfQC("<GNSSQC error>: pOutFile_MP=%u pOutFile_CS=%u\n", pQCReader->fpOutFileMP[0], pQCReader->fpOutFileCS[0]);
				return FALSE;
			}
		}

		if (pGNSSCfg->bIsOutputDataList == TRUE)
		{
			sprintf(pOFileName, "%s_DataList%s", outfilename_r, ".log");
			pQCReader->fpOutFileDataList[0] = fopen(pOFileName, "w");
			if (!pQCReader->fpOutFileDataList[0])
			{
				PrintfQC("<GNSSQC error>: pOutFileDataList=%u\n", pQCReader->fpOutFileDataList[0]);
				return FALSE;
			}
		}

#ifdef _DEBUG
		/** open log */
		sprintf(pOFileName, "%s%s", outfilename_r, ".log");
		QCOpenLogfile(pOFileName);
#endif
	}
	if (strlen(outfilename_b) > 1)
	{
		/* base file yes */
		/** MP and CS file */
		if (pGNSSCfg->bIsOutputQCLog == TRUE)
		{
			sprintf(pOFileName, "%s%s", outfilename_b, ".MP");
			pQCReader->fpOutFileMP[1] = fopen(pOFileName, "w");
			sprintf(pOFileName, "%s%s", outfilename_b, ".CS");
			pQCReader->fpOutFileCS[1] = fopen(pOFileName, "w");
			sprintf(pOFileName, "%s%s", outfilename_b, ".GF");

			if (!pQCReader->fpOutFileMP[1] || !pQCReader->fpOutFileCS[1])
			{
				return FALSE;
			}
		}

		if (pGNSSCfg->bIsOutputDataList == TRUE)
		{
			sprintf(pOFileName, "%s_DataList%s", outfilename_b, ".log");
			pQCReader->fpOutFileDataList[1] = fopen(pOFileName, "w");
			if (!pQCReader->fpOutFileDataList[1])
			{
				return FALSE;
			}
		}
	}
	if (strlen(outfilename_r) < 1 && strlen(outfilename_b) < 1)
	{
		return FALSE;
	}

	/** judge the type of the data and initialize the decoder type*/
	Time.Time = pGNSSCfg->RTCMTime.Time;
	Time.Sec = pGNSSCfg->RTCMTime.Sec;
#ifdef _DEBUG
	PrintfQC("<GNSSQC info>: initCodec rover:[%s]\n", pGNSSCfg->RoverFile);
#endif
	InitObsCodec(pGNSSCfg->RoverFile, pQCReader, 0, 0, Time);
#ifdef _DEBUG
	PrintfQC("<GNSSQC info>: initCodec base:[%s]\n", pGNSSCfg->BaseFile);
#endif
	InitObsCodec(pGNSSCfg->BaseFile, pQCReader, 1, 1, Time);
	/** find and decode navigation file in rinex format*/
	if ((pGNSSCfg->EphMode != QC_EPH_MODE_NONE) && ((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[0])->StreamFormat == GNSS_STRFMT_RINEX)
	{
		invail_nav_r = InitNavCodec(pGNSSCfg->RoverFile, pQCReader, 2,
			(pGNSSCfg->EphMode == QC_EPH_MODE_SPECIFIED) ? pGNSSCfg->RinexEphFile : NULL);/**< 2~6 p n g l c for rover*/
		if (invail_nav_r == FALSE)
		{
			PrintfQC("<GNSSQC error>: Rover station navigation file is missing.\n");
			SetQCExceptionCode(&pQCReader->QcRes.FileException[0], QC_EXCEPTION_NAV_FILE_MISSING);
		}
	}
	else
	{
	}
	if ((pGNSSCfg->EphMode != QC_EPH_MODE_NONE) && ((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[1])->StreamFormat == GNSS_STRFMT_RINEX)
	{
		invail_nav_b = InitNavCodec(pGNSSCfg->BaseFile, pQCReader, 7,
			(pGNSSCfg->EphMode == QC_EPH_MODE_SPECIFIED) ? pGNSSCfg->RinexEphFile : NULL);/**< 7~11 p n g l c for base*/
		if (invail_nav_b == FALSE)
		{
			PrintfQC("<GNSSQC error>: Base station navigation file is missing.\n");
			SetQCExceptionCode(&pQCReader->QcRes.FileException[1], QC_EXCEPTION_NAV_FILE_MISSING);
		}
	}
	else
	{
	}

	return TRUE;
}

/**********************************************************************//**
@brief detect missed satellite for use rate

@param pObs        [In]      Epoch satellite observation data
@param AntID       [In]      Antenna ID
@param QcRes       [In]      Quality check result
@param pOpt        [In]      Quality check option

@author CHC
@date 2024/04/25
@note
History:\n
**************************************************************************/
extern void DetectMissSat(QC_OBS_T* pObs, int AntID, QC_RESULT_T* pQcRes, QC_OPT_T* pOpt)
{
	int iSat;
	int iAllSat;
	int SatNo;
	int QCSys;
	unsigned int Prn = 0;
	double EleMask = pOpt->EleDeg * D2R - 1E-4;
	double MPos[3] = { 0 }, E[3] = { 0 }, Azel[2] = { 0 };
	double Distance = 0;
	double SatPosVel[6] = { 0 }; /**< sat pos xyz, vel xyz */
	double SatClock[2] = { 0 };/**< sat clock, clock drift */
	BOOL bSatList[QC_MAX_NUM_SAT] = { FALSE };

	PosXYZ2LLH(pObs->StaPos, MPos);

	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		SatNo = pObs->Data[iSat].Sat - 1;
		bSatList[SatNo] = TRUE;
	}

	/** use Rate */
	for (iAllSat = 0; iAllSat < QC_MAX_NUM_SAT; iAllSat++)
	{
		if (bSatList[iAllSat] == TRUE)
		{
			continue;
		}

		QCSys = GetQCSatSys((iAllSat + 1), &Prn);
		if (!(QCSys & pOpt->Sys))
		{
			continue;
		}

		/** no this satellite data current epoch */
		/** calculate satellite position */
		/** distinguish if miss */
		if (CalSatPosSpecifyTime(pObs->Time, AntID, (iAllSat + 1), SatPosVel, SatClock) == 0)
		{
			Distance = GeoDist(SatPosVel, pObs->StaPos, E);
			QCSatAzEl(MPos, E, Azel);
			if (Azel[1] >= EleMask)
			{
				pQcRes->SatPossibleNum[AntID][iAllSat]++;
				pQcRes->PossibleNum[AntID]++;
			}
		}
	}
}

/**********************************************//**
@brief pre-process call back

@param  pObj     [Out]    qc and decoder data struction
@param  Status   [In]     status
@param  DataID   [In]     ID of data
@param  pData    [In]     data from cgcodec library
@param  pSrcObj  [In]     callback source

@note
 Get Interval info

History:\n
*****************************************************/
void QCPreProcessCallBack(void* pObj, int32_t Status, int DataID, void* pData, void* pSrcObj)
{
	QC_TIME_T obsTime = { 0 };
	int32_t ant_id;
	double dt;
	double FracDeltaTime = 0;
	char msg[512] = { 0 };
	CHCQC_READER_T* pobj = (CHCQC_READER_T*)pObj;
	int Week = 0;
	int WeekMsCount = 0;

	ant_id = Status;/**<@TODO:: Ant info */

	/** get option Interval */
	if (pobj->pOpt->Interval > QC_MIN_SAMPLE_INTERVAL)
	{
		pobj->QcRes.Interval[ant_id] = pobj->pOpt->Interval;
		return;
	}

	switch (DataID)
	{
	case RINEX_ID_OBS:
		UTCToGPSTime(&((GNSS_OBS_T*)pData)->ObsTime, NULL, &Week, &WeekMsCount);
		obsTime = GPSTime2QCTime(Week, WeekMsCount / 1000.0);
		break;
	default:
		break;
	}

	dt = obsTime.Time - pobj->LastTime[ant_id].Time + obsTime.Sec - pobj->LastTime[ant_id].Sec;
	/** Only observation files can be QC, get interval by Obs data */
	if ((ant_id == VERIQC_ANT_ID_ROVER || ant_id == VERIQC_ANT_ID_BASE) && DataID == RINEX_ID_OBS
		&& IsInvailQCTimeRange(pobj, &obsTime, Status) && (dt - pobj->pOpt->Interval > -FLT_EPSILON))
	{
		pobj->LastTime[ant_id].Time = obsTime.Time;
		pobj->LastTime[ant_id].Sec = obsTime.Sec;
		
		if (dt < pobj->QcRes.Interval[ant_id])
		{
			FracDeltaTime = fabs(dt * 100 - round(dt * 100));
			if ((FracDeltaTime < 1e-7) && (dt > QC_MIN_SAMPLE_INTERVAL))
			{
				pobj->QcRes.Interval[ant_id] = (float)dt;
			}
		}
	}
}


/**********************************************//**
@brief First Station Date Scan

@param  pObj     [Out]    qc and decoder data struction
@param  Status   [In]     status
@param  DataID   [In]     ID of data
@param  pData    [In]     data from cgcodec library
@param  pSrcObj  [In]     callback source

@note
 Get mary message:
 1.data use rate;
 2.message record about SNR;
 3.message record about MP;
 4.message record about cycle slip

History:
1. 2024/7/25 use GNSS_OBS_T/UNI_EPHEMERIS_T/GLO_EPHEMERIS_T instead
*****************************************************/
void QCScanStationCallBack(void* pObj, int32_t Status, int DataID, void* pData, void* pSrcObj)
{
	QC_TIME_T obsTime = { 0 };
	int32_t state = 0, strfmt = STRFMT_NONE;
	int32_t ant_id, glo_prn;
	double dt;
	double FracDeltaTime = 0;
	double m_pos[3] = { 0 }, e[3] = { 0 }, azel[2] = { 0 };
	char msg[512] = { 0 };
	CHCQC_READER_T* pobj = (CHCQC_READER_T*)pObj;
	GNSS_OBS_T* pGNSSData = (GNSS_OBS_T*)pData;
	QC_OBS_T LossTempObsData = { 0 };
	int Week = 0;
	int WeekMsCount = 0;

	ant_id = Status;/**<@TODO:: Ant info */

	if ((ant_id != VERIQC_ANT_ID_BASE) && (ant_id != VERIQC_ANT_ID_ROVER))
	{
		return;
	}

	switch (DataID)
	{
	case RINEX_ID_OBS:
		UTCToGPSTime(&((GNSS_OBS_T*)pData)->ObsTime, NULL, &Week, &WeekMsCount);
		obsTime = GPSTime2QCTime(Week, WeekMsCount / 1000.0);
		GetEpochResultSPP(&((GNSS_OBS_T*)pData)->RangeData, ant_id);
		break;
	case RINEX_ID_GPSEPH:
		AddEphSPPCore((UNI_EPHEMERIS_T*)pData);
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GPS - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GPS - 1].IsHaveEph = TRUE;
		}
		break;
	case RINEX_ID_GALEPH:
	case RINEX_ID_GALEPH_FNAV:
	case RINEX_ID_GALEPH_INAV:
		AddEphSPPCore((UNI_EPHEMERIS_T*)pData);
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GAL - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GAL - 1].IsHaveEph = TRUE;
		}
		break;
	case RINEX_ID_BDSEPH:
	case RINEX_ID_BDSEPH_CNAV1:
	case RINEX_ID_BDSEPH_CNAV2:
	case RINEX_ID_BDSEPH_CNAV3:
		AddEphSPPCore((UNI_EPHEMERIS_T*)pData);
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_BDS - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_BDS - 1].IsHaveEph = TRUE;
		}
		break;
	case RINEX_ID_QZSSEPH:
		AddEphSPPCore((UNI_EPHEMERIS_T*)pData);
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_QZS - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_QZS - 1].IsHaveEph = TRUE;
		}
		break;
	case RINEX_ID_GLOEPH:
		AddGephSPPCore((GLO_EPHEMERIS_T*)pData);
		glo_prn = ((GLO_EPHEMERIS_T*)pData)->SatID;
		if (glo_prn >= QC_MIN_PRN_GLO && glo_prn <= QC_MAX_PRN_GLO)
		{
			s_FreqGLO[glo_prn] = ((GLO_EPHEMERIS_T*)pData)->Freq;
		}
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GLN - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_GLN - 1].IsHaveEph = TRUE;
		}
		break;
	case RINEX_ID_LEOEPH:
		AddLEOEphRTKCore((XW_LEO_EPHEMERIS_T*)pData);
		if (!pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_XW1 - 1].IsHaveEph)
		{
			pobj->QcRes.SysException[ant_id][QC_GNSS_SYSID_XW1 - 1].IsHaveEph = TRUE;
		}
		break;
	default:
		break;
	}

	dt = obsTime.Time - pobj->LastTime[ant_id].Time + obsTime.Sec - pobj->LastTime[ant_id].Sec;
	/** Only observation files can be QC */
	if ((ant_id == VERIQC_ANT_ID_ROVER || ant_id == VERIQC_ANT_ID_BASE) && DataID == RINEX_ID_OBS
		&& IsInvailQCTimeRange(pobj, &obsTime, Status) && (dt - pobj->pOpt->Interval > -FLT_EPSILON))
	{
		for (int iEpoch = 3; iEpoch > 0; iEpoch--)
		{
			memcpy(&pobj->pObs[ant_id][iEpoch], &pobj->pObs[ant_id][iEpoch - 1], sizeof(QC_OBS_T));
		}
		/* obs convert funtion for one epoch */
		ConvertObsEpoch(&pobj->pObs[ant_id][0], (GNSS_OBS_T*)pData, GetSPPRes(ant_id), pobj, ant_id, Week, WeekMsCount / 1000.0);

		/** Record Break */
		if ((dt > QC_BREAK_TIME_LIMIT) && (pobj->QcRes.MaxEpochNum[ant_id] != 0))/**< 60s limit */
		{
			InsertObsBreakTop(&pobj->QcRes, ant_id, &pobj->LastTime[ant_id], (float)dt);
		}

		/** check loss epoch */
		while (((dt - 2.0 * pobj->pOpt->Interval) > -FLT_EPSILON) && (pobj->QcRes.MaxEpochNum[ant_id] != 0) && (dt < QC_MAX_GAP_EPOCH))
		{
			/** set temp observation */
			LossTempObsData.Time = QCTimeAdd(&pobj->LastTime[ant_id], ((double)dt - pobj->pOpt->Interval));
			LossTempObsData.ObsNum = 0;
			memcpy(LossTempObsData.StaPos, pobj->pObs[ant_id][0].StaPos, sizeof(double) * 3);
			/** detect miss sat in loss epoch */
			if (!IsNoEphModeEnabled(pobj->pOpt))
			{
				DetectMissSat(&LossTempObsData, ant_id, &pobj->QcRes, pobj->pOpt);
			}

			dt -= pobj->pOpt->Interval;
		}

		pobj->LastTime[ant_id].Time = obsTime.Time;
		pobj->LastTime[ant_id].Sec = obsTime.Sec;

		/* execute "chcdata" function */
		QCRecordEpoch(&pobj->pObs[ant_id][0], ant_id, &pobj->QcRes, pobj->pOpt);
		if (!IsNoEphModeEnabled(pobj->pOpt))
		{
			DetectMissSat(&pobj->pObs[ant_id][0], ant_id, &pobj->QcRes, pobj->pOpt);
		}

		/* print MP file */
		PrintEpochMP(&pobj->pObs[ant_id][0].Time, pobj->fpOutFileMP[ant_id], &pobj->QcRes, ant_id, E_LOG_HEADTYPE_2);
		/* print CS file */
		PrintEpochCS(&pobj->pObs[ant_id][0].Time, pobj->fpOutFileCS[ant_id], &pobj->QcRes, ant_id, E_LOG_HEADTYPE_2);

		PrintEpochDataList(&pobj->pObs[ant_id][0].Time, pobj->fpOutFileDataList[ant_id], &pobj->pObs[ant_id][0], &pobj->QcRes.SlipFlag[ant_id][0], E_LOG_HEADTYPE_2, pobj->pOpt->bIsDoubleDiffCheckLLI,
			(pobj->pOpt->EleDeg * D2R));
	}
}

/**********************************************************************//**
@brief get Rinex ID for MsgConvert decoded message

@param pMsgDecode  [In]      Message decode info
@param SourceType  [In]      Source Type

@return Rinex ID

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
int GetDataRinexID(MSG_DECODE_T* pMsgDecode, int SourceType)
{
	int DataID = 0;

	if ((SourceType == MSG_DECODE_TYPE_RTCM3) || (SourceType == MSG_DECODE_TYPE_RTCM2))
	{
		DataID = pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo->RTCMInterimData.LastTypeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_RAWMSGA)
	{
		RAWMSG_ASCII_FILTER_INFO_T* pRawMsgAsciiFilterInfo =
			(RAWMSG_ASCII_FILTER_INFO_T*)pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiFilterInfo;
		DataID = pRawMsgAsciiFilterInfo->DecodeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_RAWMSGB)
	{
		RAWMSG_BINARY_FILTER_INFO_T* pRawMsgBinaryFilterInfo =
			(RAWMSG_BINARY_FILTER_INFO_T*)pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryFilterInfo;
		DataID = pRawMsgBinaryFilterInfo->DecodeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_HRCX)
	{
		DataID = pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo->HRCXInterimData.LastTypeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_HRC)
	{
		DataID = pMsgDecode->ProtocolFilterInfo.pHRCParseInfo->HRCInterimData.LastTypeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_CHCRAWMSGB)
	{
		CHC_RAWMSG_BINARY_FILTER_INFO_T* pCHCRawMsgBinaryFilterInfo =
			(CHC_RAWMSG_BINARY_FILTER_INFO_T*)pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryFilterInfo;
		DataID = pCHCRawMsgBinaryFilterInfo->DecodeID;
	}
	else if (SourceType == MSG_DECODE_TYPE_UNRAWMSGB)
	{
		UN_RAWMSG_BINARY_FILTER_INFO_T* pUNRawMsgBinaryFilterInfo =
			(UN_RAWMSG_BINARY_FILTER_INFO_T*)pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryFilterInfo;
		DataID = pUNRawMsgBinaryFilterInfo->DecodeID;
	}

	return GetDecodeMsgRinexID(SourceType, DataID);
}


/**********************************************************************//**
@brief get Raw File Data Type

@param SourceType  [In]      Source Type
@param RinexDataID [In]      decode msg Rinex Data ID
@param pDataType   [Out]     Raw File Data Type

@return 1 success

@author CHC
@date 2026/02/06
@note
History:\n
**************************************************************************/
int DetectFileFormatByData(int SourceType, int RinexDataID, int* pDataType)
{
	/** File Data Type properity */
	int iLoop;
	int FileDataTypeProp[][2] =
	{
		{ MSG_DECODE_TYPE_RAWMSGA,    GNSS_STRFMT_RAWMSGA},
		{ MSG_DECODE_TYPE_RAWMSGB,    GNSS_STRFMT_RAWMSGB},
		{ MSG_DECODE_TYPE_CHCRAWMSGB, GNSS_STRFMT_CHCRAWMSGB},
		{ MSG_DECODE_TYPE_UNRAWMSGB,  GNSS_STRFMT_UNRAWMSGB},
		{ MSG_DECODE_TYPE_RTCM3,      GNSS_STRFMT_RTCM3},
		{ MSG_DECODE_TYPE_RTCM2,      GNSS_STRFMT_RTCM2},
		{ MSG_DECODE_TYPE_HRCX,       GNSS_STRFMT_HRCX},
		{ MSG_DECODE_TYPE_HRC,        GNSS_STRFMT_HRC}
	};

	if (RinexDataID != RINEX_ID_OBS)
	{
		return 0;
	}

	for (iLoop = 0; iLoop < ARRAY_SIZE(FileDataTypeProp); iLoop++)
	{
		if (*pDataType == FileDataTypeProp[iLoop][1])
		{
			break;
		}
		if (SourceType == FileDataTypeProp[iLoop][0])
		{
			*pDataType = FileDataTypeProp[iLoop][1];
			break;
		}
	}

	return 1;
}

/**********************************************************************//**
@brief get decoded data from MsgConvert RTCM/RawMsg/HRCX

@param pMsgDecode  [In]      Message decode info
@param pObs        [In]      Observation
@param SourceType  [In]      Source Type
@param RinexID     [In]      Rinex ID

@return Data pointer

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
void* GetDataInfo(MSG_DECODE_T* pMsgDecode, int SourceType, int RinexID)
{
	void* pDataInfo = NULL;

	switch (RinexID)
	{
	case RINEX_ID_OBS:
	case RINEX_ID_GPSEPH:
	case RINEX_ID_GALEPH:
	case RINEX_ID_GALEPH_FNAV:
	case RINEX_ID_GALEPH_INAV:
	case RINEX_ID_BDSEPH:
	case RINEX_ID_QZSSEPH:
	case RINEX_ID_GLOEPH:
	case RINEX_ID_LEOEPH:
		if ((SourceType == MSG_DECODE_TYPE_RTCM2) || (SourceType == MSG_DECODE_TYPE_RTCM3))
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pRTCMParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_RAWMSGA)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pRawMsgAsciiParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_RAWMSGB)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pRawMsgBinaryParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_HRCX)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pHRCXParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_HRC)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pHRCParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_CHCRAWMSGB)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pCHCRawMsgBinaryParseInfo->ParsedDataBuf.pBaseAddr;
		}
		else if (SourceType == MSG_DECODE_TYPE_UNRAWMSGB)
		{
			pDataInfo = (void*)pMsgDecode->ProtocolFilterInfo.pUNRawMsgBinaryParseInfo->ParsedDataBuf.pBaseAddr;
		}
		break;
	default:
		break;
	}

	return pDataInfo;
}

/**********************************************************************//**
@brief get decoded data from MsgConvert Rinex

@param pRinexInfo            [In]      Rinex info
@param RinexFileIndex        [In]      File Index
@param RinexID               [In]      Rinex data ID

@return Data pointer

@author CHC
@date 2024/07/25
@note
History:\n
**************************************************************************/
void* GetDataInfoRinex(RINEX_INFO_T* pRinexInfo, int RinexFileIndex, int RinexID)
{
	void* pDataInfo = NULL;

	switch (RinexID)
	{
	case RINEX_ID_OBS:
		pDataInfo = (void*)&pRinexInfo->ObsData;
		break;
	case RINEX_ID_GPSEPH:
	case RINEX_ID_GALEPH:
	case RINEX_ID_GALEPH_FNAV:
	case RINEX_ID_GALEPH_INAV:
	case RINEX_ID_BDSEPH:
	case RINEX_ID_BDSEPH_CNAV1:
	case RINEX_ID_BDSEPH_CNAV2:
	case RINEX_ID_BDSEPH_CNAV3:
	case RINEX_ID_QZSSEPH:/**<@TODO:: QC2 No QZSS .q file info now */
		pDataInfo = (void*)&pRinexInfo->UniEphData[RinexFileIndex];
		break;
	case RINEX_ID_GLOEPH:
		pDataInfo = (void*)&pRinexInfo->GLOEphData;
		break;
	default:
		break;
	}

	return pDataInfo;
}

/**********************************************************************//**
@brief GNSS QC main entrance

@param pGNSSCfg     [In]     GNSS config
@param pQCReader    [In/Out] Quality check reader
@param ID           [In]     0:chcdata function 1: preprocess(get interval)
@param pMsgDecode   [In]     message decoder

@return Success or failure

@author CHC
@date 2022/11/25
@note
History:
1. 2024/07/25 use MsgConvert funcs instead
**************************************************************************/
int8_t QCProcessCore(CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_READER_T* pQCReader, uint8_t ID)
{
	int32_t i, maxcnt, tcnt = 0, file_read_cnt = 0, nr;
	char Buff[QC_READ_MAX_BUFF + 1] = { 0 };
	int RinexID = 0;
	int DecodeType = 0;
	int SourceType = 0;
	void* pDataInfo = NULL;
	unsigned int FilterOffset = 0, FuncIndex;
	unsigned int ErrMask = 0;
	unsigned int WaitConfirmMask = 0;
	unsigned int ProtocolMask = ((1 << 9) - 1);/** same with protocol size !!! */
	unsigned char DecodeData = 0;
	unsigned int DataSize = 0;
	unsigned char* pBufIn = NULL;
	unsigned int DecodeDataSize = DataSize;
	int Result = 0;
	PROTOCOL_FILTER_FRAM_ERR_E ResFilter = PROTOCOL_FILTER_ERR;
	UTC_TIME_T UTCTime = { 0 };
	MSG_DECODE_T* pMsgDecode = NULL;
	QC_CODEC_INFO_T* pCodecInfo = NULL;

	void (*pQCCallBackFuncTable[])(void*, int32_t, int, void*, void*) =
	{
		QCScanStationCallBack,
		QCPreProcessCallBack,
	};
	void (*pQCCallBack)(void*, int32_t, int, void*, void*);

	pQCCallBack = pQCCallBackFuncTable[ID];

	for (i = MAX_NUM_INPUT_FILE - 1; i >= 0; i--)
	{
		pCodecInfo = (QC_CODEC_INFO_T*)pQCReader->pCodecInfo[i];
		if (pCodecInfo->StreamFormat == STRFMT_NONE)
		{
			continue;
		}

		if (!pCodecInfo->fpInputFile)
		{
			continue;
		}

		if (pCodecInfo->StreamFormat == GNSS_STRFMT_RINEX)
		{
			/* waite epoch number */
			if (pCodecInfo->EpochWait < 0.1)
			{
				continue;
			}

			if (i < QC_MAX_NUM_OBS_FILE)
			{
				maxcnt = 1;///** epoch number for  Rinex obs file
			}
			else
			{
				maxcnt = 10000;/** Rinex nav file */
			}

			do
			{
				tcnt++;

				while ((maxcnt > 0) && (!feof(pCodecInfo->fpInputFile) ||
					(CycleBufGetDataSize(&pCodecInfo->RinexInfo.CycleBufCtrl[pCodecInfo->MsgConvertRinexIndex]) > 0)))
				{
					Result = InputRinexInfo(&pCodecInfo->RinexInfo, pCodecInfo->fpInputFile, pCodecInfo->MsgConvertRinexIndex, &UTCTime, &RinexID);
					if ((Result < 0) && (i < QC_MAX_NUM_OBS_FILE))
					{
						SetQCExceptionCode(&pQCReader->QcRes.FileException[i], QC_EXCEPTION_PARSE_EPOCH_FAILED);
					}
					Result = (Result == 0) ? RinexID : Result;
					pDataInfo = GetDataInfoRinex(&pCodecInfo->RinexInfo, pCodecInfo->MsgConvertRinexIndex, Result);

					pQCCallBack(pQCReader, pQCReader->AntID[i], Result, pDataInfo, NULL);

					maxcnt--;
				}

			} while (tcnt < 2000);

			if (((feof(pCodecInfo->fpInputFile) == 0) ||
				(CycleBufGetDataSize(&pCodecInfo->RinexInfo.CycleBufCtrl[pCodecInfo->MsgConvertRinexIndex]) > 0)) &&
				(Result >= 0))
			{
				file_read_cnt++; /**< Number of files that have not ended reading */
			}
		}
		else
		{
			pMsgDecode = pCodecInfo->pMsgDecode;

			do
			{
				/** waite epoch number */
				if (!pCodecInfo->EpochWait)
				{
					break;
				}

				tcnt++;
				if (feof(pCodecInfo->fpInputFile))
				{
					nr = 0;
				}
				else
				{
					nr = (int)fread(Buff, 1, QC_READ_MAX_BUFF, pCodecInfo->fpInputFile);
				}

				SourceType = pCodecInfo->StreamFormat;
				DecodeType = GetDecodeType(SourceType);
				CycleBufPutData(&pMsgDecode->pMsgPort->CycleBuffCtrl, Buff, nr);

				if ((GetMsgFilterOffset(pMsgDecode, &FilterOffset) < 0))
				{
					break;
				}

				/** modify MsgConvert return */
				ErrMask = 0;
				WaitConfirmMask = 0;
				FilterOffset = 0;
				DecodeData = 0;
				ResFilter = PROTOCOL_FILTER_ERR;
				DataSize = CycleBufGetDataSize(&pMsgDecode->pMsgPort->CycleBuffCtrl);
				pBufIn = pMsgDecode->pMsgPort->DecodeBuf.pBaseAddr;
				//DecodeDataSize = DataSize;

				CycleBufGetData(&pMsgDecode->pMsgPort->CycleBuffCtrl, (char*)pBufIn, DataSize);
				DecodeDataSize = DataSize;

				while (DecodeDataSize > 0)
				{
					for (FuncIndex = 0; FuncIndex < 9; FuncIndex++)
					{
						if (!(DecodeType & s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType) && (ID != QC_PRE_PROCESS_FUNC_INDEX)
							|| (s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType == MSG_DECODE_TYPE_RINEX))
						{
							ErrMask |= (1 << FuncIndex);
							continue;
						}

						ResFilter = s_ProtocolDecodeFuncTabl[FuncIndex].ProtocolFilter(pMsgDecode, pBufIn, DecodeDataSize, &FilterOffset);

						if (PROTOCOL_FILTER_OK == ResFilter)
						{
							Result = s_ProtocolDecodeFuncTabl[FuncIndex].ProtocolDecode(pMsgDecode, SourceType);

							/** QC */
							if (Result > 0)
							{
								RinexID = GetDataRinexID(pMsgDecode, s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType);
								pDataInfo = GetDataInfo(pMsgDecode, s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType, RinexID);

								pQCCallBack(pQCReader, pQCReader->AntID[i], RinexID, pDataInfo, NULL);
								if ((ID == QC_PRE_PROCESS_FUNC_INDEX) && (i < QC_MAX_NUM_OBS_FILE) && (pQCReader->QcRes.FileDataType[i] == STRFMT_NONE))
								{
									/** update stream format */
									DetectFileFormatByData(s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType, RinexID, (int*)&pCodecInfo->StreamFormat);
								}
							}

							CycleBufUpdateReadAddr(&pMsgDecode->pMsgPort->CycleBuffCtrl, FilterOffset);

							pBufIn += FilterOffset;
							DataSize -= FilterOffset;
							DecodeDataSize = DataSize;
							FilterOffset = 0;
							ErrMask = 0;

							MsgFilterClearAll(pMsgDecode);
							break;
						}
						else if (PROTOCOL_WAIT_CONFIRM == ResFilter)
						{
							WaitConfirmMask |= (1 << FuncIndex);
						}
						else
						{
							ErrMask |= (1 << FuncIndex);
						}
					}

					if ((ErrMask & ProtocolMask) == ProtocolMask)
					{
						CycleBufUpdateReadAddr(&pMsgDecode->pMsgPort->CycleBuffCtrl, 1);

						pBufIn += 1;
						DataSize--;
						DecodeDataSize = DataSize;
						FilterOffset = 0;
						ErrMask = 0;

						MsgFilterClearAll(pMsgDecode);
					}

					if (WaitConfirmMask > 0)
					{
						break;
					}
				}
			} while (tcnt < 1);
			if (feof(pCodecInfo->fpInputFile) == 0)file_read_cnt++; /**< Number of files that have not ended reading */
		}

		if (i < QC_MAX_NUM_OBS_FILE)
		{
			pQCReader->CurFilePos[i] = ftell(pCodecInfo->fpInputFile);
		}
	}

	if (file_read_cnt > 0)
	{
		return 0;//continue
	}
	else
	{
		if (ID == QC_SCAN_STATION_FUNC_INDEX)
		{
			FinalizeFileExceptionBySysState(pQCReader, 0);
			InitQCExceptionInfo(&pQCReader->QcRes.FileException[1]);
		}
		return -1;//break
	}
}


/*********************************************************************
@brief  Initialization "qc_res"
@param  CHCQC_GNSSQC_RES_T*  [I0]    pQCRes
@return none
@author CHC
@date   17 January 2023
**********************************************************************/
void InitQCRes(QC_RESULT_T* pQCRes)
{
	memset(pQCRes, 0, sizeof(QC_RESULT_T));
}


/*********************************************************************
@brief  Initialization "pQcReader"
@param  QC_READER_T*  [I0]    pQCReader
@return 0 success 1 fail
@author CHC
@date   17 January 2023
**********************************************************************/
int InitQCReader(CHCQC_READER_T* pQCReader)
{
	int iFile, iSys, iSig;
	int SuccessCount = 0;

	memset(pQCReader, 0, sizeof(CHCQC_READER_T));
	memset(pQCReader->LastTime, 0, sizeof(QC_TIME_T) * QC_MAX_NUM_OBS_FILE);

	pQCReader->pOpt = (QC_OPT_T*)VERIQC_MALLOC(sizeof(QC_OPT_T));
	if (pQCReader->pOpt == NULL)
	{
		return -1;
	}

	for (iFile = 0; iFile < MAX_NUM_INPUT_FILE; iFile++)
	{
		pQCReader->pCodecInfo[iFile] = VERIQC_MALLOC(sizeof(QC_CODEC_INFO_T));
		if (pQCReader->pCodecInfo[iFile] == NULL)
		{
			break;
		}
		memset(pQCReader->pCodecInfo[iFile], 0, sizeof(QC_CODEC_INFO_T));
		SuccessCount++;
	}

	if (SuccessCount != MAX_NUM_INPUT_FILE)
	{
		for (iFile = 0; iFile < SuccessCount; iFile++)
		{
			VERIQC_FREE(pQCReader->pCodecInfo[iFile]);
			pQCReader->pCodecInfo[iFile] = NULL;
		}

		VERIQC_FREE(pQCReader->pOpt);
		pQCReader->pOpt = NULL;

		return -1;
	}

	for (iFile = 0; iFile < QC_MAX_NUM_OBS_FILE; iFile++)
	{
		pQCReader->QcRes.Interval[iFile] = 86400;
		pQCReader->QcRes.FileDataType[iFile] = STRFMT_NONE;
		InitQCExceptionInfo(&pQCReader->QcRes.FileException[iFile]);
		((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[iFile])->StreamFormat = STRFMT_NONE;
		for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
		{
			InitQCExceptionInfo(&pQCReader->QcRes.SysException[iFile][iSys]);
			for (iSig = 0; iSig < QC_MAX_SIG_TYPE; iSig++)
			{
				pQCReader->QcRes.SysSigType[iFile][iSys][iSig] = 0;
			}
		}
		pQCReader->QcRes.AvgHealthyArcDurationSec[iFile] = 0.0f;
		pQCReader->QcRes.MaxHealthyArcDurationSec[iFile] = 0.0f;
		pQCReader->QcRes.HealthyArcEpochCountTemp[iFile] = 0;
		memset(&pQCReader->QcRes.HealthyArcStartTimeTemp[iFile], 0, sizeof(QC_TIME_T));
		pQCReader->QcRes.HealthyArcDurationSumSec[iFile] = 0.0;
		pQCReader->QcRes.HealthyArcCount[iFile] = 0;
		pQCReader->QcRes.SlipOutlierWindowCount[iFile] = 0;
		pQCReader->QcRes.ObsBreakCount[iFile] = 0;
		pQCReader->QcRes.LowCN0SatRatio[iFile] = 0.0f;
		pQCReader->QcRes.LowCN0EpochRatio[iFile] = 0.0f;
		pQCReader->QcRes.CN0DropWindowCount[iFile] = 0;
		for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
		{
			for (iSig = 0; iSig < QC_NUM_FREQ; iSig++)
			{
				int iGroup;
				for (iGroup = 0; iGroup < MP_GROUP_PERCENT_SIZE; iGroup++)
				{
					if (iSys == 0)
					{
						pQCReader->QcRes.MPGroupPercentAve[iFile][iSig][iGroup] = 0.0f;
					}
				}
				for (iGroup = 0; iGroup < MP_GROUP_PERCENT_SIZE; iGroup++)
				{
					pQCReader->QcRes.MPGroupPercent[iFile][iSys][iSig][iGroup] = 0.0f;
					pQCReader->QcRes.MPGroupNum[iFile][iSys][iSig][iGroup] = 0;
				}
			}
		}
	}

	for (iSys = 0; iSys < 2; iSys++)
	{
		for (iSig = 0; iSig < 3; iSig++)
		{
			pQCReader->QcRes.PosXYZ[iSys][iSig] = 0;
		}
	}

	InitGNSSCfg((CHCQC_GNSSCFG_T*)pQCReader->pOpt);

	return 0;
}

void FreeQCReader(CHCQC_READER_T* pQCReader)
{
	int iFile;
	QC_CODEC_INFO_T* pCodecInfo = NULL;

	for (iFile = 0; iFile < QC_MAX_NUM_OBS_FILE; iFile++)
	{
		pCodecInfo = (QC_CODEC_INFO_T*)pQCReader->pCodecInfo[iFile];
		if (pQCReader->fpOutFileCS[iFile])
		{
			fclose(pQCReader->fpOutFileCS[iFile]);
			pQCReader->fpOutFileCS[iFile] = NULL;
		}
		if (pQCReader->fpOutFileMP[iFile])
		{
			fclose(pQCReader->fpOutFileMP[iFile]);
			pQCReader->fpOutFileMP[iFile] = NULL;
		}
		if (pQCReader->fpOutFileDataList[iFile])
		{
			fclose(pQCReader->fpOutFileDataList[iFile]);
			pQCReader->fpOutFileDataList[iFile] = NULL;
		}

		MsgCodecDeinit(pCodecInfo->pMsgDecode);
	}

	for (iFile = 0; iFile < MAX_NUM_INPUT_FILE; iFile++)
	{
		pCodecInfo = (QC_CODEC_INFO_T*)pQCReader->pCodecInfo[iFile];
		if (pCodecInfo->fpInputFile)
		{
			fclose(pCodecInfo->fpInputFile);
			pCodecInfo->fpInputFile = NULL;
		}

		VERIQC_FREE(pQCReader->pCodecInfo[iFile]);
		pQCReader->pCodecInfo[iFile] = NULL;
	}

	VERIQC_FREE(pQCReader->pOpt); pQCReader->pOpt = NULL;
	VERIQC_FREE(pQCReader); pQCReader = NULL;

	ResetQCSlipInfo();
}

void ResetQCReader(CHCQC_READER_T* pQCReader)
{
	int iAnt;
	memset(pQCReader->LastTime, 0, sizeof(QC_TIME_T) * QC_MAX_NUM_OBS_FILE);
	for (iAnt = 0; iAnt < QC_MAX_NUM_OBS_FILE; iAnt++)
	{
		memset(pQCReader->pObs[iAnt], 0, sizeof(QC_OBS_T) * 4);
	}
	ResetQCfp(pQCReader);
}

void ResetQCfp(CHCQC_READER_T* pQCReader)
{
	int8_t i;
	QC_CODEC_INFO_T* pCodecInfo = NULL;

	for (i = MAX_NUM_INPUT_FILE - 1; i >= 0; i--)
	{
		pCodecInfo = (QC_CODEC_INFO_T*)pQCReader->pCodecInfo[i];
		if (pCodecInfo->StreamFormat == STRFMT_NONE)continue;
		if (pCodecInfo->fpInputFile)
		{
			fseek(pCodecInfo->fpInputFile, 0, SEEK_SET);
		}
	}
}

extern int IsInvailQCTimeRange(const CHCQC_READER_T* pObj, const QC_TIME_T* pCurTime, int ID)
{
	double dt;
	QC_TIME_T ptempTime1 = { 0 }, ptempTime2 = { 0 };
	QC_CODEC_INFO_T* pCodecInfo = (QC_CODEC_INFO_T*)pObj->pCodecInfo[ID];

	ptempTime1.Time = pCurTime->Time;
	ptempTime1.Sec = pCurTime->Sec;
	ptempTime2.Time = pObj->pOpt->StartTime.Time;
	ptempTime2.Sec = pObj->pOpt->StartTime.Sec;
	if (pObj->pOpt->StartTime.Time > 0)
	{
		dt = QCTimeDiff(&ptempTime1, &ptempTime2);
		if (dt < 0) return 0;
	}
	if (pObj->pOpt->EndTime.Time > 0)
	{
		ptempTime2.Time = pObj->pOpt->EndTime.Time;
		ptempTime2.Sec = pObj->pOpt->EndTime.Sec;
		dt = QCTimeDiff(&ptempTime1, &ptempTime2);
		if (dt > 0)
		{
			if (pCodecInfo->fpInputFile)
			{
				fseek(pCodecInfo->fpInputFile, 0, SEEK_END);
			}
			return 0;
		}
	}
	return 1;
}

/**********************************************************************//**
@brief Convert to the observation and azel information required by the QC library

@param pObs         [In/Out]     obs and azel for QC
@param pObsSrc      [In]         observation decode by MsgConvert
@param pRes         [In]         azel from VERIQC
@param pQcReader    [In]         Quality check reader
@param AntID        [In]         Antenna ID
@param Week         [In]         GPST week
@param Second       [In]         GPST second of week

@author CHC
@date
@note
History:
1. 2024/07/25 modify interface, use GNSS_OBS_T instead
**************************************************************************/
void ConvertObsEpoch(QC_OBS_T* pObs, const GNSS_OBS_T* pObsSrc, VERIQC_SIMPLIFY_RESULT_T* pRes, CHCQC_READER_T* pQcReader, int AntID, int Week, double Second)
{
	int iSat, iSat0, banNum = 0, FreID, ant_id_other = 0;
	unsigned char invail[QC_MAX_NUM_SAT] = { 0 };
	int SatIndex[QC_MAX_NUM_SAT] = { 0 };/** NOTE: INDEX from 1 */
	double m_pos[3] = { 0 }, e[3] = { 0 }, azel[2] = { 0 };
	char s_prn[4] = { 0 };
	int iFre0, bFreVail, numFre, tarFre = 0;
	int isFind = 0;
	VERIQC_SIMPLIFY_RESULT_T* pDynamicRes = NULL;
	QC_TIME_T Time = { 0 };
	QC_TIME_T SatTransTime = { 0 };
	unsigned int RangeSysID = 0;
	unsigned int RangeSigID = 0;
	unsigned int RangeSatID = 0;
	unsigned int QCSysMask = 0;
	unsigned int QCSysID = 0;
	unsigned int QCSatID = 0;
	unsigned int iObs = 0;
	int QCSatNo = 0;
	int QCSigID = 0;
	int QCCode = 0;
	int PhaseLock = 0;
	int iFre;
	BOOL bNoEphMode = FALSE;
	BOOL bHasTruePos = FALSE;
	int Toe = 0;
	double SatPosVel[6] = { 0 };
	double SatClk[3] = { 0 };

#if 0
	char tstr[64] = { 0 };
	QC_Time2Str(&pObs->time, tstr, 1);
#endif

	if (pObs == NULL || pObsSrc == NULL || pRes == NULL)
	{
		return;
	}
	bNoEphMode = IsNoEphModeEnabled(pQcReader->pOpt);

	memset(pObs, 0, sizeof(QC_OBS_T));
	memcpy(invail, pQcReader->pOpt->BanPRN, sizeof(unsigned char) * QC_MAX_NUM_SAT);

	Time = GPSTime2QCTime(Week, Second);
	pObs->Time.Time = Time.Time;
	pObs->Time.Sec = Time.Sec;
	pObs->ObsNumMax = QC_MAX_NUM_OBS;

	/** save UsedSatNum and Dop for UI */
	pQcReader->EpochSolInfo.Time = pObs->Time;
	if (!bNoEphMode)
	{
		pQcReader->EpochSolInfo.UsedSatNum = pRes->UsedSatNum;
		memcpy(pQcReader->EpochSolInfo.Dop, pRes->Dop, sizeof(float) * 5);
	}
	else
	{
		pQcReader->EpochSolInfo.UsedSatNum = 0;
		memset(pQcReader->EpochSolInfo.Dop, 0, sizeof(float) * 5);
	}

	if (pQcReader->pOpt->IsInputTrueXYZ == 0)
	{
		if (bNoEphMode)
		{
			memset(pRes->Pos, 0, sizeof(double) * 3);
		}
	}
	else
	{
		if ((AntID == VERIQC_ANT_ID_ROVER) && fabs(pQcReader->pOpt->RoverTrueXYZ[0]) > 1 && fabs(pQcReader->pOpt->RoverTrueXYZ[1]) > 1 && fabs(pQcReader->pOpt->RoverTrueXYZ[2]) > 1)
		{
			memcpy(&pRes->Pos, &pQcReader->pOpt->RoverTrueXYZ, sizeof(double) * 3);
			bHasTruePos = TRUE;
		}

		if ((AntID == VERIQC_ANT_ID_BASE) && fabs(pQcReader->pOpt->BaseTrueXYZ[0]) > 1 && fabs(pQcReader->pOpt->BaseTrueXYZ[1]) > 1 && fabs(pQcReader->pOpt->BaseTrueXYZ[2]) > 1)
		{
			memcpy(&pRes->Pos, &pQcReader->pOpt->BaseTrueXYZ, sizeof(double) * 3);
			bHasTruePos = TRUE;
		}
		if (bNoEphMode && !bHasTruePos)
		{
			memset(pRes->Pos, 0, sizeof(double) * 3);
		}
	}
	if (!bNoEphMode)
	{
		PosXYZ2LLH(pRes->Pos, m_pos);/**< realtime position */
	}

	for (iObs = 0; iObs < pObsSrc->RangeData.ObsNum; iObs++)
	{
		RangeSysID = GetRangeSystem(pObsSrc->RangeData.ObsData[iObs].ChnTrackStatus);
		QCSysMask = g_RangeToQCSysMask[RangeSysID];
		QCSysID = QCSys2Id(QCSysMask);
		QCSatID = pObsSrc->RangeData.ObsData[iObs].PRN + g_RangeToQCSatPRN[RangeSysID];
		if (QCSysMask == QC_SYS_QZS)
		{
			QCSatNo = GetQCSatNo(QC_SYS_GPS, QCSatID);
		}
		else
		{
			QCSatNo = GetQCSatNo(QCSysMask, QCSatID);
		}

		if ((QCSatNo <= 0) || (RangeSysID == RM_RANGE_SYS_NIC) || (RangeSysID == RM_RANGE_SYS_SBA))
		{
			continue;
		}

		if (SatIndex[QCSatNo - 1] == 0)
		{
			SatIndex[QCSatNo - 1] = pObs->ObsNum + 1;
			pObs->ObsNum++;

			if (!pQcReader->QcRes.SysException[AntID][QCSysID - 1].IsHaveObs)
			{
				pQcReader->QcRes.SysException[AntID][QCSysID - 1].IsHaveObs = TRUE;
			}
		}

		iSat = SatIndex[QCSatNo - 1] - 1;

		pObs->Data[iSat].Sat = QCSatNo;
		pObs->Data[iSat].Sys = QCSysMask;

		if (QCSysID == QC_GNSS_SYSID_GLN)
		{
			if (bNoEphMode)
			{
				s_FreqGLO[QCSatID] = pObsSrc->RangeData.ObsData[iObs].FreqNum - 7;
			}
			pObs->Data[iSat].GLOFreq = s_FreqGLO[QCSatID];
		}
		pObs->Data[iSat].PRN = QCSatID;

		RangeSigID = GetRangeSignalType(pObsSrc->RangeData.ObsData[iObs].ChnTrackStatus);
		QCCode = RangeToQCSigID(RangeSysID, RangeSigID);
		FreID = QCCode2FreID(QCSysMask, QCCode);
		if (FreID < 0)
		{
			continue;
		}

		if ((pObs->Data[iSat].P[FreID] != 0) && (pObs->Data[iSat].L[FreID] != 0))
		{
			continue;
		}

		pObs->Data[iSat].NumFre++;

		if (pQcReader->pOpt->AllFreq == 0)/**< freqList from user */
		{
			bFreVail = 0;
			numFre = pQcReader->pOpt->NumSigType[QCSysID - 1];
			for (iFre0 = 0; iFre0 < numFre; iFre0++)
			{
				tarFre = pQcReader->pOpt->SysSigType[QCSysID - 1][iFre0];
				if (QCCode == tarFre)
				{
					bFreVail = 1;
					break;
				}
			}
			if (bFreVail)
			{
				continue;
			}
		}
		else if (pQcReader->pOpt->AllFreq == 2)
		{
			tarFre = pQcReader->pOpt->SysSigType[QCSysID - 1][FreID];
			if (tarFre)
			{
				continue;
			}
		}

		if (fabs(pObsSrc->RangeData.ObsData[iObs].PSR) >= DBL_EPSILON)
		{
			pObs->Data[iSat].P[FreID] = pObsSrc->RangeData.ObsData[iObs].PSR;
		}
		if (fabs(pObsSrc->RangeData.ObsData[iObs].ADR) >= DBL_EPSILON)
		{
			pObs->Data[iSat].Code[FreID] = QCCode;
			pObs->Data[iSat].L[FreID] = fabs(pObsSrc->RangeData.ObsData[iObs].ADR); /**< default positive */
		}

		pObs->Data[iSat].D[FreID] = pObsSrc->RangeData.ObsData[iObs].Doppler;
		if (pObsSrc->RangeData.ObsData[iObs].CN0 > 1)
		{
			pObs->Data[iSat].SNR[FreID] = pObsSrc->RangeData.ObsData[iObs].CN0;
		}
		else if (pObs->Data[iSat].SNR[FreID] > 1)
		{

		}
		else
		{
			pObs->Data[iSat].SNR[FreID] = 0;
		}

		if (!GET_CH_TRACK_STATUS_PHASELOCK(pObsSrc->RangeData.ObsData[iObs].ChnTrackStatus))
		{
			pObs->Data[iSat].LLI[FreID] = 1;
		}
		if (pObs->Data[iSat].L[FreID] < 1e-15)
		{
			pObs->Data[iSat].LLI[FreID] = 0;
		}
	}

	memcpy(pObs->StaPos, pRes->Pos, sizeof(double) * 3);

	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		if (bNoEphMode)
		{
			pObs->Data[iSat].Azel[0] = QC_INVALID_AZEL_VALUE;
			pObs->Data[iSat].Azel[1] = QC_INVALID_AZEL_VALUE;
			if (!(pObs->Data[iSat].Sys & pQcReader->pOpt->Sys))
			{
				invail[pObs->Data[iSat].Sat - 1] = 1;
			}
			continue;
		}
		isFind = 0;
		for (iSat0 = 0; (iSat0 < (int)pRes->Num) && (iSat0 < QC_MAX_NUM_SAT); iSat0++)
		{
			/** find sat position for obs */
			if (pRes->pSatNo[iSat0] == pObs->Data[iSat].Sat)
			{
				pObs->Data[iSat].Dts[0] = pRes->pDts[iSat0 * 2 + 0];
				pObs->Data[iSat].Dts[1] = pRes->pDts[iSat0 * 2 + 1];
				pObs->Data[iSat].R = GeoDist(pRes->pSatCoors + (size_t)iSat0 * 6, pRes->Pos, e);
				memcpy(pObs->Data[iSat].E, e, sizeof(double) * 3);
				memcpy(pObs->Data[iSat].SatVelXYZ, pRes->pSatCoors + (size_t)iSat0 * 6 + 3, sizeof(double) * 3);

				if (pObs->Data[iSat].R < 0)
				{
					invail[pObs->Data[iSat].Sat - 1] = 1;
				}
				else
				{
					QCSysID = QCSys2Id(pObs->Data[iSat].Sys);
					if (pQcReader->QcRes.SysException[AntID][QCSysID - 1].IsAllEpochSPPUnsolved)
					{
						pQcReader->QcRes.SysException[AntID][QCSysID - 1].IsAllEpochSPPUnsolved = FALSE;
					}
				}

				QCSatAzEl(m_pos, e, pObs->Data[iSat].Azel);
				if (!(pObs->Data[iSat].Sys & pQcReader->pOpt->Sys))
				{
					invail[pObs->Data[iSat].Sat - 1] = 1;
				}

				isFind = 1;
				break;
			}
		}

		if (!isFind && ((pObs->Data[iSat].Sys == QC_SYS_XW1) && (pRes->Num != 0)) && (pObs->Data[iSat].Sys & pQcReader->pOpt->Sys))/** XW1 Special */
		{
			for (iFre = 0; iFre < QC_NUM_FREQ; iFre++)
			{
				if (pObs->Data[iSat].P[iFre] != 0.0)
				{
					SatTransTime = QCTimeAdd(&pObs->Time, -(float)(pObs->Data[iSat].P[iFre] / CLIGHT));
					break;
				}
			}
			if (iFre < QC_NUM_FREQ)
			{
				/** calculate LEO pos */
				if (GetLEOSatPosVelClk(SatTransTime, pObs->Time, AntID, pObs->Data[iSat].Sat, SatPosVel, SatClk, &Toe) == 0)
				{
					pObs->Data[iSat].R = GeoDist(SatPosVel, pObs->StaPos, e);
					QCSatAzEl(m_pos, e, pObs->Data[iSat].Azel);
					memcpy(pObs->Data[iSat].Dts, SatClk, sizeof(double) * 2);
					memcpy(pObs->Data[iSat].E, e, sizeof(double) * 3);
					memcpy(pObs->Data[iSat].SatVelXYZ, SatPosVel + 3, sizeof(double) * 3);

					if (pObs->Data[iSat].R < 0)
					{
						invail[pObs->Data[iSat].Sat - 1] = 1;
					}

					isFind = 1;
				}
			}
		}

		if (isFind)
		{
			if (pObs->Data[iSat].Azel[1] <= pQcReader->pOpt->EleDeg * D2R - 1E-4)
			{
				invail[pObs->Data[iSat].Sat - 1] = 1;
			}
		}
		else
		{
			invail[pObs->Data[iSat].Sat - 1] = 1;//find error
		}
	}

	for (iSat = 0; iSat < pObs->ObsNum; iSat++)
	{
		if (invail[pObs->Data[iSat].Sat - 1] == 1)
		{
			for (iSat0 = iSat; iSat0 < pObs->ObsNum; iSat0++)
			{
				memcpy(&pObs->Data[iSat0], &pObs->Data[iSat0 + 1], sizeof(QC_OBSD_T));
			}
			pObs->ObsNum -= 1;
			banNum++;
			iSat--;
		}
	}
}

/** Extract file type according to file suffix */
int DetectFileFormatBySuffix(const char* pFile)
{
	FILE* fp;
	char buff[1025];
	char LowerSuffix[16];
	int Ver;
	unsigned int Index;
	const char* pSuffix;

	if (!pFile)
	{
		return STRFMT_NONE;
	}

	/*match exact file suffix (case-insensitive)*/
	pSuffix = strrchr(pFile, '.');
	if ((pSuffix != NULL) && (strlen(pSuffix) < sizeof(LowerSuffix)))
	{
		for (Index = 0; pSuffix[Index] != '\0'; Index++)
		{
			LowerSuffix[Index] = (char)tolower((unsigned char)pSuffix[Index]);
		}
		LowerSuffix[Index] = '\0';

		if (strcmp(LowerSuffix, ".rtcm3") == 0)
		{
			return GNSS_STRFMT_RTCM3;
		}
		else if (strcmp(LowerSuffix, ".hrc") == 0)
		{
			return GNSS_STRFMT_HRC;
		}
		else if (strcmp(LowerSuffix, ".rtcm2") == 0)
		{
			return GNSS_STRFMT_RTCM2;
		}
	}

	if (!(fp = fopen(pFile, "rb")))
	{
		PrintfQC("<GNSSQC error>: fopen fail:%s.\n", pFile);
		//PrintfQC("<GNSSQC error>: fopen fail Error: %d (%s)\n", errno, strerror(errno));
		return STRFMT_NONE;
	}
	buff[0] = '\0';
	fgets(buff, 1024, fp);

	if (strstr(buff, "HUACENAV COLLECTED DATA FILE"))/*HCN file*/
	{
		buff[0] = '\0';
		fgets(buff, 1024, fp);
		if (strlen(buff) > 4U)
		{
			Ver = (int)atof(buff + 4);
			fclose(fp);
			return GetOemType(Ver);
		}
	}
	else if (strstr(buff, "RINEX VERSION / TYPE"))/*Rinex*/
	{
		fclose(fp);
		return GNSS_STRFMT_RINEX;
	}

	fclose(fp);
	return STRFMT_TEMP;
}

/**********************************************************************//**
@brief  set GNSS pre-process progress

@param Value   [In] progress value

@author CHC
@date 2026/02/09
@note
**************************************************************************/
void SetGNSSPreProcessProgress(double Value)
{
	if ((Value >= 0.0) && ((Value <= 100.0)))
	{
		s_GNSSPreProcessProgress = Value;
	}
}

/**********************************************************************//**
@brief  get GNSS pre-process progress

@return progress

@author CHC
@date 2026/02/09
@note
**************************************************************************/
double GetGNSSPreProcessProgress()
{
	return	s_GNSSPreProcessProgress;
}


