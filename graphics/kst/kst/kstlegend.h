/***************************************************************************
                          kstlegend.h  -  description
                             -------------------
    begin                : Jan 9th 2004
    copyright            : (C) 2000 The University of British Columbia
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

#include "kst2dplot.h"

enum KstLegendLayoutType { LeftTop, CentreTop, RightTop, LeftCentre, CentreCentre, RightCentre, LeftBottom, CentreBottom, RightBottom };
enum KstLegendAlignmentType { AlignmentLeft, AlignmentCentre, AlignmentRight };

class KstLegend : public KstShared, public KstRWLock {
  public:
    KstLegend(KstJustifyType in_j = CxBy, float in_X = 0.015, float in_Y = 0.02, bool is_sample = false);
    KstLegend(const KstLegend *legend);
    ~KstLegend();

    void setRelPosition(float new_x, float new_y);
    void offsetRelPosition(float offset_x, float offset_y);
    void setJustification(KstJustifyType Justify);
    void setShow(bool show);
    bool getShow() const { return ShowLegend; }
    void setFront(bool in_bFront);
    bool getFront() const { return bFront; }
    void setColorBackground( QColor in_color );
    const QColor& getColorBackground() const { return colorBackground; }
    void setColorForeground( QColor in_color );
    const QColor& getColorForeground() const { return colorForeground; }
    void setLayout(KstLegendLayoutType type) { LayoutType = type; }
    void setAlignment(KstLegendAlignmentType type) { AlignmentType = type; }
    const KstLegendAlignmentType& alignment() const { return AlignmentType; }
    KstJustifyType justification() const { return Justify; }

    /** draw the label with painter at px, py */
    void draw(KstBaseCurveList* pCurves, QPainter &p, int px, int py);

    void setSize(int size);
    void setFontName(const QString &fontName);
    const QString& fontName() const;
    int size() const;

    void save(QTextStream &ts, const QString& indent = QString::null);
    void read(const QDomElement &e);

    void setDoScalarReplacement(bool in_do);

    inline double x() const { return _x; }
    inline double y() const { return _y; }

    QRegion extents;
    int v_offset;

  private:
    /** Computes the font size, given the size of the painter, and Size */
    int fontSize(QPainter &p);

    KstJustifyType Justify;

    QString SymbolFontName;
    QString FontName;
    QColor colorBackground;
    QColor colorForeground;
    int Size;

    /** relative position of label: stored for use by the parent class: since
      the geometry of the parent may have scaled and un-scaled regions, KstLegend
      can not blindly use these.  So we leave it up to the parent.  For top,
      bottom, tick, and axis labels, they are not used at all.  */
    double _x;
    double _y;

    KstScalarList ScalarsUsed;
    KstLegendLayoutType LayoutType;
    KstLegendAlignmentType AlignmentType;

    bool bFront : 1;
    bool doScalarReplacement : 1;
    bool IsSample : 1;
    bool ShowLegend : 1;
};


class KstLegendDrag : public QDragObject {
  public:
    KstLegendDrag(QWidget *dragSource, const QString& window, Kst2DPlotPtr plot, const QPoint& hotSpot, const QPixmap& pm);
    virtual ~KstLegendDrag();

    virtual const char *format(int i = 0) const;

    virtual QByteArray encodedData(const char *mime) const;

  private:
    QString _window;
    Kst2DPlotPtr _plot;
    QPoint _hs;
};

#endif
// vim: ts=2 sw=2 et
