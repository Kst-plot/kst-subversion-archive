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
#include "kstpainter.h"
#include "kstcurvepointsymbol.h"
#include "kst_export.h"

class KST_EXPORT KstVCurve: public KstBaseCurve {
  public:
    // InterpType order must match the order of entries of _interp in 
    // curvedialogwidget.ui
    enum InterpType {InterpY=0, InterpX=1, InterpMax=2, InterpMin=3};
    KstVCurve(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
        KstVectorPtr in_EX, KstVectorPtr in_EY,
        KstVectorPtr in_EXMinus, KstVectorPtr in_EYMinus,
        const QColor &in_color);

    KstVCurve(QDomElement &e);
    virtual ~KstVCurve();

    KstVCurve::InterpType interp() const;
    void setInterp(KstVCurve::InterpType itype);

    virtual UpdateType update(int update_counter = -1);
    virtual QString propertyString() const;

    virtual int getIndexNearXY(double x, double dx, double y) const;

    virtual bool hasXError() const;
    virtual bool hasYError() const;
    virtual bool hasXMinusError() const;
    virtual bool hasYMinusError() const;

    // Note: these are -expensive-.  Don't use them on inner loops!
    virtual void point(int i, double &x1, double &y1) const;
    virtual void getEXPoint(int i, double &x1, double &y1, double &ex1);
    virtual void getEYPoint(int i, double &x1, double &y1, double &ey1);
    virtual void getEXMinusPoint(int i, double &x1, double &y1, double &ex1);
    virtual void getEYMinusPoint(int i, double &x1, double &y1, double &ey1);
    virtual void getEXPoints(int i, double &x, double &y, double &ex, double &exminus);
    virtual void getEYPoints(int i, double &x, double &y, double &ey, double &eyminus);

    virtual double yVectorOffsetValue() const;
    virtual void setYVectorOffsetScalar(const KstScalarPtr& scalar);

    KstObjectTag xVTag() const;
    KstObjectTag yVTag() const;
    KstObjectTag xETag() const;
    KstObjectTag yETag() const;
    KstObjectTag xEMinusTag() const;
    KstObjectTag yEMinusTag() const;
    KstObjectTag yVectorOffsetTag() const;

    void setXVector(KstVectorPtr new_vx);
    void setYVector(KstVectorPtr new_vy);
    void setXError(KstVectorPtr new_ex);
    void setYError(KstVectorPtr new_ey);
    void setXMinusError(KstVectorPtr new_ex);
    void setYMinusError(KstVectorPtr new_ey);

    void save(QTextStream &ts, const QString& indent = QString::null);

    QString xLabel() const;
    QString yLabel() const;
    QString topLabel() const;

    virtual KstCurveType curveType() const;

    virtual bool xIsRising() const;

    virtual double maxX() const;
    virtual double minX() const;
    virtual double meanX() const { return _meanX; } 
    virtual double meanY() const { return _meanY; }
    virtual void yRange(double xFrom, double xTo, double* yMin, double* yMax);

    virtual int samplesPerFrame() const;

    virtual void showNewDialog();
    virtual void showEditDialog();

    KstVectorPtr xVector() const;
    KstVectorPtr yVector() const;
    KstVectorPtr xErrorVector() const;
    KstVectorPtr yErrorVector() const;
    KstVectorPtr xMinusErrorVector() const;
    KstVectorPtr yMinusErrorVector() const;
    KstScalarPtr yVectorOffsetScalar() const;

    virtual bool hasPoints()    const { return _hasPoints; }
    virtual bool hasLines()     const { return _hasLines; }
    virtual bool hasBars()      const { return _hasBars; }
    virtual void setHasPoints(bool in_HasPoints);
    virtual void setHasLines(bool in_HasLines);
    virtual void setHasBars(bool in_HasBars);
    virtual void setLineWidth(int in_LineWidth);
    virtual void setLineStyle(int in_LineStyle);
    virtual void setBarStyle( int in_BarStyle);
    virtual void setPointDensity(int in_PointDensity);
    virtual void setPointStyle(int in_PointStyle);

    virtual int lineWidth()     const { return _lineWidth; }
    virtual int lineStyle()     const { return _lineStyle; }
    virtual int barStyle()      const { return _barStyle; }
    virtual int pointDensity()  const { return _pointDensity; }
    virtual int pointStyle()    const { return _pointStyle; }

    virtual QColor color() const { return _color; }
    virtual void setColor(const QColor& new_c);

    void pushColor(const QColor& c) { _colorStack.push(color()); setColor(c); }
    void popColor() { setColor(_colorStack.pop()); }
    void pushLineWidth(int w) { _widthStack.push(lineWidth()); setLineWidth(w); }
    void popLineWidth() { setLineWidth(_widthStack.pop()); }
    void pushLineStyle(int s) { _lineStyleStack.push(lineStyle()); setLineStyle(s); }
    void popLineStyle() { setLineStyle(_lineStyleStack.pop()); }
    void pushPointStyle(int s) { _pointStyleStack.push(pointStyle()); setPointStyle(s); }
    void popPointStyle() { setPointStyle(_pointStyleStack.pop()); }
    void pushHasPoints(bool h) { _hasPointsStack.push(hasPoints()); setHasPoints(h); }
    void popHasPoints() { setHasPoints(_hasPointsStack.pop()); }
    void pushHasLines(bool h) { _hasLinesStack.push(hasLines()); setHasLines(h); }
    void popHasLines() { setHasLines(_hasLinesStack.pop()); }
    void pushPointDensity(int d) { _pointDensityStack.push(pointDensity()); setPointDensity(d); }
    void popPointDensity() { setPointDensity(_pointDensityStack.pop()); } 

    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);
    virtual void paint(const KstCurveRenderContext& context);
    virtual void paintLegendSymbol(KstPainter *p, const QRect& bound);

    // see KstBaseCurve::distanceToPoint
    virtual double distanceToPoint(double xpos, double ypos, double distanceMax, const KstCurveRenderContext& context);

    // see KstBaseCurve::providerDataObject
    virtual KstDataObjectPtr providerDataObject() const;

  private:
    inline void commonConstructor(const QString& in_tag, const QColor& in_color);

    double _meanY;

    int _barStyle;
    int _lineWidth;
    int _lineStyle;
    int _pointDensity;
    int _pointStyle;

    bool _hasPoints;
    bool _hasLines;
    bool _hasBars;

    QColor _color;
    QStack<int> _widthStack;
    QStack<QColor> _colorStack;
    QStack<int> _pointStyleStack;
    QStack<int> _lineStyleStack;
    QStack<bool> _hasPointsStack;
    QStack<bool> _hasLinesStack;
    QStack<int> _pointDensityStack;
    InterpType _interp;
};

typedef QExplicitlySharedDataPointer<KstVCurve> KstVCurvePtr;
typedef KstObjectList<KstVCurvePtr> KstVCurveList;

#endif
