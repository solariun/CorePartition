# CorePartition
Partitioning a CORE into several Threads with no prioritizing, this was designed to work, virutally, with any modern micro controller or Microchip as long it uses reverse bottom - up stack structure.

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

inside your partitioning program (function) use the directive yield() to let the nano microkernel process next partition.

Please not it is not a regular thread, even though it behaves like one, it is more closed related to a core virutalization but, letting the programmer choose when to let the virutal kernel to take control over its time processing.

Since it uses the directive yield(), it will create an advange using oficial arduino boards. The oficial code uses the yeld() directive inside every I/O calling and sleep, but, it implements it using a week function definition, which means, it can be overwritten by this CorePartitioning nano microlib technology, allowing arduino to run, almost, whithout the need to use the yeild() directive that will continue to be available. Some ports of arduino framework may or not implement the yield(), but in case they do, it will make your project even more portable.


