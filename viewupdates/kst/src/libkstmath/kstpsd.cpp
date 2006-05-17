/***************************************************************************
                          kstpsd.cpp: Power Spectra for KST
                             -------------------
    begin                : Fri Feb 10 2002
    copyright            : (C) 2002 by C. Barth Netterfield
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

/** A class for handling power spectra for kst
 *@author C. Barth Netterfield
 */

#include <assert.h>
#include <math.h>

#include <qstylesheet.h>

#include <kglobal.h>
#include <klocale.h>
#include "ksdebug.h"

#include "dialoglauncher.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstpsd.h"
#include "kstobjectdefaults.h"

extern "C" void rdft(int n, int isgn, double *a);

const QString& KstPSD::INVECTOR = KGlobal::staticQString("I");
const QString& KstPSD::SVECTOR = KGlobal::staticQString("S");
const QString& KstPSD::FVECTOR = KGlobal::staticQString("F");

#define KSTPSDMAXLEN 27
KstPSD::KstPSD(const QString &in_tag, KstVectorPtr in_V,
                         double in_freq, bool in_average, int in_len,
                         bool in_apodize, bool in_removeMean,
                         const QString &in_VUnits, const QString &in_RUnits, int in_apodizeFxn, 
                         double in_gaussianSigma)
: KstDataObject() {
  commonConstructor(in_tag, in_V, in_freq, in_average, in_len,
                    in_apodize, in_removeMean,
                    in_VUnits, in_RUnits, in_apodizeFxn, in_gaussianSigma);
                    
  setDirty();
}


KstPSD::KstPSD(const QDomElement &e)
: KstDataObject(e) {
  QString in_VUnits;
  QString in_RUnits;
  QString in_tag;
  QString vecName;
  KstVectorPtr in_V;
  double in_freq = 60.0;
  bool in_average = true;
  bool in_removeMean = true;
  bool in_apodize = true;
  int in_apodizeFxn = 0;
  double in_gaussianSigma = 3.0;
  int in_len = 12;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull()) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "vectag") {
        vecName = e.text();
      } else if (e.tagName() == "sampRate") {
        in_freq = e.text().toDouble();
      } else if (e.tagName() == "average") {
        if (e.text() == "0") {
          in_average = false;
        } else {
          in_average = true;
        }
      } else if (e.tagName() == "fftLen") {
        in_len = e.text().toInt();
      } else if (e.tagName() == "apodize") {
        if (e.text() == "0") {
          in_apodize = false;
        } else {
          in_apodize = true;
        }
      } else if (e.tagName() == "apodizefxn") {
        in_apodizeFxn = e.text().toInt();
      } else if (e.tagName() == "gaussiansigma") {
        in_gaussianSigma = e.text().toDouble();
      } else if (e.tagName() == "removeMean") {
        if (e.text() == "0") {
          in_removeMean = false;
        } else {
          in_removeMean = true;
        }
      } else if (e.tagName() == "VUnits") {
        in_VUnits = e.text();
      } else if (e.tagName() == "RUnits") {
        in_RUnits = e.text();
      }
    }
    n = n.nextSibling();
  }

  _inputVectorLoadQueue.append(qMakePair(INVECTOR, vecName));
  commonConstructor(in_tag, in_V, in_freq, in_average, in_len,
                    in_apodize, in_removeMean,
                    in_VUnits, in_RUnits, in_apodizeFxn, in_gaussianSigma);
}


void KstPSD::commonConstructor(const QString& in_tag, KstVectorPtr in_V,
                               double in_freq, bool in_average, int in_len,
                               bool in_apodize, bool in_removeMean,
                               const QString& in_VUnits,
                               const QString& in_RUnits, int in_apodizeFxn, double in_gaussianSigma) {

  _typeString = i18n("Power Spectrum");
  _type = "PowerSpectrum";
  _inputVectors[INVECTOR] = in_V;
  setTagName(in_tag);
  _Freq = in_freq;
  _Average = in_average;
  _Len = in_len;
  _Apodize = in_apodize;
  _apodizeFxn = in_apodizeFxn;
  _gaussianSigma = in_gaussianSigma;
  _prevApodizeFxn = -1;
  _RemoveMean = in_removeMean;
  _vUnits = in_VUnits;
  _rUnits = in_RUnits;

  if (!_Average && _inputVectors[INVECTOR]) {
    _Len =  int(ceil(log(double(_inputVectors[INVECTOR]->length())) / log(2.0)));
  }

  if (_Len < 2) {
    _Len = 2;
  }

  if (_Len > KSTPSDMAXLEN) {
    _Len = KSTPSDMAXLEN;
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

  KstVectorPtr ov = new KstVector(in_tag+"-freq", _PSDLen);
  if (ov->length() != _PSDLen) {
    _PSDLen = 1;
    _Len = 1;
    KstDebug::self()->log(i18n("Attempted to create a PSD that used all memory."), KstDebug::Error);
  }
  KST::addVectorToList(ov);
  ov->setProvider(this);
  _fVector = _outputVectors.insert(FVECTOR, ov);
  (*_fVector)->setLabel(i18n("Frequency [%1]").arg(_rUnits));

  ov = new KstVector(in_tag+"-sv", _PSDLen);
  if (ov->length() != _PSDLen) {
    _PSDLen = 1;
    _Len = 1;
    KstDebug::self()->log(i18n("Attempted to create a PSD that used all memory."), KstDebug::Error);
  }
  KST::addVectorToList(ov);
  ov->setProvider(this);
  _sVector = _outputVectors.insert(SVECTOR, ov);
  (*_sVector)->setLabel(i18n("PSD [%1/%2^{1/2}]").arg(_vUnits).arg(_rUnits));
}


KstPSD::~KstPSD() {
  _sVector = _outputVectors.end();
  _fVector = _outputVectors.end();
  KST::vectorList.lock().writeLock();
  KST::vectorList.remove(_outputVectors[SVECTOR]);
  KST::vectorList.remove(_outputVectors[FVECTOR]);
  KST::vectorList.lock().writeUnlock();

  delete[] _w;
  _w = 0L;
  delete[] _a;
  _a = 0L;
}


const KstCurveHintList *KstPSD::curveHints() const {
  _curveHints->clear();
  _curveHints->append(new KstCurveHint(i18n("PSD Curve"), (*_fVector)->tagName(), (*_sVector)->tagName()));
  return _curveHints;
}


void KstPSD::GenW(int len) {
  int i;
  double sW = 0;

  if (len != _WLen || _prevApodizeFxn != _apodizeFxn) {
    double a = len/2;
    double x;
    
    switch (_apodizeFxn) {
      // original apodization function
      case 0: 
        for (i = 0; i < len; i++) {
          _w[i] = (1 - cos(2*M_PI*double(i)/double(len)));
          sW += _w[i] * _w[i];
        }
        sW = sqrt(sW/double(len));
        for (i = 0; i < len; i++) {
          _w[i] /= sW;
        }
        break;
      // Bartlett function
      case 1:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = (1 - fabs(x)/a);
        }
        break;
      // Blackman function 
      case 2:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = 0.42 + 0.5*cos(M_PI*x/a) + 0.08*cos(2*M_PI*x/a);
        }
        break;
      // Connes function
      case 3: 
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = pow(1-(x*x)/(a*a), 2);
        }
        break;
      // cosine function
      case 4:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = cos((M_PI*x)/(2*a));
        }
        break;
      // Gaussian function
      case 5:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = exp((-1*x*x)/(2*_gaussianSigma*_gaussianSigma));
        }
        break;
      // Hamming function
      case 6:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = 0.54 + 0.46*cos(M_PI*x/a);
        }  
        break;
      // Hanning function
      case 7:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = pow(cos((M_PI*x)/(2*a)), 2);
        }
        break;
      // Welch function
      case 8:
        for (i = 0; i < len; i++) {
          x = i-a;
          _w[i] = 1 - (x*x)/(a*a);
        }
        break;
      // uniform
      default:
        for (i = 0; i < len; i++) {
          _w[i] = 1;
        }
        break;
    }
    // remember last used apodization function and length
    _WLen = len;
    _prevApodizeFxn = _apodizeFxn;
  }
}


inline double cabs2(double r, double i) {
  return (r*r + i*i);
}


KstObject::UpdateType KstPSD::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  int i_subset, i_samp;
  int copyLen;
  double mean;
  KstVectorPtr iv = _inputVectors[INVECTOR];

  if (update_counter <= 0) {
    assert(update_counter == 0);
    force = true;
  }

  bool xUpdated = KstObject::UPDATE == iv->update(update_counter);

  _adjustLengths();

  int v_len = iv->length();
  int n_subsets = v_len / _PSDLen + 1;
  if (v_len % _PSDLen == 0) {
    --n_subsets;
  }

  // Don't touch _last_n_new if !xUpdated since it will certainly be wrong.
  if (!xUpdated && !force) {
    return setLastUpdateResult(NO_CHANGE);
  }

  _last_n_new += iv->numNew();
  assert(_last_n_new >= 0);

  if ((_last_n_new < _PSDLen/16 &&
        n_subsets - _last_n_subsets < 1 &&
        iv->length() != iv->numNew()) && !force) {
    return setLastUpdateResult(NO_CHANGE);
  }

  double *psd = (*_sVector)->value();
  double *f = (*_fVector)->value();

  for (i_samp = 0; i_samp < _PSDLen; ++i_samp) {
    psd[i_samp] = 0;
    f[i_samp] = i_samp * 0.5 * _Freq / (_PSDLen - 1);
  }

  double nf = 0;
  bool done = false;

  for (i_subset = 0; !done; ++i_subset) {
    // copy each chunk into a[] and find mean
    if (i_subset*_PSDLen + _ALen < v_len) {
      copyLen = _ALen;
    } else {
      copyLen = v_len - i_subset*_PSDLen;
      done = true;
    }

    mean = 0;
    for (i_samp = 0; i_samp < copyLen; ++i_samp) {
      _a[i_samp] = iv->interpolate(i_samp + i_subset*_PSDLen, v_len);
      mean += _a[i_samp];
    }
    if (copyLen > 1) {
      mean /= double(copyLen);
    }

    if (apodize()) {
      GenW(copyLen);
    }
     
    // remove mean and apodize
    if (removeMean() && apodize()) {
      for (i_samp = 0; i_samp < copyLen; ++i_samp) {
        _a[i_samp]= (_a[i_samp] - mean)*_w[i_samp];
      }
    } else if (removeMean()) {
      for (i_samp = 0; i_samp < copyLen; ++i_samp) {
        _a[i_samp] -= mean;
      }
    } else if (apodize()) {
      for (i_samp = 0; i_samp < copyLen; ++i_samp) {
        _a[i_samp] *= _w[i_samp];
      }
    }

    nf += copyLen;

    if (i_samp < _ALen) {
      memset(&_a[i_samp], 0, sizeof(double)*(_ALen - i_samp));
    }

    // fft a
    rdft(_ALen, 1, _a);
    psd[0] += _a[0]*_a[0];
    psd[_PSDLen-1] += _a[1]*_a[1];
    for (i_samp = 1; i_samp < _PSDLen - 1; ++i_samp) {
      psd[i_samp] += cabs2(_a[i_samp*2], _a[i_samp*2 + 1]);
    }
  }

  _last_f0 = 0;
  _last_n_subsets = n_subsets;
  _last_n_new = 0;

  nf = 1.0/(double(_Freq)*double(nf/2.0));
  for (i_samp = 0; i_samp < _PSDLen; ++i_samp) {
    psd[i_samp] = sqrt(psd[i_samp]*nf);
  }

  if (_Freq <= 0.0) {
    _Freq = 1.0;
  }

  (*_sVector)->setLabel(i18n("PSD [%1/%2^{1/2}]").arg(_vUnits).arg(_rUnits));
  (*_fVector)->setLabel(i18n("Frequency [%1]").arg(_rUnits));

  (*_sVector)->setDirty();
  (*_sVector)->update(update_counter);
  (*_fVector)->setDirty();
  (*_fVector)->update(update_counter);
  return setLastUpdateResult(UPDATE);
}


void KstPSD::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<psdobject>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<vectag>" << QStyleSheet::escape(_inputVectors[INVECTOR]->tagName()) << "</vectag>" << endl;
  ts << l2 << "<sampRate>"  << _Freq << "</sampRate>" << endl;
  ts << l2 << "<average>" << _Average << "</average>" << endl;
  ts << l2 << "<fftLen>" << _Len << "</fftLen>" << endl;
  ts << l2 << "<removeMean>" << _RemoveMean << "</removeMean>" << endl;
  ts << l2 << "<apodize>" << _Apodize << "</apodize>" << endl;
  ts << l2 << "<apodizefxn>" << _apodizeFxn << "</apodizefxn>" << endl;
  ts << l2 << "<gaussiansigma>" << _gaussianSigma << "</gaussiansigma>" << endl;
  ts << l2 << "<VUnits>" << _vUnits << "</VUnits>" << endl;
  ts << l2 << "<RUnits>" << _rUnits << "</RUnits>" << endl;
  ts << indent << "</psdobject>" << endl;
}


bool KstPSD::apodize() const {
  return _Apodize;
}


void KstPSD::setApodize(bool in_apodize)  {
  setDirty();
  _Apodize = in_apodize;
}


bool KstPSD::removeMean() const {
  return _RemoveMean;
}


void KstPSD::setRemoveMean(bool in_removeMean) {
  setDirty();
  _RemoveMean = in_removeMean;
}


bool KstPSD::average() const {
  return _Average;
}


void KstPSD::setAverage(bool in_average) {
  setDirty();
  _Average = in_average;
}


double KstPSD::freq() const {
  return _Freq;
}


void KstPSD::setFreq(double in_freq) {
  setDirty();
  if (in_freq > 0.0) {
    _Freq = in_freq;
  } else {
    _Freq = KST::objectDefaults.psdFreq();
  }
}


int KstPSD::len() const {
  return _Len;
}


void KstPSD::_adjustLengths() {
  int psdlen, len;

  if (_Average) {
    psdlen = int(pow(2,_Len-1));
  } else {
    len = int(ceil(log(double(_inputVectors[INVECTOR]->length())) / log(2.0)));
    psdlen = int(pow(2,len-1));
  }

  if (_PSDLen != psdlen) {
    _PSDLen = psdlen;
    (*_sVector)->resize(_PSDLen);
    (*_fVector)->resize(_PSDLen);

    _ALen = _PSDLen*2;
    delete[] _a;
    _a = new double[_ALen];
    delete[] _w;
    _w = new double[_ALen];

    _last_f0 = 0;
    _last_n_subsets = 0;
  }
}


void KstPSD::setLen(int in_len) {
  if (in_len >= 4 && in_len <= 27) {
    _Len = in_len;
  } else {
    _Len = 10;
  }
  setDirty();
}


QString KstPSD::vTag() const {
  return _inputVectors[INVECTOR]->tagName();
}


void KstPSD::setVector(KstVectorPtr new_v) {
  KstVectorPtr v = _inputVectors[INVECTOR];
  if (v) {
    if (v == new_v) {
      return;
    }
    v->writeUnlock();
  }

  _inputVectors.erase(INVECTOR);
  new_v->writeLock();
  _inputVectors[INVECTOR] = new_v;
  setDirty();
}


bool KstPSD::slaveVectorsUsed() const {
  return true;
}


QString KstPSD::propertyString() const {
  return i18n("PSD: %1").arg(vTag());
}


void KstPSD::_showDialog() {
  KstDialogs::self()->showPSDDialog(tagName());
}


const QString& KstPSD::vUnits() const {
  return _vUnits;
}


void KstPSD::setVUnits(const QString& units) {
  _vUnits = units;
}


const QString& KstPSD::rUnits() const {
  return _rUnits;
}


void KstPSD::setRUnits(const QString& units) {
  _rUnits = units;
}

int KstPSD::apodizeFxn() const {
  return _apodizeFxn;
}

void KstPSD::setApodizeFxn(int in_apodizeFxn) {
  setDirty();
  _apodizeFxn = in_apodizeFxn;
}

double KstPSD::gaussianSigma() const {
  return _gaussianSigma;
}

void KstPSD::setGaussianSigma(double in_gaussianSigma) {
  setDirty();
  _gaussianSigma = in_gaussianSigma;
}

 
KstDataObjectPtr KstPSD::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  QString name(tagName() + '\'');
  while (KstData::self()->dataTagNameNotUnique(name, false)) {
    name += '\'';
  }
  KstPSDPtr psd = new KstPSD(name, _inputVectors[INVECTOR], _Freq,
                             _Average, _Len, _Apodize, _RemoveMean, _vUnits, _rUnits, 
                             _apodizeFxn, _gaussianSigma);
  duplicatedMap.insert(this, KstDataObjectPtr(psd));    
  return KstDataObjectPtr(psd);
}

// vim: ts=2 sw=2 et
