/***************************************************************************
                          kstmouse.h  -  description
                             -------------------
    begin                : Fri Feb 16, 2001
    copyright            : (C) 2001 by Barth Netterfield
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
#ifndef KSTMOUSE_H
#define KSTMOUSE_H

#include <qpoint.h>
#include <qrect.h>
#include <qpainter.h>
#include <qevent.h>
#include <qcursor.h>

typedef enum {INACTIVE, XY_ZOOMBOX, Y_ZOOMBOX, X_ZOOMBOX, LABEL_TOOL} KstMouseModeType;

class KstView;
class KstPlot;

/** Stores information regarding what the mouse is suppose to be doing */
class KstMouse
{
public:
  KstMouse(KstView *view);
  virtual ~KstMouse();

  KstMouseModeType getMode();
  void setMode(KstMouseModeType in_mode);

  /** Set the location of the pressing of the mouse */
  void setPressLocation(const QPoint &in_PressLocation);
  QPoint getPressLocation();

  /** Set the last valid location of the mouse */
  void setLastLocation(const QPoint &inLocation);
  QPoint getLastLocation();

  /** Get the rectangle defined by PressLocation, LastLocation */
  QRect getMouseRect();

  void setPlotNum(const unsigned int &in_plotnum);
  unsigned int getPlotNum();

  int getLabelNum(const QMouseEvent *e);
  bool isFirstMove();
  void DrawBox(QPainter &p);
  void setZoomed();
  void unsetZoom();

  void mousePressedInPlot(const QMouseEvent *e, const QRect &plot_rect);
  void setCursor(bool in_plot, Qt::ButtonState state);
  void setCursor(bool in_plot, const QMouseEvent *e);
private:
  /** The tool mode the mouse is currently active in.  Set at a butten
   press event.  Used in mouse move or mouse release */
  KstMouseModeType Mode;

  /** Where the mouse was when it was pressed */
  QPoint PressLocation;
  QPoint LastLocation;

  unsigned int PlotNum;

  unsigned int ZoomNum;
  bool Zoomed;
  bool IsFirstMove;

  KstView *_view;

  QCursor *ArrowCursor;
  QCursor *CrossCursor;
  QCursor *IBeamCursor;
  QCursor *VResizeCursor;
  QCursor *HResizeCursor;

};

#endif
