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

void SetLocation (uint16_t nY, uint16_t nX)
{
    Serial.print ("\e[");
    Serial.print (nY);
    Serial.print (";");
    Serial.print (nX);
    Serial.print ("H");
}

// works with 256 colors
void SetColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    Serial.print ("\e[");
    Serial.print (nFgColor + 30);
    Serial.print (";");
    Serial.print (nBgColor + 40);
    Serial.print ("m");
}

void ClearCurrentLine ()
{
    Serial.print ("\e[K");
}

void ResetColor ()
{
    Serial.print (F ("\033[0m"));
}

void HideCursor ()
{
    Serial.print (F ("\033[?25l"));
}

void ShowCursor ()
{
    Serial.print (F ("\033[?25h"));
}

void ClearConsole ()
{
    Serial.print (F ("\033[2J"));
}

void ReverseColor ()
{
    Serial.print (F ("\033[7m"));
}

void Delay (uint64_t nSleep)
{
    uint32_t nMomentum = millis ();

    // delay (nSleep); return;

    do
    {
        Cpx_Yield ();
    } while ((millis () - nMomentum) < nSleep);
}

void ShowRunningThreads ()
{
    size_t nThreadID = 0;

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
            Serial.print (Cpx_GetMaxStackSizeByID (nThreadID) + Cpx_GetStructContextSize ());
            Serial.print (F ("\t"));
            Serial.print (Cpx_GetLastDutyCycleByID (nThreadID));
            Serial.print ("ms");
        }
        Serial.println ("\e[0K");
    }
}

char pszNotificationTag[] = "thread/proc/value";

void CounterThread (void* pValue)
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

// Global context for async access
uint32_t nValues[3] = {0, 0, 0};

const char pszEventualTag[] = "eventual";

uint32_t WaitForData ()
{
    CpxMsgPayload payload = {0,0,0};

    if (Cpx_WaitMessage (pszEventualTag, sizeof (pszEventualTag) - 1, &payload) == false)
    {
        Serial.print ("    Error Waiting for messages.   ");
        Cpx_Yield ();
        return 0;
    }

    return payload.nValue;
}

void eventualThread (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nRemoteValue = 1;

    uint32_t nLast = Cpx_GetCurrentTick ();

    SetLocation (6, 5);

    Serial.print (">> Eventual Thread");
    Serial.print (Cpx_GetID ());
    Serial.print (": Requested, Starting Up...");

    Cpx_Yield ();

    while (nRemoteValue)
    {
        SetLocation (6, 5);

        Serial.print (">> Eventual Thread");
        Serial.print (Cpx_GetID ());
        Serial.print (": ");
        Serial.print (nValue++);
        Serial.print (": ");
        Serial.print (nRemoteValue++);
        Serial.print (F (", Sleep Time: "));
        Serial.print (Cpx_GetLastMomentum () - nLast);

        nLast = Cpx_GetLastMomentum ();
        Serial.println (F ("ms\e[0K\n"));

        nRemoteValue = WaitForData ();
    }

    SetLocation (6, 5);
    ClearCurrentLine ();

    Serial.print (">> Eventual Thread");
    Serial.print (Cpx_GetID ());
    Serial.print (": Thread done!");

    Cpx_Yield ();
}

CpxStaticThread nStaticStack [Cpx_GetStaticThreadSize (50 * sizeof (size_t))];

void Thread (void* pValue)
{
    Cpx_EnableBroker ((void*)nValues, 1, ThreadCounterMessageHandler);

    Cpx_SubscribeTopic (pszNotificationTag, sizeof (pszNotificationTag) - 1);

    unsigned long nLast = millis ();

    uint32_t* pnValues = (uint32_t*)pValue;

    int nCount = 0;
    int nFail = 0;

    while (1)
    {
        SetLocation (5, 5);

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

        SetLocation (10, 1);
        ShowRunningThreads ();

        Serial.flush ();

        Cpx_Yield ();

        if (Cpx_GetStatusByID (4) == THREADL_NONE)
        {
            Cpx_CreateStaticThread (eventualThread, NULL, nStaticStack, sizeof (nStaticStack), 100);
            nCount = 1;
            nFail = 0;
        }

        if (nCount)
        {
            if (nCount == 15)
            {
                nCount = 0;
                if (Cpx_NotifyMessageOne (pszEventualTag, sizeof (pszEventualTag) - 1, 0, 0) == false)
                {
                    nFail++;
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

    ResetColor ();
    ClearConsole ();
    HideCursor ();
    SetLocation (1, 1);

    Serial.print ("CpxThread ");
    Serial.println (CpxVersion);
    Serial.println ("");

    Serial.println ("Starting up Thread....");
    Serial.flush ();
    Serial.flush ();

    if (Cpx_Start (5) == false)
    {
        Serial.println ("Fail to start CorePartition.");
        exit (0);
    }

    assert (Cpx_CreateThread (CounterThread, (void*)1, 30 * sizeof (size_t), 1));

    assert (Cpx_CreateThread (CounterThread, (void*)1, 30 * sizeof (size_t), 500));

    assert (Cpx_CreateThread (CounterThread, (void*)1, 30 * sizeof (size_t), 1000));

    assert (Cpx_CreateThread (Thread, (void*)nValues, 50 * sizeof (size_t), 500));
}

void loop ()
{
    Cpx_Join ();
}
