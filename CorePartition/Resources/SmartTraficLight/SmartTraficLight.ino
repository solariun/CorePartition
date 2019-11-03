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
    const uint8_t nRedLightPin = 2;
    const uint8_t nYellowLightPin = 3;
    const uint8_t nGreenLightPin = 4;
    
    const uint8_t nWalkerWaitPin = 5;
    const uint8_t nWalkerGoPin = 6;
    
    bool boolRedLight = false;
    bool boolYellowLight = false;
    bool boolGreenLight = true;
    bool boolWalkerWait = true;

    uint16_t nRedTime = 10;
    uint16_t nYellowTime = 6;
    uint16_t nGreenTime = 10;

    uint16_t nNotifyAtTime=5;
    
} TraficLightData;

bool boolAction = false;
float nTime = 0;

void __attribute__ ((noinline)) setLocation (uint16_t nY, uint16_t nX)
{
    uint8_t szTemp [10];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%uH", nY, nX);

    Serial.write (szTemp, nLen);
    Serial.flush();
}


//workis with 256 colors
void __attribute__ ((noinline)) setColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    byte szTemp [10];
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


void Delay (uint32_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        CorePartition_Yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}



void TraficLight ()
{
    
    pinMode (TraficLightData.nRedLightPin, OUTPUT);
    pinMode (TraficLightData.nYellowLightPin, OUTPUT);
    pinMode (TraficLightData.nGreenLightPin, OUTPUT);
    
    while (CorePartition_Yield ())
    {
        digitalWrite (TraficLightData.nRedLightPin, TraficLightData.boolRedLight);
        digitalWrite (TraficLightData.nYellowLightPin, TraficLightData.boolYellowLight);
        digitalWrite (TraficLightData.nGreenLightPin, TraficLightData.boolGreenLight);
    }
}


void WalkerSign ()
{
    bool nBlink = true;
    
    pinMode (TraficLightData.nWalkerWaitPin, OUTPUT);
    pinMode (TraficLightData.nWalkerGoPin, OUTPUT);
    
    while (CorePartition_Yield ())
    {
        if (TraficLightData.boolRedLight == true)
        {
            if (TraficLightData.nRedTime < TraficLightData.nNotifyAtTime
                     || nTime >= (TraficLightData.nRedTime - TraficLightData.nNotifyAtTime))
            {
                nBlink ^= 1;
            }
            else
            {
                nBlink = HIGH;
            }
            
            digitalWrite (TraficLightData.nWalkerWaitPin, LOW);
            digitalWrite (TraficLightData.nWalkerGoPin, nBlink);
        }
        else
        {
            if (TraficLightData.boolYellowLight == true &&
                (TraficLightData.nYellowTime < TraficLightData.nNotifyAtTime
                || nTime >= (TraficLightData.nYellowTime - TraficLightData.nNotifyAtTime)))
            {
                nBlink ^= 1;
            }
            else
            {
                nBlink = HIGH;
            }
            
            digitalWrite (TraficLightData.nWalkerWaitPin, nBlink);
            digitalWrite (TraficLightData.nWalkerGoPin, LOW);
        }
    }

}


void ShowRunningThreads ()
{
    size_t nCount = 0;
    
    Serial.println ();
    Serial.println (F("Listing all running threads"));
    Serial.println (F("--------------------------------------"));
    Serial.println (F("ID\tStatus\tNice\tStk/Max"));
    
    for (nCount = 0; nCount < CorePartition_GetNumberOfThreads (); nCount++)
    {
        Serial.print (nCount);
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStatusByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetNiceByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStackSizeByID (nCount));
        Serial.print (F("/"));
        Serial.println (CorePartition_GetMaxStackSizeByID (nCount));
        
        CorePartition_Yield ();
    }
    
    Serial.println ();
}


int GetPromptCommand (char * pszCommand, uint16_t nCommandSize)
{
    char chChar;
    bool boolRedraw = true;
    uint16_t nStrLen = strlen (pszCommand);
    
    
    if (pszCommand == NULL || nCommandSize < 1)
        return -1;
    
    while (CorePartition_Yield() || Serial)
   {
       if (boolRedraw == true || boolAction == true)
       {
           hideCursor ();
           Serial.print (F("\r\033[0m[\033["));
           
           Serial.print (TraficLightData.boolRedLight ? 91 :
                         TraficLightData.boolYellowLight ? 93 :
                         TraficLightData.boolGreenLight ? 92 : 0);
           
           Serial.print (F(";40m "));

           Serial.print (TraficLightData.boolRedLight ? F("RD") :
                         TraficLightData.boolYellowLight ? F("YE") :
                         TraficLightData.boolGreenLight ? F("GR") : F("--"));
           
           Serial.print (F(" \033[0m]\033[92mTraficLigth \033[94m>\033[0m "));
           Serial.print (pszCommand);
           showCursor ();
           Serial.flush ();
           
           boolRedraw = false;
       }
       
       if (Serial.available () > 0)
       {
           while (Serial.available () > 0)
           {
               chChar = Serial.read ();
               
               if (chChar == 0xD) return nStrLen;
               
               //Serial.print (F("Read: "));
               //Serial.print (chChar, HEX);
               //Serial.print (F(","));
               //Serial.println (chChar, DEC);
               
               if (chChar == 0x8 && nStrLen > 0)
               {
                   Serial.print ((char) 8);
                   Serial.print ((char) 32);
                   Serial.print ((char) 8);
                   nStrLen--;
                   pszCommand [nStrLen] = 0;
               }
               else if ((chChar >= 32 && chChar <= 127) && nStrLen < nCommandSize)
               {
                   pszCommand [nStrLen++] = chChar;
                   pszCommand [nStrLen] = 0;
               }
               
               boolRedraw = true;
           }
       }
       
       boolAction = false;
   }
}


void Terminal ()
{
    bool boolActive = false;

    
    while (CorePartition_Yield() || true)
    {
        if (Serial)
        {
            char pszLineBuffer [81]= "show threads";
            bool    boolPrintPrompt = true;
            
            setLocation(1,1);
            resetColor();
            clearConsole ();
            
            Serial.println (F("Trafic Light Manager v1.0"));
            Serial.println (F("By Gustavo Campos"));
            Serial.println ();
            
            ShowRunningThreads ();
            
            Serial.flush ();

            while (CorePartition_Yield() || Serial)
            {
                GetPromptCommand (pszLineBuffer, sizeof (pszLineBuffer));
                Serial.println ();
                Serial.flush ();
             
                if (strcmp ("show threads", pszLineBuffer) == 0)
                {
                    ShowRunningThreads ();
                }
            }
        }
        else
        {
            Serial.println ("Turining Serial off...");
        }
    }
}


void TraficLightKernel ()
{
    float nFactor = (float) CorePartition_GetNice() / 1000;
    
    while (CorePartition_Yield ())
    {
        nTime += nFactor;
        
        if (TraficLightData.boolGreenLight == true)
        {
            if (nTime >= TraficLightData.nGreenTime)
            {
                TraficLightData.boolRedLight = false;
                TraficLightData.boolYellowLight = true;
                TraficLightData.boolGreenLight = false;
                
                TraficLightData.boolWalkerWait = true;
                
                nTime=0;
                boolAction = true;
            }
        }
        else if (TraficLightData.boolYellowLight == true)
        {
            if (nTime >= TraficLightData.nYellowTime)
            {
                TraficLightData.boolRedLight = true;
                TraficLightData.boolYellowLight = false;
                TraficLightData.boolGreenLight = false;
                
                nTime=0;
                boolAction = true;
            }
        }
        else if (TraficLightData.boolRedLight == true)
        {
            if (nTime >= TraficLightData.nRedTime)
            {
                TraficLightData.boolRedLight = false;
                TraficLightData.boolYellowLight = false;
                TraficLightData.boolGreenLight = true;
                
                nTime=0;
                boolAction = true;
            }
        }
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


void setup()
{
    bool status; 

    //Initialize serial and wait for port to open:
    Serial.begin(9600);

    //Terminal ();
    //exit(0);
    
    CorePartition_Start (4);


    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);


    CorePartition_CreateThread (TraficLight, 50, 100);
    
    CorePartition_CreateThread (WalkerSign, 50, 500);

    CorePartition_CreateThread (TraficLightKernel, 100, 250);
    
    CorePartition_CreateThread (Terminal, 200, 100);
}



void loop()
{
    CorePartition_Join();
}
