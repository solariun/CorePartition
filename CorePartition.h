/*
//  CorePartition.hpp
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
*/

#ifndef Cpx_hpp
#define Cpx_hpp

#ifdef __cplusplus
extern "C"
{
#else
//#ifndef bool
//#define bool uint8_t
//#define false 0
//#define true (!false)
//#endif
#endif

#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    THREADL_NOW ,
    
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
    size_t nLockCount;
    bool bExclusiveLock;
}CpxSmartLock;

typedef void (*TopicCallback) (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payLoad);

extern uint32_t Cpx_GetCurrentTick (void);
extern void Cpx_SleepTicks (uint32_t);
extern void Cpx_StackOverflowHandler (void);

static const char Cpx_version[] = "V2.7.0 from  " __TIMESTAMP__;

/**
 * @brief Start CorePartition thread provisioning
 *
 * @param nThreadPartitions     Number of threads to be provisioned
 * @return true  true if successfully created all provisioned threads
 */
bool Cpx_Start (size_t nThreadPartitions);

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
 * @brief  Create a Isolated context Thread
 *
 * @param pFunction         Function (void Function (void* dataPointer)) as thread main
 * @param pValue            data that will be injected on Thread creation
 * @param nStackMaxSize     Size of the Stack to be used
 * @param nNice             When in time it is good to be used
 *
 * @return false            fails on more provisioned threads or no more memory to create it
 *
 * @note                    All threads will be create with the size of stack plus context size (~100 bytes)
 * @note                    Isolated thread will control dynamic stack encryption on Context Change
 *                          have in mind it will be costly on time so use it wisely.
 */
bool Cpx_CreateSecureThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);

/**
 * @brief  Callback for informing Stack Overflow Thread destruction and actions
 *
 * @param pStackOverflowHandler     Callback to be called
 *
 * @return false     only return false on errors
 */
bool Cpx_SetStackOverflowHandler (void (*pStackOverflowHandler) (void));

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
void Cpx_Sleep (uint32_t nDelayTickTime);

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
size_t Cpx_GetThreadContextSize (void);

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
 * @brief  Get Max Number of total active Threads
 *
 * @return size_t number of threads
 */
size_t Cpx_GetMaxNumberOfThreads (void);

/**
 * @brief Return the Secure Status for a thread ID
 *
 * @param nID  Thread OD
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
 * @brief   Set the default name of the current thread
 *
 * @param nID           Thread ID
 * @param pszName       Name of the Thread
 * @param nNameSize     Size of the name
 *
 * @return false        if the size is zero or name is null
 */
bool Cpx_SetThreadNameByID (size_t nID, const char* pszName, uint8_t nNameSize);

/**
* @brief Set current thread name
*
* @param pszName       String with the new thread name
* @param nSize         Size of the string with the Thread name
*
* @return  false       if the data is null
*/
#define Cpx_SetThreadName(pszName, nNameSize) Cpx_SetThreadNameByID (Cpx_GetID (), pszName, nNameSize)

/**
 * @brief Get the thread name of a specific Thread ID
 *
 * @param nID   Thread ID to return the name
 *
 * @return const char*  The associated thread's name;
 */
const char* Cpx_GetThreadNameByID (size_t nID);

/**
* @brief Get the current thread name
*
* @return      const char* The associated thread's name
*/
#define Cpx_GetThreadName() Cpx_GetThreadName (Cpx_GetID ())

/**
 * @brief   Enable Broker for the current thread
 *
 * @param   pContext    The default context to be ejected if needed
 * @param   nMaxTopics  Max topics to be handled by the current thread
 * @param   callback    Call back to be used to process thread Synchronously
 *
 * @return  false       failed to create the broker context for the current thread
 *
 * @note    The default context must not be part of the thread stack, or it will be invalid
 *          on callback time, please use global variables or from heap (new or malloc memory),
 *          AGAIN: NEVER USE A LOCAL FUNCTION VARIABLE AS CONTEXT, USE A GLOBAL VARIABLE OR
 *          A ALLOCATED MEMORY.
 */
bool Cpx_EnableBroker (void* pContext, uint8_t nMaxTopics, TopicCallback callback);

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
 */
bool Cpx_PublishTopic (const char* pszTopic, size_t length, size_t nAttribute, uint64_t nValue );

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
bool Cpx_NotifyOne(const char* pszTag, size_t nTagLength);

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
bool Cpx_NotifyMessageOne(const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue );

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
bool Cpx_NotifyAll(const char* pszTag, size_t nTagLength);

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
bool Cpx_NotifyMessageAll(const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue );

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
 * @param nLockID   LockID size_t used to notify
 * @param pnStatus  Payload to be sent, a size_t
 *
 * @return false    if LockID is invalid (== 0) or no data
 */
bool Cpx_WaitVariableLock (size_t nLockID, uint8_t* pnStatus);

size_t Cpx_WaitingVariableLock (size_t nLockID);

/**
 * @brief   Notify all/one Variable lock waiting for notification
 *
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 *
 * @return false    if LockID is invalid (== 0) or no data
 */
size_t Cpx_NotifyVariableLock (size_t nLockID, uint8_t nStatus, bool bOneOnly);

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
 * @brief   Notify all Variable lock waiting for notification
 * 
 * @param nLockID   LockID size_t used to notify
 * @param nStatus   Payload to be sent, a size_t
 * @param bOneOnly  If true only one is notified
 * 
 * @return false    if LockID is invalid (== 0) or no data
 */
#define Cpx_NotifyVariableLockAll(nLockID, nStatus) Cpx_NotifyVariableLock (nLockID, nStatus, false)


size_t Cpx_GetLockID();

size_t Cpx_GetLockIDByID(size_t nID);

#ifdef __cplusplus
}
#endif

#endif /* Cpx_hpp */
