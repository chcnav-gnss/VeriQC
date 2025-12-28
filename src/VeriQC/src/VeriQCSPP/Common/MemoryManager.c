#include <string.h>
#include <stdint.h>
#include "GNSSBase.h"

const int8_t g_BitOffs[33] = { 0,0,1,0,2,0,0,0,3,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5};

extern int32_t VeriQCMemManagerInit(VERIQC_MEM_MANAGER_T* pMemManager)
{
    if (!pMemManager) return -1;
    pMemManager->MemSize = 0;
    pMemManager->pPtr0 = NULL;
    pMemManager->MemOffset = 0;
    pMemManager->MemFirst = 0;

    pMemManager->MemAlign = 4;

    return 0;
}

extern int32_t VeriQCMemManagerFree(VERIQC_MEM_MANAGER_T* pMemManager)
{
    if (!pMemManager) return -1;
    if (pMemManager->MemOffset != pMemManager->MemSize)
    {
        pMemManager->MemOffset += 0;
    }
    pMemManager->MemSize = 0;
    pMemManager->MemOffset = 0;
    pMemManager->MemAlign = 4;
    pMemManager->MemFirst = 0;
    if (pMemManager->pPtr0)
    {
        VERIQC_FREE(pMemManager->pPtr0);
        pMemManager->pPtr0 = NULL;
    }
    return 0;
}

extern int32_t VeriQCMemManagerFreeFast(VERIQC_MEM_MANAGER_T* pMemManager)
{
    if (!pMemManager) return -1;
    if (pMemManager->MemOffset != pMemManager->MemSize)
    {
        pMemManager->MemOffset += 0;
    }
    if (pMemManager->pPtr0)
    {
        VERIQC_FREE_FAST(pMemManager->pPtr0);
        pMemManager->pPtr0 = NULL;
    }
    return 0;
}

extern int32_t VeriQCAddSize(VERIQC_MEM_MANAGER_T* pMemManager, size_t Size, int8_t Type)
{
    size_t ntotal = 0;
    int8_t align = 0;
    if (!pMemManager) return -1;
    if (Size == 0) return 0;
    ntotal = pMemManager->MemSize + Size;
    if (Type == 1)
    {
        align = pMemManager->MemAlign;
        ntotal = ((Size + (align - 1)) >> g_BitOffs[align]) << g_BitOffs[align];
        ntotal += ((pMemManager->MemSize + (align - 1)) >> g_BitOffs[align]) << g_BitOffs[align];
    }
    pMemManager->MemSize = ntotal;
    return 0;
}

extern int32_t VeriQCAllocateWhole(VERIQC_MEM_MANAGER_T* pMemManager)
{
    if (!pMemManager) return -1;
    if (pMemManager->MemSize > 0)
    {
        pMemManager->pPtr0 = VERIQC_MALLOC(pMemManager->MemSize);
        if (!pMemManager->pPtr0)
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

extern int32_t VeriQCAllocateWholeFast(VERIQC_MEM_MANAGER_T* pMemManager)
{
    if (!pMemManager) return -1;
    if (pMemManager->MemSize > 0)
    {
        pMemManager->pPtr0 = VERIQC_MALLOC_FAST(pMemManager->MemSize);
        if (!pMemManager->pPtr0)
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

extern void* VeriQCRequestMemory(VERIQC_MEM_MANAGER_T* pMemManager, size_t Size, int8_t Type)
{
    void* ptr = NULL;
    int8_t align = 0;
    size_t offset = 0;

    if (!pMemManager)
    {
        return NULL;
    }
    if (Size == 0) {
        return NULL;
    }
    if (!pMemManager->pPtr0 || pMemManager->MemSize < Size) {
        return NULL;
    }
    align = pMemManager->MemAlign;
    if (Type == 1)
    {
        pMemManager->MemOffset = ((pMemManager->MemOffset + (align - 1)) >> g_BitOffs[align]) << g_BitOffs[align];
    }
    offset = pMemManager->MemOffset + Size;
    if (offset > pMemManager->MemSize) {
        return NULL;
    }
    ptr = (void*)((uint8_t*)pMemManager->pPtr0 + pMemManager->MemOffset + pMemManager->MemFirst);
    pMemManager->MemOffset = offset;

    return ptr;
}
