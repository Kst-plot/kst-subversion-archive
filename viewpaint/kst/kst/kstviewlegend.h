/***************************************************************************
                              kstviewlegend.h
                             ----------------
    begin                : Apr 10 2004
    copyright            : (C) 2000 by cbn
                           (C) 2004 by The University of Toronto
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

#ifndef KSTVIEWLEGEND_H
#define KSTVIEWLEGEND_H

#include "kstbackbuffer.h"
#include "kstborderedviewobject.h"
#include "kstscalar.h"
#include "labelparser.h"
#include "kstbasecurve.h"

#include <qguardedptr.h>

class KstViewLegendDialogI;
class Kst2DPlot;
typedef KstSharedPtr<Kst2DPlot> Kst2DPlotPtr;

class KstViewLegend : public KstBorderedViewObject {
  Q_OBJECT
  Q_PROPERTY(QString font READ fontName WRITE setFontName)
  Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)
  Q_PROPERTY(bool transparent READ transparent WRITE setTransparent)
  Q_PROPERTY(bool vertical READ vertical WRITE setVertical)
  public:
    KstViewLegend();
    KstViewLegend(const QDomElement& e);
    virtual ~KstViewLegend();

    int ascent() const;

    void setFontName(const QString& fontName);
    const QString& fontName() const;

    void setFontSize(int size);
    int fontSize() const;

    void save(QTextStream& ts, const QString& indent = QString::null);

    void setTransparent(bool transparent);
    bool transparent() const;

    void updateSelf();
    void paintSelf(KstPainter& p, const QRegion& bounds);
    void resize(const QSize&);
    QRegion clipRegion();
    
    //virtual QMap<QString, QVariant> widgetHints(const QString& propertyName) const;
    QWidget *configWidget();

    // handle custom widget, if any: is called by KstEditViewObjectDialogI
    bool fillConfigWidget(QWidget *w, bool isNew) const;
    bool readConfigWidget(QWidget *w);

    void addCurve(KstBaseCurvePtr curve);
    void removeCurve(KstBaseCurvePtr curve);
    void clear();

    void setCurveList(Kst2DPlotPtr plot);
    
    bool vertical() const;
    void setVertical(bool vertical);

    KstBaseCurveList& curves();
  public slots:
    void adjustSizeForText(QRect w);

  protected:
    KstViewObjectFactoryMethod factory() const;
    bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
    void readBinary(QDataStream& str);
    void writeBinary(QDataStream& str);

  private:
    void drawToBuffer();
    void drawToPainter(QPainter& p);
    void tmpDrawToPainter(QPainter& p);
    void computeTextSize();

    double _rotation;
    QString _fontName;
    KstScalarList _scalarsUsed;

    bool _replace : 1;
    bool _vertical : 1;
    int _absFontSize; // points
    int _fontSize; // size relative to reference size.....
    int _textWidth, _textHeight, _ascent;
    KstBackBuffer _backBuffer;
    KstBaseCurveList _curves;
};

typedef KstSharedPtr<KstViewLegend> KstViewLegendPtr;

#endif
// vim: ts=2 sw=2 et
