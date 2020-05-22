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


void SetLocation (uint16_t nY, uint16_t nX)
{
    Serial.print ("\e[");
    Serial.print (nY);
    Serial.print (";");
    Serial.print (nX);
    Serial.print ("H");
}


//works with 256 colors
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
    Serial.print (F("\033[0m"));
}


void HideCursor ()
{
    Serial.print (F("\033[?25l"));
}


void ShowCursor ()
{
    Serial.print (F("\033[?25h"));
}


void ClearConsole ()
{
    Serial.print (F("\033[2J"));
}


void ReverseColor ()
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



void ShowRunningThreads ()
{
    size_t nThreadID = 0;
    
    Serial.println ();
    Serial.println (F("Listing all running threads"));
    Serial.println (F("--------------------------------------"));
    Serial.println (F("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime\tName"));
    
    for (nThreadID = 0; nThreadID < CorePartition_GetMaxNumberOfThreads (); nThreadID++)
    {
        Serial.print (F("\e[K"));
        if (CorePartition_GetStatusByID (nThreadID) != THREADL_NONE)
        {
            Serial.print (nThreadID);
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetStatusByID (nThreadID));
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetNiceByID (nThreadID));
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetStackSizeByID (nThreadID));
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetMaxStackSizeByID (nThreadID));
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetThreadContextSize ());
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetMaxStackSizeByID (nThreadID) + CorePartition_GetThreadContextSize ());
            Serial.print (F("\t"));
            Serial.print (CorePartition_GetLastDutyCycleByID (nThreadID));
            Serial.print ("ms\t\t");
            Serial.print (CorePartition_GetThreadNameByID (nThreadID));
        }
        Serial.println ("\e[0K");
    }
}



void CounterThread (void* pValue)
{
    uint32_t* pnValue = (uint32_t*) pValue;

    CorePartition_SetThreadName ("Counter", 7);

    while (true)
    {
        pnValue[0] ++;

        CorePartition_Yield ();
    }
}


void Thread (void* pValue)
{
    unsigned long nLast = millis();

    uint32_t* pnValues = (uint32_t*) pValue;

    CorePartition_SetThreadName ("Display", 7);

    while (1)
    {
        SetLocation (5,5);

        Serial.print (">>Thread");
        Serial.print (CorePartition_GetID()+1);
        Serial.print (": [");
        Serial.print (pnValues [0]);
        Serial.print (", ");
        Serial.print (pnValues [1]);
        Serial.print (", ");
        Serial.print (pnValues [2]);
        Serial.print ("], running: ");
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
        Serial.println ("ms\e[0k\n\n");
        
        SetLocation (8,1);
        ShowRunningThreads ();

        Serial.flush ();

        CorePartition_Yield ();
        
        if (CorePartition_GetStatusByID (4) == THREADL_NONE)
        {
            CorePartition_CreateThread (eventualThread, NULL, 25 * sizeof (size_t), 1000);
        }
            
    }
}


void eventualThread (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nLast = getTimeTick ();
    
    CorePartition_SetThreadName ("Eventual", 8);

    SetLocation (6,5);

    Serial.print (">> Eventual Thread");
    Serial.print (CorePartition_GetID());
    Serial.print (": Requested, Starting Up...");

    CorePartition_Yield ();

    while (nValue <= 5)
    {   
        SetLocation (6,5);

        Serial.print (">> Eventual Thread");
        Serial.print (CorePartition_GetID());
        Serial.print (": ");
        Serial.print (nValue++);
        Serial.print (F(", Sleep Time: "));
        Serial.print (CorePartition_GetLastMomentum () - nLast); 

        nLast = CorePartition_GetLastMomentum ();
        Serial.println (F("ms\e[0K\n"));

        CorePartition_Yield ();
    }

    SetLocation (6,5);
    ClearCurrentLine ();

    Serial.print (">> Eventual Thread");
    Serial.print (CorePartition_GetID());
    Serial.print (": Thread done!");
        
    CorePartition_Yield ();
}



static uint32_t getTimeTick()
{
   return (uint32_t) millis();
}


static void sleepTick (uint32_t nSleepTime)
{
    //delay (nSleepTime);
    delayMicroseconds  (nSleepTime > 0 ? nSleepTime * 1000 : 300);
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


uint32_t nValues [3] = { 0, 0, 0 };    


void setup()
{    
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
        
    while (!Serial);

    ResetColor ();
    ClearConsole ();
    HideCursor ();
    SetLocation (1,1);

    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();

    
    CorePartition_Start (5);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);
    

    CorePartition_CreateThread (CounterThread, &nValues [0], 25 * sizeof (size_t), 0);
    
    CorePartition_CreateThread (CounterThread, &nValues [1], 25 * sizeof (size_t), 200);

    CorePartition_CreateThread (CounterThread, &nValues [2], 25 * sizeof (size_t), 1000);

    CorePartition_CreateThread (Thread, nValues, 28 * sizeof (size_t), 250);
}


void loop()
{
    CorePartition_Join();
}
