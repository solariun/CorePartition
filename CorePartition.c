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

#define THREADL_ER_STACKOVFLW 1 /* Stack Overflow */
#define THREAD_FACTOR_MAXBYTES 8

#define THREAD_NAME_MAX 8

typedef struct Subscription Subscription;
struct Subscription
{
    TopicCallback callback;
    void* pContext;
    uint8_t nMaxTopics;
    uint8_t nTopicCount;
    uint32_t nTopicList;
};

typedef struct
{
    union
    {
        jmp_buf jmpRegisterBuffer;

        struct
        {
            void (*pFunction) (void* pStart);
            void* pValue;
        } func;
    } mem;

    Subscription* pSubscriptions;

    void* pLastStack;

    size_t nStackMaxSize;
    size_t nStackSize;
    size_t nPayload;

    uint32_t nNice;
    uint32_t nLastMomentun;
    uint32_t nNotifyUID;
    uint32_t nExecTime;

    char pszThreadName[THREAD_NAME_MAX + 1];
    uint8_t nIsolation;
    uint8_t nStatus;
    uint8_t stackPage;
} CoreThread;

static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nRunningThreads = 0;
static volatile size_t nCurrentThread;

static CoreThread** pCoreThread = NULL;
static void* pStartStck = NULL;

jmp_buf jmpJoinPointer;

static void (*stackOverflowHandler) (void) = NULL;

#define POLY 0x8408
/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

uint16_t CorePartition_CRC16 (const uint8_t* pData, size_t nSize, uint16_t nCRC)
{
    unsigned char nCount;
    unsigned int data;

    nCRC = ~nCRC;

    if (nSize == 0) return (~nCRC);

    do
    {
        for (nCount = 0, data = (unsigned int)0xff & *pData++; nCount < 8; nCount++, data >>= 1)
        {
            if ((nCRC & 0x0001) ^ (data & 0x0001))
                nCRC = (nCRC >> 1) ^ POLY;
            else
                nCRC >>= 1;
        }
    } while (--nSize);

    nCRC = ~nCRC;
    data = nCRC;
    nCRC = (nCRC << 8) | (data >> 8 & 0xff);

    return (nCRC);
}

bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler) (void))
{
    if (pStackOverflowHandler == NULL || stackOverflowHandler != NULL) return false;

    stackOverflowHandler = pStackOverflowHandler;

    return true;
}

bool CorePartition_Start (size_t nThreadPartitions)
{
    if (pCoreThread != NULL || nThreadPartitions == 0) return false;

    nMaxThreads = nThreadPartitions;

    if ((pCoreThread = (CoreThread**)malloc (sizeof (CoreThread**) * nThreadPartitions)) == NULL)
    {
        return false;
    }

    if (memset ((void*)pCoreThread, 0, sizeof (CoreThread**) * nThreadPartitions) == NULL)
    {
        return false;
    }

    return true;
}

bool CorePartition_CreateThread_ (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice, uint8_t nTaskIsolation)
{
    size_t nThread;

    if (pFunction == NULL) return false;

    /* Determine free threads */
    for (nThread = 0; nThread < nMaxThreads; nThread++)
    {
        if (pCoreThread[nThread] == NULL) break;
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

    pCoreThread[nThread]->nLastMomentun = CorePartition_GetCurrentTick ();

    pCoreThread[nThread]->pSubscriptions = NULL;

    nThreadCount++;

    return true;
}

bool CorePartition_EnableBroker (void* pContext, uint8_t nMaxTopics, TopicCallback callback)
{
    if (pCoreThread[nCurrentThread]->pSubscriptions == NULL)
    {
        Subscription* pSub = NULL;
        size_t nMemorySize = sizeof (Subscription) + (sizeof (uint32_t) * ((nMaxTopics <= 1) ? 0 : nMaxTopics - 1));

        if ((pSub = malloc (nMemorySize)) != NULL)
        {
            pSub->nTopicCount = 0;
            pSub->callback = callback;
            pSub->nMaxTopics = nMaxTopics;
            pSub->pContext = pContext;

            pCoreThread[nCurrentThread]->pSubscriptions = pSub;

            return true;
        }
    }

    return false;
}

static int32_t CorePartition_GetTopicID (const char* pszTopic, size_t length)
{
    return ((CorePartition_CRC16 ((const uint8_t*)pszTopic, length, 0) << 16) | CorePartition_CRC16 ((const uint8_t*)pszTopic, length, 0x8408));
}

bool CorePartition_IsSubscribed (const char* pszTopic, size_t length)
{
    if (pCoreThread[nCurrentThread] != NULL && pCoreThread[nCurrentThread]->pSubscriptions != NULL)
    {
        Subscription* pSub = pCoreThread[nCurrentThread]->pSubscriptions;
        int nCount = 0;
        uint32_t nTopicID = CorePartition_GetTopicID (pszTopic, length);

        for (nCount = 0; nCount < pSub->nTopicCount; nCount++)
        {
            if ((&pSub->nTopicList)[nCount] == nTopicID)
            {
                return true;
            }
        }
    }

    return false;
}

bool CorePartition_SubscribeTopic (const char* pszTopic, size_t length)
{
    Subscription* pSub = pCoreThread[nCurrentThread]->pSubscriptions;

    if (CorePartition_IsSubscribed (pszTopic, length) == false)
    {
        if (pSub != NULL && pSub->nTopicCount < pSub->nMaxTopics)
        {
            (&pSub->nTopicList)[pSub->nTopicCount++] = CorePartition_GetTopicID (pszTopic, length);
            return true;
        }
    }

    return false;
}

bool CorePartition_PublishTopic (const char* pszTopic, size_t length, size_t nAttribute, size_t nValue)
{
    uint32_t nTopicID = CorePartition_GetTopicID (pszTopic, length);
    int nThreadID = 0;
    bool bReturn = false;
    /* Go through thread lists */

    for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
    {
        if (pCoreThread[nThreadID] != NULL && pCoreThread[nThreadID]->pSubscriptions != NULL)
        {
            int nSubID = 0;
            for (nSubID = 0; nSubID < pCoreThread[nThreadID]->pSubscriptions->nTopicCount; nSubID++)
            {
                if ((&pCoreThread[nThreadID]->pSubscriptions->nTopicList)[nSubID] == nTopicID)
                {
                    pCoreThread[nThreadID]->pSubscriptions->callback (
                            pCoreThread[nThreadID]->pSubscriptions->pContext, pszTopic, length, nAttribute, nValue);
                    bReturn = true;
                }
            }
        }
    }

    return bReturn;
}

bool CorePartition_CreateSecureThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 1);
}

bool CorePartition_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
{
    return CorePartition_CreateThread_ (pFunction, pValue, nStackMaxSize, nNice, 0);
}

static void StackHandler (uint8_t* pDestine, const uint8_t* pSource, size_t nSize)
{
    const uint8_t* nTop = (const uint8_t*)pSource + nSize;

    if (pCoreThread[nCurrentThread]->nIsolation == 0)
    {
        memcpy ((void*)pDestine, (const void*)pSource, nSize);
    }
    else
    {
        srand (pCoreThread[nCurrentThread]->nLastMomentun);

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
    StackHandler ((uint8_t*)&pCoreThread[nCurrentThread]->stackPage,
                  (const uint8_t*)pCoreThread[nCurrentThread]->pLastStack,
                  pCoreThread[nCurrentThread]->nStackSize);
}

static void RestoreStack (void)
{
    StackHandler ((uint8_t*)pCoreThread[nCurrentThread]->pLastStack,
                  (const uint8_t*)&pCoreThread[nCurrentThread]->stackPage,
                  pCoreThread[nCurrentThread]->nStackSize);
}

static uint32_t CorePartition_GetNextTime (size_t nThreadID)
{
    return (uint32_t) (pCoreThread[nThreadID]->nLastMomentun + pCoreThread[nThreadID]->nNice);
}

#define _CPTHREAD(T) pCoreThread[T]

static size_t Momentum_Scheduler (void)
{
    size_t nThread = nCurrentThread;
    uint32_t nCurTime = CorePartition_GetCurrentTick ();
    uint32_t nNextTime = 0;
    uint32_t nMin = 0xFFFFFFFF;
    size_t nCThread = nCurrentThread + 1;
    static size_t nCount = 0;
    CoreThread* pThread = NULL;

    srand (nCurTime);

    nCount = 0;

    while (nCount < nMaxThreads)
    {
        if (nCThread >= nMaxThreads)
        {
            nCThread = 0;
        }

        if (NULL != (pThread = pCoreThread[nCThread]) && pThread->nStatus != THREADL_WAITTAG)
        {
            nNextTime = pThread->nLastMomentun + pThread->nNice;

            if ((THREADL_START == pThread->nStatus || THREADL_NOW == pThread->nStatus))
            {
                nThread = nCThread;
                break;
            }
            else if (nNextTime >= nCurTime)
            {
                nNextTime = nNextTime - nCurTime;
                if (nNextTime < nMin)
                {
                    nMin = nNextTime;
                    nThread = nCThread;
                }
            }
            else if (nCurTime - nNextTime < nMin)
            {
                nMin = nCurTime - nNextTime;
                nThread = nCThread;
            }
        }

        nCThread++;
        nCount++;
    } /* code */

    return nThread;
}

static void CorePartition_StopThread ()
{
    if (pCoreThread[nCurrentThread] != NULL)
    {
        if (pCoreThread[nCurrentThread]->pSubscriptions != NULL)
        {
            free (pCoreThread[nCurrentThread]->pSubscriptions);
        }

        free (pCoreThread[nCurrentThread]);
        pCoreThread[nCurrentThread] = NULL;

        nRunningThreads--;
    }
}

size_t nNextThread = 0;
void CorePartition_Join ()
{
    if (nThreadCount == 0) return;

    do
    {
        volatile uint8_t nValue = 0xAA;

        if (pCoreThread[nCurrentThread] != NULL)
        {
            pStartStck = (void*)&nValue;

            if (setjmp (jmpJoinPointer) == 0) switch (pCoreThread[nCurrentThread]->nStatus)
                {
                    case THREADL_START:

                        nRunningThreads++;

                        pCoreThread[nCurrentThread]->nStatus = THREADL_RUNNING;

                        pCoreThread[nCurrentThread]->mem.func.pFunction (pCoreThread[nCurrentThread]->mem.func.pValue);

                        CorePartition_StopThread ();

                        continue;

                    case THREADL_RUNNING:
                    case THREADL_SLEEP:
                    case THREADL_NOW:

                        longjmp (pCoreThread[nCurrentThread]->mem.jmpRegisterBuffer, 1);
                        break;

                    default:
                        break;
                }
        }

        nCurrentThread = Momentum_Scheduler ();
        /*(nCurrentThread + 1) >= nMaxThreads ? 0 : (nCurrentThread + 1); */

    } while (nRunningThreads);
}

uint8_t CorePartition_Yield ()
{
    if (nRunningThreads != 0)
    {
        volatile uint8_t nValue = 0xBB;

        pCoreThread[nCurrentThread]->nExecTime = CorePartition_GetCurrentTick () - pCoreThread[nCurrentThread]->nLastMomentun;

        pCoreThread[nCurrentThread]->pLastStack = (void*)&nValue;
        pCoreThread[nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread[nCurrentThread]->pLastStack;

        if (pCoreThread[nCurrentThread]->nStackSize > pCoreThread[nCurrentThread]->nStackMaxSize)
        {
            if (stackOverflowHandler != NULL) stackOverflowHandler ();

            CorePartition_StopThread ();

            longjmp (jmpJoinPointer, 1);
        }

        BackupStack ();

        if (setjmp (pCoreThread[nCurrentThread]->mem.jmpRegisterBuffer) == 0)
        {
            longjmp (jmpJoinPointer, 1);
        }

        pCoreThread[nCurrentThread]->pLastStack = (void*)&nValue;
        pCoreThread[nCurrentThread]->nStackSize = (size_t)pStartStck - (size_t)pCoreThread[nCurrentThread]->pLastStack;

        RestoreStack ();

        if (NULL != pCoreThread[nCurrentThread])
        {
            uint32_t nCurTime = CorePartition_GetCurrentTick ();

            if ((THREADL_RUNNING == pCoreThread[nCurrentThread]->nStatus || THREADL_SLEEP == pCoreThread[nCurrentThread]->nStatus) &&
                CorePartition_GetNextTime (nCurrentThread) > nCurTime)
            {
                CorePartition_SleepTicks (CorePartition_GetNextTime (nCurrentThread) - nCurTime);
            }
        }

        pCoreThread[nCurrentThread]->nLastMomentun = CorePartition_GetCurrentTick ();

        return 1;
    }

    return 0;
}

size_t CorePartition_WaitMessage (const char* pszTag, size_t nTagLength)
{
    if (pszTag == NULL || pCoreThread[nCurrentThread]->nStatus != THREADL_RUNNING) return 0;

    pCoreThread[nCurrentThread]->nStatus = THREADL_WAITTAG;
    pCoreThread[nCurrentThread]->nNotifyUID = CorePartition_GetTopicID (pszTag, nTagLength);

    CorePartition_Yield ();

    pCoreThread[nCurrentThread]->nStatus = THREADL_RUNNING;
    pCoreThread[nCurrentThread]->nNotifyUID = 0;

    return pCoreThread[nCurrentThread]->nPayload;
}

void CorePartition_Wait (const char* pszTag, size_t nTagLength)
{
    (void) CorePartition_WaitMessage (pszTag, nTagLength);
}

static bool CorePartition_Notify (const char* pszTag, size_t nTagLength, size_t nPayload, bool boolOne)
{
    uint32_t nTopicID = CorePartition_GetTopicID (pszTag, nTagLength);
    int nThreadID = 0;
    bool bReturn = false;
    /* Go through thread lists */

    if (pszTag != NULL && pCoreThread[nCurrentThread]->nStatus == THREADL_RUNNING)
    {
        for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
        {
            if (pCoreThread[nThreadID] != NULL && pCoreThread[nThreadID]->nStatus == THREADL_WAITTAG &&
                pCoreThread[nThreadID]->nNotifyUID == nTopicID)
            {
                pCoreThread[nThreadID]->nStatus = THREADL_NOW;
                pCoreThread[nThreadID]->nPayload = nPayload;

                bReturn = true;
                if (boolOne == true) break;
            }
        }
    }

    CorePartition_Yield ();

    return bReturn;
}


bool CorePartition_NotifyOne (const char* pszTag, size_t nTagLength)
{
    return CorePartition_Notify (pszTag, nTagLength, 0, true);
}

bool CorePartition_NotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nPayload)
{
    return CorePartition_Notify (pszTag, nTagLength, nPayload, true);
}

bool CorePartition_NotifyAll (const char* pszTag, size_t nTagLength)
{
    return CorePartition_Notify (pszTag, nTagLength, 0, false);
}

bool CorePartition_NotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nPayload)
{
    return CorePartition_Notify (pszTag, nTagLength, nPayload, false);
}

void CorePartition_Sleep (uint32_t nDelayTickTime)
{
    uint32_t nBkpNice = 0;

    if (pCoreThread[nCurrentThread]->nStatus != THREADL_RUNNING) return;

    nBkpNice = pCoreThread[nCurrentThread]->nNice;

    pCoreThread[nCurrentThread]->nStatus = THREADL_SLEEP;
    pCoreThread[nCurrentThread]->nNice = nDelayTickTime;

    CorePartition_Yield ();

    pCoreThread[nCurrentThread]->nStatus = THREADL_RUNNING;
    pCoreThread[nCurrentThread]->nNice = nBkpNice;
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
