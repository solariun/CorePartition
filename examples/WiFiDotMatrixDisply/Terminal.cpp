///
/// @author   GUSTAVO CAMPOS

/// @date   28/05/2019 19:44
/// @version  <#version#>
///
/// @copyright  (c) GUSTAVO CAMPOS, 2019
/// @copyright  Licence
///
/*
    MIT License

    Copyright (c) 2021 Gustavo Campos

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/


#include "Arduino.h"

#include "Terminal.hpp"

Terminal::Terminal (Stream& streamClient) : m_client (streamClient), m_promptString{}
{
    m_promptString = "Terminal";
}

Terminal::~Terminal ()
{
}

void Terminal::SetPromptString (const std::string& promptString)
{
    m_promptString = promptString;
}

bool Terminal::WaitAvailableForReading ()
{
    Cpx_Sleep (10);

    do
    {
        if (IsConnected () == false)
        {
            return false;
        }

        Cpx_Sleep (10);
    } while (m_client.available () == 0);

    return true;
}

bool Terminal::IsConnected ()
{
    return true;
}

Stream& Terminal::GetStream ()
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
    // m_client.println (CpxVersion);

    return true;
}

uint8_t Terminal::ParseOption (const std::string& commandLine, uint8_t nCommandIndex, std::string& returnText, bool countOnly)
{
    uint8_t nCommandOffSet = 0;

    nCommandIndex++;

    enum class state
    {
        NoText,
        Word,
        Text,
    } currentState;

    currentState = state::NoText;
    returnText.clear ();
    bool boolScape = false;

    std::string strBuffer;

    for (char chChar : commandLine)
    {
        if (currentState == state::NoText)
        {
            if (chChar == '"' || chChar == '\'')
            {
                // set Text state
                currentState = state::Text;
                nCommandOffSet++;
                continue;
            }
            else if (chChar != ' ')
            {
                // Set Word state
                currentState = state::Word;
                nCommandOffSet++;
            }
        }

        if (currentState != state::NoText)
        {
            if (boolScape == false)
            {
                if (currentState == state::Text && chChar == '"' || chChar == '\'')
                {
                    currentState = state::NoText;
                }
                else if (currentState == state::Word && chChar == ' ')
                {
                    currentState = state::NoText;
                }
                if (chChar == '\\')
                {
                    boolScape = true;
                    strBuffer.clear ();
                    continue;
                }
            }
            else if (!isdigit (chChar))
            {
                boolScape = false;
            }

            if (currentState == state::NoText)
            {
                if (countOnly == false && nCommandIndex == nCommandOffSet)
                {
                    break;
                }

                returnText.clear ();
            }
            else
            {
                if (countOnly == false)
                {
                    if (boolScape == true and isdigit (chChar))
                    {
                        strBuffer += chChar;
                    }
                    else
                    {
                        // To add special char \000\ 00 = number only
                        if (strBuffer.length () > 0)
                        {
                            returnText.push_back (static_cast<char> (atoi (strBuffer.c_str ())));
                            strBuffer.clear ();
                        }
                        else
                        {
                            returnText.push_back (chChar); /* code */
                        }
                    }
                }
            }
        }
    }

    if (nCommandIndex != nCommandOffSet)
    {
        returnText.clear ();
    }

    return nCommandOffSet;
}

bool Terminal::ReadCommandLine (std::string& readCommand)
{
    uint8_t chChar = 0;

    readCommand.clear ();

    while (WaitAvailableForReading () == true)
    {
        m_client.readBytes (&chChar, 1);
        // m_client.printf ("Read: (%u)-> [%c]\n\r", chChar, chChar >= 32 && chChar < 127 ? chChar : '.');

        if (chChar == TERMINAL_BS && readCommand.length () > 0)
        {
            m_client.print ((char)TERMINAL_BS);
            m_client.print (' ');
            m_client.print ((char)TERMINAL_BS);

            readCommand.pop_back ();
        }
        else if (chChar == '\r')
        {
            m_client.println ("");
            break;
        }
        else if (chChar >= 32 && chChar < 127)
        {
            m_client.print ((char)chChar);
            readCommand.push_back (chChar);
        }

        // m_client.print ("Read: ");
        // m_client.println (readCommand.c_str ());
    }

    if (IsConnected () == false)
    {
        return false;
    }

    return true;
}

void Terminal::ExecCommand (const std::string readCommand)
{
    std::string strCommand;

    if (ParseOption (readCommand, 0, strCommand) == 0)
    {
        m_client.println ("Logic Error, command is empty.");
    }

    for (Terminal::Command* command : m_listAssignedCommands)
    {
        if (command->m_commandName == strCommand)
        {
            command->Run (*this, static_cast<Stream&> (m_client), readCommand);

            return;
        }
    }

    m_client.println ("Error, command is invalid.");
}

bool Terminal::WaitForACommand ()
{
    std::string readCommand = "";

    do
    {
        std::string commandItem = "";
        uint8_t nNumCommands;

        if (readCommand.length () > 0)
        {
            nNumCommands = ParseOption (readCommand, 0, commandItem);

            // m_client.print ("Executing: (");
            // m_client.print (nNumCommands);
            // m_client.print ("),[");
            // m_client.print (readCommand.c_str ());
            // m_client.print ("],[");
            // m_client.print (commandItem.c_str ());
            // m_client.println ("]");

            if (commandItem == "exit")
            {
                break;
            }
            else
            {
                ExecCommand (readCommand);
            }
        }

        m_client.printf ("%s> ", m_promptString.c_str ());

    } while (ReadCommandLine (readCommand));

    return false;
}

void Terminal::AssignCommand (Terminal::Command& terminalCommand)
{
    m_listAssignedCommands.push_back (&terminalCommand);
}