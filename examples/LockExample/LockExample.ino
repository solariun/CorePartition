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

bool Lock = false;

void Cpx_LockPreemption ()
{
    Lock = 1;
}

CpxSmartLock lock;

int nProducers [7];

void Producer(void* pValue)
{
    (void) pValue;

    size_t nID = Cpx_GetID();
    
    nProducers [nID] = 0;
    
    do
    {
        Cpx_SharedLock(&lock);
        
        nProducers [nID]++;
        
        Cpx_Sleep(0);
        
        Cpx_SharedUnlock(&lock);

         
    } while (Cpx_Yield());

}

void Consumer(void* pValue)
{
    unsigned int nCount = 0;
        
    (void) pValue;

    do
    {
        Cpx_Lock(&lock);
        
        Serial.print (F ("Thread "));
        Serial.print (Cpx_GetID());
        Serial.print (F (": Values -> "));

        for (nCount=0; nCount < (sizeof (nProducers)/sizeof (int));nCount++)
        {
            Serial.print (F ("("));
            Serial.print (nCount);
            Serial.print (F (": ["));
            Serial.print (nProducers [nCount]);
            Serial.print (F ("]) "));
        }

        Serial.println ("");
        
        Serial.flush ();

        Cpx_Sleep(1);
        
        Cpx_Unlock(&lock);
        
        
    } while (Cpx_Yield());
}

void ShowRunningThreads ()
{
    size_t nThreadID = 0;

    Serial.println ();
    Serial.println (F ("Listing all running threads"));
    Serial.println (F ("--------------------------------------"));
    Serial.println (F ("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime\tName"));

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
            Serial.print (Cpx_GetThreadContextSize ());
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetMaxStackSizeByID (nThreadID) + Cpx_GetThreadContextSize ());
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetLastDutyCycleByID (nThreadID));
            Serial.print ("ms\t\t");
            Serial.print (Cpx_GetThreadNameByID (nThreadID));
        }
        Serial.println ("\e[0K");
    }
}

// uint32_t Cpx_GetCurrentTick ()
// {
//     return (uint32_t)millis ();
// }

// void Cpx_SleepTicks (uint32_t nSleepTime)
// {
//     delay (nSleepTime);
// }

void Cpx_StackOverflowHandler ()
{
    while (!Serial)
        ;

    Serial.println (F ("----------------------------------------"));

    Serial.print (F ("[ERROR] - Stack Overflow - Thread #"));
    Serial.print (Cpx_GetID());
    Serial.print (F (" Used: "));
    Serial.print (Cpx_GetStackSize ());
    Serial.print (F (", from Max: "));
    Serial.print (Cpx_GetMaxStackSize ());
    Serial.println ("");

    Serial.println (F ("----------------------------------------"));

    while (true) delay (1000);
}

#ifdef __AVR__
#define STACK_PRODUCER sizeof(size_t) * 12
#define STACK_CONSUMER sizeof(size_t) * 37
#else
#define STACK_PRODUCER sizeof(size_t) * 40
#define STACK_CONSUMER sizeof(size_t) * 60
#endif

void setup ()
{
    // Initialize serial and wait for port to open:
    Serial.begin (115200);

    while (!Serial)
    {
    };

    delay (1000);

    Serial.print ("CoreThread ");
    Serial.println (Cpx_version);
    Serial.println ("");

    Serial.println ("Starting up Thread....");
    Serial.flush ();

    if (Cpx_Start (10) == false)
    {
        Serial.println ("Fail to start CorePartition.");
        exit (0);
    }

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 0));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 700));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 700));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 800));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 500));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 1000));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 60000));

    /* --------------------------------------------------------------------- */

    assert (Cpx_CreateThread (Consumer, NULL, STACK_CONSUMER, 1000));

    assert (Cpx_CreateThread (Consumer, NULL, STACK_CONSUMER, 500));

    assert (Cpx_CreateThread (Consumer, NULL, STACK_CONSUMER, 2000));

}

void loop ()
{
    Cpx_Join ();
}