/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLOTITEMMANAGER_H
#define PLOTITEMMANAGER_H

#include <QObject>

#include <QHash>
#include <QList>

namespace Kst {

class View;
class PlotItem;

class PlotItemManager : public QObject
{
  Q_OBJECT
  public:
    static QList<PlotItem*> plotsForView(View *view);
    static QList<PlotItem*> tiedZoomPlotsForView(View *view);

  private:
    static PlotItemManager *self();
    static void cleanup();

    PlotItemManager();
    virtual ~PlotItemManager();

    void addPlot(PlotItem *plotItem);
    void removePlot(PlotItem *plotItem);

    void addTiedZoomPlot(PlotItem *plotItem);
    void removeTiedZoomPlot(PlotItem *plotItem);

  private:
    friend class PlotItem;
    QHash< View*, QList<PlotItem*> > _plotLists;
    QHash< View*, QList<PlotItem*> > _tiedZoomPlotLists;
};

}

#endif

// vim: ts=2 sw=2 et
