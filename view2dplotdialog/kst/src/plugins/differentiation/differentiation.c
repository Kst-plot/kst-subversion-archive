/*
 *  Copyright 2005, Nicolas Brisset
 *  Released under the terms of the GPL.
 *
 */
/* Plugin to compute the discrete derivative of an input vector.
   A fixed-step input is assumed. For extreme points the right- and left-
   tangents are used, for other points their mean as it tends to give better 
   results. Note that noisy signals should first be filtered ! */
   
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
 

int differentiate(const double *const inArrays[],const int inArrayLens[], const double inScalars[], double *outArrays[], int outArrayLens[], double outScalars[])
{
	int i;
	double step;
	/* Memory allocation */
	if (outArrayLens[0] != inArrayLens[0]) 
	{
	  outArrays[0] = realloc(outArrays[0], (inArrayLens[0])*sizeof(double));
	  outArrayLens[0] = inArrayLens[0];
	}
	
	step=inScalars[0];
	
	outArrays[0][0] = (inArrays[0][1] - inArrays[0][0]) / step;
	
	for (i = 1; i < inArrayLens[0]-1; i++) 
	{
	   outArrays[0][i] = (inArrays[0][i+1] - inArrays[0][i-1])/(2*step);
        }
	
	outArrays[0][i] = (inArrays[0][i] - inArrays[0][i-1]) / step; 
	
return 0;
}
