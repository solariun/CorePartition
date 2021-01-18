/*
//
//  main.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//
//               GNU GENERAL PUBLIC LICENSE
//                Version 3, 29 June 2007
//
// Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//
// Preamble
//
// The GNU General Public License is a free, copyleft license for
// software and other kinds of works.
//
// The licenses for most software and other practical works are designed
// to take away your freedom to share and change the works.  By contrast,
// the GNU General Public License is intended to guarantee your freedom to
// share and change all versions of a program--to make sure it remains free
// software for all its users.  We, the Free Software Foundation, use the
// GNU General Public License for most of our software; it applies also to
// any other work released this way by its authors.  You can apply it to
// your programs, too.
//
// See LICENSE file for the complete information
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
    int nCount = 0;

    printf ("------------------------------------\n");

    for (nCount = 0; nCount < Cpx_GetNumberOfActiveThreads (); nCount++)
    {
        printf ("%c %-4u %-10u %-10u  %u ms\n",
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

int main (int nArgs, const char* pszArg[])
{
    if (Cpx_Start (10) == false)
    {
        printf ("Error starting up Thread.");
        return (1);
    }

    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (Thread1, NULL, 256, 0));
    assert (Cpx_CreateThread (kernel, NULL, 256, 250));

    Cpx_Join ();

    return 0;
}
