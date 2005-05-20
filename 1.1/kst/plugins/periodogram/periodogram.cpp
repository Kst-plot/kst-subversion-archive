/*
 *  Periodogram plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 *  Notes:            
 *    The original version of this program was based on the algorithm 
 *    described in Press, W.H, and Rybicki, G.B., Astrophysical J. 
 *    338:277-280 (1989).  It has been rewritten using the versions 
 *    of this algorithm presented in Numerical Recipes in C 
 *    (Press, Teukolsky, Vetterling, and Flannery;  Cambridge U. Press, 1992).
 */

#include <stdlib.h>
#include <math.h>

#define ONE_PI	3.1415926535897930475926
#define TWO_PI	6.2831853071795858696103
#define MACC	4.0
#define KST_UNUSED(x) if(x){};

int max( int a, int b );
int min( int a, int b );
void spread( double y, double yy[], unsigned long n, double x, int m );
void four1( double data[], unsigned long nn, int isign );
void realft( double data[], unsigned long n, int isign );
void avevar( double const  data[], unsigned long n, double* ave, double* var );
void FastLombPeriodogram(    
    double const x[], 
    double const y[], 
    unsigned long   n, 
    double          ofac, 
    double          hifac, 
    double          wk1[], 
    double          wk2[], 
    unsigned long   ndim,
    unsigned long*  nout, 
    unsigned long*  jmax, 
    double*         prob,
    double*         pvar,
    int            iIsWindowFunction );
void SlowLombPeriodogram(
    double const    x[], 
    double const    y[], 
    unsigned long   n, 
    double          ofac, 
    double          hifac, 
    double          px[], 
    double          py[], 
    unsigned long   ndim,
    unsigned long*  nout, 
    unsigned long*  jmax, 
    double*         prob,
    double*         pvar,
    int            iIsWindowFunction );
extern "C" int periodogram( const double *const inArrays[], const int inArrayLens[],
		const double is[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );


int max( int a, int b )
{
    if( a > b )
    {
        return a; 
    }
    return b;
}

int min( int a, int b )
{
    if( a < b )
    {
        return a;
    }
    return b;
}

void spread( double y, double yy[], unsigned long n, double x, int m )
{
    int         ihi;
    int         ilo;
    int         ix;
    int         j;
    int         nden;
    double      fac;
    static int  nfac[] = {  0, 
                            1, 
                            1, 
                            2, 
                            6, 
                            24, 
                            120, 
                            720, 
                            5040, 
                            40320, 
                            362880, 
                            39916800, 
                            479001600 };

    ix = (int)x;
    if( x == (double)ix ) 
    {
        yy[ix] += y;
    }
    else 
    {
        ilo     = min( max( (int)( x - 0.5*m + 1.0 ), 1 ), (int)( n - m + 1 ) );      
        ihi     = ilo + m - 1;
        nden    = nfac[m];
        fac     = x - ilo;
        for( j = ilo + 1; j <= ihi; j++ ) 
        {
            fac *= x - (double)j;
        }
        yy[ihi] += y*fac/(double)(nden*(x - ihi));
        for( j = ihi-1; j >= ilo; j-- ) 
        {
            nden=(nden/(j+1-ilo))*(j-ihi);
            yy[j] += y*fac/(double)(nden*(x - j));
        }
    }
}

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
        istep   = mmax * 2;
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

void avevar(    
        double const  data[], 
        unsigned long n, 
        double*          ave, 
        double*         	 var )
{
    unsigned long   j;
    double          s;
    double          ep;
    
    *ave    = 0.0;
    *var    = 0.0;
    ep      = 0.0;

    if( n > 0 )
    {
        for( *ave = 0.0, j = 1; j <= n; j++) 
        {
            *ave += data[j];
        }
        *ave   /= n;

        if( n > 1 )
        {
            for( j = 1; j <= n; j++ ) 
            {
	            s       = data[j] - (*ave);
	            ep     += s;
	            *var   += s*s;
            }

            *var    = ( *var - ep * ep / n ) / ( n - 1 );
        }
    }
}

void FastLombPeriodogram(    
    double const x[], 
    double const y[], 
    unsigned long   n, 
    double          ofac, 
    double          hifac, 
    double          wk1[], 
    double          wk2[], 
    unsigned long   ndim,
    unsigned long*  nout, 
    unsigned long*  jmax, 
    double*         prob,
    double*         pvar,
    int            iIsWindowFunction )
{
    unsigned long   j;
    unsigned long   k;
    double          ave;
    double          ck;
    double          ckk;
    double          cterm;
    double          cwt;
    double          den;
    double          df;
    double          effm;
    double          expy;
    double          fac;
    double          fndim;
    double          hc2wt;
    double          hs2wt;
    double          hypo;
    double          pmax;
    double          sterm;
    double          swt;
    double          xdif;
    double          xmax;
    double          xmin;

    if( n > 0 )
    {
        *nout   = (unsigned long)( 0.5  * ofac * hifac * n );

        if( iIsWindowFunction )
        {
            ave   = 0.0;
            *pvar = 0.0;
        }
        else
        {
            avevar( y, n, &ave, pvar );
        }

        xmax = x[1];
        xmin = x[1];
        for( j = 2; j <= n; j++ ) 
        {
	        if( x[j] < xmin ) 
            {
                xmin = x[j];
            }
	        if( x[j] > xmax ) 
            {
                xmax = x[j];
            }
        }
        xdif = xmax - xmin;
        for( j = 1; j <= ndim; j++ )
        {
            wk1[j] = 0.0;
            wk2[j] = 0.0;
        }

        fac     = ndim / ( xdif * ofac );
        fndim   = ndim;

        for( j = 1; j <= n; j++ ) 
        {
	        ck  = ( x[j] - xmin ) * fac;
	        ck  = fmod( ck, fndim );
	        ckk = 2.0*(ck++);
	        ckk = fmod( ckk, fndim );
	        ++ckk;
	        spread( y[j] - ave, wk1, ndim, ck, (int)MACC );
	        spread( 1.0, wk2, ndim, ckk, (int)MACC );
        }

        realft( wk1, ndim, 1 );
        realft( wk2, ndim, 1 );
        df      = 1.0 / ( xdif * ofac );
        pmax    = -1.0;
        for( k = 3, j = 1; j <= (*nout); j++, k += 2 )
        {
	        hypo    = sqrt( wk2[k]*wk2[k] + wk2[k+1]*wk2[k+1] );
	        hc2wt   = 0.5 * wk2[k  ] / hypo;
	        hs2wt   = 0.5 * wk2[k+1] / hypo;
	        cwt     = sqrt( 0.5 + hc2wt );
	        swt     = fabs( sqrt( 0.5 - hc2wt ) );
            if( hs2wt < 0.0 )
            {
                swt *= -1.0;
            }
	        den     = 0.5*n + hc2wt*wk2[k] + hs2wt*wk2[k+1];
	        cterm   = pow( cwt*wk1[k+0] + swt*wk1[k+1], 2.0 ) / den;
            if( (double)n - den == 0.0 )
            {
    	        sterm   = 0.0;
            }
            else
            {
    	        sterm   = pow( cwt*wk1[k+1] - swt*wk1[k+0], 2.0 ) / ( (double)n - den );
            }
	        wk1[j]  = (double)j * df;
	        wk2[j]  = ( cterm + sterm );
            if( *pvar > 0.0 )
            {
                wk2[j] /= ( 2.0 * *pvar );
            }
	        if( wk2[j] > pmax ) 
            {
                pmax = wk2[ (*jmax = j) ];
            }
        }
        expy    = exp( -pmax );
        effm    = 2.0 * (*nout) / ofac;
        *prob   = effm * expy;
        if( *prob > 0.01 )
        {
            *prob = 1.0 - pow( 1.0 - expy, effm );
        }
    }
    else
    {
        *nout = 0;
    }
}

void SlowLombPeriodogram(
    double const    x[], 
    double const    y[], 
    unsigned long   n, 
    double          ofac, 
    double          hifac, 
    double          px[], 
    double          py[], 
    unsigned long   ndim,
    unsigned long*  nout, 
    unsigned long*  jmax, 
    double*         prob,
    double*         pvar,
    int            iIsWindowFunction )
{        
    KST_UNUSED( ndim )
            
    unsigned long   i;
    unsigned long   j;
    double*         wi  = NULL;
    double*         wpi = NULL;
    double*         wpr = NULL;
    double*         wr  = NULL;
    double          arg;
    double          wtemp;
    double          ave;
    double          c;
    double          cc;
    double          cwtau;
    double          effm;
    double          expy;
    double          pnow;
    double          pymax;
    double          s;
    double          ss;
    double          sumc;
    double          sumcy;
    double          sums;
    double          sumsh;
    double          sumsy;
    double          swtau;
    double          wtau;
    double          xave;
    double          xdif;
    double          xmax;
    double          xmin;
    double          yy;

    if( n > 0 )
    {
        wi  = (double*)calloc( n+1, sizeof( double ) );
        wpi = (double*)calloc( n+1, sizeof( double ) );
        wpr = (double*)calloc( n+1, sizeof( double ) );
        wr  = (double*)calloc( n+1, sizeof( double ) );

        if( wi  != NULL &&
            wpi != NULL &&
            wpr != NULL &&
            wr  != NULL )
        {
            *nout = (unsigned long)(0.5*ofac*hifac*n);
        
            if( iIsWindowFunction )
            {
                ave   = 0.0;
                *pvar = 0.0;
            }
            else
            {
                avevar( y, n, &ave, pvar );
            }

            xmax = x[1];
            xmin = x[1];
            for( j=1; j<=n; j++ ) 
            {
                if( x[j] > xmax )
                {
                    xmax = x[j];
                }
                if( x[j] < xmin )
                {
                    xmin = x[j];
                }
            }
            xdif	= xmax-xmin;
            xave    	= 0.5*(xmax+xmin);
            pymax   	= 0.0;
            pnow    	= 1.0/(xdif*ofac);
            for( j=1; j<=n; j++ ) 
            {
                arg     = TWO_PI*((x[j]-xave)*pnow);
                wpr[j]  = -2.0*(sin(0.5*arg)*sin(0.5*arg));
                wpi[j]  = sin(arg);
                wr[j]   = cos(arg);
                wi[j]   = wpi[j];
            }
            for( i=1; i<=(*nout); i++ ) 
            {
                sumsh = 0.0;
                sumc  = 0.0;
                px[i] = pnow;

                for (j=1;j<=n;j++) 
                {
                    c   = wr[j];
                    s   = wi[j];
                    sumsh += s*c;
                    sumc  += (c-s)*(c+s);
                }
                wtau    = 0.5 * atan2( 2.0 * sumsh, sumc );
                swtau   = sin(wtau);
                cwtau   = cos(wtau);
                sums    = 0.0;
                sumc    = 0.0;
                sumsy   = 0.0;
                sumcy   = 0.0;
                for( j=1; j<=n; j++ ) 
                {
                    s       = wi[j];
                    c       = wr[j];
                    ss      = s*cwtau-c*swtau;
                    cc      = c*cwtau+s*swtau;
                    sums   += ss*ss;
                    sumc   += cc*cc;
                    yy      = y[j]-ave;
                    sumsy  += yy*ss;
                    sumcy  += yy*cc;
                    wtemp   = wr[j];
                    wr[j]   = (wr[j]*wpr[j]-wi[j]*wpi[j])+wr[j];
                    wi[j]   = (wi[j]*wpr[j]+wtemp*wpi[j])+wi[j];
                }
                py[i]   = ( sumcy * sumcy / sumc + sumsy * sumsy / sums );
                if( *pvar > 0.0 )
                {
                    py[i] /= ( 2.0 * (*pvar) );
                }
                if( py[i] >= pymax ) 
                {
                    *jmax = i;
                    pymax = py[i];
                }
                pnow += 1.0/(xdif*ofac);
            }
            expy    = exp(-pymax);
            effm    = 2.0*(*nout)/ofac;
            *prob   = effm*expy;
            if( *prob > 0.01 )
            {
                *prob = 1.0 - pow( 1.0 - expy, effm );
            }
        }

        if( wi  != NULL )
        {
            free( wi );
        }
        if( wpi != NULL )
        {
            free( wpi );
        }
        if( wpr != NULL )
        {
            free( wpr );
        }
        if( wr  != NULL )
        {
            free( wr );
        }
    }
    else
    {
        *nout = 0;
    }
}

int periodogram( const double *const inArrays[], const int inArrayLens[],
                        const double inScalars[],
                        double *outArrays[], int outArrayLens[],
                        double outScalars[] )
{
    KST_UNUSED( outScalars )
    
    unsigned long lSizeWork;
    unsigned long lSizeOut = 0;
    unsigned long lSizeIn;
    unsigned long lFreqt;
    unsigned long lFreq;
    unsigned long lMax;
    double	dProb;
    double	dVar;
    void*	pResult[2];
    int 	iRetVal = -1;
        
    if( inArrayLens[0] == inArrayLens[1] )
    {
        lSizeIn = inArrayLens[0];
        if( lSizeIn > 1 )
        {
            lFreqt      = (unsigned long)( MACC * inScalars[0] * inScalars[1] * lSizeIn );
            lFreq       = 64;
            while( lFreq < lFreqt ) 
            {
                lFreq *= 2;
            }
            lSizeWork   = lFreq * 2;
           
            if( lSizeIn != lSizeWork )
            {
                pResult[0] = (double*)realloc( outArrays[0], lSizeWork * sizeof( double ) );
            }
            else
            {
                pResult[0] = outArrays[0];
            }
            
            if( lSizeIn != lSizeWork )
            {
                pResult[1] = (double*)realloc( outArrays[1], lSizeWork * sizeof( double ) );
            }
            else
            {
                pResult[1] = outArrays[1];
            }
            
            if( pResult[0] != NULL && pResult[1] != NULL )
            {
                outArrays[0] = (double*)pResult[0];
                outArrays[1] = (double*)pResult[1];
                              
                if( lSizeIn > 100 )
                {
                    FastLombPeriodogram( 
                            (double*)&(inArrays[0][-1]), 
                            (double*)&(inArrays[1][-1]), 
                            inArrayLens[0], 
                            inScalars[0], 
                            inScalars[1], 
                            (double*)&(outArrays[0][-1]),
                            (double*)&(outArrays[1][-1]),
                            lSizeWork,
                            &lSizeOut,
                            &lMax,
                            &dProb, 
                            &dVar, 
                            0 );
                }
                else
                {
                    SlowLombPeriodogram(
                            (double*)&(inArrays[0][-1]), 
                            (double*)&(inArrays[1][-1]), 
                            inArrayLens[0], 
                            inScalars[0], 
                            inScalars[1], 
                            (double*)&(outArrays[0][-1]),
                            (double*)&(outArrays[1][-1]),
                            lSizeWork,
                            &lSizeOut,
                            &lMax,
                            &dProb, 
                            &dVar, 
                            0 );
                }
             
                if( lSizeOut > 0 && lSizeOut <= lSizeWork )
                {
                    outArrayLens[0] = lSizeOut;
                    outArrayLens[1] = lSizeOut;
                
                    iRetVal = 0;
                }
            }            
        }
    }
    
    return iRetVal;
}

