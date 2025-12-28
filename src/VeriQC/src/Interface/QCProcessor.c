/*************************************************************************
		   VeriQC

	Interface Module
*-
@file   QCProcessor.h
@author CHC
@date   1st January 2023
@brief  Some Algorithm common function for QC
*****************************************************************************/

#include <float.h>
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

#define QC_READ_MAX_BUFF     QC_MIN(64, OBS_CODEC_BUF_SIZE)

static int s_FreqGLO[QC_NUM_SAT_GLO] = { 0 };

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

	if (pQcReader->pOpt->RawFileDataType == STRFMT_NONE)
	{
		type = DetectFileFormat(pFileName);
	}
	else
	{
		type = pQcReader->pOpt->RawFileDataType;
	}

	if (type == STRFMT_NONE)
	{
		if (strlen(pFileName) > 0)
		{
			PrintfQC("<GNSSQC error>: Unknown data format.\n");
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
			return;
		}

		switch (type)
		{
		case GNSS_STRFMT_RTCM2:
		case GNSS_STRFMT_RTCM3:
			if ((type == GNSS_STRFMT_RTCM2 || type == GNSS_STRFMT_RTCM3) && RTCMTime.Time == 0)
			{
				PrintfQC("<GNSSQC warning>: no approx time for rtcm decoding\n");
			}
			break;
		case GNSS_STRFMT_HRCX:
		case GNSS_STRFMT_RAWMSGA:
		case GNSS_STRFMT_RAWMSGB:
		case GNSS_STRFMT_CHCRAWMSGB:
		case GNSS_STRFMT_UNRAWMSGB:
			break;
		case GNSS_STRFMT_RINEX:
			CycleBufInit(&pCodecInfo->RinexInfo.CycleBufCtrl[RINEX_FILE_INDEX_OBS], "RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)pCodecInfo->RinexInfo.RinexDataBuf[RINEX_FILE_INDEX_OBS], 0, MsgRecv);
			pCodecInfo->MsgConvertRinexIndex = RINEX_FILE_INDEX_OBS;
			Result = OpenRinexInfo(&pCodecInfo->RinexInfo, pCodecInfo->fpInputFile, RINEX_FILE_INDEX_OBS);
			SaveRinexObsHeaderInfo(&pCodecInfo->RinexInfo.ObsHeaderInfo.ObsHeader, &pQcReader->RinexObsHeaderInfo);
			if (Result == -1)
			{
				fclose(pCodecInfo->fpInputFile);
				pCodecInfo->fpInputFile = NULL;
			}
			
			break;
		default:
			PrintfQC("<GNSSQC error>: Unknown data format.\n");
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
BOOL InitNavCodec(char* pFileName, CHCQC_READER_T* pQcReader, uint8_t OrderNumber)
{
	QC_CODEC_INFO_T* pCodecInfo = NULL;
	char fileNav[512] = { 0 };
	size_t len = 0;
	int i = 0, ord_num;
	BOOL invail_nav = FALSE;
	FILE* fp = NULL;
	char navType[] = { 'p','P' ,'n', 'N','G', 'g','L', 'l','C', 'c' };//in GREC order
	int FileIndex[] = { 
		RINEX_FILE_INDEX_MUTIEPH, RINEX_FILE_INDEX_MUTIEPH,
		RINEX_FILE_INDEX_GPSEPH, RINEX_FILE_INDEX_GPSEPH,
		RINEX_FILE_INDEX_GLOEPH, RINEX_FILE_INDEX_GLOEPH,
		RINEX_FILE_INDEX_GALEPH, RINEX_FILE_INDEX_GALEPH,
		RINEX_FILE_INDEX_BDSEPH, RINEX_FILE_INDEX_BDSEPH };
	int id = -1;
	
	memcpy(fileNav, pFileName, sizeof(char) * 512);

	for (i = 0; i < 2; i++)
	{
		len = strlen(fileNav);
		fileNav[len - 1] = navType[i];
		if ((fp = fopen(fileNav, "rb")))
		{
			invail_nav = TRUE;
			break;
		}
		else
		{
		}
	}

	/** If have mixed nav, use mixed nav. If not, search single system nav */
	if (TRUE == invail_nav)
	{
		pCodecInfo = (QC_CODEC_INFO_T*)pQcReader->pCodecInfo[OrderNumber];
		/** there is mix nav file: fileNav(fp) */
		CycleBufInit(&((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[OrderNumber])->RinexInfo.CycleBufCtrl[RINEX_FILE_INDEX_MUTIEPH],
			"RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[OrderNumber])->RinexInfo.RinexDataBuf[RINEX_FILE_INDEX_MUTIEPH], 0, MsgRecv);
		OpenRinexInfo(&((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[OrderNumber])->RinexInfo, fp, RINEX_FILE_INDEX_MUTIEPH);
		pCodecInfo->fpInputFile = fp;
		pCodecInfo->MsgConvertRinexIndex = RINEX_FILE_INDEX_MUTIEPH;
		pCodecInfo->StreamFormat = GNSS_STRFMT_RINEX;
		pCodecInfo->EpochWait = 1;
		if (OrderNumber<7)
		{
			pQcReader->AntID[OrderNumber] = VERIQC_ANT_ID_ROVER;
		}
		else
		{
			pQcReader->AntID[OrderNumber] = VERIQC_ANT_ID_BASE;
		}
	}
	else
	{
		for (i = 2; i < 10; i++)
		{
			ord_num = OrderNumber + (int)(i / 2);
			len = strlen(fileNav);
			fileNav[len - 1] = navType[i];
			if ((fp = fopen(fileNav, "rb")))
			{
				pCodecInfo = (QC_CODEC_INFO_T*)pQcReader->pCodecInfo[ord_num];
				/** there is nav file: fileNav(fp) */
				CycleBufInit(&((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[ord_num])->RinexInfo.CycleBufCtrl[FileIndex[i]],
					"RinexCycle", RINEX_DATA_CYCLE_BUF_SIZE, (char*)((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[ord_num])->RinexInfo.RinexDataBuf[FileIndex[i]], 0, MsgRecv);
				OpenRinexInfo(&((QC_CODEC_INFO_T*)pQcReader->pCodecInfo[ord_num])->RinexInfo, fp, FileIndex[i]);
				pCodecInfo->fpInputFile = fp;
				pCodecInfo->MsgConvertRinexIndex = FileIndex[i];
				pCodecInfo->StreamFormat = GNSS_STRFMT_RINEX;
				pCodecInfo->EpochWait = 1;
				invail_nav = TRUE;
				if (OrderNumber < 7)
				{
					pQcReader->AntID[ord_num] = VERIQC_ANT_ID_ROVER;
				}
				else
				{
					pQcReader->AntID[ord_num] = VERIQC_ANT_ID_BASE;
				}
			}
			else
			{
			}
		}
	}

	return invail_nav;
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
	PrintfQC("<GNSSQC info>: initCodec  base:[%s]\n", pGNSSCfg->BaseFile);
#endif
	InitObsCodec(pGNSSCfg->BaseFile, pQCReader, 1, 1, Time);
	/** find and decode navigation file in rinex format*/
	if (((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[0])->StreamFormat == GNSS_STRFMT_RINEX)
	{
		invail_nav_r = InitNavCodec(pGNSSCfg->RoverFile, pQCReader, 2);/**< 2~6 p n g i c for rover*/
		if (invail_nav_r == FALSE)
		{
			PrintfQC("<GNSSQC error>: Rover station navigation file is missing.\n");
		}
	}
	else
	{
	}
	if (((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[1])->StreamFormat == GNSS_STRFMT_RINEX)
	{
		invail_nav_b = InitNavCodec(pGNSSCfg->BaseFile, pQCReader, 7);/**< 7~11 p n g i c for base*/
		if (invail_nav_b == FALSE)
		{
			PrintfQC("<GNSSQC error>: Base station navigation file is missing.\n");
		}
	}
	else
	{
	}

	/** get file interval */
	if (pGNSSCfg->Interval < QC_MIN_SAMPLE_INTERVAL)
	{
#ifdef _DEBUG
		PrintfQC("<GNSSQC info>: getting file interval...\r\n");
#endif
		while (TRUE)
		{
			if (QCProcessCore(pGNSSCfg, pQCReader, QC_PRE_PROCESS_FUNC_INDEX) < 0)
			{
				break;
			}
		}
		pQCReader->pOpt->Interval = (pQCReader->QcRes.Interval[0] > pQCReader->QcRes.Interval[1]) ? pQCReader->QcRes.Interval[1] : pQCReader->QcRes.Interval[0];
		ResetQCReader(pQCReader);
	}
	else
	{
		pQCReader->QcRes.Interval[0] = pGNSSCfg->Interval;
		pQCReader->QcRes.Interval[1] = pGNSSCfg->Interval;
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
	/** Only observation files can be QC */
	if ((ant_id == VERIQC_ANT_ID_ROVER || ant_id == VERIQC_ANT_ID_BASE) && DataID == RINEX_ID_OBS
		&& IsInvailQCTimeRange(pobj, &obsTime, Status) && (dt - pobj->pOpt->Interval > -FLT_EPSILON))
	{
		pobj->LastTime[ant_id].Time = obsTime.Time;
		pobj->LastTime[ant_id].Sec = obsTime.Sec;

		/** get Interval */
		if (pobj->pOpt->Interval > QC_MIN_SAMPLE_INTERVAL)
		{
			pobj->QcRes.Interval[ant_id] = pobj->pOpt->Interval;
		}
		else
		{
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

	switch (DataID)
	{
	case RINEX_ID_OBS:
		UTCToGPSTime(&((GNSS_OBS_T*)pData)->ObsTime, NULL, &Week, &WeekMsCount);
		obsTime = GPSTime2QCTime(Week, WeekMsCount / 1000.0);
		GetEpochResultSPP(&((GNSS_OBS_T*)pData)->RangeData, ant_id);
		break;
	case RINEX_ID_GPSEPH:
	case RINEX_ID_GALEPH:
	case RINEX_ID_GALEPH_FNAV:
	case RINEX_ID_GALEPH_INAV:
	case RINEX_ID_BDSEPH:
	case RINEX_ID_BDSEPH_CNAV1:
	case RINEX_ID_BDSEPH_CNAV2:
	case RINEX_ID_BDSEPH_CNAV3:
	case RINEX_ID_QZSSEPH:
		AddEphSPPCore((UNI_EPHEMERIS_T*)pData);
		break;
	case RINEX_ID_GLOEPH:
		AddGephSPPCore((GLO_EPHEMERIS_T*)pData);
		glo_prn = ((GLO_EPHEMERIS_T*)pData)->SatID;
		if (glo_prn >= QC_MIN_PRN_GLO && glo_prn <= QC_MAX_PRN_GLO)
		{
			s_FreqGLO[glo_prn] = ((GLO_EPHEMERIS_T*)pData)->Freq;
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

		/** check loss epoch */
		while (((dt - 2.0 * pobj->pOpt->Interval) > -FLT_EPSILON) && (pobj->QcRes.MaxEpochNum[ant_id] != 0))
		{
			/** set temp observation */
			LossTempObsData.Time = QCTimeAdd(&pobj->LastTime[ant_id], ((float)dt - pobj->pOpt->Interval));
			LossTempObsData.ObsNum = 0;
			memcpy(LossTempObsData.StaPos, pobj->pObs[ant_id][0].StaPos, sizeof(double) * 3);
			/** detect miss sat in loss epoch */
			DetectMissSat(&LossTempObsData, ant_id, &pobj->QcRes, pobj->pOpt);

			dt -= pobj->pOpt->Interval;
		}

		pobj->LastTime[ant_id].Time = obsTime.Time;
		pobj->LastTime[ant_id].Sec = obsTime.Sec;

		/* execute "chcdata" function */
		QCRecordEpoch(&pobj->pObs[ant_id][0], ant_id, &pobj->QcRes, pobj->pOpt);
		DetectMissSat(&pobj->pObs[ant_id][0], ant_id, &pobj->QcRes, pobj->pOpt);

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
	unsigned int ProtocolMask = ((1 << 8) - 1);/** same with protocol size !!! */
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
					for (FuncIndex = 0; FuncIndex < 8; FuncIndex++)
					{
						if (!(DecodeType & s_ProtocolDecodeFuncTabl[FuncIndex].DecodeType))
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
		((QC_CODEC_INFO_T*)pQCReader->pCodecInfo[iFile])->StreamFormat = STRFMT_NONE;
		for (iSys = 0; iSys < QC_MAX_NUM_SYS; iSys++)
		{
			for (iSig = 0; iSig < QC_MAX_SIG_TYPE; iSig++)
			{
				pQCReader->QcRes.SysSigType[iFile][iSys][iSig] = 0;
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

#if 0
	char tstr[64] = { 0 };
	QC_Time2Str(&pObs->time, tstr, 1);
#endif

	if (pObs == NULL || pObsSrc == NULL || pRes == NULL)
	{
		return;
	}

	memset(pObs, 0, sizeof(QC_OBS_T));
	memcpy(invail, pQcReader->pOpt->BanPRN, sizeof(unsigned char) * QC_MAX_NUM_SAT);

	Time = GPSTime2QCTime(Week, Second);
	pObs->Time.Time = Time.Time;
	pObs->Time.Sec = Time.Sec;
	pObs->ObsNumMax = QC_MAX_NUM_OBS;

	/** save UsedSatNum and Dop for UI */
	pQcReader->EpochSolInfo.Time = pObs->Time;
	pQcReader->EpochSolInfo.UsedSatNum = pRes->UsedSatNum;
	memcpy(pQcReader->EpochSolInfo.Dop, pRes->Dop, sizeof(float) * 5);

	if (pQcReader->pOpt->IsInputTrueXYZ == 0)
	{
	}
	else
	{
		if ((AntID == VERIQC_ANT_ID_ROVER) && fabs(pQcReader->pOpt->RoverTrueXYZ[0]) > 1 && fabs(pQcReader->pOpt->RoverTrueXYZ[1]) > 1 && fabs(pQcReader->pOpt->RoverTrueXYZ[2]) > 1)
		{
			memcpy(&pRes->Pos, &pQcReader->pOpt->RoverTrueXYZ, sizeof(double) * 3);
		}

		if ((AntID == VERIQC_ANT_ID_BASE) && fabs(pQcReader->pOpt->BaseTrueXYZ[0]) > 1 && fabs(pQcReader->pOpt->BaseTrueXYZ[1]) > 1 && fabs(pQcReader->pOpt->BaseTrueXYZ[2]) > 1)
		{
			memcpy(&pRes->Pos, &pQcReader->pOpt->BaseTrueXYZ, sizeof(double) * 3);
		}
	}
	PosXYZ2LLH(pRes->Pos, m_pos);/**< realtime position */

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
		}

		iSat = SatIndex[QCSatNo - 1] - 1;

		pObs->Data[iSat].Sat = QCSatNo;
		pObs->Data[iSat].Sys = QCSysMask;

		if (QCSysID == QC_GNSS_SYSID_GLN)
		{
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
		isFind = 0;
		for (iSat0 = 0; iSat0 < (int)pRes->Num; iSat0++)
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
				QCSatAzEl(m_pos, e, pObs->Data[iSat].Azel);
				if (!(pObs->Data[iSat].Sys & pQcReader->pOpt->Sys))
				{
					invail[pObs->Data[iSat].Sat - 1] = 1;
				}
				if (pObs->Data[iSat].Azel[1] <= pQcReader->pOpt->EleDeg * D2R - 1E-4)
				{
					invail[pObs->Data[iSat].Sat - 1] = 1;
				}
				isFind = 1;
				break;
			}
		}
		if (isFind)
		{
			//find ok
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
	pObs->ObsNum += 0;
}

/** Extract file type according to file suffix */
int DetectFileFormat(const char* pFile)
{
	FILE* fp;
	char buff[1025];
	int ver;

	if (!pFile)
	{
		return STRFMT_NONE;
	}
	if (!(fp = fopen(pFile, "rb")))
	{
		PrintfQC("<GNSSQC error>: fopen fail:%s.\n", pFile);
		//PrintfQC("<GNSSQC error>: fopen fail Error: %d (%s)\n", errno, strerror(errno));
		return STRFMT_NONE;
	}
	fgets(buff, 1024, fp);
	if (strstr(pFile, ".rtcm3"))
	{
		fclose(fp);
		return GNSS_STRFMT_RTCM3;
	}
	else if (strstr(pFile, ".rtcm2"))
	{
		fclose(fp);
		return GNSS_STRFMT_RTCM2;
	}
	else if (strstr(buff, "HUACENAV COLLECTED DATA FILE"))/*HCN file*/
	{
		fgets(buff, 1024, fp);
		ver = (int)atof(buff + 4);
		fclose(fp);
		return GetOemType(ver);
	}
	else if (strstr(buff, "RINEX VERSION / TYPE"))/*Rinex*/
	{
		fclose(fp);
		return GNSS_STRFMT_RINEX;
	}
	else if (strstr(pFile, ".log1"))
	{
		fclose(fp);
		return GNSS_STRFMT_RAWMSGB;
	}
	else if (strstr(pFile, ".log2"))
	{
		fclose(fp);
		return GNSS_STRFMT_CHCRAWMSGB;
	}
	else if (strstr(pFile, ".log"))
	{
		fclose(fp);
		return GNSS_STRFMT_RAWMSGA;
	}

	fclose(fp);
	return STRFMT_NONE;
}
