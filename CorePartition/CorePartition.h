//
//  CorePartition.hpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//
//               GNU GENERAL PUBLIC LICENSE
//                Version 3, 29 June 2007
//
//Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
//Everyone is permitted to copy and distribute verbatim copies
//of this license document, but changing it is not allowed.
//
//Preamble
//
//The GNU General Public License is a free, copyleft license for
//software and other kinds of works.
//
//The licenses for most software and other practical works are designed
//to take away your freedom to share and change the works.  By contrast,
//the GNU General Public License is intended to guarantee your freedom to
//share and change all versions of a program--to make sure it remains free
//software for all its users.  We, the Free Software Foundation, use the
//GNU General Public License for most of our software; it applies also to
//any other work released this way by its authors.  You can apply it to
//your programs, too.
//
// See LICENSE file for the complete information


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

#define THREADL_NONE        0
#define THREADL_START       1
#define THREADL_RUNNING     2
#define THREADL_SLEEP       3
#define THREADL_STOPPED     4

    
    static const char CorePartition_version[] = "V2.4 beta Compiled at " __TIMESTAMP__;
    
    bool CorePartition_Start (size_t nThreadPartitions);
    
    bool CorePartition_CreateThread (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);

    bool CorePartition_CreateSecureThread (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);
    
    bool CorePartition_SetCurrentTimeInterface (uint32_t (*getCurrentTimeInterface)(void));

    bool CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(uint32_t nSleepTime));
    
    bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler)(void));

    void CorePartition_Join (void);
       
    bool CorePartition_Yield(void);
    
    void CorePartition_Sleep (uint32_t nDelayTickTime);

    size_t CorePartition_GetID(void);
    
    size_t CorePartition_GetStackSize(void);
    size_t CorePartition_GetMaxStackSize(void);

    size_t CorePartition_GetThreadContextSize (void);
    
    uint8_t CorePartition_GetStatus (void);

    uint32_t CorePartition_GetNice(void);
    
    void CorePartition_SetNice (uint32_t nNice);

    uint32_t CorePartition_GetLastMomentum (void);
    
    uint32_t CorePartition_GetLastDutyCycle (void);
    
    size_t CorePartition_GetNumberOfThreads(void);

    size_t CorePartition_GetStackSizeByID (size_t nID);
    
    size_t CorePartition_GetMaxStackSizeByID (size_t nID);
    
    uint32_t CorePartition_GetNiceByID (size_t nID);
    
    int CorePartition_GetStatusByID (size_t nID);
    
    uint32_t CorePartition_GetLastMomentumByID (size_t nID);
    
    uint32_t CorePartition_GetLastDutyCycleByID (size_t nID);
    
    /*
    uint32_t CorePartition_getFactorByID (size_t nID);
    uint32_t CorePartition_getFactor (void);
    */

    char CorePartition_IsSecureByID (size_t nID);

#ifdef __cplusplus
} // extern "C"
#endif


#ifdef __cplusplus

#endif

#endif /* CorePartition_hpp */
