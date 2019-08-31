//
//  main.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//

#include <iostream>
#include "CorePartition.hpp"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


void Thread1 ()
{
    uint nValue = 100;
    
    while (1)
    {
        yield();
        printf ("%lu:  Value: [%u]\n", getPartitionID(), nValue++);
    }
}


long getUseconds()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return tp.tv_sec * 1000000 + tp.tv_usec; //get current timestamp in milliseconds
}

void sleepUseconds(uint32_t nTime)
{
    usleep ((useconds_t) nTime);
}


void Sleep (uint64_t nSleep)
{
    uint64_t nMomentum =  getUseconds();
    
    do {
        sleepUseconds (100);
        yield();
    } while ((getUseconds() - nMomentum) < nSleep);
}


void Thread2 ()
{
    uint nValue = 2340000;
    
    while (1)
    {
        yield();
        printf ("%lu:  Value: [%u]\n", getPartitionID(), nValue++);
        
        Sleep (10000);
    }
}



void Thread3 ()
{
    uint nValue = 2340000;
    
    while (1)
    {
        yield();
        printf ("%lu:  Value: [%u]\n", getPartitionID(), nValue++);
        
        Sleep (100);
    }
}



int main(int argc, const char * argv[])
{
    CorePartition_Start(3);
    
    CreatePartition(Thread1, 256);
    CreatePartition(Thread2, 256);
    CreatePartition(Thread3, 256);
    
    join();
    
    
    return 0;
}
