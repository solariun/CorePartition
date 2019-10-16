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


static const uint64_t byteImages[] PROGMEM = {
  0x7e1818181c181800,
  0x7e060c3060663c00,
  0x3c66603860663c00,
  0x30307e3234383000,
  0x3c6660603e067e00,
  0x3c66663e06663c00,
  0x1818183030667e00,
  0x3c66663c66663c00,
  0x3c66607c66663c00,
  0x3c66666e76663c00,
  0x6666667e66663c00,
  0x3e66663e66663e00,
  0x3c66060606663c00,
  0x3e66666666663e00,
  0x7e06063e06067e00,
  0x0606063e06067e00,
  0x3c66760606663c00,
  0x6666667e66666600,
  0x3c18181818183c00,
  0x1c36363030307800,
  0x66361e0e1e366600,
  0x7e06060606060600,
  0xc6c6c6d6feeec600,
  0xc6c6e6f6decec600,
  0x3c66666666663c00,
  0x06063e6666663e00,
  0x603c766666663c00,
  0x66361e3e66663e00,
  0x3c66603c06663c00,
  0x18181818185a7e00,
  0x7c66666666666600,
  0x183c666666666600,
  0xc6eefed6c6c6c600,
  0xc6c66c386cc6c600,
  0x1818183c66666600,
  0x7e060c1830607e00,
  0x0000000000000000,
  0x7c667c603c000000,
  0x3e66663e06060600,
  0x3c6606663c000000,
  0x7c66667c60606000,
  0x3c067e663c000000,
  0x0c0c3e0c0c6c3800,
  0x3c607c66667c0000,
  0x6666663e06060600,
  0x3c18181800180000,
  0x1c36363030003000,
  0x66361e3666060600,
  0x1818181818181800,
  0xd6d6feeec6000000,
  0x6666667e3e000000,
  0x3c6666663c000000,
  0x06063e66663e0000,
  0xf0b03c36363c0000,
  0x060666663e000000,
  0x3e403c027c000000,
  0x1818187e18180000,
  0x7c66666666000000,
  0x183c666600000000,
  0x7cd6d6d6c6000000,
  0x663c183c66000000,
  0x3c607c6666000000,
  0x3c0c18303c000000
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





void printScrollBytes(uint8_t nDevice, uint64_t charLeft, uint64_t charRight, uint8_t nOffset)
{
  int i = 0;
  for(i=0;i<8;i++)
  {        
      lc.setColumn(nDevice,i, (((uint8_t*) &charLeft) [i] << (8-nOffset) | ((uint8_t*) &charRight) [i] >> nOffset));
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

uint64_t getImage (unsigned int nIndex)
{
    uint16_t nImagesItens = sizeof (byteImages) / sizeof (byteImages[0]);
    uint64_t nBuffer= 0xAA;
    
    nIndex = nIndex > nImagesItens ? nImagesItens - nIndex : nIndex;


    memcpy_PF(&nBuffer, byteImages + nIndex, sizeof (uint64_t));
    
    return nBuffer;
}




void Thread1 ()
{
    unsigned long start = millis();
    size_t nValue = 100;
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t nOffset = 0;
    uint16_t nImagesItens = sizeof (byteImages) / sizeof (byteImages[0]);
    //setCoreNice (100);

    
    while (1)
    {
          Serial.print ("\e[2;20H\e[K## Thread1: ");
          Serial.print (nValue++);
          Serial.print (", Sleep Time: ");                 
          Serial.print (millis() - start); 
          Serial.print (", ");
          Serial.print (CorePartition_GetCoreNice ());
          Serial.print (", nImagesItens [ ");
          Serial.print (nImagesItens);
          Serial.print ("]");
          Serial.println ("\n");
  
          Serial.flush();
         
          start = millis();

          
          printScrollBytes (1, getImage ( ((a+1 >= nImagesItens) ? 0 : a+1) ), getImage (a), nOffset);
          b = (a+1 >= nImagesItens) ? 0 : a+1;
          printScrollBytes (0, getImage ( ((b+1 >= nImagesItens) ? 0 : b+1) ), getImage (b), nOffset);
          
          if (nOffset + 1 >= 8)
          {
              nOffset = 0;
              a = (a+1 >= nImagesItens ) ? 0 : a+1;
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
    lc.setIntensity(1,4);      // Set the brightness to maximum value
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
    
    CreatePartition(Thread2, 100, 50);

    CreatePartition(Thread3, 100, 100);
}



void loop()
{
    join();
}
