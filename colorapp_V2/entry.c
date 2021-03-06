/***************************************************************************** 
 *                            Color Finder                                   *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2018           Toby Opferman                              *
 *****************************************************************************/

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "colormatrix.h"




UCHAR g_ColorMatrix[] = { 0, 0, 0, 1, 1, 0, 0, 0, 1, 2, 2, 2, 3, 
                          0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 2, 1, 4,
                          1, 0, 0, 0, 0, 1, 0, 2, 0, 3, 3, 4, 4,
                          1, 1, 1, 1, 1, 0, 0, 3, 3, 3, 4, 4, 4};


UCHAR g_ColorMatrix2[] = { 0, 0, 0, 1, 1, 0, 0, 0, 1, 2, 2, 2, 3,
                           0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 2, 1, 4,
                           1, 0, 0, 0, 0, 1, 0, 2, 0, 3, 3, 4, 4,
                           1, 1, 1, 1, 1, 0, 0, 3, 3, 3, 4, 4, 4, 
	                       3, 0, 1, 0, 0, 1, 0, 2, 0, 3, 4, 4, 4, 
	                       3, 0, 0, 0, 0, 1, 0, 2, 0, 3, 4, 4, 4, 
	                       1, 0, 1, 1, 0, 1, 0, 2, 0, 5, 4, 5, 4, 
	                       3, 0, 0, 0, 0, 1, 0, 2, 0, 5, 4, 5, 4,
	                       3, 0, 1, 0, 0, 1, 1, 1, 0, 5, 5, 5, 4, 
	                       3, 0, 0, 0, 0, 1, 0, 2, 0, 3, 3, 4, 4 };
/*
 * Test out collisions
 */

UCHAR g_ColorMatrix3[] = {  0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
								0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 
COLOR_MATRIX g_ColorMatrices[] = {
	{ g_ColorMatrix, 13, 4 },
    { g_ColorMatrix2, 13, 10 },
	{ g_ColorMatrix3, 12, 121 },
	{ NULL, 0, 0}
};

/*******************************************************************************
 * main                                                                        *
 *                                                                             *
 * DESCRIPTION: C Entry Point                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Standard Parameters                                                       *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   ErrorCode to OS                                                           *
 *                                                                             *
 *******************************************************************************/
int _cdecl main(int argc, char **argv)
{
	ULONG ulLargestRegion;
	ULONG Index = 0;

	while (g_ColorMatrices[Index].pColorMaxtrix)
	{
		printf("Checking Matrix #%i\n", Index);
		if (ColorMatrix_SearchLargestRegion(&g_ColorMatrices[Index], &ulLargestRegion))
		{
			printf("The largest color region is %i\n", ulLargestRegion);
		}
		Index++;
	}

    return 0; 
}

