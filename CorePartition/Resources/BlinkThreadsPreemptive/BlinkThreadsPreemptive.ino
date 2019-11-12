
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



#include "CorePartition.h"

#include "Arduino.h"


ISR(TIMER1_COMPA_vect)
{
    cli ();
    
    if (CorePartition_GetStatus () == THREADL_RUNNING)
    {
        CorePartition_Yield ();
    }

    sei ();
}


void Thread1 (void* pValue)
{
    uint8_t nPin = CorePartition_GetID() + 2;
    bool boolTogle = true;
    
    pinMode (nPin, OUTPUT);
    
    while (1)
    {
        Serial.println (CorePartition_GetStackSize ());
        
        delay (10);
        
        digitalWrite (nPin, boolTogle ? HIGH : LOW);
    
        boolTogle ^= 1;
    }
}


void StackOverflowHandler ()
{
    size_t nThreadID = CorePartition_GetID() + 2;
    uint8_t nCount;
    
    pinMode (nThreadID, OUTPUT);
    
    while (1)
    {
        for (nCount=0; nCount < nThreadID; nCount++)
        {
            digitalWrite (nThreadID, HIGH);
            delay (150);
            digitalWrite (nThreadID, LOW);
            delay (150);
        }
        
        delay (400); //550ms off
    }
}


void setup()
{

cli ();

  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 156;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

sei ();

#define LED_PIN 13

//Initialize serial and wait for port to open:
    Serial.begin(115200);
    while (!Serial);
    
    CorePartition_Start (4);

    CorePartition_SetStackOverflowHandler (StackOverflowHandler);

    CorePartition_CreateThread (Thread1, NULL, 80, 10);
    
    CorePartition_CreateThread (Thread1, NULL, 80, 20);

    CorePartition_CreateThread (Thread1, NULL, 80, 440);

    CorePartition_CreateThread (Thread1, NULL, 80, 50);

}



void loop()
{
    CorePartition_Join();
}
