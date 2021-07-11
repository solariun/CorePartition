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
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "CorePartition.h"

uint32_t getMiliseconds ()
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    /* get current timestamp in milliseconds */
    return (uint32_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void sleepUseconds (uint32_t nTime)
{
    usleep ((useconds_t)nTime);
}

void Sleep (uint32_t nSleep)
{
    long nMomentum = getMiliseconds ();

    do
    {
        sleepUseconds (10000);
        Cpx_Yield ();
    } while ((getMiliseconds () - nMomentum) < nSleep);
}

unsigned int addOne (unsigned int nValue)
{
    nValue = nValue + 1;

    Cpx_Yield ();

    return nValue;
}

uint32_t nThreadValues[5];
uint32_t nThreadExecTimes[5];

const char topicValues[] = "thread/values";
const char topicExecTime[] = "thread/execTime";

void KernelBrokerHandler (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payload)
{
    (void) nSize;

    printf (">>> %s: context: [%s] Topic: [%s]\n", __FUNCTION__, (const char*) pContext, pszTopic);

    if (strncmp (pszTopic, topicValues, sizeof (topicValues) - 1) == 0)
    {
        nThreadValues[payload.nAttribute] = *((uint32_t*)payload.nValue);
    }
    else if (strncmp (pszTopic, topicExecTime, sizeof (topicExecTime) - 1) == 0)
    {
        /* please note that it works for 32bits or 64bits, otherwise use memory pointer */
        nThreadExecTimes[payload.nAttribute] = (uint32_t)payload.nValue;
    }
}


const char* messageTest = "Context";

void kernel (void* pValue)
{
    (void) pValue; 
    
    Cpx_EnableBroker ((void*) messageTest, 2, KernelBrokerHandler);

    Cpx_SubscribeTopic (topicValues, strlen (topicValues));
    Cpx_SubscribeTopic (topicExecTime, strlen (topicExecTime));

    while (1)
    {
        printf ("Kernel: [%zu], ID(execTime)[value]: ", Cpx_GetID ());

        {
            int nCount = 0;

            for (nCount = 0; nCount < 4; nCount++)
            {
                printf (" %u:(%u ms)[%u]", nCount, nThreadExecTimes[nCount], nThreadValues[nCount]);
            }
        }

        printf ("\n");

        Cpx_Yield ();
    }
}

void Thread1 (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nSleepTime = Cpx_GetCurrentTick ();
    uint32_t nReturnedSleep = Cpx_GetCurrentTick ();
    int32_t nFactor = 0;

    (void) pValue;

    while (1)
    {
        nFactor = ((nReturnedSleep - nSleepTime) - Cpx_GetNice ());
        printf (">> %lu:  Value: [%u], Nice: [%u] : Precision:[%d]\n",
                Cpx_GetID (),
                nValue,
                Cpx_GetNice (),
                nFactor);

        nValue = nValue + 1;

        Cpx_PublishTopic (topicValues, sizeof (topicValues) - 1, (size_t)Cpx_GetID (), (size_t)&nValue);

        nSleepTime = Cpx_GetCurrentTick ();
        Cpx_Yield ();
        nReturnedSleep = Cpx_GetCurrentTick ();

        Cpx_PublishTopic (topicExecTime, sizeof (topicExecTime)-1, (size_t)Cpx_GetID (), (size_t)(nReturnedSleep - nSleepTime));
    }
}

void PrintBinary (const uint8_t* pBuffer, size_t nSize)
{
    size_t nCount = 0;
    uint8_t nCountb = 0;
    uint8_t nValue = 0;

    for (nCount = 0; nCount < nSize; nCount++)
    {
        nValue = pBuffer[nCount];

        nCountb = 0;

        do
        {
            /*printf (" nCountb: [%u] - [%u]\n", nCountb, (1 << nCountb));*/
            if ((nValue & (1 << nCountb)) != 0)
                printf ("1");
            else
                printf ("0");
        } while (++nCountb < 8);
    }

    printf ("\n");
}


/*
 * Minimal CorePartition infra-structure
 * 
 *    ** REQUIRED **
 */

void Cpx_SleepTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t)nSleepTime * 1000);
}

uint32_t Cpx_GetCurrentTick (void)
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    /* Get current timestamp in milliseconds */
    return (uint32_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void Cpx_StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", Cpx_GetID (), Cpx_GetStackSize (), Cpx_GetMaxStackSize ());
}

/* ------------------------------------ */

int main ()
{
    if (Cpx_Start (10) == false)
    {
        printf ("Error starting up Thread.");
        return (1);
    }

    assert (Cpx_CreateThread (Thread1, NULL, 256, 150));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 323));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 764));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 1500));
    assert (Cpx_CreateThread (kernel, NULL, 256, 250));

    Cpx_Join ();

    return 0;
}
