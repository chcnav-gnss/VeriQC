/*************************************************************************//**
           VeriQC
         SPP Module
 *-
@file   MemoryManager.h
@author CHC
@date   5 September 2022
@brief  manage momery allocator/free
*****************************************************************************/

#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__
#include <stdint.h>

#include "Memory.h"

//definition of memory manager
typedef struct _VERIQC_MEM_MANAGER_T
{
    void* pPtr0;                       //head address of this memory block
    size_t MemSize;                    //size of the memory block
    size_t MemOffset;                  //current pointer offset to the head address
    int8_t MemAlign;                   //allignment bytes, must be 1,2,4,8,16,32. default:8 bytes be safe
    int8_t MemFirst;                   //first align pos 
}VERIQC_MEM_MANAGER_T;

//create a memory manager
extern int32_t VeriQCMemManagerInit(VERIQC_MEM_MANAGER_T* pMemManager);

//free a memory manager
//return 0:ok, -1:error
extern int32_t VeriQCMemManagerFree(VERIQC_MEM_MANAGER_T* pMemManager);
extern int32_t VeriQCMemManagerFreeFast(VERIQC_MEM_MANAGER_T* pMemManager);

//add size to be summed
//type 0=array, 1=structer
//return 0:ok, -1:error
extern int32_t VeriQCAddSize(VERIQC_MEM_MANAGER_T* pMemManager, size_t Size, int8_t Type);

//start to allocate the whole memory whose size is calculated before.
//return 0:ok, -1:error
extern int32_t VeriQCAllocateWhole(VERIQC_MEM_MANAGER_T* pMemManager);
extern int32_t VeriQCAllocateWholeFast(VERIQC_MEM_MANAGER_T* pMemManager);

//request memory to the manager
extern void* VeriQCRequestMemory(VERIQC_MEM_MANAGER_T* pMemManager, size_t Size, int8_t Type);

#endif
