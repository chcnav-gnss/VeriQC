/**********************************************************************//**
        VeriQC

       Common Module
*-
@file    Memory.c
@author  CHC
@date    2025/10/15
@brief   memory malloc/free related funcs

**************************************************************************/
#include <malloc.h>
#include "Memory.h"

static VERIQC_ALLOCATOR_T s_VeriQCAllocator = { malloc, realloc, calloc, free };
static unsigned char s_VeriQCAllocatorFastSetMark = 0;
static VERIQC_ALLOCATOR_T s_VeriQCAllocatorFast = { malloc, realloc, calloc, free };

/** VERIQC get functions */
VERIQC_ALLOCATOR_T* VeriQCGetAllocator(void)
{
    return &s_VeriQCAllocator;
}
VERIQC_ALLOCATOR_T* VeriQCGetAllocatorFast(void)
{
    return &s_VeriQCAllocatorFast;
}
/** VERIQC replace functions */
extern int VeriQCReplaceAllocator(VERIQC_ALLOCATOR_T* pAllocatorIn)
{
    if (!pAllocatorIn)
        return -1;
    if (pAllocatorIn->LocalMalloc)
        s_VeriQCAllocator.LocalMalloc = pAllocatorIn->LocalMalloc;
    if (pAllocatorIn->LocalRealloc)
        s_VeriQCAllocator.LocalRealloc = pAllocatorIn->LocalRealloc;
    if (pAllocatorIn->LocalCalloc)
        s_VeriQCAllocator.LocalCalloc = pAllocatorIn->LocalCalloc;
    if (pAllocatorIn->LocalFree)
        s_VeriQCAllocator.LocalFree = pAllocatorIn->LocalFree;

    if (!s_VeriQCAllocatorFastSetMark)
    {
        VeriQCReplaceAllocatorFast(pAllocatorIn);
        s_VeriQCAllocatorFastSetMark = 0;
    }
    return 0;
}

extern int VeriQCReplaceAllocatorFast(VERIQC_ALLOCATOR_T* pAllocatorIn)
{
    if (!pAllocatorIn)
        return -1;
    if (pAllocatorIn->LocalMalloc)
        s_VeriQCAllocatorFast.LocalMalloc = pAllocatorIn->LocalMalloc;
    if (pAllocatorIn->LocalRealloc)
        s_VeriQCAllocatorFast.LocalRealloc = pAllocatorIn->LocalRealloc;
    if (pAllocatorIn->LocalCalloc)
        s_VeriQCAllocatorFast.LocalCalloc = pAllocatorIn->LocalCalloc;
    if (pAllocatorIn->LocalFree)
        s_VeriQCAllocatorFast.LocalFree = pAllocatorIn->LocalFree;

    s_VeriQCAllocatorFastSetMark = 1;
    return 0;
}
