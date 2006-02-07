/*
 *  Phase plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#define KST_UNUSED(x) if(x){};

void swap( double* pData[], int iOne, int iTwo );
void quicksort( double* pData[], int iLeft, int iRight );
int phase( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

void swap( double* pData[], int iOne, int iTwo )
{
    double dTemp;
    int i;
    
    for( i=0; i<2; i++ )
    {
        dTemp = pData[i][iOne];
        pData[i][iOne] = pData[i][iTwo];
        pData[i][iTwo] = dTemp;
    }
}

void quicksort( double* pData[], int iLeft, int iRight )
{
    double dVal = pData[0][iRight];
    int i = iLeft - 1;
    int j = iRight;
    
    if( iRight <= iLeft )
    {
        return;
    }
    
    while( 1 )
    {
        while( pData[0][++i] < dVal )
        {
        }
        
        while( dVal < pData[0][--j] )
        {
            if( j == iLeft )
            {
                break;
            }
        }
        if( i >= j )
        {
            break;
        }
        swap( pData, i, j );
    }
    swap( pData, i, iRight );
    quicksort( pData, iLeft, i-1 );
    quicksort( pData, i+1, iRight );
}

int phase( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] )
{
    KST_UNUSED( outScalars )
    
    double* pResult[2];
    double	dPhasePeriod = inScalars[0];
    double dPhaseZero = inScalars[1];
    int iLength;
    int iRetVal = -1;
    int i;
        
    if( dPhasePeriod > 0.0 )
    {
        if( inArrayLens[0] == inArrayLens[1] )
        {
            iLength = inArrayLens[0];
        
            if( outArrayLens[0] != iLength )
            {
                pResult[0] = realloc( outArrays[0], iLength * sizeof( double ) );
            }
            else
            {
                pResult[0] = outArrays[0];
            }
        
            if( outArrayLens[1] != iLength )
            {
                pResult[1] = realloc( outArrays[1], iLength * sizeof( double ) );
            }
            else
            {
                pResult[1] = outArrays[1];
            }
        
            if( pResult[0] != NULL && pResult[1] != NULL )
            {	
                outArrays[0] = pResult[0];
                outArrays[1] = pResult[1];
                outArrayLens[0] = iLength;
                outArrayLens[1] = iLength;
                
                /*
                determine the phase...
                */
                for( i=0; i<iLength; i++ )
                {
                    outArrays[0][i] = fmod( ( inArrays[0][i] - dPhaseZero ) / dPhasePeriod, 1.0 );
                }
                
                /*
                sort by phase...
                */
                memcpy( outArrays[1], inArrays[1], iLength * sizeof( double ) );
                quicksort( outArrays, 0, iLength-1 );
                
                iRetVal = 0;
            }
        }
    }   
    
    return iRetVal;
}
