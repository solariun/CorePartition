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

void _Sleep (uint64_t nSleep)
{
    uint32_t nMomentum =  micros();
    
    do {
        delayMicroseconds (100); yield();
    } while ((micros() - nMomentum) < nSleep);
    
    Serial.println ("Voltando...");
}



void Thread1 ()
{
    size_t nValue = 100;
    
    while (1)
    {
        Serial.print ("Thread1: ");
        Serial.print (nValue++);
        Serial.print (", ");
        Serial.println (getPartitionMemorySize());
        
        Serial.flush();
        
        digitalWrite (2, LOW);
        
        _Sleep(10);
        digitalWrite (2, HIGH);
    }
}




void Thread2 ()
{
    unsigned long start = millis();
    size_t nValue = 2340000;
    
    while (1)
    {
        Serial.print ("*****Thread2: ");
        Serial.print (nValue++);
        Serial.print (", ");
        Serial.print (millis() - start);
        Serial.print (" millis");
        Serial.print (", StackSize: ");
        Serial.println (getPartitionStackSize());
        Serial.flush();
        
        start = millis();
        
        
        digitalWrite (3, LOW);
        _Sleep(50000);
        digitalWrite (3, HIGH);
        
    }
}


void Thread3 ()
{
    size_t nValue = 10000;
    
    while (1)
    {
        Serial.print ( "Thread3: ");
        Serial.println (nValue++);
        Serial.flush();
        
        digitalWrite (4, LOW);
        _Sleep (1000);
        digitalWrite (4, HIGH);
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
    
    pinMode (2, OUTPUT);
    pinMode (3, OUTPUT);
    pinMode (4, OUTPUT);
    
}





void loop()
{
    
    
    CorePartition_Start(3);
    
    Serial.println ("Starting Thread...."); Serial.flush();
    
    CreatePartition(Thread1, 100);
    
    CreatePartition(Thread2, 100);
    
    join();
}
