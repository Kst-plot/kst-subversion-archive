/***************************************************************************
                          kstview.h  -  description
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

#ifndef KSTVIEW_H
#define KSTVIEW_H

// include files for Qt
#include <qmap.h>
#include <qwidget.h>
#include <kpopupmenu.h>
#include <kst.h>
#include <kapp.h>

class KstDoc;
class KPrinter;
class KstMouse;
class KstLabelDialogI;

/** The KstView class provides the view widget for the KstApp instance.
 * The View instance inherits QWidget as a base class and represents the
 * view object of a KTMainWindow. As KstView is part of the
 * docuement-view model, it needs a reference to the document object
 * connected with it by the KstApp class to manipulate and display
 * the document structure provided by the KstDoc class.
 *
 * Original Source Framework Automatically Generated by KDevelop,
 * KDevelop version 0.4 code generation (c) The KDevelop Team.
 *
 * Lobotomised by cbn: kst has a single doc/single view model.
 */
class KstView : public QWidget
{
  Q_OBJECT
public:
  /** Constructor for the main view */
  KstView(KstApp *parent = 0, const char *name=0);
  /** Destructor for the main view */
  virtual ~KstView();

  /** contains the implementation for printing functionality */
  void print(KPrinter *pPrinter);

  bool dataMode() const { return _dataMode; }

public slots:
  void zoomMaxSlot();
  void zoomSpikeInsensiveMaxSlot();
  void zoomOutSlot();
  void zoomPrevSlot();
  void xzoomMaxSlot();
  void xzoomOutSlot();
  void xzoomInSlot();
  void yzoomMaxSlot();
  void yzoomAcSlot();
  void yzoomOutSlot();
  void yzoomInSlot();
  void leftSlot();
  void rightSlot();
  void upSlot();
  void downSlot();
  void moveVertical(bool up);
  void moveHorizontal(bool left);
  void pauseSlot();
  void update();
  void xLogSlot();
  void yLogSlot();
  void logSlot(bool x);
  void zoomPlotSlot();

  void setDataMode(bool on);
  void forceUpdate();

  void updateTieBoxes(QPaintDevice *);

  void toggleTiedZoom();

  void printToGraphicsFile(const QString &Filename, int w, int h);

  void cleanupLayout();

  void copy();

public:
  KstMouse *MouseInfo;
  KstApp *ParentApp;
  KstLabelDialogI *labelDialog;
protected:
  virtual void paintEvent(QPaintEvent *);
  virtual void resizeEvent(QResizeEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);
  virtual void keyPressEvent(QKeyEvent *);
  virtual void keyReleaseEvent(QKeyEvent *);
  virtual void dragEnterEvent(QDragEnterEvent *e);
  virtual void dropEvent(QDropEvent *e);

  void updateTieBox(QPaintDevice *, unsigned int i_box);
private:
  bool needrecreate; /** needed for the double buffering.  */
  QPixmap *qpixmap;

  KPopupMenu *popupmenu;

  bool first_time;
  bool ZoomCurrentPlot;
  int ZoomPlotNum;

  bool _dataMode;

  /** the smallest move to consider as a box */
  int _minMove;

  int _titleId;

  int _draggablePlot, _draggableLabel;
  QPoint _draggablePoint;

  KPopupMenu *_addMenu, *_removeMenu, *_editCurveMenu;
  int _addMenuId, _removeMenuId, _editCurveMenuId;
  KstPlot *_currentPlot;
  QMap<int,QString> _curveIds;
  double _copy_x, _copy_y;

private slots:
  void dynamicMenu(bool, int);
  void dynamicMenuAdd(int);
  void dynamicMenuRemove(int);
  void dynamicMenuEdit(int);
  void deleteCurrentPlot();
  void updateMouse();

signals:
  void newStatusMsg(const QString &);
  void newDataMsg(const QString &);
};

#endif // KSTVIEW_H
