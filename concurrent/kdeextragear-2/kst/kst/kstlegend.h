/***************************************************************************
                          kstlegend.h  -  description
                             -------------------
    begin                : Jan 9th 2004
    copyright            : (C) 2000 by arw
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

#ifndef KSTLEGEND_H
#define KSTLEGEND_H

#include "kstlabel.h"
#include "kstscalar.h"
#include "kstbasecurve.h"
#include <qdom.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qstring.h>

typedef enum {LeftTop, CentreTop, RightTop, LeftCentre, CentreCentre,
              RightCentre, LeftBottom, CentreBottom, RightBottom} KstLegendLayoutType;
typedef enum {AlignmentLeft, AlignmentCentre, AlignmentRight} KstLegendAlignmentType;

class KstLegend {
public:
  KstLegend(KstJustifyType in_j = CxBy,
           float in_rotation = 0.0, float in_X=0.015,
           float in_Y=0.02, bool is_sample = false);
  ~KstLegend();

  void setRotation(float new_rotation);
  void setRelPosition(float new_x, float new_y);
  void offsetRelPosition(float offset_x, float offset_y);
  void setJustification(KstJustifyType Justify);
  void setShow(bool bShow);
  bool getShow() { return ShowLegend; }
  void setFront(bool in_bFront);
  bool getFront() { return bFront; }
  void setColorBackground( QColor in_color );
  QColor getColorBackground() { return colorBackground; }
  void setColorForeground( QColor in_color );
  QColor getColorForeground() { return colorForeground; }
  void setLayout(KstLegendLayoutType type) { LayoutType = type;}
  void setAlignment(KstLegendAlignmentType type) { AlignmentType = type;}
  float rotation() const {return Rotation;}
  KstJustifyType justification() const {return Justify;}

  /** draw the label with painter at px, py */
  void draw(KstBaseCurveList* pCurves, QPainter &p, int px, int py);

  void setSize(int size);
  void setFontName(const QString &fontName);
  QString fontName();
  int size();

  void save(QTextStream &ts);
  void read(QDomElement &e);

  void setDoScalarReplacement(bool in_do);

  inline double x() const {return _x;}
  inline double y() const {return _y;}

  QRegion extents;
  int     v_offset;

private:
  /** Computes the font size, given the size of the painter, and Size */
  int fontSize(QPainter &p);

  float Rotation;
  KstJustifyType Justify;

  QString SymbolFontName;
  QString FontName;
  QColor colorBackground;
  QColor colorForeground;
  int Size;
  bool bFront;

  /** relative position of label: stored for use by the parent class: since
   the geometry of the parent may have scaled and un-scaled regions, KstLegend
   can not blindly use these.  So we leave it up to the parent.  For top,
   bottom, tick, and axis labels, they are not used at all.  */
  double _x;
  double _y;

  KstScalarList ScalarsUsed;
  KstLegendLayoutType LayoutType;
  KstLegendAlignmentType AlignmentType;

  bool doScalarReplacement;
  bool IsSample;
  bool ShowLegend;
};


class KstLegendDrag : public QDragObject {
  public:
    KstLegendDrag(QWidget *dragSource, int p, const QPoint& hotSpot, const QPixmap& pm);
    virtual ~KstLegendDrag();

    virtual const char *format(int i = 0) const;

    virtual QByteArray encodedData(const char *mime) const;

  private:
    int _plot;
    QPoint _hs;
};

#endif
