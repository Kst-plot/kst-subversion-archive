/*
 *  Line fitting plugin demo for KST.
 *  Copyright 2003, The University of Toronto
 *  Released under the terms of the GPL.
 *
 *  Derived from Numerical Recipes in C, Press et al.
 */

#include <stdlib.h>
#include <math.h>

#define X 0
#define Y 1


int linefit(const double *const inArrays[], const int inArrayLens[],
		const double is[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int linefit(const double *const inArrays[], const int inArrayLens[],
		const double is[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
	int i = 0;
	double a = 0.0, b = 0.0, sx = 0.0, sy = 0.0, sxoss = 0.0, st2 = 0.0, chi2 = 0.0;
	double xScale;

	if (is) {} /* don't warn */

	if (inArrayLens[Y] < 1 || inArrayLens[X] < 1) {
		return -1;
	}

	for (i = 0; i < 2; i++) {
		if (outArrayLens[i] != 2) {
			outArrays[i] = realloc(outArrays[i],
						2*sizeof(double));
			outArrayLens[i] = 2;
		}
	}

	if (inArrayLens[Y] == 1) {
		outArrays[X][0] = inArrays[X][0];
		outArrays[X][1] = inArrays[X][inArrayLens[X] - 1];
		outArrays[Y][0] = inArrays[Y][0];
		outArrays[Y][1] = inArrays[Y][0];
		outScalars[0] = inArrays[Y][0];
		outScalars[1] = 0;
		outScalars[2] = chi2;
		return 0;
	}

	xScale = inArrayLens[X]/inArrayLens[Y];

	for (i = 0; i < inArrayLens[Y]; i++) {
		double z = xScale*i;
		long int idx = rint(z);
		double skew = z - floor(z); /* [0..1] */
		long int idx2 = idx + 1;
		sy += inArrays[Y][i];
		while (idx2 >= inArrayLens[Y]) {
			idx2--;
		}
		sx += inArrays[X][idx] + (inArrays[X][idx2] - inArrays[X][idx])*skew;
	}

	sxoss = sx / inArrayLens[X];

	for (i = 0; i < inArrayLens[X]; i++) {
		double t = inArrays[X][i] - sxoss;
		st2 += t * t;
		b += t * inArrays[Y][i];
	}

	b /= st2;
	a = (sy - sx*b)/inArrayLens[X];

	/* could put goodness of fit, etc, in here */

	outArrays[X][0] = inArrays[X][0];
	outArrays[X][1] = inArrays[X][inArrayLens[X]-1];
	outArrays[Y][0] = a+b*outArrays[X][0];
	outArrays[Y][1] = a+b*outArrays[X][1];

	for (i = 0; i < inArrayLens[X]; i++) {
		double z = xScale*i;
		long int idx = rint(z);
		double skew = z - floor(z); /* [0..1] */
		long int idx2 = idx + 1;
		double newX;
		double ci;
		while (idx2 >= inArrayLens[X]) {
			idx2--;
		}
		newX = inArrays[X][idx] + (inArrays[X][idx2] - inArrays[X][idx])*skew;
 		ci = inArrays[Y][i] - a - b*newX;
		chi2 += ci*ci;
	}

	outScalars[0] = a;
	outScalars[1] = b;
	outScalars[2] = chi2;

	return 0;
}

