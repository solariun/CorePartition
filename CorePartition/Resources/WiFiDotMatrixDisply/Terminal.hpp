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

class Terminal 
{
    public:

    Terminal (Stream& streamDev);

    virtual ~Terminal ();

    virtual bool isConnected ();

    virtual bool ExecuteMOTD ();   

    bool WaitForACommand();

    void SetPromptString (const std::string& promptString);

    protected:

    bool ReadCommand (std::string& readCommand);

    bool WaitAvailableForReading ();

    Stream& getStream ();

    private:

        //Default Stream used to in and out information 
        Stream&  m_client;
        std::string   m_promptString;

};