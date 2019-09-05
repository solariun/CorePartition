# CorePartition
Partitioning a CORE into several Threads with a simple prioritising scheduler (controlled by nice), this was designed to work, virtually, with any modern micro controller or Microchip as long it uses reverse bottom - up stack addressing

Be ware this Library will only work with C++, C will not use the needed memory model for creating and paining the threads.

Be aware that the CorePartitionLib is a pico Virtual Core emulation, it will slice your CPU into n Process Partitions and will create  a virtual stack page (size defines by the moment you create a partition) for each partition starting by the function it is using to initiate the virtual core.

To calculate how much memory it will consume, start with the notion that each thread will consume around 60 ~ 120 bites depending on your target default bit channel size plus the virtual stack page. 

to calculate it sum the for each thread, use the chosen stack  page size + the ThreadLight struct internally used by Lib (this last will vary accordingly the chosen target)

be AWARE it stills a draft, but fully functional till this point. If you intend  to use this code, please make a reference of me as its creator. the comercial use is permitted as long as I am notified and referenced.

Tested at:

ESP8266
Arduino Nano
Arduino DUE
Arduino MK ZERO
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


inside your partitioning program (function) use the directive yield() to let the nano microkernel process next partition.

Please note it is not a regular thread, even though it behaves like one, it is more closed related to a core virtualization but, letting the programmer choose when to allow the virtual kernel to take control over its time slot of processing.

Since it uses the directive yield(), it will create an advance using oficial arduino boards. The oficial code uses the yield() directive inside every I/O calling and sleep, but, it is implemented using a week function definition, which means, it can be overwritten by this CorePartitioning nano microlib technology, allowing arduino to run, almost, without the need of using the yield() directive that will continue to be available. Some ports of arduino framework may or not call the yield() inside their I/O and Sleep functions, but in case they do, it will make your project even more portable and feeling like preemptive cores.

This thread is HIGHLY SUITABLE for small arduinos like NANO (Works like magic)

