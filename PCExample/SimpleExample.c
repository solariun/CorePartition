#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include "CorePartition.h"

void Thread1(void* pValue)
{
     int nValue = 0;

     while (CorePartition_Yield())
     {
          printf ("Thread %zu: Value [%d] every %u ms\n", CorePartition_GetID(), nValue++, CorePartition_GetNice());
     }
}

void Thread2(void* pValue)
{
     int nValue = 0;

     while (CorePartition_Yield())
     {
          printf ("Thread %zu: Value [%d] every %u ms\n", CorePartition_GetID(), nValue++, CorePartition_GetNice());
     }
}

void CorePartition_SleepTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t) nSleepTime * 1000);
}

uint32_t CorePartition_GetCurrentTick(void)
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

     assert (CorePartition_Start (3));

     assert (CorePartition_SetStackOverflowHandler (StackOverflowHandler));

     //Every 1000 cycles with a Stack page of 210 bytes
     assert (CorePartition_CreateThread (Thread1, NULL,  210, 1000));

     //All the time with a Stack page of 150 bytes and
     //thread isolation
     assert (CorePartition_CreateSecureThread (Thread2, NULL, 150, 2000));

     assert (CorePartition_CreateSecureThread (Thread2, NULL, 150, 500));

     CorePartition_Join();
}