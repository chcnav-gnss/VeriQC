/**********************************************************************//**
		VeriQC

	   OS dependency Module
*-
@file OSDependent.h
@author CHC
@date 2022/04/07 14:11:27
@brief common methods that OS provides

**************************************************************************/
#ifndef _OS_DEPENDENT_H_
#define _OS_DEPENDENT_H_

/** Section 1: define OS types */
#define _OS_WINDOWS_		1
#define _OS_FREERTOS_		2
#define _OS_THREAD_X_		3
#define _OS_LINUX_			4

/** Section 2: judge  _TARGET_OS_ */
#ifdef WIN32
#define _TARGET_OS_ _OS_WINDOWS_
#ifdef _WIN64
  /** define something for Windows (64-bit only) */
#else
  /** define something for Windows (32-bit only) */
#endif

#elif __linux__
#define _TARGET_OS_ _OS_LINUX_

#elif defined __THREAD_X__
#define _TARGET_OS_ _OS_THREAD_X_

#else
#define _TARGET_OS_ _OS_FREERTOS_
#define FREE_RTOS 1
#endif

extern void LOW_LEVEL_PRINTF(char* fmt, ...);


#if _TARGET_OS_ == _OS_FREERTOS_
#define LowLevelPrint(...)		LOW_LEVEL_PRINTF(__VA_ARGS__)

#elif _TARGET_OS_ == _OS_WINDOWS_

#define LowLevelPrint	printf

#elif _TARGET_OS_ == _OS_THREAD_X_
#define LowLevelPrint(...)		LOW_LEVEL_PRINTF(__VA_ARGS__)
#else
#define LowLevelPrint(format,...) 	(void) 0
#endif

/** Section 9: memory functions include */
#if _TARGET_OS_ == _OS_WINDOWS_
#include <memory.h>
#include <stdlib.h>
#define MEMCPY memcpy
#define MEMSET memset
#define MEMCMP memcmp
#else
#include <string.h>
#ifndef MEMCPY
#define MEMCPY memcpy
#endif
#define MEMSET memset
#define MEMCMP memcmp

#endif


/** Section 7: task control include */

#if  _TARGET_OS_ == _OS_THREAD_X_

#define FREE   				BasicFree
#define MALLOC 				BasicMalloc
#define CALLOC 				BasicCalloc
#define MEM_INIT			BasicMemoryInit

#elif _TARGET_OS_ == _OS_WINDOWS_

#define FREE                free
#define MALLOC              malloc
#define CALLOC              calloc
#define MEM_INIT

#else

#define FREE                free
#define MALLOC              malloc
#define CALLOC              calloc
#define MEM_INIT

#endif

/** Section 12: end of line include */
#if _TARGET_OS_ == _OS_WINDOWS_
#define EOL "\n"
#elif _TARGET_OS_ == _OS_FREERTOS_
#define EOL "\r\n"
#elif _TARGET_OS_ == _OS_THREAD_X_
#define EOL "\r\n"
#elif _TARGET_OS_ == _OS_MAC_
#define EOL "\r"
#else
#define EOL "\n"
#endif

#endif
