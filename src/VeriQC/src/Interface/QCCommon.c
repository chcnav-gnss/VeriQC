/*************************************************************************//**
		VeriQC

	Interface Module
 *- 
@file   QCCommon.c
@author CHC
@date   5 September 2022
@brief  Some common interfaces
*****************************************************************************/
//#include <io.h>
//#include <direct.h>
#include "QCCommon.h"
#include "QCProcessor.h"
#include "MsgConvert.h"
#include "MsgCodec.h"
#include "MsgConvertTransQC.h"
#include "CommonBase.h"

/*************************************************************************//**
@brief Printf func of QC

@param pFormat [In] Format of Windows "print"
@param ...     [In] Value

@return Result

@author CHC
@date   5 September 2022
*****************************************************************************/
int PrintfQC(const char* pFormat, ...)
{
//#ifdef WIN32
	unsigned char Msg[256] = { 0 };
	va_list Args;
	va_start(Args, pFormat);
	vsnprintf(Msg, 256, pFormat, Args);
	va_end(Args);

	return printf(Msg);
//#else
//	return printf(Format, ...);
//#endif
}

/*********************************************************************
@brief Initialize Structure CHCQC_RINEXCFG_T
@author CHC
@date   25 November 2022
**********************************************************************/
void InitRinexCfg(CHCQC_RINEXCFG_T* pRinexCfg) 
{
	int i;
	memset(&pRinexCfg->RTCMTime, 0, sizeof(QC_TIME_T));
	memset(&pRinexCfg->SplitStart, 0, sizeof(QC_TIME_T));
	memset(&pRinexCfg->SplitEnd, 0, sizeof(QC_TIME_T));
	for (i = 0; i < MAX_SYS_STR_LENGTH; i++) 
	{
		pRinexCfg->Sys[i] = 0;
	}
	for (i = 0; i < MAX_OBS_STR_LENGTH; i++)
	{
		pRinexCfg->ObsType[i] = 0;
	}
	for (i = 0; i < MAX_FRE_STR_LENGTH; i++)
	{
		pRinexCfg->GPSFreq[i] = 0;
		pRinexCfg->SBAFreq[i] = 0;
		pRinexCfg->GLOFreq[i] = 0;
		pRinexCfg->GALFreq[i] = 0;
		pRinexCfg->QZSFreq[i] = 0;
		pRinexCfg->BDSFreq[i] = 0;
		pRinexCfg->NICFreq[i] = 0;
	}
	for (i = 0; i < MAX_FILE_NAME_LENGTH; i++)
	{
		pRinexCfg->RawFile[i] = 0;
		pRinexCfg->OutDir[i] = 0;
	}
}

void InitGnssCfgSimple(CHCQC_GNSSCFG_SIM_T* pGNSSCfgSim)
{
	pGNSSCfgSim->Mode = -1;
	pGNSSCfgSim->Mode = 255;
	pGNSSCfgSim->EleDeg = 0;
	pGNSSCfgSim->FileNum = 0;
	memset(pGNSSCfgSim->OutDir, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset(pGNSSCfgSim->CfgName, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset(pGNSSCfgSim->SpecifyRefPRN, 0, sizeof(int) * QC_MAX_NUM_SYS);
}

void InitRinexCfgSimple(CHCQC_RINEXCFG_SIM_T* pRineCfgSim)
{
	pRineCfgSim->DataType = 0;
	pRineCfgSim->FileNum = 0;
	pRineCfgSim->RinexVer = 304;
	memset(pRineCfgSim->OutDir, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset(pRineCfgSim->CfgName, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
}
/*********************************************************************
@brief Initialize Structure CHCQC_GNSSCFG_T
@author CHC
@date   25 November 2022
**********************************************************************/
void InitGNSSCfg(CHCQC_GNSSCFG_T* pGNSSCfg)
{
	int i;
	pGNSSCfg->StartTime.Time = 0;
	pGNSSCfg->StartTime.Sec = 0.0;
	pGNSSCfg->EndTime.Time = 0;
	pGNSSCfg->EndTime.Sec = 0.0;
	pGNSSCfg->Sys = QC_SYS_ALL;
	pGNSSCfg->RawFileDataType = STRFMT_NONE;
	for (i = 0; i < QC_MAX_NUM_SAT; i++)
	{
		pGNSSCfg->BanPRN[i] = 0;
	}
	for (i = 0; i < QC_MAXELEVINTERV; i++)
	{
		pGNSSCfg->SNRGroup[i] = 0;
	}
	for (i = 0; i < MAX_FILE_NAME_LENGTH; i++)
	{
		pGNSSCfg->OutDir[i] = 0;
		pGNSSCfg->RoverFile[i] = 0;
		pGNSSCfg->BaseFile[i] = 0;
	}
	for (i = 0; i < 3; i++)
	{
		pGNSSCfg->BaseTrueXYZ[i] = 0.0;
		pGNSSCfg->RoverTrueXYZ[i] = 0.0;
	}
	
	for (i = 0; i < QC_GPS_FRE_NUM; i++)
	{
		pGNSSCfg->GMpMAX[i] = 0.5f;
		pGNSSCfg->GSnrMIN[i] = 45;
		pGNSSCfg->NumGSnrMIN[i] = 2;
	}
	for (i = 0; i < QC_GLONASS_FRE_NUM; i++)
	{
		pGNSSCfg->RMpMAX[i] = 0.5f;
		pGNSSCfg->RSnrMIN[i] = 45;
		pGNSSCfg->NumRSnrMIN[i] = 2;
	}
	for (i = 0; i < QC_GALILEO_FRE_NUM; i++)
	{
		pGNSSCfg->EMpMAX[i] = 0.5f;
		pGNSSCfg->ESnrMIN[i] = 45;
		pGNSSCfg->NumESnrMIN[i] = 2;
	}
	for (i = 0; i < QC_BDS_FRE_NUM; i++)
	{
		pGNSSCfg->CMpMAX[i] = 0.5f;
		pGNSSCfg->CSnrMIN[i] = 45;
		pGNSSCfg->NumCSnrMIN[i] = 2;
	}
	for (i = 0; i < QC_QZSS_FRE_NUM; i++)
	{
		pGNSSCfg->JMpMAX[i] = 0.5f;
		pGNSSCfg->JSnrMIN[i] = 45;
		pGNSSCfg->NumJSnrMIN[i] = 2;
	}
	for (i = 0; i < QC_SBAS_FRE_NUM; i++)
	{
		pGNSSCfg->SMpMAX[i] = 0.5f;
		pGNSSCfg->SSnrMIN[i] = 45;
		pGNSSCfg->NumSSnrMIN[i] = 2;
	}
	for (i = 0; i < QC_IRNSS_FRE_NUM; i++)
	{
		pGNSSCfg->IMpMAX[i] = 0.5f;
		pGNSSCfg->ISnrMIN[i] = 45;
		pGNSSCfg->NumISnrMIN[i] = 2;
	}
	pGNSSCfg->AllFreq = 1;
	pGNSSCfg->bIsDoubleDiffCheckLLI = FALSE;
	pGNSSCfg->IsInputTrueXYZ = FALSE;
	pGNSSCfg->EleDeg = 10;
	pGNSSCfg->MPMethod = MP_METHOD_FIXED;

	pGNSSCfg->bIsOutputDataList = FALSE;
	pGNSSCfg->bIsOutputQCLog = FALSE;

	memset(pGNSSCfg->SysSigType, 0, sizeof(int) * QC_MAX_NUM_SYS * QC_MAXSIGTYPE);
	memset(pGNSSCfg->NumSigType, 0, sizeof(int) * QC_MAX_NUM_SYS);
}
/*********************************************************************
@brief Read configuration file:decode rinex
@param  fpCfgFile[I]          file pointer
@param  pRinexCfg[IO]         information
@return Success or failure
@author CHC
@date   25 November 2022
**********************************************************************/
BOOL ReadRinexCfg(FILE *fpCfgFile, CHCQC_RINEXCFG_T* pRinexCfg)
{
	char line[255], *p;
	char* pTokResult;
	int ScanResult = 0;
	int ep[6] = { 0 }, i;
	double TempVer = 3.04;
	double epo[6] = { 0.0 };
	QC_TIME_T time = { 0 };
	if (!fpCfgFile)
	{
		return FALSE;
	}
	while (!feof(fpCfgFile) && !ferror(fpCfgFile))
	{
		strcpy(line, "\n");
		fgets(line, sizeof(line), fpCfgFile);
		p = strtok(line, "=");
		p = strtok(NULL, "=");
		pTokResult = strtok(p, "#");
		if (strstr(line, "#"))
		{
			continue;
		}
		if (strstr(line, "> END_CHCRINEXCFG"))
		{
			break;
		}
		if (strstr(line, "DataType"))
		{
			pRinexCfg->DataType = atoi(p);
			continue;
		}
		if (strstr(line, "OutputType"))
		{
			pRinexCfg->OutputType = atoi(p);
			continue;
		}
		if (strstr(line, "RinexVer")) 
		{
			TempVer = atof(p);
			pRinexCfg->RinexVer = (int)(TempVer * 100);
			continue;
		}
		if (strstr(line, "Interval")) 
		{
			pRinexCfg->Interval = atoi(p);
			continue;
		}
		if (strstr(line, "Sys")) 
		{
			memcpy(pRinexCfg->Sys, p, strlen(p));
			continue;
		}
		if (strstr(line, "SplitTint"))
		{
			pRinexCfg->SplitTimeInt = atoi(p);
			continue;
		}
		if (strstr(line, "SplitStart"))
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{
				for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pRinexCfg->SplitStart.Time = time.Time;
				pRinexCfg->SplitStart.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "SplitEnd")) 
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{
				for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pRinexCfg->SplitEnd.Time = time.Time;
				pRinexCfg->SplitEnd.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "Split")) 
		{
			pRinexCfg->Split = atoi(p);
			continue;
		}
		if (strstr(line, "RTCMTime")) 
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{
				for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pRinexCfg->RTCMTime.Time = time.Time;
				pRinexCfg->RTCMTime.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "CheakSeq")) 
		{
			pRinexCfg->CheakSeq = atoi(p);
			continue;
		}
		if (strstr(line, "LeapSecond")) 
		{
			pRinexCfg->LeapSecond = atoi(p);
			continue;
		}
		if (strstr(line, "AllFreq")) 
		{
			pRinexCfg->AllFreq = atoi(p);
			continue;
		}
		if (strstr(line, "ObsType")) 
		{
			memcpy(pRinexCfg->ObsType, p, strlen(p));
			continue;
		}
		if (strstr(line, "GPS")) 
		{
			memcpy(pRinexCfg->GPSFreq, p, strlen(p));
			AdjustStr(pRinexCfg->GPSFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "SBAS")) 
		{
			memcpy(pRinexCfg->SBAFreq, p, strlen(p));
			AdjustStr(pRinexCfg->SBAFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "GLONASS")) 
		{
			memcpy(pRinexCfg->GLOFreq, p, strlen(p));
			AdjustStr(pRinexCfg->GLOFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "Galileo")) 
		{
			memcpy(pRinexCfg->GALFreq, p, strlen(p));
			AdjustStr(pRinexCfg->GALFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "QZSS")) 
		{
			memcpy(pRinexCfg->QZSFreq, p, strlen(p));
			AdjustStr(pRinexCfg->QZSFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "BDS")) 
		{
			memcpy(pRinexCfg->BDSFreq, p, strlen(p));
			AdjustStr(pRinexCfg->BDSFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "NavIC")) 
		{
			memcpy(pRinexCfg->NICFreq, p, strlen(p));
			AdjustStr(pRinexCfg->NICFreq, MAX_FRE_STR_LENGTH);
			continue;
		}
		if (strstr(line, "Pause"))
		{
			pRinexCfg->Pause = atoi(p);
			continue;
		}
		if (strstr(line, "RawFile")) 
		{
			memcpy(pRinexCfg->RawFile, p, strlen(p));
			AdjustStr(pRinexCfg->RawFile,MAX_FILE_NAME_LENGTH);
			continue;
		}
		if (strstr(line, "OutDir")) 
		{
			memcpy(pRinexCfg->OutDir, p, strlen(p));
			AdjustStr(pRinexCfg->OutDir, MAX_FILE_NAME_LENGTH);
			for (i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			{
				if ('\0' == pRinexCfg->OutDir[i])
				{
					if (i >= 1)
					{
						if (pRinexCfg->OutDir[i - 1] == '\\')
						{
							pRinexCfg->OutDir[i] += 0;
						}
						else
						{
							pRinexCfg->OutDir[i] = '\\';
						}
					}
					else
					{
						pRinexCfg->OutDir[i] = '\\';
					}
					break;
				}
			}
		}
	}
	return TRUE;
}
BOOL ReadRinexCfgSimple(FILE* fpCfgFile, CHCQC_RINEXCFG_SIM_T* pRinexCfgSim)
{
	int8_t line[255], * p;
	int i, iFile, iFiler = -1;
	char* pTokResult;

	if (!fpCfgFile)
	{
		return FALSE;
	}
	while (!feof(fpCfgFile) && !ferror(fpCfgFile))
	{
		strcpy(line, "\n");

		fgets(line, sizeof(line), fpCfgFile);
		p = strtok(line, "=");
		p = strtok(NULL, "=");
		pTokResult = strtok(p, "#");
		if (strstr(line, "#"))
		{
			continue;
		}
		if (strstr(line, "> END_CHCRINEXCFG_SIM"))
		{
			break;
		}
		if (strstr(line, "DataType"))
		{
			pRinexCfgSim->DataType = atoi(p);
			continue;
		}
		if (strstr(line, "OutputType"))
		{
			pRinexCfgSim->OutputType = atoi(p);
			continue;
		}
		if (strstr(line, "RinexVer"))
		{
			pRinexCfgSim->RinexVer = atoi(p);
			continue;
		}
		if (strstr(line, "OutDir"))
		{
			memcpy(pRinexCfgSim->OutDir, p, strlen(p));
			AdjustStr(pRinexCfgSim->OutDir, MAX_FILE_NAME_LENGTH);
			for (i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			{
				if ('\0' == pRinexCfgSim->OutDir[i])
				{
					if (i >= 1)
					{
						if (pRinexCfgSim->OutDir[i - 1] == '\\')
						{
							pRinexCfgSim->OutDir[i] += 0;
						}
						else
						{
							pRinexCfgSim->OutDir[i] = '\\';
						}
					}
					else
					{
						pRinexCfgSim->OutDir[i] = '\\';
					}
					break;
				}
			}
		}
		if (strstr(line, "FileNum"))
		{
			pRinexCfgSim->FileNum = atoi(p);
			pRinexCfgSim->ppRawFile = (char**)VERIQC_CALLOC(pRinexCfgSim->FileNum, sizeof(char*));
			for (iFile = 0; iFile < pRinexCfgSim->FileNum; iFile++)
			{
				pRinexCfgSim->ppRawFile[iFile] = (char*)VERIQC_CALLOC(MAX_FILE_NAME_LENGTH, sizeof(char));
			}
			iFiler = 0;
			continue;
		}
		if (strstr(line, "RawFile"))
		{
			if (iFiler < 0|| iFiler>= pRinexCfgSim->FileNum)continue;
			memcpy(pRinexCfgSim->ppRawFile[iFiler], p, sizeof(char) * strlen(p));
			AdjustStr(pRinexCfgSim->ppRawFile[iFiler], MAX_FILE_NAME_LENGTH);
			iFiler++;
			continue;
		}
	}

	return TRUE;
}

BOOL ReadGNSSCfgSimple(FILE* fpCfgFile, CHCQC_GNSSCFG_SIM_T* pGNSSCfgSim)
{
	int8_t line[255], * p;
	int ep[6] = { 0 }, i, iFile, iFiler, iFileb, iXYZr, iXYZb;
	char* pTokResult;
	double epo[6] = { 0.0 };
	QC_TIME_T time = { 0 };
	unsigned char sysStr[MAX_SYS_STR_LENGTH] = { 0 };
	
	iFiler = iFileb = iXYZr = iXYZb = -1;

	if (!fpCfgFile)
	{
		return FALSE;
	}
	while (!feof(fpCfgFile) && !ferror(fpCfgFile))
	{
		strcpy(line, "\n");

		fgets(line, sizeof(line), fpCfgFile);
		p = strtok(line, "=");
		p = strtok(NULL, "=");
		pTokResult = strtok(p, "#");
		if (strstr(line, "#"))
		{
			continue;
		}
		if (strstr(line, "> END_CHCGNSSCFG_SIM"))
		{
			break;
		}
		if (strstr(line, "Mode"))
		{
			pGNSSCfgSim->Mode = atoi(p);
			continue;
		}
		if (strstr(line, "Sys"))
		{
			memcpy(sysStr, p, strlen(p));
			pGNSSCfgSim->Sys = Str2Sys(sysStr);
			continue;
		}
		if (strstr(line, "EleDeg"))
		{
			pGNSSCfgSim->EleDeg = atoi(p);
			continue;
		}
		if (strstr(line, "FileNum"))
		{
			pGNSSCfgSim->FileNum = atoi(p);
			pGNSSCfgSim->ppRroverFile = (char**)VERIQC_CALLOC(pGNSSCfgSim->FileNum,sizeof(char*));
			pGNSSCfgSim->ppBaseFile = (char**)VERIQC_CALLOC(pGNSSCfgSim->FileNum, sizeof(char*));
			pGNSSCfgSim->ppBaseTrueXYZ = (double**)VERIQC_CALLOC(pGNSSCfgSim->FileNum, sizeof(double*));
			pGNSSCfgSim->ppRoverTrueXYZ = (double**)VERIQC_CALLOC(pGNSSCfgSim->FileNum, sizeof(double*));
			for (iFile = 0; iFile < pGNSSCfgSim->FileNum; iFile++)
			{
				pGNSSCfgSim->ppRroverFile[iFile] = (char*)VERIQC_CALLOC(MAX_FILE_NAME_LENGTH, sizeof(char));
				pGNSSCfgSim->ppBaseFile[iFile] = (char*)VERIQC_CALLOC(MAX_FILE_NAME_LENGTH, sizeof(char));
				pGNSSCfgSim->ppBaseTrueXYZ[iFile] = (double*)VERIQC_CALLOC(3, sizeof(double));
				pGNSSCfgSim->ppRoverTrueXYZ[iFile] = (double*)VERIQC_CALLOC(3, sizeof(double));
			}
			iFiler = iFileb = iXYZr = iXYZb = 0;
			continue;
		}
		if (strstr(line, "OutDir"))
		{
			memcpy(pGNSSCfgSim->OutDir, p, strlen(p));
			AdjustStr(pGNSSCfgSim->OutDir, MAX_FILE_NAME_LENGTH);
			
			if (pGNSSCfgSim->OutDir[strlen(pGNSSCfgSim->OutDir)] != '\\')
			{
				sprintf(pGNSSCfgSim->OutDir, "%s%s", pGNSSCfgSim->OutDir, "\\");
			}
			continue;
		}

		if (strstr(line, "RoverFile"))
		{
			if (iFiler < 0||iFiler>=pGNSSCfgSim->FileNum)continue;
			memcpy(pGNSSCfgSim->ppRroverFile[iFiler], p, sizeof(char) * strlen(p));
			AdjustStr(pGNSSCfgSim->ppRroverFile[iFiler], MAX_FILE_NAME_LENGTH);
			iFiler++;
			continue;
		}
		if (strstr(line, "BaseFile"))
		{
			if (iFileb < 0)continue;
			memcpy(pGNSSCfgSim->ppBaseFile[iFileb], p, sizeof(char) * strlen(p));
			AdjustStr(pGNSSCfgSim->ppBaseFile[iFileb], MAX_FILE_NAME_LENGTH);
			iFileb++;
			continue;
		}
		if (strstr(line, "BaseTrueXYZ"))
		{
			if (iXYZb < 0)continue;
			pTokResult = strtok(p, ",");
			for (i = 0; i < 3; i++)
			{
				pGNSSCfgSim->ppBaseTrueXYZ[iXYZb][i] = atof(p);
				p = strtok(NULL, ",");
			}
			iXYZb++;
			continue;
		}
		//#[m] Rover ecef coordinate
		if (strstr(line, "RoverTrueXYZ"))
		{
			if (iXYZr < 0)continue;
			pTokResult = strtok(p, ",");
			
			for (i = 0; i < 3; i++)
			{
				pGNSSCfgSim->ppRoverTrueXYZ[iXYZr][i] = atof(p);
				p = strtok(NULL, ",");
			}
			iXYZr++;
			continue;
		}
		if (strstr(line, "GPSRefPRN"))
		{
			pGNSSCfgSim->SpecifyRefPRN[QC_GNSS_SYSID_GPS - 1] = atoi(p);
			continue;
		}
		if (strstr(line, "GLORefPRN"))
		{
			pGNSSCfgSim->SpecifyRefPRN[QC_GNSS_SYSID_GLN - 1] = atoi(p);
			continue;
		}
		if (strstr(line, "GALRefPRN"))
		{
			pGNSSCfgSim->SpecifyRefPRN[QC_GNSS_SYSID_GAL - 1] = atoi(p);
			continue;
		}
		if (strstr(line, "BDSRefPRN"))
		{
			pGNSSCfgSim->SpecifyRefPRN[QC_GNSS_SYSID_BDS - 1] = atoi(p);
			continue;
		}

	}
	return TRUE;
}

BOOL ReadGNSSCfg(FILE *fpCfgFile, CHCQC_GNSSCFG_T* pGNSSCfg)
{
	int8_t line[255], *p;
	int ep[6] = { 0 }, i;
	int ScanResult = 0;
	char* pTokResult = 0;
	double epo[6] = { 0.0 };
	QC_TIME_T time = { 0 };
	unsigned char sysStr[MAX_SYS_STR_LENGTH] = { 0 };
	unsigned char freStr[MAX_FRE_STR_LENGTH] = { 0 };
	if (!fpCfgFile)
	{
		return FALSE;
	}
	while (!feof(fpCfgFile) && !ferror(fpCfgFile))
	{
		strcpy(line, "\n");
		fgets(line, sizeof(line), fpCfgFile);
		p = strtok(line, "=");
		p = strtok(NULL, "=");
		pTokResult = strtok(p, "#");
		if (strstr(line, "#"))
		{
			continue;
		}
		if (strstr(line, "> END_CHCGNSSCFG"))
		{
			break;
		}
		if (strstr(line, "Mode"))
		{
			pGNSSCfg->Mode = atoi(p);
			continue;
		}
		if (strstr(line, "Sys"))
		{
			memcpy(sysStr, p, strlen(p));
			pGNSSCfg->Sys = Str2Sys(sysStr);
			continue;
		}
		if (strstr(line, "BanPRN"))
		{
			Sat2BanSat(p, pGNSSCfg->BanPRN);
			continue;
		}
		if (strstr(line, "EleDeg"))
		{
			pGNSSCfg->EleDeg = atoi(p);
			continue;
		}
		if (strstr(line, "Interval"))
		{
			pGNSSCfg->Interval = (float)atof(p);
			continue;
		}
		if (strstr(line, "SNRGroup"))
		{
			//QC_MAXELEVINTERV
			/*memcpy(pGnssCfg->SNRgroup, p, strlen(p));*/
			Str2SnrGroup(p, pGNSSCfg->SNRGroup, &pGNSSCfg->SNRGroupSize);
			continue;
		}
		if (strstr(line, "IsThresholdSNR"))
		{
			pGNSSCfg->IsThesholdSNR = atoi(p);
			continue;
		}
		if (strstr(line, "IsThresholdMP"))
		{
			pGNSSCfg->IsThesholdMP = atoi(p);
			continue;
		}
		if (strstr(line, "HeightToReflector"))
		{
			pGNSSCfg->HtoReflector = atof(p);
			continue;
		}
		if (strstr(line, "RTCMTime"))
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{
				for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pGNSSCfg->RTCMTime.Time = time.Time;
				pGNSSCfg->RTCMTime.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "StartTime"))
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{	
			    for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pGNSSCfg->StartTime.Time = time.Time;
				pGNSSCfg->StartTime.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "EndTime"))
		{
			ScanResult = sscanf(p, "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
			if (ScanResult == 6)
			{
				for (i = 0; i < 6; i++) epo[i] = ep[i];
				time = Epoch2QCTime(epo);
				pGNSSCfg->EndTime.Time = time.Time;
				pGNSSCfg->EndTime.Sec = time.Sec;
			}
			continue;
		}
		if (strstr(line, "BaseTrueXYZ"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < 3; i++) 
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->BaseTrueXYZ[i] = atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		//#[m] Rover ecef coordinate
		if (strstr(line, "RoverTrueXYZ"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < 3; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->RoverTrueXYZ[i] = atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "IsIuputTrueXYZ"))
		{
			pGNSSCfg->IsInputTrueXYZ = atoi(p);
			continue;
		}
		if (strstr(line, "GPSMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GPS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->GMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "GLOMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GLONASS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->RMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "GALMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GALILEO_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->EMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "BDSMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_BDS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->CMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "QZSMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_QZSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->JMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "SBAMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_SBAS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->SMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NICMpMAX"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_IRNSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->IMpMAX[i] = (float)atof(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumGPSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GPS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumGSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumGLOSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GLONASS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumRSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumGALSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GALILEO_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumESnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumBDSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_BDS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumCSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumQZSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_QZSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumJSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumSBASnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_SBAS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumSSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NumNICSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_IRNSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->NumISnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "GPSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GPS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->GSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "GLOSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GLONASS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->RSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "GALSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_GALILEO_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->ESnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "BDSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_BDS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->CSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "QZSSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_QZSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->JSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "SBASnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_SBAS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->SSnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "NICSnrMIN"))
		{
			pTokResult = strtok(p, ",");
			for (i = 0; i < QC_IRNSS_FRE_NUM; i++)
			{
				if (NULL == p)
				{
					break;
				}
				pGNSSCfg->ISnrMIN[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			continue;
		}
		if (strstr(line, "MpCarrier"))
		{
			pGNSSCfg->MPCarrier = (float)atof(p);
			continue;
		}
		if (strstr(line, "UseRate"))
		{
			pGNSSCfg->UseRate = (float)atof(p);
			continue;
		}
		if (strstr(line, "CycleJumpRatio"))
		{
			pGNSSCfg->CJR = atoi(p);
			continue;
		}
		if (strstr(line, "MPMethod"))
		{
			pGNSSCfg->MPMethod = atoi(p);
			continue;
		}
		if (strstr(line, "OutDir"))
		{
			memcpy(pGNSSCfg->OutDir, p, strlen(p));
			AdjustStr(pGNSSCfg->OutDir, MAX_FILE_NAME_LENGTH);
			
			if (strlen(pGNSSCfg->OutDir) > 0)
			{
				if (pGNSSCfg->OutDir[strlen(pGNSSCfg->OutDir)] != '\\')
				{
					sprintf(pGNSSCfg->OutDir, "%s%s", pGNSSCfg->OutDir, "\\");
				}
			}
			
			continue;
		}
		if (strstr(line, "RoverFile"))
		{
			memcpy(pGNSSCfg->RoverFile, p, strlen(p));
			AdjustStr(pGNSSCfg->RoverFile, MAX_FILE_NAME_LENGTH);
			continue;
		}
		if (strstr(line, "BaseFile"))
		{
			memcpy(pGNSSCfg->BaseFile, p, strlen(p));
			AdjustStr(pGNSSCfg->BaseFile, MAX_FILE_NAME_LENGTH);
			continue;
		}
		if (strstr(line, "AllFreq"))
		{
			pGNSSCfg->AllFreq = atoi(p);
			continue;
		}
		if (strstr(line, "GPS"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_GPS, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_GPS, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char)* MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "SBAS"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_SBS, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_SBS, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char) * MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "GLONASS"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_GLO, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_GLO, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char) * MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "Galileo"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_GAL, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_GAL, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char) * MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "QZSS"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_QZS, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_QZS, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char) * MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "BDS"))
		{
			memcpy(freStr, p, strlen(p));
			if (pGNSSCfg->AllFreq == 0)
			{
				DecodeFreqList(p, QC_SYS_CMP, pGNSSCfg);
			}
			else if (pGNSSCfg->AllFreq == 2)
			{
				Obs2FreId(p, QC_SYS_CMP, pGNSSCfg);
			}
			memset(freStr, 0, sizeof(unsigned char) * MAX_FRE_STR_LENGTH);//reset
			continue;
		}
		if (strstr(line, "IsDoubleDiffCheckLLI"))
		{
			pGNSSCfg->bIsDoubleDiffCheckLLI = atoi(p);
			continue;
		}
		if (strstr(line, "IsOutputDataList"))
		{
			pGNSSCfg->bIsOutputDataList = atoi(p);
			continue;
		}if (strstr(line, "IsOutputQCLog"))
		{
			pGNSSCfg->bIsOutputQCLog = atoi(p);
			continue;
		}
	}
	return TRUE;
}
int Obs2FreId(int8_t* pObsString, int Sys, CHCQC_GNSSCFG_T* pGNSSCfg)
{
	const char FreList[QC_MAX_NUM_SYS][QC_NUM_FREQ][8] =
	{ {"L1", "L2C", "L5", "L1C-P", "L2P", ""},/*GPS*/
	{ "L1","L5",  "",     "",     "",""},/*SBAS*/
	{ "G1","G2", "", "", "", ""},/*GLONASS*/
	{ "E1","E5A","E5B","E5(A+B)","E6",""},/*Galileo*/
	{ "L1","L2","L5","L1C(P)","L6","L1C/B"},/*QZSS*/
	{ "B1","B2","B3","B1C","B2a","B2b"} };/*BDS*/

	int num = atoi(&pObsString[0]), iFreq, iCharS, iCharE, Len = (int)strlen(pObsString);
	int sysid = QCSys2Id(Sys) - 1;
	char m_str[8] = { 0 };
	if (num <= 0)
	{
		return num;
	}
	iCharS = 0;
	for (iFreq = 0; iFreq < num; iFreq++)
	{
		for (; iCharS < Len; iCharS++)
		{
			if (pObsString[iCharS] == ',')
			{
				iCharS++;
				break;
			}
		}
		for (iCharE = iCharS; iCharE < Len; iCharE++)
		{
			if (pObsString[iCharE] == ','|| pObsString[iCharE] == ' '|| pObsString[iCharE] == '\r'|| pObsString[iCharE] == '\0'|| pObsString[iCharE] == '\n')
			{
				break;
			}
		}
		strncpy(m_str, pObsString + iCharS, ((size_t)iCharE - iCharS));
		m_str[iCharE - iCharS] = '\0';
		for (iCharE = 0; iCharE < QC_NUM_FREQ; iCharE++)
		{
			
			if (!strcmp(FreList[sysid][iCharE], m_str))
			{
				pGNSSCfg->SysSigType[sysid][iCharE] = 1;
				break;
			}
		}
		
		memset(m_str, 0, sizeof(char) * 8);
	}
	pGNSSCfg->NumSigType[sysid] = num;
	return num;
}
/* decode option string :"1, C1C" 
* int8_t* pFreqString       [I]string 
* int Sys                   [I]SYS_?
* CHCQC_GNSSCFG_T* pGNSSCfg [O]
* return: num
*************************************/
int DecodeFreqList(int8_t* pFreqString, int Sys, CHCQC_GNSSCFG_T* pGNSSCfg)
{
	int num = atoi(&pFreqString[0]), iFreq, iChar, Len = (int)strlen(pFreqString),freq;
	int sysid = QCSys2Id(Sys) - 1;
	char m_str[3] = { 0 };
	if (num <= 0)
	{
		return num;
	}
	iChar = 0;
	for (iFreq = 0; iFreq < num; iFreq++)
	{
		for (;iChar < Len; iChar++)
		{
			if (pFreqString[iChar] == ',')
			{
				iChar++;
				break;
			}
		}
		m_str[0] = pFreqString[iChar + 1];
		m_str[1] = pFreqString[iChar + 2];
		pGNSSCfg->SysSigType[sysid][iFreq] = QCObs2Code(m_str, &freq);
		memset(m_str, 0, sizeof(char) * 3);
	}
	pGNSSCfg->NumSigType[sysid] = num;
	return num;
}

BOOL ReadCfg(char* pFileName, CHCQC_RINEXCFG_T* pRinexCfg, CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_RINEXCFG_SIM_T* pRinexCfgSim, CHCQC_GNSSCFG_SIM_T* pGNSSCfgSim, BOOL* pInvail)
{
	FILE *fp;
	char line[255];
	fp = fopen(pFileName, "rb+");
	if (fp == NULL)
	{
		return FALSE;
	}
	while (!feof(fp) && !ferror(fp))
	{
		strcpy(line, "\n");
		fgets(line, sizeof(line), fp);
		if (strstr(line, "#"))
		{
			continue;
		}
		if (strstr(line, "> START_CHCRINEXCFG_SIM"))
		{
			ReadRinexCfgSimple(fp, pRinexCfgSim);
			memcpy(pRinexCfgSim->CfgName, pFileName, sizeof(char) * strlen(pFileName));
			fclose(fp);
			pInvail[3] = TRUE;
			break;
		}
		else if (strstr(line,"> START_CHCRINEXCFG")) 
		{
			/** read pRinexCfg */
			ReadRinexCfg(fp, pRinexCfg);
			fclose(fp);
			pInvail[0] = TRUE;
			break;
		}
		if (strstr(line, "> START_CHCGNSSCFG_SIM"))
		{
			/** read pRinexCfg */
			ReadGNSSCfgSimple(fp, pGNSSCfgSim);
			memcpy(pGNSSCfgSim->CfgName, pFileName, sizeof(char) * strlen(pFileName));
			fclose(fp);
			pInvail[4] = TRUE;
			break;
		}
		else if(strstr(line, "> START_CHCGNSSCFG"))
		{
			/** read GNSS Cfg */
			ReadGNSSCfg(fp, pGNSSCfg);
			fclose(fp);
			pInvail[1] = TRUE;
			break;
		}
		if (strstr(line, "> START_CHCQCCFG"))
		{
			/** read IMU Cfg */
			fclose(fp);
			pInvail[2] = TRUE;
			break;
		}
	}

	return TRUE;
}

/**********************************************************************//**
@brief interface: Rixne function

@param pRinexCfg     [In]     Rinex config

@author CHC
@date
@note
History:
1. 2024/07/25 use MsgConvert funcs instead
**************************************************************************/
void RinexProcesser(CHCQC_RINEXCFG_T* pRinexCfg)
{
	unsigned int TypeIndex = 0;
	int InputDataType = pRinexCfg->DataType;
	int OutputDataType = pRinexCfg->OutputType;
	char OutDir[256] = { 0 };
	int SatSys = SysStr2ExSysMask(pRinexCfg->Sys);
	int Ver = pRinexCfg->RinexVer;
	MSG_CONVERT_DATA_T* pMsgConvertData = NULL;
	MSG_DECODE_T* pMsgDecode = NULL;

	pMsgConvertData = MsgConvertInit();
	if (pMsgConvertData == NULL)
	{
		return;
	}

	MEMCPY(OutDir, pRinexCfg->OutDir, strlen(pRinexCfg->OutDir));

	pMsgConvertData->AllFreqFlag = pRinexCfg->AllFreq; 
	pMsgConvertData->MaxPoshis = 1;

	pMsgConvertData->DataOutType = OutputDataType;
	QCTimeToUTCTime(&pRinexCfg->RTCMTime, &pMsgConvertData->RTCMTime);

	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[0] = SetSysTObs(pRinexCfg->GPSFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[0]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[1] = SetSysTObs(pRinexCfg->GLOFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[1]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[2] = SetSysTObs(pRinexCfg->GALFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[2]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[3] = SetSysTObs(pRinexCfg->BDSFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[3]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[4] = SetSysTObs(pRinexCfg->SBAFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[4]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[5] = SetSysTObs(pRinexCfg->QZSFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[5]);
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs[6] = SetSysTObs(pRinexCfg->NICFreq, pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf[6]);

	pMsgConvertData->GlobalObsConvertData.RINEXEphOpt.Ver = Ver;
	pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.Ver = Ver;
	if (pRinexCfg->Interval != 0)
	{
		pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.Sample = pRinexCfg->Interval;
	}

	for (TypeIndex = 0; TypeIndex < RAW_OBS_DATA_TYPE_NUM_MAX; TypeIndex++)
	{
		MEMCPY(&pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsOpt.NObs, &pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs, sizeof(int) * TOTAL_EX_GNSS_SYSTEM_NUM);
		MEMCPY(&pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsOpt.ObsTypeBuf, &pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf, sizeof(pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf));

		pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXEphOpt.Ver = Ver;
		pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsOpt.Ver = Ver;
		pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsOpt.NavSys = SatSys;
		if (pRinexCfg->Interval != 0)
		{
			pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsOpt.Sample = pRinexCfg->Interval;
			pMsgConvertData->SingleObsConvertData[TypeIndex].RINEXObsHeader.TimeInt = pRinexCfg->Interval;
		}
	}

	MEMCPY(&pMsgConvertData->RinexMultiConvertData.RINEXObsOpt.NObs, &pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.NObs, sizeof(int) * TOTAL_EX_GNSS_SYSTEM_NUM);
	MEMCPY(&pMsgConvertData->RinexMultiConvertData.RINEXObsOpt.ObsTypeBuf, &pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf, sizeof(pMsgConvertData->GlobalObsConvertData.RINEXObsOpt.ObsTypeBuf));

	pMsgConvertData->InputDataType = InputDataType;

	pMsgConvertData->ObsFreqFilterEnable = 1;
	MEMSET(pMsgConvertData->ObsFreqFilter, 0xFF, sizeof(pMsgConvertData->ObsFreqFilter));

	pMsgConvertData->ObsSatFilterEnable = 1;
	MEMSET(pMsgConvertData->ObsSatFilter, 0xFF, sizeof(pMsgConvertData->ObsSatFilter));

	/** relative file path error and "/" file path error */
	if (InputDataType == GNSS_STRFMT_RINEX)
	{
		if (OutputDataType == DATA_OUT_TYPE_RINEX)
		{
			if (InitRinexMultiInputFile(pMsgConvertData, pRinexCfg->RawFile) < 0)
			{
				return;
			}
		}
		else
		{
			if (InitRinexInputFile(pMsgConvertData, pRinexCfg->RawFile) < 0)
			{
				return;
			}
		}
	}
	else
	{
		if ((pMsgConvertData->RawFp = fopen(pRinexCfg->RawFile, "rb+")) == NULL)
		{
			PrintfQC("<Rinex error>The raw data file was not found.\n");
			return;
		}
	}

	pMsgConvertData->RINEXObsHeader.NavSys = SatSys;
	pMsgConvertData->RINEXEphHeader.Sys = 0xFF;
	pMsgConvertData->RINEXObsOpt.NavSys = SatSys;
	pMsgConvertData->RINEXEphOpt.NavSys = 0xFF;

	pMsgConvertData->RINEXObsHeader.Ver = Ver;
	pMsgConvertData->RINEXEphHeader.Ver = Ver;
	pMsgConvertData->RINEXObsOpt.Ver = Ver;
	pMsgConvertData->RINEXEphOpt.Ver = Ver;

	pMsgConvertData->RinexMultiConvertData.RINEXObsHeader.NavSys = SatSys;
	pMsgConvertData->RinexMultiConvertData.RINEXEphHeader.Sys = 0xFF;
	pMsgConvertData->RinexMultiConvertData.RINEXObsOpt.NavSys = SatSys;
	pMsgConvertData->RinexMultiConvertData.RINEXEphOpt.NavSys = 0xFF;

	pMsgConvertData->RinexMultiConvertData.RINEXObsHeader.Ver = Ver;
	pMsgConvertData->RinexMultiConvertData.RINEXEphHeader.Ver = Ver;
	pMsgConvertData->RinexMultiConvertData.RINEXObsOpt.Ver = Ver;
	pMsgConvertData->RinexMultiConvertData.RINEXEphOpt.Ver = Ver;

	if (-1 == InputDataType)
	{
		InputDataType = ParseHCNHeaderData(pMsgConvertData);
		if (-1 == InputDataType)
		{
			PrintfQC("\n.HCN header file parsing failed, please check the data header file\n");
			return;
		}
	}

#ifdef RTCM_DEBUG_INFO
	if ((GNSS_STRFMT_RTCM2 == InputDataType) || (GNSS_STRFMT_RTCM3 == InputDataType) || (GNSS_STRFMT_HRCX == InputDataType))
	{
		InitOutputFile(OUTPUT_FILE_TYPE_DEBUGINFO, pMsgConvertData, pRinexCfg->RawFile, OutDir);
	}
#endif
	if ((GNSS_STRFMT_RINEX != InputDataType) && (GNSS_STRFMT_RTKLOGA != InputDataType) && (GNSS_STRFMT_RTKLOGB != InputDataType))
	{
		InitOutputFile(OUTPUT_FILE_TYPE_BASEOBSSATCOUNT, pMsgConvertData, pRinexCfg->RawFile, OutDir);
	}

	pMsgDecode = MsgCodecInit(0, pMsgConvertData, DECODE_MODE_CONVERT_TO_SIGNAL, &pMsgConvertData->RTCMTime);
	if (pMsgDecode == NULL)
	{
		goto RET;
	}

	pMsgDecode->SamplePeriod = pRinexCfg->Interval;
	if (pRinexCfg->Split == 1)
	{/**< 1:station moving */
		pMsgConvertData->MaxPoshis = MAXPOSHIS;
	}
	else if (pRinexCfg->Split == 3)
	{
		QCTimeToUTCTime(&pRinexCfg->SplitStart, &pMsgConvertData->RINEXObsOpt.TimeStart);
		QCTimeToUTCTime(&pRinexCfg->SplitEnd, &pMsgConvertData->RINEXObsOpt.TimeEnd);

		MEMCPY(&pMsgDecode->SetStartTime, &pMsgConvertData->RINEXObsOpt.TimeStart, sizeof(UTC_TIME_T));
		MEMCPY(&pMsgDecode->SetEndTime, &pMsgConvertData->RINEXObsOpt.TimeEnd, sizeof(UTC_TIME_T));
	}

	if (InputDataType != GNSS_STRFMT_RINEX)
	{
		AllFreqDecode(pMsgConvertData, pMsgDecode, InputDataType);
		PrintfQC("\nAnalysis of observation system frequency points completed\n");
	}

	if (DATA_OUT_TYPE_RINEX == GetDataOutType(pMsgConvertData))
	{
		if (InputDataType != GNSS_STRFMT_RINEX)
		{
			InitRINEXOutputFile(pMsgConvertData, pRinexCfg->RawFile, OutDir);
			MsgCodecSetDecodeMode(pMsgDecode, DECODE_MODE_CONVERT_TO_RINEX);
		}
		else
		{
			InitRinexMultiOutputDir(pMsgConvertData, pRinexCfg->RawFile, OutDir);
			MsgCodecSetDecodeMode(pMsgDecode, DECODE_MODE_CONVERT_TO_RINEX);
		}
	}
	else
	{
		goto RET;
	}

	if (InputDataType != GNSS_STRFMT_RINEX)
	{
		DataDecodeStart(pMsgConvertData, pMsgDecode, InputDataType);
	}
	else
	{
		RinexDecodeStart(pMsgConvertData);
	}

RET:
	MsgConvertDeinit(pMsgConvertData);
	MsgCodecDeinit(pMsgDecode);

	PrintfQC("<Rinex log>Decoding finish!\r\n");
}

/**********************************************************************//**
@brief interface: GNSS QC function

@param pGNSSCfg     [In]     GNSS config
@param pQCRes       [Out]    Quality check result

@author CHC
@date
@note
History:
1. 2024/07/25 use MsgConvert funcs instead
**************************************************************************/
void GNSSProcesser(CHCQC_GNSSCFG_T* pGNSSCfg, QC_RESULT_T* pQCRes)
{
	int8_t ret = -1;
	FILE* fpRaw_b = NULL;
	CHCQC_READER_T* pQcReader = NULL;
	BOOL bState = FALSE;
	unsigned long long FileTotalSize = 0;
	unsigned long long FileCurrentSize = 0;
	unsigned int PercentControl = 1;/**< control for one progress bar 1: only QC */
	double FilePercent = 0;

	pQcReader = (CHCQC_READER_T*)VERIQC_MALLOC(sizeof(CHCQC_READER_T));
	if (pQcReader == NULL)
	{
		PrintfQC("<GNSSQC error>: Failed to create decoder/Input file does not exit.\n");
		return;
	}

	ret = InitQCReader(pQcReader);
	if (ret == -1)
	{
		PrintfQC("<GNSSQC error>: Failed to create decoder/Input file does not exit.\n");
		VERIQC_FREE(pQcReader);
		pQcReader = NULL;
		return;
	}

	MkDirs(pGNSSCfg->OutDir); /**< create output directory */
	InitRTKCore((double)pGNSSCfg->EleDeg);
	bState = SyncGNSSQCState(pGNSSCfg, pQcReader);/**< Synchronization Configuration*/
	if (!bState)
	{
		PrintfQC("<GNSSQC error>: Failed to create decoder/Input file does not exit.\n");
		return;
	}
	UpdateQCOpt(pQcReader->pOpt, &pQcReader->QcRes);  /**< QCLIB */
	/* first: obtain the start and end time, sampling and frequency information */
	pQcReader->IterNo = 0;

	/** get rover and base file total size */
	FileTotalSize = pQcReader->RawFileSize[0] + pQcReader->RawFileSize[1];
	FileTotalSize *= PercentControl;

	while (TRUE)
	{
		if ((ret = QCProcessCore(pGNSSCfg, pQcReader, pQcReader->IterNo)) < 0)
		{
			break;
		}

		FileCurrentSize = pQcReader->CurFilePos[0];
		FileCurrentSize += pQcReader->CurFilePos[1];
		FilePercent = (FileTotalSize > 0) ? (100.0 * FileCurrentSize / FileTotalSize) : 100;
		PrintfQC("<GNSSQC msg>:processing file %5.1f%%%%\r", FilePercent);
	}
	PrintfQC("\n");

	ResetQCReader(pQcReader);
	GetQCAvePos(&pQcReader->QcRes);
	UpdateQCRes(&pQcReader->QcRes, pQcReader->pOpt);  /**< QCLIB */

	/*get output file*/
	WriteQCStatisticsHtml(&pQcReader->QcRes, pQcReader->pOpt);

	memcpy(pQCRes, &pQcReader->QcRes, sizeof(QC_RESULT_T));

	EndRTKCore();
	FreeQCReader(pQcReader);
	PrintfQC("<GNSSQC msg>The quality check is completed.\n");
#ifdef _DEBUG
	QCCloseLogfile();
#endif
}


/*********************************************************************
@brief Remove the space at the beginning and end of the string, and convert "/" to "\"
@param  pString[I0]                 file pointer
@return Success or failure
@author CHC
@date   25 November 2022
**********************************************************************/
extern BOOL AdjustStr(char* pString, int MaxSize)
{
	BOOL isOpen = FALSE;
	int i, ii, sSpaceNum = 0, eSpaceNum = 0;

	if (pString)
	{
		for (i = 0; i < MaxSize; i++)
		{
			if (0 == pString[i] ) 
			{
				break;
			}
			if (47 == pString[i])
			{
#ifdef WIN32
				pString[i] = '\\';
#else
				pString[i] = '/';
#endif
			}
			if (' ' != pString[i]&& '\r' != pString[i] && '\n' != pString[i])
			{
				isOpen = TRUE;
				eSpaceNum = 0;
			}
			if (FALSE == isOpen && (' ' == pString[i] || '\r' == pString[i] || '\n' == pString[i]))
			{
				sSpaceNum++;
			}
			if (TRUE == isOpen && (' ' == pString[i] || '\r' == pString[i] || '\n' == pString[i]))
			{
				eSpaceNum++;
			}
		}
		for (ii = 0; ii < i; ii++)
		{
			pString[ii] = pString[ii + sSpaceNum];
		}
		for (ii = i - 1; ii >= i - eSpaceNum - sSpaceNum; ii--)
		{
			pString[ii] = 0;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
extern void Str2SnrGroup(char* pString, float* pSNRGroup, int* pSize)
{
	int iStr, iStr0 = 0;
	float deg;
	*pSize = 0;
	for (iStr = 0; iStr < strlen(pString); iStr++)
	{
		if (pString[iStr] == ',')
		{
			deg = (float)QCStr2Num(pString, iStr0, iStr - iStr0);
			if (deg == 0)
			{
				iStr0 = iStr + 1;
				continue;
			}
			pSNRGroup[*pSize] = deg;
			*pSize += 1;
			iStr0 = iStr + 1;
		}
		if (iStr == strlen(pString) - 1)
		{
			deg = (float)QCStr2Num(pString, iStr0, iStr - iStr0);
			if (deg == 0)
			{
				iStr0 = iStr + 1;
				continue;
			}
			pSNRGroup[*pSize] = deg;
			*pSize += 1;
			iStr0 = iStr + 1;
		}
	}
	if (pSNRGroup[*pSize - 1] != 90)
	{
		pSNRGroup[*pSize] = 90;
		*pSize += 1;
	}
}

/*********************************************************************
@brief str to int
@param  pString[I]               str eg.G,S,R,E,J,C,I
@return int [0]
@author CHC
@date   25 November 2022
**********************************************************************/
extern int Str2Sys(char* pString)
{
	int sys = 0, i;
	int len = (int)strlen(pString);
	for (i = 0; i < len; i++)
	{
		switch (pString[i])
		{
		case 'G':
		case 'g':
			sys += QC_SYS_GPS;
			break;
		case 'S':
		case 's':
			sys += QC_SYS_SBS;
			break;
		case 'R':
		case 'r':
			sys += QC_SYS_GLO;
			break;
		case 'E':
		case 'e':
			sys += QC_SYS_GAL;
			break;
		case 'J':
		case 'j':
			sys += QC_SYS_QZS;
			break;
		case 'C':
		case 'c':
			sys += QC_SYS_CMP;
			break;
		case 'I':
		case 'i':
			sys += QC_SYS_LEO;
			break;
		default:
			break;
		}
	}
	if (sys == 0)
	{
		return 255;//default : all system
	}
	else
	{
		return sys;
	}
}

extern void Sat2BanSat(char* pString, unsigned char* pSat)
{
	int sys = 0, i;
	int prn;
	int len = (int)strlen(pString);
	int n = 0;
	int SatNo = 0;
	for (i = 0; i < len - 2; i++)
	{
		if (n >= MAX_NUM_SAT_BAN)
		{
			break;
		}
		switch (pString[i])
		{
		case 'G':
		case 'g':
			sys = QC_SYS_GPS;
			break;
		case 'S':
		case 's':
			sys = QC_SYS_SBS;
			break;
		case 'R':
		case 'r':
			sys = QC_SYS_GLO;
			break;
		case 'E':
		case 'e':
			sys = QC_SYS_GAL;
			break;
		case 'J':
		case 'j':
			sys = QC_SYS_QZS;
			break;
		case 'C':
		case 'c':
			sys = QC_SYS_CMP;
			break;
		case 'I':
		case 'i':
			sys = QC_SYS_LEO;
			break;
		default:
			sys = -1;
			break;
		}
		if (sys > 0)
		{
			prn = (int)QCStr2Num(pString, i + 1, 2);
			SatNo = GetQCSatNo(sys, prn);
			if (SatNo > 0)
			{
				pSat[SatNo - 1] = 1;
			}
		}
	}
}
/*********************************************************************
@brief create dir
@param  pPath[I0]                 file path
@return Success or failure
@author CHC
@date   25 November 2022
**********************************************************************/
extern BOOL MkDirs(char* pPath)
{
	char str[512];
	int len = 0;
	if (NULL == pPath || strlen(pPath) < 1 || !strcmp(pPath, ""))
	{
		return FALSE;
	}
	strncpy(str, pPath, 512);
	str[511] = '\0';
	len = (int)strlen(str);

	for (int i = 0; i < len; i++)
	{
		if (str[i] == '\\')
		{
			str[i] = '\0';
			if (QC_ACCESS(str, 0) != 0)
			{
				if (0 != QC_MKDIR(str))
				{
					/** Error operation */
				}
			}
			str[i] = '\\';
		}
	}
	return TRUE;
}
/*---- get pure file name from complete file path ----*
* char* pFileName[i]    complete file path
* char* pPureFilename[o] pure file name with '\\'
*-------------------------------------------------------------*/
extern BOOL GetPureFilenameStr(char* pFileName, char* pPureFileName)
{
	int iChar;
	int iEnd = (int)strlen(pFileName);

	if(strlen(pPureFileName)>0)
	{
		strcpy(pPureFileName, "");
	}

	iChar = (int)strlen(pFileName);
	for (iChar; iChar >= 0; iChar--)
	{
		if (pFileName[iChar] == '\\' || pFileName[iChar] == '/')
		{
			break;
		}
	}
	if (iChar > 0)
	{
		strncpy(pPureFileName, pFileName + iChar + 1, ((size_t)iEnd - iChar - 1));
	}
	else
	{
		sprintf(pPureFileName, "%s", "");
		return FALSE;
	}
	return TRUE;
}
/*---- get dirpath from complete file path ----*
* char* pFileName[i]    complete file path
* char* pOutDir[o] dirpath with '\\'
*-------------------------------------------------------------*/
extern BOOL GetOutDirStr(char* pFileName, char* pOutDir)
{
	int iChar;
	int iEnd = (int)strlen(pFileName);

	iChar = (int)strlen(pFileName);
	for (iChar; iChar >= 0; iChar--)
	{
		if (pFileName[iChar] == '\\'|| pFileName[iChar] == '/')
		{
			iEnd = iChar;
			break;
		}
	}
	if (iChar > 0)
	{
		strncpy(pOutDir, pFileName, (size_t)iEnd + 1);
	}
	else
	{
		sprintf(pOutDir, "%s", "");
		return FALSE;
	}
	return TRUE;
}
/*---- get dirpath and file name without suffix from complete file path --*
* char* pPath[i]        OutDir path
* char* pFileName[i]    complete file path
* char* pOutFileName[o] OutDir path+filename without suffix
*-------------------------------------------------------------*/
extern BOOL GetOutFileStr(char* pPath, char* pFileName, char* pOutFileName)
{
	int iChar;
	int iEnd = (int)strlen(pFileName), iStart = -1;
	char pure_name[256] = { 0 };
	if (iEnd <= 2)
	{
		return FALSE;
	}

	for (iChar = (int)strlen(pFileName); iChar >= 0; iChar--)
	{
		if (pFileName[iChar] == '.')
		{
			iEnd = iChar;
			break;
		}
	}
	for (; iChar >= 0; iChar--)
	{
		if (pFileName[iChar] == '\\'|| pFileName[iChar] == '/')
		{
			iStart = iChar;
			break;
		}
	}
	if (iStart < iEnd)
	{
		strncpy(pure_name, pFileName + iStart + 1, ((size_t)iEnd - iStart-1));
		sprintf(pOutFileName, "%s%s", pPath, pure_name);
		return TRUE;
	}
	else
	{
		sprintf(pOutFileName, "%s", "");
		return FALSE;
	}
	
}