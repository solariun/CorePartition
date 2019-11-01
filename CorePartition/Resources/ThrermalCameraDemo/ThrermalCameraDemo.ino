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

LedControl lc=LedControl(DIN,CLK,CS,2);


void setLocation (uint16_t nY, uint16_t nX)
{
    byte szTemp [10];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%uf", nY, nX);

    Serial.write (szTemp, nLen);
}


//workis with 256 colors
void setColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    byte szTemp [10];
    uint8_t nLen = snprintf ((char*) szTemp, sizeof(szTemp), "\033[%u;%um", nFgColor + 30, nBgColor + 40);

    Serial.write (szTemp, nLen);
}


void resetColor ()
{
    Serial.print ("\033[0m");
}


void hideCursor ()
{
    Serial.print ("\033[?25l");
}


void showCursor ()
{
    Serial.print ("\033[?25h");
}


void clearConsole ()
{
    Serial.print ("\033[2J"); 
}


void reverseColor ()
{
    Serial.print ("\033[7m");   
}


void Delay (uint64_t nSleep)
{
    uint32_t nMomentum =  millis();

    //delay (nSleep); return;
    
    do {
        CorePartition_Yield();
    } while ((millis() - nMomentum ) <  nSleep);    
}


const uint64_t byteImages[] PROGMEM =
{
    0x0000000000000000, 0x00180018183c3c18, 0x0000000012246c6c, 0x0036367f367f3636, 0x000c1f301e033e0c,
    0x0063660c18336300, 0x006e333b6e1c361c, 0x0000000000030606, 0x00180c0606060c18, 0x00060c1818180c06,
    0x0000663cff3c6600, 0x00000c0c3f0c0c00, 0x060c0c0000000000, 0x000000003f000000, 0x000c0c0000000000,
    0x000103060c183060, 0x003e676f7b73633e, 0x003f0c0c0c0c0e0c, 0x003f33061c30331e, 0x001e33301c30331e,
    0x0078307f33363c38, 0x001e3330301f033f, 0x001e33331f03061c, 0x000c0c0c1830333f, 0x001e33331e33331e,
    0x000e18303e33331e, 0x000c0c00000c0c00, 0x060c0c00000c0c00, 0x00180c0603060c18, 0x00003f00003f0000,
    0x00060c1830180c06, 0x000c000c1830331e, 0x001e037b7b7b633e, 0x6666667e66663c00, 0x3e66663e66663e00,
    0x3c66060606663c00, 0x3e66666666663e00, 0x7e06063e06067e00, 0x0606063e06067e00, 0x3c66760606663c00,
    0x6666667e66666600, 0x3c18181818183c00, 0x1c36363030307800, 0x66361e0e1e366600, 0x7e06060606060600,
    0xc6c6c6d6feeec600, 0xc6c6e6f6decec600, 0x3c66666666663c00, 0x06063e6666663e00, 0x603c766666663c00,
    0x66361e3e66663e00, 0x3c66603c06663c00, 0x18181818185a7e00, 0x7c66666666666600, 0x183c666666666600,
    0xc6eefed6c6c6c600, 0xc6c66c386cc6c600, 0x1818183c66666600, 0x7e060c1830607e00, 0x001e06060606061e,
    0x00406030180c0603, 0x001e18181818181e, 0x0000000063361c08, 0x003f000000000000, 0x0000000000180c06,
    0x7c667c603c000000, 0x3e66663e06060600, 0x3c6606663c000000, 0x7c66667c60606000, 0x3c067e663c000000,
    0x0c0c3e0c0c6c3800, 0x3c607c66667c0000, 0x6666663e06060600, 0x3c18181800180000, 0x1c36363030003000,
    0x66361e3666060600, 0x1818181818181800, 0xd6d6feeec6000000, 0x6666667e3e000000, 0x3c6666663c000000,
    0x06063e66663e0000, 0xf0b03c36363c0000, 0x060666663e000000, 0x3e603c067c000000, 0x1818187e18180000,
    0x7c66666666000000, 0x183c666600000000, 0x7cd6d6d6c6000000, 0x663c183c66000000, 0x3c607c6666000000,
    0x3c0c18303c000000, 0x00380c0c070c0c38, 0x0c0c0c0c0c0c0c0c, 0x00070c0c380c0c07, 0x0000000000003b6e
 };

const int byteImagesLen = sizeof(byteImages)/8;

      
class TextScroller
{
private:
    
     int nNumberDigits = 15;
     uint8_t nOffset = 0;
     int  nIndex = nNumberDigits * (-1);
     uint8_t nSpeed;
    
    uint64_t getLetter (int nIndex, const char* pszMessage, uint16_t nMessageLen)
    {
        int nCharacter = nIndex > nMessageLen || nIndex < 0 ? ' ' : pszMessage [nIndex];
        
        return getImage (nCharacter - ' ');
    }

    uint64_t getImage (int nIndex)
    {
        uint64_t nBuffer= 0xAA;

        nIndex = nIndex > byteImagesLen ||  nIndex < 0  ? 0 : nIndex;

        memcpy_P (&nBuffer, byteImages + nIndex, sizeof (uint64_t));
        
        return nBuffer;
    }

    void printScrollBytes(uint16_t  nLocY, uint16_t nLocX, uint16_t nDigit, const uint64_t charLeft, const uint64_t charRight, uint8_t nOffset)
    {
        int i = 0;
          
        for(i=0;i<8;i++)
        {
            printRow (nLocY, nLocX, nDigit, i, (((uint8_t*) &charLeft) [i] << (8-nOffset) | ((uint8_t*) &charRight) [i] >> nOffset));
        }
    }

    
protected:
    
    virtual void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow)
    {
        static char nLine [9] = "        ";
        int8_t nOffset = 8;

        //Serial.print (nRow, BIN);
        
        while (--nOffset >= 0)
        {
           nLine [7-nOffset] = nRow & 1 != 0 ? '#' : ' ';
           nRow >>= 1;
        }

        setLocation (nLocY + nRowIndex, nLocX + (nDigit * 8));
        Serial.print (nLine);
    }

public:
    
    TextScroller (int nNumberDigits, uint8_t nSpeed) :
        nNumberDigits (nNumberDigits),
        nOffset (0),
        nSpeed (nSpeed)
    {
       nIndex = nSpeed == 0 ? 0 : nNumberDigits * (-1);
    }
    
    
    void show (int nLocY, int nLocX, const char* pszMessage, const uint16_t nMessageLen)
    {
        uint8_t nCount;
       
        if (nSpeed > 0 && nSpeed % 8 == 0) nSpeed++;
        
        if (nSpeed > 0) do
        {
          if (nOffset >= 7)
          {
              nIndex = nIndex + 1 > (int) nMessageLen ? (int) nNumberDigits * (-1) : nIndex + 1;
              nOffset = 0;
       
          }
          else
          {
              nOffset = (int) nOffset + (nSpeed  % 8);
          }
        } while (nOffset >= 7);

              
        for (nCount=0; nCount < nNumberDigits; nCount++)
        {
            printScrollBytes (nLocY, nLocX, nCount, getLetter(nIndex + 1, pszMessage, nMessageLen), getLetter(nIndex, pszMessage, nMessageLen), (uint8_t) nOffset);
          
            nIndex =  (int) nIndex + 1;
        }
          
         nIndex = (int) nIndex - (nCount - (nSpeed / 8));
     
    }
};


class MatrixTextScroller : public TextScroller 
{
protected:

    void printRow (uint16_t nLocY, uint16_t nLocX, uint16_t nDigit, uint8_t nRowIndex, uint8_t nRow) override
    {
        lc.setColumn(nDigit, nRowIndex, nRow);
    }

public:

    using TextScroller::TextScroller;
};



void Thread1 ()
{
    unsigned long start = millis();
    size_t nValue = 100;
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t nOffset = 0;
    uint16_t nImagesItens = sizeof (byteImages) / sizeof (byteImages[0]);
    //setCoreNice (100);

    MatrixTextScroller matrixTextScroller (2, 1);
    
    const char szMessage[] = "CorePartition! :) works!";
    
    while (1)
    {
          Serial.print ("\e[2;20H\e[K## Thread1: ");
          Serial.print (nValue++);
          Serial.print (", Sleep Time: ");                 
          Serial.print (millis() - start); 
          Serial.print (", ");
          Serial.print (CorePartition_GetNice ());
          Serial.print (", nImagesItens [ ");
          Serial.print (nImagesItens);
          Serial.print ("]");
          Serial.println ("\n");
  
          Serial.flush();
         
          start = millis();

          matrixTextScroller.show (0, 0, szMessage, sizeof (szMessage)-1);
          
        //digitalWrite (2, LOW);
        //Delay (2000);
        CorePartition_Yield ();
        //digitalWrite (2, HIGH);
    }
}


float fMin = 1000, fMax = 0; 


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
          Serial.print (CorePartition_GetStackSize());
          Serial.print (", Nice: ");
          Serial.print (CorePartition_GetNice());
          Serial.print (", struct Size: [");
          Serial.print (CorePartition_GetThreadStructSize ());
          Serial.print ("] bytes, Core Mem: [");
          Serial.print (CorePartition_GetUsedMemorySize ());
          Serial.print (" from ");
          Serial.print (CorePartition_GetAllocatedMemorySize ());
          Serial.println ("]\n");
      
          fMin = 1000, fMax = 0; 
          
          for(int i=AMG88xx_PIXEL_ARRAY_SIZE; i > 0; i--)
          { 
            fMin = MIN (fMin, pixels[i-1]);
            fMax = MAX (fMax, pixels[i-1]);      
          }

          CorePartition_Yield ();
          
          setLocation (1,1);

          resetColor ();
          
          for(int i=AMG88xx_PIXEL_ARRAY_SIZE; i > 0; i--)
          {
            Serial.print ("\033[48;5;");
            //Serial.print (nHotColor [map (pixels[i-1], fMin, fMin + 8, 0, sizeof (nHotColor)-1)]);
            Serial.print (map (pixels[i-1], fMin, fMax, 232, 255 ));
            Serial.print ("m  \033[0m");
            
            if( (i-1)%8 == 0 ) Serial.println();
          }
          
          Serial.flush();
              
      
  
        //digitalWrite (3, LOW);
        //Delay (1000);
        CorePartition_Yield ();
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
        Serial.print (CorePartition_GetNice());
        Serial.println ("\n");

        Serial.flush ();

        //read all the pixels
        amg.readPixels(pixels);
          
          start = millis();
                    
        //digitalWrite (4, LOW);
        //Delay (500);
        CorePartition_Yield ();
        //digitalWrite (4, HIGH);
    }
}



void Thread4 ()
{    
    char szMessage [20]=""; 
    uint8_t nSize;

    TextScroller textScroller (16, 0);

    while (1)
    {
        nSize = snprintf (szMessage, sizeof (szMessage)-1, "Min:%dc Max:%dc", (int) fMin, (int) fMax);

        setColor (1, 0);
        
        textScroller.show (10, 1, szMessage, nSize);

        resetColor ();
        
        Serial.println();
        
        Serial.print ("Min: ");
        Serial.print (fMin);
        Serial.print (" Max: ");
        Serial.println (fMax);
    
        Serial.println();
        Serial.println();

        
        CorePartition_Yield ();
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
    Serial.begin(230400);

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

    resetColor ();
    clearConsole ();
    hideCursor ();
    
    //pinMode (2, OUTPUT);
    //pinMode (3, OUTPUT);
    //pinMode (4, OUTPUT);


    /* To test interrupts jump port 2 and 5 */ 
    //pinMode(nPinOutput, OUTPUT);
    

    //pinMode(nPinInput, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(nPinInput), CorePartition_YieldPreemptive, CHANGE);

    CorePartition_Start (4);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);

    CorePartition_CreateThread (Thread1, 100, 50);
    
    CorePartition_CreateThread (Thread2, 100, 2);

    CorePartition_CreateThread (Thread3, 100, 4);

    CorePartition_CreateThread (Thread4, 100, 200);
}



void loop()
{
    CorePartition_Join();
}
