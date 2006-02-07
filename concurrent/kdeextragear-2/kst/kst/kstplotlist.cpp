/***************************************************************************
                          kstplotlist.cpp  -  description
                             -------------------
    begin                :     Dec 3 2000
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

#include "kstplotlist.h"

KstPlotList::KstPlotList(){
  setAutoDelete( TRUE );
  PlotCols = 1;
}

KstPlotList::~KstPlotList(){
}

KstPlot *KstPlotList::FindKstPlot(const QString &tag) {
  KstPlot *tmp;

  for (tmp = first(); tmp!=0; tmp=next()) {
    if (tag == tmp->tagName()) {
      return (tmp);
    }
  }
  return 0L;
}

unsigned KstPlotList::getPlotCols() {
  return PlotCols;
}

void KstPlotList::setPlotCols(unsigned in_cols) {
  PlotCols = in_cols;
}

void KstPlotList::arrangePlots(int cols) {
  int rows, n_plots;
  int i_plot;
  float width, height, ltx, lty;

  n_plots = count();
  rows = (n_plots-1)/cols + 1;
  width = 1.0/float(cols);
  height = 1.0/float(rows);

  for (i_plot=0; i_plot < n_plots; i_plot++) {
    ltx = width * float(int(i_plot%cols));
    lty = height * float(int(i_plot/cols));
    at(i_plot)->setDims(width, height, ltx, lty);
  }

  setPlotCols(cols);
}

QStringList KstPlotList::tagNames() {
  QStringList plotNames;
  for (unsigned i = 0; i < count(); i++) {
    plotNames << at(i)->tagName();
  }
  return plotNames;
}

QString KstPlotList::generatePlotName() {
  int i = count();
  QString plot;

  do {
    plot = "P"+QString::number(i++);
  } while (FindKstPlot(plot)!=0L);

  return plot;
}

KstPlot *KstPlotList::addPlot(const QString& name, int cols) {
  KstPlot *newplot = 0L;
  QString plot_name = name;

  if (plot_name.isEmpty()) {
    plot_name = generatePlotName();
  } else if (FindKstPlot(plot_name)!=0L) {
    do {
      plot_name += "'";
    } while (FindKstPlot(plot_name)!=0L);
  }

  newplot = new KstPlot(plot_name);
  append(newplot);

  if (cols <= 0) {
    cols = getPlotCols();
  }

  int n_plots = count();
  int rows = (n_plots - 1)/cols + 1;
  float width = 1.0/float(cols);
  float height = 1.0/float(rows);

  for (int i_plot = 0; i_plot < n_plots; i_plot++) {
    float ltx = width * float(i_plot%cols);
    float lty = height * float(i_plot/cols);
    at(i_plot)->setDims(width, height, ltx, lty);
  }

  setPlotCols(cols);
  return newplot;
}

/** The Plots */
KstPlotList KST::plotList;


