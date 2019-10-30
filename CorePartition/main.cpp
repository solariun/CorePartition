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
//Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
//Everyone is permitted to copy and distribute verbatim copies
//of this license document, but changing it is not allowed.
//
//Preamble
//
//The GNU General Public License is a free, copyleft license for
//software and other kinds of works.
//
//The licenses for most software and other practical works are designed
//to take away your freedom to share and change the works.  By contrast,
//the GNU General Public License is intended to guarantee your freedom to
//share and change all versions of a program--to make sure it remains free
//software for all its users.  We, the Free Software Foundation, use the
//GNU General Public License for most of our software; it applies also to
//any other work released this way by its authors.  You can apply it to
//your programs, too.
//
// See LICENSE file for the complete information


#include "CorePartition.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>




long getMiliseconds()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
}

void sleepUseconds(uint32_t nTime)
{
    usleep ((useconds_t) nTime);
}


void Sleep (uint64_t nSleep)
{
    uint64_t nMomentum =  getMiliseconds();
    
    do {
        //sleepUseconds (100000);
        CorePartition_Yield();
    } while ((getMiliseconds() - nMomentum) < nSleep);
}



void Thread1 ()
{
    unsigned int nValue = 100;
    
    while (1)
    {
        printf (">> %lu:  Value: [%u] - ScructSize: [%zu] - Memory: [%zu]\n", CorePartition_GetID(), nValue++, CorePartition_GetThreadStructSize(), CorePartition_GetAllocatedMemorySize());
        CorePartition_Yield (); //Sleep (10);
    }
}


void Thread2 ()
{
    unsigned int nValue = 200;
    
    //setCoreNice(10);
    
    while (1)
    {
        printf ("** %lu:  Value: [%u]\n", CorePartition_GetID(), nValue++);
        
        CorePartition_Yield(); //Sleep (10);
    }
}



void Thread3 ()
{
    unsigned int nValue = 2340000;
    
    //setCoreNice(200);
    
    while (1)
    {
        printf ("## %lu:  Value: [%u]\n", CorePartition_GetID(), nValue++);
        
        CorePartition_Yield(); //Sleep (10);
        
    }
}


static void sleepMSTicks (uint64_t nSleepTime)
{
    printf ("Sleep: %llu\n", nSleepTime);
    
    usleep ((useconds_t) nSleepTime * 1000);
}

static uint64_t getMsTicks(void)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return tp.tv_sec * 1000 + tp.tv_usec / 1000; //get current timestamp in milliseconds
}

int main(int argc, const char * argv[])
{
    uint64_t nValue = 0x00000000000000ffLL;
    int nCount = 0;

    
    CorePartition_Start(3);
    
    CorePartition_SetCurrentTimeInterface(getMsTicks);
    CorePartition_SetSleepTimeInterface (sleepMSTicks);
    
    CreatePartition(Thread1, 256, 3000);
    CreatePartition(Thread2, 256, 1000);
    CreatePartition(Thread3, 256, 2000);
    
    CorePartition_Join();
    
    
    return 0;
}
