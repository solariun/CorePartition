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

#ifndef CorePartition_hpp
#define CorePartition_hpp

#ifdef __cplusplus
extern "C"
{
#else
#ifndef bool
#define bool uint8_t
#define false 0
#define true (!false)
#endif 
#endif

#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define THREADL_NONE 0
#define THREADL_START 1
#define THREADL_RUNNING 2
#define THREADL_SLEEP 3
#define THREADL_STOPPED 4
#define THREADL_WAITTAG 5
#define THREADL_NOW 6

    typedef struct
    {
        size_t nThreadID;
        size_t nAttribute;
        uint64_t nValue;
    } CpxMsgPayload;

    typedef void (*TopicCallback) (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payLoad);

    extern uint32_t CorePartition_GetCurrentTick (void);
    extern void CorePartition_SleepTicks (uint32_t);

    static const char CorePartition_version[] = "V2.6.0.1 develop Compiled at " __TIMESTAMP__;

    /**
     * @brief Start CorePartition thread provisioning
     *
     * @param nThreadPartitions     Number of threads to be aprovisioned
     * @return true  true if successfully created all provisioned threads
     */
    bool CorePartition_Start (size_t nThreadPartitions);

    /**
     * @brief  Create a non-Isolated context Thread
     *
     * @param pFunction         Function (void Function (void* dataPonter)) as thread main
     * @param pValue            data that will be injected on Thread creation
     * @param nStackMaxSize     Size of the Stack to be used
     * @param nNice             When in time it is good to be used
     *
     * @return false            fails on more provisioned threads or no more memory to create it
     *
     * @note                    All threads will be create with the size of stack plus context size (~100 bytes)
     */
    bool CorePartition_CreateThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);

    /**
     * @brief  Create a Isolated context Thread
     *
     * @param pFunction         Function (void Function (void* dataPonter)) as thread main
     * @param pValue            data that will be injected on Thread creation
     * @param nStackMaxSize     Size of the Stack to be used
     * @param nNice             When in time it is good to be used
     *
     * @return false            fails on more provisioned threads or no more memory to create it
     *
     * @note                    All threads will be create with the size of stack plus context size (~100 bytes)
     * @note                    Isolated thread will control dynamic stack encryption on Context Change
     *                          have in mind it will be costy on time so use it wisely.
     */
    bool CorePartition_CreateSecureThread (void (*pFunction) (void*), void* pValue, size_t nStackMaxSize, uint32_t nNice);

    /**
     * @brief  Callback for informing Stack Overflow Thread destruction and actions
     *
     * @param pStackOverflowHandler     Callback to be called
     *
     * @return false     only return false on errors
     */
    bool CorePartition_SetStackOverflowHandler (void (*pStackOverflowHandler) (void));

    /**
     * @brief This will start all the threads
     *
     * @note At least ONE thread must be defines before using this function
     */
    void CorePartition_Join (void);

    /**
     * @brief   Function to be called inside a thread to change context
     *
     * @return true  always return true while the thread is valid
     */
    uint8_t CorePartition_Yield (void);

    /**
     * @brief  Will set the thread to a special sleep state
     *
     * @param nDelayTickTime    How much ticks to sleep
     *
     * @note    if Time has been overriden it tick will the corespond
     *          the time frame used by sleep overriden function
     */
    void CorePartition_Sleep (uint32_t nDelayTickTime);

    /**
     * @brief Get Current Thread ID
     *
     * @return size_t   Thread ID
     */
    size_t CorePartition_GetID (void);

    /**
     * @brief Get Current Thread ID
     *
     * @param   nID     A valid ID
     *
     * @return size_t   Thread ID
     *
     * @note    if a non valid ID is provided it will return 0
     */
    size_t CorePartition_GetStackSizeByID (size_t nID);

/**
 * @brief Get Current Thread Stack Size
 */
#define CorePartition_GetStackSize() CorePartition_GetStackSizeByID (CorePartition_GetID ())

    /**
     * @brief  Get total size of stack context page for a Thread ID
     *
     * @param   nID     Thread ID
     *
     * @return size_t  total size of stack context page
     */
    size_t CorePartition_GetMaxStackSizeByID (size_t nID);

/**
 * @brief Get Current Thread Total Stack context page size
 */
#define CorePartition_GetMaxStackSize() CorePartition_GetMaxStackSizeByID (CorePartition_GetID ())

    /**
     * @brief Get Thread context size
     *
     * @return size_t total size of the thread context
     */
    size_t CorePartition_GetThreadContextSize (void);

    /**
     * @brief  Get a thread status for a thread ID
     *
     * @param   nID  Thread ID
     *
     * @return uint8_t Actual thread context
     */
    uint8_t CorePartition_GetStatusByID (size_t nID);

/**
 * @brief Get current Thread Status
 */
#define CorePartition_GetStatus() CorePartition_GetStatusByID (CorePartition_GetID ())

    /**
     * @brief Current Thread Nice
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t Nice representing tick
     *
     * @note    Tick will represent the overriden time interface othersize it will
     *          be a single context switch to each
     */
    uint32_t CorePartition_GetNiceByID (size_t nID);

/**
 * @brief Get Current Thread Nice
 */
#define CorePartition_GetNice() CorePartition_GetNiceByID (CorePartition_GetID ())

    /**
     * @brief Set Current Thread Nice
     *
     * @param nNice  Nice to be used
     */
    void CorePartition_SetNice (uint32_t nNice);

    /**
     * @brief Get Current Thread last momentum on swtich back
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t the LastMomentum in Tick
     *
     * @note    Tick will represent the overriden time interface othersize it will
     *          be a single context switch to each.
     */
    uint32_t CorePartition_GetLastMomentumByID (size_t nID);

/**
 * @brief Get current Thread Last Momentum
 */
#define CorePartition_GetLastMomentum() CorePartition_GetLastMomentumByID (CorePartition_GetID ())

    /**
     * @brief  Last Duty Cycle of the current Thread
     *
     * @param   nID  Thread ID
     *
     * @return uint32_t  time in Tick
     *
     * @note    Tick will represent the overriden time interface othersize it will
     *          be a single context switch to each.
     */
    uint32_t CorePartition_GetLastDutyCycleByID (size_t nID);

/**
 * @brief  Get Current Thread DutyCalycle
 */
#define CorePartition_GetLastDutyCycle() CorePartition_GetLastDutyCycleByID (CorePartition_GetID ())

    /**
     * @brief  Get Number of total active Threads
     *
     * @return size_t numver of threads
     */
    size_t CorePartition_GetNumberOfActiveThreads (void);

    /**
     * @brief  Get Max Number of total active Threads
     *
     * @return size_t numver of threads
     */
    size_t CorePartition_GetMaxNumberOfThreads (void);

    /**
     * @brief Return the Secure Status for a thread ID
     *
     * @param nID  Thread OD
     *
     * @return char  return 'S' for secure and 'N' for normal
     */
    char CorePartition_IsSecureByID (size_t nID);

    /**
     * @brief Report if there is any running thread
     *
     * @return false in case there is none running
     */
    bool CorePartition_IsCoreRunning (void);

    /**
     * @brief   Set the default name of the current thread
     *
     * @param nID           Thread ID
     * @param pszName       Name of the Thread
     * @param nNameSize     Size of the name
     *
     * @return false        if the size is zero or name is null
     */
    bool CorePartition_SetThreadNameByID (size_t nID, const char* pszName, uint8_t nNameSize);

/**
 * @brief Set current thread name
 *
 * @param pszName       String with the new thread name
 * @param nSize         Size of the string with the Thread name
 *
 * @return  false       if the data is null
 */
#define CorePartition_SetThreadName(pszName, nNameSize) CorePartition_SetThreadNameByID (CorePartition_GetID (), pszName, nNameSize)

    /**
     * @brief Get the thread name of a specific Thread ID
     *
     * @param nID   Thread ID to return the name
     *
     * @return const char*  The associated thread's name;
     */
    const char* CorePartition_GetThreadNameByID (size_t nID);

/**
 * @brief Get the current thread name
 *
 * @return      const char* The associated thread's name
 */
#define CorePartition_GetThreadName() CorePartition_GetThreadName (CorePartition_GetID ())

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
     *          A MEMORY ALOCATED ONE.
     */
    bool CorePartition_EnableBroker (void* pContext, uint8_t nMaxTopics, TopicCallback callback);

    /**
     * @brief   Subscribe for a specific topic
     *
     * @param   pszTopic    The topic to listem for information
     * @param   length      The size of the topic string
     *
     * @return  false       if there is no more room for a new subscription
     */
    bool CorePartition_SubscribeTopic (const char* pszTopic, size_t length);

    /**
     * @brief   Public a tuple Param and Value
     *
     * @param   pszTopic    Topic name to pubish
     * @param   length      The size of the topic string
     * @param   nAttribute  A attribute to be use to identify the value
     * @param   nValue      A value for the attribute (tuple)
     *
     * @return  true    If at least one subscriber received the data.
     * 
     */
    bool CorePartition_PublishTopic (const char* pszTopic, size_t length, size_t nAttribute, uint64_t nValue );

    /**
     * @brief   Check if the current thread already subscribe to a topic
     *
     * @param   pszTopic    The topic for information
     * @param   length      The size of the topic string
     *
     * @return  false       if it was not subscribed
     */
    bool CorePartition_IsSubscribed (const char* pszTopic, size_t length);

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
    bool CorePartition_NotifyOne(const char* pszTag, size_t nTagLength);

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
    bool CorePartition_NotifyMessageOne(const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue );

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
    bool CorePartition_NotifyAll(const char* pszTag, size_t nTagLength);

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
    bool CorePartition_NotifyMessageAll(const char* pszTag, size_t nTagLength, size_t nAttribute, uint64_t nValue );

    /**
     * @brief   Wait for a specific notification from a given TAG
     * 
     * @param   pszTag        The Tag string value
     * @param   nTagLength    The length of the tag
     * 
     * @return  true    For success on receiving notification
     */
    bool CorePartition_Wait (const char* pszTag, size_t nTagLength);

    /**
     * @brief   Wait for a specific notification from a given TAG and payload
     * 
     * @param   pszTag      The Tag string value
     * @param   nTagLength  The length of the tag
     * @param   payload     The payload with the information sent by other thread   
     * 
     * @return  false   if an error ocurred
     * 
     * @note    if a Tag was notified using NotifyOne or NotifyAll
     *          the thread will receibe 0 otherwise will receive 
     *          the same value sent.
     */
    bool CorePartition_WaitMessage (const char* pszTag, size_t nTagLength, CpxMsgPayload* payload);

    /**
     * @brief   Return Context Switch lock state
     * 
     * @return  true a context swith is being performed 
     */
    bool CorePartition_IsLocked (void);

    /**
     * @brief   Lock for Context Switch
     */
    void CorePartition_Lock (void);

    /**
     * @brief  Unlock for Context Switch
     * 
     */
    void CorePartition_Unlock (void);

#ifdef __cplusplus
}
#endif


#endif /* CorePartition_hpp */
