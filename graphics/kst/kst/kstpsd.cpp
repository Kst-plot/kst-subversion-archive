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

#include "dialoglauncher.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstpsd.h"
#include "kstvectordefaults.h"

extern "C" void rdft(int n, int isgn, double *a);

static const QString& INVECTOR = KGlobal::staticQString("I");
static const QString& SVECTOR = KGlobal::staticQString("S");
static const QString& FVECTOR = KGlobal::staticQString("F");

#define KSTPSDMAXLEN 27
KstPSD::KstPSD(const QString &in_tag, KstVectorPtr in_V,
                         double in_freq, bool in_average, int in_len,
                         bool in_apodize, bool in_removeMean,
                         const QString &in_VUnits, const QString &in_RUnits)
: KstDataObject() {
  commonConstructor(in_tag, in_V, in_freq, in_average, in_len,
                    in_apodize, in_removeMean,
                    in_VUnits, in_RUnits);
                    
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
                    in_VUnits, in_RUnits);
}


void KstPSD::commonConstructor(const QString& in_tag, KstVectorPtr in_V,
                               double in_freq, bool in_average, int in_len,
                               bool in_apodize, bool in_removeMean,
                               const QString& in_VUnits,
                               const QString& in_RUnits) {

  _typeString = i18n("Power Spectrum");
  _inputVectors[INVECTOR] = in_V;
  setTagName(in_tag);
  _Freq = in_freq;
  _Average = in_average;
  _Len = in_len;
  _Apodize = in_apodize;
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

  if (len != _WLen) {
    for (i = 0; i < len; i++) {
      _w[i] = (1 - cos(2*M_PI*double(i)/double(len)));
      sW += _w[i] * _w[i];
    }
    sW = sqrt(sW/double(len));
    for (i = 0; i < len; i++) {
      _w[i] /= sW;
    }
    _WLen = len;
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
  int n_subsets;
  int v_len;
  int copyLen;
  double *psd, *f;
  double mean;
  double nf;
  KstVectorPtr iv = _inputVectors[INVECTOR];
  bool done;

  if (update_counter <= 0) {
    assert(update_counter == 0);
    force = true;
  }

  bool xUpdated = KstObject::UPDATE == iv->update(update_counter);

  _adjustLengths();

  v_len = iv->length();
  n_subsets = v_len/_PSDLen+1;
  if (v_len%_PSDLen==0) n_subsets--;

  _last_n_new += iv->numNew();

  if ((!xUpdated || (_last_n_new < _PSDLen/16 &&
          n_subsets - _last_n_subsets < 1 &&
          iv->length() != iv->numNew())) && !force ) {
    return setLastUpdateResult(NO_CHANGE);
  }

  psd = (*_sVector)->value();
  f = (*_fVector)->value();

  for (i_samp = 0; i_samp < _PSDLen; i_samp++) {
    psd[i_samp] = 0;
    f[i_samp] = i_samp*0.5*_Freq/( _PSDLen-1 );
  }

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
        iv->interpolate(i_samp + i_subset*_PSDLen, v_len)
        );
    }
    if (copyLen > 1) {
      mean /= (double)copyLen;
    }

    if (apodize()) {
      GenW(copyLen);
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
    psd[0] += _a[0]*_a[0];
    psd[_PSDLen-1] += _a[1]*_a[1];
    for (i_samp=1; i_samp<_PSDLen-1; i_samp++) {
      psd[i_samp]+= cabs2(_a[i_samp*2], _a[i_samp*2+1]);
    }
  }

  _last_f0 = 0;
  _last_n_subsets = n_subsets;
  _last_n_new = 0;

  nf = 1.0/(double(_Freq)*double(nf/2.0));
  for ( i_samp = 0; i_samp<_PSDLen; i_samp++ ) {
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
    _Freq = KST::vectorDefaults.psdFreq();
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
  KstDialogs::showPSDDialog(tagName());
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



// vim: ts=2 sw=2 et
