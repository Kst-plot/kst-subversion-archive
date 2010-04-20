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

#include <QBitmap>
#include <QInputDialog>
#include <QMetaObject>
#include <QMetaProperty>
#include <QTextDocument>

#include "kst.h"
#include "kst2dplot.h"
#include "kstaccessibility.h"
#include "kstdoc.h"
#include "ksteditviewobjectdialog.h"
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
#include "plotmimesource.h"

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
  _isResizable = true;
  _maintainAspect = false;
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
  _isResizable = true;
  _maintainAspect = false;
  setMinimumSize(DEFAULT_MINIMUM_SIZE);
  load(e);
}


KstViewObject::KstViewObject(const KstViewObject& viewObject)
: KstObject() {
  _parent = 0L;
  _newTitle = viewObject._newTitle;
  _editTitle = viewObject._editTitle;
  _foregroundColor = viewObject._foregroundColor;
  _backgroundColor = viewObject._backgroundColor;
  _aspect = viewObject._aspect;
  _margins = viewObject._margins;
  _idealSize = viewObject._idealSize;
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
  _isResizable = viewObject._isResizable;
  _maintainAspect = viewObject._maintainAspect;
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
        setTagName(KstObjectTag::fromString(el.text()));
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
      } else if (el.tagName() == "idealsize") {
        _idealSize.setWidth(el.attribute("w", "1").toInt());
        _idealSize.setHeight(el.attribute("h", "1").toInt());
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
        Kst2DPlotMap *pmap = KstApp::inst()->plotHolderWhileOpeningDocument();
        if (pmap->count(in_tag) > 0) {
          Kst2DPlotPtr plot;

          plot = (*pmap)[in_tag];
          if (plot) {
            appendChild(plot, true);
            plot->loadChildren(el);
// xxx            pmap->removeAll(in_tag);
          }
        }
      } else if (el.tagName() == "PlotGroup" || el.tagName() == "plotgroup" /* 1.1 support */) {
        KstPlotGroupPtr plotGroup;

        plotGroup = new KstPlotGroup(el);
        appendChild(plotGroup, true);
        plotGroup->loadChildren(el);
      } else if (el.tagName() == "Box") {
        KstViewBoxPtr box;
        
        box = new KstViewBox(el);
        appendChild(box, true);
        box->loadChildren(el);
      } else if (el.tagName() == "Arrow") {
        KstViewArrowPtr arrow;

        arrow = new KstViewArrow(el);
        appendChild(arrow, true);
        arrow->loadChildren(el);
      } else if (el.tagName() == "Line") {
        KstViewLinePtr line;

        line = new KstViewLine(el);
        appendChild(line, true);
        line->loadChildren(el);
      } else if (el.tagName() == "Ellipse") {
        KstViewEllipsePtr ellipse;

        ellipse = new KstViewEllipse(el);
        appendChild(ellipse, true);
        ellipse->loadChildren(el);
      } else if (el.tagName() == "Label") {
        KstViewLabelPtr label;

        label = new KstViewLabel(el);
        appendChild(label, true);
        label->loadChildren(el);
      } else if (el.tagName() == "Legend") {
        KstViewLegendPtr legend;

        legend = new KstViewLegend(el);
        appendChild(legend, true);
        legend->loadChildren(el);
      } else if (el.tagName() == "Picture") {
        KstViewPicturePtr picture;

        picture = new KstViewPicture(el);
        appendChild(picture, true);
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
  KstViewObjectList::iterator i;

  for (i = _children.begin(); i != _children.end(); ++i) {
    if (rc == NO_CHANGE) {
      rc = (*i)->update(counter);
    } else {
      (*i)->update(counter);
    }
  }

  return rc;
}


void KstViewObject::save(QTextStream& ts, const QString& indent) {
  KstViewObjectList::iterator i;

  saveAttributes(ts, indent);

  for (i = _children.begin(); i != _children.end(); ++i) {
    (*i)->save(ts, indent);
  }
}


void KstViewObject::saveAttributes(QTextStream& ts, const QString& indent) {
  KstAspectRatio aspect;

  if (_maximized) {
    aspect = _aspectOldZoomedObject;
  } else {
    aspect = _aspect;
  }

  if (transparent()) {
    ts << indent << "<transparent/>" << endl;
  }
  ts << indent << "<tag>" << Qt::escape(tagName()) << "</tag>" << endl;
  ts << indent << "<aspect x=\"" << aspect.x <<
    "\" y=\"" << aspect.y <<
    "\" w=\"" << aspect.w <<
    "\" h=\"" << aspect.h << "\" />" << endl;

  ts << indent << "<idealsize w=\"" << _idealSize.width() <<
    "\" h=\"" << _idealSize.height() <<"\" />" << endl;

  //
  // save all properties...
  //

  for (int i = 0; i < metaObject()->propertyCount(); i++) {
    ts << indent << "<" << metaObject()->property(i).name() << ">";
    if (metaObject()->property(i).type() == QVariant::String) {
      ts << Qt::escape(property(metaObject()->property(i).name()).toString());
    } else {
      ts << property(metaObject()->property(i).name()).toString().toLatin1();
    }
    ts << "</" << metaObject()->property(i).name() << ">" << endl;
  }
}


void KstViewObject::paint(KstPainter& p, const QRegion& bounds) {
  bool maximized = false;

  if (p.type() == KstPainter::P_EXPORT || p.type() == KstPainter::P_PRINT) {
    // handle the case where we have maximized plots
    for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
      if ((*i)->_maximized) {
        (*i)->paint(p, bounds);
        maximized = true;

        break;
      }
    }

    if (!maximized) {
      paintSelf(p, bounds);
      for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
        (*i)->paint(p, bounds);
      }
    }

    return;
  }

  p.save();
  p.setViewport(geometry());
  p.setWindow(geometry());
  paintUpdate();

  KstViewObjectList::iterator i;
  bool nullBounds = bounds.isEmpty();

  //
  // handle the case where we have maximized plots...
  //

  for (i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->_maximized) {
      (*i)->paint(p, bounds);
      maximized = true;

      break;
    }
  }

  if (!maximized) {
    QRegion clipRegion;

    if (nullBounds) {
      clipRegion = geometry();
    } else {
      clipRegion = bounds;
    }

    if (!_children.isEmpty()) {
      KstViewObjectList::iterator begin = _children.begin();
      KstViewObjectList::iterator i = _children.end();
      QRegion thisObjectGeometry;

      while (true) {
        --i;

        thisObjectGeometry = (*i)->geometry();

        if (nullBounds || !clipRegion.intersect(thisObjectGeometry).isEmpty()) {
          (*i)->paint(p, clipRegion);
          clipRegion -= (*i)->clipRegion();
        }

        if (i == begin) {
          break;
        }
      }
    }

    paintSelf(p, clipRegion - p.uiMask());
  }

  p.restore();

  //
  // draw any inline UI items...
  //

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

// xxx  p.flush();
}


void KstViewObject::paintSelf(KstPainter& p, const QRegion& bounds) {
  if (!bounds.isEmpty()) {
    p.setClipRegion(bounds);
  }
}


void KstViewObject::updateSelf() {
  if (dirty()) {
    if (!_maximized) {
      updateFromAspect();
    }
    invalidateClipRegion();
  }
}


void KstViewObject::paintUpdate() {
  updateSelf();
  setDirty(false);
}


void KstViewObject::drawFocusRect(KstPainter& p) {
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

  //
  // draw the eight hotpoints...
  //

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
  p.setBrush(QBrush(foregroundColor()));
  p.setPen(QPen(backgroundColor(), 0));
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
  bool rc = _children.removeAll(obj) > 0;

  if (recursive) {
    KstViewObjectList::iterator i;
  
    for (i = _children.begin(); i != _children.end(); ++i) {
      rc = (*i)->removeChild(obj, true) && rc;
    }
  }

  obj->_parent = 0L;

  return rc;
}


void KstViewObject::insertChildAfter(const KstViewObjectPtr after, KstViewObjectPtr obj, bool keepAspect) {
  KstViewObjectList::iterator i;
  bool inserted = false;

  for (i = _children.begin(); i != _children.end(); ++i) {
    if (*i == after) {
      _children.insert(i, obj);
      inserted = true;

      break;
    }
  }

  if (!inserted) {
    _children.prepend(obj);
  }

  obj->_parent = this;

  for (i = _children.begin(); i != _children.end(); ++i) {
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


KstViewObjectList KstViewObject::findChildrenType( const QString& type, bool recursive) {
  KstViewObjectList::Iterator i;
  KstViewObjectList rc;

  for (i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->type().compare(type) == 0) {
      rc.append(*i);
    }

    if (recursive) {
      rc += (*i)->findChildrenType(type, recursive);
    }
  }

  return rc;
}


void KstViewObject::clearChildren() {
  KstViewObjectList::iterator i;

  for (i = _children.begin(); i != _children.end(); ++i) {
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
  KstViewObjectList::iterator i;
  double xc = _aspect.x;
  double yc = _aspect.y;

  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));
  _geom.setSize(size.expandedTo(_minimumSize));
  updateAspect();
  _aspect.x = xc; // restore position.
  _aspect.y = yc;
  updateFromAspect();
  for (i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResized();
  }
}


void KstViewObject::resizeForPrint(const QSize& size) {
  KstViewObjectList::iterator i;

  _geomOld = _geom;
  _geom.setSize(size);
  invalidateClipRegion();
  for (i = _children.begin(); i != _children.end(); ++i) {
    (*i)->parentResizedForPrint();
  }
}


void KstViewObject::revertForPrint() {
  KstViewObjectList::Iterator i;

  _geom = _geomOld;
  invalidateClipRegion();
  for (i = _children.begin(); i != _children.end(); ++i) {
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
  static const QRect x(0, 0, 0, 0);

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
  //
  // if cols <= 0, the optimal value is chosen automatically
  //
  KstViewObjectList childrenCopy;
  double widthTotal = 0.0;
  double widthAverage = 0.0;

  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->maximized()) {
      (*i)->setMaximized(false);
      (*i)->recursively<bool>(&KstViewObject::setMaximized, false);
    }

    if ((*i)->followsFlow()) {
      childrenCopy.append(*i);
      widthTotal += (*i)->aspectRatio().w;
    }
  }

  int cnt = childrenCopy.count();

  if (cnt > 0) {
    widthAverage = widthTotal / double(cnt);

    // FIXME: don't allow regrid to a number of columns that will result in
    //        >= height() plots in a column
    if (cols <= 0) {
      if (widthAverage > 0.0) { // guess current column alignment based on the average width of existing plots
        cols = int(1.0 / widthAverage + 0.5);
        if (cols > cnt) {
          cols = int(sqrt(cnt));
        }
      } else {
        cols = int(sqrt(cnt));
      }
    }

    if (!_onGrid) {
      _onGrid = true;
      _columns = qMax(1, cols);
      cleanupRandomLayout(_columns, childrenCopy);
    } else {
      if (cols > 0) {
        _columns = cols;
      } else if (cols <= 0) {
        _columns = qMax(1, int(sqrt(cnt)));
      }
      cleanupGridLayout(_columns, childrenCopy);
    }
  }
}


void KstViewObject::cleanupGridLayout(int cols, KstViewObjectList &childrenCopy) {
  int cnt = childrenCopy.count();

  if (cols > 0 && cnt > 0) {
    KstViewObjectList::iterator it;
    QSize sz;
    int row = 0;
    int col = 0;
    int rows = ( cnt + cols - 1 ) / cols;
    int marginLeftPixels = 0;
    int marginRightPixels = 0;
    int marginTopPixels = 0;
    int marginBottomPixels = 0;

    if (_margins.left >= 0.0 && _margins.right >= 0.0 ) {
      if (_margins.left + _margins.right < 0.5) {
        marginLeftPixels = (int)((double)_geom.width() * _margins.left);
        marginRightPixels = (int)((double)_geom.width() * _margins.right);
      }
    }

    if (_margins.top >= 0.0 && _margins.bottom >= 0.0 ) {
      if (_margins.top + _margins.bottom < 0.5) {
        marginTopPixels = (int)((double)_geom.height() * _margins.top);
        marginBottomPixels = (int)((double)_geom.height()* _margins.bottom);
      }
    }

    sz.setWidth((_geom.width() - marginLeftPixels - marginRightPixels) / cols);
    sz.setHeight((_geom.height() - marginTopPixels - marginBottomPixels) / rows);

    for (it = childrenCopy.begin(); it != childrenCopy.end(); ++it) {
      KstViewObjectPtr plot;
      QPoint pt(marginLeftPixels + (sz.width() * col), marginTopPixels + (sz.height() * row));

      plot = *it;
      plot->move(pt);
      plot->resize(sz);
      plot->setDirty();

      row++;
      if (row >= rows) {
        col++;
        row = 0;
      }
    }
  }
}


void KstViewObject::cleanupRandomLayout(int cols, KstViewObjectList &childrenCopy) {
  int cnt = childrenCopy.count();

  if (cols > 0 && cnt > 0) {
    KstViewObjectList::iterator it;
    int rows = ( cnt + cols - 1 ) / cols;
    QVector<int> plotLoc(rows * cols); // what plot lives at each grid location
    QVector<int> unAssigned(cnt); // what plots haven't got a home yet?
    int n_unassigned = 0;
    int row;
    int col;
    int CR;
    int i;

    for (i = 0; i < rows * cols; ++i) {
      plotLoc[i] = -1;
    }

    //
    // put the plots on a grid. Each plot goes to the closest grid location
    // unless there is another plot which is closer, in which case the 
    // plot gets dumped into an un-assigned list for placement into a
    // random un-filled grid space.
    // the location is defined relative to the left-middle.
    // QUESTION: should we use the middle-middle instead?
    // NOTE: the choice of grid location assumes a regular grid, which is
    // broken when supressed axis/labels are taken into account.  This
    // could have an effect if the plots are grown by >50%.
    //

    for (it = childrenCopy.begin(); it != childrenCopy.end(); ++it) {
      row = int( ( (*it)->aspectRatio().y + (*it)->aspectRatio().h / 2 ) * rows );
      col = int( (*it)->aspectRatio().x * cols + 0.5 );

      if (col >= cols) {
        col = cols - 1;
      }

      if (row >= rows) {
        row = rows - 1;
      }

      CR = col + row * cols;

      if ((*it)->dirty()) {
        //
        // newly added plots get no priority...
        //

        unAssigned[n_unassigned] = i;
        n_unassigned++;
      } else if (plotLoc[CR] < 0) {
        plotLoc[CR] = i;
      } else {
        KstViewObjectList::iterator itSub;

        //
        // another plot is already at this grid point, so we
        //  put the further of the two in the unassigned list 
        //  using Manhattan distance.
        //

        double d1;
        double d2;
        int index = 0;

        d1 = fabs(double(row) - (*it)->aspectRatio().y*rows) + 
             fabs(double(col) - (*it)->aspectRatio().x*cols);
        d2 = d1 + 1.0;

        for (itSub = childrenCopy.begin(); itSub != childrenCopy.end(); ++itSub) {
          if (index == plotLoc[CR]) {
            d2 = fabs(double(row) - (*itSub)->aspectRatio().y*rows) + 
                 fabs(double(col) - (*itSub)->aspectRatio().x*cols);

            break;
          }
          index++;
        }

        if (d1 >= d2) {
          unAssigned[n_unassigned] = i;
        } else {
          unAssigned[n_unassigned] = plotLoc[CR];
          plotLoc[CR] = i;
        }
        n_unassigned++;
      }
    }

    //
    // now dump the unassigned plots in random holes...
    //

    CR = 0;
    for (i = 0; i < n_unassigned; ++i) {
      for (; plotLoc[CR] != -1; ++CR) { }
      plotLoc[CR] = unAssigned[i];
    }

    QVector<double> HR(rows);
    KstViewObject *ob;
    double sum_HR = 0.0;
    double hr;
    int marginLeftPixels = 0;
    int marginRightPixels = 0;
    int marginTopPixels = 0;
    int marginBottomPixels = 0;

    for (row=0; row<rows; row++) {
      HR[row] = 10.0;

      for (col=0; col<cols; col++) {
        CR = col + row*cols;

        if (plotLoc[CR] > -1) {
/* xxx
          hr = childrenCopy[plotLoc[CR]]->verticalSizeFactor();
          if (hr < HR[row]) {
            HR[row] = hr;
          }
*/
        }
      }
      if (HR[row] > 9.0) {
        HR[row] = 1.0;
      }
      sum_HR += HR[row];
    }

    if (_margins.left >= 0.0 && _margins.right >= 0.0 ) {
      if (_margins.left + _margins.right < 0.5) {
        marginLeftPixels = (int)((double)_geom.width() * _margins.left);
        marginRightPixels = (int)((double)_geom.width() * _margins.right);
      }
    }

    if (_margins.top >= 0.0 && _margins.bottom >= 0.0 ) {
      if (_margins.top + _margins.bottom < 0.5) {
        marginTopPixels = (int)((double)_geom.height() * _margins.top);
        marginBottomPixels = (int)((double)_geom.height() * _margins.bottom);
      }
    }

    //
    // now actually move/resize the plots
    //

    int w = (_geom.width() - marginLeftPixels - marginRightPixels)/ cols;
    int h = 0;
    int y = marginTopPixels;

    for (row=0; row<rows; row++) {
      y += h;
      h = int(double((_geom.height() - marginTopPixels - marginBottomPixels) * HR[row]/sum_HR));
      for (col=0; col<cols; col++) {
        CR = col + row*cols;
        if (plotLoc[CR] >= 0) {
          QSize sz(w, h);

          row = CR / cols;
          col = CR % cols;
          QPoint pt((w*col) + marginLeftPixels, y);

          //
          // if necessary adjust the last column so that we don't spill over...
          //

          if (col == cols-1) {
            //
            // only adjust the final width if necessary as we would rather 
            //  have a gap at the right edge of the window than a column 
            //  of plots that is significantly wider than all the others
            //

            if (w*cols > _geom.width() - marginRightPixels) {
              sz.setWidth(_geom.width() - (w*col) - marginLeftPixels -marginRightPixels);
            }
          }

          //
          // if necessary adjust the last row so that we don't spill over...
          //

          if (row == rows - 1) {
            //
            // only adjust the final height if necessary as we would rather 
            //  have a gap at the bottom edge of the window than a row of 
            //  plots that is significantly taller than all the others
            //

            if (y + h > _geom.height() - marginBottomPixels) {
              sz.setHeight(_geom.height() - marginBottomPixels - y);
            }
          }
/* xxx
          ob = childrenCopy[plotLoc[CR]];
          ob->move(pt);
          ob->resize(sz);

          // FIXME: This is here to trigger axis alignment updates when cleanup
          // happens.  Remove this once we can trigger an axis alignment update
          // without setting dirty since this is a performance penalty.
          // (It even causes non-plots to redraw)

          ob->setDirty();
*/
        }
      }
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

  invalidateClipRegion(); // not dirty, but need to rebuild
}


void KstViewObject::modifyGeometry(const QRect& rect) {
  QRect geomOld = _geom;

  _geom = rect;
  updateAspectPos();
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->updateAspect();
  }
}


void KstViewObject::setFocus(bool focus) {
  _focus = focus;
}


bool KstViewObject::focused() const {
  return _focus;
}


bool KstViewObject::maintainAspect() const {
  return _maintainAspect;
}


void KstViewObject::setMaintainAspect(bool maintain) {
  _maintainAspect = maintain;
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


KstViewObjectPtr KstViewObject::findDeepestChild(const QPoint& pos, bool borderForTransparent, bool stopAtGroup) {
  KstViewObjectPtr obj = findChild(pos, borderForTransparent);
  if (obj) {
    if (!stopAtGroup || obj->type().compare("PlotGroup") != 0) {
      KstViewObjectPtr c;

      do {
        c = obj->findDeepestChild(pos, borderForTransparent, stopAtGroup);
        if (c) {
          obj = c;
          if (stopAtGroup && obj->type().compare("PlotGroup") == 0) {
            break;
          }
        }
      } while (c);
    }
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
          } else if ((*i)->region().contains(pos)) {
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
    if ((*i)->isContainer() && (*i)->surroundingGeometry().contains(rect)) {
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


double KstViewObject::verticalSizeFactor() {
  return 1.0;
}


double KstViewObject::horizontalSizeFactor() {
  return 1.0;
}


QString KstViewObject::menuTitle() const {
  return tagName();
}


bool KstViewObject::popupMenu(QMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  Q_UNUSED(pos)

  QString menuTitle = this->menuTitle();
  QAction *action;
  bool rc = false;
  int id;

  _topObjectForMenu = topParent.data();

  if (!menuTitle.isEmpty()) {
    menu->setTitle(menuTitle);
  }

  if (_standardActions & Edit) {
    menu->addAction(i18n("&Edit..."), this, SLOT(edit()));
    rc = true;
  }

  if (_standardActions & Delete) {
    menu->addAction(i18n("&Delete"), this, SLOT(deleteObject()));
    rc = true;
  }

  if (_layoutActions & Rename) {
    menu->addAction(i18n("Re&name..."), this, SLOT(rename()));
    rc = true;
  }


  if (_standardActions & Zoom) {
    action = menu->addAction(i18n("Maximi&ze"), this, SLOT(zoomToggle()));
    if (_maximized) {
      action->setChecked(true);
    }
    rc = true;
  }

  if (_standardActions & Pause) {
    action = menu->addAction(i18n("&Pause"), this, SLOT(pauseToggle()));
    if (KstApp::inst()->paused()) {
      action->setChecked(true);
    }
    rc = true;
  }

  return rc;
}


bool KstViewObject::layoutPopupMenu(QMenu *menu, const QPoint& pos, KstViewObjectPtr topParent) {
  Q_UNUSED(pos)

  _topObjectForMenu = topParent.data();

  QAction *action;
  bool rc = false;
  int id;
  int index;

  _moveToMap.clear();

  if (!tagName().isEmpty()) {
    menu->setTitle(tagName());
  }

  if (_layoutActions & Edit) {
    menu->addAction(i18n("&Edit..."), this, SLOT(edit()));
    rc = true;
  }

  if (_layoutActions & Delete) {
    menu->addAction(i18n("&Delete"), this, SLOT(deleteObject()));
    rc = true;
  }

  if (_layoutActions & Rename) {
    menu->addAction(i18n("Re&name..."), this, SLOT(rename()));
    rc = true;
  }

  if (_layoutActions & Raise) {
    action = menu->addAction(i18n("&Raise"), this, SLOT(raise()));
    rc = true;
    if (_parent && !_parent->_children.empty() && _parent->_children.last().data() == this) {
      action->setEnabled(false);
    }
  }

  if (_layoutActions & Lower) {
    action = menu->addAction(i18n("&Lower"), this, SLOT(lower()));
    rc = true;
    if (_parent && !_parent->_children.empty() && _parent->_children.first().data() == this) {
      action->setEnabled(false);
    }
  }

  if (_layoutActions & RaiseToTop) {
    action = menu->addAction(i18n("Raise to &Top"), this, SLOT(raiseToTop()));
    rc = true;
    if (_parent && !_parent->_children.empty() && _parent->_children.last().data() == this) {
      action->setEnabled(false);
    }
  }

  if (_layoutActions & LowerToBottom) {
    action = menu->addAction(i18n("Lower to &Bottom"), this, SLOT(lowerToBottom()));
    rc = true;
    if (_parent && !_parent->_children.empty() && _parent->_children.first().data() == this) {
      action->setEnabled(false);
    }
  }

  if (_layoutActions & MoveTo) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator it;
    QAction *action;
    QMenu *submenu = new QMenu(menu);
    bool hasEntry = false;
    int i = 0;

    action = menu->addMenu(submenu);
    submenu->setTitle(i18n("&Move To"));
    
    windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);
  
    for (it = windows.constBegin(); it != windows.constEnd(); ++it) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*it);
      if (viewWindow) {
        KstTopLevelViewPtr tlv;

        tlv = kst_cast<KstTopLevelView>(topParent);
        if (viewWindow && (!tlv || tlv != viewWindow->view())) {
          hasEntry = true;
          submenu->addAction((*it)->windowTitle(), this, SLOT(moveTo(int)));
          _moveToMap[i] = (*it)->windowTitle();
          i++;
        }
      }
    }

    action->setEnabled(hasEntry);

    rc = true;
  }

  if (_layoutActions & CopyTo) {
    KstTopLevelViewPtr tlv;
    QAction *action;
    QMenu *submenu = new QMenu(menu);
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator it;
    bool hasEntry = false;
    int i = 0;
    
    tlv = kst_cast<KstTopLevelView>(topParent);

    action = menu->addMenu(submenu);
    submenu->setTitle(i18n("&Copy To"));

    for (it = windows.constBegin(); it != windows.constEnd(); ++it) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*it);
      if (viewWindow) {
        hasEntry = true;
        if (tlv && tlv == viewWindow->view()) {
          submenu->addAction(i18n("%1 (here)").arg((*it)->windowTitle()), this, SLOT(copyTo(int)));
        } else {
          submenu->addAction((*it)->windowTitle(), this, SLOT(copyTo(int)));
        }
        _copyToMap[i] = (*it)->windowTitle();
        i++;
      }
    }

    action->setEnabled(hasEntry);

    rc = true;
  }

  return rc;
}


void KstViewObject::edit() {
  KstTopLevelViewPtr tlv;

  tlv = kst_cast<KstTopLevelView>(KstViewObjectPtr(_topObjectForMenu));
  showDialog(tlv, false);
}


void KstViewObject::deleteObject() {
  KstApp::inst()->document()->setModified();
  KstViewObjectPtr vop(this);

  if (_topObjectForMenu) {
    KstTopLevelViewPtr tlv;

    tlv = kst_cast<KstTopLevelView>(KstViewObjectPtr(_topObjectForMenu));
    if (tlv && vop == tlv->pressTarget()) {
      tlv->clearPressTarget();
    }

    if (this->_parent) {
      this->_parent->invalidateClipRegion();
    }

// xxx    _topObjectForMenu->removeChild(this, true);
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


KstViewObject* KstViewObject::copyObjectQuietly(KstViewObject &parent, const QString& name) const {
  Q_UNUSED(parent)
  Q_UNUSED(name)

  return 0L;
}


KstViewObject* KstViewObject::copyObjectQuietly() const {
  return 0L;
}


void KstViewObject::raiseToTop() {
  if (_parent) {
    KstViewObjectPtr obj;

    obj = this;
    if (_parent->_children.contains(obj)) {
      _parent->_children.removeAll(obj);
      _parent->_children.append(obj);
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


void KstViewObject::lowerToBottom() {
  if (_parent) {
    KstViewObjectPtr obj;

    obj = this;
    if (_parent->_children.contains(obj)) {
      _parent->_children.removeAll(obj);
      _parent->_children.prepend(obj);
      KstApp::inst()->document()->setModified();
      setDirty();
    }
  }
}


void KstViewObject::raise() {
  if (_parent) {
    KstViewObjectPtr obj;
/* xxx
    obj = this;
    if (_parent->_children.contains(obj)) {
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
*/
  }
}


void KstViewObject::lower() {
  if (_parent) {
/* xxx
    KstViewObjectPtr t = this;
    KstViewObjectList::iterator it = _parent->_children.find(t);

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
*/
  }
}


void KstViewObject::remove() {
  KstApp::inst()->document()->setModified();
  KstViewObjectPtr vop(this);
  KstViewObjectPtr tlp;

  tlp = topLevelParent();
  if (tlp) {
    KstTopLevelViewPtr tlv;

    tlv = kst_cast<KstTopLevelView>(tlp);
    if (tlv && vop == tlv->pressTarget()) {
      tlv->clearPressTarget();
    }

    if (this->_parent) {
      this->_parent->invalidateClipRegion();
    }

// xxx    tlp->removeChild(this, true);
    tlp = 0L;
  }

  while (!_children.isEmpty()) {
    removeChild(_children.first());
  }

  vop = 0L; // basically "delete this;"
  QTimer::singleShot(0, KstApp::inst(), SLOT(updateDialogs()));
}


void KstViewObject::moveTo(int id) {
  QString windowName = _moveToMap[id];

  if (_parent && !windowName.isEmpty()) {
/* xxx
    QMdiSubWindow *window = KstApp::inst()->findWindow(windowName);
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(window);

    if (viewWindow) {
      KstViewObjectPtr t = this;
      KstViewObjectList::Iterator it = _parent->_children.find(t);

      if (it != _parent->_children.end()) {
        KstApp::inst()->document()->setModified();
        setDirty();
        _parent->_children.remove(it);
        viewWindow->view()->appendChild(t, true);
        viewWindow->view()->paint(KstPainter::P_PAINT);
      }
    }
*/
  }
}


void KstViewObject::copyTo(int id) {
  QString windowName = _copyToMap[id];

  if (!windowName.isEmpty()) {
/* xxx
    QMdiSubWindow *window = KstApp::inst()->findWindow(windowName);
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(window);
    if (viewWindow) {
      setDirty();
      KstApp::inst()->document()->setModified();
      copyObjectQuietly(*(window->view().data()));
      viewWindow->view()->paint(KstPainter::P_PAINT);
    }
*/
  }
}


void KstViewObject::updateFromAspect() {
  // FIXME: also take into account the maximum minimum child size in our children
  setMinimumSize(minimumSize().expandedTo(QSize(_children.count(), _children.count())));

  const QRect myOldGeom(_geom);

  if (_parent) {
    const QRect geom(_parent->geometry());

    _geom.setLeft(geom.left() + int((_aspect.x * (double)geom.width())+0.5));
    _geom.setTop(geom.top() + int((_aspect.y * (double)geom.height())+0.5));
    _geom.setRight(geom.left() + int(((_aspect.x + _aspect.w) * (double)geom.width()) - 0.5));
    _geom.setBottom(geom.top() + int(((_aspect.y + _aspect.h) * (double)geom.height()) - 0.5));

    if (_maintainAspect == true) {
      QSize maintainingSize(_idealSize);

      maintainingSize.scale(_geom.size(), Qt::KeepAspectRatio);
      _geom.setSize(maintainingSize);
    }
  }

  if (_geom.width() < _minimumSize.width() || _geom.height() < _minimumSize.height()) {
    _geom.setSize(_geom.size().expandedTo(_minimumSize));
  }

  assert(_geom.left() >= 0 && _geom.top() >= 0 && !_geom.size().isNull());

  if (myOldGeom != _geom) {
    setDirty();
  }
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
  _idealSize = _geom.size();
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
// xxx    list.append(this);
  }

  if (!has || (has && matchRecurse)) {
    KstViewObjectList::iterator i;

    for (i = _children.begin(); i != _children.end(); ++i) {
      (*i)->recursivelyQuery(method, list, matchRecurse);
    }
  }
}


void KstViewObject::detach() {
  if (_parent) {
// xxx    _parent->removeChild(this);
    _parent = 0L;
  }
}


void KstViewObject::rename() {
  QString oldName = tagName();
  QString newName;
  bool ok = false;
  bool done;

// xxx  newName = QInputDialog::getText(KstApp::inst()->activeSubWindow(), QObject::tr("Kst"), QObject::tr("Enter a new name for %1:").arg(tagName()), QLineEdit::Normal, tagName(), &ok);

  done = !ok;
  while (!done) {
    setTagName(KstObjectTag(newName+"tmpholdingstring", KstObjectTag::globalTagContext));
/* xxx
    if (KstData::self()->viewObjectNameNotUnique(newName)) {
      newName = QInputDialog::getText(KstApp::inst()->activeSubWindow(), QObject::tr("Kst"), QObject::tr("%1 is not a unique name: Enter a new name for %2:").arg(newName).arg(oldName), QLineEdit::Normal, oldName, &ok);

      done = !ok;
    } else {
      done = ok = true;
    }
*/
  }

  if (ok) {
    setTagName(KstObjectTag(newName, KstObjectTag::globalTagContext)); // FIXME: handle tag context
     KstApp::inst()->updateViewManager(true);
  } else {
    setTagName(KstObjectTag(oldName, KstObjectTag::globalTagContext)); // FIXME: handle tag context
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
  KstViewObjectList::iterator i;

  str << type();
  str << tagName();
  str << _geom << _backgroundColor << _foregroundColor;
  // _parent should not be sent since it is meaningless in a drag context
  str << _standardActions << _layoutActions << _aspect << _idealSize;

  str << _children.count();
  for (i = _children.begin(); i != _children.end(); ++i) {
    str << *i;
  }
}


QDataStream& operator>>(QDataStream& str, KstViewObjectPtr obj) {
  obj->readBinary(str);

  return str;
}


void KstViewObject::readBinary(QDataStream& str) {
  QString tagName;
  uint cc = 0;
  uint i;

  str >> tagName;
  setTagName(KstObjectTag(tagName, KstObjectTag::globalTagContext)); // FIXME: tag context

  // FIXME: rename objects if they cause a namespace conflict
  str >> _geom >> _backgroundColor >> _foregroundColor;
  str >> _standardActions >> _layoutActions >> _aspect >> _idealSize;

  _children.clear();
  str >> cc;

  for (i = 0; i < cc; ++i) {
    KstViewObjectPtr o;
    QString type;
    
    str >> type;
    o = KstViewObjectFactory::self()->createA(type);
    if (o.data()) {
      str >> o;
      appendChild(o, true);
    } else {
      break;
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


bool KstViewObject::paste(QMimeSource* source, KstViewObjectList* list) {
  QStringList plotList;
  QString window;
  bool rc = false;

  if (source && source->provides(PlotMimeSource::mimeType())) {
/* xxx
    QDataStream ds(source->encodedData(PlotMimeSource::mimeType()), QIODevice::ReadOnly);
    KstViewWindow *w;

    ds >> window;
    ds >> plotList;

    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(window));
    if (w && plotList.size() > 0) {
      for (size_t i=0; i<plotList.size(); i++) {
        KstViewObjectPtr copy = w->view()->findChild(plotList[i]);
        KstViewObject *created;

        if (copy) {
          QString plotName;
          bool duplicate = true;
          int number = 0;

          while (duplicate) {
            if (number == 0) {
              plotName = copy->tagName();
            } else if (number == 1) {
              plotName = i18n("%1-copy").arg(copy->tagName());
            } else {
              plotName = i18n("%1-copy%2").arg(copy->tagName()).arg(number);
            }
            number++;
            if (findChild(plotName)) {
              duplicate = true;
            } else {
              duplicate = false;
            }
          }

          created = copy->copyObjectQuietly(*this, plotName);
          if (created) {
            if (list) {
              list->append(created);
            }
          }
        }
      }

      rc = true;
    }
*/
  }

  return rc;
}


bool KstViewObject::isContainer() const {
  return _container;
}


void KstViewObject::selectAll() {
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->setSelected(true);
  }
}


void KstViewObject::unselectAll() {
  KstViewObjectList::iterator i;

  for (i = _children.begin(); i != _children.end(); ++i) {
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


const QString& KstViewObject::editTitle() const {
  return _editTitle;
}


const QString& KstViewObject::newTitle() const {
  return _newTitle;
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


bool KstViewObject::complexObject() const {
  return false;
}


QRegion KstViewObject::clipRegion() {
  if (_clipMask.isEmpty()) {
    if (transparent()) {
      QBitmap bm(_geom.bottomRight().x(), _geom.bottomRight().y());

      if (!bm.isNull()) {
        KstPainter p;

        p.begin(&bm);
        p.setMakingMask(true);
// xxx        p.setViewXForm(true);
        paint(p, QRegion());
// xxx        p.flush();
        p.end();
        _clipMask = QRegion(bm);
      } else {
        _clipMask = QRegion(); // only invalidate our own variable
      }
    } else {
      _clipMask = QRegion(_geom);
    }
  }

  return _clipMask;
}


QRegion KstViewObject::region() {
  return clipRegion();
}


void KstViewObject::setFollowsFlow(bool follow) {
  _followsFlow = follow;
}


bool KstViewObject::followsFlow() const {
  return _followsFlow;
}


void KstViewObject::setDirty(bool dirty) {
  if (dirty) {
    invalidateClipRegion();
  }
  KstObject::setDirty(dirty);
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

  if (_isResizable) {
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
  }

  return direction;
}


bool KstViewObject::showDialog(KstTopLevelViewPtr invoker, bool isNew) {
  bool rc = false;

  if (!_dialogLock) {
    KstEditViewObjectDialog dlg(KstApp::inst());
    KstViewObjectPtr obj;

    if (isNew) {
      dlg.setNew();
    }

    obj = this;

    dlg.showEditViewObjectDialog(obj, invoker);
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
  KstViewObjectList::const_iterator i;

  if (dirty()) {
    return true;
  }

  for (i = _children.begin(); i != _children.end(); ++i) {
    if ((*i)->objectDirty()) {
      return true;
    }
  }

  return false;
}


QWidget *KstViewObject::configWidget(QWidget *parent) {
  Q_UNUSED(parent)

  return 0L;
}


bool KstViewObject::fillConfigWidget(QWidget *w, bool isNew) const {
  Q_UNUSED(w)
  Q_UNUSED(isNew)

  return false;
}


bool KstViewObject::readConfigWidget(QWidget *w, bool editMultipleMode) {
  Q_UNUSED(w)
  Q_UNUSED(editMultipleMode)

  return false;
}


void KstViewObject::connectConfigWidget(QWidget *parent, QWidget *w) const {
  Q_UNUSED(w)
  Q_UNUSED(parent)
}


void KstViewObject::populateEditMultiple(QWidget *w) {
  Q_UNUSED(w)
}

bool KstViewObject::supportsDefaults() {
  return true;
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


bool KstViewObject::isResizable() const {
  return _isResizable;
}


void KstViewObject::invalidateClipRegion() {
  _clipMask = QRegion();
  if (_parent) {
    if (_parent->complexObject() || _parent->transparent()) {
      _parent->invalidateClipRegion();
    }
  }
}


KstViewObjectPtr KstViewObject::parent() const {
  KstViewObjectPtr obj;
    
  obj = static_cast<KstViewObject*>(_parent);

  return obj;
}


KstViewObjectPtr KstViewObject::topLevelParent() const {
  KstViewObjectPtr obj;
  KstViewObject *p = _parent;

  if (p) {
    while (p->_parent) {
      p = p->_parent;
    }
  }

  obj = static_cast<KstViewObject*>(p);

  return obj;
}

#include "kstviewobject.moc"
