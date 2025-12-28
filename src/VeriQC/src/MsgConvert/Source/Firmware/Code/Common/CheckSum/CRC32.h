/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file CRC32.c
@author CHC

@date 2022/11/30 10:32:57
@brief CRC32 for common use

**************************************************************************/
#ifndef _CRC32_H_
#define _CRC32_H_

#ifdef __cplusplus
extern "C" {
#endif


unsigned int CalculateCRC32(unsigned char * pDataBuffer, int BuffLength);
unsigned int CalculateCRC32Continue(unsigned long CRC, unsigned char * pDataBuffer, int BuffLength);


#ifdef __cplusplus
}
#endif
#endif
