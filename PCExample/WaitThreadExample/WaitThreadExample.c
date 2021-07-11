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


#include <stdlib.h>
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

char szTagName[] = "kernel/notify";

void PrintThreads ()
{
    size_t nCount = 0;

    printf ("------------------------------------\n");

    for (nCount = 0; nCount < Cpx_GetNumberOfThreads (); nCount++)
    {
        printf ("%c %-4zu %-10u %-10u  %u ms\n",
                nCount == Cpx_GetID () ? '*' : ' ',
                nCount,
                Cpx_GetNiceByID (nCount),
                Cpx_GetStatusByID (nCount),
                Cpx_GetLastDutyCycleByID (nCount));
    }

    printf ("------------------------------------\n");
}

void kernel (void* pValue)
{
    uint32_t nCurTime = Cpx_GetCurrentTick ();

    size_t nCounter = 0;

    (void) pValue;

    while (Cpx_Yield ())
    {
        nCurTime = Cpx_GetCurrentTick ();

        if (Cpx_NotifyMessageOne (szTagName, sizeof (szTagName) - 1, 1, nCounter++) == false)
        {
            printf ("\n>>> No waiting thread to notify.\n");
        }

        printf ("Kenrel Sleept for: %d ms", (Cpx_GetCurrentTick () - nCurTime));

    }
}

void Thread1 (void* pValue)
{
    CpxMsgPayload payload={0,0,0};

    (void) pValue;

    while (true)
    {
        if ((Cpx_WaitMessage (szTagName, sizeof (szTagName) - 1, &payload)) == false)
        {
            printf ("Error waiting for messages\n");
            exit(1);
        }
        else
        {
            printf ("Thread %zu: received a notification. payload: [%zu::%llu from %zu]\n",
                    Cpx_GetID (),
                    payload.nAttribute,
                    payload.nValue,
                    payload.nThreadID);
        }

        PrintThreads ();

        Cpx_Sleep (1000);
    }
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
        printf ("Error starting up Thread.\n");
        return (1);
    }

    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (kernel, NULL, 256, 250));

    printf ("Starting Threads... \n");

    Cpx_Join ();

    printf ("All threads stopped or where blocked.\n");

    return 0;
}
