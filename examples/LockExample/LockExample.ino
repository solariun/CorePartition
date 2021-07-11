///
/// @author   GUSTAVO CAMPOS
/// @author   GUSTAVO CAMPOS
/// @date   28/05/2019 19:44
/// @version  <#version#>
///
/// @copyright  (c) GUSTAVO CAMPOS, 2019
/// @copyright  Licence
///
/*
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


#include <assert.h>
#include "Arduino.h"
#include "CorePartition.h"

#define THREAD_VALUES_ATTRB 1

bool Lock = false;

void ShowRunningThreads ()
{
    size_t nThreadID = 0;

    Serial.println ();
    Serial.println (F ("Listing all running threads"));
    Serial.println (F ("--------------------------------------"));
    Serial.println (F ("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime\tName"));

    for (nThreadID = 0; nThreadID < Cpx_GetMaxNumberOfThreads (); nThreadID++)
    {
        
        //Serial.print (F ("\e[K"));

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
            Serial.print (Cpx_GetMaxStackSizeByID (nThreadID) + Cpx_GetStructContextSize ());
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetLastDutyCycleByID (nThreadID));
            Serial.print ("ms");
        }
        Serial.println ("");
    }
}

CpxSmartLock lock;

int nProducers[6];

void Producer (void* pValue)
{
    (void)pValue;

    size_t nID = Cpx_GetID ();

    nProducers[nID] = 0;

    do
    {
        Cpx_SharedLock (&lock);

        nProducers[nID]++;

        Cpx_Sleep (0);

        Cpx_SharedUnlock (&lock);

    } while (Cpx_Yield ());
}

void Consumer_print ()
{
        unsigned int nCount = 0;

        Serial.print (F ("Thread "));
        Serial.print (Cpx_GetID ());
        Serial.print (F (": Values -> "));

        for (nCount = 0; nCount < (sizeof (nProducers) / sizeof (int)); nCount++)
        {
            Serial.print (F ("("));
            Serial.print (nCount);
            Serial.print (F (": ["));
            Serial.print (nProducers[nCount]);
            Serial.print (F ("]"));
            Serial.print (F (") "));
            yield ();
        }

        Serial.print (", ");
        Serial.println (Cpx_GetStackSize ());

        Serial.flush ();
}

void Consumer (void* pValue)
{
    (void)pValue;

    do
    {
        Cpx_Lock (&lock);

        Consumer_print ();

        Cpx_Sleep (0);
        
        Cpx_Unlock (&lock);

    } while (Cpx_Yield ());
}

uint32_t Cpx_GetCurrentTick ()
{
    yield(); 
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

    Serial.println (F ("----------------------------------------"));

    Serial.print (F ("[ERROR] - Stack Overflow - Thread #"));
    Serial.print (Cpx_GetID ());
    Serial.print (F (" Used: "));
    Serial.print (Cpx_GetStackSize ());
    Serial.print (F (", from Max: "));
    Serial.print (Cpx_GetMaxStackSize ());
    Serial.println ("");

    Serial.println (F ("----------------------------------------"));

    while (true) { delay (1000); yield (); }
}

#ifdef __AVR__
#define STACK_PRODUCER sizeof (size_t) * 20
#define STACK_CONSUMER sizeof (size_t) * 40
#else
#define STACK_PRODUCER sizeof (size_t) * 40
#define STACK_CONSUMER sizeof (size_t) * 60
#endif


void ShowThreads (void* nValue)
{
    (void) nValue;

    while (Cpx_Yield ())
    {
        Serial.println ("");
        Serial.println ("....PING....");
    }
}


CpxThread* ppThreadList [10];

uint8_t nStaticContext [3][Cpx_GetStaticThreadSize (STACK_CONSUMER)];

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

    if (Cpx_StaticStart (ppThreadList, sizeof (ppThreadList)) == false)
    {
        Serial.println ("Fail to start CorePartition.");
        exit (0);
    }
    

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 0));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 700));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 700));


    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 500));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 1000));

    assert (Cpx_CreateThread (Producer, NULL, STACK_PRODUCER, 60000));

    assert (Cpx_CreateThread (ShowThreads, NULL, STACK_PRODUCER, 1000));

    /* --------------------------------------------------------------------- */

    assert (Cpx_CreateStaticThread (Consumer, NULL, (CpxStaticThread*) nStaticContext[0], sizeof (nStaticContext [0]), 1000));

    assert (Cpx_CreateStaticThread (Consumer, NULL, (CpxStaticThread*) nStaticContext[1], sizeof (nStaticContext [1]), 1400));
    
    assert (Cpx_CreateStaticThread (Consumer, NULL, (CpxStaticThread*) nStaticContext[2], sizeof (nStaticContext [2]), 5000));

    Cpx_Join ();

    Serial.println ("Error...");

    ShowRunningThreads ();

}

void loop ()
{
}
