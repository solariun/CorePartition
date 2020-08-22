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

unsigned int addOne (unsigned int nValue)
{
    nValue = nValue + 1;

    CorePartition_Yield ();

    return nValue;
}

uint32_t nThreadValues[5];
uint32_t nThreadExecTimes[5];

const char topicValues[] = "thread/values";
const char topicExecTime[] = "thread/execTime";

void KernelBrokerHandler (void* pContext, const char* pszTopic, size_t nSize, CpxMsgPayload payload)
{
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
    CorePartition_EnableBroker ((void*) messageTest, 2, KernelBrokerHandler);

    CorePartition_SubscribeTopic (topicValues, strlen (topicValues));
    CorePartition_SubscribeTopic (topicExecTime, strlen (topicExecTime));

    while (1)
    {
        printf ("Kernel: [%zu], ID(execTime)[value]: ", CorePartition_GetID ());

        {
            int nCount = 0;

            for (nCount = 0; nCount < 4; nCount++)
            {
                printf (" %u:(%u ms)[%u]", nCount, nThreadExecTimes[nCount], nThreadValues[nCount]);
            }
        }

        printf ("\n");

        CorePartition_Yield ();
    }
}

void Thread1 (void* pValue)
{
    uint32_t nValue = 0;
    uint32_t nSleepTime = CorePartition_GetCurrentTick ();
    uint32_t nReturnedSleep = CorePartition_GetCurrentTick ();
    int32_t nFactor = 0;

    while (1)
    {
        nFactor = ((nReturnedSleep - nSleepTime) - CorePartition_GetNice ());
        printf (">> %lu:  Value: [%u], Nice: [%u] : Precision:[%d]\n",
                CorePartition_GetID (),
                nValue,
                CorePartition_GetNice (),
                nFactor);

        nValue = nValue + 1;

        CorePartition_PublishTopic (topicValues, sizeof (topicValues) - 1, (size_t)CorePartition_GetID (), (size_t)&nValue);

        nSleepTime = CorePartition_GetCurrentTick ();
        CorePartition_Yield ();
        nReturnedSleep = CorePartition_GetCurrentTick ();

        CorePartition_PublishTopic (topicExecTime, sizeof (topicExecTime)-1, (size_t)CorePartition_GetID (), (size_t)(nReturnedSleep - nSleepTime));
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

    assert (CorePartition_CreateThread (Thread1, NULL, 256, 150));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 323));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 764));
    assert (CorePartition_CreateThread (Thread1, NULL, 256, 1500));
    assert (CorePartition_CreateSecureThread (kernel, NULL, 256, 250));

    CorePartition_Join ();

    return 0;
}
