//
//  CorePartition.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//

#include "CorePartition.h"

#include <stdlib.h>

#define THREADL_NONE        0
#define THREADL_START       1
#define THREADL_RUNNING     2
#define THREADL_IDLE        3
#define THREADL_STOPPED     4
#define THREADL_SWITCHING   5

#define THREADL_ER_STACKOVFLW 1 //Stack Overflow



typedef struct
{
    uint8_t   nStatus;
    
    uint8_t   nErrorType;
    
    size_t    nStackMaxSize;
    size_t    nStackSize;
    
    jmp_buf   jmpJoinPointer;
    jmp_buf   jmpYieldPointer;
    
    void(*pFunction)(void);
    
    uint32_t            nNice;
    uint64_t            nLastMomentun;
    
    void*               pLastStack;
    
    uint8_t*            pnStackPage;
    
} ThreadLight;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nCurrentThread;
static volatile size_t nStartedCores=0;


static ThreadLight* pThreadLight = NULL;
static ThreadLight* pCurrentThread = NULL;


void*               pStartStck = NULL;


/*
 static void printStruct ()
 {
 pCurrentThread = &pThreadLight [nCurrentThread];
 
 printf (ThreadLight ID: (%lu) Struct\n, nCurrentThread);
 printf (\t        nStatus:  [%-17u]\n, pCurrentThread->nStatus);
 printf (\t     nErrorType:  [%-17u]\n\n, pCurrentThread->nErrorType);
 printf (\t  nStackMaxSize:  [%-17lu]\n, pCurrentThread->nStackMaxSize);
 printf (\t     nStackSize:  [%-17lu]\n, pCurrentThread->nStackSize);
 printf (\t     pStartStck:  [0x%-16lX]\n, (size_t) pCurrentThread->pStartStck);
 printf (\t     pLastStack:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pLastStack);
 printf (\t    pnStackPage:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pnStackPage);
 
 printf (\t    strAssign:  [%-16s]\n\n,  pCurrentThread->strAssign);
 }
 */


static uint64_t getDefaultCTime()
{
   static uint64_t nCounter=0;

   return (++nCounter);
}

uint64_t (*getCTime)(void) = getDefaultCTime;


uint8_t CorePartition_SetCurrentTimeInterface (uint64_t (*getCurrentTimeInterface)(void))
{
    if (getCTime != getDefaultCTime || getCurrentTimeInterface == NULL)
        return false;
    
    getCTime = getCurrentTimeInterface;
    
    return true;
}

static void sleepDefaultCTime (uint64_t nSleepTime)
{
    nSleepTime = getCTime() + nSleepTime;
    
    while (getCTime() <= nSleepTime);
}


void (*sleepCTime)(uint64_t nSleepTime) = sleepDefaultCTime;

uint8_t CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(uint64_t nSleepTime))
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
    
    return true;
}



bool CreatePartition (void(*pFunction)(void), size_t nStackMaxSize, uint32_t nNice)
{
    if (nThreadCount >= nMaxThreads || pFunction == NULL) return false;
    
    pThreadLight[nThreadCount].nStatus = THREADL_START;

    pThreadLight[nThreadCount].nErrorType = THREADL_NONE;

    pThreadLight [nThreadCount].nStackMaxSize = nStackMaxSize;
    
    pThreadLight[nThreadCount].nStackSize = 0;

    pThreadLight[nThreadCount].pFunction = pFunction;
    
    pThreadLight[nThreadCount].nNice = 1;

    pThreadLight [nThreadCount].pnStackPage = (uint8_t*) malloc(sizeof (uint8_t) * pThreadLight [nThreadCount].nStackMaxSize);
    
    pThreadLight [nThreadCount].nLastMomentun = getCTime();

    pThreadLight [nThreadCount].nNice = nNice;
    
    nThreadCount++;
    
    return true;
}


static inline void BackupStack()
{
    memcpy(pCurrentThread->pnStackPage, pCurrentThread->pLastStack, pCurrentThread->nStackSize);
}


static inline void RestoreStack()
{
    memcpy(pCurrentThread->pLastStack, pCurrentThread->pnStackPage, pCurrentThread->nStackSize);
}



static inline uint64_t getSleepTime (uint64_t nCurTime)
{
    uint64_t nMin = 1 << sizeof (nMin);
    size_t nCThread=0;

    
#define __CALC(TH) (uint64_t) (pThreadLight [TH].nLastMomentun +  pThreadLight [TH].nNice - nCurTime)
    
    for (nCThread = 0; nCThread < nMaxThreads; nCThread++)
       if (nMin > __CALC (nCThread)) nMin =  __CALC (nCThread);

    return (uint64_t) (nMin > nCurTime ? 0 : nMin);
}

static inline size_t Scheduler ()
{
    static uint64_t nCounter = 0;
    
    if (nCounter == 0) nCounter = getCTime ();
       
    while (1)
    {
        if (++nCurrentThread <= nMaxThreads)
        {
            if (nCounter > (pThreadLight [nCurrentThread].nLastMomentun +  pThreadLight [nCurrentThread].nNice))
            {
                pThreadLight [nCurrentThread].nLastMomentun = nCounter;
                return nCurrentThread;
            }
        }
        else
        {
            nCounter = getCTime ();
            sleepCTime (getSleepTime (nCounter));
            
            nCurrentThread = -1;
        }
    }
}



void join ()
{
    if (nThreadCount == 0) return;


    
    volatile uint8_t nValue = 0xAA;
    pStartStck =  (void*) &nValue;
    
    do
    {
        pCurrentThread = &pThreadLight [nCurrentThread];
        
        //printStruct();
        
        if (pCurrentThread->nStatus != THREADL_NONE)
        {
            switch (pCurrentThread->nStatus)
            {
                case THREADL_START:
                    
                    if (setjmp(pCurrentThread->jmpJoinPointer) == 0)
                    {
                        pCurrentThread->nStatus = THREADL_RUNNING;
                        
                        nStartedCores++;
                        
                        pCurrentThread->pFunction ();
                        
                        
                        pCurrentThread->nStatus = THREADL_STOPPED;
                    }
                    break;
                    
                case THREADL_RUNNING:
                    
                    if (setjmp(pCurrentThread->jmpJoinPointer) == 0)
                    {
                        longjmp(pCurrentThread->jmpYieldPointer, 1);
                    }
                    
                    break;
            }
        }
    } while ((nCurrentThread = Scheduler())+1);
    //} while ((nCurrentThread = (nCurrentThread + 1 >= nMaxThreads) ? 0 : nCurrentThread+1)+1);
}



//void yield() __attribute__ ((noinline));

void yield()
{
    
    if (nThreadCount == 0) return;
    
    volatile uint8_t nValue = 0xBB;
    pCurrentThread->pLastStack = (void*) &nValue;
    
    pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;
    
    if (pCurrentThread->nStackSize > pCurrentThread->nStackMaxSize)
    {
        free (pCurrentThread->pnStackPage);
        pCurrentThread->nStatus = THREADL_STOPPED;
        
        longjmp(pCurrentThread->jmpJoinPointer, 1);
    }
    
    BackupStack();
    if (setjmp(pCurrentThread->jmpYieldPointer) == 0)
    {
        longjmp(pCurrentThread->jmpJoinPointer, 1);
    }
    
    pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;
    
    RestoreStack();
}


size_t CorePartition_GetPartitionID()
{
    return nCurrentThread;
}



size_t CorePartition_GetPartitionStackSize()
{
    return pCurrentThread->nStackSize;
}

size_t CorePartition_GetPartitionMaxStackSize()
{
    return pCurrentThread->nStackMaxSize;
}


size_t CorePartition_GetPartitionAllocatedMemorySize(void)
{
    return CorePartition_GetThreadStructSize () + CorePartition_GetPartitionMaxStackSize ();
}

size_t CorePartition_GetPartitionUsedMemorySize(void)
{
    return CorePartition_GetThreadStructSize () + CorePartition_GetPartitionStackSize ();
}


size_t CorePartition_GetThreadStructSize(void)
{
    return sizeof (ThreadLight);
}


bool CorePartition_IsAllCoresStarted(void)
{
    return nStartedCores == nMaxThreads;
}


bool CorePartition_IsCoreRunning(void)
{
    return pCurrentThread->nStatus == THREADL_RUNNING;
}


uint32_t CorePartition_GetCoreNice()
{
    return pCurrentThread->nNice;
}


void CorePartition_SetCoreNice (uint32_t nNice)
{
    pCurrentThread->nNice = nNice == 0 ? 1 : nNice;
}
