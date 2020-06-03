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


void Sleep (uint32_t nSleep)
{
    long nMomentum = getMiliseconds ();

    do
    {
        sleepUseconds (10000);
        CorePartition_Yield ();
    } while ((getMiliseconds () - nMomentum) < nSleep);
}

static void sleepMSTicks (uint32_t nSleepTime)
{
    usleep ((useconds_t)nSleepTime * 1000);
}

static uint32_t getMsTicks (void)
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    /* Get current timestamp in milliseconds */
    return (uint32_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}


unsigned int addOne (unsigned int nValue)
{
    nValue = nValue + 1;

    CorePartition_Yield ();

    return nValue;
}

void Thread1 (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nSleepTime = getMsTicks ();
    uint32_t nReturnedSleep = getMsTicks ();

    while (1)
    {
        printf (">> %lu:  Value: [%u], Nice: [%u] Sleep Time: [%u ms]\n",
                CorePartition_GetID (),
                nValue,
                CorePartition_GetNice (),
                nReturnedSleep - nSleepTime);
        nValue = nValue + 1;

        nSleepTime = getMsTicks ();
        CorePartition_Yield ();
        nReturnedSleep = getMsTicks ();
    }
}


static void StackOverflowHandler ()
{
    printf ("Error, Thread#%zu Stack %zu / %zu max\n", CorePartition_GetID (), CorePartition_GetStackSize (), CorePartition_GetMaxStackSize ());
}

int main (int argc, const char* argv[])
{
    if (CorePartition_Start (4) == false)
    {
        printf ("Error starting up Thread.");
        return (1);
    }

    assert (CorePartition_SetCurrentTimeInterface (getMsTicks));
    assert (CorePartition_SetSleepTimeInterface (sleepMSTicks));
    assert (CorePartition_SetStackOverflowHandler (StackOverflowHandler));

    assert (CorePartition_CreateThread (Thread1, NULL, 256, 500));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 600));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 1000));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 1500));

    CorePartition_Join ();


    return 0;
}
