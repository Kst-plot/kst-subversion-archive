/***************************************************************************
                          kstplot.h - one plot for kst
                             -------------------
    begin                : Sun Nov 12 2000
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

#ifndef KSTPLOT_H
#define KSTPLOT_H

#include <qfont.h>
#include <qtextstream.h>
#include <qptrlist.h>

#include "kstlabel.h"
#include "kstlegend.h"
#include "kstbasecurve.h"

/** A class representing one plot for kst

 *@author cbn
 */

typedef enum {AUTO, AC, FIXED, AUTOUP, NOSPIKE} KstScaleModeType;

struct KstPlotScale {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  KstScaleModeType xscalemode;
  KstScaleModeType yscalemode;
};

// FIXME: make KSharedPtr asap!! - could be causing bugs already
// DONT fixme - grumpy barth says ksharedptrs are evil - they just mask
// bugs (very effectivly).
class KstPlot {
public:

  KstPlot(const QString &in_tag,
          float width = 1, float height = 1,
          float tlx = 0, float tly = 0,
          KstScaleModeType yscale = AUTO,
          KstScaleModeType xscale = AUTO,
          double xmin=0, double ymin=0,
          double xmax=1, double ymax=1);

  KstPlot(QDomElement &e, KstBaseCurveList &curveList);

  ~KstPlot();

  void paint(QPainter &pd, double X_LeftBorder=0);
  void drawDotAt(QPainter& p, double x, double y);
  double getXBorder(QPainter &p);
  void addCurve(KstBaseCurve *curve);
  void removeCurve(KstBaseCurve *curve);
  void setXScaleMode(KstScaleModeType scalemode);
  void setYScaleMode(KstScaleModeType scalemode);

  /** Set the scale */
  void setScale(double xmin, double ymin,
                double xmax, double ymax);
  /** Set the X scale */
  void setXScale(double xmin, double xmax);
  /** Set the y scale */
  void setYScale(double ymin, double ymax);

  /** Set the log_scale */
  void setLScale(double xmin, double ymin,
                double xmax, double ymax);
  /** Set the X log_scale */
  void setLXScale(double xmin, double xmax);
  /** Set the y log_scale */
  void setLYScale(double ymin, double ymax);
  /** Push the scale setting (X,Y, mode) onto scale list */
  void pushScale();
  /** Pop the scale settings off of scale list: true if stack not empty */
  bool popScale();

  void setLog(bool x_log, bool y_log);
  bool isXLog() const;
  bool isYLog() const;

  void setTagName(const QString &new_tag);
  void getScale(double &xmin, double &ymin, double &xmax, double &ymax) const;
  void getLScale(double &xmin, double &ymin, double &xmax, double &ymax) const;

  KstScaleModeType getXScaleMode() const;
  KstScaleModeType getYScaleMode() const;

  void setXLabel(const QString &xlabel);
  void setYLabel(const QString &ylabel);
  void setTopLabel(const QString &toplabel);
  void initFonts(const QFont &font, int font_size = 0);

  /** The dimentions describe where on the view this plot lands. */
  void setDims(float width, float height,
               float ltx, float tly);
  float width() const;
  float height() const;
  float topLeftX() const;
  float topLeftY() const;

  QRect GetPlotRegion() const;
  QRect GetWinRegion() const;
  QRect GetTieBoxRegion() const;
  QRect GetPlotAndAxisRegion() const;

  bool isTied() const;
  void toggleTied();
  void setTied(bool is_tied);

  const QString& tagName() const;

  /** Labels */
  KstLabel *XLabel;
  KstLabel *YLabel;
  KstLabel *TopLabel;
  KstLabel *TickLabel;
  KstLegend *Legend;

  /** Arbitrary Labels */
  KstLabelList labelList;


  KstBaseCurveList Curves;

  void GenerateDefaultLabels();

  void save(QTextStream &ts);
  /* kstview tells kstplot where to offset the plot to */
  void setpixrect(int x0, int y0);

  void setBackgroundColor(QColor in_color) {BackgroundColor = in_color;}
  void setForegroundColor(QColor in_color) {ForegroundColor = in_color;}
  QColor backgroundColor() {return (BackgroundColor);}
  QColor foregroundColor() {return (ForegroundColor);};

private:
  inline void commonConstructor(const QString &in_tag,
                                float width,
                                float height,
                                float tlx,
                                float tly,
                                KstScaleModeType yscale,
                                KstScaleModeType xscale,
                                double xmin,
                                double ymin,
                                double xmax,
                                double ymax,
                                bool x_log = false,
                                bool y_log = false);
  /** range and domain of plot: not plot dimentions */
  double XMin;
  double XMax;
  double YMin;
  double YMax;

  bool XLog;
  bool YLog;

  KstScaleModeType XScaleMode;
  KstScaleModeType YScaleMode;

  QPtrList<KstPlotScale> PlotScaleList;

  /** Used by KstView to store width (0 to 1) */
  float Width;
  /** Used by KstView to store height (0 to 1) */
  float Height;
  /** Used by KstView to store plot location (0 to 1) */
  float TopLeftX;
  /** Used by KstView to store plot location (0 to 1) */
  float TopLeftY;

  /** Stores border limits.  Set by KstPlot::paint().
      Stored here to be Used to determine mouse mode */
  QRect PlotRegion;
  QRect RelPlotRegion;
  QRect WinRegion;
  QRect RelWinRegion;
  QRect PlotAndAxisRegion;
  QRect RelPlotAndAxisRegion;

  void updateScale();

  QString Tag;

  void setBorders(double &xleft_bdr_px, double &xright_bdr_px,
                  double &ytop_bdr_px, double &ybot_bdr_px,
                  double YTick, double Yorg,
                  QPainter &p);
  void setTicks(double &tick, double &org,
                double max, double min, bool is_log);
  bool IsTied;

  QColor BackgroundColor;
  QColor ForegroundColor;

};


#endif
