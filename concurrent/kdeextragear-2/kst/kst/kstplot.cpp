/***************************************************************************
                          kstplot.cpp: a plot with curves for kst
                             -------------------
    begin                : Fri Nov 16 2000
    copyright            : (C) 2000 by cbn
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
#include <math.h>
#include <limits.h>

#include <kdebug.h>

#include <qpainter.h>
#include <qrect.h>
#include <qcolor.h>
#include <qnamespace.h>
#include <qstringlist.h>

#include "kstplot.h"
#include "kstsettings.h"


KstPlot::KstPlot(const QString &in_tag,
                 float width_in,
                 float height_in,
                 float tlx_in, float tly_in,
                 KstScaleModeType yscale_in,
                 KstScaleModeType xscale_in,
                 double xmin_in, double ymin_in,
                 double xmax_in, double ymax_in) {
  commonConstructor(in_tag, width_in, height_in, tlx_in, tly_in,
                    yscale_in, xscale_in, xmin_in, ymin_in,
                    xmax_in, ymax_in);
}

KstPlot::KstPlot(QDomElement &e, KstBaseCurveList &in_curveList) {
  QString in_tag = "unknown";
  float width_in=1, height_in=1, tlx_in=0, tly_in=0;
  KstScaleModeType yscale_in = AUTO, xscale_in = AUTO;
  double xmin_in=0, ymin_in=0, xmax_in=1, ymax_in=1;
  QStringList ctaglist;
  KstLabel *in_toplabel=0, *in_xlabel=0, *in_ylabel=0, *in_ticklabel=0;
  KstLabel *in_alabel=0;
  KstLegend *in_legend=0;
  bool x_log=false, y_log=false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "width") {
        width_in = e.text().toDouble();
      } else if (e.tagName() == "height") {
        height_in = e.text().toDouble();
      } else if (e.tagName() == "pos_x") {
        tlx_in = e.text().toDouble();
      } else if (e.tagName() == "pos_y") {
        tly_in = e.text().toDouble();
      } else if (e.tagName() == "xscalemode") {
        xscale_in = (KstScaleModeType) e.text().toInt();
      } else if (e.tagName() == "yscalemode") {
        yscale_in = (KstScaleModeType) e.text().toInt();
      } else if (e.tagName() == "xmin") {
        xmin_in = e.text().toDouble();
      } else if (e.tagName() == "xmax") {
        xmax_in = e.text().toDouble();
      } else if (e.tagName() == "ymin") {
        ymin_in = e.text().toDouble();
      } else if (e.tagName() == "ymax") {
        ymax_in = e.text().toDouble();
      } else if (e.tagName() == "toplabel") {
        if (in_toplabel)
          delete in_toplabel;
        in_toplabel = new KstLabel(" ");
        in_toplabel->read(e);
      } else if (e.tagName() == "xlabel") {
        if (in_xlabel)
          delete in_xlabel;
        in_xlabel = new KstLabel(" ");
        in_xlabel->read(e);
      } else if (e.tagName() == "ylabel") {
        if (in_ylabel)
          delete in_ylabel;
        in_ylabel = new KstLabel(" ");
        in_ylabel->read(e);
      } else if (e.tagName() == "ticklabel") {
        if (in_ticklabel)
          delete in_ticklabel;
        in_ticklabel = new KstLabel(" ");
        in_ticklabel->read(e);
      } else if (e.tagName() == "legend") {
        if (in_legend)
          delete in_legend;
        in_legend = new KstLegend();
        in_legend->read(e);
      } else if (e.tagName() == "label") {
        in_alabel = new KstLabel(" ");
        in_alabel->read(e);
        labelList.append(in_alabel);
      } else if (e.tagName() == "curvetag") {
        ctaglist.append(e.text());
      } else if (e.tagName() == "xlog") {
        x_log = true;
      } else if (e.tagName() == "ylog") {
        y_log = true;
      }
    }
    n = n.nextSibling();
  }

  commonConstructor(in_tag, width_in, height_in, tlx_in, tly_in,
                    yscale_in, xscale_in, xmin_in, ymin_in,
                    xmax_in, ymax_in, x_log, y_log);

  for (unsigned i = 0; i < ctaglist.count(); i++) {
    KstBaseCurveList::Iterator it = in_curveList.findTag(ctaglist[i]);
    if (it != in_curveList.end()) {
      addCurve(*it);
    }
  }

  if (in_toplabel) {
    delete TopLabel;
    TopLabel = in_toplabel;
  }

  if (in_xlabel) {
    delete XLabel;
    XLabel = in_xlabel;
  }

  if (in_ylabel) {
    delete YLabel;
    YLabel = in_ylabel;
  }

  if (in_ticklabel) {
    delete TickLabel;
    TickLabel = in_ticklabel;
  }

  if (in_legend) {
    delete Legend;
    Legend = in_legend;
  }

  TickLabel->setDoScalarReplacement(false);
}

inline void KstPlot::commonConstructor(const QString &in_tag,
                                       float width_in,
                                       float height_in,
                                       float tlx_in,
                                       float tly_in,
                                       KstScaleModeType yscale_in,
                                       KstScaleModeType xscale_in,
                                       double xmin_in,
                                       double ymin_in,
                                       double xmax_in,
                                       double ymax_in,
                                       bool x_log,
                                       bool y_log) {

  XLog = x_log;
  YLog = y_log;

  Tag = in_tag;
  IsTied=false;

  Width = width_in;
  Height = height_in;
  TopLeftX = tlx_in;
  TopLeftY = tly_in;
  if (Width <= 0) Width = 0.05;
  if (Width >= 1) Width = 1.0;
  if (Height <= 0) Height = 0.05;
  if (Height >= 1) Height = 1.0;
  if (TopLeftX < 0) TopLeftX = 0;
  if (TopLeftY < 0) TopLeftY = 0;
  if (TopLeftX + Width > 1) TopLeftX = 1-Width;
  if (TopLeftY + Height > 1) TopLeftY = 1-Height;


  XMin = xmin_in;
  XMax = xmax_in;
  YMin = ymin_in;
  YMax = ymax_in;

  XScaleMode = xscale_in;
  YScaleMode = yscale_in;

  // Verify that scale limits make sense.  If not, go to auto.
  if (XMax <= XMin) { // not OK: ignore request
    XMin = 0;
    XMax = 1;
    if (XScaleMode != AUTOUP) {
      XScaleMode = AUTO;
    }
  }
  if (YMax <= YMin) {
    YMin = 0;
    YMax = 1;
    if (YScaleMode != AUTOUP) {
      YScaleMode = AUTO;
    }
  }

  // Turn on AutoDeletion
  PlotScaleList.setAutoDelete(true);
  //Push Scale onto PlotScaleList
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

  TickLabel = new KstLabel;
  TickLabel->setRotation(0);
  TickLabel->setDoScalarReplacement(false);

  Legend = new KstLegend();
  Legend->setJustification(CxBy);
  Legend->setRotation(0);

  setForegroundColor("black");
  setBackgroundColor("white");

  labelList.setAutoDelete(true);
}


KstPlot::~KstPlot() {
  delete XLabel;
  XLabel = 0L;
  delete YLabel;
  YLabel = 0L;
  delete TopLabel;
  TopLabel = 0L;
  delete TickLabel;
  TickLabel = 0L;
  delete Legend;
  Legend = 0L;
}


/** Set the dimentions of the plot.  0,0 is top left corner of view.
    The size of the view is 1,1 */
void KstPlot::setDims(float width_in, float height_in,
                      float tlx_in, float tly_in) {
  Width = width_in;
  Height = height_in;
  TopLeftX = tlx_in;
  TopLeftY = tly_in;
  // Force almost sensible limits
  if (Width <= 0) Width = 0.05;
  if (Width >= 1) Width = 1.0;
  if (Height <= 0) Height = 0.05;
  if (Height >= 1) Height = 1.0;
  if (TopLeftX < 0) TopLeftX = 0;
  if (TopLeftY < 0) TopLeftY = 0;
  if (TopLeftX + Width > 1) TopLeftX = 1 - Width;
  if (TopLeftY + Height > 1) TopLeftY = 1 - Height;

}

/** Return fractional dimentions of plot */
float KstPlot::width() const {
  return Width;
}

float KstPlot::height() const {
  return Height;
}

float KstPlot::topLeftX() const {
  return TopLeftX;
}

float KstPlot::topLeftY() const {
  return TopLeftY;
}

/*** initialize the fonts in a plot: boost size by font_size ***/
void KstPlot::initFonts(const QFont &in_font, int font_size) {
  // Still have to set symbol font info as well //
  // We also may want to change different label fonts separately. //
  int point_size;

  point_size = in_font.pointSize() + font_size;

  TopLabel->setFontName(in_font.family());
  TopLabel->setSize(point_size - 12);

  XLabel->setFontName(in_font.family());
  XLabel->setSize(point_size - 12);

  YLabel->setFontName(in_font.family());
  YLabel->setSize(point_size - 12);

  TickLabel->setFontName(in_font.family());
  TickLabel->setSize(point_size - 12);

  Legend->setFontName(in_font.family());
  Legend->setSize(point_size - 12);

}
  /* set font sizes */


void KstPlot::setTopLabel(const QString &in_label) {
  TopLabel->setText(in_label);
}

void KstPlot::setXLabel(const QString &in_label) {
  XLabel->setText(in_label);
}

void KstPlot::setYLabel(const QString &in_label) {
  YLabel->setText(in_label);
}

void KstPlot::setXScale(double xmin_in, double xmax_in) {
  if (xmax_in > xmin_in) { // only accept new range if valid
    XMax = xmax_in;
    XMin = xmin_in;
  }
}

void KstPlot::setYScale(double ymin_in, double ymax_in) {
  if (ymax_in > ymin_in) { // only accept new range if valid
    YMax = ymax_in;
    YMin = ymin_in;
  }
}

void KstPlot::setLXScale(double xmin_in, double xmax_in) {
  if (xmax_in > xmin_in) { // only accept new range if valid
    if (XLog) {
      XMax = pow(10.0, xmax_in);
      XMin = pow(10.0, xmin_in);
    } else {
      XMax = xmax_in;
      XMin = xmin_in;
    }
  }
}

void KstPlot::setLYScale(double ymin_in, double ymax_in) {
  if (ymax_in > ymin_in) { // only accept new range if valid
    if (YLog) {
      YMax = pow(10.0, ymax_in);
      YMin = pow(10.0, ymin_in);
    } else {
      YMax = ymax_in;
      YMin = ymin_in;
    }
  }
}

void KstPlot::setTagName(const QString &new_tag) {
  Tag = new_tag;
}

void KstPlot::setScale(double xmin_in, double ymin_in,
                  double xmax_in, double ymax_in) {
  setXScale(xmin_in, xmax_in);
  setYScale(ymin_in, ymax_in);
}

void KstPlot::setLScale(double xmin_in, double ymin_in,
                  double xmax_in, double ymax_in) {
  setLXScale(xmin_in, xmax_in);
  setLYScale(ymin_in, ymax_in);
}

void KstPlot::getScale(double &xmin, double &ymin,
                       double &xmax, double &ymax) const {
  xmin = XMin;
  xmax = XMax;
  ymin = YMin;
  ymax = YMax;
}

void KstPlot::getLScale(double &x_min, double &y_min,
                       double &x_max, double &y_max) const {
  if (XLog) {
    x_min = XMin > 0 ? log10(XMin) : -350;
    x_max = XMax > 0 ? log10(XMax) : -340;
  } else {
    x_max = XMax;
    x_min = XMin;
  }

  if (YLog) {
    y_min = YMin > 0 ? log10(YMin) : -350;
    y_max = YMax > 0 ? log10(YMax) : -340;
  } else {
    y_max = YMax;
    y_min = YMin;
  }
}


KstScaleModeType KstPlot::getXScaleMode() const {
  return XScaleMode;
}

KstScaleModeType KstPlot::getYScaleMode() const {
  return YScaleMode;
}

void KstPlot::setXScaleMode(KstScaleModeType scalemode_in) {
  XScaleMode = scalemode_in;
}

void KstPlot::setYScaleMode(KstScaleModeType scalemode_in) {
  YScaleMode = scalemode_in;
}

void KstPlot::addCurve(KstBaseCurve *incurve) {
  Curves.append(incurve);
}

void KstPlot::removeCurve(KstBaseCurve *incurve) {
  Curves.remove(incurve);
}

void KstPlot::updateScale() {
  double mid, delta;

  switch (XScaleMode) {
      case AUTO:  // set scale so all of all curves fits
        if (Curves.isEmpty()) {
          XMin = 0;
          XMax = 1.0;
        } else {
          if (XLog) {
            XMin = Curves[0]->minPosX();
          } else {
            XMin = Curves[0]->minX();
          }
          XMax = Curves[0]->maxX();
          for (unsigned i = 1; i < Curves.count(); i++) {
            if (XLog) {
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
          if (XMax <= XMin) {  // if curves had no variation in them
            XMin -= 0.1;
            XMax = XMin + 0.2;
          }
        }
        break;
      case NOSPIKE:  // set scale so all of all curves fits
        if (Curves.isEmpty()) {
          XMin = 0;
          XMax = 1.0;
        } else {
          if (XLog) {
            XMin = Curves[0]->minPosX();
          } else {
            XMin = Curves[0]->ns_minX();
          }
          XMax = Curves[0]->ns_maxX();
          for (unsigned i = 1; i < Curves.count(); i++) {
            if (XLog) {
              if (XMin > Curves[i]->minPosX()) {
                XMin = Curves[i]->minPosX();
              }
            } else {
              if (XMin > Curves[i]->ns_minX()) {
                XMin = Curves[i]->ns_minX();
              }
            }
            if (XMax < Curves[i]->ns_maxX()) {
              XMax = Curves[i]->ns_maxX();
            }
          }
          if (XMax <= XMin) {  // if curves had no variation in them
            XMin -= 0.1;
            XMax = XMin + 0.2;
          }
        }
        break;
      case AC: // keep range const, but set mid to mid of all curves
        if (XMax <= XMin) { // make sure that range is legal
          XMin = 0; XMax = 1;
        }
        if (Curves.isEmpty()) {
          XMin = -0.5;
          XMax = 0.5;
        } else {
          mid = Curves[0]->midX();
          for (unsigned i = 1; i < Curves.count(); i++) {
            mid += Curves[i]->midX();
          }
          mid /= Curves.count();
          delta = XMax - XMin;
          XMin = mid - delta / 2.0;
          XMax = mid + delta / 2.0;
        }

        break;
      case FIXED:  // don't change the range
        if (XMin >= XMax) {  // has to be legal, even for fixed scale...
          if (XMax == 0) {
            XMax =  0.5;
            XMin = -0.5;
          } else {
            XMax += XMax*0.01;
            XMin -= XMin*0.01;
          }
        }
        break;
      case AUTOUP:  // only change up
        for (unsigned i = 0; i < Curves.count(); i++) {
          if (XLog) {
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

        if (XMin >= XMax) {  // has to be legal, even for autoup...
          if (XMax == 0) {
            XMax =  0.5;
            XMin = -0.5;
          } else {
            XMax += XMax * 0.01;
            XMax = XMin * 0.02;
          }
        }
        break;
      default:
        fprintf(stderr, "Bug in KstPlot::updateScale: bad scale mode\n");
        break;
  }

  switch (YScaleMode) {
      case AUTO:  // set scale so all of all curves fits
        if (Curves.isEmpty()) {
          YMin = 0;
          YMax = 1.0;
        } else {
          bool isPsd = false;

          if (YLog) {
            YMin = Curves[0]->minPosY();
          } else {
            YMin = Curves[0]->minY();
          }
          YMax = Curves[0]->maxY();
          if (Curves[0]->type() == KST_PSDCURVE)
            isPsd = true;
          for (unsigned i = 1; i < Curves.count(); i++) {
            if (YLog) {
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
            if (Curves[0]->type() == KST_PSDCURVE) {
              isPsd = true;
            }
          }
          if (YMax <= YMin) {  // if curves had no variation in them
            YMin -= 0.1;
            YMax = YMin + 0.2;
          }
          if (isPsd && !YLog) { /* psd plots should default to 0min */
            YMin = 0;
          }
        }
        break;
      case NOSPIKE:  // set scale so all of all curves fits
        if (Curves.isEmpty()) {
          YMin = 0;
          YMax = 1.0;
        } else {
          bool isPsd = false;

          YMin = YLog ? Curves[0]->minPosY() : Curves[0]->ns_minY();
          YMax = Curves[0]->ns_maxY();

          if (Curves[0]->type() == KST_PSDCURVE) {
            isPsd = true;
          }

          for (unsigned i = 1; i < Curves.count(); i++) {
            if (YLog) {
              if (YMin > Curves[i]->minPosY()) {
                YMin = Curves[i]->minPosY();
              }
            } else {
              if (YMin > Curves[i]->ns_minY()) {
                YMin = Curves[i]->ns_minY();
              }
            }
            if (YMax < Curves[i]->ns_maxY()) {
              YMax = Curves[i]->ns_maxY();
            }
            if (Curves[0]->type() == KST_PSDCURVE) {
              isPsd = true;
            }
          }
          if (YMax <= YMin) {  // if curves had no variation in them
            YMin -= 0.1;
            YMax = YMin + 0.2;
          }
          if (isPsd && !YLog) { /* psd plots should default to 0min */
            YMin = 0;
          }
        }
        break;
      case AC: // keep range const, but set mid to mean of all curves
        if (YMax <= YMin) { // make sure that range is legal
          YMin = 0;
          YMax = 1;
        }
        if (Curves.isEmpty()) {
          YMin = -0.5;
          YMax = 0.5;
        } else {
          mid = Curves[0]->midY();
          for (unsigned i = 1; i < Curves.count(); i++) {
            mid += Curves[i]->midY();
          }
          mid /= Curves.count();
          delta = YMax - YMin;
          YMin = mid - delta / 2.0;
          YMax = mid + delta / 2.0;
        }

        break;
      case FIXED:  // don't change the range
        if (YMin >= YMax) {  // has to be legal, even for fixed scale...
          if (YMax == 0) {
            YMax =  0.5;
            YMin = -0.5;
          } else {
            YMax += YMax*0.01;
            YMin -= YMin*0.01;
          }
        }
        break;
      case AUTOUP:  // only change up
        for (unsigned i = 0; i < Curves.count(); i++) {
          if (YLog) {
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

        if (YMin >= YMax) {  // has to be legal, even for autoup...
          if (YMax == 0) {
            YMax =  0.5;
            YMin = -0.5;
          } else {
            YMax += YMax*0.01;
            YMin -= YMin*0.01;
          }
        }
        break;
      default:
        fprintf(stderr, "Bug in KstPlot::updateScale: bad scale mode\n");
        break;
  }

}


inline int d2i(double x) {
  return int(floor(x+0.5));
}

inline void SafeD2I(double &dx, int &x) {

  /* make sure that we can work in integers, which is faster */
  /* we have to covert for p.drawline, so we may as well do it now */
  if (dx > double(INT_MAX)) {
    x = INT_MAX;
  } else if (dx < double(INT_MIN)) {
    x = INT_MIN;
  } else {
    x = int(dx);
  }
}

static void genAxisTikLabel(char *label, double z, bool isLog) {
  if (isLog) {
    if (z > -4 && z < 4) {
      sprintf(label, "%.9g", pow(10,z));
    } else {
      sprintf(label, "10^{%.0f}", z);
    }
  } else {
    sprintf(label, "%.9g", z);
  }
}

double KstPlot::getXBorder(QPainter &p) {
  double YTick, Yorg; // Tick interval and position
  double x_min, x_max, y_min, y_max;

  double xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px;
  int x_px, y_px;

  updateScale();

  getLScale(x_min, y_min, x_max, y_max);

  QRect v = p.window();
  x_px = v.width();
  y_px = v.height();

  setTicks(YTick, Yorg, y_max, y_min, YLog);

  setBorders(xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px,
             YTick, Yorg, p);

  return xleft_bdr_px;
}

void KstPlot::setTicks(double &tick, double &org,
                       double max, double min, bool is_log) {
  double St,Mant1,Mant2;  // used to generate tick interval
  double Exp,Log,FLog;    // used to generate tick interval

  if (is_log && max - min < 11 && max - min > 1) {
    tick = 1.0;
  } else {
    /* Determine tik Interval */
    St = (max - min) / 5.0;       /* tiks */
    Log = log10(St);
    FLog = (double)floor(Log);
    Exp = pow(10., FLog);
    Mant1 = fabs(Exp - St) < fabs(2.0 * Exp - St) ? Exp : 2.0 * Exp;
    Mant2 = fabs(5.0 * Exp - St) < fabs(10.0 * Exp - St) ? 5.0 * Exp : 10.0 * Exp;
    tick = fabs(Mant1 - St) < fabs(Mant2 - St) ? Mant1 : Mant2;
  }

  /* Determine Location of Origin */
  if (min > 0) {
    org = (double)ceil(min / tick) * tick;
  } else if (max < 0) {
    org = (double)floor(max / tick) * tick;
  } else {
    org = 0;
  }
}


void setTickPix(double &xtickpix, double &ytickpix, int x_pix, int y_pix) {
  /* set tick size: 4 points on a full letter size plot */
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

void KstPlot::setBorders(double &xleft_bdr_px, double &xright_bdr_px,
                         double &ytop_bdr_px, double &ybot_bdr_px,
                         double YTick, double Yorg,
                         QPainter &p) {
  int x_px, y_px,i;
  char TmpStr[120];

  QRect v = p.window();

  x_px = v.width();
  y_px = v.height();

  /*********************************************/
  /* Set Borders                               */
  ytop_bdr_px = 1.3 * TopLabel->lineSpacing(p);
  ybot_bdr_px = 1.3 * XLabel->lineSpacing(p) + TickLabel->ascent(p);

  double ifloat = (YMax - Yorg)/YTick;
  i = int(floor(ifloat));

  // necessary?
  if (double(i) == ifloat) {
    --i;
  }

  --i;

  genAxisTikLabel(TmpStr, i * YTick + Yorg, YLog);
  TickLabel->setJustification(RxCy);
  TickLabel->setText(TmpStr);
  xleft_bdr_px = TickLabel->width(p);

  /* now check min label */
  ifloat = (YMin - Yorg)/YTick;
  i = int(floor(ifloat));

  // necessary?
  if (double(i) == ifloat) {
    ++i;
  }

  ++i;

  genAxisTikLabel(TmpStr, i * YTick + Yorg, YLog);
  TickLabel->setText(TmpStr);
  i = TickLabel->width(p);
  if (i > xleft_bdr_px) {
    xleft_bdr_px = i;
  }
  xleft_bdr_px += 1.5 * YLabel->lineSpacing(p) + 5;

  xright_bdr_px = x_px / 30;

  xleft_bdr_px = ceil(xleft_bdr_px);
  xright_bdr_px = ceil(xright_bdr_px);
  ytop_bdr_px = ceil(ytop_bdr_px);
  ybot_bdr_px = ceil(ybot_bdr_px);

}


void KstPlot::drawDotAt(QPainter& p, double x, double y) {
  if (XLog) {
    x = x > 0 ? log10(x) : -350;
  }
  if (YLog) {
    y = y > 0 ? log10(y) : -350;
  }

  double x_min, x_max, y_min, y_max;
  int X1, Y1;
  getLScale(x_min, y_min, x_max, y_max);

  X1 = int(PlotRegion.width() * (x - x_min) / (x_max - x_min) + PlotRegion.left());
  Y1 = int(PlotRegion.height() * (y_max - y) / (y_max - y_min) + PlotRegion.top());

  if (!PlotRegion.contains(X1, Y1)) {
    return;
  }
  p.setPen(QPen(QColor(0,0,0), 0));
  p.drawArc((int)X1 - 3, (int)Y1 - 3, 6, 6, 0, 5760);
  p.setPen(QPen(QColor(255,0,0), 0));
  p.drawArc((int)X1 - 2, (int)Y1 - 2, 4, 4, 0, 5760);
  p.drawArc((int)X1 - 1, (int)Y1 - 1, 2, 2, 0, 5760);
}


void KstPlot::paint(QPainter &p, double in_xleft_bdr_px) {
  double XTick, YTick, Xorg, Yorg; // Tick interval and position
  double x_min, x_max, y_min, y_max;

  // Plot dimentions in pixels
  double xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px;
  double x_orig_px,y_orig_px,xtick_len_px,ytick_len_px;
  double xtick_px,ytick_px;
  int x_px, y_px;

  // Used for drawing curves
  double Lx,Hx,Ly,Hy;
  double m_X,b_X,m_Y,b_Y;
  double rX,rY, rEX, rEY;
  double X1,Y1, EX, EY;
  double last_x1, last_y1;
  double X2,Y2;
  int X2i, last_x1i=0;
  double maxY=0, minY=0;
  int penWidth;

  int i_pt;
  KstBaseCurve *c;

  char TmpStr[120];

  int i, j;
  bool overlap, nopoint;

  updateScale();

  getLScale(x_min, y_min, x_max, y_max);

  QRect v = p.window();
  //p.fillRect(v, printMode ? backgroundColor() : KstSettings::globalSettings()->backgroundColor);
  p.fillRect(v, KstSettings::globalSettings()->backgroundColor);

  x_px = v.width();
  y_px = v.height();

  penWidth = x_px / 999;
  if (penWidth == 1) {
    penWidth = 0;
  }

  p.setPen(QPen(KstSettings::globalSettings()->foregroundColor, penWidth));

  setTicks(XTick, Xorg, x_max, x_min, XLog);
  setTicks(YTick, Yorg, y_max, y_min, YLog);

  setTickPix(xtick_len_px, ytick_len_px, x_px, y_px);

  setBorders(xleft_bdr_px, xright_bdr_px, ytop_bdr_px, ybot_bdr_px, YTick, Yorg, p);
  if (in_xleft_bdr_px > 0.001) { // x border overridden
    xleft_bdr_px = in_xleft_bdr_px;
  }


  x_orig_px = (Xorg - x_min) / (x_max - x_min) *
         (double)(x_px - (xleft_bdr_px + xright_bdr_px)) + xleft_bdr_px;
  y_orig_px = (y_max-Yorg) / (y_max-y_min) *
         (double)(y_px - (ytop_bdr_px + ybot_bdr_px)) + ytop_bdr_px;
  xtick_px = (XTick / (x_max - x_min)) * ((double)x_px - (xleft_bdr_px + xright_bdr_px));
  ytick_px = (YTick / (y_max - y_min)) * ((double)y_px - (ytop_bdr_px + ybot_bdr_px));

  /* return if the plot is too small to draw */
  if (x_px - xright_bdr_px - xleft_bdr_px < 10) {
    return;
  }

  if (y_px - ybot_bdr_px - ytop_bdr_px + 1.0 - ytop_bdr_px < 10) {
    return;
  }

  RelPlotRegion.setRect(d2i(xleft_bdr_px),
                        d2i(ytop_bdr_px),
                        d2i(x_px - xright_bdr_px - xleft_bdr_px + 1.0),
                        d2i(y_px - ybot_bdr_px - ytop_bdr_px + 1.0));


  RelWinRegion.setRect(0, 0, (int)x_px, (int)y_px);

  RelPlotAndAxisRegion.setRect(
    d2i(YLabel->lineSpacing(p) + 1),
    d2i(ytop_bdr_px),
    d2i(x_px - YLabel->lineSpacing(p) - xright_bdr_px),
    d2i(y_px - XLabel->lineSpacing(p) - ytop_bdr_px));

  /******************************************************************/
  /* Draw the axis and labels */
  /* Draw Labels */

  YLabel->draw(p, (YLabel->lineSpacing(p) - YLabel->ascent(p))/2, y_px/2);
  XLabel->draw(p, x_px/2, y_px-(XLabel->lineSpacing(p) - XLabel->ascent(p))/2);
  TopLabel->draw(p, d2i(xleft_bdr_px), d2i(0.85*(ytop_bdr_px)));

  /* Draw Axis */
  p.drawRect(RelPlotRegion);

  if (XLog) {
    /* Draw X Ticks */
    for (i = -1; xtick_px * i + x_orig_px > xleft_bdr_px - 1; i--); // find starting i;
    //i++;
    for (;xtick_px * i + x_orig_px < x_px - xright_bdr_px + 1; i++) {
      // draw major ticks
      X1 = (x_orig_px + (double)i * xtick_px);
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
      if (XTick == 1.0) {
        for (j = 2; j < 10; j++) {
          X2 = log10((double)j) * (double)xtick_px + X1;
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
    }
  } else {
    /* Draw X Ticks */
    for (i = -1; xtick_px * i / 5.0 + x_orig_px > xleft_bdr_px - 1; i--); // find starting i
    i++;
    for (; xtick_px * i / 5 + x_orig_px < x_px - xright_bdr_px ; i++) {
      X1 = x_orig_px + (double)i * xtick_px / 5.0;
      if (i % 5 == 0) {
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

  /* Draw Y Ticks */
  if (YLog) {
    for (i = -1; ytick_px * i + y_orig_px > ytop_bdr_px - 1; i--);
    //i++;
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
      if (YTick == 1.0) {
        for (j = 2; j < 10; j++) {
          Y2 = (-log10((double)j) + 1.0) * (double)ytick_px + Y1;
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
    }
  } else {
    for (i = -1; ytick_px * i / 5 + y_orig_px > ytop_bdr_px - 1; i--);
    i++;
    for (; ytick_px * i / 5 + y_orig_px < y_px - ybot_bdr_px + 1; i++) {
      Y1 = y_orig_px + (double)i * ytick_px / 5.0;
      if (i % 5 == 0) {
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

  /* Print Numbers */
  /* x axis numbers */
  TickLabel->setJustification(CxTy);
  for (i = -1; xtick_px * i + x_orig_px > xleft_bdr_px - 1; i--);
  i++;
  for (; xtick_px * i + x_orig_px < x_px - xright_bdr_px + 1; i++) {
    genAxisTikLabel(TmpStr, i * XTick + Xorg, XLog);
    TickLabel->setText(TmpStr);
    TickLabel->draw(p, d2i(x_orig_px + i * xtick_px),
                   d2i(y_px - (0.85 * ybot_bdr_px)));
  }

  /* y axis numbers */
  TickLabel->setJustification(RxCy);
  for (i = -1; ytick_px * i + y_orig_px > ytop_bdr_px - 1; i--);
  i++;
  for (; ytick_px * i + y_orig_px < y_px - ybot_bdr_px + 1; i++) {
    genAxisTikLabel(TmpStr, -(i*YTick) + Yorg, YLog);
    TickLabel->setText(TmpStr);
    TickLabel->draw(p, d2i(xleft_bdr_px - TickLabel->lineSpacing(p) / 4),
                   d2i(y_orig_px + i * ytick_px));
  }

  /*** plot the arbitrary labels **/
  for (KstLabel *label = labelList.first(); label; label = labelList.next()) {
    label->draw(p, d2i(xleft_bdr_px + RelPlotRegion.width() * label->x()),
                d2i(ytop_bdr_px + RelPlotRegion.height() * label->y()));
  }

  /*** plot the legend **/
  if( !Legend->getFront()) {
    Legend->draw( &Curves, p, d2i(xleft_bdr_px + RelPlotRegion.width() * Legend->x()),
                d2i(ytop_bdr_px + RelPlotRegion.height() * Legend->y()));
  }

  /*******************************************************************/
  /* Plot the Curves */
  Lx = (double)xleft_bdr_px + 1;
  Hx = (double)(x_px - xright_bdr_px - 1);
  Ly = (double)ytop_bdr_px + 1;
  Hy = (double)(y_px - ybot_bdr_px - 1);
  m_X = (Hx - Lx)/(x_max - x_min);
  m_Y = (Ly - Hy)/(y_max - y_min);
  b_X = Lx - m_X * x_min;
  b_Y = Hy - m_Y * y_min;

  for (int i_curve = 0; i_curve < (int)Curves.count(); i_curve++) {

    c = Curves[i_curve];
    overlap = false;
    if (c->sampleCount() > 0) {

      int i0, iN;
      if (c->xIsRising()) {
        i0 = c->getIndexNearX(x_min);
        if (i0>0) i0--;
        iN = c->getIndexNearX(x_max);
        if (iN<c->sampleCount() - 1) iN++;
      } else {
        i0 = 0;
        iN = c->sampleCount() - 1;
      }

      p.setPen(QPen(c->getColor(), penWidth));

      if (c->hasLines()) {

        /* Read i_pt = 0 */
        nopoint = false;
        c->getPoint(i0, rX, rY);

// Optimize - isnan seems expensive, at least in gcc debug mode
//            cachegrind backs this up.
#define isnan(x) (x != x)
        if (isnan(rX)) {
          nopoint = true;
        }
        if (isnan(rY)) {
          nopoint = true;
        }

        if (XLog) {
          if (rX > 0) {
            rX = log10(rX);
          } else {
            rX = -350;
          }
        }

        if (YLog) {
          if (rY > 0) {
            rY = log10(rY);
          } else {
            rY = -350;
          }
        }

        X1 = m_X*rX + b_X;
        Y1 = m_Y*rY + b_Y;

        last_x1 = X1;
        last_x1i = (int)X1;
        last_y1 = Y1;

        for (i_pt = i0 + 1; i_pt <= iN; i_pt++) {

          X2 = last_x1;
          X2i = last_x1i;
          Y2 = last_y1;

          /* read next i_pt */
          nopoint = false;
          c->getPoint(i_pt, rX, rY);
          while ((isnan(rX) || isnan(rY)) && i_pt < iN) {
#undef isnan
            nopoint = true;
            i_pt++;
            c->getPoint(i_pt, rX, rY);
          }

          if (XLog) {
            rX = rX > 0 ? log10(rX) : -350;
          }
          if (YLog) {
            rY = rY > 0 ? log10(rY) : -350;
          }

          X1 = m_X*rX + b_X;
          Y1 = m_Y*rY + b_Y;

          last_x1 = X1;
          last_x1i = (int)X1;
          last_y1 = Y1;
          if (nopoint) {
            if (overlap) {
              if (X2 >= Lx && X2 <= Hx) {
                if (maxY > Hy && minY <= Hy) {
                  maxY = Hy;
                }
                if (minY < Ly && maxY >= Ly) {
                  minY = Ly;
                }
                if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
                  p.drawLine((int)X2, (int)minY, (int)X2, (int)maxY);
                }
              }
              overlap = false;
            }
          } else if (last_x1i == X2i) {
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
            if (!((X1 < Lx && X2 < Lx) || (X1 > Hx && X2 > Hx))) {
              /* trim all lines to be within plot */
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
            }

            if (overlap) {
              if (X2 >= Lx && X2 <= Hx) {
                if (maxY > Hy && minY <= Hy)
                  maxY = Hy;
                if (minY < Ly && maxY >= Ly)
                  minY = Ly;
                if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
                  p.drawLine((int)X2, (int)minY, (int)X2, (int)maxY);
                }
              }
              overlap = false;
            }

            /* make sure both ends are in range: */
            if (X1 >= Lx && X1 <= Hx && X2 >= Lx && X2 <= Hx) {
              if (Y1 >= Ly && Y1 <= Hy && Y2 >= Ly && Y2 <= Hy) {
                p.drawLine((int)X1, (int)Y1, (int)X2, (int)Y2);
              }
            }
          }  /* end if (X1 == X2) */
        } // end for
      } // end if c->hasLines()

      if (c->hasPoints()) {
        for (i_pt = i0; i_pt < iN; i_pt++) {
          c->getPoint(i_pt, rX, rY);
          if (XLog) {
            rX = rX > 0 ? log10(rX) : -350;
          }
          if (YLog) {
            rY = rY > 0 ? log10(rY) : -350;
          }

          X1 = m_X * rX + b_X;
          Y1 = m_Y * rY + b_Y;
          if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y1 <= Hy) {
            c->Point.draw(&p, (int)X1, (int)Y1);
          }
        }
      }
      if (c->hasXError()) {
        bool do_low_flag = true;
        bool do_high_flag = true;

        for (i_pt = i0; i_pt < iN; i_pt++) {
          do_low_flag = true;
          do_high_flag = true;
          c->getEXPoint(i_pt, rX, rY, rEX);
          EX = m_X * rEX;
          X1 = m_X * rX + b_X - EX;
          X2 = m_X * rX + b_X + EX;
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
            p.drawLine((int)X1, (int) Y1, (int) X2, (int) Y1);
            if (do_low_flag) {
              p.drawLine((int)X1, (int)Y1 + c->Point.getDim(&p),
                  (int)X1, (int)Y1 - c->Point.getDim(&p));
            }
            if (do_high_flag) {
              p.drawLine((int)X2, (int)Y1 + c->Point.getDim(&p),
                  (int)X2, (int)Y1 - c->Point.getDim(&p));
            }
          }
        }
      }

      if (c->hasYError()) {
        bool do_low_flag = true;
        bool do_high_flag = true;

        for (i_pt = i0; i_pt<iN; i_pt++) {
          do_low_flag = true;
          do_high_flag = true;
          c->getEYPoint(i_pt, rX, rY, rEY);
          EY = m_Y * rEY;
          X1 = m_X * rX + b_X;
          Y1 = m_Y * rY + b_Y - EY;
          Y2 = m_Y * rY + b_Y + EY;
          if (Y1 < Ly && Y2 > Ly) {
            Y1 = Ly;
            do_low_flag = false;
          }
          if (Y1 < Hy && Y2 > Hy) {
            Y2 = Hy;
            do_high_flag = false;
          }

          if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y2 <= Hy) {
            p.drawLine((int)X1, (int) Y1, (int) X1, (int) Y2);
            if (do_low_flag) {
              p.drawLine((int)X1 + c->Point.getDim(&p), (int)Y1,
                  (int)X1 - c->Point.getDim(&p), (int)Y1);
            }
            if (do_high_flag) {
              p.drawLine((int)X1 + c->Point.getDim(&p), (int)Y2,
                  (int)X1 - c->Point.getDim(&p), (int)Y2);
            }
          }
        }
      }
    }
  }

  /*** plot the legend **/
  if( Legend->getFront()) {
    Legend->draw( &Curves, p, d2i(xleft_bdr_px + RelPlotRegion.width() * Legend->x()),
                d2i(ytop_bdr_px + RelPlotRegion.height() * Legend->y()));
  }

  p.flush();

}

QRect KstPlot::GetPlotRegion() const {
  return PlotRegion;
}

QRect KstPlot::GetWinRegion() const {
  return WinRegion;
}

QRect KstPlot::GetPlotAndAxisRegion() const {
  return PlotAndAxisRegion;
}

QRect KstPlot::GetTieBoxRegion() const {
  int left, top;
  const int dim=11;

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


const QString& KstPlot::tagName() const {
  return Tag;
}

void KstPlot::setpixrect(int x0, int y0) {
  PlotRegion = RelPlotRegion;
  PlotRegion.moveBy(x0, y0);
  WinRegion = RelWinRegion;
  WinRegion.moveBy(x0, y0);
  PlotAndAxisRegion = RelPlotAndAxisRegion;
  PlotAndAxisRegion.moveBy(x0,y0);
}


void KstPlot::save(QTextStream &ts) {
  ts << "  <tag>" << tagName() << "</tag>" << endl;
  ts << "  <width>" << width() << "</width>" << endl;
  ts << "  <height>" << height() << "</height>" << endl;
  ts << "  <pos_x>" << topLeftX() << "</pos_x>" << endl;
  ts << "  <pos_y>" << topLeftY() << "</pos_y>" << endl;

  ts << "  <xscalemode>" << XScaleMode << "</xscalemode>" << endl;
  ts << "  <yscalemode>" << YScaleMode << "</yscalemode>" << endl;

  ts << "  <xmin>" << XMin << "</xmin>" << endl;
  ts << "  <xmax>" << XMax << "</xmax>" << endl;
  ts << "  <ymin>" << YMin << "</ymin>" << endl;
  ts << "  <ymax>" << YMax << "</ymax>" << endl;

  ts << "  <toplabel>" << endl;
  TopLabel->save(ts);
  ts << "  </toplabel>" << endl;

  ts << "  <xlabel>" << endl;
  XLabel->save(ts);
  ts << "  </xlabel>" << endl;

  ts << "  <ylabel>" << endl;
  YLabel->save(ts);
  ts << "  </ylabel>" << endl;

  ts << "  <ticklabel>" << endl;
  TickLabel->save(ts);
  ts << "  </ticklabel>" << endl;

  ts << "  <legend>" << endl;
  Legend->save(ts);
  ts << "  </legend>" << endl;

  if (isXLog()) ts << "  <xlog/>" << endl;
  if (isYLog()) ts << "  <ylog/>" << endl;

  for (unsigned i = 0; i < labelList.count(); i++) {
    ts << "  <label>" << endl;
    labelList.at(i)->save(ts);
    ts << "  </label>" << endl;
  }

  for (unsigned i = 0; i < Curves.count(); i++) {
    ts << "  <curvetag>" << Curves[i]->tagName() << "</curvetag>" << endl;
  }

}

void KstPlot::pushScale() {
  struct KstPlotScale *ps;
  ps = new struct KstPlotScale;

  ps->xmin = XMin;
  ps->ymin = YMin;
  ps->xmax = XMax;
  ps->ymax = YMax;
  ps->xscalemode = XScaleMode;
  ps->yscalemode = YScaleMode;

  PlotScaleList.append(ps);
}

bool KstPlot::popScale() {
  struct KstPlotScale *ps;

  if (PlotScaleList.count() > 1) {
    PlotScaleList.removeLast();
    ps = PlotScaleList.last();
    XMin = ps->xmin;
    XMax = ps->xmax;
    YMin = ps->ymin;
    YMax = ps->ymax;
    XScaleMode = ps->xscalemode;
    YScaleMode = ps->yscalemode;
    return true;
  }
  return false;
}

/****************************************************************/
/*                                                              */
/*        Place a '\' in front of special characters (ie, '_')  */
/*                                                              */
/****************************************************************/
static void EscapeSpecialChars(QString &label) {
  unsigned int i_char;

  for (i_char = 0; i_char < label.length(); i_char++) {
    if (label.at(i_char) == '_') {
      label.insert(i_char, '\\');
      i_char++;
    }
  }
}

void KstPlot::GenerateDefaultLabels() {
  int n_curves, i_curve;
  QString xlabel, ylabel, toplabel;

  n_curves = Curves.count();

  if (n_curves < 1) return;

  if (n_curves == 1) {
    xlabel = Curves[0]->getXLabel();
    ylabel = Curves[0]->getYLabel();
    toplabel = Curves[0]->getTopLabel();
  } else {
    xlabel = Curves[0]->getXLabel();
    ylabel = QString::null;
    toplabel = QString::null;

    ylabel = Curves[0]->getYLabel();
    toplabel = Curves[0]->getTopLabel();
    for (i_curve = 1; i_curve < n_curves - 1; i_curve++) {
      ylabel += QString(", ") + Curves[i_curve]->getYLabel();
      if (toplabel != Curves[i_curve]->getTopLabel()) {
        toplabel += QString(", ") + Curves[i_curve]->getTopLabel();
      }
    }

    // FIXME: i18n is broken here.
    ylabel += QString(" and ") + Curves[n_curves - 1]->getYLabel();
    if (toplabel != Curves[i_curve]->getTopLabel() &&
        !Curves[i_curve]->getTopLabel().isEmpty()) {
      toplabel += QString(" and ") + Curves[n_curves - 1]->getTopLabel();
    }
  }

  EscapeSpecialChars(xlabel);
  EscapeSpecialChars(ylabel);
  EscapeSpecialChars(toplabel);

  setXLabel(xlabel);
  setYLabel(ylabel);
  setTopLabel(toplabel);
}

void KstPlot::setLog(bool x_log, bool y_log) {
  XLog = x_log;
  YLog = y_log;
}

bool KstPlot::isXLog() const {
  return XLog;
}

bool KstPlot::isYLog() const {
  return YLog;
}

bool KstPlot::isTied() const {
  return IsTied;
}

void KstPlot::toggleTied() {
  IsTied = !IsTied;
}

void KstPlot::setTied(bool in_tied) {
  IsTied = in_tied;
}

// vim: ts=2 sw=2 et
