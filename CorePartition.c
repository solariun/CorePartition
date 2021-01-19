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

#ifdef __cplusplus
extern "C"
{
#endif

#include "CorePartition.h"

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

    enum __cp_lock_type
    {
        CP_LOCK_NONE = 0,
        CP_LOCK_SIMPLE,
        CP_LOCK_SHARED
    };

    typedef struct
    {
        void* pLastStack;
        Subscription* pSubscriptions;

        union
        {
            jmp_buf jmpRegisterBuffer;

            struct
            {
                void (*pFunction) (void* pStart);
                void* pValue;
            } func;
        } mem;

        size_t nStackMaxSize;
        size_t nStackSize;

        uint32_t nNice;
        uint32_t nLastMomentun;
        uint32_t nExecTime;

        uint32_t nNotifyUID;
        void* pnVariableLockID;
        CpxMsgPayload payload;

        uint8_t nStatus;
        uint8_t stackPage;
    } CoreThread;

    static volatile size_t nMaxThreads = 0;
    static volatile size_t nThreadCount = 0;
    static volatile size_t nRunningThreads = 0;
    static volatile size_t nCurrentThread;

    CoreThread** pCoreThread = NULL;
    CoreThread* pCurrentThread = NULL;
    void* pStartStck = NULL;

    jmp_buf jmpJoinPointer;

#define Cpx_SetState(nNewState) pCurrentThread->nStatus = nNewState
#define Cpx_NowYield()                  \
    {                                   \
        Cpx_SetState (THREADL_NOW);     \
        Cpx_Yield ();                   \
        Cpx_SetState (THREADL_RUNNING); \
    }

    bool bCpxPreemptionState = false;
    bool bCpxPLockQuest = false;
    bool bCpxPLock = false;

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
        TRACE ("Error, Thread#%zu Stack %zu / %zu max\n", Cpx_GetID (), Cpx_GetStackSize (), Cpx_GetMaxStackSize ());
    }

    /*
     * --------------------------------------------------
     * String functions not capable of overloading
     * --------------------------------------------------
     */


    /*
     * -------------------------------------------------- 
     * KERNEL FUNCTIONS ---------------------------------
     * --------------------------------------------------
     */

    static void Cpx_PrintDebugInfo ()
    {
        size_t nCount = 0;
        CoreThread* pLCoreThread = NULL;

        YYTRACE ("------------------------------------------------\n");
        YYTRACE ("Context size: [%zu], size_t: [%zu] in bytes\n", Cpx_GetThreadContextSize(), sizeof (size_t));
        YYTRACE ("------------------------------------------------\n");
        YYTRACE ("  %-4s  %-4s  %5s/%-5s  %-8s  %-8s\n", "TID", "Stat", " Stack", "Max", "Nice", " VarLock");
        YYTRACE ("------------------------------------------------\n");

        for (nCount = 0; nCount < nMaxThreads; nCount++)
        {
            if ((pLCoreThread = pCoreThread[nCount]) != NULL)
            {
                YYTRACE ("%1c %-4zu  %-4u  %5zu/%-5zu  %-8u  %-8zx\n",
                         nCurrentThread == nCount ? '*' : ' ',
                         nCount,
                         pLCoreThread->nStatus,
                         pLCoreThread->nStackSize,
                         pLCoreThread->nStackMaxSize,
                         pLCoreThread->nNice,
                         (size_t)pLCoreThread->pnVariableLockID);
            }
        }

        YYTRACE ("------------------------------------------------\n");
    }

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

    bool Cpx_Start (size_t nThreadPartitions)
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

    bool Cpx_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice)
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

        pCoreThread[nThread]->nStackMaxSize = nStackMaxSize;

        pCoreThread[nThread]->mem.func.pValue = pValue;

        pCoreThread[nThread]->nStatus = THREADL_START;

        pCoreThread[nThread]->nStackSize = 0;

        pCoreThread[nThread]->mem.func.pFunction = pFunction;

        pCoreThread[nThread]->nNice = 1;

        pCoreThread[nThread]->nExecTime = 0;

        pCoreThread[nThread]->nNice = nNice;

        pCoreThread[nThread]->nLastMomentun = Cpx_GetCurrentTick ();

        pCoreThread[nThread]->pSubscriptions = NULL;

        pCoreThread[nThread]->nNotifyUID = 0;

        pCoreThread[nThread]->payload = (CpxMsgPayload){0, 0, 0};

        pCoreThread[nThread]->pnVariableLockID = 0;

        nThreadCount++;

        return true;
    }

    static void BackupStack (void)
    {
        memcpy ((uint8_t*)&pCurrentThread->stackPage, (const uint8_t*)pCurrentThread->pLastStack, pCurrentThread->nStackSize);
    }

    static void RestoreStack (void)
    {
        memcpy ((uint8_t*)pCurrentThread->pLastStack, (const uint8_t*)&pCurrentThread->stackPage, pCurrentThread->nStackSize);
    }

    static uint32_t Cpx_GetNextTime (size_t nThreadID)
    {
        return (uint32_t) (pCoreThread[nThreadID]->nLastMomentun + pCoreThread[nThreadID]->nNice);
    }

#define _CPTHREAD(T) pCoreThread[T]

    static size_t Momentum_Scheduler (void)
    {
        size_t nThread = nCurrentThread;
        uint32_t nCurTime = Cpx_GetCurrentTick ();
        uint32_t nNextTime = 0;
        uint32_t nMin = 0xFFFFFFFF;
        size_t nCThread = nCurrentThread + 1;
        static size_t nCount = 0;
        CoreThread* pThread = NULL;

        nCount = 0;

        /*
         * Check for system dead lock.
         */
        for (nCount = 0; nCount < nMaxThreads; nCount++)
        {
            if (pCoreThread[nCount] != NULL && pCoreThread[nCount]->nStatus >= THREADL_RUNNING)
            {
                break;
            }
        }

        Cpx_PrintDebugInfo ();

        if (nCount == nMaxThreads) nRunningThreads = 0;

        if (nRunningThreads > 0)
        {
            nCount = 0;
            nCThread = nCurrentThread + 1;

            while (nCount < nMaxThreads)
            {
                if (nCThread >= nMaxThreads)
                {
                    nCThread = 0;
                }

                if (NULL != (pThread = pCoreThread[nCThread]) && pThread->nStatus >= THREADL_RUNNING)
                {
                    nNextTime = pThread->nLastMomentun + pThread->nNice;

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
            if (pCurrentThread->pSubscriptions != NULL)
            {
                free (pCurrentThread->pSubscriptions);
            }

            free (pCurrentThread);

            pCurrentThread = NULL;
            pCoreThread[nCurrentThread] = NULL;

            nRunningThreads--;
        }
    }

    size_t nNextThread = 0;
    void Cpx_Join (void)
    {
        if (nThreadCount == 0) return;

        pCurrentThread = pCoreThread[0];

        do
        {
            volatile uint8_t nValue = 0xAA;

            if (pCurrentThread != NULL)
            {
                pStartStck = (void*)&nValue;

                if (setjmp (jmpJoinPointer) == 0) switch (pCurrentThread->nStatus)
                    {
                        case THREADL_START:

                            nRunningThreads++;

                            pCurrentThread->nStatus = THREADL_RUNNING;

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

            nCurrentThread = Momentum_Scheduler ();
            pCurrentThread = pCoreThread[nCurrentThread];
            /*(nCurrentThread + 1) >= nMaxThreads ? 0 : (nCurrentThread + 1); */

        } while (nRunningThreads);

        TRACE ("Leaving...  running: %zu\n", nRunningThreads);

        Cpx_PrintDebugInfo ();
    }

    void Cpx_SetMomentun (void)
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

        pCurrentThread->nLastMomentun = Cpx_GetCurrentTick ();
    }

    void Cpx_UpdateExecTime (void)
    {
        pCurrentThread->nExecTime = (Cpx_GetCurrentTick () - pCurrentThread->nLastMomentun);
    }

    void Cpx_CheckStackOverflow (void)
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
        if (nRunningThreads > 0)
        {
            volatile uint8_t nValue = 0xBB;

            Cpx_UpdateExecTime ();

            pCurrentThread->pLastStack = (void*)&nValue;
            pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;

            Cpx_CheckStackOverflow ();

            BackupStack ();

            if (setjmp (pCurrentThread->mem.jmpRegisterBuffer) == 0)
            {
                longjmp (jmpJoinPointer, 1);
            }

            pCurrentThread->pLastStack = (void*)&nValue;
            pCurrentThread->nStackSize = (size_t)pStartStck - (size_t)pCurrentThread->pLastStack;

            RestoreStack ();

            Cpx_SetMomentun ();

            pCurrentThread->nStatus = THREADL_RUNNING;

            return 1;
        }

        return 0;
    }

    void Cpx_Sleep (uint32_t nDelayTickTime)
    {
        uint32_t nBkpNice = 0;

        VERIFY (pCurrentThread->nStatus == THREADL_RUNNING, );

        nBkpNice = pCurrentThread->nNice;

        pCurrentThread->nStatus = THREADL_SLEEP;
        pCurrentThread->nNice = nDelayTickTime;

        Cpx_Yield ();

        pCurrentThread->nStatus = THREADL_RUNNING;
        pCurrentThread->nNice = nBkpNice;
    }

    /*
     * -------------------------------------------------- 
     *   GET/SET'S FOR MANAGING THREADS -----------------
     * -------------------------------------------------- 
     */

    size_t Cpx_GetID (void)
    {
        return nCurrentThread;
    }

    size_t Cpx_GetStackSizeByID (size_t nID)
    {
        VERIFY (nID < nMaxThreads || pCoreThread[nID] != NULL, 0);

        return pCoreThread[nID]->nStackSize;
    }

    size_t Cpx_GetMaxStackSizeByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nStackMaxSize;
    }

    uint32_t Cpx_GetNiceByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nNice;
    }

    uint8_t Cpx_GetStatusByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nStatus;
    }

    uint32_t Cpx_GetLastDutyCycleByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nExecTime;
    }

    uint32_t Cpx_GetLastMomentumByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->nLastMomentun;
    }

    size_t Cpx_GetNumberOfActiveThreads (void)
    {
        return nRunningThreads;
    }

    size_t Cpx_GetMaxNumberOfThreads (void)
    {
        return nMaxThreads;
    }

    size_t Cpx_GetThreadContextSize (void)
    {
        return sizeof (CoreThread);
    }

    bool Cpx_IsCoreRunning (void)
    {
        return nRunningThreads > 0 ? true : false;
    }

    void Cpx_SetNice (uint32_t nNice)
    {
        pCurrentThread->nNice = nNice == 0 ? 1 : nNice;
    }

    /*
     * -------------------------------------------------- 
     *  BROKER BASED IPC IMPLEMENTATION -----------------
     * -------------------------------------------------- 
     */

    bool Cpx_EnableBroker (void* pContext, uint8_t nMaxTopics, TopicCallback callback)
    {
        if (pCurrentThread->pSubscriptions == NULL)
        {
            Subscription* pSub = NULL;
            size_t nMemorySize = sizeof (Subscription) + (sizeof (uint32_t) * ((nMaxTopics <= 1) ? 0 : nMaxTopics - 1));

            if ((pSub = malloc (nMemorySize)) != NULL)
            {
                pSub->nTopicCount = 0;
                pSub->callback = callback;
                pSub->nMaxTopics = nMaxTopics;
                pSub->pContext = pContext;

                pCurrentThread->pSubscriptions = pSub;

                return true;
            }
        }

        return false;
    }

    static int32_t Cpx_GetTopicID (const char* pszTopic, size_t length)
    {
        return ((int32_t) ((Cpx_CRC16 ((const uint8_t*)pszTopic, length, 0) << 15) | Cpx_CRC16 ((const uint8_t*)pszTopic, length, 0x8408)));
    }

    bool Cpx_IsSubscribed (const char* pszTopic, size_t length)
    {
        if (pCurrentThread != NULL && pCurrentThread->pSubscriptions != NULL)
        {
            Subscription* pSub = pCurrentThread->pSubscriptions;
            int nCount = 0;
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
        Subscription* pSub = pCurrentThread->pSubscriptions;

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
        size_t nSubID = 0;

        CpxMsgPayload payload = {nCurrentThread, nAttribute, nValue};

        for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
        {
            if (pCoreThread[nThreadID] != NULL && pCoreThread[nThreadID]->pSubscriptions != NULL)
            {
                for (nSubID = 0; nSubID < pCoreThread[nThreadID]->pSubscriptions->nTopicCount; nSubID++)
                {
                    if ((&pCoreThread[nThreadID]->pSubscriptions->nTopicList)[nSubID] == nTopicID)
                    {
                        pCoreThread[nThreadID]->pSubscriptions->callback (
                                pCoreThread[nThreadID]->pSubscriptions->pContext, pszTopic, length, payload);
                        bReturn = true;
                    }
                }
            }
        }

        return bReturn;
    }

    bool Cpx_WaitMessage (const char* pszTag, size_t nTagLength, CpxMsgPayload* payload)
    {
        if (pszTag == NULL || nTagLength == 0 || pCurrentThread->nStatus != THREADL_RUNNING) return false;

        pCurrentThread->nStatus = THREADL_WAITTAG;
        pCurrentThread->nNotifyUID = Cpx_GetTopicID (pszTag, nTagLength);

        Cpx_Yield ();

        *payload = pCurrentThread->payload;

        return true;
    }

    bool Cpx_Wait (const char* pszTag, size_t nTagLength)
    {
        if (pszTag == NULL || nTagLength == 0 || pCurrentThread->nStatus != THREADL_RUNNING) return false;

        pCurrentThread->nStatus = THREADL_WAITTAG;

        pCurrentThread->nNotifyUID = Cpx_GetTopicID (pszTag, nTagLength);

        Cpx_Yield ();

        return true;
    }

    bool Cpx_SetTopicID (const char* pszTag, size_t nTagLength, uint32_t* pnTopicID)
    {
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
                if (pCoreThread[nThreadID] != NULL && pCoreThread[nThreadID]->nStatus == THREADL_WAITTAG &&
                    pCoreThread[nThreadID]->nNotifyUID == nTopicID)
                {
                    pCoreThread[nThreadID]->nStatus = THREADL_NOW;
                    pCoreThread[nThreadID]->payload = (CpxMsgPayload){nCurrentThread, nAttribute, nValue};
                    pCoreThread[nThreadID]->nNotifyUID = 0;

                    bReturn = true;

                    if (boolOne == true) break;
                }
            }
        }

        return bReturn;
    }

    bool Cpx_NotifyOne (const char* pszTag, size_t nTagLength)
    {
        bool bResult = Cpx_Notify (pszTag, nTagLength, 0, 0, true);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_NotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = Cpx_Notify (pszTag, nTagLength, nAttribute, nValue, true);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_NotifyAll (const char* pszTag, size_t nTagLength)
    {
        bool bResult = Cpx_Notify (pszTag, nTagLength, 0, 0, false);

        Cpx_NowYield ();

        return bResult;
    }

    bool Cpx_NotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue)
    {
        bool bResult = Cpx_Notify (pszTag, nTagLength, nAttribute, nValue, false);

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

        if (pnLockID > 0)
        {
            TRACE ("%s: ThreadID: [%zu], nLockID: [%zx]\n", __FUNCTION__, nCurrentThread, (size_t)pnLockID);

            pCurrentThread->pnVariableLockID = (void*) pnLockID;

            pCurrentThread->nStatus = THREADL_LOCK;

            if (Cpx_Yield ())
            {
                if (pnStatus != NULL)
                {
                    *pnStatus = (size_t)pCurrentThread->pnVariableLockID;
                    pCurrentThread->payload.nAttribute = 0;
                }

                TRACE ("%s: Thread #%zu, Received trap variable [%zx]\n", __FUNCTION__, nCurrentThread, (size_t)pnLockID);

                nReturn = true;
            }

            pCurrentThread->nStatus = THREADL_RUNNING;
        }

        return nReturn;
    }

    size_t Cpx_NotifyVariableLock (void* pnLockID, size_t nStatus, bool bOneOnly)
    {
        size_t nNotifiedCount = 0;

        if (nRunningThreads > 0 && pnLockID > 0)
        {
            size_t nThreadID = 0;

            for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
            {
                if (pCoreThread[nThreadID] != NULL)
                {
                    if (pCoreThread[nThreadID]->pnVariableLockID == pnLockID && pCoreThread[nThreadID]->nStatus == THREADL_LOCK)
                    {
                        TRACE ("%s: TID#%-4zu (%-16zx) Notifying  -> Thread #%zu\n", __FUNCTION__, nCurrentThread, (size_t)pnLockID, nThreadID);

                        pCoreThread[nThreadID]->pnVariableLockID = (void*)nStatus;
                        pCoreThread[nThreadID]->nStatus = THREADL_NOW;

                        nNotifiedCount++;

                        if (bOneOnly) break;
                    }
                }
            }

            TRACE ("%s: Thread #%zu Notified (%zu] from ID [%zx] \n", __FUNCTION__, nCurrentThread, nNotifiedCount, (size_t)pnLockID);

            if (nNotifiedCount > 0)
            {
                Cpx_NowYield ();
            }
        }

        return nNotifiedCount;
    }

    size_t Cpx_WaitingVariableLock (void* pnLockID)
    {
        size_t nNotifiedCount = 0;

        if (nRunningThreads > 0 && pnLockID > 0)
        {
            size_t nThreadID = 0;

            for (nThreadID = 0; nThreadID < nMaxThreads; nThreadID++)
            {
                if (pCoreThread[nThreadID] != NULL)
                {
                    if (pCoreThread[nThreadID]->pnVariableLockID == pnLockID && pCoreThread[nThreadID]->nStatus == THREADL_LOCK)
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
        if (pLock == false) return false;

        pLock->nSharedLockCount = 0;
        pLock->bExclusiveLock = false;

        return true;
    }

    bool Cpx_TryLock (CpxSmartLock* pLock)
    {
        if (pLock == false) return false;

        /* Wait all the locks to be done */
        if (pLock->nSharedLockCount > 0 || pLock->bExclusiveLock == true) return false;

        pLock->bExclusiveLock = true;

        return true;
    }

#define Cpx_PrintLock(pLock)                                                              \
    TRACE ("%s: Thread #%zu Lock: [%c](waiting: [%zu]), Shared: [%zu](waiting: [%zu])\n", \
           __FUNCTION__,                                                                  \
           nCurrentThread,                                                                \
           pLock->bExclusiveLock ? 'T' : 'F',                                             \
           Cpx_WaitingVariableLock ((void*)&pLock->bExclusiveLock),                       \
           pLock->nSharedLockCount,                                                       \
           Cpx_WaitingVariableLock ((void*)&pLock->nSharedLockCount));

    bool Cpx_Lock (CpxSmartLock* pLock)
    {
        if (pLock == NULL) return false;

        Cpx_PrintLock (pLock);

        /* Get exclusive lock */
        while (pLock->bExclusiveLock)
        {
            if (pLock->bExclusiveLock == true)
            {
                Cpx_WaitVariableLock ((void*)&pLock->bExclusiveLock, NULL);
            }
        }

        pLock->bExclusiveLock = true;

        /* Wait all shared locks to be done */
        while (pLock->nSharedLockCount)
        {
            if (pLock->nSharedLockCount)
            {
                Cpx_WaitVariableLock ((void*)&pLock->nSharedLockCount, NULL);
            }
        }

        return true;
    }

    bool Cpx_SharedLock (CpxSmartLock* pLock)
    {
        if (pLock == NULL) return false;

        Cpx_PrintLock (pLock);

        while (pLock->bExclusiveLock > 0)
        {
            if (pLock->bExclusiveLock > 0)
            {
                Cpx_WaitVariableLock ((void*)&pLock->bExclusiveLock, NULL);
            }
        }

        pLock->nSharedLockCount++;

        Cpx_NotifyVariableLock ((void*)&pLock->nSharedLockCount, 0, false);
        Cpx_NotifyVariableLock ((void*)&pLock->bExclusiveLock, 0, false);

        return true;
    }

    bool Cpx_SharedUnlock (CpxSmartLock* pLock)
    {
        if (pLock == NULL) return false;

        Cpx_PrintLock (pLock);

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
        if (pLock == NULL) return false;

        Cpx_PrintLock (pLock);

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
        return pCurrentThread->pnVariableLockID;
    }

    void* Cpx_GetLockIDByID (size_t nID)
    {
        return (nID >= nMaxThreads || NULL == pCoreThread[nID]) ? 0 : pCoreThread[nID]->pnVariableLockID;
    }

#ifdef __cplusplus
}
#endif
