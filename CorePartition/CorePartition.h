//
//  CorePartition.hpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//

#ifndef CorePartition_hpp
#define CorePartition_hpp


#ifdef __cplusplus
extern "C"{
#else
#define false 0
#define true 1
#define bool uint8_t
#endif
    
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

    
    static const char CorePartition_version[] = "V2.1 Compiled:" __TIMESTAMP__;
    
    bool CorePartition_Start (size_t nThreadPartitions);
    
    bool CreatePartition (void(*pFunction)(void), size_t nStackMaxSize, uint32_t nNice);
    
    uint8_t CorePartition_SetCurrentTimeInterface (uint64_t (*getCurrentTimeInterface)(void));
    uint8_t CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(uint64_t nSleepTime));

    void join (void);
    
    void yield(void);
    
    size_t CorePartition_GetPartitionID(void);
    
    size_t CorePartition_GetPartitionStackSize(void);
    size_t CorePartition_GetPartitionMaxStackSize(void);

    
    size_t CorePartition_GetPartitionUsedMemorySize(void);
    size_t CorePartition_GetPartitionAllocatedMemorySize(void);

    size_t CorePartition_GetThreadStructSize (void);

    size_t CorePartition_GetThreadStructSize(void);
    
    bool CorePartition_IsAllCoresStarted(void);
        
    uint32_t CorePartition_GetCoreNice(void);
    
    void CorePartition_SetCoreNice (uint32_t nNice);
    
#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus

#endif

#endif /* CorePartition_hpp */
