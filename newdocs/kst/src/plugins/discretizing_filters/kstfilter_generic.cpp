/*
 *  General filter plugin for KST.
 *  Copyright 2005, the kst development team
 *  Released under the terms of the GNU GPL v2+.
 *  Author: Nicolas Brisset
 */

#include <math.h>
#include "filter.h"
#include "polynom.h"
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

extern "C" int kstfilter_generic(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[],
		const char* inStrings[], char *outStrings[]);

int kstfilter_generic(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[],
		const char* inStrings[], char *outStrings[])
{

int i = 0, length = inArrayLens[0];

// Extract polynom coefficients and instantiate polynoms
QStringList numCoeffs = QStringList::split(QRegExp("\\s*(,|;|:)\\s*"), inStrings[0]);
QStringList denCoeffs = QStringList::split(QRegExp("\\s*(,|;|:)\\s*"), inStrings[1]);
int numDegree = numCoeffs.count() - 1, denDegree = denCoeffs.count() - 1;
polynom<double> Num(numDegree), Den(denDegree);
double tmpDouble = 0.0;
bool ok = false;
for (i=0; i<=numDegree; i++) {
  tmpDouble = numCoeffs[i].toDouble(&ok);
  if (ok) Num[i]= tmpDouble;
  else Num[i] = 0.0;
}
for (i=0; i<=denDegree; i++) {
  tmpDouble = denCoeffs[i].toDouble(&ok);
  if (ok) Den[i] = tmpDouble;
  else Den[i] = 0.0;
}

// Time step
double DeltaT = inScalars[0];

// Allocate storage for output vectors
outArrays[0] = (double *) realloc(outArrays[0], length * sizeof(double));
outArrayLens[0] = length;

// Create filter
filter<double> theFilter(Num,Den,DeltaT);
double in = 0.0;
theFilter.ConnectTo(in);
theFilter.Reset();
for (int i=0; i<length; i++) {
  in = inArrays[0][i];
  theFilter.NextTimeStep();
  outArrays[0][i] = theFilter.out;
}

return 0;
}
