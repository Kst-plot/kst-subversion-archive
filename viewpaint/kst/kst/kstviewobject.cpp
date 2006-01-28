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
#include <unistd.h>

// include files for Qt
#include <qbitmap.h>
#include <qdeepcopy.h>
#include <qmetaobject.h>
#include <qstylesheet.h>

// include files for KDE
#include <kdatastream.h>
#include "ksdebug.h"

// application specific includes
#include "kst.h"
#include "kst2dplot.h" // Yuck, fix this
#include "kstaccessibility.h"
#include "kstdoc.h"
#include "ksteditviewobjectdialog_i.h"
#include "kstmath.h"
#include "kstobject.h"
#include "kstplotgroup.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "ksttoplevelview.h"
#include "kstviewarrow.h"
#include "kstviewbox.h"
#include "kstviewellipse.h"
#include "kstviewlabel.h"
#include "kstviewlegend.h"
#include "kstviewobject.h"
#include "kstviewobjectfactory.h"
#include "kstviewpicture.h"
#include "kstviewwindow.h"

#define DEFAULT_LAYOUT_ACTIONS (Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo | Copy | CopyTo | Edit)
#define DEFAULT_MINIMUM_SIZE QSize(3, 3)

KstViewObject::KstViewObject(const QString& type)
: KstObject(), _geom(0, 0, 1, 1), _type(type) {
  _parent = 0L;
  _standardActions = 0;
  _layoutActions = DEFAULT_LAYOUT_ACTIONS;
  _maximized = false;
  updateAspect();
  _onGrid = false;
  _columns = 0;
  _container = true;
  _focus = false;
  _selected = false;
  _foregroundColor = KstSettings::globalSettings()->foregroundColor;
  _backgroundColor = KstSettings::globalSettings()->backgroundColor;
  _transparent = false;
  _followsFlow = false;
  _dialogLock = false;
  _fallThroughTransparency = true;
  setMinimumSize(DEFAULT_MINIMUM_SIZE);
}


KstViewObject::KstViewObject(const QDomElement& e)
: KstObject() {
  _layoutActions = DEFAULT_LAYOUT_ACTIONS;
  _foregroundColor = KstSettings::globalSettings()->foregroundColor;
  _backgroundColor = KstSettings::globalSettings()->backgroundColor;
  _parent = 0L;
  _container = true;
  _transparent = false;
  _followsFlow = false;
  _dialogLock = false;
  _fallThroughTransparency = true;
  setMinimumSize(DEFAULT_MINIMUM_SIZE);
  load(e);
}


KstViewObject::KstViewObject(const KstViewObject& viewObject)
: KstObject() {
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
  _container = true;
  _dialogLock = false;
  _selected = false;
  _fallThroughTransparency = true;
  _geom = viewObject._geom;
  _transparent = viewObject._transparent;
  _followsFlow = viewObject._followsFlow;
  setMinimumSize(DEFAULT_MINIMUM_SIZE);

  setContentsRect(viewObject.contentsRect());

  for (KstViewObjectList::ConstIterator i = viewObject._children.begin(); i != viewObject._children.end(); ++i) {
    (*i)->copyObjectQuietly(*this);
  }
}


void KstViewObject::load(const QDomElement& e) {
  _children.clear();
  _standardActions = 0;
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
      } else if (el.tagName() == "transparent") {
        _transparent = true;
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
      if (el.tagName() == "Plot" ||
          el.tagName() == "plot") {
        // get the <tag> value properly
        QString in_tag;
        QDomNode plotNode = el.firstChild();
        while (!plotNode.isNull()) {
          QDomElement plotElem = plotNode.toElement();
          if (!plotElem.isNull()) {
            if (plotElem.tagName() == "tag") {
              in_tag = plotElem.text();
            }
          }
          plotNode = plotNode.nextSibling();
        }
        // FIXME: View Object should not know about this!!!!
        Kst2DPlotMap *pmap = KstApp::inst()->plotHolderWhileOpeningDocument();
        if (pmap->count(in_tag) > 0) {
          Kst2DPlotPtr plot = (*pmap)[in_tag];
          if (plot) {
            appendChild(plot.data(), true);
            plot->loadChildren(el);
            pmap->erase(in_tag);
          }
        }
      } else if (el.tagName() == "plotgroup") {
        KstPlotGroupPtr plotGroup = new KstPlotGroup(el);
        appendChild(plotGroup.data(), true);
        plotGroup->loadChildren(el);
      } else if (el.tagName() == "Box") {
        KstViewBoxPtr box = new KstViewBox(el);
        appendChild(box.data(), true);
        box->loadChildren(el);
      } else if (el.tagName() == "Arrow") {
        KstViewArrowPtr arrow = new KstViewArrow(el);
        appendChild(arrow.data(), true);
        arrow->loadChildren(el);
      } else if (el.tagName() == "Line") {
        KstViewLinePtr line = new KstViewLine(el);
        appendChild(line.data(), true);
        line->loadChildren(el);
      } else if (el.tagName() == "Ellipse") {
        KstViewEllipsePtr ellipse = new KstViewEllipse(el);
        appendChild(ellipse.data(), true);
        ellipse->loadChildren(el);
      } else if (el.tagName() == "Label") {
        KstViewLabelPtr label = new KstViewLabel(el);
        appendChild(label.data(), true);
        label->loadChildren(el);
      } else if (el.tagName() == "Legend") {
        KstViewLegendPtr legend = new KstViewLegend(el);
        appendChild(legend.data(), true);
        legend->loadChildren(el);
      } else if (el.tagName() == "Picture") {
        KstViewPicturePtr picture = new KstViewPicture(el);
        appendChild(picture.data(), true);
        picture->loadChildren(el);
      }
    }
    n = n.nextSibling();
  }
}


KstObject::UpdateType KstViewObject::update(int counter) {
  if (checkUpdateCounter(counter)) {
    return lastUpdateResult();
  }

  KstObject::UpdateType rc = updateChildren(counter);

  return setLastUpdateResult(rc);
}


KstObject::UpdateType KstViewObject::updateChildren(int counter) {
  KstObject::UpdateType rc = NO_CHANGE;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if (rc == NO_CHANGE) {
      rc = (*i)->update(counter);
    } else {
      (*i)->update(counter);
    }
  }
  return rc;
}


void KstViewObject::save(QTextStream& ts, const QString& indent) {
  KstAspectRatio aspect;

  if (_maximized) {
    aspect = _aspectOldZoomedObject;
  } else {
    aspect = _aspect;
  }

  if (_transparent) {
    ts << indent << "<transparent/>" << endl;
  }
  ts << indent << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << indent << "<aspect x=\"" << aspect.x <<
    "\" y=\"" << aspect.y <<
    "\" w=\"" << aspect.w <<
    "\" h=\"" << aspect.h << "\" />" << endl;

  // save all properties
  for (int i = 0; i < metaObject()->numProperties(true); i++) {
    ts << indent << "<" << metaObject()->property(i, true)->name() << ">";
    ts << property(metaObject()->property(i, true)->name()).toString().latin1();
    ts << "</" << metaObject()->property(i, true)->name() << ">" << endl;
  }

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    // FIXME: SO BROKEN!!!  Get rid of this!
    if ((*i)->type() == "Plot") {
      (*i)->saveTagOnce(ts, indent); // special case: only save tags for plots when they are children
    } else {
      (*i)->save(ts, indent + "  "); // all other view objects are saved normally
    }
  }
}


void KstViewObject::saveTag(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<" << type() << ">" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->saveTag(ts, l2);
  }
  ts << indent << "</" << type() << ">" << endl;
}


void KstViewObject::saveTagOnce(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<" << type() << ">" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->type() == "Plot") {
      (*i)->saveTagOnce(ts, indent); // special case: only save tags for plots when they are children
    } else {
      (*i)->save(ts, indent + "  "); // all other view objects are saved normally
    }
  }
  ts << indent << "</" << type() << ">" << endl;
}


void KstViewObject::paint(KstPainter& p, const QRegion& bounds) {
  if (p.type() == KstPainter::P_EXPORT || p.type() == KstPainter::P_PRINT) {
    paintSelf(p, bounds);
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      (*i)->paint(p, bounds);
    }
    return;
  }

  p.save();
  p.setViewport(geometry());
  p.setWindow(geometry());

  paintUpdate();

  bool maximized = false;
  bool nullBounds = bounds.isNull();

  // handle the case where we have maximized plots
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->_maximized) {
      (*i)->paint(p, bounds);
      maximized = true;
      break;
    }
  }

  QRegion clipRegion;
  if (nullBounds) {
    clipRegion = geometry();
  } else {
    clipRegion = bounds;
  }
  if (!maximized && !_children.isEmpty()) {
    KstViewObjectList::Iterator begin = _children.begin();
    for (KstViewObjectList::Iterator i = _children.fromLast();; --i) {
      const QRegion thisObjectGeometry((*i)->geometry());
      if (nullBounds || !clipRegion.intersect(thisObjectGeometry).isEmpty()) {
#ifdef BENCHMARK
        QTime t;
        t.start();
#endif
        (*i)->paint(p, clipRegion);
        clipRegion -= (*i)->clipRegion();
#ifdef BENCHMARK
        int x = t.elapsed();
        kstdDebug() << "   -> object " << (*i)->tagName() << " took " << x << "ms" << endl;
#endif
      }
      if (i == begin) {
        break;
      }
    }
  }

  // Paint ourself
  paintSelf(p, clipRegion - p.uiMask());

  p.restore();

  // Draw any inline UI items
  if (p.drawInlineUI() && isSelected()) {
    if (_parent) {
      p.save();
      p.setViewport(_parent->geometry());
      p.setWindow(_parent->geometry());
      p.setClipping(false);
      drawSelectRect(p);
      p.restore();
    }
  }

  p.flush();
}


void KstViewObject::paintSelf(KstPainter& p, const QRegion& bounds) {
  if (!bounds.isNull()) {
    p.setClipRegion(bounds);
  }
  if (!_transparent) {
    p.fillRect(geometry(), _backgroundColor);
  }
}


void KstViewObject::updateSelf() {
  if (dirty()) {
    _clipMask = QRegion();
  }
}


void KstViewObject::paintUpdate() {
  updateSelf();
  setDirty(false);
}


void KstViewObject::drawFocusRect(KstPainter& p) {
  // to draw a focus rect, put something here
  // draw the 8 hotpoints
  const QRect geom(geometry());
  const QPoint topLeft(geom.topLeft());
  const QPoint topRight(geom.topRight());
  const QPoint bottomLeft(geom.bottomLeft());
  const QPoint bottomRight(geom.bottomRight());
  const QPoint topMiddle(QPoint((topLeft.x() + topRight.x())/2, topLeft.y()));
  const QPoint bottomMiddle(QPoint((bottomLeft.x() + bottomRight.x())/2, bottomLeft.y()));
  const QPoint middleLeft(QPoint(topLeft.x(), (topLeft.y() + bottomLeft.y())/2));
  const QPoint middleRight(QPoint(topRight.x(), (topRight.y() + bottomRight.y())/2));

  int dx = KST_RESIZE_BORDER_W/2;
  int width = 2*dx + 1;

  p.uiMask() += QRect(topLeft.x()-dx, topLeft.y()-dx, width, width);
  p.uiMask() += QRect(topRight.x()-dx, topRight.y()-dx, width, width);
  p.uiMask() += QRect(bottomLeft.x()-dx, bottomLeft.y()-dx, width, width);
  p.uiMask() += QRect(bottomRight.x()-dx, bottomRight.y()-dx, width, width);
  p.uiMask() += QRect(topMiddle.x()-dx, topMiddle.y()-dx, width, width);
  p.uiMask() += QRect(bottomMiddle.x()-dx, bottomMiddle.y()-dx, width, width);
  p.uiMask() += QRect(middleLeft.x()-dx, middleLeft.y()-dx, width, width);
  p.uiMask() += QRect(middleRight.x()-dx, middleRight.y()-dx, width, width);

  p.drawRect(topLeft.x()-dx, topLeft.y()-dx, width, width);
  p.drawRect(topRight.x()-dx, topRight.y()-dx, width, width);
  p.drawRect(bottomLeft.x()-dx, bottomLeft.y()-dx, width, width);
  p.drawRect(bottomRight.x()-dx, bottomRight.y()-dx, width, width);
  p.drawRect(topMiddle.x()-dx, topMiddle.y()-dx, width, width);
  p.drawRect(bottomMiddle.x()-dx, bottomMiddle.y()-dx, width, width);
  p.drawRect(middleLeft.x()-dx, middleLeft.y()-dx, width, width);
  p.drawRect(middleRight.x()-dx, middleRight.y()-dx, width, width);
}


void KstViewObject::drawSelectRect(KstPainter& p) {
  p.setBrush(QBrush(Qt::green));
  p.setPen(QPen(Qt::black, 0));
  drawFocusRect(p);
}


void KstViewObject::appendChild(KstViewObjectPtr obj, bool keepAspect) {
  obj->_parent = this;
  _children.append(obj);

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
      rc = (*i)->removeChild(obj, true) && rc;
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
  double xc = _aspect.x;  // preserve position in double precision to avoid wander on resize...
  double yc = _aspect.y;

  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));
  _geom.setSize(size.expandedTo(_minimumSize));
  updateAspect();
  _aspect.x = xc; // restore position.
  _aspect.y = yc;
  updateFromAspect();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResized();
  }
}


void KstViewObject::resizeForPrint(const QSize& size) {
  _geomOld = _geom;
  _geom.setSize(size);
  _clipMask = QRegion();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResizedForPrint();
  }
}


void KstViewObject::revertForPrint() {
  _geom = _geomOld;
  _clipMask = QRegion();
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


void KstViewObject::internalAlignment(KstPainter& p, QRect& plotRegion) {
  Q_UNUSED(p)
  static const QRect x(0,0,0,0);

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->internalAlignment(p, plotRegion);
    if (!plotRegion.isNull()) {
      KST::alignment.setPosition((*i)->geometry(), plotRegion);
    }
  }
  
  plotRegion = x;
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
  _clipMask = QRegion();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentRevertedForPrint();
  }
}


void KstViewObject::setBackgroundColor(const QColor& color) {
  if (_backgroundColor != color) {
    _backgroundColor = color;
    setDirty();
  }
}


QColor KstViewObject::backgroundColor() const {
  return _backgroundColor;
}


void KstViewObject::setForegroundColor(const QColor& color) {
  if (_foregroundColor != color) {
    _foregroundColor = color;
    setDirty();
  }
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
  KstViewObjectList childrenCopy;

  for (KstViewObjectList::ConstIterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->followsFlow()) {
      childrenCopy.append(*i);
    }
  }

  int cnt = childrenCopy.count();
  if (cnt < 1) {
    return;
  }

  // FIXME: don't allow regrid to a number of columns that will result in
  //        >= height() plots in a column
  if (!_onGrid) {
    if (cols <= 0) {
      cols = int(sqrt(cnt));
    }
    _onGrid = true;
    _columns = QMAX(1, cols);
  } else {
    if (cols > 0) {
      _columns = cols;
    } else if (cols <= 0){
      _columns = QMAX(1, int(sqrt(cnt)));
    }
  }
  int rows = ( cnt + _columns - 1 ) / _columns;

  //
  // the following is an attempt to arrange objects on a grid.
  //  This should behave as both a snap-to-grid when the objects
  //  are already roughly aligned to the desired grid, but should
  //  also act to retain the inherent order when this is not the
  //  case. The inherent order is defined by identifying objects
  //  from left-to-right and top-to-bottom based on the position
  //  of their top-left corner.
  //
  double minDistance = 0.0;
  int pos = 0;
  int x = 0;
  int y = 0;
  int w = _geom.width() / _columns;
  int h = _geom.height() / rows;

  //kstdDebug() << "cleanup with w=" << w << " and h=" << h << endl;
  //kstdDebug() << "columns=" << _columns  << endl;
  for (int i = 0; i < cnt; ++i) {
    KstViewObjectList::Iterator nearest = childrenCopy.end();
    QPoint pt(x, y);
    QSize sz(w, h);

    // adjust the last column to be sure that we don't spill over
    if (pos % _columns == _columns - 1) {
      sz.setWidth(_geom.width() - x);
    }

    // adjust the last row to be sure that we don't spill over
    if (pos / _columns == rows - 1) {
      sz.setHeight(_geom.height() - y);
    }

    for (KstViewObjectList::Iterator it = childrenCopy.begin(); it != childrenCopy.end(); ++it) {
      double distance = rows * d2i(ceil(double(rows) * 2.0 * (*it)->aspectRatio().y)) + (*it)->aspectRatio().x * rows + (*it)->aspectRatio().y;

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
  const QPoint offset(pos - _geom.topLeft());

  if (!offset.isNull()) {
    _geom.moveTopLeft(pos);
    updateAspectPos();
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      (*i)->parentMoved(offset);
    }
  }

  _clipMask = QRegion(); // not dirty, but need to rebuild
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
  for (--i; ; --i) {
    if ((*i)->tagName() == name) {
      return *i;
    }
    if (recursive) {
      KstViewObjectPtr rc = (*i)->findChild(name, recursive);
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


KstViewObjectPtr KstViewObject::findDeepestChild(const QPoint& pos, bool borderForTransparent) {
  KstViewObjectPtr obj = findChild(pos, borderForTransparent);
  if (obj) {
    KstViewObjectPtr c;
    do {
      c = obj->findDeepestChild(pos, borderForTransparent);
      if (c) {
        obj = c;
      }
    } while (c);
  }
  return obj;
}


KstViewObjectPtr KstViewObject::findChild(const QPoint& pos, bool borderForTransparent) {
  KstViewObjectPtr obj;

  if (!_geom.contains(pos) || _children.isEmpty()) {
    return obj;
  }

  KstViewObjectList::Iterator i = _children.end();
  for (--i; ; --i) {
    if ((*i)->surroundingGeometry().contains(pos)) {
      if ((*i)->_maximized) {
        obj = *i;
        break;
      }
      if (!obj) {
        if ((*i)->transparent()) {
          if (!(*i)->fallThroughTransparency() && (*i)->geometry().contains(pos)) {
            obj = *i;
          } else if ((*i)->clipRegion().contains(pos)) {
            obj = *i;
          } else if (borderForTransparent && (*i)->geometry().contains(pos)) {
            const QRect g((*i)->geometry());
            if ((pos.x() >= g.left() && pos.x() <= g.left() + KST_RESIZE_BORDER_W) ||
                (pos.x() <= g.right() && pos.x() >= g.right() - KST_RESIZE_BORDER_W) ||
                (pos.y() >= g.top() && pos.y() <= g.top() + KST_RESIZE_BORDER_W) ||
                (pos.y() <= g.bottom() && pos.y() >= g.bottom() - KST_RESIZE_BORDER_W)) {
              obj = *i;
            }
          }
        } else {
          obj = *i;
        }
      }
    }
    // one last check - hotpoints
    if (!obj && (*i)->isSelected() && (*i)->directionFor(pos) > 0) {
      obj = *i;
    }
    if (i == _children.begin()) {
      break;
    }
  }

  return obj;
}


KstViewObjectPtr KstViewObject::findDeepestChild(const QRect& rect) {
  KstViewObjectPtr obj = findChild(rect);
  if (obj) {
    KstViewObjectPtr c;
    do {
      c = obj->findDeepestChild(rect);
      if (c) {
        obj = c;
      }
    } while (c);
  }
  return obj;
}


KstViewObjectPtr KstViewObject::findChild(const QRect& rect) {
  KstViewObjectPtr obj;

  if (!_geom.contains(rect) || _children.isEmpty()) {
    return obj;
  }

  KstViewObjectList::Iterator i = _children.end();
  for (--i; ; --i) {
    if ((*i)->_container && (*i)->surroundingGeometry().contains(rect)) {
      obj = *i;
      break;
    }
    if (i == _children.begin()) {
      break;
    }
  }

  return obj;
}


const KstAspectRatio& KstViewObject::aspectRatio() const {
  return _aspect;
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
  bool rc = false;
  int id;
  QString menuTitle = this->menuTitle();

  _topObjectForMenu = topParent;

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

  _topObjectForMenu = topParent;

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


void KstViewObject::edit() {
  KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(KstViewObjectPtr(_topObjectForMenu));
  showDialog(tlv);
}


void KstViewObject::deleteObject() {
  KstApp::inst()->document()->setModified();
  KstViewObjectPtr vop(this);
  if (_topObjectForMenu) {
    KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(KstViewObjectPtr(_topObjectForMenu));
    if (tlv && vop == tlv->pressTarget()) {
      tlv->clearPressTarget();
    }
    _topObjectForMenu->removeChild(this, true);
    _topObjectForMenu = 0L;
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
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      _parent->_children.remove(it);
      _parent->_children.append(t);
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


void KstViewObject::lowerToBottom() {
  if (_parent) {
    KstViewObjectPtr t = this;
    KstViewObjectList::Iterator it = _parent->_children.find(t);

    if (it != _parent->_children.end()) {
      _parent->_children.remove(it);
      _parent->_children.prepend(t);
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


void KstViewObject::raise() {
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
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


void KstViewObject::lower() {
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
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


// FIXME: delete
void KstViewObject::moveTo(int id) {
  QString windowName = _moveToMap[id];

  if (_parent && !windowName.isEmpty()) {
    KMdiChildView *mdiChild = KstApp::inst()->findWindow(windowName);
    KstViewWindow *window = dynamic_cast<KstViewWindow*>(mdiChild);
    if (window) {
      KstViewObjectPtr t = this;
      KstViewObjectList::Iterator it = _parent->_children.find(t);
      if (it != _parent->_children.end()) {
        KstApp::inst()->document()->setModified();
        setDirty();
        _parent->_children.remove(it);
        window->view()->appendChild(t, true);
        window->view()->paint(KstPainter::P_PAINT);
      }
    }
  }
}


// FIXME: delete
void KstViewObject::copyTo(int id) {
  QString windowName = _copyToMap[id];

  if (!windowName.isEmpty()) {
    KMdiChildView *mdiChild = KstApp::inst()->findWindow(windowName);
    KstViewWindow *window = dynamic_cast<KstViewWindow*>(mdiChild);
    if (window) {
      setDirty();
      KstApp::inst()->document()->setModified();
      copyObjectQuietly(*(window->view().data()));
    }
  }
}


void KstViewObject::updateFromAspect() {
  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));
  if (_parent) {
    const QRect geom(_parent->geometry());
    _geom.setLeft(geom.left() + int(_aspect.x * geom.width()));
    _geom.setTop(geom.top() + int(_aspect.y * geom.height()));
    _geom.setRight(geom.left() + int((_aspect.x + _aspect.w) * geom.width()) - 1);
    _geom.setBottom(geom.top() + int((_aspect.y + _aspect.h) * geom.height()) - 1);
  }
  if (_geom.width() >= _minimumSize.width() || _geom.height() >= _minimumSize.height()) {
    _geom.setSize(_geom.size().expandedTo(_minimumSize));
  }
  assert(_geom.left() >= 0 && _geom.top() >= 0);
  setDirty();
}


void KstViewObject::updateAspectPos() {
  if (_parent) {
    const QRect geom(_parent->geometry());
    _aspect.x = double(geometry().left() - geom.left()) / double(geom.width());
    _aspect.y = double(geometry().top() - geom.top()) / double(geom.height());
  } else {
    _aspect.x = 0.0;
    _aspect.y = 0.0;
  }
}


void KstViewObject::updateAspectSize() {
  if (_parent) {
    const QRect geom(_parent->geometry());
    _aspect.w = double(geometry().width()) / double(geom.width());
    _aspect.h = double(geometry().height()) / double(geom.height());
  } else {
    _aspect.w = 0.0;
    _aspect.h = 0.0;
  }
}


void KstViewObject::updateAspect() {
  updateAspectSize();
  updateAspectPos();
  setDirty();
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
  setDirty();
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


KstHandlerFactoryMethod KstViewObject::handlerFactory() const {
  return 0L;
}


QDataStream& operator<<(QDataStream& str, KstViewObjectPtr obj) {
  obj->writeBinary(str);
  return str;
}


void KstViewObject::writeBinary(QDataStream& str) {
  str << type();
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
  kstdDebug() << "Decoding " << tagName << " from drag." << endl;
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


void KstViewObject::removeFocus(KstPainter& p) {
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


bool KstViewObject::contains(const KstViewObjectPtr child) const {
  for (KstViewObjectList::ConstIterator i = _children.begin(); i != _children.end(); ++i) {
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
  _minimumSize = sz.expandedTo(QSize(1, 1)); // 1,1 is the absolute minimum
}


const QSize& KstViewObject::minimumSize() const {
  return _minimumSize;
}


void KstViewObject::setTransparent(bool transparent) {
  if (_transparent != transparent) {
    _transparent = transparent;
    setDirty();
  }
}


bool KstViewObject::transparent() const {
  return _transparent;
}


QRegion KstViewObject::clipRegion() {
  if (_clipMask.isNull()) {
    if (_transparent) {
      QBitmap bm(_geom.bottomRight().x(), _geom.bottomRight().y(), true);
      if (!bm.isNull()) {
        KstPainter p;
        p.setMakingMask(true);
        p.begin(&bm);
        p.setViewXForm(true);
        paint(p, QRegion());
        p.flush();
        p.end();
        _clipMask = QRegion(bm);
      } else {
        _clipMask = QRegion();
      }
    } else {
      _clipMask = QRegion(_geom);
    }
  }

  return _clipMask;
}


void KstViewObject::setFollowsFlow(bool follow) {
  _followsFlow = follow;
}


bool KstViewObject::followsFlow() const {
  return _followsFlow;
}


// id=0 v1.2.0 setDirty() was called.  data == 0 means false, else true
void KstViewObject::virtual_hook(int id, void *data) {
  switch (id) {
    case 0:
      if (data) {
        _clipMask = QRegion();
      }
      break;
    default:
      break;
  }
  KstObject::virtual_hook(id, data);
}


QMap<QString, QVariant > KstViewObject::widgetHints(const QString& propertyName) const {
  Q_UNUSED(propertyName)
  return QMap<QString, QVariant>();
}


inline bool pointsCloseEnough(const QPoint& point1, const QPoint& point2) {
  const int dx = KST_RESIZE_BORDER_W/2;
  return point1.x() <= point2.x() + dx &&
      point1.x() >= point2.x() - dx &&
      point1.y() <= point2.y() + dx &&
      point1.y() >= point2.y() - dx;
}


signed int KstViewObject::directionFor(const QPoint& pos) {
  // no hotpoints if object is not selected - can only move it
  if (!isSelected()) {
    return NONE;
  }

  signed int direction = NONE;

  const QRect geom(geometry());
  const QPoint topLeft(geom.topLeft());
  const QPoint topRight(geom.topRight());
  const QPoint bottomLeft(geom.bottomLeft());
  const QPoint bottomRight(geom.bottomRight());
  const QPoint topMiddle(QPoint((topLeft.x() + topRight.x())/2, topLeft.y()));
  const QPoint bottomMiddle(QPoint((bottomLeft.x() + bottomRight.x())/2, bottomLeft.y()));
  const QPoint middleLeft(QPoint(topLeft.x(), (topLeft.y() + bottomLeft.y())/2));
  const QPoint middleRight(QPoint(topRight.x(), (topRight.y() + bottomRight.y())/2));

  if (pointsCloseEnough(topLeft, pos)) {
    direction |= UP | LEFT;
  } else if (pointsCloseEnough(topRight, pos)) {
    direction |= UP | RIGHT;
  } else if (pointsCloseEnough(bottomLeft, pos)) {
    direction |= DOWN | LEFT;
  } else if (pointsCloseEnough(bottomRight, pos)) {
    direction |= DOWN | RIGHT;
  } else if (pointsCloseEnough(topMiddle, pos)) {
    direction |= UP;
  } else if (pointsCloseEnough(bottomMiddle, pos)) {
    direction |= DOWN;
  } else if (pointsCloseEnough(middleLeft, pos)) {
    direction |= LEFT;
  } else if (pointsCloseEnough(middleRight, pos)) {
    direction |= RIGHT;
  }

  return direction;
}


bool KstViewObject::showDialog(KstTopLevelViewPtr invoker, bool isNew) {
  bool rc = false;
  if (!_dialogLock) {
    KstEditViewObjectDialogI dlg(KstApp::inst());
    if (isNew) {
      dlg.setNew();
    }
    dlg.showEditViewObjectDialog(this, invoker);
    rc = QDialog::Rejected != dlg.exec();
  }
  return rc;
}


void KstViewObject::drawShadow(KstPainter& p, const QPoint& pos) {
  // default is a rectangle
  QRect rect(geometry());
  rect.moveTopLeft(pos);
  p.drawRect(rect);
}


QRect KstViewObject::surroundingGeometry() const {
  return geometry();
}


bool KstViewObject::objectDirty() const {
  if (dirty()) {
    return true;
  }

  for (KstViewObjectList::ConstIterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->objectDirty()) {
      return true;
    }
  }
  return false;
}


QWidget *KstViewObject::configWidget() {
  return 0L;
}


bool KstViewObject::fillConfigWidget(QWidget *w, bool isNew) const {
  Q_UNUSED(w)
  Q_UNUSED(isNew)
  return false;
}


bool KstViewObject::readConfigWidget(QWidget *w) {
  Q_UNUSED(w)
  return false;
}


KstGfxMouseHandler *KstViewObject::createHandler() {
  return 0L;
}


void KstViewObject::setDialogLock(bool lock) {
  _dialogLock = lock;
}


bool KstViewObject::dialogLocked() const {
  return _dialogLock;
}


bool KstViewObject::fallThroughTransparency() const {
  return _fallThroughTransparency;
}

#include "kstviewobject.moc"
// vim: ts=2 sw=2 et
