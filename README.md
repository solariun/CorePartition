# CorePartition
Partitioning a CORE into several Threads with no prioritizing, this was designed to work, virtally, with any modern micro controller or Microchip as long it uses reverse bottom - up stack addressing.

Be aware that the CorePartitionLib is a pico Virutal Core emulation, it will slice your CPU into n Porcess Partitions and will create  a virtual stack page for each partition starting by the function it is using to initiate the virutal core.

To calculate how much memory it will consume, start with the notion that each thread willl consume aroud 60 ~ 120 bites depending on your target bit channel size plus the virtual stack page. 

 to calculate it sum the bigger virtal stack page size used + each thread and its virual stack page (60)

be AWARE it stills a draft, but fully functional till this point. If you intend  to use this code, please make a reference of me as its creator. the comercial use is permitted as long as I am notifield and well referenced.

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

calculating size: 

bigger stack page: 220, each 60+210 and 60+220 

220 + (60+210) + (60 + 220) : 770 bites 

inside your partitioning program (function) use the directive yield() to let the nano microkernel process next partition.

Please note it is not a regular thread, even though it behaves like one, it is more closed related to a core virutalization but, letting the programmer choose when to allow the virutal kernel to take control over its time slot of processing.

Since it uses the directive yield(), it will create an advange using oficial arduino boards. The oficial code uses the yeld() directive inside every I/O calling and sleep, but, it is implemented using a week function definition, which means, it can be overwritten by this CorePartitioning nano microlib technology, allowing arduino to run, almost, whithout the need of using the yeild() directive that will continue to be available. Some ports of arduino framework may or not call the yield() inside their I/O and Sleep functions, but in case they do, it will make your project even more portable and feeling like preeptive cores.


