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

#pragma noinline

#include "CorePartition.h"

#include "Arduino.h"

#include <assert.h>

struct
{
    const uint8_t nRedLightPin    = 0;
    const uint8_t nYellowLightPin = 1;
    const uint8_t nGreenLightPin  = 2;

    const uint8_t nWalkerWaitPin = 3;
    const uint8_t nWalkerGoPin   = 4;

    bool boolRedLight    = false;
    bool boolYellowLight = false;
    bool boolGreenLight  = true;
    bool boolWalkerWait  = true;

    uint16_t nRedTime    = 10;
    uint16_t nYellowTime = 6;
    uint16_t nGreenTime  = 10;

    uint16_t nNotifyAtTime = 5;

    bool boolAttention = false;

} TraficLightData;


uint32_t nTime = 0;


void TraficLight (void* pValue)
{
    uint8_t nBlink = 1;

    pinMode (TraficLightData.nRedLightPin, OUTPUT);
    pinMode (TraficLightData.nYellowLightPin, OUTPUT);
    pinMode (TraficLightData.nGreenLightPin, OUTPUT);

    while (true)
    {
        nBlink = nBlink ^ 1;

        digitalWrite (TraficLightData.nRedLightPin, TraficLightData.boolRedLight == true ? HIGH : LOW);
        digitalWrite (
                TraficLightData.nYellowLightPin,
                TraficLightData.boolAttention == true ? nBlink ? HIGH : LOW : TraficLightData.boolYellowLight == true ? HIGH : LOW);
        digitalWrite (TraficLightData.nGreenLightPin, TraficLightData.boolGreenLight == true ? HIGH : LOW);

        Cpx_Yield ();
    }
}


void WalkerSign (void* pValue)
{
    uint8_t nBlink = 1;


    pinMode (TraficLightData.nWalkerWaitPin, OUTPUT);
    pinMode (TraficLightData.nWalkerGoPin, OUTPUT);

    while (true)
    {
        if (TraficLightData.boolAttention == true)
        {
            nBlink = nBlink ^ 1;

            digitalWrite (TraficLightData.nWalkerWaitPin, LOW);
            digitalWrite (TraficLightData.nWalkerGoPin, nBlink ? HIGH : LOW);
        }
        else if (TraficLightData.boolRedLight == true)
        {
            if (TraficLightData.nRedTime < TraficLightData.nNotifyAtTime || nTime >= (TraficLightData.nRedTime - TraficLightData.nNotifyAtTime))
            {
                nBlink = nBlink ^ 1;
            }
            else
            {
                nBlink = HIGH;
            }

            digitalWrite (TraficLightData.nWalkerWaitPin, LOW);
            digitalWrite (TraficLightData.nWalkerGoPin, nBlink ? HIGH : LOW);
        }
        else
        {
            if (TraficLightData.boolYellowLight == true && (TraficLightData.nYellowTime < TraficLightData.nNotifyAtTime ||
                                                            nTime >= (TraficLightData.nYellowTime - TraficLightData.nNotifyAtTime)))
            {
                nBlink = nBlink ^ 1;
            }
            else
            {
                nBlink = HIGH;
            }

            digitalWrite (TraficLightData.nWalkerWaitPin, nBlink ? HIGH : LOW);
            digitalWrite (TraficLightData.nWalkerGoPin, LOW);
        }

        Cpx_Yield ();
    }
}


void __attribute__ ((noinline)) setTraficLights (bool boolRed, bool boolYellow, bool boolGreen)
{
    TraficLightData.boolRedLight    = boolRed;
    TraficLightData.boolYellowLight = boolYellow;
    TraficLightData.boolGreenLight  = boolGreen;

    TraficLightData.boolWalkerWait = !boolRed;

    nTime = 0;
}

void TraficLightKernel (void* pValue)
{
    uint32_t nFactor      = Cpx_GetNice ();
    uint32_t nTimeCounter = 0;

    nTime = 0;

    while (true)
    {
        nTimeCounter += nFactor;

        /* Not a beautiful code but will save
           some unecessary cycles calculating
           the secconds from mileseconds.
        */
        if (nTimeCounter > 1000)
        {
            nTime++;
            nTimeCounter = 0;
        }

        if (TraficLightData.boolAttention == true)
        {
            setTraficLights (false, true, false);
        }
        else if (TraficLightData.boolGreenLight == true && nTime >= TraficLightData.nGreenTime)
        {
            setTraficLights (false, true, false);
        }
        else if (TraficLightData.boolYellowLight == true && nTime >= TraficLightData.nYellowTime)
        {
            setTraficLights (true, false, false);
        }
        else if (TraficLightData.boolRedLight == true && nTime >= TraficLightData.nRedTime)
        {
            setTraficLights (false, false, true);
        }

        Cpx_Yield ();
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
    size_t nThreadID = Cpx_GetID () + 1;
    uint8_t nCount;

    pinMode (1, OUTPUT);

    while (1)
    {
        for (nCount = 0; nCount < nThreadID; nCount++)
        {
            digitalWrite (1, HIGH);
            delay (150);
            digitalWrite (1, LOW);
            delay (150);
        }

        delay (400);  // 550ms off
    }
}

void setup ()
{
    bool status;

    assert (Cpx_Start (4));

    assert (Cpx_CreateThread (TraficLight, NULL, 10 * sizeof (size_t), 500));

    assert (Cpx_CreateThread (WalkerSign, NULL, 10 * sizeof (size_t), 500));

    assert (Cpx_CreateThread (TraficLightKernel, NULL, 10 * sizeof (size_t), 250));
}


void loop ()
{
    Cpx_Join ();
}
