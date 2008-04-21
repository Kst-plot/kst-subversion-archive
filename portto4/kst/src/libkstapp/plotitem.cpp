/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plotitem.h"

#include "viewitemzorder.h"
#include "plotitemmanager.h"
#include "plotrenderitem.h"

#include "layoutboxitem.h"
#include "viewgridlayout.h"
#include "debug.h"

#include "application.h"
#include "mainwindow.h"
#include "tabwidget.h"

#include "datacollection.h"
#include "dataobjectcollection.h"
#include "cartesianrenderitem.h"

#include "plotitemdialog.h"

#include "math_kst.h"

#include "settings.h"

#include <QMenu>
#include <QDebug>

static qreal TOP_MARGIN = 20.0;
static qreal BOTTOM_MARGIN = 0.0;
static qreal LEFT_MARGIN = 0.0;
static qreal RIGHT_MARGIN = 20.0;
static int FULL_PRECISION = 15;
static qreal JD1900 = 2415020.5;
static qreal JD1970 = 2440587.5;
static qreal JD_RJD = 2400000.0;
static qreal JD_MJD = 2400000.5;

namespace Kst {

PlotItem::PlotItem(View *parent)
  : ViewItem(parent),
  _isTiedZoom(false),
  _xAxisZoomMode(Auto),
  _yAxisZoomMode(AutoBorder),
  _isLeftLabelVisible(true),
  _isBottomLabelVisible(true),
  _isRightLabelVisible(true),
  _isTopLabelVisible(true),
  _isBottomAxisVisible(true),
  _isLeftAxisVisible(true),
  _calculatedLeftLabelMargin(0.0),
  _calculatedRightLabelMargin(0.0),
  _calculatedTopLabelMargin(0.0),
  _calculatedBottomLabelMargin(0.0),
  _calculatedLabelMarginWidth(0.0),
  _calculatedLabelMarginHeight(0.0),
  _calculatedAxisMarginWidth(0.0),
  _calculatedAxisMarginHeight(0.0),
  _leftLabelFontScale(0.0),
  _bottomLabelFontScale(0.0),
  _topLabelFontScale(0.0),
  _rightLabelFontScale(0.0),
  _xAxisLog(false),
  _yAxisLog(false),
  _xAxisReversed(false),
  _yAxisReversed(false),
  _xAxisBaseOffset(false),
  _yAxisBaseOffset(false),
  _xAxisInterpret(false),
  _yAxisInterpret(false),
  _xAxisDisplay(AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS),
  _yAxisDisplay(AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS),
  _xAxisInterpretation(AXIS_INTERP_CTIME),
  _yAxisInterpretation(AXIS_INTERP_CTIME),
  _xAxisMajorTickMode(Normal),
  _yAxisMajorTickMode(Normal),
  _xAxisMinorTickCount(4),
  _yAxisMinorTickCount(4),
  _xAxisSignificantDigits(9),
  _yAxisSignificantDigits(9),
  _drawXAxisMajorTicks(true),
  _drawXAxisMinorTicks(true),
  _drawYAxisMajorTicks(true),
  _drawYAxisMinorTicks(true),
  _drawXAxisMajorGridLines(true),
  _drawXAxisMinorGridLines(false),
  _drawYAxisMajorGridLines(true),
  _drawYAxisMinorGridLines(false),
  _xAxisMajorGridLineColor(Qt::gray),
  _xAxisMinorGridLineColor(Qt::gray),
  _yAxisMajorGridLineColor(Qt::gray),
  _yAxisMinorGridLineColor(Qt::gray),
  _xAxisMajorGridLineStyle(Qt::DashLine),
  _xAxisMinorGridLineStyle(Qt::DashLine),
  _yAxisMajorGridLineStyle(Qt::DashLine),
  _yAxisMinorGridLineStyle(Qt::DashLine),
  _xAxisPlotMarkers(true),
  _yAxisPlotMarkers(false),
  _zoomMenu(0)
 {

  setName("Plot");
  setZValue(PLOT_ZVALUE);
  setBrush(Qt::white);

  _leftLabelFont = parentView()->defaultFont();
  _bottomLabelFont = parentView()->defaultFont();
  _topLabelFont = parentView()->defaultFont();
  _rightLabelFont = parentView()->defaultFont();

  createActions();

  PlotItemManager::self()->addPlot(this);

  // Set the initial projection.
  setProjectionRect(QRectF(QPointF(-0.1, -0.1), QPointF(0.1, 0.1)));
  renderItem(PlotRenderItem::Cartesian);
}


PlotItem::~PlotItem() {
  PlotItemManager::self()->removePlot(this);
}


QString PlotItem::plotName() const {
  return name();
}


void PlotItem::save(QXmlStreamWriter &xml) {
  xml.writeStartElement("plot");
  xml.writeAttribute("tiedzoom", QVariant(_isTiedZoom).toString());
  xml.writeAttribute("bottomaxisvisible", QVariant(_isBottomAxisVisible).toString());
  xml.writeAttribute("leftaxisvisible", QVariant(_isLeftAxisVisible).toString());
  xml.writeAttribute("leftlabelvisible", QVariant(_isLeftLabelVisible).toString());
  xml.writeAttribute("bottomlabelvisible", QVariant(_isBottomLabelVisible).toString());
  xml.writeAttribute("rightlabelvisible", QVariant(_isRightLabelVisible).toString());
  xml.writeAttribute("toplabelvisible", QVariant(_isTopLabelVisible).toString());
  xml.writeAttribute("leftlabeloverride", _leftLabelOverride);
  xml.writeAttribute("leftlabelfont", QVariant(_leftLabelFont).toString());
  xml.writeAttribute("leftlabelfontscale", QVariant(_leftLabelFontScale).toString());
  xml.writeAttribute("bottomlabeloverride", _bottomLabelOverride);
  xml.writeAttribute("bottomlabelfont", QVariant(_bottomLabelFont).toString());
  xml.writeAttribute("bottomlabelfontscale", QVariant(_bottomLabelFontScale).toString());
  xml.writeAttribute("toplabeloverride", _topLabelOverride);
  xml.writeAttribute("toplabelfont", QVariant(_topLabelFont).toString());
  xml.writeAttribute("toplabelfontscale", QVariant(_topLabelFontScale).toString());
  xml.writeAttribute("rightlabeloverride", _rightLabelOverride);
  xml.writeAttribute("rightlabelfont", QVariant(_rightLabelFont).toString());
  xml.writeAttribute("rightlabelfontscale", QVariant(_rightLabelFontScale).toString());
  xml.writeAttribute("xaxislog", QVariant(_xAxisLog).toString());
  xml.writeAttribute("yaxislog", QVariant(_yAxisLog).toString());
  xml.writeAttribute("xaxisreversed", QVariant(_xAxisReversed).toString());
  xml.writeAttribute("yaxisreversed", QVariant(_yAxisReversed).toString());
  xml.writeAttribute("xaxisbaseoffset", QVariant(_xAxisBaseOffset).toString());
  xml.writeAttribute("yaxisbaseoffset", QVariant(_yAxisBaseOffset).toString());
  xml.writeAttribute("xaxisinterpret", QVariant(_xAxisInterpret).toString());
  xml.writeAttribute("yaxisinterpret", QVariant(_yAxisInterpret).toString());
  xml.writeAttribute("xaxisinterpretation", QVariant(_xAxisInterpretation).toString());
  xml.writeAttribute("yaxisinterpretation", QVariant(_yAxisInterpretation).toString());
  xml.writeAttribute("xaxisdisplay", QVariant(_xAxisDisplay).toString());
  xml.writeAttribute("yaxisdisplay", QVariant(_yAxisDisplay).toString());
  xml.writeAttribute("xaxismajortickmode", QVariant(_xAxisMajorTickMode).toString());
  xml.writeAttribute("yaxismajortickmode", QVariant(_yAxisMajorTickMode).toString());
  xml.writeAttribute("xaxisminortickcount", QVariant(_xAxisMinorTickCount).toString());
  xml.writeAttribute("yaxisminortickcount", QVariant(_yAxisMinorTickCount).toString());
  xml.writeAttribute("xaxisdrawmajorticks", QVariant(_drawXAxisMajorTicks).toString());
  xml.writeAttribute("xaxisdrawminorticks", QVariant(_drawXAxisMinorTicks).toString());
  xml.writeAttribute("yaxisdrawmajorticks", QVariant(_drawYAxisMajorTicks).toString());
  xml.writeAttribute("yaxisdrawminorticks", QVariant(_drawYAxisMinorTicks).toString());
  xml.writeAttribute("xaxisdrawmajorgridlines", QVariant(_drawXAxisMajorGridLines).toString());
  xml.writeAttribute("xaxisdrawminorgridlines", QVariant(_drawXAxisMinorGridLines).toString());
  xml.writeAttribute("yaxisdrawmajorgridlines", QVariant(_drawYAxisMajorGridLines).toString());
  xml.writeAttribute("yaxisdrawminorgridlines", QVariant(_drawYAxisMinorGridLines).toString());
  xml.writeAttribute("xaxisdrawmajorgridlinecolor", QVariant(_xAxisMajorGridLineColor).toString());
  xml.writeAttribute("xaxisdrawminorgridlinecolor", QVariant(_xAxisMinorGridLineColor).toString());
  xml.writeAttribute("yaxisdrawmajorgridlinecolor", QVariant(_yAxisMajorGridLineColor).toString());
  xml.writeAttribute("yaxisdrawminorgridlinecolor", QVariant(_yAxisMinorGridLineColor).toString());
  xml.writeAttribute("xaxisdrawmajorgridlinestyle", QVariant(_xAxisMajorGridLineStyle).toString());
  xml.writeAttribute("xaxisdrawminorgridlinestyle", QVariant(_xAxisMinorGridLineStyle).toString());
  xml.writeAttribute("yaxisdrawmajorgridlinestyle", QVariant(_yAxisMajorGridLineStyle).toString());
  xml.writeAttribute("yaxisdrawminorgridlinestyle", QVariant(_yAxisMinorGridLineStyle).toString());
  xml.writeAttribute("xaxissignificantdigits", QVariant(_xAxisSignificantDigits).toString());
  xml.writeAttribute("yaxissignificantdigits", QVariant(_yAxisSignificantDigits).toString());
  xml.writeAttribute("xzoommode", QVariant(_xAxisZoomMode).toString());
  xml.writeAttribute("yzoommode", QVariant(_yAxisZoomMode).toString());

  ViewItem::save(xml);
  foreach (PlotRenderItem *renderer, renderItems()) {
    renderer->saveInPlot(xml);
  }
  xml.writeStartElement("projectionrect");
  xml.writeAttribute("x", QVariant(projectionRect().x()).toString());
  xml.writeAttribute("y", QVariant(projectionRect().y()).toString());
  xml.writeAttribute("width", QVariant(projectionRect().width()).toString());
  xml.writeAttribute("height", QVariant(projectionRect().height()).toString());
  xml.writeEndElement();
  xml.writeEndElement();
}


void PlotItem::edit() {
  PlotItemDialog editDialog(this);
  editDialog.exec();
}


void PlotItem::createActions() {
  _zoomMaximum = new QAction(tr("Zoom Maximum"), this);
  _zoomMaximum->setShortcut(Qt::Key_M);
  registerShortcut(_zoomMaximum);
  connect(_zoomMaximum, SIGNAL(triggered()), this, SLOT(zoomMaximum()));

  _zoomMaxSpikeInsensitive = new QAction(tr("Zoom Max Spike Insensitive"), this);
  _zoomMaxSpikeInsensitive->setShortcut(Qt::Key_S);
  registerShortcut(_zoomMaxSpikeInsensitive);
  connect(_zoomMaxSpikeInsensitive, SIGNAL(triggered()), this, SLOT(zoomMaxSpikeInsensitive()));

  _zoomYMeanCentered = new QAction(tr("Y-Zoom Mean-centered"), this);
  _zoomYMeanCentered->setShortcut(Qt::Key_A);
  registerShortcut(_zoomYMeanCentered);
  connect(_zoomYMeanCentered, SIGNAL(triggered()), this, SLOT(zoomYMeanCentered()));

  _zoomXMaximum = new QAction(tr("X-Zoom Maximum"), this);
  _zoomXMaximum->setShortcut(Qt::CTRL+Qt::Key_M);
  registerShortcut(_zoomXMaximum);
  connect(_zoomXMaximum, SIGNAL(triggered()), this, SLOT(zoomXMaximum()));

  _zoomXRight = new QAction(tr("X-Zoom Right"), this);
  _zoomXRight->setShortcut(Qt::Key_Right);
  registerShortcut(_zoomXRight);
  connect(_zoomXRight, SIGNAL(triggered()), this, SLOT(zoomXRight()));

  _zoomXLeft= new QAction(tr("X-Zoom Left"), this);
  _zoomXLeft->setShortcut(Qt::Key_Left);
  registerShortcut(_zoomXLeft);
  connect(_zoomXLeft, SIGNAL(triggered()), this, SLOT(zoomXLeft()));

  _zoomXOut = new QAction(tr("X-Zoom Out"), this);
  _zoomXOut->setShortcut(Qt::SHIFT+Qt::Key_Right);
  registerShortcut(_zoomXOut);
  connect(_zoomXOut, SIGNAL(triggered()), this, SLOT(zoomXOut()));

  _zoomXIn = new QAction(tr("X-Zoom In"), this);
  _zoomXIn->setShortcut(Qt::SHIFT+Qt::Key_Left);
  registerShortcut(_zoomXIn);
  connect(_zoomXIn, SIGNAL(triggered()), this, SLOT(zoomXIn()));

  _zoomNormalizeXtoY = new QAction(tr("Normalize X Axis to Y Axis"), this);
  _zoomNormalizeXtoY->setShortcut(Qt::Key_N);
  registerShortcut(_zoomNormalizeXtoY);
  connect(_zoomNormalizeXtoY, SIGNAL(triggered()), this, SLOT(zoomNormalizeXtoY()));

  _zoomLogX = new QAction(tr("Log X Axis"), this);
  _zoomLogX->setShortcut(Qt::Key_G);
  _zoomLogX->setCheckable(true);
  registerShortcut(_zoomLogX);
  connect(_zoomLogX, SIGNAL(triggered()), this, SLOT(zoomLogX()));

  _zoomYLocalMaximum = new QAction(tr("Y-Zoom Local Maximum"), this);
  _zoomYLocalMaximum->setShortcut(Qt::SHIFT+Qt::Key_L);
  registerShortcut(_zoomYLocalMaximum);
  connect(_zoomYLocalMaximum, SIGNAL(triggered()), this, SLOT(zoomYLocalMaximum()));

  _zoomYMaximum = new QAction(tr("Y-Zoom Maximum"), this);
  _zoomYMaximum->setShortcut(Qt::SHIFT+Qt::Key_M);
  registerShortcut(_zoomYMaximum);
  connect(_zoomYMaximum, SIGNAL(triggered()), this, SLOT(zoomYMaximum()));

  _zoomYUp= new QAction(tr("Y-Zoom Up"), this);
  _zoomYUp->setShortcut(Qt::Key_Up);
  registerShortcut(_zoomYUp);
  connect(_zoomYUp, SIGNAL(triggered()), this, SLOT(zoomYUp()));

  _zoomYDown= new QAction(tr("Y-Zoom Down"), this);
  _zoomYDown->setShortcut(Qt::Key_Down);
  registerShortcut(_zoomYDown);
  connect(_zoomYDown, SIGNAL(triggered()), this, SLOT(zoomYDown()));

  _zoomYOut = new QAction(tr("Y-Zoom Out"), this);
  _zoomYOut->setShortcut(Qt::SHIFT+Qt::Key_Up);
  registerShortcut(_zoomYOut);
  connect(_zoomYOut, SIGNAL(triggered()), this, SLOT(zoomYOut()));

  _zoomYIn = new QAction(tr("Y-Zoom In"), this);
  _zoomYIn->setShortcut(Qt::SHIFT+Qt::Key_Down);
  registerShortcut(_zoomYIn);
  connect(_zoomYIn, SIGNAL(triggered()), this, SLOT(zoomYIn()));

  _zoomNormalizeYtoX = new QAction(tr("Normalize Y Axis to X Axis"), this);
  _zoomNormalizeYtoX->setShortcut(Qt::SHIFT+Qt::Key_N);
  registerShortcut(_zoomNormalizeYtoX);
  connect(_zoomNormalizeYtoX, SIGNAL(triggered()), this, SLOT(zoomNormalizeYtoX()));

  _zoomLogY = new QAction(tr("Log Y Axis"), this);
  _zoomLogY->setShortcut(Qt::Key_L);
  _zoomLogY->setCheckable(true);
  registerShortcut(_zoomLogY);
  connect(_zoomLogY, SIGNAL(triggered()), this, SLOT(zoomLogY()));

  createZoomMenu();
}


void PlotItem::createZoomMenu() {
  if (_zoomMenu) {
    delete _zoomMenu;
  }

  _zoomMenu = new QMenu;
  _zoomMenu->setTitle(tr("Zoom"));

  _zoomMenu->addAction(_zoomMaximum);
  _zoomMenu->addAction(_zoomMaxSpikeInsensitive);
  _zoomMenu->addAction(_zoomYMeanCentered);

  _zoomMenu->addSeparator();

  _zoomMenu->addAction(_zoomXMaximum);
  _zoomMenu->addAction(_zoomXRight);
  _zoomMenu->addAction(_zoomXLeft);
  _zoomMenu->addAction(_zoomXOut);
  _zoomMenu->addAction(_zoomXIn);
  _zoomMenu->addAction(_zoomNormalizeXtoY);
  _zoomMenu->addAction(_zoomLogX);

  _zoomMenu->addSeparator();

  _zoomMenu->addAction(_zoomYLocalMaximum);
  _zoomMenu->addAction(_zoomYMaximum);
  _zoomMenu->addAction(_zoomYUp);
  _zoomMenu->addAction(_zoomYDown);
  _zoomMenu->addAction(_zoomYOut);
  _zoomMenu->addAction(_zoomYIn);
  _zoomMenu->addAction(_zoomNormalizeYtoX);
  _zoomMenu->addAction(_zoomLogY);
}


void PlotItem::addToMenuForContextEvent(QMenu &menu) {
  menu.addMenu(_zoomMenu);
}


QList<PlotRenderItem*> PlotItem::renderItems() const {
  return _renderers.values();
}


PlotRenderItem *PlotItem::renderItem(PlotRenderItem::RenderType type) {
  if (_renderers.contains(type))
    return _renderers.value(type);

  switch (type) {
  case PlotRenderItem::Cartesian:
    {
      CartesianRenderItem *renderItem = new CartesianRenderItem(this);
      _renderers.insert(type, renderItem);
      return renderItem;
    }
  case PlotRenderItem::Polar:
  case PlotRenderItem::Sinusoidal:
  default:
    return 0;
  }
}


void PlotItem::paint(QPainter *painter) {
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->drawRect(rect());
  painter->restore();

  painter->save();
  painter->setFont(parentView()->defaultFont());

  setCalculatedAxisMarginWidth(calculateLeftTickLabelBound(painter).width());
  setCalculatedAxisMarginHeight(calculateBottomTickLabelBound(painter).height());

  setCalculatedLeftLabelMargin(calculateLeftLabelBound(painter).width());
  setCalculatedRightLabelMargin(calculateRightLabelBound(painter).width());
  setCalculatedTopLabelMargin(calculateTopLabelBound(painter).height());
  setCalculatedBottomLabelMargin(calculateBottomLabelBound(painter).height());

//  qDebug() << "=============> leftLabel:" << leftLabel() << endl;
  paintLeftLabel(painter);
//  qDebug() << "=============> bottomLabel:" << bottomLabel() << endl;
  paintBottomLabel(painter);
//  qDebug() << "=============> rightLabel:" << rightLabel() << endl;
  paintRightLabel(painter);
//  qDebug() << "=============> topLabel:" << topLabel() << endl;
  paintTopLabel(painter);

  paintPlot(painter);
  paintTickLabels(painter);
  paintPlotMarkers(painter);

  painter->restore();
}


void PlotItem::paintPlot(QPainter *painter) {
  paintMajorGridLines(painter);
  paintMinorGridLines(painter);

  painter->save();
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(plotRect());
  painter->restore();

  paintMajorTicks(painter);
  paintMinorTicks(painter);
}


void PlotItem::paintMajorGridLines(QPainter *painter) {

  QRectF rect = plotRect();

  if (_drawXAxisMajorGridLines) {
    QVector<QLineF> xMajorTickLines;
    foreach (qreal x, _bottomAxisMajorTicks) {
      QPointF p1 = QPointF(mapXToPlot(x), plotRect().bottom());
      QPointF p2 = p1 - QPointF(0, rect.height());
      xMajorTickLines << QLineF(p1, p2);
    }

    painter->save();
    painter->setPen(QPen(QBrush(_xAxisMajorGridLineColor), 1.0, _xAxisMajorGridLineStyle));
    painter->drawLines(xMajorTickLines);
    painter->restore();
  }

  if (_drawYAxisMajorGridLines) {
    QVector<QLineF> yMajorTickLines;
    foreach (qreal y, _leftAxisMajorTicks) {
      QPointF p1 = QPointF(plotRect().left(), mapYToPlot(y));
      QPointF p2 = p1 + QPointF(rect.width(), 0);
      yMajorTickLines << QLineF(p1, p2);
    }

    painter->save();
    painter->setPen(QPen(QBrush(_yAxisMajorGridLineColor), 1.0, _yAxisMajorGridLineStyle));
    painter->drawLines(yMajorTickLines);
    painter->restore();
  }
}


void PlotItem::paintMinorGridLines(QPainter *painter) {

  QRectF rect = plotRect();

  if (_drawXAxisMinorGridLines) {
    QVector<QLineF> xMinorTickLines;
    foreach (qreal x, _bottomAxisMinorTicks) {
      QPointF p1 = QPointF(mapXToPlot(x), plotRect().bottom());
      QPointF p2 = p1 - QPointF(0, rect.height());
      xMinorTickLines << QLineF(p1, p2);
    }
    painter->save();
    painter->setPen(QPen(QBrush(_xAxisMinorGridLineColor), 1.0, _xAxisMinorGridLineStyle));
    painter->drawLines(xMinorTickLines);
    painter->restore();
  }

  if (_drawYAxisMinorGridLines) {
    QVector<QLineF> yMinorTickLines;
    foreach (qreal y, _leftAxisMinorTicks) {
      QPointF p1 = QPointF(plotRect().left(), mapYToPlot(y));
      QPointF p2 = p1 + QPointF(rect.width(), 0);
      yMinorTickLines << QLineF(p1, p2);
    }

    painter->save();
    painter->setPen(QPen(QBrush(_yAxisMinorGridLineColor), 1.0, _yAxisMinorGridLineStyle));
    painter->drawLines(yMinorTickLines);
    painter->restore();
  }
}


void PlotItem::paintMajorTicks(QPainter *painter) {

  qreal majorTickLength = qMin(rect().width(), rect().height()) * .02; //two percent

  if (_drawXAxisMajorTicks) {
    QVector<QLineF> xMajorTickLines;
    foreach (qreal x, _bottomAxisMajorTicks) {
      QPointF p1 = QPointF(mapXToPlot(x), plotRect().bottom());
      QPointF p2 = p1 - QPointF(0, majorTickLength);
      xMajorTickLines << QLineF(p1, p2);

      p1.setY(plotRect().top());
      p2 = p1 + QPointF(0, majorTickLength);
      xMajorTickLines << QLineF(p1, p2);
    }

    painter->drawLines(xMajorTickLines);
  }

  if (_drawYAxisMajorTicks) {
    QVector<QLineF> yMajorTickLines;
    foreach (qreal y, _leftAxisMajorTicks) {
      QPointF p1 = QPointF(plotRect().left(), mapYToPlot(y));
      QPointF p2 = p1 + QPointF(majorTickLength, 0);
      yMajorTickLines << QLineF(p1, p2);

      p1.setX(plotRect().right());
      p2 = p1 - QPointF(majorTickLength, 0);
      yMajorTickLines << QLineF(p1, p2);
    }

    painter->drawLines(yMajorTickLines);
  }
}


void PlotItem::paintMinorTicks(QPainter *painter) {

  qreal minorTickLength = qMin(rect().width(), rect().height()) * 0.01; //one percent

  if (_drawXAxisMinorTicks) {
    QVector<QLineF> xMinorTickLines;
    foreach (qreal x, _bottomAxisMinorTicks) {
      QPointF p1 = QPointF(mapXToPlot(x), plotRect().bottom());
      QPointF p2 = p1 - QPointF(0, minorTickLength);
      xMinorTickLines << QLineF(p1, p2);

      p1.setY(plotRect().top());
      p2 = p1 + QPointF(0, minorTickLength);
      xMinorTickLines << QLineF(p1, p2);
    }

    painter->drawLines(xMinorTickLines);
  }

  if (_drawYAxisMinorTicks) {
    QVector<QLineF> yMinorTickLines;
    foreach (qreal y, _leftAxisMinorTicks) {
      QPointF p1 = QPointF(plotRect().left(), mapYToPlot(y));
      QPointF p2 = p1 + QPointF(minorTickLength, 0);
      yMinorTickLines << QLineF(p1, p2);

      p1.setX(plotRect().right());
      p2 = p1 - QPointF(minorTickLength, 0);
      yMinorTickLines << QLineF(p1, p2);
    }

    painter->drawLines(yMinorTickLines);
  }
}


void PlotItem::paintBottomTickLabels(QPainter *painter) {

  QRectF xLabelRect;
  int flags = Qt::TextSingleLine | Qt::AlignCenter;

  QMapIterator<qreal, QString> xLabelIt(_bottomAxisLabels);
  while (xLabelIt.hasNext()) {
    xLabelIt.next();

    QRectF bound = painter->boundingRect(QRectF(), flags, xLabelIt.value());
    QPointF p = QPointF(mapXToPlot(xLabelIt.key()), plotRect().bottom() + bound.height() / 2.0);
    bound.moveCenter(p);

    if (xLabelRect.isValid()) {
      xLabelRect = xLabelRect.united(bound);
    } else {
      xLabelRect = bound;
    }

    if ((rect().left() < bound.left()) && (rect().right() > bound.right())) {
      painter->drawText(bound, flags, xLabelIt.value());
    }
  }

  if (!_bottomBaseLabel.isEmpty()) {
    QRectF bound = painter->boundingRect(QRectF(), flags, _bottomBaseLabel);
    QPointF p = QPointF(plotRect().left(), plotRect().bottom() + bound.height() * 2.0);
    bound.moveBottomLeft(p);

    if (xLabelRect.isValid()) {
      xLabelRect = xLabelRect.united(bound);
    } else {
      xLabelRect = bound;
    }

    painter->drawText(bound, flags, _bottomBaseLabel);
  }
  _xLabelRect = xLabelRect;

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "xLabelRect:" << xLabelRect;
//   painter->fillRect(xLabelRect, Qt::green);
//   painter->restore();
}


void PlotItem::paintLeftTickLabels(QPainter *painter) {

  QRectF yLabelRect;
  int flags = Qt::TextSingleLine | Qt::AlignVCenter;

  QMapIterator<qreal, QString> yLabelIt(_leftAxisLabels);
  while (yLabelIt.hasNext()) {
    yLabelIt.next();

    QRectF bound = painter->boundingRect(QRectF(), flags, yLabelIt.value());
    bound.setWidth(bound.width() + 6);
    QPointF p = QPointF(plotRect().left() - (bound.width() / 2.0), mapYToPlot(yLabelIt.key()));
    bound.moveCenter(p);

    if (yLabelRect.isValid()) {
      yLabelRect = yLabelRect.united(bound);
    } else {
      yLabelRect = bound;
    }

    if ((rect().top() < bound.top()) && (rect().bottom() > bound.bottom())) {
      painter->drawText(bound, flags, yLabelIt.value());
    }
  }

  if (!_leftBaseLabel.isEmpty()) {
    painter->save();
    QTransform t;
    t.rotate(90.0);
    painter->rotate(-90.0);

    QRectF bound = painter->boundingRect(QRectF(), flags, _leftBaseLabel);
    bound = QRectF(bound.x(), bound.bottomRight().y() - bound.width(), bound.height(), bound.width());
    QPointF p = QPointF(rect().left(), plotRect().bottom());
    bound.moveBottomLeft(p);

    if (yLabelRect.isValid()) {
      yLabelRect = yLabelRect.united(bound);
    } else {
      yLabelRect = bound;
    }

    painter->drawText(t.mapRect(bound), flags, _leftBaseLabel);
    painter->restore();
  }
  _yLabelRect = yLabelRect;

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "yLabelRect:" << yLabelRect;
//   painter->fillRect(yLabelRect, Qt::green);
//   painter->restore();
}


void PlotItem::paintTickLabels(QPainter *painter) {

  if (isBottomAxisVisible()) {
    paintBottomTickLabels(painter);
  }

  if (isLeftAxisVisible()) {
    paintLeftTickLabels(painter);
  }
}


void PlotItem::paintPlotMarkers(QPainter *painter) {

  QRectF rect = plotRect();

  QVector<QLineF> xPlotMarkers;
  _xAxisPlotMarkers.updateMarkers();
  foreach (double x, _xAxisPlotMarkers.markers()) {
    if (x > _xMin && x < _xMax) {
      QPointF p1 = QPointF(mapXToPlot(x), plotRect().bottom());
      QPointF p2 = p1 - QPointF(0, rect.height());
      xPlotMarkers << QLineF(p1, p2);
    }
  }

  if (!xPlotMarkers.isEmpty()) {
    painter->save();
    painter->setPen(QPen(QBrush(_xAxisPlotMarkers.lineColor()), _xAxisPlotMarkers.lineWidth(), _xAxisPlotMarkers.lineStyle()));
    painter->drawLines(xPlotMarkers);
    painter->restore();
  }

  QVector<QLineF> yPlotMarkers;
  _yAxisPlotMarkers.updateMarkers();
  foreach (double y, _yAxisPlotMarkers.markers()) {
    if (y > _yMin && y < _yMax) {
      QPointF p1 = QPointF(plotRect().left(), mapYToPlot(y));
      QPointF p2 = p1 + QPointF(rect.width(), 0);
      yPlotMarkers << QLineF(p1, p2);
    }
  }

  if (!yPlotMarkers.isEmpty()) {
    painter->save();
    painter->setPen(QPen(QBrush(_yAxisPlotMarkers.lineColor()), _yAxisPlotMarkers.lineWidth(), _yAxisPlotMarkers.lineStyle()));
    painter->drawLines(yPlotMarkers);
    painter->restore();
  }

}


QString PlotItem::interpretLabel(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double base, double lastValue) {
  double value = convertTimeValueToJD(axisInterpretation, base);
  double scaleValue = convertTimeValueToJD(axisInterpretation, lastValue) - value;

  switch (axisInterpretation) {
    case AXIS_INTERP_YEAR:
      scaleValue *= 365.25 * 24.0 * 60.0 * 60.0;
      break;
    case AXIS_INTERP_CTIME:
      break;
    case AXIS_INTERP_JD:
    case AXIS_INTERP_MJD:
    case AXIS_INTERP_RJD:
      scaleValue *= 24.0 * 60.0 * 60.0;
      break;
    case AXIS_INTERP_AIT:
      break;
  }

  QString label;

  // print value in appropriate format
  switch (axisDisplay) {
    case AXIS_DISPLAY_YEAR:
      value -= JD1900 + 0.5;
      value /= 365.25;
      value += 1900.0;
      label = i18n("J");
      label += QString::number(value, 'g', FULL_PRECISION);
      label += " [years]";
      break;
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
      label = convertJDToDateString(axisInterpretation, axisDisplay, value);
      if( scaleValue > 10.0 * 24.0 * 60.0 * 60.0 ) {
        label += i18n(" [days]");
      } else if( scaleValue > 10.0 * 24.0 * 60.0 ) {
        label += i18n(" [hours]");
      } else if( scaleValue > 10.0 * 60.0 ) {
        label += i18n(" [minutes]");
      } else {
        label += i18n(" [seconds]");
      }
      break;
    case AXIS_DISPLAY_JD:
      label = i18n("JD");
      label += QString::number(value, 'g', FULL_PRECISION);
      label += " [days]";
      break;
    case AXIS_DISPLAY_MJD:
      value -= JD_MJD;
      label = i18n("MJD");
      label += QString::number(value, 'g', FULL_PRECISION);
      label += " [days]";
      break;
    case AXIS_DISPLAY_RJD:
      value -= JD_RJD;
      label = i18n("RJD");
      label += QString::number(value, 'g', FULL_PRECISION);
      label += " [days]";
      break;
  }

  return label;
}


double PlotItem::interpretOffset(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double base, double value) {
  double offset;
  offset = value - base;

  offset = convertTimeDiffValueToDays(axisInterpretation, offset);

  // convert difference to desired format
  switch (axisDisplay) {
    case AXIS_DISPLAY_YEAR:
      offset /= 365.25;
      break;
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
      offset *= 24.0 * 60.0 * 60.0;
      break;
    case AXIS_DISPLAY_JD:
    case AXIS_DISPLAY_MJD:
    case AXIS_DISPLAY_RJD:
      break;
  }
  return offset;
}


double PlotItem::convertTimeValueToJD(KstAxisInterpretation axisInterpretation, double valueIn) {
  double value = valueIn;

  switch (axisInterpretation) {
    case AXIS_INTERP_YEAR:
      value -= 1900.0;
      value *= 365.25;
      value += JD1900 + 0.5;
      break;
    case AXIS_INTERP_CTIME:
      value /= 24.0 * 60.0 * 60.0;
      value += JD1970;
      break;
    case AXIS_INTERP_JD:
      break;
    case AXIS_INTERP_MJD:
      value += JD_MJD;
      break;
    case AXIS_INTERP_RJD:
      value += JD_RJD;
      break;
    case AXIS_INTERP_AIT:
      value -= 86400.0 * (365.0 * 12.0 + 3.0);
      // current difference (seconds) between UTC and AIT
      // refer to the following for more information:
      // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
      value -= 32.0;
      value /= 24.0 * 60.0 * 60.0;
      value += JD1970;
    default:
      break;
  }

  return value;
}


double PlotItem::convertTimeDiffValueToDays(KstAxisInterpretation axisInterpretation, double offsetIn) {
  double offset = offsetIn;

  switch (axisInterpretation) {
    case AXIS_INTERP_YEAR:
      offset *= 365.25;
      break;
    case AXIS_INTERP_CTIME:
      offset /= 24.0 * 60.0 * 60.0;
      break;
    case AXIS_INTERP_JD:
    case AXIS_INTERP_MJD:
    case AXIS_INTERP_RJD:
      break;
    case AXIS_INTERP_AIT:
      offset /= 24.0 * 60.0 * 60.0;
      break;
    default:
      break;
  }

  return offset;
}


QString PlotItem::convertJDToDateString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double dJD) {
  QString label;
  QDate date;

  int accuracy;
  double xdelta = (projectionRect().right()-projectionRect().left())/double(projectionRect().width());
  xdelta = convertTimeDiffValueToDays(axisInterpretation, xdelta);
  xdelta *= 24.0 * 60.0 * 60.0;

  if (xdelta == 0.0) {
    accuracy = FULL_PRECISION;
  } else {
    accuracy = 1 - int(log10(xdelta));
    if (accuracy < 0) {
      accuracy = 0;
    }
  }

  // utcOffset() is returned in seconds... as it must be since
  //  some time zones are not an integer number of hours offset
  //  from UTC...
  dJD += double(Settings::globalSettings()->utcOffset()) / 86400.0;

  // get the date from the Julian day number
  double dJDDay = floor(dJD);
  double dJDFraction = dJD - dJDDay;

  // gregorian calendar correction
  if (dJD >= 2299160.5) {
    double tmp = int( ( (dJDDay - 1867216.0) - 0.25 ) / 36524.25 );
    dJDDay += 1.0 + tmp - floor(0.25*tmp);
  }

  // correction for half day offset
  double dDayFraction = dJDFraction + 0.5;
  if (dDayFraction >= 1.0) {
    dDayFraction -= 1.0;
    dJDDay += 1.0;
  }

  // get time of day from day fraction
  int hour   = int(dDayFraction*24.0);
  int minute = int((dDayFraction*24.0 - double(hour))*60.0);
  double second = ((dDayFraction*24.0 - double(hour))*60.0 - double(minute))*60.0;

  if (accuracy >= 0) {
    second *= pow(10.0, accuracy);
    second  = floor(second+0.5);
    second /= pow(10.0,accuracy);
    if (second >= 60.0) {
      second -= 60.0;
      minute++;
      if (minute == 60) {
        minute = 0;
        hour++;
        if (hour == 24) {
          hour = 0;
        }
      }
    }
  }

  double j2 = dJDDay + 1524.0;
  double j3 = floor(6680.0 + ( (j2 - 2439870.0) - 122.1 )/365.25);
  double j4 = floor(j3 * 365.25);
  double j5 = floor((j2 - j4)/30.6001);

  int day = int(j2 - j4 - floor(j5*30.6001));
  int month = int(j5 - 1.0);
  if (month > 12) {
    month -= 12;
  }
  int year = int(j3 - 4715.0);
  if (month > 2) {
    --year;
  }
  if (year <= 0) {
    --year;
  }
  // check how many decimal places for the seconds we actually need to show
  if (accuracy > 0) {
    QString strSecond;

    strSecond.sprintf("%02.*f", accuracy, second);
    for (int i=strSecond.length()-1; i>0; i--) {
      if (strSecond.at(i) == '0') {
        accuracy--;
      } else if (!strSecond.at(i).isDigit()) {
        break;
      }
    }
  }

  if (accuracy < 0) {
    accuracy = 0;
  }

  QString seconds;
  QString hourminute;
  hourminute.sprintf(" %02d:%02d:", hour, minute);
  seconds.sprintf(" %02.*f", accuracy, second);
  switch (axisDisplay) {
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
      label.sprintf("%d/%02d/%02d", year, month, day);
      label += hourminute + seconds;
      break;
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
      label.sprintf("%02d/%02d/%d", day, month, year);
      label += hourminute + seconds;
      break;
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
      date.setYMD(year, month, day);
      label = date.toString(Qt::TextDate).toAscii();
      label += hourminute + seconds;
      break;
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
      date.setYMD(year, month, day);
      label = date.toString(Qt::LocalDate).toAscii();
      label += hourminute + seconds;
      break;
    default:
      break;
  }
  return label;
}


QRectF PlotItem::plotAxisRect() const {
  qreal left = isLeftLabelVisible() ? leftLabelMargin() : 0.0;
  qreal bottom = isBottomLabelVisible() ? bottomLabelMargin() : 0.0;
  qreal right = isRightLabelVisible() ? rightLabelMargin() : 0.0;
  qreal top = isTopLabelVisible() ? topLabelMargin() : 0.0;

  QPointF topLeft(rect().topLeft() + QPointF(left, top));
  QPointF bottomRight(rect().bottomRight() - QPointF(right, bottom));

  return QRectF(topLeft, bottomRight);
}


QRectF PlotItem::plotRect() const {
  //the PlotRenderItems use this to set their rects
  QRectF plot = plotAxisRect();
  qreal xOffset = isBottomAxisVisible() ? axisMarginHeight() : 0.0;
  qreal yOffset = isLeftAxisVisible() ? axisMarginWidth() : 0.0;

  plot.setLeft(plot.left() + yOffset);
  plot.setBottom(plot.bottom() - xOffset);
  return plot;
}


QRectF PlotItem::projectionRect() const {
  return _projectionRect;
}


bool PlotItem::isTiedZoom() const {
  return _isTiedZoom;
}


void PlotItem::setTiedZoom(bool tiedZoom) {
  if (_isTiedZoom == tiedZoom)
    return;

  _isTiedZoom = tiedZoom;

  if (_isTiedZoom)
    PlotItemManager::self()->addTiedZoomPlot(this);
  else
    PlotItemManager::self()->removeTiedZoomPlot(this);

  //FIXME ugh, this is expensive, but need to redraw the renderitems checkboxes...
  update();
}


PlotItem::ZoomMode PlotItem::xAxisZoomMode() const {
  return _xAxisZoomMode;
}


void PlotItem::setXAxisZoomMode(ZoomMode mode) {
  _xAxisZoomMode = mode;
}


PlotItem::ZoomMode PlotItem::yAxisZoomMode() const {
  return _yAxisZoomMode;
}


void PlotItem::setYAxisZoomMode(ZoomMode mode) {
  _yAxisZoomMode = mode;
}


qreal PlotItem::leftLabelMargin() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotLabelMarginWidth(this);
  } else {
    return calculatedLeftLabelMargin();
  }
}


qreal PlotItem::rightLabelMargin() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotLabelMarginWidth(this);
  } else {
    return calculatedRightLabelMargin();
  }
}


qreal PlotItem::topLabelMargin() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotLabelMarginHeight(this);
  } else {
    return calculatedTopLabelMargin();
  }
}


qreal PlotItem::bottomLabelMargin() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotLabelMarginHeight(this);
  } else {
    return calculatedBottomLabelMargin();
  }
}


qreal PlotItem::axisMarginWidth() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotAxisMarginWidth(this);
  } else {
    return calculatedAxisMarginWidth();
  }
}


qreal PlotItem::axisMarginHeight() const {
  ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(parentItem());
  if (viewItem && viewItem->layout()) {
    return viewItem->layout()->plotAxisMarginHeight(this);
  } else {
    return calculatedAxisMarginHeight();
  }
}


bool PlotItem::xAxisLog() const {
  return _xAxisLog;
}


void PlotItem::setXAxisLog(bool log) {
  _xAxisLog = log;
}


bool PlotItem::yAxisLog() const {
  return _yAxisLog;
}


void PlotItem::setYAxisLog(bool log) {
  _yAxisLog = log;
}


int PlotItem::xAxisSignificantDigits() const {
  return _xAxisSignificantDigits;
}


void PlotItem::setXAxisSignificantDigits(const int digits) {
  _xAxisSignificantDigits = digits;
}


int PlotItem::yAxisSignificantDigits() const {
  return _yAxisSignificantDigits;
}


void PlotItem::setYAxisSignificantDigits(const int digits) {
  _yAxisSignificantDigits = digits;
}


PlotItem::MajorTickMode PlotItem::xAxisMajorTickMode() const {
  return _xAxisMajorTickMode;
}


void PlotItem::setXAxisMajorTickMode(PlotItem::MajorTickMode mode) {
  _xAxisMajorTickMode = mode;
}


PlotItem::MajorTickMode PlotItem::yAxisMajorTickMode() const {
  return _yAxisMajorTickMode;
}


void PlotItem::setYAxisMajorTickMode(PlotItem::MajorTickMode mode) {
  _yAxisMajorTickMode = mode;
}


int PlotItem::xAxisMinorTickCount() const {
  return _xAxisMinorTickCount;
}


void PlotItem::setXAxisMinorTickCount(const int count) {
  _xAxisMinorTickCount = count;
}


int PlotItem::yAxisMinorTickCount() const {
  return _yAxisMinorTickCount;
}


void PlotItem::setYAxisMinorTickCount(const int count) {
  _yAxisMinorTickCount = count;
}


bool PlotItem::drawXAxisMajorTicks() const {
  return _drawXAxisMajorTicks;
}


void PlotItem::setDrawXAxisMajorTicks(bool draw) {
  _drawXAxisMajorTicks = draw;
}


bool PlotItem::drawYAxisMajorTicks() const {
  return _drawYAxisMajorTicks;
}


void PlotItem::setDrawYAxisMajorTicks(bool draw) {
  _drawYAxisMajorTicks = draw;
}


bool PlotItem::drawXAxisMinorTicks() const {
  return _drawXAxisMinorTicks;
}


void PlotItem::setDrawXAxisMinorTicks(bool draw) {
  _drawXAxisMinorTicks = draw;
}


bool PlotItem::drawYAxisMinorTicks() const {
  return _drawYAxisMinorTicks;
}


void PlotItem::setDrawYAxisMinorTicks(bool draw) {
  _drawYAxisMinorTicks = draw;
}


bool PlotItem::drawXAxisMajorGridLines() const {
  return _drawXAxisMajorGridLines;
}


void PlotItem::setDrawXAxisMajorGridLines(bool draw) {
  _drawXAxisMajorGridLines = draw;
}


bool PlotItem::drawYAxisMajorGridLines() const {
  return _drawYAxisMajorGridLines;
}


void PlotItem::setDrawYAxisMajorGridLines(bool draw) {
  _drawYAxisMajorGridLines = draw;
}


bool PlotItem::drawXAxisMinorGridLines() const {
  return _drawXAxisMinorGridLines;
}


void PlotItem::setDrawXAxisMinorGridLines(bool draw) {
  _drawXAxisMinorGridLines = draw;
}


bool PlotItem::drawYAxisMinorGridLines() const {
  return _drawYAxisMinorGridLines;
}


void PlotItem::setDrawYAxisMinorGridLines(bool draw) {
  _drawYAxisMinorGridLines = draw;
}


QColor PlotItem::xAxisMajorGridLineColor() const {
  return _xAxisMajorGridLineColor;
}


void PlotItem::setXAxisMajorGridLineColor(const QColor &color) {
  _xAxisMajorGridLineColor = color;
}


QColor PlotItem::xAxisMinorGridLineColor() const {
  return _xAxisMinorGridLineColor;
}


void PlotItem::setXAxisMinorGridLineColor(const QColor &color) {
  _xAxisMinorGridLineColor = color;
}


QColor PlotItem::yAxisMajorGridLineColor() const {
  return _yAxisMajorGridLineColor;
}


void PlotItem::setYAxisMajorGridLineColor(const QColor &color) {
  _yAxisMajorGridLineColor = color;
}


QColor PlotItem::yAxisMinorGridLineColor() const {
  return _yAxisMinorGridLineColor;
}


void PlotItem::setYAxisMinorGridLineColor(const QColor &color) {
  _yAxisMinorGridLineColor = color;
}


Qt::PenStyle PlotItem::xAxisMajorGridLineStyle() const {
  return _xAxisMajorGridLineStyle;
}


void PlotItem::setXAxisMajorGridLineStyle(const Qt::PenStyle style) {
  _xAxisMajorGridLineStyle = style;
}


Qt::PenStyle PlotItem::xAxisMinorGridLineStyle() const {
  return _xAxisMinorGridLineStyle;
}


void PlotItem::setXAxisMinorGridLineStyle(const Qt::PenStyle style) {
  _xAxisMinorGridLineStyle = style;
}


Qt::PenStyle PlotItem::yAxisMajorGridLineStyle() const {
  return _yAxisMajorGridLineStyle;
}


void PlotItem::setYAxisMajorGridLineStyle(const Qt::PenStyle style) {
  _yAxisMajorGridLineStyle = style;
}


Qt::PenStyle PlotItem::yAxisMinorGridLineStyle() const {
  return _yAxisMinorGridLineStyle;
}


void PlotItem::setYAxisMinorGridLineStyle(const Qt::PenStyle style) {
  _yAxisMinorGridLineStyle = style;
}


void PlotItem::updateScale() {
  if (_xAxisLog) {
    _xMax = logXHi(projectionRect().right());
    _xMin = logXLo(projectionRect().left());
  } else {
    _xMax = projectionRect().right();
    _xMin = projectionRect().left();
  }

  if (_yAxisLog) {
    _yMax = logYHi(projectionRect().bottom());
    _yMin = logYLo(projectionRect().top());
  } else {
    _yMax = projectionRect().bottom();
    _yMin = projectionRect().top();
  }
}


QRectF PlotItem::mapToProjection(const QRectF &rect) {
  QRectF projRect;

  // Invert and convert points.
  QPointF topLeft = mapToProjection(rect.bottomLeft());
  QPointF bottomRight = mapToProjection(rect.topRight());

  projRect.setTopLeft(topLeft);

  projRect.setWidth(bottomRight.x() - topLeft.x());
  projRect.setHeight(bottomRight.y() - topLeft.y());

  return projRect;
}


QPointF PlotItem::mapToProjection(const QPointF &point) {
  QRectF pr = plotRect();
  double xpos, ypos;

  updateScale();

  if (_xAxisReversed) {
    xpos = (double)(pr.right() - point.x())/(double)pr.width();
  } else {
    xpos = (double)(point.x() - pr.left())/(double)pr.width();
  }
  xpos = xpos * (_xMax - _xMin) + _xMin;

  if (_xAxisLog) {
    xpos = pow(10, xpos);
  }

  if (_yAxisReversed) {
    ypos = (double)(point.y() - pr.top())/(double)pr.height();
  } else {
    ypos = (double)(pr.bottom() - point.y())/(double)pr.height();
  }
  ypos = ypos * (_yMax - _yMin) + _yMin;

  if (_yAxisLog) {
    ypos = pow(10, ypos);
  }

  return QPointF(xpos, ypos);
}


QPointF PlotItem::mapToPlot(const QPointF &point) const {
  return QPointF(mapXToPlot(point.x()), mapYToPlot(point.y()));
}


qreal PlotItem::mapXToPlot(const qreal &x) const {
  QRectF pr = plotRect();
  double newX = x;

  if (_xAxisLog) {
    newX = logXLo(x);
  }

  newX -= _xMin;
  newX = newX / (_xMax - _xMin);

  newX = newX * pr.width();

  if (_xAxisLog && x == -350) {
    newX = 0;
  }

  if (_xAxisReversed) {
    newX = pr.right() - newX;
  } else {
    newX = newX + pr.left();
  }
  return newX;
}


qreal PlotItem::mapYToPlot(const qreal &y) const {
  QRectF pr = plotRect();
  double newY = y;

  if (_yAxisLog) {
    newY = logYLo(y);
  }

  newY -= _yMin;
  newY = newY / (_yMax - _yMin);

  newY = newY * pr.height();

  if (_yAxisLog && y == -350) {
    newY = 0;
  }

  if (_yAxisReversed) {
    newY = newY + pr.top();
  } else {
    newY = pr.bottom() - newY;
  }
  return newY;
}


QFont PlotItem::rightLabelFont() const {
  return _rightLabelFont;
}


void PlotItem::setRightLabelFont(const QFont &font) {
  _rightLabelFont = font;
}


QFont PlotItem::topLabelFont() const {
  return _topLabelFont;
}


void PlotItem::setTopLabelFont(const QFont &font) {
  _topLabelFont = font;
}


QFont PlotItem::leftLabelFont() const {
  return _leftLabelFont;
}


void PlotItem::setLeftLabelFont(const QFont &font) {
  _leftLabelFont = font;
}


QFont PlotItem::bottomLabelFont() const {
  return _bottomLabelFont;
}


void PlotItem::setBottomLabelFont(const QFont &font) {
  _bottomLabelFont = font;
}


qreal PlotItem::rightLabelFontScale() const {
  return _rightLabelFontScale;
}


void PlotItem::setRightLabelFontScale(const qreal scale) {
  _rightLabelFontScale = scale;
}


qreal PlotItem::leftLabelFontScale() const {
  return _leftLabelFontScale;
}


void PlotItem::setLeftLabelFontScale(const qreal scale) {
  _leftLabelFontScale = scale;
}


qreal PlotItem::topLabelFontScale() const {
  return _topLabelFontScale;
}


void PlotItem::setTopLabelFontScale(const qreal scale) {
  _topLabelFontScale = scale;
}


qreal PlotItem::bottomLabelFontScale() const {
  return _bottomLabelFontScale;
}


void PlotItem::setBottomLabelFontScale(const qreal scale) {
  _bottomLabelFontScale = scale;
}


QString PlotItem::leftLabelOverride() const {
  if (_leftLabelOverride.isEmpty()) {
    return leftLabel();
  } else {
    return _leftLabelOverride;
  }
}


void PlotItem::setLeftLabelOverride(const QString &label) {
  if (label == leftLabel()) {
    _leftLabelOverride.clear();
  } else {
    _leftLabelOverride = label;
  }
}


QString PlotItem::bottomLabelOverride() const {
  if (_bottomLabelOverride.isEmpty()) {
    return bottomLabel();
  } else {
    return _bottomLabelOverride;
  }
}


void PlotItem::setBottomLabelOverride(const QString &label) {
  if (label == bottomLabel()) {
    _bottomLabelOverride.clear();
  } else {
    _bottomLabelOverride = label;
  }
}


QString PlotItem::topLabelOverride() const {
  if (_topLabelOverride.isEmpty()) {
    return topLabel();
  } else {
    return _topLabelOverride;
  }
}


void PlotItem::setTopLabelOverride(const QString &label) {
  if (label == topLabel()) {
    _topLabelOverride.clear();
  } else {
    _topLabelOverride = label;
  }
}


QString PlotItem::rightLabelOverride() const {
  if (_rightLabelOverride.isEmpty()) {
    return rightLabel();
  } else {
    return _rightLabelOverride;
  }
}


void PlotItem::setRightLabelOverride(const QString &label) {
  if (label == rightLabel()) {
    _rightLabelOverride.clear();
  } else {
    _rightLabelOverride = label;
  }
}


QString PlotItem::leftLabel() const {
  foreach (PlotRenderItem *renderer, renderItems()) {
    if (!renderer->leftLabel().isEmpty())
      return renderer->leftLabel();
  }
  return QString();
}


QString PlotItem::bottomLabel() const {
  foreach (PlotRenderItem *renderer, renderItems()) {
    if (!renderer->bottomLabel().isEmpty())
      return renderer->bottomLabel();
  }
  return QString();
}


QString PlotItem::rightLabel() const {
  foreach (PlotRenderItem *renderer, renderItems()) {
    if (!renderer->rightLabel().isEmpty())
      return renderer->rightLabel();
  }
  return QString();
}


QString PlotItem::topLabel() const {
  foreach (PlotRenderItem *renderer, renderItems()) {
    if (!renderer->topLabel().isEmpty())
      return renderer->topLabel();
  }
  return QString();
}


void PlotItem::setTopSuppressed(bool suppressed) {
  setTopLabelVisible(!suppressed);
}


void PlotItem::setRightSuppressed(bool suppressed) {
  setRightLabelVisible(!suppressed);
}


void PlotItem::setLeftSuppressed(bool suppressed) {
  setLeftLabelVisible(!suppressed);
  setLeftAxisVisible(!suppressed);
}


void PlotItem::setBottomSuppressed(bool suppressed) {
  setBottomLabelVisible(!suppressed);
  setBottomAxisVisible(!suppressed);
}


bool PlotItem::isBottomAxisVisible() const {
  return _isBottomAxisVisible;
}


void PlotItem::setBottomAxisVisible(bool visible) {
  if (_isBottomAxisVisible == visible)
    return;

  _isBottomAxisVisible = visible;
  emit marginsChanged();
}


bool PlotItem::isLeftAxisVisible() const {
  return _isLeftAxisVisible;
}


void PlotItem::setLeftAxisVisible(bool visible) {
  if (_isLeftAxisVisible == visible)
    return;

  _isLeftAxisVisible = visible;
  emit marginsChanged();
}


bool PlotItem::isLeftLabelVisible() const {
  return _isLeftLabelVisible;
}


void PlotItem::setLeftLabelVisible(bool visible) {
  if (_isLeftLabelVisible == visible)
    return;

  _isLeftLabelVisible = visible;
  emit marginsChanged();
}


bool PlotItem::isBottomLabelVisible() const {
  return _isBottomLabelVisible;
}


void PlotItem::setBottomLabelVisible(bool visible) {
  if (_isBottomLabelVisible == visible)
    return;

  _isBottomLabelVisible = visible;
  emit marginsChanged();
}


bool PlotItem::isRightLabelVisible() const {
  return _isRightLabelVisible;
}


void PlotItem::setRightLabelVisible(bool visible) {
  if (_isRightLabelVisible == visible)
    return;

  _isRightLabelVisible = visible;
  emit marginsChanged();
}


bool PlotItem::isTopLabelVisible() const {
  return _isTopLabelVisible;
}


void PlotItem::setTopLabelVisible(bool visible) {
  if (_isTopLabelVisible == visible)
    return;

  _isTopLabelVisible = visible;
  emit marginsChanged();
}


void PlotItem::setLabelsVisible(bool visible) {
  setLeftLabelVisible(visible);
  setRightLabelVisible(visible);
  setBottomLabelVisible(visible);
  setTopLabelVisible(visible);
  setBottomAxisVisible(visible);
  setLeftAxisVisible(visible);
}


bool PlotItem::xAxisReversed() const {
  return _xAxisReversed;
}


void PlotItem::setXAxisReversed(const bool enabled) {
  _xAxisReversed = enabled;
}


bool PlotItem::yAxisReversed() const {
  return _yAxisReversed;
}


void PlotItem::setYAxisReversed(const bool enabled) {
  _yAxisReversed = enabled;
}


bool PlotItem::xAxisBaseOffset() const {
  return _xAxisBaseOffset;
}


void PlotItem::setXAxisBaseOffset(const bool enabled) {
  _xAxisBaseOffset = enabled;
}


bool PlotItem::yAxisBaseOffset() const {
  return _yAxisBaseOffset;
}


void PlotItem::setYAxisBaseOffset(const bool enabled) {
  _yAxisBaseOffset = enabled;
}


bool PlotItem::xAxisInterpret() const {
  return _xAxisInterpret;
}


void PlotItem::setXAxisInterpret(const bool enabled) {
  _xAxisInterpret = enabled;
}


bool PlotItem::yAxisInterpret() const {
  return _yAxisInterpret;
}


void PlotItem::setYAxisInterpret(const bool enabled) {
  _yAxisInterpret = enabled;
}


KstAxisDisplay PlotItem::xAxisDisplay() const {
  return _xAxisDisplay;
}


void PlotItem::setXAxisDisplay(const KstAxisDisplay display) {
  _xAxisDisplay = display;
}


KstAxisDisplay PlotItem::yAxisDisplay() const {
  return _yAxisDisplay;
}


void PlotItem::setYAxisDisplay(const KstAxisDisplay display) {
  _yAxisDisplay = display;
}


KstAxisInterpretation PlotItem::xAxisInterpretation() const {
  return _xAxisInterpretation;
}


void PlotItem::setXAxisInterpretation(const KstAxisInterpretation display) {
  _xAxisInterpretation = display;
}


KstAxisInterpretation PlotItem::yAxisInterpretation() const {
  return _yAxisInterpretation;
}


void PlotItem::setYAxisInterpretation(const KstAxisInterpretation display) {
  _yAxisInterpretation = display;
}


qreal PlotItem::calculatedLabelMarginWidth() const {
  qreal m = qMax(_calculatedLeftLabelMargin, _calculatedRightLabelMargin);

  //No more than 1/4 the width of the plot
  if (width() < m * 4)
    return width() / 4;

  return m;
}


qreal PlotItem::calculatedLeftLabelMargin() const {
  qreal m = qMax(LEFT_MARGIN, _calculatedLeftLabelMargin);

  //No more than 1/4 the width of the plot
  if (width() < m * 4)
    return width() / 4;

  return m;
}


void PlotItem::setCalculatedLeftLabelMargin(qreal margin) {
  qreal before = this->calculatedLeftLabelMargin();
  _calculatedLeftLabelMargin = margin;
  if (before != this->calculatedLeftLabelMargin())
    emit marginsChanged();
}


qreal PlotItem::calculatedRightLabelMargin() const {
  qreal m = qMax(RIGHT_MARGIN, _calculatedRightLabelMargin);

  //No more than 1/4 the width of the plot
  if (width() < m * 4)
    return width() / 4;

  return m;
}


void PlotItem::setCalculatedRightLabelMargin(qreal margin) {
  qreal before = this->calculatedRightLabelMargin();
  _calculatedRightLabelMargin = margin;
  if (before != this->calculatedRightLabelMargin())
    emit marginsChanged();
}


qreal PlotItem::calculatedLabelMarginHeight() const {
  qreal m = qMax(_calculatedTopLabelMargin, _calculatedBottomLabelMargin);

  //No more than 1/4 the height of the plot
  if (height() < m * 4)
    return height() / 4;

  return m;
}


// void PlotItem::setCalculatedLabelMarginHeight(qreal marginHeight) {
//   qreal before = this->calculatedLabelMarginHeight();
//   _calculatedLabelMarginHeight = marginHeight;
//   if (before != this->calculatedLabelMarginHeight())
//     emit marginsChanged();
// }


qreal PlotItem::calculatedTopLabelMargin() const {
  qreal m = qMax(TOP_MARGIN, _calculatedTopLabelMargin);

  //No more than 1/4 the height of the plot
  if (height() < m * 4)
    return height() / 4;

  return m;
}


void PlotItem::setCalculatedTopLabelMargin(qreal margin) {
  qreal before = this->calculatedTopLabelMargin();
  _calculatedTopLabelMargin = margin;
  if (before != this->calculatedTopLabelMargin())
    emit marginsChanged();
}


qreal PlotItem::calculatedBottomLabelMargin() const {
  qreal m = qMax(BOTTOM_MARGIN, _calculatedBottomLabelMargin);

  //No more than 1/4 the height of the plot
  if (height() < m * 4)
    return height() / 4;

  return m;
}


void PlotItem::setCalculatedBottomLabelMargin(qreal margin) {
  qreal before = this->calculatedBottomLabelMargin();
  _calculatedBottomLabelMargin = margin;
  if (before != this->calculatedBottomLabelMargin())
    emit marginsChanged();
}


QRectF PlotItem::topLabelRect(bool calc) const {
  if (calc)
    return QRectF(0.0, 0.0, width() - calculatedLeftLabelMargin() - calculatedRightLabelMargin(), calculatedTopLabelMargin());
  else
    return QRectF(0.0, 0.0, width() - leftLabelMargin() - rightLabelMargin(), topLabelMargin());
}


QRectF PlotItem::bottomLabelRect(bool calc) const {
  if (calc)
    return QRectF(0.0, 0.0, width() - calculatedLeftLabelMargin() - calculatedRightLabelMargin(), calculatedBottomLabelMargin());
  else
    return QRectF(0.0, 0.0, width() - leftLabelMargin() - rightLabelMargin(), bottomLabelMargin());
}


QRectF PlotItem::leftLabelRect(bool calc) const {
  if (calc)
    return QRectF(0.0, 0.0, calculatedLeftLabelMargin(), height() - calculatedTopLabelMargin() - calculatedBottomLabelMargin());
  else
    return QRectF(0.0, 0.0, leftLabelMargin(), height() - topLabelMargin() - bottomLabelMargin());
}


QRectF PlotItem::rightLabelRect(bool calc) const {
  if (calc)
    return QRectF(0.0, 0.0, calculatedRightLabelMargin(), height() - calculatedTopLabelMargin() - calculatedBottomLabelMargin());
  else
    return QRectF(0.0, 0.0, rightLabelMargin(), height() - topLabelMargin() - bottomLabelMargin());
}


QFont PlotItem::calculatedLeftLabelFont() {
  QFont font(_leftLabelFont);
  font.setPointSizeF(parentView()->defaultFont(_leftLabelFontScale).pointSizeF());

  return font;
}


QFont PlotItem::calculatedRightLabelFont() {
  QFont font(_rightLabelFont);
  font.setPointSizeF(parentView()->defaultFont(_rightLabelFontScale).pointSizeF());

  return font;
}


QFont PlotItem::calculatedTopLabelFont() {
  QFont font(_topLabelFont);
  font.setPointSizeF(parentView()->defaultFont(_topLabelFontScale).pointSizeF());

  return font;
}


QFont PlotItem::calculatedBottomLabelFont() {
  QFont font(_bottomLabelFont);
  font.setPointSizeF(parentView()->defaultFont(_bottomLabelFontScale).pointSizeF());

  return font;
}


void PlotItem::paintLeftLabel(QPainter *painter) {
  if (!isLeftLabelVisible())
    return;

  painter->save();
  QTransform t;
  t.rotate(90.0);
  painter->rotate(-90.0);

  painter->setFont(calculatedLeftLabelFont());

  QRectF leftLabel = leftLabelRect(false);
  leftLabel.moveTopRight(plotAxisRect().topLeft());
  painter->drawText(t.mapRect(leftLabel), Qt::TextWordWrap | Qt::AlignCenter, leftLabelOverride());

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "leftLabel:" << t.mapRect(leftLabel) << endl;
//   painter->fillRect(t.mapRect(leftLabel), Qt::red);
//   painter->restore();

  painter->restore();
}


QSizeF PlotItem::calculateLeftLabelBound(QPainter *painter) {
  if (!isLeftLabelVisible())
    return QSizeF();

  painter->save();
  QTransform t;
  t.rotate(90.0);
  painter->rotate(-90.0);

  painter->setFont(calculatedLeftLabelFont());

  QRectF leftLabelBound = painter->boundingRect(t.mapRect(leftLabelRect(true)),
                                                Qt::TextWordWrap | Qt::AlignCenter, leftLabelOverride());
  painter->restore();

  QSizeF margins;
  margins.setWidth(leftLabelBound.height());
  return margins;
}


void PlotItem::paintBottomLabel(QPainter *painter) {
  if (!isBottomLabelVisible())
    return;

  painter->save();

  painter->setFont(calculatedBottomLabelFont());

  QRectF bottomLabel = bottomLabelRect(false);
  bottomLabel.moveTopLeft(plotAxisRect().bottomLeft());
  painter->drawText(bottomLabel, Qt::TextWordWrap | Qt::AlignCenter, bottomLabelOverride());

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "bottomLabel:" << bottomLabel;
//   painter->fillRect(bottomLabel, Qt::red);
//   painter->restore();

  painter->restore();
}


QSizeF PlotItem::calculateBottomLabelBound(QPainter *painter) {
  if (!isBottomLabelVisible())
    return QSizeF();

  painter->save();

  painter->setFont(calculatedBottomLabelFont());

  QRectF bottomLabelBound = painter->boundingRect(bottomLabelRect(true),
                                                  Qt::TextWordWrap | Qt::AlignCenter, bottomLabelOverride());
  painter->restore();

  QSizeF margins;
  margins.setHeight(bottomLabelBound.height());
  return margins;
}


void PlotItem::paintRightLabel(QPainter *painter) {
  if (!isRightLabelVisible())
    return;

  painter->save();
  QTransform t;
  t.rotate(-90.0);
  painter->rotate(90.0);

  painter->setFont(calculatedRightLabelFont());

  //same as left but painter is translated
  QRectF rightLabel = rightLabelRect(false);
  rightLabel.moveTopLeft(plotAxisRect().topRight());
  painter->drawText(t.mapRect(rightLabel), Qt::TextWordWrap | Qt::AlignCenter, rightLabelOverride());

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "rightLabel:" << t.mapRect(rightLabel) << endl;
//   painter->fillRect(t.mapRect(rightLabel), Qt::red);
//   painter->restore();

  painter->restore();
}


QSizeF PlotItem::calculateRightLabelBound(QPainter *painter) {
  if (!isRightLabelVisible())
    return QSizeF();

  painter->save();
  QTransform t;
  t.rotate(-90.0);
  painter->rotate(90.0);

  painter->setFont(calculatedRightLabelFont());

  QRectF rightLabelBound = painter->boundingRect(t.mapRect(rightLabelRect(true)),
                                                 Qt::TextWordWrap | Qt::AlignCenter, rightLabelOverride());
  painter->restore();

  QSizeF margins;
  margins.setWidth(rightLabelBound.height());
  return margins;
}


void PlotItem::paintTopLabel(QPainter *painter) {
  if (!isTopLabelVisible())
    return;

  painter->save();

  painter->setFont(calculatedTopLabelFont());

  QRectF topLabel = topLabelRect(false);
  topLabel.moveBottomLeft(plotAxisRect().topLeft());
  painter->drawText(topLabel, Qt::TextWordWrap | Qt::AlignCenter, topLabelOverride());

//   painter->save();
//   painter->setOpacity(0.3);
// //   qDebug() << "topLabel:" << topLabel;
//   painter->fillRect(topLabel, Qt::red);
//   painter->restore();

  painter->restore();
}


QSizeF PlotItem::calculateTopLabelBound(QPainter *painter) {
  if (!isTopLabelVisible())
    return QSizeF();

  painter->save();

  painter->setFont(calculatedTopLabelFont());

  QRectF topLabelBound = painter->boundingRect(topLabelRect(true),
                                               Qt::TextWordWrap | Qt::AlignCenter, topLabelOverride());

  painter->restore();

  QSizeF margins;
  margins.setHeight(topLabelBound.height());
  return margins;
}


qreal PlotItem::calculatedAxisMarginWidth() const {
  return _calculatedAxisMarginWidth;
}


void PlotItem::setCalculatedAxisMarginWidth(qreal marginWidth) {
  qreal before = this->calculatedAxisMarginWidth();
  _calculatedAxisMarginWidth = marginWidth;
  if (before != this->calculatedAxisMarginWidth())
    emit marginsChanged();
}


qreal PlotItem::calculatedAxisMarginHeight() const {
  return _calculatedAxisMarginHeight;
}


void PlotItem::setCalculatedAxisMarginHeight(qreal marginHeight) {
  qreal before = this->calculatedAxisMarginHeight();
  _calculatedAxisMarginHeight = marginHeight;
  if (before != this->calculatedAxisMarginHeight())
    emit marginsChanged();
}


void PlotItem::computeLogTicks(QList<qreal> *MajorTicks, QList<qreal> *MinorTicks, QMap<qreal, QString> *Labels, qreal min, qreal max, MajorTickMode tickMode) {

  qreal tick;
  if (max - min <= (double)tickMode*1.5) {
    // show in logarithmic mode with major ticks nicely labelled and the
    // specified number of minor ticks between each major label
    tick = 1.0;
  } else {
    // show in logarithmic mode with major ticks nicely labelled and no minor ticks
    tick = floor((max - min) / (double)tickMode);
  }

  int Low = ceil(min);
  int High = floor(max)+1;
  bool minorLabels = ((High - Low) <= 1);
  for (int i = Low - 1; i <= High; i+=tick) {
    qreal majorPoint = pow(10, i);
    if (majorPoint == 0) majorPoint = -350;
    if (i >= min && i <= max) {
      *MajorTicks << majorPoint;
      *Labels->insert(majorPoint, QString::number(majorPoint, 'g', FULL_PRECISION));
    }

    if (tick == 1.0) {
      // draw minor lines
      bool first = true;
      qreal powMin = pow(10, min), powMax = pow(10, max);
      for (int j = 2; j < 10; j++) {
        qreal minorPoint = majorPoint * j;
        if (minorPoint >= powMin && minorPoint <= powMax) {
          *MinorTicks << minorPoint;
          if (minorLabels && first) {
            *Labels->insert(minorPoint, QString::number(minorPoint, 'g', FULL_PRECISION));
            first = false;
          }
        }
      }
    }
  }
  if (minorLabels && !MinorTicks->isEmpty()) {
    qreal lastMinorTick = MinorTicks->last();
    if (MajorTicks->isEmpty() || MajorTicks->last() < lastMinorTick) {
      if (!Labels->contains(lastMinorTick)) {
        *Labels->insert(lastMinorTick, QString::number(lastMinorTick, 'g', FULL_PRECISION));
      }
    }
  }
}


void PlotItem::generateAxes() {

  QMap<qreal, QString> leftLabels;
  QMap<qreal, QString> bottomLabels;
  QList<qreal> xTicks;
  QList<qreal> xMinTicks;
  updateScale();
  if (_xAxisLog) {
    computeLogTicks(&xTicks, &xMinTicks, &bottomLabels, _xMin, _xMax, _xAxisMajorTickMode);
  } else {
    qreal xMajorTickSpacing = computedMajorTickSpacing(Qt::Horizontal);
    qreal firstXTick = ceil(projectionRect().left() / xMajorTickSpacing) * xMajorTickSpacing;

    int ix = 0;
    qreal nextXTick = firstXTick;
    while (1) {
      nextXTick = firstXTick + (ix++ * xMajorTickSpacing);
      if (!projectionRect().contains(nextXTick, projectionRect().y()))
        break;
      xTicks << nextXTick;
      bottomLabels.insert(nextXTick, QString::number(nextXTick, 'g', FULL_PRECISION));
    }

    qreal xMinorTickSpacing = 0;
    if (_xAxisMinorTickCount > 0) {
      xMinorTickSpacing = xMajorTickSpacing / _xAxisMinorTickCount;
    }

    if (xMinorTickSpacing != 0) {
      qreal firstXMinorTick = (firstXTick - xMajorTickSpacing) + xMinorTickSpacing;

      ix = 0;
      qreal nextXMinorTick = firstXMinorTick;
      while (1) {
        nextXMinorTick = firstXMinorTick + (ix++ * xMinorTickSpacing);
        if (projectionRect().right() < nextXMinorTick)
          break;
        if (!xTicks.contains(nextXMinorTick) && projectionRect().contains(nextXMinorTick, projectionRect().y())) {
          xMinTicks << nextXMinorTick;
        }
      }
    }
  }

  QList<qreal> yTicks;
  QList<qreal> yMinTicks;
  if (_yAxisLog) {
    computeLogTicks(&yTicks, &yMinTicks, &leftLabels, _yMin, _yMax, _yAxisMajorTickMode);
  } else {
    qreal yMajorTickSpacing = computedMajorTickSpacing(Qt::Vertical);
    qreal firstYTick = ceil(projectionRect().top() / yMajorTickSpacing) * yMajorTickSpacing;

    int iy = 0;
    qreal nextYTick = firstYTick;
    while (1) {
      nextYTick = firstYTick + (iy++ * yMajorTickSpacing);
      if (!projectionRect().contains(projectionRect().x(), nextYTick))
        break;
      yTicks << nextYTick;
      leftLabels.insert(nextYTick, QString::number(nextYTick, 'g', FULL_PRECISION));
    }

    qreal yMinorTickSpacing = 0;
    if (_yAxisMinorTickCount > 0) {
      yMinorTickSpacing = yMajorTickSpacing / _yAxisMinorTickCount;
    }
    qreal firstYMinorTick = (firstYTick - yMajorTickSpacing) + yMinorTickSpacing;

    if (yMinorTickSpacing != 0) {
      iy = 0;
      qreal nextYMinorTick = firstYMinorTick;
      while (1) {
        nextYMinorTick = firstYMinorTick + (iy++ * yMinorTickSpacing);
        if (projectionRect().bottom() < nextYMinorTick)
          break;
        if (!yTicks.contains(nextYMinorTick) && projectionRect().contains(projectionRect().x(), nextYMinorTick)) {
          yMinTicks << nextYMinorTick;
        }
      }
    }
  }

  _bottomAxisMajorTicks = xTicks;
  _bottomAxisMinorTicks = xMinTicks;
  _leftAxisMajorTicks = yTicks;
  _leftAxisMinorTicks = yMinTicks;

  _leftAxisLabels.clear();
  _leftBaseLabel.clear();

  int longest = 0, shortest = 1000;
  qreal yBase;
  QMapIterator<qreal, QString> iLabel(leftLabels);
  while (iLabel.hasNext()) {
    iLabel.next();
    if (iLabel.value().length() < shortest) {
      shortest = iLabel.value().length();
      yBase = iLabel.key();
    }
    if (iLabel.value().length() > longest) {
      longest = iLabel.value().length();
    }
  }

  if (_yAxisBaseOffset || _yAxisInterpret || (longest > _yAxisSignificantDigits) ) {
    if (_yAxisInterpret) {
      _leftBaseLabel = interpretLabel(_yAxisInterpretation, _yAxisDisplay, yBase, (_leftAxisMajorTicks).last());
    } else {
      _leftBaseLabel = QString::number(yBase);
    }
    QMapIterator<qreal, QString> i(leftLabels);
    while (i.hasNext()) {
      i.next();
      qreal offset;
      if (_yAxisInterpret) {
        offset = interpretOffset(_yAxisInterpretation, _yAxisDisplay, yBase, i.key());
      } else {
        offset = i.key() - yBase;
      }
      QString label;
      if (offset < 0) {
        label += "-";
        offset = offset * -1;
      } else if (offset > 0) {
        label += "+";
      }
      label += "[";
      label += QString::number(offset, 'g', _yAxisSignificantDigits);
      label += "]";
      _leftAxisLabels.insert(i.key(), label);
    }
  } else {
    _leftAxisLabels = leftLabels;
  }

  _bottomAxisLabels.clear();
  _bottomAxisLabels.clear();

  longest = 0;
  shortest = 1000;
  qreal xBase;
  QMapIterator<qreal, QString> iBottomLabel(bottomLabels);
  while (iBottomLabel.hasNext()) {
    iBottomLabel.next();
    if (iBottomLabel.value().length() < shortest) {
      shortest = iBottomLabel.value().length();
      xBase = iBottomLabel.key();
    }
    if (iBottomLabel.value().length() > longest) {
      longest = iBottomLabel.value().length();
    }
  }

  if (_xAxisBaseOffset || _xAxisInterpret || (longest > _xAxisSignificantDigits) ) {
    if (_xAxisInterpret) {
      _bottomBaseLabel = interpretLabel(_xAxisInterpretation, _xAxisDisplay, xBase, (_bottomAxisMajorTicks).last());
    } else {
      _bottomBaseLabel = QString::number(xBase);
    }
    QMapIterator<qreal, QString> i(bottomLabels);
    while (i.hasNext()) {
      i.next();
      qreal offset;
      if (_xAxisInterpret) {
        offset = interpretOffset(_xAxisInterpretation, _xAxisDisplay, xBase, i.key());
      } else {
        offset = i.key() - xBase;
      }
      QString label;
      if (offset < 0) {
        label += "-";
        offset = offset * -1;
      } else if (offset > 0) {
        label += "+";
      }
      label += "[";
      label += QString::number(offset);
      label += "]";
      _bottomAxisLabels.insert(i.key(), label);
    }
  } else {
    _bottomAxisLabels = bottomLabels;
  }
}


/*
 * Major ticks are always spaced by D = A*10B where B is an integer,
 * and A is 1, 2 or 5. So: 1, 0.02, 50, 2000 are all possible major tick
 * spacings, but 30 is not.
 *
 * A and B are chosen so that there are as close as possible to M major ticks
 * on the axis (but at least 2). The value of M is set by the requested
 * MajorTickMode.
 */
qreal PlotItem::computedMajorTickSpacing(Qt::Orientation orientation) const {
  qreal R = orientation == Qt::Horizontal ? projectionRect().width() : projectionRect().height();
  qreal M = orientation == Qt::Horizontal ? xAxisMajorTickMode() : yAxisMajorTickMode();
  qreal B = floor(log10(R/M));

  qreal d1 = 1 * pow(10, B);
  qreal d2 = 2 * pow(10, B);
  qreal d5 = 5 * pow(10, B);

  qreal r1 = d1 * M - 1;
  qreal r2 = d2 * M - 1;
  qreal r5 = d5 * M - 1;

#ifdef MAJOR_TICK_DEBUG
  qDebug() << "MajorTickMode:" << M << "Range:" << R
           << "\n\tranges:" << r1 << r2 << r5
           << "\n\tspaces:" << d1 << d2 << d5
           << endl;
#endif

  qreal s1 = qAbs(r1 - R);
  qreal s2 = qAbs(r2 - R);
  qreal s5 = qAbs(r5 - R);

  if (s1 < s2 && s1 < s5)
    return d1;
  else if (s2 < s5)
    return d2;
  else
    return d5;
}


QSizeF PlotItem::calculateBottomTickLabelBound(QPainter *painter) {
  QRectF xLabelRect;
  int flags = Qt::TextSingleLine | Qt::AlignCenter;

  if (isBottomAxisVisible()) {
    QMapIterator<qreal, QString> xLabelIt(_bottomAxisLabels);
    while (xLabelIt.hasNext()) {
      xLabelIt.next();

      QRectF bound = painter->boundingRect(QRectF(), flags, xLabelIt.value());
      QPointF p(mapXToPlot(xLabelIt.key()), plotRect().bottom() + bound.height() / 2.0);
      bound.moveCenter(p);

      if (xLabelRect.isValid()) {
        xLabelRect = xLabelRect.united(bound);
      } else {
        xLabelRect = bound;
      }
    }
  }

  if (!_bottomBaseLabel.isEmpty()) {
    qreal height = painter->boundingRect(QRectF(), flags, _bottomBaseLabel).height();
    if (calculatedBottomLabelMargin() < height) {
      xLabelRect.setHeight(xLabelRect.height() + (height - calculatedBottomLabelMargin()));
    }
  }
  return xLabelRect.size();
}


QSizeF PlotItem::calculateLeftTickLabelBound(QPainter *painter) {
  QRectF yLabelRect;
  int flags = Qt::TextSingleLine | Qt::AlignCenter;
  if (isLeftAxisVisible()) {

    QMapIterator<qreal, QString> yLabelIt(_leftAxisLabels);
    while (yLabelIt.hasNext()) {
      yLabelIt.next();

      QRectF bound = painter->boundingRect(QRectF(), flags, yLabelIt.value());
      bound.setWidth(bound.width() + 6);
      QPointF p(plotRect().left() - bound.width() / 2.0, mapYToPlot(yLabelIt.key()));
      bound.moveCenter(p);

      if (yLabelRect.isValid()) {
        yLabelRect = yLabelRect.united(bound);
      } else {
        yLabelRect = bound;
      }
    }
  }
  if (!_leftBaseLabel.isEmpty()) {
    qreal height = painter->boundingRect(QRectF(), flags, _leftBaseLabel).height();
    if (calculatedLeftLabelMargin() < height) {
      yLabelRect.setWidth(yLabelRect.width() + (height - calculatedLeftLabelMargin()));
    }
  }
  return yLabelRect.size();
}


void PlotItem::setProjectionRect(const QRectF &rect) {
  if (!(_projectionRect == rect || rect.isEmpty() || !rect.isValid())) {
    qDebug() << "=== setProjectionRect() ======================>\n"
              << "before:" << _projectionRect << "\n"
              << "after:" << rect << endl;

    _projectionRect = rect;
    emit marginsChanged();
  }
  generateAxes();
  update(); //slow, but need to update everything...
}


QRectF PlotItem::computedProjectionRect() {
  QRectF rect;
  foreach (PlotRenderItem *renderer, renderItems()) {
    if (!renderer->computedProjectionRect().isEmpty()) {
      if (rect.isValid()) {
        rect = rect.united(renderer->computedProjectionRect());
      } else {
        rect = renderer->computedProjectionRect();
      }
    }
  }

  if (!rect.isValid())
    rect = QRectF(QPointF(-0.1, -0.1), QPointF(0.1, 0.1)); //default

  return rect;
}


void PlotItem::computedRelationalMax(qreal &minimum, qreal &maximum) {
  QRectF rect;
  foreach (PlotRenderItem *renderer, renderItems()) {
    foreach (RelationPtr relation, renderer->relationList()) {
        if (relation->ignoreAutoScale())
          continue;

        qreal min, max;
        relation->yRange(projectionRect().left(),
                        projectionRect().right(),
                        &min, &max);

        //If the axis is in log mode, the lower extent will be the
        //minimum value larger than zero.
        if (yAxisLog())
          minimum = minimum <= 0.0 ? min : qMin(min, minimum);
        else
          minimum = qMin(min, minimum);

        maximum = qMax(max, maximum);
    }
  }
}


void PlotItem::computeBorder(Qt::Orientation orientation, qreal &minimum, qreal &maximum) const {
  QRectF rect;
  foreach (PlotRenderItem *renderer, renderItems()) {
    qreal min, max;
    renderer->computeBorder(orientation, &min, &max);
    minimum = qMin(min, minimum);
    maximum = qMax(max, maximum);
  }
}


void PlotItem::resetSelectionRect() {
  foreach (PlotRenderItem *renderer, renderItems()) {
    resetSelectionRect();
  }
}


void PlotItem::zoomFixedExpression(const QRectF &projection) {
  qDebug() << "zoomFixedExpression" << endl;
  ZoomCommand *cmd = new ZoomFixedExpressionCommand(this, projection);
  cmd->redo();
}


void PlotItem::zoomMaximum() {
  qDebug() << "zoomMaximum" << endl;
  ZoomCommand *cmd = new ZoomMaximumCommand(this);
  cmd->redo();
}


void PlotItem::zoomMaxSpikeInsensitive() {
  qDebug() << "zoomMaxSpikeInsensitive" << endl;
  ZoomCommand *cmd = new ZoomMaxSpikeInsensitiveCommand(this);
  cmd->redo();
}


void PlotItem::zoomYMeanCentered() {
  qDebug() << "zoomYMeanCentered" << endl;
  ZoomCommand *cmd = new ZoomYMeanCenteredCommand(this);
  cmd->redo();
}


void PlotItem::zoomXMaximum() {
  qDebug() << "zoomXMaximum" << endl;
  ZoomCommand *cmd = new ZoomXMaximumCommand(this);
  cmd->redo();
}


void PlotItem::zoomXRight() {
  qDebug() << "zoomXRight" << endl;
  ZoomCommand *cmd = new ZoomXRightCommand(this);
  cmd->redo();
}


void PlotItem::zoomXLeft() {
  qDebug() << "zoomXLeft" << endl;
  ZoomCommand *cmd = new ZoomXLeftCommand(this);
  cmd->redo();
}


void PlotItem::zoomXOut() {
  qDebug() << "zoomXOut" << endl;
  resetSelectionRect();
  ZoomCommand *cmd = new ZoomXOutCommand(this);
  cmd->redo();
}


void PlotItem::zoomXIn() {
  qDebug() << "zoomXIn" << endl;
  resetSelectionRect();
  ZoomCommand *cmd = new ZoomXInCommand(this);
  cmd->redo();
}


void PlotItem::zoomNormalizeXtoY() {
  qDebug() << "zoomNormalizeXtoY" << endl;

  if (xAxisLog() || yAxisLog())
    return; //apparently we don't want to do anything here according to kst2dplot...

  ZoomCommand *cmd = new ZoomNormalizeXToYCommand(this);
  cmd->redo();
}


void PlotItem::zoomLogX() {
  qDebug() << "zoomLogX" << endl;
  setXAxisLog(_zoomLogX->isChecked());
  setProjectionRect(computedProjectionRect()); //need to recompute
  update();
}


void PlotItem::zoomYLocalMaximum() {
  qDebug() << "zoomYLocalMaximum" << endl;
  ZoomCommand *cmd = new ZoomYLocalMaximumCommand(this);
  cmd->redo();
}


void PlotItem::zoomYMaximum() {
  qDebug() << "zoomYMaximum" << endl;
  ZoomCommand *cmd = new ZoomYMaximumCommand(this);
  cmd->redo();
}


void PlotItem::zoomYUp() {
  qDebug() << "zoomYUp" << endl;
  ZoomCommand *cmd = new ZoomYUpCommand(this);
  cmd->redo();
}


void PlotItem::zoomYDown() {
  qDebug() << "zoomYDown" << endl;
  ZoomCommand *cmd = new ZoomYDownCommand(this);
  cmd->redo();
}


void PlotItem::zoomYOut() {
  qDebug() << "zoomYOut" << endl;
  resetSelectionRect();
  ZoomCommand *cmd = new ZoomYOutCommand(this);
  cmd->redo();
}


void PlotItem::zoomYIn() {
  qDebug() << "zoomYIn" << endl;
  resetSelectionRect();
  ZoomCommand *cmd = new ZoomYInCommand(this);
  cmd->redo();
}


void PlotItem::zoomNormalizeYtoX() {
  qDebug() << "zoomNormalizeYtoX" << endl;

  if (xAxisLog() || yAxisLog())
    return; //apparently we don't want to do anything here according to kst2dplot...

  ZoomCommand *cmd = new ZoomNormalizeYToXCommand(this);
  cmd->redo();
}


void PlotItem::zoomLogY() {
  qDebug() << "zoomLogY" << endl;
  setYAxisLog(_zoomLogY->isChecked());
  setProjectionRect(computedProjectionRect()); //need to recompute
  update();
}


ZoomState PlotItem::currentZoomState() {
  ZoomState zoomState;
  zoomState.item = this; //the origin of this ZoomState
  zoomState.projectionRect = projectionRect();
  zoomState.xAxisZoomMode = xAxisZoomMode();
  zoomState.yAxisZoomMode = yAxisZoomMode();
  zoomState.isXAxisLog = xAxisLog();
  zoomState.isYAxisLog = yAxisLog();
  zoomState.xLogBase = 10.0;
  zoomState.yLogBase = 10.0;
  return zoomState;
}


void PlotItem::setCurrentZoomState(ZoomState zoomState) {
  setXAxisZoomMode(ZoomMode(zoomState.xAxisZoomMode));
  setYAxisZoomMode(ZoomMode(zoomState.yAxisZoomMode));
  setXAxisLog(zoomState.isXAxisLog);
  setYAxisLog(zoomState.isYAxisLog);
  setProjectionRect(zoomState.projectionRect);
}


void CreatePlotCommand::createItem() {
  _item = new PlotItem(_view);
  _view->setCursor(Qt::CrossCursor);

  CreateCommand::createItem();
}

void CreatePlotForCurve::createItem() {
  QPointF center = _view->sceneRect().center();
  center -= QPointF(100.0, 100.0);

  _item = new PlotItem(_view);
  _item->setPos(center);
  _item->setViewRect(0.0, 0.0, 200.0, 200.0);
  //_item->setZValue(1);
  _view->scene()->addItem(_item);

  if (_createLayout) {
    _view->createLayout();
  }

  if (_appendToLayout && _view->layoutBoxItem()) {
    _view->layoutBoxItem()->appendItem(_item);
  }

  creationComplete(); //add to undo stack
}


PlotItemFactory::PlotItemFactory()
: GraphicsFactory() {
  registerFactory("plot", this);
}


PlotItemFactory::~PlotItemFactory() {
}


ViewItem* PlotItemFactory::generateGraphics(QXmlStreamReader& xml, ObjectStore *store, View *view, ViewItem *parent) {
  PlotItem *rc = 0;
  double x = 0, y = 0, w = 10, h = 10;
  while (!xml.atEnd()) {
    bool validTag = true;
    if (xml.isStartElement()) {
      if (xml.name().toString() == "plot") {
        Q_ASSERT(!rc);
        rc = new PlotItem(view);
        if (parent) {
          rc->setParent(parent);
        }
        QXmlStreamAttributes attrs = xml.attributes();
        QStringRef av;
        av = attrs.value("tiedzoom");
        if (!av.isNull()) {
          rc->setTiedZoom(QVariant(av.toString()).toBool());
        }
        av = attrs.value("leftlabelvisible");
        if (!av.isNull()) {
          rc->setLeftLabelVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("bottomlabelvisible");
        if (!av.isNull()) {
          rc->setBottomLabelVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("rightlabelvisible");
        if (!av.isNull()) {
          rc->setRightLabelVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("toplabelvisible");
        if (!av.isNull()) {
          rc->setTopLabelVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("bottomaxisvisible");
        if (!av.isNull()) {
          rc->setBottomAxisVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("leftaxisvisible");
        if (!av.isNull()) {
          rc->setLeftAxisVisible(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxislog");
        if (!av.isNull()) {
          rc->setXAxisLog(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxislog");
        if (!av.isNull()) {
          rc->setYAxisLog(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisreversed");
        if (!av.isNull()) {
          rc->setXAxisReversed(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisreversed");
        if (!av.isNull()) {
          rc->setYAxisReversed(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisbaseoffset");
        if (!av.isNull()) {
          rc->setXAxisBaseOffset(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisbaseoffset");
        if (!av.isNull()) {
          rc->setYAxisBaseOffset(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisinterpret");
        if (!av.isNull()) {
          rc->setXAxisInterpret(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisinterpret");
        if (!av.isNull()) {
          rc->setYAxisInterpret(QVariant(av.toString()).toBool());
        }
        av = attrs.value("leftlabeloverride");
        if (!av.isNull()) {
          rc->setLeftLabelOverride(av.toString());
        }
        av = attrs.value("leftlabelfont");
        if (!av.isNull()) {
          QFont font;
          font.fromString(av.toString());
          rc->setLeftLabelFont(font);
        }
        av = attrs.value("bottomlabeloverride");
        if (!av.isNull()) {
          rc->setBottomLabelOverride(av.toString());
        }
        av = attrs.value("bottomlabelfont");
        if (!av.isNull()) {
          QFont font;
          font.fromString(av.toString());
          rc->setBottomLabelFont(font);
        }
        av = attrs.value("toplabeloverride");
        if (!av.isNull()) {
          rc->setTopLabelOverride(av.toString());
        }
        av = attrs.value("toplabelfont");
        if (!av.isNull()) {
          QFont font;
          font.fromString(av.toString());
          rc->setTopLabelFont(font);
        }
        av = attrs.value("rightlabeloverride");
        if (!av.isNull()) {
          rc->setRightLabelOverride(av.toString());
        }
        av = attrs.value("rightlabelfont");
        if (!av.isNull()) {
          QFont font;
          font.fromString(av.toString());
          rc->setRightLabelFont(font);
        }
        av = attrs.value("xaxisinterpretation");
        if (!av.isNull()) {
          rc->setXAxisInterpretation((KstAxisInterpretation)QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxisinterpretation");
        if (!av.isNull()) {
          rc->setYAxisInterpretation((KstAxisInterpretation)QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxisdisplay");
        if (!av.isNull()) {
          rc->setXAxisDisplay((KstAxisDisplay)QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxisdisplay");
        if (!av.isNull()) {
          rc->setYAxisDisplay((KstAxisDisplay)QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxismajortickmode");
        if (!av.isNull()) {
          rc->setXAxisMajorTickMode((PlotItem::MajorTickMode)QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxismajortickmode");
        if (!av.isNull()) {
          rc->setYAxisMajorTickMode((PlotItem::MajorTickMode)QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxisminortickcount");
        if (!av.isNull()) {
          rc->setXAxisMinorTickCount(QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxisminortickcount");
        if (!av.isNull()) {
          rc->setYAxisMinorTickCount(QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxisdrawmajorticks");
        if (!av.isNull()) {
          rc->setDrawXAxisMajorTicks(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisdrawminorticks");
        if (!av.isNull()) {
          rc->setDrawXAxisMinorTicks(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisdrawmajorticks");
        if (!av.isNull()) {
          rc->setDrawYAxisMajorTicks(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisdrawminorticks");
        if (!av.isNull()) {
          rc->setDrawYAxisMinorTicks(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisdrawmajorgridlines");
        if (!av.isNull()) {
          rc->setDrawXAxisMajorGridLines(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisdrawminorgridlines");
        if (!av.isNull()) {
          rc->setDrawXAxisMinorGridLines(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisdrawmajorgridlines");
        if (!av.isNull()) {
          rc->setDrawYAxisMajorGridLines(QVariant(av.toString()).toBool());
        }
        av = attrs.value("yaxisdrawminorgridlines");
        if (!av.isNull()) {
          rc->setDrawYAxisMinorGridLines(QVariant(av.toString()).toBool());
        }
        av = attrs.value("xaxisdrawmajorgridlinecolor");
        if (!av.isNull()) {
          rc->setXAxisMajorGridLineColor(QColor(av.toString()));
        }
        av = attrs.value("xaxisdrawminorgridlinecolor");
        if (!av.isNull()) {
          rc->setXAxisMinorGridLineColor(QColor(av.toString()));
        }
        av = attrs.value("yaxisdrawmajorgridlinecolor");
        if (!av.isNull()) {
          rc->setYAxisMajorGridLineColor(QColor(av.toString()));
        }
        av = attrs.value("yaxisdrawminorgridlinecolor");
        if (!av.isNull()) {
          rc->setYAxisMinorGridLineColor(QColor(av.toString()));
        }
        av = attrs.value("xaxisdrawmajorgridlinestyle");
        if (!av.isNull()) {
          rc->setXAxisMajorGridLineStyle((Qt::PenStyle)QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxisdrawminorgridlinestyle");
        if (!av.isNull()) {
          rc->setXAxisMinorGridLineStyle((Qt::PenStyle)QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxisdrawmajorgridlinestyle");
        if (!av.isNull()) {
          rc->setYAxisMajorGridLineStyle((Qt::PenStyle)QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxisdrawminorgridlinestyle");
        if (!av.isNull()) {
          rc->setYAxisMinorGridLineStyle((Qt::PenStyle)QVariant(av.toString()).toInt());
        }
        av = attrs.value("xaxissignificantdigits");
        if (!av.isNull()) {
          rc->setXAxisSignificantDigits(QVariant(av.toString()).toInt());
        }
        av = attrs.value("yaxissignificantdigits");
        if (!av.isNull()) {
          rc->setYAxisSignificantDigits(QVariant(av.toString()).toInt());
        }
        av = attrs.value("xzoommode");
        if (!av.isNull()) {
          rc->setXAxisZoomMode((PlotItem::ZoomMode)av.toString().toInt());
        }
        av = attrs.value("yzoommode");
        if (!av.isNull()) {
          rc->setYAxisZoomMode((PlotItem::ZoomMode)av.toString().toInt());
        }

      // TODO add any specialized PlotItem Properties here.
      } else if (xml.name().toString() == "projectionrect") {
        QXmlStreamAttributes attrs = xml.attributes();
        QStringRef av;
        av = attrs.value("width");
        if (!av.isNull()) {
          w = av.toString().toDouble();
        }
        av = attrs.value("height");
        if (!av.isNull()) {
          h = av.toString().toDouble();
        }
        av = attrs.value("x");
        if (!av.isNull()) {
          x = av.toString().toDouble();
        }
        av = attrs.value("y");
        if (!av.isNull()) {
          y = av.toString().toDouble();
        }
        xml.readNext();
      } else if (xml.name().toString() == "cartesianrender") {
        Q_ASSERT(rc);
        PlotRenderItem * renderItem = rc->renderItem(PlotRenderItem::Cartesian);
        if (renderItem) {
          validTag = renderItem->configureFromXml(xml, store);
        }
      } else {
        Q_ASSERT(rc);
        if (!rc->parse(xml, validTag) && validTag) {
          ViewItem *i = GraphicsFactory::parse(xml, store, view, rc);
          if (!i) {
          }
        }
      }
    } else if (xml.isEndElement()) {
      if (xml.name().toString() == "plot") {
        break;
      } else {
        validTag = false;
      }
    }
    if (!validTag) {
      qDebug("invalid Tag\n");
      Debug::self()->log(QObject::tr("Error creating plot object from Kst file."), Debug::Warning);
      delete rc;
      return 0;
    }
    xml.readNext();
  }
  rc->setProjectionRect(QRectF(QPointF(x, y), QSizeF(w, h)));
  return rc;
}


ZoomCommand::ZoomCommand(PlotItem *item, const QString &text)
    : ViewItemCommand(item, text) {

  if (!item->isTiedZoom()) {
    _originalStates << item->currentZoomState();
  } else {
    QList<PlotItem*> plots = PlotItemManager::tiedZoomPlotsForView(item->parentView());
    foreach (PlotItem *plotItem, plots) {
      _originalStates << plotItem->currentZoomState();
    }
  }
}


ZoomCommand::~ZoomCommand() {
}


void ZoomCommand::undo() {
  foreach (ZoomState state, _originalStates) {
    state.item->setCurrentZoomState(state);
  }
}


void ZoomCommand::redo() {
  foreach (ZoomState state, _originalStates) {
    applyZoomTo(state.item);
  }
}


/*
 * X axis zoom to FixedExpression, Y axis zoom to FixedExpression.
 */
void ZoomFixedExpressionCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::FixedExpression);
  item->setYAxisZoomMode(PlotItem::FixedExpression);
  item->setProjectionRect(_fixed);
}


/*
 * X axis zoom to Auto, Y axis zoom to AutoBorder.
 */
void ZoomMaximumCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::Auto);
  item->setYAxisZoomMode(PlotItem::AutoBorder);
  item->setProjectionRect(item->computedProjectionRect());
}


/*
 * X axis zoom to Auto, Y axis zoom to SpikeInsensitive.
 */
void ZoomMaxSpikeInsensitiveCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::Auto);
  item->setYAxisZoomMode(PlotItem::SpikeInsensitive);
  item->setProjectionRect(item->computedProjectionRect());
}


/*
 * X axis zoom to Auto, Y axis zoom to Mean Centered.
 */
void ZoomYMeanCenteredCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::Auto);
  item->setYAxisZoomMode(PlotItem::MeanCentered);
  item->setProjectionRect(item->computedProjectionRect());
}


/*
 * X axis zoom to auto, Y zoom not changed.
 */
void ZoomXMaximumCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::Auto);
  QRectF compute = item->computedProjectionRect();
  item->setProjectionRect(QRectF(compute.x(),
                           item->projectionRect().y(),
                           compute.width(),
                           item->projectionRect().height()));
}

/*
 * X axis zoom changed to fixed and shifted to right:
 *       new_xmin = xmin + (xmax - xmin)*0.10;
 *       new_xmax = xmax + (xmax – xmin)*0.10;
 */
void ZoomXRightCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dx = (item->xMax() - item->xMin())*0.10;
  if (item->xAxisLog()) { 
    compute.setLeft(pow(10, item->xMin() + dx));
    compute.setRight(pow(10, item->xMax() + dx));
  } else {
    compute.setLeft(compute.left() + dx);
    compute.setRight(compute.right() + dx);
  }

  item->setProjectionRect(compute);
}

/*
 * X axis zoom changed to fixed and shifted to :
 *       new_xmin = xmin - (xmax - xmin)*0.10;
 *       new_xmax = xmax - (xmax – xmin)*0.10;
 */
void ZoomXLeftCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dx = (item->xMax() - item->xMin())*0.10;
  if (item->xAxisLog()) { 
    compute.setLeft(pow(10, item->xMin() - dx));
    compute.setRight(pow(10, item->xMax() - dx));
  } else {
    compute.setLeft(compute.left() - dx);
    compute.setRight(compute.right() - dx);
  }

  item->setProjectionRect(compute);
}

/*
 * X axis zoom changed to fixed and increased:
 *       new_xmin = xmin - (xmax - xmin)*0.25;
 *       new_xmax = xmax + (xmax – xmin)*0.25;
 */
void ZoomXOutCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dx = (item->xMax() - item->xMin())*0.25;
  if (item->xAxisLog()) { 
    compute.setLeft(pow(10, item->xMin() - dx));
    compute.setRight(pow(10, item->xMax() + dx));
  } else {
    compute.setLeft(compute.left() - dx);
    compute.setRight(compute.right() + dx);
  }

  item->setProjectionRect(compute);
//   item->update();
}


/*
 * X axis zoom changed to fixed and decreased:
 *       new_xmin = xmin + (xmax - xmin)*0.1666666;
 *       new_xmax = xmax - (xmax – xmin)*0.1666666;
 */
void ZoomXInCommand::applyZoomTo(PlotItem *item) {
  item->setXAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dx = (item->xMax() - item->xMin())*0.1666666;
  if (item->xAxisLog()) { 
    compute.setLeft(pow(10, item->xMin() + dx));
    compute.setRight(pow(10, item->xMax() - dx));
  } else {
    compute.setLeft(compute.left() + dx);
    compute.setRight(compute.right() - dx);
  }

  item->setProjectionRect(compute);
}


/*
 * Normalize X axis to Y axis: Given the current plot aspect ratio, change
 * the X axis range to have the same units per mm as the Y axis range. Particularly
 * useful for images.
 */
void ZoomNormalizeXToYCommand::applyZoomTo(PlotItem *item) {
  QRectF compute = item->projectionRect();
  qreal mean = compute.center().x();
  qreal range = item->plotRect().width() * compute.height() / item->plotRect().height();

  compute.setLeft(mean - (range / 2.0));
  compute.setRight(mean + (range / 2.0));

  item->setXAxisZoomMode(PlotItem::FixedExpression);
  item->setProjectionRect(compute);
}


/*
 * When zoomed in in X, auto zoom Y, only
 * counting points within the current X range. (eg, curve goes from x=0 to 100, but
 * we are zoomed in to x = 30 to 40. Adjust Y zoom to include all points with x
 * values between 30 and 40.
 */
void ZoomYLocalMaximumCommand::applyZoomTo(PlotItem *item) {
  qreal minimum = item->yAxisLog() ? 0.0 : -0.1;
  qreal maximum = 0.1;
  item->computedRelationalMax(minimum, maximum);

  item->computeBorder(Qt::Vertical, minimum, maximum);

  item->setYAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();
  compute.setTop(minimum);
  compute.setBottom(maximum);

  item->setProjectionRect(compute);
}


/*
 * Y axis zoom to auto, X zoom not changed.
 */
void ZoomYMaximumCommand::applyZoomTo(PlotItem *item) {
  item->setYAxisZoomMode(PlotItem::Auto);
  QRectF compute = item->computedProjectionRect();
  item->setProjectionRect(QRectF(item->projectionRect().x(),
                           compute.y(),
                           item->projectionRect().width(),
                           compute.height()));
}


/*
 * Y axis zoom up. If the Y zoom mode is not
 * Mean Centered, change to Fixed (expression).
 *             new_ymin = ymin + (ymax - ymin)*0.1;
 *             new_ymax = ymax + (ymax - ymin)*0.1;
 */
void ZoomYUpCommand::applyZoomTo(PlotItem *item) {
  if (item->yAxisZoomMode() != PlotItem::MeanCentered)
    item->setYAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dy = (item->yMax() - item->yMin())*0.1;
  if (item->yAxisLog()) { 
    compute.setTop(pow(10, item->yMin() + dy));
    compute.setBottom(pow(10, item->yMax() + dy));
  } else {
    compute.setTop(compute.top() + dy);
    compute.setBottom(compute.bottom() + dy);
  }

  item->setProjectionRect(compute);
}


/*
 * Y axis zoom down. If the Y zoom mode is not
 * Mean Centered, change to Fixed (expression).
 *             new_ymin = ymin - (ymax - ymin)*0.10;
 *             new_ymax = ymax - (ymax - ymin)*0.10;
 */
void ZoomYDownCommand::applyZoomTo(PlotItem *item) {
  if (item->yAxisZoomMode() != PlotItem::MeanCentered)
    item->setYAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dy = (item->yMax() - item->yMin())*0.1;
  if (item->yAxisLog()) { 
    compute.setTop(pow(10, item->yMin() - dy));
    compute.setBottom(pow(10, item->yMax() - dy));
  } else {
    compute.setTop(compute.top() - dy);
    compute.setBottom(compute.bottom() - dy);
  }

  item->setProjectionRect(compute);
}


/*
 * Y axis zoom increased. If the Y zoom mode is not
 * Mean Centered, change to Fixed (expression).
 *             new_ymin = ymin - (ymax - ymin)*0.25;
 *             new_ymax = ymax + (ymax - ymin)*0.25;
 */
void ZoomYOutCommand::applyZoomTo(PlotItem *item) {
  if (item->yAxisZoomMode() != PlotItem::MeanCentered)
    item->setYAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dy = (item->yMax() - item->yMin())*0.25;
  if (item->yAxisLog()) { 
    compute.setTop(pow(10, item->yMin() - dy));
    compute.setBottom(pow(10, item->yMax() + dy));
  } else {
    compute.setTop(compute.top() - dy);
    compute.setBottom(compute.bottom() + dy);
  }

  item->setProjectionRect(compute);
//   item->update();
}


/*
 * Y axis zoom decreased. If the Y zoom mode is not
 * Mean Centered, change to Fixed (expression).
 *             new_ymin = ymin + (ymax - ymin)*0.1666666;
 *             new_ymax = ymax - (ymax – ymin)*0.1666666;
 */
void ZoomYInCommand::applyZoomTo(PlotItem *item) {
  if (item->yAxisZoomMode() != PlotItem::MeanCentered)
    item->setYAxisZoomMode(PlotItem::FixedExpression);

  QRectF compute = item->projectionRect();

  qreal dy = (item->yMax() - item->yMin())*0.1666666;
  if (item->yAxisLog()) { 
    compute.setTop(pow(10, item->yMin() + dy));
    compute.setBottom(pow(10, item->yMax() - dy));
  } else {
    compute.setTop(compute.top() + dy);
    compute.setBottom(compute.bottom() - dy);
  }

  item->setProjectionRect(compute);
//   item->update();
}


/*
 * Normalize Y axis to X axis: Given the current plot aspect ratio,
 * change the Y axis range to have the same units per mm as the X axis range.
 * Particularly useful for images.
 */
void ZoomNormalizeYToXCommand::applyZoomTo(PlotItem *item) {
  QRectF compute = item->projectionRect();
  qreal mean = compute.center().y();
  qreal range = item->plotRect().height() * compute.width() / item->plotRect().width();

  compute.setTop(mean - (range / 2.0));
  compute.setBottom(mean + (range / 2.0));

  item->setYAxisZoomMode(PlotItem::FixedExpression);
  item->setProjectionRect(compute);
}


}

// vim: ts=2 sw=2 et
