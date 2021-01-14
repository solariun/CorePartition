# CorePartitionOS 

Version 2.6.0 release

![License information](https://raw.githubusercontent.com/solariun/CorePartition/master/License.png)

CorePartitionOS is a Operational System for, virtually, ANY SINGLE CORE PROCESSOR, really lightweight (context ranging from 60bits on 8bits processors), compatible with 8bits, 32bits and 64bits Processors and Microcontrollers and can virtually work EVERYWHERE. The concept behind the CorePartitionOS is a powerful implementations based exclusively on C standard principles, you will not find any Assembler code, but do not be deceived about it, it truly deploys contextualized Threads.

Using Cooperative thread, the developer will be able to control when to change context, so you can precise tune your code to spend little thread memory page, allow more memory for your application and by consequence everything related to the function will be atomic. CorePartitionOS will also offer async messages using a state of the art in kernel message broker using Publish / Subscriber principles and a sync wait /notify (one and all) and sync messages with a novel approach, you will be able to lock a process using Wait that will block the thread to come back running till a notification arrives, that wait/notify structure is based on tags (like topics) and can also carry messages, just use notifyMessage and use WaitMessage for receiving it.

That CorePartitionOS uses a different scheduler, a Timer based one, which means you can specify when a thread will be good to go. Note that it is not a Rigid real time OS, instead, the timer will control the context switch procedure soon it is due to be executed and sleep the processor when nothing is due to be executed. 

PLEASE NOTE THAT:
This Thread Library is a full Cooperative Thread Library, (it is not a task lib, it will really contextualize the thread and memory) that uses stack context pages on heap and scheduler managements. It was design to work virtually into any single core processor, microcontroller but will perform amazingly as a thread for your software, using little memory and giving you a power and control need for complex projects.

# CorePartitionOS as Library

   The CorePartitionOS also can transform your software into a multithread one, since the execution is highly manageable, you can create portable softwares that will execute similarly anywhere from small linux embedded targets to super computers but also gives you something never seen before on a embedded OS, you can create the full implementation on your desktop, creating a detachable Hardware Abstraction Layer you can create the logic and easily implemente the HAL for any target you need, just leave the thread and inter process communication for CorePartitionOS Library. It will decrease the need for target to test logic execution and, also, brings a rare opportunity for automatized tests. 

   It is compatible with Linux, Windows, Mac, Unix like operational systems and DOS

# Preemptive Threads

This lib can, also, perform as Preemptive, but since the intent is a universal lib, I will leave it up to you, just define a timer to call Cpx_Yield () a good lock and it will run. There is a example of how to do it.



# NEW
# 2.6.1 

- Introducing Async Message Broker using Publish/Subscriber and topics to control async process intercommunications.
- Introducing Sync Wait / Notify thread blocking procedures using Tags as the lock identifier and optionally capable to send messages 
- Introducing Message Payload, messages will be always identified (what thread generated that) and with a (32bits)Attribute and (64bitds)Value, sou a single topic or tag can do more than just sending a simple message, can send parameters and values (tuple) across the system.
- Rewritten Scheduler to be adapt to the new functions.
- Dropping full FreeRTOS support. It means it still can work, but we will no longer officially support it executing it as part of the RTOS solution, only of Computers OS's

# 2.5.0 

- Better memory management, 
- Now using memory provisioning instead of full empty context, 
- Better scheduler (lighter and faster),
- Memory safe-critical principals
- Better thread with stack
- Speed and amazing stability improvements
- Dynamic full stack size key for Secure Thread (Stack Isolation)
- Now you can name a thread up to 8 characters
- Experimental FreeRTOS support 


# Arduino

As always reported, it is fully compatible with Arduino, any one, with Single Core or NO OS, to use it download the zip file from the master branch (https://github.com/solariun/CorePartition/archive/master.zip) and import it in at arduino -> Sketch -> Include Library -> Add ZIP Library... and you are good to go.

Arduino official documentation for adding libraries: https://www.arduino.cc/en/guide/libraries

# Important information

CorePartitionOS was design for single cores processors and micro controllers, since it will not switch between different cores, instead it will use one and slice it while  controling it to create  threads, deploying a full multi thread environment. Note that it will also create cooperative thread for softwares, enabling it to be able to use lightweight Threads without compromising the whole system.

By default it will use Cooperative thread, which means the developer will need to call Cpx_Yield(), Cpx_Wait(), Cpx_WaitMessage() or Cpx_Sleep() to trigger context switching. But, by using a timer you will be able to make it preemptive. An example of prenptivess is also provided for a micro controller Atmel 328Pm, run it at Arduino IDE and a NANO boards.

All the examples are done using Arduino, why? First because it will act as a HAL (Hardware Abstraction Layer) interface, so, doesn't mater the processor or microcontroller, this Thread will deploy the same results, and will be ready for any hardware interaction: timer, interruption and architecture.

CorePartitionOS really deploys threads, it is not proto-thread, task lib or any re-entrant thing, it is a fully thread implementation with memory page to isolate the thread context and even with a secure context (just introduced)


# Minimal C Resources

This lib uses NO ASSEMBLER, it will benefit from standard C (minimo of C99) principles, proving it is capable to create threads to any environment.

To compile this lib make sure your toolchain or software compiler works with standard C and provide

malloc
free
setjmp
longjmp
srand
rand

for 8bits processor it will approximately consume 41 bytes for thread controller and 57 bytes for each context + the memory page you choose to save your thread stack.  



# Preemption Ready 
CorePartition is Preemption ready, two examples of full preemption is already provided, including a full Thermal camera with Led Display example also with Preemption (Both example was done using Arduino NANO (AVR Atmel328P). NOTE that since it relays on Timer, it will not be part of the lib, you will have to implement the timer yourself, but a full example of how to do it is provided.


# Introducing Thread Isolation 

CorePartition will introduce Thread Isolation, it will dynamically encrypt stack on the backup and restore of the thread stack memory page, it does not intend to be the best security, but one more barrier against digital threats since it will create noice to the power fluctuation. Every thread with Secure Memory Page, will be encrypted using a key with the same size of the context and dynamically changed on every context switch. The developer will have no power or awareness of the procedure and the whole memory page will encrypted on memory with fully dynamic key that changes on every context switch.

Note that it will ONLY encrypt the stack, heap will remain original.
This feature will remain on Experimental for certain time.

# Momentum Scheduler

*The Momentum Scheduler* is Timer based Scheduler optimised to only allow thread to come back to work only its "nice" time or later than that, it means it will work on soft real time as long as the developer keep all the functions clean and fast till the point it call context for a context switch. For some big logic, there will have two way to keep it peace for all the functions, using Cpx_Yield, that will comply with the nice principle or Cpx_Sleep that you can dynamically call a specialized nice. If you are using a Tick interface to work as milliseconds, nice will me n milliseconds, examples of how to do it is also provided for Desktop application and processor (through Arduino exempla for keeping it simple).

HIGHLY suitable for Arduino (All official single cores models included) as well, a .ino project is also provided with an example.

Be aware that the CorePartitionOS Lib is a pico Virtual Core emulation, it will slice your CPU or user space application into n threads (partitions) and will create  a virtual stack page (the size of each stack page will be defined by the moment you create a partition), for each partition starting by a function already assigned to initiate the virtual core.

To calculate how much memory it will consume, start with the notion that each thread will consume around (8btis)60 ~ 170 bites depending on your target default bit channel size (8bits, 16bits, 32bits ... processor) plus the virtual stack page. 

Be AWARE that it comes with no warrant or guarantees, since I still have a limited number to target to test, but for those it is fully functional till this point. If you intend  to use this code, please make a reference of me as its creator.  The commercial use is also permitted as long as I am notified and referenced in the code.


## Important
    
It is mandatory the implementation of all momentum interfaces with a proper time/counter function. It will ensure stability and the developer will be able to use time to control thread process, examples of it is provided here and in every example.
   
Tested at:

ESP8266 8 different boars including ESP-01

ESP32 - No OS, also tested with Spressif RTOS but just for fun, I would not use it, use it with ESP8266 (FULLY compatible) or without FREERTOS, but it will only use one core.

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

MEGA2560 

MEGA1280

Sipeed Longan Nano (GD32VF103 32-bit rv32imac RISC-V “Bumblebee Core” @ 108 MHz)

Maix Bit Risc-V  

testes with I2C chain connections
tested with ISP chain connections 

tested and developed at OSX
tested at Linux
tested at Linux PI Zero, 1, 3 
tested on Windows 

tested on BeOS
tested on HPUX
tested on Solaris


If you want to start, what about you dust off a old arduino, like a nano, and open the thread.ino or LowMememryExample.ino example that comes with examples and have a look at it?


# A Simple example

This is how to use it 

```
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include "CorePartition.h"

void Thread1(void* pValue)
{
     int nValue = 0;

     while (Cpx_Yield())
     {
          printf ("Thread %zu: Value [%d] every %u ms\n", Cpx_GetID(), nValue++, Cpx_GetNice());
     }
}

void Thread2(void* pValue)
{
     int nValue = 0;

     while (Cpx_Yield())
     {
          printf ("Thread %zu: Value [%d] every %u ms\n", Cpx_GetID(), nValue++, Cpx_GetNice());
     }
}

void Cpx_SleepTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t) nSleepTime * 1000);
}

uint32_t Cpx_GetCurrentTick(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return (uint32_t) tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
}

static void StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", Cpx_GetID(), Cpx_GetStackSize(), Cpx_GetMaxStackSize());
}
    

int main ()
{

     assert (Cpx_Start (3));

     assert (Cpx_SetStackOverflowHandler (StackOverflowHandler));

     //Every 1000 cycles with a Stack page of 210 bytes
     assert (Cpx_CreateThread (Thread1, NULL,  210, 1000));

     //All the time with a Stack page of 150 bytes and
     //thread isolation
     assert (Cpx_CreateSecureThread (Thread2, NULL, 150, 2000));

     assert (Cpx_CreateSecureThread (Thread2, NULL, 150, 500));

     Cpx_Join();
}
```

Inside your partitioned program (function) use the directive Cpx_Yield() to let the nano microkernel process next thread, so do not forget to call Cpx_Yield() or use Cpx_Sleep() regularly.

# Arduino Boards

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic) and ATTINY85 

But it is suitable for ALL single core ARDUINOs.... just try it out.... it will work !


# Some visual examples 

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller

![GIF-2019-10-15-22-17-50](https://user-images.githubusercontent.com/1805792/66883029-7812a280-ef9a-11e9-9a61-f04ce62eb25f.gif)

ATTiny with 4 threads at 1Mhz

![regift](https://user-images.githubusercontent.com/1805792/67900756-1dae4000-fb5d-11e9-9cc4-b648c7680208.gif)

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller
![IMG_5502](https://user-images.githubusercontent.com/1805792/68585528-64b00580-047a-11ea-8d73-e45c9f3f441f.GIF)

ATmega238p 
![regift](https://user-images.githubusercontent.com/1805792/68585742-fb7cc200-047a-11ea-8ba5-3b9619c1962e.gif)

