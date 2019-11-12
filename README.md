# CorePartition

Version 2.3.1

![License information](https://raw.githubusercontent.com/solariun/CorePartition/master/License.png)

Partitioning a CORE into several Threads with a fast scheduler capable to be specialised through Tick and ticksleep interface, this way you can use nanoseconds, milliseconds or even real ticks to predict time, turning it into a powerful real time processor.  This lib was designed to work, virtually, with any modern micro controller or Microchip as long as it uses reverse bottom - up stack addressing, but was aiming single core processors and user space like MSDOS, linux applications, windows applications and Mac to allow desktop softwares and processor to split a core into functions and with a momentum scheduler.


# Preemption Ready 
NOW! CorePartition is Preemption ready a example of full preemption is already provided, including a full Thermal camera with Led Display example also with Preemption. NOTE that since it relays on Timer, it will not be part of the lib, you will have to implement the timer yourself, but a full example of how to do it is provided.


# Momentum Scheduller

*The Momentum Scheduler* is optimised to only allow thread to come back to work only upon its "nice" time or later that, with means it will work on real time as long as the developer keep all the functions clean. For some big logic, there will have two way to keep it peace for all the functions, using CorePartition_Yield, that will comply with the nice principle or CorePartition_Sleep that you can dynamically call a specialised nice. If you are using a Tick interface to work as milliseconds, nice will me n milliseconds, examples of how to do it is also provided for Desktop application and processor (through Arduino exemplo for keeping it simple).

HIGHLY suitable for Arduino (All official models included) as well, a .ino project is also provided with an example.

Be aware that the CorePartition Lib is a pico Virtual Core emulation, it will slice your CPU or user space application into n threads (partitions) and will create  a virtual stack page (the size of each stack page will be defined by the moment you create a partition), for each partition starting by a function already assigned to initiate the virtual core.

To calculate how much memory it will consume, start with the notion that each thread will consume around 60 ~ 170 bites depending on your target default bit channel size (8bits, 16bits, 32bits ... processor) plus the virtual stack page. 

Be AWARE comes with no warrant or guarantees, since I still have a limited number to target to test, but for those it is  fully functional till this point. If you intend  to use this code, please make a reference of me as its creator.  The comercial use is also permitted as long as I am notified and referenced in the code.

Tested at:

ESP8266 8 different boars including ESP-01

Arduino Nano (avr 328p)
Arduino Nano (avr 168) -> Thread.ino must have 2 threads due to memory
Arduino Nano (avr 168p) -> Thread.ino must have 2 threads due to memory

ATTiny85 (1Mhz / 512 bytes ram / 8K rom) -> Use BlinkThread.ino for testing

Sparkfun micro pro (Atmel32u4)

Arduino DUE (Arm m3)

Arduino MK ZERO (arm m0)

Arduino Genuino zero

STM32F103 (Bluepill)

MEGA2506 

MEGA1260

testes with I2C chain connections
tested with ISP chain connections 

tested and developed at OSX
tested at Linux
tested at Linux PI Zero, 1, 3 


This is how to use it 

```
#include "CorePartition.h"

void Thread1(void* pValue)
{
int nValue = 100;

while (1)
{
printf ("Thread1: Value [%d]\n", nValue++);

CorePartition_Yield();
}
}

void Thread2(void* pValue)
{
int nValue = 1000;

while (1)
{
printf ("Thread2: Value [%d]\n", nValue++);

CorePartition_Yield();
}
}


int main ()
{

CorePartition_Start (2);

//Every 1000 cycles with a Stack page of 210 bytes
CorePartition_CreateThread (Thread1, NULL,  210, 1000);

//All the time with a Stack page of 150 bytes
CorePartition_CreateThread (Thread2, NULL, 150, 0);

join();
}
```

inside your partitioned program (function) use the directive yield() to let the nano microkernel process next thread.

Please note it is not a regular thread, even though it behaves like one, it is a cooperative thread, once it will  let the programmer choose when to yield control control to other threads. 

# Arduino Boards

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic) and ATTINY85

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller

![GIF-2019-10-15-22-17-50](https://user-images.githubusercontent.com/1805792/66883029-7812a280-ef9a-11e9-9a61-f04ce62eb25f.gif)

ATTiny with 4 threads at 1Mhz

![regift](https://user-images.githubusercontent.com/1805792/67900756-1dae4000-fb5d-11e9-9cc4-b648c7680208.gif)

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller
![IMG_5502](https://user-images.githubusercontent.com/1805792/68585528-64b00580-047a-11ea-8d73-e45c9f3f441f.GIF)

ATmega238p 
![regift](https://user-images.githubusercontent.com/1805792/68585742-fb7cc200-047a-11ea-8ba5-3b9619c1962e.gif)

