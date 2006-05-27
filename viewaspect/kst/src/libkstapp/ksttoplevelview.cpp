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
#include <math.h>

// include files for Qt

// include files for KDE
#include "ksdebug.h"
#include <kdeversion.h>
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif
#include <klocale.h>
#include <kmessagebox.h>

// application specific includes
#include "kst.h"
#include "kst2dplot.h"
#include "kstaccessibility.h"
#include "kstdoc.h"
#include "kstgfxmousehandler.h"
#include "kstplotgroup.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "ksttoplevelview.h"
#include "kstviewline.h"
#include "kstviewobjectfactory.h"
#include "kstviewwidget.h"

#define STICKY_THRESHOLD 10

KstTopLevelView::KstTopLevelView(QWidget *parent, const char *name, WFlags w)
: KstViewObject("TopLevelView"), _w(new KstViewWidget(this, parent, name, w)) {
  _onGrid = true;
  setTagName(name);
  commonConstructor(); 
}


KstTopLevelView::KstTopLevelView(const QDomElement& e, QWidget *parent, const char *name, WFlags w)
: KstViewObject(e), _w(new KstViewWidget(this, parent, name, w)) {
  commonConstructor();
  loadChildren(e);
}


void KstTopLevelView::commonConstructor() {
  _type = "TopLevelView";
  _focusOn = false;
  _pressDirection = -1;
  _moveOffset = QPoint(-1, -1);
  _moveOffsetSticky = QPoint(0, 0);
  _backgroundColor = _w->backgroundColor();
  _mouseGrabbed = false;
  _activeHandler = 0L;
  _mode = Unknown;
  setViewMode(KstApp::inst()->currentViewMode(), KstApp::inst()->currentCreateType());
}


KstTopLevelView::~KstTopLevelView() {
  for (QMap<QString,KstGfxMouseHandler*>::Iterator i = _handlers.begin(); i != _handlers.end(); ++i) {
    delete i.data();
  }
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
    (*i)->save(ts, indent);
  }
}


void KstTopLevelView::resized(const QSize& size) {
  KstViewObject::resize(size);
}


void KstTopLevelView::updateAlignment(KstPainter& p) {
  QRect plotRegion;

  KST::alignment.reset();  
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->internalAlignment(p, plotRegion);
    if (!plotRegion.isNull()) {
      KST::alignment.setPosition((*i)->geometry(), plotRegion);
    }
  }
}


void KstTopLevelView::resize(const QSize& size) {
  _w->resize(size);
  KstViewObject::resize(size);
}


void KstTopLevelView::paint(KstPainter& p, const QRegion& bounds) {
  updateAlignment(p);
#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  KstViewObject::paint(p, bounds);
#ifdef BENCHMARK
  int x = t.elapsed();
  kstdDebug() << " -> Parent class took " << x << "ms" << endl;
#endif
}


void KstTopLevelView::paint(KstPainter::PaintType type) {
  paint(type, QRegion(geometry()));
}


void KstTopLevelView::paint(KstPainter::PaintType type, const QRegion& bounds) {
  KstPainter p(type);
  p.setDrawInlineUI(_mode == LayoutMode && type != KstPainter::P_EXPORT && type != KstPainter::P_PRINT);
  p.begin(_w);
  p.setViewXForm(true);
  // Paint everything else first so that geometries are properly updated.
  paint(p, bounds);

  // now, check what has the focus and repaint the focus rect, as all focus rects are now lost
  if (_hoverFocus) {
    p.setClipping(false);
    p.setRasterOp(Qt::NotROP);
    p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
    p.setBrush(Qt::NoBrush);
    _hoverFocus->drawFocusRect(p);
  }
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
    if (_hoverFocus) {
      KstPainter p;
      p.begin(_w);
      p.setViewXForm(true);
      _hoverFocus->setFocus(false);
      p.setRasterOp(Qt::NotROP);
      p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
      p.setBrush(Qt::NoBrush);
      _hoverFocus->drawFocusRect(p);
      p.end();
      _hoverFocus = 0L;
    }
  }
}


void KstTopLevelView::updateFocus(const QPoint& pos) {
  if (_activeHandler) {
    _activeHandler->updateFocus(this, pos);
    return;  
  }

  if (_mode == DisplayMode || _mode == Unknown || tracking()) {
    return;
  }
  
  //TODO: make this work better with click-select mode
  
  KstViewObjectPtr p = findDeepestChild(pos, false);
  if (p) {
    KstViewObjectPtr p2 = p;
    while (p2->_parent && p2->_parent->_container) {
      p2 = p2->_parent;
    }
    if (p2->_parent && !p2->_parent->_container) {
      p = p2->_parent;
    }
  }
  if (p) {
    if (p->focused()) {
      setCursorFor(pos, p);
      _focusOn = true; // just in case - seems to be false on occasion
      return;
    }
    p->setFocus(true);
    if (_focusOn) { // something else has the focus, clear it
      clearFocus();
    }
    setCursorFor(pos, p);
    KstPainter painter;
    painter.begin(_w);
    painter.setRasterOp(Qt::NotROP);
    painter.setPen(QPen(Qt::black, 0, Qt::SolidLine));
    painter.setBrush(Qt::NoBrush);
    p->drawFocusRect(painter);
    painter.end();
    _focusOn = true;
    _hoverFocus = p;
  } else {
    clearFocus();
  }
}


void KstTopLevelView::setViewMode(ViewMode v, const QString& objectType) {
  KstApp::inst()->slotUpdateDataMsg(QString::null);
  if (_mode == LayoutMode && v != LayoutMode) {
    recursively<bool>(&KstViewObject::setSelected, false);
    clearFocus();
    paint(KstPainter::P_PAINT);
  } else if (_mode == DisplayMode && v != DisplayMode) {
    recursively<bool>(&KstViewObject::setMaximized, false);
  }

  _mode = v;
  
  // change the mouse handler
  if (_mode == CreateMode || _mode == LabelMode) {
    _activeHandler = handlerForObject(objectType);
  } else {
    _activeHandler = 0L;
  }

  _w->setDragEnabled(_mode != DisplayMode && _mode != Unknown);
}


void KstTopLevelView::setCursorFor(const QPoint& pos, KstViewObjectPtr p) {
  // cursor directions are the same for centered resize
  signed direction = p->directionFor(pos) & ~CENTEREDRESIZE;

  if (direction & ENDPOINT) {
    _cursor.setShape(Qt::CrossCursor);  
  } else {
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
  if (_activeHandler) {
    _activeHandler->handlePress(this, pos, shift);
    return true;
  }
  
  _mouseMoved = false;
  
  //kstdDebug() << "HANDLE PRESS" << endl;
  _pressDirection = -1;

  if (_mode != LayoutMode) {
    _pressTarget = 0L;
    return false;
  }  
  
  _pressTarget = findDeepestChild(pos, false);
  if (_pressTarget) {
    KstViewObjectPtr p = _pressTarget;
    while (p->_parent && (p->_parent->_container || kst_cast<KstPlotGroup>((KstViewObjectPtr)p->_parent)) && !kst_cast<KstTopLevelView>((KstViewObjectPtr)p->_parent)) {
      p = p->_parent;
    }
    if (p->_parent && !p->_parent->_container && !kst_cast<KstTopLevelView>((KstViewObjectPtr)p->_parent)) {
      _pressTarget = p->_parent;
    } else if (p && !p->_container) {
      _pressTarget = p;
    }
  }

  if (!_pressTarget) {
    _moveOffset = pos;
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

  assert(_pressTarget);
  
  _pressDirection = _pressTarget->directionFor(pos);
  
  if (shift && _pressDirection < 1) {
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
    paint(KstPainter::P_PAINT);
    return true;
  }

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
  
  // single click selects a single object if it is not part of the current list
  if (!_selectionList.contains(_pressTarget)) {
    _selectionList.clear();
    recursively<bool>(&KstViewObject::setSelected, false);
    _selectionList.append(_pressTarget);
  }
  _pressTarget->setSelected(true);

  _pressTarget->setFocus(false);
  paint(KstPainter::P_PAINT);
  return true;
}


QRect KstTopLevelView::newSize(const QRect& oldSize, int direction, const QPoint& pos, bool keepAspect) {
  QRect r = oldSize;
  double aspect = (double)oldSize.height()/(double)oldSize.width();

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
 
  if (keepAspect) {
    r = correctWidthForRatio(r, aspect, direction);  
  }
  
  int tempRight = r.right();
  int tempLeft = r.left();
  
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
  
  if (keepAspect) {
    r = correctHeightForRatio(r, aspect, direction, tempRight, tempLeft);
  }
  
  return resizeSnapToObjects(r, direction);
}


QRect KstTopLevelView::newSizeCentered(const QRect& oldSize, int direction, const QPoint& pos, bool keepAspect) {
  const QPoint center((oldSize.left() + oldSize.right())/2, (oldSize.top() + oldSize.bottom())/2);
  QRect rect = oldSize;
  double aspect = (double)oldSize.height()/(double)oldSize.width();
  switch (_pressDirection & (UP|DOWN)) {
    case UP:
      if (pos.y() <= center.y()) {
        rect.setTop(pos.y());
        rect.setBottom(2*center.y() - pos.y());
      } else {
        rect.setTop(center.y());
        rect.setBottom(center.y());  
      }
      break;
    case DOWN:
      if (pos.y() >= center.y()) {
        rect.setBottom(pos.y());
        rect.setTop(2*center.y() - pos.y());
      } else {
        rect.setTop(center.y());
        rect.setBottom(center.y());  
      }
    default:
      break;
  }
  
  if (keepAspect) {
    rect = correctWidthForRatio(rect, aspect, direction);
    rect.moveCenter(center);
  }
  
  int tempLeft = rect.left();
  int tempRight = rect.right();
  
  switch (direction & (LEFT|RIGHT)) {
    case LEFT:
      if (pos.x() <= center.x()) {
        rect.setLeft(pos.x());
        rect.setRight(2*center.x() - pos.x());
      } else {
        rect.setLeft(center.x());
        rect.setRight(center.x());  
      }
      break;
    case RIGHT:
      if (pos.x() >= center.x()) {
        rect.setRight(pos.x());
        rect.setLeft(2*center.x() - pos.x());
      } else {
        rect.setRight(center.x());
        rect.setLeft(center.x());  
      }
      break;
  }
  if (keepAspect) {
    rect = correctHeightForRatio(rect, aspect, direction, tempRight, tempLeft);
    rect.moveCenter(center);
  }
  
  return resizeSnapToObjects(rect, direction);
}


bool KstTopLevelView::tiedZoomPrev(const QString& plotName) {
  Kst2DPlotList pl = findChildrenType<Kst2DPlot>(true);
  bool repaint = false;
  for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
    Kst2DPlotPtr p = *i;
    if (p->isTied() && p->tagName() != plotName) {
      if (p->tiedZoomPrev(widget())) {
        repaint = true;
      }
    }
  }
  return repaint;
}


bool KstTopLevelView::tiedZoomMode(int zoom, bool flag, double center, int mode, int modeExtra, const QString& plotName) {
  Kst2DPlotList pl = findChildrenType<Kst2DPlot>(true);
  bool repaint = false;
  for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
    Kst2DPlotPtr p = *i;
    if (p->isTied() && p->tagName() != plotName) {
      p->tiedZoomMode((ZoomType)zoom, flag, center, (KstScaleModeType)mode, (KstScaleModeType)modeExtra);
      repaint = true;
    }
  }
  return repaint;
}


bool KstTopLevelView::tiedZoom(bool x, double xmin, double xmax, bool y, double ymin, double ymax, const QString& plotName) {
  Kst2DPlotList pl = findChildrenType<Kst2DPlot>(true);
  bool repaint = false;
  for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
    Kst2DPlotPtr p = *i;
    if (p->isTied() && p->tagName() != plotName) {
      p->tiedZoom(x, xmin, xmax, y, ymin, ymax);
      repaint = true;
    }
  }
  return repaint;
}


QRect KstTopLevelView::resizeSnapToObjects(const QRect& objGeometry, int direction) {
  QRect r = objGeometry;

  int iXMin = STICKY_THRESHOLD;
  int iYMin = STICKY_THRESHOLD;

  // check for "sticky" borders
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if (_pressTarget != *i) {           
      const QRect rect((*i)->geometry());
      int overlapLo = r.top() > rect.top() ? r.top() : rect.top();
      int overlapHi = r.bottom() < rect.bottom() ? r.bottom() : rect.bottom();
      if (overlapHi - overlapLo > 0) {
        if (direction & LEFT) {
          if (labs(r.left() - rect.left()) < labs(iXMin)) {
            iXMin = r.left() - rect.left();
          } else if (labs(r.left() - rect.right()) < labs(iXMin)) {
            iXMin = r.left() - rect.right();              
          }
        } else if (direction & RIGHT) {
          if (labs(r.right() - rect.left()) < labs(iXMin)) {
            iXMin = r.right() - rect.left();
          } else if (labs(r.right() - rect.right()) < labs(iXMin)) {
            iXMin = r.right() - rect.right();
          }
        }                 
      }

      overlapLo = r.left() > rect.left() ? r.left() : rect.left();
      overlapHi = r.right() < rect.right() ? r.right() : rect.right();
      if (overlapHi - overlapLo > 0) {
        if (direction & UP) {
          if (labs(r.top() - rect.top()) < labs(iYMin)) {
            iYMin = r.top() - rect.top();
          } else if (labs(r.top() - rect.bottom()) < labs(iYMin)) {
            iYMin = r.top() - rect.bottom();              
          }
        } else if (direction & DOWN) {
          if (labs(r.bottom() - rect.top()) < labs(iYMin)) {
            iYMin = r.bottom() - rect.top();
          } else if (labs(r.bottom() - rect.bottom()) < labs(iYMin)) {
            iYMin = r.bottom() - rect.bottom();
          }
        }
      }
    }
  }

  if (labs(iYMin) < STICKY_THRESHOLD) {
    if (direction & UP) {
      r.setTop(r.top() - iYMin);
    } else if (direction & DOWN) {
      r.setBottom(r.bottom() - iYMin);
    }
  }

  if (labs(iXMin) < STICKY_THRESHOLD) {
    if (direction & LEFT) {
      r.setLeft(r.left() - iXMin);
    } else if (direction & RIGHT) {
      r.setRight(r.right() - iXMin);
    }
  }

  return r.normalize();
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


void KstTopLevelView::pressMove(const QPoint& pos, bool shift) {
  if (_activeHandler) {
    _activeHandler->pressMove(this, pos, shift, _geom);  
    return;
  }
  
  // in these cases there is nothing to do         
  if (_mode == DisplayMode || _mode == Unknown) {
    _pressTarget = 0L;
    return;
  }
  
  if (_pressDirection == -1 && _pressTarget) { // menu released
    return;
  }

  if (shift && _moveOffset == QPoint(-1, -1) && _pressDirection < 1) {
    return;
  }
  
  _mouseMoved = true;
  
  // handle as in layout mode
  pressMoveLayoutMode(pos, shift);
}


void KstTopLevelView::pressMoveLayoutMode(const QPoint& pos, bool shift) {
  if (_pressTarget) {
    if (_pressDirection == 0) {
      // moving an object
      pressMoveLayoutModeMove(pos, shift);
      KstApp::inst()->slotUpdateDataMsg(i18n("(x0,y0)-(x1,y1)", "(%1,%2)-(%3,%4)").arg(_prevBand.topLeft().x()).arg(_prevBand.topLeft().y()).arg(_prevBand.bottomRight().x()).arg(_prevBand.bottomRight().y()));
    } else if (_pressTarget->isResizable()) { 
      if (_pressDirection & ENDPOINT) {
        // moving an endpoint of an object
        pressMoveLayoutModeEndPoint(pos, shift);
      } else if (_pressDirection & CENTEREDRESIZE) {
        // resizing an object with fixed center
        pressMoveLayoutModeCenteredResize(pos, shift);
      } else {
        // resizing a rectangular object
        pressMoveLayoutModeResize(pos, shift);
      }
      KstApp::inst()->slotUpdateDataMsg(i18n("(x0,y0)-(x1,y1)", "(%1,%2)-(%3,%4)").arg(_prevBand.topLeft().x()).arg(_prevBand.topLeft().y()).arg(_prevBand.bottomRight().x()).arg(_prevBand.bottomRight().y()));
    }
  } else {
    // selecting objects
    pressMoveLayoutModeSelect(pos, shift);
  }  
}


void KstTopLevelView::pressMoveLayoutModeMove(const QPoint& pos, bool shift) {
  Q_UNUSED(shift)
  
  const QRect old(_prevBand);

  QRect r(_pressTarget->geometry());
  for (KstViewObjectList::ConstIterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    r = r.unite((*i)->geometry());
  }
  const QPoint originalTopLeft(r.topLeft());
  QPoint topLeft(pos - _moveOffset - _pressTarget->geometry().topLeft() + r.topLeft());
  r.moveTopLeft(topLeft);
  _moveOffsetSticky = QPoint(0, 0);
      
  int xMin = STICKY_THRESHOLD;
  int yMin = STICKY_THRESHOLD;

  snapToBorders(&xMin, &yMin, _selectionList, _pressTarget, r);

  if (labs(xMin) < STICKY_THRESHOLD) {
    _moveOffsetSticky.setX(xMin);
    topLeft.setX(topLeft.x() - xMin);
  }
  if (labs(yMin) < STICKY_THRESHOLD) {
    _moveOffsetSticky.setY(yMin);
    topLeft.setY(topLeft.y() - yMin);
  }

  r.moveTopLeft(topLeft);

  if (!_geom.contains(r, true)) {
    slideInto(_geom, r);
  }
  _prevBand = r;
  if (_prevBand != old) {
    KstPainter p;
        
    p.begin(_w);
    p.setRasterOp(Qt::NotROP);
    p.setPen(QPen(Qt::black, 0, Qt::DotLine));
    if (_selectionList.isEmpty()) {
      if (old.topLeft() != QPoint(-1, -1)) {
        _pressTarget->drawShadow(p, old.topLeft());  
      }
      _pressTarget->drawShadow(p, r.topLeft());
    } else {
      for (KstViewObjectList::Iterator iter = _selectionList.begin(); iter != _selectionList.end(); ++iter) {
        if (old.topLeft() != QPoint(-1, -1)) {
          (*iter)->drawShadow(p, old.topLeft() + (*iter)->geometry().topLeft() - originalTopLeft);
        }
        (*iter)->drawShadow(p, r.topLeft() + (*iter)->geometry().topLeft() - originalTopLeft);
      }
    }
    p.end();
  }
}


void KstTopLevelView::pressMoveLayoutModeResize(const QPoint& pos, bool shift) {
  const QRect old(_prevBand);
  _prevBand = newSize(_pressTarget->geometry(), _pressDirection, pos, shift).intersect(_pressTarget->_parent->_geom);
  if (_prevBand != old) {
    KstPainter p;
        
    p.begin(_w);
    p.setRasterOp(Qt::NotROP);
    p.setPen(QPen(Qt::black, 0, Qt::DotLine));
    if (old.topLeft() != QPoint(-1, -1)) {
      p.drawRect(old);
    } 
    p.drawRect(_prevBand);
    p.end();
  }
}


void KstTopLevelView::pressMoveLayoutModeSelect(const QPoint& pos, bool shift) {
  Q_UNUSED(shift)
  
  const QRect old(_prevBand);
  QRect r;
  r.setTopLeft(_moveOffset);
  r.setBottomRight(pos);
  _prevBand = r.normalize().intersect(_geom);
  if (old != _prevBand) {
    KstPainter p;
    p.begin(_w);
    p.setRasterOp(Qt::NotROP);
    p.drawWinFocusRect(old);
    p.drawWinFocusRect(_prevBand);
    p.end();
  }
  KstApp::inst()->slotUpdateDataMsg(QString::null);
}


void KstTopLevelView::pressMoveLayoutModeEndPoint(const QPoint& pos_in, bool shift) {
  // FIXME: remove this!!  Should not know about any specific type
  // for now we only know how to deal with lines 

  QPoint pos = pos_in;

  //pos must be inside the tlv
  pos.setX(QMAX(pos.x(), geometry().left()));
  pos.setX(QMIN(pos.x(), geometry().right()));
  pos.setY(QMIN(pos.y(), geometry().bottom()));
  pos.setY(QMAX(pos.y(), geometry().top()));

  if (KstViewLinePtr line = kst_cast<KstViewLine>(_pressTarget)) {
    const QRect old(_prevBand);
    double aspect;
    if (line->to().x() != line->from().x()) {
      aspect = double(line->to().y() - line->from().y()) / double(line->to().x() - line->from().x());
    } else {
      if (line->to().y() < line->from().y()) {
        aspect = -1.0E300;
      } else {
        aspect = 1.0E300;  
      }
    }
    QPoint fromPoint, toPoint;
    if (_pressDirection & UP) {
      // UP means we are on the start endpoint
      toPoint = line->to();
      if (shift) {
        double absAspect = fabs(aspect);
        if (absAspect < 500 && (double(abs((pos.y() - toPoint.y())/(pos.x() - toPoint.x()))) < aspect || absAspect < 0.1)) {
          fromPoint = QPoint(pos.x(), toPoint.y() + int(aspect*(pos.x() - toPoint.x())));
        } else {
          fromPoint = QPoint(toPoint.x() + int((pos.y() - toPoint.y())/aspect), pos.y());
        }
      } else {
        fromPoint = pos;
      }
    } else if (_pressDirection & DOWN) {
      // DOWN means we are on the end endpoint
      fromPoint = line->from();
      if (shift) {
        double absAspect = fabs(aspect);
        if (absAspect < 500 && (double(abs((pos.y() - toPoint.y())/(pos.x() - toPoint.x()))) < aspect || absAspect < 0.1)) {
          toPoint = QPoint(pos.x(), fromPoint.y() + int(aspect*(pos.x() - fromPoint.x())));
        } else {
          toPoint = QPoint(fromPoint.x() + int((pos.y() - fromPoint.y())/aspect), pos.y());
        }
      } else {
        toPoint = pos;
      }
    } else {
      abort();
    }

    _prevBand.setTopLeft(fromPoint);
    _prevBand.setBottomRight(toPoint);

    if (old != _prevBand) {
      KstPainter p;
      p.begin(_w);
      p.setPen(QPen(Qt::black, 0, Qt::DotLine));
      p.setRasterOp(Qt::NotROP);
      if (old.topLeft() != QPoint(-1, -1)) {
        p.drawLine(old.topLeft(), old.bottomRight());
      } 
      p.drawLine(_prevBand.topLeft(), _prevBand.bottomRight());
      p.end();
    }
  }
}


void KstTopLevelView::pressMoveLayoutModeCenteredResize(const QPoint& pos, bool shift) {
  //centered resize means that the center of the object stays constant
  const QRect old(_prevBand);
  
  _prevBand = newSizeCentered(_pressTarget->geometry(), _pressDirection, pos, shift).intersect(_pressTarget->_parent->_geom);
  if (_prevBand != old) {
    KstPainter p;

    p.begin(_w);
    p.setPen(QPen(Qt::black, 0, Qt::DotLine));
    p.setRasterOp(Qt::NotROP);
    if (old.topLeft() != QPoint(-1, -1)) {
      p.drawEllipse(old);
    } 
    p.drawEllipse(_prevBand);
    p.end();
  }
}


void KstTopLevelView::releasePressLayoutMode(const QPoint& pos, bool shift) {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    if (_pressDirection == 0) {
      // we are moving object(s)
      releasePressLayoutModeMove(pos, shift);
    } else if (_pressDirection & ENDPOINT) {
      // moving an endpoint of an object
      releasePressLayoutModeEndPoint(pos, shift);
    } else if (_pressDirection & CENTEREDRESIZE) {
      // resizing an object whose center is fixed
      releasePressLayoutModeCenteredResize(pos, shift);
    } else {
      // we are resizing rectangular object(s)
      releasePressLayoutModeResize(pos, shift);
    }
    _pressTarget->setFocus(true);   
  } else { 
    // selecting objects using rubber band
    releasePressLayoutModeSelect(pos, shift);
  }
  _pressTarget = 0L;
  _pressDirection = -1;
  _moveOffset = QPoint(-1, -1);
  _moveOffsetSticky = QPoint(0, 0);
}


void KstTopLevelView::releasePressLayoutModeMove(const QPoint& pos, bool shift) {
  Q_UNUSED(shift)

  QRect obj(_pressTarget->geometry());
  const QRect old(obj);

  // the list of other selected objects
  if (!_selectionList.isEmpty()) {
    for (KstViewObjectList::ConstIterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      obj = obj.unite((*i)->geometry());
    } 
  }
  const QPoint objOffset(old.topLeft() - obj.topLeft());

  // do the move
  obj.moveTopLeft(pos - _moveOffset - _moveOffsetSticky - old.topLeft() + obj.topLeft());
  if (!_geom.contains(obj, true)) {
    slideInto(_geom, obj);
  }

  // This is not entirely correct.  findDeepestChild could actually return an
  // object inside the selection list or even presstarget.  We should get
  // something that includes none of those.  This is most likely the parent of
  // the returned object in that case.
  KstViewObjectPtr container = findDeepestChild(obj);
  bool updateViewManager = false;
  
  if (!container) {
    container = this;
  }
  if (container != _pressTarget && !container->children().contains(_pressTarget)) {
    _pressTarget->detach();
    container->appendChild(_pressTarget);
    updateViewManager = true;
  }
  _pressTarget->move(obj.topLeft() + objOffset);
  for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    if (*i != _pressTarget) {
      KstViewObjectPtr thisObj = *i; // ref
      if (container != thisObj && !container->children().contains(thisObj)) {
        thisObj->detach();
        container->appendChild(thisObj);
        updateViewManager = true;
      }
      thisObj->move(_pressTarget->position() + thisObj->geometry().topLeft() - old.topLeft());
    }
  }
  
  if (updateViewManager) {
    KstApp::inst()->updateViewManager(true);
  }
  _onGrid = false;
}


void KstTopLevelView::releasePressLayoutModeResize(const QPoint& pos, bool shift) {
  Q_UNUSED(pos)
  Q_UNUSED(shift)
  
  if (_prevBand.topLeft() != QPoint(-1, -1)) {
    _prevBand = _prevBand.normalize();
    _pressTarget->move(_prevBand.topLeft());
    _pressTarget->resize(_prevBand.size());
  }
  _onGrid = false;
}


void KstTopLevelView::releasePressLayoutModeSelect(const QPoint& pos, bool shift) {
  Q_UNUSED(pos)
  
  KstPainter p;
    
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


void KstTopLevelView::releasePressLayoutModeEndPoint(const QPoint& pos, bool shift) {
  Q_UNUSED(shift)

  if (KstViewLinePtr line = kst_cast<KstViewLine>(_pressTarget)) {
    if (_prevBand.left() != -1 && _prevBand.top() != -1) {
      if (_pressDirection & UP) {
        // UP means we are on the start endpoint
        const QPoint toPoint(line->to());
        QRect band(pos, toPoint);
        band = band.normalize().intersect(geometry());
        if (toPoint == band.topLeft()) {
          line->setFrom(band.bottomRight());
        } else if (toPoint == band.bottomLeft()) {
          line->setFrom(band.topRight());
        } else if (toPoint == band.topRight()) {
          line->setFrom(band.bottomLeft());
        } else {
          line->setFrom(band.topLeft());
        }
      } else if (_pressDirection & DOWN) {
        // DOWN means we are on the end endpoint
        const QPoint fromPoint(line->from());
        QRect band(fromPoint, pos);
        band = band.normalize().intersect(geometry());
        if (fromPoint == band.topLeft()) {
          line->setTo(band.bottomRight());
        } else if (fromPoint == band.bottomLeft()) {
          line->setTo(band.topRight());
        } else if (fromPoint == band.topRight()) {
          line->setTo(band.bottomLeft());
        } else {
          line->setTo(band.topLeft());
        }
      } else {
        abort();
      }
      _onGrid = false;

      // reparent
      QRect obj(_pressTarget->geometry());
      KstViewObjectPtr container = findDeepestChild(obj);

      if (!container) {
        container = this;
      }
      if (container != _pressTarget && !container->children().contains(_pressTarget)) {
        _pressTarget->detach();
        container->appendChild(_pressTarget);
      }
    }
  }
}


void KstTopLevelView::releasePressLayoutModeCenteredResize(const QPoint& pos, bool shift) {
  Q_UNUSED(pos)
  Q_UNUSED(shift)
      
  if (_prevBand.topLeft() != QPoint(-1, -1)) {    
    _prevBand = _prevBand.normalize();    
    _pressTarget->move(_prevBand.topLeft());
    _pressTarget->resize(_prevBand.size());
  }
  _onGrid = false;
}


void KstTopLevelView::releasePress(const QPoint& pos, bool shift) { 
  if (_activeHandler) {
    _activeHandler->releasePress(this, pos, shift);
    return;
  }
  
  // in these cases nothing should be done
  if (_mode == DisplayMode || _mode == Unknown) {
    _pressTarget = 0L;
    return; 
  }

  KstApp::inst()->slotUpdateDataMsg(QString::null);

  if (_pressDirection == -1 && _pressTarget) { // menu released
    _pressTarget = 0L;
    return;
  }

  // we're in layout mode
  releasePressLayoutMode(pos, shift);

  updateFocus(pos);
  
  paint(KstPainter::P_PAINT);
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
  _pressTarget = findDeepestChild(pos, false);
  if (_pressTarget) {
    KstViewObjectPtr p = _pressTarget;
    while (p->_parent && p->_parent->_container) {
      p = p->_parent;
    }
    if (p->_parent && !p->_parent->_container) {
      _pressTarget = p->_parent;
    }
  }

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
      if (rc) {
        menu->insertSeparator();
      }

      KPopupMenu *subMenu = new KPopupMenu(menu);
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

  if (rc) {
    menu->insertSeparator();
  }
  KPopupMenu *subMenu = new KPopupMenu(menu);
  subMenu->insertItem(i18n("Default Tile"), this, SLOT(cleanupDefault()));
  subMenu->insertItem(i18n("Custom..."), this, SLOT(cleanupCustom()));
  menu->insertItem(i18n("Cleanup Layout"), subMenu);
  rc = true;

  // don't release the _pressTarget reference here as we are not yet done with
  //  it, since several of the menu handlers require it in order to function...
  // _pressTarget = 0L; // release reference

  return rc;
}


void KstTopLevelView::makeSameWidth() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    const QRect gg(_pressTarget->geometry());
    QSize size;
    size.setWidth( gg.width() );
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      size.setHeight((*i)->geometry().height());
      (*i)->resize(size);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::makeSameHeight() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    const QRect gg(_pressTarget->geometry());
    QSize size;
    size.setHeight( gg.height() );
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      size.setWidth((*i)->geometry().width());
      (*i)->resize(size);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::makeSameSize() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    QSize size;
    const QRect gg(_pressTarget->geometry());
    size.setHeight(gg.height());
    size.setWidth(gg.width());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      (*i)->resize(size);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::correctPosition(KstViewObjectPtr obj, QPoint point) {
  QRect rect(obj->geometry());
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
    const QRect gg(_pressTarget->geometry());
    QPoint point;
    point.setX(gg.x());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setY((*i)->geometry().y());
      correctPosition(*i, point);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::alignRight() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    const QRect gg(_pressTarget->geometry());
    QPoint point;
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      const QRect g((*i)->geometry());
      point.setX(gg.x() + gg.width() - g.width());
      point.setY(g.y());
      correctPosition(*i, point);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::alignTop() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    const QRect gg(_pressTarget->geometry());
    QPoint point;
    point.setY(gg.y());
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      point.setX((*i)->geometry().x());
      correctPosition(*i, point);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::alignBottom() {
  if (_pressTarget) {
    KstApp::inst()->document()->setModified();
    const QRect gg(_pressTarget->geometry());
    QPoint point;
    for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
      const QRect g((*i)->geometry());
      point.setX(g.x());
      point.setY(gg.y() + gg.height() - g.height());
      correctPosition(*i, point);
    }
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::packVertically() {
  int iTop = 0;
  int iBottom = 0;
  int iCount = 0;

  for (KstViewObjectList::ConstIterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    const QRect g((*i)->geometry());
    if (i == _selectionList.begin()) {
      iTop = g.y();
      iBottom = g.y() + g.height();
    } else {
      if (g.y() < iTop) {
        iTop = g.y();
      }
      if (g.y() + g.height() > iBottom) {
        iBottom = g.y() + g.height();
      }
    }
    ++iCount;
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
        const QRect g((*i)->geometry());
        if (i == selectionList.begin()) {
          iPosMin = g.y();
          temp = i;
        } else if (g.y() < iPosMin) {
          iPosMin = g.y();
          temp = i;
        }
      }
      size.setWidth((*temp)->geometry().width());
      size.setHeight(iHeight);
      (*temp)->resize(size);
      point.setX((*temp)->geometry().x());
      point.setY(iPos);
      correctPosition(*temp, point);

      selectionList.erase(temp);
      iPos += iHeight;
    }
    KstApp::inst()->document()->setModified();
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::packHorizontally() {
  int iLeft = 0;
  int iRight = 0;
  int iCount = 0;

  for (KstViewObjectList::ConstIterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    const QRect g((*i)->geometry());
    if (i == _selectionList.begin()) {
      iLeft = g.x();
      iRight = g.x() + g.width();
    } else {
      if (g.x() < iLeft) {
        iLeft = g.x();
      }
      if (g.x() + g.width() > iRight) {
        iRight = g.x() + g.width();
      }
    }
    ++iCount;
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
        const QRect g((*i)->geometry());
        if (i == selectionList.begin()) {
          iPosMin = g.x();
          temp = i;
        } else if (g.x() < iPosMin) {
          iPosMin = g.x();
          temp = i;
        }
      }
      size.setWidth(iWidth);
      size.setHeight((*temp)->geometry().height());
      (*temp)->resize(size);
      point.setX(iPos);
      point.setY((*temp)->geometry().y());
      correctPosition(*temp, point);

      selectionList.erase(temp);
      iPos += iWidth;
    }
    KstApp::inst()->document()->setModified();
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::groupSelection() {
  KstPlotGroupPtr pg = new KstPlotGroup;
  QRect gg(_selectionList.first()->geometry());

  // First build the container
  for (KstViewObjectList::ConstIterator it = _selectionList.begin(); it != _selectionList.end(); ++it) {
    gg |= (*it)->geometry();
  }

  pg->move(gg.topLeft());
  pg->resize(gg.size());

  // Then add the items
  for (KstViewObjectList::Iterator it = _selectionList.begin(); it != _selectionList.end(); ++it) {
    (*it)->setSelected(false);
    (*it)->setFocus(false);
    (*it)->detach();
    pg->appendChild(*it);
  }

  if (!pg->children().isEmpty()) {
    appendChild(pg.data());
    KstApp::inst()->document()->setModified();
    paint(KstPainter::P_PAINT);
  }
}


void KstTopLevelView::cancelMouseOperations() {
  if (_mode == LayoutMode) {
    clearFocus();
    if (_pressTarget || _prevBand.isValid()) {
      paint(KstPainter::P_PAINT);
    }
    //_pressTarget = 0L;
    _prevBand = QRect(-1, -1, 0, 0);
    return;
  }
  
  // other graphics modes - delete the current drawing object
  if (_activeHandler) {
    _activeHandler->cancelMouseOperations(this);  
  }
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


void KstTopLevelView::cleanupDefault() {
  // roughly layout in a square
  cleanup();
}


void KstTopLevelView::cleanupCustom() {
  bool ok = false;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  int numCols = KInputDialog::getInteger(i18n("Number of Columns"), 
                                         i18n("Select number of columns:"), 
                                         int(sqrt(_children.count())), 
                                         1, 
                                         _children.count(), 
                                         1, 
                                         &ok, 
                                         0L);
  if (ok) {
    cleanup(numCols);
  } 
#else
  for (;;) { 
    QString numColsString = KLineEditDlg::getText(i18n("Enter number of columns:"), i18n("Number of Columns"), &ok, 0L);
    if (ok) {
      unsigned int numCols = numColsString.toInt();
      if (numCols < 1 || numCols > _children.count()) {
        KMessageBox::sorry(KstApp::inst(), i18n("Enter a number of columns between 1 and %d").arg(_selectionList.count()));
      } else {
        cleanup(numCols);
        break;
      }
    } else {
      break;
    }
  }
#endif
}


void KstTopLevelView::release() {
  _hoverFocus = 0L;
  _pressTarget = 0L;
  _mouseGrabber = 0L;
  _mouseGrabbed = false;
  _selectionList.clear();
  clearChildren();
  if (_w) {
    _w->release();
  }
}


QSize KstTopLevelView::averageChildSize() const {
  int widths = 0, heights = 0;
  for (KstViewObjectList::ConstIterator i = _children.begin(); i != _children.end(); ++i) {
    const QRect g((*i)->geometry());
    widths += g.width();
    heights += g.height();
  }
  int c = _children.count();
  return c > 0 ? QSize(widths / c, heights / c) : QSize(0, 0);
}


void KstTopLevelView::saveDefaults(KstViewObjectPtr object) {
  KstGfxMouseHandler *handler = handlerForObject(object->type());
  if (handler) {
    handler->saveDefaults(object);
  }
}


bool KstTopLevelView::handleDoubleClick(const QPoint& pos, bool shift) {
  handlePress(pos, shift);
  if (_pressTarget) {
    _pressTarget->showDialog(this);  
  }  
  return true;
}


void KstTopLevelView::deleteSelectedObjects() {
  for (KstViewObjectList::Iterator i = _selectionList.begin(); i != _selectionList.end(); ++i) {
    KstViewObjectPtr selection = *i;
    if (selection && selection->parent()) {
      selection->parent()->removeChild(selection);
    }
  }
  _selectionList.clear();

  clearFocus();
  paint(KstPainter::P_PAINT);
}


QRect KstTopLevelView::correctWidthForRatio(const QRect& oldRect, double ratio, int direction) {
  QRect r = oldRect;
  if (direction & (UP|DOWN)) {
    int negOne;
    if (r.width() == 0) {
      negOne = 1;  
    } else {
      negOne = r.width()/abs(r.width());  
    }
    int newWidth;
    if (ratio == 0) {
      newWidth = abs(r.width());  
    } else {
      newWidth = (int)(abs(r.height())/ratio);
    }
    if (direction & LEFT) {
      r.setLeft(r.right() - negOne * newWidth);  
    } else if (direction & RIGHT) {
      r.setRight(r.left() + negOne * newWidth);  
    } else {
      r.setLeft((_pressTarget->geometry().left() + _pressTarget->geometry().right())/2 - negOne*newWidth/2);
      r.setRight(r.left() + negOne*newWidth);
    }  
  }
  return r;
}


QRect KstTopLevelView::correctHeightForRatio(const QRect& oldRect, double ratio, int direction, int origRight, int origLeft) {
  QRect r = oldRect;
  if (direction & (LEFT|RIGHT)) {
    int newHeight = (int)(abs(r.width())*ratio);
    int negOne, negOneW;
    if (r.height() == 0) {
      negOne = 1;  
    } else {
      negOne = r.height()/abs(r.height());  
    }
    if (r.width() == 0) {
      negOneW = 1;  
    } else {
      negOneW = r.width()/abs(r.width());
    }
    if (direction & UP) {
      if (newHeight > abs(r.height())) {
        r.setTop(r.bottom() - negOne*newHeight);
      } else {
        if (negOneW < 0) {
          if (direction & RIGHT) {
            r.setLeft(origLeft - (origRight-origLeft));
            r.setRight(origLeft);  
          } else {
            r.setLeft(origRight + (origRight - origLeft));
            r.setRight(origRight);  
          }
        } else {
          r.setLeft(origLeft);
          r.setRight(origRight);  
        } 
      }
    } else if (direction & DOWN) {
      if (newHeight > abs(r.height())) {
        r.setBottom(r.top() + negOne*newHeight);
      } else {
        if (negOneW < 0) {
          if (direction & RIGHT) {
            r.setLeft(origLeft - (origRight-origLeft));
            r.setRight(origLeft);  
          } else {
            r.setLeft(origRight + (origRight - origLeft));
            r.setRight(origRight);  
          }
        } else {
          r.setLeft(origLeft);
          r.setRight(origRight);  
        } 
      }
    } else {
      r.setTop((_pressTarget->geometry().top() + _pressTarget->geometry().bottom())/2 - negOne*newHeight/2);
      r.setBottom(r.top() + negOne*newHeight);
    }  
  }
  return r;
}


KstGfxMouseHandler *KstTopLevelView::handlerForObject(const QString& objType) {
  QMap<QString,KstGfxMouseHandler*>::Iterator i = _handlers.find(objType);
  if (i != _handlers.end()) {
    return i.data();
  }

  KstGfxMouseHandler *rc = KstViewObjectFactory::self()->createHandlerFor(objType);
  if (rc) {
    _handlers[objType] = rc;
  }
  return rc;
}

#include "ksttoplevelview.moc"
// vim: ts=2 sw=2 et
