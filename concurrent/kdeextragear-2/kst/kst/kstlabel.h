/***************************************************************************
                          kstlabel.h  -  description
                             -------------------
    begin                : Fri Sep 22 2000
    copyright            : (C) 2000 by cbn
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

#ifndef KSTLABEL_H
#define KSTLABEL_H

#include "kstscalar.h"
#include <qdom.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qstring.h>

typedef enum {CxBy, CxTy, CxCy, LxBy, LxTy,
              LxCy, RxBy, RxTy, RxCy} KstJustifyType;

/** A class of labels for kstplot

 *@author cbn
 */

class KstLabel {
public:
  KstLabel( );
  KstLabel(const QString &in_text, KstJustifyType in_j = CxBy,
           float in_rotation = 0.0, float in_X=0.0,
           float in_Y=0.0, bool is_sample = false);
  ~KstLabel();

  void setText(const QString &text);
  void setRotation(float new_rotation);
  void setRelPosition(float new_x, float new_y);
  void offsetRelPosition(float offset_x, float offset_y);
  void setJustification(KstJustifyType Justify);

  KstJustifyType justification() const {return Justify;}

  /** Return width and height of the label (from font metrics) */
  int width(QPainter &p);
  int ascent(QPainter &p);
  int lineSpacing(QPainter &p);

  float rotation() const {return Rotation;}

  /** draw the label with painter at px, py */
  void draw(QPainter &p, int px, int py, bool bJustify=true, bool doDraw=true);

  /** Interpret special characters, default = true */
  void setInterpreted(bool interpreted) { _interpret = interpreted; }

  void setSize(int size);
  void setFontName(const QString &fontName);
  QString fontName();
  int size();

  void save(QTextStream &ts);
  void read(QDomElement &e);

  const QString& text() const;

  void setDoScalarReplacement(bool in_do);

  inline double x() const {return _x;}
  inline double y() const {return _y;}

  QRegion extents;
  int     v_offset;
  
private:
  /** Computes the font size, given the size of the painter, and Size */
  int fontSize(QPainter &p);

  bool _interpret;
  float Rotation;
  QString Text;
  KstJustifyType Justify;

  QString SymbolFontName;
  QString FontName;
  int Size;
  float Width; /* set by draw */

  /** relative position of label: stored for use by the parent class: since
   the geometry of the parent may have scaled and un-scaled regions, KstLabel
   can not blindly use these.  So we leave it up to the parent.  For top,
   bottom, tick, and axis labels, they are not used at all.  */
  double _x;
  double _y;

  KstScalarList ScalarsUsed;

  bool doScalarReplacement;
  bool IsSample;
};


class KstLabelDrag : public QDragObject {
  public:
    KstLabelDrag(QWidget *dragSource, int p, int n, const QPoint& hotSpot, const QPixmap& pm);
    virtual ~KstLabelDrag();

    virtual const char *format(int i = 0) const;

    virtual QByteArray encodedData(const char *mime) const;

  private:
    int _plot, _label;
    QPoint _hs;
};

typedef QPtrList<KstLabel> KstLabelList;

#endif
