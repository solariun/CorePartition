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

    
    static char CorePartition_version[] = "V2.0";
    
    bool CorePartition_Start (size_t nThreadPartitions);
    
    bool CreatePartition (void(*pFunction)(void), size_t nStackMaxSize);
    
    void join (void);
    
    void yield(void);
    
    size_t getPartitionID(void);
    
    size_t getPartitionStackSize(void);
    
    size_t getPartitionMemorySize(void);
    
    bool isAllCoresStarted(void);
        
    bool getCoreNice(void);
    
    void setCoreNice (uint8_t nNice);
    
#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus

#endif

#endif /* CorePartition_hpp */
