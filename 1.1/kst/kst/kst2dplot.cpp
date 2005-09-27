/***************************************************************************
                              kst2dplot.cpp
                             ---------------
    begin                : Mar 28, 2004
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
#include <time.h>
#include <float.h>
#include <stdlib.h>
#ifdef __sun
#include <ieeefp.h>
#endif
#include <limits.h>
#include <math.h>

// include files for Qt
#include <qbitmap.h>
#include <qclipboard.h>
#include <qpointarray.h>
#include <qstylesheet.h>

// include files for KDE
#include "ksdebug.h"

// application specific includes
#include "kst2dplot.h"
#include "kstdebug.h"
#include "kstdoc.h"
#include "kstfitdialog_i.h"
#include "kstfilterdialog_i.h"
#include "kstlabeldialog_i.h"
#include "kstlegend.h"
#include "kstlinestyle.h"
#include "kstplotdialog_i.h"
#include "kstsettings.h"
#include "ksttimers.h"
#include "kstviewwindow.h"
#include "kstvcurve.h"
#include "kstvectordialog_i.h"
#include "plotmimesource.h"

#define JD1900                  2415020.5
#define JD1970                  2440587.5
#define JD_RJD                  2400000.0
#define JD_MJD                  2400000.5
#define MAX_NUM_POLYLINES       1000
#define TICK_HYSTERESIS_FACTOR  1.25
#define DIFFERENCE_PRECISION    7
#define LABEL_PRECISION         9
#ifndef DBL_EPSILON
  #define DBL_EPSILON           2.2204460492503131e-016
#endif
#ifndef DBL_DIG
  #define FULL_PRECISION        15
#else
  #define FULL_PRECISION        DBL_DIG
#endif

#if defined(__SVR4) && defined(__sun)
  inline int isinf(double x) { return x == x && !finite(x); }
#endif

inline int d2i(double x) {
  return int(floor(x+0.5));
}

Kst2DPlot::Kst2DPlot(const QString& in_tag,
                 KstScaleModeType yscale_in,
                 KstScaleModeType xscale_in,
                 double xmin_in, double ymin_in,
                 double xmax_in, double ymax_in)
: KstPlotBase("Kst2DPlot"), _buffer(8) {
  // must stay here for plot loading correctness
  _pos_x = 0.0;
  _pos_y = 0.0;
  _width = 0.0;
  _height = 0.0;
  _xOffsetMode = false;
  _yOffsetMode = false;

  // defaults
  _majorGridColor = KstSettings::globalSettings()->majorColor;
  _minorGridColor = KstSettings::globalSettings()->minorColor;
  _majorGridColorDefault = KstSettings::globalSettings()->majorGridColorDefault;
  _minorGridColorDefault = KstSettings::globalSettings()->minorGridColorDefault;

  _xMajorGrid = KstSettings::globalSettings()->xMajor;
  _xMinorGrid = KstSettings::globalSettings()->xMinor;
  _yMajorGrid = KstSettings::globalSettings()->yMajor;
  _yMinorGrid = KstSettings::globalSettings()->yMinor;
  _xMajorTicks = 5;
  _yMajorTicks = 5;
  _reqXMinorTicks = -1;
  _reqYMinorTicks = -1;
  _isXAxisInterpreted = KstSettings::globalSettings()->xAxisInterpret;
  _xAxisInterpretation = KstSettings::globalSettings()->xAxisInterpretation;
  _xAxisDisplay = KstSettings::globalSettings()->xAxisDisplay;
  _xAxisTimezoneLocal = true;
  _xAxisTimezoneHrs = 0.0;
  _isYAxisInterpreted = KstSettings::globalSettings()->yAxisInterpret;
  _yAxisInterpretation = KstSettings::globalSettings()->yAxisInterpretation;
  _yAxisDisplay = KstSettings::globalSettings()->yAxisDisplay;
  _yAxisTimezoneLocal = true;
  _yAxisTimezoneHrs = 0.0;

  commonConstructor(in_tag, yscale_in, xscale_in, xmin_in, ymin_in,
                    xmax_in, ymax_in);
}


Kst2DPlot::Kst2DPlot(const QDomElement& e)
: KstPlotBase(e), _buffer(8) {
  QString in_tag = "unknown";
  KstScaleModeType yscale_in = AUTOBORDER, xscale_in = AUTO;
  double xmin_in = 0, ymin_in = 0, xmax_in = 1, ymax_in = 1;
  QStringList ctaglist;
  KstLabelPtr in_toplabel, in_xlabel, in_ylabel;
  KstLabelPtr in_xticklabel, in_yticklabel;
  KstLabelPtr in_xfullticklabel, in_yfullticklabel;
  KstLabelPtr in_alabel;
  KstLegendPtr in_legend;
  bool x_log = false, y_log = false;
  QStringList in_imageNames;
  QString in_curveToMarkersName;
  bool in_curveToMarkersRisingDetect = false;
  bool in_curveToMarkersFallingDetect = false;
  KstMarker marker;

  marker.isRising = false;
  marker.isFalling = false;

  // for older .kst files that don't have these settings
  _xMajorGrid = false;
  _xMinorGrid = false;
  _yMajorGrid = false;
  _yMinorGrid = false;
  _xMajorTicks = 5;
  _yMajorTicks = 5;
  _reqXMinorTicks = -1;
  _reqYMinorTicks = -1;
  _isXAxisInterpreted = KstSettings::globalSettings()->xAxisInterpret;
  _xAxisInterpretation = KstSettings::globalSettings()->xAxisInterpretation;
  _xAxisDisplay = KstSettings::globalSettings()->xAxisDisplay;
  _xAxisTimezoneLocal = true; 
  _xAxisTimezoneHrs = 0.0;
  _isYAxisInterpreted = KstSettings::globalSettings()->yAxisInterpret;
  _yAxisInterpretation = KstSettings::globalSettings()->yAxisInterpretation;
  _yAxisDisplay = KstSettings::globalSettings()->yAxisDisplay;
  _yAxisTimezoneLocal = true; 
  _yAxisTimezoneHrs = 0.0;
  
  // must stay here for plot loading correctness
  _pos_x = 0.0;
  _pos_y = 0.0;
  _width = 0.0;
  _height = 0.0;
  _xOffsetMode = false;
  _yOffsetMode = false;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); // try to convert the node to an element.
    if (!el.isNull()) { // the node was really an element.
      if (el.tagName() == "width") {
        _width = el.text().toDouble();
      } else if (el.tagName() == "height") {
        _height = el.text().toDouble();
      } else if (el.tagName() == "pos_x") {
        _pos_x = el.text().toDouble();
      } else if (el.tagName() == "pos_y") {
        _pos_y = el.text().toDouble();
      } else if (el.tagName() == "xscalemode") {
        xscale_in = (KstScaleModeType) el.text().toInt();
      } else if (el.tagName() == "yscalemode") {
        yscale_in = (KstScaleModeType) el.text().toInt();
      } else if (el.tagName() == "xmin") {
        xmin_in = el.text().toDouble();
      } else if (el.tagName() == "xmax") {
        xmax_in = el.text().toDouble();
      } else if (el.tagName() == "ymin") {
        ymin_in = el.text().toDouble();
      } else if (el.tagName() == "ymax") {
        ymax_in = el.text().toDouble();
      } else if (el.tagName() == "toplabel") {
        delete in_toplabel;
        in_toplabel = new KstLabel(" ");
        in_toplabel->read(el);
      } else if (el.tagName() == "xlabel") {
        delete in_xlabel;
        in_xlabel = new KstLabel(" ");
        in_xlabel->read(el);
      } else if (el.tagName() == "ylabel") {
        delete in_ylabel;
        in_ylabel = new KstLabel(" ");
        in_ylabel->read(el);
      } else if (el.tagName() == "ticklabel") {
        delete in_xticklabel;
        delete in_yticklabel;
        in_xticklabel = new KstLabel(" ");
        in_xticklabel->read(el);
        in_yticklabel = new KstLabel(" ");
        in_yticklabel->read(el);
      } else if (el.tagName() == "xticklabel") {
        delete in_xticklabel;
        in_xticklabel = new KstLabel(" ");
        in_xticklabel->read(el);
      } else if (el.tagName() == "yticklabel") {
        delete in_yticklabel;
        in_yticklabel = new KstLabel(" ");
        in_yticklabel->read(el);
      } else if (el.tagName() == "xfullticklabel") {
        delete in_xfullticklabel;
        in_xfullticklabel = new KstLabel(" ");
        in_xfullticklabel->read(el);
      } else if (el.tagName() == "yfullticklabel") {
        delete in_yfullticklabel;
        in_yfullticklabel = new KstLabel(" ");
        in_yfullticklabel->read(el);
      } else if (el.tagName() == "legend") {
        in_legend = new KstLegend;
        in_legend->read(el);
      } else if (el.tagName() == "label") {
        in_alabel = new KstLabel(" ");
        in_alabel->read(el);
        _labelList.append(in_alabel);
      } else if (el.tagName() == "curvetag") {
        ctaglist.append(el.text());
      } else if (el.tagName() == "xlog") {
        x_log = true;
      } else if (el.tagName() == "ylog") {
        y_log = true;
      } else if (el.tagName() == "plotforecolor") {
        _foregroundColor.setNamedColor(el.text());
      } else if (el.tagName() == "plotbackcolor") {
        _backgroundColor.setNamedColor(el.text());
      } else if (el.tagName() == "plotmarker") {
        if (el.attribute("value").isEmpty()) {
          marker.isRising = false;
          marker.isFalling = false;
          marker.value = el.text().toDouble();
          _plotMarkers.append(marker);
        } else {
          marker.isRising = !el.attribute("rising").isEmpty();
          marker.isFalling = !el.attribute("falling").isEmpty();
          marker.value = el.attribute("value").toDouble();
          _plotMarkers.append(marker);
        }
      } else if (el.tagName() == "image") {
        in_imageNames.append(el.text());
      } else if (el.tagName() == "curvetomarkersname") {
        in_curveToMarkersName = el.text();
      } else if (el.tagName() == "curvetomarkersrisingdetect") {
        in_curveToMarkersRisingDetect = (el.text() != "0");
      } else if (el.tagName() == "curvetomarkersfallingdetect") {
        in_curveToMarkersFallingDetect = (el.text() != "0");
      } else if (el.tagName() == "xmajorgrid") {
        _xMajorGrid = (el.text() != "0");
      } else if (el.tagName() == "ymajorgrid") {
        _yMajorGrid = (el.text() != "0");
      } else if (el.tagName() == "xminorgrid") {
        _xMinorGrid = (el.text() != "0");
      } else if (el.tagName() == "yminorgrid") {
        _yMinorGrid = (el.text() != "0");
      } else if (el.tagName() == "majorgridcolor") {
        _majorGridColor.setNamedColor(el.text());
      } else if (el.tagName() == "minorgridcolor") {
        _minorGridColor.setNamedColor(el.text());
      } else if (el.tagName() == "yminorticks") {
        _reqYMinorTicks = el.text().toInt();
      } else if (el.tagName() == "xminorticks") {
        _reqXMinorTicks = el.text().toInt();
      } else if (el.tagName() == "majorgridcolordefault") {
        _majorGridColorDefault = (el.text() != "0");
      } else if (el.tagName() == "minorgridcolordefault") {
        _minorGridColorDefault = (el.text() != "0");
      } else if (el.tagName() == "xinterpret") {
        _isXAxisInterpreted = (el.text() != "0");
      } else if (el.tagName() == "xinterpretas") {
        _xAxisInterpretation = (KstAxisInterpretation)el.text().toInt();
      } else if (el.tagName() == "xdisplayas") {
        _xAxisDisplay = (KstAxisDisplay)el.text().toInt();
      } else if (el.tagName() == "xtimezonelocal") {
        _xAxisTimezoneLocal = (el.text() != "0");
      } else if (el.tagName() == "xtimezone") {
        _xAxisTimezoneHrs = el.text().toDouble();
      } else if (el.tagName() == "yinterpret") {
        _isYAxisInterpreted = (el.text() != "0");
      } else if (el.tagName() == "yinterpretas") {
        _yAxisInterpretation = (KstAxisInterpretation)el.text().toInt();
      } else if (el.tagName() == "ydisplayas") {
        _yAxisDisplay = (KstAxisDisplay)el.text().toInt();
      } else if (el.tagName() == "ytimezonelocal") {
        _yAxisTimezoneLocal = (el.text() != "0");
      } else if (el.tagName() == "ytimezone") {
        _yAxisTimezoneHrs = el.text().toDouble();
      } else if ( el.tagName() == "xoffsetmode") {
        _xOffsetMode = (el.text() != "0");
      } else if ( el.tagName() == "yoffsetmode") {
        _yOffsetMode = (el.text() != "0");
      }
    }
    n = n.nextSibling();
  }

  commonConstructor(tagName(), yscale_in, xscale_in, xmin_in, ymin_in,
                    xmax_in, ymax_in, x_log, y_log);

  KstBaseCurveList l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
  for (unsigned i = 0; i < ctaglist.count(); i++) {
    KstBaseCurveList::Iterator it = l.findTag(ctaglist[i]);
    if (it != l.end()) {
      addCurve(*it);
    }
  }

  if (in_toplabel) {
    TopLabel = in_toplabel;
  }

  if (in_xlabel) {
    XLabel = in_xlabel;
    XLabel->setJustification(CxBy);
  }

  if (in_ylabel) {
    YLabel = in_ylabel;
    YLabel->setJustification(CxTy);
  }

  if (in_xticklabel) {
    XTickLabel = in_xticklabel;
    XTickLabel->setJustification(CxTy);
    XTickLabel->setDoScalarReplacement(false);
  }

  if (in_yticklabel) {
    YTickLabel = in_yticklabel;
    YTickLabel->setJustification(RxCy);
    YTickLabel->setDoScalarReplacement(false);
  }

  if (in_xfullticklabel) {
    XFullTickLabel = in_xfullticklabel;
    XFullTickLabel->setJustification(LxBy);
    XFullTickLabel->setRotation(0);
    XFullTickLabel->setDoScalarReplacement(false);
  }

  if (in_yfullticklabel) {
    YFullTickLabel = in_yfullticklabel;
    YFullTickLabel->setJustification(LxTy);
    YFullTickLabel->setRotation(270);
    YFullTickLabel->setDoScalarReplacement(false);
  }

  if (in_legend) {
    Legend = in_legend;
  }

  // add the images
  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);
  for (uint i = 0; i < in_imageNames.count(); i++) {
    KstImageList::Iterator image_iter = images.findTag(*(in_imageNames.at(i)));
    addImage(*image_iter);
  }

  if (!in_curveToMarkersName.isEmpty()) {
    KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
    KstBaseCurveList::iterator curves_iter = curves.findTag(in_curveToMarkersName);
    setCurveToMarkers(*curves_iter, in_curveToMarkersRisingDetect, in_curveToMarkersFallingDetect);
  }
}


// FIXME: broken copy constructor
Kst2DPlot::Kst2DPlot(const Kst2DPlot& plot, const QString& name)
: KstPlotBase(plot), _buffer(8) {
  QString plotName;
  
  _type = "Kst2DPlot";

  if (name.isEmpty()) {
    plotName = KST::suggestPlotName();
  } else {
    plotName = name;
  }
  
  commonConstructor(plotName,
                    plot._xScaleMode,
                    plot._yScaleMode,
                    plot.XMin,
                    plot.YMin,
                    plot.XMax,
                    plot.YMax,
                    plot._xLog,
                    plot._yLog);

  _xOffsetMode = plot._xOffsetMode;
  _yOffsetMode = plot._yOffsetMode;
  _xMajorGrid = plot._xMajorGrid;
  _xMinorGrid = plot._xMinorGrid;
  _yMajorGrid = plot._yMajorGrid;
  _yMinorGrid = plot._yMinorGrid;
  _xMajorTicks = plot._xMajorTicks;
  _yMajorTicks = plot._yMajorTicks;
  _reqXMinorTicks = plot._reqXMinorTicks;
  _reqYMinorTicks = plot._reqYMinorTicks;
  _isXAxisInterpreted = plot._isXAxisInterpreted;
  _xAxisInterpretation = plot._xAxisInterpretation;
  _xAxisDisplay = plot._xAxisDisplay;
  _xAxisTimezoneLocal = plot._xAxisTimezoneLocal;
  _xAxisTimezoneHrs = plot._xAxisTimezoneHrs;
  _isYAxisInterpreted = plot._isYAxisInterpreted;
  _yAxisInterpretation = plot._yAxisInterpretation;
  _yAxisDisplay = plot._yAxisDisplay;
  _yAxisTimezoneLocal = plot._yAxisTimezoneLocal;
  _yAxisTimezoneHrs = plot._yAxisTimezoneHrs;
  setDirty(plot.dirty());
  _zoomPaused = plot._zoomPaused;
  _curveToMarkersRisingDetect = plot._curveToMarkersRisingDetect;
  _curveToMarkersFallingDetect = plot._curveToMarkersFallingDetect;
  _isLogLast = plot._isLogLast;

  _aspect = plot._aspect;
  _aspectOldZoomedObject = plot._aspectOldZoomedObject;

  TopLabel = new KstLabel(plot.TopLabel);

  XLabel = new KstLabel(plot.XLabel);

  YLabel = new KstLabel(plot.YLabel);

  XTickLabel = new KstLabel(plot.XTickLabel);

  YTickLabel = new KstLabel(plot.YTickLabel);

  XFullTickLabel = new KstLabel(plot.XFullTickLabel);

  YFullTickLabel = new KstLabel(plot.YFullTickLabel);

  Legend = new KstLegend(plot.Legend);

  Curves = plot.Curves;
  _images = plot._images;

  for (KstLabelList::ConstIterator i = plot._labelList.begin(); i != plot._labelList.end(); ++i) {
    _labelList.append(new KstLabel(*i));
  }
}

void Kst2DPlot::commonConstructor(const QString &in_tag,
                                KstScaleModeType yscale_in,
                                KstScaleModeType xscale_in,
                                double xmin_in,
                                double ymin_in,
                                double xmax_in,
                                double ymax_in,
                                bool x_log,
                                bool y_log) {
  _zoomPaused = false;
  setDirty(true);
  _oldSize.setWidth(0);
  _oldSize.setHeight(0);
  _hasFocus = false;
  _copy_x = _copy_y = KST::NOPOINT;
  _cursor_x = _cursor_y = KST::NOPOINT;
  _cursorOffset = false;
  _standardActions |= Delete | Edit | Zoom | Pause;
  _draggableLabel = -1;
  _type = "plot";
  _xLog = x_log;
  _yLog = y_log;
  _tickYLast = 0.0;
  _stLast = 0.0;
  _autoTickYLast = 0;
  _isLogLast = false;

  setTagName(in_tag);
  _isTied = false;

  XMin = xmin_in;
  XMax = xmax_in;
  YMin = ymin_in;
  YMax = ymax_in;

  _xScaleMode = xscale_in;
  _yScaleMode = yscale_in;

  // verify that scale limits make sense.  If not, go to auto.
  if (XMax <= XMin) { // not OK: ignore request
    XMin = 0;
    XMax = 1;
    if (_xScaleMode != AUTOUP) {
      _xScaleMode = AUTO;
    }
  }
  if (YMax <= YMin) {
    YMin = 0;
    YMax = 1;
    if (_yScaleMode != AUTOUP) {
      _yScaleMode = AUTO;
    }
  }

  _plotScaleList.setAutoDelete(true);
  pushScale();

  XLabel = new KstLabel;
  XLabel->setJustification(CxBy);
  XLabel->setRotation(0);

  YLabel = new KstLabel;
  YLabel->setJustification(CxTy);
  YLabel->setRotation(270);

  TopLabel = new KstLabel;
  TopLabel->setJustification(LxBy);
  TopLabel->setRotation(0);

  XTickLabel = new KstLabel;
  XTickLabel->setJustification(CxTy);
  XTickLabel->setRotation(0);
  XTickLabel->setDoScalarReplacement(false);

  YTickLabel = new KstLabel;
  YTickLabel->setJustification(RxCy);
  YTickLabel->setRotation(0);
  YTickLabel->setDoScalarReplacement(false);

  XFullTickLabel = new KstLabel;
  XFullTickLabel->setJustification(LxBy);
  XFullTickLabel->setRotation(0);
  XFullTickLabel->setDoScalarReplacement(false);

  YFullTickLabel = new KstLabel;
  YFullTickLabel->setJustification(LxTy);
  YFullTickLabel->setRotation(270);
  YFullTickLabel->setDoScalarReplacement(false);

  Legend = new KstLegend;
  Legend->setJustification(CxBy);

  // let this Kst2DPlot register doc changes.
  connect(this, SIGNAL(modified()), KstApp::inst(), SLOT(registerDocChange()));
}


Kst2DPlot::~Kst2DPlot() {
}

/*** initialize the fonts in a plot: boost size by font_size ***/
void Kst2DPlot::initFonts(const QFont& in_font, int font_size) {
  // Still have to set symbol font info as well
  // We also may want to change different label fonts separately.
  int point_size = in_font.pointSize() + font_size;

  TopLabel->setFontName(in_font.family());
  TopLabel->setSize(point_size - 12);

  XLabel->setFontName(in_font.family());
  XLabel->setSize(point_size - 12);

  YLabel->setFontName(in_font.family());
  YLabel->setSize(point_size - 12);

  XTickLabel->setFontName(in_font.family());
  XTickLabel->setSize(point_size - 12);

  YTickLabel->setFontName(in_font.family());
  YTickLabel->setSize(point_size - 12);

  XFullTickLabel->setFontName(in_font.family());
  XFullTickLabel->setSize(point_size - 12);

  YFullTickLabel->setFontName(in_font.family());
  YFullTickLabel->setSize(point_size - 12);

  Legend->setFontName(in_font.family());
  Legend->setSize(point_size - 12);
}


void Kst2DPlot::setTopLabel(const QString& in_label) {
  setDirty();
  TopLabel->setText(in_label);
}


void Kst2DPlot::setXLabel(const QString& in_label) {
  setDirty();
  XLabel->setText(in_label);
}


void Kst2DPlot::setYLabel(const QString& in_label) {
  setDirty();
  YLabel->setText(in_label);
}


bool Kst2DPlot::checkRange(double &min_in, double &max_in) {
  double diff  = fabs(1000.0 * min_in) * DBL_EPSILON;
  bool rc = true;

  if (isnan(min_in) || isnan(max_in) ||
             isinf(min_in) || isinf(max_in)) {
    rc = false;
  }

  if (rc && max_in <= min_in + diff) {
    max_in = min_in + diff;
  }

  return rc;
}


bool Kst2DPlot::checkLRange(double &min_in, double &max_in, bool bIsLog) {
  double diff  = fabs(1000.0 * min_in) * DBL_EPSILON;
  bool rc = true;

  if (bIsLog) {
    if (isnan(pow(10.0,min_in)) || isnan(pow(10.0,max_in)) ||
        isinf(pow(10.0,min_in)) || isinf(pow(10.0,max_in))) {
      rc = false;
    }
  } else if (isnan(min_in) || isnan(max_in) ||
             isinf(min_in) || isinf(max_in)) {
    rc = false;
  }

  if (rc && max_in <= min_in + diff) {
    max_in = min_in + diff;
  }

  return rc;
}


bool Kst2DPlot::setXScale(double xmin_in, double xmax_in) {
  bool rc = false;

  if (checkRange(xmin_in, xmax_in)) {
    XMax = xmax_in;
    XMin = xmin_in;
    rc = true;
  }

  return rc;
}


bool Kst2DPlot::setYScale(double ymin_in, double ymax_in) {
  bool rc = false;

  if (checkRange(ymin_in, ymax_in)) {
    YMax = ymax_in;
    YMin = ymin_in;
    rc = true;
  }

  return rc;
}


bool Kst2DPlot::setLXScale(double xmin_in, double xmax_in) {
  bool rc = false;

  if (checkLRange(xmin_in, xmax_in, _xLog)) {
    if (_xLog) {
      XMax = pow(10.0, xmax_in);
      XMin = pow(10.0, xmin_in);
    } else {
      XMax = xmax_in;
      XMin = xmin_in;
    }
    rc = true;
  }

  return rc;
}


bool Kst2DPlot::setLYScale(double ymin_in, double ymax_in) {
  bool rc = false;

  if (checkLRange(ymin_in, ymax_in, _yLog)) {
    if (_yLog) {
      YMax = pow(10.0, ymax_in);
      YMin = pow(10.0, ymin_in);
    } else {
      YMax = ymax_in;
      YMin = ymin_in;
    }
    rc = true;
  }

  return rc;
}


void Kst2DPlot::setScale(double xmin_in, double ymin_in,
                  double xmax_in, double ymax_in) {
  setXScale(xmin_in, xmax_in);
  setYScale(ymin_in, ymax_in);
}


bool Kst2DPlot::setLScale(double xmin_in, double ymin_in,
                  double xmax_in, double ymax_in) {
  bool rc = false;

  if (setLXScale(xmin_in, xmax_in)) {
    rc = true;
  }
  if (setLYScale(ymin_in, ymax_in)) {
    rc = true;
  }

  return rc;
}


void Kst2DPlot::getScale(double &xmin, double &ymin,
                       double &xmax, double &ymax) const {
  xmin = XMin;
  xmax = XMax;
  ymin = YMin;
  ymax = YMax;
}


inline double logX(double x) {
  return x > 0.0 ? log10(x) : -350.0;
}


inline double logY(double y) {
  return y > 0.0 ? log10(y) : -350.0;
}


void Kst2DPlot::getLScale(double& x_min, double& y_min,
                       double& x_max, double& y_max) const {
  if (_xLog) {
    x_min = logX(XMin);
    x_max = XMax > 0 ? log10(XMax) : -340;
  } else {
    x_max = XMax;
    x_min = XMin;
  }

  if (_yLog) {
    y_min = logY(YMin);
    y_max = YMax > 0 ? log10(YMax) : -340;
  } else {
    y_max = YMax;
    y_min = YMin;
  }
}


KstScaleModeType Kst2DPlot::xScaleMode() const {
  return _xScaleMode;
}


KstScaleModeType Kst2DPlot::yScaleMode() const {
  return _yScaleMode;
}


void Kst2DPlot::setXScaleMode(KstScaleModeType scalemode_in) {
  _xScaleMode = scalemode_in;
}


void Kst2DPlot::setYScaleMode(KstScaleModeType scalemode_in) {
  _yScaleMode = scalemode_in;
}


void Kst2DPlot::addCurve(KstBaseCurvePtr incurve) {
  Curves.append(incurve);
  setDirty();
  KstApp::inst()->document()->setModified();
}


void Kst2DPlot::addLabel(KstLabelPtr label) {
  _labelList.append(label);
  setDirty();
}


void Kst2DPlot::fitCurve(int id) {
  KMdiChildView* c = KstApp::inst()->activeWindow();
  if (c) {
    KstBaseCurvePtr curve = *(Curves.findTag(_curveRemoveMap[id]));
    if (curve) {
      KstFitDialogI::globalInstance()->show_setCurve(_curveRemoveMap[id], tagName(), c->caption());
      if (_menuView) {
        _menuView->paint();
      }
    }
  }
}


void Kst2DPlot::filterCurve(int id) {
  KMdiChildView* c = KstApp::inst()->activeWindow();
  if (c) {
    KstBaseCurvePtr curve = *(Curves.findTag(_curveRemoveMap[id]));
    if (curve) {
      KstFilterDialogI::globalInstance()->show_setCurve(_curveRemoveMap[id], tagName(), c->caption());
      if (_menuView) {
        _menuView->paint();
      }
    }
  }
}


void Kst2DPlot::removeCurve(KstBaseCurvePtr incurve) {
  Curves.remove(incurve);
  setDirty();
  KstApp::inst()->document()->setModified();
}


void Kst2DPlot::updateScale() {
  double mid, delta;
  double imgX, imgY, imgWidth, imgHeight;
  bool first;
  int start;
  int count;

  switch (_xScaleMode) {
    case AUTOBORDER:  // set scale so all of all curves fits
    case AUTO:  // set scale so all of all curves fits
      XMin = 0.0;
      XMax = 1.0;
      first = true;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_xLog) {
            if (first || XMin > Curves[i]->minPosX()) {
              XMin = Curves[i]->minPosX();
            }
          } else {
            if (first || XMin > Curves[i]->minX()) {
              XMin = Curves[i]->minX();
            }
          }
          if (first || XMax < Curves[i]->maxX()) {
            XMax = Curves[i]->maxX();
          }
          first = false;
        }
      }

      if (XMax <= XMin) {  // if curves had no variation in them
        XMin -= 0.1;
        XMax  = XMin + 0.2;
      }

      // now do the same thing for images
      if (!_images.isEmpty() && Curves.isEmpty()) {
        _images[0]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        XMin = imgX;
        XMax = imgX + imgWidth;
        start = 1;
      } else {
        start = 0;
      }
      for (unsigned i = start; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (XMin > imgX) {
          XMin = imgX;
        }
        if (XMax < imgX + imgWidth) {
          XMax = imgX + imgWidth;
        }
      }
      if (_xLog && XMin <= 0.0) {
        XMin = pow(10.0, -350.0);
      }
      if (_xScaleMode == AUTOBORDER) {
        if (_xLog) {
          XMin = log10(XMin);
          XMax = (XMax > 0.0) ? log10(XMax): 0.0;
          double dx = (XMax-XMin)/40.0;
          XMax += dx;
          XMin -= dx;
          XMax = pow(10.0, XMax);
          XMin = pow(10.0, XMin);
        } else {
          double dx = (XMax-XMin)/40.0;
          XMax += dx; 
          XMin -= dx;
        }
      }
      break;

    case NOSPIKE:  // set scale so all of all curves fits
      XMin = 0.0;
      XMax = 1.0;
      first = true;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_xLog) {
            if (first || XMin > Curves[i]->minPosX()) {
              XMin = Curves[i]->minPosX();
            }
          } else {
            if (first || XMin > Curves[i]->ns_minX()) {
              XMin = Curves[i]->ns_minX();
            }
          }
          if (first || XMax < Curves[i]->ns_maxX()) {
            XMax = Curves[i]->ns_maxX();
          }

          first = false;
        }
      }
      if (XMax <= XMin) {  // if curves had no variation in them
        XMin -= 0.1;
        XMax = XMin + 0.2;
      }

      // now do the same thing for images
      if (!_images.isEmpty() && Curves.isEmpty()) {
        _images[0]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        XMin = imgX;
        XMax = imgX + imgWidth;
        start = 1;
      } else {
        start = 0;
      }
      for (unsigned i = start; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (XMin > imgX) {
          XMin = imgX;
        }
        if (XMax < imgX + imgWidth) {
          XMax = imgX + imgWidth;
        }
      }
      if (_xLog && XMin < 0.0) {
        XMin = pow(10.0, -350.0);
      }
      break;

    case AC: // keep range const, but set mid to mid of all curves
      first = true;
      count = 0;
      mid = 0.0;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          mid += Curves[i]->midX();
          count++;
        }
      }
      if (count > 0) {
        if (XMax <= XMin) { // make sure that range is legal
          XMin = 0.0;
          XMax = 1.0;
        }
        mid /= (double)count;
        delta = XMax - XMin;
        XMin = mid - delta / 2.0;
        XMax = mid + delta / 2.0;
      } else {
        XMin = -0.5;
        XMax =  0.5;
      }
      break;

    case FIXED:  // don't change the range
      if (XMin > XMax) {  // has to be legal, even for fixed scale...
        double tmp = XMax;
        XMax = XMin;
        XMin = tmp;
      } else if (XMin == XMax) {
        if (XMin == 0.0) {
          XMin = -0.5;
          XMax =  0.5;
        } else {
          XMax += fabs(XMax) * 0.01;
          XMin -= fabs(XMin) * 0.01;
        }
      }
      break;

    case AUTOUP:  // only change up
      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_xLog) {
            if (XMin > Curves[i]->minPosX()) {
              XMin = Curves[i]->minPosX();
            }
          } else {
            if (XMin > Curves[i]->minX()) {
              XMin = Curves[i]->minX();
            }
          }
          if (XMax < Curves[i]->maxX()) {
            XMax = Curves[i]->maxX();
          }
        }
      }

      // check images as well
      for (unsigned i = 0; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (XMin > imgX) {
          XMin = imgX;
        }
        if (XMax < imgX + imgWidth) {
          XMax = imgX + imgWidth;
        }
      }
      if (_xLog && XMin < 0.0) {
        XMin = pow(10.0, -350.0);
      }
      if (XMin > XMax) {
        double tmp = XMax;
        XMax = XMin;
        XMin = tmp;
      } else if (XMin == XMax) {
        XMax += fabs(XMax) * 0.01;
        XMin -= fabs(XMin) * 0.01;
      }
      break;

    default:
      kstdWarning() << "Bug in Kst2DPlot::updateScale: bad scale mode" << endl;
      break;
  }


  switch (_yScaleMode) {
    case AUTOBORDER:  // set scale so all of all curves fits
    case AUTO:  // set scale so all of all curves fits
      YMin = 0.0;
      YMax = 1.0;
      first = true;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_yLog) {
            if (first || YMin > Curves[i]->minPosY()) {
              YMin = Curves[i]->minPosY();
            }
          } else {
            if (first || YMin > Curves[i]->minY()) {
              YMin = Curves[i]->minY();
            }
          }
          if (first || YMax < Curves[i]->maxY()) {
            YMax = Curves[i]->maxY();
          }
          first = false;
        }
      }

      if (YMax <= YMin) {  // if curves had no variation in them
        YMin -= 0.1;
        YMax  = YMin + 0.2;
      }

      // now do the same thing for images
      if (!_images.isEmpty() && Curves.isEmpty()) {
        _images[0]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        YMin = imgY;
        YMax = imgY + imgHeight;
        start = 1;
      } else {
        start = 0;
      }
      for (unsigned i = start; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (YMin > imgY) {
          YMin = imgY;
        }
        if (YMax < imgY + imgHeight) {
          YMax = imgY + imgHeight;
        }
      }
      if (_yLog && YMin < 0.0) {
        YMin = pow(10.0, -350.0);
      }
      if (_yScaleMode == AUTOBORDER) {
        if (_yLog) {
          YMin = log10(YMin);
          YMax = (YMax > 0) ? log10(YMax): 0;
          double dy = (YMax-YMin)/40.0;
          YMax += dy;
          YMin -= dy;
          YMax = pow(10.0, YMax);
          YMin = pow(10.0, YMin);
        } else {
          double dy = (YMax-YMin)/40.0;
          YMax += dy;
          YMin -= dy;
        }
      }

      break;

    case NOSPIKE:  // set scale so all of all curves fits
      YMin = 0.0;
      YMax = 1.0;
      first = true;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_yLog) {
            if (first || YMin > Curves[i]->minPosY()) {
              YMin = Curves[i]->minPosY();
            }
          } else {
            if (first || YMin > Curves[i]->ns_minY()) {
              YMin = Curves[i]->ns_minY();
            }
          }
          if (first || YMax < Curves[i]->ns_maxY()) {
            YMax = Curves[i]->ns_maxY();
          }

          first = false;
        }
      }
      if (YMax <= YMin) {  // if curves had no variation in them
        YMin -= 0.1;
        YMax = YMin + 0.2;
      }

      // now do the same thing for images
      if (!_images.isEmpty() && Curves.isEmpty()) {
        _images[0]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        YMin = imgY;
        YMax = imgY + imgHeight;
        start = 1;
      } else {
        start = 0;
      }
      for (unsigned i = start; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (YMin > imgY) {
          YMin = imgY;
        }
        if (YMax < imgY + imgHeight) {
          YMax = imgY + imgHeight;
        }
      }
      if (_yLog && YMin < 0.0) {
        YMin = pow(10.0, -350.0);
      }
      break;

    case AC: // keep range const, but set mid to mean of all curves
      first = true;
      count = 0;
      mid = 0.0;

      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          mid += Curves[i]->midY();
          count++;
        }
      }
      if (count > 0) {
        if (YMax <= YMin) { // make sure that range is legal
          YMin = 0.0;
          YMax = 1.0;
        }
        mid /= (double)count;
        delta = YMax - YMin;
        YMin = mid - delta / 2.0;
        YMax = mid + delta / 2.0;
      } else {
        YMin = -0.5;
        YMax =  0.5;
      }
      break;

    case FIXED:  // don't change the range
      if (YMin > YMax) {  // has to be legal, even for fixed scale...
        double tmp = YMax;
        YMax = YMin;
        YMin = tmp;
      } else if (YMin == YMax) {
        if (YMin == 0.0) {
          YMin = -0.5;
          YMax =  0.5;
        } else {
          YMax += fabs(YMax) * 0.01;
          YMin -= fabs(YMin) * 0.01;
        }
      }
      break;

    case AUTOUP:  // only change up
      for (unsigned i = 0; i < Curves.count(); i++) {
        if (!Curves[i]->ignoreAutoScale()) {
          if (_yLog) {
            if (YMin > Curves[i]->minPosY()) {
              YMin = Curves[i]->minPosY();
            }
          } else {
            if (YMin > Curves[i]->minY()) {
              YMin = Curves[i]->minY();
            }
          }
          if (YMax < Curves[i]->maxY()) {
            YMax = Curves[i]->maxY();
          }
        }
      }

      // check images as well
      for (unsigned i = 0; i < _images.count(); i++) {
        _images[i]->matrixDimensions(imgX, imgY, imgWidth, imgHeight);
        if (YMin > imgY) {
          YMin = imgY;
        }
        if (YMax < imgY + imgHeight) {
          YMax = imgY + imgHeight;
        }
      }
      if (_yLog && YMin < 0.0) {
        YMin = pow(10.0, -350.0);
      }
      if (YMin >= YMax) {  // has to be legal, even for autoup...
        if (YMax == 0.0) {
          YMin = -0.5;
          YMax =  0.5;
        } else {
          YMax += YMax * 0.01;
          YMin -= YMin * 0.01;
        }
      }
      break;

    default:
      kstdWarning() << "Bug in Kst2DPlot::updateScale: bad scale mode" << endl;
      break;
  }
}


double Kst2DPlot::convertTimeValueToJD(KstAxisInterpretation axisInterpretation, double valueIn) {
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


double Kst2DPlot::convertTimeDiffValueToDays(KstAxisInterpretation axisInterpretation, double diffIn) {
  double diff = diffIn;

  switch (axisInterpretation) {
    case AXIS_INTERP_YEAR:
      diff *= 365.25;
      break;
    case AXIS_INTERP_CTIME:
      diff /= 24.0 * 60.0 * 60.0;
      break;
    case AXIS_INTERP_JD:
    case AXIS_INTERP_MJD:
    case AXIS_INTERP_RJD:
      break;
    case AXIS_INTERP_AIT:
      diff /= 24.0 * 60.0 * 60.0;
      break;
    default:
      break;
  }

  return diff;
}


void Kst2DPlot::convertJDToDateString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay,                                         bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, 
                                      uint& length, double dJD) {
  QDate date;
  QRect pr = GetPlotRegion();
  double xmin, ymin, xmax, ymax;
  double xdelta;
  int accuracy = 0;
  
  // check how many decimal places we need based on the scale
  getLScale(xmin,ymin,xmax,ymax);
  if (isXLog()) {
    xdelta = (pow(10.0,xmax)-pow(10.0,xmin))/(double)pr.width();
  } else {
    xdelta = (xmax-xmin)/(double)pr.width();
  }
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
    
  if (axisTimezoneLocal) {
    dJD += timezoneHrs() / 24.0;
  } else {
    dJD += axisTimezoneHrs / 24.0;  
  }
  
  length = 0;

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
  int minute = int((dDayFraction*24.0 - (double)hour)*60.0);
  double second = ((dDayFraction*24.0 - (double)hour)*60.0 - (double)minute)*60.0;
  
  if (accuracy >= 0) {
    second *= pow(10.0,accuracy);
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

  if (accuracy == 0) {
    if ((int)second % 10 != 0) {
      length += 6;
    }
    else if ((int)second != 0) {
      length += 5;
    }
    else if (minute % 10 != 0) {
      length += 4;
    }
    else if (minute != 0) {
      length += 3;
    }
    else if (hour % 10 != 0) {
      length += 2;
    }
    else if (hour != 0) {
      length += 1;
    }
  } else {
    length += 6;
    length += accuracy;
  }
  
  switch (axisDisplay) {
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
      label.sprintf("%d/%02d/%02d %02d:%02d:%02.*f", year, month, day, hour, minute, accuracy, second);
      break;
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
      label.sprintf("%02d/%02d/%d %02d:%02d:%02.*f", day, month, year, hour, minute, accuracy, second);
      break;
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
      date.setYMD(year, month, day);
      label.sprintf("%s %02d:%02d:%02.*f", date.toString(Qt::TextDate).ascii(), 
                                            hour, minute, accuracy, second);
      break;
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
      date.setYMD(year, month, day);
      label.sprintf("%s %02d:%02d:%02.*f", date.toString(Qt::LocalDate).ascii(), 
                                            hour, minute, accuracy, second);
      break;
    case AXIS_DISPLAY_KDE_SHORTDATE:
      label = KGlobal::locale()->formatDateTime(
                                               QDateTime(QDate(year, month, day), 
                                               QTime(hour, minute, (int)second)), true, true);
      // handle the fractional seconds
      if (accuracy > 0) {
        QString strSecond;
        
        strSecond.sprintf(" + %0.*f seconds", accuracy, second - floor(second));
        label += strSecond;
      }
      break;
    case AXIS_DISPLAY_KDE_LONGDATE:
      label = KGlobal::locale()->formatDateTime(
                                               QDateTime(QDate(year, month, day), 
                                               QTime(hour, minute, (int)second)), false, true);
      // handle the fractional seconds
      if (accuracy > 0) {
        QString strSecond;
        
        strSecond.sprintf(" + %0.*f seconds", accuracy, second - floor(second));
        label += strSecond;
      }
      break;
    default:
      break;
  }
}


void Kst2DPlot::convertTimeValueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, uint& length, double z, bool isLog) {
  double value;

  if (isLog) {
    value = pow(10.0, z);
  } else {
    value = z;
  }

  value = convertTimeValueToJD(axisInterpretation, value);

  // print value in appropriate format
  switch (axisDisplay) {
    case AXIS_DISPLAY_YEAR:
      value -= JD1900 + 0.5;
      value /= 365.25;
      value += 1900.0;
      label = QString::number(value, 'g', FULL_PRECISION);
      length = label.length();
      break;
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
    case AXIS_DISPLAY_KDE_SHORTDATE:
    case AXIS_DISPLAY_KDE_LONGDATE:
      convertJDToDateString(axisInterpretation, axisDisplay, axisTimezoneLocal, axisTimezoneHrs, label, length, value);
      break;
    case AXIS_DISPLAY_JD:
      label = QString::number(value, 'g', FULL_PRECISION);
      length = label.length();
      break;
    case AXIS_DISPLAY_MJD:
      value -= JD_MJD;
      label = QString::number(value, 'g', FULL_PRECISION);
      length = label.length();
      break;
    case AXIS_DISPLAY_RJD:
      value -= JD_RJD;
      label = QString::number(value, 'g', FULL_PRECISION);
      length = label.length();
      break;
  }
}


void Kst2DPlot::convertDiffTimevalueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double& zdiff, double zbase, double zvalue, bool isLog, double scale) {
  if (isLog) {
    zdiff = pow(10.0, zvalue) - pow(10.0, zbase);
  } else {
    zdiff = zvalue - zbase;
  }

  zdiff = convertTimeDiffValueToDays(axisInterpretation, zdiff);

  // convert difference to desired format
  switch (axisDisplay) {
    case AXIS_DISPLAY_YEAR:
      zdiff /= 365.25;
      break;
    case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
    case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
    case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
    case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
    case AXIS_DISPLAY_KDE_SHORTDATE:
    case AXIS_DISPLAY_KDE_LONGDATE:
      zdiff *= 24.0 * 60.0 * 60.0;
      break;
    case AXIS_DISPLAY_JD:
    case AXIS_DISPLAY_MJD:
    case AXIS_DISPLAY_RJD:
      break;
  }

  zdiff *= scale;
}


void Kst2DPlot::genAxisTickLabelFullPrecision(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, uint& length, double z, bool isLog, bool isInterpreted) {
  if (isInterpreted) {
    convertTimeValueToString(axisInterpretation, axisDisplay, 
                             axisTimezoneLocal, axisTimezoneHrs, 
                             label, length, z, isLog);
  } else if (isLog) {
    label = QString::number(pow(10, z), 'g', FULL_PRECISION);
    length = label.length();
  } else {
    label = QString::number(z, 'g', FULL_PRECISION);
    length = label.length();
  }
}


void Kst2DPlot::genAxisTickLabelDifference(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, bool isLog, bool isInterpreted, double scale) {
  double zdiff;

  if (isInterpreted) {
    convertDiffTimevalueToString(axisInterpretation, axisDisplay, zdiff, zbase, zvalue, isLog, scale);
  } else {
    if (isLog) {
      zdiff = pow(10.0, zvalue) - pow(10.0, zbase);
    } else {
      zdiff = zvalue - zbase;
    }
  }

  if (zdiff > 0.0) {
    label = i18n("+[numeric value]", "+[%1]").arg(zdiff, 0, 'g', DIFFERENCE_PRECISION);
  } else if (zdiff < 0.0) {
    label = i18n("-[numeric value]", "-[%1]").arg(-zdiff, 0, 'g', DIFFERENCE_PRECISION);
  } else {
    label = i18n("[zero value]", "[%1]").arg(0.0, 0, 'g', 0);
  }
}


void Kst2DPlot::genOffsetLabel(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, double Min, double Max, bool isLog, bool isInterpreted) {
  double zdiff;
  
  if (isInterpreted) {
    QString strPrefix;
    QString strUnits;
    double scale;
    double range;
    int base;
    
    if (isLog) {
      zbase = log10(zbase);
    }
    getPrefixUnitsScale(isInterpreted, axisInterpretation, axisDisplay, isLog, Min, Max, range, scale, base, strPrefix, strUnits);
    convertDiffTimevalueToString(axisInterpretation, axisDisplay, zdiff, zbase, zvalue, isLog, scale);
    label = QString("%1 %2").arg(zdiff, 0, 'g', DIFFERENCE_PRECISION).arg(strUnits);
  } else {
    if (isLog) {
      zdiff = pow(10.0, zvalue) - pow(10.0, zbase);
    } else {
      zdiff = zvalue - zbase;
    }
    label = QString("%1").arg(zdiff, 0, 'g', DIFFERENCE_PRECISION);
  }
}


void Kst2DPlot::genAxisTickLabel(QString& label, double z, bool isLog) {
  if (isLog) {
    if (z > -4 && z < 4) {
      label = QString::number(pow(10, z), 'g', LABEL_PRECISION);
    } else {
      label = i18n("10 to the power of %1", "10^{%1}").arg(z, 0, 'f', 0);
    }
  } else {
    label = QString::number(z, 'g', LABEL_PRECISION);
  }
}


void Kst2DPlot::getPrefixUnitsScale(bool isInterpreted, KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool bLog, double Min, double Max, double& range, double& scale, int& base, QString& strPrefix, QString& strUnits) {
  base = 10;
  range = 1.0;
  scale = 1.0;
  
  if (isInterpreted) {  
    // determine the time range
    if (bLog) {
      range = pow(10.0,Max) - pow(10.0,Min);
    } else {
      range = Max - Min;
    }

    // convert time range to seconds
    switch (axisInterpretation) {
      case AXIS_INTERP_YEAR:
        range *= 365.25 * 24.0 * 60.0 * 60.0;
        break;
      case AXIS_INTERP_CTIME:
        break;
      case AXIS_INTERP_JD:
      case AXIS_INTERP_MJD:
      case AXIS_INTERP_RJD:
        range *= 24.0 * 60.0 * 60.0;
        break;
      case AXIS_INTERP_AIT:
        break;
    }

    switch (axisDisplay) {
      case AXIS_DISPLAY_YEAR:
        strPrefix = i18n("Prefix for Julian Year", "J");
        strUnits  = i18n("years");
        break;
      case AXIS_DISPLAY_YYMMDDHHMMSS_SS:
      case AXIS_DISPLAY_DDMMYYHHMMSS_SS:
      case AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS:
      case AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS:
      case AXIS_DISPLAY_KDE_SHORTDATE:
      case AXIS_DISPLAY_KDE_LONGDATE:
        if( range > 10.0 * 24.0 * 60.0 * 60.0 ) {
          scale /= 24.0 * 60.0 * 60.0;
          strUnits  = i18n("days");
        } else if( range > 10.0 * 24.0 * 60.0 ) {
          scale /= 60.0 * 60.0;
          strUnits  = i18n("hours");
          base = 24;
        } else if( range > 10.0 * 60.0 ) {
          scale /= 60.0;
          strUnits  = i18n("minutes");
          base = 60;
        } else {
          strUnits  = i18n("seconds");
          base = 60;
        }
        break;
      case AXIS_DISPLAY_JD:
        strPrefix = i18n("Prefix for Julian Date", "JD");
        strUnits  = i18n("days");
        break;
      case AXIS_DISPLAY_MJD:
        strPrefix = i18n("Prefix for Modified Julian Date", "MJD");
        strUnits  = i18n("days");
        break;
      case AXIS_DISPLAY_RJD:
        strPrefix = i18n("Prefix for Reduced Julian Date", "RJD");
        strUnits  = i18n("days");
        break;
    }
  }
}


void Kst2DPlot::genAxisTickLabels(QPainter& p, TickParameters &tp,
                                  double Min, double Max, bool bLog, KstLabelPtr Label, 
                                  KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay,
                                  bool axisTimezoneLocal, double axisTimezoneHrs, 
                                  bool isX, bool isInterpreted, bool isOffsetMode) {
  QString strTmp;
  QString strTmpOld;
  QString strUnits;
  QString strPrefix;
  double dWidth;
  double dHeight;
  double range;
  double scale = 1.0;
  bool bDuplicate = false;
  uint uiShortestLength = 1000;
  uint length;
  int iShort = 0;
  int i;
  int base;

  getPrefixUnitsScale(isInterpreted, axisInterpretation, axisDisplay, bLog, Min, Max, range, scale, base, strPrefix, strUnits);

  tp.maxWidth = 0.0;
  tp.maxHeight = 0.0;
  tp.labels.clear();
  tp.delta = false;

  setTicks(tp.tick, tp.org, Max*scale, Min*scale, bLog, isX, base);
  tp.tick /= scale;
  tp.org  /= scale;

  tp.iLo = (int)((Min-tp.org)/tp.tick);
  tp.iHi = (int)((Max-tp.org)/tp.tick)+1;
  iShort = tp.iLo;

  // determine the values, and determine if we need to use delta values
  for (i = tp.iLo; i < tp.iHi; i++) {
    genAxisTickLabel(strTmp, (double)i * tp.tick + tp.org, bLog);
    Label->setText(strTmp);
    dWidth  = Label->rotatedWidth(p);
    dHeight = Label->rotatedHeight(p);
    if (dWidth > tp.maxWidth) {
      tp.maxWidth = dWidth;
    }
    if (dHeight > tp.maxHeight) {
      tp.maxHeight = dHeight;
    }
    if (strTmp == strTmpOld) {
      bDuplicate = true;
    }
    tp.labels.append(strTmp);
    strTmpOld = strTmp;

    genAxisTickLabelFullPrecision(axisInterpretation, axisDisplay, 
                                  axisTimezoneLocal, axisTimezoneHrs,
                                  strTmp, length, (double)i * tp.tick + tp.org, bLog, isInterpreted);
    if (length < uiShortestLength) {
      iShort = i;
      uiShortestLength = length;
    }
  }

  // determine the values when using delta values.
  if (bDuplicate || isInterpreted || isOffsetMode) {
    tp.maxWidth = 0.0;
    tp.maxHeight = 0.0;
    tp.delta = true;
    tp.labels.clear();
    for (i = tp.iLo; i < tp.iHi; i++) {
      if (i == iShort) {
        genAxisTickLabelFullPrecision(axisInterpretation, axisDisplay, 
                                      axisTimezoneLocal, axisTimezoneHrs, 
                                      strTmp, length, 
                                      (double)iShort * tp.tick + tp.org, bLog, isInterpreted);
        if (!strUnits.isEmpty()) {
          strTmp = i18n( "<Prefix e.g. JD><Value> [Units]", "%1%2 [%3]" ).arg(strPrefix).arg(strTmp).arg(strUnits);
        }
        tp.labels.prepend(strTmp);
      }
      if (bLog) {
        genAxisTickLabelDifference(axisInterpretation, axisDisplay, strTmp, 
                                  (double)iShort * tp.tick + tp.org, (double)i * tp.tick + tp.org, 
                                  bLog, isInterpreted, scale);
      } else {
        genAxisTickLabelDifference(axisInterpretation, axisDisplay, strTmp, (double)iShort * tp.tick,    
                                  (double)i * tp.tick, bLog, isInterpreted, scale);
      }
      tp.labels.append(strTmp);
      Label->setText(strTmp);
      dWidth  = Label->rotatedWidth(p);
      dHeight = Label->rotatedHeight(p);
      if (dWidth > tp.maxWidth) {
        tp.maxWidth = dWidth;
      }
      if (dHeight > tp.maxHeight) {
        tp.maxHeight = dHeight;
      }
    }
  }
}


void Kst2DPlot::internalAlignment(KstPaintType type, QPainter& p, QRect& plotRegion) {
  Q_UNUSED(type)

  TickParameters tpx;
  TickParameters tpy;
  QRect rectWindow;
  double x_min, x_max, y_min, y_max;
  double xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px;
  bool offsetX, offsetY;

  if (_oldSize != size()) {
    XLabel->setSize(XLabel->size());
    XLabel->setSize(YLabel->size());
    TopLabel->setSize(TopLabel->size());
    XTickLabel->setSize(XTickLabel->size());
    YTickLabel->setSize(YTickLabel->size());
    XFullTickLabel->setSize(XFullTickLabel->size());
    YFullTickLabel->setSize(YFullTickLabel->size());
  }
  
  rectWindow = p.window();
  p.setWindow(0, 0, size().width(), size().height());
  updateScale();
  getLScale(x_min, y_min, x_max, y_max);
#ifdef BENCHMARK
  QTime t;
  t.start();
#endif
  setBorders(xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px,
             tpx, tpy, p, offsetX, offsetY);
#ifdef BENCHMARK
  kstdDebug() << "setBorders took: " << t.elapsed() << endl;
#endif
  p.setWindow(rectWindow);

  plotRegion.setLeft( d2i(xleft_bdr_px) );
  plotRegion.setRight( d2i(xright_bdr_px) );
  plotRegion.setTop( d2i(ytop_bdr_px) );
  plotRegion.setBottom( d2i(ybot_bdr_px) );
}


static void set2dPlotTickPix(double& xtickpix, double& ytickpix, int x_pix, int y_pix) {
  // set tick size: 4 points on a full letter size plot
  if (x_pix < y_pix) {
    xtickpix = 4.0 * x_pix / 540.0;
    ytickpix = 4.0 * y_pix / 748.0;
  } else {
    ytickpix = 4.0 * y_pix / 540.0;
    xtickpix = 4.0 * x_pix / 748.0;
  }
  xtickpix = (xtickpix + ytickpix) / 2.0; // average of x and y scaling
  if (xtickpix < 2.0) {
    xtickpix = 2.0; // but at least 2 pixels
  }
  ytickpix = xtickpix;
}


void Kst2DPlot::setBorders(double& xleft_bdr_px, double& xright_bdr_px,
                         double& ytop_bdr_px, double& ybot_bdr_px,
                         TickParameters &tpx,  TickParameters &tpy,
                         QPainter& p, bool& bOffsetX, bool& bOffsetY) {
  double x_min, y_min, x_max, y_max;
  int x_px, y_px;

  QRect v = p.window();

  bOffsetX = false;
  bOffsetY = false;
  x_px = v.width();
  y_px = v.height();

  int yTickLabelLineSpacing = YTickLabel->lineSpacing(p);
  int topLabelLineSpacing = TopLabel->lineSpacing(p);

  // calculate the top border
  ytop_bdr_px = 1.1 * topLabelLineSpacing;
  if (ytop_bdr_px < 1) {
    ytop_bdr_px = 0.5 * yTickLabelLineSpacing;
  }
  getLScale(x_min, y_min, x_max, y_max);

  // calculate the bottom border
  genAxisTickLabels(p, tpx, x_min, x_max, _xLog, XTickLabel, _xAxisInterpretation, _xAxisDisplay, 
                    _xAxisTimezoneLocal, _xAxisTimezoneHrs, true, _isXAxisInterpreted, _xOffsetMode);
  ybot_bdr_px  = tpx.maxHeight;
  int xLabelLineSpacing = XLabel->lineSpacing(p);
  ybot_bdr_px += xLabelLineSpacing;

  // calculate the left border
  genAxisTickLabels(p, tpy, y_min, y_max, _yLog, YTickLabel, _yAxisInterpretation, _yAxisDisplay,
                    _yAxisTimezoneLocal, _yAxisTimezoneHrs, false, _isYAxisInterpreted, _yOffsetMode);

  xleft_bdr_px  = tpy.maxWidth;
  int yLabelLineSpacing = YLabel->lineSpacing(p);
  xleft_bdr_px += 5 * yLabelLineSpacing / 4;
  xleft_bdr_px += yTickLabelLineSpacing / 4;

  // calculate the right border
  xright_bdr_px = x_px / 30;

  int tpxLabelCount = tpx.labels.count();
  int tpyLabelCount = tpy.labels.count();

  // calculate offsets if we are using delta values
  if (tpx.delta && tpxLabelCount > 0 && tpy.delta && tpyLabelCount > 0) {
    XFullTickLabel->setText(tpx.labels[0]);
    YFullTickLabel->setText(tpy.labels[0]);
    int xFullTickLabelLineSpacing = XFullTickLabel->lineSpacing(p);
    int yFullTickLabelLineSpacing = YFullTickLabel->lineSpacing(p);
    int xFullTickLabelWidth = XFullTickLabel->width(p);
    int yFullTickLabelWidth = YFullTickLabel->width(p);
    
    if (xFullTickLabelWidth + xleft_bdr_px + yFullTickLabelLineSpacing >= (x_px - XLabel->width(p))/2.0) {
      ybot_bdr_px += xFullTickLabelLineSpacing;
      bOffsetX = true;
    } else if (xFullTickLabelLineSpacing > xLabelLineSpacing) {
      ybot_bdr_px += xFullTickLabelLineSpacing - xLabelLineSpacing;
    }
    if (yFullTickLabelWidth + ybot_bdr_px >= (y_px - YLabel->width(p))/2.0) {
      xleft_bdr_px += yFullTickLabelLineSpacing;
      bOffsetY = true;
    } else if (yFullTickLabelLineSpacing > yLabelLineSpacing) {
      xleft_bdr_px += yFullTickLabelLineSpacing - yLabelLineSpacing;
    }
  } else if (tpx.delta && tpxLabelCount > 0) {
    XFullTickLabel->setText(tpx.labels[0]);
    int xFullTickLabelLineSpacing = XFullTickLabel->lineSpacing(p);
    int xFullTickLabelWidth = XFullTickLabel->width(p);
    if (xFullTickLabelWidth + xleft_bdr_px >= (x_px - XLabel->width(p))/2.0) {
      ybot_bdr_px += xFullTickLabelLineSpacing;
      bOffsetX = true;
    } else if (xFullTickLabelLineSpacing > xLabelLineSpacing) {
      ybot_bdr_px += xFullTickLabelLineSpacing - xLabelLineSpacing;
    }
  } else if (tpy.delta && tpyLabelCount > 0) {
    YFullTickLabel->setText(tpy.labels[0]);
    int yFullTickLabelLineSpacing = YFullTickLabel->lineSpacing(p);
    int yFullTickLabelWidth = YFullTickLabel->width(p);
    if (yFullTickLabelWidth + ybot_bdr_px >= (y_px - YLabel->width(p))/2.0) {
      xleft_bdr_px += yFullTickLabelLineSpacing;
      bOffsetY = true;
    } else if (yFullTickLabelLineSpacing > yLabelLineSpacing) {
      xleft_bdr_px += yFullTickLabelLineSpacing - yLabelLineSpacing;
    }
  }

  // round off all the border values
  xleft_bdr_px  = ceil(xleft_bdr_px);
  xright_bdr_px = ceil(xright_bdr_px);
  ytop_bdr_px   = ceil(ytop_bdr_px);
  ybot_bdr_px   = ceil(ybot_bdr_px);
}


void Kst2DPlot::drawDotAt(QPainter& p, double x, double y) {
  if (_xLog) {
    x = logX(x);
  }
  if (_yLog) {
    y = logY(y);
  }

  int X1 = d2i(_m_X * x + _b_X) + position().x();
  int Y1 = d2i(_m_Y * y + _b_Y) + position().y();

  if (PlotRegion.contains(X1, Y1)) {
    p.setRasterOp(Qt::XorROP);
    p.setPen(QPen(QColor("gray"), 1));
    p.drawEllipse(X1 - 3, Y1 - 3, 6, 6);
    p.setRasterOp(Qt::CopyROP);
  }
}


void Kst2DPlot::drawPlusAt(QPainter& p, double x, double y) {
  if (_xLog) {
    x = logX(x);
  }
  if (_yLog) {
    y = logY(y);
  }

  int X1 = d2i(_m_X * x + _b_X) + position().x();
  int Y1 = d2i(_m_Y * y + _b_Y) + position().y();

  if (PlotRegion.contains(X1, Y1)) {
    p.setRasterOp(Qt::XorROP);
    p.setPen(QPen(QColor("gray"), 1));
    p.drawLine(X1 - 3, Y1, X1 + 4, Y1);
    p.drawLine(X1, Y1 - 3, X1, Y1 + 4);
    p.setRasterOp(Qt::CopyROP);
  }
}


void Kst2DPlot::edit() {
  KstApp *app = KstApp::inst();
  KMdiChildView *c = app->activeWindow();

  if (c) {
    app->showPlotDialog(c->caption(), tagName());
  } else {
    app->showPlotDialog();
  }
}


void Kst2DPlot::move(const QPoint& pos) {
  QPoint offset = pos - _geom.topLeft();

  PlotRegion.moveBy(offset.x(), offset.y());
  WinRegion.moveBy(offset.x(), offset.y());
  PlotAndAxisRegion.moveBy(offset.x(), offset.y());

  KstPlotBase::move(pos);
}


void Kst2DPlot::parentResized() {
  KstPlotBase::parentResized();
  setDirty();
}


void Kst2DPlot::parentMoved(const QPoint& offset) {
  PlotRegion.moveBy(offset.x(), offset.y());
  WinRegion.moveBy(offset.x(), offset.y());
  PlotAndAxisRegion.moveBy(offset.x(), offset.y());

  KstPlotBase::parentMoved(offset);
}


void Kst2DPlot::resize(const QSize& size) {
  KstPlotBase::resize(size);
  setDirty();
}


void Kst2DPlot::updateTieBox(QPainter& p) {
  QRect tr = GetTieBoxRegion();
  p.setPen(foregroundColor());
  if (isTied()) {
    p.fillRect(tr, QColor((foregroundColor().red() + backgroundColor().red()) / 2,
                          (foregroundColor().green() + backgroundColor().green()) / 2,
                          (foregroundColor().blue() + backgroundColor().blue()) / 2));
  } else {
    p.fillRect(tr, backgroundColor());
  }
  p.drawRect(tr);
  if (_hasFocus) {
    tr.setSize(tr.size() / 2);
    tr.moveTopLeft(tr.topLeft() + QPoint(3*tr.width()/4, 3*tr.height()/4));
    p.fillRect(tr, foregroundColor());
  }
}


void Kst2DPlot::updateDirtyFromLabels() {
  bool dirty = this->dirty();
  dirty = dirty || XLabel->dirty() || YLabel->dirty() || TopLabel->dirty();
  XLabel->setDirty(false);
  YLabel->setDirty(false);
  TopLabel->setDirty(false);
  for (KstLabelList::Iterator i = _labelList.begin(); i != _labelList.end(); ++i) {
    dirty = dirty || (*i)->dirty();
    (*i)->setDirty(false);
  }
  setDirty(dirty);
}


void Kst2DPlot::paint(KstPaintType type, QPainter& p) {
  if (type == P_EXPORT || type == P_PRINT) {
    QRect window_hold = p.window();
    QRect viewport_hold = p.viewport();

    p.setViewport(geometry());
    if (type == P_PRINT) {
      draw(p, type, 5.0);
    } else {
      draw(p, type, 1.0);
    }
    p.setWindow(window_hold);
    p.setViewport(viewport_hold);
  } else {
    if (_zoomPaused) {
      return;
    }

    // check for optimizations
    QSize sizeNew = size();
    QRect alignment = KST::alignment.limits(geometry());
    bool doDraw = true;

    updateDirtyFromLabels();

    if (!dirty()) {
      if (type == P_PAINT) {
        if (_oldSize == sizeNew) {
          doDraw = false;
        }
      } else if (type == P_ZOOM) {
        if (alignment == _oldAlignment) {
          doDraw = false;
        }
      }
    }

    if (doDraw) {
      draw();
      setDirty(false);
    }

    _oldAlignment = alignment;
    _oldSize = sizeNew;

    _buffer.paintInto(p, geometry());
    drawCursorPos(p);
    updateTieBox(p);

    // we might need to redraw the datamode marker.
    KstTopLevelViewPtr tlv = KstApp::inst()->activeView();
    if (tlv) {
      KstViewWidget *view = tlv->widget();
      if (view) {
        _copy_x = _copy_y = KST::NOPOINT;
        if (GetPlotRegion().contains(_mouse.tracker)) {
          updateMousePos(_mouse.tracker);
          if (KstApp::inst()->dataMode()) {
            highlightNearestDataPoint(false, view, _mouse.tracker);
          }
        }
      }
    }
  }

  KstPlotBase::paint(type, p);
}


void Kst2DPlot::draw() {
  if (_zoomPaused) {
    return;
  }

  // Precondition: w and h are both > 0
  _buffer.buffer().resize(size());
  assert(!_buffer.buffer().isNull()); // Want to find these crashes
  if (_buffer.buffer().isNull()) {    // Because this is garbage
    return;
  }

  _buffer.buffer().fill(backgroundColor());
  QPainter p(&_buffer.buffer());
  p.setWindow(0, 0, geometry().width(), geometry().height());
  draw(p, P_PAINT);
  setDirty(false);
}


void Kst2DPlot::draw(QPainter &p, KstPaintType type, double resolutionEnhancement) {
  if (_zoomPaused) {
    return;
  }

  QRect old_window = p.window();
  double in_xleft_bdr_px, in_xright_bdr_px;
  double in_ytop_bdr_px, in_ybot_bdr_px;
  double x_min, x_max, y_min, y_max;
  TickParameters tpx,  tpy;
  double xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px;
  double x_orig_px, y_orig_px, xtick_len_px, ytick_len_px;
  double xtick_px, ytick_px;
  double Lx, Hx, Ly, Hy;
  double m_X, m_Y, b_X, b_Y;
  bool offsetX, offsetY;
  int x_px, y_px;
  int penWidth;

#ifdef BENCHMARK
  ++KstDebug::self()->drawCounter()[tagName()];
  kstdDebug() << ">>>>>>>>>>>>>>>>>>>> DRAWING PLOT " << tagName() << endl;
  QTime bench_time;
  int i_bt=0, bt[15];
  QString bt_label[15];

  bench_time.start();
#endif

  p.setWindow(0, 0, (int)(p.viewport().width() * resolutionEnhancement),
                    (int)(p.viewport().height() * resolutionEnhancement));

  if (type != P_PRINT && type != P_EXPORT) {
    updateScale();
  }
  getLScale(x_min, y_min, x_max, y_max);

  p.fillRect(0, 0, p.window().width(), p.window().height(), _backgroundColor);
  x_px = p.window().width();
  y_px = p.window().height();

  penWidth = x_px / 999;
  if (penWidth == 1) {
    penWidth = 0;
  }
  p.setPen(QPen(_foregroundColor, penWidth));

  set2dPlotTickPix(xtick_len_px, ytick_len_px, x_px, y_px);

  setBorders(xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px,
             tpx, tpy, p, offsetX, offsetY);

  // use a common plot region for plots that are aligned and of the same
  //  dimension, in either the horizontal or vertical sense...
  KST::alignment.limits(geometry(), in_xleft_bdr_px, in_xright_bdr_px, in_ytop_bdr_px, in_ybot_bdr_px, resolutionEnhancement);
  if (in_xleft_bdr_px > 0.001) { // x-left border overridden
    xleft_bdr_px = in_xleft_bdr_px;
  }
  if (in_xright_bdr_px > 0.001) { // x-right border overridden
    xright_bdr_px = in_xright_bdr_px;
  }
  if (in_ybot_bdr_px > 0.001) { // y-bottom border overridden
    ybot_bdr_px = in_ybot_bdr_px;
  }
  if (in_ytop_bdr_px > 0.001) { // y-top border overridden
    ytop_bdr_px = in_ytop_bdr_px;
  }

  QRect RelPlotRegion(d2i(xleft_bdr_px),
      d2i(ytop_bdr_px),
      d2i(x_px - xright_bdr_px - xleft_bdr_px + 1.0),
      d2i(y_px - ybot_bdr_px - ytop_bdr_px + 1.0));
  QRect RelPlotAndAxisRegion(d2i(YLabel->lineSpacing(p) + 1.0),
                        d2i(ytop_bdr_px),
                        d2i(x_px - YLabel->lineSpacing(p) - xright_bdr_px),
                        d2i(y_px - XLabel->lineSpacing(p) - ytop_bdr_px));
  QRect RelWinRegion(0, 0, d2i(x_px), d2i(y_px));

  x_orig_px = (tpx.org - x_min) / (x_max - x_min) * double(RelPlotRegion.width()-1) + xleft_bdr_px;
  y_orig_px = (y_max - tpy.org) / (y_max - y_min) * double(RelPlotRegion.height()-1) + ytop_bdr_px;
  xtick_px = (tpx.tick / (x_max - x_min)) * double(RelPlotRegion.width()-1);
  ytick_px = (tpy.tick / (y_max - y_min)) * double(RelPlotRegion.height()-1);

  if (type != P_PRINT && type != P_EXPORT) {
    setPixRect(RelPlotRegion, RelWinRegion, RelPlotAndAxisRegion);
  }

  // only attempt to draw if plot is big enough
  kstdDebug () << tagName() << ": x_px = " << x_px << " xright_bdr_px = " << xright_bdr_px << " xleft_bdr_px = " << xleft_bdr_px << " y_px = " << y_px << " ybot_bdr_px = " << ybot_bdr_px << " ytop_bdr_px = " << ytop_bdr_px << endl;
  if (x_px - xright_bdr_px - xleft_bdr_px >= 10.0 &&
      y_px - ybot_bdr_px - ytop_bdr_px + 1.0 - ytop_bdr_px >= 10.0) {
    Lx = RelPlotRegion.left();
    Hx = RelPlotRegion.right();
    Ly = RelPlotRegion.top();
    Hy = RelPlotRegion.bottom();
    m_X =  double(RelPlotRegion.width()-1)/(x_max - x_min);
    m_Y = -double(RelPlotRegion.height()-1)/(y_max - y_min);
    b_X = Lx - m_X * x_min;
    b_Y = Ly - m_Y * y_max;
    if (type != P_PRINT && type != P_EXPORT) {
      _m_X = m_X;
      _m_Y = m_Y;
      _b_X = b_X;
      _b_Y = b_Y;
    }

#ifdef BENCHMARK
    bt_label[i_bt] = "Initialization";
    bt[i_bt++] = bench_time.elapsed();
#endif
    plotLabels(p, x_px, y_px, xleft_bdr_px, ytop_bdr_px);
#ifdef BENCHMARK
    bt_label[i_bt] = "Plot Labels";
    bt[i_bt++] = bench_time.elapsed();
#endif
    plotImages(p, Lx, Hx, Ly, Hy, m_X, m_Y, b_X, b_Y, x_max, y_max, x_min, y_min);
#ifdef BENCHMARK
    bt_label[i_bt] = "Plot Images";
    bt[i_bt++] = bench_time.elapsed();
#endif
    // must plot grid lines before axes
    plotGridLines(p,
                  tpx.tick, xleft_bdr_px, xright_bdr_px, x_orig_px, xtick_px, xtick_len_px, x_px,
                  tpy.tick, ytop_bdr_px, ybot_bdr_px, y_orig_px, ytick_px, ytick_len_px, y_px);

#ifdef BENCHMARK
    bt_label[i_bt] = "Plot Grid Lines";
    bt[i_bt++] = bench_time.elapsed();
#endif
    // plot the legend now if its in the background
    if (!Legend->getFront()) {
      Legend->draw(&Curves, p,
          d2i(xleft_bdr_px + double(RelPlotRegion.width()) * Legend->x()),
          d2i(ytop_bdr_px + double(RelPlotRegion.height()) * Legend->y()));
#ifdef BENCHMARK
      bt_label[i_bt] = "plot Legend";
      bt[i_bt++] = bench_time.elapsed();
#endif    
    }

    plotCurves(p, Lx, Hx, Ly, Hy, m_X, m_Y, b_X, b_Y, penWidth);
#ifdef BENCHMARK
    bt_label[i_bt] = "Plot Curves";
    bt[i_bt++] = bench_time.elapsed();
#endif
    p.setPen(QPen(_foregroundColor, penWidth));
    plotAxes(p, RelPlotRegion,
        tpx, xleft_bdr_px, xright_bdr_px, x_orig_px, xtick_px, xtick_len_px, x_px,
        tpy, ytop_bdr_px, ybot_bdr_px, y_orig_px, ytick_px, ytick_len_px, y_px,
        offsetX, offsetY);
#ifdef BENCHMARK
    bt_label[i_bt] = "plot Axes";
    bt[i_bt++] = bench_time.elapsed();
#endif
    plotPlotMarkers(p, m_X, b_X, x_max, x_min, y_px, ytop_bdr_px, ybot_bdr_px);
#ifdef BENCHMARK
    bt_label[i_bt] = "plot Markers";
    bt[i_bt++] = bench_time.elapsed();
#endif
    // plot the legend now if its in the foreground
    if (Legend->getFront()) {
      Legend->draw(&Curves, p,
          d2i(xleft_bdr_px + double(RelPlotRegion.width()) * Legend->x()),
          d2i(ytop_bdr_px + double(RelPlotRegion.height()) * Legend->y()));
#ifdef BENCHMARK
      bt_label[i_bt] = "plot Legend";
      bt[i_bt++] = bench_time.elapsed();
#endif    
    }

    // plot the arbitrary labels
    for (KstLabelList::Iterator i = _labelList.begin(); i != _labelList.end(); ++i) {
      (*i)->draw(p, d2i(xleft_bdr_px + double(RelPlotRegion.width()) * (*i)->x()),
                  d2i(ytop_bdr_px + double(RelPlotRegion.height()) * (*i)->y()));
    }
#ifdef BENCHMARK
      bt_label[i_bt] = "plot arbitrary Labels";
      bt[i_bt++] = bench_time.elapsed();
#endif    

    p.flush();

#ifdef BENCHMARK
    bt_label[i_bt] = "Flush Painter";
    bt[i_bt++] = bench_time.elapsed();

    int j;
    kstdDebug() << "Plot Benchmark stats:" << endl;
    kstdDebug() << "   " << bt_label[0] << ": " << bt[0] << "ms" << endl;
    for (j=1; j<i_bt; j++) {
      kstdDebug() << "   " << bt_label[j] << ": " << bt[j]-bt[j-1] << "ms" << endl;
    }
    kstdDebug() << "Plot Total: " << bt[i_bt-1] << "ms" << endl;
#endif
  }

  p.setWindow(old_window);
}


QRect Kst2DPlot::GetPlotRegion() const {
  return PlotRegion;
}


QRect Kst2DPlot::GetWinRegion() const {
  return WinRegion;
}


QRect Kst2DPlot::GetPlotAndAxisRegion() const {
  return PlotAndAxisRegion;
}


QRect Kst2DPlot::GetTieBoxRegion() const {
  int left, top;
  const int dim = 11;

  if (WinRegion.right() - PlotRegion.right() > dim + 3) {
    left = PlotRegion.right() + 2;
  } else {
    left = WinRegion.right() - dim - 1;
  }
  if (PlotRegion.top() - WinRegion.top() > dim + 3) {
    top = PlotRegion.top() - 2 - dim;
  } else {
    top = WinRegion.top()+1;
  }

  return QRect(left, top, dim, dim);
}

void Kst2DPlot::setPixRect(const QRect& RelPlotRegion, const QRect& RelWinRegion, const QRect& RelPlotAndAxisRegion) {
  PlotRegion = RelPlotRegion;
  PlotRegion.moveBy(geometry().x(), geometry().y());
  WinRegion = RelWinRegion;
  WinRegion.moveBy(geometry().x(), geometry().y());
  PlotAndAxisRegion = RelPlotAndAxisRegion;
  PlotAndAxisRegion.moveBy(geometry().x(), geometry().y());
}


KstObject::UpdateType Kst2DPlot::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  // TODO: check labels too

  bool updated = false;
  for (uint i = 0; i < Curves.count(); i++) {
    KstBaseCurvePtr cp = Curves[i];
    cp->writeLock();
    updated = (UPDATE == cp->update(update_counter)) || updated;
    cp->writeUnlock();
  }

  updated = (UPDATE == KstPlotBase::update(update_counter)) || updated;
  return setLastUpdateResult(updated ? UPDATE : NO_CHANGE);
}


void Kst2DPlot::save(QTextStream& ts, const QString& indent) {
  unsigned i;
  QString l2 = indent + "  ";

  KstPlotBase::save(ts, indent);

  ts << indent << "<xscalemode>" << _xScaleMode << "</xscalemode>" << endl;
  ts << indent << "<yscalemode>" << _yScaleMode << "</yscalemode>" << endl;

  ts << indent << "<xmin>" << XMin << "</xmin>" << endl;
  ts << indent << "<xmax>" << XMax << "</xmax>" << endl;
  ts << indent << "<ymin>" << YMin << "</ymin>" << endl;
  ts << indent << "<ymax>" << YMax << "</ymax>" << endl;

  ts << indent << "<toplabel>" << endl;
  TopLabel->save(ts, l2);
  ts << indent << "</toplabel>" << endl;

  ts << indent << "<xlabel>" << endl;
  XLabel->save(ts, l2, false);
  ts << indent << "</xlabel>" << endl;

  ts << indent << "<ylabel>" << endl;
  YLabel->save(ts, l2, false);
  ts << indent << "</ylabel>" << endl;

  ts << indent << "<xticklabel>" << endl;
  XTickLabel->save(ts, l2, false);
  ts << indent << "</xticklabel>" << endl;

  ts << indent << "<yticklabel>" << endl;
  YTickLabel->save(ts, l2, false);
  ts << indent << "</yticklabel>" << endl;

  ts << indent << "<xfullticklabel>" << endl;
  XFullTickLabel->save(ts, l2, false);
  ts << indent << "</xfullticklabel>" << endl;

  ts << indent << "<yfullticklabel>" << endl;
  YFullTickLabel->save(ts, l2, false);
  ts << indent << "</yfullticklabel>" << endl;

  ts << indent << "<legend>" << endl;
  Legend->save(ts, l2);
  ts << indent << "</legend>" << endl;

  if (isXLog()) {
    ts << indent << "<xlog/>" << endl;
  }
  if (isYLog()) {
    ts << indent << "<ylog/>" << endl;
  }

  for (KstLabelList::Iterator i = _labelList.begin(); i != _labelList.end(); ++i) {
    ts << indent << "<label>" << endl;
    (*i)->save(ts, l2);
    ts << indent << "</label>" << endl;
  }

  for (i = 0; i < Curves.count(); i++) {
    ts << indent << "<curvetag>" << QStyleSheet::escape(Curves[i]->tagName()) << "</curvetag>" << endl;
  }

  // save the plot colors, but only if they are different from default
  if (_foregroundColor != KstSettings::globalSettings()->foregroundColor) {
    ts << indent << "<plotforecolor>" << QStyleSheet::escape(_foregroundColor.name()) << "</plotforecolor>" << endl;
  }
  if (_backgroundColor != KstSettings::globalSettings()->backgroundColor) {
    ts << indent << "<plotbackcolor>" << QStyleSheet::escape(_backgroundColor.name()) << "</plotbackcolor>" << endl;
  }

  // save the plot markers
  // makes sure autogenerated markers are here
  updateMarkersFromCurve();
  for (i = 0; i < _plotMarkers.count(); i++) {
    ts << indent << "<plotmarker value=\"" << _plotMarkers[i].value << "\"";
    if (_plotMarkers[i].isRising) {
      ts << " rising=\"1\"";
    } else if (_plotMarkers[i].isFalling) {
      ts << " falling=\"1\"";
    }
    ts << " />" << endl;
  }
  if (hasCurveToMarkers()) {
    ts << indent << "<curvetomarkersname>" << _curveToMarkers->tagName() <<  "</curvetomarkersname>" <<endl;
    ts << indent << "<curvetomarkersrisingdetect>" << _curveToMarkersRisingDetect << "</curvetomarkersrisingdetect>" <<endl;
    ts << indent << "<curvetomarkersfallingdetect>" << _curveToMarkersFallingDetect << "</curvetomarkersfallingdetect>" <<endl;
  }

  // save the image names
  for (i = 0; i < _images.count(); i++) {
    ts << indent << "<image>" << _images[i]->tagName() << "</image>" << endl;
  }

  // save grid line settings
  ts << indent << "<xmajorgrid>" << _xMajorGrid << "</xmajorgrid>" << endl;
  ts << indent << "<ymajorgrid>" << _yMajorGrid << "</ymajorgrid>" << endl;
  ts << indent << "<xminorgrid>" << _xMinorGrid << "</xminorgrid>" << endl;
  ts << indent << "<yminorgrid>" << _yMinorGrid << "</yminorgrid>" << endl;
  ts << indent << "<majorgridcolor>" << _majorGridColor.name() << "</majorgridcolor>" << endl;
  ts << indent << "<minorgridcolor>" << _minorGridColor.name() << "</minorgridcolor>" << endl;
  ts << indent << "<majorgridcolordefault>" << _majorGridColorDefault << "</majorgridcolordefault>" << endl;
  ts << indent << "<minorgridcolordefault>" << _minorGridColorDefault << "</minorgridcolordefault>" << endl;

  // minor ticks
  ts << indent << "<xminorticks>" << _reqXMinorTicks << "</xminorticks>" << endl;
  ts << indent << "<yminorticks>" << _reqYMinorTicks << "</yminorticks>" << endl;

  // axis interpretation settings
  ts << indent << "<xinterpret>" << _isXAxisInterpreted << "</xinterpret>" << endl;
  ts << indent << "<xinterpretas>" << _xAxisInterpretation << "</xinterpretas>" << endl;
  ts << indent << "<xdisplayas>" << _xAxisDisplay << "</xdisplayas>" << endl;
  ts << indent << "<xtimezonelocal>" << _xAxisTimezoneLocal << "</xtimezonelocal>" << endl;
  ts << indent << "<xtimezone>" << _xAxisTimezoneHrs << "</xtimezone>" << endl;
  ts << indent << "<yinterpret>" << _isYAxisInterpreted << "</yinterpret>" << endl;
  ts << indent << "<yinterpretas>" << _yAxisInterpretation << "</yinterpretas>" << endl;
  ts << indent << "<ydisplayas>" << _yAxisDisplay << "</ydisplayas>" << endl;
  ts << indent << "<ytimezonelocal>" << _yAxisTimezoneLocal << "</ytimezonelocal>" << endl;
  ts << indent << "<ytimezone>" << _yAxisTimezoneHrs << "</ytimezone>" << endl;

  ts << indent << "<xoffsetmode>" << _xOffsetMode << "</xoffsetmode>" << endl;
  ts << indent << "<yoffsetmode>" << _yOffsetMode << "</yoffsetmode>" << endl;
}


void Kst2DPlot::saveTag(QTextStream& ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<" << type() << ">" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
    (*i)->saveTag(ts, l2);
  }
  ts << indent << "</" << type() << ">" << endl;
}


void Kst2DPlot::pushScale() {
  KstPlotScale *ps = new KstPlotScale;
  ps->xmin = XMin;
  ps->ymin = YMin;
  ps->xmax = XMax;
  ps->ymax = YMax;
  ps->xscalemode = _xScaleMode;
  ps->yscalemode = _yScaleMode;
  ps->xlog = _xLog;
  ps->ylog = _yLog;

  _plotScaleList.append(ps);
}


bool Kst2DPlot::popScale() {
  if (_plotScaleList.count() > 1) {
    _plotScaleList.removeLast();
    KstPlotScale *ps = _plotScaleList.last();
    XMin = ps->xmin;
    XMax = ps->xmax;
    YMin = ps->ymin;
    YMax = ps->ymax;
    _xScaleMode = ps->xscalemode;
    _yScaleMode = ps->yscalemode;
    _xLog = ps->xlog;
    _yLog = ps->ylog;
    return true;
  }
  return false;
}


/****************************************************************/
/*                                                              */
/*        Place a '\' in front of special characters (ie, '_')  */
/*                                                              */
/****************************************************************/
static void EscapeSpecialChars(QString& label) {
  unsigned int i_char;

  for (i_char = 0; i_char < label.length(); i_char++) {
    if (label.at(i_char) == '_') {
      label.insert(i_char, '\\');
      i_char++;
    }
  }
}


void Kst2DPlot::GenerateDefaultLabels() {
  QStringList xlabels, ylabels, toplabels;
  QString label, xlabel, ylabel, toplabel;
  int n_curves, i_curve, n_images, i_count;

  n_curves = Curves.count();
  n_images = _images.count();

  if (n_curves > 0) {
    for( i_curve = 0; i_curve < n_curves; i_curve++) {
      if (xlabels.findIndex(Curves[i_curve]->xLabel()) == -1) {
        xlabels.append(Curves[i_curve]->xLabel());
      }
      if (ylabels.findIndex(Curves[i_curve]->yLabel()) == -1) {
        ylabels.append(Curves[i_curve]->yLabel());
      }
      if (toplabels.findIndex(Curves[i_curve]->topLabel()) == -1) {
        toplabels.append(Curves[i_curve]->topLabel());
      }
    }

    i_count = xlabels.count();
    for (i_curve = 0; i_curve < i_count; i_curve++) {
      if (i_curve == i_count - 1) {
        xlabel += xlabels[i_curve];
      } else if (i_curve < i_count-2) {
        xlabel += i18n("name in a list", "%1, ").arg(xlabels[i_curve]);
      } else if (i_curve == i_count-2) {
        xlabel += i18n("penultimate name in a list", "%1 and ").arg(xlabels[i_curve]);
      }
    }

    i_count = ylabels.count();
    if (i_count<4) { // only fill if there are 1, 2 or 3 different label....
      // otherwise a legend box should be used.
      for (i_curve = 0; i_curve < i_count; i_curve++) {
	if (i_curve == i_count - 1) {
	  ylabel += ylabels[i_curve];
	} else if (i_curve < i_count-2) {
	  ylabel += i18n("name in a list", "%1, ").arg(ylabels[i_curve]);
	} else if (i_curve == i_count-2) {
	  ylabel += i18n("penultimate name in a list", "%1 and ").arg(ylabels[i_curve]);
	}
      }
    }
  }

  // labels for images
  if (n_images > 0 && n_curves == 0) {
    xlabel = i18n("X");
    ylabel = i18n("Y");
  }

  // create the top label
  i_count = toplabels.count() + n_images;
  for (i_curve = 0; i_curve < i_count; i_curve++) {
    QString label;

    if (i_curve < (int)toplabels.count()) {
      label = toplabels[i_curve];
    } else {
      label = _images[i_curve-toplabels.count()]->tagName();
    }
    
    if (i_curve == i_count - 1) {
      toplabel += label;
    } else if (i_curve < i_count-2) {
      toplabel += i18n("name in a list", "%1, ").arg(label);
    } else if (i_curve == i_count-2) {
      toplabel += i18n("penultimate name in a list", "%1 and ").arg(label);
    }
  }

  EscapeSpecialChars(xlabel);
  EscapeSpecialChars(ylabel);
  EscapeSpecialChars(toplabel);

  setXLabel(xlabel);
  setYLabel(ylabel);
  setTopLabel(toplabel);
}


void Kst2DPlot::setTicks(double& tick, double& org,
                         double max, double min, bool is_log, bool isX,
                         int base) {
  double St = 0.0;
  double Exp;
  int auto_tick;
  int majorDensity = isX ? _xMajorTicks : _yMajorTicks;
  double *ticks;
  int *autominor;
  int nt;

  static double b10_ticks[] = {1.0, 2.0, 5.0, 10.0};
  static int b10_autominor[]= {  5,   4,   5,  5};
  static int n_b10_ticks = sizeof(b10_ticks) / sizeof(double);
  static double b24_ticks[] = {1.0, 2.0, 4.0, 6.0, 12.0, 24.0};
  static int b24_autominor[]= {  5,   4,   4,   6,    6,  6};
  static int n_b24_ticks = sizeof(b24_ticks) / sizeof(double);
  static double b60_ticks[] = 
    {1.0, 2.0, 5.0, 10.0, 15.0, 20.0, 30.0, 60.0};
  static int b60_autominor[] =
    {  5,   4,   5,    5,    3,    4,    6,    6};

  static int n_b60_ticks = sizeof(b60_ticks) / sizeof(double);

  ticks = b10_ticks;
  nt = n_b10_ticks;
  autominor = b10_autominor;

  // check for hysteresis of y-axis tick spacing...
  St = (max - min) / (double)majorDensity;
  if (!isX && is_log == _isLogLast && _stLast != 0.0 &&
        St/_stLast < TICK_HYSTERESIS_FACTOR &&
        St/_stLast > 1.0/TICK_HYSTERESIS_FACTOR) {
    St = _stLast;
    tick = _tickYLast;
    auto_tick = _autoTickYLast;
  } else if (is_log) {
    if (max - min <= (double)majorDensity && max - min > 1.5) {
      auto_tick = 9;
      tick = 1.0;
    } else if (max - min >= (double)majorDensity) {
      auto_tick = 0;
      tick = floor((max - min) / (double)majorDensity);
      if (tick == 1.0) {
        auto_tick = 9;
      }
    } else {
      auto_tick = 0;
      Exp = pow(10.0, floor(log10(St)));

      tick = ticks[0] * Exp;
      for (int i=1; i<nt; i++) {
        if (fabs((ticks[i] * Exp) - St) < fabs(tick - St)) {
          tick = ticks[i] * Exp;
        }
      }
    }
  } else {
    // determine tick interval...
    Exp = 0;
    if (base == 60) {
      if ((b60_ticks[0]*0.7<St) && 
          (b60_ticks[n_b60_ticks-1]>St*0.7)) {
        Exp = 1.0;
        ticks = b60_ticks;
        autominor = b60_autominor;
        nt = n_b60_ticks;
      }
    } else if (base == 24) {
      if ((b24_ticks[0]*0.7<St) && 
          (b24_ticks[n_b24_ticks-1]>St*0.7)) {
        Exp = 1.0;
        ticks = b24_ticks;
        autominor = b24_autominor;
        nt = n_b24_ticks;
      }
    } 

    if (Exp<0.5) {
      Exp = pow(10.0, floor(log10(St)));
    }

    tick = ticks[0] * Exp;
    auto_tick = autominor[0];
    for (int i=1; i<nt; i++) {
      if (fabs((ticks[i] * Exp) - St) < fabs(tick - St)) {
        tick = ticks[i] * Exp;
        auto_tick = autominor[i];
      }
    }
  }

  if (isX) {
    _xMinorTicks = (_reqXMinorTicks < 0) ? auto_tick : _reqXMinorTicks;
  } else {
    _yMinorTicks = (_reqYMinorTicks < 0) ? auto_tick : _reqYMinorTicks;
  }

  // determine location of the origin...
  if (min > 0.0) {
    org = ceil(min / tick) * tick;
  } else if (max < 0.0) {
    org = floor(max / tick) * tick;
  } else {
    org = 0.0;
  }

  if (!isX) {
    _stLast    = St;
    _tickYLast = tick;
    _autoTickYLast = auto_tick;
    _isLogLast = is_log;
  }
}


void Kst2DPlot::setLog(bool x_log, bool y_log) {
  _xLog = x_log;
  _yLog = y_log;
}


bool Kst2DPlot::isXLog() const {
  return _xLog;
}


bool Kst2DPlot::isYLog() const {
  return _yLog;
}


void Kst2DPlot::setXAxisInterpretation( bool isXAxisInterpreted, KstAxisInterpretation xAxisInterpretation, KstAxisDisplay xAxisDisplay, bool xAxisTimezoneLocal, double xAxisTimezoneHrs) {
  _isXAxisInterpreted = isXAxisInterpreted;
  if (_isXAxisInterpreted) {
    _xAxisInterpretation = xAxisInterpretation;
    _xAxisDisplay = xAxisDisplay;
    _xAxisTimezoneLocal = xAxisTimezoneLocal;
    _xAxisTimezoneHrs = xAxisTimezoneHrs;
  }
}


void Kst2DPlot::getXAxisInterpretation( bool& isXAxisInterpreted, KstAxisInterpretation& xAxisInterpretation, KstAxisDisplay& xAxisDisplay, bool& xAxisTimezoneLocal, double& xAxisTimezoneHrs) const {
  isXAxisInterpreted = _isXAxisInterpreted;
  xAxisInterpretation = _xAxisInterpretation;
  xAxisDisplay = _xAxisDisplay;
  xAxisTimezoneLocal = _xAxisTimezoneLocal;
  xAxisTimezoneHrs = _xAxisTimezoneHrs;
}


void Kst2DPlot::setYAxisInterpretation( bool isYAxisInterpreted, KstAxisInterpretation yAxisInterpretation, KstAxisDisplay yAxisDisplay, bool yAxisTimezoneLocal, double yAxisTimezoneHrs) {
  _isYAxisInterpreted = isYAxisInterpreted;
  if (_isYAxisInterpreted) {
    _yAxisInterpretation = yAxisInterpretation;
    _yAxisDisplay = yAxisDisplay;
    _yAxisTimezoneLocal = yAxisTimezoneLocal;
    _yAxisTimezoneHrs = yAxisTimezoneHrs;
  }
}


void Kst2DPlot::getYAxisInterpretation( bool& isYAxisInterpreted, KstAxisInterpretation& yAxisInterpretation, KstAxisDisplay& yAxisDisplay, bool& yAxisTimezoneLocal, double& yAxisTimezoneHrs) const {
  isYAxisInterpreted = _isYAxisInterpreted;
  yAxisInterpretation = _yAxisInterpretation;
  yAxisDisplay = _yAxisDisplay;
  yAxisTimezoneLocal = _yAxisTimezoneLocal;
  yAxisTimezoneHrs = _yAxisTimezoneHrs;
}


bool Kst2DPlot::isTied() const {
  return _isTied;
}


void Kst2DPlot::toggleTied() {
  _isTied = !_isTied;
}


void Kst2DPlot::setTied(bool in_tied) {
  _isTied = in_tied;
}


void Kst2DPlot::editCurve(int id) {
  KstBaseCurvePtr curve = *(Curves.findTag(_curveEditMap[id]));
  if (curve) {
    curve->showDialog();
  }
}


void Kst2DPlot::editObject(int id) {
  KstDataObjectPtr dop = *(KST::dataObjectList.findTag(_objectEditMap[id]));
  if (dop) {
    dop->showDialog();
  }
}


void Kst2DPlot::editVector(int id) {
  KstVectorDialogI::globalInstance()->show_Edit(_objectEditMap[id]);
}


void Kst2DPlot::matchAxis(int id) {
  Kst2DPlotPtr p = (Kst2DPlot*)_plotMap[id];
  if (p) {
    double x0, x1, y0, y1;
    p->getScale(x0, y0, x1, y1);
    setLog(p->isXLog(), p->isYLog());
    setXScaleMode(FIXED);
    setYScaleMode(FIXED);
    setXScale(x0, x1);
    setYScale(y0, y1);
    pushScale();
    setDirty();
    if (_menuView) {
      _menuView->paint();
    }
  }
}


void Kst2DPlot::copyObject() {
  if (_layoutMenuView) {
    KstTopLevelViewPtr tlv = _layoutMenuView->viewObject();
    if (tlv) {
      KstViewWindow *vw = dynamic_cast<KstViewWindow*>(_layoutMenuView->parent());
      if (vw) {
        QStringList plotList;
      
        plotList.append(tagName());  
      
        PlotMimeSource *newplots = new PlotMimeSource(vw->caption(), plotList);
      
        QApplication::clipboard()->setData(newplots, QClipboard::Clipboard);
      }
    }    
  }
}


void Kst2DPlot::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  QString plotName;  

  if (name.isEmpty()) {
    plotName = KST::suggestPlotName();
  } else {
    plotName = name;
  }
  
  parent.appendChild(new Kst2DPlot(*this, plotName), true);  
}


void Kst2DPlot::removeCurve(int id) {
  KstBaseCurvePtr curve = *(Curves.findTag(_curveRemoveMap[id]));
  if (curve) {
    Curves.remove(curve);
    setDirty();
    if (_menuView) {
      _menuView->paint();
    }
    KstApp::inst()->document()->setModified();
  }
}


bool Kst2DPlot::popupMenu (KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  bool hasEntry = false;

  if (globalZoomType() == LABEL_TOOL) { // no menu in text mode. it's confusing
    return false;
  }

  KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(topLevelParent);
  _menuView = tlv ? tlv->widget() : 0L;
  KstViewObject::popupMenu(menu, pos, topLevelParent);

  KPopupMenu *submenu = new KPopupMenu(menu);
  int id = menu->insertItem(i18n("&Match Axis"), submenu);
  Kst2DPlotList pl = globalPlotList();
  int i = 0;
  _plotMap.clear();
  for (Kst2DPlotList::Iterator j = pl.begin(); j != pl.end(); ++j) {
    if ((*j).data() != this) {
      _plotMap[i] = *j; // don't think there is any way around this.
                        // We have to hope that it's safe until the menu is
                        // done.
      submenu->insertItem((*j)->tagName(), i);
      submenu->connectItem(i++, this, SLOT(matchAxis(int)));
      hasEntry = true;
    }
  }
  menu->setItemEnabled(id, hasEntry);
  hasEntry = false;

  submenu = new KPopupMenu(menu);
  menu->insertItem(i18n("Z&oom"), submenu);
  submenu->insertItem(i18n("Zoom &Maximum"), this, SLOT(menuZoomMax()), Key_M);
  submenu->insertItem(i18n("Zoom Max &Spike Insensitive"),
                      this, SLOT(menuZoomSpikeInsensitiveMax()), Key_S);
  submenu->insertItem(i18n("Zoom P&revious"), this, SLOT(menuZoomPrev()), Key_R);
  submenu->insertItem(i18n("Y-Zoom Mean-centered"), this, SLOT(menuYZoomAc()), Key_A);
  submenu->insertSeparator();
  submenu->insertItem(i18n("X-Zoom Maximum"),
                        this, SLOT(menuXZoomMax()), CTRL + Key_M);
  submenu->insertItem(i18n("X-Zoom Out"),
                        this, SLOT(menuXZoomOut()), SHIFT + Key_Right);
  submenu->insertItem(i18n("X-Zoom In"),
                        this, SLOT(menuXZoomIn()), SHIFT + Key_Left);
  submenu->insertItem(i18n("Normalize X Axis to Y Axis"),
                        this, SLOT(menuXNormalize()), Key_N);
  submenu->insertItem(i18n("Toggle Log X Axis"),
                        this, SLOT(menuXLogSlot()), Key_G);
  submenu->insertSeparator();
  submenu->insertItem(i18n("Y-Zoom Maximum"),
                        this, SLOT(menuYZoomMax()), SHIFT + Key_M);
  submenu->insertItem(i18n("Y-Zoom Out"),
                        this, SLOT(menuYZoomOut()), SHIFT + Key_Up);
  submenu->insertItem(i18n("Y-Zoom In"),
                        this, SLOT(menuYZoomIn()), SHIFT + Key_Down);
  submenu->insertItem(i18n("Normalize Y Axis to X Axis"),
                        this, SLOT(menuYNormalize()), SHIFT + Key_N);
  submenu->insertItem(i18n("Toggle Log Y Axis"),
                        this, SLOT(menuYLogSlot()), Key_L);
  submenu = new KPopupMenu(menu);
  menu->insertItem(i18n("&Scroll"), submenu);
  submenu->insertItem(i18n("Left"), this, SLOT(menuMoveLeft()), Key_Left);
  submenu->insertItem(i18n("Right"), this, SLOT(menuMoveRight()), Key_Right);
  submenu->insertItem(i18n("Up"), this, SLOT(menuMoveUp()), Key_Up);
  submenu->insertItem(i18n("Down"), this, SLOT(menuMoveDown()), Key_Down);
  submenu->insertSeparator();

  // disable next or previous marker items if necessary
  #ifndef MARKER_NUM_SEGS
  #define MARKER_NUM_SEGS 50  //sort of get around rounding errors?  Also used in MoveToMarker function
  #endif
  double xmin, xmax;
  double tempVal;
  getLScale(xmin, tempVal, xmax, tempVal);
  double currCenter = ((xmax + xmin)/2.0) + (xmax - xmin)/MARKER_NUM_SEGS;
  if (_xLog) {
    currCenter = pow(10, currCenter);
  }
  id = submenu->insertItem(i18n("Next Marker"), this, SLOT(menuNextMarker()), ALT + Key_Right);
  submenu->setItemEnabled(id, nextMarker(currCenter, tempVal));
  id = submenu->insertItem(i18n("Previous Marker"), this, SLOT(menuPrevMarker()), ALT + Key_Left);
  currCenter = ((xmax + xmin)/2.0) - (xmax - xmin)/MARKER_NUM_SEGS;
  if (_xLog) {
    currCenter = pow(10, currCenter);
  }
  submenu->setItemEnabled(id, prevMarker(currCenter, tempVal) && (!_xLog || tempVal > 0));

  int n_curves = Curves.count();
  int n_images = _images.count();
  menu->insertSeparator();

  _objectEditMap.clear();
  _curveEditMap.clear();
  _curveFitMap.clear();
  _curveRemoveMap.clear();
  _imageEditMap.clear();
  _imageRemoveMap.clear();

  // Edit menu
  submenu = new KPopupMenu(menu);
  hasEntry = false;
  for (i = 0; i < n_curves; i++) {
    const QString& tag = Curves[i]->tagName();
    _curveEditMap[i] = tag;
    submenu->insertItem(i18n("Type: Name", "Curve: %1").arg(tag), i);
    submenu->connectItem(i, this, SLOT(editCurve(int)));
    KstVCurvePtr vc = kst_cast<KstVCurve>(Curves[i]);
    if (vc) {
      KstObjectPtr provider = vc->yVector()->provider();
      if (provider) {
        KstDataObjectPtr dop = kst_cast<KstDataObject>(provider);
        if (dop) {
          _objectEditMap[i + n_curves] = dop->tagName();
          submenu->insertItem(i18n("Type: Name", "%1: %2").arg(dop->typeString()).arg(dop->tagName()), i + n_curves);
          submenu->connectItem(i + n_curves, this, SLOT(editObject(int)));
        }
      } else {
        KstRVectorPtr rv = kst_cast<KstRVector>(vc->yVector());
        if (rv) {
          _objectEditMap[i + n_curves] = rv->tagName();
          submenu->insertItem(i18n("Type: Name", "Vector: %1").arg(rv->tagName()), i + n_curves);
          submenu->connectItem(i + n_curves, this, SLOT(editVector(int)));
        }
      }
    }
    hasEntry = true;
  }

  // Repeat edit menu for images...
  for (i = n_curves; i < n_images + n_curves; i++) {
    const QString& tag = _images[i-n_curves]->tagName();
    _imageEditMap[i] = tag;
    submenu->insertItem(tag, i);
    submenu->connectItem(i, this, SLOT(editImage(int)));
    hasEntry = true;
  }
  id = menu->insertItem(i18n("Edit"), submenu);
  menu->setItemEnabled(id, hasEntry);

  // Fit menu...
  submenu = new KPopupMenu(menu);
  hasEntry = false;
  for (i = 0; i < n_curves; i++) {
    const QString& tag = Curves[i]->tagName();
    _curveFitMap[i] = tag;
    submenu->insertItem(tag, i);
    submenu->connectItem(i, this, SLOT(fitCurve(int)));
    hasEntry = true;
  }
  id = menu->insertItem(i18n("Fit"), submenu);
  menu->setItemEnabled(id, hasEntry);

  // Filter menu...
  submenu = new KPopupMenu(menu);
  hasEntry = false;
  for (i = 0; i < n_curves; i++) {
    const QString& tag = Curves[i]->tagName();
    _curveFitMap[i] = tag;
    submenu->insertItem(tag, i);
    submenu->connectItem(i, this, SLOT(filterCurve(int)));
    hasEntry = true;
  }
  id = menu->insertItem(i18n("Filter"), submenu);
  menu->setItemEnabled(id, hasEntry);

  // Remove menu...
  submenu = new KPopupMenu(menu);
  hasEntry = false;
  for (i = 0; i < n_curves; i++) {
    const QString& tag = Curves[i]->tagName();
    _curveRemoveMap[i] = tag;
    submenu->insertItem(tag, i);
    submenu->connectItem(i, this, SLOT(removeCurve(int)));
    hasEntry = true;
  }

  // Repeat remove menu for images...
  for (i = n_curves; i < n_images + n_curves; i++) {
    const QString& tag = _images[i-n_curves]->tagName();
    _imageRemoveMap[i] = tag;
    submenu->insertItem(tag, i);
    submenu->connectItem(i, this, SLOT(removeImage(int)));
    hasEntry = true;
  }
  id = menu->insertItem(i18n("Remove"), submenu);
  menu->setItemEnabled(id, hasEntry);

  return true;
}

bool Kst2DPlot::layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent) {
  _layoutActions |= Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo | Copy | CopyTo;

  KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(topLevelParent);
  _layoutMenuView = tlv ? tlv->widget() : 0L;
  KstViewObject::layoutPopupMenu(menu, pos, topLevelParent);
  return true;
}


KstViewObjectPtr create_Kst2DPlot() {
  return KstViewObjectPtr(new Kst2DPlot);
}


KstViewObjectFactoryMethod Kst2DPlot::factory() const {
  return &create_Kst2DPlot;
}


bool Kst2DPlot::mouseHandler() const {
  return true;
}


void Kst2DPlot::removeFocus(QPainter& p) {
  _mouse.tracker = _mouse.lastLocation = QPoint(-1, -1);
  p.setClipRegion(_lastClipRegion);
  setHasFocus(false);
  updateTieBox(p);
}


void Kst2DPlot::setHasFocus(bool has) {
  _hasFocus = has;
}


void Kst2DPlot::unsetCursorPos(QWidget *view) {
  if (_cursorOffset) {
    drawCursorPos(view);
  
    _cursor_x = KST::NOPOINT;
    _cursor_y = KST::NOPOINT;
    _cursorOffset = false;
  } else {
    setCursorPos(view);
  }
}


void Kst2DPlot::setCursorPos(QWidget *view) {
  QRect pr = GetPlotRegion();
      
  if (pr.contains(_mouse.tracker)) {
    QString name;
    double xmin, ymin;
    double xmax, ymax;
    bool found = false;
  
    drawCursorPos(view);
      
    if (KstApp::inst()->dataMode()) {
      if (getNearestDataPoint(_mouse.tracker, _cursor_x, _cursor_y, name, xmin, xmax, ymin, ymax)) {
        found = true;
      }
    }
    
    if (!found) {
      double xmin, xmax, ymin, ymax;
    
      getLScale(xmin, ymin, xmax, ymax);
  
      _cursor_x = (double)(_mouse.tracker.x() - pr.left())/(double)pr.width();
      _cursor_x = _cursor_x * (xmax - xmin) + xmin;
      if (isXLog()) {
        _cursor_x = pow(10.0, _cursor_x);
      }
  
      _cursor_y = (double)(_mouse.tracker.y() - pr.top())/(double)pr.height();
      _cursor_y = _cursor_y * (ymin - ymax) + ymax;         
      if (isYLog()) {
        _cursor_y = pow(10.0, _cursor_y);
      }      
    }

    _cursorOffset = true;
        
    drawCursorPos(view);
  }
}


void Kst2DPlot::updateMousePos(const QPoint& pos) {
  QString xlabel, ylabel;
  QString msgXOffset;
  QString msgYOffset;
  QRect pr = GetPlotRegion();
  double xmin, ymin, xmax, ymax, xpos, ypos;
  uint length;
  
  getLScale(xmin, ymin, xmax, ymax);

  xpos = (double)(pos.x() - pr.left())/(double)pr.width();
  xpos = xpos * (xmax - xmin) + xmin;

  ypos = (double)(pos.y() - pr.top())/(double)pr.height();
  ypos = ypos * (ymin - ymax) + ymax;

  _copy_x = xpos;
  _copy_y = ypos;
  
  if (_isXAxisInterpreted) {
    genAxisTickLabelFullPrecision(_xAxisInterpretation, _xAxisDisplay, 
                                  _xAxisTimezoneLocal, _xAxisTimezoneHrs, 
                                  xlabel, length, xpos, isXLog(), true);
    if (_cursorOffset) {
      genOffsetLabel(_xAxisInterpretation, _xAxisDisplay, msgXOffset, 
                  _cursor_x, xpos, xmin, xmax, isXLog(), _isXAxisInterpreted);
    }
  } else {
    double xdelta;
    int iXPrecision;
    
    xdelta = (xmax-xmin)/(double)pr.width();
    if (isXLog()) {
      xpos = pow(10.0, xpos);
      iXPrecision = (int)(ceil(log10(fabs(xpos)))-floor(log10(xpos*pow(10.0,xdelta)-xpos)));
    } else {
      iXPrecision = (int)(ceil(log10(fabs(xpos)))-floor(log10(xdelta)));
    }
    if (iXPrecision < 1) {
      iXPrecision = 1;
    }
    xlabel = QString::number(xpos,'G',iXPrecision);    
    
    if (_cursorOffset) {
      msgXOffset = QString::number(xpos-_cursor_x,'G',iXPrecision);  
    }
  }

  if (_isYAxisInterpreted) {
    genAxisTickLabelFullPrecision(_yAxisInterpretation, _yAxisDisplay, 
                                  _yAxisTimezoneLocal, _yAxisTimezoneHrs,                                   
                                  ylabel, length, ypos, isYLog(), true);
    if (_cursorOffset) {
      genOffsetLabel(_yAxisInterpretation, _yAxisDisplay, msgYOffset, 
                  _cursor_y, ypos, ymin, ymax, isYLog(), _isYAxisInterpreted);
    }
  } else {
    double ydelta;
    int iYPrecision;
    
    ydelta = (ymax-ymin)/(double)pr.height();
    if (isYLog()) {
      ypos = pow(10.0, ypos);
      iYPrecision = (int)(ceil(log10(fabs(ypos)))-floor(log10(ypos*pow(10.0,ydelta)-ypos)));
    } else {
      iYPrecision = (int)(ceil(log10(fabs(ypos)))-floor(log10(ydelta)));
    }
    if (iYPrecision < 1) {
      iYPrecision = 1;
    }    
    ylabel = QString::number(ypos,'G',iYPrecision);
    
    if (_cursorOffset) {
      msgYOffset = QString::number(ypos-_cursor_y,'G',iYPrecision);  
    }
  }
      
  if (_cursorOffset) {     
    KstApp::inst()->slotUpdateDataMsg(i18n("(x, y) [Offset: x,y]", "(%1, %2) [Offset: %3, %4]").arg(xlabel).arg(ylabel).arg(msgXOffset).arg(msgYOffset));
  } else {
    KstApp::inst()->slotUpdateDataMsg(i18n("(x, y)", "(%1, %2)").arg(xlabel).arg(ylabel));
  }
}


bool Kst2DPlot::getNearestDataPoint(const QPoint& pos, double& newxpos, double& newypos, QString& name, double& xmin, double &xmax, double& ymin, double& ymax) {
  bool rc = false;

  if (Curves.count() > 0) {
    QRect pr = GetPlotRegion();
    double xpos, ypos;
    double near_x, near_y;
    double distance;
    double best_distance = 1.0E300;
    double dx_per_pix;
    int i_near_x;
    
    getLScale(xmin, ymin, xmax, ymax);

    // find mouse location in plot units
    xpos = (double)(pos.x() - pr.left())/(double)pr.width() * (xmax - xmin) + xmin;
    if (isXLog()) {
      xpos = pow(10.0, xpos);
    }

    // convert 1 pixel to plot units.
    dx_per_pix = (double)(pos.x()+2 - pr.left()) / (double)pr.width() * (xmax - xmin) + xmin;
    if (isXLog()) {
      dx_per_pix = pow(10.0, dx_per_pix);
    }
    dx_per_pix -= xpos;

    ypos = (double)(pos.y() - pr.top())/(double)pr.height();
    ypos = ypos * (ymin - ymax) + ymax;

    if (isYLog()) {
      ypos = pow(10.0, ypos);
    }

    if (Curves.count() > 0) {
      for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
        i_near_x = (*i)->getIndexNearXY(xpos, dx_per_pix, ypos);
        (*i)->point(i_near_x, near_x, near_y);
        distance = fabs(ypos - near_y);
        if (distance < best_distance || !rc ) {
          newypos = near_y;
          newxpos = near_x;
          best_distance = distance;
          name = (*i)->tagName(); 
          rc = true;
        }
      }
    }
  }
  
  return rc;
}


void Kst2DPlot::drawCursorPos(QPainter& p) {
  if (_cursorOffset) {
    drawPlusAt(p, _cursor_x, _cursor_y); 
  }
}


void Kst2DPlot::drawCursorPos(QWidget *view) {
  QPainter p(view);
    
  drawCursorPos(p);
}


void Kst2DPlot::highlightNearestDataPoint(bool bRepaint, QWidget *view, const QPoint& pos) {
  QString msg;

  if (Curves.count() > 0 || _images.count() > 0) {
    QString name;
    double newxpos, newypos;
    double xmin, ymin;
    double xmax, ymax;
    uint length;
    
    if (getNearestDataPoint(pos, newxpos, newypos, name, xmin, ymin, xmax, ymax)) {
      QString xlabel;
      QString ylabel;
      QString msgXOffset;
      QString msgYOffset;
      
      if (_copy_x != newxpos || _copy_y != newypos) {
        QPainter p(view);

        p.setClipRegion(_lastClipRegion);
        if (bRepaint && _copy_x != KST::NOPOINT && _copy_y != KST::NOPOINT) {
          drawDotAt(p, _copy_x, _copy_y);
        }
        _copy_x = newxpos;
        _copy_y = newypos;
        drawDotAt(p, newxpos, newypos);
      }
      
      if (_isXAxisInterpreted) {
        genAxisTickLabelFullPrecision(_xAxisInterpretation, _xAxisDisplay, 
                                      _xAxisTimezoneLocal, _xAxisTimezoneHrs, 
                                      xlabel, length, newxpos, isXLog(), true);
        if (_cursorOffset) {        
          if (isXLog()) {
            genOffsetLabel(_xAxisInterpretation, _xAxisDisplay, msgXOffset, 
                  _cursor_x, log10(newxpos), xmin, xmax, isXLog(), _isXAxisInterpreted);
          } else {
            genOffsetLabel(_xAxisInterpretation, _xAxisDisplay, msgXOffset, 
                  _cursor_x, newxpos, xmin, xmax, isXLog(), _isXAxisInterpreted);
          }
        }
      } else {
        xlabel = QString::number(newxpos,'G');
        if (_cursorOffset) {
          msgXOffset = QString::number(newxpos-_cursor_x,'G');  
        }
      }
      
      if (_isYAxisInterpreted) {
        genAxisTickLabelFullPrecision(_yAxisInterpretation, _yAxisDisplay, 
                                      _yAxisTimezoneLocal, _yAxisTimezoneHrs, 
                                      ylabel, length, newypos, isYLog(), true);
        if (_cursorOffset) {
          if (isYLog()) {
            genOffsetLabel(_yAxisInterpretation, _yAxisDisplay, msgYOffset, 
                  _cursor_y, log10(newypos), ymin, ymax, isYLog(), _isYAxisInterpreted);
          } else {
            genOffsetLabel(_yAxisInterpretation, _yAxisDisplay, msgYOffset, 
                  _cursor_y, newypos, ymin, ymax, isYLog(), _isYAxisInterpreted);
          }
        }
      } else {
        ylabel = QString::number(newypos,'G');
        if (_cursorOffset) {
          msgYOffset = QString::number(newypos-_cursor_y,'G');
        }
      }
        
      if (_cursorOffset) {     
        msg = i18n("Curve name, (x, y) [Offset: x,y]", "%3 (%1, %2) [Offset: %4, %5]").arg(xlabel).arg(ylabel).arg(name).arg(msgXOffset).arg(msgYOffset);
      } else {
        msg = i18n("Curve name, (x, y)", "%3 (%1, %2)").arg(xlabel).arg(ylabel).arg(name);
      }
    }

    // display the z value of the topmost image underneath cursor, if available...
    if (_images.count() > 0) {
      double zValue;
      double xpos = 0.0;
      double ypos = 0.0;
      bool found = false;
      int i = _images.count() - 1;

      while (i >= 0 && !found) {
        if (_images[i]->getNearestZ(xpos, ypos, zValue)) {
          found = true;
        }
        i--;
      }
      
      if (found) {
        QString xlabel, ylabel;
        uint length;
        
        if (_isXAxisInterpreted) {
          genAxisTickLabelFullPrecision(_xAxisInterpretation, _xAxisDisplay,                                  
                                        _xAxisTimezoneLocal, _xAxisTimezoneHrs, 
                                        xlabel, length, xpos, isXLog(), true);
        } else {
          xlabel = QString::number(xpos,'G');
        }
        
        if (_isYAxisInterpreted) {
          genAxisTickLabelFullPrecision(_yAxisInterpretation, _yAxisDisplay, 
                                        _yAxisTimezoneLocal, _yAxisTimezoneHrs, 
                                        ylabel, length, ypos, isYLog(), true);
        } else {
          ylabel = QString::number(ypos,'G');
        }
                
        if (!msg.isEmpty()) {
          msg = i18n("Label, Image name (x, y, z)", "%5, %4 (%1, %2, %3)" ).arg(xlabel).arg(ylabel).arg(zValue,0,'G').arg(_images[i+1]->tagName()).arg(msg);
        } else {
          msg = i18n("Image name (x, y, z)", "%4 (%1, %2, %3)" ).arg(xlabel).arg(ylabel).arg(zValue,0,'G').arg(_images[i+1]->tagName());
        }
      }
    }
  }

  KstApp::inst()->slotUpdateDataMsg(msg);
}


void Kst2DPlot::mouseMoveEvent(QWidget *view, QMouseEvent *e) {
  int x, y;
  int width, height;
  int i_label = -1;

  if (e->pos() == QPoint(-1, -1)) {
    QPainter p(view);
    removeFocus(p);
    return;
  }

  if (!_hasFocus) {
    KstViewWidget *w = dynamic_cast<KstViewWidget*>(view);
    QPainter p(view);
    if (w) {
      w->viewObject()->forEachChild<QPainter&>(&KstViewObject::removeFocus, p);
    }
    setHasFocus(true);
    p.setClipRegion(_lastClipRegion);
    updateTieBox(p);
  }

  _mouse.tracker = e->pos();

  if (globalZoomType() == LABEL_TOOL) { // HACK
    _mouse.mode = LABEL_TOOL;
  } else if (_mouse.mode == LABEL_TOOL) {
    _mouse.mode = INACTIVE;
  }

  if (e->state() & Qt::LeftButton && _mouse.mode == LABEL_TOOL &&
        (i_label = labelNumber(e)) >= 0 && _draggableLabel == i_label) {
    // Start a drag
    KstLabelPtr label = _labelList[i_label];
    if (label) {
#define LABEL_TRANSPARENT
      QRegion oldExtents = label->extents;
      QPixmap pm(GetWinRegion().width(), GetWinRegion().height());
      QRect rectBounding = oldExtents.boundingRect();

#ifdef LABEL_TRANSPARENT
      QBitmap bm(GetWinRegion().width(), GetWinRegion().height(), true);
      { // Scope is needed to kill off painter before we resize
        QPainter p(&bm);
        label->draw(p, 0, label->v_offset, false);
      }
#else
      pm.fill(_backgroundColor);
#endif

      { // Scope is needed to kill off painter before we resize
        QPainter p(&pm);
        p.setPen(QPen(_foregroundColor));
        label->draw(p, 0, label->v_offset, false);
      }

      // do not allow the pixmap to be increased in size, else we will be
      //  drawing a partially uninitialised pixmap during the drag operation...
      width = rectBounding.width();
      if (GetWinRegion().width() < width) {
        width = GetWinRegion().width();
      }
      height = rectBounding.height();
      if (GetWinRegion().height() < height) {
        height = GetWinRegion().height();
      }

      pm.resize(width, height);
#ifdef LABEL_TRANSPARENT
      bm.resize(width, height);
      pm.setMask(bm);
#endif

      label->extents = oldExtents; // restore them in case the drag is canceled
      QDragObject *d = new KstLabelDrag(view, static_cast<KstViewWindow*>(view->parent())->caption(), this, i_label, _draggablePoint - GetWinRegion().topLeft() - rectBounding.topLeft(), pm);
      d->dragMove();
      _draggableLabel = -2;
      static_cast<KstViewWidget*>(view)->viewObject()->releaseMouse(this);
    }
    return;
  } else if (e->state() & Qt::LeftButton && _mouse.mode == LABEL_TOOL && legendUnder(e)) {
    // Start a legend drag
    QRegion oldExtents = Legend->extents;
    QPixmap pm(GetWinRegion().width(), GetWinRegion().height());
    pm.fill(_backgroundColor);
    QRect rectBounding = oldExtents.boundingRect();

    { // Scope is needed to kill off painter before we resize
      QPainter p(&pm);
      p.setBackgroundColor(_backgroundColor);
      Legend->draw(&Curves, p, 0, 0);
    }

    // do not allow the pixmap to be increased in size, else we will be
    // drawing a partially uninitialised pixmap during the drag operation...
    width = rectBounding.width();
    if (GetWinRegion().width() < width) {
      width = GetWinRegion().width();
    }
    height = rectBounding.height();
    if (GetWinRegion().height() < height) {
      height = GetWinRegion().height();
    }

    pm.resize(width, height);
    Legend->extents = oldExtents; // restore them in case the drag is canceled
    QDragObject *d = new KstLegendDrag(view, static_cast<KstViewWindow*>(view->parent())->caption(), this, _draggablePoint - GetWinRegion().topLeft() - rectBounding.topLeft(), pm);
    d->dragMove();
    _draggableLabel = -2;
    static_cast<KstViewWidget*>(view)->viewObject()->releaseMouse(this);
    return;
  }

  KstMouseModeType newType = _mouse.mode;
  QRect pr = GetPlotRegion();
  if (e->state() & Qt::LeftButton && _mouse.zooming()) {
    // LEAVE BLANK
  } else if (KstApp::inst()->dataMode() && pr.contains(e->pos())) {
    highlightNearestDataPoint(true, view, e->pos());
  } else if (pr.contains(e->pos())) {
    updateMousePos(e->pos());
  } else {
    KstApp::inst()->slotUpdateDataMsg(QString::null);
  }

  if (_mouse.mode == XY_ZOOMBOX) {
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

    zoomRectUpdate(view, newType, x, y);
    setCursorForMode(view, _mouse.mode, e->pos());
  } else if (_mouse.mode == Y_ZOOMBOX) {
    x = pr.right();

    if (e->y() > pr.bottom()) {
      y = pr.bottom() + 1;
    } else if (e->y() < pr.top()) {
      y = pr.top();
    } else {
      y = e->y();
    }

    zoomRectUpdate(view, newType, x, y);
    setCursorForMode(view, _mouse.mode, e->pos());
  } else if (_mouse.mode == X_ZOOMBOX) {
    if (e->x() > pr.right()) {
      x = pr.right() + 1;
    } else if (e->x() < pr.left()) {
      x = pr.left();
    } else {
      x = e->x();
    }

    y = pr.bottom();
    zoomRectUpdate(view, newType, x, y);
    setCursorForMode(view, _mouse.mode, e->pos());
  } else if (_mouse.mode == LABEL_TOOL) {
    if (legendUnder(e)) {
      view->setCursor(QCursor(Qt::ArrowCursor));
    } else if (labelNumber(e) < 0) {
      setCursorForMode(view, LABEL_TOOL, e->pos());
    } else {
      view->setCursor(QCursor(Qt::ArrowCursor));
    }
  } else {
    ButtonState s = e->stateAfter();
    if (pr.contains(e->pos())) {
      if (s & Qt::ShiftButton) {
        setCursorForMode(view, Y_ZOOMBOX, e->pos());
      } else if (s & Qt::ControlButton) {
        setCursorForMode(view, X_ZOOMBOX, e->pos());
      } else {
        setCursorForMode(view, globalZoomType(), e->pos());
      }
    } else {
      view->setCursor(QCursor(Qt::ArrowCursor));
    }
  }
}


bool Kst2DPlot::legendUnder(QMouseEvent *e) {
  QPoint pt = GetWinRegion().topLeft();
  QPoint ptCheck(e->x() - pt.x(), e->y() - pt.y());
  return Legend && Legend->extents.contains(ptCheck);
}


int Kst2DPlot::labelNumber(QMouseEvent *e) {
  int i_label = -1;
  uint cnt = _labelList.count();
  QPoint pt = GetWinRegion().topLeft();
  QPoint ptCheck(e->x() - pt.x(), e->y() - pt.y());
  for (uint i = 0; i < cnt; ++i) {
    if (_labelList[i]->extents.contains(ptCheck)) {
      i_label = i;
      break;
    }
  }
  return i_label;
}


void Kst2DPlot::mousePressEvent(QWidget *view, QMouseEvent *e) {
  QRect win_rect, plot_rect, tie_rect, plot_and_axis_rect;
  KstApp *ParentApp = KstApp::inst();

  static_cast<KstViewWidget*>(view)->viewObject()->grabMouse(this);

  /* Find where the mouse was to determine which mode to be in */
  /* which button */
  if (e->button() == Qt::LeftButton) {
    _draggableLabel = labelNumber(e);
    _draggablePoint = e->pos();

    win_rect = GetWinRegion();
    plot_rect = GetPlotRegion();
    tie_rect = GetTieBoxRegion();
    plot_and_axis_rect = GetPlotAndAxisRegion();
    //kstdDebug() << e->pos() << " " << win_rect << " " << plot_rect << endl;
    if (tie_rect.contains(e->pos())) {
      toggleTied();
      // So inefficient, but I have some sort of weird bug making it necessary
      static_cast<KstViewWidget*>(view)->paint();
      return;
    } else if (plot_rect.contains(e->pos())) {
      if (_mouse.mode == LABEL_TOOL) {
      } else {
        if (e->state() & Qt::ShiftButton) {
          _mouse.mode = Y_ZOOMBOX;
        } else if (e->state() & Qt::ControlButton) {
          _mouse.mode = X_ZOOMBOX;
        } else {
          _mouse.mode = globalZoomType();
          assert(_mouse.mode != INACTIVE);
        }
        if (_mouse.mode != LABEL_TOOL) {
          _mouse.plotGeometry = GetPlotRegion();
          _mouse.zoomStart(_mouse.mode, e->pos());
          _zoomPaused = true;
        }
      }
      return;
    } else if (plot_and_axis_rect.contains(e->pos())) {
      ParentApp->plotDialog()->show_I(static_cast<KstViewWidget*>(view)->viewObject()->tagName(), tagName());
      ParentApp->plotDialog()->TabWidget->setCurrentPage(LIMITS_TAB);
      return;
    } else if (win_rect.contains(e->pos())) {
      ParentApp->plotDialog()->show_I(static_cast<KstViewWidget*>(view)->viewObject()->tagName(), tagName());
      ParentApp->plotDialog()->TabWidget->setCurrentPage(LABELS_TAB);
      return;
    }
  } else if (e->button() == Qt::RightButton) {
    win_rect = GetPlotRegion();
    if (win_rect.contains(e->pos())) {
      _mouse.mode = INACTIVE;
      _mouse.pressLocation = e->pos();
      return;
    }
  } else if (e->button() == Qt::MidButton) {
    win_rect = GetWinRegion();
    if (win_rect.contains(e->pos())) {
      _mouse.mode = INACTIVE;
      _mouse.pressLocation = e->pos();
      zoomPrev(static_cast<KstViewWidget *>(view));
      return;
    }
  } else {
    // cout << "unknown button: " << e->button() << "\n";
  }
}


void Kst2DPlot::mouseReleaseEvent(QWidget *view, QMouseEvent *e) {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax, new_ymin, new_ymax;
  QRect plotregion;
  bool doUpdate = false;

  _zoomPaused = false;
  static_cast<KstViewWidget*>(view)->viewObject()->releaseMouse(this);

  _mouse.tracker = e->pos();

  QRect newg = _mouse.mouseRect();
  if (_mouse.mode == XY_ZOOMBOX) {
    if (_mouse.rectBigEnough()) {
      QPainter p(view);
      p.setClipRegion(_lastClipRegion);
      p.setRasterOp(Qt::NotROP);
      p.drawWinFocusRect(newg);

      getLScale(xmin, ymin, xmax, ymax);
      plotregion = GetPlotRegion();
      new_xmin = (double)(newg.left() - plotregion.left())/
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_xmax = (double)(newg.right() - plotregion.left() + 1) /
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_ymin = (double)(newg.bottom() - plotregion.top() + 1) /
                 (double)plotregion.height() * (ymin - ymax) + ymax;
      new_ymax = (double)(newg.top() - plotregion.top())/
                 (double)plotregion.height() * (ymin - ymax) + ymax;

      setXScaleMode(FIXED);
      setYScaleMode(FIXED);
      if (setLScale(new_xmin, new_ymin, new_xmax, new_ymax)) {
        pushScale();
        doUpdate = true;
        _mouse.lastLocation = _mouse.pressLocation;
        if (isTied()) {
          Kst2DPlotList pl = static_cast<KstViewWidget*>(view)->viewObject()->findChildrenType<Kst2DPlot>(true);
          for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
            Kst2DPlotPtr p = *i;
            if (p->isTied() && p.data() != this) {
              p->setXScaleMode(FIXED);
              p->setYScaleMode(FIXED);
              p->setLScale(new_xmin, new_ymin, new_xmax, new_ymax);
              p->pushScale();
              p->setDirty();
            }
          }
        }
      }
    }
  } else if (_mouse.mode == Y_ZOOMBOX) {
    if (newg.height() >= _mouse.minMove) {
      QPainter p(view);
      p.setClipRegion(_lastClipRegion);
      p.setRasterOp(Qt::NotROP);
      p.drawWinFocusRect(newg);

      getLScale(xmin, ymin, xmax, ymax);
      plotregion = GetPlotRegion();
      new_ymin = (double)(newg.bottom() - plotregion.top() + 1) /
                 (double)plotregion.height() * (ymin - ymax) + ymax;
      new_ymax = (double)(newg.top() - plotregion.top()) /
                 (double)plotregion.height() * (ymin - ymax) + ymax;

      setYScaleMode(FIXED);
      if (setLYScale(new_ymin, new_ymax)) {
        pushScale();
        doUpdate = true;
        _mouse.lastLocation = _mouse.pressLocation;
        if (isTied()) {
          Kst2DPlotList pl = static_cast<KstViewWidget*>(view)->viewObject()->findChildrenType<Kst2DPlot>(true);
          for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
            Kst2DPlotPtr p = *i;
            if (p->isTied() && p.data() != this) {
              p->setYScaleMode(FIXED);
              p->setLYScale(new_ymin, new_ymax);
              p->pushScale();
              p->setDirty();
            }
          }
        }
      }
    }
  } else if (_mouse.mode == X_ZOOMBOX) {
    if (newg.width() >= _mouse.minMove) {
      QPainter p(view);
      p.setClipRegion(_lastClipRegion);
      p.setRasterOp(Qt::NotROP);
      p.drawWinFocusRect(newg);

      getLScale(xmin, ymin, xmax, ymax);
      plotregion = GetPlotRegion();
      new_xmin = (double)(newg.left() - plotregion.left()) /
                 (double)plotregion.width() * (xmax - xmin) + xmin;
      new_xmax = (double)(newg.right() -
                          plotregion.left() + 1) /
                 (double)plotregion.width() * (xmax - xmin) + xmin;

      setXScaleMode(FIXED);
      if (setLXScale(new_xmin, new_xmax)) {
        pushScale();
        doUpdate = true;
        _mouse.lastLocation = _mouse.pressLocation;
        if (isTied()) {
          Kst2DPlotList pl = static_cast<KstViewWidget*>(view)->viewObject()->findChildrenType<Kst2DPlot>(true);
          for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
            Kst2DPlotPtr p = *i;
            if (p->isTied() && p.data() != this) {
              p->setXScaleMode(FIXED);
              p->setLXScale(new_xmin, new_xmax);
              p->pushScale();
              p->setDirty();
            }
          }
        }
      }
    }
  } else if (_mouse.mode == LABEL_TOOL) {
    plotregion = GetPlotRegion();

    double x = double(e->x() - plotregion.left()) / double(plotregion.width());
    double y = double(e->y() - plotregion.top()) / double(plotregion.height());
    int i_label;

    if ((i_label = labelNumber(e)) >= 0 && _draggableLabel == i_label)  {
      KstApp::inst()->labelDialog()->showI(this, i_label, x, y);
    } else if (legendUnder(e)) {
      KstApp::inst()->plotDialog()->show_I(static_cast<KstViewWidget*>(view)->viewObject()->tagName(), tagName());
      KstApp::inst()->plotDialog()->TabWidget->setCurrentPage(LEGEND_TAB);
    } else if (plotregion.contains(e->pos()) && _draggableLabel == -1) {
      KstApp::inst()->labelDialog()->showI(this, -1, x, y);
    }

    return; // no need to update, and we don't want to set INACTIVE
  }

  _mouse.mode = INACTIVE;
  //needrecreate = true;
  if (doUpdate) {
    setDirty();
    static_cast<KstViewWidget*>(view)->paint();
    //emit modified();
  }
}


KstMouse::KstMouse()
{
  mode = INACTIVE;
  label = -1;
  minMove = 2;
  lastLocation = QPoint(-1, -1);
  tracker = QPoint(-1, -1);
}


void KstMouse::zoomUpdate(KstMouseModeType t, const QPoint& location) {
  mode = t;
  lastLocation = location;
}


void KstMouse::zoomStart(KstMouseModeType t, const QPoint& location) {
  mode = t;
  pressLocation = lastLocation = location;
}


bool KstMouse::rectBigEnough() const {
  QRect r = mouseRect();
  return r.width() >= minMove && r.height() >= minMove;
}


QRect KstMouse::mouseRect() const {
  QRect rc = QRect(QMIN(pressLocation.x(), lastLocation.x()), QMIN(pressLocation.y(), lastLocation.y()), QABS(pressLocation.x() - lastLocation.x()), QABS(pressLocation.y() - lastLocation.y()));
  switch (mode) {
    case X_ZOOMBOX:
      rc.setTop(plotGeometry.top());
      rc.setBottom(plotGeometry.bottom());
      break;
    case Y_ZOOMBOX:
      rc.setLeft(plotGeometry.left());
      rc.setRight(plotGeometry.right());
      break;
    default:
      break;
  }
  return rc;
}


bool KstMouse::zooming() const {
  return mode == XY_ZOOMBOX || mode == X_ZOOMBOX || mode == Y_ZOOMBOX;
}


void Kst2DPlot::zoomRectUpdate(QWidget *view, KstMouseModeType t, int x, int y) {
  QPoint newp(x, y);

  if (_mouse.lastLocation != newp) {
    QPainter p(view);
    p.setClipRegion(_lastClipRegion);
    p.setRasterOp(Qt::NotROP);
    if (_mouse.rectBigEnough()) {
      p.drawWinFocusRect(_mouse.mouseRect());
    }
    _mouse.zoomUpdate(t, newp);
    if (_mouse.rectBigEnough()) {
      p.drawWinFocusRect(_mouse.mouseRect());
    }
  }
}


void Kst2DPlot::setCursorForMode(QWidget *view) {
  Q_ASSERT(false);
  setCursorForMode(view, _mouse.mode, _mouse.tracker);
}


void Kst2DPlot::setCursorForMode(QWidget *view, KstMouseModeType mode, const QPoint& pos) {
  switch (mode) {
    case Y_ZOOMBOX:
      view->setCursor(QCursor(Qt::SizeVerCursor));
      break;
    case X_ZOOMBOX:
      view->setCursor(QCursor(Qt::SizeHorCursor));
      break;
    case XY_ZOOMBOX:
      view->setCursor(QCursor(Qt::CrossCursor));
      break;
    case LABEL_TOOL:
      view->setCursor(QCursor(Qt::IbeamCursor));
      break;
    default:
      if (GetPlotRegion().contains(pos)) {
        view->setCursor(QCursor(Qt::CrossCursor));
      } else {
        view->setCursor(QCursor(Qt::ArrowCursor));
      }
      break;
  }
}


void Kst2DPlot::keyReleaseEvent(QWidget *view, QKeyEvent *e) {
  if (_mouse.mode != INACTIVE) {
    e->ignore();
    return;
  }

  KstMouseModeType newType = globalZoomType();
  QPoint c = _mouse.lastLocation;
  QRect pr = GetPlotRegion();
  int x = _mouse.pressLocation.x(), y = _mouse.pressLocation.y();

  if (newType == Y_ZOOMBOX) {
    if (c.y() > pr.bottom()) {
      y = pr.bottom() + 1;
    } else if (c.y() < pr.top()) {
      y = pr.top();
    } else {
      y = c.y();
    }
  } else if (newType == X_ZOOMBOX) {
    if (c.x() > pr.right()) {
      x = pr.right() + 1;
    } else if (c.x() < pr.left()) {
      x = pr.left();
    } else {
      x = c.x();
    }
  } else {
    if (c.x() > pr.right()) {
      x = pr.right() + 1;
    } else if (c.x() < pr.left()) {
      x = pr.left();
    } else {
      x = c.x();
    }

    if (c.y() > pr.bottom()) {
      y = pr.bottom() + 1;
    } else if (c.y() < pr.top()) {
      y = pr.top();
    } else {
      y = c.y();
    }
  }

  if (_mouse.zooming()) {
    QPoint newp(x, y);
    QPainter p(view);
    p.setClipRegion(_lastClipRegion);
    p.setRasterOp(Qt::NotROP);
    if (_mouse.rectBigEnough()) {
      p.drawWinFocusRect(_mouse.mouseRect());
    }

    _mouse.zoomUpdate(newType, newp);
    if (_mouse.rectBigEnough()) {
      p.drawWinFocusRect(_mouse.mouseRect());
    }
  }
  setCursorForMode(view, newType, c);
  e->accept();
}


void Kst2DPlot::cancelZoom(QWidget *view) {
  if (_mouse.rectBigEnough()) {
    QPainter p(view);
    p.setClipRegion(_lastClipRegion);
    p.setRasterOp(Qt::NotROP);
    p.drawWinFocusRect(_mouse.mouseRect());
  }

  _mouse.lastLocation = _mouse.pressLocation; // make rectBigEnough() false
  _mouse.mode = INACTIVE;
}


void Kst2DPlot::menuXZoomMax() {
  if (_menuView) {
    xZoomMax(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYZoomMax() {
  if (_menuView) {
    yZoomMax(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuZoomMax() {
  if (_menuView) {
    zoomMax(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuXLogSlot() {
  if (_menuView) {
    xLogSlot(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYLogSlot() {
  if (_menuView) {
    yLogSlot(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuZoomPrev() {
  if (_menuView) {
    zoomPrev(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYZoomAc() {
  if (_menuView) {
    yZoomAc(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuZoomSpikeInsensitiveMax() {
  if (_menuView) {
    zoomSpikeInsensitiveMax(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuXZoomIn() {
  if (_menuView) {
    xZoomIn(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuXZoomOut() {
  if (_menuView) {
    xZoomOut(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYZoomIn() {
  if (_menuView) {
    yZoomIn(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYZoomOut() {
  if (_menuView) {
    yZoomOut(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuXNormalize() {
  if (_menuView) {
    xZoomNormal(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuYNormalize() {
  if (_menuView) {
    yZoomNormal(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuMoveUp() {
  if (_menuView) {
    moveUp(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuMoveDown() {
  if (_menuView) {
    moveDown(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuMoveLeft() {
  if (_menuView) {
    moveLeft(_menuView);
    _menuView->paint();
  }
}


void Kst2DPlot::menuMoveRight() {
  if (_menuView) {
    moveRight(_menuView);
    _menuView->paint();
  }
}

void Kst2DPlot::menuNextMarker() {
  if (_menuView) {
    moveToNextMarker(_menuView);
    _menuView->paint();
  }
}

void Kst2DPlot::menuPrevMarker() {
  if (_menuView) {
    moveToPrevMarker(_menuView);
    _menuView->paint();
  }
}

void Kst2DPlot::moveLeft(KstViewWidget *view) {
  Q_UNUSED(view)

  if (moveSelfHorizontal(true)) {
    updateTiedPlots(&Kst2DPlot::moveSelfHorizontal, true);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::moveRight(KstViewWidget *view) {
  Q_UNUSED(view)

  if (moveSelfHorizontal(false)) {
    updateTiedPlots(&Kst2DPlot::moveSelfHorizontal, false);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::moveUp(KstViewWidget *view) {
  Q_UNUSED(view)

  if (moveSelfVertical(true)) {
    updateTiedPlots(&Kst2DPlot::moveSelfVertical, true);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::moveDown(KstViewWidget *view) {
  Q_UNUSED(view)

  if (moveSelfVertical(false)) {
    updateTiedPlots(&Kst2DPlot::moveSelfVertical, false);
    pushScale();
    setDirty();
  }
}

void Kst2DPlot::moveToNextMarker(KstViewWidget *view) {
  double newCenter, currCenter;
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;

  getLScale(xmin, ymin, xmax, ymax);
  currCenter = ((xmax + xmin) / 2.0) + (xmax - xmin)/MARKER_NUM_SEGS;
  if (_xLog) {
    currCenter = pow(10, currCenter);
  }
  if (nextMarker(currCenter, newCenter)) {
    if (_xLog) {
      newCenter = logX(newCenter);
    }
    new_xmin = newCenter - (xmax - xmin)/2.0;
    new_xmax = newCenter + (xmax - xmin)/2.0;
    setXScaleMode(FIXED);
    setLXScale(new_xmin, new_xmax);

    // now move all all the other tied plots to the same center
    if (_xLog) {
      newCenter = pow(10, newCenter);
    }
    updateTiedPlots(&Kst2DPlot::moveSelfToCenter, newCenter);
    pushScale();
    setDirty();
    view->paint();
  }
}

void Kst2DPlot::moveToPrevMarker(KstViewWidget *view) {
  double newCenter, currCenter;
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;

  getLScale(xmin, ymin, xmax, ymax);
  currCenter = ((xmax + xmin) / 2.0) - (xmax - xmin)/MARKER_NUM_SEGS;
  if (_xLog) {
    currCenter = pow(10, currCenter);
  }
  if (prevMarker(currCenter, newCenter)) {
    if (_xLog) {
      if (newCenter > 0) {
        newCenter = logX(newCenter);
      } else {
        return; //don't scroll left past 0 in log mode
      }
    }
    new_xmin = newCenter - (xmax - xmin)/2.0;
    new_xmax = newCenter + (xmax - xmin)/2.0;
    setXScaleMode(FIXED);
    setLXScale(new_xmin, new_xmax);

    // now move all the other tied plots to the same center
    if (_xLog) {
      newCenter = pow(10, newCenter);
    }
    updateTiedPlots(&Kst2DPlot::moveSelfToCenter, newCenter);
    pushScale();
    setDirty();
    view->paint();
  }
}

void Kst2DPlot::moveSelfToCenter(double center) {
  // log the center if necessary
  if (_xLog) {
    center = logX(center);
  }

  // refuse to move if it's not possible
  if (_xLog && center <= -350) {
    return;
  }

  double xmin, xmax, ymin, ymax;
  getLScale(xmin, ymin, xmax, ymax);

  double new_xmin = center - (xmax - xmin)/2.0;
  double new_xmax = center + (xmax - xmin)/2.0;
  setXScaleMode(FIXED);
  setLXScale(new_xmin, new_xmax);
}


bool Kst2DPlot::moveSelfHorizontal(bool left) {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;

  getLScale(xmin, ymin, xmax, ymax);

  if (left) {
    new_xmin = xmin - (xmax - xmin)*0.1;
    new_xmax = xmax - (xmax - xmin)*0.1;
  } else {
    new_xmin = xmin + (xmax - xmin)*0.1;
    new_xmax = xmax + (xmax - xmin)*0.1;
  }

  setXScaleMode(FIXED);
  return setLXScale(new_xmin, new_xmax);
}


bool Kst2DPlot::moveSelfVertical(bool up) {
  double xmin, xmax, ymin, ymax;
  double new_ymin, new_ymax;

  getLScale(xmin, ymin, xmax, ymax);

  if (up) {
    new_ymin = ymin + (ymax - ymin)*0.25;
    new_ymax = ymax + (ymax - ymin)*0.25;
  } else {
    new_ymin = ymin - (ymax - ymin)*0.25;
    new_ymax = ymax - (ymax - ymin)*0.25;
  }

  setYScaleMode(FIXED);
  return setLYScale(new_ymin, new_ymax);
}


bool Kst2DPlot::zoomSelfVertical(bool in) {
  double xmin, xmax, ymin, ymax;
  double new_ymin, new_ymax;

  getLScale(xmin, ymin, xmax, ymax);

  if (in) {
    new_ymin = ymin + (ymax - ymin)*0.1666666;
    new_ymax = ymax - (ymax - ymin)*0.1666666;
  } else {
    new_ymin = ymin - (ymax - ymin)*0.25;
    new_ymax = ymax + (ymax - ymin)*0.25;
  }

  setYScaleMode(FIXED);
  return setLYScale(new_ymin, new_ymax);
}


void Kst2DPlot::yZoomIn(KstViewWidget *view) {
  Q_UNUSED(view)

  if (zoomSelfVertical(true)) {
    updateTiedPlots(&Kst2DPlot::zoomSelfVertical, true);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::yZoomOut(KstViewWidget *view) {
  Q_UNUSED(view)

  if (zoomSelfVertical(false)) {
    updateTiedPlots(&Kst2DPlot::zoomSelfVertical, false);
    pushScale();
    setDirty();
  }
}


bool Kst2DPlot::zoomSelfHorizontal(bool in) {
  double xmin, xmax, ymin, ymax;
  double new_xmin, new_xmax;

  getLScale(xmin, ymin, xmax, ymax);

  if (in) {
    new_xmin = xmin + (xmax - xmin)*0.1666666;
    new_xmax = xmax - (xmax - xmin)*0.1666666;
  } else {
    new_xmin = xmin - (xmax - xmin)*0.25;
    new_xmax = xmax + (xmax - xmin)*0.25;
  }

  setXScaleMode(FIXED);
  return setLXScale(new_xmin, new_xmax);
}


void Kst2DPlot::xZoomIn(KstViewWidget *view) {
  Q_UNUSED(view)

  if (zoomSelfHorizontal(true)) {
    updateTiedPlots(&Kst2DPlot::zoomSelfHorizontal, true);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::xZoomOut(KstViewWidget *view) {
  Q_UNUSED(view)

  if (zoomSelfHorizontal(false)) {
    updateTiedPlots(&Kst2DPlot::zoomSelfHorizontal, false);
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::xZoomNormal(KstViewWidget *view) {
  if (!isXLog() && !isYLog()) {
    double xmin, xmax, ymin, ymax;
    double new_xmin, new_xmax;
    double mean;
    double range;

    getLScale(xmin, ymin, xmax, ymax);
    mean = xmin + ((xmax - xmin) / 2.0);
    range = (double)PlotRegion.width() * (ymax - ymin) / (double)PlotRegion.height();

    new_xmin = mean - (range / 2.0);
    new_xmax = mean + (range / 2.0);
    setXScaleMode(FIXED);
    setXScale(new_xmin, new_xmax);
    if (isTied()) {
      Kst2DPlotList pl = static_cast<KstViewWidget*>(view)->viewObject()->findChildrenType<Kst2DPlot>(true);
      for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
        Kst2DPlotPtr p = *i;
        if (p->isTied() && p.data() != this) {
          p->setXScaleMode(FIXED);
          p->setXScale(new_xmin, new_xmax);
          p->pushScale();
          p->setDirty();
        }
      }
    }
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::yZoomNormal(KstViewWidget *view) {
  if (!isXLog() && !isYLog()) {
    double xmin, xmax, ymin, ymax;
    double new_ymin, new_ymax;
    double mean;
    double range;

    getLScale(xmin, ymin, xmax, ymax);
    mean = ymin + ((ymax - ymin) / 2.0);
    range = (double)PlotRegion.height() * (xmax - xmin) / (double)PlotRegion.width();

    new_ymin = mean - (range / 2.0);
    new_ymax = mean + (range / 2.0);
    setYScaleMode(FIXED);
    setYScale(new_ymin, new_ymax);
    if (isTied()) {
      Kst2DPlotList pl = static_cast<KstViewWidget*>(view)->viewObject()->findChildrenType<Kst2DPlot>(true);
      for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
        Kst2DPlotPtr p = *i;
        if (p->isTied() && p.data() != this) {
          p->setYScaleMode(FIXED);
          p->setYScale(new_ymin, new_ymax);
          p->pushScale();
          p->setDirty();
        }
      }
    }
    pushScale();
    setDirty();
  }
}


void Kst2DPlot::xZoomMax(KstViewWidget *view) {
  Q_UNUSED(view)

  setXScaleMode(AUTO);
  updateTiedPlots(&Kst2DPlot::setXScaleMode, AUTO);
  pushScale();
  setDirty();
}


void Kst2DPlot::yZoomMax(KstViewWidget *view) {
  Q_UNUSED(view)

  setYScaleMode(AUTO);
  updateTiedPlots(&Kst2DPlot::setYScaleMode, AUTO);
  pushScale();
  setDirty();
}


void Kst2DPlot::zoomMax(KstViewWidget *view) {
  Q_UNUSED(view)

  setXScaleMode(AUTO);
  setYScaleMode(AUTOBORDER);
  // Hmm could make a method that does both at once.
  updateTiedPlots(&Kst2DPlot::setXScaleMode, AUTO);
  updateTiedPlots(&Kst2DPlot::setYScaleMode, AUTOBORDER);
  pushScale();
  setDirty();
}


void Kst2DPlot::xLogSlot(KstViewWidget *view) {
  Q_UNUSED(view)

  setLog(!isXLog(), isYLog());
  setDirty();
}


void Kst2DPlot::yLogSlot(KstViewWidget *view) {
  Q_UNUSED(view)

  setLog(isXLog(), !isYLog());
  setDirty();
}


void Kst2DPlot::zoomPrev(KstViewWidget *view) {
  if (popScale()) {
    if (isTied()) {
      Kst2DPlotList pl = view->viewObject()->findChildrenType<Kst2DPlot>(true);
      for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
        Kst2DPlotPtr p = *i;
        if (p->isTied() && p.data() != this) {
          p->popScale();
          p->cancelZoom(view);
          p->setDirty();
        }
      }
    }
    setDirty();
  }
}


void Kst2DPlot::yZoomAc(KstViewWidget *view) {
  Q_UNUSED(view)

  setYScaleMode(AC);
  pushScale();
  updateTiedPlots(&Kst2DPlot::setYScaleMode, AC);
  setDirty();
}


void Kst2DPlot::zoomSpikeInsensitiveMax(KstViewWidget *view) {
  Q_UNUSED(view)

  setXScaleMode(NOSPIKE);
  setYScaleMode(NOSPIKE);
  updateTiedPlots(&Kst2DPlot::setXScaleMode, NOSPIKE);
  updateTiedPlots(&Kst2DPlot::setYScaleMode, NOSPIKE);
  pushScale();
  setDirty();
}


void Kst2DPlot::keyPressEvent(QWidget *vw, QKeyEvent *e) {
  if (_mouse.mode == LABEL_TOOL) {
    e->ignore();
    return;
  }

  bool handled = true;
  ButtonState s = e->stateAfter();
  KstViewWidget *view = static_cast<KstViewWidget*>(vw);
  QPoint cursorPos = _mouse.tracker;
  switch (e->key()) {
    case Key_A:
      yZoomAc(view);
      break;
    case Key_C:
      if (s & Qt::ShiftButton) {
        unsetCursorPos(view);
      } else {
        setCursorPos(view);
      }
      break;
    case Key_E:
      edit();
      break;
    case Key_G:
      xLogSlot(view);
      break;
    case Key_L:
      yLogSlot(view);
      break;
    case Key_M:
      if (s & Qt::ShiftButton) {
        yZoomMax(view);
      } else if (s & Qt::ControlButton) {
        xZoomMax(view);
        view->paint();
      } else {
        zoomMax(view);
      }
      break;
    case Key_N:
      if (s & Qt::ShiftButton) {
        yZoomNormal(view);
      } else {
        xZoomNormal(view);
      }
      break;
    case Key_P:
      pauseToggle();
      setDirty();
      break;
    case Key_R:
      zoomPrev(view);
      break;
    case Key_S:
      zoomSpikeInsensitiveMax(view);
      break;
    case Key_Z:
      zoomToggle();
      cancelZoom(view);
      setDirty();
      break;
    case Key_Left:
      if (s & Qt::ShiftButton) {
        xZoomIn(view);
      } else if (s & Qt::AltButton) {
        moveToPrevMarker(view);
      } else {
        moveLeft(view);
      }
      break;
    case Key_Right:
      if (s & Qt::ShiftButton) {
        xZoomOut(view);
      } else if (s & Qt::AltButton) {
        moveToNextMarker(view);
      } else {
        moveRight(view);
      }
      break;
    case Key_Up:
      if (s & Qt::ShiftButton) {
        yZoomOut(view);
      } else {
        moveUp(view);
      }
      break;
    case Key_Down:
      if (s & Qt::ShiftButton) {
        yZoomIn(view);
      } else {
        moveDown(view);
      }
      break;
    case Key_Insert:
      if (!e->isAutoRepeat() && GetPlotRegion().contains(cursorPos)) {
        if (_xLog) {
          setPlotMarker(pow(10,((cursorPos.x() - position().x() - _b_X) / _m_X)), false, false);
        } else {
          setPlotMarker((cursorPos.x() - position().x() - _b_X) / _m_X, false, false);
        }
        setDirty();
      }
      break;
    default:
      handled = false;
      break;
  }

  if (handled) {
    view->paint();
    e->accept();
    //emit modified();
    return;
  }

  if (_mouse.zooming()) {
    KstMouseModeType newType = _mouse.mode;

    if (e->key() == Qt::Key_Escape) {
      cancelZoom(view);
    } else {
      QPoint newp = _mouse.lastLocation;

      QPainter p(view);
      p.setClipRegion(_lastClipRegion);
      p.setRasterOp(Qt::NotROP);
      if (_mouse.rectBigEnough()) {
        p.drawWinFocusRect(_mouse.mouseRect());
      }

      _mouse.zoomUpdate(newType, newp);
      if (_mouse.rectBigEnough()) {
        p.drawWinFocusRect(_mouse.mouseRect());
      }
    }
    setCursorForMode(view, _mouse.mode, cursorPos);
  } else {
    if (_mouse.mode == INACTIVE && GetPlotRegion().contains(cursorPos)) {
      if (s & Qt::ShiftButton) {
        setCursorForMode(view, Y_ZOOMBOX, cursorPos);
      } else if (s & Qt::ControlButton) {
        setCursorForMode(view, X_ZOOMBOX, cursorPos);
      } else {
        setCursorForMode(view, globalZoomType(), cursorPos);
      }
    } else {
      e->ignore();
      return;
    }
  }
  e->accept();
}


KstMouseModeType Kst2DPlot::globalZoomType() const {
  switch (KstApp::inst()->getZoomRadio()) {
    case KstApp::XZOOM:
      return X_ZOOMBOX;
    case KstApp::YZOOM:
      return Y_ZOOMBOX;
    case KstApp::XYZOOM:
      return XY_ZOOMBOX;
    case KstApp::TEXT:
      return LABEL_TOOL;
    case KstApp::LAYOUT:
      return LAYOUT_TOOL;
    default:
      break;
  }

  return INACTIVE;
}


void Kst2DPlot::dragEnterEvent(QWidget *view, QDragEnterEvent *e) {
  _draggableLabel = -2;

  e->accept(e->provides("application/x-kst-label-number") ||
            e->source() == view && e->provides("application/x-kst-legend"));
}


void Kst2DPlot::dragMoveEvent(QWidget *view, QDragMoveEvent *e) {
  _draggableLabel = -2;

  e->accept(e->provides("application/x-kst-label-number") ||
            e->source() == view && e->provides("application/x-kst-legend"));
}


void Kst2DPlot::dropEvent(QWidget *view, QDropEvent *e) {
  QString windowname;
  QString plotname;
  QString curvename;
  QPoint hs;
  bool accept = false;

  _mouse.tracker = e->pos();
    
  if (e->provides("application/x-kst-label-number")) {
    QDataStream ds(e->encodedData("application/x-kst-label-number"), IO_ReadOnly);
    int label;

    ds >> windowname >> plotname >> label >> hs;

    if (GetPlotRegion().contains(e->pos())) {
      KstApp *app = KstApp::inst();
      KstViewWindow *w = dynamic_cast<KstViewWindow*>(app->findWindow(windowname));
      if (w) {
        Kst2DPlotPtr p = kst_cast<Kst2DPlot>(w->view()->findChild(plotname));

        if (p) {
          KstLabelPtr l = p->_labelList[label];
          if (l) {
            p->_labelList.remove(l);
            p->setDirty();
            _labelList.append(l);

            QPoint pos = e->pos() - GetWinRegion().topLeft() - hs;
            QRect rectBounding = l->extents.boundingRect();
            pos.setX(pos.x() - rectBounding.left());
            pos.setY(pos.y() - rectBounding.top());
            QSize divisor = GetPlotRegion().size();
            l->offsetRelPosition(float(pos.x())/divisor.width(), float(pos.y())/divisor.height());
            accept = true;

            if (!_hasFocus) {
              QPainter p(view);
              w->view()->forEachChild<QPainter&>(&Kst2DPlot::removeFocus, p);
              setHasFocus(true);
            }

            if (e->source() != view) {
              static_cast<KstViewWidget*>(e->source())->paint();
            }

            setDirty();
          }
          static_cast<KstViewWidget*>(view)->paint();
        }
      }
    }
  } else if (e->provides("application/x-kst-legend")) {
    if (e->source() == view) {
      QDataStream ds(e->encodedData("application/x-kst-legend"), IO_ReadOnly);

      ds >> windowname >> plotname >> hs;

      if (GetPlotRegion().contains(e->pos())) {
        KstApp *app = KstApp::inst();
        KstViewWindow *w = dynamic_cast<KstViewWindow*>(app->findWindow(windowname));
        if (w) {
          Kst2DPlotPtr p = kst_cast<Kst2DPlot>(w->view()->findChild(plotname));

          if (p.data() == this) {
            KstLegendPtr l = Legend;

            QPoint pos = e->pos() - GetWinRegion().topLeft() - hs;
            QRect rectBounding = l->extents.boundingRect();
            pos.setX(pos.x() - rectBounding.left());
            pos.setY(pos.y() - rectBounding.top());
            QSize divisor = GetPlotRegion().size();
            l->offsetRelPosition(float(pos.x())/divisor.width(), float(pos.y())/divisor.height());
            accept = true;

            if (!_hasFocus) {
              QPainter p(view);
              w->view()->forEachChild<QPainter&>(&Kst2DPlot::removeFocus, p);
              setHasFocus(true);
            }
            setDirty();
            static_cast<KstViewWidget*>(view)->update(geometry());
          }
        }
      }
    }
  }

  if (accept) {
    KstApp::inst()->document()->setModified();
  }
  e->accept(accept);
}


void Kst2DPlot::copy() {
  // Don't set the selection because while it does make sense, it
  // is far too easy to swipe over Kst while trying to paste a selection
  // from one window to another.

  // FIXME: we should also provide a custom mime source so that we can
  //        actually manipulate points of data within Kst.
  QString msg = i18n("(%1, %2)").arg(_copy_x, 0, 'G').arg(_copy_y, 0, 'G');
  QApplication::clipboard()->setText(msg);
}


// FIXME: this does not strip out dupes.  Why?  Because we can't have them
//        by definition at present.  this might change one day.  Unfortunately
//        removal of dupes makes O(n) code become O(n^2).
Kst2DPlotList Kst2DPlot::globalPlotList() {
  Kst2DPlotList rc;
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  if (it) {
    while (it->currentItem()) {
      KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (view) {
        Kst2DPlotList sub = view->view()->findChildrenType<Kst2DPlot>(true);
        rc += sub;
      }
      it->next();
    }
    app->deleteIterator(it);
  }
  return rc;
}


#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

void Kst2DPlot::wheelEvent(QWidget *view, QWheelEvent *e) {
  KstViewWidget *vw = dynamic_cast<KstViewWidget*>(view);
  if (vw && GetPlotRegion().contains(e->pos())) {
    bool forward = e->delta() >= 0;
    int absDelta = forward ? e->delta() : -e->delta();
    bool alt = e->state() & Qt::AltButton;
    if (e->state() & Qt::ControlButton) {
      for (int i = 0; i < absDelta/WHEEL_DELTA; ++i) {
        if (forward) {
          xZoomIn(vw);
        } else {
          xZoomOut(vw);
        }
      }
      vw->paint();
    } else if (e->state() & Qt::ShiftButton) {
      for (int i = 0; i < absDelta/WHEEL_DELTA; ++i) {
        if (forward) {
          yZoomIn(vw);
        } else {
          yZoomOut(vw);
        }
      }
      vw->paint();
    } else {
      for (int i = 0; i < absDelta/WHEEL_DELTA; ++i) {
        if (forward) {
          if (alt) {
            moveUp(vw);
          } else {
            moveRight(vw);
          }
        } else {
          if (alt) {
            moveDown(vw);
          } else {
            moveLeft(vw);
          }
        }
      }
      vw->paint();
    }

    e->accept();
  }
}


bool Kst2DPlot::setPlotMarker(const double xValue, bool isRising, bool isFalling) {
  KstMarker marker;

  KstMarkerList::Iterator iter = _plotMarkers.begin();
  while (iter != _plotMarkers.end() && (*iter).value < xValue) {
    ++iter;
  }
  if (iter == _plotMarkers.end() || (*iter).value != xValue) {
    marker.value = xValue;
    marker.isRising = isRising;
    marker.isFalling = isFalling;
    _plotMarkers.insert(iter, marker);

    return true;
  }

  return false;
}

/*
bool Kst2DPlot::removePlotMarker(const double xValue) {
  return _plotMarkers.remove(xValue);
}
*/

const KstMarkerList Kst2DPlot::plotMarkers(const double minX, const double maxX) {
  updateMarkersFromCurve();
  KstMarkerList foundMarkers;
  KstMarkerList::ConstIterator marks_iter = _plotMarkers.begin();
  while (marks_iter != _plotMarkers.end()) {
    if ((*marks_iter).value < maxX && (*marks_iter).value > minX) {
      foundMarkers.append(*marks_iter);
    }
    ++marks_iter;
  }
  return KstMarkerList(foundMarkers);
}


const KstMarkerList& Kst2DPlot::plotMarkers() {
  updateMarkersFromCurve();
  return _plotMarkers;
}


void Kst2DPlot::setPlotMarkerList(const KstMarkerList& newMarkers) {
  _plotMarkers = KstMarkerList(newMarkers);
}


bool Kst2DPlot::nextMarker(const double currentPosition, double& marker) {
  KstMarkerList::Iterator iter = _plotMarkers.begin();
  while (iter != _plotMarkers.end() && (*iter).value < currentPosition) {
    ++iter;
  }
  if (iter == _plotMarkers.end()) {
    return false;
  }
  marker = (*iter).value;

  return true;
}


bool Kst2DPlot::prevMarker(const double currentPosition, double& marker) {
  KstMarkerList::Iterator iter = _plotMarkers.begin();
  if (iter == _plotMarkers.end() || (*iter).value >= currentPosition) {
    return false;
  }
  while (iter != _plotMarkers.end() && (*iter).value < currentPosition) {
    ++iter;
  }
  --iter;
  marker = (*iter).value;
  return true;
}


void Kst2DPlot::addImage(KstImagePtr inImage, bool set_dirty) {
  _images.append(inImage);
  if (set_dirty) {
    setDirty();
    KstApp::inst()->document()->setModified();
  }
}


void Kst2DPlot::removeImage(KstImagePtr inImage, bool set_dirty) {
  _images.remove(inImage);
  if (set_dirty) {
    setDirty();
    KstApp::inst()->document()->setModified();
  }
}


void Kst2DPlot::removeAllImages(bool set_dirty) {
  _images.clear();
  if (set_dirty) {
    setDirty();
    KstApp::inst()->document()->setModified();
  }
}


bool Kst2DPlot::hasImage(KstImagePtr inImage) const {
  return _images.find(inImage) != _images.end();
}


void Kst2DPlot::setCurveToMarkers(KstBaseCurvePtr curve, bool risingDetect, bool fallingDetect) {
  _curveToMarkers = curve;
  _curveToMarkersRisingDetect = risingDetect;
  _curveToMarkersFallingDetect = fallingDetect;
}


bool Kst2DPlot::hasCurveToMarkers() const {
  return _curveToMarkers != 0L;
}


void Kst2DPlot::removeCurveToMarkers() {
  _curveToMarkers = 0L;
}


KstBaseCurvePtr Kst2DPlot::curveToMarkers() const {
  return _curveToMarkers;
}


void Kst2DPlot::updateMarkersFromCurve() {
  if (hasCurveToMarkers()) {
    int count = _curveToMarkers->sampleCount();

    if (count > 0) {
      double prevX, prevY;
      double curX, curY;

      // scan through the whole curve
      _curveToMarkers->point(0, prevX, prevY);
      for (int i = 1; i < count; i++) {
        _curveToMarkers->point(i, curX, curY);
        if (_curveToMarkersRisingDetect && prevY == 0.0 && curY > 0.0) {
          setPlotMarker(curX, true, false);
        }
        if (_curveToMarkersFallingDetect && prevY > 0.0 && curY == 0.0) {
          setPlotMarker(prevX, false, true);
        }
        prevX = curX;
        prevY = curY;
      }
    }
  }
}


bool Kst2DPlot::curveToMarkersRisingDetect() const {
  return _curveToMarkersRisingDetect;
}


bool Kst2DPlot::curveToMarkersFallingDetect() const {
  return _curveToMarkersFallingDetect;
}


void Kst2DPlot::editImage(int id) {
  KstImagePtr image = *(_images.findTag(_imageEditMap[id]));
  if (image) {
    image->showDialog();
  }
}


void Kst2DPlot::removeImage(int id) {
  KstImagePtr image = *(_images.findTag(_imageRemoveMap[id]));
  if (image) {
    removeImage(image);
    if (_menuView) {
      _menuView->paint();
    }
  }
}


void Kst2DPlot::plotImages(QPainter& p,
                           double Lx, double Hx, double Ly, double Hy,
                           double m_X, double m_Y, double b_X, double b_Y,
                           double x_max, double y_max, double x_min, double y_min) {

  for (uint i = 0; i < _images.count(); i++) {
    double x, y, width, height;
    double img_Lx_pix = 0, img_Ly_pix = 0, img_Hx_pix = 0, img_Hy_pix = 0;

    KstImagePtr image = *(_images.at(i));
    image->matrixDimensions(x, y, width, height);

    // figure out where the image will be on the plot
    if (_xLog) {
      x_min = pow(10, x_min);
      x_max = pow(10, x_max);
    }
    if (_yLog) {
      y_min = pow(10, y_min);
      y_max = pow(10, y_max);
    }

    // only draw if img is visible
    if (!(x > x_max || y > y_max || x + width < x_min || y + height < y_min)) {
      if (x < x_min) {
        img_Lx_pix = Lx;
      } else {
        if (_xLog) {
          img_Lx_pix = logX(x) * m_X + b_X;
        } else {
          img_Lx_pix = x * m_X + b_X;
        }
      }
      if (y < y_min) {
        img_Hy_pix = Hy;
      } else {
        if (_yLog) {
          img_Hy_pix = logY(y) * m_Y + b_Y;
        } else {
          img_Hy_pix = y * m_Y + b_Y;
        }
      }
      if (x + width > x_max) {
        img_Hx_pix = Hx;
      } else {
        if (_xLog) {
          img_Hx_pix = logX(x + width) * m_X + b_X;
        } else {
          img_Hx_pix = (x + width) * m_X + b_X;
        }
      }
      if (y + height > y_max) {
        img_Ly_pix = Ly;
      } else {
        if (_yLog) {
          img_Ly_pix = logY(y + height) * m_Y + b_Y;
        } else {
          img_Ly_pix = (y + height) * m_Y + b_Y;
        }
      }

      // color map
      if (image->hasColorMap()) {
        QImage tempImage(d2i(img_Hx_pix-img_Lx_pix),
            d2i(img_Hy_pix-img_Ly_pix - 1), 32);
        for (int i=0; i < d2i(img_Hx_pix-img_Lx_pix); i++) {
          for (int j = 0; j < d2i(img_Hy_pix-img_Ly_pix - 1); j++) {
            double new_x, new_y;
            if (_xLog) {
              new_x = pow(10, (i + img_Lx_pix - b_X) / m_X);
            } else {
              new_x = (i + img_Lx_pix - b_X) / m_X;
            }
            if (_yLog) {
              new_y = pow(10, (j + 1 + img_Ly_pix - b_Y) / m_Y);
            } else {
              new_y = (j + 1 + img_Ly_pix - b_Y) / m_Y;
            }
            QRgb rgb = image->getMappedColor(new_x, new_y);
            tempImage.setPixel(i,j, rgb);
          }
        }
        p.drawImage(d2i(img_Lx_pix), d2i(img_Ly_pix + 1), tempImage);
      }
      //*******************************************************************
      //CONTOURS
      //*******************************************************************
      #ifndef CONTOUR_STEP
      #define CONTOUR_STEP 5
      #endif
      //draw the contourmap
      if (image->hasContourMap()) {
        QColor tempColor = image->contourColor();
        bool variableWeight = (image->contourWeight() < 0);
        if (!variableWeight) {
          // + 1 because 0 and 1 are the same width
          p.setPen(QPen(tempColor, image->contourWeight() + 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
        }
        // do the drawing for each contour line
        QValueList<double> lines = image->contourLines();
        QPoint lastPoint; //used to remember the previous point
        bool hasPrevBottom = false;
        for (uint k = 0; k < lines.count(); k++) {
          if (variableWeight) {
            // + 1 because 0 and 1 are the same width
            p.setPen(QPen(tempColor, k+1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
          }
          for (int i = d2i(ceil(img_Lx_pix)); i + CONTOUR_STEP < d2i(floor(img_Hx_pix)); i += CONTOUR_STEP) {
            for (int j = d2i(ceil(img_Ly_pix)); j + CONTOUR_STEP < d2i(floor(img_Hy_pix)); j += CONTOUR_STEP) {
              //look at this group of 4 pixels and get the z values
              double zTL, zTR, zBL, zBR;
              double new_x_small = (i - b_X) / m_X, new_y_small = (j +1  - b_Y) / m_Y;
              double new_x_large = (i+CONTOUR_STEP - b_X) / m_X, new_y_large = (j+1+CONTOUR_STEP - b_Y) / m_Y;

              if (_xLog) {
                new_x_small = pow(10, new_x_small);
                new_x_large = pow(10, new_x_large);
              }
              if (_yLog) {
                new_y_small = pow(10, new_y_small);
                new_y_large = pow(10, new_y_large);
              }

              image->getNearestZ(new_x_small, new_y_small, zTL);
              image->getNearestZ(new_x_large, new_y_small, zTR);
              image->getNearestZ(new_x_small, new_y_large, zBL);
              image->getNearestZ(new_x_large, new_y_large, zBR);

              // determine the lines to draw
              int numPoints = 0;
              bool passTop = false, passBottom = false, passLeft = false, passRight = false;
              QPoint topPoint, bottomPoint, leftPoint, rightPoint;

              // passes through the top
              if (hasPrevBottom) {
                topPoint = lastPoint;
                numPoints++;
                passTop = true;
              } else if (j==d2i(ceil(img_Ly_pix)) && ((lines[k] < zTR && lines[k] > zTL) || (lines[k] < zTL && lines[k] > zTR))) {
                numPoints++;
                passTop = true;
                topPoint.setX((int)(((lines[k] - zTL)*CONTOUR_STEP + (zTR - zTL)*i) / (zTR - zTL)));
                topPoint.setY(j);
              }
              hasPrevBottom = false;

              // passes through the bottom
              if ((lines[k] < zBR && lines[k] > zBL) || (lines[k] < zBL && lines[k] > zBR)) {
                numPoints++;
                passBottom = true;
                bottomPoint.setX((int)(((lines[k] - zBL)*CONTOUR_STEP + (zBR - zBL)*i) / (zBR - zBL)));
                bottomPoint.setY(j+CONTOUR_STEP);
                if (j + 2*CONTOUR_STEP < d2i(floor(img_Hy_pix))) {
                  lastPoint = bottomPoint;
                  hasPrevBottom = true;
                }
              }

              // passes through the left
              if ((lines[k] < zBL && lines[k] > zTL) || (lines[k] < zTL && lines[k] > zBL)) {
                numPoints++;
                passLeft = true;
                leftPoint.setY((int)(((lines[k] - zTL)*CONTOUR_STEP + (zBL - zTL)*j) / (zBL - zTL)));
                leftPoint.setX(i);
              }

              // passes through the right
              if ((lines[k] < zBR && lines[k] > zTR) || (lines[k] < zTR && lines[k] > zBR)) {
                numPoints++;
                passRight = true;
                rightPoint.setY((int)(((lines[k] - zTR)*CONTOUR_STEP + (zBR - zTR)*j) / (zBR - zTR)));
                rightPoint.setX(i+CONTOUR_STEP);
              }

              if (numPoints == 4) {
                // draw a cross
                p.drawLine(topPoint, bottomPoint);
                p.drawLine(rightPoint, leftPoint);
              } else if (numPoints == 3) {
                // draw a V opening to non-intersecting side
                if (!passTop) {
                  p.drawLine(leftPoint, bottomPoint);
                  p.drawLine(bottomPoint, rightPoint);
                } else if (!passLeft) {
                  p.drawLine(topPoint, rightPoint);
                  p.drawLine(rightPoint, bottomPoint);
                } else if (!passBottom) {
                  p.drawLine(leftPoint, topPoint);
                  p.drawLine(topPoint, rightPoint);
                } else {
                  p.drawLine(topPoint, leftPoint);
                  p.drawLine(leftPoint, bottomPoint);
                }
              } else if (numPoints == 2) {
                // two points - connect them
                QPoint point1, point2;
                bool true1 = false;

                if (passTop) {
                  point1 = topPoint;
                  true1 = true;
                }
                if (passBottom) {
                  if (true1) {
                    point2 = bottomPoint;
                  } else {
                    point1 = bottomPoint;
                    true1 = true;
                  }
                }
                if (passLeft) {
                  if (true1) {
                    point2 = leftPoint;
                  } else {
                    point1 = leftPoint;
                    true1 = true;
                  }
                }
                if (passRight) {
                  point2 = rightPoint;
                }
                p.drawLine(point1,point2);
              }
            }
          }
        }
      }
    }
  }
}

void Kst2DPlot::plotCurves(QPainter& p,
                           double Lx, double Hx, double Ly, double Hy,
                           double m_X, double m_Y, double b_X, double b_Y, int penWidth) {
  KstBaseCurvePtr c;
  QRegion clipRegion;
  double maxY = 0.0, minY = 0.0;
  double rX, rY, rEX, rEY;
  double X1 = 0.0, Y1 = 0.0;
  double X2 = 0.0, Y2 = 0.0;
  double last_x1, last_y1;
  bool overlap;
  bool clipping;
  int i_pt;

  clipRegion = p.clipRegion();
  clipping = p.hasClipping();
  p.setClipRect((int)Lx, (int)Ly, (int)(Hx-Lx), (int)(Hy-Ly), QPainter::CoordPainter);

#ifdef BENCHMARK
  QTime bench_time, benchtmp;
  int b_1 = 0, b_2 = 0, b_3 = 0, b_4 = 0;
#endif
  for (int i_curve = 0; i_curve < (int)Curves.count(); i_curve++) {
#ifdef BENCHMARK
    bench_time.start();
#endif
    c = Curves[i_curve];
    c->readLock();
    overlap = false;

#ifdef BENCHMARK
    benchtmp.start();
#endif
    if (c->sampleCount() > 0) {
      Qt::PenStyle style = KstLineStyle[c->lineStyle()];
      int i0, iN;
      int width = c->lineWidth() < penWidth ? penWidth : c->lineWidth();
      
      if (c->xIsRising()) {
        i0 = c->getIndexNearX(XMin);
        if (i0 > 0) {
          i0--;
        }
        iN = c->getIndexNearX(XMax);
        if (iN < c->sampleCount() - 1) {
          iN++;
        }
      } else {
        i0 = 0;
        iN = c->sampleCount() - 1;
      }

#ifdef BENCHMARK
      clock_t linesStart = clock();
#endif
      if (c->hasLines()) {
        QPointArray points(MAX_NUM_POLYLINES);
        double lineWidth;
        bool foundNan;
        int lastPlottedX = 0;
        int lastPlottedY = 0;
        int index = 0;
        int i0Start = i0;

        lineWidth = (double)width;
        p.setPen(QPen(c->color(), width, style));

// Optimize - isnan seems expensive, at least in gcc debug mode
//            cachegrind backs this up.
#undef isnan
#define isnan(x) (x != x)
        c->point(i0, rX, rY);

        // if invalid point then look backward for the last valid point.
        while ((isnan(rX) || isnan(rY)) && i0 > 0) {
          c->point(--i0, rX, rY);
        }

        // if invalid point then look forward for the next valid point...
        if (isnan(rX) || isnan(rY)) {
          i0 = i0Start;
          while ((isnan(rX) || isnan(rY)) && i0 < iN) {
            c->point(++i0, rX, rY);
          }
        }

        if (_xLog) {
          rX = logX(rX);
        }
        if (_yLog) {
          rY = logY(rY);
        }
        last_x1 = m_X*rX + b_X;
        last_y1 = m_Y*rY + b_Y;

        i_pt = i0;
        while (i_pt < iN) {
          X2 = last_x1;
          Y2 = last_y1;

          c->point(++i_pt, rX, rY);
          foundNan = false;

          // if necessary continue looking for the first valid point...
          while ((isnan(rX) || isnan(rY)) && i_pt < iN) {
#undef isnan
            foundNan = true;
            c->point(++i_pt, rX, rY);
          }

          if (i_pt <= iN) {
            if (_xLog) {
              rX = logX(rX);
            }
            if (_yLog) {
              rY = logY(rY);
            }
            X1 = m_X*rX + b_X;
            Y1 = m_Y*rY + b_Y;
            last_x1 = X1;
            last_y1 = Y1;

            if (!foundNan) {
              if (d2i(X1) == d2i(X2)) {
                if (overlap) {
                  if (Y1 > maxY) {
                    maxY = Y1;
                  }
                  if (Y1 < minY) {
                    minY = Y1;
                  }
                } else {
                  if (Y1 < Y2) {
                    minY = Y1;
                    maxY = Y2;
                  } else {
                    maxY = Y1;
                    minY = Y2;
                  }
                  overlap = true;
                }
              } else {
                if (overlap) {
                  if (X2 >= Lx && X2 <= Hx) {
                    if (maxY <= Hy && minY >= Ly) {
                      if (style == Qt::SolidLine) {
                        minY -= lineWidth/1.99999;
                        maxY += lineWidth/1.99999;
                        p.drawLine(d2i(X2), d2i(minY), d2i(X2), d2i(maxY));
                      } else {
                        int X2i   = d2i(X2);
                        int Y2i   = d2i(Y2);
                        int maxYi = d2i(maxY);
                        int minYi = d2i(minY);

                        if (index >= MAX_NUM_POLYLINES-2) {
                          p.drawPolyline(points, 0, index);
                          index = 0;
                        }
                        if (minYi == maxYi) {
                          points.setPoint(index++, X2i, maxYi);
                        } else if (Y2 == minY) {
                          points.setPoint(index++, X2i, maxYi);
                          points.setPoint(index++, X2i, minYi);
                        } else if (Y2 == maxY) {
                          points.setPoint(index++, X2i, minYi);
                          points.setPoint(index++, X2i, maxYi);
                        } else {
                          points.setPoint(index++, X2i, minYi);
                          points.setPoint(index++, X2i, maxYi);
                          if (Y2 >= Ly && Y2 <= Hy) {
                            points.setPoint(index++, X2i, Y2i);
                          }
                        }
                        lastPlottedX = X2i;
                        lastPlottedY = Y2i;
                      }
                    } else {
                      if (maxY > Hy && minY <= Hy) {
                        maxY = Hy;
                      }
                      if (minY < Ly && maxY >= Ly) {
                        minY = Ly;
                      }
                      if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
                        p.drawLine(d2i(X2), d2i(minY), d2i(X2), d2i(maxY));
                      }
                    }
                  }
                  overlap = false;
                }

                if (!((X1 < Lx && X2 < Lx) || (X1 > Hx && X2 > Hx))) {
                  // trim the line to be within the plot...
                  if (isinf(X1)) {
                    Y1 = Y2;
                    if (X1 > 0.0) {
                      X1 = Hx;
                    } else {
                      X1 = Lx;
                    }
                  }

                  if (isinf(X2)) {
                    Y2 = Y1;
                    if (X2 > 0.0) {
                      X2 = Hx;
                    } else {
                      X2 = Lx;
                    }
                  }

                  if (isinf(Y1)) {
                    X1 = X2;
                    if (Y1 > 0.0) {
                      Y1 = Hy;
                    } else {
                      Y1 = Ly;
                    }
                  }

                  if (isinf(Y2)) {
                    X2 = X1;
                    if (Y2 > 0.0) {
                      Y2 = Hy;
                    } else {
                      Y2 = Ly;
                    }
                  }

                  if (X1 < Lx && X2 > Lx) {
                    Y1 = (Y2 - Y1) / (X2 - X1) * (Lx - X1) + Y1;
                    X1 = Lx;
                  } else if (X2 < Lx && X1 > Lx) {
                    Y2 = (Y1 - Y2) / (X1 - X2) * (Lx - X2) + Y2;
                    X2 = Lx;
                  }

                  if (X1 < Hx && X2 > Hx) {
                    Y2 = (Y2 - Y1) / (X2 - X1) * (Hx - X1) + Y1;
                    X2 = Hx;
                  } else if (X2 < Hx && X1 > Hx) {
                    Y1 = (Y1 - Y2) / (X1 - X2) * (Hx - X2) + Y2;
                    X1 = Hx;
                  }

                  if (Y1 < Ly && Y2 > Ly) {
                    X1 = (X2 - X1) / (Y2 - Y1) * (Ly - Y1) + X1;
                    Y1 = Ly;
                  } else if (Y2 < Ly && Y1 > Ly) {
                    X2 = (X1 - X2) / (Y1 - Y2) * (Ly - Y2) + X2;
                    Y2 = Ly;
                  }

                  if (Y1 < Hy && Y2 > Hy) {
                    X2 = (X2 - X1) / (Y2 - Y1) * (Hy - Y1) + X1;
                    Y2 = Hy;
                  } else if (Y2 < Hy && Y1 > Hy) {
                    X1 = (X1 - X2) / (Y1 - Y2) * (Hy - Y2) + X2;
                    Y1 = Hy;
                  }

                  if (X1 >= Lx && X1 <= Hx && X2 >= Lx && X2 <= Hx &&
                      Y1 >= Ly && Y1 <= Hy && Y2 >= Ly && Y2 <= Hy) {
                    if (style == Qt::SolidLine) {
                      p.drawLine(d2i(X1), d2i(Y1), d2i(X2), d2i(Y2));
                    } else {
                      int X1i = d2i(X1);
                      int Y1i = d2i(Y1);
                      int X2i = d2i(X2);
                      int Y2i = d2i(Y2);

                      if (index == 0) {
                        points.setPoint(index++, X2i, Y2i);
                        points.setPoint(index++, X1i, Y1i);
                      } else if (lastPlottedX == X2i &&
                          lastPlottedY == Y2i &&
                          index < MAX_NUM_POLYLINES) {
                        points.setPoint(index++, X1i, Y1i);
                      } else {
                        if (index > 1) {
                          p.drawPolyline(points, 0, index);
                        }
                        index = 0;
                        points.setPoint(index++, X2i, Y2i);
                        points.setPoint(index++, X1i, Y1i);
                      }
                      lastPlottedX = X1i;
                      lastPlottedY = Y1i;
                    }
                  }
                }
              } // end if (X1 == X2)
            } // end if (!foundNan)
          } // end if (i_pt <= iN)
        } // end while

        // we might a have polyline left undrawn...
        if (index > 1) {
          p.drawPolyline(points, 0, index);
          index = 0;
        }

        // we might have some overlapping points still unplotted...
        if (overlap) {
          if (X2 >= Lx && X2 <= Hx) {
            if (maxY > Hy && minY <= Hy)
              maxY = Hy;
            if (minY < Ly && maxY >= Ly)
              minY = Ly;
            if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
              p.drawLine(d2i(X2), d2i(minY), d2i(X2), d2i(maxY));
            }
          }
          overlap = false;
        }
      } // end if c->hasLines()
#ifdef BENCHMARK
      clock_t linesEnd = clock();
      kstdDebug() << "        Lines clocks: " << (linesEnd - linesStart) << endl;
      b_1 = benchtmp.elapsed();
#endif

      // draw the bargraph bars, if any...
      if (c->hasBars()) {
        bool has_top = true;
        bool has_bot = true;
        bool has_left = true;
        bool has_right = true;
        bool visible = true;
        double rX2 = 0.0;
        double drX;

        if (c->barStyle() == 1) { // filled
          p.setPen(QPen(_foregroundColor, width, style));
        } else {
          p.setPen(QPen(c->color(), width, style));
        }

        drX = (c->maxX() - c->minX())/double(c->sampleCount());
        for (i_pt = i0; i_pt <= iN; i_pt++) {
          visible = has_bot = has_top = has_left = has_right = true;

          // determine the bar position width.
          // NOTE: this assumes even X spacing if XError is not defined
          // There may a better way...
          if (c->hasXError()) {
            c->getEXPoint(i_pt, rX, rY, drX);
          } else {
            c->point(i_pt, rX, rY);
          }
          rX -= drX/2.0;
          rX2 = rX + drX;
          if (_xLog) {
            rX = logX(rX);
            rX2 = logX(rX2);
          }
          if (_yLog) {
            rY = logY(rY);
          }

          X1 = m_X * rX + b_X;
          X2 = m_X * rX2 + b_X;
          if (X1 > Hx || X2 < Lx) {
            visible = false;
          } else {
            if (X1 < Lx) {
              has_left = false;
              X1 = Lx;
            }
            if (X2 > Hx) {
              has_right = false;
              X2 = Hx;
            }
          }

          // determine where the top of the bar is and whether
          // to draw the top line
          Y1 = m_Y * rY + b_Y;
          if (Y1 < Ly) {
            Y1 = Ly;
            has_top = false;
          }
          if (Y1 > Hy) {
            Y1 = Hy;
            has_top = false;
          }

          // determine where the bottom of the bar is and whether
          // to draw the bottom line
          if (_yLog) {
            Y2 = Hy;
            has_bot = false;
          } else {
            Y2 = b_Y;
            if (Y2 < Ly) {
              Y2 = Ly;
              has_bot = false;
            }
            if (Y2 > Hy) {
              Y2 = Hy;
              has_bot = false;
            }
          }

          if (Y1==Ly && Y2==Ly) {
            visible = false;
          }
          else if (Y1==Hy && Y2==Hy) {
            visible = false;
          }

          if (visible) {
            if (c->barStyle() == 1) { // filled
              p.fillRect(d2i(X1), d2i(Y1), d2i(X2)-d2i(X1), d2i(Y2)-d2i(Y1), c->color());
            }
            if (has_top) {
              p.drawLine(d2i(X1-(width/2)), d2i(Y1), d2i(X2+(width/2)), d2i(Y1));
            }
            if (has_bot) {
              p.drawLine(d2i(X1-(width/2)), d2i(Y2), d2i(X2-(width/2)), d2i(Y2));
            }
            if (has_left) {
              p.drawLine(d2i(X1), d2i(Y1-(width/2)), d2i(X1), d2i(Y2+(width/2)));
            }
            if (has_right) {
              p.drawLine(d2i(X2), d2i(Y1-(width/2)), d2i(X2), d2i(Y2+(width/2)));
            }
          }
        }
      }

#ifdef BENCHMARK
      b_2 = benchtmp.elapsed();
#endif

      p.setPen(QPen(c->color(), width));

      // draw the points, if any...
      if (c->hasPoints()) {
        if (c->hasLines() && c->pointDensity() != 0) {
          QRegion rgn((int)Lx, (int)Ly, (int)(Hx-Lx), (int)(Hy-Ly), QRegion::Rectangle);
          QPoint  point;
          int     size;

          size = (int)pow(3.0, KSTPOINTDENSITY_MAXTYPE - c->pointDensity());
          if (Hx-Lx > Hy-Ly) {
            size = (int)(Hx-Lx) / size;
          } else {
            size = (int)(Hy-Ly) / size;
          }
          for (i_pt = i0; i_pt <= iN; i_pt++) {
            c->point(i_pt, rX, rY);
            if (_xLog) {
              rX = logX(rX);
            }
            if (_yLog) {
              rY = logY(rY);
            }

            point.setX( d2i(m_X * rX + b_X) );
            point.setY( d2i(m_Y * rY + b_Y) );
            if (rgn.contains(point)) {
              c->Point.draw(&p, point.x(), point.y(), c->lineWidth());
              rgn -= QRegion(point.x()-(size/2), point.y()-(size/2), size, size, QRegion::Ellipse);
            }
          }
        } else {
          for (i_pt = i0; i_pt <= iN; i_pt++) {
            c->point(i_pt, rX, rY);
            if (_xLog) {
              rX = logX(rX);
            }
            if (_yLog) {
              rY = logY(rY);
            }

            X1 = m_X * rX + b_X;
            Y1 = m_Y * rY + b_Y;
            if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y1 <= Hy) {
              c->Point.draw(&p, d2i(X1), d2i(Y1), c->lineWidth());
            }
          }
        }
      }
#ifdef BENCHMARK
    b_3 = benchtmp.elapsed();
#endif

      // draw the x-errors, if any...
      if ((c->hasXError() || c->hasXMinusError()) && !c->hasBars()) {
        double rX1;
        double rX2;
        double rEXLo, rEXHi;
        bool do_low_flag = true;
        bool do_high_flag = true;
        bool errorPlus = c->hasXError();
        bool errorMinus = c->hasXMinusError();
        bool errorSame = false;

        if (errorPlus && errorMinus && c->xETag() == c->xEMinusTag()) {
          errorSame = true;
        }

        for (i_pt = i0; i_pt <= iN; i_pt++) {
          do_low_flag = true;
          do_high_flag = true;

          if (errorSame) {
            c->getEXPoint(i_pt, rX, rY, rEX);
            if (_xLog) {
              rX1 = logX(rX-fabs(rEX));
              rX2 = logX(rX+fabs(rEX));
            } else {
              rX1 = rX-fabs(rEX);
              rX2 = rX+fabs(rEX);
            }
          } else if (errorPlus && errorMinus) {
            c->getEXPoints(i_pt, rX, rY, rEXLo, rEXHi);
            if (_xLog) {
              rX1 = logX(rX-fabs(rEXLo));
              rX2 = logX(rX+fabs(rEXHi));
            } else {
              rX1 = rX-fabs(rEXLo);
              rX2 = rX+fabs(rEXHi);
            }          
          } else if (errorPlus) {
            c->getEXPoint(i_pt, rX, rY, rEX);
            if (_xLog) {
              rX1 = logX(rX);
              rX2 = logX(rX+fabs(rEX));
            } else {
              rX1 = rX;
              rX2 = rX+fabs(rEX);
            }
            do_low_flag = false;
          } else {
            c->getEXMinusPoint(i_pt, rX, rY, rEX);
            if (_xLog) {
              rX1 = logX(rX-fabs(rEX));
              rX2 = logX(rX);
            } else {
              rX1 = rX-fabs(rEX);
              rX2 = rX;
            }
            do_high_flag = false;
          }

          if (_yLog) {
            rY = logY(rY);
          }

          X1 = m_X * rX1 + b_X;
          X2 = m_X * rX2 + b_X;
          Y1 = m_Y * rY + b_Y;

          if (X1 < Lx && X2 > Lx) {
            X1 = Lx;
            do_low_flag = false;
          }
          if (X1 < Hx && X2 > Hx) {
            X2 = Hx;
            do_high_flag = false;
          }

          if (X1 >= Lx && X2 <= Hx && Y1 >= Ly && Y1 <= Hy) {
            p.drawLine(d2i(X1), d2i(Y1), d2i(X2), d2i(Y1));
            if (do_low_flag) {
              p.drawLine(d2i(X1), d2i(Y1) + c->Point.dim(&p),
                  d2i(X1), d2i(Y1) - c->Point.dim(&p));
            }
            if (do_high_flag) {
              p.drawLine(d2i(X2), d2i(Y1) + c->Point.dim(&p),
                  d2i(X2), d2i(Y1) - c->Point.dim(&p));
            }
          }
        }
      }

      // draw the y-errors, if any...
      if (c->hasYError() || c->hasYMinusError()) {
        double rY1;
        double rY2;
        double rEYLo, rEYHi;
        bool do_low_flag = true;
        bool do_high_flag = true;
        bool errorPlus = c->hasYError();
        bool errorMinus = c->hasYMinusError();
        bool errorSame = false;

        if (errorPlus && errorMinus && c->yETag() == c->yEMinusTag()) {
          errorSame = true;
        }

        for (i_pt = i0; i_pt <= iN; i_pt++) {
          do_low_flag = true;
          do_high_flag = true;

          if (errorSame) {
            c->getEYPoint(i_pt, rX, rY, rEY);
            if (_yLog) {
              rY1 = logY(rY-fabs(rEY));
              rY2 = logY(rY+fabs(rEY));
            } else {
              rY1 = rY-fabs(rEY);
              rY2 = rY+fabs(rEY);
            }
          } else if (errorPlus && errorMinus) {
            c->getEYPoints(i_pt, rX, rY, rEYLo, rEYHi);
            if (_yLog) {
              rY1 = logY(rY-fabs(rEYLo));
              rY2 = logY(rY+fabs(rEYHi));
            } else {
              rY1 = rY-fabs(rEYLo);
              rY2 = rY+fabs(rEYHi);
            }
          } else if (errorPlus) {
            c->getEYPoint(i_pt, rX, rY, rEY);
            if (_yLog) {
              rY1 = logY(rY);
              rY2 = logY(rY+fabs(rEY));
            } else {
              rY1 = rY;
              rY2 = rY+fabs(rEY);
            }
            do_low_flag = false;
          } else {
            c->getEYMinusPoint(i_pt, rX, rY, rEY);
            if (_yLog) {
              rY1 = logY(rY-fabs(rEY));
              rY2 = logY(rY);
            } else {
              rY1 = rY-fabs(rEY);
              rY2 = rY;
            }
            do_high_flag = false;
          }

          if (_xLog) {
            rX = logX(rX);
          }

          X1 = m_X * rX + b_X;
          Y1 = m_Y * rY1 + b_Y;
          Y2 = m_Y * rY2 + b_Y;

          if (Y1 < Ly && Y2 > Ly) {
            Y1 = Ly;
            do_low_flag = false;
          }
          if (Y1 < Hy && Y2 > Hy) {
            Y2 = Hy;
            do_high_flag = false;
          }

          if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y2 <= Hy) {
            p.drawLine(d2i(X1), d2i(Y1), d2i(X1), d2i(Y2));
            if (do_low_flag) {
              p.drawLine(d2i(X1) + c->Point.dim(&p), d2i(Y1),
                  d2i(X1) - c->Point.dim(&p), d2i(Y1));
            }
            if (do_high_flag) {
              p.drawLine(d2i(X1) + c->Point.dim(&p), d2i(Y2),
                  d2i(X1) - c->Point.dim(&p), d2i(Y2));
            }
          }
        }
      } // end if (c->hasYError())
    } // end if (c->sampleCount() > 0)
#ifdef BENCHMARK
    b_4 = benchtmp.elapsed();
#endif
    c->readUnlock();
#ifdef BENCHMARK
    int i = bench_time.elapsed();
                       kstdDebug() << "Plotting curve " << i_curve << ": " << i << "ms" << endl;
                       kstdDebug() << "    Without locks: " << b_4 << "ms" << endl;
    if (b_1 > 0)       kstdDebug() << "            Lines: " << b_1 << "ms" << endl;
    if (b_2 - b_1 > 0) kstdDebug() << "             Bars: " << (b_2 - b_1) << "ms" << endl;
    if (b_3 - b_2 > 0) kstdDebug() << "           Points: " << (b_3 - b_2) << "ms" << endl;
    if (b_4 - b_3 > 0) kstdDebug() << "           Errors: " << (b_4 - b_3) << "ms" << endl;
#endif
  } // next curve

  p.setClipRegion(clipRegion);
  p.setClipping(clipping);
}


void Kst2DPlot::plotPlotMarkers(QPainter& p,
                           double m_X, double b_X, double x_max, double x_min,
                           double y_px, double ytop_bdr_px, double ybot_bdr_px) {
  p.setPen(QPen(foregroundColor(), 0, Qt::DashLine));

  KstMarkerList marks;
  if (_xLog) {
    marks = plotMarkers(pow(10,x_min), pow(10,x_max));
  } else {
    marks = plotMarkers(x_min, x_max);
  }

  // plot each one
  KstMarkerList::iterator marks_iter = marks.begin();
  double mark_px;
  if (_xLog) {
    double new_x;

    while (marks_iter != marks.end()) {
      new_x = logX((*marks_iter).value);
      if (new_x <= x_max && new_x >= x_min) {
        mark_px = m_X * new_x + b_X;
        p.drawLine(d2i(mark_px),
                   d2i(ytop_bdr_px),
                   d2i(mark_px),
                   d2i(y_px - ybot_bdr_px));
      }
      ++marks_iter;
    }
  } else {
    while (marks_iter != marks.end()) {
      mark_px = m_X * (*marks_iter).value + b_X;
      p.drawLine(d2i(mark_px),
                 d2i(ytop_bdr_px),
                 d2i(mark_px),
                 d2i(y_px - ybot_bdr_px));
      ++marks_iter;
    }
  }
}


void Kst2DPlot::plotAxes(QPainter& p, QRect& plotRegion,
                         TickParameters tpx,
                         double xleft_bdr_px, double xright_bdr_px,
                         double x_orig_px, double xtick_px,
                         double xtick_len_px, int x_px,
                         TickParameters tpy,
                         double ytop_bdr_px, double ybot_bdr_px,
                         double y_orig_px, double ytick_px,
                         double ytick_len_px, int y_px,
                         bool offsetX, bool offsetY) {
  QString TmpStr, TmpStrOld;
  double X1, Y1;
  double X2, Y2;
  int i, j;

  // draw axis
  p.drawRect(plotRegion);

  // draw x-ticks
  if (_xLog) {
    i = (int)floor((double)_xMajorTicks*(xleft_bdr_px - 1.0 - x_orig_px)/xtick_px);
    for (;xtick_px * i + x_orig_px < x_px - xright_bdr_px + 1; i++) {
      // draw major ticks
      X1 = (x_orig_px + double(i) * xtick_px);
      if (X1 > xleft_bdr_px && X1 < x_px - xright_bdr_px) {
        p.drawLine(d2i(X1),
                   d2i(ytop_bdr_px),
                   d2i(X1),
                   d2i(ytop_bdr_px + 2.0 * xtick_len_px));

        p.drawLine(d2i(X1),
                   d2i(y_px - ybot_bdr_px),
                   d2i(X1),
                   d2i(y_px - ybot_bdr_px - 2.0 * xtick_len_px));
      }

      // draw minor ticks
      for (j = 1; j < _xMinorTicks; j++) {
        X2 = log10((double)j/((double)_xMinorTicks)*(pow(10,tpx.tick)-1.0)+1.0)/
             tpx.tick * (double)xtick_px + X1;
        if (X2 > xleft_bdr_px && X2 < x_px - xright_bdr_px) {
          p.drawLine(d2i(X2),
                    d2i(ytop_bdr_px),
                    d2i(X2),
                    d2i(ytop_bdr_px + xtick_len_px));

          p.drawLine(d2i(X2),
                    d2i(y_px - ybot_bdr_px),
                    d2i(X2),
                    d2i(y_px - ybot_bdr_px - xtick_len_px));
        }
      }
    }
  } else {
    i = (int)ceil( (double)_xMinorTicks * ( xleft_bdr_px - 1.0 - x_orig_px ) / xtick_px );
    for (; xtick_px * i / _xMinorTicks + x_orig_px < x_px - xright_bdr_px ; i++) {
      X1 = x_orig_px + (double)i * xtick_px / (double)_xMinorTicks;
      if (i % _xMinorTicks == 0) {
        p.drawLine(d2i(X1),
                   d2i(ytop_bdr_px),
                   d2i(X1),
                   d2i(ytop_bdr_px + 2.0 * xtick_len_px));

        p.drawLine(d2i(X1),
                   d2i(y_px - ybot_bdr_px),
                   d2i(X1),
                   d2i(y_px - ybot_bdr_px - 2 * xtick_len_px));
      } else {
        p.drawLine(d2i(X1),
                   d2i(ytop_bdr_px),
                   d2i(X1),
                   d2i(ytop_bdr_px + xtick_len_px));
        p.drawLine(d2i(X1),
                   d2i(y_px - ybot_bdr_px),
                   d2i(X1),
                   d2i(y_px - ybot_bdr_px - xtick_len_px));
      }
    }
  }

  // draw y ticks
  if (_yLog) {
    i = (int)floor( (double)_yMajorTicks * ( ytop_bdr_px - 1.0 - y_orig_px ) / ytick_px );
    for (; ytick_px * i + y_orig_px < y_px - ybot_bdr_px + 1; i++) {
      // draw major ticks
      Y1 = y_orig_px + (double)i * ytick_px;
      if (Y1 > ytop_bdr_px) {
        p.drawLine(d2i(xleft_bdr_px),
                   d2i(Y1),
                   d2i(xleft_bdr_px + 2.0 * ytick_len_px - 1.0),
                   d2i(Y1));
        p.drawLine(d2i(x_px - xright_bdr_px),
                   d2i(Y1),
                   d2i(x_px - xright_bdr_px - 2.0 * ytick_len_px - 1.0),
                   d2i(Y1));
      }

      // draw minor ticks
      for (j = 1; j < _yMinorTicks; j++) {
        Y2 = (1.0 - log10((double)j/((double)_yMinorTicks)*(pow(10,tpy.tick)-1.0)+1.0)/tpy.tick) * (double)ytick_px + Y1;
        if (Y2 > ytop_bdr_px && Y2 < y_px - ybot_bdr_px) {
          p.drawLine(d2i(xleft_bdr_px),
                    d2i(Y2),
                    d2i(xleft_bdr_px + ytick_len_px),
                    d2i(Y2));
          p.drawLine(d2i(x_px - xright_bdr_px),
                    d2i(Y2),
                    d2i(x_px - xright_bdr_px - ytick_len_px),
                    d2i(Y2));
        }
      }
    }
  } else {
    i = (int)ceil( (double)_yMinorTicks * ( ytop_bdr_px - 1.0 - y_orig_px ) / ytick_px );
    for (; ytick_px * i / _yMinorTicks + y_orig_px < y_px - ybot_bdr_px + 1; i++) {
      Y1 = y_orig_px + (double)i * ytick_px / (double)_yMinorTicks;
      if (i % _yMinorTicks == 0) {
        p.drawLine(d2i(xleft_bdr_px),
                   d2i(Y1),
                   d2i(xleft_bdr_px + 2.0 * ytick_len_px),
                   d2i(Y1));
        p.drawLine(d2i(x_px - xright_bdr_px),
                   d2i(Y1),
                   d2i(x_px - xright_bdr_px - 2.0 * ytick_len_px),
                   d2i(Y1));
      } else {
        p.drawLine(d2i(xleft_bdr_px),
                   d2i(Y1),
                   d2i(xleft_bdr_px + ytick_len_px),
                   d2i(Y1));
        p.drawLine(d2i(x_px - xright_bdr_px),
                   d2i(Y1),
                   d2i(x_px - xright_bdr_px - ytick_len_px),
                   d2i(Y1));
      }
    }
  }

  // x axis numbers
  if (tpx.delta && !tpx.labels.isEmpty()) {
    XFullTickLabel->setText(tpx.labels[0]);
    if (offsetX && !XLabel->text().isEmpty()) {
      XFullTickLabel->draw(p, d2i(xleft_bdr_px), d2i(y_px) - XLabel->lineSpacing(p));
    } else {
      XFullTickLabel->draw(p, d2i(xleft_bdr_px), d2i(y_px));
    }
    tpx.labels.pop_front();
  }
  int yTickPos = d2i(y_px - ybot_bdr_px+XTickLabel->lineSpacing(p)*0.15);
  for (i=tpx.iLo; i<tpx.iHi; i++) {
    XTickLabel->setText(tpx.labels[i-tpx.iLo]);
    if (XTickLabel->rotation() == 0) {
      XTickLabel->draw(p, d2i(x_orig_px + (double)i * xtick_px), yTickPos);
    } else if (XTickLabel->rotation() > 0) {
      XTickLabel->draw(p, d2i(x_orig_px + (double)i * xtick_px + (0.5 * XTickLabel->rotatedWidth(p))), d2i(y_px - ybot_bdr_px + 0.4*XTickLabel->rotatedHeight(p)));
    } else if (XTickLabel->rotation() < 0) {
      XTickLabel->draw(p, d2i(x_orig_px + (double)i * xtick_px - (0.5 * XTickLabel->rotatedWidth(p))), d2i(y_px - ybot_bdr_px + 0.4 * XTickLabel->rotatedHeight(p)));
    }
  }

  // y axis numbers
  if (tpy.delta && !tpy.labels.isEmpty()) {
    YFullTickLabel->setText(tpy.labels[0]);
    if (offsetY && !YLabel->text().isEmpty()) {
      YFullTickLabel->draw(p, YLabel->lineSpacing(p), d2i(y_px - ybot_bdr_px));
    } else {
      YFullTickLabel->draw(p, 0, d2i(y_px - ybot_bdr_px));    
    }
    tpy.labels.pop_front();
  }
  int xTickPos = d2i(xleft_bdr_px - YTickLabel->lineSpacing(p) / 4);
  for (i=tpy.iLo; i<tpy.iHi; i++) {
    YTickLabel->setText(tpy.labels[i-tpy.iLo]);
    if (YTickLabel->rotation() == 0) {
      YTickLabel->draw(p, xTickPos, d2i(y_orig_px - (double)i * ytick_px));
    } else if (YTickLabel->rotation() > 0) {
      YTickLabel->draw(p, xTickPos, d2i(y_orig_px - (double)i * ytick_px));
    } else if (YTickLabel->rotation() < 0) {
      YTickLabel->draw(p, xTickPos, d2i(y_orig_px - (double)i * ytick_px));
    }
  }
}


void Kst2DPlot::plotLabels(QPainter &p, int x_px, int y_px, double xleft_bdr_px, double ytop_bdr_px) {
  XLabel->setResized();
  XLabel->draw(p, x_px/2, y_px);

  YLabel->setResized();
  YLabel->draw(p, (YLabel->lineSpacing(p) - YLabel->ascent(p))/2, y_px/2);

  TopLabel->setResized();
  TopLabel->draw(p, d2i(xleft_bdr_px), d2i(0.95 * (ytop_bdr_px)));
}


void Kst2DPlot::setGridLines(bool xMajor, bool yMajor, bool xMinor, bool yMinor,
                    const QColor& majorColor, const QColor& minorColor,
                    bool majorGridColorDefault, bool minorGridColorDefault) {
  _xMajorGrid = xMajor;
  _yMajorGrid = yMajor;
  _xMinorGrid = xMinor;
  _yMinorGrid = yMinor;
  _majorGridColor = majorColor;
  _minorGridColor = minorColor;
  _majorGridColorDefault = majorGridColorDefault;
  _minorGridColorDefault = minorGridColorDefault;
}


void Kst2DPlot::plotGridLines(QPainter& p,
    double XTick, double xleft_bdr_px, double xright_bdr_px,
    double x_orig_px, double xtick_px, double xtick_len_px, int x_px,
    double YTick, double ytop_bdr_px, double ybot_bdr_px,
    double y_orig_px, double ytick_px, double ytick_len_px, int y_px) {
  QColor defaultGridColor( (_foregroundColor.red() + _backgroundColor.red()) / 2,
      (_foregroundColor.green() + _backgroundColor.green()) / 2,
      (_foregroundColor.blue() + _backgroundColor.blue()) / 2 );
  double X1, Y1;
  double X2, Y2;
  int i, j;

  // draw X grid lines
  if (_xLog) {
    i = (int)floor( (double)_xMinorTicks * ( xleft_bdr_px - 1.0 - x_orig_px ) / xtick_px );
    for (;xtick_px * i + x_orig_px < x_px - xright_bdr_px + 1; i++) {

      // draw major lines
      X1 = (x_orig_px + (double)i * xtick_px);
      if (_xMajorGrid && X1 > xleft_bdr_px && X1 < x_px - xright_bdr_px) {
        if (_majorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_majorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(X1),
                  d2i(ytop_bdr_px + 2.0 * xtick_len_px),
                  d2i(X1),
                  d2i(y_px - ybot_bdr_px - 2.0 * xtick_len_px));
      }

      // draw minor lines
      for (j = 1; j < _xMinorTicks; j++) {
        X2 = log10((double)j/((double)_xMinorTicks)*(pow(10,XTick)-1.0)+1.0)/XTick * (double)xtick_px + X1;
        if (_xMinorGrid && X2 > xleft_bdr_px && X2 < x_px - xright_bdr_px) {
          if (_minorGridColorDefault) {
            p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
          } else {
            p.setPen(QPen(_minorGridColor, 0, Qt::DotLine));
          }
          p.drawLine(d2i(X2),
                    d2i(ytop_bdr_px + xtick_len_px),
                    d2i(X2),
                    d2i(y_px - ybot_bdr_px - xtick_len_px));
        }
      }
    }
  } else {
    i = (int)ceil( (double)_xMinorTicks * ( xleft_bdr_px - 1.0 - x_orig_px ) / xtick_px );
    for (; xtick_px * i / _xMinorTicks + x_orig_px < x_px - xright_bdr_px ; i++) {
      X1 = x_orig_px + (double)i * xtick_px / (double)_xMinorTicks;
      if (_xMajorGrid && i % _xMinorTicks == 0) {
        if (_majorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_majorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(X1),
                  d2i(ytop_bdr_px + 2.0 * xtick_len_px),
                  d2i(X1),
                  d2i(y_px - ybot_bdr_px - 2 * xtick_len_px));
      } else if (_xMinorGrid && i % _xMinorTicks != 0) {
        if (_minorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_minorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(X1),
                  d2i(ytop_bdr_px + xtick_len_px),
                  d2i(X1),
                  d2i(y_px - ybot_bdr_px - xtick_len_px));
      }
    }
  }

  // draw Y grid lines
  if (_yLog) {
    i = (int)floor( (double)_yMinorTicks * ( ytop_bdr_px - 1.0 - y_orig_px ) / ytick_px );
    for (; ytick_px * i + y_orig_px < y_px - ybot_bdr_px + 1; i++) {

      // draw major lines
      Y1 = y_orig_px + (double)i * ytick_px;
      if (_yMajorGrid && Y1 > ytop_bdr_px) {
        if (_majorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_majorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(xleft_bdr_px + 2.0 * ytick_len_px - 1.0),
                  d2i(Y1),
                  d2i(x_px - xright_bdr_px - 2.0 * ytick_len_px - 1.0),
                  d2i(Y1));
      }

      // draw minor lines
      for (j = 1; j < _yMinorTicks; j++) {
        Y2 = (1.0 - (log10((double)j/((double)_yMinorTicks)*(pow(10,YTick)-1.0)+1.0)/YTick)) * (double)ytick_px + Y1;
        if (_yMinorGrid && Y2 > ytop_bdr_px && Y2 < y_px - ybot_bdr_px) {
          if (_minorGridColorDefault) {
            p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
          } else {
            p.setPen(QPen(_minorGridColor, 0, Qt::DotLine));
          }
          p.drawLine(d2i(xleft_bdr_px + ytick_len_px),
                    d2i(Y2),
                    d2i(x_px - xright_bdr_px - ytick_len_px),
                    d2i(Y2));
        }
      }
    }
  } else {
    i = (int)ceil( (double)_yMinorTicks * ( ytop_bdr_px - 1.0 - y_orig_px ) / ytick_px );
    for (; ytick_px * i / _yMinorTicks + y_orig_px < y_px - ybot_bdr_px + 1; i++) {
      Y1 = y_orig_px + (double)i * ytick_px / (double)_yMinorTicks;
      if (_yMajorGrid && i % _yMinorTicks == 0) {
        if (_majorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_majorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(xleft_bdr_px + 2.0 * ytick_len_px),
                  d2i(Y1),
                  d2i(x_px - xright_bdr_px - 2.0 * ytick_len_px),
                  d2i(Y1));
      } else if (_yMinorGrid && i % _yMinorTicks != 0) {
        if (_minorGridColorDefault) {
          p.setPen(QPen(defaultGridColor, 0, Qt::DotLine));
        } else {
          p.setPen(QPen(_minorGridColor, 0, Qt::DotLine));
        }
        p.drawLine(d2i(xleft_bdr_px + ytick_len_px),
                  d2i(Y1),
                  d2i(x_px - xright_bdr_px - ytick_len_px),
                  d2i(Y1));
      }
    }
  }
}


void Kst2DPlot::setXMinorTicks(int minorTicks) {
  if (minorTicks >= 0) {
    _reqXMinorTicks = minorTicks + 1;
  } else {
    _reqXMinorTicks = -1;
  }
}


void Kst2DPlot::setYMinorTicks(int minorTicks) {
  if (minorTicks >= 0) {
    _reqYMinorTicks = minorTicks + 1;
  } else {
    _reqYMinorTicks = -1;
  }
}


void Kst2DPlot::setXMajorTicks(int majorTicks) {
  if (majorTicks > 0) {
    _xMajorTicks = majorTicks;
  } else {
    _xMajorTicks = 1;
  }
}


void Kst2DPlot::setYMajorTicks(int majorTicks) {
  if (majorTicks > 0) {
    _yMajorTicks = majorTicks;
  } else {
    _yMajorTicks = 1;
  }
}


void Kst2DPlot::pushAdjustLineWidth(int adjustment) {
  for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
    (*i)->pushLineWidth((*i)->lineWidth() + adjustment);
  }
}


void Kst2DPlot::popLineWidth() {
  for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
    (*i)->popLineWidth();
  }
}


void Kst2DPlot::pushCurveColor(const QColor& c) {
  for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
    (*i)->pushColor(c);
  }
}


void Kst2DPlot::popCurveColor() {
  for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
    (*i)->popColor();
  }
}


void Kst2DPlot::pushPlotColors() {
  _colorStack.push(_backgroundColor);
  _colorStack.push(_foregroundColor);
  _colorStack.push(_majorGridColor);
  _colorStack.push(_minorGridColor);
  _backgroundColor = Qt::white;
  _foregroundColor = Qt::black;
  _majorGridColor = Qt::black;
  _minorGridColor = Qt::black;
}


void Kst2DPlot::popPlotColors() {
  _minorGridColor = _colorStack.pop();
  _majorGridColor = _colorStack.pop();
  _foregroundColor = _colorStack.pop();
  _backgroundColor = _colorStack.pop();
}


QString Kst2DPlot::menuTitle() const {
  return i18n("Plot: %1").arg(tagName());
}


void Kst2DPlot::mouseDoubleClickEvent(QWidget *view, QMouseEvent *e) {
  Q_UNUSED(view)
  KstBaseCurvePtr curve;
  QRect pr = GetPlotRegion();
  QPoint pos = e->pos();
  int i_near_x;
  double best_distance = 1.0E300;
  double xmin, ymin, xmax, ymax;
  getLScale(xmin, ymin, xmax, ymax);

  // find mouse location in plot units
  double xpos = double(pos.x() - pr.left())/double(pr.width()) * (xmax - xmin) + xmin;
  if (isXLog()) {
    xpos = pow(10.0, xpos);
  }

  double ypos = double(pos.y() - pr.top()) / double(pr.height()) * (ymin - ymax) + ymax;

  if (isYLog()) {
    ypos = pow(10.0, ypos);
  }

  // convert 1 pixel to plot units.
  double dx_per_pix = double(pos.x()+2 - pr.left()) / double(pr.width()) * (xmax - xmin) + xmin;
  if (isXLog()) {
    dx_per_pix = pow(10.0, dx_per_pix);
  }
  dx_per_pix -= xpos;

  for (KstBaseCurveList::Iterator i = Curves.begin(); i != Curves.end(); ++i) {
    i_near_x = (*i)->getIndexNearXY(xpos, dx_per_pix, ypos);
    double near_x, near_y;
    (*i)->point(i_near_x, near_x, near_y);
    double distance;
    if (isYLog()) {
      distance = fabs(log(ypos/near_y));
    } else {
      distance = fabs(ypos - near_y);
    }
    if (distance < best_distance || curve.data() == 0L) {
      best_distance = distance;
      curve = *i;
    }
  }

  if (curve && best_distance/fabs((ymax - ymin) / pr.height()) <= 5) {
    KST::vectorList.lock().readLock();
    KstVectorPtr vp = *KST::vectorList.findTag(curve->yVTag());
    KST::vectorList.lock().readUnlock();
    KstDataObjectPtr provider;
    if (vp) {
      vp->readLock();
      provider = kst_cast<KstDataObject>(vp->provider());
      vp->readUnlock();
    }

    if (provider) {
      provider->showDialog();
    } else {
      curve->showDialog();
    }
  }

  e->accept();
}


double Kst2DPlot::timezoneHrs() {
  time_t t;
  time(&t);
  tm gmtResult;
  tm ltResult;
  tm *gmtRc = gmtime_r(&t, &gmtResult);
  tm *ltRc = localtime_r(&t, &ltResult);
  if (!gmtRc || !ltRc) {
    return 0.0;
  }
  ltRc->tm_isdst = 0;
  return difftime(mktime(gmtRc), mktime(ltRc)) / (-60.0 * 60.0);
}


#undef LABEL_PRECISION
#include "kst2dplot.moc"

// vim: ts=2 sw=2 et
