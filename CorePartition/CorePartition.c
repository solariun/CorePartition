//
//  CorePartition.cpp
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

#include "CorePartition.h"

#include <stdlib.h>


#define THREADL_ER_STACKOVFLW 1 //Stack Overflow


typedef struct
{
    uint8_t   nStatus;
    
    uint8_t   nErrorType;
    
    size_t    nStackMaxSize;
    size_t    nStackSize;
 
    union
    {
        jmp_buf   jmpRegisterBuffer;
    
        struct
        {
            void(*pFunction)(void* pStart);
            void* pValue;
        } func;
    } mem;
    
    
    uint32_t            nNice;
    uint32_t            nLastMomentun;

    void*               pLastStack;
    uint8_t*            pnStackPage;
    
    uint32_t            nExecTime;
} ThreadLight;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nCurrentThread = 0;

static ThreadLight* pThreadLight = NULL;
static ThreadLight* pCurrentThread = NULL;

static void*  pStartStck = NULL;

jmp_buf jmpJoinPointer;


void (*stackOverflowHandler)(void) = NULL;


bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler)(void))
{
    if (pStackOverflowHandler == NULL || stackOverflowHandler != NULL)
        return false;
    
    stackOverflowHandler = pStackOverflowHandler;
    
    return true;
}


uint32_t getDefaultCTime()
{
   static uint32_t nCounter=0;

   return (++nCounter);
}

uint32_t (*getCTime)(void) = getDefaultCTime;


bool CorePartition_SetCurrentTimeInterface (uint32_t (*getCurrentTimeInterface)(void))
{
    if (getCTime != getDefaultCTime || getCurrentTimeInterface == NULL)
        return false;
    
    getCTime = getCurrentTimeInterface;
    
    return true;
}

void sleepDefaultCTime (uint32_t nSleepTime)
{
    nSleepTime = getCTime() + nSleepTime;
    
    while (getCTime() <= nSleepTime);
}


void (*sleepCTime)(uint32_t nSleepTime) = sleepDefaultCTime;

bool CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(uint32_t nSleepTime))
{
    if (sleepCTime != sleepDefaultCTime || getSleepTimeInterface == NULL)
        return false;
    
    sleepCTime = getSleepTimeInterface;
    
    return true;
}


bool CorePartition_Start (size_t nThreadPartitions)
{
    if (pThreadLight != NULL || nThreadPartitions == 0) return false;
    
    nMaxThreads = nThreadPartitions;
    
    pThreadLight = (ThreadLight*) malloc (sizeof (ThreadLight) * nThreadPartitions);
    
    memset((void*) pThreadLight, 0, sizeof (ThreadLight) * nThreadPartitions);
    
    return true;
}



bool CorePartition_CreateThread (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    size_t nThread;
    
    if (nThreadCount >= nMaxThreads || pFunction == NULL) return false;
 
    //Determine free threads
    
    for (nThread=0; nThread < nMaxThreads; nThread++)
    {
        if (pThreadLight [nThread].nStatus == THREADL_NONE || pThreadLight [nThread].nStatus == THREADL_STOPPED)
            break;
    }
    
    //If it leaves here it means a serious bug
    if (nThread == nMaxThreads) return false;
    
    pThreadLight [nThread].pnStackPage = (uint8_t*) malloc(sizeof (uint8_t) * pThreadLight [nThread].nStackMaxSize);
    
    if (pThreadLight [nThread].pnStackPage == NULL) return false;
 
    pThreadLight[nThread].mem.func.pValue = pValue;
    
    pThreadLight[nThread].nStatus = THREADL_START;

    pThreadLight[nThread].nErrorType = THREADL_NONE;

    pThreadLight [nThread].nStackMaxSize = nStackMaxSize;
    
    pThreadLight[nThread].nStackSize = 0;

    pThreadLight[nThread].mem.func.pFunction = pFunction;
    
    pThreadLight[nThread].nNice = 1;

    pThreadLight [nThread].nLastMomentun = getCTime();

    pThreadLight [nThread].nExecTime = 0;
    
    pThreadLight [nThread].nNice = nNice;
    
    nThreadCount++;
    
    return true;
}

inline static void BackupStack(void)
{
    memcpy(pCurrentThread->pnStackPage, pCurrentThread->pLastStack, pCurrentThread->nStackSize);
}


inline static void RestoreStack(void)
{
    memcpy(pCurrentThread->pLastStack, pCurrentThread->pnStackPage, pCurrentThread->nStackSize);
}

inline static void SleepBeforeTask (uint32_t nCurTime)
{
    uint32_t nMin=0;
    size_t nCThread=0;
    
#define __NEXTIME(TH) (pThreadLight [TH].nLastMomentun +  pThreadLight [TH].nNice - 1)
#define __CALC(TH) (uint32_t) (__NEXTIME(TH) - nCurTime)
    
    nMin = __CALC(0);
    
    for (nCThread = 0; nCThread <= nThreadCount; nCThread++)
    {
        if (pThreadLight [nCThread].nStatus == THREADL_STOPPED)
            continue;
        else if (__NEXTIME (nCThread) <= nCurTime)
            nMin = 0;
        else if (nMin > __CALC (nCThread))
            nMin = __CALC (nCThread) + 1;
    }
        
    if (nMin > 0) sleepCTime (nMin);
}


inline static size_t Scheduler (void)
{
    static uint32_t nCounter = 0;
  
    if (nCounter == 0) nCounter = getCTime ();
    
    pThreadLight [nCurrentThread].nExecTime = (uint32_t) ((uint32_t)(getCTime () - pThreadLight [nCurrentThread].nLastMomentun));
    
    while (1)
    {
        nCounter = getCTime();
        
        if ( ++nCurrentThread <= nThreadCount )
        {
            if (nCounter >= ((uint32_t)(pThreadLight [nCurrentThread].nLastMomentun +  pThreadLight [nCurrentThread].nNice)))
            {
                pThreadLight [nCurrentThread].nLastMomentun = nCounter;
                
                return nCurrentThread;
            }
        }
        else
        {
            nCurrentThread = -1;
            SleepBeforeTask (getCTime ());
        }
    }
    
    return 0;
}


static void CorePartition_StopThread ()
{
    pCurrentThread->nStatus = THREADL_STOPPED;
    pCurrentThread->nStackMaxSize = 0;
    free (pCurrentThread->pnStackPage);
    
    nThreadCount--;
}

void CorePartition_Join ()
{
    volatile uint8_t nValue = 0xAA;
    pStartStck =  (void*) &nValue;
    
    if (nThreadCount == 0) return;
 
    do
    {
        pCurrentThread = &pThreadLight [nCurrentThread];
        
        if (pCurrentThread->nStatus != THREADL_NONE)
        {
            if (setjmp(jmpJoinPointer) == 0) switch (pCurrentThread->nStatus)
            {
                case THREADL_START:

                    pCurrentThread->nStatus = THREADL_RUNNING;
                    
                    pCurrentThread->mem.func.pFunction (pCurrentThread->mem.func.pValue);
                    
                    CorePartition_StopThread ();
                    
                    break;
                    
                case THREADL_RUNNING:
                case THREADL_SLEEP:
                    
                    longjmp(pCurrentThread->mem.jmpRegisterBuffer, 1);
                
                    break;
                
                default:
                    break;
            }
        }
    } while ((nCurrentThread = Scheduler())+1);
}


bool CorePartition_Yield ()
{
    if (pCurrentThread != NULL)
    {
        volatile uint8_t nValue = 0xBB;
        pCurrentThread->pLastStack = (void*) &nValue;

        pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;

        if (pCurrentThread->nStackSize > pCurrentThread->nStackMaxSize)
        {
            
            CorePartition_StopThread ();
            
            if (stackOverflowHandler != NULL) stackOverflowHandler ();
            
            longjmp(jmpJoinPointer, 1);
        }
        
        BackupStack();
                
        if (setjmp(pCurrentThread->mem.jmpRegisterBuffer) == 0)
        {
            longjmp(jmpJoinPointer, 1);
        }
        
        pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;
        
        RestoreStack();
        
        return true;
    }
    
    return false;
}


inline void CorePartition_Sleep (uint32_t nDelayTickTime)
{
    uint32_t nBkpNice = 0;
    
    if (pCurrentThread == NULL) return;
    
    nBkpNice = pCurrentThread->nNice;
    
    pCurrentThread->nStatus = THREADL_SLEEP;
    pCurrentThread->nNice = nDelayTickTime;
    
    CorePartition_Yield();
    
    pCurrentThread->nStatus = THREADL_RUNNING;
    pCurrentThread->nNice = nBkpNice;
}


size_t CorePartition_GetID()
{
    return nCurrentThread;
}

size_t CorePartition_GetStackSizeByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nStackSize;
}

size_t CorePartition_GetMaxStackSizeByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nStackMaxSize;
}

uint32_t CorePartition_GetNiceByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nNice;
}

int CorePartition_GetStatusByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nStatus;
}

uint32_t CorePartition_GetLastDutyCycleByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nExecTime;
}

uint32_t CorePartition_GetLastMomentumByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nLastMomentun;
}

uint32_t CorePartition_GetLastMomentum (void)
{
    return CorePartition_GetLastMomentumByID(nCurrentThread);
}

size_t CorePartition_GetNumberOfThreads(void)
{
    return nMaxThreads;
}

uint32_t CorePartition_GetLastDutyCycle (void)
{
    return CorePartition_GetLastDutyCycleByID(nCurrentThread);
}

uint8_t CorePartition_GetStatus (void)
{
    return CorePartition_GetStatusByID(nCurrentThread);
}

size_t CorePartition_GetStackSize(void)
{
    return CorePartition_GetStackSizeByID(nCurrentThread);
}

size_t CorePartition_GetMaxStackSize(void)
{
    return CorePartition_GetMaxStackSizeByID(nCurrentThread);
}

size_t CorePartition_GetThreadContextSize(void)
{
    return sizeof (ThreadLight);
}

bool CorePartition_IsCoreRunning(void)
{
    return pCurrentThread == NULL ? false : pCurrentThread->nStatus == THREADL_RUNNING;
}

uint32_t CorePartition_GetNice(void)
{
    return CorePartition_GetNiceByID(nCurrentThread);
}

void CorePartition_SetNice (uint32_t nNice)
{
    if (pCurrentThread == NULL)
        return;

    pCurrentThread->nNice = nNice == 0 ? 1 : nNice;
}

