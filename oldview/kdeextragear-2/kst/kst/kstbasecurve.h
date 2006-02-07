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
#include "kstpoint.h"
#include "kstdataobject.h"

#include <qstring.h>
#include <qcolor.h>

/**A class for handling curves for kst

 *@author C. Barth Netterfield
 */

typedef enum {KST_VCURVE, KST_PSDCURVE, KST_EQUATIONCURVE, KST_HISTOGRAM} KstCurveType;

class KstBaseCurve: public KstDataObject {
public:
  KstBaseCurve();
  KstBaseCurve(QDomElement& e);
  virtual ~KstBaseCurve();

  virtual void save(QTextStream &ts) = 0;
  virtual QString propertyString() const = 0;
  virtual void getPoint(int i, double &x1, double &y1) = 0;
  virtual int getIndexNearX(double x);
  virtual int getIndexNearXY(double x, double dx, double y);
  virtual KstCurveType type() const = 0;

  virtual void getEXPoint(int i, double &x1, double &y1, double &ex1) { Q_UNUSED(i); x1 = y1 = ex1 = 0;}
  virtual void getEYPoint(int i, double &x1, double &y1, double &ey1) { Q_UNUSED(i); x1 = y1 = ey1 = 0;}

  virtual double maxX()    const { return MaxX; }
  virtual double minX()    const { return MinX; }
  virtual double ns_maxX()    const { return MaxX; }
  virtual double ns_minX()    const { return MinX; }
  virtual double minPosX() const { return MinPosX; }
  virtual double meanX()   const { return MeanX; }
  virtual double midX()    const { return (MaxX+MinX)*0.5; }
  virtual double maxY()    const { return MaxY; }
  virtual double minY()    const { return MinY; }
  virtual double ns_maxY()    const { return MaxY; }
  virtual double ns_minY()    const { return MinY; }
  virtual double minPosY() const { return MinPosY; }
  virtual double meanY()   const { return MeanY; }
  virtual double midY()    const { return (MaxY+MinY)*0.5; }

  virtual int sampleCount() const { return NS; }

  virtual QColor getColor() const { return Color; }

  virtual void setColor(const QColor &new_c) { Color = new_c; }

  virtual bool hasXError() const { return false; }
  virtual bool hasYError() const { return false; }

  virtual QString getXLabel() const   { return QString::null; }
  virtual QString getYLabel() const   { return QString::null; }
  virtual QString getTopLabel() const { return QString::null; }

  virtual void setHasPoints(bool in_HasPoints) { HasPoints = in_HasPoints; }
  virtual void setHasLines(bool in_HasLines)   { HasLines = in_HasLines; }
  virtual void setLineWidth(int in_LineWidth) { LineWidth = in_LineWidth; }
  virtual void setLineStyle(int in_LineStyle)  { LineStyle = in_LineStyle; }
  virtual bool hasPoints() const { return HasPoints; }
  virtual bool hasLines()  const { return HasLines; }
  virtual int lineWidth() const { return LineWidth; }
  virtual int lineStyle()  const { return LineStyle; }

  virtual bool slaveVectorsUsed() const { return false; }

  virtual bool xIsRising() const {return false;}

  virtual int samplesPerFrame() const { return 1; }

  KstPoint Point;

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

  int NumUsed;

  bool HasPoints;
  bool HasLines;
  int  LineWidth;
  int  LineStyle;
  QColor Color;

private:
  void commonConstructor();

};


typedef KstSharedPtr<KstBaseCurve> KstBaseCurvePtr;
typedef KstObjectList<KstBaseCurvePtr> KstBaseCurveList;

#endif
