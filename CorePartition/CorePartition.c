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

#define THREADL_NONE        0
#define THREADL_START       1
#define THREADL_RUNNING     2
#define THREADL_SLEEP       3
#define THREADL_STOPPED     4


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
    uint64_t            nLastMomentun;

    void*               pLastStack;
    uint8_t*            pnStackPage;
    
} ThreadLight;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nCurrentThread;

static volatile ThreadLight* pThreadLight = NULL;
static volatile ThreadLight* pCurrentThread = NULL;



static void*  pStartStck = NULL;

jmp_buf jmpJoinPointer;

/*
 static void printStruct ()
 {
 pCurrentThread = &pThreadLight [nCurrentThread];
 
 printf (ThreadLight ID: (%lu) Struct\n, nCurrentThread);
 printf (\t        nStatus:  [%-17u]\n, pCurrentThread->nStatus);
 printf (\t     nErrorType:  [%-17u]\n\n, pCurrentThread->nErrorType);
 printf (\t  nStackMaxSize:  [%-17lu]\n, pCurrentThread->nStackMaxSize);
 printf (\t     nStackSize:  [%-17lu]\n, pCurrentThread->nStackSize);
 printf (\t     pStartStck:  [0x%-16lX]\n, (size_t) pCurrentThread->pStartStck); printf (\t     pLastStack:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pLastStack);
 printf (\t    pnStackPage:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pnStackPage);
 
 printf (\t    strAssign:  [%-16s]\n\n,  pCurrentThread->strAssign);
 }
 */


void (*stackOverflowHandler)(void) = NULL;

bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler)(void))
{
    if (pStackOverflowHandler == NULL || stackOverflowHandler != NULL)
        return false;
    
    stackOverflowHandler = pStackOverflowHandler;
    
    return true;
}


static uint64_t getDefaultCTime()
{
   static uint64_t nCounter=0;

   return (++nCounter);
}

uint64_t (*getCTime)(void) = getDefaultCTime;


bool CorePartition_SetCurrentTimeInterface (uint64_t (*getCurrentTimeInterface)(void))
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

bool CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(uint64_t nSleepTime))
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
    if (nThreadCount >= nMaxThreads || pFunction == NULL) return false;
    
    pThreadLight[nThreadCount].mem.func.pValue = pValue;
    
    pThreadLight[nThreadCount].nStatus = THREADL_START;

    pThreadLight[nThreadCount].nErrorType = THREADL_NONE;

    pThreadLight [nThreadCount].nStackMaxSize = nStackMaxSize;
    
    pThreadLight[nThreadCount].nStackSize = 0;

    pThreadLight[nThreadCount].mem.func.pFunction = pFunction;
    
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
    uint64_t nMin;
    size_t nCThread=0;
    
#define __NEXTIME(TH) (pThreadLight [TH].nLastMomentun +  pThreadLight [TH].nNice)
#define __CALC(TH) (uint64_t) ( __NEXTIME(TH) - nCurTime)
    
    nMin = __CALC(0);
    
    for (nCThread = 0; nCThread < nMaxThreads; nCThread++)
    {
        if (pThreadLight [nCThread].nStatus == THREADL_STOPPED)
            continue;
        else if (__NEXTIME (nCThread) <= nCurTime)
            return 0;
        else if (nMin > __CALC (nCThread))
            nMin = __CALC (nCThread);
    }
        
    return nMin;
}

static size_t Scheduler ()
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
            while (nCounter == getCTime ());
            
            sleepCTime (getSleepTime (getCTime ()));
            
            nCounter = getCTime ();
            
            nCurrentThread = -1;
        }
    }
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
                    
                    pCurrentThread->nStatus = THREADL_STOPPED;

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
            free (pCurrentThread->pnStackPage);
            pCurrentThread->nStatus = THREADL_STOPPED;
            
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

size_t CorePartition_GetNumberOfThreads()
{
    return nMaxThreads;
}

int CorePartition_GetStatusByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pThreadLight [nID].nStatus;
}

uint8_t CorePartition_GetStatus ()
{
    return pCurrentThread == NULL ? 0 : pCurrentThread->nStatus;
}


size_t CorePartition_GetStackSize()
{
    return pCurrentThread == NULL ? 0 : pCurrentThread->nStackSize;
}

size_t CorePartition_GetMaxStackSize()
{
    return pCurrentThread == NULL ? 0 : pCurrentThread->nStackMaxSize;
}

size_t CorePartition_GetThreadContextSize(void)
{
    return sizeof (ThreadLight);
}

bool CorePartition_IsCoreRunning(void)
{
    return pCurrentThread == NULL ? false : pCurrentThread->nStatus == THREADL_RUNNING;
}

uint32_t CorePartition_GetNice()
{
    return pCurrentThread == NULL ? 0 : pCurrentThread->nNice;
}

void CorePartition_SetNice (uint32_t nNice)
{
    if (pCurrentThread == NULL)
        return;

    pCurrentThread->nNice = nNice == 0 ? 1 : nNice;
}
