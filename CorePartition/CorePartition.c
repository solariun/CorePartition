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
#define THREAD_FACTOR_MAXBYTES 8

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
    
    
    uint32_t   nNice;
    uint32_t   nLastMomentun;
    uint32_t   nLastBackup;

    void*      pLastStack;
    uint8_t*   pnStackPage;
    
    uint32_t   nExecTime;
    
    uint8_t             nIsolation;
} CoreThread;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nRunningThread = 0;
static volatile size_t nCurrentThread;

CoreThread* pCoreThread = NULL;
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


volatile uint32_t getDefaultCTime()
{
   static uint32_t nCounter=0;

   return (++nCounter);
}

volatile uint32_t (*getCTime)(void) = (volatile uint32_t (*)(void)) getDefaultCTime;


volatile static uint32_t getTime()
{
    return getCTime ();
}

bool CorePartition_SetCurrentTimeInterface (uint32_t (*getCurrentTimeInterface)(void))
{
    if (getCTime != getDefaultCTime || getCurrentTimeInterface == NULL)
        return false;
    
    getCTime = (volatile uint32_t (*)(void)) getCurrentTimeInterface;
    
    return true;
}

void sleepDefaultCTime (uint32_t nSleepTime)
{
    nSleepTime = getCTime() + nSleepTime;
    
    while (getCTime() <= nSleepTime);
}


volatile void (*sleepCTime)(const uint32_t nSleepTime) = (volatile void (*)(const uint32_t)) sleepDefaultCTime;

bool CorePartition_SetSleepTimeInterface (void (*getSleepTimeInterface)(const uint32_t nSleepTime))
{
    if ((void*) sleepCTime != (void*) sleepDefaultCTime || getSleepTimeInterface == NULL)
        return false;
    
    sleepCTime = (volatile void (*)(const uint32_t)) getSleepTimeInterface;
    
    return true;
}


static void sleepTime(const uint32_t nSleepTime)
{
    sleepCTime (nSleepTime);
}

bool CorePartition_Start (size_t nThreadPartitions)
{
    if (pCoreThread != NULL || nThreadPartitions == 0) return false;
    
    nMaxThreads = nThreadPartitions;
    
    pCoreThread = (CoreThread*) malloc (sizeof (CoreThread) * nThreadPartitions);
    
    memset((void*) pCoreThread, 0, sizeof (CoreThread) * nThreadPartitions);
    
    srand (getCTime ());
    
    return true;
}



bool CorePartition_CreateThread_ (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice, uint8_t nTaskIsolation)
{
    size_t nThread;
    
    if (nThreadCount >= nMaxThreads || pFunction == NULL) return false;
 
    //Determine free threads
    
    for (nThread=0; nThread < nMaxThreads; nThread++)
    {
        if (pCoreThread [nThread].nStatus == THREADL_NONE || pCoreThread [nThread].nStatus == THREADL_STOPPED)
            break;
    }
    
    //If it leaves here it means a serious bug
    if (nThread == nMaxThreads) return false;
    
    
    //adjust the size to be mutiple of the size_t lenght
    pCoreThread [nThread].nStackMaxSize = nStackMaxSize + (nStackMaxSize % sizeof (size_t));
    
    if ((pCoreThread [nThread].pnStackPage = (uint8_t*) malloc(sizeof (uint8_t) * pCoreThread [nThread].nStackMaxSize)) == NULL)
    {
        memset ((void*) &pCoreThread [nThread], 0, sizeof (CoreThread));
        return false;
    }
        
    pCoreThread[nThread].mem.func.pValue = pValue;
    
    pCoreThread[nThread].nStatus = THREADL_START;

    pCoreThread[nThread].nErrorType = THREADL_NONE;
    
    pCoreThread[nThread].nStackSize = 0;

    pCoreThread[nThread].mem.func.pFunction = pFunction;
    
    pCoreThread[nThread].nNice = 1;

    pCoreThread [nThread].nExecTime = 0;
    
    pCoreThread [nThread].nNice = nNice;
    
    pCoreThread [nThread].nIsolation = nTaskIsolation;

    pCoreThread [nThread].nLastBackup = getCTime ();
    
    pCoreThread [nThread].nLastMomentun = getCTime();
    
    nThreadCount++;
    
    return true;
}



bool CorePartition_CreateSecureThread (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 1);
}



bool CorePartition_CreateThread (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 0);
}



static void CryptBlob (uint8_t* pBlob, size_t nSize)
{
    if (pCoreThread [nCurrentThread].nIsolation)
    {
        uint32_t nKey [4];
        
        srand(pCoreThread [nCurrentThread].nLastBackup);
        
        nKey[0] = (uint32_t) rand();
        nKey[1] = (uint32_t) rand();
        nKey[2] = (uint32_t) rand();
        nKey[3] = (uint32_t) rand();

        while (nSize-- > 0)
        {
            *pBlob = *pBlob ^ ((uint8_t*)nKey) [(nSize % sizeof (nKey))];
            pBlob++;
        }
    }
}



static void BackupStack(void)
{
    memcpy (pCoreThread [nCurrentThread].pnStackPage, pCoreThread [nCurrentThread].pLastStack, pCoreThread [nCurrentThread].nStackSize);
    CryptBlob(pCoreThread [nCurrentThread].pnStackPage, pCoreThread [nCurrentThread].nStackSize);
}



static void RestoreStack(void)
{
    memcpy (pCoreThread [nCurrentThread].pLastStack, pCoreThread [nCurrentThread].pnStackPage, pCoreThread [nCurrentThread].nStackSize);
    CryptBlob(pCoreThread [nCurrentThread].pLastStack, pCoreThread [nCurrentThread].nStackSize);
}



//inline static void SleepBeforeTask (uint32_t nCurTime)
//{
//    uint32_t nMin;
//    size_t nCThread=0;
//
//#define __NEXTIME(TH) (pCoreThread [TH].nLastMomentun +  pCoreThread [TH].nNice)
//#define __CALC(TH) (uint32_t) (__NEXTIME(TH) - nCurTime)
//
//    nMin = 0xFFFFFFFF;
//
//    for (nCThread = 0; nCThread < nMaxThreads; nCThread++)
//    {
//        if (pCoreThread [nCThread].nStatus == THREADL_STOPPED)
//            continue;
//        else if (__NEXTIME (nCThread) <= nCurTime)
//        {
//            nMin = 0;
//            //nCurrentThread = nCThread;
//            break;
//        }
//        else if (nMin > __CALC (nCThread))
//        {
//            //nCurrentThread = nCThread;
//            nMin = __CALC (nCThread);
//        }
//    }
//
//
//    sleepTime ((nMin+1));
//}
//

inline static size_t nextThread (uint32_t nCurTime, size_t nCurrentThread)
{
    uint32_t nMin;
    size_t nCThread = nCurrentThread + 1;
    size_t nThread = nCurrentThread;

    if (pCoreThread [nCurrentThread].nStatus == THREADL_START)
    {
        return nCurrentThread;
    }
    
    pCoreThread [nCurrentThread].nExecTime = nCurTime - pCoreThread [nCurrentThread].nLastMomentun;
    
#define __NEXTIME(TH) (pCoreThread [TH].nLastMomentun +  pCoreThread [TH].nNice)
#define __CALC(TH) (uint32_t) (__NEXTIME(TH) - nCurTime)
    
    nMin = __CALC(nThread);
    
    for (size_t nCount=0; nCount < nMaxThreads; nCount++, nCThread++)
    {
        nCThread = nCThread >= nMaxThreads ? 0 : nCThread;

        //printf ("%s :Max:[%zu) [%u] - (%zu)[%u] - Min: %zu \n", __FUNCTION__, nMaxThreads, nCurTime, nCThread, (uint32_t) __NEXTIME(nCThread), nThread);
        
        if (pCoreThread [nCThread].nStatus == THREADL_STOPPED)
            continue;
        else if (__NEXTIME (nCThread) <= nCurTime)
        {
            nThread = nCThread;
            break;
        }
        else if (nMin > __CALC (nCThread))
        {
            nThread = nCThread;
            nMin = __CALC (nCThread);
        }
    }
    
    //printf ("%s :Return - Max:[%zu) [%u] - (%zu)[%u]\n", __FUNCTION__, nMaxThreads, nCurTime, nCThread, (uint32_t) __CALC (nCThread));
    
    srand(nCurTime);
    return nThread;
}



inline static size_t Scheduler (void)
{
    return nextThread(getTime(), nCurrentThread);
}



static void CorePartition_StopThread ()
{
    pCoreThread [nCurrentThread].nStatus = THREADL_STOPPED;
    pCoreThread [nCurrentThread].nStackMaxSize = 0;
    free (pCoreThread [nCurrentThread].pnStackPage);
    
    nRunningThread--;
}



void CorePartition_Join ()
{
    uint8_t nValue = 0xAA;
    pStartStck =  (void*) &nValue;
    
    if (nThreadCount == 0) return;
 
    do
    {
        if (pCoreThread [nCurrentThread].nStatus != THREADL_NONE)
        {
            if (setjmp(jmpJoinPointer) == 0) switch (pCoreThread [nCurrentThread].nStatus)
            {
                case THREADL_START:

                    nRunningThread++;
                    
                    pCoreThread [nCurrentThread].nStatus = THREADL_RUNNING;
                    
                    pCoreThread [nCurrentThread].mem.func.pFunction (pCoreThread [nCurrentThread].mem.func.pValue);
                    
                    CorePartition_StopThread ();
                    
                    break;
                    
                case THREADL_RUNNING:
                case THREADL_SLEEP:
                    
                    longjmp(pCoreThread [nCurrentThread].mem.jmpRegisterBuffer, 1);
                
                    break;
                
                default:
                    break;
            }
        }
    } while ((nCurrentThread = Scheduler())+1);
    
    nCurrentThread;
}


static void performIdle (const size_t nThread)
{
    uint32_t nNow = getTime();
    int32_t nNextEvent = (int32_t) (pCoreThread [nThread].nLastMomentun + pCoreThread [nThread].nNice) - nNow;


    if (nNextEvent > 0)
    {
        //printf ("sleeping for: %d\n\n", nNextEvent);
        sleepCTime(nNextEvent);
    }
    
    pCoreThread [nCurrentThread].nLastMomentun = (uint32_t) nNow;
}


bool CorePartition_Yield ()
{
    if (nRunningThread > 0)
    {
        uint8_t nValue = 0xBB;
        pCoreThread [nCurrentThread].pLastStack = (void*)&nValue;
        //pCoreThread [nCurrentThread].pLastStack = alloca(0);
        
        pCoreThread [nCurrentThread].nStackSize = (size_t)pStartStck - (size_t)pCoreThread [nCurrentThread].pLastStack;

        if (pCoreThread [nCurrentThread].nStackSize > pCoreThread [nCurrentThread].nStackMaxSize)
        {
            
            CorePartition_StopThread ();
            
            if (stackOverflowHandler != NULL) stackOverflowHandler ();
            
            longjmp(jmpJoinPointer, 1);
        }
        
        BackupStack();
                
        if (setjmp(pCoreThread [nCurrentThread].mem.jmpRegisterBuffer) == 0)
        {
            longjmp(jmpJoinPointer, 1);
        }

        //This existis to re-align after jump alignment optmizations
        pCoreThread [nCurrentThread].pLastStack = (void*)&nValue;
        //pCoreThread [nCurrentThread].pLastStack = alloca(0);
        pCoreThread [nCurrentThread].nStackSize = (size_t)pStartStck - (size_t)pCoreThread [nCurrentThread].pLastStack;
        RestoreStack();

        if (pCoreThread [nCurrentThread].nIsolation != 0)
            pCoreThread [nCurrentThread].nLastBackup = rand();
    
        performIdle(nCurrentThread);

        return true;
    }
        
    return false;
}


inline void CorePartition_Sleep (uint32_t nDelayTickTime)
{
    uint32_t nBkpNice = 0;
    
    if (pCoreThread [nCurrentThread].nStatus != THREADL_RUNNING) return;
    
    nBkpNice = pCoreThread [nCurrentThread].nNice;
    
    pCoreThread [nCurrentThread].nStatus = THREADL_SLEEP;
    pCoreThread [nCurrentThread].nNice = nDelayTickTime;
    
    CorePartition_Yield();
    
    pCoreThread [nCurrentThread].nStatus = THREADL_RUNNING;
    pCoreThread [nCurrentThread].nNice = nBkpNice;
}


size_t CorePartition_GetID()
{
    return nCurrentThread;
}

size_t CorePartition_GetStackSizeByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nStackSize;
}

size_t CorePartition_GetMaxStackSizeByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nStackMaxSize;
}

uint32_t CorePartition_GetNiceByID(size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nNice;
}

int CorePartition_GetStatusByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nStatus;
}

/*
uint32_t CorePartition_getFactorByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return  pCoreThread [nID].nProcTime;
}

uint32_t CorePartition_getFactor (void)
{
    return  pCoreThread [nCurrentThread].nProcTime;
}
*/

char CorePartition_IsSecureByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nIsolation != 0 ? 'S' : 'N';
}


uint32_t CorePartition_GetLastDutyCycleByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nExecTime;
}

uint32_t CorePartition_GetLastMomentumByID (size_t nID)
{
    if (nID >= nMaxThreads) return 0;
    
    return pCoreThread [nID].nLastMomentun;
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
    return sizeof (CoreThread);
}

bool CorePartition_IsCoreRunning(void)
{
    return pCoreThread [nCurrentThread].nStatus == THREADL_RUNNING;
}

uint32_t CorePartition_GetNice(void)
{
    return CorePartition_GetNiceByID(nCurrentThread);
}

void CorePartition_SetNice (uint32_t nNice)
{
    pCoreThread [nCurrentThread].nNice = nNice == 0 ? 1 : nNice;
}



