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

#include "Stream.h"
#include <string>
#include <list>
#include "CorePartition.h"

#define TERMINAL_BS     8



class Terminal 
{
public:

    class ThreadStream : public Stream
    {
        public:
            using Print::write;
            virtual size_t write(const uint8_t *buffer, size_t size) override;
    };

    class Command
    {
    public:
            
        std::string  m_commandName;
        std::string  m_commandDescription;

        virtual void Run (Terminal& terminal, Terminal::ThreadStream& client, const std::string commandLine) = 0;
    };

    /**
     * @brief Construct a new Terminal object
     * 
     * @param streamDev  Stream object for in and out procedures
     */
    Terminal (Terminal::ThreadStream& streamDev);


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
    bool WaitForACommand();


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


        //Default Stream used to in and out information 
        Terminal::ThreadStream&  m_client;

        //Default prompt message
        std::string   m_promptString;

        //List of assigned commands Stream, command name, command help
        std::list<Terminal::Command*> m_listAssignedCommands;

};