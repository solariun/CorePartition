/*
//  CorePartition.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//
//               GNU GENERAL PUBLIC LICENSE
//                Version 3, 29 June 2007
//
// Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//
// Preamble
//
// The GNU General Public License is a free, copyleft license for
// software and other kinds of works.
//
// The licenses for most software and other practical works are designed
// to take away your freedom to share and change the works.  By contrast,
// the GNU General Public License is intended to guarantee your freedom to
// share and change all versions of a program--to make sure it remains free
// software for all its users.  We, the Free Software Foundation, use the
// GNU General Public License for most of our software; it applies also to
// any other work released this way by its authors.  You can apply it to
// your programs, too.
//
// See LICENSE file for the complete information
*/

#include "CorePartition.h"
#include <stdlib.h>


#define THREADL_ER_STACKOVFLW 1
#define THREAD_FACTOR_MAXBYTES 8

#define THREAD_NAME_MAX 8

typedef struct
{
    size_t nStackMaxSize;
    size_t nStackSize;

    union
    {
        jmp_buf jmpRegisterBuffer;

        struct
        {
            void (*pFunction) (void* pStart);
            void* pValue;
        } func;
    } mem;

    char pszThreadName[THREAD_NAME_MAX + 1];

    void* pLastStack;
    uint32_t nNice;
    uint32_t nLastMomentun;
    uint32_t nLastBackup;
    uint32_t nExecTime;
    uint8_t nIsolation;
    uint8_t nStatus;
    uint8_t stackPage;
} CoreThread;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nRunningThreads = 0;
static volatile size_t nCurrentThread;

CoreThread** pCoreThread = NULL;
static void* pStartStck = NULL;

jmp_buf jmpJoinPointer;


static void (*stackOverflowHandler) (void) = NULL;

bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler) (void))
{
    if (NULL == pStackOverflowHandler || NULL != stackOverflowHandler) return false;

    stackOverflowHandler = pStackOverflowHandler;

    return true;
}


static uint32_t (*getCTime) (void) = NULL;


bool CorePartition_SetCurrentTimeInterface (uint32_t (*pTimeInterface) (void))
{
    if (NULL == pTimeInterface) return false;

    getCTime = (uint32_t (*) (void))pTimeInterface;

    return true;
}


void (*sleepCTime) (const uint32_t nSleepTime) = NULL;
bool CorePartition_SetSleepTimeInterface (void (*pSleepTime) (const uint32_t nSleepTime))
{
    if (NULL == pSleepTime) return false;

    sleepCTime = (void (*) (const uint32_t))pSleepTime;

    return true;
}


static uint32_t nTimeTick = 0;
static uint32_t GetTicks ()
{
    nTimeTick++;

    return nTimeTick;
}


static void SleepTicks (const uint32_t nSleepValue)
{
    uint32_t nSleepTicks = nSleepValue;
    uint32_t nCTime = 1;

    while (nSleepTicks-- && nCTime) nCTime = GetTicks ();
}


bool CorePartition_CreateThread_ (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice, uint8_t nTaskIsolation)
{
    size_t nThread;

    if (pFunction == NULL) return false;

    /* Determine free threads */
    for (nThread = 0; nThread < nMaxThreads; nThread++)
    {
        if (NULL == pCoreThread[nThread]) break;
    }

    /* If it leaves here it means a serious bug */
    if (nThread == nMaxThreads) return false;


    if ((pCoreThread[nThread] = (CoreThread*)malloc ((sizeof (uint8_t) * nStackMaxSize) + sizeof (CoreThread))) == NULL)
    {
        return false;
    }

    CorePartition_SetThreadNameByID (nThread, "thread", 6);

    pCoreThread[nThread]->nStackMaxSize = nStackMaxSize;

    pCoreThread[nThread]->mem.func.pValue = pValue;

    pCoreThread[nThread]->nStatus = THREADL_START;

    pCoreThread[nThread]->nStackSize = 0;

    pCoreThread[nThread]->mem.func.pFunction = pFunction;

    pCoreThread[nThread]->nNice = 1;

    pCoreThread[nThread]->nExecTime = 0;

    pCoreThread[nThread]->nNice = nNice;

    pCoreThread[nThread]->nIsolation = nTaskIsolation;

    pCoreThread[nThread]->nLastBackup = getCTime ();

    pCoreThread[nThread]->nLastMomentun = getCTime ();

    nThreadCount++;

    return true;
}


bool CorePartition_CreateSecureThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 1);
}


bool CorePartition_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 0);
}


static void fastmemcpy (uint8_t* pDestine, const uint8_t* pSource, size_t nSize)
{
    const uint8_t* nTop = (const uint8_t*)pSource + nSize;

    if (0 == pCoreThread[nCurrentThread]->nIsolation)
    {
        memcpy ((void*)pDestine, (const void*)pSource, nSize);
    }
    else
    {
        srand (pCoreThread[nCurrentThread]->nLastBackup);

        for (; pSource <= nTop;)
        {
            *((uint8_t*)pDestine) = *(pSource) ^ ((uint8_t) (rand () % 255));
            pSource++;
            pDestine++;
        }
    }
}


static void BackupStack (void)
{
    fastmemcpy ((uint8_t*)&pCoreThread[nCurrentThread]->stackPage,
                (const uint8_t*)pCoreThread[nCurrentThread]->pLastStack,
                pCoreThread[nCurrentThread]->nStackSize);
}


static void RestoreStack (void)
{
    fastmemcpy ((uint8_t*)pCoreThread[nCurrentThread]->pLastStack,
                (const uint8_t*)&pCoreThread[nCurrentThread]->stackPage,
                pCoreThread[nCurrentThread]->nStackSize);
}

#define __NEXTIME(TH) ((uint32_t) (pCoreThread[TH]->nLastMomentun + pCoreThread[TH]->nNice))
#define __CALC(TH) (uint32_t) (__NEXTIME (TH) - nCurTime)

static size_t Momentum_Scheduler (void)
{
    uint32_t nCurTime;
    uint32_t nMin;
    size_t nCThread;
    size_t nThread;
    size_t nCount;

    nCurTime = getCTime ();
    nCThread = nCurrentThread + 1;
    nThread = nCurrentThread;

    srand (nCurTime);

    nMin = ~((uint32_t)0);
    for (nCount = 0; nCount < nMaxThreads; nCount++, nCThread++)
    {
        nCThread = nCThread >= nMaxThreads ? 0 : nCThread;

        if (pCoreThread[nCThread] == NULL)
        {
            continue;
        }
        else if (0 == pCoreThread[nCThread]->nNice || nCurTime >= __NEXTIME (nCThread) || THREADL_START == pCoreThread[nCThread]->nStatus)
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
    } /* code */

    if (NULL != pCoreThread[nThread])
    {
        sleepCTime ((nMin / 2));
    }

    return nThread;
}


// static size_t Classic_Scheduler (void)
// {
//     static uint32_t nTimeTick = 0;

//     uint32_t nMin;
//     size_t nCThread;
//     size_t nThread;

//     nMin = ~((uint32_t)0);
//     nCThread = nCurrentThread + 1;
//     nThread = nCurrentThread;

//     while (nRunningThreads)
//     {
//         if (nCThread >= nMaxThreads)
//         {
//             nTimeTick++;
//             nCThread = 0;
//         }

//         if (pCoreThread[nCThread] != NULL)
//         {
//             if (pCoreThread[nCThread]->nNice == 0 || (nTimeTick % pCoreThread[nCThread]->nNice) == 0 ||
//                 THREADL_START == pCoreThread[nCThread]->nStatus)
//             {
//                 nThread = nCThread;
//                 nMin = 0;
//                 break;
//             }
//         }

//         nCThread++;
//     }

//     if (NULL != pCoreThread[nThread])
//     {
//         pCoreThread[nThread]->nLastMomentun = getCTime ();
//     }

//     return nThread;
// }


bool CorePartition_Start (size_t nThreadPartitions)
{
    if (NULL != pCoreThread || 0 == nThreadPartitions) return false;

    nMaxThreads = nThreadPartitions;

    if (NULL == (pCoreThread = (CoreThread**)malloc (sizeof (CoreThread**) * nThreadPartitions)))
    {
        return false;
    }

    if (memset ((void*)pCoreThread, 0, sizeof (CoreThread**) * nThreadPartitions) == NULL)
    {
        return false;
    }


    if (false == CorePartition_SetCurrentTimeInterface (GetTicks) || false == CorePartition_SetSleepTimeInterface (SleepTicks))
    {
        return false;
    }

    return true;
}

static void CorePartition_StopThread ()
{
    if (NULL != pCoreThread[nCurrentThread])
    {
        free (pCoreThread[nCurrentThread]);
        pCoreThread[nCurrentThread] = NULL;

        nRunningThreads--;
    }
}


size_t nNextThread = 0;
void CorePartition_Join ()
{
    volatile uint8_t nValue = 0xAA;

    if (0 == nThreadCount || NULL == getCTime || NULL == sleepCTime) return;

    do
    {
        if (NULL != pCoreThread[nCurrentThread])
        {
            pStartStck = (void*)&nValue;

            if (setjmp (jmpJoinPointer) == 0) switch (pCoreThread[nCurrentThread]->nStatus)
                {
                    case THREADL_START:

                        nRunningThreads++;

                        pCoreThread[nCurrentThread]->nStatus = THREADL_RUNNING;

                        pCoreThread[nCurrentThread]->mem.func.pFunction (pCoreThread[nCurrentThread]->mem.func.pValue);

                        nNextThread = Momentum_Scheduler ();

                        CorePartition_StopThread ();

                        /* the use of continue here is
                         * to force memory realignment for the
                         * stack starting, do not replace to Break
                         * can break freeRTOS stack control
                         */

                        continue;

                    case THREADL_RUNNING:
                    case THREADL_SLEEP:

                        longjmp (pCoreThread[nCurrentThread]->mem.jmpRegisterBuffer, 1);

                        break;

                    default:
                        break;
                }
        }

        nCurrentThread = nNextThread;

    } while (pCoreThread[nCurrentThread] != NULL);
}


static void CorePartition_Yield_IntoVoid ()
{
    volatile uint8_t nValue = 0xBB;

    pCoreThread[nCurrentThread]->pLastStack = (void*)&nValue;
    pCoreThread[nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread[nCurrentThread]->pLastStack;

    if (pCoreThread[nCurrentThread]->nStackSize > pCoreThread[nCurrentThread]->nStackMaxSize)
    {
        if (NULL != stackOverflowHandler) stackOverflowHandler ();

        CorePartition_StopThread ();

        longjmp (jmpJoinPointer, 1);
    }

    BackupStack ();

    if (setjmp (pCoreThread[nCurrentThread]->mem.jmpRegisterBuffer) == 0)
    {
        longjmp (jmpJoinPointer, 1);
    }

    /* This exists to re-align after jump alignment optimizations */
    pCoreThread[nCurrentThread]->pLastStack = (void*)&nValue;
    pCoreThread[nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread[nCurrentThread]->pLastStack;

    RestoreStack ();

    pCoreThread[nCurrentThread]->nLastBackup = pCoreThread[nCurrentThread]->nLastMomentun;
}

uint8_t CorePartition_Yield ()
{
    if (nRunningThreads != 0)
    {
        uint32_t nCurTime = 0;

        pCoreThread[nCurrentThread]->nExecTime = getCTime () - pCoreThread[nCurrentThread]->nLastMomentun;

        nNextThread = Momentum_Scheduler ();

        CorePartition_Yield_IntoVoid ();


        nCurTime = getCTime ();

        while (__NEXTIME (nCurrentThread) > nCurTime)
        {
            sleepCTime ((__CALC (nCurrentThread) / 2));
            nCurTime = getCTime ();
        }

        pCoreThread[nCurrentThread]->nLastMomentun = getCTime ();

        return 1;
    }

    return 0;
}

void CorePartition_Sleep (uint32_t nDelayTickTime)
{
    uint32_t nBkpNice = 0;

    if (THREADL_RUNNING != pCoreThread[nCurrentThread]->nStatus) return;

    nBkpNice = CorePartition_GetNiceByID (nCurrentThread);

    pCoreThread[nCurrentThread]->nStatus = THREADL_SLEEP;

    CorePartition_SetNice (nDelayTickTime);

    CorePartition_Yield ();

    pCoreThread[nCurrentThread]->nStatus = THREADL_RUNNING;

    CorePartition_SetNice (nBkpNice);
}


size_t CorePartition_GetID ()
{
    return nCurrentThread;
}


size_t CorePartition_GetStackSizeByID (size_t nID)
{
    if (nID >= nMaxThreads || pCoreThread[nID] == NULL) return 0;

    return pCoreThread[nID]->nStackSize;
}


size_t CorePartition_GetMaxStackSizeByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nStackMaxSize;
}


uint32_t CorePartition_GetNiceByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nNice;
}


uint8_t CorePartition_GetStatusByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nStatus;
}


char CorePartition_IsSecureByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : (pCoreThread[nID]->nIsolation != 0 ? 'S' : 'N');
}


uint32_t CorePartition_GetLastDutyCycleByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nExecTime;
}


uint32_t CorePartition_GetLastMomentumByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nLastMomentun;
}


size_t CorePartition_GetNumberOfActiveThreads (void)
{
    return nRunningThreads;
}


size_t CorePartition_GetMaxNumberOfThreads (void)
{
    return nMaxThreads;
}


size_t CorePartition_GetThreadContextSize (void)
{
    return sizeof (CoreThread);
}


bool CorePartition_IsCoreRunning (void)
{
    return nRunningThreads > 0 ? true : false;
}


void CorePartition_SetNice (uint32_t nNice)
{
    pCoreThread[nCurrentThread]->nNice = nNice == 0 ? 1 : nNice;
}


bool CorePartition_SetThreadNameByID (size_t nID, const char* pszName, uint8_t nNameSize)
{
    if (NULL != pszName && nNameSize > 0)
    {
        uint8_t nCopySize = (nNameSize > THREAD_NAME_MAX ? THREAD_NAME_MAX : nNameSize);

        memcpy (pCoreThread[nID]->pszThreadName, pszName, nCopySize);

        pCoreThread[nID]->pszThreadName[nCopySize] = '\0';

        return true;
    }

    return false;
}


const char* CorePartition_GetThreadNameByID (size_t nID)
{
    return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? "-" : pCoreThread[nID]->pszThreadName;
}
