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
#endif
    
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
    
    
    bool CorePartition_Start (size_t nThreadPartitions);
    
    bool CreatePartition (void(*pFunction)(), size_t nStackMaxSize);
    
    void join ();
    
    void yield();
    
    size_t getPartitionID();
    
    
#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus

#endif

#endif /* CorePartition_hpp */
