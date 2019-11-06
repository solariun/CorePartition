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


#pragma noinline

#include "CorePartition.h"

#include "Arduino.h"


struct 
{
    const uint8_t nRedLightPin = 0;
    const uint8_t nYellowLightPin = 1;
    const uint8_t nGreenLightPin = 2;
    
    const uint8_t nWalkerWaitPin = 3;
    const uint8_t nWalkerGoPin = 4;
    
    bool boolRedLight = false;
    bool boolYellowLight = false;
    bool boolGreenLight = true;
    bool boolWalkerWait = true;
    
    uint16_t nRedTime = 10;
    uint16_t nYellowTime = 6;
    uint16_t nGreenTime = 10;

    uint16_t nNotifyAtTime=5;
    
    bool boolAttention = false;
    
} TraficLightData;


uint32_t nTime = 0;



void TraficLight (void* pValue)
{
    uint8_t nBlink = 1;

    pinMode (TraficLightData.nRedLightPin, OUTPUT);
    pinMode (TraficLightData.nYellowLightPin, OUTPUT);
    pinMode (TraficLightData.nGreenLightPin, OUTPUT);
    
    while (CorePartition_Yield ())
    {
        nBlink = nBlink ^ 1;
        
            digitalWrite (TraficLightData.nRedLightPin, TraficLightData.boolRedLight == true ? HIGH : LOW);
            digitalWrite (TraficLightData.nYellowLightPin, TraficLightData.boolAttention == true ? nBlink ? HIGH : LOW : TraficLightData.boolYellowLight == true ? HIGH : LOW);
            digitalWrite (TraficLightData.nGreenLightPin, TraficLightData.boolGreenLight == true ? HIGH : LOW);
    }
}


void WalkerSign (void* pValue)
{
    uint8_t nBlink = 1;
    
    
    pinMode (TraficLightData.nWalkerWaitPin, OUTPUT);
    pinMode (TraficLightData.nWalkerGoPin, OUTPUT);
    
    while (CorePartition_Yield ())
    {
        if (TraficLightData.boolAttention == true)
        {
            nBlink = nBlink ^ 1;
            
            digitalWrite (TraficLightData.nWalkerWaitPin, LOW);
            digitalWrite (TraficLightData.nWalkerGoPin, nBlink ? HIGH : LOW);
        }
        else if (TraficLightData.boolRedLight == true)
        {
            if (TraficLightData.nRedTime < TraficLightData.nNotifyAtTime
                     || nTime >= (TraficLightData.nRedTime - TraficLightData.nNotifyAtTime))
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
            if (TraficLightData.boolYellowLight == true &&
                (TraficLightData.nYellowTime < TraficLightData.nNotifyAtTime
                || nTime >= (TraficLightData.nYellowTime - TraficLightData.nNotifyAtTime)))
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
    }
}


void __attribute__ ((noinline)) setTraficLights (bool boolRed, bool boolYellow, bool boolGreen)
{
    TraficLightData.boolRedLight = boolRed;
    TraficLightData.boolYellowLight = boolYellow;
    TraficLightData.boolGreenLight = boolGreen;
    
    TraficLightData.boolWalkerWait = ! boolRed;
    
    nTime=0;
}

void  TraficLightKernel (void* pValue)
{
    uint32_t nFactor = CorePartition_GetNice();
    uint32_t nTimeCounter = 0;
    
    nTime=0;
    
    while (CorePartition_Yield ())
    {
        nTimeCounter += nFactor;
        
        /* Not a beautiful code but will save
           some unecessary cycles calculating
           the secconds from mileseconds.
        */
        if (nTimeCounter > 1000)
        {
            nTime ++;
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
    }

}




static uint64_t getTimeTick()
{
   return (uint64_t) millis();
}

static void sleepTick (uint64_t nSleepTime)
{
    delayMicroseconds  ((nSleepTime + 1) * 1000);
}

void StackOverflowHandler ()
{
    size_t nThreadID = CorePartition_GetID() + 1;
    uint8_t nCount;
    
    pinMode (1, OUTPUT);
    
    while (1)
    {
        for (nCount=0; nCount < nThreadID; nCount++)
        {
            digitalWrite (1, HIGH);
            delay (150);
            digitalWrite (1, LOW);
            delay (150);
        }
        
        delay (400); //550ms off
    }
}

void setup()
{
    bool status; 

    CorePartition_Start (4);

    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);

    CorePartition_CreateThread (TraficLight, NULL, 10 * sizeof (size_t), 500);
    
    CorePartition_CreateThread (WalkerSign, NULL, 10 * sizeof (size_t), 500);

    CorePartition_CreateThread (TraficLightKernel, NULL, 10 * sizeof (size_t), 250);
}



void loop()
{
    CorePartition_Join();
}
