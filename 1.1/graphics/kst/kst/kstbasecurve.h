/***************************************************************************
                     kstbasecurve.h: base curve type for kst
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

#ifndef KSTBASECURVE_H
#define KSTBASECURVE_H

#include <qvaluestack.h>

#include "kstpoint.h"
#include "kstdataobject.h"

/**A class for handling curves for kst
 *@author C. Barth Netterfield
 */

enum KstCurveType { KST_VCURVE, KST_HISTOGRAM };

class KstBaseCurve : public KstDataObject {
  public:
    KstBaseCurve();
    KstBaseCurve(const QDomElement& e);
    virtual ~KstBaseCurve();

    virtual void _showDialog() { }
    virtual void save(QTextStream &ts, const QString& indent = QString::null) = 0;
    virtual QString propertyString() const = 0;
    virtual void point(int i, double &x1, double &y1) = 0;
    virtual int getIndexNearX(double x);
    virtual int getIndexNearXY(double x, double dx, double y);
    virtual KstCurveType type() const = 0;

    virtual void getEXPoint(int i, double &x1, double &y1, double &ex1) { Q_UNUSED(i) x1 = y1 = ex1 = 0.0;}
    virtual void getEYPoint(int i, double &x1, double &y1, double &ey1) { Q_UNUSED(i) x1 = y1 = ey1 = 0.0;}
    virtual void getEXMinusPoint(int i, double &x1, double &y1, double &ex1) { Q_UNUSED(i) x1 = y1 = ex1 = 0.0;}
    virtual void getEYMinusPoint(int i, double &x1, double &y1, double &ey1) { Q_UNUSED(i) x1 = y1 = ey1 = 0.0;}
    virtual void getEXPoints(int i, double &x1, double &y1, double &ex1, double &ex2) { Q_UNUSED(i) x1 = y1 = ex1 = ex2 = 0.0;}
    virtual void getEYPoints(int i, double &x1, double &y1, double &ey1, double &ey2) { Q_UNUSED(i) x1 = y1 = ey1 = ey2 = 0.0;}

    virtual double maxX() const; //    const {  }
    virtual double minX() const; // { return MinX; }
    virtual double ns_maxX() const { return maxX(); } // for vcurves, these
    virtual double ns_minX() const { return minX(); } // ignore spikes.
    virtual double minPosX() const { return MinPosX; }
    virtual double meanX() const { return MeanX; }
    virtual double midX() const { return (MaxX+MinX)*0.5; }
    virtual double maxY() const { return MaxY; }
    virtual double minY() const { return MinY; }
    virtual double ns_maxY() const { return MaxY; }
    virtual double ns_minY() const { return MinY; }
    virtual double minPosY() const { return MinPosY; }
    virtual double meanY() const { return MeanY; }
    virtual double midY() const { return (MaxY+MinY)*0.5; }

    virtual int sampleCount() const { return NS; }

    virtual QColor color() const { return Color; }
    virtual void setColor(const QColor& new_c);
    void pushColor(const QColor& c) { _colorStack.push(color()); setColor(c); }
    void popColor() { setColor(_colorStack.pop()); }

    virtual bool hasXError() const { return false; }
    virtual bool hasYError() const { return false; }
    virtual bool hasXMinusError() const { return false; }
    virtual bool hasYMinusError() const { return false; }

    virtual QString xLabel() const   { return QString::null; }
    virtual QString yLabel() const   { return QString::null; }
    virtual QString topLabel() const { return QString::null; }

    virtual void setHasPoints(bool in_HasPoints);
    virtual void setHasLines(bool in_HasLines);
    virtual void setHasBars(bool in_HasBars);
    virtual void setLineWidth(int in_LineWidth);
    virtual void setLineStyle(int in_LineStyle);
    virtual void setBarStyle( int in_BarStyle);
    virtual void setPointDensity(int in_PointDensity);
    virtual bool hasPoints()    const { return HasPoints; }
    virtual bool hasLines()     const { return HasLines; }
    virtual bool hasBars()      const { return HasBars; }
    virtual int lineWidth()     const { return LineWidth; }
    virtual int lineStyle()     const { return LineStyle; }
    virtual int barStyle()      const { return BarStyle; }
    virtual int pointDensity()  const { return PointDensity; }

    virtual void setIgnoreAutoScale(bool ignoreAutoScale);
    virtual bool ignoreAutoScale() const { return _ignoreAutoScale; }

    void pushLineWidth(int w) { _widthStack.push(lineWidth()); setLineWidth(w); }
    void popLineWidth() { setLineWidth(_widthStack.pop()); }

    virtual bool slaveVectorsUsed() const { return false; }

    virtual bool xIsRising() const { return false; }

    virtual int samplesPerFrame() const { return 1; }

    virtual QString xVTag() const { return "<None>"; }
    virtual QString yVTag() const { return "<None>"; }
    virtual QString yETag() const { return "<None>"; }
    virtual QString xETag() const { return "<None>"; }
    virtual QString yEMinusTag() const { return "<None>"; }
    virtual QString xEMinusTag() const { return "<None>"; }
    KstPoint Point;

    virtual bool deleteDependents();

  protected:
    double MaxX;
    double MinX;
    double MinPosX;
    double MeanX;
    double MaxY;
    double MinY;
    double MinPosY;
    double MeanY;
    int NS;

    bool _ignoreAutoScale;
    bool HasPoints;
    bool HasLines;
    bool HasBars;
    int BarStyle;
    int LineWidth;
    int LineStyle;
    int PointDensity;
    QColor Color;
    QValueStack<int> _widthStack;
    QValueStack<QColor> _colorStack;

  private:
    void commonConstructor();
};


typedef KstSharedPtr<KstBaseCurve> KstBaseCurvePtr;
typedef KstObjectList<KstBaseCurvePtr> KstBaseCurveList;

#endif
// vim: ts=2 sw=2 et
