/***************************************************************************
                          kstvcurve.h: defines a curve for kst
                             -------------------
    begin                : Fri Oct 22 2000
    copyright            : (C) 2000 by C. Barth Netterfield
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

#ifndef KSTVCURVE_H
#define KSTVCURVE_H

#include "kstbasecurve.h"

/**A class for handling curves for kst
 *@author C. Barth Netterfield
 */

class KstVCurve: public KstBaseCurve {
  public:
    KstVCurve(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
        KstVectorPtr in_EX, KstVectorPtr in_EY,
        KstVectorPtr in_EXMinus, KstVectorPtr in_EYMinus,
        const QColor &in_color);
    KstVCurve(QDomElement &e);
    virtual ~KstVCurve();

    virtual UpdateType update(int update_counter = -1);
    virtual QString propertyString() const;

    virtual bool hasXError() const;
    virtual bool hasYError() const;
    virtual bool hasXMinusError() const;
    virtual bool hasYMinusError() const;

    virtual void point(int i, double &x1, double &y1);
    virtual void getEXPoint(int i, double &x1, double &y1, double &ex1);
    virtual void getEYPoint(int i, double &x1, double &y1, double &ey1);
    virtual void getEXMinusPoint(int i, double &x1, double &y1, double &ex1);
    virtual void getEYMinusPoint(int i, double &x1, double &y1, double &ey1);
    virtual void getEXPoints(int i, double &x, double &y, double &ex, double &exminus);
    virtual void getEYPoints(int i, double &x, double &y, double &ey, double &eyminus);

    QString xVTag() const;
    QString yVTag() const;
    QString xETag() const;
    QString yETag() const;
    QString xEMinusTag() const;
    QString yEMinusTag() const;

    void setXVector(KstVectorPtr new_vx);
    void setYVector(KstVectorPtr new_vy);
    void setXError(KstVectorPtr new_ex);
    void setYError(KstVectorPtr new_ey);
    void setXMinusError(KstVectorPtr new_ex);
    void setYMinusError(KstVectorPtr new_ey);

    /** Save curve information */
    void save(QTextStream &ts, const QString& indent = QString::null);

    QString xLabel() const;
    QString yLabel() const;
    QString topLabel() const;

    virtual KstCurveType type() const;

    virtual bool loadInputs();

    virtual bool xIsRising() const {return VX->isRising(); }

    virtual double ns_maxX()    const { return _ns_maxx; }
    virtual double ns_minX()    const { return _ns_minx; }
    virtual double ns_maxY()    const { return _ns_maxy; }
    virtual double ns_minY()    const { return _ns_miny; }

    virtual int samplesPerFrame() const;

    virtual void _showDialog();

    virtual bool uses(KstObjectPtr p) const;

    KstVectorPtr xVector() const;
    KstVectorPtr yVector() const;
    KstVectorPtr xErrorVector() const;
    KstVectorPtr yErrorVector() const;
    KstVectorPtr xMinusErrorVector() const;
    KstVectorPtr yMinusErrorVector() const;

  private:
    inline void commonConstructor(const QString &in_tag,
        const QColor &in_color);

    KstVectorPtr VX;
    KstVectorPtr VY;
    KstVectorPtr EX;
    KstVectorPtr EY;
    KstVectorPtr EXMinus;
    KstVectorPtr EYMinus;

    double _ns_maxx;
    double _ns_minx;
    double _ns_maxy;
    double _ns_miny;
};

typedef KstSharedPtr<KstVCurve> KstVCurvePtr;
typedef KstObjectList<KstVCurvePtr> KstVCurveList;

#endif
// vim: ts=2 sw=2 et
