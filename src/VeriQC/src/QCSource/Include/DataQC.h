/*************************************************************************//**
           VeriQC
    Data Quality Check Module
 *-
@file   DataDC.h
@author CHC
@date   28 February 2022
@brief  Some external function interfaces
*****************************************************************************/
#ifndef _DATA_QC_H_
#define _DATA_QC_H_
#include "stdint.h"
#include "VeriQC.h"
#include "Memory.h"

#define MAXNUM_FIlE 500

#define QC_MIN(x,y)    ((x)<(y)?(x):(y))
#define QC_MAX(x,y)    ((x)>(y)?(x):(y))
#define QC_SQRT(x)     ((x)<=0.0?0.0:sqrt(x))
#define QC_SQR(x)      ((x)*(x))
//#define INS_ENABLE

#define CLIGHT		        299792458.0 		/**< speed of light (m/s) */
#define PI                  3.1415926535897932  /**< pi */
#define D2R                 (PI/180.0)          /**< deg to rad */
#define R2D                 (180.0/PI)          /**< rad to deg */

typedef enum _QC_MODE_E
{
	QC_MODE_NONE =   -1,               /**< -1:NULL mode */
	QC_MODE_ZERO_BL = 0,               /**< 0: Zero Baseline, reserved */
	QC_MODE_DIFF_SE = 1,               /**< 1: diff sat and epoch, reserved */
	QC_MODE_MINI_BL = 2,               /**< 2: short baseline, reserved */
	QC_MODE_DIFF_TF = 3,               /**< 3: tripleDiff epoch, reserved */
	QC_MODE_DIFF_MAX = 0xFFFFFFFF
}QC_MODE_E;

typedef enum _QC_GNSS_SYSID_E
{
	QC_GNSS_SYSID_NUL   = 0,     //NULL
	QC_GNSS_SYSID_GPS   = 1,     //GPS
	QC_GNSS_SYSID_SBAS  = 2,     //SBAS
	QC_GNSS_SYSID_GLN   = 3,     //GLONASS
	QC_GNSS_SYSID_GAL   = 4,     //Galileo
	QC_GNSS_SYSID_QZS   = 5,     //qzss
	QC_GNSS_SYSID_BDS   = 6,     //BDS
	//QC_GNSS_SYSID_IRN   = 7,     //IRNSS(reserve)
	QC_GNSS_SYSID_MAX = 0xFFFFFFFF
}QC_GNSS_SYSID_E;

extern uint8_t QCObs2Code(const int8_t* pObs, int32_t* pFreq);
extern double QCSatAzEl(const double* pPos, const double* pE, double* pAzEl);
extern double QCStr2Num(const int8_t* pStr, int32_t Pos, int32_t Width);
extern int QCCode2FreID(int Sys, int Code);
extern int QCSatNo2Str(int SatNo, char* pStr);
extern void QCRecordEpoch(QC_OBS_T* pObs, int AntID, QC_RESULT_T* pQcRes, QC_OPT_T* pOpt);

extern int QCSys2Id(int Sys);
extern int QCId2Sys(int Id);
extern int QCId2SysFreNum(int Id);

extern char* GetQCBranchStr(void);

extern int QCLogTrace(const char* pFormat, ...);
extern void QCOpenLogfile(char* pFileName);
extern void QCCloseLogfile();

extern void ResetQCSlipInfo();

#endif