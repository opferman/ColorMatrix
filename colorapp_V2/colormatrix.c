/***************************************************************************** 
 *                            Color Matrix                                   *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2018           Toby Opferman                              *
 *****************************************************************************/

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <threadpool.h>
#include "colormatrix.h"



#define INVALID_THREAD_ID (-1)
#define COLOR_QUEUE_DEPTH 30

#pragma pack(1)
typedef struct _COLOR_NODE_DATA {

    ULONG64 ThreadId : 32;
    ULONG64 ColorInvalid : 1;
    ULONG64 Reserved  : 31;

} COLOR_NODE_DATA, *PCOLOR_NODE_DATA;

#pragma pack(1)
typedef struct _COLOR_NODE {

    union {
        COLOR_NODE_DATA Data;
        ULONG64 AsUlong64;
    } u;

} COLOR_NODE, *PCOLOR_NODE;

typedef struct _BUCKET_COUNT {

    ULONG Count;
    ULONG RedirectionThreadId;

} BUCKET_COUNT, *PBUCKET_COUNT;

#pragma pack(1)
typedef struct _PACKED_CONTEXT_DATA {

    USHORT IndexX : 16;
    USHORT IndexY : 16;
    ULONG ThreadId : 32;

} PACKED_CONTEXT_DATA, *PPACKED_CONTEXT_DATA;

#pragma pack(1)
typedef struct _PACKED_CONTEXT {

    union {
        PACKED_CONTEXT_DATA Packed;
        ULONG64 AsUlong64;
    } u;

} PACKED_CONTEXT, *PPACKED_CONTEXT;

#pragma pack(1)
typedef struct _MATRIX_TRACKER_INTERNAL {

    union {
        ULONG64 ColorNodeAsULONG64;
        COLOR_NODE ColorNode;
    } Color;

} MATRIX_TRACKER_INTERNAL, *PMATRIX_TRACKER_INTERNAL;

#define NEIGHBOR_CHUNK_ALLOC  20

typedef struct _THREAD_CONTEXT {

    ULONG Count;
    PACKED_CONTEXT PackedContext;
    struct _THREAD_CONTEXT *pNextThread;

    ULONG CurrentNeighbors;
    ULONG MaxNeighbors;
    ULONG *pNeighbors;

} THREAD_CONTEXT, *PTHREAD_CONTEXT;


typedef struct _MATRIX_INTERNAL_CONTEXT {

    PMATRIX_TRACKER_INTERNAL pMatrixTracker;
    PCOLOR_MATRIX pColorMatrix;
    ULONG64 ThreadCount;
	ULONG64 ThreadIds;
	ULONG64 PixelsExamined;
	ULONG64 TotalPixels;
    HANDLE hWaitThreadsComplete;
    ULONG NumberOfWorkerThreads;
	HTHREADPOOL hThreadPool;
	PTHREAD_CONTEXT pThreadHead;

} MATRIX_INTERNAL_CONTEXT, *PMATRIX_INTERNAL_CONTEXT;

/*
 * Internal Functions
 */
ULONG ColorMatrix_Internal_GetProcessorCount(void);
BOOL ColorMatrix_Internal_ColorCompleted(HTHREADPOOL hThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem, PVOID pContext, BOOL bCanceled);
BOOL ColorMatrix_Internal_ColorCounterWorkItem(HTHREADPOOL hLocalThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem);
BOOL ColorMatrix_Internal_AcquireColorBlock(PMATRIX_INTERNAL_CONTEXT pMatrixContext, COLOR_NODE ColorNode, UCHAR Color, ULONG Index, PACKED_CONTEXT PackedInformation, ULONG *pThreadId);
ULONG ColorMatrix_Internal_CenterBlock(PMATRIX_INTERNAL_CONTEXT pMatrixContext, COLOR_NODE ColorNode, UCHAR Color, PACKED_CONTEXT PackedInformation, PTHREAD_CONTEXT pThreadContext, ULONG Index);
void ColorMatrix_Internal_CalculateLargestBlock(ULONG NumberOfBuckets, PTHREAD_CONTEXT pThreadContextHead, ULONG *pLargestSize);
void ColorMatrix_Internal_AssociateThreads(PTHREAD_CONTEXT pThreadContextHead, ULONG AssociatedThreadId);
BOOL ColorMatrix_CreateWorkerThread(PMATRIX_INTERNAL_CONTEXT pMatrixContext, PACKED_CONTEXT PackedInformation, ULONG Index);


/*******************************************************************************
 * ColorMatrix_SearchLargestRegion                                             *
 *                                                                             *
 * DESCRIPTION: Find the largest adjacent color region                         *
 *                                                                             *
 * INPUT                                                                       *
 *   Color Matrix, Return Size                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   TRUE or FALSE                                                             *
 *                                                                             *
 *******************************************************************************/
BOOL ColorMatrix_SearchLargestRegion(PCOLOR_MATRIX pColorMatrix, ULONG *pLargestSize)
{
    BOOL bCompleted = FALSE;
	MATRIX_INTERNAL_CONTEXT MatrixContext = { 0 };
    BOOL bNoFailures;
    PACKED_CONTEXT PackedInformation;
    PTHREAD_CONTEXT pThreadContext = NULL;
	ULONG NumberOfWorkerThreads;

    NumberOfWorkerThreads = ColorMatrix_Internal_GetProcessorCount();

    if (NumberOfWorkerThreads > 1)
    {
        NumberOfWorkerThreads = NumberOfWorkerThreads - 1;
    }

	MatrixContext.hThreadPool = ThreadPool_Create(NumberOfWorkerThreads, ColorMatrix_Internal_ColorCounterWorkItem, COLOR_QUEUE_DEPTH, &MatrixContext);

    if (MatrixContext.hThreadPool)
    {
        MatrixContext.NumberOfWorkerThreads = NumberOfWorkerThreads;
        MatrixContext.pColorMatrix = pColorMatrix;
        MatrixContext.pMatrixTracker = LocalAlloc(LMEM_ZEROINIT, pColorMatrix->SizeX*pColorMatrix->SizeY * sizeof(MATRIX_TRACKER_INTERNAL));

        if (MatrixContext.pMatrixTracker)
        {
            MatrixContext.hWaitThreadsComplete = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (MatrixContext.hWaitThreadsComplete != NULL)
            {

                bNoFailures = FALSE;
                MatrixContext.ThreadCount  = 1;
				MatrixContext.ThreadIds    = 1;
				MatrixContext.TotalPixels = pColorMatrix->SizeX*pColorMatrix->SizeY;
				
                PackedInformation.u.Packed.IndexX = 0;
                PackedInformation.u.Packed.IndexY = 0;
                PackedInformation.u.Packed.ThreadId = 1;

                pThreadContext = LocalAlloc(LMEM_ZEROINIT, sizeof(THREAD_CONTEXT));

                if (pThreadContext)
                {
					  MatrixContext.pThreadHead = pThreadContext;
                      pThreadContext->PackedContext = PackedInformation;

					  if (ColorMatrix_Internal_ColorCounterWorkItem(MatrixContext.hThreadPool, &MatrixContext, 0, pThreadContext))
					  {
						  bNoFailures = TRUE;
					  }
                }

                if (bNoFailures)
                {
					if (!(InterlockedDecrement64(&MatrixContext.ThreadCount) == 0 && MatrixContext.PixelsExamined == MatrixContext.TotalPixels))
					{
						WaitForSingleObject(MatrixContext.hWaitThreadsComplete, INFINITE);
					}

                    ColorMatrix_Internal_CalculateLargestBlock((ULONG)(MatrixContext.ThreadIds), MatrixContext.pThreadHead, pLargestSize);
                    bCompleted = TRUE;
                }
                else
                {
                    if (InterlockedDecrement64(&MatrixContext.ThreadCount) != 0)
                    {
                        WaitForSingleObject(MatrixContext.hWaitThreadsComplete, INFINITE);
                    }
                }

                /*
                 * Free Memory
                 */
                do {
                    pThreadContext = MatrixContext.pThreadHead;

                    if (pThreadContext)
                    {
                        if (pThreadContext->pNeighbors)
                        {
                            LocalFree(pThreadContext->pNeighbors);
                        }
						MatrixContext.pThreadHead = pThreadContext->pNextThread;
                        LocalFree(pThreadContext);
                    }

                } while (MatrixContext.pThreadHead);

            }

            if (MatrixContext.hWaitThreadsComplete)
            {
                CloseHandle(MatrixContext.hWaitThreadsComplete);
            }

            LocalFree(MatrixContext.pMatrixTracker);
        }

		ThreadPool_Destroy(MatrixContext.hThreadPool);
    }

    return bCompleted;
}


/*******************************************************************************
* ColorMatrix_CreateWorkerThread                                              *
*                                                                             *
* DESCRIPTION: Find the largest adjacent color region                         *
*                                                                             *
* INPUT                                                                       *
*   Color Matrix, Return Size                                                 *
*                                                                             *
* OUTPUT                                                                      *
*   TRUE or FALSE                                                             *
*                                                                             *
*******************************************************************************/
BOOL ColorMatrix_CreateWorkerThread(PMATRIX_INTERNAL_CONTEXT pMatrixContext, PACKED_CONTEXT PackedInformation, ULONG Index)
{
	BOOL bWorkerCreated = FALSE;
	BOOL bSwapped = FALSE;
	PTHREAD_CONTEXT pThreadContext;
	PTHREAD_CONTEXT pTemp;

	
	pThreadContext = LocalAlloc(LMEM_ZEROINIT, sizeof(THREAD_CONTEXT));

	if (pThreadContext)
	{
		InterlockedIncrement64(&pMatrixContext->ThreadCount);
		PackedInformation.u.Packed.ThreadId = (ULONG)InterlockedIncrement64(&pMatrixContext->ThreadIds);
		pThreadContext->PackedContext = PackedInformation;

		do { 

			pTemp = pMatrixContext->pThreadHead;
			pThreadContext->pNextThread = pTemp;

			if (InterlockedCompareExchange64((ULONG64 *)&pMatrixContext->pThreadHead, (ULONG64)pThreadContext, (ULONG64)pTemp) == (ULONG64)pTemp)
			{
				bSwapped = TRUE;
			}
		
		} while (bSwapped == FALSE);
		
		bWorkerCreated = ThreadPool_SendThreadWorkAsync(pMatrixContext->hThreadPool, 0, pThreadContext, ColorMatrix_Internal_ColorCompleted, NULL);
	}


	return bWorkerCreated;
}


/*******************************************************************************
* ColorMatrix_Internal_CalculateLargestBlock                                            *
*                                                                             *
* DESCRIPTION: Get the largest block                        *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
void ColorMatrix_Internal_CalculateLargestBlock(ULONG NumberOfBuckets, PTHREAD_CONTEXT pThreadContextHead, ULONG *pLargestSize)
{
    PBUCKET_COUNT pBucketCount = NULL;
    ULONG LowestThreadId;
    ULONG NewLowestThreadId;
    ULONG NextRedirection;
    ULONG Index;
#ifdef DEBUG_CODE
	ULONG DebugSize;
#endif

    pBucketCount = LocalAlloc(LMEM_ZEROINIT, sizeof(BUCKET_COUNT)*NumberOfBuckets);

    if (pBucketCount)
    {
		*pLargestSize = 0;
        
        do {
            
            LowestThreadId = pThreadContextHead->PackedContext.u.Packed.ThreadId;

            while (pBucketCount[LowestThreadId-1].RedirectionThreadId)
            {
                LowestThreadId = pBucketCount[LowestThreadId-1].RedirectionThreadId;
            }

            pBucketCount[LowestThreadId-1].Count += pThreadContextHead->Count;


			if (pBucketCount[LowestThreadId - 1].Count > *pLargestSize)
			{
				*pLargestSize = pBucketCount[LowestThreadId - 1].Count;
			}

            /*
             * Consolidate Neighbors using redirection
             */
            for (Index = 0; Index < pThreadContextHead->CurrentNeighbors; Index++)
            {
                NextRedirection = pThreadContextHead->pNeighbors[Index];

                while (pBucketCount[NextRedirection-1].RedirectionThreadId)
                {
                    NextRedirection = pBucketCount[NextRedirection-1].RedirectionThreadId;
                }

                if (NextRedirection < LowestThreadId)
                {
                    pBucketCount[LowestThreadId-1].RedirectionThreadId = NextRedirection;
                    pBucketCount[NextRedirection-1].Count += pBucketCount[LowestThreadId-1].Count;
                    pBucketCount[LowestThreadId-1].Count = 0;
                    LowestThreadId = NextRedirection;

					if (pBucketCount[LowestThreadId - 1].Count > *pLargestSize)
					{
						*pLargestSize = pBucketCount[LowestThreadId - 1].Count;
					}
                }
                else
                {
                    if (NextRedirection != LowestThreadId)
                    {
                        pBucketCount[NextRedirection - 1].RedirectionThreadId = LowestThreadId;
                        pBucketCount[LowestThreadId - 1].Count += pBucketCount[NextRedirection - 1].Count;
                        pBucketCount[NextRedirection - 1].Count = 0;

						if (pBucketCount[LowestThreadId - 1].Count > *pLargestSize)
						{
							*pLargestSize = pBucketCount[LowestThreadId - 1].Count;
						}
                    }
                }
            }

            pThreadContextHead = pThreadContextHead->pNextThread;

        } while (pThreadContextHead);
                
#ifdef DEBUG_CODE
		/*
		 * Debug Code
		 */ 
		DebugSize = 0;
        
        for(Index = 0; Index < NumberOfBuckets; Index++)
        {
            if (pBucketCount[Index].Count > DebugSize)
            {
				DebugSize = pBucketCount[Index].Count;
            }

            if (pBucketCount[Index].Count != 0 && pBucketCount[Index].RedirectionThreadId != 0)
            {
                printf("Error: Bucket %i is incorrect\n", Index);
            }
            else
            {
                if (pBucketCount[Index].Count)
                {
                    printf("Bucket %i Count %i\n", Index, pBucketCount[Index].Count);
                }

                if (pBucketCount[Index].RedirectionThreadId)
                {
                    printf("Bucket %i Redirects to %i\n", Index, pBucketCount[Index].RedirectionThreadId-1);
                }

            }
		}

		if (DebugSize != *pLargestSize)
		{
			printf("Debug Size %i != Large Size %i\n", DebugSize, *pLargestSize);
		}

		/*
		 * End Debug Code
		 */
#endif

        LocalFree(pBucketCount);
    }
}

/*******************************************************************************
* ColorMatrix_Internal_GetProcessorCount                                            *
*                                                                             *
* DESCRIPTION: Get the number of processors                            *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
ULONG ColorMatrix_Internal_GetProcessorCount(void)
{
    ULONG ProcessorCount;
    ULONG64 ProcessAffinity;
    ULONG64 SystemAffinity;

    /*
    * Ignore processor groups and only create threads within 1 group.
    */
    if (GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffinity, &SystemAffinity))
    {
        ProcessorCount = 0;

        /*
        * The simple way.
        */
        while (ProcessAffinity)
        {
            ProcessorCount++;
            ProcessAffinity >>= 1;
        }
    }
    else
    {
        ProcessorCount = 1;
    }

    return ProcessorCount;
}



/*******************************************************************************
* ColorMatrix_Internal_AcquireColorBlock                                            *
*                                                                             *
* DESCRIPTION: Own this color block                               *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
BOOL ColorMatrix_Internal_AcquireColorBlock(PMATRIX_INTERNAL_CONTEXT pMatrixContext, COLOR_NODE ColorNode, UCHAR Color, ULONG Index, PACKED_CONTEXT PackedInformation, ULONG *pThreadId)
{
    BOOL bColorBlockAcquired = FALSE;
    COLOR_NODE ExistingNode;

    *pThreadId = INVALID_THREAD_ID;

    if (pMatrixContext->pColorMatrix->pColorMaxtrix[Index] == Color)
    {
        ExistingNode = pMatrixContext->pMatrixTracker[Index].Color.ColorNode;

        if (ExistingNode.u.Data.ColorInvalid == 0)
        {
            if (InterlockedCompareExchange64(&pMatrixContext->pMatrixTracker[Index].Color.ColorNodeAsULONG64, ColorNode.u.AsUlong64, ExistingNode.u.AsUlong64) == ExistingNode.u.AsUlong64)
            {
                bColorBlockAcquired = TRUE;
				InterlockedIncrement64(&pMatrixContext->PixelsExamined);
            }
            else
            {
                *pThreadId = (ULONG)pMatrixContext->pMatrixTracker[Index].Color.ColorNode.u.Data.ThreadId;
            }
        }
        else
        {
            *pThreadId = (ULONG)ExistingNode.u.Data.ThreadId;
        }
            
    }
	else
	{
		ExistingNode = pMatrixContext->pMatrixTracker[Index].Color.ColorNode;

		if (ExistingNode.u.Data.ColorInvalid == 0)
		{
			ColorMatrix_CreateWorkerThread(pMatrixContext, PackedInformation, Index);
		}
	}

    return bColorBlockAcquired;
}

/*******************************************************************************
* ColorMatrix_Internal_ColorCounterWorkItem                                            *
*                                                                             *
* DESCRIPTION: Find this color's largest region                               *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
BOOL ColorMatrix_Internal_ColorCounterWorkItem(HTHREADPOOL hLocalThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem)
{
    BOOL bCompleted = FALSE;
    PACKED_CONTEXT PackedInformation;
    USHORT IndexX, IndexY;
    ULONG CurrentIndex;
    UCHAR Color;
    PTHREAD_CONTEXT pThreadContext = (PTHREAD_CONTEXT)pWorkItem;
    COLOR_NODE ColorNode = { 0 };
    PMATRIX_INTERNAL_CONTEXT pMatrixContext = (PMATRIX_INTERNAL_CONTEXT)pGlobalContext;
    ULONG ThreadId;

    
    PackedInformation = pThreadContext->PackedContext;
    IndexX = PackedInformation.u.Packed.IndexX;
    IndexY = PackedInformation.u.Packed.IndexY;

    CurrentIndex = (IndexY*pMatrixContext->pColorMatrix->SizeX + IndexX);
    Color = pMatrixContext->pColorMatrix->pColorMaxtrix[CurrentIndex];
    
    ColorNode.u.Data.ThreadId = PackedInformation.u.Packed.ThreadId;
    ColorNode.u.Data.ColorInvalid = 1;
    
    /*
     * Perform up front check
     */

    if (pMatrixContext->pMatrixTracker[CurrentIndex].Color.ColorNode.u.Data.ColorInvalid == 0)
    {
        if (ColorMatrix_Internal_AcquireColorBlock(pMatrixContext, ColorNode, Color, CurrentIndex, PackedInformation, &ThreadId))
        {
            pThreadContext->Count = ColorMatrix_Internal_CenterBlock(pMatrixContext, ColorNode, Color, PackedInformation, pThreadContext, CurrentIndex) + 1;
            bCompleted = TRUE;
        }
    }
    
    return bCompleted;
}

/*******************************************************************************
* ColorMatrix_Internal_CenterBlock                                            *
*                                                                             *
* DESCRIPTION: Own this color block                               *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
ULONG ColorMatrix_Internal_CenterBlock(PMATRIX_INTERNAL_CONTEXT pMatrixContext, COLOR_NODE ColorNode, UCHAR Color, PACKED_CONTEXT PackedInformation, PTHREAD_CONTEXT pThreadContext, ULONG Index)
{
    ULONG Count = 0;
    ULONG CellThreadId;
    ULONG CurrentIndex=0;
    PACKED_CONTEXT CurrentPackedInformation;

    /*
     * Go Up
     */
    if (PackedInformation.u.Packed.IndexY != 0)
    {
        CurrentPackedInformation = PackedInformation;
        CurrentPackedInformation.u.Packed.IndexY -= 1;
        CurrentIndex = Index - pMatrixContext->pColorMatrix->SizeX;


        if (ColorMatrix_Internal_AcquireColorBlock(pMatrixContext, ColorNode, Color, CurrentIndex, CurrentPackedInformation, &CellThreadId))
        {
            Count += ColorMatrix_Internal_CenterBlock(pMatrixContext, ColorNode, Color, CurrentPackedInformation, pThreadContext, CurrentIndex) + 1;
        }
        else
        {
            if (CellThreadId != INVALID_THREAD_ID && CellThreadId != ColorNode.u.Data.ThreadId)
            {
                ColorMatrix_Internal_AssociateThreads(pThreadContext, CellThreadId);
            }
        }
    }


    /*
     * Go Down
     */
        
    if (PackedInformation.u.Packed.IndexY != pMatrixContext->pColorMatrix->SizeY - 1)
    {
        CurrentIndex = Index + pMatrixContext->pColorMatrix->SizeX;
        CurrentPackedInformation = PackedInformation;
        CurrentPackedInformation.u.Packed.IndexY += 1;

        if (ColorMatrix_Internal_AcquireColorBlock(pMatrixContext, ColorNode, Color, CurrentIndex, CurrentPackedInformation, &CellThreadId))
        {
            Count += ColorMatrix_Internal_CenterBlock(pMatrixContext, ColorNode, Color, CurrentPackedInformation, pThreadContext, CurrentIndex) + 1;
        }
        else
        {
            if (CellThreadId != INVALID_THREAD_ID && CellThreadId != ColorNode.u.Data.ThreadId)
            {
                ColorMatrix_Internal_AssociateThreads(pThreadContext, CellThreadId);
            }
        }
    }


    /*
     * Go Left
     */		
    if (PackedInformation.u.Packed.IndexX != 0)
    {
        CurrentPackedInformation = PackedInformation;
        CurrentPackedInformation.u.Packed.IndexX -= 1;
        CurrentIndex = Index - 1;

        if (ColorMatrix_Internal_AcquireColorBlock(pMatrixContext, ColorNode, Color, CurrentIndex, CurrentPackedInformation, &CellThreadId))
        {
            Count += ColorMatrix_Internal_CenterBlock(pMatrixContext, ColorNode, Color, CurrentPackedInformation, pThreadContext, CurrentIndex) + 1;
        }
        else
        {
            if (CellThreadId != INVALID_THREAD_ID && CellThreadId != ColorNode.u.Data.ThreadId)
            {
                ColorMatrix_Internal_AssociateThreads(pThreadContext, CellThreadId);
            }
        }
    }



    /*
     * Go Right
     */
    if (PackedInformation.u.Packed.IndexX != pMatrixContext->pColorMatrix->SizeX - 1)
    {
        CurrentPackedInformation = PackedInformation;
        CurrentPackedInformation.u.Packed.IndexX += 1;
        CurrentIndex = Index + 1;

        if (ColorMatrix_Internal_AcquireColorBlock(pMatrixContext, ColorNode, Color, CurrentIndex, CurrentPackedInformation, &CellThreadId))
        {
            Count += ColorMatrix_Internal_CenterBlock(pMatrixContext, ColorNode, Color, CurrentPackedInformation, pThreadContext, CurrentIndex) + 1;
        }
        else
        {
            if (CellThreadId != INVALID_THREAD_ID && CellThreadId != ColorNode.u.Data.ThreadId)
            {
                ColorMatrix_Internal_AssociateThreads(pThreadContext, CellThreadId);
            }
        }
    }


    return Count;
}



/*******************************************************************************
* ColorMatrix_Internal_ColorCompleted                                            *
*                                                                             *
* DESCRIPTION: Find this color's largest region                               *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
BOOL ColorMatrix_Internal_ColorCompleted(HTHREADPOOL hThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem, PVOID pContext, BOOL bCanceled)
{
    PMATRIX_INTERNAL_CONTEXT pMatrixContext = (PMATRIX_INTERNAL_CONTEXT)pGlobalContext;

    if (InterlockedDecrement64(&pMatrixContext->ThreadCount) == 0 && pMatrixContext->PixelsExamined == pMatrixContext->TotalPixels)
    {
        SetEvent(pMatrixContext->hWaitThreadsComplete);
    }

    return TRUE;
}

/*******************************************************************************
* ColorMatrix_Internal_AssociateThreads                                            *
*                                                                             *
* DESCRIPTION: Find this color's largest region                               *
*                                                                             *
* INPUT                                                                       *
*                                                    *
*                                                                             *
* OUTPUT                                                                      *
*                                                                *
*                                                                             *
*******************************************************************************/
void ColorMatrix_Internal_AssociateThreads(PTHREAD_CONTEXT pThreadContextHead, ULONG AssociatedThreadId)
{
    ULONG *NewMemory;

    if (pThreadContextHead->pNeighbors == NULL)
    {
        pThreadContextHead->pNeighbors = LocalAlloc(LMEM_ZEROINIT, sizeof(ULONG)*NEIGHBOR_CHUNK_ALLOC);

        if (pThreadContextHead->pNeighbors)
        {
            pThreadContextHead->pNeighbors[0] = AssociatedThreadId;
            pThreadContextHead->CurrentNeighbors = 1;
            pThreadContextHead->MaxNeighbors = NEIGHBOR_CHUNK_ALLOC;
        }
    }
    else
    {
        if (pThreadContextHead->CurrentNeighbors == pThreadContextHead->MaxNeighbors)
        {
            NewMemory = LocalReAlloc(pThreadContextHead->pNeighbors, sizeof(ULONG)*NEIGHBOR_CHUNK_ALLOC*pThreadContextHead->MaxNeighbors, LMEM_ZEROINIT);

            if (NewMemory)
            {
                pThreadContextHead->pNeighbors = NewMemory;
                pThreadContextHead->pNeighbors[pThreadContextHead->CurrentNeighbors] = AssociatedThreadId;
                pThreadContextHead->CurrentNeighbors++;
                pThreadContextHead->MaxNeighbors += NEIGHBOR_CHUNK_ALLOC;
            }

        }
        else
        {
            pThreadContextHead->pNeighbors[pThreadContextHead->CurrentNeighbors] = AssociatedThreadId;
            pThreadContextHead->CurrentNeighbors++;
        }
    }
}