/**********************************************************************//**
		VeriQC

	   Common Module
*-
@file   DataTypes.h
@author CHC
@date   2022/04/07 19:52:10
@brief  Type def for basic types

**************************************************************************/
#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef int INTFLOAT3;
typedef int INTFLOAT16;
typedef int INTFLOAT20;
typedef int INTFLOAT31;

typedef double DOUBLE32;
typedef double DOUBLE20;

typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifdef WIN32
#include <BaseTsd.h>
#endif


#ifndef NULL
#define NULL ((void *)0)
#endif
#if defined (__GNUC__)
	#define INT64 long long int
	#define UINT64 long long unsigned int
	#define CONSTANTS64(Value) (Value##LL)
	#define CONSTANTU64(Value) (Value##ULL)
#else
	#ifndef WIN32
	typedef __int64 INT64;
	typedef unsigned __int64 UINT64;
	#endif
	#define CONSTANTS64(Value) (Value##i64)
	#define CONSTANTU64(Value) (Value##ui64)
#endif

#if defined(__GNUC__)
#define _PACKED_    __attribute__((packed))
#else
#define _PACKED_
#endif

/** used for int/int64 convert to pointer */
#ifdef __64BIT__
typedef INT64 PTR;
#define GetAddrLow32(_Addr) ((unsigned int)(((INT64)_Addr) & 0xFFFFFFFF))
#define GetFullAddr(_AddrHigh,_Addr) 	((_AddrHigh) | ((INT64)_Addr))
#else
typedef int PTR;
#define GetAddrLow32(_Addr)				((unsigned int)(_Addr))
#define GetFullAddr(_AddrHigh,_Addr) 	(_Addr)
#endif

#define NA				0xFF
#define NA8				0xFF
#define NA16			0xFFFF
#define NA32			0xFFFFFFFF
#define NA64			0xFFFFFFFFFFFFFFFFull

typedef int (*CALLBACK_FUNC_T) (void *pParam);

#define GET_LOW32(_Data64Bit) (unsigned int)((_Data64Bit) & 0xFFFFFFFF)
#define GET_HIGH32(_Data64Bit) (unsigned int)(((_Data64Bit) >> 32) & 0xFFFFFFFF)

#ifdef __cplusplus
}
#endif

#endif
