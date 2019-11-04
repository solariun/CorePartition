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


uint32_t  nTime = 0;

void __attribute__ ((noinline)) setLocation (uint16_t nY, uint16_t nX)
{
    uint8_t szTemp [15];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%uH", nY, nX);

    Serial.write (szTemp, nLen);
}


//workis with 256 colors
void __attribute__ ((noinline)) setColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    byte szTemp [15];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%um", nFgColor + 30, nBgColor + 40);

    Serial.write (szTemp, nLen);
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



class ReadCommand
{
private:
    
    char* pszCommand;
    uint8_t nCommandSize;
    uint8_t nCmdLen;
    
    static bool boolRedraw;
    
    ReadCommand () = delete;

public:
    
    static void forceRedraw ()
    {
        ReadCommand::boolRedraw = true;
    }
    
    ReadCommand (uint8_t nMaxCommand) : nCommandSize (nMaxCommand), nCmdLen(0)
    {
        while ((pszCommand = new char [(nMaxCommand * sizeof (char)) + 1]) == NULL) delay (10);
        
        forceRedraw();
    }
    
    ~ReadCommand ()
    {
        delete [] pszCommand;
    }
    
    uint8_t getCommand ()
    {
        char chChar;
        uint8_t nStrLen = 0;
        
        if (pszCommand == NULL || nCommandSize < 1)
            return 0;

        pszCommand [0] = '\0';
        
        forceRedraw ();
        
        while (CorePartition_Yield() || Serial)
       {
           if (boolRedraw == true)
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
                       Serial.flush();
                       
                       nStrLen--;
                       pszCommand [nStrLen] = 0;
                   }
                   else if ((chChar >= 32 && chChar <= 127) && nStrLen < nCommandSize)
                   {
                       pszCommand [nStrLen++] = chChar;
                       pszCommand [nStrLen] = 0;
                   }
                   
                   forceRedraw ();
               }
           }
           else
           {
               boolRedraw = false;
           }
       }
        
        return 0;
    }
    
    const char* getStr()
    {
        return pszCommand;
    }
};

bool ReadCommand::boolRedraw = true;



void TraficLight ()
{
    pinMode (TraficLightData.nRedLightPin, OUTPUT);
    pinMode (TraficLightData.nYellowLightPin, OUTPUT);
    pinMode (TraficLightData.nGreenLightPin, OUTPUT);
    
    while (CorePartition_Yield ())
    {
        digitalWrite (TraficLightData.nRedLightPin, TraficLightData.boolRedLight == true ? HIGH : LOW);
        digitalWrite (TraficLightData.nYellowLightPin, TraficLightData.boolYellowLight == true ? HIGH : LOW);
        digitalWrite (TraficLightData.nGreenLightPin, TraficLightData.boolGreenLight == true ? HIGH : LOW);
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
            digitalWrite (TraficLightData.nWalkerGoPin, nBlink ? HIGH : LOW);
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
            
            digitalWrite (TraficLightData.nWalkerWaitPin, nBlink ? HIGH : LOW);
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
    Serial.println (F("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem"));
    
    for (nCount = 0; nCount < CorePartition_GetNumberOfThreads (); nCount++)
    {
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
        Serial.println (F(""));
        
        //CorePartition_Yield ();
    }
    
    Serial.println ();
}


/*
    Case pszText is NULL, it will return start pointer of the option
 
    nOption is the number of entended option.
 */
/*
const char* compareOption (uint16_t nOption, const char* pszCmds, uint8_t nCmdsLen, const char* pszText, uint8_t* pnTextLen)
{
    if (pszCmds == NULL || nCmdsLen == 0 || pnTextLen == NULL) return NULL;
    
    if (pszText != NULL && *pnTextLen == 0) return NULL;
    
    uint8_t nCount=0;
    
    for (nCount=0; nCount < nCmdsLen && nOption > 0; nCount++)
    {
        if (pszCmds [nCount] == ' ') nOption--;
    }
    
    if (nOption > 0) return NULL;
    
    if (pszText == NULL)
    {
        for (*pnTextLen=0; *pnTextLen < nCmdsLen && pszCmds [nCount + *pnTextLen] != ' '; *pnTextLen++);
    }
    
    return (pszCmds + nCount);
}

*/


void Terminal ()
{
    bool boolActive = false;
    int nCommandLen = 0;
    
    while (CorePartition_Yield() || true)
    {
        if (Serial)
        {
            ReadCommand readCommand (60);
            
            bool    boolPrintPrompt = true;
            
            
            resetColor();
            setLocation(1,1);
            clearConsole ();
            
            Serial.println (F("Trafic Light Manager v1.0"));
            Serial.println (F("By Gustavo Campos"));
            Serial.print   (F("Thread #"));
            Serial.println (CorePartition_GetID());
            
            ShowRunningThreads ();
            
            Serial.flush ();

            while (CorePartition_Yield() || Serial)
            {
                nCommandLen = readCommand.getCommand ();
                
                Serial.println ();
                Serial.flush ();
                
                if (nCommandLen == '\0')
                    continue;
                else if (strcmp ("show threads" , readCommand.getStr ()) == 0)
                {
                    ShowRunningThreads ();
                }
                
                Serial.flush ();
            }
        }
        else
        {
            Serial.println ("Turining Serial off...");
        }
    }
}

void __attribute__ ((noinline)) setTraficLights (bool boolRed, bool boolYellow, bool boolGreen)
{
    TraficLightData.boolRedLight = boolRed;
    TraficLightData.boolYellowLight = boolYellow;
    TraficLightData.boolGreenLight = boolGreen;
    
    TraficLightData.boolWalkerWait = ! boolRed;
    
    ReadCommand::forceRedraw ();
    
    nTime=0;
}

void  TraficLightKernel ()
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
        
        
        if (TraficLightData.boolGreenLight == true && nTime >= TraficLightData.nGreenTime)
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
    delayMicroseconds  (nSleepTime * 1000);
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
    bool status; 

    //Initialize serial and wait for port to open:
    Serial.begin(9600);

    //Terminal ();
    //exit(0);
    
    pinMode (13, OUTPUT);
    
    CorePartition_Start (4);


    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);

    CorePartition_CreateThread (TraficLight, 20 * sizeof (size_t), 100);
    
    CorePartition_CreateThread (WalkerSign, 30 * sizeof (size_t), 500);

    CorePartition_CreateThread (TraficLightKernel, 30 * sizeof (size_t), 250);
    
    CorePartition_CreateThread (Terminal, 35 * sizeof (size_t), 50);
}



void loop()
{
    CorePartition_Join();
}
