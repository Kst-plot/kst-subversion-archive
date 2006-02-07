/***************************************************************************
                          kstpsdcurve.h: Power Spectra for KST
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

#ifndef KSTPSDCURVE_H
#define KSTPSDCURVE_H

#include "kstpoint.h"
#include "kstbasecurve.h"
#include <qstring.h>
#include <qcolor.h>

class KstPSDCurve: public KstBaseCurve {
public:
  KstPSDCurve(const QString &in_tag, KstVectorPtr in_V, double freq,
              int len,
              const QString &VUnits, const QString &RUnits,
              const QColor &in_color);
  KstPSDCurve(QDomElement &e);
  virtual ~KstPSDCurve();

  virtual UpdateType update(int update_counter = -1);

  virtual void getPoint(int i, double &x1, double &y1);
  virtual void save(QTextStream &ts);
  virtual QString propertyString() const;

  double getFreq() const;
  void setFreq(double in_freq);

  int getLen() const;
  void setLen(int in_len);

  QString getVTag() const;
  void setVector(KstVectorPtr);

  virtual QString getYLabel() const;
  virtual QString getXLabel() const;

  QString VUnits;
  QString RUnits;

  virtual KstCurveType type() const;

  virtual bool slaveVectorsUsed() const;

  bool appodize()                  const { return Appodize; }
  bool removeMean()                const { return RemoveMean; }
  void setAppodize(bool in_appodize)      { Appodize = in_appodize; }
  void setRemoveMean(bool in_remove_mean) { RemoveMean = in_remove_mean; }

protected:
  virtual void _showDialog();

private:
  double Freq;
  int Len;

  double norm_factor;

  int last_f0;
  int last_n_subsets;
  int last_n_new;

  double *w;
  int PSDLen;
  double *a;
  int ALen;

  void GenW();

  void commonConstructor(const QString &in_tag, KstVectorPtr in_V,
                         double freq, int len,
                         const QString &VUnits, const QString &RUnits,
                         const QColor &in_color);

  bool Appodize;
  bool RemoveMean;

  KstVectorMap::Iterator _sVector, _fVector;
};

typedef KstSharedPtr<KstPSDCurve> KstPSDCurvePtr;
typedef KstObjectList<KstPSDCurvePtr> KstPSDCurveList;

#endif
