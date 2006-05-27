/***************************************************************************
                     kstpsdgenerator.cpp: Power Spectra Generator for KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by C. Barth Netterfield
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** A class for generating power spectra 
 *  based on KstPSD
 */

#include <assert.h>
#include <math.h>

#include <kglobal.h>
#include <klocale.h>

#include "kstdebug.h"
#include "kstpsd.h"
#include "kstpsdgenerator.h"

extern "C" void rdft(int n, int isgn, double *a);

#define KSTPSDGENERATORMAXLEN 27
KstPSDGenerator::KstPSDGenerator(QValueVector<double>* inVector, double in_freq,
                                 bool in_average, int in_len,
                                 bool in_apodize, bool in_removeMean, int in_apodizeFxn,
                                 double in_gaussianSigma)
{
  _inputVector = inVector;
  _Freq = in_freq;
  _Average = in_average;
  _Len = in_len;
  _Apodize = in_apodize;
  _apodizeFxn = in_apodizeFxn;
  _gaussianSigma = in_gaussianSigma;
  _prevApodizeFxn = -1;
  _RemoveMean = in_removeMean;

  if (!_Average && _inputVector) {
    _Len =  int(ceil(log(double(_inputVector->size())) / log(2.0)));
  }

  if (_Len < 2) {
    _Len = 2;
  }

  if (_Len > KSTPSDGENERATORMAXLEN) {
    _Len = KSTPSDGENERATORMAXLEN;
  }

  if (_Freq <= 0.0) {
    _Freq = 1.0;
  }

  _PSDLen = int(pow(2.0, (double)(_Len-1)));
  _ALen = _PSDLen*2;
  _a = new double[_ALen];
  _w = new double[_ALen];
  _WLen = 0;

  _last_f0 = 0;
  _last_n_subsets = 0;
  _last_n_new = 0;

  _frequencyVector = new QValueVector<double>(_PSDLen);
  if ((int)_frequencyVector->size() != _PSDLen) {
    _PSDLen = 1;
    _Len = 1;
    KstDebug::self()->log(i18n("Insufficient memory for frequency vector of PSDGenerator."), KstDebug::Error);
  }

  _powerVector = new QValueVector<double>(_PSDLen);
  if ((int)_powerVector->size() != _PSDLen) {
    _PSDLen = 1;
    _Len = 1;
    KstDebug::self()->log(i18n("Insufficient memory for power vector of PSDGenerator."), KstDebug::Error);
  }
}

KstPSDGenerator::~KstPSDGenerator() {
  delete[] _w;
  _w = 0L;
  delete[] _a;
  _a = 0L;
  
  delete _frequencyVector;
  _frequencyVector = 0L;
  delete _powerVector;
  _powerVector = 0L;
}

void KstPSDGenerator::generateWindow(int len) {
  if (len > 0 && len != _WLen || _prevApodizeFxn != _apodizeFxn) {
    KstPSD::createWindow(_apodizeFxn, _w, len, _gaussianSigma);
    
    // remember last used apodization function and length
    _WLen = len;
    _prevApodizeFxn = _apodizeFxn;
  }
}


double KstPSDGenerator::cabs2(double r, double i) {
  return (r*r + i*i);
}


void KstPSDGenerator::updateNow() {
  int i_subset, i_samp;
  int n_subsets;
  int v_len;
  int copyLen;
  QValueVector<double> psd, f;
  double mean;
  double nf;
  bool done;
  
  adjustLengths();
  
  v_len = _inputVector->size();
  n_subsets = v_len/_PSDLen+1;
  if (v_len%_PSDLen==0) n_subsets--;

  // always update when asked
  _last_n_new = _inputVector->size();

  for (i_samp = 0; i_samp < _PSDLen; i_samp++) {
    (*_powerVector)[i_samp] = 0;
    (*_frequencyVector)[i_samp] = i_samp*0.5*_Freq/( _PSDLen-1 );
  }
  _frequencyVectorStep = 0.5*_Freq/(_PSDLen - 1);

  nf = 0;
  done = false;
  for (i_subset = 0; !done; i_subset++) {
    // copy each chunk into a[] and find mean
    if (i_subset*_PSDLen + _ALen < v_len) {
      copyLen = _ALen;
    } else {
      copyLen = v_len - i_subset*_PSDLen;
      done = true;
    }

    mean = 0;
    for (i_samp = 0; i_samp < copyLen; i_samp++) {
      mean += (
        _a[i_samp] =
        _inputVector->at(i_samp + i_subset*_PSDLen)
        );
    }
    if (copyLen > 1) {
      mean /= (double)copyLen;
    }

    if (apodize()) {
      generateWindow(copyLen);
    }
     
    // remove mean and apodize
    if (removeMean() && apodize()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        _a[i_samp]= (_a[i_samp]-mean)*_w[i_samp];
      }
    } else if (removeMean()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        _a[i_samp] -= mean;
      }
    } else if (apodize()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        _a[i_samp] *= _w[i_samp];
      }
    }
    nf += copyLen;


    for (;i_samp < _ALen; i_samp++) {
      _a[i_samp] = 0.0;
    }
    // fft a
    rdft(_ALen, 1, _a);
    (*_powerVector)[0] += _a[0]*_a[0];
    (*_powerVector)[_PSDLen-1] += _a[1]*_a[1];
    for (i_samp=1; i_samp<_PSDLen-1; i_samp++) {
      (*_powerVector)[i_samp]+= cabs2(_a[i_samp*2], _a[i_samp*2+1]);
    }
  }

  _last_f0 = 0;
  _last_n_subsets = n_subsets;
  _last_n_new = 0;
  nf = 1.0/(double(_Freq)*double(nf/2.0));
  for ( i_samp = 0; i_samp<_PSDLen; i_samp++ ) {
    (*_powerVector)[i_samp] = sqrt((*_powerVector)[i_samp]*nf);
  }
  if (_Freq <= 0.0) {
    _Freq = 1.0;
  }
}


bool KstPSDGenerator::apodize() const {
  return _Apodize;
}


void KstPSDGenerator::setApodize(bool in_apodize)  {
  _Apodize = in_apodize;
}


bool KstPSDGenerator::removeMean() const {
  return _RemoveMean;
}


void KstPSDGenerator::setRemoveMean(bool in_removeMean) {
  _RemoveMean = in_removeMean;
}


bool KstPSDGenerator::average() const {
  return _Average;
}


void KstPSDGenerator::setAverage(bool in_average) {
  _Average = in_average;
}


double KstPSDGenerator::freq() const {
  return _Freq;
}


void KstPSDGenerator::setFreq(double in_freq) {
  if (in_freq > 0.0) {
    _Freq = in_freq;
  } else {
    _Freq = 1;
  }
}


int KstPSDGenerator::length() const {
  return _Len;
}

void KstPSDGenerator::setLength(int in_len) {
  if (in_len >= 4 && in_len <= 27) {
    _Len = in_len;
  } else {
    _Len = 10;
  }
}

void KstPSDGenerator::adjustLengths() {
  int psdlen, len;

  if (_Average) {
    psdlen = int(pow(2,_Len-1));
  } else {
    len = int(ceil(log(double(_inputVector->size())) / log(2.0)));
    psdlen = int(pow(2,len-1));
  }

  if (_PSDLen != psdlen) {
    _PSDLen = psdlen;
    _powerVector->resize(_PSDLen);
    _frequencyVector->resize(_PSDLen);

    _ALen = _PSDLen*2;
    delete[] _a;
    _a = new double[_ALen];
    delete[] _w;
    _w = new double[_ALen];

    _last_f0 = 0;
    _last_n_subsets = 0;
  }
}

void KstPSDGenerator::setInputVector(QValueVector<double>* inVector) {
  _inputVector = inVector;
}

double KstPSDGenerator::frequencyVectorStep() const {
  return _frequencyVectorStep;
}

int KstPSDGenerator::apodizeFxn() const {
  return _apodizeFxn;  
}

void KstPSDGenerator::setApodizeFxn(bool in_apodizeFxn) {
  _apodizeFxn = in_apodizeFxn;
}

double KstPSDGenerator::gaussianSigma() const {
  return _gaussianSigma;
}

void KstPSDGenerator::setGaussianSigma(double in_gaussianSigma) {
  _gaussianSigma = in_gaussianSigma;
}

// vim: ts=2 sw=2 et
