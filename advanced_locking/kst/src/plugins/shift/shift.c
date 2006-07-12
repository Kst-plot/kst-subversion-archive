/* Plugin to compute a shifted vector. It is a simple copy of the input vector, 
   padded with zeros at the beginning or the end according to the sign of the 
   shift value, and truncated at the other end ! */
   
#include <stdlib.h>
#include <stdio.h>

int shift(const double *const inArrays[],const int inArrayLens[], const double inScalars[], double *outArrays[], int outArrayLens[], double outScalars[])
{
  int i, delay = 0;
  const double nan = 0.0 / 0.0;

  /* Memory allocation */
  if (outArrayLens[0] != inArrayLens[0]) {
    outArrays[0] = realloc(outArrays[0], (inArrayLens[0])*sizeof(double));
    outArrayLens[0] = inArrayLens[0];
  }

  delay=(int)inScalars[0];
  /* Protect against invalid inputs */
  if (delay > inArrayLens[0]) delay = inArrayLens[0];
  if (delay < -inArrayLens[0]) delay = -inArrayLens[0];

  /* First case: positive shift (forwards/right shift)*/
  if (delay >= 0) {
    /* Pad beginning with zeros */
    for (i = 0; i < delay; i++)
    {
      outArrays[0][i] = nan;
    }
    /* Then, copy values with the right offset */
    for (i = delay; i < inArrayLens[0]; i++) {
       outArrays[0][i] = inArrays[0][i-delay];
    }
  }

  /* Second case: negative shift (backwards/left shift)*/
  else if (delay < 0) {
    delay = -delay; /* Easier to visualize :-) */
    /* Copy values with the right offset */
    for (i = 0; i < inArrayLens[0]-delay; i++) {
       outArrays[0][i] = inArrays[0][i+delay];
    }
    /* Pad end with zeros */
    for (i = inArrayLens[0]-delay; i < inArrayLens[0]; i++) {
      outArrays[0][i] = nan;
    }
  }

return 0;
}
