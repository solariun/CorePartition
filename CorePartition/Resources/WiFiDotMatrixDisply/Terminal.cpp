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
        if (IsConnected () == false)
        {
            return false;
        }

        CorePartition_Sleep (0);
    } while (m_client.available() == 0);

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
    //m_client.println (CorePartition_version);

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

    for (char chChar : commandLine)
    {
        if (currentState == state::NoText)
        {

            if (chChar == '"' || chChar == '\'')
            {
                //set Text state
                currentState = state::Text;
                nCommandOffSet++;
                continue;

            }
            else if (chChar != ' ')
            {
                //Set Word state
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
                }
            }
            else
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
                    returnText.push_back (chChar);
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
        //m_client.printf ("Read: (%u)-> [%c]\n\r", chChar, chChar >= 32 && chChar < 127 ? chChar : '.');

        if (chChar == TERMINAL_BS && readCommand.length () > 0)
        {
            m_client.print ((char) TERMINAL_BS);
            m_client.print (' ');
            m_client.print ((char) TERMINAL_BS);

            readCommand.pop_back ();
        }
        else if (chChar == '\r')
        {
            m_client.println ("");
            break;
        }
        else if (chChar >= 32 && chChar < 127)
        {
            m_client.print ((char) chChar);
            readCommand.push_back (chChar);
        }

        //m_client.print ("Read: ");
        //m_client.println (readCommand.c_str ());
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
            command->Run (*this, m_client, readCommand);

            return;
        }
    }

    m_client.println ("Error, command is invalid.");
}


bool Terminal::WaitForACommand()
{
    std::string readCommand = "";

    do 
    {        
        std::string commandItem = "";
        uint8_t nNumCommands;

        if (readCommand.length() > 0)
        {
            nNumCommands = ParseOption (readCommand, 0, commandItem);

            m_client.print ("Executing: (");
            m_client.print (nNumCommands);
            m_client.print ("),[");
            m_client.print (readCommand.c_str ());
            m_client.print ("],[");
            m_client.print (commandItem.c_str ());
            m_client.println ("]");

            if (commandItem == "exit")
            {
                break;
            }
            else
            {
                ExecCommand (readCommand);
            }
            
        }

        m_client.printf ("%s> ", m_promptString.c_str());

    } while (ReadCommandLine (readCommand));

    return false;
}


void Terminal::AssignCommand (Terminal::Command& terminalCommand)
{
    m_listAssignedCommands.push_back (&terminalCommand);
}