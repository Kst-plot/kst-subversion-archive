/*
 *  General non-linear fit plugin for KST.
 *  Copyright 2005, The kst development team
 *  Released under the terms of the GPL.
 */

#include <muParser.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_vector.h>
#include "../common.h"

int function_f(const gsl_vector *x, void *params, gsl_vector *f);
int function_df(const gsl_vector *x, void *params, gsl_matrix *J);
int function_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J);

struct fit {
  size_t n;
  const double *XtoFit;
  const double *YtoFit;
  double *YFitted;
  double *xvar;
  double *parameters;
  size_t numParams;
  mu::Parser *parser;
};

extern "C" int kstfit_general_levenberg_marquardt(const double *const inArrays[], const int inArrayLens[],
              const double inScalars[], double *outArrays[], int outArrayLens[],
              double outScalars[], const char* inStrings[], char *outStrings[]);

int kstfit_general_levenberg_marquardt(const double *const inArrays[], const int inArrayLens[],
              const double inScalars[], double *outArrays[], int outArrayLens[],
              double outScalars[], const char* inStrings[], char *outStrings[])
{
  KST_UNUSED(outStrings);

  int iReturn = -1;

  if (inArrayLens[X] >= 2 && inArrayLens[Y] >= 2) {
    mu::Parser parser;
    double* pResult[4];
    double* pDelete = 0L;
    double* pInputX;
    double* pInputY;
    double *parameters;
    double tolerance = inScalars[0];
    double xvar;
    char *token;
    char *toSplit;
    char *endPtr;
    int maxIterations = (int)inScalars[1];
    int n = inArrayLens[0];
    int paramsNumber = 2;
    int startsNumber = 0;
    int iLengthData;
    int i;
    int j;

    iLengthData = inArrayLens[X];
    if( inArrayLens[Y] > iLengthData ) {
      iLengthData = inArrayLens[Y];
    }

    if (inArrayLens[X] == iLengthData) {
      pInputX = (double*)inArrays[X];
    } else {
      pDelete = (double*)malloc(iLengthData * sizeof( double ));
      pInputX = pDelete;
      for( i=0; i<iLengthData; i++) {
        pInputX[i] = interpolate( i, iLengthData, inArrays[X], inArrayLens[X] );
      }
    }

    if (inArrayLens[Y] == iLengthData) {
      pInputY = (double*)inArrays[Y];
    } else {
      pDelete = (double*)malloc(iLengthData * sizeof( double ));
      pInputY = pDelete;
      for( i=0; i<iLengthData; i++) {
        pInputY[i] = interpolate( i, iLengthData, inArrays[Y], inArrayLens[Y] );
      }
    }

    //
    // count the number of parameter names
    //
    toSplit = strdup(inStrings[1]);
    token = strtok( toSplit, ",;:" );
    while( token != NULL ) {
      paramsNumber++;
      token = strtok( NULL, ",;:" );
    }
    free(toSplit);

    if( iLengthData > paramsNumber ) {
      if( outArrayLens[0] != iLengthData ) {
        pResult[0] = (double*)realloc( outArrays[0], iLengthData * sizeof( double ) );
      } else {
        pResult[0] = outArrays[0];
      }

      if( outArrayLens[1] != iLengthData ) {
        pResult[1] = (double*)realloc( outArrays[1], iLengthData * sizeof( double ) );
      } else {
        pResult[1] = outArrays[1];
      }

      if( outArrayLens[2] != paramsNumber ) {
        pResult[2] = (double*)realloc( outArrays[2], paramsNumber * sizeof( double ) );
      } else {
        pResult[2] = outArrays[2];
      }

      if( outArrayLens[3] != paramsNumber * paramsNumber ) {
        pResult[3] = (double*)realloc( outArrays[3], paramsNumber * paramsNumber * sizeof( double ) );
      } else {
        pResult[3] = outArrays[3];
      }

      if( pResult[0] != NULL &&
          pResult[1] != NULL &&
          pResult[2] != NULL &&
          pResult[3] != NULL ) {
        outArrays[0] = pResult[0];
        outArrayLens[0] = iLengthData;
        outArrays[1] = pResult[1];
        outArrayLens[1] = iLengthData;
        outArrays[2] = pResult[2];
        outArrayLens[2] = paramsNumber;
        outArrays[3] = pResult[3];
        outArrayLens[3] = paramsNumber * paramsNumber;

        parameters = new double[paramsNumber];
        for (i=0; i<paramsNumber; i++) {
          parameters[i] = 0.0;
        }
        paramsNumber = 0;

        //
        // set the parameter names
        //
        toSplit = strdup(inStrings[1]);
        token = strtok( toSplit, ",;:" );
        while( token != NULL ) {
          char *paramName = strdup(token);
          char *paramPointer = paramName;

          while (paramPointer[0] == ' ') {
            paramPointer++;
          }
          while (paramPointer[strlen(paramPointer)] == ' ') {
            paramPointer[strlen(paramPointer)] = '\0';
          }

          try {
            parser.DefineVar(paramPointer, &parameters[paramsNumber]);
          } catch (mu::Parser::exception_type &e) {
          }

          paramsNumber++;
          token = strtok( NULL, ",;:" );
          free( paramName );
        }
        free(toSplit);

        //
        // set parameter initial guesses
        //
        double pInit[paramsNumber];
        toSplit = strdup(inStrings[2]);
        token = strtok( toSplit, ",;:" );
        while( token != NULL ) {
          pInit[startsNumber] = strtod(token, &endPtr);
          startsNumber++;
          token = strtok( NULL, ",;:" );
        }
        free(toSplit);

        for (i=startsNumber; i<paramsNumber; i++) {
          pInit[i] = 0.0;
        }

        if (strstr(inStrings[0], "pi") != 0L) {
          parser.DefineConst("pi", M_PI);
        }
        if (strstr(inStrings[0], "Pi") != 0L) {
          parser.DefineConst("Pi", M_PI);
        }
        if (strstr(inStrings[0], "PI") != 0L) {
          parser.DefineConst("PI", M_PI);
        }
        parser.DefineVar("x", &xvar);
        parser.SetExpr(inStrings[0]);

        gsl_vector_view x = gsl_vector_view_array(pInit, paramsNumber);

        struct fit d = { n, inArrays[0], inArrays[1], outArrays[0], &xvar, parameters, paramsNumber, &parser };

        const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;

        gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, n, paramsNumber);
        if (s != 0L) {
          gsl_multifit_function_fdf f;
          gsl_matrix *covar = gsl_matrix_alloc(paramsNumber, paramsNumber);
          if (covar != 0L) {
            f.f = &function_f;
            f.df = &function_df;
            f.fdf = &function_fdf;
            f.n = n;
            f.p = paramsNumber;
            f.params = &d;

            gsl_multifit_fdfsolver_set(s, &f, &x.vector);

            int status;
            int iteration = 0;

            do {
              iteration++;
              status = gsl_multifit_fdfsolver_iterate(s);
              if (status) {
                break;
              }
              status = gsl_multifit_test_delta(s->dx, s->x, tolerance, tolerance);
            } while (status == GSL_CONTINUE && iteration < maxIterations);

            gsl_multifit_covar(s->J, 0.0, covar);

            for( i=0; i<n; i++ ) {
              xvar = inArrays[0][i];

              try {
                outArrays[0][i] = parser.Eval();
                outArrays[1][i] = inArrays[1][i] - outArrays[0][i];
              } catch (mu::Parser::exception_type &e) {
                outArrays[0][i] = 0.0;
                outArrays[1][i] = 0.0;
              }
            }

            for (i=0; i<paramsNumber; i++) {
              outArrays[2][i] = parameters[i];
              for (j=0; j<paramsNumber; j++) {
                outArrays[3][(i*paramsNumber)+j] = gsl_matrix_get( covar, i, j );
              }
            }

            //
            // determine the value of chi^2/nu
            //
            outScalars[0] = gsl_blas_dnrm2( s->f );

            iReturn = 0;

            gsl_matrix_free(covar);
          }

          gsl_multifit_fdfsolver_free(s);
        }
      }
    }

    if( pDelete )
    {
      free( pDelete );
    }
  }

  return iReturn;
}

int function_f(const gsl_vector *x, void *params, gsl_vector *f) {
  size_t n = ((struct fit*)params)->n;
  size_t numParams = ((struct fit*)params)->numParams;
  const double *XtoFit = ((struct fit*)params)->XtoFit;
  const double *YtoFit = ((struct fit*)params)->YtoFit;
  double *YFitted = ((struct fit*)params)->YFitted;
  double* parameters = ((struct fit*)params)->parameters;
  double* xvar = ((struct fit*)params)->xvar;
  mu::Parser* parser = ((struct fit*)params)->parser;
  size_t i;

  try {
    for (i=0; i<numParams; i++) {
      parameters[i] = gsl_vector_get(x, i);
    }

    for (i = 0; i < n; i++) {
      *xvar = XtoFit[i];
      YFitted[i] = parser->Eval();
      gsl_vector_set(f, i, (YFitted[i] - YtoFit[i]));
    }
  }
  catch (mu::Parser::exception_type &e) {
    return GSL_FAILURE;
  }

  return GSL_SUCCESS;
}

int function_df(const gsl_vector *x, void *params, gsl_matrix *J) {
  size_t n = ((struct fit*)params)->n;
  size_t numParams = ((struct fit*)params)->numParams;
  const double *XtoFit = ((struct fit*)params)->XtoFit;
  double* parameters = ((struct fit*)params)->parameters;
  double* xvar = ((struct fit*)params)->xvar;
  mu::Parser* parser = ((struct fit*)params)->parser;
  size_t i;

  try {
    for (i=0; i<numParams; i++) {
      parameters[i] = gsl_vector_get(x, i);
    }

    size_t j;

    for (i = 0; i < n; i++) {
      // Jacobian matrix J(i,j) = dfi / dxj
      *xvar = XtoFit[i];
      for (j=0; j<numParams; j++) {
        gsl_matrix_set(J, i, j, parser->Diff(&parameters[j], *xvar));
      }
    }
  } catch (mu::Parser::exception_type &e) {
    return GSL_FAILURE;
  }

  return GSL_SUCCESS;
}


int function_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J) {
  function_f(x, params, f);
  function_df(x, params, J);

  return GSL_SUCCESS;
}
