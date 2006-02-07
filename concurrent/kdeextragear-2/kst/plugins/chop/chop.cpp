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

extern "C" int chop( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

int chop( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] )
{
    KST_UNUSED( inScalars )
    KST_UNUSED( outScalars )
    
    double* pResult[3];
    int iRetVal = -1;
    int iLength = inArrayLens[0];
    int iLengthNew = iLength / 2;
    int i;
    
    if( iLength > 1 )
    {   
        for( i=0; i<4; i++ )
        {
            if( outArrayLens[i] != iLengthNew )
            {
                pResult[i] = (double*)realloc( outArrays[i], iLengthNew * sizeof( double ) );
            }
            else
            {
                pResult[i] = outArrays[i];
            }
        }
        
        if( pResult[0] != NULL && pResult[1] != NULL && pResult[2] != NULL && pResult[3] != NULL )
        {	
            for( i=0; i<4; i++ )
            {
                outArrays[i] = pResult[i];
                outArrayLens[i] = iLengthNew;
            }
            
            for( i=0; i<iLength; i+=2 )
            {
                outArrays[0][i/2] = inArrays[0][i+0];
                outArrays[1][i/2] = inArrays[0][i+1];
                outArrays[2][i/2] = inArrays[0][i+0] - inArrays[0][i+1];
                outArrays[3][i/2] = i/2;
            }
            
            iRetVal = 0;
        }
    }   
    
    return iRetVal;
}
