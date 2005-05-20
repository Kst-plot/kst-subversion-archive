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
#include "kstplotdefines.h"
#include "kstimage.h"
#include "kstlabel.h"
#include "kstviewwidget.h"

#include <qvaluestack.h>

class KstLegend;
typedef KstSharedPtr<KstLegend> KstLegendPtr;
class Kst2DPlot;
typedef KstObjectList<KstSharedPtr<Kst2DPlot> > Kst2DPlotList;

enum KstScaleModeType { AUTO, AC, FIXED, AUTOUP, NOSPIKE, AUTOBORDER };

struct KstPlotScale {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  KstScaleModeType xscalemode;
  KstScaleModeType yscalemode;
  bool xlog, ylog;
};

struct KstMarker {
  double value;
  bool isRising;
  bool isFalling;
};

typedef QValueList<KstMarker> KstMarkerList;

enum KstMouseModeType { INACTIVE, XY_ZOOMBOX, Y_ZOOMBOX, X_ZOOMBOX, LABEL_TOOL, LAYOUT_TOOL };

struct KstMouse {
  KstMouse();
  KstMouseModeType mode;
  int label, minMove;
  QPoint lastLocation, pressLocation;  // for zooming primarily
  QPoint tracker; // for tracking the mouse location
  QRect plotGeometry;
  bool zooming() const;
  void zoomStart(KstMouseModeType t, const QPoint& location);
  void zoomUpdate(KstMouseModeType t, const QPoint& location);
  QRect mouseRect() const;
  bool rectBigEnough() const;
};


class Kst2DPlot : public KstPlotBase {
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
  static double timezoneHrs( );
  
  virtual UpdateType update(int update_counter);
  virtual void save(QTextStream& ts, const QString& indent = QString::null);
  virtual void saveTag(QTextStream& ts, const QString& indent = QString::null);

  virtual bool popupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);
  virtual bool layoutPopupMenu(KPopupMenu *menu, const QPoint& pos, KstViewObjectPtr topLevelParent);

  void drawDotAt(QPainter& p, double x, double y);
  void drawPlusAt(QPainter& p, double x, double y);
  void internalAlignment(KstPaintType type, QPainter& p, QRect& plotRegion);
  void addCurve(KstBaseCurvePtr curve);
  void addLabel(KstLabelPtr label);
  void removeCurve(KstBaseCurvePtr curve);
  void setXScaleMode(KstScaleModeType scalemode);
  void setYScaleMode(KstScaleModeType scalemode);

  void pushAdjustLineWidth(int adjustment);
  void popLineWidth();
  void pushCurveColor(const QColor& c);
  void popCurveColor();
  void pushPlotColors();
  void popPlotColors();

  /** Set the scale */
  void setScale(double xmin, double ymin, double xmax, double ymax);
  /** Set the X scale */
  bool setXScale(double xmin, double xmax);
  /** Set the y scale */
  bool setYScale(double ymin, double ymax);

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
  void setXAxisInterpretation( bool isXAxisInterpreted, KstAxisInterpretation xAxisInterpretation, KstAxisDisplay xAxisDisplay, bool xAxisTimezoneLocal, double xAxisTimezoneHrs);
  void getXAxisInterpretation( bool& isXAxisInterpreted, KstAxisInterpretation& xAxisInterpretation, KstAxisDisplay& xAxisDisplay, bool& xAxisTimezoneLocal, double& xAxisTimezoneHrs) const;
  void setYAxisInterpretation( bool isYAxisInterpreted, KstAxisInterpretation yAxisInterpretation, KstAxisDisplay yAxisDisplay, bool yAxisTimezoneLocal, double yAxisTimezoneHrs);
  void getYAxisInterpretation( bool& isYAxisInterpreted, KstAxisInterpretation& yAxisInterpretation, KstAxisDisplay& yAxisDisplay, bool& yAxisTimezoneLocal, double& yAxisTimezoneHrs ) const;
  void getScale(double& xmin, double& ymin, double& xmax, double& ymax) const;
  void getLScale(double& xmin, double& ymin, double& xmax, double& ymax) const;

  KstScaleModeType xScaleMode() const;
  KstScaleModeType yScaleMode() const;

  void setXLabel(const QString& xlabel);
  void setYLabel(const QString& ylabel);
  void setTopLabel(const QString& toplabel);
  void initFonts(const QFont& font, int font_size = 0);

  QRect GetPlotRegion() const;
  QRect GetWinRegion() const;
  QRect GetTieBoxRegion() const;
  QRect GetPlotAndAxisRegion() const;

  bool isTied() const;
  void toggleTied();
  void setTied(bool is_tied);

  virtual QString menuTitle() const;

  /** Labels */
  KstLabelPtr XLabel;
  KstLabelPtr YLabel;
  KstLabelPtr TopLabel;
  KstLabelPtr XTickLabel;
  KstLabelPtr YTickLabel;
  KstLabelPtr XFullTickLabel;
  KstLabelPtr YFullTickLabel;
  KstLegendPtr Legend;

  double _width;
  double _height;
  double _pos_x;
  double _pos_y;

  /** Arbitrary Labels */
  KstLabelList _labelList;

  KstBaseCurveList Curves;
  KstImageList _images;

  void GenerateDefaultLabels();

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
  virtual void dragEnterEvent(QWidget *view, QDragEnterEvent *e);
  virtual void dropEvent(QWidget *view, QDropEvent *e);
  virtual void dragMoveEvent(QWidget *view, QDragMoveEvent *e);
  virtual void wheelEvent(QWidget *view, QWheelEvent *e);
  virtual void setHasFocus(bool hasFocus);
  virtual void removeFocus(QPainter& p);

  void cancelZoom(QWidget *view);
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
  void setCurveToMarkers(KstBaseCurvePtr curve, bool risingDetect, bool fallingDetect);
  bool hasCurveToMarkers() const;
  void removeCurveToMarkers();
  KstBaseCurvePtr curveToMarkers() const;
  bool curveToMarkersRisingDetect() const;
  bool curveToMarkersFallingDetect() const;

  void setXOffsetMode(bool xOffsetMode) { _xOffsetMode = xOffsetMode; }
  void setYOffsetMode(bool yOffsetMode) { _yOffsetMode = yOffsetMode; }
  bool xOffsetMode() const { return _xOffsetMode; }
  bool yOffsetMode() const { return _yOffsetMode; }

  // images
  void addImage(KstImagePtr inImage, bool set_dirty = true);
  void removeImage(KstImagePtr inImage, bool set_dirty = true);
  void removeAllImages(bool set_dirty = true);
  bool hasImage(KstImagePtr inImage) const;

  // plot grid lines
  void setGridLines(bool xMajor, bool yMajor, bool xMinor, bool yMinor,
                    const QColor& majorColor, const QColor& minorColor,
                    bool majorGridColorDefault, bool minorGridColorDefault);
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

signals:
  void modified();

public slots:
  void copy();

  virtual void edit();
  virtual void copyObject();
  virtual void copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
  void draw(); // draw into back buffer
  void draw(QPainter &p, KstPaintType type, double resolutionEnhancement = 1); // This actually does the drawing
  virtual void paint(KstPaintType type, QPainter& p);
  void editCurve(int id);
  void editObject(int id);
  void editVector(int id);
  void editImage(int id);
  void matchAxis(int id);
  void fitCurve(int id);
  void filterCurve(int id);
  void removeCurve(int id);
  void removeImage(int id);

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

protected:
  virtual KstViewObjectFactoryMethod factory() const;

private:
  void updateDirtyFromLabels();

  template<class T> void updateTiedPlots(void (Kst2DPlot::*method)(T), T arg);
  template<class T> void updateTiedPlots(bool (Kst2DPlot::*method)(T), T arg);

  void setCursorPos(QWidget *view);
  void unsetCursorPos(QWidget *view);
  void drawCursorPos(QWidget *view);
  void drawCursorPos(QPainter& p);
  void updateMousePos(const QPoint& pos);
  bool getNearestDataPoint(const QPoint& pos, double& newxpos, double& newypos, QString& name, double& xmin, double &xmax, double& ymin, double& ymax);
  void highlightNearestDataPoint(bool repaint, QWidget *view, const QPoint& pos);
  void updateTieBox(QPainter&);
  int labelNumber(QMouseEvent *e);
  bool legendUnder(QMouseEvent *e);
  KstMouseModeType globalZoomType() const;
  void setCursorForMode(QWidget *view, KstMouseModeType mode, const QPoint& pos);
  void setCursorForMode(QWidget *view); // gets the mode from the mouse
  void zoomRectUpdate(QWidget *view, KstMouseModeType t, int x, int y);
  inline void commonConstructor(const QString& in_tag,
                                KstScaleModeType yscale,
                                KstScaleModeType xscale,
                                double xmin,
                                double ymin,
                                double xmax,
                                double ymax,
                                bool x_log = false,
                                bool y_log = false);
  void setBorders(double& xleft_bdr_px, double& xright_bdr_px,
                         double& ytop_bdr_px, double& ybot_bdr_px,
                         TickParameters &tpx,  TickParameters &tpy,
                         QPainter& p, bool& bOffsetX, bool& bOffsetY);
  void setTicks(double& tick, double& org,
                double max, double min, bool is_log, bool isX, int base);
  double convertTimeValueToJD(KstAxisInterpretation axisInterpretation, double valueIn);
  double convertTimeDiffValueToDays(KstAxisInterpretation axisInterpretation, double diffIn);
  void convertJDToDateString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, uint& length, double dJD);
  void convertTimeValueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, uint& length, double z, bool isLog);
  void convertDiffTimevalueToString(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, double& zdiff, double zbase, double zvalue, bool isLog, double scale);
  void genAxisTickLabelFullPrecision(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool axisTimezoneLocal, double axisTimezoneHrs, QString& label, uint& length, double z, bool isLog, bool isInterpreted);
  void genAxisTickLabelDifference(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, bool isLog, bool isInterpreted, double scale);
  void getPrefixUnitsScale(bool isInterpreted, KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, bool bLog, double Min, double Max, double& range, double& scale, int& base, QString& strPrefix, QString& strUnits);
  void genOffsetLabel(KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, QString& label, double zbase, double zvalue, double Min, double Max, bool isLog, bool isInterpreted);
  void genAxisTickLabels(QPainter& p, TickParameters &tp, double Min, double Max, bool bLog, KstLabelPtr Label,
                         KstAxisInterpretation axisInterpretation, KstAxisDisplay axisDisplay, 
                         bool axisTimezoneLocal, double axisTimezoneHours, 
                         bool isX, bool isInterpreted, bool isOffsetMode);

  // range and domain of plot: not plot dimentions
  double XMin, XMax, YMin, YMax;
  double _copy_x, _copy_y;
  double _cursor_x, _cursor_y;
  bool _cursorOffset;
  
  // grid lines
  QColor _majorGridColor;
  QColor _minorGridColor;
  bool _xMajorGrid : 1;
  bool _xMinorGrid : 1;
  bool _yMajorGrid : 1;
  bool _yMinorGrid : 1;
  bool _majorGridColorDefault : 1;
  bool _minorGridColorDefault : 1;

  bool _xLog : 1;
  bool _yLog : 1;
  bool _isTied : 1;
  bool _zoomPaused : 1;
  bool _curveToMarkersRisingDetect : 1;
  bool _curveToMarkersFallingDetect : 1;
  bool _isLogLast : 1;

  bool _xOffsetMode : 1;
  bool _yOffsetMode : 1;

  int _draggableLabel : 15; // I think this should be enough

  bool _isXAxisInterpreted, _isYAxisInterpreted;
  KstAxisInterpretation _xAxisInterpretation, _yAxisInterpretation;
  KstAxisDisplay _xAxisDisplay, _yAxisDisplay;
  bool _xAxisTimezoneLocal, _yAxisTimezoneLocal;
  double _xAxisTimezoneHrs, _yAxisTimezoneHrs;

  KstScaleModeType _xScaleMode, _yScaleMode;

  QPtrList<KstPlotScale> _plotScaleList;

  /** Stores border limits.  Set by KstPlot::paint().
      Stored here to be Used to determine mouse mode */
  QRect PlotRegion;
  QRect WinRegion;
  QRect PlotAndAxisRegion;

  void updateScale();

  KstMouse _mouse;
  QPoint _draggablePoint;

  QMap<int, QString> _curveEditMap, _curveFitMap, _curveRemoveMap, _imageEditMap, _imageRemoveMap, _objectEditMap;
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
  // the curve used for auto marker generation
  KstBaseCurvePtr _curveToMarkers;
  void updateMarkersFromCurve();  //auto-generates the markers from _curveToMarkers

  // minor tick marks.
  int _reqXMinorTicks; // -1 means use auto
  int _reqYMinorTicks; // -1 means use auto
  int _xMinorTicks;
  int _yMinorTicks;

  int _xMajorTicks;
  int _yMajorTicks;

  QValueStack<QColor> _colorStack;

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
      bool offsetX, bool offsetY);
  void plotGridLines(QPainter& p,
      double XTick, double xleft_bdr_px, double xright_bdr_px,
      double x_orig_px, double xtick_px, double xtick_len_px, int x_px,
      double YTick, double ytop_bdr_px, double ybot_bdr_px,
      double y_orig_px, double ytick_px, double ytick_len_px, int y_px);
  void plotCurves(QPainter& p,
      double Lx, double Hx, double Ly, double Hy,
      double m_X, double m_Y, double b_X, double b_Y, int penWidth);
  void plotImages(QPainter& p,
      double Lx, double Hx, double Ly, double Hy,
      double m_X, double m_Y, double b_X, double b_Y,
      double x_max, double y_max, double x_min, double y_min);
  void plotPlotMarkers(QPainter& p,
      double b_X, double b_Y, double x_max, double x_min,
      double y_px, double ytop_bdr_px, double ybot_bdr_px);
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
