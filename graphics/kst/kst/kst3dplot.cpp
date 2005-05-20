/***************************************************************************
                              kst3dplot.cpp
                             ---------------
    begin                : Mar 28, 2004
    copyright            : (C) 2004 The University of British Columbia
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

#include "kst3dplot.h"

#include <qpainter.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>


Kst3DPlot::Kst3DPlot() : KstPlotBase("Kst3DPlot") {
}


Kst3DPlot::Kst3DPlot(QDomElement& e) : KstPlotBase(e) {
}


Kst3DPlot::~Kst3DPlot() {
}


KstObject::UpdateType Kst3DPlot::update() {
  return NO_CHANGE;
}


void Kst3DPlot::save(QTextStream& ts) {
}


void Kst3DPlot::paint(KstPaintType type, QPainter& p) {
  KstPlotBase::paint(type, p);
}


bool Kst3DPlot::popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  _standardActions |= Delete;

  KstViewObject::popupMenu(menu, pos, topLevelParent);
  return true;
}


bool Kst3DPlot::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  _layoutActions |= Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename;

  KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
  return true;
}


KstViewObjectPtr create_Kst3DPlot() {
  return KstViewObjectPtr(new Kst3DPlot);
}


KstViewObjectFactoryMethod Kst3DPlot::factory() const {
  return &create_Kst3DPlot;
}

#include "kst3dplot.moc"
// vim: ts=2 sw=2 et
