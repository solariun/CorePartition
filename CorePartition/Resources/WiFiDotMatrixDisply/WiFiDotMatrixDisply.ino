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

#include <ESP8266WiFi.h>



//Designed for NodeMCU ESP8266
//################# DISPLAY CONNECTIONS ################
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)



#include "CorePartition.h"

#include "Arduino.h"

#include <Wire.h>
#include <LedControl.h>
#include <string>

#include "Terminal.hpp"

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#ifndef STASSID
#define STASSID "WhiteKingdom2.4Ghz"
#define STAPSK  "Creative01000"
#endif

int DIN = D4; //MISO - NodeMCU - D4 (TXD1)
int CS =  D7; //MOSI  - NodeMCU - D7 (HMOSI)
int CLK = D5; //SS    - NodeMCU - D5 (HSCLK)


// Utilities


// Functions

#define MAX_LED_MATRIX 4
LedControl lc=LedControl(DIN,CLK,CS, MAX_LED_MATRIX);


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


void resetColor ()
{
    Serial.print ("\033[0m");
}


void hideCursor ()
{
    Serial.print ("\033[?25l");
}


void showCursor ()
{
    Serial.print ("\033[?25h");
}


void clearConsole ()
{
    Serial.print ("\033[2J"); 
}


void reverseColor ()
{
    Serial.print ("\033[7m");   
}


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        CorePartition_Yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}



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
        Serial.print (CorePartition_IsSecureByID (nCount));
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



const uint64_t byteImages[] PROGMEM =
{
    0x0000000000000000, 0x00180018183c3c18, 0x0000000012246c6c, 0x0036367f367f3636, 0x000c1f301e033e0c,
    0x0063660c18336300, 0x006e333b6e1c361c, 0x0000000000030606, 0x00180c0606060c18, 0x00060c1818180c06,
    0x0000663cff3c6600, 0x00000c0c3f0c0c00, 0x060c0c0000000000, 0x000000003f000000, 0x000c0c0000000000,
    0x000103060c183060, 0x003e676f7b73633e, 0x003f0c0c0c0c0e0c, 0x003f33061c30331e, 0x001e33301c30331e,
    0x0078307f33363c38, 0x001e3330301f033f, 0x001e33331f03061c, 0x000c0c0c1830333f, 0x001e33331e33331e,
    0x000e18303e33331e, 0x000c0c00000c0c00, 0x060c0c00000c0c00, 0x00180c0603060c18, 0x00003f00003f0000,
    0x00060c1830180c06, 0x000c000c1830331e, 0x001e037b7b7b633e, 0x6666667e66663c00, 0x3e66663e66663e00,
    0x3c66060606663c00, 0x3e66666666663e00, 0x7e06063e06067e00, 0x0606063e06067e00, 0x3c66760606663c00,
    0x6666667e66666600, 0x3c18181818183c00, 0x1c36363030307800, 0x66361e0e1e366600, 0x7e06060606060600,
    0xc6c6c6d6feeec600, 0xc6c6e6f6decec600, 0x3c66666666663c00, 0x06063e6666663e00, 0x603c766666663c00,
    0x66361e3e66663e00, 0x3c66603c06663c00, 0x18181818185a7e00, 0x7c66666666666600, 0x183c666666666600,
    0xc6eefed6c6c6c600, 0xc6c66c386cc6c600, 0x1818183c66666600, 0x7e060c1830607e00, 0x001e06060606061e,
    0x00406030180c0603, 0x001e18181818181e, 0x0000000063361c08, 0x003f000000000000, 0x0000000000180c06,
    0x7c667c603c000000, 0x3e66663e06060600, 0x3c6606663c000000, 0x7c66667c60606000, 0x3c067e663c000000,
    0x0c0c3e0c0c6c3800, 0x3c607c66667c0000, 0x6666663e06060600, 0x3c18181800180000, 0x1c36363030003000,
    0x66361e3666060600, 0x1818181818181800, 0xd6d6feeec6000000, 0x6666667e3e000000, 0x3c6666663c000000,
    0x06063e66663e0000, 0xf0b03c36363c0000, 0x060666663e000000, 0x3e603c067c000000, 0x1818187e18180000,
    0x7c66666666000000, 0x183c666600000000, 0x7cd6d6d6c6000000, 0x663c183c66000000, 0x3c607c6666000000,
    0x3c0c18303c000000, 0x00380c0c070c0c38, 0x0c0c0c0c0c0c0c0c, 0x00070c0c380c0c07, 0x0000000000003b6e
 };

const int byteImagesLen = sizeof(byteImages)/8;

      
class TextScroller
{
private:
    
     int nNumberDigits = 15;
     uint8_t nOffset = 0;
     int  nIndex = nNumberDigits * (-1);
     uint8_t nSpeed;
    
    uint64_t getLetter (int nIndex, const char* pszMessage, uint16_t nMessageLen)
    {
        int nCharacter = nIndex > nMessageLen || nIndex < 0 ? ' ' : pszMessage [nIndex];
        
        return getImage (nCharacter - ' ');
    }

    uint64_t getImage (int nIndex)
    {
        uint64_t nBuffer= 0xAA;

        nIndex = nIndex > byteImagesLen ||  nIndex < 0  ? 0 : nIndex;

        memcpy_P (&nBuffer, byteImages + nIndex, sizeof (uint64_t));
        
        return nBuffer;
    }

    void printScrollBytes(uint16_t  nLocY, uint16_t nLocX, uint16_t nDigit, const uint64_t charLeft, const uint64_t charRight, uint8_t nOffset)
    {
        int i = 0;
          
        for(i=0;i<8;i++)
        {
            printRow (nLocY, nLocX, nDigit, i, (((uint8_t*) &charLeft) [i] << (8-nOffset) | ((uint8_t*) &charRight) [i] >> nOffset));
        }
        
        //CorePartition_Sleep (1);
    }

    
protected:
    
    virtual void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow)
    {
        static char nLine [9] = "        ";
        int8_t nOffset = 8;

        //Serial.print (nRow, BIN);
        
        while (--nOffset >= 0)
        {
           nLine [7-nOffset] = nRow & 1 != 0 ? '#' : ' ';
           nRow >>= 1;
        }

        setLocation (nLocY + nRowIndex, nLocX + (nDigit * 8));
        Serial.print (nLine);
    }

public:
    
    TextScroller (int nNumberDigits, uint8_t nSpeed) :
        nNumberDigits (nNumberDigits),
        nOffset (0),
        nSpeed (nSpeed)
    {
       nIndex = nSpeed == 0 ? 0 : nNumberDigits * (-1);
    }
    
    
    bool show (int nLocY, int nLocX, const char* pszMessage, const uint16_t nMessageLen)
    {
        uint8_t nCount;
       
        if (nSpeed > 0 && nSpeed % 8 == 0) nSpeed++;
        
        if (nSpeed > 0) do
        {
          if (nOffset >= 7)
          {
              nIndex = nIndex + 1 > (int) nMessageLen ? (int) nNumberDigits * (-1) : nIndex + 1;
              nOffset = 0;
              
              if ((int) nNumberDigits * (-1) == nIndex) return false;
          }
          else
          {
              nOffset = (int) nOffset + (nSpeed  % 8);
          }
        } while (nOffset >= 8);

              
        for (nCount=0; nCount < nNumberDigits; nCount++)
        {
            printScrollBytes (nLocY, nLocX, nCount, getLetter(nIndex + 1, pszMessage, nMessageLen), getLetter(nIndex, pszMessage, nMessageLen), (uint8_t) nOffset);
          
            nIndex =  (int) nIndex + 1;
        }
          
        nIndex = (int) nIndex - (nCount - (nSpeed / 8));
        
        return true;
    }
};


class MatrixTextScroller : public TextScroller 
{
protected:

    void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow) override
    {
        lc.setRow(nDigit, 7-nRowIndex, nRow);
    }

public:

    using TextScroller::TextScroller;
};



std::string strDisplay = "CorePartition Works!";

/// LedDisplayShow - Will update information o Display
/// @param pValue  Information injected from CorePartition on startup
void LedDisplayShow (void* pValue)
{
    unsigned long start = millis();
    size_t nValue = 0;
    
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t nOffset = 0;
    uint16_t nImagesItens = sizeof (byteImages) / sizeof (byteImages[0]);
    //setCoreNice (100);

    MatrixTextScroller matrixTextScroller (4, 2);
    
    uint8_t nStep = 0;
    
    while (1)
    {
            matrixTextScroller.show (0, 0, strDisplay.c_str(), strDisplay.length ());
        
        CorePartition_Yield ();
    }
}


#define MAX_SRV_CLIENTS 5
const char* ssid = STASSID;
const char* password = STAPSK;

const int port = 23;

template <class T> class ReferenceClass
{
public:
    T ref;
    
    ReferenceClass(T reference) : ref (reference) {}
};


WiFiServer server(port);
WiFiClient serverClients[MAX_SRV_CLIENTS];

class WiFiCTerminal : public Terminal
{
    public:
    
    using Terminal::Terminal;

    bool isConnected ()
    {
        return ((WiFiClient&) getStream ()).connected ();
    }
};


void ClientHandler (void* pSrvClient)
{
    pSrvClient;
    
    WiFiClient client = server.available ();
    
    Serial.println ("Starting Client.....");
        
    WiFiCTerminal terminal (client);

    terminal.ExecuteMOTD ();

    while (terminal.WaitForACommand () && client.connected ())
        CorePartition_Yield ();
    
    client.println ("Bye Bye");

    Serial.println ("");
    Serial.printf ("Desconecting %s\n\r", client.remoteIP ().toString ().c_str ());
}



/// Will Listen for New Clients coming in
/// @param pValue Information injected from CorePartition on startup
void TelnetListner (void* pValue)
{
    WiFi.mode(WIFI_STA);
    
    WiFi.begin(ssid, password);
    
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    
    strDisplay = "Connecting to ";
    strDisplay += ssid;
    
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print (".");
        CorePartition_Sleep(500);
    }
    
    strDisplay = "connected, address=";
    strDisplay += WiFi.localIP().toString().c_str();
    strDisplay += ", Waiting connection...";
    

    //start server
    server.begin();
    server.setNoDelay(true);
    
    Serial.print("Ready! Use 'telnet ");
    Serial.print(WiFi.localIP());
    Serial.printf(" %d' to connect\n", port);
    
    
    while (CorePartition_Yield ())
    {
        if (server.hasClient())
        {
            Serial.print   ("Attending new client...");
            
            //Create a thread to start handling
            //otherwise send busy and close it
            if (CorePartition_CreateThread (ClientHandler, (void*) nullptr, 512, 200) == false)
            {
                server.available().println("LedDisplay: Busy");
                
                // hints: server.available() is a WiFiClient with short-term scope
                // when out of scope, a WiFiClient will
                // - flush() - all data will be sent
                // - stop() - automatically too

                Serial.printf("server is busy with %d active connections\n", MAX_SRV_CLIENTS);
            }
        }
    }
}


void SerialTerminalHandler (void* injection)
{
    Terminal serial (Serial);

    while (CorePartition_Yield ())
    {
        serial.ExecuteMOTD ();

        while (serial.WaitForACommand ())
            CorePartition_Yield ();
    }
}


/// Espcializing CorePartition Tick as Milleseconds
uint32_t getTimeTick()
{
   return (uint32_t) millis();
}


/// Especializaing CorePartition Idle time
/// @param nSleepTime How log the sleep will lest
void sleepTick (uint32_t nSleepTime)
{
    delay (nSleepTime);
}


/// Stack overflow Handler
void StackOverflowHandler ()
{
    while  (!Serial);
    
    Serial.print (F("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (CorePartition_GetID ());
    Serial.println (F("--------------------------------------"));
    ShowRunningThreads ();
    Serial.flush ();
    exit(0);
}


/// setup function from Arduino Standards
void setup()
{
    bool status; 
    
    //Initialize serial and wait for port to open:
    Serial.begin(115200);

    resetColor ();
    clearConsole ();
    setLocation (1,1);
    hideCursor ();
    
    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();

    
    
    //Initializing displays 1 and 2
    
    uint8_t nCount;
    
    for (nCount=0; nCount < MAX_LED_MATRIX; nCount++)
    {
        lc.shutdown(nCount,false);       //The MAX72XX is in power-saving mode on startup
        lc.setIntensity(nCount,4);      // Set the brightness to maximum value
        lc.clearDisplay(nCount);         // and clear the display
    }

    CorePartition_Start (8);
    

    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);

    //CorePartition_CreateSecureThread  (SerialTerminalHandler, NULL, 500, 200);
    CorePartition_CreateThread (LedDisplayShow, NULL, 300, 50);
    CorePartition_CreateThread (TelnetListner, NULL, 300, 500);
}



/// Main Loop for Arduino Standards
void loop()
{
    CorePartition_Join();
}
