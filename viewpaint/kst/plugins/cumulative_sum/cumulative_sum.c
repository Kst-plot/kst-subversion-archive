/*
 *  Copyright 2005, Nicolas Brisset
 *  Released under the terms of the GPL.
 *
 */
/* Plugin to compute the "integral" of a signal.
   Watch out, the resulting vector is 1 sample longer!
   In some cases, this might disturb but it is better to
   not truncate the last point... This can be used as an 
   integration, but it is not exactly correct! */
   
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


int cumulative_sum(const double *const inArrays[],const int inArrayLens[], const double inScalars[], double *outArrays[], int outArrayLens[], double outScalars[])
{
	int i;
	/* Memory allocation */
	if (outArrayLens[0] != inArrayLens[0]) 
	{
	  outArrays[0] = realloc(outArrays[0], (inArrayLens[0]+1)*sizeof(double));
	  outArrayLens[0] = inArrayLens[0]+1;
	}
	
	outArrays[0][0] = 0.0;
	for (i = 0; i < inArrayLens[0]; i++) 
	{
	   outArrays[0][i+1] = inArrays[0][i]*inScalars[0] + outArrays[0][i];
        }
	
return 0;
}
