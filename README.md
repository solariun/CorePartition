# CorePartition

Version 2.2

![License information](https://raw.githubusercontent.com/solariun/CorePartition/master/License.png)

Partitioning a CORE into several Threads with a fast scheduler capable to be specialised through Tick and ticksleep interface, this way you can use nanoseconds, milliseconds or even real ticks to predict time, turning it into a powerful real time processor.  This lib was designed to work, virtually, with any modern micro controller or Microchip as long as it uses reverse bottom - up stack addressing, but was aiming single core processors and user space like MSDOS, linux applications, windows applications and Mac to replace pthread for a more controled execution envirionment. 

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
void Thread1()
{
    int nValue = 100;
    
    while (1)
    {
        printf ("Thread1: Value [%d]\n", nValue++);
        
        yield();
    }
}

void Thread2()
{
    int nValue = 1000;

    while (1)
    {
        printf ("Thread2: Value [%d]\n", nValue++);
        
        yield();
    }
}


int main ()
{

    ThreadLight_Start(2);
    
    //Every 1000 cycles with a Stack page of 210 bytes
    CreatePartition(Thread1, 210, 1000);
    
    //All the time with a Stack page of 150 bytes
    CreatePartition(Thread2, 150, 0);

    join();
}
```

inside your partitioned program (function) use the directive yield() to let the nano microkernel process next thread.

Please note it is not a regular thread, even though it behaves like one, it is a cooperative thread, once it will  let the programmer choose when to yeld control control to other threads. 

Since it uses the directive yield(), it will create an advantage using oficial arduino boards. The oficial code uses the yield() directive inside every I/O blockings and sleep procedures. Arduino implements a  null  yeld  using a week function definition, which means, it will be override by this CorePartitioning nano microlib technology, allowing arduino to run, almost, without the need of using the yield() directive that will continue to be available. Some ports of arduino framework may or not call the yield() inside their I/O and Sleep functions, but in case they do, it will make your project even more portable and feeling like preemptive cores.

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic) and ATTINY85

ATmega238p with Thermal cam (I2C) and 2 DotMatrix 8x8 ISP chained. 3 threads 1: reading cam, 2: showing cam, 3-Text Scroller

![GIF-2019-10-15-22-17-50](https://user-images.githubusercontent.com/1805792/66883029-7812a280-ef9a-11e9-9a61-f04ce62eb25f.gif)

ATTiny with 4 threads at 1Mhz

![regift](https://user-images.githubusercontent.com/1805792/67900756-1dae4000-fb5d-11e9-9cc4-b648c7680208.gif)
