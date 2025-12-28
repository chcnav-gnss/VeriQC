/**********************************************************************//**
		VeriQC

		HRCX Encoder Module
*-
@file   HRCXIDDefine.h
@author CHC
@date   2024/02/27
@brief

**************************************************************************/
#ifndef _HRCX_ID_DEFINES_H_
#define _HRCX_ID_DEFINES_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** HRCX message id */
#define HRCX_MSG_ID_HRCXOBS				(23401)
#define HRCX_MSG_ID_HRCXEPH				(23402)
#define HRCX_MSG_ID_HRCXSOL				(23403)

/** HRCX message internal index */
#define HRCX_MSG_INDEX_HRCXOBS			(0)
#define HRCX_MSG_INDEX_HRCXEPH			(HRCX_MSG_INDEX_HRCXOBS + 1)
#define HRCX_MSG_INDEX_HRCXSOL			(HRCX_MSG_INDEX_HRCXEPH + 1)

#define HRCX_MSG_INDEX_MAX				(HRCX_MSG_INDEX_HRCXSOL + 1)

#ifdef __cplusplus
}
#endif

#endif /** _HRCX_ID_DEFINES_H_ */

