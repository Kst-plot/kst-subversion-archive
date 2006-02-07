/***************************************************************************
                          ksthistogram.cpp: Power Spectra for KST
                             -------------------
    begin                : July 2002
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

/** A class for handling histograms for kst

 *@author C. Barth Netterfield
 */

#include "ksthistogram.h"
#include <stdlib.h>
#include <math.h>

#include "kstdoc.h"
#include "kstdatacollection.h"
#include "ksthsdialog_i.h"

#include <klocale.h>

KstHistogram::KstHistogram(const QString &in_tag, KstVectorPtr in_V,
                           double xmin_in, double xmax_in,
                           int in_n_bins,
                           KstHsNormType in_norm_mode,
                           const QColor &in_color)
: KstBaseCurve() {
  commonConstructor(in_tag, in_V, xmin_in, xmax_in, in_n_bins, in_norm_mode,
                    in_color);
}

KstHistogram::KstHistogram(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag;
  KstVectorPtr in_V;
  QColor in_color("magenta");
  double xmax_in=1, xmin_in=-1;
  int in_n_bins = 10;
  KstHsNormType in_norm_mode;

  setHasPoints(false);
  setHasLines(true);

  in_norm_mode = KST_HS_NUMBER;
  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "vectag") {
        KstVectorList::Iterator it = KST::vectorList.findTag(e.text());
        if (it != KST::vectorList.end())
          in_V = *it;
      } else if (e.tagName() == "color") {
        in_color.setNamedColor(e.text());
      } else if (e.tagName() == "hasLines") {
	HasLines = (e.text() != "0");
      } else if (e.tagName() == "hasPoints") {
	HasPoints = (e.text() != "0");
      } else if (e.tagName() == "pointType") {
	Point.setType(e.text().toInt());
      } else if (e.tagName() == "NormMode") {
        if (e.text()=="NUMBER") in_norm_mode = KST_HS_NUMBER;
        else if (e.text()=="PERCENT") in_norm_mode = KST_HS_PERCENT;
        else if (e.text()=="FRACTION") in_norm_mode = KST_HS_FRACTION;
        else if (e.text()=="MAX_ONE") in_norm_mode = KST_HS_MAX_ONE;
      } else if (e.tagName() == "minX") {
        xmin_in = e.text().toDouble();
      } else if (e.tagName() == "maxX") {
        xmax_in = e.text().toDouble();
      } else if (e.tagName() == "numBins") {
        in_n_bins = e.text().toInt();
      }
    }
    n = n.nextSibling();
  }

  commonConstructor(in_tag, in_V, xmin_in, xmax_in, in_n_bins, in_norm_mode,
                    in_color);

}


void KstHistogram::commonConstructor(const QString &in_tag, KstVectorPtr in_V,
                                     double xmin_in,
                                     double xmax_in,
                                     int in_n_bins,
                                     KstHsNormType in_norm_mode,
                                     const QColor &in_color) {
  _typeString = i18n("Histogram");
  NumUsed = 0;

  NormMode = in_norm_mode;

  _tag = in_tag;
  V = in_V;

  if (xmax_in>xmin_in) {
    MaxX = xmax_in;
    MinX = xmin_in;
  } else {
    MinX = xmax_in;
    MaxX = xmin_in;
  }
  if (MaxX==MinX) {
    MaxX+=1;
    MinX-=1;
  }

  NBins = in_n_bins;
  if (NBins<2) NBins = 2;
  Bins = (unsigned long *)malloc(in_n_bins*sizeof(unsigned long *));

  Color = in_color;

  update();
}

KstHistogram::~KstHistogram() {
  if (Bins != NULL) {
    free(Bins);
    Bins = NULL;
  }
}

KstObject::UpdateType KstHistogram::update(int update_counter) {
  int i_bin, i_pt, ns;
  double y=0;

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  if (update_counter > 0) {
    V->update(update_counter);
  }

  NS = 3*NBins+1;
  W = (MaxX - MinX)/double(NBins);

  for (i_bin = 0; i_bin < NBins; i_bin++) {
    Bins[i_bin] = 0;
  }

  ns = V->sampleCount();
  for (i_pt = 0; i_pt < ns ; i_pt++) {
    y = V->interpolate(i_pt, ns);
    i_bin = (int)floor((y-MinX)/W);
    if ((i_bin >= 0) && (i_bin < NBins)) {
      Bins[i_bin]++;
    }
  }

  MaxY = MinY = MeanY = 0;
  MinPosY = ns;

  for (i_bin=0; i_bin<NBins; i_bin++) {
    y=Bins[i_bin];
    if (y>MaxY) MaxY = y;
    if ((y>0) && (y<MinPosY)) MinPosY = y;
    MeanY += y;
  }

  if (NBins>0) MeanY/=NBins;
  else MeanY = 0;

  switch (NormMode) {
      case KST_HS_NUMBER:
        Normalization = 1.0;
        break;
      case KST_HS_PERCENT:
        if (ns>0) Normalization = 100.0/(double)ns;
        else Normalization = 1.0;
        break;
      case KST_HS_FRACTION:
        if (ns>0) Normalization = 1.0/(double)ns;
        else Normalization = 1.0;
        break;
      case KST_HS_MAX_ONE:
        if (MaxY>0) Normalization = 1.0/MaxY;
        else Normalization = 1.0;
        break;
      default:
        Normalization = 1.0;
        break;
  }
  MaxY *=Normalization;
  MinPosY *=Normalization;
  MeanY *= Normalization;

  MeanX = (MaxX + MinX)/2.0;
  if (MinX>0) MinPosX = MinX;
  else if (MaxX>0) MinPosX = W/5.0;
  else MinPosX = 1.0;

  return (UPDATE);
}

void KstHistogram::getPoint(int i, double &x, double &y) {
  x = (i+1)/3 * W + MinX;
  if (i%3==0) {
    y = 0;
  } else if ((i>=0) && (i<NS)) {
    y = Bins[i/3]*Normalization;
  } else {
    y=0;
  }
}

int KstHistogram::getBins() const {
  return NBins;
}

void KstHistogram::setXRange(double xmin_in, double xmax_in) {
  if (xmax_in > xmin_in) {
    MaxX = xmax_in;
    MinX = xmin_in;
  } else if (xmax_in<xmin_in) {
    MinX = xmax_in;
    MaxX = xmin_in;
  } else {
    MinX = xmax_in-1;
    MaxX = xmax_in+1;
  }
  W = (MaxX - MinX)/double(NBins);
}

void KstHistogram::setNBins(int in_n_bins) {

  if (Bins!=NULL) free(Bins);

  NBins = in_n_bins;

  if (NBins<2) NBins = 2;
  Bins = (unsigned long *)malloc(in_n_bins*sizeof(unsigned long *));
  W = (MaxX - MinX)/(double)NBins;
}

QString KstHistogram::getVTag() const {
  return V->tagName();
}

void KstHistogram::setVector(KstVectorPtr new_v) {
  V = new_v;
}

QString KstHistogram::getYLabel() const {
  switch (NormMode) {
      case KST_HS_NUMBER:
        return i18n("Number in Bin");
        break;
      case KST_HS_PERCENT:
        return i18n("Percent in Bin");
        break;
      case KST_HS_FRACTION:
        return i18n("Fraction in Bin");
        break;
      case KST_HS_MAX_ONE:
        return i18n("Histogram");
        break;
  }
  return i18n("Histogram");
}

QString KstHistogram::getXLabel() const {
  return V->label();
}

KstCurveType KstHistogram::type() const {
  return KST_HISTOGRAM;
}

void KstHistogram::save(QTextStream &ts) {
  ts << " <histogram>\n";
  ts << "  <tag>" << _tag << "</tag>\n";
  ts << "  <vectag>" << V->tagName() << "</vectag>\n";

  ts << "  <color>" << Color.name() << "</color>\n";
  ts << "  <hasLines>" << HasLines << "</hasLines>\n";
  ts << "  <hasPoints>" << HasPoints << "</hasPoints>\n";
  ts << "  <pointType>" << Point.getType() << "</pointType>\n";

  ts << "  <numBins>"  << NBins << "</numBins>\n";
  ts << "  <minX>" << MinX << "</minX>\n";
  ts << "  <maxX>" << MaxX << "</maxX>\n";
  ts << " </histogram>\n";
}

double KstHistogram::getVMax() const {
  return V->max();
}

double KstHistogram::getVMin() const {
  return V->min();
}

int KstHistogram::getVNumSamples() const {
  return V->sampleCount();
}

QString KstHistogram::propertyString() const {
  return i18n("Histogram: %1").arg(getVTag());
}

void KstHistogram::_showDialog() {
  KstHsDialogI::globalInstance()->show_I(tagName());
}

