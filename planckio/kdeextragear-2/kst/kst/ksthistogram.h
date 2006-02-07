/***************************************************************************
                          ksthistogram.h: Histogram for KST
                             -------------------
    begin                : Wed July 11 2002
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
#ifndef KSTHISTOGRAM_H
#define KSTHISTOGRAM_H

#include "kstpoint.h"
#include "kstbasecurve.h"
#include <qstring.h>
#include <qcolor.h>

typedef enum {KST_HS_NUMBER, KST_HS_PERCENT, KST_HS_FRACTION, KST_HS_MAX_ONE} KstHsNormType;

class KstHistogram: public KstBaseCurve {
public:
  KstHistogram(const QString &in_tag, KstVectorPtr in_V,
               double xmin_in, double xmax_in,
               int in_n_bins,
               KstHsNormType new_norm_in,
               const QColor &in_color);
  KstHistogram(QDomElement &e);
  virtual ~KstHistogram();

  virtual UpdateType update(int update_counter = -1);

  virtual void getPoint(int i, double &x1, double &y1);
  virtual void save(QTextStream &ts);
  virtual QString propertyString() const;

  int getBins() const;
  void setNBins(int in_n_bins);

  void setXRange(double xmin_in, double xmax_in);

  QString getVTag() const;

  void setVector(KstVectorPtr);

  virtual QString getYLabel() const;
  virtual QString getXLabel() const;

  virtual KstCurveType type() const;

  double getVMax() const;
  double getVMin() const;
  int getVNumSamples() const;

  bool isNormNum()        const { return (NormMode == KST_HS_NUMBER); }
  void setIsNormNum()           { NormMode = KST_HS_NUMBER; }
  bool isNormPercent()    const { return (NormMode == KST_HS_PERCENT); }
  void setIsNormPercent()       { NormMode = KST_HS_PERCENT; }
  bool isNormFraction()   const { return (NormMode == KST_HS_FRACTION); }
  void setIsNormFraction()      { NormMode = KST_HS_FRACTION; }
  bool isNormOne()        const { return (NormMode == KST_HS_MAX_ONE); }
  void setIsNormOne()           { NormMode = KST_HS_MAX_ONE; }

protected:
  virtual void _showDialog();

private:
  KstVectorPtr V;

  int NBins;
  unsigned long *Bins;
  double Normalization;

  double W;

  void commonConstructor(const QString &in_tag, KstVectorPtr in_V,
                         double xmin_in, double xmax_in,
                         int in_n_bins,
                         KstHsNormType in_norm,
                         const QColor &in_color);

  KstHsNormType NormMode;
};

typedef KSharedPtr<KstHistogram> KstHistogramPtr;
typedef KstObjectList<KstHistogramPtr> KstHistogramList;

#endif
