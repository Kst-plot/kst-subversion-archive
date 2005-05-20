/**************************************************************************
              kstplotdialog_i.cpp - plot dialog: inherits designer dialog
                             -------------------
    begin                :  2000
    copyright            : (C) 2000-2002 by Barth Netterfield
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
 
// include files for Qt
#include <qcheckbox.h>
#include <qfontdatabase.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// include files for KDE
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>
#include <kmessagebox.h>

// application specific includes
#include "kstlegend.h"
#include "kstplotdefines.h"
#include "kstplotdialog_i.h"
#include "kstsettings.h"
#include "kstviewwindow.h"
#include "plotlistbox.h"

#ifndef DBL_DIG
  #define LABEL_PRECISION        15
#else
  #define LABEL_PRECISION        DBL_DIG
#endif

struct MajorTickSpacing {
  const char *label;
  int majorTickDensity;
};

struct LegendLayout {
  const char *label;
  KstLegendLayoutType type;
};

struct LegendAlignment {
  const char *label;
  KstLegendAlignmentType type;
};

const MajorTickSpacing MajorTickSpacings[] = {
  { I18N_NOOP("Coarse"), 2 },
  { I18N_NOOP("Default"), 5 },
  { I18N_NOOP("Fine"), 10 },
  { I18N_NOOP("Very fine"), 15 }
};

const LegendAlignment LegendAlignments[] = {
  { I18N_NOOP("Left"), AlignmentLeft },
  { I18N_NOOP("Center"), AlignmentCentre },
  { I18N_NOOP("Right"), AlignmentRight }
};

const unsigned int numMajorTickSpacings = sizeof( MajorTickSpacings ) / sizeof( MajorTickSpacing );
const unsigned int numLegendAlignments = sizeof( LegendAlignments ) / sizeof( LegendAlignment );

/*
 *  Constructs a KstPlotDialogI which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KstPlotDialogI::KstPlotDialogI(KstDoc *in_doc, QWidget* parent,
                               const char* name, bool modal, WFlags fl)
: KstPlotDialog(parent, name, modal, fl ) {
  QFontDatabase qfd;
  unsigned int i;

  doc = in_doc;
  setScalarDestTopLabel();

  FontComboBox->setEditable(false);
  LegendFontComboBox->setEditable(false);

  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(TabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTab()));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(_window, SIGNAL(activated(int)), this, SLOT(changeWindow()));
  connect(_newWindow, SIGNAL(clicked()), this, SLOT(newWindow()));

  // axes range
  connect(XAC, SIGNAL(toggled(bool)), XACRange, SLOT(setEnabled(bool)));
  connect(XFixed, SIGNAL(toggled(bool)), XFixedMin, SLOT(setEnabled(bool)));
  connect(XFixed, SIGNAL(toggled(bool)), XFixedMax, SLOT(setEnabled(bool)));
  connect(YAC, SIGNAL(toggled(bool)), YACRange, SLOT(setEnabled(bool)));
  connect(YFixed, SIGNAL(toggled(bool)), YFixedMin, SLOT(setEnabled(bool)));
  connect(YFixed, SIGNAL(toggled(bool)), YFixedMax, SLOT(setEnabled(bool)));
  
  // adding/removing curves
  connect(DisplayedCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(DisplayedCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(CurveUnplot, SIGNAL(clicked()), this, SLOT(removeDisplayedCurve()));
  connect(CurvePlot, SIGNAL(clicked()), this, SLOT(addDisplayedCurve()));

  connect(AutoLabel, SIGNAL(clicked()),
          this, SLOT(applyAutoLabels()));

  connect(ScalarList, SIGNAL(activated(int)),
          this, SLOT(insertCurrentScalar()));

  connect(YAxisText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestYLabel()));
  connect(YAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestYLabel()));
  connect(XAxisText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestXLabel()));
  connect(XAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestXLabel()));
  connect(TopLabelText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestTopLabel()));
  connect(TopLabelText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestTopLabel()));

  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXInterpret, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXDisplay, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), textLabelXDisplayAs, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), this, SLOT(updateXTimezoneButtons()));
  connect(_comboBoxXDisplay, SIGNAL(activated(int)), this, SLOT(updateXTimezoneButtons()));
  connect(_checkBoxXTimezoneLocal, SIGNAL(toggled(bool)), this, SLOT(updateXTimezoneButtons()));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), _comboBoxYInterpret, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), _comboBoxYDisplay, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), textLabelYDisplayAs, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), this, SLOT(updateYTimezoneButtons()));
  connect(_comboBoxYDisplay, SIGNAL(activated(int)), this, SLOT(updateYTimezoneButtons()));
  connect(_checkBoxYTimezoneLocal, SIGNAL(toggled(bool)), this, SLOT(updateYTimezoneButtons()));
  
  SampleLabel = new KstLabel("\0", CxCy, 0.0, 0.0, 0.0, true);
  FontComboBox->setFonts(qfd.families());

  LegendFontComboBox->setFonts(qfd.families());

  LegendAlignment->setEditable(false);
  for (i = 0; i < numLegendAlignments; i++) {
    LegendAlignment->insertItem(i18n(LegendAlignments[i].label));
  }

  for (i = 0; i < numMajorTickSpacings; i++) {
    _xMajorTickSpacing->insertItem(i18n(MajorTickSpacings[i].label));
    _yMajorTickSpacing->insertItem(i18n(MajorTickSpacings[i].label));
  }

  for (i = 0; i < numAxisInterpretations; i++) {
    _comboBoxXInterpret->insertItem(i18n(AxisInterpretations[i].label));
    _comboBoxYInterpret->insertItem(i18n(AxisInterpretations[i].label));
  }
  for (i = 0; i < numAxisDisplays; i++) {
    _comboBoxXDisplay->insertItem(i18n(AxisDisplays[i].label));
    _comboBoxYDisplay->insertItem(i18n(AxisDisplays[i].label));
  }

  _comboBoxXInterpret->setEnabled(false);
  _comboBoxXDisplay->setEnabled(false);
  textLabelXDisplayAs->setEnabled(false);
  updateXTimezoneButtons();

  _comboBoxYInterpret->setEnabled(false);
  _comboBoxYDisplay->setEnabled(false);
  textLabelYDisplayAs->setEnabled(false);
  updateYTimezoneButtons();
  
  
  appearanceThisPlot->setChecked(true);
  axesThisPlot->setChecked(true);
  rangeThisPlot->setChecked(true);
  markersThisPlot->setChecked(true);

  // plot markers
  connect(AddPlotMarker, SIGNAL(clicked()),
          this, SLOT(addPlotMarker()));
  connect(RemovePlotMarker, SIGNAL(clicked()),
          this, SLOT(removePlotMarker()));
  connect(RemoveAllPlotMarkers, SIGNAL(clicked()),
          this, SLOT(removeAllPlotMarkers()));
  connect(PlotMarkerList, SIGNAL(clicked(QListBoxItem*)),
          this, SLOT(updateButtons()));
  connect(PlotMarkerList, SIGNAL(selectionChanged()),
          this, SLOT(updateButtons()));
  connect(NewPlotMarker, SIGNAL(returnPressed()),
          this, SLOT(addPlotMarker()));
  connect(NewPlotMarker, SIGNAL(textChanged(const QString &)),
          this, SLOT(updateButtons()));
  connect(UseCurve, SIGNAL(clicked()),
          this, SLOT(updateButtons()));

  // grid lines
  connect(_xMajorGrid, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_yMajorGrid, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_xMinorGrid, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_yMinorGrid, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));

  connect(_xMinorTicksAuto, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_yMinorTicksAuto, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMajorGridColor, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMinorGridColor, SIGNAL(clicked()),
          this, SLOT(updateAxesButtons()));

  QColor defaultColor((KstSettings::globalSettings()->foregroundColor.red() + KstSettings::globalSettings()->backgroundColor.red())/2,
                      (KstSettings::globalSettings()->foregroundColor.green() + KstSettings::globalSettings()->backgroundColor.green())/2,
                      (KstSettings::globalSettings()->foregroundColor.blue() + KstSettings::globalSettings()->backgroundColor.blue())/2);
  _majorGridColor->setColor(defaultColor);
  _minorGridColor->setColor(defaultColor);
}

KstPlotDialogI::~KstPlotDialogI() {
}

void KstPlotDialogI::show_I(const QString& window, const QString& plot) {
  _windowName = window;
  _plotName = plot;

  show();
  raise();
  update();
}

void KstPlotDialogI::show_I() {
  show();
  raise();
  update();
}

void KstPlotDialogI::newTab() {
}

void KstPlotDialogI::updateWindow() {
  updateWindowList();
}

void KstPlotDialogI::setMajorSpacing(QComboBox* majorTicks, int spacing) {
  for (int i = 0; i < (int)numMajorTickSpacings; i++) {
    if (MajorTickSpacings[i].majorTickDensity <= spacing) {
      majorTicks->setCurrentItem(i);
    }
  }
}

void KstPlotDialogI::update(int new_index) {
  Q_UNUSED(new_index)

  if (!isShown()) {
    return;
  }

  updateScalarCombo();
  updateWindowList();
  updatePlotList();
  updateCurveLists();
  updatePlotMarkers();
  updateGridSettings();

  int i_plot;
  unsigned int i;

  if (Select->count() > 0) {
    i_plot = Select->currentItem();
  } else {
    i_plot = -1;
  }

  KstApp *app = KstApp::inst();
  KMdiChildView *c = app->findWindow(_window->currentText());

  if (c) {
    KstTopLevelViewPtr view = static_cast<KstViewWindow*>(c)->view();
    _reGrid->setChecked(view->onGrid());
    _plotColumns->setValue(view->columns());
  }

  if (i_plot >= 0 && c) {
    KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);
    if (!plot) {
      return;
    }

    // insert the current plot name in the plot name edit box
    Name->setText(plot->tagName());

    // fill the log axis check boxes
    XIsLog->setChecked(plot->isXLog());
    YIsLog->setChecked(plot->isYLog());

    _checkBoxXOffsetMode->setChecked(plot->xOffsetMode());
    _checkBoxYOffsetMode->setChecked(plot->yOffsetMode());

    double xmin, ymin, xmax, ymax;
    plot->getScale(xmin, ymin, xmax, ymax);

    XFixedMin->setText(QString::number(xmin, 'g', 16));
    XFixedMax->setText(QString::number(xmax, 'g', 16));
    YFixedMin->setText(QString::number(ymin, 'g', 16));
    YFixedMax->setText(QString::number(ymax, 'g', 16));

    XACRange->setText(QString::number(xmax - xmin, 'g', 16));
    YACRange->setText(QString::number(ymax - ymin, 'g', 16));

    switch (plot->xScaleMode()) {
      case AUTO:
        XAuto->setChecked(true);
        break;
      case AUTOBORDER:
        XAutoBorder->setChecked(true);
        break;
      case AC:
        XAC->setChecked(true);
        break;
      case FIXED:
        XFixed->setChecked(true);
        break;
      case AUTOUP:
        XAutoUp->setChecked(true);
        break;
      case NOSPIKE:
        XNoSpikes->setChecked(true);
        break;
      default:
        XAuto->setChecked(true);
        break;
    }

    switch (plot->yScaleMode()) {
      case AUTO:
        YAuto->setChecked(true);
        break;
      case AUTOBORDER:
        YAutoBorder->setChecked(true);
        break;
      case AC:
        YAC->setChecked(true);
        break;
      case FIXED:
        YFixed->setChecked(true);
        break;
      case AUTOUP:
        YAutoUp->setChecked(true);
        break;
      case NOSPIKE:
        YNoSpikes->setChecked(true);
        break;
      default:
        YAuto->setChecked(true);
        break;
    }

    // update the major and minor tick settings
    _xMinorTicks->setValue(plot->xMinorTicks());
    _yMinorTicks->setValue(plot->yMinorTicks());
    _xMinorTicksAuto->setChecked(plot->xMinorTicksAuto());
    _yMinorTicksAuto->setChecked(plot->yMinorTicksAuto());
    setMajorSpacing(_xMajorTickSpacing, plot->xMajorTicks());
    setMajorSpacing(_yMajorTickSpacing, plot->yMajorTicks());

    NumberFontSize->setValue(plot->XTickLabel->size());

    XAxisText->setText(plot->XLabel->text());
    XLabelFontSize->setValue(plot->XLabel->size());

    YAxisText->setText(plot->YLabel->text());
    YLabelFontSize->setValue(plot->YLabel->size());

    TopLabelText->setText(plot->TopLabel->text());
    TopLabelFontSize->setValue(plot->TopLabel->size());

    FontComboBox->setCurrentFont(plot->TopLabel->fontName());

    // update the x-axis interpretation
    KstAxisInterpretation xAxisInterpretation;
    KstAxisInterpretation yAxisInterpretation;
    KstAxisDisplay xAxisDisplay;
    KstAxisDisplay yAxisDisplay;
    bool xAxisInterpret;
    bool yAxisInterpret;
    bool xAxisTimezoneLocal;
    bool yAxisTimezoneLocal;
    double xAxisTimezoneHrs;
    double yAxisTimezoneHrs;
    
    plot->getXAxisInterpretation(xAxisInterpret, xAxisInterpretation, xAxisDisplay, 
                                 xAxisTimezoneLocal, xAxisTimezoneHrs);
    _checkBoxXInterpret->setChecked(xAxisInterpret);
    _comboBoxXInterpret->setEnabled(xAxisInterpret);
    _comboBoxXDisplay->setEnabled(xAxisInterpret);
    textLabelXDisplayAs->setEnabled(xAxisInterpret);
    _checkBoxXTimezoneLocal->setChecked(xAxisTimezoneLocal);
    lineEditXTimezone->setText(QString::number(xAxisTimezoneHrs));
            
    if (xAxisInterpret) {
      for (i=0; i<numAxisInterpretations; i++) {
        if (AxisInterpretations[i].type == xAxisInterpretation) {
          _comboBoxXInterpret->setCurrentItem(i);
          break;
        }
      }
      for (i=0; i<numAxisDisplays; i++) {
        if (AxisDisplays[i].type == xAxisDisplay) {
          _comboBoxXDisplay->setCurrentItem(i);
          break;
        }
      }
    } else {
      _comboBoxXInterpret->setCurrentItem(0);
      _comboBoxXDisplay->setCurrentItem(0);
    }
    updateXTimezoneButtons();
    
    plot->getYAxisInterpretation(yAxisInterpret, yAxisInterpretation, yAxisDisplay,
                                 yAxisTimezoneLocal, yAxisTimezoneHrs);
    _checkBoxYInterpret->setChecked(yAxisInterpret);
    _comboBoxYInterpret->setEnabled(yAxisInterpret);
    _comboBoxYDisplay->setEnabled(yAxisInterpret);
    textLabelYDisplayAs->setEnabled(yAxisInterpret);
    _checkBoxYTimezoneLocal->setChecked(yAxisTimezoneLocal);
    lineEditYTimezone->setText(QString::number(yAxisTimezoneHrs));
    
    if (yAxisInterpret) {
      for (i=0; i<numAxisInterpretations; i++) {
        if (AxisInterpretations[i].type == yAxisInterpretation) {
          _comboBoxYInterpret->setCurrentItem(i);
          break;
        }
      }
      for (i=0; i<numAxisDisplays; i++) {
        if (AxisDisplays[i].type == yAxisDisplay) {
          _comboBoxYDisplay->setCurrentItem(i);
          break;
        }
      }
    } else {
      _comboBoxYInterpret->setCurrentItem(0);
      _comboBoxYDisplay->setCurrentItem(0);
    }
    updateYTimezoneButtons();
    
    // initialize all the legend settings for the current plot
    LegendFontComboBox->setCurrentFont(plot->Legend->fontName());
    LegendFontSize->setValue(plot->Legend->size());
    ShowLegend->setChecked(plot->Legend->getShow());

    legendInFront->setChecked(plot->Legend->getFront());
    legendColors->setForeground(plot->Legend->getColorForeground());
    legendColors->setBackground(plot->Legend->getColorBackground());

    KstLegendAlignmentType legendAlignmentType = plot->Legend->alignment();
    for (i=0; i<numLegendAlignments; i++) {
      if (LegendAlignments[i].type == legendAlignmentType) {
        LegendAlignment->setCurrentItem(i);
        break;
      }
    }

    // initialize the plot color widget
    plotColors->setForeground(plot->foregroundColor());
    plotColors->setBackground(plot->backgroundColor());
  } else {
    FontComboBox->setCurrentFont(KstApp::inst()->defaultFont());
    LegendFontComboBox->setCurrentFont(KstApp::inst()->defaultFont());
  }

  updateButtons();
  updateAxesButtons();
}

void KstPlotDialogI::fill2DPlotList(Kst2DPlotList& plots) {
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  while (it->currentItem()) {
    KstViewWindow *c = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (c) {
      KstPlotBaseList allplots = c->view()->findChildrenType<KstPlotBase>(true);
      for (KstPlotBaseList::Iterator i = allplots.begin(); i != allplots.end(); ++i) {
        Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(*i);
        if (plot) {
          plots += plot;
        }
      }
    }
    it->next();
  }
  app->deleteIterator(it);
}


void KstPlotDialogI::applyAppearance(Kst2DPlotPtr plot) {
  Kst2DPlotList plots;
  Kst2DPlotPtr plotExtra;
  KstLegendLayoutType layout = LeftTop;
  KstLegendAlignmentType alignment = AlignmentRight;

  // only apply label text to this plot, despite radio button values
  plot->XLabel->setText(XAxisText->text());
  plot->YLabel->setText(YAxisText->text());
  plot->TopLabel->setText(TopLabelText->text());

  if (appearanceThisPlot->isChecked()) {
    plots += plot;
  } else if (appearanceThisWindow->isChecked()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (c) {
      plots  = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);
    }
  } else {
    fill2DPlotList(plots);
  }

  alignment = LegendAlignments[LegendAlignment->currentItem()].type;

  for (uint i = 0; i < plots.size(); i++) {
    plotExtra = plots[i];

    plotExtra->setForegroundColor(plotColors->foreground());
    plotExtra->setBackgroundColor(plotColors->background());

    plotExtra->XLabel->setFontName(FontComboBox->currentText());
    plotExtra->XLabel->setSize(XLabelFontSize->value());

    plotExtra->YLabel->setFontName(FontComboBox->currentText());
    plotExtra->YLabel->setSize(YLabelFontSize->value());

    plotExtra->TopLabel->setFontName(FontComboBox->currentText());
    plotExtra->TopLabel->setSize(TopLabelFontSize->value());

    plotExtra->XTickLabel->setFontName(FontComboBox->currentText());
    plotExtra->XTickLabel->setSize(NumberFontSize->value());
    plotExtra->XTickLabel->setRotation(_spinBoxXAngle->value());
    plotExtra->XFullTickLabel->setFontName(FontComboBox->currentText());
    plotExtra->XFullTickLabel->setSize(NumberFontSize->value());
    plotExtra->YTickLabel->setFontName(FontComboBox->currentText());
    plotExtra->YTickLabel->setSize(NumberFontSize->value());
    plotExtra->YTickLabel->setRotation(_spinBoxYAngle->value());
    plotExtra->YFullTickLabel->setFontName(FontComboBox->currentText());
    plotExtra->YFullTickLabel->setSize(NumberFontSize->value());

    plotExtra->Legend->setFontName(LegendFontComboBox->currentText());
    plotExtra->Legend->setSize(LegendFontSize->value());
    plotExtra->Legend->setShow(ShowLegend->isChecked());
    plotExtra->Legend->setLayout(layout);
    plotExtra->Legend->setAlignment(alignment);
    plotExtra->Legend->setFront(legendInFront->isChecked());
    plotExtra->Legend->setColorForeground(legendColors->foreground());
    plotExtra->Legend->setColorBackground(legendColors->background());
  }
}

void KstPlotDialogI::applyAxes(Kst2DPlotPtr plot) {
  Kst2DPlotList plots;
  Kst2DPlotPtr  plotExtra;

  if (axesThisPlot->isChecked()) {
    plots += plot;
  } else if (axesThisWindow->isChecked()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (c) {
      plots  = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);
    }
  } else {
    fill2DPlotList(plots);
  }

  for (uint i = 0; i < plots.size(); i++) {    
    plotExtra = plots[i];
    plotExtra->setLog(XIsLog->isChecked(), YIsLog->isChecked());

    plotExtra->setXOffsetMode(_checkBoxXOffsetMode->isChecked());
    plotExtra->setYOffsetMode(_checkBoxYOffsetMode->isChecked());

    if (_checkBoxXInterpret->isChecked()) {      
      double dTimezone = lineEditXTimezone->text().toDouble();
    
      plotExtra->setXAxisInterpretation(true, 
            AxisInterpretations[_comboBoxXInterpret->currentItem()].type,
            AxisDisplays[_comboBoxXDisplay->currentItem()].type, 
            _checkBoxXTimezoneLocal->isChecked(),
            dTimezone );
    } else {
      plotExtra->setXAxisInterpretation(false, 
                                        AXIS_INTERP_CTIME, 
                                        AXIS_DISPLAY_YEAR,
                                        true,
                                        0.0);
    }

    if (_checkBoxYInterpret->isChecked()) {
      double dTimezone = lineEditYTimezone->text().toDouble();
      
      plotExtra->setYAxisInterpretation(true, 
            AxisInterpretations[_comboBoxYInterpret->currentItem()].type,
            AxisDisplays[_comboBoxYDisplay->currentItem()].type,
            _checkBoxYTimezoneLocal->isChecked(),
            dTimezone );
    } else {
      plotExtra->setYAxisInterpretation(false, 
                                        AXIS_INTERP_CTIME, 
                                        AXIS_DISPLAY_YEAR,
                                        true,
                                        0.0);
    }
        
    // minor tick settings.
    if (_xMinorTicksAuto->isChecked()) {
      plotExtra->setXMinorTicks(-1);
    } else {
      plotExtra->setXMinorTicks(_xMinorTicks->value());
    }
    if (_yMinorTicksAuto->isChecked()) {
      plotExtra->setYMinorTicks(-1);
    } else {
      plotExtra->setYMinorTicks(_yMinorTicks->value());
    }

    // major tick settings.
    plotExtra->setXMajorTicks(
       MajorTickSpacings[_xMajorTickSpacing->currentItem()].majorTickDensity);
    plotExtra->setYMajorTicks(
       MajorTickSpacings[_yMajorTickSpacing->currentItem()].majorTickDensity);

    // gridlines.
    plotExtra->setGridLines(_xMajorGrid->isChecked(),
                     _yMajorGrid->isChecked(),
                     _xMinorGrid->isChecked(),
                     _yMinorGrid->isChecked(),
                     _majorGridColor->color(),
                     _minorGridColor->color(),
                     _checkBoxDefaultMajorGridColor->isChecked(),
                     _checkBoxDefaultMinorGridColor->isChecked());
  }
}


void KstPlotDialogI::applyRange(Kst2DPlotPtr plot) {
  Kst2DPlotList plots;
  Kst2DPlotPtr  plotExtra;

  if (rangeThisPlot->isChecked()) {
    plots += plot;
  } else if (rangeThisWindow->isChecked()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (c) {
      plots  = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);
    }
  } else {
    fill2DPlotList(plots);
  }

  for (uint i = 0; i < plots.size(); i++) {
    plotExtra = plots[i];

    // do X Scale
    if (XAC->isChecked()) {
      plotExtra->setXScaleMode(AC);
      plotExtra->setXScale(0, XACRange->text().toDouble());
    } else if (XFixed->isChecked()) {
      plotExtra->setXScaleMode(FIXED);
      plotExtra->setXScale(XFixedMin->text().toDouble(), 
			   XFixedMax->text().toDouble());
    } else if (XAutoUp->isChecked()) {
      plotExtra->setXScaleMode(AUTOUP);
    } else if (XAuto->isChecked()) {
      plotExtra->setXScaleMode(AUTO);
    } else if (XAutoBorder->isChecked()) {
      plotExtra->setXScaleMode(AUTOBORDER);
    } else if (XNoSpikes->isChecked()) {
      plotExtra->setXScaleMode(NOSPIKE);
    } else {
      KstDebug::self()->log(i18n( "Internal error: No X scale type checked in %1." ).arg(Select->currentText()), KstDebug::Error);
    }

    // do Y Scale
    if (YAC->isChecked()) {
      plotExtra->setYScaleMode(AC);
      plotExtra->setYScale(0, YACRange->text().toDouble());
    } else if (YFixed->isChecked()) {
      plotExtra->setYScaleMode(FIXED);
      plotExtra->setYScale(YFixedMin->text().toDouble(), 
			   YFixedMax->text().toDouble());
    } else if (YAutoUp->isChecked()) {
      plotExtra->setYScaleMode(AUTOUP);
    } else if (YAuto->isChecked()) {
      plotExtra->setYScaleMode(AUTO);
    } else if (YAutoBorder->isChecked()) {
      plotExtra->setYScaleMode(AUTOBORDER);
    } else if (YNoSpikes->isChecked()) {
      plotExtra->setYScaleMode(NOSPIKE);
    } else {
      KstDebug::self()->log(i18n( "Internal error: No Y scale type checked in %1." ).arg(Select->currentText()), KstDebug::Error);
    }
  }
}

void KstPlotDialogI::applySettings(KMdiChildView *c, Kst2DPlotPtr plot) {
  if (_reGrid->isChecked()) {
    static_cast<KstViewWindow*>(c)->view()->cleanup(_plotColumns->value());
  }

  // add the curves
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  plot->Curves.clear();
  for (unsigned i = 0; i < DisplayedCurveList->count(); i++) {
    KstBaseCurveList::Iterator it = curves.findTag(DisplayedCurveList->text(i));
    if (it != curves.end()) {
      plot->Curves.append(*it);
    }
  }
  curves.clear();

  // add the images
  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);
  plot->removeAllImages();
  for (unsigned i = 0; i < DisplayedCurveList->count(); i++) {
    KstImageList::Iterator it = images.findTag(DisplayedCurveList->text(i));
    if (it != images.end()) {
      plot->addImage(*it);
    }
  }
  images.clear();

  applyAppearance(plot);
  applyAxes(plot);
  applyRange(plot);
  applyPlotMarkers(plot);

  plot->setDirty();
  static_cast<KstViewWindow*>(c)->view()->paint(P_PLOT);

  // make sure we paint all the necessary windows
  if (appearanceAll->isChecked() ||
      axesAll->isChecked() ||
      rangeAll->isChecked()) {
    KMdiIterator<KMdiChildView*> *it = KstApp::inst()->createIterator();
    while (it->currentItem()) {
      KstViewWindow *c = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (c) {
        c->view()->paint(P_PLOT);
      }
      it->next();
    }
    KstApp::inst()->deleteIterator(it);
  }

  _plotName = Name->text().stripWhiteSpace();
  update();

  emit docChanged();
}


void KstPlotDialogI::new_I() {
  if (checkPlotName()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (!c) {
      QString name = KstApp::inst()->newWindow();
      if (name.isEmpty()) {
        return;
      }
      c = KstApp::inst()->findWindow(name);
      if (c) {
        _window->insertItem(name);
        _window->setCurrentText(name);
      }
    }

    if (c) {
      Kst2DPlotPtr plot = static_cast<KstViewWindow*>(c)->view()->createPlot<Kst2DPlot>(Name->text().stripWhiteSpace());
      if (plot) {
        applySettings(c, plot);
      }
    }
  }
}

void KstPlotDialogI::edit_I() {
  int index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(this, i18n("You need to select an active plot to edit."));
    return;
  }

  if (Name->text().stripWhiteSpace() != Select->currentText() && !checkPlotName()) {
    return;
  }

  KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
  if (c) {
    Kst2DPlotList plots = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);

    if (index >= (int)plots.count()) {
      new_I();
    } else {
      Kst2DPlotPtr plot = *plots.at(index);

      plot->setTagName(Name->text().stripWhiteSpace());

      applySettings(c, plot);
    }
  }
}

void KstPlotDialogI::delete_I() {
  int index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(this, i18n("You need to select an active plot to delete."));
    return;
  }

  KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
  if (!c) {
    return;
  }

  KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
  static_cast<KstViewWindow*>(c)->view()->removeChild(obj);

  if (_reGrid->isChecked()) {
    static_cast<KstViewWindow*>(c)->view()->cleanup(_plotColumns->value());
  }

  update();

  static_cast<KstViewWindow*>(c)->view()->paint(P_PLOT);

  emit docChanged();
}


void KstPlotDialogI::updatePlotMarkers() {
  KstApp* app = KstApp::inst();

  PlotMarkerList->clear();

  KMdiChildView *c = app->findWindow(_window->currentText());
  if (Select->count() > 0 && c) {
    KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);

    if (plot) {
      for (KstMarkerList::ConstIterator it = plot->plotMarkers().begin(); it != plot->plotMarkers().end(); it++) {
        if ((*it).isRising) {
          PlotMarkerList->insertItem(QString("%1 [%2]").arg(QString::number((*it).value, 'g', LABEL_PRECISION)).arg(i18n("rising")));
        } else if ((*it).isFalling) {
          PlotMarkerList->insertItem(QString("%1 [%2]").arg(QString::number((*it).value, 'g', LABEL_PRECISION)).arg(i18n("falling")));
        } else {
          PlotMarkerList->insertItem(QString::number((*it).value, 'g', LABEL_PRECISION));
        }
      }

      // update the auto-generation settings
      KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
      CurveCombo->clear();
      for (KstBaseCurveList::iterator curves_iter = curves.begin(); curves_iter != curves.end(); curves_iter++) {
        (*curves_iter)->readLock();
        CurveCombo->insertItem((*curves_iter)->tagName());
        (*curves_iter)->readUnlock();
      }
      if (plot->hasCurveToMarkers()) {
        UseCurve->setChecked(true);
        Both->setChecked(false);
        Falling->setChecked(false);
        Rising->setChecked(false);
        if (plot->curveToMarkersFallingDetect()) {
          if (plot->curveToMarkersRisingDetect()) {
            Both->setChecked(true);
          } else {
            Falling->setChecked(true);
          }
        } else {
          Rising->setChecked(true);
        }
        int curveComboIndex;
        for (curveComboIndex = 0; curveComboIndex < CurveCombo->count(); curveComboIndex++) {
          if (CurveCombo->text(curveComboIndex) == plot->curveToMarkers()->tagName()) {
            break;
          }
        }
        CurveCombo->setCurrentItem(curveComboIndex);
      }
    }
  }

}


void KstPlotDialogI::removeDisplayedCurve() {
  uint count = DisplayedCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (DisplayedCurveList->isSelected(i)) {
        AvailableCurveList->insertItem(DisplayedCurveList->text(i));
        DisplayedCurveList->removeItem(i);
      }
    }
    updateButtons();
  }
}


void KstPlotDialogI::removeDisplayedCurve(QListBoxItem* item){
  int index = DisplayedCurveList->index(item);
  if (index != -1) {
    QString str = DisplayedCurveList->text(index);
    DisplayedCurveList->removeItem(index);
    AvailableCurveList->insertItem(str);
    updateButtons();
  }
}


void KstPlotDialogI::addDisplayedCurve() {
  uint count = AvailableCurveList->count();

  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (AvailableCurveList->isSelected(i)) {
        DisplayedCurveList->insertItem(AvailableCurveList->text(i));
        AvailableCurveList->removeItem(i);
      }
    }
    updateButtons();
  }
}


void KstPlotDialogI::addDisplayedCurve(QListBoxItem* pItem){
  int index = AvailableCurveList->index(pItem);
  if (index != -1) {
    QString str = AvailableCurveList->text(index);
    AvailableCurveList->removeItem(index);
    DisplayedCurveList->insertItem(str);
    updateButtons();
  }
}


void KstPlotDialogI::applyAutoLabels() {
  KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
  if (!c) {
    return;
  }

  KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
  Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);
  if (plot) {
    plot->GenerateDefaultLabels();
    update();
  }
}


void KstPlotDialogI::updateButtons() {
  bool selected = false;
  uint count = AvailableCurveList->count();

  for (uint i = 0; i < count; i++) {
    if (AvailableCurveList->isSelected(i)) {
      selected = true;
    }
  }
  if (selected && !CurvePlot->isEnabled()) {
    CurvePlot->setEnabled(true);
  } else if (!selected && CurvePlot->isEnabled()) {
    CurvePlot->setEnabled(false);
  }

  selected = false;
  count = DisplayedCurveList->count();
  for (uint i = 0; i < count; i++) {
    if (DisplayedCurveList->isSelected(i)) {
      selected = true;
    }
  }
  if (selected && !CurveUnplot->isEnabled()) {
    CurveUnplot->setEnabled(true);
  } else if (!selected && CurveUnplot->isEnabled()) {
    CurveUnplot->setEnabled(false);
  }

  // updates for Plot Markers tab
  AddPlotMarker->setEnabled(!NewPlotMarker->text().isEmpty());

  selected = false;
  count = PlotMarkerList->count();
  for (uint i = 0; i < count; i++) {
    if (PlotMarkerList->isSelected(i)) {
      selected = true;
    }
  }
  RemovePlotMarker->setEnabled(selected);
  RemoveAllPlotMarkers->setEnabled(count > 0);

  // updates for auto-generation marker curve section
  CurveCombo->setEnabled(UseCurve->isChecked());
  Rising->setEnabled(UseCurve->isChecked());
  Falling->setEnabled(UseCurve->isChecked());
  Both->setEnabled(UseCurve->isChecked());
}


void KstPlotDialogI::updateCurveLists() {
  KstApp* app = KstApp::inst();
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);

  DisplayedCurveList->clear();
  AvailableCurveList->clear();

  KMdiChildView *c = app->findWindow(_window->currentText());
  if (Select->count() && c) {
    KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);

    if (plot) {
      // add curves while retaining the order in the plot
      for (KstBaseCurveList::iterator it = plot->Curves.begin(); it != plot->Curves.end(); it++) {
        (*it)->readLock();
        DisplayedCurveList->insertItem((*it)->tagName());
        (*it)->readUnlock();
      }
      for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); it++) {
        (*it)->readLock();
        if (plot->Curves.find(*it) == plot->Curves.end()) {
          AvailableCurveList->insertItem((*it)->tagName());
        }
        (*it)->readUnlock();
      }

      // add images while retaining the order in the plot
      for (KstImageList::iterator it = plot->_images.begin(); it != plot->_images.end(); it++) {
        (*it)->readLock();
        DisplayedCurveList->insertItem((*it)->tagName());
        (*it)->readUnlock();
      }
      for (KstImageList::iterator it = images.begin(); it != images.end(); it++) {      
        (*it)->readLock();
        if (!plot->hasImage(*it)) {
          AvailableCurveList->insertItem((*it)->tagName());
        }
        (*it)->readUnlock();
      }
    }
  } else {
    for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); it++) {
      (*it)->readLock();
      AvailableCurveList->insertItem((*it)->tagName());
      (*it)->readUnlock();
    }

    // add images as well
    for (KstImageList::iterator it = images.begin(); it != images.end(); it++) {
      (*it)->readLock();
      AvailableCurveList->insertItem((*it)->tagName());
      (*it)->readUnlock();
    }
  }
}

void KstPlotDialogI::updateScalarCombo() {
  ScalarList->clear();
  KST::scalarList.lock().readLock();
  for (int index = 0; index < (int)KST::scalarList.count(); index++) {
    KstScalarPtr s = KST::scalarList[index];
    s->readLock();
    ScalarList->insertItem(s->tagLabel());
    s->readUnlock();
  }
  KST::scalarList.lock().readUnlock();
}

void KstPlotDialogI::updateWindowList() {
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  QString oldText;

  if (_window->count()) {
    oldText = _window->currentText();
  }

  _window->clear();

  while (it->currentItem()) {
    KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (v) {
      _window->insertItem(v->caption());
      if (!oldText.isEmpty() && oldText == v->caption()) {
        _window->setCurrentItem(_window->count() - 1);
      }
      if (!_windowName.isEmpty() && _windowName == v->caption()) {
        _window->setCurrentItem(_window->count() - 1);
        oldText = QString::null;
      }
    }
    it->next();
  }

  app->deleteIterator(it);

  _windowName = QString::null;
}

void KstPlotDialogI::changeWindow() {
  updatePlotList();
  update(-1);
}

void KstPlotDialogI::updatePlotList() {
  KstApp *app = KstApp::inst();
  QString oldName;

  if (Select->count()) {
    oldName = Select->currentText();
  }

  Select->clear();

  KMdiChildView *c = app->findWindow(_window->currentText());
  if (c) {
    Kst2DPlotList plots = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);

    for (Kst2DPlotList::Iterator i = plots.begin(); i != plots.end(); ++i) {
      Select->insertItem((*i)->tagName());
      if (!oldName.isEmpty() && oldName == (*i)->tagName()) {
        Select->setCurrentItem(Select->count() - 1);
      }
      if (!_plotName.isEmpty() && _plotName == (*i)->tagName()) {
        Select->setCurrentItem(Select->count() - 1);
        oldName = QString::null;
      }
    }
  }

  _plotName = QString::null;
}

void KstPlotDialogI::addPlotMarker() {
  // silently do nothing if there is no text, to
  // be consistent with "Add" button disabled
  if (!NewPlotMarker->text().isEmpty()) {
    bool ok;
    double newMarkerVal = NewPlotMarker->text().toDouble(&ok);

    if (ok) {
      uint i = 0;
      QString stringnum;

      stringnum.setNum(newMarkerVal, 'g', LABEL_PRECISION);
      while (i < PlotMarkerList->count() && PlotMarkerList->text(i).toDouble() < newMarkerVal) {
        i++;
      }
      if (i == PlotMarkerList->count()) {
        PlotMarkerList->insertItem(stringnum, -1);
        NewPlotMarker->clear();
      } else if (newMarkerVal != PlotMarkerList->text(i).toDouble()) {
        PlotMarkerList->insertItem(stringnum, i);
        NewPlotMarker->clear();
      } else {
        KMessageBox::sorry(this,
                         i18n("A plot marker with equal (or very close) value already exists."),
                         i18n("Kst"));
        NewPlotMarker->selectAll();
      }
    } else {
      KMessageBox::sorry(this,
                       i18n("The text you have entered is not a valid number. Please enter a valid number."),
                       i18n("Kst"));
      NewPlotMarker->selectAll();
    }
  }
}

void KstPlotDialogI::removePlotMarker() {
  uint count = PlotMarkerList->count();
  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (PlotMarkerList->isSelected(i)) {
        PlotMarkerList->removeItem(i);
      }
    }
    updateButtons();
  }
}

void KstPlotDialogI::removeAllPlotMarkers() {
  uint count = PlotMarkerList->count();
  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      PlotMarkerList->removeItem(i);
    }
    updateButtons();
  }
}

void KstPlotDialogI::applyPlotMarkers(Kst2DPlotPtr plot) {
  KstMarkerList newMarkers;
  KstMarker marker;
  Kst2DPlotList plots;
  Kst2DPlotPtr plotExtra;
  unsigned int i;

  if (markersThisPlot->isChecked()) {
    plots += plot;
  } else if (markersThisWindow->isChecked()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (c) {
      plots  = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);
    }
  } else {
    fill2DPlotList(plots);
  }

  for (i = 0; i < PlotMarkerList->count(); i++) {
    marker.value = PlotMarkerList->text(i).toDouble();
    marker.isRising = false;
    marker.isFalling = false;
    if (PlotMarkerList->text(i).find( i18n("rising")) != -1) {
      marker.isRising = true;
    } else if (PlotMarkerList->text(i).find( i18n("falling")) != -1) {
      marker.isFalling = true;
    }
    newMarkers.append(marker);
  }

  for (i = 0; i < plots.size(); i++) {
    plotExtra = plots[i];
    plotExtra->setPlotMarkerList(newMarkers);

    // apply the auto-generation settings
    if (UseCurve->isChecked()) {
      KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
      KstBaseCurveList::iterator curves_iter = curves.findTag(CurveCombo->currentText());
      bool falling = Falling->isChecked(), rising = Rising->isChecked();
      if (Both->isChecked()) {
        falling = true;
        rising = true;
      }
      plotExtra->setCurveToMarkers(*curves_iter, rising, falling);
    } else {
      plotExtra->removeCurveToMarkers();
    }
  }
}

void KstPlotDialogI::newWindow() {
  QString name = KstApp::inst()->newWindow();
  if (!name.isEmpty()) {
    _window->setCurrentText(name);
    updatePlotList();
  }
}

bool KstPlotDialogI::checkPlotName() {
  bool rc = true;

  // first check if name is blank
  if (!Name->text().stripWhiteSpace().isEmpty()) {
    // now check if it's a duplicate
    KstApp *app = KstApp::inst();
    KMdiIterator<KMdiChildView*> *iter;
    QString name = Name->text().stripWhiteSpace();
    KstViewObjectPtr viewObject;

    iter = app->createIterator();
    if (iter) {
      while (iter->currentItem()) {
        KMdiChildView *childview = iter->currentItem();
        KstViewWindow *viewwindow = dynamic_cast<KstViewWindow*>(childview);
        if (viewwindow) {
          viewObject = viewwindow->view()->findChild(name);
          if (viewObject) {
            QString message = i18n("%1: not a unique plot name.\n"
                "Change it to a unique name.").arg(Name->text().stripWhiteSpace());

            KMessageBox::sorry(this, message);
            Name->selectAll();
            Name->setFocus();

            rc = false;
            break;
          }
        }
        iter->next();
      }
      app->deleteIterator(iter);
    }
  } else {
    KMessageBox::sorry(this, i18n("The plot name is empty. Please enter a unique name for the plot."));
    Name->setFocus();
    rc = false;
  }

  return rc;
}


void KstPlotDialogI::insertCurrentScalar() {
  ScalarDest->insert(ScalarList->currentText());
}

void KstPlotDialogI::updateGridSettings() {
  KstApp* app = KstApp::inst();
  KMdiChildView *c = app->findWindow(_window->currentText());
  if (Select->count() > 0 && c) {
    KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);
    if (plot) {
      // update the checks and buttons
      _xMajorGrid->setChecked(plot->hasXMajorGrid());
      _xMinorGrid->setChecked(plot->hasXMinorGrid());
      _yMajorGrid->setChecked(plot->hasYMajorGrid());
      _yMinorGrid->setChecked(plot->hasYMinorGrid());
      _majorGridColor->setColor(plot->majorGridColor());
      _minorGridColor->setColor(plot->minorGridColor());
      _checkBoxDefaultMajorGridColor->setChecked(plot->defaultMajorGridColor());
      _checkBoxDefaultMinorGridColor->setChecked(plot->defaultMinorGridColor());
    }
  }
}

void KstPlotDialogI::updateAxesButtons() {
  bool major = _xMajorGrid->isChecked() || _yMajorGrid->isChecked();
  bool minor = _xMinorGrid->isChecked() || _yMinorGrid->isChecked();

  _checkBoxDefaultMajorGridColor->setEnabled(major);
  _checkBoxDefaultMinorGridColor->setEnabled(minor);
  _majorGridColor->setEnabled(major && !_checkBoxDefaultMajorGridColor->isChecked());
  _minorGridColor->setEnabled(minor && !_checkBoxDefaultMinorGridColor->isChecked());
  _xMinorTicks->setEnabled(!_xMinorTicksAuto->isChecked());
  _yMinorTicks->setEnabled(!_yMinorTicksAuto->isChecked());
}

void KstPlotDialogI::updateXTimezoneButtons() {
  bool enable = false;
  
  if (_checkBoxXInterpret->isChecked()) {
    int item = _comboBoxXDisplay->currentItem();
    if (item >= 0 && item < (int)numAxisDisplays) {
      if( AxisDisplays[item].type == AXIS_DISPLAY_YYMMDDHHMMSS_SS      ||
          AxisDisplays[item].type == AXIS_DISPLAY_DDMMYYHHMMSS_SS      ||
          AxisDisplays[item].type == AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS  ||
          AxisDisplays[item].type == AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS ||
          AxisDisplays[item].type == AXIS_DISPLAY_KDE_SHORTDATE        ||
          AxisDisplays[item].type == AXIS_DISPLAY_KDE_LONGDATE          ) {
        enable = true;
      }
    }
  }

  textLabelXTimezone->setEnabled(enable);
  _checkBoxXTimezoneLocal->setEnabled(enable);
  if (enable && _checkBoxXTimezoneLocal->isChecked()) {
    enable = false;
  }
  lineEditXTimezone->setEnabled(enable);
}

void KstPlotDialogI::updateYTimezoneButtons() {
  bool enable = false;
  
  if (_checkBoxYInterpret->isChecked()) {
    int item = _comboBoxYDisplay->currentItem();
    if (item >= 0 && item < (int)numAxisDisplays) {
      if( AxisDisplays[item].type == AXIS_DISPLAY_YYMMDDHHMMSS_SS      ||
          AxisDisplays[item].type == AXIS_DISPLAY_DDMMYYHHMMSS_SS      ||
          AxisDisplays[item].type == AXIS_DISPLAY_QTTEXTDATEHHMMSS_SS  ||
          AxisDisplays[item].type == AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS ||
          AxisDisplays[item].type == AXIS_DISPLAY_KDE_SHORTDATE        ||
          AxisDisplays[item].type == AXIS_DISPLAY_KDE_LONGDATE          ) {
        enable = true;
      }
    }
  }

  textLabelYTimezone->setEnabled(enable);
  _checkBoxYTimezoneLocal->setEnabled(enable);
  if (enable && _checkBoxYTimezoneLocal->isChecked()) {
    enable = false;
  }
  lineEditYTimezone->setEnabled(enable);
}

#include "kstplotdialog_i.moc"
// vim: et ts=2 sw=2
