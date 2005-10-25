/***************************************************************************
                ksttoplevelview.cpp: toplevel view objects
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
#include <stdlib.h>

// include files for Qt

// include files for KDE
#include "ksdebug.h"
#include <klocale.h>

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "kstplotgroup.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "ksttoplevelview.h"

#define STICKY_THRESHOLD 10

KstTopLevelView::KstTopLevelView(QWidget *parent, const char *name, WFlags w)
: KstViewObject("KstTopLevelView"), _w(new KstViewWidget(this, parent, name, w)) {
  _onGrid = true;
  setTagName(name);
  commonConstructor();
}


KstTopLevelView::KstTopLevelView(const QDomElement& e, QWidget *parent, const char *name, WFlags w) : KstViewObject(e), _w(new KstViewWidget(this, parent, name, w)) {
  commonConstructor();
  loadChildren(e);
}


void KstTopLevelView::commonConstructor() {
  _focusOn = false;
  setViewMode(_mode = DisplayMode);
  _pressDirection = -1;
  _moveOffset = QPoint(-1, -1);
  _moveOffsetSticky = QPoint(0, 0);
  _backgroundColor = _w->backgroundColor();
  _mouseGrabbed = false;
}


KstTopLevelView::~KstTopLevelView() {
}


void KstTopLevelView::save(QTextStream& ts, const QString& indent) {
  bool temp_onGrid;
  if (_maximized) {
    temp_onGrid = _prevOnGrid;
  } else {
    temp_onGrid = _onGrid;
  }
  if (temp_onGrid && _columns > 0) { // only if on a grid and we have contents
    ts << indent << "<columns>" << _columns << "</columns>" << endl;
  }
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->saveTag(ts, indent);
  }
}


void KstTopLevelView::paint(KstPaintType type, QPainter& p) {
#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  if (type == P_PRINT || type == P_EXPORT) {
    updateAlignment(type, p);
    p.fillRect(geometry(), QBrush(KstSettings::globalSettings()->backgroundColor, SolidPattern));
  } else {
    updateAlignment(type, p);
  }
#ifdef BENCHMARK
  int x = t.elapsed();
  kstdDebug() << " -> Toplevelview internally took " << x << "ms" << endl;
  kstdDebug() << "      type was " << (int)type << endl;
  t.start();
#endif
  QRegion clipRegion(geometry());
  p.setClipRegion(clipRegion);
  KstViewObject::paint(type, p);
#ifdef BENCHMARK
  x = t.elapsed();
  kstdDebug() << " -> Parent class took " << x << "ms" << endl;
#endif
}


void KstTopLevelView::resized(const QSize& size) {
  KstViewObject::resize(size);
}


void KstTopLevelView::updateAlignment(KstPaintType type, QPainter& p) {
  QRect plotRegion;

  KST::alignment.reset();  
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->internalAlignment(type, p, plotRegion);
    KST::alignment.setPosition((*i)->geometry(), plotRegion);
  }
}


void KstTopLevelView::resize(const QSize& size) {
  _w->resize(size);
  KstViewObject::resize(size);
}


void KstTopLevelView::paint(KstPaintType type) {
  QPainter p;
  QRegion r(_w->geometry());

  p.begin(_w);
  p.setViewXForm(true);
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    r -= QRegion((*i)->geometry());
  }
  p.setClipRegion(r);
  _lastClipRegion = r;
  p.fillRect(geometry(), QBrush(_backgroundColor));
  p.setClipping(false);
  paint(type, p);
  p.end();
}


void KstTopLevelView::clearFocus() {
  if (_focusOn) {
    _pressDirection = -1;
    _moveOffset = QPoint(-1, -1);
    _moveOffsetSticky = QPoint(0, 0);
    _w->unsetCursor();
    _focusOn = false;
    //recursively<bool>(&KstViewObject::setFocus, false);
    QPainter p;
    p.begin(_w);
    p.setViewXForm(true);
    if (_hoverFocus) {
      _hoverFocus->setFocus(false);
      _hoverFocus->drawFocusRect(p);
      _hoverFocus = 0L;
    }
    paint(P_PAINT, p);
    p.end();
  }
}


void KstTopLevelView::updateFocus(const QPoint& pos) {
  if (_mode != LayoutMode || tracking()) {
    return;
  }

  KstViewObjectPtr p = findChild(pos);
  if (p) {
    setCursorFor(pos, p->geometry());
    if (p->focused()) {
      _focusOn = true; // just in case - seems to be false on occasion
      return;
    }
    if (_focusOn) { // something else has the focus, clear it
      p->setFocus(true);
      clearFocus();
      _focusOn = true;
    } else {
      p->setFocus(true);
      _focusOn = true;
      paint(P_PAINT);
    }
    _hoverFocus = p;
  } else {
    clearFocus();
  }
}


void KstTopLevelView::setViewMode(ViewMode v) {
  KstApp::inst()->slotUpdateDataMsg(QString::null);
  if (_mode == LayoutMode && v != LayoutMode) {
    recursively<bool>(&KstViewObject::setSelected, false);
    clearFocus();
    paint(P_PAINT);
  } else if (_mode == DisplayMode && v != DisplayMode) {
    recursively<bool>(&KstViewObject::setMaximized, false);
  }

  _mode = v;

  _w->setDragEnabled(_mode == LayoutMode);
}


static char directionFor(const QPoint& pos, const QRect& objGeom) {
  char direction = 0;
#define UP    1
#define DOWN  2
#define LEFT  4
#define RIGHT 8
  if (pos.x() < objGeom.left() + 4) {
    direction |= LEFT;
  } else if (pos.x() > objGeom.right() - 4) {
    direction |= RIGHT;
  }

  if (pos.y() < objGeom.top() + 4) {
    direction |= UP;
  } else if (pos.y() > objGeom.bottom() - 4) {
    direction |= DOWN;
  }

  return direction;
}


void KstTopLevelView::setCursorFor(const QPoint& pos, const QRect& objGeom) {
  char direction = directionFor(pos, objGeom);

  switch (direction) {
    case UP:
    case DOWN:
      _cursor.setShape(Qt::SizeVerCursor);
      break;
    case LEFT:
    case RIGHT:
      _cursor.setShape(Qt::SizeHorCursor);
      break;
    case UP|LEFT:
    case DOWN|RIGHT:
      _cursor.setShape(Qt::SizeFDiagCursor);
      break;
    case UP|RIGHT:
    case DOWN|LEFT:
      _cursor.setShape(Qt::SizeBDiagCursor);
      break;
    default:
      _cursor.setShape(Qt::SizeAllCursor);
      break;
  }

  if (_cursor.shape() != _w->cursor().shape()) {
    _w->setCursor(_cursor);
  }
}


void KstTopLevelView::restartMove() {
  _pressDirection = 0;
  _cursor.setShape(Qt::SizeAllCursor);
  _w->setCursor(_cursor);
  assert(_pressTarget);
}


bool KstTopLevelView::handlePress(const QPoint& pos, bool shift) {
  //kstdDebug() << "HANDLE PRESS" << endl;
  _pressDirection = -1;

  if (_mode != LayoutMode) {
    _pressTarget = 0L;
    return false;
  }

  _prevBand = QRect(-1, -1, 0, 0);
  _moveOffset = QPoint(-1, -1);
  _moveOffsetSticky = QPoint(0, 0);

  if (!_focusOn) {
    _pressTarget = 0L;
    _cursor.setShape(Qt::ArrowCursor);
    _w->setCursor(_cursor);
    _moveOffset = pos; // use _moveOffset to store our start point
    return true;
  }

  _pressTarget = findChild(pos);
  assert(_pressTarget);

  if (shift) {
    KstViewObjectList::Iterator it = _selectionList.find(_pressTarget);

    if (_pressTarget->isSelected()) {
      _pressTarget->setSelected(false);
      if (it != _selectionList.end()) {
        _selectionList.remove(it);
      }
    } else {
      _pressTarget->setSelected(true);
      if (it == _selectionList.end()) {
        _selectionList.append(_pressTarget);
      }
    }
    _pressTarget = 0L;
    _pressDirection = -1;
    _moveOffset = QPoint(-1, -1);
    _moveOffsetSticky = QPoint(0, 0);
    updateFocus(pos);
    paint(P_PAINT);

    return true;
  }

  _pressDirection = directionFor(pos, _pressTarget->geometry());
  if (_pressDirection == 0) {
    _moveOffset = pos - _pressTarget->position();
    _selectionList.clear();
    if (_pressTarget->isSelected()) {
      recursivelyQuery(&KstViewObject::isSelected, _selectionList, false);
    } else {
      recursively<bool>(&KstViewObject::setSelected, false);
    }
  } else {
    _selectionList.clear();
    recursively<bool>(&KstViewObject::setSelected, false);
  }

  _pressTarget->setFocus(false);

  return true;
}


QRect KstTopLevelView::newSize(const QRect& oldSize, int direction, const QPoint& pos) {
  QRect r = oldSize;
  QPoint save;
  
  switch (direction & (UP|DOWN)) {
    case UP:
      r.setTop(pos.y());
      break;
    case DOWN:
      r.setBottom(pos.y());
      break;
    default:
      break;
  }
  
  switch (direction & (LEFT|RIGHT)) {  
    case LEFT:
      r.setLeft(pos.x());
      break;
    case RIGHT:
      r.setRight(pos.x());
      break;
    default:
      break;
  }
  
  {
    int iXMin = STICKY_THRESHOLD;
    int iYMin = STICKY_THRESHOLD;

    // check for "sticky" borders
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      if (_pressTarget != *i) {           
        if (direction & LEFT) {
          if (labs(r.left() - (*i)->geometry().left()) < labs(iXMin)) {
            iXMin = r.left() - (*i)->geometry().left();
          } else if (labs(r.left() - (*i)->geometry().right()) < labs(iXMin)) {
            iXMin = r.left() - (*i)->geometry().right();              
          }
        } else if (direction & RIGHT) {
          if (labs(r.right() - (*i)->geometry().left()) < labs(iXMin)) {
            iXMin = r.right() - (*i)->geometry().left();
          } else if (labs(r.right() - (*i)->geometry().right()) < labs(iXMin)) {
            iXMin = r.right() - (*i)->geometry().right();
          }
        }                 
        
        if (direction & UP) {
          if (labs(r.top() - (*i)->geometry().top()) < labs(iYMin)) {
            iYMin = r.top() - (*i)->geometry().top();
          } else if (labs(r.top() - (*i)->geometry().bottom()) < labs(iYMin)) {
            iYMin = r.top() - (*i)->geometry().bottom();              
          }
        } else if (direction & DOWN) {
          if (labs(r.bottom() - (*i)->geometry().top()) < labs(iYMin)) {
            iYMin = r.bottom() - (*i)->geometry().top();
          } else if (labs(r.bottom() - (*i)->geometry().bottom()) < labs(iYMin)) {
            iYMin = r.bottom() - (*i)->geometry().bottom();
          }
        }
      }
    }
  
    if (labs(iXMin) < STICKY_THRESHOLD) {
      if (direction & LEFT) {
        r.setLeft(r.left() - iXMin);
      } else if (direction & RIGHT) {
        r.setRight(r.right() - iXMin);
      }
    }
    if (labs(iYMin) < STICKY_THRESHOLD) {
      if (direction & UP) {
        r.setTop(r.top() - iYMin);
      } else if (direction & DOWN) {
        r.setBottom(r.bottom() - iYMin);
      }
    }    
  }
  
  r = r.normalize();
  
  return r;
}


static void slideInto(const QRect& region, QRect& obj) {
  if (obj.left() < region.left()) {
    obj.moveLeft(region.left());
  }

  if (obj.right() > region.right()) {
    obj.moveRight(region.right());
  }

  if (obj.bottom() > region.bottom()) {
    obj.moveBottom(region.bottom());
  }

  if (obj.top() < region.top()) {
    obj.moveTop(region.top());
  }
}


// Optimize me: can store, for instance, the painter I think
void KstTopLevelView::pressMove(const QPoint& pos, bool shift) {
  if (_mode != LayoutMode) {
    _pressTarget = 0L;
    return;
  }

  if (_pressDirection == -1 && _pressTarget) { // menu released
    return;
  }

  if (shift && _moveOffset == QPoint(-1, -1)) {
    return;
  }

  if (_pressTarget) {
    if (_pressDirection == 0) {
      // moving a plot.
      QRect old = _prevBand;
      QPoint topLeft;
      QRect r;
      
      r = _pressTarget->geometry();
      if (!_selectionList.isEmpty()) {
        for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
          r = r.unite((*i)->geometry());
        }
      }      
      topLeft = pos - _moveOffset - _pressTarget->geometry().topLeft() + r.topLeft();
      r.moveTopLeft(topLeft);
      _moveOffsetSticky = QPoint(0, 0);
      
      {
        int iXMin = STICKY_THRESHOLD;
        int iYMin = STICKY_THRESHOLD;
      
        // check for "sticky" borders
        for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
          if (_selectionList.find(*i) == _selectionList.end() && _pressTarget != *i) {           
            if (labs(r.left() - (*i)->geometry().left()) < labs(iXMin)) {
              iXMin = r.left() - (*i)->geometry().left();
            } else if (labs(r.left() - (*i)->geometry().right()) < labs(iXMin)) {
              iXMin = r.left() - (*i)->geometry().right();              
            } else if (labs(r.right() - (*i)->geometry().left()) < labs(iXMin)) {
              iXMin = r.right() - (*i)->geometry().left();
            } else if (labs(r.right() - (*i)->geometry().right()) < labs(iXMin)) {
              iXMin = r.right() - (*i)->geometry().right();
            }                 
            
            if (labs(r.top() - (*i)->geometry().top()) < labs(iYMin)) {
              iYMin = r.top() - (*i)->geometry().top();
            } else if (labs(r.top() - (*i)->geometry().bottom()) < labs(iYMin)) {
              iYMin = r.top() - (*i)->geometry().bottom();              
            } else if (labs(r.bottom() - (*i)->geometry().top()) < labs(iYMin)) {
              iYMin = r.bottom() - (*i)->geometry().top();
            } else if (labs(r.bottom() - (*i)->geometry().bottom()) < labs(iYMin)) {
              iYMin = r.bottom() - (*i)->geometry().bottom();
            }
          }
        }
      
        if (labs(iXMin) < STICKY_THRESHOLD) {
          _moveOffsetSticky.setX(iXMin);
          topLeft.setX(topLeft.x() - iXMin);
        }
        if (labs(iYMin) < STICKY_THRESHOLD) {
          _moveOffsetSticky.setY(iYMin);
          topLeft.setY(topLeft.y() - iYMin);
        }
        r.moveTopLeft(topLeft);      
      }
      
      if (!_geom.contains(r, true)) {
        slideInto(_geom, r);
      }
      _prevBand = r;
      if (_prevBand != old) {
        QPainter p;
        
        p.begin(_w);
        p.setRasterOp(Qt::NotROP);
        p.drawWinFocusRect(old);
        p.drawWinFocusRect(r);
        p.end();
      }
    } else {
      // resizing a plot.
      QRect old = _prevBand;
      
      _prevBand = newSize(_pressTarget->geometry(), _pressDirection, pos);
      _prevBand = _prevBand.intersect(_geom);
      if (_prevBand != old) {
        QPainter p;
        
        p.begin(_w);
        p.setRasterOp(Qt::XorROP);
        if (old.topLeft() != QPoint(-1, -1)) {
          p.drawWinFocusRect(old);
        } else {     
          p.drawWinFocusRect(_pressTarget->geometry());
        }
        p.drawWinFocusRect(_prevBand);
        p.end();
      }
    }
    KstApp::inst()->slotUpdateDataMsg(i18n("(x0,y0)-(x1,y1)", "(%1,%2)-(%3,%4)").arg(_prevBand.topLeft().x()).arg(_prevBand.topLeft().y()).arg(_prevBand.bottomRight().x()).arg(_prevBand.bottomRight().y()));
  } else {
    // selecting plots.
    QRect old = _prevBand;
    QRect r;
    r.setTopLeft(_moveOffset);
    r.setBottomRight(pos);
    _prevBand = r.normalize().intersect(_geom);
    if (old != _prevBand) {
      QPainter p;
      p.begin(_w);
      p.setRasterOp(Qt::NotROP);
      p.drawWinFocusRect(old);
      p.drawWinFocusRect(_prevBand);
      p.end();
    }
    KstApp::inst()->slotUpdateDataMsg(QString::null);
  }
}


void KstTopLevelView::releasePress(const QPoint& pos, bool shift) { 
  if (_mode != LayoutMode) {
    _pressTarget = 0L;
    return;
  }

  KstApp::inst()->slotUpdateDataMsg(QString::null);

  if (_pressDirection == -1 && _pressTarget) { // menu released
    _pressTarget = 0L;
    return;
  }

  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    if (_pressDirection == 0) {
      QRect obj = _pressTarget->geometry();
      QRect old = obj;

      if (!_selectionList.isEmpty()) {
        for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
          obj = obj.unite((*i)->geometry());
        }
      }
      QPoint objOffset = _pressTarget->geometry().topLeft() - obj.topLeft();
      obj.moveTopLeft(pos - _moveOffset - _moveOffsetSticky - 
                      _pressTarget->geometry().topLeft() + obj.topLeft());
      if (!_geom.contains(obj, true)) {
        slideInto(_geom, obj);
      }
      _pressTarget->move(obj.topLeft() + objOffset);
      for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
        if (*i != _pressTarget) {
          (*i)->move(_pressTarget->position() + (*i)->geometry().topLeft() - old.topLeft());
        }
      }
      _onGrid = false;
    } else {
      QRect r = newSize(_pressTarget->geometry(), _pressDirection, pos).intersect(_geom);
        
      _pressTarget->move(r.topLeft());
      _pressTarget->resize(r.size());
      _onGrid = false;
    }
    _pressTarget->setFocus(true);   
  } else { // rubber band
    QPainter p;
    
    p.begin(_w);
    p.setRasterOp(Qt::NotROP);
    p.drawWinFocusRect(_prevBand);
    p.end();
    if (shift) {
      for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
        if (_prevBand.contains((*i)->geometry().center())) {
          (*i)->setSelected(true);
        }
      }
    } else {
      forEachChild<const QRect&>(&KstViewObject::updateSelection, _prevBand);
    }
    _prevBand = QRect(-1, -1, 0, 0);
  }

  _pressTarget = 0L;
  _pressDirection = -1;
  _moveOffset = QPoint(-1, -1);
  _moveOffsetSticky = QPoint(0, 0);
  
  updateFocus(pos);
  
  paint(P_PAINT);
}


bool KstTopLevelView::tracking() const {
  return _pressDirection != -1 || _moveOffset != QPoint(-1, -1);
}


bool KstTopLevelView::trackingIsMove() const {
  return _pressDirection == 0;
}


void KstTopLevelView::menuClosed() {
  _w->setFocus();
}


bool KstTopLevelView::popupMenu(KPopupMenu *menu, const QPoint& pos) {
  bool rc = false;
  // Want to clear focus without repaint
  _pressTarget = findChild(pos);

  if (_focusOn) {
    _pressDirection = -1;
    _moveOffset = QPoint(-1, -1);
    _w->unsetCursor();
  }

  _selectionList.clear();
  recursivelyQuery(&KstViewObject::isSelected, _selectionList, false);

  if (_pressTarget) {
    connect(menu, SIGNAL(aboutToHide()), this, SLOT(menuClosed()));
    if (_mode == LayoutMode) {
      rc = _pressTarget->layoutPopupMenu(menu, pos - _pressTarget->position(), this) || rc;
    } else {
      rc = _pressTarget->popupMenu(menu, pos - _pressTarget->position(), this) || rc;
    }
  }

  if (_selectionList.count() > 1) {
    if (_pressTarget && _mode == LayoutMode) {
      KPopupMenu* subMenu;

      if (rc) {
        menu->insertSeparator();
      }

      subMenu = new KPopupMenu(menu);
      subMenu->insertItem(i18n("Width"), this, SLOT(makeSameWidth()));
      subMenu->insertItem(i18n("Height"), this, SLOT(makeSameHeight()));
      subMenu->insertItem(i18n("Size"), this, SLOT(makeSameSize()));
      menu->insertItem(i18n("Make Same"), subMenu);

      subMenu = new KPopupMenu(menu);
      subMenu->insertItem(i18n("Left"), this, SLOT(alignLeft()));
      subMenu->insertItem(i18n("Right"), this, SLOT(alignRight()));
      subMenu->insertItem(i18n("Top"), this, SLOT(alignTop()));
      subMenu->insertItem(i18n("Bottom"), this, SLOT(alignBottom()));
      menu->insertItem(i18n("Align"), subMenu);

      subMenu = new KPopupMenu(menu);
      subMenu->insertItem(i18n("Horizontally"), this, SLOT(packHorizontally()));
      subMenu->insertItem(i18n("Vertically"), this, SLOT(packVertically()));
      menu->insertItem(i18n("Pack"), subMenu);

      menu->insertSeparator();
    }
    menu->insertItem(i18n("Group Objects"), this, SLOT(groupSelection()));
    rc = true;
  }

  if (_mode == LayoutMode) {
    if (rc) {
      menu->insertSeparator();
    }
    menu->insertItem(i18n("Cleanup Layout"), this, SLOT(cleanupAction()));
    rc = true;
  }

  // don't release the _pressTarget reference here as we are not yet done with
  //  it, since several of the menu handlers require it in order to function...
  // _pressTarget = 0L; // release reference

  return rc;
}


void KstTopLevelView::makeSameWidth() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QSize size;
    size.setWidth( gg.width() );
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      size.setHeight((*i)->geometry().height());
      (*i)->resize(size);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::makeSameHeight() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QSize size;
    size.setHeight( gg.height() );
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      size.setWidth((*i)->geometry().width());
      (*i)->resize(size);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::makeSameSize() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QSize size;
    QRect gg = _pressTarget->geometry();
    size.setHeight(gg.height());
    size.setWidth(gg.width());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      (*i)->resize(size);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::checkPosition(KstViewObjectPtr obj, QPoint point) {
  QRect rect = obj->geometry();
  rect.moveTopLeft(point);
  if (!_geom.contains(rect, true)) {
    slideInto(_geom, rect);
    point = rect.topLeft();
  }
  obj->move(point);
}


void KstTopLevelView::alignLeft() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QPoint point;
    point.setX(gg.x());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setY((*i)->geometry().y());
      checkPosition(*i, point);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::alignRight() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QPoint point;
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setX(gg.x() + gg.width() - (*i)->geometry().width());
      point.setY((*i)->geometry().y());
      checkPosition(*i, point);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::alignTop() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QPoint point;
    point.setY(gg.y());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setX((*i)->geometry().x());
      checkPosition(*i, point);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::alignBottom() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QRect gg = _pressTarget->geometry();
    QPoint point;
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setX((*i)->geometry().x());
      point.setY(gg.y() + gg.height() - (*i)->geometry().height());
      checkPosition(*i, point);
    }
    paint(P_PAINT);
  }
}


void KstTopLevelView::packVertically() {
  int iTop = 0;
  int iBottom = 0;
  int iCount = 0;

  for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    if (i == _selectionList.begin()) {
      iTop = (*i)->geometry().y();
      iBottom = (*i)->geometry().y() + (*i)->geometry().height();
    } else {
      if ((*i)->geometry().y() < iTop) {
        iTop = (*i)->geometry().y();
      }
      if ((*i)->geometry().y() + (*i)->geometry().height() > iBottom) {
        iBottom = (*i)->geometry().y() + (*i)->geometry().height();
      }
    }
    iCount++;
  }

  if (iBottom > iTop && iCount > 0) {
    KstViewObjectList selectionList = _selectionList;
    KstViewObjectList::Iterator temp;
    QPoint point;
    QSize size;
    int iPosMin = 0;
    int iPos = iTop;
    int iHeight = (iBottom - iTop) / iCount;

    while (!selectionList.empty()) {
      for (KstViewObjectList::Iterator i = selectionList.begin(); i != selectionList.end(); ++i) {
        if (i == selectionList.begin()) {
          iPosMin = (*i)->geometry().y();
          temp = i;
        } else if ((*i)->geometry().y() < iPosMin) {
          iPosMin = (*i)->geometry().y();
          temp = i;
        }
      }
      size.setWidth((*temp)->geometry().width());
      size.setHeight(iHeight);
      (*temp)->resize(size);
      point.setX((*temp)->geometry().x());
      point.setY(iPos);
      checkPosition(*temp, point);

      selectionList.erase(temp);
      iPos += iHeight;
    }
    KstApp::inst()->document()->setModified();
    paint(P_PAINT);
  }
}


void KstTopLevelView::packHorizontally() {
  int iLeft = 0;
  int iRight = 0;
  int iCount = 0;

  for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    if (i == _selectionList.begin()) {
      iLeft = (*i)->geometry().x();
      iRight = (*i)->geometry().x() + (*i)->geometry().width();
    } else {
      if ((*i)->geometry().x() < iLeft) {
        iLeft = (*i)->geometry().x();
      }
      if ((*i)->geometry().x() + (*i)->geometry().width() > iRight) {
        iRight = (*i)->geometry().x() + (*i)->geometry().width();
      }
    }
    iCount++;
  }

  if (iRight > iLeft && iCount > 0) {
    KstViewObjectList selectionList = _selectionList;
    KstViewObjectList::Iterator temp;
    QPoint point;
    QSize size;
    int iPosMin = 0;
    int iPos = iLeft;
    int iWidth = (iRight - iLeft) / iCount;

    while (!selectionList.empty()) {
      for (KstViewObjectList::Iterator i = selectionList.begin(); i != selectionList.end(); ++i) {
        if (i == selectionList.begin()) {
          iPosMin = (*i)->geometry().x();
          temp = i;
        } else if ((*i)->geometry().x() < iPosMin) {
          iPosMin = (*i)->geometry().x();
          temp = i;
        }
      }
      size.setWidth(iWidth);
      size.setHeight((*temp)->geometry().height());
      (*temp)->resize(size);
      point.setX(iPos);
      point.setY((*temp)->geometry().y());
      checkPosition(*temp, point);

      selectionList.erase(temp);
      iPos += iWidth;
    }
    KstApp::inst()->document()->setModified();
    paint(P_PAINT);
  }
}


void KstTopLevelView::groupSelection() {
  KstPlotGroupPtr pg = new KstPlotGroup;
  KstViewObjectList::Iterator it;
  QRect gg = _selectionList.first()->geometry();

  if (pg) {
    // First build the container
    for (it = _selectionList.begin(); it != _selectionList.end(); ++it) {
      gg |= (*it)->geometry();
    }

    pg->move(gg.topLeft());
    pg->resize(gg.size());

    // Then add the items
    for (it = _selectionList.begin(); it != _selectionList.end(); ++it) {
      (*it)->setSelected(false);
      (*it)->setFocus(false);
      (*it)->detach();
      pg->appendChild(*it);
    }

    if (!pg->children().isEmpty()) {
      appendChild(pg.data());
    }

    KstApp::inst()->document()->setModified();
    paint(P_PAINT);
  }
}


void KstTopLevelView::cancelMouseOperations() {
  if (_mode == LayoutMode) {
    clearFocus();
    //_pressTarget = 0L;
    _prevBand = QRect(-1, -1, 0, 0);
  }
}


bool KstTopLevelView::removeChild(KstViewObjectPtr obj, bool recursive) {
  if (_pressTarget == obj) {
    _pressTarget = 0L;
  }

  return KstViewObject::removeChild(obj, recursive);
}


void KstTopLevelView::cleanup(int cols) {
  KstViewObject::cleanup(cols);
}


bool KstTopLevelView::grabMouse(KstViewObjectPtr me) {
  if (_mouseGrabbed) {
    return false;
  }
  _mouseGrabbed = true;
  _mouseGrabber = me;
  return true;
}


void KstTopLevelView::releaseMouse(KstViewObjectPtr me) {
  if (_mouseGrabbed && _mouseGrabber == me) {
    _mouseGrabbed = false;
    _mouseGrabber = 0L;
  }
}


KstViewWidget *KstTopLevelView::widget() const {
  return _w;
}


void KstTopLevelView::cleanupAction() {
  cleanup();
}


void KstTopLevelView::release() {
  _hoverFocus = 0L;
  _pressTarget = 0L;
  _mouseGrabber = 0L;
  _mouseGrabbed = false;
  _selectionList.clear();
  clearChildren(); // FIXME: remove this once we get rid of _parent
  if (_w) {
    _w->release();
  }
}


QSize KstTopLevelView::averageChildSize() {
  int widths = 0, heights = 0;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    widths += (*i)->geometry().width();
    heights += (*i)->geometry().height();
  }
  return _children.count() > 0 ? QSize(widths / _children.count(), heights / _children.count()) : QSize(0, 0);
}


#include "ksttoplevelview.moc"
// vim: ts=2 sw=2 et
