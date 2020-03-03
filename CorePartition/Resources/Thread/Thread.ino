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




#include "Arduino.h"
#include "CorePartition.h"


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
    
    
    bool show (int nLocY, int nLocX, const char* pszMessage, const uint16_t nMessageLen)
    {
        uint8_t nCount;
       
        if (nSpeed > 0 && nSpeed % 8 == 0) nSpeed++;
        
        if (nSpeed > 0) do
        {
          if (nOffset >= 7)
          {
              nIndex = nIndex + 1 > (int) nMessageLen ? (int) nNumberDigits * (-1) : nIndex + 1;
              nOffset = 0;
              
              if ((int) nNumberDigits * (-1) == nIndex) return false;
          }
          else
          {
              nOffset = (int) nOffset + (nSpeed  % 8);
          }
        } while (nOffset >= 8);

              
        for (nCount=0; nCount < nNumberDigits; nCount++)
        {
            printScrollBytes (nLocY, nLocX, nCount, getLetter(nIndex + 1, pszMessage, nMessageLen), getLetter(nIndex, pszMessage, nMessageLen), (uint8_t) nOffset);
          
            nIndex =  (int) nIndex + 1;
        }
          
        nIndex = (int) nIndex - (nCount - (nSpeed / 8));
        
        return true;
    }
};


void ShowRunningThreads ()
{
    size_t nCount = 0;
    
    Serial.println ();
    Serial.println (F("Listing all running threads"));
    Serial.println (F("--------------------------------------"));
    Serial.println (F("ID\tStatus\tNice\tStkUsed\tStkMax\tCtx\tUsedMem\tExecTime"));
    
    for (nCount = 0; nCount < CorePartition_GetNumberOfThreads (); nCount++)
    {
        Serial.print (F("\e[K"));
        Serial.print (nCount);
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStatusByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetNiceByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetMaxStackSizeByID (nCount));
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetThreadContextSize ());
        Serial.print (F("\t"));
        Serial.print (CorePartition_GetMaxStackSizeByID (nCount) + CorePartition_GetThreadContextSize ());
        Serial.print (F("\t"));
            Serial.print (CorePartition_GetLastDutyCycleByID (nCount));
        Serial.println ("ms");
    }
}


void ThreadTOP (void* pValue)
{
    while (CorePartition_Yield ())
    {
        resetColor ();
        
        setLocation (45, 10);
        
        ShowRunningThreads ();
        
        Serial.flush ();
    }
}




void Thread1 (void* pValue)
{
    int nSize;
    size_t nValue = 1;

    static char szMessage [60] = "";

    TextScroller textScroller (15, 8);

    
    while (CorePartition_Yield ())
    {
        setColor (1, 0);
        
        nSize = snprintf (szMessage, sizeof (szMessage) -1, "Thread #1 : %ld .", nValue++);

        textScroller.show (5, 7, szMessage, nSize);

        Serial.flush ();
    }
}




void Thread2 (void* pValue)
{
    static char szMessage [] = "Thread #2: CorePartition is Universal, working with Windows, Mac, Linux, ARV, ARM, ESP, Tensy, RISC-V....";
        
    TextScroller textScroller (15, 8);
    
    while (CorePartition_Yield ())
    {
        Serial.flush ();
        
        setColor (4, 15);
        
        textScroller.show (15, 7, szMessage, sizeof (szMessage) - 1);
        
        Serial.flush ();
    }
}



void Thread3 (void* pValue)
{
    static char szMessage [] = "#3 - This is really a Thread for small processors as well....";
        
    TextScroller textScroller (15, 8);

    
    while (CorePartition_Yield ())
    {
        setColor (3, 0);
        
        textScroller.show (25, 10, szMessage, sizeof (szMessage) - 1);
        
        resetColor ();
        
        Serial.flush ();
    }
}

void Thread4 (void* pValue)
{
    size_t nValue = 100;

    static char szMessage [] = "#4 - Works every where from Windwows, Linux, Mac, to AVR, ARM, RISC-V and ESP";
        
    TextScroller textScroller (15, 8);
    
    while (CorePartition_Yield ())
    {
        setColor (2, 0);
        
        textScroller.show (35, 10, szMessage, sizeof (szMessage) - 1);
        
        resetColor ();
        
        Serial.flush ();
    }
}

static uint32_t getTimeTick()
{
   return (uint32_t) millis();
}

static void sleepTick (uint32_t nSleepTime)
{
    resetColor ();
    Serial.print (nSleepTime);
    
    if (nSleepTime)  delayMicroseconds  (nSleepTime * 1000);
}

void StackOverflowHandler ()
{
    while  (!Serial);
    
    Serial.print (F("[ERROR] - Stack Overflow - Thread #"));
    Serial.println (CorePartition_GetID ());
    Serial.println (F("--------------------------------------"));
    ShowRunningThreads ();
    Serial.flush ();
    
    while (true);
}


void setup()
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);

    delay (1000);
    
    while (!Serial);

    
    delay (1000);
   
    setLocation (1,1);
    resetColor ();
    hideCursor ();
    clearConsole ();
    
    Serial.print ("CoreThread ");
    Serial.println (CorePartition_version);
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();


    
    //pinMode (2, OUTPUT);
    //pinMode (3, OUTPUT);
    //pinMode (4, OUTPUT);


    /* To test interrupts jump port 2 and 5 */ 
    //pinMode(nPinOutput, OUTPUT);
    


    //pinMode(nPinInput, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(nPinInput), CorePartition_YieldPreemptive, CHANGE);
  

    //Thread1 ();


    CorePartition_Start(5);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);
    CorePartition_SetStackOverflowHandler (StackOverflowHandler);
    
    
    
    CorePartition_CreateSecureThread (ThreadTOP, NULL, 13 * sizeof (size_t), 0);
    
    CorePartition_CreateThread (Thread1, NULL, 25 * sizeof (size_t), 300);

    CorePartition_CreateThread (Thread2, NULL, 30 * sizeof (size_t), 400);
    
    CorePartition_CreateSecureThread (Thread3, NULL, 25 * sizeof (size_t), 1000);
    
    CorePartition_CreateThread (Thread4, NULL, 25 * sizeof (size_t), 2000);


}



void loop()
{
    CorePartition_Join();
}
