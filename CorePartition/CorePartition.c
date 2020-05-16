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

#define THREAD_NAME_MAX   10

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
    
    char       pszThreadName [THREAD_NAME_MAX + 1];
    
    uint32_t   nNice;
    uint32_t   nLastMomentun;
    uint32_t   nLastBackup;

    void*      pLastStack;
    
    uint32_t   nExecTime;
    
    uint8_t    nIsolation;

    uint8_t    stackPage;

} CoreThread;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nRunningThreads = 0;
static volatile size_t nCurrentThread;

CoreThread** pCoreThread = NULL;
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


uint32_t (*getCTime)(void) = (uint32_t (*)(void)) getDefaultCTime;

static uint32_t getTime()
{
    return getCTime ();
}


bool CorePartition_SetCurrentTimeInterface (uint32_t (*pTimeInterface)(void))
{
    if (getCTime != getDefaultCTime || pTimeInterface == NULL)
        return false;
    
    getCTime = (uint32_t (*)(void)) pTimeInterface;
    
    return true;
}


void sleepDefaultCTime (uint32_t nSleepTime)
{
    nSleepTime = getCTime() + nSleepTime;
    
    while (getCTime() <= nSleepTime);
}


void (*sleepCTime)(const uint32_t nSleepTime) = ( void (*)(const uint32_t)) sleepDefaultCTime;

bool CorePartition_SetSleepTimeInterface (void (*pSleepTime)(const uint32_t nSleepTime))
{
    if ((void*) sleepCTime != (void*) sleepDefaultCTime || pSleepTime == NULL)
        return false;
    
    sleepCTime = (void (*)(const uint32_t)) pSleepTime;
    
    return true;
}


bool CorePartition_Start (size_t nThreadPartitions)
{
    if (pCoreThread != NULL || nThreadPartitions == 0) return false;
    
    nMaxThreads = nThreadPartitions;
    
    if ((pCoreThread = (CoreThread**) malloc (sizeof (CoreThread**) * nThreadPartitions)) == NULL)
    {
        return false;
    }
    
    if (memset((void*) pCoreThread, 0, sizeof (CoreThread**) * nThreadPartitions) == NULL)
    {
        return false;
    }
    
    srand (getCTime ());
    
    return true;
}


bool CorePartition_CreateThread_ (void(*pFunction)(void*), void* pValue, size_t nStackMaxSize, uint32_t nNice, uint8_t nTaskIsolation)
{
    size_t nThread;
    
    if (pFunction == NULL) return false;
 
    //Determine free threads
    
    for (nThread=0; nThread < nMaxThreads; nThread++)
    {
        if (pCoreThread [nThread] == NULL)
            break;
    }
    
    //If it leaves here it means a serious bug
    if (nThread == nMaxThreads) return false;
    
        
    
    if ((pCoreThread [nThread] = (CoreThread*) malloc((sizeof (uint8_t) * nStackMaxSize) + sizeof (CoreThread))) == NULL)
    {
        return false;
    }

    CorePartition_SetThreadName (nThread, "thread", 6);

    //adjust the size to be mutiple of the size_t length
    pCoreThread [nThread]->nStackMaxSize = nStackMaxSize + (nStackMaxSize % sizeof (size_t));
 
    pCoreThread[nThread]->mem.func.pValue = pValue;
    
    pCoreThread[nThread]->nStatus = THREADL_START;

    pCoreThread[nThread]->nErrorType = THREADL_NONE;
    
    pCoreThread[nThread]->nStackSize = 0;

    pCoreThread[nThread]->mem.func.pFunction = pFunction;
    
    pCoreThread[nThread]->nNice = 1;

    pCoreThread [nThread]->nExecTime = 0;
    
    pCoreThread [nThread]->nNice = nNice;
    
    pCoreThread [nThread]->nIsolation = nTaskIsolation;

    pCoreThread [nThread]->nLastBackup = getCTime ();
    
    pCoreThread [nThread]->nLastMomentun = getCTime();
    
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


inline static void fastmemcpy (uint8_t* pDestine, const uint8_t* pSource, size_t nSize)
{
    const uint8_t* nTop = (const uint8_t*) pSource + nSize;

    if (pCoreThread [nCurrentThread]->nIsolation != 0)
    {
        srand(pCoreThread [nCurrentThread]->nLastBackup);
        
        for (;pSource <= nTop;) 
        {
            *((uint8_t*) pDestine) = *(pSource) ^ ((uint8_t) (rand() % 255) );
            pSource++; 
            pDestine++;
        }
    }
    else
        memcpy((void*)pDestine, (const void*) pSource, nSize);
}


inline static void BackupStack(void)
{
    fastmemcpy ((uint8_t*) &pCoreThread [nCurrentThread]->stackPage, (const uint8_t*) pCoreThread [nCurrentThread]->pLastStack, pCoreThread [nCurrentThread]->nStackSize);
    //pCoreThread [nCurrentThread]->nProcTime = getCTime () - pCoreThread [nCurrentThread]->nLastBackup;
}


inline static void RestoreStack(void)
{
    fastmemcpy ((uint8_t*) pCoreThread [nCurrentThread]->pLastStack, (const uint8_t*) &pCoreThread [nCurrentThread]->stackPage, pCoreThread [nCurrentThread]->nStackSize);
}


static inline size_t Scheduler (void)
{
    uint32_t nCurTime;
    uint32_t nMin;
    size_t nCThread;
    size_t nThread;
    size_t nCount;

    nCurTime = getTime();
    nCThread = nCurrentThread+1;
    nThread = nCurrentThread;
    
    srand (nCurTime);
    
#define __NEXTIME(TH) (pCoreThread [TH]->nLastMomentun +  pCoreThread [TH]->nNice)
#define __CALC(TH) (uint32_t) (__NEXTIME(TH) - nCurTime)
    
    nMin =  ~((uint32_t)0);
    for (nCount=0; nCount < nMaxThreads; nCount++, nCThread++)
    {
        nCThread = nCThread >= nMaxThreads ? 0 : nCThread;
                
        //printf ("%zu . nMin:\n", nCThread, nMin);

        if (pCoreThread [nCThread] == NULL)
        {
            continue;
        }
        else if (__NEXTIME (nCThread) <= nCurTime || pCoreThread [nCThread]->nStatus == THREADL_START)
        {
            nThread = nCThread;
            nMin = 0;
            break;
        }
        else if (nMin > __CALC (nCThread))
        {
            nThread = nCThread;
            nMin = __CALC (nCThread);
        }
    }        /* code */
    
    if (pCoreThread [nThread] != NULL)
    {
        sleepCTime (nMin + 1);
        pCoreThread [nThread]->nLastMomentun = nCurTime;
    }

    return nThread;
}


static void CorePartition_StopThread ()
{
    if (pCoreThread [nCurrentThread] != NULL)
    {
        free (pCoreThread [nCurrentThread]);
        pCoreThread [nCurrentThread] = NULL;
    }
    
    nRunningThreads--;
}


void CorePartition_Join ()
{    
    if (nThreadCount == 0) return;
 
    do
    {
        if (pCoreThread [nCurrentThread] != NULL)
        {
             volatile uint8_t nValue = 0xAA;
             pStartStck =  (void*) &nValue;

            if (setjmp(jmpJoinPointer) == 0) switch (pCoreThread [nCurrentThread]->nStatus)
            {
                case THREADL_START:
                    
                    nRunningThreads++;
                    
                    pCoreThread [nCurrentThread]->nStatus = THREADL_RUNNING;
                    
                    pCoreThread [nCurrentThread]->mem.func.pFunction (pCoreThread [nCurrentThread]->mem.func.pValue);
                    
                    CorePartition_StopThread ();
                    
                    break;
                    
                case THREADL_RUNNING:
                case THREADL_SLEEP:
                    
                    longjmp(pCoreThread [nCurrentThread]->mem.jmpRegisterBuffer, 1);
                
                    break;
                
                default:
                    break;
            }
        }
        
        nCurrentThread = Scheduler (); //(nCurrentThread + 1) >= nMaxThreads ? 0 : (nCurrentThread + 1);
        
    } while (true);
}


void CorePartition_Yield ()
{
    if (nRunningThreads != 0)
    {
        volatile uint8_t nValue = 0xBB; 

        pCoreThread [nCurrentThread]->nExecTime = getCTime() - pCoreThread [nCurrentThread]->nLastMomentun;
    
        pCoreThread [nCurrentThread]->pLastStack = (void*)&nValue;
        //pCoreThread [nCurrentThread]->pLastStack = alloca(0);
        
        pCoreThread [nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread [nCurrentThread]->pLastStack;

        if (pCoreThread [nCurrentThread]->nStackSize > pCoreThread [nCurrentThread]->nStackMaxSize)
        {
            CorePartition_StopThread ();
            
            if (stackOverflowHandler != NULL) stackOverflowHandler ();
            
            longjmp(jmpJoinPointer, 1);
        }
        
        BackupStack();
                
        if (setjmp(pCoreThread [nCurrentThread]->mem.jmpRegisterBuffer) == 0)
        {
            longjmp(jmpJoinPointer, 1);
        }
        
        //This existis to re-align after jump alignment optmizations
        pCoreThread [nCurrentThread]->pLastStack = (void*)&nValue;
        //pCoreThread [nCurrentThread]->pLastStack = alloca(0);
        pCoreThread [nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread [nCurrentThread]->pLastStack;

        RestoreStack();        

        pCoreThread [nCurrentThread]->nLastBackup = pCoreThread [nCurrentThread]->nLastMomentun;
    }

}


void CorePartition_Sleep (uint32_t nDelayTickTime)
{
    uint32_t nBkpNice = 0;
    
    if (pCoreThread [nCurrentThread]->nStatus != THREADL_RUNNING) return;
    
    nBkpNice = pCoreThread [nCurrentThread]->nNice;
    
    pCoreThread [nCurrentThread]->nStatus = THREADL_SLEEP;
    pCoreThread [nCurrentThread]->nNice = nDelayTickTime;
    
    CorePartition_Yield();
    
    pCoreThread [nCurrentThread]->nStatus = THREADL_RUNNING;
    pCoreThread [nCurrentThread]->nNice = nBkpNice;
}


size_t CorePartition_GetID()
{
    return nCurrentThread;
}


size_t CorePartition_GetStackSizeByID(size_t nID)
{
    if (nID >= nMaxThreads || pCoreThread [nID] == NULL) return 0;
    
    return pCoreThread [nID]->nStackSize;
}


size_t CorePartition_GetMaxStackSizeByID(size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? 0 : pCoreThread [nID]->nStackMaxSize;
}


uint32_t CorePartition_GetNiceByID(size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? 0 : pCoreThread [nID]->nNice;
}


uint8_t CorePartition_GetStatusByID (size_t nID)
{
    return (nID >= nMaxThreads ||  NULL == pCoreThread [nID] ) ? 0 : pCoreThread [nID]->nStatus;
}


char CorePartition_IsSecureByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? 0 : (pCoreThread [nID]->nIsolation != 0 ? 'S' : 'N');
}


uint32_t CorePartition_GetLastDutyCycleByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? 0 : pCoreThread [nID]->nExecTime;
}


uint32_t CorePartition_GetLastMomentumByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? 0 : pCoreThread [nID]->nLastMomentun;
}


size_t CorePartition_GetNumberOfActiveThreads(void)
{
    return nRunningThreads;
}


size_t CorePartition_GetMaxNumberOfThreads(void)
{
    return nMaxThreads;
}


size_t CorePartition_GetThreadContextSize(void)
{
    return sizeof (CoreThread);
}


bool CorePartition_IsCoreRunning(void)
{
    return nRunningThreads > 0 ? true : false;
}


void CorePartition_SetNice (uint32_t nNice)
{
    pCoreThread [nCurrentThread]->nNice = nNice == 0 ? 1 : nNice;
}


bool CorePartition_SetThreadName (size_t nID, const char* pszName, uint8_t nNameSize)
{
    if (NULL != pszName && nNameSize > 0)
    {
        uint8_t nCopySize = (nNameSize > THREAD_NAME_MAX ? THREAD_NAME_MAX : nNameSize);

        memcpy (pCoreThread [nID]->pszThreadName, pszName, nCopySize);

        pCoreThread [nID]->pszThreadName [nCopySize] = '\0';

        return true;
    }
    
    return false;
}


const char* CorePartition_GetThreadName (size_t nID)
{
     return (nID >= nMaxThreads || NULL == pCoreThread [nID]) ? "-" : pCoreThread [nID]->pszThreadName;
}


