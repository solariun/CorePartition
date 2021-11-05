/*
    CoreParitionOS Initiative

    MIT License

    Copyright (c) 2021 Gustavo Campos

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "CorePartition.h"

#include <assert.h>
#include <stdlib.h>

#define NOTRACE 1 ? (void)0 : (void)printf

#ifdef __DEBUG__

#define VERIFY(term, ret) \
    if (!(term))          \
    {                     \
        return ret;       \
    }
#define YYTRACE printf
#define TRACE YYTRACE

#else

#define VERIFY(term, ret) \
    if (!(term))          \
    {                     \
        return ret;       \
    }
#define YYTRACE printf
#define TRACE NOTRACE

#endif

#define Cpx_SetBit(var, bitValue) (var |= bitBValue)
#define Cpx_UnsetBit(var, bitValue) (var &= (~bitValue))
#define Cpx_ToggleBit(var, bitValue) (var ^= bitValue)

#define THREADL_ER_STACKOVFLW 1 /* Stack Overflow */
#define THREAD_FACTOR_MAXBYTES 8

#define THREAD_NAME_MAX 8

    enum __cp_lock_type
    {
        CP_LOCK_NONE = 0,
        CP_LOCK_SIMPLE,
        CP_LOCK_SHARED
    };

    enum __cpx_thread_controller_type
    {
        CPX_CTRL_TYPE_STATIC = 1,
        CPX_CTRL_BROKER_STATIC = 2
    };

    /* Max thread allowed  */
    static volatile size_t nMaxThreads = 0;

    /* Threads instiated */
    static volatile size_t nThreadCount = 0;

    /* Threads in running state */
    static volatile size_t nRunningThreads = 0;

    /* ID for the current thread */
    static volatile size_t nCurrentThread = 0;

    /* Thread execution pointer list */
    CpxThread** pCpxThread = NULL;

    /* The current thread context pointer */
    CpxThread* pCurrentThread = NULL;

    /* The stack start point */
    void* pStartStck = NULL;

    /* The kernal jump back point */
    jmp_buf jmpJoinPointer;

#define Cpx_SetState(nNewState) pCurrentThread->nStatus = nNewState
#define Cpx_NowYield()              \
    {                               \
        Cpx_SetState (THREADL_NOW); \
        Cpx_Yield ();               \
    }

    /*
     * WEAK FUNCTION TO BE OVERLOADED BY APPLICATIONS
     */
#pragma weak Cpx_GetCurrentTick
    uint32_t Cpx_GetCurrentTick (void)
    {
        static uint32_t nSleepTime;
        return (uint32_t)nSleepTime++;
    }

#pragma weak Cpx_SleepTicks
    void Cpx_SleepTicks (uint32_t nSleepTime)
    {
        while (nSleepTime)
        {
            nSleepTime--;
            Cpx_GetCurrentTick ();
        }
    }

#pragma weak Cpx_StackOverflowHandler
    void Cpx_StackOverflowHandler (void)
    {
        (void)0;
    }

    /*
     * --------------------------------------------------
     * String functions not capable of overloading
     * --------------------------------------------------
     */

    bool Cpx_IsCoreRunning (void)
    {
        VERIFY (pCpxThread != NULL && pCurrentThread != NULL, false);

        return true;
    }

    void Cpx_InternalSetNice (uint32_t nNice)
    {
        pCurrentThread->nNice = nNice;
    }

    void Cpx_InternalSetStatus (uint8_t nStatus)
    {
        pCurrentThread->nStatus = nStatus;
    }

    /*
     * --------------------------------------------------
     * KERNEL FUNCTIONS ---------------------------------
     * --------------------------------------------------
     */

#define POLY 0x8408
    /*
    //                                      16   12   5
    // this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
    // This works out to be 0x1021, but the way the algorithm works
    // lets us use 0x8408 (the reverse of the bit pattern).  The high
    // bit is always assumed to be set, thus we only use 16 bits to
    // represent the 17 bit value.
    */

    uint16_t Cpx_CRC16 (const uint8_t* pData, size_t nSize, uint16_t nCRC)
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

    static bool Cpx_CommonStart (size_t nThreadPartitions, CpxThread** ppStaticCpxThread)
    {
        VERIFY (pCpxThread == NULL, false);

        nMaxThreads = nThreadPartitions;

        if (ppStaticCpxThread == NULL)
        {
#ifndef _CPX_NO_HEAP_
            if ((pCpxThread = (CpxThread**)malloc (sizeof (CpxThread**) * nThreadPartitions)) == NULL)
            {
                return false;
            }
#else
        return false;
#endif
        }
        else
        {
            pCpxThread = ppStaticCpxThread;
        }

        /* Initiating all reserved threads */
        {
            size_t nCount = 0;

            for (nCount=0; nCount < nMaxThreads; nCount++)
            {
                pCpxThread [nCount] = NULL;
            }
        }

        return true;
    }

    bool Cpx_StaticStart (CpxThread** ppStaticThread, size_t nStaticThreadSize)
    {
        VERIFY (nStaticThreadSize > sizeof (CpxThread**), false);

        return Cpx_CommonStart (nStaticThreadSize / sizeof (CpxThread**), ppStaticThread);
    }

    bool Cpx_Start (size_t nThreadPartitions)
    {
        return Cpx_CommonStart (nThreadPartitions, NULL);
    }

    static bool Cpx_CreateThreadInit (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice, CpxThread* pStaticContext)
    {
        size_t nThread;

        VERIFY (pFunction != NULL, false);

        /* Determine free threads */
        for (nThread = 0; nThread < nMaxThreads; nThread++)
        {
            if (pCpxThread[nThread] == NULL) break;
        }

        /* determine if no free threads here*/
        if (nThread == nMaxThreads) return false;

        /* Starts to assign the appropriate thread context and set type */
        if (pStaticContext == NULL)
        {
#ifndef _CPX_NO_HEAP_
            /* HEAP initializations */
            if ((pCpxThread[nThread] = (CpxThread*)malloc ((sizeof (uint8_t) * nStackMaxSize) + sizeof (CpxThread))) == NULL)
            {
                return false;
            }

            pCpxThread[nThread]->nThreadController = 0;
#else
        return false;
#endif
        }
        else
        {
            /* Static initializations */

            pCpxThread[nThread] = (CpxThread*)pStaticContext;

            pCpxThread[nThread]->nThreadController = CPX_CTRL_TYPE_STATIC;
        }

        /* Initializations */
        pCpxThread[nThread]->nStackMaxSize = nStackMaxSize;

        pCpxThread[nThread]->mem.func.pValue = pValue;

        pCpxThread[nThread]->nStatus = THREADL_START;

        pCpxThread[nThread]->nStackSize = 0;

        pCpxThread[nThread]->mem.func.pFunction = pFunction;

        pCpxThread[nThread]->nNice = 1;

        pCpxThread[nThread]->nExecTime = 0;

        pCpxThread[nThread]->nNice = nNice;

        pCpxThread[nThread]->nLastMomentun = Cpx_GetCurrentTick ();

        pCpxThread[nThread]->pSubscriptions = NULL;

        pCpxThread[nThread]->nNotifyUID = 0;

        /* Union - nSleepTime and pnVariableLockID will be impressed */
        pCpxThread[nThread]->control.payload = (CpxMsgPayload){0, 0, 0};

        nThreadCount++;
        nRunningThreads++;

        return true;
    }

    bool Cpx_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
    {
        return Cpx_CreateThreadInit (pFunction, pValue, nStackMaxSize, nNice, NULL);
    }

    bool Cpx_CreateStaticThread (void (*pFunction) (void*), void* pValue, CpxStaticThread* pStaticThread, size_t nStaticThreadSize, uint32_t nNice)
    {
        VERIFY (nStaticThreadSize > sizeof (CpxThread), false);

        return Cpx_CreateThreadInit (pFunction, pValue, Cpx_GetStaticContextSize (nStaticThreadSize), nNice, (CpxThread*)pStaticThread);
    }

    static void Cpx_BackupStack (void)
    {
        memcpy ((uint8_t*)&pCurrentThread->stackPage, (const uint8_t*)pCurrentThread->pLastStack, pCurrentThread->nStackSize);
    }

    static void Cpx_RestoreStack (void)
    {
        memcpy ((uint8_t*)pCurrentThread->pLastStack, (const uint8_t*)&pCurrentThread->stackPage, pCurrentThread->nStackSize);
    }

    static uint32_t Cpx_GetNextTime (size_t nThreadID)
    {
        return (uint32_t) (pCpxThread[nThreadID]->nLastMomentun + ((THREADL_SLEEP != pCpxThread[nThreadID]->nStatus) ?
                pCpxThread[nThreadID]->nNice : pCpxThread[nThreadID]->control.nSleepTime));
    }

#define _CPTHREAD(T) pCpxThread[T]

    static size_t Momentum_Scheduler (void)
    {
        size_t nThread = nCurrentThread;
        uint32_t nCurTime = 0;
        uint32_t nNextTime = 0;
        uint32_t nMin = 0xFFFFFFFF;
        size_t nCThread = nCurrentThread + 1;
        static size_t nCount = 0;
        CpxThread* pThread = NULL;

        nCount = 0;

        if (nRunningThreads > 0)
        {
            nCount = 0;
            nCThread = nCurrentThread + 1;

            nCurTime = Cpx_GetCurrentTick ();

            while (nCount < nMaxThreads)
            {
                if (nCThread >= nMaxThreads)
                {
                    nCThread = 0;
                }

                if (NULL != (pThread = pCpxThread[nCThread]) && pThread->nStatus >= THREADL_RUNNING)
                {
                    nNextTime = pThread->nLastMomentun + ((THREADL_SLEEP != pThread->nStatus) ? pThread->nNice : pThread->control.nSleepTime);

                    if (nNextTime < nCurTime || (THREADL_START == pThread->nStatus || THREADL_NOW == pThread->nStatus))
                    {
                        nThread = nCThread;
                        return nThread;
                    }
                    else if (nNextTime - nCurTime < nMin)
                    {
                        nMin = nNextTime - nCurTime;
                        nThread = nCThread;
                    }
                }

                nCThread++;
                nCount++;
            } /* code */
        }

        return nThread;
    }

    static void Cpx_StopThread (void)
    {
        if (pCurrentThread != NULL)
        {
            if (pCurrentThread->nStatus != THREADL_WAITTAG && pCurrentThread->nStatus != THREADL_LOCK)
            {
                nRunningThreads--;
            }

            if ((pCurrentThread->nThreadController & CPX_CTRL_BROKER_STATIC) == 0 && pCurrentThread->pSubscriptions != NULL)
            {
#ifndef _CPX_NO_HEAP_
                free (pCurrentThread->pSubscriptions);
#endif
            }

            if ((pCurrentThread->nThreadController & CPX_CTRL_TYPE_STATIC) == 0)
            {
#ifndef _CPX_NO_HEAP_
                free (pCurrentThread);
#endif
            }

            pCurrentThread = NULL;
            pCpxThread[nCurrentThread] = NULL;

            nThreadCount--;
            nRunningThreads--;
        }
    }

    void Cpx_Join (void)
    {
        VERIFY (pCpxThread != NULL, );
        VERIFY (nThreadCount > 0, );

        pCurrentThread = pCpxThread[0];

        /* Porting for dealing with watchdogs */

        volatile uint8_t nValue = 0xAA;

        do
        {
            if (pCurrentThread != NULL)
            {
                pStartStck = (void*)&nValue;

                if (setjmp (jmpJoinPointer) == 0)
                {
                    switch (pCurrentThread->nStatus)
                    {
                        case THREADL_START:

                            pCurrentThread->nStatus = THREADL_RUNNING;

                            /* Porting for dealing with watchdogs */
                            pCurrentThread->mem.func.pFunction (pCurrentThread->mem.func.pValue);

                            Cpx_StopThread ();

                            continue;

                        case THREADL_RUNNING:
                        case THREADL_SLEEP:
                        case THREADL_NOW:

                            longjmp (pCurrentThread->mem.jmpRegisterBuffer, 1);
                            break;

                        default:
                            break;
                    }
                }
            }

            nCurrentThread = Momentum_Scheduler ();
            pCurrentThread = pCpxThread[nCurrentThread];
            /*(nCurrentThread + 1) >= nMaxThreads ? 0 : (nCurrentThread + 1); */

        } while (nRunningThreads > 0);

        pCurrentThread = NULL;
    }

    static void Cpx_SetMomentun (void)
    {
        uint32_t nCurTime = Cpx_GetCurrentTick ();

        if (NULL != pCurrentThread)
        {
            if ((THREADL_RUNNING == pCurrentThread->nStatus || THREADL_SLEEP == pCurrentThread->nStatus) &&
                Cpx_GetNextTime (nCurrentThread) > nCurTime)
            {
                Cpx_SleepTicks (Cpx_GetNextTime (nCurrentThread) - nCurTime);
            }
        }
    }

    static void Cpx_UpdateExecTime (void)
    {
        pCurrentThread->nExecTime = (Cpx_GetCurrentTick () - pCurrentThread->nLastMomentun);
    }

    static void Cpx_HandleStackOverflow (void)
    {
        if (pCurrentThread->nStackSize > pCurrentThread->nStackMaxSize)
        {
            Cpx_StackOverflowHandler ();

            Cpx_StopThread ();

            longjmp (jmpJoinPointer, 1);
        }
    }

    uint8_t Cpx_Yield (void)
    {
        VERIFY (pCurrentThread != NULL, 0);

        Cpx_UpdateExecTime ();

        {
            volatile uint8_t nValue = 0xBB;

            pCurrentThread->pLastStack = (void*)&nValue;
            pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;

            Cpx_HandleStackOverflow ();

            Cpx_BackupStack ();

            if (setjmp (pCurrentThread->mem.jmpRegisterBuffer) == 0)
            {
                longjmp (jmpJoinPointer, 1);
            }

            pCurrentThread->pLastStack = (void*)&nValue;
            pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;

            Cpx_RestoreStack ();

            assert (((uint8_t*)pCurrentThread->pLastStack)[pCurrentThread->nStackSize] == 0xAA && ((uint8_t*)pCurrentThread->pLastStack)[0] == 0xBB);
        }

        Cpx_SetMomentun ();

        pCurrentThread->control.nSleepTime = 0;
        pCurrentThread->nStatus = THREADL_RUNNING;

        pCurrentThread->nLastMomentun = Cpx_GetCurrentTick ();

        return 1;
    }

    void Cpx_SetSleep (uint32_t nDelayTickTime)
    {
        if (Cpx_IsCoreRunning ())
        {
            pCurrentThread->control.nSleepTime = nDelayTickTime;
            pCurrentThread->nStatus = THREADL_SLEEP;
        }
    }

    /*
     * --------------------------------------------------
     *   GET/SET'S FOR MANAGING THREADS -----------------
     * --------------------------------------------------
     */

    size_t Cpx_GetID (void)
    {
        VERIFY (Cpx_IsCoreRunning (), 0);

        return nCurrentThread;
    }

    size_t Cpx_GetStackSizeByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning (), 0);
        VERIFY (nID < nMaxThreads || pCpxThread[nID] != NULL, 0);

        return pCpxThread[nID]->nStackSize;
    }

    size_t Cpx_GetMaxStackSizeByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads && NULL != pCpxThread[nID], 0);

        return pCpxThread[nID]->nStackMaxSize;
    }

    uint32_t Cpx_GetNiceByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads && NULL != pCpxThread[nID], 0);

        return pCpxThread[nID]->nNice;
    }

    uint8_t Cpx_GetStatusByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads && NULL != pCpxThread[nID], 0);

        return pCpxThread[nID]->nStatus;
    }

    uint32_t Cpx_GetLastDutyCycleByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads && NULL != pCpxThread[nID], 0);

        return pCpxThread[nID]->nExecTime;
    }

    uint32_t Cpx_GetLastMomentumByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads && NULL != pCpxThread[nID], 0);

        return pCpxThread[nID]->nLastMomentun;
    }

    size_t Cpx_GetNumberOfActiveThreads (void)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        return nRunningThreads;
    }

    size_t Cpx_GetNumberOfThreads (void)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        return nThreadCount;
    }

    size_t Cpx_GetMaxNumberOfThreads (void)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        return nMaxThreads;
    }

    size_t Cpx_GetStructContextSize (void)
    {
        return sizeof (CpxThread);
    }

    size_t Cpx_GetContextSizeByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads || NULL != pCpxThread[nID], 0);

        {
            CpxThread* pThread = pCpxThread[nID];
            size_t nContextSize = 0;

            if (pThread->pSubscriptions != NULL)
            {
                nContextSize += Cpx_GetStaticBrokerSize (pThread->pSubscriptions->nMaxTopics);
            }

            nContextSize += Cpx_GetStaticThreadSize (pThread->nStackMaxSize);

            return nContextSize;
        }
    }

    void Cpx_SetNice (uint32_t nNice)
    {
        VERIFY (Cpx_IsCoreRunning (), );

        pCurrentThread->nNice = nNice;
    }

    bool Cpx_IsThreadStaticByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads || NULL != pCpxThread[nID], 0);

        return (pCpxThread [nID]->nThreadController & CPX_CTRL_TYPE_STATIC) ? true : false;
    }

    bool Cpx_IsBrokerStaticByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning () && nID < nMaxThreads || NULL != pCpxThread[nID], 0);

        return (pCpxThread [nID]->nThreadController & CPX_CTRL_BROKER_STATIC) ? true : false;
    }

    /*
     * --------------------------------------------------
     *  BROKER BASED IPC IMPLEMENTATION -----------------
     * --------------------------------------------------
     */

    static bool Cpx_CommonEnableBroker (void* pUserContext, uint16_t nMaxTopics, TopicCallback callback, CpxSubscriptions* pStaticSubscription)
    {
        if (pCurrentThread->pSubscriptions == NULL)
        {
            if (pStaticSubscription == NULL)
            {
#ifndef _CPX_NO_HEAP_
                size_t nMemorySize = sizeof (CpxSubscriptions) + (sizeof (uint32_t) * ((nMaxTopics <= 1) ? 0 : nMaxTopics - 1));

                VERIFY ((pStaticSubscription = malloc (nMemorySize)) != NULL, false);

                /*
                 * Set Static broker bit to 0 (disable)
                 */
                pCurrentThread->nThreadController &= (~CPX_CTRL_BROKER_STATIC);
#else
            return false;
#endif
            }
            else
            {
                /*
                 * Set Static broker bit to 1 (disable)
                 */
                pCurrentThread->nThreadController |= CPX_CTRL_BROKER_STATIC;
            }

            pStaticSubscription->nTopicCount = 0;
            pStaticSubscription->callback = callback;
            pStaticSubscription->nMaxTopics = nMaxTopics;
            pStaticSubscription->pContext = pUserContext;

            pCurrentThread->pSubscriptions = pStaticSubscription;

            return true;
        }

        return false;
    }

    bool Cpx_EnableStaticBroker (void* pUserContext, CpxStaticBroker* pStaticBroker, size_t nStaticBrokerSize, TopicCallback callback)
    {
        VERIFY (nStaticBrokerSize >= sizeof (CpxSubscriptions), false);
        VERIFY (Cpx_IsCoreRunning (), false);

        return Cpx_CommonEnableBroker (pUserContext, Cpx_GetStaticBrokerMaxTopics (nStaticBrokerSize), callback, (CpxSubscriptions*)pStaticBroker);
    }

    bool Cpx_EnableBroker (void* pUserContext, uint16_t nMaxTopics, TopicCallback callback)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        return Cpx_CommonEnableBroker (pUserContext, nMaxTopics, callback, NULL);
    }

    static int32_t Cpx_GetTopicID (const char* pszTopic, size_t length)
    {
        return ((int32_t) ((Cpx_CRC16 ((const uint8_t*)pszTopic, length, 0) << 15) | Cpx_CRC16 ((const uint8_t*)pszTopic, length, 0x8408)));
    }

    bool Cpx_IsSubscribed (const char* pszTopic, size_t length)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        if (pCurrentThread->pSubscriptions != NULL)
        {
            CpxSubscriptions* pSub = pCurrentThread->pSubscriptions;
            uint16_t nCount = 0;
            uint32_t nTopicID = Cpx_GetTopicID (pszTopic, length);

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

    bool Cpx_SubscribeTopic (const char* pszTopic, size_t length)
    {
        CpxSubscriptions* pSub = pCurrentThread->pSubscriptions;

        VERIFY (Cpx_IsCoreRunning (), false);

        if (Cpx_IsSubscribed (pszTopic, length) == false)
        {
            if (pSub != NULL && pSub->nTopicCount < pSub->nMaxTopics)
            {
                (&pSub->nTopicList)[pSub->nTopicCount++] = Cpx_GetTopicID (pszTopic, length);
                return true;
            }
        }

        return false;
    }

    bool Cpx_PublishTopic (const char* pszTopic, size_t length, size_t nAttribute, uint64_t nValue)
    {
        bool bReturn = false;

        size_t nThreadID = 0;
        uint32_t nTopicID = Cpx_GetTopicID (pszTopic, length);
        uint16_t nSubID = 0;

        VERIFY (Cpx_IsCoreRunning (), false);

        CpxMsgPayload payload = {nCurrentThread, nAttribute, nValue};

        for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
        {
            if (pCpxThread[nThreadID] != NULL && pCpxThread[nThreadID]->pSubscriptions != NULL)
            {
                for (nSubID = 0; nSubID < pCpxThread[nThreadID]->pSubscriptions->nTopicCount; nSubID++)
                {
                    if ((&pCpxThread[nThreadID]->pSubscriptions->nTopicList)[nSubID] == nTopicID)
                    {
                        pCpxThread[nThreadID]->pSubscriptions->callback (pCpxThread[nThreadID]->pSubscriptions->pContext, pszTopic, length, payload);
                        bReturn = true;
                    }
                }
            }
        }

        return bReturn;
    }

    bool Cpx_WaitMessage (const char* pszTag, size_t nTagLength, CpxMsgPayload* payload)
    {
        VERIFY (pszTag != NULL && nTagLength > 0, false);

        VERIFY (Cpx_IsCoreRunning (), false);

        VERIFY (pCurrentThread->nStatus == THREADL_RUNNING, false);

        pCurrentThread->nStatus = THREADL_WAITTAG;
        pCurrentThread->nNotifyUID = Cpx_GetTopicID (pszTag, nTagLength);

        nRunningThreads--;

        Cpx_Yield ();

        nRunningThreads++;

        *payload = pCurrentThread->control.payload;

        return true;
    }

    bool Cpx_Wait (const char* pszTag, size_t nTagLength)
    {
        VERIFY (pszTag != NULL && nTagLength > 0, false);

        VERIFY (Cpx_IsCoreRunning (), false);

        VERIFY (pCurrentThread->nStatus == THREADL_RUNNING, false);

        pCurrentThread->nStatus = THREADL_WAITTAG;

        pCurrentThread->nNotifyUID = Cpx_GetTopicID (pszTag, nTagLength);

        nRunningThreads--;

        Cpx_Yield ();

        nRunningThreads++;

        return true;
    }

    bool Cpx_SetTopicID (const char* pszTag, size_t nTagLength, uint32_t* pnTopicID)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        if (pnTopicID != NULL && (*pnTopicID = Cpx_GetTopicID (pszTag, nTagLength)) > 0)
        {
            return true;
        }

        return false;
    }

    static bool Cpx_Notify (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue, bool boolOne)
    {
        uint32_t nTopicID = 0;
        size_t nThreadID = 0;
        bool bReturn = false;

        /* Go through thread lists */
        if (pszTag == NULL || nTagLength == 0 || (nTopicID = Cpx_GetTopicID (pszTag, nTagLength)) == 0)
        {
            return false;
        }

        if (pszTag != NULL && nTagLength && pCurrentThread->nStatus == THREADL_RUNNING)
        {
            for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
            {
                if (pCpxThread[nThreadID] != NULL && pCpxThread[nThreadID]->nStatus == THREADL_WAITTAG &&
                    pCpxThread[nThreadID]->nNotifyUID == nTopicID)
                {
                    pCpxThread[nThreadID]->nStatus = THREADL_NOW;
                    pCpxThread[nThreadID]->control.payload = (CpxMsgPayload){nCurrentThread, nAttribute, nValue};
                    pCpxThread[nThreadID]->nNotifyUID = 0;

                    bReturn = true;

                    if (boolOne == true) break;
                }
            }
        }

        return bReturn;
    }

    bool Cpx_SafeNotifyOne (const char* pszTag, size_t nTagLength)
    {
        bool bResult = false;

        VERIFY (Cpx_IsCoreRunning (), false);

        bResult = Cpx_Notify (pszTag, nTagLength, 0, 0, true);

        return bResult;
    }

    bool Cpx_NotifyOne (const char* pszTag, size_t nTagLength)
    {
        bool bResult = Cpx_SafeNotifyOne (pszTag, nTagLength);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_SafeNotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = false;

        VERIFY (Cpx_IsCoreRunning (), false);

        bResult = Cpx_Notify (pszTag, nTagLength, nAttribute, nValue, true);

        return bResult;
    }

    bool Cpx_NotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = Cpx_SafeNotifyMessageOne (pszTag, nTagLength, nAttribute, nValue);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_SafeNotifyAll (const char* pszTag, size_t nTagLength)
    {
        bool bResult = false;

        VERIFY (Cpx_IsCoreRunning (), false);

        bResult = Cpx_Notify (pszTag, nTagLength, 0, 0, false);

        return bResult;
    }

    bool Cpx_NotifyAll (const char* pszTag, size_t nTagLength)
    {
        bool bResult = Cpx_SafeNotifyAll (pszTag, nTagLength);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_SafeNotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = false;

        VERIFY (Cpx_IsCoreRunning (), false);

        bResult = Cpx_Notify (pszTag, nTagLength, nAttribute, nValue, false);

        return bResult;
    }

    bool Cpx_NotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = Cpx_SafeNotifyMessageAll (pszTag, nTagLength, nAttribute, nValue);

        Cpx_NowYield ();

        return bResult;
    }

    /*
     * --------------------------------------------------
     *  THREAD SYNC FUNCTION (SMART LOCK / VARIABLELOCK)-
     * --------------------------------------------------
     */

    bool Cpx_WaitVariableLock (void* pnLockID, size_t* pnStatus)
    {
        uint8_t nReturn = false;

        VERIFY (Cpx_IsCoreRunning (), false);

        if (pCurrentThread != NULL && pnLockID != NULL)
        {
            pCurrentThread->control.pnVariableLockID = (void*)pnLockID;

            pCurrentThread->nStatus = THREADL_LOCK;

            nRunningThreads--;

            if (Cpx_Yield ())
            {
                if (pnStatus != NULL)
                {
                    *pnStatus = (size_t)pCurrentThread->control.pnVariableLockID;
                    pCurrentThread->control.payload.nAttribute = 0;
                }

                nReturn = true;
            }

            nRunningThreads++;
        }

        return nReturn;
    }

    size_t Cpx_SafeNotifyVariableLock (void* pnLockID, size_t nStatus, bool bOneOnly)
    {
        size_t nNotifiedCount = 0;

        VERIFY (pCpxThread != NULL, 0);

        if (pCurrentThread != NULL && nRunningThreads > 0 && pnLockID != NULL)
        {
            size_t nThreadID = 0;

            for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
            {
                if (pCpxThread[nThreadID] != NULL)
                {
                    if (pCpxThread[nThreadID]->control.pnVariableLockID == pnLockID && pCpxThread[nThreadID]->nStatus == THREADL_LOCK)
                    {
                        pCpxThread[nThreadID]->control.pnVariableLockID = (void*)nStatus;
                        pCpxThread[nThreadID]->nStatus = THREADL_NOW;

                        nNotifiedCount++;

                        if (bOneOnly) break;
                    }
                }
            }

            if (nNotifiedCount > 0)
            {
                Cpx_NowYield ();
            }
        }

        return nNotifiedCount;
    }

    size_t Cpx_NotifyVariableLock (void* pnLockID, size_t nStatus, bool bOneOnly)
    {
        size_t nCounter = Cpx_SafeNotifyVariableLock (pnLockID, nStatus, bOneOnly);

        if (nCounter)
        {
            Cpx_NowYield ();
        }

        return nCounter;
    }

    size_t Cpx_WaitingVariableLock (void* pnLockID)
    {
        size_t nNotifiedCount = 0;

        VERIFY (Cpx_IsCoreRunning (), 0);

        if (pCurrentThread != NULL && nRunningThreads > 0 && pnLockID != NULL)
        {
            size_t nThreadID = 0;

            for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
            {
                if (pCpxThread[nThreadID] != NULL)
                {
                    if (pCpxThread[nThreadID]->control.pnVariableLockID == pnLockID && pCpxThread[nThreadID]->nStatus == THREADL_LOCK)
                    {
                        nNotifiedCount++;
                    }
                }
            }
        }

        return nNotifiedCount;
    }

    bool Cpx_LockInit (CpxSmartLock* pLock)
    {
        VERIFY (pLock != NULL, false);

        pLock->nSharedLockCount = 0;
        pLock->bExclusiveLock = false;

        return true;
    }

    bool Cpx_TryLock (CpxSmartLock* pLock)
    {
        VERIFY (Cpx_IsCoreRunning (), false);
        VERIFY (pLock != NULL, false);

        /* check if  all the locks are done */
        if (pLock->nSharedLockCount > 0 || pLock->bExclusiveLock == true) return false;

        pLock->bExclusiveLock = true;

        return true;
    }

    bool Cpx_Lock (CpxSmartLock* pLock)
    {
        VERIFY (Cpx_IsCoreRunning (), false);
        VERIFY (pLock != NULL, false);

        /* Get exclusive lock */
        while (pLock->bExclusiveLock && Cpx_WaitVariableLock ((void*)&pLock->bExclusiveLock, NULL))
            ;

        pLock->bExclusiveLock = true;

        /* Wait all shared locks to be done */
        while (pLock->nSharedLockCount && Cpx_WaitVariableLock ((void*)&pLock->nSharedLockCount, NULL))
            ;

        return true;
    }

    bool Cpx_SharedLock (CpxSmartLock* pLock)
    {
        VERIFY (Cpx_IsCoreRunning (), false);
        VERIFY (pLock != NULL, false);

        while (pLock->bExclusiveLock > 0 && Cpx_WaitVariableLock ((void*)&pLock->bExclusiveLock, NULL))
            ;
        pLock->nSharedLockCount++;

        Cpx_NotifyVariableLock ((void*)&pLock->nSharedLockCount, 0, false);
        Cpx_NotifyVariableLock ((void*)&pLock->bExclusiveLock, 0, false);

        return true;
    }

    bool Cpx_SharedUnlock (CpxSmartLock* pLock)
    {
        VERIFY (Cpx_IsCoreRunning (), false);
        VERIFY (pLock != NULL, false);

        if (pLock->nSharedLockCount)
        {
            pLock->nSharedLockCount--;

            Cpx_NotifyVariableLock ((void*)&pLock->nSharedLockCount, 0, false);

            return true;
        }

        return false;
    }

    bool Cpx_Unlock (CpxSmartLock* pLock)
    {
        VERIFY (Cpx_IsCoreRunning (), false);

        VERIFY (pLock != NULL, false);

        if (pLock->bExclusiveLock == true)
        {
            pLock->bExclusiveLock = false;

            Cpx_NotifyVariableLock ((void*)&pLock->nSharedLockCount, 0, false);
            Cpx_NotifyVariableLock ((void*)&pLock->bExclusiveLock, 0, true);

            return true;
        }

        return false;
    }

    void* Cpx_GetLockID ()
    {
        VERIFY (Cpx_IsCoreRunning (), NULL);

        return (pCurrentThread == NULL) ? NULL : pCurrentThread->control.pnVariableLockID;
    }

    void* Cpx_GetLockIDByID (size_t nID)
    {
        VERIFY (Cpx_IsCoreRunning (), NULL);

        return (nID >= nMaxThreads || NULL == pCpxThread[nID]) ? NULL : pCpxThread[nID]->control.pnVariableLockID;
    }

#ifdef __cplusplus
}
#endif