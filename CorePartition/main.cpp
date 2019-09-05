//
//  main.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//


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
        sleepUseconds (100000);
        yield();
    } while ((getMiliseconds() - nMomentum) < nSleep);
}



void Thread1 ()
{
    unsigned int nValue = 100;
    
    while (1)
    {
        printf (">> %lu:  Value: [%u]\n", getPartitionID(), nValue++);
        Sleep (665);
    }
}


void Thread2 ()
{
    unsigned int nValue = 200;
    
    setCoreNice(10);
    
    while (1)
    {
        printf ("** %lu:  Value: [%u]\n", getPartitionID(), nValue++);
        
        Sleep (1397);
    }
}



void Thread3 ()
{
    unsigned int nValue = 2340000;
    
    setCoreNice(100);
    
    while (1)
    {
        printf ("## %lu:  Value: [%u]\n", getPartitionID(), nValue++);
        
        Sleep (2000);
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
