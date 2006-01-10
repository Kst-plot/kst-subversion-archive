/***************************************************************************
                          kstcsd.cpp: Cumulative Spectral Decay for KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by University of British Columbia
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

/** A class for handling cumulative spectral decays for kst
 */

#include <assert.h>
#include <math.h>

#include <qstylesheet.h>

#include <kglobal.h>
#include <klocale.h>

#include "dialoglauncher.h"
#include "kstcsd.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstpsdgenerator.h"
#include "kstvectordefaults.h"

extern "C" void rdft(int n, int isgn, double *a);

static const QString& INVECTOR = KGlobal::staticQString("I");
static const QString& OUTMATRIX = KGlobal::staticQString("M");

#define KSTCSDMAXLEN 27
KstCSD::KstCSD(const QString &in_tag, KstVectorPtr in_V,
               double in_freq, bool in_average, bool in_removeMean,
               bool in_apodize, int in_apodizeFxn, int in_windowSize, int in_length,
               double in_gaussianSigma, const QString &in_vectorUnits, const QString &in_rateUnits)
: KstDataObject() {
  commonConstructor(in_tag, in_V, in_freq, in_average, in_removeMean,
                    in_apodize, in_apodizeFxn, in_windowSize, in_length, in_gaussianSigma, 
                    in_vectorUnits, in_rateUnits, in_V->tagName());
  setDirty();
}


KstCSD::KstCSD(const QDomElement &e)
: KstDataObject(e) {
  
    QString in_tag;
    QString vecName;
    QString in_vectorUnits, in_rateUnits;
    KstVectorPtr in_V;
    double in_freq = 60.0;
    bool in_average = true;
    int in_length = 8;
    bool in_removeMean = true;
    bool in_apodize = true;
    int in_apodizeFxn = 0;
    int in_windowSize = 5000;
    double in_gaussianSigma = 3.0;
    
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
          in_average = (e.text() != "0");
        } else if (e.tagName() == "fftLen") {
          in_length = e.text().toInt();
        } else if (e.tagName() == "apodize") {
          in_apodize = (e.text() != "0");
        } else if (e.tagName() == "apodizefxn") {
          in_apodizeFxn = e.text().toInt();
        } else if (e.tagName() == "gaussiansigma") {
          in_gaussianSigma = e.text().toDouble();
        } else if (e.tagName() == "removeMean") {
          in_removeMean = (e.text() != "0");
        } else if (e.tagName() == "windowsize") {
          in_windowSize = e.text().toInt();
        } else if (e.tagName() == "vectorunits") {
          in_vectorUnits = e.text();
        } else if (e.tagName() == "rateunits") {
          in_rateUnits = e.text();
        }
      }
      n = n.nextSibling();
    }
    
    _inputVectorLoadQueue.append(qMakePair(INVECTOR, vecName));
    
    commonConstructor(in_tag, in_V, in_freq, in_average, in_removeMean,
                      in_apodize, in_apodizeFxn, in_windowSize, in_length, in_gaussianSigma,
                      in_vectorUnits, in_rateUnits, vecName);
}


void KstCSD::commonConstructor(const QString& in_tag, KstVectorPtr in_V,
                               double in_freq, bool in_average, bool in_removeMean,
                               bool in_apodize, int in_apodizeFxn, int in_windowSize, int in_length,
                               double in_gaussianSigma, const QString& in_vectorUnits, const QString& in_rateUnits,
                               const QString& vecName) {

  _typeString = i18n("Cumulative Spectral Decay");
  _inputVectors[INVECTOR] = in_V;
  setTagName(in_tag);
  _frequency = in_freq;
  _average = in_average;
  _apodize = in_apodize;
  _windowSize = in_windowSize;
  _apodizeFxn = in_apodizeFxn;
  _gaussianSigma = in_gaussianSigma;
  _removeMean = in_removeMean;
  _length = in_length;
  _vectorUnits = in_vectorUnits;
  _rateUnits = in_rateUnits;
  
  if (!_average) {
    _length = int(ceil(log(_windowSize)/log(2.0)));
  }

  if (_length < 2) {
    _length = 2;
  }

  if (_length > KSTCSDMAXLEN) {
    _length = KSTCSDMAXLEN;
  }

  if (_frequency <= 0.0) {
    _frequency = 1.0;
  }
  
  int psdLength = int(pow(2.0, (double)(_length-1)));
  
  KstMatrixPtr outMatrix = new KstMatrix(in_tag+"-csd", psdLength, 1);
  outMatrix->setLabel(i18n("Power [%1/%2^{1/2}]").arg(_vectorUnits).arg(_rateUnits));
  outMatrix->setXLabel(i18n("%1 [%2]").arg(vecName).arg(_vectorUnits));
  outMatrix->setYLabel(i18n("Frequency [%1]").arg(_rateUnits));
  if (outMatrix->sampleCount() != psdLength) {
    _length = 1;
    KstDebug::self()->log(i18n("Attempted to create a CSD that used all memory."), KstDebug::Error);
  }
  
  outMatrix->setProvider(this);
  _outMatrix = _outputMatrices.insert(OUTMATRIX, outMatrix);
  KST::addMatrixToList(outMatrix);
  (*_outMatrix)->setDirty();
}


KstCSD::~KstCSD() {
  _outMatrix = _outputMatrices.end();
  KST::matrixList.lock().writeLock();
  KST::matrixList.remove(_outputMatrices[OUTMATRIX]);
  KST::matrixList.lock().writeUnlock();
}

KstObject::UpdateType KstCSD::update(int update_counter) {
  
  KstVectorPtr inVector = _inputVectors[INVECTOR];

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  if (update_counter <= 0) {
    assert(update_counter == 0);
    force = true;
  }

  bool xUpdated = KstObject::UPDATE == inVector->update(update_counter);
  // if vector was not changed, don't update the CSD
  if ((!xUpdated) && !force ) {
    return setLastUpdateResult(NO_CHANGE);
  }
  
  // create a psd generator
  KstPSDGenerator psdGenerator(0L, _frequency, _average, _length,
                               _apodize, _removeMean, _apodizeFxn, _gaussianSigma);
  int xSize = 0;
  for (int i=0; i < inVector->length(); i+= _windowSize + 1) {
    int vectorSize = _windowSize;
    // determine size of actual input data
    if (i + _windowSize >= inVector->length()) {
      if (i == 0) {
        // if this is the one and only window, get a PSD
        vectorSize = i + _windowSize - inVector->length();
      } else {
        // don't PSD the last window if it is chopped off
        break;  
      }
    }
    
    // copy input vector elements into subvector
    QValueVector<double> psdInputVector(_windowSize, 0);
    double* inVectorArray = inVector->value();
    for (int j=0; j < vectorSize; j++) {
      psdInputVector[j] = inVectorArray[i+j];
    }

    // set the vector and calculate PSD
    psdGenerator.setInputVector(&psdInputVector);
    psdGenerator.updateNow();
    
    // resize output matrix
    (*_outMatrix)->resize(xSize+1, psdGenerator.powerVector()->size());
    
    // copy elements to output matrix
    for (uint j=0; j < psdGenerator.powerVector()->size(); j++) {
      (*_outMatrix)->setValueRaw(xSize, j, psdGenerator.powerVector()->at(j));
    }
    xSize++;
  }
 
  (*_outMatrix)->change((*_outMatrix)->tagName(), xSize, psdGenerator.frequencyVector()->size(), 0, 0, _windowSize, psdGenerator.frequencyVectorStep());
  
  (*_outMatrix)->update(update_counter);
      
  return setLastUpdateResult(UPDATE);
}

void KstCSD::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<csdobject>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<vectag>" << QStyleSheet::escape(_inputVectors[INVECTOR]->tagName()) << "</vectag>" << endl;
  ts << l2 << "<sampRate>"  << _frequency << "</sampRate>" << endl;
  ts << l2 << "<average>" << _average << "</average>" << endl;
  ts << l2 << "<fftLen>" << _length << "</fftLen>" << endl;
  ts << l2 << "<removeMean>" << _removeMean << "</removeMean>" << endl;
  ts << l2 << "<apodize>" << _apodize << "</apodize>" << endl;
  ts << l2 << "<apodizefxn>" << _apodizeFxn << "</apodizefxn>" << endl;
  ts << l2 << "<windowsize>" << _windowSize << "</windowsize>" << endl;
  ts << l2 << "<vectorunits>" << _vectorUnits << "</vectorunits>" << endl;
  ts << l2 << "<rateunits>" << _rateUnits << "</rateunits>" << endl;
  ts << indent << "</csdobject>" << endl;
}


QString KstCSD::vTag() const {
  return _inputVectors[INVECTOR]->tagName();
}


void KstCSD::setVector(KstVectorPtr new_v) {
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


bool KstCSD::slaveVectorsUsed() const {
  return true;
}


QString KstCSD::propertyString() const {
  return i18n("CSD: %1").arg(_inputVectors[INVECTOR]->tagName());
}


void KstCSD::_showDialog() {
  KstDialogs::showCSDDialog(tagName());
}

bool KstCSD::apodize() const {
  return _apodize;
}


void KstCSD::setApodize(bool in_apodize)  {
  setDirty();
  _apodize = in_apodize;
}


bool KstCSD::removeMean() const {
  return _removeMean;
}


void KstCSD::setRemoveMean(bool in_removeMean) {
  setDirty();
  _removeMean = in_removeMean;
}


bool KstCSD::average() const {
  return _average;
}


void KstCSD::setAverage(bool in_average) {
  setDirty();
  _average = in_average;
}


double KstCSD::freq() const {
  return _frequency;
}


void KstCSD::setFreq(double in_freq) {
  setDirty();
  if (in_freq > 0.0) {
    _frequency = in_freq;
  } else {
    _frequency = KST::vectorDefaults.psdFreq();
  }
}

int KstCSD::apodizeFxn() const {
  return _apodizeFxn;
} 

void KstCSD::setApodizeFxn(int in_fxn) {
  setDirty();
  _apodizeFxn = in_fxn;
}

int KstCSD::length() const { 
  return _length;  
}

void KstCSD::setLength(int in_length) {
  _length = in_length;
}


int KstCSD::windowSize() const {
  return _windowSize;
}


void KstCSD::setWindowSize(int in_size) {
  setDirty();
  _windowSize = in_size;  
}

double KstCSD::gaussianSigma() const {
  return _gaussianSigma;
}

void KstCSD::setGaussianSigma(double in_sigma) {
  setDirty();
  _gaussianSigma = in_sigma;
}


KstMatrixPtr KstCSD::outputMatrix() const {
  return *_outMatrix;  
}


const QString& KstCSD::vectorUnits() const {
  return _vectorUnits;  
}


void KstCSD::setVectorUnits(const QString& units) {
  _vectorUnits = units;
}


const QString& KstCSD::rateUnits() const {
  return _rateUnits;
}


void KstCSD::setRateUnits(const QString& units) {
  _rateUnits = units;
}

 
KstDataObjectPtr KstCSD::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  QString name(tagName() + '\'');
  while (KST::dataTagNameNotUnique(name, false)) {
    name += '\'';
  }
  KstCSDPtr csd = new KstCSD(name, _inputVectors[INVECTOR], _frequency, _average, _removeMean,
                             _apodize, _apodizeFxn, _windowSize, _length, _gaussianSigma, _vectorUnits, _rateUnits);
  duplicatedMap.insert(this, KstDataObjectPtr(csd));
  return KstDataObjectPtr(csd);
}

// vim: ts=2 sw=2 et
