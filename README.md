# CorePartitionOS

Version 2.7.0 release

![image](https://user-images.githubusercontent.com/1805792/125191254-6591cf80-e239-11eb-9e89-d7500e793cd4.png)

CorePartitionOS is a Operational System for, virtually, ANYTHING, but will always use a SINGLE CORE, really lightweight (context ranging from 48 bytes for 8bits processor/microcontroller to 168 bytes for 64 bits processor), compatible with 8, 32, 64 and 128 bits Processors and Microcontrollers and can virtually work EVERYWHERE. The concept behind the CorePartitionOS is a powerful implementations based exclusively on C standard principles, based on an non-stack displacement, allowing the code zero-assembler but deploying real threads.

This OS / Lib works entirely with **cooperative thread**, the developer will be able to control *when to change context*, so you can precise tune your code to use really small stack memory page, allowing more memory for your application. Since it relies on *cooperative threads* everything related to the function will be **atomic** while executing. CorePartitionOS, also, offers async and sync IPC's (*inter process communication*) using a state of the art *in kernel* message broker (Publish / Subscriber principles) and a sync wait /notify (one and all) using two principals TAGs (that resembles async topics) and Variables (based on pointer). Async messages offer a novel approach, while waiting your thread will be on 'lock' state, which means it will not resume unless the TAG or Variable (pointer based) is notified.


CorePartitionOS is based in a total novel approach, it relays entirely on ticks, but now, you can define what tick means, CoreParittionOS will allow you to use, seconds, microseconds, milliseconds or any measurement you want, just by implementing Cpx_GetCurrentTicks() and Cpx_SleepTicks. CorePartitionOS also have, a state of the art, scheduler based on ticks and not priority, meaning you say how often, in time, your thread will resume working, meaning that a thread will be always called, at least, at a constant pace, blinking a led, for example, would only need a Cpx_Yield(), to toggle your led in a constant pace.

CorePartition is a Cooperative Thread Operational System designed for performance, low memory usage.

# CorePartitionOS as Library

The CorePartitionOS also can transform your software into a multithread one, since the execution is highly manageable, you can create portable software that will execute similarly anywhere from small linux embedded targets to super computers, in the same way it will in a embedded application into a microcontroller. You can create full desktop based implementations, entirely detachable to a Hardware and, by using a abstraction Layer, you can create the logic and easily implement a HAL for any target you need. CorePartitionOS Library's Threads and inter process communications were designed to work in a OS in the same way it will in a embedded application, since they use the same code! :) This approach will decrease the need for target to test logic execution and, also, bringing a rare opportunity for automatized tests.

   It is compatible with Linux, Windows, Mac, Unix like operational systems and DOS

# Preemptive Threads

     This OS is now ONLY COOPERATIVE, like that we can focus on bringing all the power of Atomic execution to you.

# NEW
# 2.7.0
- NOW it has MIT license more free more options.

- All functions now starts with Cpx_ instead of CorePartition_.

- There will be no more SetStackOverflow, now just by using Cpx_StackOverflowHandler

- Full static implementation is now possible, processor, like old pics with no malloc now can fully benefit from it. Although, setjmp and longjmp still a requirement.

- Full Lock and SharedLock implementation, while locked, the thread is in a "kernel lock state" and can only be resumed uppon proper unlocking from other threads
     Cpx_Lock, Cpx_SharedLock, Cpx_Unlock, Cpx_SharedUnlock.

- Introducing VariableLock, based on Pointer, with that novel approach, any variable, can be used to notify (one or all) other threads. Alternatively transmitting a payload with Attribute and Value is possible.

- DeadLock is now detectable, join will leave if that condition arose and you can check with Cpx_GetNumberOfActiveThreads, it is returns zero a deadlock happened.

- Dropping support for secure threads, I will implement a more modular due to the next version.

- Dropping support to Preemptive Threads, you can still use the core with timer to trigger it, but, Locks are not tuned for that, be advised.

### Working in progress
     IPCs for interrupt request time. Soon a 2.7.1 will be released.

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

# Testimonials
![image](https://user-images.githubusercontent.com/1805792/125501709-538b5ebb-2234-493a-ba2b-dbfd9bf05b35.png)

# Arduino

As always reported, it is fully compatible with Arduino, any one, with Single Core or NO OS, to use it download the zip file from the master branch (https://github.com/solariun/CorePartition/archive/master.zip) and import it in at arduino -> Sketch -> Include Library -> Add ZIP Library... and you are good to go.

Arduino official documentation for adding libraries: https://www.arduino.cc/en/guide/libraries

# Important information

CorePartitionOS was design for single cores processors and micro controllers, since it will not switch between different cores, instead it will use one and slice it while  controlling it to deliver threads and deploy a full multi-thread environment. Note that CorePartitionOS uses only cooperative thread, enabling it to be able to use lightweight Threads without compromising the whole system, it is FULL ATOMIC.

Since CorePartitionOS is a Cooperative thread based system, developers will need to call Cpx_Yield(), Cpx_Wait(), Cpx_WaitMessage() or Cpx_Sleep(), Cpx_WaitVariableLock to trigger context switching.

All the examples are done using Arduino, why? First because its a HAL (Hardware Abstraction Layer) interface implemented by a lot manufactures, so, doesn't mater the processor or microcontroller, this Thread will deploy the same results, and will be ready for any hardware interaction: timer, interruption and architecture.

CorePartitionOS really deploys threads, it is not proto-thread, task lib or any re-entrant library, it is a fully thread implementation with memory page to isolate the thread context.


# Minimal C Resources

This lib uses NO ASSEMBLER, it will benefit from standard C (minimo of C99) principles, proving it is capable to create threads to any environment.

To compile this lib make sure your toolchain or software compiler works with standard C and provide

if no static calls are used:
     malloc
     free
Mandatory:
     setjmp
     longjmp


# Momentum Scheduler

*The Momentum Scheduler* is a Timer based Scheduler optimized to allow thread to come back to work only when its "nice" is due to, it means it will work on soft real time as long as the developer keep all the functions clean and fast and tuned. For some big logic, there will have two ways to keep it pace for all the functions, using Cpx_Yield, that will comply with the nice principle or Cpx_Sleep that you can dynamically call a specialized nice (requires a bit more stack). If you are using a Tick interface to work as milliseconds, nice will me n milliseconds, examples of how to do it is also provided for Desktop application and microcontrollers (through Arduino examples for keeping it simple).

HIGHLY suitable for Arduino (All official single cores models included) as well, sone .ino example projects are, also, provided.

To calculate how much memory it will consume, start with the notion that each thread will consume around (8btis)60 ~ 170 bites depending on your target default bit channel size (8bits, 16bits, 32bits ... processor) plus the virtual stack page. There are some functions that will help you calculate that:
     Cpx_GetStaticContextSize
     Cpx_GetStaticThreadSize

Be AWARE that this library comes with no warrant or guarantees, since I still have a limited number to target to test, but for those it is fully functional till this point. If you intend to use this code, please make a reference of me as its creator. The commercial use is also permitted and i do not demand third part code disclose as long as I am referenced in the code. It would be nice of you to notify me. If you want any tailored function or support get in contact: lgustavocampos@gmail.com


## Important

It is not mandatory the implementation of all momentum interfaces with a proper time/counter function. But I would strongly suggest you to, since you will be able to control time in a smooth manner. Otherwise you will use "kernel cycles" as ticks.

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

static void Cpx_StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", Cpx_GetID(), Cpx_GetStackSize(), Cpx_GetMaxStackSize());
}


int main ()
{

     assert (Cpx_Start (3));

     //Every 1000 cycles with a Stack page of 210 bytes
     assert (Cpx_CreateThread (Thread1, NULL,  210, 1000));

     //All the time with a Stack page of 150 bytes and
     //thread isolation
     assert (Cpx_CreateThread (Thread2, NULL, 150, 2000));

     assert (Cpx_CreateThread (Thread2, NULL, 150, 500));

     Cpx_Join();
}
```

Inside your partitioned program (function) use the directive Cpx_Yield() to let the nano microkernel switch context to the next thread, so do not forget to call Cpx_Yield() or Cpx_Lock, any Cpx_Wait* or use Cpx_Sleep() regularly.

# Arduino Boards

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic) and ATTINY85, but not limited to it, any Arduino board is compatible, note that CorePartitionOS only workigs with a Single Core, meaning, others cores will not be managed by it.

# Watchdog Notes
Some bords have a "untamed" watchdoge, specially ESP32, it requires a notification in a specific pace, so, if you want to use it, please make sure you understand your board Watchdog Polices.
   Ex: To proper use ESP32 with this OS, always implement specialized ticks and never use anything less than 1 millisecond.

# Some visual examples

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller

![GIF-2019-10-15-22-17-50](https://user-images.githubusercontent.com/1805792/66883029-7812a280-ef9a-11e9-9a61-f04ce62eb25f.gif)

ATTiny with 4 threads at 1Mhz

![regift](https://user-images.githubusercontent.com/1805792/67900756-1dae4000-fb5d-11e9-9cc4-b648c7680208.gif)

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller
![IMG_5502](https://user-images.githubusercontent.com/1805792/68585528-64b00580-047a-11ea-8d73-e45c9f3f441f.GIF)

ATmega238p
![regift](https://user-images.githubusercontent.com/1805792/68585742-fb7cc200-047a-11ea-8ba5-3b9619c1962e.gif)

