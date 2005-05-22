/***************************************************************************
                   kstviewobject.cpp: base class for view objects
                             -------------------
    begin                : Mar 11, 2004
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
#include <math.h>
#include <stdlib.h>

// include files for Qt
#include <qdeepcopy.h>
#include <qstylesheet.h>

// include files for KDE
#include <kdatastream.h>
#include <kdebug.h>

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "kstobject.h"
#include "kstplotgroup.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewobject.h"
#include "kstviewobjectfactory.h"
#include "kstviewwindow.h"

KstViewObject::KstViewObject(const QString& type) : KstObject(), _geom(0, 0, 1, 1), _type(type) {
  _parent = 0L;
  _standardActions = 0;
  _layoutActions = 0;
  _maximized = false;
  updateAspect();
  _onGrid = false;
  _columns = 0;
  _focus = false;
  _selected = false;
  _foregroundColor = KstSettings::globalSettings()->foregroundColor;
  _backgroundColor = KstSettings::globalSettings()->backgroundColor;
  setMinimumSize(QSize(1, 1));
}


KstViewObject::KstViewObject(const QDomElement& e) : KstObject() {
  _foregroundColor = KstSettings::globalSettings()->foregroundColor;
  _backgroundColor = KstSettings::globalSettings()->backgroundColor;
  _parent = 0L;
  setMinimumSize(QSize(1, 1));
  load(e);
}


KstViewObject::KstViewObject(const KstViewObject& viewObject) : KstObject() {
  _parent = 0L;
  _foregroundColor = viewObject._foregroundColor;
  _backgroundColor = viewObject._backgroundColor;
  _aspect = viewObject._aspect;
  _standardActions = viewObject._standardActions;
  _layoutActions = viewObject._layoutActions;
  _maximized = false;
  _onGrid = viewObject._onGrid;
  _columns = viewObject._columns;
  _focus = false;
  _selected = false;
  _geom = viewObject._geom;
  setMinimumSize(QSize(1, 1));

  setContentsRect(viewObject.contentsRect());

  for (KstViewObjectList::ConstIterator i = viewObject._children.begin(); i != viewObject._children.end(); ++i) {
    (*i)->copyObjectQuietly(*this);
  }
}


void KstViewObject::load(const QDomElement& e) {
  _children.clear();
  _standardActions = 0;
  _layoutActions = 0;
  _maximized = false;
  _onGrid = false;
  _columns = 0;
  _focus = false;
  _selected = false;
  _geom = QRect(0, 0, 1, 1);
  updateAspect();

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); // try to convert the node to an element.
    if (!el.isNull()) { // the node was really an element.
      if (el.tagName() == "tag") {
        setTagName(el.text());
      } else if (el.tagName() == "columns") {
        _onGrid = true;
        _columns = el.text().toInt();
      } else if (el.tagName() == "aspect") {
        _aspect.x = el.attribute("x", "0.0").toDouble();
        _aspect.y = el.attribute("y", "0.0").toDouble();
        _aspect.w = el.attribute("w", "1.0").toDouble();
        _aspect.h = el.attribute("h", "1.0").toDouble();
      }
    }
    n = n.nextSibling();
  }
}


KstViewObject::~KstViewObject() {
  _parent = 0L;
}


void KstViewObject::loadChildren(const QDomElement& e) {
  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement el = n.toElement();
    if (!el.isNull()) {
      if (el.tagName() == "plot") {
        QString in_tag = el.text();
        if (KstApp::inst()->plotHolderWhileOpeningDocument().count(in_tag) > 0) {
          Kst2DPlotPtr plot = KstApp::inst()->plotHolderWhileOpeningDocument()[in_tag];
          if (plot) {
            appendChild(plot.data(), true);
            plot->loadChildren(el);
            KstApp::inst()->plotHolderWhileOpeningDocument().erase(in_tag);
          }
        }
      } else if (el.tagName() == "plotgroup") {
        KstPlotGroupPtr plotGroup = new KstPlotGroup(el);
        appendChild(plotGroup.data(), true);
        plotGroup->loadChildren(el);
      }
    }
    n = n.nextSibling();
  }
}


KstObject::UpdateType KstViewObject::update(int counter) {
  if (checkUpdateCounter(counter)) {
    return lastUpdateResult();
  }

  KstObject::UpdateType rc = NO_CHANGE;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if (rc == NO_CHANGE) {
      rc = (*i)->update(counter);
    } else {
      (*i)->update(counter);
    }
  }

  return setLastUpdateResult(rc);
}


void KstViewObject::save(QTextStream& ts, const QString& indent) {
  KstAspectRatio aspect;

  if (_maximized) {
    aspect = _aspectOldZoomedObject;
  } else {
    aspect = _aspect;
  }

  ts << indent << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << indent << "<aspect x=\"" << aspect.x <<
    "\" y=\"" << aspect.y <<
    "\" w=\"" << aspect.w <<
    "\" h=\"" << aspect.h << "\" />" << endl;

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->save(ts, indent + "  ");
  }
}


void KstViewObject::saveTag(QTextStream& ts, const QString& indent) {
  KstAspectRatio aspect;

  if (_maximized) {
    aspect = _aspectOldZoomedObject;
  } else {
    aspect = _aspect;
  }

  ts << indent << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << indent << "<aspect x=\"" << aspect.x <<
    "\" y=\"" << aspect.y <<
    "\" w=\"" << aspect.w <<
    "\" h=\"" << aspect.h << "\" />" << endl;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->saveTag(ts, indent + "  ");
  }
}


void KstViewObject::paint(KstPaintType type, QPainter& p) {
  bool hadClipping = p.hasClipping();
  QRegion oldRegion = p.clipRegion();
  bool maximized = false;

  // handle the case where we have maximized plots
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->_maximized) {
      (*i)->_lastClipRegion = (*i)->geometry();
      (*i)->paint(type, p);
      maximized = true;
      break;
    }
  }

  if (!maximized) {
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
#ifdef BENCHMARK
      QTime t;
      t.start();
#endif
      QRegion clipRegion = oldRegion;
      KstViewObjectList::Iterator j = i;
      for (++j; j != _children.end(); ++j) {
        clipRegion -= QRegion((*j)->geometry());
      }
      (*i)->_lastClipRegion = clipRegion;
      p.setClipRegion(clipRegion);
      (*i)->paint(type, p);
#ifdef BENCHMARK
      int x = t.elapsed();
      kdDebug() << "   -> object " << (*i)->tagName() << " took " << x << "ms" << endl;
#endif
      if ((*i)->_maximized) {
        break;
      }
    }
  }

  p.setClipRegion(oldRegion);

  if (_focus) {
    //kdDebug() << "Object " << tagName() << " has focus" << endl;
    drawFocusRect(p);
  }

  if (isSelected()) {
    //kdDebug() << "Object " << tagName() << " is selected" << endl;
    drawSelectRect(p);
  }

  p.setClipping(hadClipping);
}


void KstViewObject::drawFocusRect(QPainter& p) {
  QPen pen(Qt::black, 2);
  p.setPen(pen);
  QRect r;
  r.setX(_geom.left());
  r.setY(_geom.top());
  r.setWidth(_geom.width());
  r.setHeight(_geom.height());
  Qt::RasterOp op = p.rasterOp();
  // FIXME: Qt bug?  Appears that clipping is ignored here
  p.setRasterOp(Qt::NotROP);
  p.drawWinFocusRect(r);
  p.setRasterOp(op);
}


void KstViewObject::drawSelectRect(QPainter& p) {
  QPen pen(Qt::black, 2);
  p.setPen(pen);
  QRect r;
  r.setX(_geom.left() + 2);
  r.setY(_geom.top() + 2);
  r.setWidth(_geom.width() - 4);
  r.setHeight(_geom.height() - 4);
  Qt::RasterOp op = p.rasterOp();
  p.setRasterOp(Qt::CopyROP);
  p.drawRect(r);
  p.setRasterOp(op);
}


void KstViewObject::appendChild(KstViewObjectPtr obj, bool keepAspect) {
  obj->_parent = this;
  _children.append(obj);

  for (KstViewObjectList::Iterator i = children().begin(); i != children().end(); ++i) {
    if ((*i)->maximized()) {
      (*i)->setMaximized(false);
    }
  }

  if (keepAspect) {
    obj->updateFromAspect();
  } else {
    obj->updateAspect();
  }
}


void KstViewObject::prependChild(KstViewObjectPtr obj, bool keepAspect) {
  obj->_parent = this;
  _children.prepend(obj);

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->maximized()) {
      (*i)->setMaximized(false);
    }
  }

  if (keepAspect) {
    obj->updateFromAspect();
  } else {
    obj->updateAspect();
  }
}


bool KstViewObject::removeChild(KstViewObjectPtr obj, bool recursive) {
  bool rc = _children.remove(obj) > 0;
  if (recursive) {
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      rc = rc && (*i)->removeChild(obj, true);
    }
  }

  obj->_parent = 0L;
  return rc;
}


void KstViewObject::insertChildAfter(const KstViewObjectPtr after, KstViewObjectPtr obj, bool keepAspect) {
  KstViewObjectList::Iterator i = _children.find(after);
  if (i != _children.end()) {
    _children.insert(i, obj);
  } else {
    _children.prepend(obj);
  }
  obj->_parent = this;

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->maximized()) {
      (*i)->setMaximized(false);
    }
  }

  if (keepAspect) {
    obj->updateFromAspect();
  } else {
    obj->updateAspect();
  }
}


void KstViewObject::clearChildren() {
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->_parent = 0L;
  }
  _children.clear();
}


const KstViewObjectList& KstViewObject::children() const {
  return _children;
}


KstViewObjectList& KstViewObject::children() {
  return _children;
}


void KstViewObject::resize(const QSize& size) {
  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));
  _geom.setSize(size.expandedTo(_minimumSize));
  updateAspect();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResized();
  }
}


void KstViewObject::resizeForPrint(const QSize& size) {
  _geomOld = _geom;
  _geom.setSize(size);
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResizedForPrint();
  }
}


void KstViewObject::revertForPrint() {
  _geom = _geomOld;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentRevertedForPrint();
  }
}


void KstViewObject::resizeFromAspect(double x, double y, double w, double h) {
  _aspect.x = x;
  _aspect.y = y;
  _aspect.w = w;
  _aspect.h = h;
  updateFromAspect();
}


QSize KstViewObject::size() const {
  return _geom.size();
}


void KstViewObject::internalAlignment(KstPaintType type, QPainter& p, QRect& plotRegion) {
  Q_UNUSED(type)
  Q_UNUSED(p)

  plotRegion.setLeft(0);
  plotRegion.setRight(0);
  plotRegion.setTop(0);
  plotRegion.setLeft(0);
}


void KstViewObject::parentResized() {
  updateFromAspect();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResized();
  }
}


void KstViewObject::parentResizedForPrint() {
  _geomOld = _geom;
  updateFromAspect();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResizedForPrint();
  }
}


void KstViewObject::parentRevertedForPrint() {
  _geom = _geomOld;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentRevertedForPrint();
  }
}


void KstViewObject::setBackgroundColor(const QColor& color) {
  _backgroundColor = color;
}


QColor KstViewObject::backgroundColor() const {
  return _backgroundColor;
}


void KstViewObject::setForegroundColor(const QColor& color) {
  _foregroundColor = color;
}


QColor KstViewObject::foregroundColor() const {
  return _foregroundColor;
}


int KstViewObject::columns() const {
  return _columns;
}


void KstViewObject::setColumns(int cols) {
  _columns = cols;
}


bool KstViewObject::onGrid() const {
  return _onGrid;
}


void KstViewObject::setOnGrid(bool on_grid) {
  _onGrid = on_grid;
}


void KstViewObject::cleanup(int cols) {
  if (_children.count() < 1) {
    return;
  }

  // FIXME: don't allow regrid to a number of columns that will result in
  //        >= height() plots in a column
  if (!_onGrid) {
    if (cols <= 0) {
      cols = int(sqrt(_children.count()));
    }
    _onGrid = true;
    _columns = QMAX(1, cols);
  } else {
    if (cols > 0) {
      _columns = cols;
    } else if (_columns <= 0) {
      _columns = QMAX(1, int(sqrt(_children.count())));
    }
  }

  double minDistance = 0.0;
  int pos = 0;
  int x = 0;
  int y = 0;
  int w = _geom.width() / _columns;
  int lastRow = _children.count() / _columns;
  int h = _geom.height() / (lastRow + (_children.count() % _columns > 0 ? 1 : 0));

  KstViewObjectList childrenCopy = QDeepCopy<KstViewObjectList>(_children);

  //kdDebug() << "cleanup with w=" << w << " and h=" << h << endl;
  //kdDebug() << "columns=" << _columns  << endl;
  for (unsigned i = 0; i < _children.count(); ++i) {
    KstViewObjectList::Iterator nearest = childrenCopy.end();
    QPoint pt(x, y);
    QSize sz(w, h);

    // adjust the last column to be sure that we don't spill over
    if (pos % _columns == _columns - 1) {
      sz.setWidth(_geom.width() - x);
    }

    // adjust the last row to be sure that we don't spill over
    if ((pos + 1) / _columns > lastRow) {
      sz.setHeight(_geom.height() - y);
    }

    for (KstViewObjectList::Iterator it = childrenCopy.begin(); it != childrenCopy.end(); ++it) {
      // find plot closest to the desired position, based on top-left corner...
      double distance = double((x - (*it)->geometry().x()) * (x - (*it)->geometry().x())) + 
                        double((y - (*it)->geometry().y()) * (y - (*it)->geometry().y()));
      if (it == childrenCopy.begin() || distance < minDistance) {
        minDistance = distance;
        nearest = it;
      }
    }

    if (nearest != childrenCopy.end()) {
      KstViewObjectPtr vop = *nearest;
      vop->move(pt);
      vop->resize(sz);
      childrenCopy.remove(vop);
    }

    if (++pos % _columns == 0) {
      x = 0;
      y += h;
    } else {
      x += w;
    }
  }
}


QPoint KstViewObject::position() const {
  return _geom.topLeft();
}


void KstViewObject::parentMoved(const QPoint& offset) {
  updateFromAspect();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentMoved(offset);
  }
}


void KstViewObject::move(const QPoint& pos) {
  QPoint offset = pos - _geom.topLeft();

  if (!offset.isNull()) {
    _geom.moveTopLeft(pos);
    updateAspectPos( );
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      (*i)->parentMoved(offset);
    }
  }
}


void KstViewObject::setFocus(bool focus) {
  _focus = focus;
}


bool KstViewObject::focused() const {
  return _focus;
}


void KstViewObject::recursively(void (KstViewObject::*method)(), bool self) {
  if (self) {
    (this->*method)();
  }
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->recursively(method, true);
  }
}


KstViewObjectPtr KstViewObject::findChild(const QString& name, bool recursive) {
  if (tagName() == name || _children.isEmpty()) {
    return KstViewObjectPtr();
  }

  KstViewObjectList::Iterator i = _children.end();
  KstViewObjectPtr rc;
  for (--i; ; --i) {
    if ((*i)->tagName() == name) {
      return *i;
    }
    if (recursive) {
      rc = (*i)->findChild(name, recursive);
      if (rc) {
        return rc;
      }
    }
    if (i == _children.begin()) {
      break;
    }
  }

  return KstViewObjectPtr();
}


KstViewObjectPtr KstViewObject::findChild(const QPoint& pos) {
  KstViewObjectPtr obj;

  if (!_geom.contains(pos) || _children.isEmpty()) {
    return obj;
  }

  KstViewObjectList::Iterator i = _children.end();
  for (--i; ; --i) {
    if (QRect((*i)->position(), (*i)->size()).contains(pos)) {
      if (obj == 0L || (*i)->_maximized) {
        obj = *i;
        if ((*i)->_maximized) {
          break;
        }
      }
    }
    if (i == _children.begin()) {
      break;
    }
  }

  return obj;
}


const QRect& KstViewObject::geometry() const {
  return _geom;
}


QRect KstViewObject::contentsRect() const {
  return _geom;
}


void KstViewObject::setContentsRect(const QRect& rect) {
  _geom = rect;
}


QString KstViewObject::menuTitle() const {
  return tagName();
}


bool KstViewObject::popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  Q_UNUSED(pos)
  Q_UNUSED(topParent)
  bool rc = false;
  int id;
  QString menuTitle = this->menuTitle();

  if (!menuTitle.isEmpty()) {
    menu->insertTitle(menuTitle);
  }

  if (_standardActions & Edit) {
    menu->insertItem(i18n("&Edit..."), this, SLOT(edit()));
    rc = true;
  }

  if (_standardActions & Delete) {
    menu->insertItem(i18n("&Delete"), this, SLOT(deleteObject()));
    rc = true;
  }

  if (_standardActions & Copy) {
    menu->insertItem(i18n("&Copy"), this, SLOT(copyObject()));
    rc = true;
  }

  if (_standardActions & Zoom) {
    id = menu->insertItem(i18n("Maximi&ze"), this, SLOT(zoomToggle()));
    if (_maximized) {
      menu->setItemChecked(id,true);
    }
    rc = true;
  }

  if (_standardActions & Pause) {
    id = menu->insertItem(i18n("&Pause"), this, SLOT(pauseToggle()));
    if (KstApp::inst()->paused()) {
      menu->setItemChecked(id, true);
    }
    rc = true;
  }

  return rc;
}


bool KstViewObject::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  Q_UNUSED(pos)

  bool rc = false;
  int id;

  _moveToMap.clear();

  if (!tagName().isEmpty()) {
    menu->insertTitle(tagName());
  }

  if (_layoutActions & Delete) {
    menu->insertItem(i18n("&Delete"), this, SLOT(deleteObject()));
    rc = true;
  }

  if (_layoutActions & Copy) {
    menu->insertItem(i18n("&Copy"), this, SLOT(copyObject()));
    rc = true;
  }

  if (_layoutActions & Raise) {
    menu->insertItem(i18n("&Raise"), this, SLOT(raise()));
    rc = true;
  }

  if (_layoutActions & Lower) {
    menu->insertItem(i18n("&Lower"), this, SLOT(lower()));
    rc = true;
  }

  if (_layoutActions & RaiseToTop) {
    menu->insertItem(i18n("Raise to &Top"), this, SLOT(raiseToTop()));
    rc = true;
  }

  if (_layoutActions & LowerToBottom) {
    menu->insertItem(i18n("Lower to &Bottom"), this, SLOT(lowerToBottom()));
    rc = true;
  }

  if (_layoutActions & Rename) {
    menu->insertItem(i18n("Re&name..."), this, SLOT(rename()));
    rc = true;
  }

  if (_layoutActions & Edit) {
    menu->insertItem(i18n("&Edit..."), this, SLOT(edit()));
    rc = true;
  }

  if (_layoutActions & MoveTo) {
    int i = 0;
    bool hasEntry = false;

    KPopupMenu *submenu = new KPopupMenu(menu);

    id = menu->insertItem(i18n("&Move To"), submenu);

    KMdiIterator<KMdiChildView*> *it = KstApp::inst()->createIterator();
    while (it->currentItem()) {
      KstViewWindow *c = dynamic_cast<KstViewWindow*>(it->currentItem());
      KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(topParent);

      if (c && (!tlv || tlv != c->view())) {
        hasEntry = true;
        submenu->insertItem(it->currentItem()->caption(), i);
        submenu->connectItem(i, this, SLOT(moveTo(int)));
        _moveToMap[i] = it->currentItem()->caption();
        i++;
      }
      it->next();
    }
    KstApp::inst()->deleteIterator(it);

    menu->setItemEnabled(id, hasEntry);

    rc = true;
  }

  if (_layoutActions & CopyTo) {
    int i = 0;
    bool hasEntry = false;

    KPopupMenu *submenu = new KPopupMenu(menu);

    id = menu->insertItem(i18n("&Copy To"), submenu);

    KMdiIterator<KMdiChildView*> *it = KstApp::inst()->createIterator();
    while (it->currentItem()) {
      KstViewWindow *c = dynamic_cast<KstViewWindow*>(it->currentItem());
      KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(topParent);

      if (c) {
        hasEntry = true;
        if (tlv && tlv == c->view()) {
          submenu->insertItem(i18n("%1 (here)").arg(it->currentItem()->caption()), i);
        } else {
          submenu->insertItem(it->currentItem()->caption(), i);
        }
        submenu->connectItem(i, this, SLOT(copyTo(int)));
        _copyToMap[i] = it->currentItem()->caption();
        i++;
      }
      it->next();
    }
    KstApp::inst()->deleteIterator(it);

    menu->setItemEnabled(id, hasEntry);

    rc = true;
  }
    
  return rc;
}


void KstViewObject::deleteObject() {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  KstViewObjectPtr vop(this);
  if (_parent) {
    _parent->removeChild(this);
    _parent = 0L;
  }
  while (!_children.isEmpty()) {
    removeChild(_children.first());
  }
  vop = 0L; // basically "delete this;"
  QTimer::singleShot(0, KstApp::inst(), SLOT(updateDialogs()));
}


void KstViewObject::copyObject() {
}


void KstViewObject::copyObjectQuietly(KstViewObject &parent, const QString& name) const {
  Q_UNUSED(parent)
  Q_UNUSED(name)
}


void KstViewObject::raiseToTop() {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      _parent->_children.remove(it);
      _parent->_children.append(t);
    }
  }
}


void KstViewObject::lowerToBottom() {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      _parent->_children.remove(it);
      _parent->_children.prepend(t);
    }
  }
}


void KstViewObject::raise() {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      it = _parent->_children.remove(it);
      ++it;
      if (it != _parent->_children.end()) {
        _parent->_children.insert(it, t);
      } else {
        _parent->_children.append(t);
      }
    }
  }
}


void KstViewObject::lower() {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      it = _parent->_children.remove(it);
      if (!_parent->_children.isEmpty() && it != _parent->_children.begin()) {
        --it;
        _parent->_children.insert(it, t);
      } else {
        _parent->_children.prepend(t);
      }
    }
  }
}


void KstViewObject::moveTo(int id) {
  // FIXME: eliminate _parent!!
  KstApp::inst()->document()->setModified();
  QString windowName = _moveToMap[id];
  KMdiChildView *mdiChild;
  KstViewWindow *window;

  if (!windowName.isEmpty()) {
    mdiChild = KstApp::inst()->findWindow(windowName);
    if (mdiChild) {
      window = dynamic_cast<KstViewWindow*>(mdiChild);
      if (window) {
        if (_parent) {
          KstViewObjectPtr t = this;
          KstViewObjectList::Iterator it = _parent->_children.find(t);

          if (it != _parent->_children.end()) {
            _parent->_children.remove(it);

            window->view()->appendChild(t, true);
            window->view()->paint(P_PAINT);
          }
        }
      }
    }
  }
}


void KstViewObject::copyTo(int id) {
  KstApp::inst()->document()->setModified();
  QString windowName = _copyToMap[id];
  KMdiChildView *mdiChild;
  KstViewWindow *window;

  if (!windowName.isEmpty()) {
    mdiChild = KstApp::inst()->findWindow(windowName);
    if (mdiChild) {
      window = dynamic_cast<KstViewWindow*>(mdiChild);
      if (window) {
        copyObjectQuietly(*(window->view().data()));
      }
    }
  }
}


void KstViewObject::updateFromAspect() {
  // FIXME: eliminate _parent!!
  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));
  if (_parent) {
    _geom.setLeft(_parent->geometry().left() + int(_aspect.x * _parent->geometry().width()));
    _geom.setTop(_parent->geometry().top() + int(_aspect.y * _parent->geometry().height()));
    _geom.setRight(_parent->geometry().left() + int((_aspect.x + _aspect.w) * _parent->geometry().width()) - 1);
    _geom.setBottom(_parent->geometry().top() + int((_aspect.y + _aspect.h) * _parent->geometry().height()) - 1);
  }
  if (_geom.width() >= _minimumSize.width() || _geom.height() >= _minimumSize.height()) {
    _geom.setSize(_geom.size().expandedTo(_minimumSize));
  }
  assert(_geom.left() >= 0 && _geom.top() >= 0);
}


void KstViewObject::updateAspectPos() {
  // FIXME: eliminate _parent!!
  if (_parent) {
    _aspect.x = double(geometry().left() - _parent->geometry().left()) / double(_parent->geometry().width());
    _aspect.y = double(geometry().top() - _parent->geometry().top()) / double(_parent->geometry().height());
  } else {
    _aspect.x = 0.0;
    _aspect.y = 0.0;
  }
}


void KstViewObject::updateAspectSize() {
  // FIXME: eliminate _parent!!
  if (_parent) {
    _aspect.w = double(geometry().width()) / double(_parent->geometry().width());
    _aspect.h = double(geometry().height()) / double(_parent->geometry().height());
  } else {
    _aspect.w = 0.0;
    _aspect.h = 0.0;
  }
}


void KstViewObject::updateAspect() {
  updateAspectSize();
  updateAspectPos();
}


void KstViewObject::updateSelection(const QRect& region) {
  setSelected(region.contains(_geom.center()));
}


bool KstViewObject::isSelected() const {
  return _selected;
}


void KstViewObject::setSelected(bool selected) {
  _selected = selected;
}


bool KstViewObject::maximized() const {
  return _maximized;
}


void KstViewObject::setMaximized(bool maximized) {
  if (_maximized != maximized) {
    zoomToggle();
  }
}


void KstViewObject::zoomToggle() {
  if (_maximized) {
    _maximized = false;
    _aspect = _aspectOldZoomedObject;
    if (_parent && _parent->_maximized) {
      _parent->zoomToggle();
    }
    updateFromAspect();
    setOnGrid(_prevOnGrid);
  } else {
    _maximized = true;
    _aspectOldZoomedObject = _aspect;
    if (_parent && !_parent->_maximized) {
      _parent->zoomToggle();
    }
    resizeFromAspect(0.0, 0.0, 1.0, 1.0);
    _prevOnGrid = onGrid();
    setOnGrid(false);
  }

  for (KstViewObjectList::Iterator it = _children.begin(); it != _children.end(); ++it) {
    (*it)->parentResized();
  }
}


void KstViewObject::recursivelyQuery(bool (KstViewObject::*method)() const, KstViewObjectList& list, bool matchRecurse) {
  bool has = (this->*method)();
  if (has) {
    list.append(this);
  }

  if (!has || (has && matchRecurse)) {
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      (*i)->recursivelyQuery(method, list, matchRecurse);
    }
  }
}


void KstViewObject::detach() {
  if (_parent) {
    _parent->removeChild(this);
    _parent = 0L;
  }
}


void KstViewObject::rename() {
  bool ok = false;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  QString newName = KInputDialog::getText(i18n("Kst"), i18n("Enter a new name for %1:").arg(tagName()), tagName(), &ok);
#else
  QString newName = KLineEditDlg::getText(i18n("Enter a new name for %1:").arg(tagName()), tagName(), &ok, 0L);
#endif
  if (ok) {
    setTagName(newName);
  }
}


KstViewObjectFactoryMethod KstViewObject::factory() const {
  return 0L;
}


QDataStream& operator<<(QDataStream& str, KstViewObjectPtr obj) {
  obj->writeBinary(str);
  return str;
}


void KstViewObject::writeBinary(QDataStream& str) {
  // FIXME: this must be done at startup if we want to be able
  //        to drag between processes
  assert(factory());
  KstViewObjectFactory::self()->registerType(this, factory());

  str << KstViewObjectFactory::self()->typeOf(this);
  str << tagName();
  str << _geom << _backgroundColor << _foregroundColor;
  // _parent should not be sent since it is meaningless in a drag context
  str << _standardActions << _layoutActions << _aspect;

  str << _children.count();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    str << *i;
  }
}


QDataStream& operator>>(QDataStream& str, KstViewObjectPtr obj) {
  obj->readBinary(str);
  return str;
}


void KstViewObject::readBinary(QDataStream& str) {
  QString tagName;
  str >> tagName;
  setTagName(tagName);
  kdDebug() << "Decoding " << tagName << " from drag." << endl;
  // FIXME: rename objects if they cause a namespace conflict
  str >> _geom >> _backgroundColor >> _foregroundColor;
  str >> _standardActions >> _layoutActions >> _aspect;

  _children.clear();
  uint cc = 0;
  str >> cc;
  for (uint i = 0; i < cc; ++i) {
    QString type;
    str >> type;
    KstViewObjectPtr o = KstViewObjectFactory::self()->createA(type);
    if (o.data()) {
      str >> o;
      appendChild(o, true);
    } else {
      abort();
      // FIXME: can't decode this one!  How to recover?
    }
  }
}


void KstViewObject::pauseToggle() {
  KstApp::inst()->togglePaused();
}


bool KstViewObject::mouseHandler() const {
  return false;
}


void KstViewObject::setHasFocus(bool hasFocus) {
  _hasFocus = hasFocus;
}


void KstViewObject::removeFocus(QPainter& p) {
  Q_UNUSED(p)
}


void KstViewObject::mouseMoveEvent(QWidget *view, QMouseEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::mousePressEvent(QWidget *view, QMouseEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::mouseDoubleClickEvent(QWidget *view, QMouseEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::mouseReleaseEvent(QWidget *view, QMouseEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::keyPressEvent(QWidget *view, QKeyEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::keyReleaseEvent(QWidget *view, QKeyEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::dragMoveEvent(QWidget *view, QDragMoveEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::dragEnterEvent(QWidget *view, QDragEnterEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::dropEvent(QWidget *view, QDropEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::wheelEvent(QWidget *view, QWheelEvent *e) {
  Q_UNUSED(view)
  Q_UNUSED(e)
}


void KstViewObject::selectAll() {
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->setSelected(true);
  }
}


void KstViewObject::unselectAll() {
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->setSelected(false);
  }
}


bool KstViewObject::contains(KstViewObjectPtr child) {
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i).data() == child.data() || (*i)->contains(child)) {
      return true;
    }
  }
  return false;
}


const QString& KstViewObject::type() const {
  return _type;
}


void KstViewObject::setMinimumSize(const QSize& sz) {
  _minimumSize = sz.expandedTo(QSize(3, 3)); // 3,3 is the absolute minimum
}


const QSize& KstViewObject::minimumSize() const {
  return _minimumSize;
}

#include "kstviewobject.moc"
// vim: ts=2 sw=2 et
