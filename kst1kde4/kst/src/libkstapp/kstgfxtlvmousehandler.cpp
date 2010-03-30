/***************************************************************************
                 kstgfxtlvmousehandler.cpp  -  description
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

#include "kst.h"
#include "kstdoc.h"
#include "kstgfxtlvmousehandler.h"
#include "kstgfxmousehandlerutils.h"

KstGfxTLVMouseHandler::KstGfxTLVMouseHandler()
: KstGfxMouseHandler() {
  // initial default settings before any sticky settings
  KstTopLevelViewPtr defaultView;

  defaultView = new KstTopLevelView;
// xxx  defaultView->setBackgroundColor(KstApp::inst()->paletteBackgroundColor());
  _defaultObject = KstViewObjectPtr(defaultView);
  _currentDefaultObject = KstViewObjectPtr(defaultView);
}


KstGfxTLVMouseHandler::~KstGfxTLVMouseHandler() {
}


void KstGfxTLVMouseHandler::applyDefaults(KstTopLevelViewPtr view) {
  copyDefaults(KstViewObjectPtr(view));
}


void KstGfxTLVMouseHandler::pressMove(KstTopLevelViewPtr view, const QPoint& pos, bool shift, const QRect& geom) {
  Q_UNUSED(view)
  Q_UNUSED(pos)
  Q_UNUSED(shift)
  Q_UNUSED(geom)
}


void KstGfxTLVMouseHandler::releasePress(KstTopLevelViewPtr view, const QPoint& pos, bool shift) {
  Q_UNUSED(view)
  Q_UNUSED(pos)
  Q_UNUSED(shift)
}

