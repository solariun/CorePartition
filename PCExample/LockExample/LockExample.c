#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include "CorePartition.h"

CpxSmartLock lock;

int nProducers [10];

void Procuder(void* pValue)
{
    size_t nID = CorePartition_GetID();
    
    nProducers [nID] = 0;
    
    while (true)
    {
        CorePartition_SharedLock(&lock);
        
        nProducers [nID]++;
        
        CorePartition_Yield();
        
        CorePartition_SharedUnlock(&lock);
    }

}

void Consumer(void* pValue)
{
    int nCount = 0;
        
    while (true)
    {
        CorePartition_Lock(&lock);
        
        printf ("Thread %zu: Values ", CorePartition_GetID());
        for (nCount=0; nCount < 10;nCount++)
        {
            printf ("(%u: [%d]) ", nCount, nProducers [nCount]);
        }
        printf (" LOCK: L:(%u), SL:(%zu)\n", lock.bExclusiveLock, lock.nSharedLockCounter);
          
        CorePartition_Sleep(1);
        
        CorePartition_Unlock(&lock);
        
        CorePartition_Yield();
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

    assert (CorePartition_Start (20));

    assert (CorePartition_SetStackOverflowHandler (StackOverflowHandler));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 100));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 333));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 444));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 555));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 280));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 160));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 777));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 777));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 1000));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 60000));

    assert (CorePartition_CreateThread (Consumer, NULL, 300, 2000));
    
    assert (CorePartition_CreateThread (Consumer, NULL, 300, 1000));

    assert (CorePartition_CreateThread (Consumer, NULL, 300, 500));
    
    assert (CorePartition_CreateThread (Consumer, NULL, 300, 3000));

    
    CorePartition_LockInit(&lock);
    
     CorePartition_Join();
}
