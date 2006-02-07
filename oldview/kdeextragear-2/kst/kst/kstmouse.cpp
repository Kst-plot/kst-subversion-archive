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

#include <stdio.h>
#include <iostream>

#include "kstdatacollection.h"
#include "kstplot.h"
#include "kstmouse.h"
#include "kstplotlist.h"
#include "kstview.h"
#include "qbitmap.h"

#include <kdebug.h>

KstMouse::KstMouse(KstView *view) {
  uchar resize_bits[2*16];
  QBitmap *resize_bitmap;
  int i;

  Mode = INACTIVE;
  PressLocation = QPoint(1,1);
  LastLocation = QPoint(2,2);
  IsFirstMove=false;
  PlotNum=0;
  ZoomNum = 0;
  Zoomed = false;
  _view = view;

  CrossCursor = new QCursor(Qt::crossCursor);
  ArrowCursor = new QCursor(Qt::arrowCursor);
  IBeamCursor = new QCursor(Qt::IbeamCursor);

  /* build HResizeCursor */
  for (i = 0; i < 16; i++) {
    resize_bits[2*i] = 0;
    resize_bits[2*i+1] = 0x80;
  }
  resize_bits[1]  = 0x00;
  resize_bits[16] = 0x03;
  resize_bits[17] = 0xe0;
  resize_bitmap = new QBitmap(16,16,resize_bits,FALSE);
  HResizeCursor = new QCursor(*resize_bitmap, *resize_bitmap);
  delete resize_bitmap;

  /* build VResizeCursor */
  for (i = 0; i < 32; i++) {
    resize_bits[i] = 0;
  }
  resize_bits[16] = 0x7f;
  resize_bits[17] = 0xff;
  resize_bits[13] = 0x80;
  resize_bits[15] = 0x80;
  resize_bits[19] = 0x80;
  resize_bits[21] = 0x80;
  resize_bitmap = new QBitmap(16,16,resize_bits,FALSE);
  VResizeCursor = new QCursor(*resize_bitmap, *resize_bitmap);
  delete resize_bitmap;
  resize_bitmap = 0L;
}

KstMouse::~KstMouse() {
  delete HResizeCursor;
  HResizeCursor = 0L;
  delete VResizeCursor;
  VResizeCursor = 0L;
  delete CrossCursor;
  CrossCursor = 0L;
  delete ArrowCursor;
  ArrowCursor = 0L;
  delete IBeamCursor;
  IBeamCursor = 0L;
}

KstMouseModeType KstMouse::getMode() {
  return Mode;
}

void KstMouse::setMode(KstMouseModeType in_mode) {
  Mode = in_mode;
}

QPoint KstMouse::getPressLocation() {
  return PressLocation;
}

void KstMouse::setPressLocation(const QPoint &in_PressLocation) {
  PressLocation = in_PressLocation;
  LastLocation = in_PressLocation;
  IsFirstMove = true;
}

QPoint KstMouse::getLastLocation() {
  return LastLocation;
}

void KstMouse::setLastLocation(const QPoint &in_Location) {
  QPainter p(_view);

  if (!IsFirstMove) {
    DrawBox(p);
  }
  IsFirstMove = false;
  LastLocation = in_Location;
  DrawBox(p);
}

void KstMouse::setPlotNum(const unsigned int &in_plotnum) {
  PlotNum = in_plotnum;
}

unsigned int KstMouse::getPlotNum() {
  return Zoomed ? ZoomNum : PlotNum;
}

void KstMouse::setZoomed() {
  ZoomNum = PlotNum;
  Zoomed = true;
}

void KstMouse::unsetZoom() {
  Zoomed = false;
}

QRect KstMouse::getMouseRect() {
  int x1, x2, y1, y2;

  x1 = PressLocation.x();
  x2 = LastLocation.x();
  y1 = PressLocation.y();
  y2 = LastLocation.y();

  if (x1 == x2) {
    x2++;
  } else if (x1 > x2) {
    int t = x1; x1 = x2; x2 = t;
  }

  if (y1 == y2) {
    y2++;
  } else if (y1 > y2) {
    int t = y1; y1 = y2; y2 = t;
  }

  return QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

bool KstMouse::isFirstMove() {
  return IsFirstMove;
}

void KstMouse::DrawBox(QPainter &p) {
  if (getMode() != LABEL_TOOL) {
    p.setRasterOp(Qt::NotROP);
    p.drawRect(getMouseRect());
  }
}

void KstMouse::mousePressedInPlot(const QMouseEvent *e,
                                  const QRect &plot_rect) {
  int mzr = _view->ParentApp->getMouseZoomRadio();

  if (mzr == LABEL_TOOL) {
    setMode(LABEL_TOOL);
    setPressLocation(e->pos());
  } else if (e->state() & Qt::ShiftButton) {
    setMode(Y_ZOOMBOX);
    setPressLocation(QPoint(plot_rect.left()+1, e->y()));
  } else if (e->state() & Qt::ControlButton) {
    setMode(X_ZOOMBOX);
    setPressLocation(QPoint(e->x(), plot_rect.top()+1));
  } else {
    switch (mzr) {
        case Y_ZOOMBOX:
          setMode(Y_ZOOMBOX);
          setPressLocation(QPoint(plot_rect.left()+1, e->y()));
          break;
        case X_ZOOMBOX:
          setMode(X_ZOOMBOX);
          setPressLocation(QPoint(e->x(), plot_rect.top()+1));
          break;
        default:
          setMode(XY_ZOOMBOX);
          setPressLocation(e->pos());
          break;
    }
  }
}

void KstMouse::setCursor(bool in_plot, const QMouseEvent *e) {
  if (_view->ParentApp->getMouseZoomRadio() == LABEL_TOOL) {
    if (getLabelNum(e) >= 0) {
      _view->setCursor(*ArrowCursor);
      setMode(LABEL_TOOL);
      return;
    }
    
    if (getLegend(e)) {
      _view->setCursor(*ArrowCursor);
      setMode(LABEL_TOOL);
      return;      
    }
  }

  setCursor(in_plot, e->state());
}

void KstMouse::setCursor(bool in_plot, Qt::ButtonState state) {
  int mzr = _view->ParentApp->getMouseZoomRadio();
  if (in_plot) {
    if (mzr == LABEL_TOOL) {
      _view->setCursor(*IBeamCursor);
    } else if (getMode() == X_ZOOMBOX) {
      _view->setCursor(*HResizeCursor);
    } else if (getMode() == Y_ZOOMBOX) {
      _view->setCursor(*VResizeCursor);
    } else if (state & Qt::ShiftButton) {
      _view->setCursor(*VResizeCursor);
    } else if (state & Qt::ControlButton) {
      _view->setCursor(*HResizeCursor);
    } else if (mzr == X_ZOOMBOX) {
      _view->setCursor(*HResizeCursor);
    } else if (mzr == Y_ZOOMBOX) {
      _view->setCursor(*VResizeCursor);
    } else {
      _view->setCursor(*CrossCursor);
    }
  } else {
    _view->setCursor(*ArrowCursor);
  }
}

int KstMouse::getLabelNum(const QMouseEvent *e) {
  KstPlot *plot = KST::plotList.at(getPlotNum());
  if (plot) {
    QPoint pt = plot->GetWinRegion().topLeft();
    QPoint ptCheck( e->x() - pt.x(), e->y() - pt.y() );
    
    for(unsigned i = 0; i < plot->labelList.count(); i++) {
      if (plot->labelList.at(i)->extents.contains(ptCheck)) {
        return i;
      }
    }
  }

  return -1;
}

bool KstMouse::getLegend(const QMouseEvent *e) {
  KstPlot *plot = KST::plotList.at(getPlotNum());
  if (plot) {
    QPoint pt = plot->GetWinRegion().topLeft();
    QPoint ptCheck( e->x() - pt.x(), e->y() - pt.y() );
    
    if (plot->Legend->extents.contains(ptCheck)) {
      return true;
    }
  }

  return false;
}
// vim: ts=2 sw=2 et
