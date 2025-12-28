/**********************************************************************//**
        VeriQC

       Common Module
*-
@file    Memory.h
@author  CHC
@date    2025/10/15
@brief   memory malloc/free related funcs

**************************************************************************/
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>
#include "VeriQC.h"

VERIQC_ALLOCATOR_T* VeriQCGetAllocatorFast(void);

#define VERIQC_MALLOC_FAST      VeriQCGetAllocatorFast()->LocalMalloc
#define VERIQC_FREE_FAST        VeriQCGetAllocatorFast()->LocalFree
#define VERIQC_CALLOC_FAST      VeriQCGetAllocatorFast()->LocalCalloc
#define VERIQC_REALLOC_FAST     VeriQCGetAllocatorFast()->LocalRealloc

extern int VeriQCReplaceAllocatorFast(VERIQC_ALLOCATOR_T* pAllocatorIn);
extern int VeriQCReplaceAllocator(VERIQC_ALLOCATOR_T* pAllocatorIn);

#endif