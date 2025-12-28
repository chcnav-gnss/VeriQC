/*************************************************************************//**
		VeriQC

	Interface Module
 *- 
@file   QCCommon.h
@author CHC
@date   5 September 2022
@brief  Some common interfaces
*****************************************************************************/
#ifndef _QC_COMMON_FUNC_H_
#define _QC_COMMON_FUNC_H_

#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "VeriQCSPP.h"
#include "string.h"
#include "DataQC.h"
#include "PrintFunc.h"
#include "MsgConvertDataTypes.h"

/** Number of input file Define */
#define MAX_OBS_CODEC_BUF_SIZE     (50*1024)
#define MAX_NAV_CODEC_BUF_SIZE     512
#define STRFMT_NONE                -1

#define QC_SCAN_STATION_FUNC_INDEX  0
#define QC_PRE_PROCESS_FUNC_INDEX   1

/**---------------------------------struct for QC------------------------------------------*/

/** GNSS Cfg */

typedef struct _CHCQC_RINEXCFG_SIM_T
{
	char CfgName[MAX_FILE_NAME_LENGTH];
	unsigned int DataType;                   /**< data type */
	unsigned int OutputType;                 /**< data output type */
	unsigned int RinexVer;                   /**< output rinex file version */
	char OutDir[MAX_FILE_NAME_LENGTH];       /**< rinex output file dir */
	int FileNum;
	char **ppRawFile;                        /**< gnss raw file name */
}CHCQC_RINEXCFG_SIM_T;

typedef struct _CHCQC_GNSSCFG_SIM_T
{
	int Mode;                                         /**< GNSS QC mode(0:zero baseline 1:diff sat and epoch 2:short baseline 3:tripleDiff epoch) */
	int Sys;                                          /**< gnss system choose(GPS:G,SBAS:S,GLO:R,GLA:E,QZS:J,BDS:C,IRNSS:I) */
	int FileNum;
	unsigned int EleDeg;                              /**< ele mask deg */
	char OutDir[MAX_FILE_NAME_LENGTH];                /**< output file dir */
	char CfgName[MAX_FILE_NAME_LENGTH];               /**<  */
	
	char **ppRroverFile;                              /**< rover obs file name and path [MAX_FILE_NAME_LENGTH*fileNum] */
	char **ppBaseFile;                                /**< base obs file name and path [MAX_FILE_NAME_LENGTH*fileNum] */
	double **ppBaseTrueXYZ;                           /**< [3*fileNum] */
	double **ppRoverTrueXYZ;                          /**< [3*fileNum] */

	int SpecifyRefPRN[QC_MAX_NUM_SYS];                /**< user specify reference satellite PRN, GPS, BDS, GLO, GAL, from 1 */
}CHCQC_GNSSCFG_SIM_T;

typedef struct _QC_CODEC_INFO_T
{
	FILE* fpInputFile;                   /**< file pointer */
	RINEX_INFO_T RinexInfo;              /**< Rinex file info */
	RINEX_FILE_INDEX_E MsgConvertRinexIndex;/**< Rinex file index */
	MSG_DECODE_T* pMsgDecode;            /**< Msg Decoder */
	uint32_t EpochWait;                  /**< epoch number need to waite when the time difference */
	int8_t StreamFormat;				 /**< STRFMT_???*/
}QC_CODEC_INFO_T;


/**================================= functions ==============================================*/
extern BOOL AdjustStr(char* pString, int MaxSize);
extern int Str2Sys(char* pString);
extern void Str2SnrGroup(char* pString, float* pSNRGroup, int* pSize);
extern void Sat2BanSat(char* pString, unsigned char* pSat);
#ifdef WIN32
#define QC_MKDIR(_FilePath)  _mkdir(_FilePath)
#define QC_ACCESS(_FilePath,_Mode)  _access(_FilePath,_Mode)
#else
#define QC_MKDIR(_FilePath)  mkdir(_FilePath,S_IRWXU)
#define QC_ACCESS(_FilePath,_Mode)  access(_FilePath,_Mode)
#endif
extern BOOL GetOutFileStr(char* pPath, char* pFileName, char* pOutFileName);
extern BOOL GetPureFilenameStr(char* pFileName, char* pPureFileName);
extern BOOL GetOutDirStr(char* pFileName, char* pOutDir);

/** read cfg file */
int Obs2FreId(int8_t* pObsString, int Sys, CHCQC_GNSSCFG_T* pGNSSCfg);
int DecodeFreqList(int8_t* pFreqString, int Sys, CHCQC_GNSSCFG_T* pGNSSCfg);
BOOL ReadCfg(char* pFilenName, CHCQC_RINEXCFG_T* pRinexCfg, CHCQC_GNSSCFG_T* pGNSSCfg, CHCQC_RINEXCFG_SIM_T* pRinexCfgSimple, CHCQC_GNSSCFG_SIM_T* pGNSSCfgSimple, BOOL* pInvail);
void InitGnssCfgSimple(CHCQC_GNSSCFG_SIM_T* pGNSSCfgSimple);
void InitRinexCfgSimple(CHCQC_RINEXCFG_SIM_T* pRineCfgSimple);
BOOL ReadRinexCfgSimple(FILE* fpInputFile, CHCQC_RINEXCFG_SIM_T* pRineCfgSimple);
BOOL ReadGNSSCfgSimple(FILE* fpInputFile, CHCQC_GNSSCFG_SIM_T* pGNSSCfgSimple);

#endif