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


void __attribute__ ((noinline)) setLocation (uint16_t nY, uint16_t nX)
{
    uint8_t szTemp [15];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%uH", nY, nX);

    Serial.write (szTemp, nLen);
    Serial.flush();
}


//workis with 256 colors
void __attribute__ ((noinline)) setColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    byte szTemp [15];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%um", nFgColor + 30, nBgColor + 40);

    Serial.write (szTemp, nLen);
    Serial.flush();
}


void __attribute__ ((noinline)) resetColor ()
{
    Serial.print (F("\033[0m"));
}


void __attribute__ ((noinline)) hideCursor ()
{
    Serial.print (F("\033[?25l"));
}


void __attribute__ ((noinline)) showCursor ()
{
    Serial.print (F("\033[?25h"));
}


void __attribute__ ((noinline)) clearConsole ()
{
    Serial.print (F("\033[2J"));
}


void __attribute__ ((noinline)) reverseColor ()
{
    Serial.print (F("\033[7m"));
}


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        CorePartition_Yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}


void Thread (void* pValue)
{
    unsigned long nLast = millis();
    size_t nValue = 0;
    
    while (1)
    {
        Serial.print ("\e[");
        Serial.print ((CorePartition_GetID()*2) + 6);
        Serial.print (";10H\e[K>> Thread");
        Serial.print (CorePartition_GetID()+1);
        Serial.print (": ");
        Serial.print (nValue++);
        Serial.print (", running: ");
        Serial.print (CorePartition_GetNumberOfActiveThreads ());
        Serial.print (", Sleep Time: ");
        Serial.print ((unsigned long) CorePartition_GetLastMomentum () - nLast);  nLast = CorePartition_GetLastMomentum ();
        Serial.print ("ms, Nice: ");
        Serial.print (CorePartition_GetNice());
        Serial.print (", CTX: ");
        Serial.print (CorePartition_GetThreadContextSize ());
        Serial.print ("b, Stack: ");
        Serial.print (CorePartition_GetStackSize ());
        Serial.print ("/");
        Serial.print (CorePartition_GetMaxStackSize ());
        Serial.print (", DutyCycle Time: ");
        Serial.print (CorePartition_GetLastDutyCycle ());
        Serial.println ("ms\n");

    
        CorePartition_Yield ();
        
        if (CorePartition_GetStatusByID (3) == THREADL_NONE)
        {
            CorePartition_CreateThread (eventualThread, NULL, 25 * sizeof (size_t), 2000);
        }
            
    }
}

void eventualThread (void* pValue)
{
    int nValue = 0;
    unsigned long nLast = millis ();

    Serial.print ("\e[");
    Serial.print ((CorePartition_GetID()*2) + 6);
    Serial.print (";10H\e[K>> Eventual Thread");
    Serial.print (CorePartition_GetID()+1);
    Serial.print (": Requested, Starting Up...");

    while (nValue <= 5)
    {
        CorePartition_Yield ();
        
        Serial.print ("\e[");
        Serial.print ((CorePartition_GetID()*2) + 6);
        Serial.print (";10H\e[K>> Eventual Thread");
        Serial.print (CorePartition_GetID()+1);
        Serial.print (": ");
        Serial.print (++nValue);
        Serial.print (F(", Sleep Time: "));
        Serial.print ((uint32_t) CorePartition_GetLastMomentum () - nLast); nLast = CorePartition_GetLastMomentum ();
        Serial.println (F("ms\n"));

    }
    
    Serial.print ("\e[");
    Serial.print ((CorePartition_GetID()*2) + 6);
    Serial.print (";10H\e[K>> Eventual Thread");
    Serial.print (CorePartition_GetID()+1);
    Serial.print (": Thread done!");
        
    CorePartition_Yield ();
}


void __attribute__ ((noinline)) ShowRunningThreads ()
{
    size_t nCount = 0;
    
    Serial.println ();
    Serial.println (F("Listing all running threads"));
    Serial.println (F("--------------------------------------"));
    Serial.println (F("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));
    
    for (nCount = 0; nCount < CorePartition_GetNumberOfActiveThreads (); nCount++)
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
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetLastDutyCycleByID (nCount));
        Serial.println ("ms");
    }
}


static uint32_t getTimeTick()
{
   return (uint32_t) millis();
}


static void sleepTick (uint32_t nSleepTime)
{
    delay (nSleepTime);
    //delayMicroseconds  (nSleepTime * 1000);
}


void StackOverflowHandler ()
{
    while  (!Serial);
    
    Serial.print (F("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (CorePartition_GetID ());
    Serial.println (F("--------------------------------------"));
    ShowRunningThreads ();
    Serial.flush ();
}


void setup()
{    
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
        
    while (!Serial);
    
    delay (1000);

    resetColor ();
    clearConsole ();
    hideCursor ();
    setLocation (1,1);

    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();

    
    CorePartition_Start (4);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);
    
    CorePartition_CreateThread (Thread, NULL, 25 * sizeof (size_t), 500);
    
    CorePartition_CreateThread (Thread, NULL, 25 * sizeof (size_t), 1000);

    CorePartition_CreateThread (Thread, NULL, 25 * sizeof (size_t), 0);

}



void loop()
{
    CorePartition_Join();
}
