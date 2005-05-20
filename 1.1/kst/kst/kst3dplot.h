/***************************************************************************
                              kst2dplot.h
                             -------------
    begin                : Mar 28, 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef KST3DPLOT_H
#define KST3DPLOT_H

#include "kstplotbase.h"

class Kst3DPlot : public KstPlotBase {
  Q_OBJECT
  public:
    Kst3DPlot();
    Kst3DPlot(QDomElement& e);
    virtual ~Kst3DPlot();

    virtual UpdateType update();
    virtual void save(QTextStream& ts);

    virtual bool popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
    virtual bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);

  public slots:
    virtual void paint(KstPaintType type, QPainter& p);

  protected:
    virtual KstViewObjectFactoryMethod factory() const;
};

typedef KstSharedPtr<Kst3DPlot> Kst3DPlotPtr;
typedef KstObjectList<Kst3DPlotPtr> Kst3DPlotList;


#endif
// vim: ts=2 sw=2 et
