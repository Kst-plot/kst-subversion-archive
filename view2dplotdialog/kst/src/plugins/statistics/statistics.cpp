/*
 *  Statistics plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#define KST_UNUSED(x) if(x){};

void swap( double* pData, int iOne, int iTwo );
void quicksort( double* pData, int iLeft, int iRight );
extern "C" int statistics( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

void swap( double* pData, int iOne, int iTwo )
{
  double dTemp;
    
  dTemp = pData[iOne];
  pData[iOne] = pData[iTwo];
  pData[iTwo] = dTemp;
}

void quicksort( double* pData, int iLeft, int iRight )
{
  double dVal = pData[iRight];
  int i = iLeft - 1;
  int j = iRight;
    
  if( iRight <= iLeft )
  {
    return;
  }
    
  while( 1 )
  {
    while( pData[++i] < dVal )
    {
    }
        
    while( dVal < pData[--j] )
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

int statistics( const double *const inArrays[], const int inArrayLens[],
                const double inScalars[],
                double *outArrays[], int outArrayLens[],
                double outScalars[] )
{
  KST_UNUSED( inScalars )
  KST_UNUSED( outArrays )
  KST_UNUSED( outArrayLens )
    
  double* pCopy;
  double dMean = 0.0;
  double dMedian = 0.0;
  double dStandardDeviation = 0.0;
  double dTotal = 0.0;
  double dSquaredTotal = 0.0;
  double dMinimum = 0.0;
  double dMaximum = 0.0;
  double dVariance = 0.0;
  double dAbsoluteDeviation = 0.0;
  double dSkewness = 0.0;
  double dKurtosis = 0.0;
  int iLength;
  int iRetVal = -1;
  int i;
  
  if( inArrayLens[0] > 0 )
  {
    iLength = inArrayLens[0];
    
    for( i=0; i<iLength; i++ )
    {
      if( i == 0 || inArrays[0][i] < dMinimum )
      {
        dMinimum = inArrays[0][i];
      }
      if( i == 0 || inArrays[0][i] > dMaximum )
      {
        dMaximum = inArrays[0][i];
      }
      dTotal += inArrays[0][i];
      dSquaredTotal += inArrays[0][i] * inArrays[0][i];
    }
    
    dMean = dTotal / (double)iLength;
    if( iLength > 1 )
    {
      dVariance  = 1.0 / ( (double)iLength - 1.0 );
      dVariance *= dSquaredTotal - ( dTotal * dTotal / (double)iLength ); 
      if( dVariance > 0.0 )
      {
        dStandardDeviation = sqrt( dVariance );
      }
      else
      {
        dVariance = 0.0;
        dStandardDeviation = 0.0;
      }
    }            
    
    for( i=0; i<iLength; i++ ) {
      dAbsoluteDeviation += fabs( inArrays[0][i] - dMean );
      dSkewness				   += pow( inArrays[0][i] - dMean, 3.0 );
      dKurtosis				   += pow( inArrays[0][i] - dMean, 4.0 );
    }
    dAbsoluteDeviation /= (double)iLength;
    dSkewness				   /= (double)iLength * pow( dStandardDeviation, 3.0 );
    dKurtosis				   /= (double)iLength * pow( dStandardDeviation, 4.0 );
    dKurtosis				   -= 3.0;
    
    /*
    sort by phase...
    */
    pCopy = (double*)calloc( iLength, sizeof( double ) );
    if( pCopy != NULL )
    {
      memcpy( pCopy, inArrays[0], iLength * sizeof( double ) );
      quicksort( pCopy, 0, iLength-1 );
      dMedian = pCopy[ iLength / 2 ];
      
      free( pCopy );
    }
    
    outScalars[0] = dMean;
    outScalars[1] = dMinimum;
    outScalars[2] = dMaximum;
    outScalars[3] = dVariance;
    outScalars[4] = dStandardDeviation;
    outScalars[5] = dMedian;
    outScalars[6] = dAbsoluteDeviation;
    outScalars[7] = dSkewness;
    outScalars[8] = dKurtosis;
    
    iRetVal = 0;
  }   
  
  return iRetVal;
}
