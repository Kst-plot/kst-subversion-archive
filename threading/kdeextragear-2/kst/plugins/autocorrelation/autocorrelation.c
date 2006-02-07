/*
 *  Auto-correlation plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 */

#include <stdlib.h>
#include <math.h>

#define KST_UNUSED(x) if(x){};

int autocorrelation( const double *const inArrays[], const int inArrayLens[],
		const double is[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

int autocorrelation( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] )
{
    KST_UNUSED( inScalars )
    KST_UNUSED( outScalars )
    
    int iRetVal = -1;
      
    if( inArrayLens[0] > 0 )
    {
        double	dSum  = 0.0;
        double	dAvg;
        double	dSumSq = 0.0;
        double	dStd;
        double	dCor 		= 0.0;
        int	iNumPoints	= inArrayLens[0];
        int 	iNumSteps 	= iNumPoints / 2;
        int	i;
        int	j;
        
        for( i=0; i<iNumPoints; i++ )  
        {
            dSum 	+= inArrays[0][i];
            dSumSq	+= inArrays[0][i] * inArrays[0][i];
        }
        
        if( outArrayLens[0] != iNumSteps ) 
        {
            outArrays[0] 	= realloc( outArrays[0], iNumSteps*sizeof(double) );
            outArrayLens[0] 	= iNumSteps;
        }
        
        if( outArrayLens[1] != iNumSteps ) 
        {
            outArrays[1] 	= realloc( outArrays[1], iNumSteps*sizeof(double) );
            outArrayLens[1] 	= iNumSteps;
        }

        if( iNumPoints > 0 )
        {
            dAvg = dSum / iNumPoints;
            dStd = sqrt( ( dSumSq / (double)iNumPoints ) - ( dAvg * dAvg ) );
            if( dStd > 0.0 )
            {              
                iRetVal = 0;
            
                for( i=0; i<iNumSteps; i++ )  
                {
                    dCor = 0.0;
                    for( j=0; j<iNumPoints-i; j++ )
                    {
                        dCor += ( inArrays[0][j] - dAvg ) * ( inArrays[0][i+j] - dAvg );
                    }
                    dCor /= dStd * dStd;
                    dCor /= (double)( iNumPoints - i );
		
                    outArrays[0][i] = (double)i;
                    outArrays[1][i] = dCor;
                }
            }
        }        
    }
    
    return iRetVal;
}

