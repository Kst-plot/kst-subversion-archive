/***************************************************************************
                 kstgfxpicturemousehandler.cpp  -  description
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

#include "kstgfxpicturemousehandler.h"
#include "kstgfxmousehandlerutils.h"
#include "kst.h"
#include "kstdoc.h"
#include "kstviewpicture.h"
#include "kstviewwidget.h"

KstGfxPictureMouseHandler::KstGfxPictureMouseHandler()
: KstGfxMouseHandler() {
  // initial default settings before any sticky settings
  KstViewPicturePtr defaultPicture;

  defaultPicture = new KstViewPicture;
  defaultPicture->setBorderWidth(0);
  defaultPicture->setBorderColor(Qt::black);
  defaultPicture->setRefreshTimer(0);
  defaultPicture->setMaintainAspect(true);
  _defaultObject = KstViewObjectPtr(defaultPicture);
  _currentDefaultObject = KstViewObjectPtr(defaultPicture);
}


KstGfxPictureMouseHandler::~KstGfxPictureMouseHandler() {
}


void KstGfxPictureMouseHandler::pressMove(KstTopLevelViewPtr view, const QPoint& pos, bool shift, const QRect& geom) {
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
      p.drawLine(old.topLeft(), old.bottomRight());
      p.drawLine(old.topRight(), old.bottomLeft());
    } 
    p.drawRect(_prevBand);
    p.drawLine(_prevBand.topRight(), _prevBand.bottomLeft());
    p.drawLine(_prevBand.topLeft(), _prevBand.bottomRight());
    p.end();
  }
}


void KstGfxPictureMouseHandler::releasePress(KstTopLevelViewPtr view, const QPoint& pos, bool shift) {
  Q_UNUSED(shift)

  if (!_mouseDown) {
    // if mouse was never down, pretend it wasn't released
    return;
  }
  _mouseDown = false;

  // once released, create a picture and popup the edit dialog
  if (!_cancelled && _mouseOrigin != pos) {
    KstViewPicturePtr picture;

    picture = new KstViewPicture;
    copyDefaults(KstViewObjectPtr(picture));
    picture->move(_prevBand.topLeft());
    picture->resize(_prevBand.size());
    if (picture->showDialog(view, true)) {
      KstViewObjectPtr container = view->findDeepestChild(_prevBand);
      if (!container) {
        container = view;
      }
      container->appendChild(KstViewObjectPtr(picture));
      container->invalidateClipRegion();
      KstApp::inst()->document()->setModified();
      KstApp::inst()->updateViewManager(true);
    }
  }
  _prevBand = QRect(-1, -1, 0, 0);
}

