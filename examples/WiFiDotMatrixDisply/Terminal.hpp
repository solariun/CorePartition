///
/// @author   GUSTAVO CAMPOS
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


#include <list>
#include <string>
#include "CorePartition.h"
#include "Stream.h"

#define TERMINAL_BS 8

class Terminal
{
public:
    class Command
    {
    public:
        std::string m_commandName;
        std::string m_commandDescription;

        virtual void Run (Terminal& terminal, Stream& client, const std::string commandLine) = 0;
    };

    /**
     * @brief Construct a new Terminal object
     *
     * @param streamDev  Stream object for in and out procedures
     */
    Terminal (Stream& streamDev);

    /**
     * @brief Destroy the Terminal object
     *
     */
    virtual ~Terminal ();

    /**
     * @brief Specialized isConnected for special devices like WiFi
     *
     * @return true if it is conencted
     */
    virtual bool IsConnected ();

    /**
     * @brief Specialize Greetings message
     *
     * @return true if it was OK.
     */
    virtual bool ExecuteMOTD ();

    /**
     * @brief Wait until a command is entered
     *
     * @return false in case of disconnection or I/O fail
     */
    bool WaitForACommand ();

    /**
     * @brief Set the Prompt String
     *
     * @param promptString Prompt to be used
     */
    void SetPromptString (const std::string& promptString);

    /**
     * @brief Assign a command to be used with Terminal
     *
     * @param commandFunction       Function call back to execute the command
     * @param commandName           Name of the commando to be used by the terminal
     * @param commandDescription    Description of the Command to be shown on Help
     */

    void AssignCommand (Terminal::Command& terminalCommand);

    /**
     * @brief Get the Option string from a specific Index
     *
     * @param commandLine       The read command line from prompt
     * @param nCommandIndex     The index of command starting from 0
     * @param returnText        Return Text from the CommandLine
     * @param countOnly         will only count and return the total itens
     *
     * @return uint8_t  return the index number, otherwise return
     *                  0 for the error.
     *                  if you ask 5 it will return
     */
    uint8_t ParseOption (const std::string& commandLine, uint8_t nCommandIndex, std::string& returnText, bool countOnly = false);

protected:
    friend class TerminalCommand;

    /**
     * @brief read a line from Stream
     *
     * @param readCommand return the read command line
     *
     * @return false in disconnection or I/O error
     */
    bool ReadCommandLine (std::string& readCommand);

    /**
     * @brief Wait for data being available for reading
     *
     * @return false  in disconnection or I/O errors
     */
    bool WaitAvailableForReading ();

    /**
     * @brief Get the Stream object
     *
     * @return Stream&
     */
    Stream& GetStream ();

    /**
     * @brief execute command based on assigned command list
     *
     * @param readCommand   command read by the   terminal
     */
    void ExecCommand (const std::string readCommand);

private:
    // Default Stream used to in and out information
    Stream& m_client;

    // Default prompt message
    std::string m_promptString;

    // List of assigned commands Stream, command name, command help
    std::list<Terminal::Command*> m_listAssignedCommands;
};