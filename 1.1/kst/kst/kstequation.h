/***************************************************************************
                          kstequation.h: Equation for KST
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
#ifndef KSTEQUATION_H
#define KSTEQUATION_H

#include "kstdataobject.h"

#define MAX_DIV_REG 100

namespace Equation {
  class Node;
}

class KstEquation : public KstDataObject {
  public:
    KstEquation(const QString& in_tag, const QString& equation, double x0, double x1, int nx);
    KstEquation(const QString& in_tag, const QString& equation, KstVectorPtr xvector, bool do_interp);
    KstEquation(const QDomElement& e);
    virtual ~KstEquation();

    virtual UpdateType update(int update_counter = -1);

    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual QString propertyString() const;

    /** equations used to edit the vector */
    void setEquation(const QString &Equation);
    void setExistingXVector(KstVectorPtr xvector, bool do_interp);

    virtual QString equation() const { return _equation; }
    KstVectorPtr vX() const { return *_xVector; }
    KstVectorPtr vY() const { return *_yVector; }

    bool doInterp() const { return _doInterp; }

    bool isValid() const;

    virtual void setTagName(const QString& tag);

    virtual void _showDialog();

    virtual QString xVTag() const { return (*_xVector)->tagName(); }
    virtual QString yVTag() const { return (*_yVector)->tagName(); }

    virtual bool uses(KstObjectPtr p) const;

    const KstCurveHintList *curveHints() const;

  private:
    QString _equation;

    KstVectorMap VectorsUsed;

    void commonConstructor(const QString& in_tag, const QString& equation);

    bool FillY(bool force = false);
    bool _isValid : 1;
    bool _doInterp : 1;

    int _numNew, _numShifted, _interp, _ns;

    static const QString XVECTOR;
    static const QString OUTVECTOR;
    KstVectorMap::Iterator _xVector, _yVector;
    Equation::Node *_pe;
};

typedef KstSharedPtr<KstEquation> KstEquationPtr;
typedef KstObjectList<KstEquationPtr> KstEquationList;

#endif
// vim: ts=2 sw=2 et
