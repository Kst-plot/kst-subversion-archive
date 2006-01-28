/***************************************************************************
                              kst2dplot.h
                             -------------
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

#ifndef KST2DPLOT_H
#define KST2DPLOT_H

#include <time.h>

#include "kstbackbuffer.h"
#include "kstbasecurve.h"
#include "kstcolorsequence.h"
#include "kstplotbase.h"
#include "kstplotdefines.h"
#include "kstviewwidget.h"
#include "kstviewlegend.h"
#include "kst_export.h"

#include <qvaluestack.h>

namespace Equation {
  class Node;
}

class Kst2DPlot;
typedef KstObjectList<KstSharedPtr<Kst2DPlot> > Kst2DPlotList;
class KstViewLabel;
typedef KstSharedPtr<KstViewLabel> KstViewLabelPtr;
class KstPlotLabel;

enum KstScaleModeType { AUTO = 0, AC = 1, FIXED = 2, AUTOUP = 3, NOSPIKE = 4, AUTOBORDER = 5, EXPRESSION = 6 };

struct KstPlotScale {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  KstScaleModeType xscalemode;
  KstScaleModeType yscalemode;
  bool xlog, ylog;
  QString xMinExp, xMaxExp, yMinExp, yMaxExp;
};

struct KstMarker {
  double value;
  bool isRising;
  bool isFalling;
};

typedef QValueList<KstMarker> KstMarkerList;

enum KstMouseModeType { INACTIVE, XY_ZOOMBOX, Y_ZOOMBOX, X_ZOOMBOX, LAYOUT_TOOL };

struct KstMouse {
  KstMouse();
  KstMouseModeType mode;
  int label, minMove;
  QPoint lastLocation, pressLocation;  // for zooming primarily
  QPoint tracker; // for tracking the mouse location
  QPoint lastGuideline; // for tracking the last guideline location
  QRect plotGeometry;
  bool zooming() const;
  void zoomStart(KstMouseModeType t, const QPoint& location);
  void zoomUpdate(KstMouseModeType t, const QPoint& location);
  QRect mouseRect() const;
  bool rectBigEnough() const;
};


class KST_EXPORT Kst2DPlot : public KstPlotBase {
  Q_OBJECT
public:
  Kst2DPlot(const QString& in_tag = "SomePlot",
          KstScaleModeType yscale = AUTOBORDER,
          KstScaleModeType xscale = AUTO,
          double xmin = 0, double ymin = 0,
          double xmax = 1, double ymax = 1);
  Kst2DPlot(const QDomElement& e);
  Kst2DPlot(const Kst2DPlot& plot, const QString& name);
  virtual ~Kst2DPlot();

  static Kst2DPlotList globalPlotList();
  static bool checkRange(double& min_in, double& max_in);
  static bool checkLRange(double& min_in, double& max_in, bool isLog);
  static void genAxisTickLabel(QString& label, double z, bool isLog);

  virtual UpdateType update(int update_counter);
  virtual void save(QTextStream& ts, const QString& indent = QString::null);

  virtual bool popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
  virtual bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);

  void drawGraphicSelectionAt(QPainter& p, const QPoint& pos);
  void drawDotAt(QPainter& p, double x, double y);
  void drawPlusAt(QPainter& p, double x, double y);
  void internalAlignment(KstPainter& p, QRect& plotRegion);
  void addCurve(KstBaseCurvePtr curve);
  void removeCurve(KstBaseCurvePtr curve);
  void clearCurves();
  void setXScaleMode(KstScaleModeType scalemode);
  void setYScaleMode(KstScaleModeType scalemode);

  void pushAdjustLineWidth(int adjustment);
  void popLineWidth();
  void pushCurveColor(const QColor& c);
  void popCurveColor();
  void pushCurveHasPoints(bool yes);
  void popCurveHasPoints();
  void pushCurveHasLines(bool yes);
  void popCurveHasLines();
  void pushCurvePointDensity(int pointDensity);
  void popCurvePointDensity();
  void pushPlotColors();
  void popPlotColors();

  /** Set the scale */
  void setScale(double xmin, double ymin, double xmax, double ymax);
  /** Set the X scale */
  bool setXScale(double xmin, double xmax);
  /** Set the y scale */
  bool setYScale(double ymin, double ymax);

  // set the X Expressions
  bool setXExpressions(const QString& minExp, const QString& maxExp);
  // set the Y Expressions
  bool setYExpressions(const QString& minExp, const QString& maxExp);

  /** Set the log_scale */
  bool setLScale(double xmin, double ymin, double xmax, double ymax);
  /** Set the X log_scale */
  bool setLXScale(double xmin, double xmax);
  /** Set the y log_scale */
  bool setLYScale(double ymin, double ymax);
  /** Push the scale setting (X,Y, mode) onto scale list */
  void pushScale();
  /** Pop the scale settings off of scale list: true if stack not empty */
  bool popScale();

  void setLog(bool x_log, bool y_log);
  bool isXLog() const;
  bool isYLog() const;
  void setXAxisInterpretation( bool isXAxisInterpreted, KstAxisInterpretation xAxisInterpretation, KstAxisDisplay xAxisDisplay);
  void getXAxisInterpretation( bool& isXAxisInterpreted, KstAxisInterpretation& xAxisInterpretation, KstAxisDisplay& xAxisDisplay) const;
  void setYAxisInterpretation( bool isYAxisInterpreted, KstAxisInterpretation yAxisInterpretation, KstAxisDisplay yAxisDisplay);
  void getYAxisInterpretation( bool& isYAxisInterpreted, KstAxisInterpretation& yAxisInterpretation, KstAxisDisplay& yAxisDisplay) const;
  void getScale(double& xmin, double& ymin, double& xmax, double& ymax) const;

  // get expressions for the scale
  void getXScaleExps(QString& minExp, QString& maxExp);
  void getYScaleExps(QString& minExp, QString& maxExp);

  void getLScale(double& xmin, double& ymin, double& xmax, double& ymax) const;

  KstScaleModeType xScaleMode() const;
  KstScaleModeType yScaleMode() const;

  KstPlotLabel *xLabel() const;
  KstPlotLabel *yLabel() const;
  KstPlotLabel *topLabel() const;
  KstPlotLabel *xTickLabel() const;
  KstPlotLabel *yTickLabel() const;
  KstPlotLabel *fullTickLabel() const;


  QRect GetPlotRegion() const;
  QRect GetWinRegion() const;
  QRect GetTieBoxRegion() const;
  QRect GetPlotAndAxisRegion() const;

  bool isTied() const;
  void toggleTied();
  void setTied(bool is_tied);

  virtual QString menuTitle() const;

  double _width;
  double _height;
  double _pos_x;
  double _pos_y;

  KstBaseCurveList Curves;

  void generateDefaultLabels(bool xl = true, bool yl = true, bool zl = true);

  /* kstview tells kstplot where to offset the plot to */
  void setPixRect(const QRect& RelPlotRegion, const QRect& RelWinRegion, const QRect& RelPlotAndAxisRegion);

  virtual void move(const QPoint&);
  virtual void resize(const QSize&);
  virtual void parentResized();
  virtual void parentMoved(const QPoint&);

  virtual bool mouseHandler() const;
  virtual void mouseMoveEvent(QWidget *view, QMouseEvent *e);
  virtual void mousePressEvent(QWidget *view, QMouseEvent *e);
  virtual void mouseDoubleClickEvent(QWidget *view, QMouseEvent *e);
  virtual void mouseReleaseEvent(QWidget *view, QMouseEvent *e);
  virtual void keyPressEvent(QWidget *view, QKeyEvent *e);
  virtual void keyReleaseEvent(QWidget *view, QKeyEvent *e);
  virtual void wheelEvent(QWidget *view, QWheelEvent *e);
  virtual void setHasFocus(bool hasFocus);

  void cancelZoom(QWidget *view);
  void zoomSelfYLocalMax(bool unused);
  bool moveSelfHorizontal(bool left);
  bool moveSelfVertical(bool up);
  bool zoomSelfHorizontal(bool in);
  bool zoomSelfVertical(bool in);
  double xLeft() const;
  void moveUp(KstViewWidget *);
  void moveDown(KstViewWidget *);
  void moveLeft(KstViewWidget *);
  void moveRight(KstViewWidget *);
  void xZoomIn(KstViewWidget *);
  void yZoomIn(KstViewWidget *);
  void xZoomOut(KstViewWidget *);
  void yZoomOut(KstViewWidget *);
  void xZoomMax(KstViewWidget *);
  void yZoomMax(KstViewWidget *);
  void yZoomLocalMax(KstViewWidget *);
  void zoomMax(KstViewWidget *);
  void xLogSlot(KstViewWidget *);
  void yLogSlot(KstViewWidget *);
  void zoomPrev(KstViewWidget *);
  void yZoomAc(KstViewWidget *);
  void xZoomNormal(KstViewWidget *);
  void yZoomNormal(KstViewWidget *);
  void zoomSpikeInsensitiveMax(KstViewWidget *);

  // plot Markers
  bool setPlotMarker(const double xValue, bool isRisng = false, bool isFalling = false);
  bool removePlotMarker(const double xValue);
  void setPlotMarkerList(const KstMarkerList& newMarkers);
  const KstMarkerList plotMarkers(const double minX, const double maxX);
  const KstMarkerList& plotMarkers();
  bool nextMarker(const double currentPosition, double& marker);
  bool prevMarker(const double currentPosition, double& marker);
  void moveToNextMarker(KstViewWidget*);
  void moveToPrevMarker(KstViewWidget*);
  void moveSelfToCenter(double center);

  // curve related plot marker functions
  void setCurveToMarkers(KstVCurvePtr curve, bool risingDetect, bool fallingDetect);
  bool hasCurveToMarkers() const;
  void removeCurveToMarkers();
  KstVCurvePtr curveToMarkers() const;
  bool curveToMarkersRisingDetect() const;
  bool curveToMarkersFallingDetect() const;

  void setXOffsetMode(bool xOffsetMode) { _xOffsetMode = xOffsetMode; }
  void setYOffsetMode(bool yOffsetMode) { _yOffsetMode = yOffsetMode; }
  bool xOffsetMode() const { return _xOffsetMode; }
  bool yOffsetMode() const { return _yOffsetMode; }

  // plot grid lines
  void setGridLinesColor(const QColor& majorColor, const QColor& minorColor,
                         bool majorGridColorDefault, bool minorGridColorDefault);
  void setXGridLines(bool xMajor, bool xMinor);
  void setYGridLines(bool yMajor, bool yMinor);
  bool hasXMajorGrid() const { return _xMajorGrid; }
  bool hasYMajorGrid() const { return _yMajorGrid; }
  bool hasXMinorGrid() const { return _xMinorGrid; }
  bool hasYMinorGrid() const { return _yMinorGrid; }
  const QColor& majorGridColor() const { return _majorGridColor; }
  const QColor& minorGridColor() const { return _minorGridColor; }
  bool defaultMajorGridColor() const { return _majorGridColorDefault; }
  bool defaultMinorGridColor() const { return _minorGridColorDefault; }

  void setXMinorTicks(int minorTicks); //set to -1 for auto
  void setYMinorTicks(int minorTicks); //set to -1 for auto
  int xMinorTicks() const { return _xMinorTicks - 1; }
  int yMinorTicks() const { return _yMinorTicks - 1; }
  bool xMinorTicksAuto() const { return _reqXMinorTicks < 0; }
  bool yMinorTicksAuto() const { return _reqYMinorTicks < 0; }
  void setXMajorTicks(int majorTicks);
  void setYMajorTicks(int majorTicks);
  int xMajorTicks() const { return _xMajorTicks; }
  int yMajorTicks() const { return _yMajorTicks; }

  // set and get tick mark display options
  void setXTicksInPlot(bool yes);
  void setXTicksOutPlot(bool yes);
  void setYTicksInPlot(bool yes);
  void setYTicksOutPlot(bool yes);
  bool xTicksInPlot() const;
  bool xTicksOutPlot() const;
  bool yTicksInPlot() const;
  bool yTicksOutPlot() const;

  bool suppressTop() const;
  bool suppressBottom() const;
  bool suppressLeft() const;
  bool suppressRight() const;
  void setSuppressTop(bool yes);
  void setSuppressBottom(bool yes);
  void setSuppressLeft(bool yes);
  void setSuppressRight(bool yes);

  // change EXPRESSION mode to FIXED mode if possible
  void optimizeXExps();
  void optimizeYExps();

  // transformed opposite axes
  void setXTransformedExp(const QString& exp);
  void setYTransformedExp(const QString& exp);
  const QString& xTransformedExp() const;
  const QString& yTransformedExp() const;

  // axes reversal
  void setXReversed(bool yes);
  void setYReversed(bool yes);
  bool xReversed() const;
  bool yReversed() const;

  // change the color mode for the plot
  // If any of pointStylePriority, lineStylePriority, lineWidthPriority are == -1, then the
  // corresponding curve property is not cycled or altered. The remaining priorities must
  // be in the range [0, X-1] with no duplicates, where X = number of priorities != -1
  void changeToMonochrome(int pointStylePriority, int lineStylePriority, int lineWidthPriority,
                          int maxLineWidth, int pointDensity, bool forPrint);
  // undo changes made by changeToMonochrome
  // PRE: pointStylePriority, lineStylePriority, lineWidthPriority must have same values as when
  //      passed to changeToMonochrome (otherwise behaviour is not as expected)
  bool undoChangeToMonochrome(int pointStylePriority, int lineStylePriority, int lineWidthPriority);

  //convenience routines for working with viewLegends
  KstViewLegendPtr legend();

  KstViewLegendPtr getOrCreateLegend();
  
  virtual QRect contentsRect() const;

signals:
  void modified();

public slots:
  void copy();

  virtual void edit();
  virtual void copyObject();
  virtual void copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
  void draw(); // draw into back buffer
  void draw(KstPainter& p, double resolutionEnhancement = 1); // This actually does the drawing
  void paintSelf(KstPainter& p, const QRegion& bounds);
  void updateSelf();
  void editCurve(int id);
  void editObject(int id);
  void editVector(int id);
  void editMatrix(int id);
  void matchAxes(int id);
  void matchXAxis(int id);
  void fitCurve(int id);
  void filterCurve(int id);
  void removeCurve(int id);

  // used in layout mode
  bool showDialog(KstTopLevelViewPtr invoker, bool isNew = false);

protected slots:
  void menuMoveUp();
  void menuMoveDown();
  void menuMoveLeft();
  void menuMoveRight();
  void menuXZoomIn();
  void menuXZoomOut();
  void menuYZoomIn();
  void menuYZoomOut();
  void menuXZoomMax();
  void menuYZoomMax();
  void menuYZoomLocalMax();
  void menuZoomMax();
  void menuXLogSlot();
  void menuYLogSlot();
  void menuZoomPrev();
  void menuYZoomAc();
  void menuXNormalize();
  void menuYNormalize();
  void menuZoomSpikeInsensitiveMax();
  void menuNextMarker();
  void menuPrevMarker();

  void timezoneChanged(const QString& tz, int utcOffset);

private:
  // for backwards compatibility with old labels - convert an old saved label to a KstViewLabel
  KstViewLabelPtr convertLabelToViewLabel(const QDomElement &e);

  void updateDirtyFromLabels();

  template<class T> void updateTiedPlots(void (Kst2DPlot::*method)(T), T arg);
  template<class T> void updateTiedPlots(bool (Kst2DPlot::*method)(T), T arg);

  void setCursorPos(QWidget *view);
  void unsetCursorPos(QWidget *view);
  void drawCursorPos(QWidget *view);
  void drawCursorPos(QPainter& p);
  void updateMousePos(const QPoint& pos);
  void getCursorPos(const QPoint& pos, double &xpos, double &ypos, double &xmin, double &xmax, double &ymin, double& ymax);
  bool getNearestDataPoint(const QPoint& pos, QString& name, double &newxpos, double &newypos, double xpos, double ypos, double xmin, double xmax);
  void highlightNearestDataPoint(bool repaint, KstPainter *p, const QPoint& pos);
  void updateTieBox(QPainter&);
  bool legendUnder(QMouseEvent *e);
  KstMouseModeType globalZoomType() const;
  void setCursorForMode(QWidget *view, KstMouseModeType mode, const QPoint& pos);
  void setCursorForMode(QWidget *view); // gets the mode from the mouse
  void zoomRectUpdate(QWidget *view, KstMouseModeType t, int x, int y);
  inline void commonConstructor(const QString& in_tag,
                                KstScaleModeType xscale,
                                KstScaleModeType yscale,
                                double xmin,
                                double ymin,
                                double xmax,
                                double ymax,
                                bool x_log = false,
                                bool y_log = false);
  void setBorders(double& xleft_bdr_px, double& xright_bdr_px,
                         double& ytop_bdr_px, double& ybot_bdr_px,
                         TickParameters &tpx,  TickParameters &tpy,
                         QPainter& p, bool& bOffsetX, bool& bOffsetY,
                         double xtick_len_px, double ytick_len_px);
  void setTicks(double& tick, double& org,
                double max, double min, bool is_log, bool isX, int base);
  double convertTimeValueToJD(KstAxisInterpretation axisInterpretation, double valueIn);
  double convertTimeDiffValueToDays(KstAxisInterpretation axisInterpretation, double diffIn);
  void convertJDToDateString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, uint& length, double dJD);
  void convertTimeValueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, uint& length, double z, bool isLog);
  void convertDiffTimevalueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double& zdiff, double zbase, double zvalue, bool isLog, double scale);
  void genAxisTickLabelFullPrecision(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, uint& length, double z, bool isLog, bool isInterpreted);
  void genAxisTickLabelDifference(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, bool isLog, bool isInterpreted, double scale);
  void getPrefixUnitsScale(bool isInterpreted, KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool bLog, double Min, double Max, double& range, double& scale, int& base, QString& strPrefix, QString& strUnits);
  void genOffsetLabel(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, double Min, double Max, bool isLog, bool isInterpreted);
  void genAxisTickLabels(TickParameters &tp, double Min, double Max, bool bLog, KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool isX, bool isInterpreted, bool isOffsetMode);

  // helper function needed before setBorders
  void set2dPlotTickPix(double& xtickpix, double& ytickpix, int x_pix, int y_pix);

  void updateXYGuideline(QWidget *view, const QPoint& oldPos, const QPoint& newPos, const QRect& pr, KstMouseModeType gzType);

  // range and domain of plot: not plot dimentions
  double XMin, XMax, YMin, YMax;

  double _copy_x, _copy_y;
  double _cursor_x, _cursor_y;

  // grid lines
  QColor _majorGridColor;
  QColor _minorGridColor;
  bool _xMajorGrid : 1;
  bool _xMinorGrid : 1;
  bool _yMajorGrid : 1;
  bool _yMinorGrid : 1;
  bool _majorGridColorDefault : 1;
  bool _minorGridColorDefault : 1;

  bool _cursorOffset : 1;

  bool _xLog : 1;
  bool _yLog : 1;
  bool _isTied : 1;
  bool _zoomPaused : 1;
  bool _curveToMarkersRisingDetect : 1;
  bool _curveToMarkersFallingDetect : 1;
  bool _isLogLast : 1;

  bool _xOffsetMode : 1;
  bool _yOffsetMode : 1;

  // tick mark display options
  bool _xTicksInPlot : 1;
  bool _xTicksOutPlot : 1;
  bool _yTicksInPlot : 1;
  bool _yTicksOutPlot : 1;

  // suppress labels and axes
  bool _suppressTop : 1;
  bool _suppressBottom : 1;
  bool _suppressLeft : 1;
  bool _suppressRight : 1;

  bool _xMinParsedValid : 1, _xMaxParsedValid : 1, _yMinParsedValid : 1, _yMaxParsedValid : 1;
  bool _xTransformed : 1, _yTransformed : 1;

  bool _isXAxisInterpreted : 1, _isYAxisInterpreted : 1;

  // reverse axes
  bool _xReversed : 1, _yReversed : 1;

  bool _drawingGraphics : 1;

  KstAxisInterpretation _xAxisInterpretation, _yAxisInterpretation;
  KstAxisDisplay _xAxisDisplay, _yAxisDisplay;

  KstScaleModeType _xScaleMode, _yScaleMode;

  QPtrList<KstPlotScale> _plotScaleList;

  // hold last minimum and maximum y values that were plotted
  double _yPlottedMinCached, _yPlottedMaxCached;


  /** Stores border limits.  Set by KstPlot::paint().
      Stored here to be Used to determine mouse mode */
  QRect PlotRegion;
  QRect WinRegion;
  QRect PlotAndAxisRegion;

  void updateScale();
  void adjustFontSize();

  KstMouse _mouse;
  QMap<int, QString> _curveEditMap, _curveFitMap, _curveRemoveMap, _objectEditMap;
  QMap<int, QGuardedPtr<Kst2DPlot> > _plotMap;

  KstBackBuffer _buffer;
  QGuardedPtr<KstViewWidget> _menuView;
  QGuardedPtr<KstViewWidget> _layoutMenuView;
  QSize _oldSize;
  QRect _oldAlignment;
  double _m_X, _b_X, _m_Y, _b_Y;
  double _tickYLast, _stLast;
  int _autoTickYLast;

  // plot markers. This needs to remain sorted
  KstMarkerList _plotMarkers;
  // the vcurve used for auto marker generation
  KstVCurvePtr _curveToMarkers;
  void updateMarkersFromCurve();  //auto-generates the markers from _curveToMarkers

  // minor tick marks.
  int _reqXMinorTicks; // -1 means use auto
  int _reqYMinorTicks; // -1 means use auto
  int _xMinorTicks;
  int _yMinorTicks;

  int _xMajorTicks;
  int _yMajorTicks;

  KstPlotLabel *_xLabel, *_yLabel, *_topLabel, *_xTickLabel, *_yTickLabel, *_fullTickLabel;

  QValueStack<QColor> _colorStack;

  // for range expressions
  bool reparse(const QString& stringExp, Equation::Node** eqNode);
  bool reparseToText(QString& stringExp);
  Equation::Node* _xMinParsed;
  Equation::Node* _xMaxParsed;
  Equation::Node* _yMinParsed;
  Equation::Node* _yMaxParsed;

  // expressions for range and domain of plot
  QString _xMinExp, _xMaxExp, _yMinExp, _yMaxExp;

  // for the transformed axes
  QString _xTransformedExp, _yTransformedExp;

  // helper functions for draw(...)
  void plotLabels(QPainter& p, int x_px, int y_px, double xleft_bdr_px, double ytop_bdr_px);
  void plotAxes(QPainter& p, QRect& plotRegion,
      TickParameters tpx,
      double xleft_bdr_px, double xright_bdr_px,
      double x_orig_px, double xtick_px,
      double xtick_len_px, int x_px,
      TickParameters tpy,
      double ytop_bdr_px, double ybot_bdr_px,
      double y_orig_px, double ytick_px,
      double ytick_len_px, int y_px,
      bool offsetY);
  void plotGridLines(QPainter& p,
      double XTick, double xleft_bdr_px, double xright_bdr_px,
      double x_orig_px, double xtick_px, double xtick_len_px, int x_px,
      double YTick, double ytop_bdr_px, double ybot_bdr_px,
      double y_orig_px, double ytick_px, double ytick_len_px, int y_px);
  void plotPlotMarkers(QPainter& p,
      double b_X, double b_Y, double x_max, double x_min,
      double y_px, double ytop_bdr_px, double ybot_bdr_px, int penWidth);

  KstScaleModeType _xScaleModeDefault;
  KstScaleModeType _yScaleModeDefault;
};

typedef KstSharedPtr<Kst2DPlot> Kst2DPlotPtr;
typedef KstObjectMap<Kst2DPlotPtr> Kst2DPlotMap;

template<class T>
void Kst2DPlot::updateTiedPlots(void (Kst2DPlot::*method)(T), T arg) {
  if (isTied()) {
    Kst2DPlotList pl = globalPlotList();

    for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
      Kst2DPlotPtr p = *i;
      if (p->isTied() && p.data() != this) {
        (p->*method)(arg);
        p->pushScale();
        p->setDirty();
      }
    }
  }
}

template<class T>
void Kst2DPlot::updateTiedPlots(bool (Kst2DPlot::*method)(T), T arg) {
  if (isTied()) {
    Kst2DPlotList pl = globalPlotList();

    for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
      Kst2DPlotPtr p = *i;
      if (p->isTied() && p.data() != this) {
        if ((p->*method)(arg)) {
          p->pushScale();
          p->setDirty();
        }
      }
    }
  }
}

#endif
// vim: ts=2 sw=2 et
