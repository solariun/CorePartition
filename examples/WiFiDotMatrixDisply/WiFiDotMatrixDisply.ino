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
// Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//
// Preamble
//
// The GNU General Public License is a free, copyleft license for
// software and other kinds of works.
//
// The licenses for most software and other practical works are designed
// to take away your freedom to share and change the works.  By contrast,
// the GNU General Public License is intended to guarantee your freedom to
// share and change all versions of a program--to make sure it remains free
// software for all its users.  We, the Free Software Foundation, use the
// GNU General Public License for most of our software; it applies also to
// any other work released this way by its authors.  You can apply it to
// your programs, too.
//
// See LICENSE file for the complete information

#include <ESP8266WiFi.h>

// Designed for NodeMCU ESP8266
//################# DISPLAY CONNECTIONS ################
// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

#include "CorePartition.h"
#include "Terminal.hpp"

#include "Arduino.h"
#include "user_interface.h"

#include <LedControl.h>
#include <Wire.h>
#include <assert.h>
#include <string>

//#include "LedMatrix.cpp"

#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

#ifndef STASSID
#define STASSID "WhiteKingdom2.4Ghz"
#define STAPSK "Creative01000"
#endif

int DIN = D4;  // MISO - NodeMCU - D4 (TXD1)
int CS = D7;   // MOSI  - NodeMCU - D7 (HMOSI)
int CLK = D5;  // SS    - NodeMCU - D5 (HSCLK)

// Utilities

// Functions

#define MAX_LED_MATRIX 8
LedControl lc = LedControl (DIN, CLK, CS, MAX_LED_MATRIX);

void SetLocation (Stream& device, uint16_t nY, uint16_t nX)
{
    uint8_t szTemp[15];
    uint8_t nLen = snprintf ((char*)szTemp, sizeof (szTemp), "\033[%u;%uH", nY, nX);

    device.write (szTemp, nLen);
}

// workis with 256 colors
void SetColor (Stream& device, const uint8_t nFgColor, const uint8_t nBgColor)
{
    byte szTemp[15];
    uint8_t nLen = snprintf ((char*)szTemp, sizeof (szTemp), "\033[%u;%um", nFgColor + 30, nBgColor + 40);

    device.write (szTemp, nLen);
}

void LocalEcho (Stream& device, bool state)
{
    device.flush ();
    device.print ("\033[12");
    device.print (state ? "l" : "h");
    device.flush ();
}

void ResetColor (Stream& device)
{
    device.print ("\033[0m");
}

void HideCursor (Stream& device)
{
    device.print ("\033[?25l");
}

void ShowCursor (Stream& device)
{
    device.print ("\033[?25h");
}

void ClearConsole (Stream& device)
{
    device.print ("\033c\033[2J");
}

void ReverseColor (Stream& device)
{
    device.print ("\033[7m");
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

void ShowRunningThreads (Stream& client)
{
    size_t nCount = 0;

    client.println ();
    client.println (F ("Listing all running threads"));
    client.println (F ("--------------------------------------"));
    client.println (F ("ID\tName\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));

    for (nCount = 0; nCount < Cpx_GetNumberOfActiveThreads (); nCount++)
    {
        if (Cpx_GetStatusByID (nCount) > 0)
        {
            client.print (F ("\e[K"));
            client.print (nCount);
            client.printf ("\t%-8s", Cpx_GetThreadNameByID (nCount));
            client.print (F ("\t"));
            client.print (Cpx_GetStatusByID (nCount));
            client.print (Cpx_IsSecureByID (nCount));
            client.print (F ("\t"));
            client.print (Cpx_GetNiceByID (nCount));
            client.print (F ("\t"));
            client.print (Cpx_GetStackSizeByID (nCount));
            client.print (F ("\t"));
            client.print (Cpx_GetMaxStackSizeByID (nCount));
            client.print (F ("\t"));
            client.print (Cpx_GetThreadContextSize ());
            client.print (F ("\t"));
            client.print (Cpx_GetMaxStackSizeByID (nCount) + Cpx_GetThreadContextSize ());
            client.print (F ("\t"));
            client.print (Cpx_GetLastDutyCycleByID (nCount));
            client.println ("ms");
            client.flush ();
            Cpx_Yield ();
        }
    }
}

enum GRAPH_CHAR
{
    CHAR_FULL_SIGNAL = 127,
    CHAR_EMPTY_BAT,        // 128
    CHAR_BAT_1,            // 129
    CHAR_BAT_2,            // 130
    CHAR_BAT_3,            // 131
    CHAR_BAT_CHARG_1,      // 132
    CHAR_BAT_CHARG_2,      // 133
    CHAR_OK,               // 134
    CHAR_ARROW_DOWN,       // 135
    CHAR_ARROW_LEFT,       // 136
    CHAR_ARROW_UP,         // 137
    CHAR_ARROW_RIGHT,      // 138
    CHAR_ATTENTION,        // 139
    CHAR_WIFI,             // 140
    CHAR_WIFI_REV,         // 141
    CHAR_BELL,             // 142
    CHAR_CLOCK,            // 143
    CHAR_SUN,              // 144
    CHAR_CLOUD,            // 145
    CHAR_RAINING,          // 146
    CHAR_PARTIALLY_CLOUD,  // 147
    CHAR_UMBRELLA,         // 148
    CHAR_PIN,              // 149
    CHAR_SIGNAL_0,         // 150
    CHAR_SIGNAL_1,         // 151
    CHAR_SIGNAL_2,         // 152
    CHAR_SIGNAL_3,         // 153
    CHAR_SIGNAL_OK,        // 154
    CHAR_INFO,             // 155
    CHAR_NO,               // 156
    CHAR_NO_2,             // 157
    CHAR_NO_3,             // 158
    CHAR_OPEN_ENVELOP,     // 159
    CHAR_PAW,              // 160
    CHAR_CLOSE_ENVELOPE,   // 161
    CHAR_POWER,            // 162
    CHAR_ANIME_1,          // 163
    CHAR_ANIME_2,          // 164
    CHAR_ANIME_3,          // 165
    CHAR_ANIME_4,          // 166
    CHAR_ANIME_5,          // 167
    CHAR_ANIME_6,          // 168
    CHAR_ANIME_7,          // 169
    CHAR_ANIME_8,          // 170
    CHAR_ANIME_9,          // 171
};

const uint64_t byteImages[] PROGMEM = {
        0x0000000000000000, 0x00180018183c3c18, 0x0000000012246c6c, 0x0036367f367f3636, 0x000c1f301e033e0c, 0x0063660c18336300, 0x006e333b6e1c361c,
        0x0000000000030606, 0x00180c0606060c18, 0x00060c1818180c06, 0x0000663cff3c6600, 0x00000c0c3f0c0c00, 0x060c0c0000000000, 0x000000003f000000,
        0x000c0c0000000000, 0x000103060c183060, 0x003e676f7b73633e, 0x003f0c0c0c0c0e0c, 0x003f33061c30331e, 0x001e33301c30331e, 0x0078307f33363c38,
        0x001e3330301f033f, 0x001e33331f03061c, 0x000c0c0c1830333f, 0x001e33331e33331e, 0x000e18303e33331e, 0x000c0c00000c0c00, 0x060c0c00000c0c00,
        0x00180c0603060c18, 0x00003f00003f0000, 0x00060c1830180c06, 0x000c000c1830331e, 0x001e037b7b7b633e, 0x6666667e66663c00, 0x3e66663e66663e00,
        0x3c66060606663c00, 0x3e66666666663e00, 0x7e06063e06067e00, 0x0606063e06067e00, 0x3c66760606663c00, 0x6666667e66666600, 0x3c18181818183c00,
        0x1c36363030307800, 0x66361e0e1e366600, 0x7e06060606060600, 0xc6c6c6d6feeec600, 0xc6c6e6f6decec600, 0x3c66666666663c00, 0x06063e6666663e00,
        0x603c766666663c00, 0x66361e3e66663e00, 0x3c66603c06663c00, 0x18181818185a7e00, 0x7c66666666666600, 0x183c666666666600, 0xc6eefed6c6c6c600,
        0xc6c66c386cc6c600, 0x1818183c66666600, 0x7e060c1830607e00, 0x001e06060606061e, 0x00406030180c0603, 0x001e18181818181e, 0x0000000063361c08,
        0x003f000000000000, 0x0000000000180c06, 0x7c667c603c000000, 0x3e66663e06060600, 0x3c6606663c000000, 0x7c66667c60606000, 0x3c067e663c000000,
        0x0c0c3e0c0c6c3800, 0x3c607c66667c0000, 0x6666663e06060600, 0x3c18181800180000, 0x1c36363030003000, 0x66361e3666060600, 0x1818181818181800,
        0xd6d6feeec6000000, 0x6666667e3e000000, 0x3c6666663c000000, 0x06063e66663e0000, 0xf0b03c36363c0000, 0x060666663e000000, 0x3e603c067c000000,
        0x1818187e18180000, 0x7c66666666000000, 0x183c666600000000, 0x7cd6d6d6c6000000, 0x663c183c66000000, 0x3c607c6666000000, 0x3c0c18303c000000,
        0x00380c0c070c0c38, 0x0c0c0c0c0c0c0c0c, 0x00070c0c380c0c07, 0x0000000000003b6e, 0x5554545050404000, 0x3f21212121212121, 0x3f212d2121212121,
        0x3f212d212d212121, 0x3f212d212d212d21, 0x3f212d2d2d212121, 0x3f212d2d2d2d2d2d, 0x00040a1120408000, 0x081c3e7f1c1c1c1c, 0x0010307fff7f3010,
        0x1c1c1c1c7f3e1c08, 0x00080cfefffe0c08, 0x40e040181e0f0f07, 0x939398cc6730180f, 0xffa9a9b7d9eff1ff, 0x1800ff7a3a3a3c18, 0x3c428199b985423c,
        0x18423ca5a53c4218, 0x0000ff81864c3800, 0x1428ff81864c3800, 0x0000ff81864f3e05, 0x0000ff81864f3e05, 0x18141010fe7c3800, 0x08081c1c3e363e1c,
        0x00aa000000000000, 0x000a080000000000, 0x002a282000000000, 0x00aaa8a0840a1000, 0x3c46e7e7e3ff663c, 0x0082442810284482, 0x003844aa92aa4438,
        0x003864f2ba9e4c38, 0xff8199a5c3422418, 0x7e3cdbc383343624, 0xff8199a5c3ff0000, 0x3c66c39999db5a18, 0xff000001010000ff, 0xff000003030000ff,
        0xff000006060000ff, 0xff00000c0c0000ff, 0xff000018180000ff, 0xff000030300000ff, 0xff000060600000ff, 0xff0000c0c00000ff, 0xff000080800000ff};

const int byteImagesLen = sizeof (byteImages) / 8;

class TextScroller
{
private:
    int nNumberDigits = 15;
    uint8_t nOffset = 0;
    int nIndex = nNumberDigits * (-1);
    uint8_t nSpeed;

    uint64_t getLetter (int nIndex, const char* pszMessage, uint16_t nMessageLen)
    {
        int nCharacter = nIndex > nMessageLen || nIndex < 0 ? ' ' : pszMessage[nIndex];

        return getImage (nCharacter - ' ');
    }

    uint64_t getImage (int nIndex)
    {
        uint64_t nBuffer = 0xAA;

        nIndex = nIndex > byteImagesLen || nIndex < 0 ? 0 : nIndex;

        memcpy_P (&nBuffer, byteImages + nIndex, sizeof (uint64_t));

        return nBuffer;
    }

    void printScrollBytes (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, const uint64_t charLeft, const uint64_t charRight, uint8_t nOffset)
    {
        int i = 0;

        for (i = 0; i < 8; i++)
        {
            printRow (nLocY, nLocX, nDigit, i, (((uint8_t*)&charLeft)[i] << (8 - nOffset) | ((uint8_t*)&charRight)[i] >> nOffset));
        }

        // Cpx_Sleep (1);
    }

protected:
    virtual void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow)
    {
        static char nLine[9] = "        ";
        int8_t nOffset = 8;

        // Serial.print (nRow, BIN);

        while (--nOffset >= 0)
        {
            nLine[7 - nOffset] = nRow & 1 != 0 ? '#' : ' ';
            nRow >>= 1;
        }

        SetLocation (Serial, nLocY + nRowIndex, nLocX + (nDigit * 8));
        Serial.print (nLine);
    }

public:
    TextScroller (int nNumberDigits, uint8_t nSpeed) : nNumberDigits (nNumberDigits), nOffset (0), nSpeed (nSpeed)
    {
        nIndex = nSpeed == 0 ? 0 : nNumberDigits * (-1);
    }

    void setSpeed (uint8_t nSpeed)
    {
        this->nSpeed = nSpeed;
    }

    uint8_t getSpeed ()
    {
        return this->nSpeed;
    }

    bool show (int nLocY, int nLocX, const char* pszMessage, const uint16_t nMessageLen)
    {
        uint8_t nCount;

        if (nSpeed > 0 && nSpeed % 8 == 0) nSpeed++;

        if (nSpeed > 0) do
            {
                if (nOffset >= 7)
                {
                    nIndex = nIndex + 1 > (int)nMessageLen ? (int)nNumberDigits * (-1) : nIndex + 1;
                    nOffset = 0;

                    if ((int)nNumberDigits * (-1) == nIndex) return false;
                }
                else
                {
                    nOffset = (int)nOffset + (nSpeed % 8);
                }
            } while (nOffset >= 8);
        else
        {
            nOffset = 0;
            nIndex = 0;
        }

        for (nCount = 0; nCount < nNumberDigits; nCount++)
        {
            printScrollBytes (nLocY,
                              nLocX,
                              nCount,
                              getLetter (nIndex + 1, pszMessage, nMessageLen),
                              getLetter (nIndex, pszMessage, nMessageLen),
                              (uint8_t)nOffset);

            nIndex = (int)nIndex + 1;
        }

        nIndex = (int)nIndex - (nCount - (nSpeed / 8));

        return true;
    }

    void Reset ()
    {
        nOffset = 0;
        nIndex = 0;
    }
};

class MatrixTextScroller : public TextScroller
{
protected:
    void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow) override
    {
        lc.setRow (nDigit, 7 - nRowIndex, nRow);
    }

public:
    using TextScroller::TextScroller;
};

std::string strDisplay = "CorePartition Works!";

MatrixTextScroller matrixTextScroller (MAX_LED_MATRIX, 2);

const char* topicDisplay = "display";

#define MATRIX_DISPLAY_CHANGE 1

void LedDisplayBrokerHandler (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payload)
{
    if (strncmp (pszTopic, topicDisplay, sizeof (topicDisplay)-1) == 0)
    {
        switch (payload.nAttribute)
        {
            case MATRIX_DISPLAY_CHANGE:
                strDisplay = (const char*) payload.nValue;
                matrixTextScroller.Reset();
                break;
            
            default:
                break;
        }
    }
}


char chContext = 'A';

/// LedDisplayShow - Will update information o Display
/// @param pValue  Information injected from CorePartition on startup
void LedDisplayShow (void* pValue)
{
    unsigned long start = millis ();
    size_t nValue = 0;

    Cpx_EnableBroker ((void*) &chContext, 1, LedDisplayBrokerHandler);

    Cpx_SubscribeTopic (topicDisplay, sizeof (topicDisplay)-1);

    Cpx_SetThreadNameByID (Cpx_GetID (), "Display", 7);

    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t nOffset = 0;
    uint16_t nImagesItens = sizeof (byteImages) / sizeof (byteImages[0]);

    uint8_t nStep = 0;

    while (1)
    {
        matrixTextScroller.show (0, 0, strDisplay.c_str (), strDisplay.length ());
        Cpx_Yield ();
    }
}

#define MAX_SRV_CLIENTS 5
const char* ssid = STASSID;
const char* password = STAPSK;

const int port = 23;

template <class T>
class ReferenceClass
{
public:
    T ref;

    ReferenceClass (T reference) : ref (reference)
    {
    }
};

WiFiServer server (port);
WiFiClient serverClients[MAX_SRV_CLIENTS];

class WiFiCTerminal : public Terminal
{
public:
    using Terminal::Terminal;

    bool IsConnected ()
    {
        return ((WiFiClient&)GetStream ()).connected ();
    }
};

const char* GetWiFiStatus ()
{
    wl_status_t nStatus = WiFi.status ();

    switch (nStatus)
    {
        case WL_CONNECTED:
            return "CONNECTED";
        case WL_NO_SHIELD:
            return "NO WIFI DEVICE";
        case WL_IDLE_STATUS:
            return "IDLE";
        case WL_NO_SSID_AVAIL:
            return "NO SSID FOUND";
        case WL_SCAN_COMPLETED:
            return "SCAN COMPLETED";
        case WL_CONNECTION_LOST:
            return "CONNECTION LOST";
        case WL_DISCONNECTED:
            return "DISCONNECTED";
    }

    return "Unknown";
}

class CommandShow : public Terminal::Command
{
public:
    CommandShow ()
    {
        m_commandName = "show";
        m_commandDescription = "use show [threads|display|memory] to display information";
    }

    void Run (Terminal& terminal, Stream& client, const std::string commandLine) override
    {
        uint8_t nNumCommands;
        std::string strOption;

        if ((nNumCommands = terminal.ParseOption (commandLine, 1, strOption)) > 0)
        {
            if (nNumCommands > 2)
            {
                client.printf ("Warning: detected more options (%u) than necessary, aborting. \n", nNumCommands);
                client.println (m_commandDescription.c_str ());

                return;
            }

            if (strOption == "threads")
            {
                ShowRunningThreads (client);
            }
            else if (strOption == "display")
            {
                client.print ("Message: [");
                client.print (strDisplay.c_str ());
                client.println ("]");
            }
            else if (strOption == "memory")
            {
                client.print ("free heap: ");
                client.print (system_get_free_heap_size () / 1024);
                client.println ("Kb");
            }
            else if (strOption == "system")
            {
                client.println ("ESP8266 System ------------------");
                client.printf ("%-20s: [%u]\r\n", "Processor ID", system_get_chip_id ());
                client.printf ("%-20s: [%s]\r\n", "SDK Version", system_get_sdk_version ());
                client.printf ("%-20s: [%uMhz]\r\n", "CPU Freequency", system_get_cpu_freq ());
                client.printf ("%-20s: [%u Bytes]\r\n", "Memory", system_get_free_heap_size ());
                if (WiFi.status () != WL_NO_SHIELD)
                {
                    client.println ("-[Connection]----------------------");
                    client.printf ("%-20s: [%s] (%u)\r\n", "Status", GetWiFiStatus (), WiFi.status ());
                    client.printf ("%-20s: [%s]\r\n", "Mac Address", WiFi.macAddress ().c_str ());
                    if (WiFi.status () == WL_CONNECTED)
                    {
                        client.printf ("%-20s: [%s]\r\n", "SSID", WiFi.SSID ().c_str ());
                        client.printf ("%-20s: [%d dBm]\r\n", "Signal", WiFi.RSSI ());
                        client.printf ("%-20s: [%u Mhz]\r\n", "Channel", WiFi.channel ());
                        client.printf ("%-20s: [%s]\r\n", "IPAddress", WiFi.localIP ().toString ().c_str ());
                        client.printf ("%-20s: [%s]\r\n", "Net Mask", WiFi.subnetMask ().toString ().c_str ());
                        client.printf ("%-20s: [%s]\r\n", "Gateway", WiFi.gatewayIP ().toString ().c_str ());
                        client.printf ("%-20s: [%s]\r\n", "DNS", WiFi.dnsIP ().toString ().c_str ());
                    }
                }
                client.println ("-[Process]----------------------");
                ShowRunningThreads (client);
            }
            else
            {
                client.println ("Error, invalid option");
                client.println (m_commandDescription.c_str ());
            }
        }
    }
};

class CommandDisplay : public Terminal::Command
{
public:
    CommandDisplay ()
    {
        m_commandName = "display";
        m_commandDescription = "use show 'message to display' to display on Led display";
    }

    void Run (Terminal& terminal, Stream& client, const std::string commandLine) override
    {
        uint8_t nNumCommands;

        if ((nNumCommands = terminal.ParseOption (commandLine, 1, strDisplay)) > 0)
        {
            if (nNumCommands > 2)
            {
                client.printf ("Warning: detected more options (%u) than necessary, aborting. \n\r", nNumCommands);
                client.println (m_commandDescription.c_str ());

                return;
            }

            client.print ("Showing message: [");
            client.print (strDisplay.c_str ());
            client.println ("]");

            Cpx_PublishTopic (topicDisplay, sizeof (topicDisplay)-1, MATRIX_DISPLAY_CHANGE, (size_t) strDisplay.c_str());
        }
    }
};

void ClientHandler (void* pSrvClient)
{
    Cpx_SetThreadNameByID (Cpx_GetID (), "R_Client", 8);
    pSrvClient;

    WiFiClient client = server.available ();

    Serial.println ("Starting Client.....");

    WiFiCTerminal terminal ((Stream&)client);

    // Setting remote terminal to no echo
    client.printf ("%c%c%c", 255, 251, 1);
    client.printf ("%c%c%c", 255, 254, 1);
    client.printf ("%c%c%c", 255, 251, 3);
    client.printf ("%c%c%c", 255, 254, 3);

    terminal.ExecuteMOTD ();

    CommandDisplay commandDisplay;
    terminal.AssignCommand (commandDisplay);

    CommandShow commandShow;
    terminal.AssignCommand (commandShow);

    while (terminal.WaitForACommand () && client.connected ()) Cpx_Yield ();

    if (client.connected () == true)
    {
        client.stop ();
    }
}

/// Will Listen for New Clients coming in
/// @param pValue Information injected from CorePartition on startup
void TelnetListener (void* pValue)
{
    Cpx_SetThreadNameByID (Cpx_GetID (), "Listener", 8);
    WiFi.mode (WIFI_STA);

    WiFi.begin (ssid, password);

    Serial.print ("\nConnecting to ");
    Serial.println (ssid);

    strDisplay = "Connecting to ";
    strDisplay += ssid;
    Cpx_PublishTopic (topicDisplay, sizeof (topicDisplay)-1, MATRIX_DISPLAY_CHANGE, (size_t) strDisplay.c_str());

    std::string strValue;
    uint8_t nOffset = 0;
    bool bRev = false;

    uint8_t nSpeed = matrixTextScroller.getSpeed ();
    matrixTextScroller.setSpeed (0);

    while (WiFi.status () != WL_CONNECTED)
    {
        strDisplay = ((char)bRev ? CHAR_WIFI_REV : CHAR_WIFI);
        if (nOffset % 3 == 0) bRev = !bRev;

        strDisplay.push_back ((char)' ');

        strDisplay.push_back ((char)CHAR_ANIME_1 + nOffset);
        nOffset = nOffset < 8 ? nOffset + 1 : 0;

        Cpx_Sleep (100);
    }

    matrixTextScroller.setSpeed (nSpeed);

    strDisplay = (char)CHAR_OK;
    strDisplay += ': ';
    strDisplay += (char)CHAR_WIFI;
    strDisplay += ':';
    strDisplay += WiFi.localIP ().toString ().c_str ();
    Cpx_PublishTopic (topicDisplay, sizeof (topicDisplay)-1, MATRIX_DISPLAY_CHANGE, (size_t) strDisplay.c_str());

    // start server
    server.begin ();
    server.setNoDelay (true);

    Serial.print ("Ready! Use 'telnet ");
    Serial.print (WiFi.localIP ());
    Serial.printf (" %d' to connect\n", port);

    while (true)
    {
        if (server.hasClient ())
        {
            Serial.print ("Attending new client...");

            // Create a thread to start handling
            // otherwise send busy and close it
            if (Cpx_CreateThread (ClientHandler, (void*)nullptr, 512, 300) == false)
            {
                server.available ().println ("LedDisplay: Busy");

                // hints: server.available() is a WiFiClient with short-term scope
                // when out of scope, a WiFiClient will
                // - flush() - all data will be sent
                // - stop() - automatically too

                Serial.printf ("server is busy with %d active connections\n", MAX_SRV_CLIENTS);
            }
        }

        Cpx_Yield ();
    }
}

void SerialTerminalHandler (void* injection)
{
    Cpx_SetThreadNameByID (Cpx_GetID (), "Terminal", 8);

    Terminal serial (reinterpret_cast<Stream&> (Serial));

    CommandDisplay commandDisplay;
    serial.AssignCommand (commandDisplay);

    CommandShow commandShow;
    serial.AssignCommand (commandShow);

    while (true)
    {
        serial.ExecuteMOTD ();

        while (serial.WaitForACommand ()) Cpx_Yield ();
    }
}

/// Espcializing CorePartition Tick as Milleseconds
uint32_t Cpx_GetCurrentTick ()
{
    return (uint32_t)millis ();
}

/// Specializing CorePartition Idle time
/// @param nSleepTime How log the sleep will lest
void Cpx_SleepTicks (uint32_t nSleepTime)
{
    delay (nSleepTime);
}

/// Stack overflow Handler
void StackOverflowHandler ()
{
    while (!Serial)
        ;

    Serial.print (F ("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (Cpx_GetID ());
    Serial.println (F ("--------------------------------------"));
    ShowRunningThreads ((Stream&)(Serial));
    Serial.flush ();
    exit (0);
}

/// setup function from Arduino Standards
void setup ()
{
    bool status;

    // Initialize serial and wait for port to open:
    Serial.begin (115200);

    ResetColor (Serial);
    ClearConsole (Serial);
    SetLocation (Serial, 1, 1);
    ShowCursor (Serial);

    LocalEcho (Serial, false);

    Serial.print ("CoreThread ");
    Serial.println (Cpx_version);
    Serial.println ("");

    Serial.println ("Starting up Thread....");
    Serial.flush ();
    Serial.flush ();

    // Initializing displays 1 and 2

    uint8_t nCount;

    for (nCount = 0; nCount < MAX_LED_MATRIX; nCount++)
    {
        lc.shutdown (nCount, false);  // The MAX72XX is in power-saving mode on startup
        lc.setIntensity (nCount, 4);  // Set the brightness to maximum value
        lc.clearDisplay (nCount);     // and clear the display
    }

    for (nCount = 0; nCount < MAX_LED_MATRIX; nCount++)
    {
        lc.shutdown (nCount, false);  // The MAX72XX is in power-saving mode on startup
        lc.setIntensity (nCount, 4);  // Set the brightness to maximum value
        lc.clearDisplay (nCount);     // and clear the display
    }

    // Max threads on system.
    if (Cpx_Start (25) == false)
    {
        Serial.println ("Error staring up Threads.");
        exit (1);
    }

    assert (Cpx_SetStackOverflowHandler (StackOverflowHandler));

    assert (Cpx_CreateSecureThread (SerialTerminalHandler, NULL, 500, 200));
    assert (Cpx_CreateThread (LedDisplayShow, NULL, 300, 80));
    assert (Cpx_CreateThread (TelnetListener, NULL, 300, 500));
}

/// Main Loop for Arduino Standards
void loop ()
{
    Cpx_Join ();
}
