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



#include "CorePartition.h"

#include "Arduino.h"


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        CorePartition_Yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}



void Thread1 (void* pValue)
{
    uint8_t nPin = CorePartition_GetID() + 1;
    
    pinMode (nPin, OUTPUT);
    
    while (1)
    {
        digitalWrite (nPin, HIGH);
        
        CorePartition_Yield ();

        digitalWrite (nPin, LOW);

        CorePartition_Yield ();
    }

}



static uint64_t getTimeTick()
{
   return (uint64_t) millis();
}

static void sleepTick (uint64_t nSleepTime)
{
    delayMicroseconds  (nSleepTime * 1000);
}

void StackOverflowHandler ()
{
    size_t nThreadID = CorePartition_GetID() + 1;
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
    
    for (nCount = 0; nCount < CorePartition_GetNumberOfThreads (); nCount++)
    {
        Serial.print (F("\e[K"));
        Serial.print (nCount);
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStatusByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetNiceByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetMaxStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetThreadContextSize ());
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetMaxStackSizeByID (nCount) + CorePartition_GetThreadContextSize ());
        Serial.println ();
    }
}


void WhachDog (void* pValue)
{
    Serial.begin(230400);
    
    while (CorePartition_Yield ())
    {
        ShowRunningThreads ();
    }
}
#endif


void setup()
{
    bool status; 
    
    //Initialize serial and wait for port to open:

#ifdef _DEBUG
    CorePartition_Start (5);
#else
    CorePartition_Start (4);
#endif

    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);

    CorePartition_CreateThread (Thread1, NULL, 20, 50);
    
    CorePartition_CreateThread (Thread1, NULL, 20, 1000);

    CorePartition_CreateThread (Thread1, NULL, 20, 812);

    CorePartition_CreateThread (Thread1, NULL, 20, 200);

#ifdef _DEBUG
    CorePartition_CreateThread (WhachDog, 20, 200);
#endif
    
}



void loop()
{
    CorePartition_Join();
}
