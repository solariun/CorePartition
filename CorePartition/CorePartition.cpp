//
//  CorePartition.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//

#include "CorePartition.hpp"

#include <alloca.h>
#include <stdlib.h>


#define THREADL_NONE        0
#define THREADL_START       1
#define THREADL_RUNNING     2
#define THREADL_IDLE        3
#define THREADL_STOPPED     4

#define THREADL_ER_STACKOVFLW 1 //Stack Overflow

typedef struct
{
    uint8_t   nStatus = THREADL_NONE;
    
    uint8_t   nErrorType = THREADL_NONE;
    
    size_t    nStackMaxSize = 0;
    size_t    nStackSize = 0;
    jmp_buf   jmpJoinPointer;
    jmp_buf   jmpYieldPointer;
    
    void(*pFunction)();
    
    void*               pStartStck;
    void*               pLastStack;
    
    uint8_t*            pnStackPage = nullptr;
    
} ThreadLight;


static size_t nMaxThreads = 0;
static size_t nThreadCount = 0;
static size_t nCurrentThread;


static ThreadLight* pThreadLight = nullptr;
static ThreadLight* pCurrentThread = nullptr;


/*
 static void printStruct ()
 {
 pCurrentThread = &pThreadLight [nCurrentThread];
 
 printf (ThreadLight ID: (%lu) Struct\n, nCurrentThread);
 printf (\t        nStatus:  [%-17u]\n, pCurrentThread->nStatus);
 printf (\t     nErrorType:  [%-17u]\n\n, pCurrentThread->nErrorType);
 printf (\t  nStackMaxSize:  [%-17lu]\n, pCurrentThread->nStackMaxSize);
 printf (\t     nStackSize:  [%-17lu]\n, pCurrentThread->nStackSize);
 printf (\t     pStartStck:  [0x%-16lX]\n, (size_t) pCurrentThread->pStartStck);
 printf (\t     pLastStack:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pLastStack);
 printf (\t    pnStackPage:  [0x%-16lX]\n\n, (size_t) pCurrentThread->pnStackPage);
 
 printf (\t    strAssign:  [%-16s]\n\n,  pCurrentThread->strAssign);
 }
 */

bool CorePartition_Start (size_t nThreadPartitions)
{
    if (pThreadLight != nullptr || nThreadPartitions == 0) return false;
    
    nMaxThreads = nThreadPartitions;
    
    pThreadLight = (ThreadLight*) malloc (sizeof (ThreadLight) * nThreadPartitions);
    
    return true;
}



bool CreatePartition (void(*pFunction)(), size_t nStackMaxSize)
{
    if (nThreadCount >= nMaxThreads) return true;
    
    pThreadLight [nThreadCount].nStackMaxSize = nStackMaxSize;
    
    pThreadLight [nThreadCount].pnStackPage = (uint8_t*) malloc(sizeof (uint8_t) * pThreadLight [nThreadCount].nStackMaxSize);
    
    pThreadLight[nThreadCount].pFunction = pFunction;
    
    pThreadLight[nThreadCount].nStatus = THREADL_START;
    
    nThreadCount++;
    return true;
}


static inline void BackupStack()
{
    memcpy(pCurrentThread->pnStackPage, pCurrentThread->pLastStack, pCurrentThread->nStackSize);
}


static inline void RestoreStack()
{
    memcpy(pCurrentThread->pLastStack, pCurrentThread->pnStackPage, pCurrentThread->nStackSize);
}

void join ()
{
    if (nThreadCount == 0) return;
    
    do
    {
        pCurrentThread = &pThreadLight [nCurrentThread];
        
        //printStruct();
        
        if (pCurrentThread->nStatus != THREADL_NONE)
        {
            switch (pCurrentThread->nStatus)
            {
                case THREADL_START:
                    
                    if (setjmp(pCurrentThread->jmpJoinPointer) == 0)
                    {
                        pCurrentThread->nStatus = THREADL_RUNNING;
                        
                        pCurrentThread->pStartStck =  alloca(0);
                        pCurrentThread->pFunction ();
                        
                        
                        pCurrentThread->nStatus = THREADL_START;
                    }
                    break;
                    
                case THREADL_RUNNING:
                    
                    if (setjmp(pCurrentThread->jmpJoinPointer) == 0)
                    {
                        longjmp(pCurrentThread->jmpYieldPointer, 1);
                    }
                    
                    break;
            }
        }
    } while ((nCurrentThread = (nCurrentThread + 1 >= nMaxThreads) ? 0 : nCurrentThread+1)+1);
}


//void yield() __attribute__ ((noinline));

void yield()
{
    if (nThreadCount == 0) return;
    
    pCurrentThread->pLastStack = alloca(0);
    
    pCurrentThread->nStackSize = (size_t)pCurrentThread->pStartStck - (size_t)pCurrentThread->pLastStack;
    
    if (pCurrentThread->nStackSize > pCurrentThread->nStackMaxSize)
    {
        free (pCurrentThread->pnStackPage);
        pCurrentThread->nStatus = THREADL_STOPPED;
        
        longjmp(pCurrentThread->jmpJoinPointer, 1);
    }
    
    BackupStack();
    if (setjmp(pCurrentThread->jmpYieldPointer) == 0)
    {
        longjmp(pCurrentThread->jmpJoinPointer, 1);
    }
    
    //pCurrentThread->pLastStack = alloca(0);
    pCurrentThread->nStackSize = (size_t)pCurrentThread->pStartStck - (size_t)pCurrentThread->pLastStack;
    
    RestoreStack();
}


size_t getPartitionID()
{
    return nCurrentThread;
}


size_t getPartitionStackSize()
{
    return pCurrentThread->nStackSize;
}


size_t getPartitionMemorySize()
{
    return sizeof (ThreadLight);
}

