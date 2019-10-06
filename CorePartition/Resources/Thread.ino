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



#include "CorePartition.h"

#include "Arduino.h"


// Set parameters


// Include application, user and local libraries
// !!! Help http://bit.ly/2CL22Qp


// Define structures and classes


// Define variables and constants


// Prototypes  
// !!! Help: http://bit.ly/2l0ZhTa


// Utilities


// Functions


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}


volatile uint32_t nCount = 10;


void Thread1 ()
{
    unsigned long start = millis();
    size_t nValue = 100;
    
    //setCoreNice (100);
    
    while (1)
    {
          Serial.print ("## Thread1: ");
          Serial.print (nValue++);
          Serial.print (", Sleep Time: ");
          Serial.print (millis() - start);  start = millis();
          Serial.print (", ");
          Serial.print (CorePartition_GetPartitionUsedMemorySize());
          Serial.print (", All Cores Started? [ ");
          Serial.print (CorePartition_IsAllCoresStarted() ? "YES" : "NO");
          Serial.print ("], Nice: ");
          Serial.print (CorePartition_GetCoreNice());
          Serial.println ("\n");

          Serial.flush();
        
        //digitalWrite (2, LOW);
        //Delay (2000);
        yield ();
        //digitalWrite (2, HIGH);
    }
}




void Thread2 ()
{
    unsigned long start = millis();
    size_t nValue = 2340000;

    //setCoreNice (500);

    while (1)
    {
          Serial.print ("++ Thread2: ");
          Serial.print (nValue++);
          Serial.print (", Sleep Time: ");
          Serial.print (millis() - start);  start = millis();
          Serial.print (" millis");
          Serial.print (", StackSize: ");
          Serial.print (CorePartition_GetPartitionStackSize());
          Serial.print (", Nice: ");
          Serial.print (CorePartition_GetCoreNice());
          Serial.print (", struct Size: [");
          Serial.print (CorePartition_GetThreadStructSize ());
          Serial.print ("] bytes, Core Mem: [");
          Serial.print (CorePartition_GetPartitionUsedMemorySize ());
          Serial.print (" from ");
          Serial.print (CorePartition_GetPartitionAllocatedMemorySize ());
          Serial.println ("]\n");
          
          Serial.flush();
                
       
        
        
        //digitalWrite (3, LOW);
        //Delay (1000);
        yield ();
        //digitalWrite (3, HIGH);
        
    }
}


void Thread3 ()
{
    unsigned long start = millis();
    size_t nValue = 10000;

    //setCoreNice (50);
    
    while (1)
    {
        Serial.print (">> Thread3: ");
        Serial.print (nValue++);
        Serial.print (", Sleep Time: ");
        Serial.print (millis() - start);  start = millis();
        Serial.print (", Nice: ");
        Serial.print (CorePartition_GetCoreNice());
        Serial.println ("\n");

        Serial.flush ();
      
        //digitalWrite (4, LOW);
        //Delay (500);
        yield ();
        //digitalWrite (4, HIGH);
    }
}


static uint64_t getTimeTick()
{
   return (uint64_t) millis();
}

static void sleepTick (uint64_t nSleepTime)
{
    delayMicroseconds  (nSleepTime * 1000);
}

void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);

    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println (", Starting Thread...."); Serial.flush();Serial.flush();

    delay (1000);
    
    //pinMode (2, OUTPUT);
    //pinMode (3, OUTPUT);
    //pinMode (4, OUTPUT);


    /* To test interrupts jump port 2 and 5 */ 
    //pinMode(nPinOutput, OUTPUT);
    


    //pinMode(nPinInput, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(nPinInput), YieldPreemptive, CHANGE);

    CorePartition_Start(3);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);

    CreatePartition(Thread1, 100, 2000);
    
    CreatePartition(Thread2, 100, 21);

    CreatePartition(Thread3, 100, 1000);
}



void loop()
{
    join();
}
