/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file    CRC24Q.h
@author  CHC
@date    2022/12/01
@brief   common bit stream processing

**************************************************************************/
#ifndef _CRC_24Q_H_
#define _CRC_24Q_H_
#include "DataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int CalcCRC24Q(const unsigned char *pData, unsigned int DataSize);
BOOL JudgeCRC24Q(const unsigned char *pData, unsigned int ByteNum);
unsigned int JudgeCRC24QForU32(const unsigned int *pBitStream, unsigned int BitNum);
unsigned int CalcCRC24QForU32(const unsigned int *pBitStream, unsigned int BitNum);

#ifdef __cplusplus
}
#endif



#endif /**< _CRC_24Q_H_ */
