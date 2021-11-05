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


#ifndef Cpx_hpp
#define Cpx_hpp

#ifdef __cplusplus
extern "C"
{
#else
#if 0
#ifndef bool
#define bool uint8_t
#define false 0
#define true (!false)
#endif
#endif
#endif

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

    /* Official version */
    #define CPX_VERSION "2.7.1"
    static const char CpxVersionCode[]=CPX_VERSION;
    static const char CpxVersion[] = CPX_VERSION "/" __TIMESTAMP__;

    /*
     * IMPORTANT!
     * KEEP RUNNING STATES ALWAYS
     * ABOVE RUNNING AND BLOCKING
     * STATES BELOW RUNNING.
     */
    enum __THREADL_TYPES
    {
        /*
         * BLocking
         */
        THREADL_NONE = 0,
        THREADL_STOPPED,
        THREADL_LOCK,
        THREADL_WAITTAG,

        /*
         * Running
         */
        THREADL_RUNNING = 100,
        THREADL_START,
        THREADL_NOW,

        THREADL_SLEEP = 200
    };

    typedef struct
    {
        size_t nThreadID;
        size_t nAttribute;
        uint64_t nValue;
    } CpxMsgPayload;

    typedef struct
    {
        size_t nSharedLockCount;
        bool bExclusiveLock;
    } CpxSmartLock;

    /*
     * TOPIC Callback for broker implemenation
     */
    typedef void (*TopicCallback) (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payLoad);

    /*
     * Defining subscription structure
     */
    typedef struct CpxSubscriptions CpxSubscriptions;
    struct CpxSubscriptions
    {
        TopicCallback callback;
        void* pContext;
        uint16_t nMaxTopics;
        uint16_t nTopicCount;
        uint32_t nTopicList;
    };

    /*
     *  Global definition for CpxThread
     */
    typedef struct
    {
        void* pLastStack;
        CpxSubscriptions* pSubscriptions;

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

        /*
         * This will be, also, used as
         * buffer for sleep.
         */
        uint32_t nNotifyUID;

        union
        {
            uint32_t nSleepTime;
            void* pnVariableLockID;
            CpxMsgPayload payload; /* data */
        } control;

        uint8_t nThreadController;
        uint8_t nStatus;
        uint8_t stackPage;
    } CpxThread;

    /* Static Memory types */
    typedef uint8_t CpxStaticThread;
    typedef uint8_t CpxStaticBroker;

    /* External prototypes for functions overloading */
    extern uint32_t Cpx_GetCurrentTick (void);
    extern void Cpx_SleepTicks (uint32_t);
    extern void Cpx_StackOverflowHandler (void);

    /**
     * @brief Start CorePartition thread provisioning
     *
     * @param nThreadPartitions     Number of threads to be provisioned
     *
     * @return true  true if successfully created all provisioned threads
     */
    bool Cpx_Start (size_t nThreadPartitions);

    /**
     * @brief Start CorePartition thread provisioning
     *
     * @param ppStaticThread     Static Thread pointer type CpxThread**
     * @param nStaticThreadSize  Static Thread size in bytes
     *
     * @return true  true if successfully created all provisioned threads
     */
    bool Cpx_StaticStart (CpxThread** ppStaticThread, size_t nStaticThreadSize);

    /**
     * @brief  Create a non-Isolated context Thread
     *
     * @param pFunction         Function (void Function (void* dataPointer)) as thread main
     * @param pValue            data that will be injected on Thread creation
     * @param nStackMaxSize     Size of the Stack to be used
     * @param nNice             When in time it is good to be used
     *
     * @return false            fails on more provisioned threads or no more memory to create it
     *
     * @note                    All threads will be create with the size of stack plus context size (~100 bytes)
     */
    bool Cpx_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);

/**
 * @brief   Return Context size (Stack Memory page) from a Static Memory size
 *
 * @param nStaticThreadSize    The size (in bytes) of the stack memory
 *
 * @return  the size of the context
 */
#define Cpx_GetStaticContextSize(nStaticThreadSize) (nStaticThreadSize >= sizeof (CpxThread) ? (nStaticThreadSize - sizeof (CpxThread)) : 0)

/**
 * @brief   Return the full Static Thread context in bytes
 *
 * @param   nStackMaxSize   Stack memory page size
 *
 * @return  The size of the full Static Thread (context + stak memory page)
 */
#define Cpx_GetStaticThreadSize(nStackMaxSize) (sizeof (CpxThread) + nStackMaxSize)

    /**
     * @brief   Create a Thread using a static Context + virtual stack page
     *
     * @param pFunction         Function (void Function (void* dataPointer)) as thread main
     * @param pValue            data that will be injected on Thread creation
     * @param pStaticThread     The Static context + virtual stack pointer
     * @param nStaticThreadSize Size of the Static Thread in bytes
     * @param nNice             When in time it is good to be used
     *
     * @note    No memory will be created.
     *
     * @return false    In case of parameter error
     */

    bool Cpx_CreateStaticThread (void (*pFunction) (void*), void* pValue, CpxStaticThread* pStaticThread, size_t nStaticThreadSize, uint32_t nNice);

    /**
     * @brief This will start all the threads
     *
     * @note At least ONE thread must be defines before using this function
     */
    void Cpx_Join (void);

    /**
     * @brief   Function to be called inside a thread to change context
     *
     * @return true  always return true while the thread is valid
     *
     * @note    Cooperative state yield, should not be used with preemption
     *          for speed reasons, that will not complay with LockKernel,
     *          for this use PreemptionYield.
     */
    uint8_t Cpx_Yield (void);

/**
 * @brief  Will set the thread to a special sleep state
 *
 * @param nDelayTickTime    How much ticks to sleep
 *
 * @note    if Time has been overridden it tick will  correspond
 *          to the time frame used by sleep overridden function
 */
#define Cpx_Sleep(nDelayTickTime)      \
    {                                  \
        Cpx_SetSleep (nDelayTickTime); \
        Cpx_Yield ();                  \
    }

    /**
     * @brief  Will set the thread sleep parameter before Yield
     *
     * @param nDelayTickTime    How much ticks to sleep
     *
     * @note    if Time has been overridden it tick will  correspond
     *          to the time frame used by sleep overridden function
     */
    void Cpx_SetSleep (uint32_t nDelayTickTime);

    /**
     * @brief Get Current Thread ID
     *
     * @return size_t   Thread ID
     */
    size_t Cpx_GetID (void);

    /**
     * @brief Get Current Thread ID
     *
     * @param   nID     A valid ID
     *
     * @return size_t   Thread ID
     *
     * @note    if a non valid ID is provided it will return 0
     */
    size_t Cpx_GetStackSizeByID (size_t nID);

/**
 * @brief Get Current Thread Stack Size
 */
#define Cpx_GetStackSize() Cpx_GetStackSizeByID (Cpx_GetID ())

    /**
     * @brief  Get total size of stack context page for a Thread ID
     *
     * @param   nID     Thread ID
     *
     * @return size_t  total size of stack context page
     */
    size_t Cpx_GetMaxStackSizeByID (size_t nID);

/**
 * @brief Get Current Thread Total Stack context page size
 */
#define Cpx_GetMaxStackSize() Cpx_GetMaxStackSizeByID (Cpx_GetID ())

    /**
     * @brief Get Thread context size
     *
     * @return size_t total size of the thread context
     */
    size_t Cpx_GetStructContextSize (void);

    /**
     * @brief   Get Full context size (Contex + stack + Broker)
     *
     * @return size_t
     */
    size_t Cpx_GetContextSizeByID (size_t nID);

/**
 * @brief Return Full context size from current thread
 */
#define Cpx_GetContexSize() (Cpx_GetContextSizeByID (Cpx_GetID ()))

    /**
     * @brief  Get a thread status for a thread ID
     *
     * @param   nID  Thread ID
     *
     * @return uint8_t Actual thread context
     */
    uint8_t Cpx_GetStatusByID (size_t nID);

/**
 * @brief Get current Thread Status
 */
#define Cpx_GetStatus() Cpx_GetStatusByID (Cpx_GetID ())

    /**
     * @brief Current Thread Nice
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t Nice representing tick
     *
     * @note    Tick will represent the overridden time interface otherwise it will
     *          be a single context switch to each
     */
    uint32_t Cpx_GetNiceByID (size_t nID);

/**
 * @brief Get Current Thread Nice
 */
#define Cpx_GetNice() Cpx_GetNiceByID (Cpx_GetID ())

    /**
     * @brief Set Current Thread Nice
     *
     * @param nNice  Nice to be used
     */
    void Cpx_SetNice (uint32_t nNice);

    /**
     * @brief Get Current Thread last momentum on switch back
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t the LastMomentum in Tick
     *
     * @note    Tick will represent the overridden time interface otherwise it will
     *          be a single context switch to each.
     */
    uint32_t Cpx_GetLastMomentumByID (size_t nID);

/**
 * @brief Get current Thread Last Momentum
 */
#define Cpx_GetLastMomentum() Cpx_GetLastMomentumByID (Cpx_GetID ())

    /**
     * @brief  Last Duty Cycle of the current Thread
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t  time in Tick
     *
     * @note    Tick will represent the overridden time interface otherwise it will
     *          be a single context switch to each.
     */
    uint32_t Cpx_GetLastDutyCycleByID (size_t nID);

/**
 * @brief  Get Current Thread DutyCycle
 */
#define Cpx_GetLastDutyCycle() Cpx_GetLastDutyCycleByID (Cpx_GetID ())

    /**
     * @brief  Get Number of total active Threads
     *
     * @return size_t number of threads
     */
    size_t Cpx_GetNumberOfActiveThreads (void);

    /**
     * @brief Get Number of assigned Threads
     *
     * @return size_t number of threads
     */
    size_t Cpx_GetNumberOfThreads (void);

    /**
     * @brief  Get Max Number of total active Threads
     *
     * @return size_t number of threads
     */
    size_t Cpx_GetMaxNumberOfThreads (void);

    /**
     * @brief Return the Secure Status for a thread ID
     *
     * @param nID  Thread ID
     *
     * @return char  return 'S' for secure and 'N' for normal
     */
    char Cpx_IsSecureByID (size_t nID);

    /**
     * @brief Report if there is any running thread
     *
     * @return false in case there is none running
     */
    bool Cpx_IsCoreRunning (void);

    /**
     * @brief   Return if a Threads was statically initiated
     *
     * @param   nID     Thread ID
     *
     * @return  false   if it was not statically initiated
     */
    bool Cpx_IsThreadStaticByID (size_t nID);

    /**
     * @brief   Return if the current thread was statically initiated
     *
     * @param   nID     Thread ID
     *
     * @return  false   if it was not statically initiated
     */
#define Cpx_IsThreadStatic() Cpx_IsThreadStaticByID (Cpx_GetID ())

    /**
     * @brief   Return if a Thread's Broker interface was statically initiated
     *
     * @param   nID     Thread ID
     *
     * @return  false   if it was not statically initiated
     */
    bool Cpx_IsBrokerStaticByID (size_t nID);

    /**
     * @brief   Return if the curremt Thread's Broker interface
     *          was statically initiated
     *
     * @param   nID     Thread ID
     *
     * @return  false   if it was not statically initiated
     */
#define Cpx_IsBrokerStatic() Cpx_IsThreadStaticByID (Cpx_GetID ())

    /**
     * @brief   Enable Broker for the current thread
     *
     * @param   pUserContext    The default context to be ejected if needed
     * @param   nMaxTopics  Max topics to be handled by the current thread
     * @param   callback    Call back to be used to process thread Synchronously
     *
     * @return  false       failed to create the broker context for the current thread
     *
     * @note    The default context must not be part of the thread§ stack, or it will be invalid
     *          on callback time, please use global variables or from heap (new or malloc memory),
     *          AGAIN: NEVER USE A LOCAL FUNCTION VARIABLE AS CONTEXT, USE A GLOBAL VARIABLE OR
     *          A ALLOCATED MEMORY.
     */
    bool Cpx_EnableBroker (void* pUserContext, uint16_t nMaxTopics, TopicCallback callback);

    /**
     * @brief   Return the size of a static Broker Context Size for a Thread
     *
     * @param   nMaxTopics  Max number of Topics (minimal 1, smaller will be automatically set to 1)
     *
     * @return  The struct + topic list in bytes to be used
     */
#define Cpx_GetStaticBrokerSize(nMaxTopics) \
    (sizeof (CpxSubscriptions) + (sizeof (uint32_t) * ((nMaxTopics <= 1) ? sizeof (uint32_t) : nMaxTopics - 1)))

    /*
     * Calculate how much subscription a broker size can do.
     */
#define Cpx_GetStaticBrokerMaxTopics(nStaticSubsSize) \
    (nStaticSubsSize <= sizeof (CpxSubscriptions) ? 0 : ((nStaticSubsSize - sizeof (CpxSubscriptions)) / sizeof (uint32_t)) + 1)

    /**
     * @brief   Enable Broker for the current thread
     *
     * @param   pUserContext        The default context to be ejected if needed
     * @param   pStaticBroker       Static Broker CpxSubscriptions pointer
     * @param   nStaticBrokerSize   Static Broker CpxSubscriptions pointer size in bytes
     * @param   callback            Call back to be used to process thread Synchronously
     *
     * @return  false       failed to create the broker context for the current thread
     *
     * @note    The default context must not be part of the thread stack, or it will be invalid
     *          on callback time, please use global variables or from heap (new or malloc memory),
     *          AGAIN: NEVER USE A LOCAL FUNCTION VARIABLE AS CONTEXT, USE A GLOBAL VARIABLE OR
     *          A ALLOCATED MEMORY.
     */
    bool Cpx_EnableStaticBroker (void* pUserContext, CpxStaticBroker* pStaticBroker, size_t nStaticBrokerSize, TopicCallback callback);

    /**
     * @brief   Subscribe for a specific topic
     *
     * @param   pszTopic    The topic to listen for information
     * @param   length      The size of the topic string
     *
     * @return  false       if there is no more room for a new subscription
     */
    bool Cpx_SubscribeTopic (const char* pszTopic, size_t length);

    /**
     * @brief   Public a tuple Param and Value
     *
     * @param   pszTopic    Topic name to publish
     * @param   length      The size of the topic string
     * @param   nAttribute  A attribute to be use to identify the value
     * @param   nValue      A value for the attribute (tuple)
     *
     * @return  true    If at least one subscriber received the data.
     *
     * @note    By nature, publish is safe, although, any Broker handler should NEVER call
     *          for any function that change context and always use safe functions instead.
     */
    bool Cpx_PublishTopic (const char* pszTopic, size_t length, size_t nAttribute, uint64_t nValue);

    /**
     * @brief   Check if the current thread already subscribe to a topic
     *
     * @param   pszTopic    The topic for information
     * @param   length      The size of the topic string
     *
     * @return  false       if it was not subscribed
     */
    bool Cpx_IsSubscribed (const char* pszTopic, size_t length);

    /**
     * @brief   Notify ONE TAG assigned as waiting thread
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     *
     * @return true         At least one thread will be notified;
     *
     * @note    Please note that any notification triggers a context switch yield
     */
    bool Cpx_NotifyOne (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Safely notify ONE TAG assigned as waiting thread
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     *
     * @return true         At least one thread will be notified;
     *
     * @note    This will not trigger context change, ideal for IRQs.
     *          Will be processed on next Context Change call.
     */
    bool Cpx_SafeNotifyOne (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Notify ONE TAGs assigned as waiting thread with a Message payload
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     * @param   nAttribute  The Attribute Value to be sent
     * @param   nValue      The Value of the Attribute to be sent
     *
     * @return true         At least one thread will be notified;
     *
     * @note    Please note that any notification triggers a context switch yield
     */
    bool Cpx_NotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue);

    /**
     * @brief   Safely notify ONE TAGs assigned as waiting thread with a Message payload
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     * @param   nAttribute  The Attribute Value to be sent
     * @param   nValue      The Value of the Attribute to be sent
     *
     * @return true         At least one thread will be notified;
     *
     * @note    This will not trigger context change, ideal for IRQs.
     *          Will be processed on next Context Change call.
     */
    bool Cpx_SafeNotifyMessageOne (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue);

    /**
     * @brief   Notify ALL TAGs assigned as waiting thread
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     *
     * @return true         At least one thread will be notified;
     *
     * @note    Please note that any notification triggers a context switch yield
     */
    bool Cpx_NotifyAll (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Safely notify ALL TAGs assigned as waiting thread
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     *
     * @return true         At least one thread will be notified;
     *
     * @note    This will not trigger context change, ideal for IRQs.
     *          Will be processed on next Context Change call.
     */
    bool Cpx_SafelyNotifyAll (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Notify ALL TAGs assigned as waiting thread with a Message payload
     *
     * @param   pszTag      The TAG string value
     * @param   nTagLength  The length of the tag
     * @param   nAttribute  The Attribute Value to be sent
     * @param   nValue      The Value of the Attribute to be sent
     *
     * @return true         At least one thread will be notified;
     *
     * @note    Please note that any notification triggers a context switch yield
     */
    bool Cpx_NotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue);

    /**
     * @brief   Safely notify ALL TAGs assigned as waiting thread with a Message payload
     *
     * @param   pszTag      The TAG string value
     * @param   nTagLength  The length of the tag
     * @param   nAttribute  The Attribute Value to be sent
     * @param   nValue      The Value of the Attribute to be sent
     *
     * @return true         At least one thread will be notified;
     *
     * @note    This will not trigger context change, ideal for IRQs.
     *          Will be processed on next Context Change call.
     */
    bool Cpx_SafelyNotifyMessageAll (const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue);

    /**
     * @brief   Wait for a specific notification from a given TAG
     *
     * @param   pszTag        The Tag string value
     * @param   nTagLength    The length of the tag
     *
     * @return  true    For success on receiving notification
     */
    bool Cpx_Wait (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Wait for a specific notification from a given TAG and payload
     *
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     * @param   payload     The payload with the information sent by other thread
     *
     * @return  false   if an error occurred
     *
     * @note    if a Tag was notified using NotifyOne or NotifyAll
     *          the thread will receive 0 otherwise will receive
     *          the same value sent.
     */
    bool Cpx_WaitMessage (const char* pszTag, size_t nTagLength, CpxMsgPayload* payload);

    /**
     * @brief   Init SmartLock variable
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   the lock is null
     *
     * @note    If you re initialise it will unlock all locks
     */
    bool Cpx_LockInit (CpxSmartLock* pLock);

    /**
     * @brief   Do a exclusive Lock and set to Simple lock
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   the lock is null
     *
     * @note    Wait till Lock is unlocked (type none) and lock it
     *          set to type Simple and lock, SharedLock will wait
     *          till it is unlocked.
     */
    bool Cpx_Lock (CpxSmartLock* pLock);

    /**
     * @brief   Like Lock() but only locks in case it is unlocked
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   If lock is null or lock is locked
     *
     * @note    Wait till exclusive Lock is unlocked (type none) and lock it
     *          set to type Simple and lock, SharedLock will wait
     *          till it is unlocked.
     */
    bool Cpx_TryLock (CpxSmartLock* pLock);

    /**
     * @brief   Can act as multiple locks
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   If lock is null
     *
     * @note    Can acquire multiples locks and lock()
     *          will wait till all multiples locks has been unlocked
     *          to lock exclusively
     */
    bool Cpx_SharedLock (CpxSmartLock* pLock);

    /**
     * @brief   Unlock exclusive locks
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   If lock is null
     */
    bool Cpx_Unlock (CpxSmartLock* pLock);

    /**
     * @brief   Unlock shared locks
     *
     * @param   pLock      The Lock variable
     *
     * @return  false   If lock is null
     */
    bool Cpx_SharedUnlock (CpxSmartLock* pLock);

    /**
     * @brief  Wait for a Variable Locks notification
     *
     * @param nLockID   Variable address
     * @param pnStatus  Payload to be sent, a size_t
     *
     * @return false    if LockID is invalid (== 0) or no data
     */
    bool Cpx_WaitVariableLock (void* nLockID, size_t* pnStatus);

    /**
     * @brief   Return how much threads are locked waiting for a variable notification
     *
     * @param nLockID   Variable address
     *
     * @return size_t   How much active waiting for a variable
     */
    size_t Cpx_WaitingVariableLock (void* nLockID);

    /**
     * @brief   Notify all/one Variable lock waiting for notification
     *
     * @param nLockID   Variable address
     * @param nStatus   Payload to be sent, a size_t
     * @param bOneOnly  If true only one is notified
     *
     * @return false    if LockID is invalid (== 0) or no data
     */
    size_t Cpx_NotifyVariableLock (void* nLockID, size_t nStatus, bool bOneOnly);

    /**
     * @brief   Safely notify all/one Variable lock waiting for notification
     *
     * @param nLockID   Variable address
     * @param nStatus   Payload to be sent, a size_t
     * @param bOneOnly  If true only one is notified
     *
     * @return false    if LockID is invalid (== 0) or no data
     *
     * @note    This will not trigger context change, ideal for IRQs.
     *          Will be processed on next Context Change call.
     */
    size_t Cpx_SafeNotifyVariableLock (void* nLockID, size_t nStatus, bool bOneOnly);

/**
 * @brief   Notify one Variable lock waiting for notification
 *
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 *
 * @return false    if LockID is invalid (== 0) or no data
 */
#define Cpx_NotifyVariableLockOne(nLockID, nStatus) Cpx_NotifyVariableLock (nLockID, nStatus, true)

/**
 * @brief   Safely notify one Variable lock waiting for notification
 *
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 *
 * @return false    if LockID is invalid (== 0) or no data
 *
 * @note    This will not trigger context change, ideal for IRQs.
 *          Will be processed on next Context Change call.
 */
#define Cpx_SafeNotifyVariableLockOne(nLockID, nStatus) Cpx_SafeNotifyVariableLock (nLockID, nStatus, true)

/**
 * @brief   Notify all Variable lock waiting for notification
 *
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 *
 * @return false    if LockID is invalid (== 0) or no data
 */
#define Cpx_NotifyVariableLockAll(nLockID, nStatus) Cpx_NotifyVariableLock (nLockID, nStatus, false)

/**
 * @brief   Safely notify all Variable lock waiting for notification
 *
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 *
 * @return false    if LockID is invalid (== 0) or no data
 *
 * @note    This will not trigger context change, ideal for IRQs.
 *          Will be processed on next Context Change call.
 */
#define Cpx_SafelyNotifyVariableLockAll(nLockID, nStatus) Cpx_SafelyNotifyVariableLock (nLockID, nStatus, false)

    void* Cpx_GetLockID (void);

    void* Cpx_GetLockIDByID (size_t nID);

#ifdef __cplusplus
}
#endif

#endif /* Cpx_hpp */
