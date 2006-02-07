/***************************************************************************
                          kstplotlist.h  -  description
                             -------------------
    begin                : Sat Dec 3 2000
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

#ifndef KSTPLOTLIST_H
#define KSTPLOTLIST_H

#include <qptrlist.h>
#include <qstringlist.h>
#include "kstplot.h"

/**The list of files: a qtlist of kstplots, + purging
  *@author cbn
  */

class KstPlotList : public QPtrList<KstPlot>  {
public:
  KstPlotList();
  ~KstPlotList();
  KstPlot *FindKstPlot(const QString &tag);
  void arrangePlots(int cols);
  unsigned getPlotCols();
  void setPlotCols(unsigned in_cols);
  QStringList tagNames();
  QString generatePlotName();
  KstPlot* addPlot(const QString& name = QString::null, int cols = -1);
private:
  unsigned PlotCols;
};

namespace KST {
  /** The Plots: declared in kstplotlist.cpp */
  extern KstPlotList plotList;
}

#endif
