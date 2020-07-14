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

#include "CorePartition.h"
#include "Terminal.hpp"

#include "Arduino.h"
#include "user_interface.h"

#include <assert.h>
#include <string>

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
        CorePartition_Yield ();
    } while ((millis () - nMomentum) < nSleep);
}

void ShowRunningThreads (Stream& client)
{
    size_t nCount = 0;

    client.println ();
    client.println (F ("Listing all running threads"));
    client.println (F ("--------------------------------------"));
    client.println (F ("ID\tName\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));

    for (nCount = 0; nCount < CorePartition_GetNumberOfActiveThreads (); nCount++)
    {
        if (CorePartition_GetStatusByID (nCount) > 0)
        {
            client.print (F ("\e[K"));
            client.print (nCount);
            client.printf ("\t%-8s", CorePartition_GetThreadNameByID (nCount));
            client.print (F ("\t"));
            client.print (CorePartition_GetStatusByID (nCount));
            client.print (CorePartition_IsSecureByID (nCount));
            client.print (F ("\t"));
            client.print (CorePartition_GetNiceByID (nCount));
            client.print (F ("\t"));
            client.print (CorePartition_GetStackSizeByID (nCount));
            client.print (F ("\t"));
            client.print (CorePartition_GetMaxStackSizeByID (nCount));
            client.print (F ("\t"));
            client.print (CorePartition_GetThreadContextSize ());
            client.print (F ("\t"));
            client.print (CorePartition_GetMaxStackSizeByID (nCount) + CorePartition_GetThreadContextSize ());
            client.print (F ("\t"));
            client.print (CorePartition_GetLastDutyCycleByID (nCount));
            client.println ("ms");
            client.flush ();
            CorePartition_Yield ();
        }
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
    CommandShow () : Terminal::Command ("show")
    {
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

                return;
            }

            if (strOption == "threads")
            {
                ShowRunningThreads (client);
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
            }
        }
    }
};

class CommandDisplay : public Terminal::Command
{
public:
    CommandDisplay () : Terminal::Command ("display")
    {
    }

    void Run (Terminal& terminal, Stream& client, const std::string commandLine) override
    {
        uint8_t nNumCommands;
        std::string strDisplay;

        int nItens;

        client.printf ("Value: %d\n", nItens);

        for (int nCount = 1; (nItens = terminal.ParseOption (commandLine, nCount, strDisplay)) > 0; nCount++)
        {
            client.printf ("%d:  ret: %d - value: [%s]\n\r", nCount, nItens, strDisplay.c_str ());
        }
    }
};

void ClientHandler (void* pSrvClient)
{
    CorePartition_SetThreadNameByID (CorePartition_GetID (), "R_Client", 8);
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

    while (terminal.WaitForACommand () && client.connected ()) CorePartition_Yield ();

    if (client.connected () == true)
    {
        client.stop ();
    }
}

/// Will Listen for New Clients coming in
/// @param pValue Information injected from CorePartition on startup
void TelnetListener (void* pValue)
{
    CorePartition_SetThreadNameByID (CorePartition_GetID (), "Listener", 8);
    WiFi.mode (WIFI_STA);

    WiFi.begin (ssid, password);

    Serial.print ("\nConnecting to ");
    Serial.println (ssid);

    std::string strValue;
    uint8_t nOffset = 0;
    bool bRev = false;

    while (WiFi.status () != WL_CONNECTED)
    {
        Serial.println ("Connecing...");
        CorePartition_Sleep (100);
    }

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
            if (CorePartition_CreateThread (ClientHandler, (void*)nullptr, 512, 300) == false)
            {
                server.available ().println ("LedDisplay: Busy");

                // hints: server.available() is a WiFiClient with short-term scope
                // when out of scope, a WiFiClient will
                // - flush() - all data will be sent
                // - stop() - automatically too

                Serial.printf ("server is busy with %d active connections\n", MAX_SRV_CLIENTS);
            }
        }

        CorePartition_Yield ();
    }
}

void SerialTerminalHandler (void* injection)
{
    CorePartition_SetThreadNameByID (CorePartition_GetID (), "Terminal", 8);

    Terminal serial (reinterpret_cast<Stream&> (Serial));

    CommandDisplay commandDisplay;
    serial.AssignCommand (commandDisplay);

    CommandShow commandShow;
    serial.AssignCommand (commandShow);

    while (true)
    {
        serial.ExecuteMOTD ();

        while (serial.WaitForACommand ()) CorePartition_Yield ();
    }
}

/// Espcializing CorePartition Tick as Milleseconds
uint32_t CorePartition_GetCurrentTick ()
{
    return (uint32_t)millis ();
}

/// Specializing CorePartition Idle time
/// @param nSleepTime How log the sleep will lest
void CorePartition_SleepTicks (uint32_t nSleepTime)
{
    delay (nSleepTime);
}

/// Stack overflow Handler
void StackOverflowHandler ()
{
    while (!Serial)
        ;

    Serial.print (F ("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (CorePartition_GetID ());
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
    Serial.println (CorePartition_version);
    Serial.println ("");

    Serial.println ("Starting up Thread....");
    Serial.flush ();
    Serial.flush ();

    // Initializing displays 1 and 2

    uint8_t nCount;

    // Max threads on system.
    if (CorePartition_Start (25) == false)
    {
        Serial.println ("Error staring up Threads.");
        exit (1);
    }

    assert (CorePartition_SetStackOverflowHandler (StackOverflowHandler));

    assert (CorePartition_CreateSecureThread (SerialTerminalHandler, NULL, 500, 200));
    assert (CorePartition_CreateThread (TelnetListener, NULL, 300, 500));
}

/// Main Loop for Arduino Standards
void loop ()
{
    CorePartition_Join ();
}
