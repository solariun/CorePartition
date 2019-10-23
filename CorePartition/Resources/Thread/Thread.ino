


#include "Arduino.h"
#include "CorePartition.h"

const uint64_t byteImages[] PROGMEM = {
  0x0000000000000000,
  0x00180018183c3c18,
  0x0000000012246c6c,
  0x0036367f367f3636,
  0x000c1f301e033e0c,
  0x0063660c18336300,
  0x006e333b6e1c361c,
  0x0000000000030606,
  0x00180c0606060c18,
  0x00060c1818180c06,
  0x0000663cff3c6600,
  0x00000c0c3f0c0c00,
  0x060c0c0000000000,
  0x000000003f000000,
  0x000c0c0000000000,
  0x000103060c183060,
  0x003e676f7b73633e,
  0x003f0c0c0c0c0e0c,
  0x003f33061c30331e,
  0x001e33301c30331e,
  0x0078307f33363c38,
  0x001e3330301f033f,
  0x001e33331f03061c,
  0x000c0c0c1830333f,
  0x001e33331e33331e,
  0x000e18303e33331e,
  0x000c0c00000c0c00,
  0x060c0c00000c0c00,
  0x00180c0603060c18,
  0x00003f00003f0000,
  0x00060c1830180c06,
  0x000c000c1830331e,
  0x001e037b7b7b633e,
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
  0x001e06060606061e,
  0x00406030180c0603,
  0x001e18181818181e,
  0x0000000063361c08,
  0x003f000000000000,
  0x0000000000180c06,
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
  0x3c0c18303c000000,
  0x00380c0c070c0c38,
  0x0c0c0c0c0c0c0c0c,
  0x00070c0c380c0c07,
  0x0000000000003b6e
};

const int byteImagesLen = sizeof(byteImages)/8;


void setLocation (uint16_t nY, uint16_t nX)
{
    Serial.print ("\e[");
    Serial.print (nY);
    Serial.print (";");
    Serial.print (nX);
    Serial.print ("H");
}


//workis with 256 colors
void setColor (const uint8_t nFgColor, const uint8_t nBgColor)
{
    Serial.print ("\e[");
    Serial.print ("38:5:");
    Serial.print (nFgColor);
    Serial.print ("m");

    Serial.print ("\e[");
    Serial.print ("48:5:");
    Serial.print (nBgColor);
    Serial.print ("m");
}


void resetColor ()
{
    Serial.print ("\e[0m");
}


void hideCursor ()
{
    Serial.print ("\e[?25l");
}


void showCursor ()
{
    Serial.print ("\e[?25h");
}


uint64_t getImage (int nIndex)
{
    uint64_t nBuffer= 0xAA;

    nIndex = nIndex > byteImagesLen ||  nIndex < 0  ? 0 : nIndex;

    memcpy_P (&nBuffer, byteImages + nIndex, sizeof (uint64_t));
    
    return nBuffer;
}



void PrintRow (uint16_t nLocY, uint16_t nLocX, uint8_t nRowIndex, uint8_t nRow)
{
    static char nLine [9] = "        ";
    int8_t nOffset = 8;

    //Serial.print (nRow, BIN);
    
    while (--nOffset >= 0)
    {
       nLine [7-nOffset] = nRow & 1 != 0 ? '*' : ' ';
       nRow >>= 1;
    }

    setLocation (nLocY + nRowIndex, nLocX); 
    Serial.println (nLine);
}


void printScrollBytes(uint16_t  nLocY, uint16_t nLocX, const uint64_t charLeft, const uint64_t charRight, uint8_t nOffset)
{
    int i = 0;
      
    for(i=0;i<8;i++)
    {        
        PrintRow (nLocY, nLocX, i, (((uint8_t*) &charLeft) [i] << (8-nOffset) | ((uint8_t*) &charRight) [i] >> nOffset));
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


uint64_t getLetter (int nIndex, const char* pszMessage, uint16_t nMessageLen)
{
    int nCharacter = nIndex > nMessageLen || nIndex < 0 ? ' ' : pszMessage [nIndex];
    
    return getImage (nCharacter - ' ');
}


void ScrollText (int nLocY, int nLocX, int* nIndex, uint8_t* nOffset, uint8_t nNumberDigits, const char* pszMessage, const uint16_t nMessageLen)
{
    uint8_t nCount; 
   
          
    for (nCount=0; nCount < nNumberDigits; nCount++)
    {   
        printScrollBytes (nLocY, nLocX + (nCount * 8), getLetter(*nIndex + 1, pszMessage, nMessageLen), getLetter(*nIndex, pszMessage, nMessageLen), (uint8_t) *nOffset);
    
        *nIndex =  (int) *nIndex + 1;
    }   
      
      *nIndex = (int) *nIndex - (nCount-1);
  
          
    if (*nOffset >= 7)
    {   
        *nIndex = *nIndex + 1 > (int) nMessageLen ? (int) nNumberDigits * (-1) : *nIndex + 1;
        *nOffset = 0;
 
    }
    else
    {
        *nOffset = (int) *nOffset + 1;

        Serial.println ();
        Serial.print (*nOffset);
    }
}

      



void Thread1 ()
{
    size_t nValue = 100;

    static char szMessage [] = "Thread #1 - CorePartition Thread manager";
     
    static int nNumberDigits = 10;

    uint8_t nOffset = 0;
    
    int  nIndex = nNumberDigits * (-1);
   

    //setCoreNice (100);


    Serial.println ("Starting up Thread #1");
    
    while (1)
    {

        setColor (168, 52);
        hideCursor ();
        
        ScrollText (5, 10, &nIndex, &nOffset, nNumberDigits, szMessage, sizeof (szMessage) - 1);

        showCursor ();
        resetColor ();
        
        Serial.flush();
        
      yield ();
    }
}




void Thread2 ()
{
    size_t nValue = 100;

    static char szMessage [] = "Thread #2 - CorePartition is for Small processors as well";
     
    static int nNumberDigits = 8;

    uint8_t nOffset = 0;
    
    int  nIndex = nNumberDigits * (-1);
   

    //setCoreNice (100);


    Serial.println ("Starting up Thread #1");
    
    while (1)
    {

        setColor (46, 22);
        hideCursor ();
        
        ScrollText (15, 10, &nIndex, &nOffset, nNumberDigits, szMessage, sizeof (szMessage) - 1);

        showCursor ();
        resetColor ();
        
        Serial.flush();
        
      yield ();
    }
}


void Thread3 ()
{
    size_t nValue = 100;

    static char szMessage [] = "Thread #3 - Works on Windows, Mac and Linux";
     
    static int nNumberDigits = 10;

    uint8_t nOffset = 0;
    
    int  nIndex = nNumberDigits * (-1);
   

    //setCoreNice (100);


    Serial.println ("Starting up Thread #1");
    
    while (1)
    {

        setColor (27, 17);
        hideCursor ();
        
        ScrollText (25, 10, &nIndex, &nOffset, nNumberDigits, szMessage, sizeof (szMessage) - 1);

        showCursor ();
        resetColor ();
        
        Serial.flush();
        
      yield ();
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
    Serial.println ("");
    
    Serial.println ("Starting up Thread...."); Serial.flush();Serial.flush();

    delay (1000);
    
    //pinMode (2, OUTPUT);
    //pinMode (3, OUTPUT);
    //pinMode (4, OUTPUT);


    /* To test interrupts jump port 2 and 5 */ 
    //pinMode(nPinOutput, OUTPUT);
    


    //pinMode(nPinInput, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(nPinInput), YieldPreemptive, CHANGE);


    //Thread1 ();

    CorePartition_Start(3);
    
    CorePartition_SetCurrentTimeInterface(getTimeTick);
    CorePartition_SetSleepTimeInterface(sleepTick);

    CreatePartition(Thread1, 150, 0);
    
    CreatePartition(Thread2, 150, 0);

    CreatePartition(Thread3, 150, 0);

}



void loop()
{
    join();
}
