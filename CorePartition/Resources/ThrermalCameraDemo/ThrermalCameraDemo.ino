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

//#include "U8glib.h"
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <LedControl.h>


#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

uint8_t nHotColor[] = {17, 17, 17, 17, 18, 18, 19, 19,  21, 21,  33,  33,  51,  51,  85, 85, 82, 82, 155, 155, 226, 226, 214, 214, 172, 172, 160, 160, 124, 124, 52, 255 };

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 


int DIN = 12 ; //MISO
int CS =  11; //MOSI
int CLK = 10; //SS


// Utilities


// Functions


byte byteImages [][8]={
   {0x7C,0x7C,0x60,0x7C,0x7C,0x60,0x7C,0x7C},
    {0x78,0x7C,0x66,0x66,0x66,0x66,0x7C,0x78},
   {0x66,0x66,0x66,0x66,0x66,0x66,0x7E,0x7E},
   {0x7E,0x7E,0x60,0x60,0x60,0x60,0x7E,0x7E},
   {0x7E,0x7E,0x66,0x7E,0x7E,0x66,0x7E,0x7E},
   {0x7E,0x7C,0x60,0x7C,0x3E,0x06,0x3E,0x7E}, 
   {0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18},
   {0x7E,0x7E,0x66,0x66,0x66,0x66,0x7E,0x7E},
   {0xE7,0xFF,0xFF,0xDB,0xDB,0xDB,0xC3,0xC3},
   {0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C},
   {0x3C,0x42,0xA5,0x81,0xBD,0x81,0x42,0x3C},
   {0x3C,0x42,0xA5,0x81,0x99,0xA5,0x42,0x3C}
}; 

LedControl lc=LedControl(DIN,CLK,CS,2);

void printByte(byte character [])
{
  int i = 0;
  for(i=0;i<8;i++)
  {
    lc.setColumn(0,i,character[i]);
  }
}

void printScrollBytes(uint8_t nDevice, byte charLeft [], byte charRight [], uint8_t nOffset)
{
  int i = 0;
  for(i=0;i<8;i++)
  {
    lc.setColumn(nDevice,i,(charLeft [i] << (8-nOffset) | charRight [i] >> nOffset));
  }
}


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}


volatile uint32_t nCount = 10;

char szTemp [40];

void Thread1 ()
{
    unsigned long start = millis();
    size_t nValue = 100;
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t nOffset = 0;
    
    //setCoreNice (100);

    
    while (1)
    {
          Serial.print ("\e[2;20H\e[K## Thread1: ");
          Serial.print (nValue++);
          Serial.print (", Sleep Time: ");                 
          Serial.print (millis() - start); 
          Serial.print (", ");
          Serial.print (CorePartition_GetPartitionUsedMemorySize());
          Serial.print (", All Cores Started? [ ");
          Serial.print (CorePartition_IsAllCoresStarted() ? "YES" : "NO");
          Serial.print ("], Nice: ");
          Serial.print (CorePartition_GetCoreNice());
          Serial.println ("\n");

          Serial.flush();
         
          start = millis();

          printScrollBytes (1, byteImages [((a+1 >= sizeof (byteImages)/8) ? 0 : a+1)], byteImages [a], nOffset);
          b = (a+1 >= sizeof (byteImages)/8) ? 0 : a+1;
          printScrollBytes (0, byteImages [((b+1 >= sizeof (byteImages)/8) ? 0 : b+1)], byteImages [b], nOffset);
          
          if (nOffset + 1 >= 8)
          {
              nOffset = 0;
              a = (a+1 >= sizeof (byteImages)/8) ? 0 : a+1;
          }
          else
          {
              nOffset++;
          }
          
          
          
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
    bool boolCls = false;

    //setCoreNice (500);

    while (1)
    {
      
          if (boolCls == false)
          {
             boolCls = true;
             Serial.print ("\e[2J");
          }
          
          Serial.print ("\e[4;20H\e[K++ Thread2: ");
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

         float fmin = 1000, fmax = 0; 
      
          
          for(int i=AMG88xx_PIXEL_ARRAY_SIZE; i > 0; i--)
          {
            
            fmin = MIN (fmin, pixels[i-1]);
            fmax = MAX (fmax, pixels[i-1]);      
          }

          Serial.print ("\e[1:1H");

          for(int i=AMG88xx_PIXEL_ARRAY_SIZE; i > 0; i--)
          {
            Serial.print ("\x1b[48;5;");
            //Serial.print (nHotColor [map (pixels[i-1], fmin, fmin + 8, 0, sizeof (nHotColor)-1)]);
            Serial.print (map (pixels[i-1], fmin, fmin + 10, 232, 255 ));
            Serial.print ("m  \e[0m");
            
            if( (i-1)%8 == 0 ) Serial.println();
          }
      
          Serial.println();
          
          Serial.print ("Min: ");
          Serial.print (fmin);
          Serial.print (" Max: ");
          Serial.println (fmax);
      
          Serial.println();
          Serial.println();
        
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
        Serial.print ("\e[6;20H\e[K>> Thread3: ");
        Serial.print (nValue++);
        Serial.print (", Sleep Time: ");
        Serial.print (millis() - start);  //start = millis();
        Serial.print (", Nice: ");
        Serial.print (CorePartition_GetCoreNice());
        Serial.println ("\n");

        Serial.flush ();

        //read all the pixels
        amg.readPixels(pixels);
          
          start = millis();
                    
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
    bool status; 
    
    //Initialize serial and wait for port to open:
    Serial.begin(115200);

    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();

       
    // default settings
    status = amg.begin();
    if (!status) 
    {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }

    //Initializing displays 1 and 2
    
    lc.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(0,8);      // Set the brightness to maximum value
    lc.clearDisplay(0);         // and clear the display
    
    lc.shutdown(1,false);       //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(1,8);      // Set the brightness to maximum value
    lc.clearDisplay(1);         // and clear the display

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

    CreatePartition(Thread1, 100, 100);
    
    CreatePartition(Thread2, 100, 150);

    CreatePartition(Thread3, 100, 1);
}



void loop()
{
    join();
}
