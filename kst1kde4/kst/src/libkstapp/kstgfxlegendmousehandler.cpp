/***************************************************************************
                 kstgfxlegendmousehandler.cpp  -  description
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by University of British Columbia
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

#include <stdlib.h>

#include <QPainter>

#include "kstgfxlegendmousehandler.h"
#include "kstgfxmousehandlerutils.h"
#include "kstsettings.h"
#include "kst.h"
#include "kstdoc.h"
#include "kstviewlegend.h"
#include "kstviewwidget.h"

KstGfxLegendMouseHandler::KstGfxLegendMouseHandler()
: KstGfxMouseHandler() {
  // initial default settings before any sticky settings
  KstViewLegendPtr defaultLegend;

  defaultLegend = new KstViewLegend;
  _defaultObject = KstViewObjectPtr(defaultLegend);
  _currentDefaultObject = KstViewObjectPtr(defaultLegend); 
}


KstGfxLegendMouseHandler::~KstGfxLegendMouseHandler() {
}


void KstGfxLegendMouseHandler::pressMove(KstTopLevelViewPtr view, const QPoint& pos, bool shift, const QRect& geom) {
  if (_cancelled || !_mouseDown) {
    return;
  }

  QRect old = _prevBand;

  _prevBand = KstGfxMouseHandlerUtils::newRect(pos, _mouseOrigin, geom, shift);

  if (old != _prevBand) {
    QPainter p;
    p.begin(view->widget());
    p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
// xxx    p.setRasterOp(Qt::NotROP);
    if (old.topLeft() != QPoint(-1, -1)) {
      p.drawRect(old);
    } 
    p.drawRect(_prevBand);
    p.end();
  }
}


void KstGfxLegendMouseHandler::releasePress(KstTopLevelViewPtr view, const QPoint& pos, bool shift) {
  Q_UNUSED(shift)

  if (!_mouseDown) {
    // if mouse was never down, pretend it wasn't released
    return;
  }
  _mouseDown = false;

  // once released, create a new text object and popup the edit dialog
  if (!_cancelled) {
    QSize size(0,0);
    KstViewLegendPtr legend;

    legend = new KstViewLegend;
    copyDefaults(KstViewObjectPtr(legend));

    if (_mouseOrigin != pos) {
      legend->move(_prevBand.topLeft());
      size = _prevBand.size();
    } else {
      legend->move(pos);
    }

    bool wasClick = size == QSize(0, 0);

    if (size.width() < 3) {
      size.setWidth(3);
    }
    if (size.height() < 3) {
      size.setHeight(3);
    }
    legend->resize(size);

    if (legend->showDialog(view, true)) {
      KstViewObjectPtr container;

      if (wasClick) {
        container = view->findDeepestChild(pos);
      } else {
        container = view->findDeepestChild(_prevBand);
      }
      if (!container) {
        container = view;
      }
      if (wasClick) {
        legend->adjustSizeForText(container->contentsRect());
      }
      container->appendChild(KstViewObjectPtr(legend));
      container->invalidateClipRegion();
      KstApp::inst()->document()->setModified();
      KstApp::inst()->updateViewManager(true);
    }
  }
  _prevBand = QRect(-1, -1, 0, 0);
}
