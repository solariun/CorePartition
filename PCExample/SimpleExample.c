#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#include "CorePartition.h"

CpxSmartLock lock;

int nProducers [10];

void Procuder(void* pValue)
{
    size_t nID = Cpx_GetID();
    
    nProducers [nID] = 0;
    
    while (true)
    {
        Cpx_SharedLock(&lock);
        
        nProducers [nID]++;
        
        Cpx_Yield();
        
        Cpx_SharedUnlock(&lock);
    }

}

void Consumer(void* pValue)
{
    int nCount = 0;
        
    while (true)
    {
        Cpx_Lock(&lock);
        
        printf ("Thread %zu: Values ", Cpx_GetID());
        for (nCount=0; nCount < 10;nCount++)
        {
            printf ("(%u: [%d]) ", nCount, nProducers [nCount]);
        }
        printf (" LOCK: L:(%u), SL:(%zu)\n", lock.bExclusiveLock, lock.nSharedLockCounter);
          
        Cpx_Sleep(1);
        
        Cpx_Unlock(&lock);
        
        Cpx_Yield();
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

    assert (Cpx_Start (20));

    assert (Cpx_SetStackOverflowHandler (StackOverflowHandler));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 100));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 333));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 444));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 555));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 280));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 160));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 777));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 777));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 1000));

    assert (Cpx_CreateThread (Procuder, NULL, 300, 60000));

    assert (Cpx_CreateThread (Consumer, NULL, 300, 2000));
    
    assert (Cpx_CreateThread (Consumer, NULL, 300, 1000));

    assert (Cpx_CreateThread (Consumer, NULL, 300, 500));
    
    assert (Cpx_CreateThread (Consumer, NULL, 300, 3000));

    
    Cpx_LockInit(&lock);
    
     Cpx_Join();
}
