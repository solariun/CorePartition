# CorePartition

Version 2.2

Partitioning a CORE into several Threads with a fast schedule that can be specialised using Tick and ticksleep interface, this way you can use nanoseconds, milliseconds or even real ticks to predict time, turning it into a powerful real time processor.  This lib was designed to work, virtually, with any modern micro controller or Microchip as long as it uses reverse bottom - up stack addressing.

This version now is compatible with C and C++ also HIGHLY suitable for Arduino (All official models included) as well, a .ino project is also provided with an example.

Be aware that the CorePartition Lib is a pico Virtual Core emulation, it will slice your CPU into n Process Partitions and will create  a virtual stack page (size defines by the moment you create a partition), for each partition starting by a function already assigned to initiate the virtual core.

To calculate how much memory it will consume, start with the notion that each thread will consume around 60 ~ 120 bites depending on your target default bit channel size (8bits, 16bits, 32bits ... processor) plus the virtual stack page. 

to calculate it sum the for each thread, use the chosen stack  page size + the ThreadLight struct internally used by Lib (this last will vary accordingly the chosen target)

Be AWARE comes with no warrant of guarantees, since I still have a limited number to target to test, but for those it is  fully functional till this point. If you intend  to use this code, please make a reference of me as its creator.  The comercial use is also permitted as long as I am notified and referenced in the code.

Tested at:

ESP8266 8 different boars including ESP-01
Arduino Nano (avr 328p)
Sparkfun micro pro (Atmel32u4)
Arduino DUE (Arm m3)
Arduino MK ZERO (arm m0)
Arduino Genuino zero
STM32F103 (Bluepill)
MEGA2506 
MEGA1260


OSX
Linux
Linux PI Zero, 1, 3 
STM32F1

This is how to use it 


int main ()
{

ThreadLight_Start(2);   
CreatePartition(Thread1, 210);
CreatePartition(Thread2, 220);

join();
}


inside your partitioned program (function) use the directive yield() to let the nano microkernel process next thread.

Please note it is not a regular thread, even though it behaves like one, it is more closed related to a core virtualisation but, letting the programmer choose when to allow the virtual kernel to take control over its time slot of processing. (Cooperative Thread alike)

Since it uses the directive yield(), it will create an advantage using oficial arduino boards. The oficial code uses the yield() directive inside every I/O calling and sleep procedures, but, it is implemented using a week function definition, which means, it can be overwritten by this CorePartitioning nano microlib technology, allowing arduino to run, almost, without the need of using the yield() directive that will continue to be available. Some ports of arduino framework may or not call the yield() inside their I/O and Sleep functions, but in case they do, it will make your project even more portable and feeling like preemptive cores.

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic)

