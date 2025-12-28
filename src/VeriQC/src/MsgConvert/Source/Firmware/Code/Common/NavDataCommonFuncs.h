/**********************************************************************//**
		VeriQC

		MsgConvert Module
*-
@file   NavDataCommonFuncs.h
@author CHC
@date   2022/11/18 11:57:27
@brief  common functions of navigation msg

**************************************************************************/
#ifndef _NAV_DATA_COMMON_FUNCTIONS_H_
#define _NAV_DATA_COMMON_FUNCTIONS_H_

#include "Common/DataTypes.h"

#include "GNSSNavDataType.h"

signed int CalcEphInterimParam(double SqrtGM, double DotOmegaEarth, UNI_EPHEMERIS_T* pEph);

signed int CalcEphInterimParamGEO(double SqrtGM, double DotOmegaEarth, UNI_EPHEMERIS_T* pEph);
signed int CalcEphInterimParamBD3(UNI_EPHEMERIS_T* pEph);

#endif /**< _NAV_DATA_COMMON_FUNCTIONS_H_ */
