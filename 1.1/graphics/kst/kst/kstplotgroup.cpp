/***************************************************************************
                              kstplotgroup.cpp
                             -------------------
    begin                : Mar 21, 2004
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

#include <assert.h>

// include files for Qt

// include files for KDE
#include <kdebug.h>
#include <klocale.h>

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "kstplotgroup.h"
#include "ksttimers.h"

static int pgcount = 0;

KstPlotGroup::KstPlotGroup() : KstMetaPlot("KstPlotGroup") {
  _standardActions |= Delete | Raise | Zoom | Lower | RaiseToTop | LowerToBottom;
  _layoutActions |= Delete | Copy | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo;
  setTagName(i18n("Plot Group %1").arg(++pgcount));
  _type = "plotgroup";
  setBorderColor(Qt::blue);
}


KstPlotGroup::KstPlotGroup(const QDomElement& e) : KstMetaPlot(e) {
  _standardActions |= Delete | Raise | Zoom | Lower | RaiseToTop | LowerToBottom;
  _layoutActions |= Delete | Copy | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo;
  setTagName(i18n("Plot Group %1").arg(++pgcount));
  _type = "plotgroup";
  setBorderColor(Qt::blue);
}


KstPlotGroup::KstPlotGroup(const KstPlotGroup& plotGroup) : KstMetaPlot(plotGroup) {
  _type = "plotgroup";

  setTagName(i18n("Plot Group %1").arg(++pgcount));
}


KstPlotGroup::~KstPlotGroup() {
}


void KstPlotGroup::setHasFocus(bool hasFocus) {
  _hasFocus = hasFocus;
  forEachChild<bool>(&KstViewObject::setHasFocus, hasFocus);
}


void KstPlotGroup::removeFocus(QPainter& p) {
  forEachChild<QPainter&>(&KstViewObject::removeFocus, p);
}


void KstPlotGroup::copyObject() {
  KstApp::inst()->document()->setModified();
  if (_parent) {
    _parent->appendChild(new KstPlotGroup(*this), true);
  }
  QTimer::singleShot(0, KstApp::inst(), SLOT(updateDialogs()));
}


void KstPlotGroup::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  parent.appendChild(new KstPlotGroup(*this), true);
}


bool KstPlotGroup::removeChild(KstViewObjectPtr obj, bool recursive) {
  if (KstViewObject::removeChild(obj, recursive)) {
    if (_children.count() > 1) {
      QRect gg = _children.first()->geometry();
      for (KstViewObjectList::Iterator it = _children.begin(); it != _children.end(); ++it) {
        gg |= (*it)->geometry();
      }

      _geom = gg;
      updateAspect();
      for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
        updateAspect();
      }
    } else {
      if (_parent) { // can be false if we are being deleted already
        flatten();
      }
    }

    return true;
  }

  return false;
}


void KstPlotGroup::flatten() {
  assert(_parent);
  // FIXME: remove dependency on _parent
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->setSelected(false);
    (*i)->setFocus(false);
    _parent->insertChildAfter(KstViewObjectPtr(this), *i);
  }
  _parent->removeChild(this);
  KstApp::inst()->document()->setModified();
}


void KstPlotGroup::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstMetaPlot::save(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


void KstPlotGroup::saveTag(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstMetaPlot::saveTag(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


void KstPlotGroup::paint(KstPaintType type, QPainter& p) {
  QRegion clipRegion(geometry());
  QBrush brush(_backgroundColor);
  bool hadClipping = p.hasClipping();
  QRegion oldRegion = p.clipRegion();

  if (KstApp::inst()->getZoomRadio() == KstApp::LAYOUT) {
    setBorderWidth(3);
  } else {
    setBorderWidth(0);
  }

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    clipRegion -= QRegion((*i)->geometry());
  }
  p.setClipRegion(clipRegion);
  p.fillRect(geometry(), brush);

  p.setClipRegion(oldRegion);
  p.setClipping(hadClipping);

#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  KstMetaPlot::paint(type, p);
#ifdef BENCHMARK
  int x = t.elapsed();
  kdDebug() << "    -> metaplot parent took " << x << "ms" << endl;
#endif
  p.setClipping(false);
}


bool KstPlotGroup::popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  KstMetaPlot::popupMenu(menu, pos, topParent);
  KstViewObjectPtr c = findChild(pos + position());
  if (c) {
    KPopupMenu *s = new KPopupMenu(menu);
    if (c->popupMenu(s, pos - c->position(), topParent)) {
      menu->insertItem(c->tagName(), s);
    } else {
      delete s;
    }
  }
  return true;
}


bool KstPlotGroup::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  KstMetaPlot::layoutPopupMenu(menu, pos, topParent);
  menu->insertItem(i18n("&Ungroup"), this, SLOT(flatten()));
  KstViewObjectPtr c = findChild(pos + position());
  if (c) {
    KPopupMenu *s = new KPopupMenu(menu);
    if (c->layoutPopupMenu(s, pos - c->position(), topParent)) {
      menu->insertItem(c->tagName(), s);
    } else {
      delete s;
    }
  }
  return true;
}


KstViewObjectPtr create_KstPlotGroup() {
  return KstViewObjectPtr(new KstPlotGroup);
}


KstViewObjectFactoryMethod KstPlotGroup::factory() const {
  return &create_KstPlotGroup;
}

#include "kstplotgroup.moc"
// vim: ts=2 sw=2 et
