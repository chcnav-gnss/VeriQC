/*************************************************************************//**
		   VeriQC
	Veri Quality Check
 *- 
@file   Entrance.c
@author CHC
@date   5 September 2022
@brief  Entrance and exit of VeriQC
*****************************************************************************/
#include "VeriQCSPP.h"
#include "QCCommon.h"
#include "CommonBase.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


static void ChangeSim2CfgRinex(CHCQC_RINEXCFG_T* pCfg, CHCQC_RINEXCFG_SIM_T* pCfgSim, int iFile)
{
	if (iFile == 0)
	{
		pCfg->DataType = pCfgSim->DataType;
		pCfg->RinexVer = pCfgSim->RinexVer;
		memset(pCfg->OutDir, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
		strncpy(pCfg->OutDir, pCfgSim->OutDir, strlen(pCfgSim->OutDir));
	}
	memset(pCfg->RawFile, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	strncpy(pCfg->RawFile, pCfgSim->ppRawFile[iFile], strlen(pCfgSim->ppRawFile[iFile]));
}
static void ChangeSim2CfgQC(CHCQC_GNSSCFG_T* pCfg, CHCQC_GNSSCFG_SIM_T* pCfgSim,int iFile)
{
	if (iFile == 0)
	{
		pCfg->Mode = pCfgSim->Mode;
		pCfg->Sys = pCfgSim->Sys;
		pCfg->EleDeg = pCfgSim->EleDeg;
		memset(pCfg->OutDir, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
		strncpy(pCfg->OutDir, pCfgSim->OutDir, strlen(pCfgSim->OutDir));
	}
	memset(pCfg->RoverFile,0,sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset(pCfg->BaseFile, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset(pCfg->RoverTrueXYZ, 0, sizeof(double) * 3);
	memset(pCfg->BaseTrueXYZ, 0, sizeof(double) * 3);
	strncpy(pCfg->RoverFile, pCfgSim->ppRroverFile[iFile], strlen(pCfgSim->ppRroverFile[iFile]));
	strncpy(pCfg->BaseFile, pCfgSim->ppBaseFile[iFile], strlen(pCfgSim->ppBaseFile[iFile]));
	memcpy(pCfg->RoverTrueXYZ, pCfgSim->ppRoverTrueXYZ[iFile], sizeof(double) * 3);
	memcpy(pCfg->BaseTrueXYZ, pCfgSim->ppBaseTrueXYZ[iFile], sizeof(double) * 3);
	if (fabs(pCfg->RoverTrueXYZ[0])>1&& fabs(pCfg->RoverTrueXYZ[1]) > 1&& fabs(pCfg->RoverTrueXYZ[2]) > 1)
	{
		pCfg->IsInputTrueXYZ = 1;
	}
	else
	{
		pCfg->IsInputTrueXYZ = 0;
	}
}

static void ConvertGNSSCfgFileStr(char* pGNSSCfg, char* pGNSSCfgSim)
{
	int iChar, size_str, iScan, ScanNum = 0;
	char scan_str[5] = { 0 };

	size_str = (int)strlen(pGNSSCfgSim);
	for (iChar = size_str - 1; iChar >= 0; iChar--) 
	{
		for (iScan = 3; iScan > 0; iScan--)
		{
			scan_str[iScan] = scan_str[iScan - 1];
		}
		scan_str[0] = pGNSSCfgSim[iChar];
		if (strcmp(scan_str, "_sim") == 0)
		{
			iScan += 0;
			strncpy(pGNSSCfg, pGNSSCfgSim, iChar);
			sprintf(pGNSSCfg, "%s%s", pGNSSCfg, pGNSSCfgSim+ iChar+4);
			break;
		}
	}
}

/* Single file processing */
int main(int argc, char** argv)
{
	char s_GnssCfg[MAX_FILE_NAME_LENGTH] = { 0 };
	int infileNum, iFile;
	CHCQC_RINEXCFG_T *pRinexCfg = (CHCQC_RINEXCFG_T*)VERIQC_MALLOC(sizeof(CHCQC_RINEXCFG_T));
	CHCQC_GNSSCFG_T *pGnssCfg = (CHCQC_GNSSCFG_T*)VERIQC_MALLOC(sizeof(CHCQC_GNSSCFG_T));
	CHCQC_GNSSCFG_SIM_T* pGnssCfg_sim = (CHCQC_GNSSCFG_SIM_T*)VERIQC_MALLOC(sizeof(CHCQC_GNSSCFG_SIM_T));
	CHCQC_RINEXCFG_SIM_T* pRinexCfg_sim = (CHCQC_RINEXCFG_SIM_T*)VERIQC_MALLOC(sizeof(CHCQC_RINEXCFG_SIM_T));
	QC_RESULT_T* pQCRes = (QC_RESULT_T*)VERIQC_MALLOC(sizeof(QC_RESULT_T));
	BOOL invail_cfg[5] = { 0 }; /**< RinexDecode, GNSS QC, IMU QC, RinexDecode Simple,  GNSS QC Simple*/

	PrintfQC("CHCQC2.0    VerNum=%d VerStr=%s\r\n\n", GetQCVersion(), GetQCVersionStr());

	//_CrtSetBreakAlloc(603199);

	/** init config */
	InitRinexCfg(pRinexCfg);
	InitGNSSCfg(pGnssCfg);
	InitGnssCfgSimple(pGnssCfg_sim);
	InitRinexCfgSimple(pRinexCfg_sim);
	SetExePath(argv[0]);
	/** read config file */
	if (argc <2 ) 
	{
		PrintfQC("There is no cfg!\r\n");
		return 0;	
	}
	else 
	{
		for (infileNum = 1; infileNum < argc; infileNum++) 
		{
			if (argv[infileNum])
			{
				ReadCfg(argv[infileNum], pRinexCfg, pGnssCfg, pRinexCfg_sim, pGnssCfg_sim, invail_cfg);
			}
		}
	}
	/** end read config file */

	/** decode raw file */
 	if (invail_cfg[0])
	{
		RinexProcesser(pRinexCfg);
		if (pRinexCfg->Pause)
		{
			system("pause");
		}
		else
		{
		}
	}
	else
	{
	}

	//memset(pQCRes, 0, sizeof(QC_RESULT_T));

	/** QC process raw file */
	if (invail_cfg[1])
	{
		pGnssCfg->bIsOutputDataList = 1;
		GNSSProcesser(pGnssCfg, pQCRes);
	}
	else
	{
	}

	/** decode process raw mutli_files */
	if (invail_cfg[3])
	{
		ConvertGNSSCfgFileStr(s_GnssCfg, pRinexCfg_sim->CfgName);
		ReadCfg(s_GnssCfg, pRinexCfg, pGnssCfg, pRinexCfg_sim, pGnssCfg_sim, invail_cfg);
		PrintfQC("<Rinex log>:file number is %d \n", pRinexCfg_sim->FileNum);
		for (iFile = 0; iFile < pRinexCfg_sim->FileNum; iFile++)
		{
			ChangeSim2CfgRinex(pRinexCfg, pRinexCfg_sim, iFile);
			PrintfQC("<Rinex log>:file[%d] is  being processed\n", iFile);
			RinexProcesser(pRinexCfg);
		}
		if (pRinexCfg->Pause)
		{
			system("pause");
		}
		else
		{
		}
	}
	/** QC process raw mutli_files */
	if (invail_cfg[4])
	{
		ConvertGNSSCfgFileStr(s_GnssCfg, pGnssCfg_sim->CfgName);
		ReadCfg(s_GnssCfg, pRinexCfg, pGnssCfg, pRinexCfg_sim, pGnssCfg_sim, invail_cfg);
		PrintfQC("<GNSSQC msg>:file number is %d \n", pGnssCfg_sim->FileNum);
		for (iFile= 0; iFile < pGnssCfg_sim->FileNum; iFile++)
		{
			ChangeSim2CfgQC(pGnssCfg, pGnssCfg_sim, iFile);
			PrintfQC("<GNSSQC msg>:file[%d] is  being processed\n", iFile);
			GNSSProcesser(pGnssCfg, pQCRes);
		}
	}
	else
	{
	}

	/** Clear memory */
	VERIQC_FREE(pRinexCfg);
	VERIQC_FREE(pGnssCfg);
	VERIQC_FREE(pGnssCfg_sim);
	VERIQC_FREE(pRinexCfg_sim);
	VERIQC_FREE(pQCRes);

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
