/***************************************************************************
                          kstequationcurve.h: Power Spectra for KST
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

/** A class for handling equations for kst
 *@author C. Barth Netterfield
 */
#ifndef KSTEQUATIONCURVE_H
#define KSTEQUATIONCURVE_H

#include "kstpoint.h"
#include "kstbasecurve.h"
#include "kstscalar.h"
#include <qstring.h>
#include <qcolor.h>

#define MAX_DIV_REG 100

namespace Equation {
  class Node;
}

class KstEquationCurve : public KstBaseCurve {
public:
  KstEquationCurve(const QString &in_tag, const QString &equation,
                   double x0, double x1, int nx,
                   const QColor &in_color);
  KstEquationCurve(const QString &in_tag, const QString &equation,
                   KstVectorPtr xvector, bool do_interp,
                   const QColor &in_color);
  KstEquationCurve(QDomElement &e);
  virtual ~KstEquationCurve();

  virtual UpdateType update(int update_counter = -1);

  virtual void getPoint(int i, double &x1, double &y1);
  virtual void save(QTextStream &ts);
  virtual QString propertyString() const;

  /** equations used to edit the vector */
  void setEquation(const QString &Equation);
  void setExistingXVector(KstVectorPtr xvector, bool do_interp);
  void setStaticXVector(double xmin, double xmax, int n);

  bool isXStatic() const { return _staticX; }

  virtual QString getYLabel() const;
  virtual QString getXLabel() const;

  virtual KstCurveType type() const;

  virtual QString equation() const { return Equation; }
  KstVectorPtr vX() { return _inputVectors[XVECTOR]; }

  virtual bool slaveVectorsUsed() const;
  bool doInterp() const { return DoInterp; }

  bool isValid() const;

  virtual void setTagName(const QString& tag);

  virtual bool loadInputs();

protected:
  virtual void _showDialog();

private:
  QString Equation;

  char String[1024];

  bool _staticX;

  KstVectorList VectorsUsed;

  void commonConstructor(const QString &in_tag, const QString &equation,
                         const QColor &in_color);

  bool FillY(bool force = false);
  bool IsValid;

  int NumNew, NumShifted;

  bool DoInterp;
  int Interp;

  static const QString XVECTOR;
  static const QString OUTVECTOR;
  KstVectorMap::Iterator _xVector, _yVector;
  Equation::Node *_pe;
};

typedef KstSharedPtr<KstEquationCurve> KstEquationCurvePtr;
typedef KstObjectList<KstEquationCurvePtr> KstEquationCurveList;

#endif
