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

#include "Arduino.h"

#include "Terminal.hpp"

Terminal::Terminal (Stream& streamClient) : m_client (streamClient), m_promptString {}
{
    m_promptString = "Terminal";
}

Terminal::~Terminal ()
{} 


void Terminal::SetPromptString (const std::string& promptString)
{
    m_promptString = promptString;
}

bool Terminal::WaitAvailableForReading ()
{
    CorePartition_Sleep (0);
    
    do
    {
        if (isConnected () == false)
        {
            return false;
        }

        CorePartition_Sleep (0);
    } while (m_client.available() == 0);

    return true;
}

bool Terminal::isConnected ()
{
    return true;
}


Stream& Terminal::getStream ()
{
   return m_client;
}

bool Terminal::ExecuteMOTD ()
{
    m_client.println ("*****************************************");
    m_client.println ("* WiFi Dot Matrix OS                    *");
    m_client.println ("*                                       *");
    m_client.println ("* Welcome to the future                 *");
    m_client.println ("*****************************************");
    //m_client.println (CorePartition_version);

    return true;
}


bool Terminal::ReadCommand (std::string& readCommand)
{
    uint8_t szChar = 0;

    while (WaitAvailableForReading () == true)
    {
        m_client.readBytes (&szChar, 1);

        m_client.printf ("Read: (%u)-> [%c]\n\r", szChar, szChar);
    }

    if (isConnected () == false)
    {
        return false;
    }

    return true;
}

bool Terminal::WaitForACommand()
{
    std::string readCommand = "";

    do 
    {
        if (readCommand.length() > 0)
        {
            m_client.print ("Command: ");
            m_client.println (readCommand.c_str());
        }

        m_client.printf ("%s> ", m_promptString.c_str());

    } while (ReadCommand (readCommand));

    return false;
}
