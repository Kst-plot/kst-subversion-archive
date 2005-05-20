/*
 *  Bin plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>

extern "C" int bin(const double *const inArrays[], const int inArrayLens[],
                const double is[],
                double *outArrays[], int outArrayLens[],
                double outScalars[]);

int bin(const double *const inArrays[], const int inArrayLens[],
                const double is[],
                double *outArrays[], int outArrayLens[],
                double outScalars[])
//Bin the elements into the given size bins, additional elements at the end of the
//input vector are ignored.
//Returns -1 on error, 0 on success.
{
	//Cast the binsize to an integer if it is not one
	int binsize=(int)is[0];

	//Make sure there is at least 1 element in the input vector
	//Make sure the bin size is at least 1
	if (inArrayLens[0] < 1 || binsize < 1) {
		return -1;
	}

	//now set the outputs
	outArrayLens[0]=(int) (inArrayLens[0] / binsize);

	//resize the output array
	outArrays[0]=(double*)realloc(outArrays[0], outArrayLens[0]*sizeof(double));

	//now bin the data
	for (int i=0; i<outArrayLens[0]; i++)
	{
		outArrays[0][i]=0;
		//add up the elements for this bin
		for (int j=0; j<binsize; j++)
		{
			outArrays[0][i]+=inArrays[0][i*binsize+j];
		}
		//find the mean
		outArrays[0][i]/=binsize;
	}
	return 0;
}
