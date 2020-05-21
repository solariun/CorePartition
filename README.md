# CorePartition

Version 2.5

![License information](https://raw.githubusercontent.com/solariun/CorePartition/master/License.png)

Partitioning a CORE into several Threads with a fast scheduler capable to be specialised through Tick and tick-sleep interface, this way you can use nanoseconds, milliseconds or even real ticks to predict time, turning it into a powerful real time processor.  This lib was designed to work, virtually, with any modern micro controller or Microchip as long as it uses reverse bottom - up stack addressing, but was aiming single core processors and user space like MSDOS, linux applications, windows applications and Mac to allow desktop softwares and processor to split a core into functions and with a momentum scheduler.

     This Thread Lig is a full Cooperative Thread Library, (it is not a task lib, it will really contextualize the thread and memory) that uses stack context and scheduler managements. It was design to work virtually into any processor, microcontroller but will perform amazingly as a thread for your software, using little memory and giving you a power and control need for complex projects.

     This lib can, also, perform as Preemptive, but since the intent is a universal lib, I will leave it up to you, just define a timer to call CorePartition_Yield () a good lock and it will run. There is a example of how to do it.

# NEW
# 2.5.1

     - Better memory management, 
     - Now using memory provisioning instead of full empty context, 
     - Better scheduler (lighter and faster),
     - Memory safe-critical principals
     - Better thread with stack
     - Speed and amazing stability improvements
     - Dynamic full stack size key for Secure Thread (Stack Isolation)
     - Now you can name a thread up to 8 characters
     - Experimental FreeRTOS support 

# FreeRTOS

     Now we are adding support for FreeRTOS, which means you can run cooperative and highly manageable threads over your existing FreeRTOS. To use it run it on the main after initializing all the threads. Do not mix CorePartition over different threads, it must stain confined in a single one.

# Arduino

    As always reported, it is fully compatible with Arduino, any one, to use it download it the zip file (https://github.com/solariun/CorePartition/archive/master.zip) and import it at arduino -> Sketch -> Include Library -> Add ZIP Library... and you are good to go.

    

# Important information

Core partition was design for single cores processors and micro controllers, since it will have a unique thread-context. But it i will also works as a lib for a  softwares, enabling it to be able to use Threads without compromising whole system.

By default it will use Cooperative thread, which mean the developer will need to call Yield() for changing context. But, by using a timer you will be able to make it preemptive. An example of perceptiveness is also provided for a micro controller Atmel 328P. Use Arduino IDE and a NANO for better results.

All the resources examples are done using Arduino, why? First because it will abstract the who hardware interface, so, doesn't mater the processor or microcontroler, this Thread will deploy the same results, since this is its philosophy (ready fir any hardware interaction, timer, interruption and architecture)

CoreParition really deploy threads, it is not proto-thread or any re-entrant thing, it is a fully thread implementation with memory page to isolate the thread context and even with a secure context (just introduced)


# Minimal Resources 

To compile this lib make sure your toolchain or software compiler works with standard C and provide  

memaloc
free
setjmp
longjmo
srand
rand

for 8bits processor it will approximately consume 41 bytes for thread controller and 47 bytes for each context + the memory page you choose to save your thread stack.  



# Preemption Ready 
NOW! CorePartition is Preemption ready a example of full preemption is already provided, including a full Thermal camera with Led Display example also with Preemption. NOTE that since it relays on Timer, it will not be part of the lib, you will have to implement the timer yourself, but a full example of how to do it is provided.


# Introducing Thread Isolation 

Now, CorePartition will introduce Thread Isolation, it will dynamically encrypt stack on back and restore of the memory page, it does not intend to be the best security, but one more barrier against digital threats. Every thread with Secure Memory Page, will be encrypted using a 128 bits key that will be dynamically changed every context switch. The developer will have no power or awareness of the procedure and the whole memory page will encrypted on memory.

Note that it will ONLY encrypt the stack, heap will remain original.

This feature will remain on Experimental for certain time.

# Momentum Scheduler

*The Momentum Scheduler* is optimise to only allow thread to come back to work only upon its "nice" time or later that, with means it will work on real time as long as the developer keep all the functions clean. For some big logic, there will have two way to keep it peace for all the functions, using CorePartition_Yield, that will comply with the nice principle or CorePartition_Sleep that you can dynamically call a specialized nice. If you are using a Tick interface to work as milliseconds, nice will me n milliseconds, examples of how to do it is also provided for Desktop application and processor (through Arduino exempla for keeping it simple).

HIGHLY suitable for Arduino (All official models included) as well, a .ino project is also provided with an example.

Be aware that the CorePartition Lib is a pico Virtual Core emulation, it will slice your CPU or user space application into n threads (partitions) and will create  a virtual stack page (the size of each stack page will be defined by the moment you create a partition), for each partition starting by a function already assigned to initiate the virtual core.

To calculate how much memory it will consume, start with the notion that each thread will consume around 60 ~ 170 bites depending on your target default bit channel size (8bits, 16bits, 32bits ... processor) plus the virtual stack page. 

Be AWARE comes with no warrant or guarantees, since I still have a limited number to target to test, but for those it is  fully functional till this point. If you intend  to use this code, please make a reference of me as its creator.  The commercial use is also permitted as long as I am notified and referenced in the code.


## Important
    
     If possible it is  HIGHLY RECOMMEND to implement the momentum with a proper time function. It will ensure stability and the developer will be able to use time to control thread process
   
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

Arduino NANO 33 SENCE nRF52840

STM32F103 (Bluepill)

MEGA2506 

MEGA1260

Sipeed Longan Nano (GD32VF103 32-bit rv32imac RISC-V “Bumblebee Core” @ 108 MHz)

Maix Bit Risc-V  

testes with I2C chain connections
tested with ISP chain connections 

tested and developed at OSX
tested at Linux
tested at Linux PI Zero, 1, 3 


If you want to start, what about you dust off a old arduino, like a nano, and open the thread.ino example that comes with resource and have a look at it?

# windows users

This thread has been developed and test on windows, mac, some Unix and Linux but it has been developed using a macintosh, so I am using some unix features like link and it is not available on windows. So, in order to have all the exemples running on windows, copy CorePartition.c and CorePartition.h to the directory you want, open the the ino file using your Arduino IDE, select your board and port and just flash it.  it is fully compatible with all arduinos.



# A Simple example

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

/*
 * I totally advise for the use of the 
 * momentum interface to setup a time measurement
 * for the CorePartition Kernel, also, some 
 * controllers like ESP required a realignment
 * that can be done by calling  sleep, so 
 * using it is highly recommended 
 * / 

static void sleepMSTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t) nSleepTime * 1000);
}

static uint32_t getMsTicks(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return (uint32_t) tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
}

static void StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", CorePartition_GetID(), CorePartition_GetStackSize(), CorePartition_GetMaxStackSize());
}
    

int main ()
{

     CorePartition_Start (2);

     CorePartition_SetCurrentTimeInterface(getMsTicks);
     CorePartition_SetSleepTimeInterface (sleepMSTicks);
     CorePartition_SetStackOverflowHandler (StackOverflowHandler);

     //Every 1000 cycles with a Stack page of 210 bytes
     CorePartition_CreateThread (Thread1, NULL,  210, 1000);

     //All the time with a Stack page of 150 bytes and
     //thread isolation
     CorePartition_CreateSecureThread (Thread2, NULL, 150, 0);

     join();
}
```

inside your partitioned program (function) use the directive yield() to let the nano microkernel process next thread.

Please note it is not a regular thread, even though it behaves like one, it is a cooperative thread, once it will  let the programmer choose when to yield control control to other threads. 

# Arduino Boards

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic) and ATTINY85 

But it is suitable for ALL ARDUINOS.... just try it out.... it will work !


# Some visual examples 

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller

![GIF-2019-10-15-22-17-50](https://user-images.githubusercontent.com/1805792/66883029-7812a280-ef9a-11e9-9a61-f04ce62eb25f.gif)

ATTiny with 4 threads at 1Mhz

![regift](https://user-images.githubusercontent.com/1805792/67900756-1dae4000-fb5d-11e9-9cc4-b648c7680208.gif)

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller
![IMG_5502](https://user-images.githubusercontent.com/1805792/68585528-64b00580-047a-11ea-8d73-e45c9f3f441f.GIF)

ATmega238p 
![regift](https://user-images.githubusercontent.com/1805792/68585742-fb7cc200-047a-11ea-8ba5-3b9619c1962e.gif)

