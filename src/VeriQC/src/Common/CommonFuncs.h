/**********************************************************************//**
			 VeriQC

			 Common Module
 *-
 @file CommonFuncs.h
 @author CHC
 @date 2022/04/05 14:22:31

@brief common used operations and functions for all the module

**************************************************************************/
#ifndef _COMMON_FUNCS_H_
#define _COMMON_FUNCS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__linux)
#define PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#elif defined(__aarch)

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "Common/DataTypes.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((unsigned int)(sizeof(x) / sizeof((x)[0])))
#endif

/** data is rounded up modulo 4/8 */
#define MEM_CEILBY8(d)     ((((d) >> 3) + (((d) & 0x7) != 0)) << 3)

#ifndef BIT_MASK
#define BIT_MASK(_n)  ((1u << (_n)) - 1)
#endif

#define ABS(_x) (((_x) > 0) ? (_x) : -(_x))

#ifndef CUBE
#define CUBE(x) ((x) * (x) * (x))
#endif

#ifndef MIN
#define MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))
#endif

#ifndef MAX
#define MAX(_x,_y) ((_x) < (_y) ? (_y) : (_x))
#endif

#ifndef SQUARE
#define SQUARE(_x) ((_x)*(_x))
#endif

/** error trap */
#ifdef WIN32
	#define ASSERT_DEBUG_PRINTF      printf
#else
	#define ASSERT_DEBUG_PRINTF      printf
#endif

#ifdef WIN32
	#define ERROR_TRAP(...)  \
		do{\
			ASSERT_DEBUG_PRINTF("ERROR:%s,%d", __FUNCTION__, __LINE__); \
			ASSERT_DEBUG_PRINTF("\n" __VA_ARGS__); \
			while (1); \
		} while (0)
#else
	#define ERROR_TRAP(...)  \
		do{\
			ASSERT_DEBUG_PRINTF("\r\n#E,%s,%d\r\n", __FUNCTION__, __LINE__);\
			ASSERT_DEBUG_PRINTF("\r\n" __VA_ARGS__); \
			ErrorTrap();\
		} while (0)

#endif

/** assert for debug  */
#define ASSERT(_NecessaryCondition_, ...)  \
	do{\
		if (!(_NecessaryCondition_))\
		{\
			ERROR_TRAP(__VA_ARGS__); \
		}\
	} while (0) //-V547



#if defined (__GNUC__)
	#ifdef  INLINE
	#undef 	INLINE
	#endif

	#define INLINE	static inline
#else
	#define INLINE	__inline
#endif

/** for cache line align  */
#if defined (__GNUC__)
	#define CACHE_LINE_ALIGN	__attribute__ ((aligned (32)))
#else
	#define CACHE_LINE_ALIGN
#endif

/** for double align  */
#if defined (__GNUC__)
	#define DOUBLE_ALIGN	__attribute__ ((aligned (8)))
#else
	#define DOUBLE_ALIGN
#endif



/** check the buffer tail integrity */
#define CheckBufferOverflow(Buffer)  \
		(0x5a5a5a5a != *(unsigned int *)((((PTR)&Buffer[0] + sizeof(Buffer))&(~0x3)) - 4) )



#ifdef WIN32
/** make new directory  */
#define MKDIR(PathStr, pFilePath)  \
	do{\
		char FileNameStr[256]; \
		sprintf(FileNameStr, PathStr "%s", pFilePath); \
		if (_access(FileNameStr, 0)) \
		{ \
			int iChar; \
			for (iChar = 1; iChar < sizeof(FileNameStr); iChar++) \
			{ \
				if ('\\' == FileNameStr[iChar-1]) \
				{ \
					char TempChar = FileNameStr[iChar]; \
					FileNameStr[iChar] = 0; \
					if (_access(FileNameStr, 0)) \
					{ \
						_mkdir(FileNameStr); \
					} \
					FileNameStr[iChar] = TempChar; \
				} \
				if ('\0' == FileNameStr[iChar]) \
				{ \
					break; \
				} \
			} \
		} \
	} while (0)
#elif __linux
#define MKDIR(PathStr, pFilePath)  \
	do{\
		char FileNameStr[256]; \
		sprintf(FileNameStr, PathStr "%s", pFilePath); \
		if (_access(FileNameStr, 0)) \
		{ \
			int iChar; \
			for (iChar = 1; iChar < sizeof(FileNameStr); iChar++) \
			{ \
				if ('\\' == FileNameStr[iChar-1]) \
				{ \
					char TempChar = FileNameStr[iChar]; \
					FileNameStr[iChar] = 0; \
					if (_access(FileNameStr, 0)) \
					{ \
						mkdir(FileNameStr, S_IRWXU); \
					} \
					FileNameStr[iChar] = TempChar; \
				} \
				if ('\0' == FileNameStr[iChar]) \
				{ \
					break; \
				} \
			} \
		} \
	} while (0)
#endif

/** save memory to file & load memory from file  */
#ifdef WIN32

/** for debug only  */
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <io.h>

#else  /**< #ifdef WIN32 */



#endif  /**< #ifdef WIN32 */



/**********************************************************************//**
@brief check whether float == 0

which the memory is 64 bit zero

@param dData  [In] input double

@return if is absolute zero

@author CHC
@date 2022/06/28
@note
History:\n
- 2022.06.28 Start up
**************************************************************************/
INLINE BOOL IsAbsoluteFloatZero(const float dData)
{
	const unsigned int * pInt  = (const unsigned int *)&dData;
	if(0 == *(pInt))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**********************************************************************//**
@brief check whether double == 0

which the memory is 64 bit zero

@param dData  [In] input double

@return if is absolute zero

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
**************************************************************************/
INLINE BOOL IsAbsoluteDoubleZero(const double dData)
{
	const unsigned int * pInt  = (const unsigned int *)&dData;
	if((0 == *(pInt++)) && (0 == *(pInt)))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/****************************************************************************//**
@brief Judge Where a double equal to Zero

@param Value [In] a double type data
@todo can be simplified

@retval 1 -- value Equal to zero; 0 -- value unequal to zero

@author CHC
@date 2022/04/06 11:10:39
@note
*******************************************************************************/
INLINE BOOL EqualZeroDouble(const double Value)
{
	if( ABS(Value) <= 1e-15 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**********************************************************************//**
@brief get 2^Exp as float

@param Exp [In] exponent of 2

@return 2^Exp

@author CHC
@date 2024/08/02
@note
History:
**************************************************************************/
INLINE float GetFloat2Exp(int Exp)
{
	unsigned int Data = (0x7F + Exp) << 23;
	return *(float *)&Data;
}

/**********************************************************************//**
@brief get 2^Exp as double

@param Exp [In] exponent of 2

@return 2^Exp

@author CHC
@date 2024/08/02
@note
History:
**************************************************************************/
INLINE double GetDouble2Exp(int Exp)
{
	union {
		unsigned int U32[2];
		double F64;
	}Data;

	Data.U32[1] = (0x3FF + Exp) << 20;
	Data.U32[0] = 0;
	return Data.F64;
}

/**********************************************************************//**
@brief get float * 2^Exp

@param Value  [In] input float
@param Exp    [In] 2 exp

@return the (Value * 2^Exp)

@author CHC
@date 2024/08/02
@note
**************************************************************************/
INLINE float FloatMultiple2Exp(float Value, signed int Exp)
{
	return Value * GetFloat2Exp(Exp);
}

/**********************************************************************//**
@brief get double * 2^Exp

@param Value  [In] input double
@param Exp    [In] 2 exp

@return the (Value * 2^Exp)

@author CHC
@date 2024/08/02
@note
**************************************************************************/
INLINE double DoubleMultiple2Exp(double Value, signed int Exp)
{
	return Value * GetDouble2Exp(Exp);
}

/**********************************************************************//**
@brief Get the (Value / 2^Scale) for float

@param Value [In] signed int
@param Scale [In] 2 exp

@return the (Value / 2^Scale)

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
- 2024.08.01 change to inline function, by CHC
**************************************************************************/
INLINE float FloatIScale2(signed int Value, signed int Scale)
{
	union {
		unsigned int U32;
		float F32;
	} Data;

	Data.F32 = (float)Value;
	if (Value != 0)
	{
		Data.U32 -= (Scale << 23);
	}

	return Data.F32;
}

/**********************************************************************//**
@brief Get the (Value / 2^Scale) for float

@param Value [In] signed int
@param Scale [In] 2 exp

@return the (Value / 2^Scale)

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
- 2024.08.01 change to inline function, by CHC
**************************************************************************/
INLINE float FloatUScale2(unsigned int Value, signed int Scale)
{
	union {
		unsigned int U32;
		float F32;
	} Data;

	Data.F32 = (float)Value;
	if (Value != 0)
	{
		Data.U32 -= (Scale << 23);
	}

	return Data.F32;
}

/**********************************************************************//**
@brief Get the (Value / 2^Scale) for double

@param Value [In] signed int
@param Scale [In] 2 exp

@return the (Value/2^Scale)

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
- 2024.08.01 change to inline function, using multiplication is faster on ARC, by CHC
**************************************************************************/
INLINE double DoubleIScale2(signed int Value, signed int Scale)
{
	return (double)Value * GetDouble2Exp(-Scale);
}
/**********************************************************************//**
@brief Get the (Value/2^Scale) for double

@param Value [In] signed int
@param Scale [In] 2 exp

@return the (Value/2^Scale)

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
- 2024.08.01 change to inline function, using multiplication is faster on ARC, by CHC
**************************************************************************/
INLINE double DoubleUScale2(unsigned int Value, signed int Scale)
{
	return (double)Value * GetDouble2Exp(-Scale);
}
/**********************************************************************//**
@brief Get the (Value/2^Scale) for double

@param Value [In] double
@param Scale [In] 2 exp

@return the (Value/2^Scale)

@author CHC
@date 2022/04/03 19:52:10
@note
History:\n
- 2022.04.03 Start up
- 2024.08.01 change to inline function, using multiplication is faster on ARC, by CHC
**************************************************************************/
INLINE double DoubleScale2(double Value, signed int Scale)
{
	return Value * GetDouble2Exp(-Scale);
}

int IsNaN(double dData);

#ifdef __cplusplus
}
#endif
#endif
