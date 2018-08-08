/***************************************************************************** 
 *                            Color Finder                                   *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2018           Toby Opferman                              *
 *****************************************************************************/

#ifndef __COLORMATRIX_H__
#define __COLORMATRIX_H__

typedef struct _COLOR_MATRIX
{
	UCHAR *pColorMaxtrix;
	ULONG SizeX, SizeY;
        
} COLOR_MATRIX, *PCOLOR_MATRIX;

BOOL ColorMatrix_SearchLargestRegion(PCOLOR_MATRIX pColorMatrix, ULONG *pLargestSize);

#endif