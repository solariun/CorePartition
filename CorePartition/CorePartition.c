//
//  CorePartition.cpp
//  CorePartition
//
//  Created by GUSTAVO CAMPOS on 14/07/2019.
//  Copyright © 2019 GUSTAVO CAMPOS. All rights reserved.
//

#include "CorePartition.h"

#include <stdlib.h>

#define THREADL_NONE        0
#define THREADL_START       1
#define THREADL_RUNNING     2
#define THREADL_IDLE        3
#define THREADL_STOPPED     4
#define THREADL_SWITCHING   5

#define THREADL_ER_STACKOVFLW 1 //Stack Overflow



typedef struct
{
    uint8_t   nStatus;
    
    uint8_t   nErrorType;
    
    size_t    nStackMaxSize;
    size_t    nStackSize;
    
    jmp_buf   jmpJoinPointer;
    jmp_buf   jmpYieldPointer;
    
    void(*pFunction)(void);
    
    uint8_t             nNice;
    
    void*               pStartStck;
    void*               pLastStack;
    
    uint8_t*            pnStackPage;
    
} ThreadLight;


static volatile size_t nMaxThreads = 0;
static volatile size_t nThreadCount = 0;
static volatile size_t nCurrentThread;
static volatile size_t nStartedCores=0;


static ThreadLight* pThreadLight = NULL;
static ThreadLight* pCurrentThread = NULL;


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
    if (pThreadLight != NULL || nThreadPartitions == 0) return false;
    
    nMaxThreads = nThreadPartitions;
    
    pThreadLight = (ThreadLight*) malloc (sizeof (ThreadLight) * nThreadPartitions);
    
    return true;
}



bool CreatePartition (void(*pFunction)(void), size_t nStackMaxSize)
{
    if (nThreadCount >= nMaxThreads || pFunction == NULL) return false;
    
    pThreadLight[nThreadCount].nStatus = THREADL_START;

    pThreadLight[nThreadCount].nErrorType = THREADL_NONE;

    pThreadLight [nThreadCount].nStackMaxSize = nStackMaxSize;
    
    pThreadLight[nThreadCount].nStackSize = 0;

    pThreadLight[nThreadCount].pFunction = pFunction;
    
    pThreadLight[nThreadCount].nNice = 1;

    pThreadLight [nThreadCount].pnStackPage = (uint8_t*) malloc(sizeof (uint8_t) * pThreadLight [nThreadCount].nStackMaxSize);
    
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



static inline size_t Scheduler ()
{
    static size_t nCounter = 0;
    
    while (1)
    {
        if (++nCurrentThread <= nMaxThreads)
        {
            if (nCounter % pThreadLight [nCurrentThread].nNice == 0)
            {
                return nCurrentThread;
            }
        }
        else
        {
            nCurrentThread = -1; nCounter++;
        }
    }
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
                        volatile uint8_t nValue = 0xAA;
                        pCurrentThread->pStartStck =  (void*) &nValue;
                        pCurrentThread->nStatus = THREADL_RUNNING;
                        
                        nStartedCores++;
                        
                        pCurrentThread->pFunction ();
                        
                        
                        pCurrentThread->nStatus = THREADL_STOPPED;
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
    } while ((nCurrentThread = Scheduler())+1);
    //} while ((nCurrentThread = (nCurrentThread + 1 >= nMaxThreads) ? 0 : nCurrentThread+1)+1);
}



//void yield() __attribute__ ((noinline));

void yield()
{
    
    if (nThreadCount == 0) return;
    
    volatile uint8_t nValue = 0xBB;
    pCurrentThread->pLastStack = (void*) &nValue;
    
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



bool isAllCoresStarted(void)
{
    return nStartedCores == nMaxThreads;
}


bool isCoreRunning(void)
{
    return pCurrentThread->nStatus == THREADL_RUNNING;
}


bool getCoreNice()
{
    return pCurrentThread->nNice;
}


void setCoreNice (uint8_t nNice)
{
    pCurrentThread->nNice = nNice == 0 ? 1 : nNice;
}
