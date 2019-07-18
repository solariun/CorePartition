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
#include "ThreadLight.hpp"


// Set parameters


// Include application, user and local libraries
// !!! Help http://bit.ly/2CL22Qp


// Define structures and classes


// Define variables and constants


// Prototypes
// !!! Help: http://bit.ly/2l0ZhTa


// Utilities


// Functions


void Thread1 ()
{
    size_t nValue = 100;
    
    while (1)
    {
        printf ("%lu:  Value: [%u]\r\n", getPartitionID(), nValue++);
        Serial.println ("Thread1 ");
        
        Serial.flush();
        
        yield();
    }
}



void Sleep (uint64_t nSleep)
{
    uint32_t nMomentum =  micros();
    
    do {
        delayMicroseconds (100); yield();
    } while ((micros() - nMomentum) < nSleep);
    
    Serial.println ("Voltando...");
}



void Thread2 ()
{
    size_t nValue = 2340000;
    
    while (1)
    {
        printf ("%lu:  Value: [%u]\r\n", getPartitionID(), nValue++);
        Serial.println ("*****Thread2 ");
        Serial.flush();
        
        Sleep(200000);
        //delayMicroseconds (1000);
    }
}



void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    
    /*
     // prints title with ending line break
     Serial.println("press <ENTER>:");
     
     while (Serial.available() == 0) {
     ;
     }
     
     Serial.read();
     */
    
    
    
}





void loop()
{
    
    
    ThreadLight_Start(2);
    
    Serial.println ("Starting Thread.."); Serial.flush();
    
    CreatePartition(Thread1, 210);
    CreatePartition(Thread2, 220);
    
    join();
}
