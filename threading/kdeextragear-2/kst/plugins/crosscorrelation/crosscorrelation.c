/*
 *  Auto-correlation plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ONE_PI	3.1415926535897930475926
#define TWO_PI	6.2831853071795858696103
#define KST_UNUSED(x) if(x){};

void four1( double data[], unsigned long nn, int isign );
void realft( double data[], unsigned long n, int isign );
void twofft( double* pData[], double* pResult[], int iLength );
int crosscorrelation( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

void four1( double data[], unsigned long nn, int isign )
{
    unsigned long   n;
    unsigned long   mmax;
    unsigned long   m;
    unsigned long   j;
    unsigned long   istep;
    unsigned long   i;
    double          dtemp;
    double          wtemp;
    double          wr;
    double          wpr;
    double          wpi;
    double          wi;
    double          theta;
    double          tempr;
    double          tempi;

    n = nn << 1;
    j = 1;
    for( i = 1; i < n; i += 2 ) 
    {
	    if( j > i ) 
        {
            dtemp       = data[i];
            data[i]     = data[j];
            data[j]     = dtemp;

            dtemp       = data[i+1];
            data[i+1]   = data[j+1];
            data[j+1]   = dtemp;
        }
    	m = n >> 1;
	    while( m >= 2 && j > m )
        {
	        j -= m;
	        m >>= 1;
	    }
	    j += m;
    }

    mmax = 2;
    while( n > mmax ) 
    {
	    istep   = mmax << 1;
	    theta   = isign*(TWO_PI/mmax);
	    wtemp   = sin(0.5*theta);
	    wpr     = -2.0*wtemp*wtemp;
	    wpi     = sin(theta);
	    wr      = 1.0;
	    wi      = 0.0;

        for( m = 1; m < mmax; m += 2 ) 
        {
	        for( i = m;i <= n;i += istep ) 
            {
		        j           = i + mmax;
		        tempr       = wr*data[j]   - wi*data[j+1];
		        tempi       = wr*data[j+1] + wi*data[j];
		        data[j]     = data[i]   - tempr;
		        data[j+1]   = data[i+1] - tempi;
		        data[i]    += tempr;
		        data[i+1]  += tempi;
	        }
	        wr = (wtemp = wr)*wpr - wi*wpi + wr;
	        wi = wi*wpr + wtemp*wpi + wi;
	    }
	    mmax = istep;
    }
}

void realft( double data[], unsigned long n, int isign )
{
    unsigned long   i;
    unsigned long   i1;
    unsigned long   i2;
    unsigned long   i3;
    unsigned long   i4;
    unsigned long   np3;
    double          c1 = 0.5;
    double          c2;
    double          h1r;
    double          h1i;
    double          h2r;
    double          h2i;
    double          wr;
    double          wi;
    double          wpr;
    double          wpi;
    double          wtemp;
    double          theta;

    theta = ONE_PI / (double)(n>>1);
    if( isign == 1 ) 
    {
	    c2      = -0.5;
	    four1( data, n>>1, 1 );
    } 
    else 
    {
	    c2      = 0.5;
	    theta   = -theta;
    }
    wtemp   = sin(0.5*theta);
    wpr     = -2.0*wtemp*wtemp;
    wpi     = sin(theta);
    wr      = 1.0 + wpr;
    wi      = wpi;
    np3     = n+3;
    for( i = 2; i <= (n>>2); i++ ) 
    {
        i1          =  (2*i) - 1;
        i2          =  i1 + 1;
        i3          =  np3 - i2;
	    i4          =  i3 + 1;
	    h1r         =  c1*(data[i1] + data[i3]);
	    h1i         =  c1*(data[i2] - data[i4]);
	    h2r         = -c2*(data[i2] + data[i4]);
	    h2i         =  c2*(data[i1] - data[i3]);
	    data[i1]    =  h1r + wr*h2r - wi*h2i;
	    data[i2]    =  h1i + wr*h2i + wi*h2r;
	    data[i3]    =  h1r - wr*h2r + wi*h2i;
	    data[i4]    = -h1i + wr*h2i + wi*h2r;
        wtemp       = wr;
	    wr          = wr*wpr - wi*wpi + wr;
	    wi          = wi*wpr + wtemp*wpi + wi;
    }

    if( isign == 1 ) 
    {
        h1r     = data[1];
	    data[1] = h1r + data[2];
	    data[2] = h1r - data[2];
    } 
    else 
    {
        h1r     = data[1];
	    data[1] = c1*(h1r + data[2]);
	    data[2] = c1*(h1r - data[2]);
	    four1( data, n>>1, -1 );
    }
}

void twofft( double* pData[], double* pResult[], int iLength )
{
    unsigned long 	ulNum;
    double		dRep;
    double		dRem;
    double		dAim;
    double		dAip;
    int		i;
    int		j;
    
    ulNum = iLength + iLength;
    
    for( i=0, j=0; i<iLength; i++, j+=2 )
    {
        pResult[0][j+0] = pData[0][i];
        pResult[0][j+1] = pData[1][i];
    }
    
    four1( &(pResult[0][-1]), iLength, 1 );
    
    pResult[1][0] = pResult[0][1];
    pResult[0][1] = 0.0;
    pResult[1][1] = 0.0;
    
    for( i=2; i<=iLength; i+=2 )
    {
        dRep = 0.5 * ( pResult[0][i+0] + pResult[0][ulNum+0-i] );
        dRem = 0.5 * ( pResult[0][i+0] - pResult[0][ulNum+0-i] );
        dAip = 0.5 * ( pResult[0][i+1] + pResult[0][ulNum+1-i] );
        dAim = 0.5 * ( pResult[0][i+1] - pResult[0][ulNum+1-i] );
        
        pResult[0][i] = dRep;
        pResult[0][i+1] = dAim;
        pResult[0][ulNum-i] = dRep;
        pResult[0][ulNum+1-i] = -dAim;
        
        pResult[1][i] = dAip;
        pResult[1][i+1] = -dRem;
        pResult[1][ulNum-i] = dAip;
        pResult[1][ulNum+1-i] = dRem;        
    }
}

int crosscorrelation( const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] )
{
    KST_UNUSED( inScalars )
    KST_UNUSED( outScalars )
    
    double* pData[2];
    double* pResult[2];
    double* pTransform[2];
    double dTemp;
    int iRetVal = -1;
    int iNumPoints = 8;
    int iLength;
    int i;
    
    if( inArrayLens[0] == inArrayLens[1] )
    {
        iLength = inArrayLens[0];
        
        /*
        // determine the number of points required...
        */
        while( iNumPoints < iLength && iNumPoints > 0 )
        {
            iNumPoints *= 2;
        }
  
        if( iNumPoints > 0 )
        {
            pData[0] = calloc( iNumPoints, sizeof( double ) );               
            if( pData[0] != NULL )
            {
                pData[1] = calloc( iNumPoints, sizeof( double ) );
                if( pData[1] != NULL )
                {         
                    memcpy( pData[0], inArrays[0], iLength * sizeof( double ) );
                    memcpy( pData[1], inArrays[1], iLength * sizeof( double ) );
           
                    if( outArrayLens[0] != iNumPoints )
                    {
                        pResult[0] = realloc( outArrays[0], iNumPoints * sizeof( double ) );
                        pResult[1] = realloc( outArrays[1], iNumPoints * sizeof( double ) );
                    }
                    else
                    {
                        pResult[0] = outArrays[0];
                        pResult[1] = outArrays[1];
                    }
            
                    if( pResult[0] != NULL && pResult[1] != NULL )
                    {
                        outArrays[0] = pResult[0];
                        outArrayLens[0] = iNumPoints;
                        
                        outArrays[1] = pResult[1];
                        outArrayLens[1] = iNumPoints;

                        pTransform[0] = calloc( 2 * iNumPoints, sizeof( double ) );
                        if( pTransform[0] != NULL )
                        {
                            pTransform[1] = calloc( 2 * iNumPoints, sizeof( double ) );
                            if( pTransform[1] != NULL )
                            {                              
                                twofft( pData, pTransform, iNumPoints );
    
                                for( i=0; i<iNumPoints; i++ )
                                {
                                    outArrays[0][i] = (double)( i - ( iNumPoints / 2 ) - 1 );
                                }
                        
                                for( i=0; i<iNumPoints; i+=2 )
                                {
                                    dTemp = pTransform[1][i+0];
                                    pTransform[1][i+0] = ( pTransform[0][i+0] * dTemp + pTransform[0][i+1] * pTransform[1][i+1] ) / (double)( iNumPoints / 2 );
                                    pTransform[1][i+1] = ( pTransform[0][i+1] * dTemp - pTransform[0][i+0] * pTransform[1][i+1] ) / (double)( iNumPoints / 2 );
                                }
                    
                                pTransform[1][1] = pTransform[1][iNumPoints];
                            
                                realft( &(pTransform[1][-1]), iNumPoints, -1 );
                        
                                memcpy( &(outArrays[1][iNumPoints / 2]), &(pTransform[1][0]), sizeof( double ) * iNumPoints / 2 );
                                memcpy( &(outArrays[1][0]), &(pTransform[1][iNumPoints / 2]), sizeof( double ) * iNumPoints / 2 );

                                iRetVal = 0;
                        
                                free( pTransform[1] );
                            }                    
                            free( pTransform[0] );
                        } 	
                    } 
                    free( pData[1] );
                }
                free( pData[0] );
            }
        }
    }

    return iRetVal;
}

