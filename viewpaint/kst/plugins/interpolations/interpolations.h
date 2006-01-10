/*
 *  Generic non-linear fit plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#define KST_UNUSED(x) if(x){};

#define X 					0
#define Y 					1
#define X_INTERP 		2

int interpolate(
  const double *const inArrays[], 
  const int inArrayLens[],
  const double inScalars[],
  double *outArrays[], 
  int outArrayLens[],
	double outScalars[],
  const gsl_interp_type* pType);

int interpolate(
  const double *const inArrays[], 
  const int inArrayLens[],
  const double inScalars[],
  double *outArrays[], 
  int outArrayLens[],
	double outScalars[],
  const gsl_interp_type* pType) {
  
  KST_UNUSED( inScalars )
  KST_UNUSED( outScalars )
  
  gsl_interp_accel*	pAccel = NULL;
  gsl_interp*				pInterp = NULL;
  gsl_spline*				pSpline = NULL;
  int i = 0;
  int iLengthData;
  int	iLengthInterp;
  int iReturn = -1;
  double* pResult[1];
  
  iLengthData = inArrayLens[X];
  if( inArrayLens[Y] < iLengthData ) {
    iLengthData = inArrayLens[Y];
  }
  
  iLengthInterp = inArrayLens[X_INTERP];
  if( iLengthInterp > 0 ) {
    if( outArrayLens[0] != iLengthInterp ) {
      pResult[0] = (double*)realloc( outArrays[0], iLengthInterp * sizeof( double ) );
    } else {
      pResult[0] = outArrays[0];
    }
    
    if( pResult[0] != NULL ) {
      outArrays[0] = pResult[0];
      outArrayLens[0] = iLengthInterp;
     
      pInterp = gsl_interp_alloc( pType, iLengthData );
      if( pInterp != NULL ) {
        //
        // check that we have enough data points...
        //
        if( (unsigned int)iLengthData > gsl_interp_min_size( pInterp ) ) {    
          pAccel  = gsl_interp_accel_alloc( );
          if( pAccel != NULL ) {
            pSpline = gsl_spline_alloc( pType, iLengthData );
            if( pSpline != NULL ) {
              if( !gsl_spline_init( pSpline, inArrays[X], inArrays[Y], iLengthData ) ) {
                for( i=0; i<iLengthInterp; i++ ) {
                  outArrays[0][i] = gsl_spline_eval( pSpline, inArrays[X_INTERP][i], pAccel );
                }
                
                iReturn = 0;
              }
              gsl_spline_free( pSpline );
            }
            gsl_interp_accel_free( pAccel );
          }
        }
        gsl_interp_free( pInterp );
      }
    }
  }
  
  return iReturn;
}
