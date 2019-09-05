///
/// @mainpage  Threaduino
///
/// @details  Description of the project
/// @n
/// @n
/// @n @a   Developed with [embedXcode+](https://embedXcode.weebly.com)
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


///
/// @file   Threaduino.ino
/// @brief    Main sketch
///
/// @details  <#details#>
/// @n @a   Developed with [embedXcode+](https://embedXcode.weebly.com)
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
/// @n
///


#include "Arduino.h"
#include "CorePartition.hpp"


// Set parameters


// Include application, user and local libraries
// !!! Help http://bit.ly/2CL22Qp


// Define structures and classes


// Define variables and constants


// Prototypes
// !!! Help: http://bit.ly/2l0ZhTa


// Utilities


// Functions

void safe_yield()
{
    //noInterrupts();
    _yield();
    //interrupts();
}

void _Sleep (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();
    
    //delay (nSleep); return;
    
    do {
        safe_yield();
    } while ((millis() - nMomentum ) <  nSleep);
}


volatile uint32_t nCount = 10;


void Thread1 ()
{
    size_t nValue = 100;
    
    safe_yield(); while (1)
    {
        blockCore(true);
        Serial.print ("Thread1: ");
        Serial.print (nValue++);
        Serial.print (", ");
        Serial.print (getPartitionMemorySize());
        Serial.print (", Interrupt Count: ");
        Serial.println (nCount);
        Serial.print (", All Cores: ");
        Serial.println (isAllCoresStarted());
        Serial.flush();
        Serial.flush();
        blockCore (false);
        
        //digitalWrite (2, LOW);
        
        _Sleep(10);
        //digitalWrite (2, HIGH);
    }
}




void Thread2 ()
{
    unsigned long start = millis();
    size_t nValue = 2340000;
    
    safe_yield(); while (1)
    {
        blockCore (true);
        Serial.print ("*****Thread2: ");
        Serial.print (nValue++);
        Serial.print (", ");
        Serial.print (millis() - start);
        Serial.print (" millis");
        Serial.print (", StackSize: ");
        Serial.println (getPartitionStackSize());
        Serial.flush();
        blockCore (false);
        
        start = millis();
        
        
        digitalWrite (3, LOW);
        _Sleep(500);
        digitalWrite (3, HIGH);
        
    }
}


void Thread3 ()
{
    size_t nValue = 10000;
    
    safe_yield(); while (1)
    {
        blockCore (true);
        Serial.print ( ">>>>>> Thread3: ");
        Serial.println (nValue++);
        Serial.flush ();
        blockCore (false);
        
        digitalWrite (4, LOW);
        _Sleep (100);
        digitalWrite (4, HIGH);
    }
}



void YieldPreemptive()
{
    static unsigned long time;
    static bool status = LOW;
    
    if (isAllCoresStarted() && isCoreRunning() && millis() - time > 10 && (time = millis()))
    {
        nCount++;
        
        status = !status;
        
        digitalWrite (LED_BUILTIN, status);
    }
}


void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    
    
    /*
     // prints title with ending line break
     Serial.println("press <ENTER>:");
     
     while (Serial.available() == 0) {
     ;
     }
     
     Serial.read();
     */
    
    pinMode (3, OUTPUT);
    pinMode (4, OUTPUT);
    
    
    /* To test interrupts jump port 2 and 5 */
    /*
        int nPinOutput = 5;
        int nPinInput  = 2;
     
        pinMode(nPinOutput, OUTPUT);
        analogWrite(nPinOutput, 10);
     
        pinMode(nPinInput, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(nPinInput), YieldPreemptive, CHANGE);
    */
}





void loop()
{
    
    
    CorePartition_Start(3);
    
    Serial.println ("Starting Thread...."); Serial.flush();
    
    CreatePartition(Thread1, 100);
    
    CreatePartition(Thread2, 100);
    
    CreatePartition(Thread3, 100);
    
    
    join();
}
