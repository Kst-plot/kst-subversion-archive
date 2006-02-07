/***************************************************************************
                          kstview.cpp  -  description
                             -------------------
    begin                : Tue Aug 22 13:46:13 CST 2000
    copyright            : (C) 2000 by Barth Netterfield
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
#include <stdio.h>

// include files for Qt
#include <qpainter.h>
#include <qevent.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qtabwidget.h>

#include <kdebug.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kurl.h>

// application specific includes
#include "kstview.h"
#include "kstdoc.h"
#include "kstplotlist.h"
#include "kst.h"
#include "kstplotdialog_i.h"
#include "kstmouse.h"
#include "kstlabeldialog_i.h"
#include "kstdatacollection.h"

/** setup the view widget */
KstView::KstView(KstApp *parent, const char *name)
: QWidget(parent, name) {

  setAcceptDrops(true);

  MouseInfo = new KstMouse(this);
  labelDialog = new KstLabelDialogI(parent, "Edit Label");

  first_time = true;
  _minMove = 3;
  _dataMode = false;

  ZoomCurrentPlot = false;
  ZoomPlotNum = 0;

  ParentApp = parent;
  needrecreate = true;
  qpixmap = 0;
  setBackgroundMode(NoBackground);
  setMouseTracking(true);

  /* initialize the popup menu */
  popupmenu = new KPopupMenu(this);
  QPopupMenu *submenu;
  submenu = new KPopupMenu(this);
  _titleId = popupmenu->insertTitle(i18n("Plot"));
  popupmenu->insertItem(i18n("Z&oom"), submenu);
  submenu->insertItem(i18n("Zoom &Maximum"), this, SLOT(zoomMaxSlot()), Key_M);
  submenu->insertItem(i18n("Zoom Max &Spike Insensivie"),
                      this, SLOT(zoomSpikeInsensiveMaxSlot()), Key_S);
  submenu->insertItem(i18n("Zoom P&revious"), this, SLOT(zoomPrevSlot()), Key_R);
  submenu->insertItem(i18n("Y-Zoom AC Coupled"), this, SLOT(yzoomAcSlot()), Key_A);
  submenu->insertSeparator();
  submenu->insertItem(i18n("X-Zoom Maximum"),
                        this, SLOT(xzoomMaxSlot()), CTRL + Key_M);
  submenu->insertItem(i18n("X-Zoom Out"),
                        this, SLOT(xzoomOutSlot()), SHIFT + Key_Right);
  submenu->insertItem(i18n("X-Zoom In"),
                        this, SLOT(xzoomInSlot()), SHIFT + Key_Left);
  submenu->insertItem(i18n("Toggle Log X Axis"),
                        this, SLOT(xLogSlot()), Key_G);
  submenu->insertSeparator();
  submenu->insertItem(i18n("Y-Zoom Maximum"),
                        this, SLOT(yzoomMaxSlot()), SHIFT + Key_M);
  submenu->insertItem(i18n("Y-Zoom Out"),
                        this, SLOT(yzoomOutSlot()), SHIFT + Key_Up);
  submenu->insertItem(i18n("Y-Zoom In"),
                        this, SLOT(yzoomInSlot()), SHIFT + Key_Down);
  submenu->insertItem(i18n("Toggle Log Y Axis"), this, SLOT(yLogSlot()), Key_L);
  submenu = new KPopupMenu(this);
  popupmenu->insertItem(i18n("&Scroll"), submenu);
  submenu->insertItem(i18n("Left"), this, SLOT(leftSlot()), Key_Left);
  submenu->insertItem(i18n("Right"), this, SLOT(rightSlot()), Key_Right);
  submenu->insertItem(i18n("Up"), this, SLOT(upSlot()), Key_Up);
  submenu->insertItem(i18n("Down"), this, SLOT(downSlot()), Key_Down);

  popupmenu->insertSeparator();
  popupmenu->insertItem(i18n("Toggle &Pause"), this, SLOT(pauseSlot()), Key_P);
  popupmenu->insertItem(i18n("Toggle &Zoom Current Plot"),
                        this, SLOT(zoomPlotSlot()), Key_Z);

  popupmenu->insertSeparator();
  popupmenu->insertItem(i18n("&Edit Plots"), parent,
                        SLOT(showPlotDialog()), Key_E);

  popupmenu->insertItem(i18n("&Delete Plot"), this, SLOT(deleteCurrentPlot()), Key_Delete);
  popupmenu->insertItem(i18n("Clean Up Layout"), this, SLOT(cleanupLayout()));

  _addMenu = new KPopupMenu(this);
  _removeMenu = new KPopupMenu(this);
  _editCurveMenu = new KPopupMenu(this);
  _addMenuId = popupmenu->insertItem(i18n("&Add To Plot"), _addMenu);
  popupmenu->setItemEnabled(_addMenuId, false);
  _removeMenuId = popupmenu->insertItem(i18n("&Remove From Plot"), _removeMenu);
  popupmenu->setItemEnabled(_removeMenuId, false);
  _editCurveMenuId = popupmenu->insertItem(i18n("Edit &Curve"), _editCurveMenu);
  popupmenu->setItemEnabled(_editCurveMenuId, false);

  setFocusPolicy(QWidget::StrongFocus);
  setFocus();
}

KstView::~KstView() {
  delete qpixmap;
  qpixmap = 0L;
  delete MouseInfo;
  MouseInfo = 0L;
}

// FIXME: make this return an error code!
void KstView::printToGraphicsFile(const QString &filename, int in_w, int in_h) {
  float w, h, tlx, tly;
  QRect v;
  KstPlot *pl = 0L;
  double XBorder, maxXBorder = 0;

  QPixmap tmpPixmap(in_w, in_h);
  tmpPixmap.fill();

  QPainter p(&tmpPixmap);

  v = p.viewport();

  // Find max border width
  for (unsigned i = 0; i < KST::plotList.count(); i++) {
    pl = KST::plotList.at(i);
    w = v.width() * pl->width();
    h = v.height() * pl->height();
    tly = v.height() * pl->topLeftY();
    tlx = v.width() * pl->topLeftX();
    p.setViewport((int)tlx, (int)tly, (int)w, (int)h);
    p.setWindow((int)0, (int)0, (int)w, (int)h);
    XBorder = pl->getXBorder(p);
    if (XBorder > maxXBorder)
      maxXBorder = XBorder;
  }

  for (unsigned i = 0; i < KST::plotList.count(); i++) {
    pl = KST::plotList.at(i);
    w = v.width() * pl->width();
    h = v.height() * pl->height();
    tly = v.height() * pl->topLeftY();
    tlx = v.width() * pl->topLeftX();
    p.setViewport((int)tlx, (int)tly, (int)w, (int)h);
    p.setWindow((int)0, (int)0, (int)w, (int)h);
    pl->paint(p, maxXBorder);
  }

  QString type = KImageIO::type(filename);
  if (type.isEmpty()) {
    type = "PNG";
  }

  KURL url = KURL::fromPathOrURL(filename);
  if (url.isLocalFile()) {
    tmpPixmap.save(url.path(), type.latin1());
  } else {
    KTempFile tf;
    tf.close();
    tf.setAutoDelete(true);
    tmpPixmap.save(tf.name(), type.latin1());
#if KDE_IS_VERSION(3,1,90)
    KIO::NetAccess::upload(tf.name(), url, this);
#else
    KIO::NetAccess::upload(tf.name(), url);
#endif
  }
}

void KstView::print(KPrinter *printer)
{
  int i;
  float w, h, tlx, tly;
  QRect v;
  KstPlot *pl;
  double XBorder, maxXBorder = 0;

  QPainter p(printer);

  // Hack: if the plot is empty, at least trigger a postscript header so that
  // we don't get a warning about x-zerosize file not being printable
  if (KST::plotList.isEmpty()) {
    p.moveTo(1,1);
    return;
  }

  v = p.viewport();

  // Find max border width
  for (i = 0; i < (int)KST::plotList.count(); i++) {
    pl = KST::plotList.at(i);
    w = v.width() * pl->width();
    h = v.height() * pl->height();
    tly = v.height() * pl->topLeftY();
    tlx = v.width() * pl->topLeftX();
    p.setViewport((int)tlx, (int)tly, (int)w, (int)h);
    w = 3000.0/h * w;
    h = 3000.0;
    p.setWindow((int)0, (int)0, (int)w, (int)h);
    XBorder = pl->getXBorder(p);
    if (XBorder > maxXBorder)
      maxXBorder = XBorder;
  }

  for (i = 0; i < (int)KST::plotList.count(); i++) {
    pl = KST::plotList.at(i);
    w = v.width() * pl->width();
    h = v.height() * pl->height();
    tly = v.height() * pl->topLeftY();
    tlx = v.width() * pl->topLeftX();
    p.setViewport((int)tlx, (int)tly, (int)w, (int)h);
    w = 3000.0/h * w;
    h = 3000.0;
    p.setWindow((int)0, (int)0, (int)w, (int)h);
    pl->paint(p, maxXBorder);
  }
}

void KstView::paintEvent(QPaintEvent *) {
  unsigned int i;
  QPixmap *tmpPixmap = 0L;
  KstPlot *pl;
  QPainter *p = 0L;
  float w, h, tlx, tly;
  double XBorder, maxXBorder = 0;
  unsigned int i0, i1;
  int n_plots;

  if (qpixmap == 0L) {
    qpixmap = new QPixmap(width(), height());
    qpixmap->setOptimization(QPixmap::MemoryOptim);
    qpixmap->fill();
    erase();
    return;
  }

  n_plots = KST::plotList.count();

  if (n_plots == 0) {
    i0 = 0;
    i1 = 0;
  } else if (ZoomCurrentPlot) {
    i0 = ZoomPlotNum;
    i1 = i0 + 1;
  } else {
    i0 = 0;
    i1 = n_plots;
  }

  if (needrecreate) {
    qpixmap->resize(width(), height());
    qpixmap->fill();

    // Find max border width
    for (i = i0; i < i1; i++) {
      /* Draw each plot to its own pixmap */
      pl = KST::plotList.at(i);
      if (ZoomCurrentPlot) {
        w = width();
        h = height();
        tly = 0;
        tlx = 0;
      } else {
        w = width() * pl->width();
        h = height() * pl->height();
        tly = height() * pl->topLeftY();
        tlx = width() * pl->topLeftX();
      }
      if (i0 == i1) { // only one plot - draw directly onto qpixmap
        tmpPixmap = qpixmap;
        p = new QPainter(tmpPixmap);
      } else if (i == i0) {
        tmpPixmap = new QPixmap(int(w),int(h));
        tmpPixmap->setOptimization(QPixmap::MemoryOptim);
        p = new QPainter(tmpPixmap);
      } else if (tmpPixmap->width() != w || tmpPixmap->height() != h) {
        delete p;
        tmpPixmap->resize(int(w),int(h));
        p = new QPainter(tmpPixmap);
      }

      XBorder = pl->getXBorder(*p);
      if (XBorder > maxXBorder)
        maxXBorder = XBorder;
    }

    for (i = i0; i < i1; i++) {
      /* Draw each plot to its own pixmap */
      pl = KST::plotList.at(i);
      if (ZoomCurrentPlot) {
        w = width();
        h = height();
        tly = 0;
        tlx = 0;
      } else {
        w = width() * pl->width();
        h = height() * pl->height();
        tly = height() * pl->topLeftY();
        tlx = width() * pl->topLeftX();
      }
      if (tmpPixmap->width() != w || tmpPixmap->height() != h) {
        delete p;
        tmpPixmap->resize((int)w,(int)h);
        p = new QPainter(tmpPixmap);
      }
      pl->paint(*p, maxXBorder);
      /* copy the tmppix map to the qpixmap */
      if (i0 != i1) {
        bitBlt(qpixmap, (int)tlx, (int)tly, tmpPixmap, 0, 0, -1, -1,
               QWidget::CopyROP, true);
      }
      pl->setpixrect((int)tlx, (int)tly);
    }
    delete p;
    if (tmpPixmap != qpixmap)
      delete tmpPixmap;
  }

  bitBlt(this, 0, 0, qpixmap, 0, 0, -1, -1, QWidget::CopyROP, true);
  updateTieBoxes(this);

  needrecreate = false;

  // Redraw the mouse box if needed
  if (MouseInfo->getMode() != INACTIVE) {
    QPainter p(this);
    if (!MouseInfo->isFirstMove())
      MouseInfo->DrawBox(p);
  }
}

void KstView::resizeEvent(QResizeEvent *) {
  needrecreate = true;
}


void KstView::update() {
  needrecreate = true;
  paintEvent(NULL);
  updateMouse();
}

void KstView::updateMouse() {
  int plot = MouseInfo->getPlotNum();
  QString msg;
  KstPlot* pPlot;
  if (plot >= 0 && (pPlot = KST::plotList.at(plot))) {
    QRect plot_rect = pPlot->GetPlotRegion();
    QPoint pos = mapFromGlobal(QCursor::pos());
    double xmin, ymin, xmax, ymax, xpos, ypos;
    pPlot->getLScale(xmin, ymin, xmax, ymax);

    xpos = (double)(pos.x() - plot_rect.left())/(double)plot_rect.width();
    xpos = xpos * (xmax - xmin) + xmin;
    if (pPlot->isXLog()) {
      xpos = pow(10.0, xpos);
    }

    ypos = (double)(pos.y() - plot_rect.top())/(double)plot_rect.height();
    ypos = ypos * (ymin - ymax) + ymax;

    if (pPlot->isYLog()) {
      ypos = pow(10.0, ypos);
    }
 
    if (_dataMode) {
      double newypos = ypos;
      KstBaseCurvePtr curve;
      double delta = 9e99;

      for (KstBaseCurveList::Iterator i = pPlot->Curves.begin(); i != pPlot->Curves.end(); ++i) {
        double xpt, ypt;
        (*i)->getPoint(int(xpos), xpt, ypt);
        if (fabs(ypos - ypt) < delta) {
          delta = fabs(ypos - ypt);
          newypos = ypt;
          curve = *i;
        }
      }
      if (curve.data()) {
        msg = i18n("%3 (%1, %2)").arg(int(xpos)).arg(newypos,0,'G').arg(curve->tagName());
        emit newDataMsg(msg);
        return;
      }
    }

    msg = i18n("(%1, %2)").arg(xpos,0,'G').arg(ypos,0,'G');
  }
  emit newDataMsg(msg);
}

void KstView::mouseMoveEvent(QMouseEvent *e) {
  int x,y;
  int i, i0, iN;
  bool in_plot = false;
  int i_label;
  int plot_num = MouseInfo->getPlotNum();

  if (e->state() & Qt::LeftButton &&
        MouseInfo->getMode() == LABEL_TOOL &&
        (i_label = MouseInfo->getLabelNum(e)) >= 0 &&
        _draggableLabel == i_label &&
        plot_num == _draggablePlot) {
    // Start a drag
    KstPlot *plot = KST::plotList.at(plot_num);
    KstLabel *label = plot->labelList.at(i_label);
    QRect oldExtents = label->extents;
    QPixmap pm(plot->GetWinRegion().width(), plot->GetWinRegion().height());
    pm.fill();
    { // Scope is needed to kill off the painter before we resize
      QPainter p(&pm);
      label->draw(p, 2, oldExtents.height() + 2, true);
    }
    pm.resize(oldExtents.width() + 4, oldExtents.height() + 4);
    label->extents = oldExtents; // restore them incase the drag is canceled
    QDragObject *d = new KstLabelDrag(this, plot_num, i_label, _draggablePoint - plot->GetWinRegion().topLeft() - oldExtents.topLeft(), pm);
    d->dragMove();
    _draggableLabel = -1;
    _draggablePlot = -1;
    return;
  } else if (MouseInfo->getMode() == XY_ZOOMBOX) {
    QRect pr = KST::plotList.at(plot_num)->GetPlotRegion();

    if (e->x() > pr.right()) {
      x = pr.right() + 1;
    } else if (e->x() < pr.left()) {
      x = pr.left();
    } else {
      x = e->x();
    }

    if (e->y() > pr.bottom()) {
      y = pr.bottom() + 1;
    } else if (e->y() < pr.top()) {
      y = pr.top();
    } else {
      y = e->y();
    }

    MouseInfo->setLastLocation(QPoint(x,y));
  } else if (MouseInfo->getMode() == Y_ZOOMBOX) {
    QRect pr = KST::plotList.at(plot_num)->GetPlotRegion();

    x = pr.right();

    if (e->y() > pr.bottom()) {
      y = pr.bottom() + 1;
    } else if (e->y() < pr.top()) {
      y = pr.top();
    } else {
      y = e->y();
    }

    MouseInfo->setLastLocation(QPoint(x,y));
  } else if (MouseInfo->getMode() == X_ZOOMBOX) {
    QRect pr = KST::plotList.at(plot_num)->GetPlotRegion();

    if (e->x() > pr.right()) {
      x = pr.right() + 1;
    } else if (e->x() < pr.left()) {
      x = pr.left();
    } else {
      x = e->x();
    }

    y = pr.bottom();

    MouseInfo->setLastLocation(QPoint(x,y));
  }

  if (ZoomCurrentPlot) {
    i0 = ZoomPlotNum;
    iN = ZoomPlotNum+1;
  } else {
    i0 = 0;
    iN = KST::plotList.count();
  }

  for (i = i0; i < iN; i++) {
    QRect plot_rect = KST::plotList.at(i)->GetPlotRegion();
    if (plot_rect.contains(e->pos())) {
      if (MouseInfo->getMode() == INACTIVE) {
        MouseInfo->setPlotNum(i);
        if (i != plot_num) {
          updateTieBoxes(this);
        }
      } else if (MouseInfo->getMode() == LABEL_TOOL) {
        MouseInfo->setPlotNum(i);
      }
      in_plot = true;
      break;
    }
  }
  updateMouse();
  MouseInfo->setCursor(in_plot, e);
}

void KstView::mousePressEvent(QMouseEvent *e) {
  unsigned int i, i0, iN;
  QRect win_rect, plot_rect, tie_rect, plot_and_axis_rect;
  KstPlot *plot;

  if (ZoomCurrentPlot) {
    i0 = ZoomPlotNum;
    iN = ZoomPlotNum+1;
  } else {
    i0 = 0;
    iN = KST::plotList.count();
  }

  /* Find where the mouse was to determine which mode to be in */
  /* which button */
  if (e->button() == Qt::LeftButton) {
    _draggablePlot = -1;
    _draggableLabel = MouseInfo->getLabelNum(e);
    _draggablePoint = e->pos();
    /* search through the plots */
    for (i = i0; i < iN; i++) {
      plot = KST::plotList.at(i);
      win_rect = plot->GetWinRegion();
      plot_rect = plot->GetPlotRegion();
      tie_rect = plot->GetTieBoxRegion();
      plot_and_axis_rect = plot->GetPlotAndAxisRegion();
      if (tie_rect.contains(e->pos())) {
        plot->toggleTied();
        updateTieBoxes(this);
        break;
      } else if (plot_rect.contains(e->pos())) {
        if (_draggableLabel == -1 || MouseInfo->getMode() != LABEL_TOOL) {
          MouseInfo->mousePressedInPlot(e, plot_rect);
        } else {
          _draggablePlot = i;
        }
        MouseInfo->setPlotNum(i);
        return;
      } else if (plot_and_axis_rect.contains(e->pos())) {
        MouseInfo->setPlotNum(i);
        ParentApp->plotDialog->show_I();
        ParentApp->plotDialog->Select->setCurrentItem(i);
        ParentApp->plotDialog->update();
        ParentApp->plotDialog->TabWidget->setCurrentPage(LIMITS_TAB);
        break;
      } else if (win_rect.contains(e->pos())) {
        MouseInfo->setPlotNum(i);
        ParentApp->plotDialog->show_I();
        ParentApp->plotDialog->Select->setCurrentItem(i);
        ParentApp->plotDialog->update();
        ParentApp->plotDialog->TabWidget->setCurrentPage(LABELS_TAB);
        break;
      }
    }
  } else if (e->button() == Qt::RightButton) {
    for (i = i0; i < iN; i++) {
      KstPlot *p = KST::plotList.at(i);
      win_rect = p->GetPlotRegion();
      if (win_rect.contains(e->pos())) {
        MouseInfo->setMode(INACTIVE);
        MouseInfo->setPressLocation(e->pos());
        MouseInfo->setPlotNum(i);
        popupmenu->changeTitle(_titleId, i18n("Plot: %1").arg(p->tagName()));

        // Update the add/remove menus.
        _addMenu->clear();
        _removeMenu->clear();
        _editCurveMenu->clear();
        _currentPlot = p;
        int id = 100;
        bool addItemEnabled = false, removeItemEnabled = false;
        KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
        for (KstBaseCurveList::Iterator i = curves.begin(); i != curves.end(); ++i) {
          if (!p->Curves.contains((*i).data())) {
            _addMenu->insertItem((*i)->tagName(), this, SLOT(dynamicMenuAdd(int)), 0, id);
            addItemEnabled = true;
          } else {
            _removeMenu->insertItem((*i)->tagName(), this, SLOT(dynamicMenuRemove(int)), 0, id);
            removeItemEnabled = true;
            _editCurveMenu->insertItem((*i)->tagName(), this, SLOT(dynamicMenuEdit(int)), 0, id);
          }
          _curveIds[id] = (*i)->tagName();
          ++id;
        }

        popupmenu->setItemEnabled(_addMenuId, addItemEnabled);
        popupmenu->setItemEnabled(_removeMenuId, removeItemEnabled);
        popupmenu->setItemEnabled(_editCurveMenuId, removeItemEnabled);

        popupmenu->popup(mapToGlobal(e->pos()));
        return;
      }
    }
  } else if (e->button() == Qt::MidButton) {
    for (i = i0; i < iN; i++) {
      win_rect = KST::plotList.at(i)->GetPlotRegion();
      if (win_rect.contains(e->pos())) {
        MouseInfo->setMode(INACTIVE);
        MouseInfo->setPressLocation(e->pos());
        MouseInfo->setPlotNum(i);
        zoomPrevSlot();
        return;
      }
    }
    return;
  } else {
    // cout << "unknown button: " << e->button() << "\n";
  }
}

void KstView::mouseReleaseEvent(QMouseEvent *e) {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax, new_ymin, new_ymax;
  QRect plotregion;
  KstPlot *plot, *iplot;
  unsigned int count;

  _draggableLabel = -1;
  _draggablePlot = -1;

  if (KST::plotList.isEmpty()) {
    return;
  }

  unsigned int i_plot = MouseInfo->getPlotNum();
  plot = KST::plotList.at(i_plot);

  if (MouseInfo->getMode() == XY_ZOOMBOX) {
    /* erase the zoombox: */
    QPainter p(this);
    if (!MouseInfo->isFirstMove()) MouseInfo->DrawBox(p);
    if ((MouseInfo->getMouseRect().width() >_minMove) &&
        (MouseInfo->getMouseRect().height() >_minMove)) {

      plot->getLScale(xmin, ymin, xmax, ymax);
      plotregion = plot->GetPlotRegion();
      new_xmin = (double)(MouseInfo->getMouseRect().left() -
                          plotregion.left())/
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_xmax = (double)(MouseInfo->getMouseRect().right() -
                          plotregion.left() + 1)/
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_ymin = (double)(MouseInfo->getMouseRect().bottom() -
                          plotregion.top()+1)/
                 (double)plotregion.height() * (ymin - ymax) + ymax;
      new_ymax = (double)(MouseInfo->getMouseRect().top() - plotregion.top())/
                 (double)plotregion.height() * (ymin - ymax) + ymax;


      count = KST::plotList.count();
      for (i_plot = 0; i_plot < count; i_plot++) {
        iplot = KST::plotList.at(i_plot);
        if (plot == iplot || iplot->isTied()) {
          iplot->setXScaleMode(FIXED);
          iplot->setYScaleMode(FIXED);
          iplot->setLScale(new_xmin, new_ymin, new_xmax, new_ymax);
          iplot->pushScale();
        }
      }
    }
  } else if (MouseInfo->getMode() == Y_ZOOMBOX) {
    /* erase the zoombox: */
    QPainter p(this);
    if (!MouseInfo->isFirstMove()) MouseInfo->DrawBox(p);
    if (MouseInfo->getMouseRect().height() >_minMove) {

      plot->getLScale(xmin, ymin, xmax, ymax);
      plotregion = plot->GetPlotRegion();
      new_ymin = (double)(MouseInfo->getMouseRect().bottom() -
                          plotregion.top()+1)/
                 (double)plotregion.height() * (ymin - ymax) + ymax;
      new_ymax = (double)(MouseInfo->getMouseRect().top() - plotregion.top())/
                 (double)plotregion.height() * (ymin - ymax) + ymax;

      count = KST::plotList.count();
      for (i_plot = 0; i_plot < count; i_plot++) {
        iplot = KST::plotList.at(i_plot);
        if (plot == iplot || iplot->isTied()) {
          iplot->setYScaleMode(FIXED);
          iplot->setLYScale(new_ymin, new_ymax);
          iplot->pushScale();
        }
      }
    }
  } else   if (MouseInfo->getMode() == X_ZOOMBOX) {
    /* erase the zoombox: */
    QPainter p(this);
    if (!MouseInfo->isFirstMove()) MouseInfo->DrawBox(p);

    if (MouseInfo->getMouseRect().width() > _minMove) {

      plot->getLScale(xmin, ymin, xmax, ymax);
      plotregion = plot->GetPlotRegion();
      new_xmin = (double)(MouseInfo->getMouseRect().left() - plotregion.left())/
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_xmax = (double)(MouseInfo->getMouseRect().right() -
                          plotregion.left() + 1)/
                 (double)plotregion.width() * (xmax - xmin) + xmin;

      count = KST::plotList.count();
      for (i_plot = 0; i_plot < count; i_plot++) {
        iplot = KST::plotList.at(i_plot);
        if (plot == iplot || iplot->isTied()) {
          iplot->setXScaleMode(FIXED);
          iplot->setLXScale(new_xmin, new_xmax);
          iplot->pushScale();
        }
      }
    }
  } else if (MouseInfo->getMode() == LABEL_TOOL) {
    int i_label = MouseInfo->getLabelNum(e);

    plotregion = plot->GetPlotRegion();
    if (i_label >= 0 || plotregion.contains(e->pos()))  {
      double x = double(e->x() - plotregion.left()) / double(plotregion.width());
      double y = double(e->y() - plotregion.top()) / double(plotregion.height());
      labelDialog->showI(i_plot, i_label, x, y);
    }
  }

  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();

  // Why?
  mouseMoveEvent(e);
}


void KstView::dragEnterEvent(QDragEnterEvent *e) {
  e->accept(e->provides("application/x-kst-label-number"));
}


void KstView::dropEvent(QDropEvent *e) {
  if (e->provides("application/x-kst-label-number")) {
    if (e->source() == this) {
      QDataStream ds(e->encodedData("application/x-kst-label-number"), IO_ReadOnly);
      int plot, label;
      QPoint hs;
      ds >> plot >> label >> hs;
      int posplot = -1, plotCount = KST::plotList.count();
      KstPlot *plotPtr = 0L;
      for (int i = 0; i < plotCount; i++) {
        plotPtr = KST::plotList.at(i);
        if (plotPtr->GetPlotRegion().contains(e->pos())) {
          posplot = i;
          break;
        }
      }

      if (posplot == -1) {
        e->accept(false);
        return;
      }

      KstLabel *l;
      if (posplot != plot) { // move to a new plot
        l = KST::plotList.at(plot)->labelList.take(label);
        assert(l);
        plotPtr->labelList.append(l);
      } else {
        l = plotPtr->labelList.at(label);
      }

      QPoint pos = e->pos() - plotPtr->GetPlotRegion().topLeft() - hs + QPoint(2 /* border offset */, 2 /* border offset */ + 8 /* fudge factor */);
      QSize divisor = plotPtr->GetPlotRegion().size();
      l->move(float(pos.x())/divisor.width(), float(pos.y())/divisor.height());
      l->extents.setX(pos.x());
      l->extents.setY(pos.y());
      e->accept(true);
      update();
      return;
    }
  }

  e->accept(false);
}


void KstView::zoomMaxSlot() {
  unsigned int count = KST::plotList.count();

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    KstPlot *iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(AUTO);
      iplot->setYScaleMode(AUTO);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::zoomSpikeInsensiveMaxSlot() {
  unsigned int count = KST::plotList.count();

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    KstPlot *iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(NOSPIKE);
      iplot->setYScaleMode(NOSPIKE);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::keyPressEvent(QKeyEvent *e) {
  MouseInfo->setCursor(true, e->stateAfter());

  if (e->key() == Qt::Key_Escape) {
    MouseInfo->setMode(INACTIVE);
    update();
  } else {
    e->ignore();
  }
}

void KstView::keyReleaseEvent(QKeyEvent *e) {
  MouseInfo->setCursor(true, e->stateAfter());
}

void KstView::xzoomMaxSlot() {
  unsigned int count = KST::plotList.count();
  KstPlot *iplot;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(AUTO);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::yzoomMaxSlot() {
  unsigned int count = KST::plotList.count();
  KstPlot *iplot;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setYScaleMode(AUTO);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::yzoomAcSlot() {
  unsigned int count = KST::plotList.count();
  KstPlot *iplot;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setYScaleMode(AC);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::zoomOutSlot() {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax, new_ymin, new_ymax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  new_xmin = xmin - (xmax - xmin)*0.25;
  new_xmax = xmax + (xmax - xmin)*0.25;
  new_ymin = ymin - (ymax - ymin)*0.25;
  new_ymax = ymax + (ymax - ymin)*0.25;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(FIXED);
      iplot->setYScaleMode(FIXED);
      iplot->setLScale(new_xmin, new_ymin, new_xmax, new_ymax);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();

}

void KstView::xzoomInSlot() {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  new_xmin = xmin + (xmax - xmin)*0.1666666;
  new_xmax = xmax - (xmax - xmin)*0.1666666;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(FIXED);
      iplot->setLXScale(new_xmin, new_xmax);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}
void KstView::xzoomOutSlot() {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  new_xmin = xmin - (xmax - xmin)*0.25;
  new_xmax = xmax + (xmax - xmin)*0.25;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(FIXED);
      iplot->setLXScale(new_xmin, new_xmax);
      iplot->pushScale();
    }
  }

  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::yzoomInSlot() {
  double xmin, xmax, ymin, ymax;
  double new_ymin, new_ymax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  new_ymin = ymin + (ymax - ymin)*0.16666666;
  new_ymax = ymax - (ymax - ymin)*0.16666666;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setYScaleMode(FIXED);
      iplot->setLYScale(new_ymin, new_ymax);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::yzoomOutSlot() {
  double xmin, xmax, ymin, ymax;
  double new_ymin, new_ymax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  new_ymin = ymin - (ymax - ymin)*0.25;
  new_ymax = ymax + (ymax - ymin)*0.25;

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setYScaleMode(FIXED);
      iplot->setLYScale(new_ymin, new_ymax);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::zoomPrevSlot() {
  unsigned int count = KST::plotList.count();
  bool popped = false;
  KstPlot *plot;

  /* pop prev zoom on all tied plots */
  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    plot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || plot->isTied()) {
      if (plot->popScale()) {
        popped = true;
      }
    }
  }

  if (popped) {
    needrecreate = true;
    update();
  }

}

void KstView::upSlot() {
  moveVertical(true);
}

void KstView::downSlot() {
  moveVertical(false);
}

void KstView::moveVertical(bool up) {
  double xmin, xmax, ymin, ymax;
  double new_ymin, new_ymax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  if (up) {
    new_ymin = ymin + (ymax - ymin)*0.25;
    new_ymax = ymax + (ymax - ymin)*0.25;
  } else {
    new_ymin = ymin - (ymax - ymin)*0.25;
    new_ymax = ymax - (ymax - ymin)*0.25;
  }


  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      if (ParentApp->getMouseZoomRadio() == X_ZOOMBOX) {
        iplot->getLScale(xmin, ymin, xmax, ymax);
        if (up) {
          new_ymin = ymin + (ymax - ymin)*0.25;
          new_ymax = ymax + (ymax - ymin)*0.25;
        } else {
          new_ymin = ymin - (ymax - ymin)*0.25;
          new_ymax = ymax - (ymax - ymin)*0.25;
        }
      }

      iplot->setYScaleMode(FIXED);
      iplot->setLYScale(new_ymin, new_ymax);
      iplot->pushScale();
    }
  }

  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();

}

void KstView::rightSlot() {
  moveHorizontal(false);
}


void KstView::leftSlot() {
  moveHorizontal(true);
}

void KstView::moveHorizontal(bool left) {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  plot->getLScale(xmin, ymin, xmax, ymax);

  if (left) {
    new_xmin = xmin - (xmax - xmin)*0.1;
    new_xmax = xmax - (xmax - xmin)*0.1;
  } else {
    new_xmin = xmin + (xmax - xmin)*0.1;
    new_xmax = xmax + (xmax - xmin)*0.1;
  }

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setXScaleMode(FIXED);
      iplot->setLXScale(new_xmin, new_xmax);
      iplot->pushScale();
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::pauseSlot(){
  ParentApp->togglePaused();
}

void KstView::xLogSlot() {
  logSlot(true);
}

void KstView::yLogSlot() {
  logSlot(false);
}


void KstView::logSlot(bool x) {
  unsigned int count = KST::plotList.count();
  KstPlot *plot, *iplot;
  bool xLog, yLog;

  plot = KST::plotList.at(MouseInfo->getPlotNum());
  if (!plot) {
    return;
  }
  xLog = x ? !plot->isXLog() : plot->isXLog();
  yLog = x ? plot->isYLog() : !plot->isYLog();

  for (unsigned i_plot = 0; i_plot < count; i_plot++) {
    iplot = KST::plotList.at(i_plot);
    if (i_plot == MouseInfo->getPlotNum() || iplot->isTied()) {
      iplot->setLog(xLog, yLog);
    }
  }
  MouseInfo->setMode(INACTIVE);
  needrecreate = true;
  update();
}

void KstView::updateTieBox(QPaintDevice *pd, unsigned int i) {
  int s_dim;
  int s_left, s_top;
  QRect tie_box;
  KstPlot *plot;

  if (ZoomCurrentPlot) return;

  QPainter p(pd);
  p.setPen(QColor("black"));

  plot = KST::plotList.at(i);
  tie_box = plot->GetTieBoxRegion();
  s_dim = tie_box.width()/3;
  s_left = tie_box.left() + tie_box.width()/2 - s_dim/2;
  s_top = tie_box.top() + tie_box.width()/2 - s_dim/2;

  if (plot->isTied()) {
    p.fillRect(tie_box, QColor("slate grey"));
  } else {
    p.fillRect(tie_box, QColor("white"));
  }
  p.drawRoundRect(tie_box);
  if (i == MouseInfo->getPlotNum()) {
    p.fillRect(s_left, s_top, s_dim, s_dim, QColor("black"));
  } else {
    p.fillRect(s_left, s_top, s_dim, s_dim, QColor("white"));
  }
}

void KstView::updateTieBoxes(QPaintDevice *pd) {
  unsigned int count = KST::plotList.count();

  if (ZoomCurrentPlot) return;

  for (unsigned i = 0; i < count; i++) {
    updateTieBox(pd, i);
  }
}

void KstView::toggleTiedZoom() {
  unsigned count = KST::plotList.count();
  KstPlot *plot;
  unsigned num_tied = 0;
  bool new_tie_state;

  // the plots get to vote on whether we are going to tie or untie...
  for (unsigned i = 0; i < count; i++) {
    plot = KST::plotList.at(i);
    if (plot->isTied())
      num_tied++;
  }

  new_tie_state = num_tied <= count/2;

  for (unsigned i = 0; i < count; i++) {
    plot = KST::plotList.at(i);
    plot->setTied(new_tie_state);
  }
  updateTieBoxes(this);
}

void KstView::zoomPlotSlot() {
  ZoomCurrentPlot = !ZoomCurrentPlot;

  if (ZoomCurrentPlot) {
    MouseInfo->setZoomed();
  } else {
    MouseInfo->unsetZoom();
  }

  ZoomPlotNum = MouseInfo->getPlotNum();
  update();
}

void KstView::forceUpdate() {
  needrecreate = true;
  paintEvent(0L);
}

void KstView::dynamicMenuAdd(int id) {
   dynamicMenu(true, id);
}

void KstView::dynamicMenuRemove(int id) {
   dynamicMenu(false, id);
}

void KstView::dynamicMenuEdit(int id) {
  KstDataObjectList::Iterator i = KST::dataObjectList.findTag(_curveIds[id]);
  if (i == KST::dataObjectList.end()) {
    return;
  }
  (*i)->showDialog();

  _currentPlot = 0L;
}

void KstView::dynamicMenu(bool add, int id) {
  if (!_currentPlot) {
    return;
  }

  KstDataObjectList::Iterator i = KST::dataObjectList.findTag(_curveIds[id]);
  KstBaseCurve *di = 0;
  if (i == KST::dataObjectList.end() || !(di = dynamic_cast<KstBaseCurve*>((*i).data()))) {
    return;
  }
  KstBaseCurvePtr p(di);
  if (add) {
    if (!_currentPlot->Curves.contains(p)) {
      _currentPlot->addCurve(p);
      p = 0;
      _currentPlot = 0L;
      ParentApp->updateDialogs();
      update();
    } else {
      _currentPlot = 0L;
    }
  } else {
    _currentPlot->removeCurve(p);
    p = 0;
    _currentPlot = 0L;
    ParentApp->updateDialogs();
    update();
  }
}

void KstView::deleteCurrentPlot() {
  if (_currentPlot) {
    KST::plotList.remove(_currentPlot);
    _currentPlot = 0L;
    ParentApp->updateDialogs();
    update();
  }
}

void KstView::cleanupLayout() {
  needrecreate = true;
  KST::plotList.arrangePlots(KST::plotList.getPlotCols());
  update();
}

void KstView::setDataMode(bool on) {
  _dataMode = on;
  updateMouse();
}

#include "kstview.moc"
// vim: ts=2 sw=2 et
