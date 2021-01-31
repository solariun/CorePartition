///
/// @author   GUSTAVO CAMPOS
/// @author   GUSTAVO CAMPOS
/// @date   28/05/2019 19:44
/// @version  <#version#>
///
/// @copyright  (c) GUSTAVO CAMPOS, 2019
/// @copyright  Licence
///
/// @see    ReadMe.txt for references
///
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

#include <assert.h>
#include "Arduino.h"
#include "CorePartition.h"

#define THREAD_VALUES_ATTRB 1
/* Static Contexts */

CpxThread* pThreadContexts[3];

/* Consummer and producer threads */
CpxStaticThread pStaticCounter[1][Cpx_GetStaticThreadSize (25 * sizeof (size_t))];
CpxStaticThread pStaticConsumer[1][Cpx_GetStaticThreadSize (30 * sizeof (size_t))];

/* Eventual Thread */
CpxStaticThread pStaticStack[1][Cpx_GetStaticThreadSize (25 * sizeof (size_t))];

/* Broker static memory */
CpxStaticBroker brokerSubscriptions[Cpx_GetStaticBrokerSize (1)];

void ShowRunningThreads ()
{
    size_t nThreadID = 0;
    size_t nTotalMemory = 0;

    Serial.println ();
    Serial.println (F ("Listing all running threads"));
    Serial.println (F ("--------------------------------------"));
    Serial.println (F ("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));

    for (nThreadID = 0; nThreadID < Cpx_GetMaxNumberOfThreads (); nThreadID++)
    {
        Serial.print (F ("\e[K"));
        if (Cpx_GetStatusByID (nThreadID) != THREADL_NONE)
        {
            Serial.print (nThreadID);
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetStatusByID (nThreadID));
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetNiceByID (nThreadID));
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetStackSizeByID (nThreadID));
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetMaxStackSizeByID (nThreadID));
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetStructContextSize ());
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetContextSizeByID (nThreadID));
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetLastDutyCycleByID (nThreadID));
            Serial.print ("ms");
            Serial.print ("Static: ");
            Serial.print ((pThreadContexts[nThreadID]->nThreadController & 1) ? "Y" : "N");
            Serial.print ("Broker: ");
            Serial.print ((pThreadContexts[nThreadID]->nThreadController & 2) ? "Y" : "N");

            nTotalMemory += Cpx_GetContextSizeByID (nThreadID);
        }
        Serial.println ("\e[0K");
    }

    Serial.print ("\e[0KTotal Context + stack: ");
    Serial.println (nTotalMemory);
    Serial.println ("\e[0K");
}
// Global context for async access
uint32_t nValues[3] = {0, 0, 0};

char pszNotificationTag[] = "thread/proc/value";

void ProducerThread (void* pValue)
{
    // no c++ cast added to support AVR c++ 98
    uint32_t nValue = (uint32_t) ((size_t) (pValue));

    while (true)
    {
        nValue++;

        Cpx_PublishTopic (pszNotificationTag, sizeof (pszNotificationTag) - 1, THREAD_VALUES_ATTRB, (uint64_t)nValue);

        Cpx_Yield ();
    }
}

void ThreadCounterMessageHandler (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payload)
{
    uint32_t* nValues = (uint32_t*)pContext;

    if (payload.nThreadID <= 2 && payload.nAttribute == THREAD_VALUES_ATTRB)
    {
        nValues[payload.nThreadID] = (uint32_t)payload.nValue;
    }
}

const char pszEventualTag[] = "eventual";

uint32_t WaitForData ()
{
    CpxMsgPayload payload = {0, 0, 0};

    if (Cpx_WaitMessage (pszEventualTag, sizeof (pszEventualTag) - 1, &payload) == false)
    {
        Serial.print ("    Error Waiting for messages.   ");
        return 0;
    }
    
    return (uint32_t) payload.nValue;
}

void eventualThread (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nRemoteValue = 1;

    uint32_t nLast = Cpx_GetCurrentTick ();

    Serial.print (">> Eventual Thread");
    Serial.print (Cpx_GetID ());
    Serial.print (": Requested, Starting Up...");

    while (nRemoteValue)
    {
        nRemoteValue = WaitForData ();

        Serial.print (">> Eventual Thread");
        Serial.print (Cpx_GetID ());
        Serial.print (": ");
        Serial.print (nValue++);
        Serial.print (": ");
        Serial.print (nRemoteValue);
        Serial.print (F (", Sleep Time: "));
        Serial.print (Cpx_GetLastMomentum () - nLast);

        nLast = Cpx_GetLastMomentum ();
        Serial.println (F ("ms\e[0K\n"));
        Serial.flush ();
                    
        Cpx_Yield ();
    }

    Serial.println ("------------------------------------------");
    Serial.print ("Thread #"); 
    Serial.print (Cpx_GetID ());
    Serial.println (" eventual thread DONE!");
    Serial.println ("------------------------------------------");
    Serial.flush ();
    
    Cpx_Yield ();

}

void ConsumerThread (void* pValue)
{
    Cpx_EnableStaticBroker ((void*)nValues, brokerSubscriptions, sizeof (brokerSubscriptions), ThreadCounterMessageHandler);

    Cpx_SubscribeTopic (pszNotificationTag, sizeof (pszNotificationTag) - 1);

    unsigned long nLast = millis ();

    uint32_t* pnValues = (uint32_t*)&nValues;

    int nCount = 0;
    int nFail = 0;

    while (1)
    {
        Serial.print (">>Thread");
        Serial.print (Cpx_GetID () + 1);
        Serial.print (": [");
        Serial.print (pnValues[0]);
        Serial.print (", ");
        Serial.print (pnValues[1]);
        Serial.print (", ");
        Serial.print (pnValues[2]);
        Serial.print ("], running: ");
        Serial.print (Cpx_GetNumberOfActiveThreads ());
        Serial.print (", Sleep Time: ");
        Serial.print ((unsigned long)Cpx_GetLastMomentum () - nLast);
        nLast = Cpx_GetLastMomentum ();
        Serial.print ("ms, Nice: ");
        Serial.print (Cpx_GetNice ());
        Serial.print (", CTX: ");
        Serial.print (Cpx_GetStructContextSize ());
        Serial.print ("b, Stack: ");
        Serial.print (Cpx_GetStackSize ());
        Serial.print ("/");
        Serial.print (Cpx_GetMaxStackSize ());
        Serial.print (", Notification fail:");
        Serial.print (nFail);
        Serial.print (", DutyCycle Time: ");
        Serial.print (Cpx_GetLastDutyCycle ());
        Serial.println ("ms\e[0k\n\n");

        Serial.flush ();

        Cpx_Yield ();

        if (Cpx_GetStatusByID (2) == THREADL_NONE)
        {
            Cpx_CreateStaticThread (eventualThread, NULL, pStaticStack [0], sizeof (pStaticStack [0]), 100);
            nCount = 1;
            nFail = 0;
        }

        if (nCount)
        {
            if (nCount == 15)
            {
                if (Cpx_NotifyMessageOne (pszEventualTag, sizeof (pszEventualTag) - 1, 0, 0) == false)
                {
                    nFail++;
                }
                {
                   nCount = 0;
                }
            }
            else
            {
                if (Cpx_NotifyMessageOne (pszEventualTag, sizeof (pszEventualTag) - 1, 0, 1) == false)
                {
                    nFail++;
                }
                else
                {
                    nCount++;
                }
            }
        }
 
    }
}

uint32_t Cpx_GetCurrentTick ()
{
    return (uint32_t)millis ();
}

void Cpx_SleepTicks (uint32_t nSleepTime)
{
    delay (nSleepTime);
}

void Cpx_StackOverflowHandler ()
{
    while (!Serial)
        ;

    Serial.print (F ("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (Cpx_GetID ());
    Serial.println (F ("--------------------------------------"));
    ShowRunningThreads ();
    Serial.flush ();

    while (true) delay (1000);
}

void setup ()
{
    // Initialize serial and wait for port to open:
    Serial.begin (115200);

    while (!Serial)
    {
    };

    delay (1000);

    Serial.print ("CpxThread ");
    Serial.println (CpxVersion);
    Serial.println ("");

    Serial.println ("Starting up Thread....");
    Serial.flush ();
    Serial.flush ();

#if 1
    if (Cpx_StaticStart (pThreadContexts, sizeof (pThreadContexts)) == false)
    {
        Serial.println ("Fail to start CorePartition.");
        exit (0);
    }
#else
    if (Cpx_Start (5) == false)
    {
        Serial.println ("Fail to start CorePartition.");
        exit (0);
    }
#endif

    /* Producer Threads */
    assert (Cpx_CreateStaticThread (ProducerThread, (void*)1, pStaticCounter[0], sizeof (pStaticCounter[0]), 1));

    // assert (Cpx_CreateStaticThread (ProducerThread, (void*)1, pStaticCounter[1], sizeof (pStaticCounter[1]), 500));

    // assert (Cpx_CreateStaticThread (ProducerThread, (void*)1, pStaticCounter[2], sizeof (pStaticCounter[2]), 1000));

    /* Consumer Threads */
    assert (Cpx_CreateStaticThread (ConsumerThread, (void*)nValues, pStaticConsumer[0], sizeof (pStaticConsumer[0]), 100));
}

void loop ()
{
    Serial.print ("Joining");
    Cpx_Join ();
}
