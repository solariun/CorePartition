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

    for (nCount = 0; nCount < CorePartition_GetNumberOfActiveThreads (); nCount++)
    {
        printf ("%-4u %-10s %-10u %u\n",
                nCount,
                CorePartition_GetThreadNameByID (nCount),
                CorePartition_GetNiceByID (nCount),
                CorePartition_GetStatusByID (nCount));
    }

    printf ("------------------------------------\n");
}

void kernel (void* pValue)
{
    uint32_t nCurTime = CorePartition_GetCurrentTick ();
    CorePartition_SetThreadName ("Kernel", 6);

    while (1)
    {
        nCurTime = CorePartition_GetCurrentTick ();
        CorePartition_NotifyOne (szTagName, sizeof (szTagName) - 1);
        printf ("Kenrel Sleept for: %d", (CorePartition_GetCurrentTick () - nCurTime));

        PrintThreads ();
    }
}

void Thread1 (void* pValue)
{
    while (1)
    {
        CorePartition_Wait (szTagName, sizeof (szTagName) - 1);
        printf ("Thread %zu: received a notification.\n", CorePartition_GetID ());
        CorePartition_Sleep (1000);
    }
}

/*
 * Minimal CorePartition infra-structure
 *
 *    ** REQUIRED **
 */

void CorePartition_SleepTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t)nSleepTime * 1000);
}

uint32_t CorePartition_GetCurrentTick (void)
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    /* Get current timestamp in milliseconds */
    return (uint32_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", CorePartition_GetID (), CorePartition_GetStackSize (), CorePartition_GetMaxStackSize ());
}

/* ------------------------------------ */

int main (int nArgs, const char* pszArg[])
{
    if (CorePartition_Start (10) == false)
    {
        printf ("Error starting up Thread.");
        return (1);
    }

    assert (CorePartition_SetStackOverflowHandler (StackOverflowHandler));

    assert (CorePartition_CreateThread (Thread1, NULL, 256, 0));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 0));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 0));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 0));
    assert (CorePartition_CreateThread (kernel, NULL, 256, 250));

    CorePartition_Join ();

    return 0;
}
