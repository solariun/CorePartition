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
    
    do
    {
        CorePartition_SharedLock(&lock);
        
        nProducers [nID]++;
        
        CorePartition_Sleep(0);
        
        CorePartition_SharedUnlock(&lock);
        
    } while (CorePartition_Yield());

}

void Consumer(void* pValue)
{
    int nCount = 0;
        
    do
    {
        CorePartition_Lock(&lock);
        
        printf ("Thread %zu: Values ", CorePartition_GetID());
        for (nCount=0; nCount < 10;nCount++)
        {
            printf ("(%u: [%d]) ", nCount, nProducers [nCount]);
        }
        
        printf (" LOCK: L:(%u), SL:(%zu)\n", lock.bExclusiveLock, lock.nSharedLockCount);
          
        //CorePartition_Sleep(0);
        
        CorePartition_Unlock(&lock);
        
    } while (CorePartition_Yield());
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

    /*
     * PRODUCERS
     */
    
    assert (CorePartition_CreateThread (Procuder, NULL, 300, 1));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 333));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 444));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 555));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 280));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 160));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 777));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 777));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 1000));

    assert (CorePartition_CreateThread (Procuder, NULL, 300, 60000));

    /*
     * CONSUMERS
     */

    assert (CorePartition_CreateThread (Consumer, NULL, 300, 2000));

    assert (CorePartition_CreateThread (Consumer, NULL, 300, 1000));

    assert (CorePartition_CreateThread (Consumer, NULL, 300, 1000));
    
//    assert (CorePartition_CreateThread (Consumer, NULL, 300, 3000));

    
    CorePartition_LockInit(&lock);
    
     CorePartition_Join();
    
    printf ("----------------------------------------------------\n");
    printf ("System finished, dead lock or all threads has ended.\n");
    printf ("----------------------------------------------------\n");
}
