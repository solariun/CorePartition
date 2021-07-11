/*
    MIT License

    Copyright (c) 2021 Gustavo Campos

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

#include "CorePartition.h"

CpxSmartLock lock;

int nProducers[10];

void Procuder (void* pValue)
{
    size_t nID = Cpx_GetID ();
    uint32_t nLastDuty = 0;

    (void) pValue;
    
    nProducers[nID] = 0;

    do
    {
        Cpx_SharedLock (&lock);

        nProducers[nID]++;

        /*
         * This will emulate preemption
         */
        Cpx_Sleep(0);

        Cpx_SharedUnlock (&lock);

        nLastDuty = Cpx_GetCurrentTick ();
    } while (Cpx_Yield ());
}

void Consumer (void* pValue)
{
    int nCount = 0;

    (void) pValue;

    do
    {
        Cpx_Lock (&lock);

        printf ("Thread %zu: Values ", Cpx_GetID ());

        for (nCount = 0; nCount < 10; nCount++)
        {
            printf ("(%u: [%d]) ", nCount, nProducers[nCount]);
        }

        printf (" LOCK: L:(%u), SL:(%zu) Running: [%zu]\n", lock.bExclusiveLock, lock.nSharedLockCount, Cpx_GetNumberOfActiveThreads ());

        fflush (stdout);

        /*
         * This will emulate preemption
         */
        Cpx_Sleep(0);

        Cpx_Unlock (&lock);

    } while (Cpx_Yield ());
}

void Cpx_SleepTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t)nSleepTime * 1000);
}

uint32_t Cpx_GetCurrentTick (void)
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    return (uint32_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;  /* get current timestamp in milliseconds */
}

void Cpx_StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", Cpx_GetID (), Cpx_GetStackSize (), Cpx_GetMaxStackSize ());
}

CpxStaticThread nConsumerContexts [4][Cpx_GetStaticThreadSize (300)];

CpxStaticThread nProducerContexts [10][Cpx_GetStaticThreadSize (300)];

int main ()
{
    assert (Cpx_Start (20));

    /*
     * PRODUCERS
     */

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [0], sizeof (nProducerContexts [0]), 10));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [1], sizeof (nProducerContexts [1]), 333));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [2], sizeof (nProducerContexts [2]), 444));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [3], sizeof (nProducerContexts [3]), 555));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [4], sizeof (nProducerContexts [4]), 280));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [5], sizeof (nProducerContexts [5]), 160));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [6], sizeof (nProducerContexts [6]), 777));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [7], sizeof (nProducerContexts [7]), 777));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [8], sizeof (nProducerContexts [8]), 1000));

    assert (Cpx_CreateStaticThread (Procuder, NULL, nProducerContexts [9], sizeof (nProducerContexts [9]), 60000));

    /*
     * CONSUMERS
     */

    assert (Cpx_CreateStaticThread (Consumer, NULL, nConsumerContexts [0], sizeof (nConsumerContexts [0]), 2000));

    assert (Cpx_CreateStaticThread (Consumer, NULL, nConsumerContexts [1], sizeof (nConsumerContexts [0]), 1000));

    assert (Cpx_CreateStaticThread (Consumer, NULL, nConsumerContexts [2], sizeof (nConsumerContexts [0]), 823));

    assert (Cpx_CreateStaticThread (Consumer, NULL, nConsumerContexts [3], sizeof (nConsumerContexts [0]), 3000));

    Cpx_LockInit (&lock);

    Cpx_Join ();

    printf ("----------------------------------------------------\n");
    printf ("System finished by dead lock or all threads has ended.\n");
    printf ("----------------------------------------------------\n");
}
