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

#include <qdragobject.h>
#include <qpainter.h>

#include "kstplotbase.h"
#include "kstsharedptr.h"
#include "rwlock.h"

enum KstJustifyType { CxBy, CxTy, CxCy, LxBy, LxTy, LxCy, RxBy, RxTy, RxCy };

/** A class of labels for kstplot
 *@author cbn
 */

class KstLabel : public KstShared, public KstRWLock {
  public:
    KstLabel();
    KstLabel(const QString& in_text, KstJustifyType in_j = CxBy,
        float in_rotation = 0.0, float in_X = 0.0,
        float in_Y = 0.0, bool is_sample = false);
    KstLabel(const KstLabel* label);
    ~KstLabel();

    void setDirty(bool dirty = true);
    bool dirty() const;

    void setText(const QString& text);
    void setRotation(float new_rotation);
    void setRelPosition(float new_x, float new_y);
    void offsetRelPosition(float offset_x, float offset_y);
    void setJustification(KstJustifyType Justify);
    void setResized();

    KstJustifyType justification() const { return _justification; }

    /** Return width and height of the label (from font metrics) */
    int width(QPainter& p);
    int ascent(QPainter& p);
    int rotatedWidth(QPainter& p);
    int rotatedHeight(QPainter& p);
    int lineSpacing(QPainter& p);

    const QString& text() const;
    float rotation() const { return Rotation; }
    double rotationRadians() const { return 3.1415926535897932333796 * (double)Rotation / 180.0; }

    const QColor& color() const;
    void setColor(const QColor& color);
    bool usePlotColor() const;
    void setUsePlotColor(bool usePlotColor);

    /** draw the label with painter at px, py */
    void draw(QPainter& p, int px, int py, bool justify = true, bool doDraw = true);

    /** Interpret special characters, default = true */
    void setInterpreted(bool interpreted);
    bool interpreted() const;

    void setSize(int size);
    void setFontName(const QString& fontName);
    QString fontName() const;
    int size() const;

    void save(QTextStream& ts, const QString& indent = QString::null,
              bool save_pos=true);
    void read(const QDomElement& e);

    void setDoScalarReplacement(bool in_do);
    bool doScalarReplacement() const;

    inline double x() const { return _x; }
    inline double y() const { return _y; }

    QRegion extents;
    int     v_offset;

  private:
    /** Computes the font size, given the size of the painter, and Size */
    int fontSize(QPainter& p);

    float Rotation;
    QString Text;
    KstJustifyType _justification;

    QString SymbolFontName;
    QString FontName;
    QColor _color;
    int Size;
    float Width; /* set by draw */
    int LineSpacing;

    /** relative position of label: stored for use by the parent class: since
      the geometry of the parent may have scaled and un-scaled regions, KstLabel
      can not blindly use these.  So we leave it up to the parent.  For top,
      bottom, tick, and axis labels, they are not used at all.  */
    double _x;
    double _y;

    KstScalarList ScalarsUsed;
    KstStringList StringsUsed;

    bool _interpret : 1;
    bool _usePlotColor : 1;
    bool _doScalarReplacement : 1;
    bool IsSample : 1;
    bool _dirty : 1;
};


class KstLabelDrag : public QDragObject {
  public:
    KstLabelDrag(QWidget *dragSource, const QString& window, KstPlotBasePtr plot, int n, const QPoint& hotSpot, const QPixmap& pm);
    virtual ~KstLabelDrag();

    virtual const char *format(int i = 0) const;

    virtual QByteArray encodedData(const char *mime) const;

  private:
    int _label;
    QString _window;
    KstPlotBasePtr _plot;
    QPoint _hs;
};

typedef KstSharedPtr<KstLabel> KstLabelPtr;
typedef QValueList<KstLabelPtr> KstLabelList;

#endif
// vim: ts=2 sw=2 et
