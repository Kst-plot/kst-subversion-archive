/***************************************************************************
                   crosspowerspectrum.cpp
                             -------------------
    begin                : 09/08/06
    copyright            : (C) 2006 The University of Toronto
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

#include "crosspowerspectrum.h"
#include "crossspectrumdialog_i.h"

#include <qstylesheet.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include <kstdatacollection.h>

#define KSTPSDMAXLEN 27

extern "C" void rdft(int n, int isgn, double *a); //fast fourier transform...

static const QString& VECTOR_ONE = KGlobal::staticQString("Vector One");
static const QString& VECTOR_TWO = KGlobal::staticQString("Vector Two");

//in scalars
static const QString& FFT_LENGTH = KGlobal::staticQString("FFT Length = 2^");
static const QString& SAMPLE_RATE = KGlobal::staticQString("Sample Rate");

static const QString& REAL = KGlobal::staticQString("Cross Spectrum: Real");
static const QString& IMAGINARY = KGlobal::staticQString("Cross Spectrum: Imaginary");
static const QString& FREQUENCY = KGlobal::staticQString("Frequency");

KST_KEY_DATAOBJECT_PLUGIN( crossspectrum )

K_EXPORT_COMPONENT_FACTORY( kstobject_crossspectrum,
    KGenericFactory<CrossPowerSpectrum>( "kstobject_crossspectrum" ) )

CrossPowerSpectrum::CrossPowerSpectrum( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstDataObject() {
  _typeString = i18n("Plugin");
  _type = "Plugin";
}


CrossPowerSpectrum::~CrossPowerSpectrum() {
}


QString CrossPowerSpectrum::v1Tag() const {
  KstVectorPtr v = v1();
  if (v) {
    return v->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::v2Tag() const {
  KstVectorPtr v = v2();
  if (v) {
    return v->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::fftTag() const {
  KstScalarPtr s = fft();
  if (s) {
    return s->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::sampleTag() const {
  KstScalarPtr s = sample();
  if (s) {
    return s->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::realTag() const {
  KstVectorPtr v = real();
  if (v) {
    return v->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::imaginaryTag() const {
  KstVectorPtr v = imaginary();
  if (v) {
    return v->tagName();
  }
  return QString::null;
}


QString CrossPowerSpectrum::frequencyTag() const {
  KstVectorPtr v = frequency();
  if (v) {
    return v->tagName();
  }
  return QString::null;
}


KstVectorPtr CrossPowerSpectrum::v1() const {
  return *_inputVectors.find(VECTOR_ONE);
}


KstVectorPtr CrossPowerSpectrum::v2() const {
  return *_inputVectors.find(VECTOR_TWO);
}


KstScalarPtr CrossPowerSpectrum::fft() const {
  return *_inputScalars.find(FFT_LENGTH);
}


KstScalarPtr CrossPowerSpectrum::sample() const {
  return *_inputScalars.find(SAMPLE_RATE);
}


KstVectorPtr CrossPowerSpectrum::real() const {
  return *_outputVectors.find(REAL);
}


KstVectorPtr CrossPowerSpectrum::imaginary() const {
  return *_outputVectors.find(IMAGINARY);
}


KstVectorPtr CrossPowerSpectrum::frequency() const {
  return *_outputVectors.find(FREQUENCY);
}


void CrossPowerSpectrum::setV1(KstVectorPtr new_v1) {
  if (new_v1) {
    _inputVectors[VECTOR_ONE] = new_v1;
  } else {
    _inputVectors.remove(VECTOR_ONE);
  }
  setDirty();
}


void CrossPowerSpectrum::setV2(KstVectorPtr new_v2) {
  if (new_v2) {
    _inputVectors[VECTOR_TWO] = new_v2;
  } else {
    _inputVectors.remove(VECTOR_TWO);
  }
  setDirty();
}


void CrossPowerSpectrum::setFFT(KstScalarPtr new_fft) {
  if (new_fft) {
    _inputScalars[FFT_LENGTH] = new_fft;
  } else {
    _inputScalars.remove(FFT_LENGTH);
  }
  setDirty();
}


void CrossPowerSpectrum::setSample(KstScalarPtr new_sample) {
  if (new_sample) {
    _inputScalars[SAMPLE_RATE] = new_sample;
  } else {
    _inputScalars.remove(SAMPLE_RATE);
  }
  setDirty();
}


void CrossPowerSpectrum::setReal(const QString &name) {
  QString tname;
  if (name.isEmpty()) {
    tname = i18n("the real part of a complex number", "real");
  } else {
    tname = name;
  }
  KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
  KstVectorPtr v = new KstVector(KstObjectTag(tname, tag()), 0, this, false);
  _outputVectors.insert(REAL, v);
}


void CrossPowerSpectrum::setImaginary(const QString &name) {
  QString tname;
  if (name.isEmpty()) {
    tname = i18n("the imaginary part of a complex number", "imaginary");
  } else {
    tname = name;
  }
  KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
  KstVectorPtr v = new KstVector(KstObjectTag(tname, tag()), 0, this, false);
  _outputVectors.insert(IMAGINARY, v);
}


void CrossPowerSpectrum::setFrequency(const QString &name) {
  QString tname;
  if (name.isEmpty()) {
    tname = i18n("frequency");
  } else {
    tname = name;
  }
  KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
  KstVectorPtr v = new KstVector(KstObjectTag(tname, tag()), 0, this, false);
  _outputVectors.insert(FREQUENCY, v);
}


KstObject::UpdateType CrossPowerSpectrum::update(int updateCounter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  if (!v1() || !v2() || !fft() || !sample()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  if (!real() || !imaginary() || !frequency()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  if (recursed()) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool depUpdated = force;

  writeLockInputsAndOutputs();

  depUpdated = ( ( UPDATE == v1()->update(updateCounter) ) || depUpdated );
  depUpdated = ( ( UPDATE == v2()->update(updateCounter) ) || depUpdated );

  depUpdated = ( ( UPDATE == fft()->update(updateCounter) ) || depUpdated );
  depUpdated = ( ( UPDATE == sample()->update(updateCounter) ) || depUpdated );

  if (depUpdated) {
    crossspectrum();

    vectorRealloced(real(), real()->value(), real()->length());
    real()->setNewAndShift(real()->length(), real()->numShift());

    vectorRealloced(imaginary(), imaginary()->value(), imaginary()->length());
    imaginary()->setNewAndShift(imaginary()->length(), imaginary()->numShift());

    vectorRealloced(frequency(), frequency()->value(), frequency()->length());
    frequency()->setNewAndShift(frequency()->length(), frequency()->numShift());
  }

  unlockInputsAndOutputs();

  return setLastUpdateResult(depUpdated ? UPDATE : NO_CHANGE);
}


void CrossPowerSpectrum::crossspectrum() {
  KstVectorPtr v1 = *_inputVectors.find(VECTOR_ONE);
  KstVectorPtr v2 = *_inputVectors.find(VECTOR_TWO);

  KstScalarPtr fft = *_inputScalars.find(FFT_LENGTH);
  KstScalarPtr sample = *_inputScalars.find(SAMPLE_RATE);

  KstVectorPtr real = *_outputVectors.find(REAL);
  KstVectorPtr imaginary = *_outputVectors.find(IMAGINARY);
  KstVectorPtr frequency = *_outputVectors.find(FREQUENCY);

  double *a, *b;
  double sampleRate = sample->value();
  double df;
  int i, xpsLen;
  int dv0, dv1, vLen;
  int iSubset, nSubsets;
  int iSamp, copyLen;

  if (sampleRate == 0.0) {
    sampleRate = 1.0;
  }

  // parse fft length
  xpsLen = int(fft->value() - 0.99);
  if (xpsLen > KSTPSDMAXLEN ) {
    xpsLen = KSTPSDMAXLEN;
  }
  if (xpsLen < 2) {
    xpsLen = 2;
  }
  xpsLen = int(pow( 2.0, xpsLen ));

  // input vector lengths
  vLen = (v1->length() < v2->length()) ? v1->length() : v2->length();
  dv0 = vLen/v1->length();
  dv1 = vLen/v2->length();

  while (xpsLen > vLen) {
    xpsLen /= 2;
  }

  // allocate the lengths
  if (real->length() != xpsLen) {
    real->resize( xpsLen, false );
    imaginary->resize( xpsLen, false );
    frequency->resize( xpsLen, false );
  }

  // fill the frequency and zero the xps
  df = sampleRate/( 2.0*double( xpsLen-1 ) );
  for (i=0; i<xpsLen; i++) {
    frequency->value()[i] = double( i ) * df;
    real->value()[i] = 0.0;
    imaginary->value()[i] = 0.0;
  }

  // allocate input arrays
  int ALen = xpsLen * 2;
  a = new double[ALen];
  b = new double[ALen];

  // do the fft's
  nSubsets = vLen/xpsLen + 1;

  for (iSubset=0; iSubset<nSubsets; iSubset++ ) {
    double meanA = 0.0;
    double meanB = 0.0;

    // copy each chunk into a[] and find mean
    if (iSubset*xpsLen + ALen <= vLen) {
      copyLen = ALen;
    } else {
      copyLen = vLen - iSubset*xpsLen;
    }

    for (iSamp = 0; iSamp < copyLen; iSamp++) {
      i = ( iSamp + iSubset*xpsLen ) / dv0;
      meanA += (a[iSamp] = v1->value()[i]);
      i = ( iSamp + iSubset*xpsLen ) / dv1;
      meanB += (b[iSamp] = v2->value()[i]);
    }

    if (copyLen > 1) {
      meanA /= (double)copyLen;
      meanB /= (double)copyLen;
    }

    // remove mean and apodize
    if (!isnan(meanA) && !isnan(meanB)) {
      for (iSamp=0; iSamp<copyLen; iSamp++) {
        a[iSamp] -= meanA;
        b[iSamp] -= meanB;
      }
    }

    for (;iSamp < ALen; iSamp++) {
      a[iSamp] = 0.0;
      b[iSamp] = 0.0;
    }

    // fft
    rdft(ALen, 1, a);
    rdft(ALen, 1, b);

    // sum each bin into psd[]
    real->value()[0] += a[0]*b[0];
    real->value()[xpsLen-1] += a[1]*b[1];
    for (iSamp=1; iSamp<xpsLen-1; iSamp++) {
      real->value()[iSamp] += a[iSamp*2] * b[iSamp*2] + a[iSamp*2+1] * b[iSamp*2+1];
      imaginary->value()[iSamp] += -a[iSamp*2] * b[iSamp*2+1] + a[iSamp*2+1] * b[iSamp*2];
    } // (a+ci)(b+di)* = ab+cd +i(-ad + cb)
  }

  // renormalize
  double normFactor = 1.0/((sampleRate*double(xpsLen))*double(nSubsets));
  for (i=0; i<xpsLen; i++) {
    real->value()[i] *= normFactor;
    imaginary->value()[i] *= normFactor;
  }

  delete[] b;
  delete[] a;
}


QString CrossPowerSpectrum::propertyString() const {
  return "crosspowerspectrum";
}


KstDataObjectPtr CrossPowerSpectrum::makeDuplicate(KstDataObjectDataObjectMap&) {
  return 0;
}


void CrossPowerSpectrum::showNewDialog() {
  CrossSpectrumDialogI *dialog = new CrossSpectrumDialogI;
  dialog->show();
}


void CrossPowerSpectrum::showEditDialog() {
  CrossSpectrumDialogI *dialog = new CrossSpectrumDialogI;
  dialog->showEdit(tagName());
}


void CrossPowerSpectrum::load(const QDomElement &e) {
  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(KstObjectTag::fromString(e.text()));
      } else if (e.tagName() == "ivector") {
        _inputVectorLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "iscalar") {
        _inputScalarLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "istring") {
        _inputStringLoadQueue.append(qMakePair(e.attribute("name"), e.text()));
      } else if (e.tagName() == "ovector") {
        KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
        KstVectorPtr v;
        if (e.attribute("scalarList", "0").toInt()) {
          v = new KstVector(KstObjectTag(e.text(), tag()), 0, this, true);
        } else {
          v = new KstVector(KstObjectTag(e.text(), tag()), 0, this, false);
        }
        _outputVectors.insert(e.attribute("name"), v);
      } else if (e.tagName() == "oscalar") {
        KstWriteLocker blockScalarUpdates(&KST::scalarList.lock());
        KstScalarPtr sp = new KstScalar(KstObjectTag(e.text(), tag()), this);
        _outputScalars.insert(e.attribute("name"), sp);
      } else if (e.tagName() == "ostring") {
        KstWriteLocker blockStringUpdates(&KST::stringList.lock());
        KstStringPtr sp = new KstString(KstObjectTag(e.text(), tag()), this);
        _outputStrings.insert(e.attribute("name"), sp);
      }
    }
    n = n.nextSibling();
  }
}


void CrossPowerSpectrum::save(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<plugin name=\"Cross Power Spectrum\">" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstVectorMap::Iterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    ts << l2 << "<ivector name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ivector>" << endl;
  }
  for (KstScalarMap::Iterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    ts << l2 << "<iscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</iscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _inputStrings.begin(); i != _inputStrings.end(); ++i) {
    ts << l2 << "<istring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</istring>" << endl;
  }
  for (KstVectorMap::Iterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    ts << l2 << "<ovector name=\"" << QStyleSheet::escape(i.key());
    if (i.data()->isScalarList()) {
      ts << "\" scalarList=\"1";
    }
    ts << "\">" << QStyleSheet::escape(i.data()->tagName())
        << "</ovector>" << endl;
  }
  for (KstScalarMap::Iterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    ts << l2 << "<oscalar name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</oscalar>" << endl;
  }
  for (KstStringMap::Iterator i = _outputStrings.begin(); i != _outputStrings.end(); ++i) {
    ts << l2 << "<ostring name=\"" << QStyleSheet::escape(i.key()) << "\">"
        << QStyleSheet::escape(i.data()->tagName())
        << "</ostring>" << endl;
  }
  ts << indent << "</plugin>" << endl;
}

#include "crosspowerspectrum.moc"

