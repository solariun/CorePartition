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


#include "CorePartition.h"

#include "Arduino.h"

#include <assert.h>

void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;

    do {
        Cpx_Yield();
    } while ((millis() - nMomentum ) <  nSleep);
}

void Thread1 (void* pValue)
{
    uint8_t nPin = Cpx_GetID() + 1;

    pinMode (nPin, OUTPUT);

    while (1)
    {
        digitalWrite (nPin, HIGH);

        Cpx_Yield ();

        digitalWrite (nPin, LOW);

        Cpx_Yield ();
    }

}

static uint32_t Cpx_GetCurrentTick()
{
   return (uint32_t) millis();
}

void Cpx_SleepTicks (uint32_t nSleepTime)
{
    delay (nSleepTime);
}

void Cpx_StackOverflowHandler ()
{
    size_t nThreadID = Cpx_GetID() + 1;
    uint8_t nCount;

    pinMode (nThreadID, OUTPUT);

    while (1)
    {
        for (nCount=0; nCount < nThreadID; nCount++)
        {
            digitalWrite (nThreadID, HIGH);
            delay (150);
            digitalWrite (nThreadID, LOW);
            delay (150);
        }

        delay (400); //550ms off
    }
}

//#define  _DEBUG

#ifdef _DEBUG
void __attribute__ ((noinline)) ShowRunningThreads ()
{
    size_t nCount = 0;

    Serial.println ();
    Serial.println (F("Listing all running threads"));
    Serial.println (F("--------------------------------------"));
    Serial.println (F("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));

    for (nCount = 0; nCount < Cpx_GetNumberOfActiveThreads (); nCount++)
    {
        Serial.print (F("\e[K"));
        Serial.print (nCount);
        Serial.print (F("\t"));
        Serial.print (Cpx_GetStatusByID (nCount));
        Serial.print (F("\t"));
        Serial.print (Cpx_GetNiceByID (nCount));
        Serial.print (F("\t"));
        Serial.print (Cpx_GetStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (Cpx_GetMaxStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (Cpx_GetStructContextSize ());
        Serial.print (F("\t"));
        Serial.print (Cpx_GetMaxStackSizeByID (nCount) + Cpx_GetStructContextSize ());
        Serial.println ();
    }
}

void WhachDog (void* pValue)
{
    Serial.begin(230400);

    while (true)
    {
        ShowRunningThreads ();
        Cpx_Yield ();
    }
}
#endif

void setup()
{

    pinMode (0, OUTPUT);

    digitalWrite (0, HIGH);
    delay (250);
    digitalWrite (0, HIGH);
    delay(250);

    digitalWrite (0, HIGH);
    delay (250);
    digitalWrite(0, HIGH);
    delay (250);

    digitalWrite (0, HIGH);
    delay (250);
    digitalWrite(0, HIGH);
    delay(250);

#ifdef _DEBUG
    assert (Cpx_Start (5));
#else
    assert (Cpx_Start (4));
#endif

    assert (Cpx_CreateThread (Thread1, NULL, 25, 50));

    assert (Cpx_CreateThread (Thread1, NULL, 25, 1000));

    assert (Cpx_CreateThread (Thread1, NULL, 25, 812));

    assert (Cpx_CreateThread (Thread1, NULL, 25, 200));

#ifdef _DEBUG
    assert (Cpx_CreateThread (WhachDog, 20, 200));
#endif
}

void loop()
{
    Cpx_Join();
}
