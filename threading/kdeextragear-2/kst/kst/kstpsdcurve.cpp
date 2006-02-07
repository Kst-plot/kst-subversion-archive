/***************************************************************************
                          kstpsdcurve.cpp: Power Spectra for KST
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

#include <math.h>
#include <stdlib.h>
#include <iostream>

#include <klocale.h>

#include "kstdatacollection.h"
#include "kstdoc.h"
#include "kstpsdcurve.h"
#include "kstpsddialog_i.h"
#include "kstsettings.h"

extern "C" void rdft(int n, int isgn, double *a);

static const QString INVECTOR = "I";
static const QString SVECTOR = "S";
static const QString FVECTOR = "F";

KstPSDCurve::KstPSDCurve(const QString &in_tag, KstVectorPtr in_V,
                         double in_freq, int in_len,
                         const QString &in_VUnits, const QString &in_RUnits,
                         const QColor &in_color)
: KstBaseCurve() {
  setHasPoints(false);
  setHasLines(true);

  commonConstructor(in_tag, in_V, in_freq, in_len,
                    in_VUnits, in_RUnits, in_color);
}

KstPSDCurve::KstPSDCurve(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag;
  KstVectorPtr in_V;
  QColor in_color("blue");
  double in_freq=60.0;
  int in_len = 12;
  QString in_VUnits;
  QString in_RUnits;

  setHasPoints(false);
  setHasLines(true);

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "vectag") {
        KST::vectorList.lock().readLock();
        KstVectorList::Iterator it = KST::vectorList.findTag(e.text());
        if (it != KST::vectorList.end()) {
          in_V = *it;
        }
        KST::vectorList.lock().readUnlock();
      } else if (e.tagName() == "color") {
        in_color.setNamedColor(e.text());
      } else if (e.tagName() == "hasLines") {
	if (e.text() == "0") HasLines = false;
	else HasLines = true;
      } else if (e.tagName() == "hasPoints") {
	if (e.text() == "0") HasPoints = false;
	else HasPoints = true;
      } else if (e.tagName() == "pointType") {
	Point.setType(e.text().toInt());
      } else if (e.tagName() == "sampRate") {
        in_freq = e.text().toDouble();
      } else if (e.tagName() == "fftLen") {
        in_len = e.text().toInt();
      } else if (e.tagName() == "VUnits") {
        in_VUnits = e.text();
      } else if (e.tagName() == "RUnits") {
        in_RUnits = e.text();
      }
    }
    n = n.nextSibling();
  }

  commonConstructor(in_tag, in_V, in_freq, in_len,
                    in_VUnits, in_RUnits, in_color);
}

void KstPSDCurve::commonConstructor(const QString &in_tag, KstVectorPtr in_V,
                                    double in_freq, int in_len,
                                    const QString &in_VUnits,
                                    const QString &in_RUnits,
                                    const QColor &in_color) {

  _typeString = i18n("Power Spectrum");
  NumUsed = 0;
  _inputVectors[INVECTOR] = in_V;
  Color = in_color;
  _tag = in_tag;
  Freq = in_freq;
  Len = in_len;
  VUnits = in_VUnits;
  RUnits = in_RUnits;

  PSDLen = int(pow(2, Len-1));

  ALen = PSDLen*2;
  a = new double[ALen];
  w = new double[ALen];
  GenW();

  last_f0 = 0;
  last_n_subsets = 0;
  last_n_new = 0;
  setRemoveMean(true);
  setAppodize(true);

  KstVectorPtr iv = new KstVector(_tag+"-freq", 2);
  _fVector = _outputVectors.insert(FVECTOR, iv);

  iv = new KstVector(_tag+"-sv", PSDLen);
  _sVector = _outputVectors.insert(SVECTOR, iv);

  update();
}

KstPSDCurve::~KstPSDCurve() {
  _sVector = _outputVectors.end();
  _fVector = _outputVectors.end();
  KST::vectorList.lock().writeLock();
  KST::vectorList.remove(_outputVectors[SVECTOR]);
  KST::vectorList.remove(_outputVectors[FVECTOR]);
  KST::vectorList.lock().writeUnlock();

  delete[] w;
  w = 0L;
  delete[] a;
  a = 0L;
}

void KstPSDCurve::GenW() {
  int i;
  double sW = 0;

  for (i = 0; i < ALen; i++) {
    //w[i] = 4;
    w[i] = (1 - cos(2*M_PI*double(i)/double(ALen)));
    sW += w[i] * w[i];
  }
  sW = sqrt(sW/double(ALen));
  for (i = 0; i < ALen; i++) {
    w[i] /= sW;
  }
}

inline double cabs(double r, double i) {
  return sqrt(r*r + i*i);
}

KstObject::UpdateType KstPSDCurve::update(int update_counter) {
  int i_subset, i_samp;
  int n_subsets;
  int v_len;
  int copyLen;
  double mean;
  double y;
  bool force = false;
  KstVectorPtr iv = _inputVectors[INVECTOR];

  double *psd;

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  if (update_counter <= 0) {
    force = true;
  } else {
    iv->update(update_counter);
  }

  v_len = iv->sampleCount();

  n_subsets = v_len/PSDLen+1;

  last_n_new += iv->numNew();

  if ((last_n_new < PSDLen/16) && (n_subsets - last_n_subsets < 1) && !force) {
    return NO_CHANGE;
  }

  psd = (*_sVector)->value();

  for (i_samp = 0; i_samp < PSDLen; i_samp++) {
    psd[i_samp] = 0;
  }

  for (i_subset = 0; i_subset < n_subsets; i_subset++) {
    /* copy each chunk into a[] and find mean */
    if (i_subset*PSDLen + ALen <= v_len) {
      copyLen = ALen;
    } else {
      copyLen = v_len - i_subset*PSDLen;
    }
    mean = 0;
    for (i_samp = 0; i_samp < copyLen; i_samp++) {
      mean += (
        a[i_samp] =
        iv->interpolate(i_samp + i_subset*PSDLen, v_len)
        );
    }
    if (copyLen>1) mean/=(double)copyLen;

    /* Remove Mean and apodize */
    if (removeMean() && appodize()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        a[i_samp]= (a[i_samp]-mean)*w[i_samp];
      }
    } else if (removeMean()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        a[i_samp] -= mean;
      }
    } else if (appodize()) {
      for (i_samp=0; i_samp<copyLen; i_samp++) {
        a[i_samp] *= w[i_samp];
      }
    }
    for (;i_samp < ALen; i_samp++) a[i_samp] = 0.0;

    /* fft a */
    rdft(ALen, 1, a);
    /* sum each bin into psd[] */
    psd[0]+=a[0];
    psd[PSDLen-1] += a[1];
    for (i_samp=1; i_samp<PSDLen-1; i_samp++) {
      psd[i_samp]+= cabs(a[i_samp*2], a[i_samp*2+1]);
    }
  }

  last_f0 = 0;
  last_n_subsets = n_subsets;
  last_n_new = 0;

  norm_factor = 1.0/(sqrt(double(Freq)*double(PSDLen))*double(n_subsets));

  psd[0]*=norm_factor;

  MaxY = MinY = mean = psd[0];
  if (psd[0]>0)
    MinPosY = psd[0];
  else (MinPosY = 1.0e300);

  /* normalize psd */
  for (i_samp=1; i_samp<PSDLen; i_samp++) {
    y = (psd[i_samp]*=norm_factor);
    if (y>MaxY)
      MaxY=y;
    if (y<MinY)
      MinY=y;
    if ((y>0) && (y<MinPosY))
      MinPosY = y;
    mean +=y;
  }

  if (PSDLen > 0)
    MeanY = mean/PSDLen;
  else MeanY = 0; // should never ever happen...

  NS = PSDLen;

  if (Freq <= 0)
    Freq = 1.0;

  MaxX = Freq/2.0;
  MinX = 0;
  MinPosX = 1.0/double(NS) * MaxX;
  MeanX = MaxX/2.0;

  double *f = (*_fVector)->value();
  f[0] = 0;
  f[1] = Freq/2.0;

  (*_sVector)->update(update_counter);
  (*_fVector)->update(update_counter);

  return UPDATE;
}

void KstPSDCurve::getPoint(int i, double &x, double &y) {
  x = (*_fVector)->interpolate(i, NS);
//xxxdouble(i)/double(NS) * MaxX;
  y = (*_sVector)->interpolate(i, NS);
}

void KstPSDCurve::save(QTextStream &ts) {
  ts << " <psd>" << endl;
  ts << "  <tag>" << _tag << "</tag>" << endl;
  ts << "  <vectag>" << _inputVectors[INVECTOR]->tagName() << "</vectag>" << endl;

  ts << "  <color>" << Color.name() << "</color>" << endl;
  ts << "  <hasLines>" << HasLines << "</hasLines>" << endl;
  ts << "  <hasPoints>" << HasPoints << "</hasPoints>" << endl;
  ts << "  <pointType>" << Point.getType() << "</pointType>" << endl;

  ts << "  <sampRate>"  << Freq << "</sampRate>" << endl;
  ts << "  <fftLen>" << Len << "</fftLen>" << endl;
  ts << "  <VUnits>" << VUnits << "</VUnits>" << endl;
  ts << "  <RUnits>" << RUnits << "</RUnits>" << endl;
  ts << " </psd>" << endl;
}

double KstPSDCurve::getFreq() const {
  return Freq;
}

void KstPSDCurve::setFreq(double in_freq) {
  if (in_freq > 0.0) {
    Freq = in_freq;
  } else {
    Freq = KstSettings::globalSettings()->psdSampleRate;
  }
}

int KstPSDCurve::getLen() const {
  return Len;
}

void KstPSDCurve::setLen(int in_len) {
  if (in_len >= 4 && in_len <= 24) {
    Len = in_len;
  } else {
    Len = 10;
  }

  PSDLen = int(pow(2,Len-1));
  (*_sVector)->resize(PSDLen);

  ALen = PSDLen*2;
  delete[] a;
  a = new double[ALen];
  delete[] w;
  w = new double[ALen];
  GenW();

  last_f0 = 0;
  last_n_subsets = 0;

}

QString KstPSDCurve::getYLabel() const {
  return i18n("PSD of %1 (%2/%3^{1/2})").arg(_inputVectors[INVECTOR]->label()).arg(VUnits).arg(RUnits);
}

QString KstPSDCurve::getXLabel() const {
  return i18n("Frequency (%1)").arg(RUnits);
}

QString KstPSDCurve::getVTag() const {
  return _inputVectors[INVECTOR]->tagName();
}

void KstPSDCurve::setVector(KstVectorPtr new_v) {
  _inputVectors[INVECTOR] = new_v;
}

KstCurveType KstPSDCurve::type() const {
  return KST_PSDCURVE;
}

bool KstPSDCurve::slaveVectorsUsed() const {
  return true;
}

QString KstPSDCurve::propertyString() const {
  return i18n("PSD: %1").arg(getVTag());
}

void KstPSDCurve::_showDialog() {
  KstPsdDialogI::globalInstance()->show_I(tagName());
}

// vim: ts=2 sw=2 et
