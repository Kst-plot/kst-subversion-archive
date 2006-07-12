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
#include <qstyle.h>

// include files for KDE
#include <kcolorbutton.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>
#include <kmessagebox.h>
#include <kiconloader.h>

// application specific includes
#include "kstdataobjectcollection.h"
#include "kstdebug.h"
#include "kstlinestyle.h"
#include "kstviewlegend.h"
#include "kstplotdefines.h"
#include "kstplotdialog_i.h"
#include "kstplotlabel.h"
#include "kstsettings.h"
#include "kstviewwindow.h"
#include "plotlistbox.h"
#include "scalarselector.h"
#include "vectorselector.h"

#ifndef DBL_DIG
  #define LABEL_PRECISION        15
#else
  #define LABEL_PRECISION        DBL_DIG
#endif

struct MajorTickSpacing {
  const char *label;
  int majorTickDensity;
};

const MajorTickSpacing MajorTickSpacings[] = {
  { I18N_NOOP("Coarse"), 2 },
  { I18N_NOOP("Default"), 5 },
  { I18N_NOOP("Fine"), 10 },
  { I18N_NOOP("Very fine"), 15 }
};

const unsigned int numMajorTickSpacings = sizeof( MajorTickSpacings ) / sizeof( MajorTickSpacing );

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

  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(TabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTab()));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(_window, SIGNAL(activated(int)), this, SLOT(changeWindow()));
  connect(_newWindow, SIGNAL(clicked()), this, SLOT(newWindow()));

  // axes range
  connect(XAC, SIGNAL(toggled(bool)), XACRange, SLOT(setEnabled(bool)));
  connect(YAC, SIGNAL(toggled(bool)), YACRange, SLOT(setEnabled(bool)));
  connect(YExpression, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));
  connect(XExpression, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));
  
  connect(scalarSelectorX1, SIGNAL(activated(const QString&)),
          XExpressionMin, SLOT(insert(const QString&)));
  connect(scalarSelectorY1, SIGNAL(activated(const QString&)),
          YExpressionMin, SLOT(insert(const QString&)));
  connect(scalarSelectorX2, SIGNAL(activated(const QString&)),
          XExpressionMax, SLOT(insert(const QString&)));
  connect(scalarSelectorY2, SIGNAL(activated(const QString&)),
          YExpressionMax, SLOT(insert(const QString&)));
  
  // adding/removing curves
  connect(DisplayedCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(DisplayedCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeDisplayedCurve()));
  connect(_add, SIGNAL(clicked()), this, SLOT(addDisplayedCurve()));
  connect(_up, SIGNAL(clicked()),
          DisplayedCurveList, SLOT(up()));
  connect(_down, SIGNAL(clicked()),
          DisplayedCurveList, SLOT(down()));

  connect(AutoLabel, SIGNAL(clicked()), this, SLOT(applyAutoLabels()));

  connect(ScalarList, SIGNAL(activated(int)), this, SLOT(insertCurrentScalar()));

  connect(YAxisText, SIGNAL(selectionChanged()), this, SLOT(setScalarDestYLabel()));
  connect(YAxisText, SIGNAL(textChanged(const QString &)), this, SLOT(setScalarDestYLabel()));
  connect(XAxisText, SIGNAL(selectionChanged()), this, SLOT(setScalarDestXLabel()));
  connect(XAxisText, SIGNAL(textChanged(const QString &)), this, SLOT(setScalarDestXLabel()));
  connect(TopLabelText, SIGNAL(selectionChanged()), this, SLOT(setScalarDestTopLabel()));
  connect(TopLabelText, SIGNAL(textChanged(const QString &)), this, SLOT(setScalarDestTopLabel()));

  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXInterpret, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXDisplay, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), textLabelXDisplayAs, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), _comboBoxYInterpret, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), _comboBoxYDisplay, SLOT(setEnabled(bool)));
  connect(_checkBoxYInterpret, SIGNAL(toggled(bool)), textLabelYDisplayAs, SLOT(setEnabled(bool)));
  connect(_xTransformTop, SIGNAL(toggled(bool)), _xTransformTopExp, SLOT(setEnabled(bool)));
  connect(_yTransformRight, SIGNAL(toggled(bool)), _yTransformRightExp, SLOT(setEnabled(bool)));
  connect(_checkBoxDefaultMarkerColor, SIGNAL(toggled(bool)), _colorMarker, SLOT(setDisabled(bool)));
  
  connect(_pushButtonEditLegend, SIGNAL(clicked()), this, SLOT(editLegend()));
  
  FontComboBox->setFonts(qfd.families());

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

  _comboBoxYInterpret->setEnabled(false);
  _comboBoxYDisplay->setEnabled(false);
  textLabelYDisplayAs->setEnabled(false);
 
  appearanceThisPlot->setChecked(true);
  XAxisThisPlot->setChecked(true);
  YAxisThisPlot->setChecked(true);
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
  connect(UseVector, SIGNAL(clicked()),
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
  
  /* set defaults */
  plotColors->setBackground(KstSettings::globalSettings()->backgroundColor);
  plotColors->setForeground(KstSettings::globalSettings()->foregroundColor);

  _xMajorTickSpacing->setCurrentItem(1);
  _yMajorTickSpacing->setCurrentItem(1);

  _yMarksInsidePlot->setChecked(true);
  _xMarksInsidePlot->setChecked(true);
  
  _comboBoxTopLabelJustify->insertItem(i18n("Left"));
  _comboBoxTopLabelJustify->insertItem(i18n("Right"));
  _comboBoxTopLabelJustify->insertItem(i18n("Center"));
  
  // FIXME: should use kstsettings
  _axisPenWidth->setValue(0);
  _majorPenWidth->setValue(0);
  _minorPenWidth->setValue(0);
  
  _up->setPixmap(BarIcon("up"));
  _up->setEnabled(false);
  _down->setPixmap(BarIcon("down"));
  _down->setEnabled(false);
  _add->setPixmap(BarIcon("forward"));
  _add->setEnabled(false);
  _remove->setPixmap(BarIcon("back"));
  _remove->setEnabled(false);

  _newWindow->setPixmap(SmallIcon("window_new"));

  _colorMarker->setColor(QColor("black"));
  _spinBoxMarkerLineWidth->setValue(0);
  _checkBoxDefaultMarkerColor->setChecked(true);
  fillMarkerLineCombo();
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

  _vectorForMarkers->update();
  scalarSelectorX1->update();
  scalarSelectorY1->update();
  scalarSelectorX2->update();
  scalarSelectorY2->update();
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

    QString xMinExp, xMaxExp, yMinExp, yMaxExp;
    plot->getXScaleExps(xMinExp, xMaxExp);
    plot->getYScaleExps(yMinExp, yMaxExp);

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
        XExpression->setChecked(true); // treat fixed ranges as expressions
        XExpressionMin->setText(QString::number(xmin, 'g', 16));
        XExpressionMax->setText(QString::number(xmax, 'g', 16));
        break;
      case EXPRESSION:
        XExpression->setChecked(true);
        XExpressionMin->setText(xMinExp);
        XExpressionMax->setText(xMaxExp);
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
        YExpression->setChecked(true);
        YExpressionMin->setText(QString::number(ymin, 'g', 16));
        YExpressionMax->setText(QString::number(ymax, 'g', 16));
        break;
      case EXPRESSION:
        YExpression->setChecked(true);
        YExpressionMin->setText(yMinExp);
        YExpressionMax->setText(yMaxExp);
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

    NumberFontSize->setValue(plot->xTickLabel()->fontSize());
    _spinBoxXAngle->setValue((int)plot->xTickLabel()->rotation());
    _spinBoxYAngle->setValue((int)plot->yTickLabel()->rotation());

    XAxisText->setText(plot->xLabel()->text());
    XLabelFontSize->setValue(plot->xLabel()->fontSize());

    YAxisText->setText(plot->yLabel()->text());
    YLabelFontSize->setValue(plot->yLabel()->fontSize());

    TopLabelText->setText(plot->topLabel()->text());
    TopLabelFontSize->setValue(plot->topLabel()->fontSize());
    FontComboBox->setCurrentFont(plot->topLabel()->fontName());
    switch (plot->topLabel()->justification()) {
      case KST_JUSTIFY_H_LEFT:
        _comboBoxTopLabelJustify->setCurrentItem(0);
        break;
      case KST_JUSTIFY_H_RIGHT:
        _comboBoxTopLabelJustify->setCurrentItem(1);
        break;
      case KST_JUSTIFY_H_CENTER:
        _comboBoxTopLabelJustify->setCurrentItem(2);
        break;
      default:
        _comboBoxTopLabelJustify->setCurrentItem(0);
        break;
    }
    
    // update the x-axis interpretation
    KstAxisInterpretation xAxisInterpretation;
    KstAxisInterpretation yAxisInterpretation;
    KstAxisDisplay xAxisDisplay;
    KstAxisDisplay yAxisDisplay;
    bool xAxisInterpret;
    bool yAxisInterpret;

    plot->getXAxisInterpretation(xAxisInterpret, xAxisInterpretation, xAxisDisplay);
    _checkBoxXInterpret->setChecked(xAxisInterpret);
    _comboBoxXInterpret->setEnabled(xAxisInterpret);
    _comboBoxXDisplay->setEnabled(xAxisInterpret);
    textLabelXDisplayAs->setEnabled(xAxisInterpret);

    if (xAxisInterpret) {
      for (i = 0; i < numAxisInterpretations; i++) {
        if (AxisInterpretations[i].type == xAxisInterpretation) {
          _comboBoxXInterpret->setCurrentItem(i);
          break;
        }
      }
      for (i = 0; i < numAxisDisplays; i++) {
        if (AxisDisplays[i].type == xAxisDisplay) {
          _comboBoxXDisplay->setCurrentItem(i);
          break;
        }
      }
    } else {
      _comboBoxXInterpret->setCurrentItem(0);
      _comboBoxXDisplay->setCurrentItem(0);
    }

    plot->getYAxisInterpretation(yAxisInterpret, yAxisInterpretation, yAxisDisplay);
    _checkBoxYInterpret->setChecked(yAxisInterpret);
    _comboBoxYInterpret->setEnabled(yAxisInterpret);
    _comboBoxYDisplay->setEnabled(yAxisInterpret);
    textLabelYDisplayAs->setEnabled(yAxisInterpret);

    if (yAxisInterpret) {
      for (i = 0; i < numAxisInterpretations; i++) {
        if (AxisInterpretations[i].type == yAxisInterpretation) {
          _comboBoxYInterpret->setCurrentItem(i);
          break;
        }
      }
      for (i = 0; i < numAxisDisplays; i++) {
        if (AxisDisplays[i].type == yAxisDisplay) {
          _comboBoxYDisplay->setCurrentItem(i);
          break;
        }
      }
    } else {
      _comboBoxYInterpret->setCurrentItem(0);
      _comboBoxYDisplay->setCurrentItem(0);
    }
    
    // initialize the legend settings for the current plot
    KstViewLegendPtr vl = plot->legend();
    if (vl) {
      ShowLegend->setChecked(true);
      TrackContents->setChecked(vl->trackContents());
    } else { // plot does not currently have a legend: use defaults.
      ShowLegend->setChecked(false);
    }
    // initialize the plot color widget
    plotColors->setForeground(plot->foregroundColor());
    plotColors->setBackground(plot->backgroundColor());

    _axisPenWidth->setValue(plot->axisPenWidth());

      //update the tick display options
    _xMarksInsidePlot->setChecked(plot->xTicksInPlot() && !plot->xTicksOutPlot());
    _xMarksOutsidePlot->setChecked(plot->xTicksOutPlot() && !plot->xTicksInPlot());
    _xMarksInsideAndOutsidePlot->setChecked(plot->xTicksOutPlot() && plot->xTicksInPlot());

    _yMarksInsidePlot->setChecked(plot->yTicksInPlot() && !plot->yTicksOutPlot());
    _yMarksOutsidePlot->setChecked(plot->yTicksOutPlot() && !plot->yTicksInPlot());
    _yMarksInsideAndOutsidePlot->setChecked(plot->yTicksOutPlot() && plot->yTicksInPlot());

    //update axis suppression
    _suppressTop->setChecked(plot->suppressTop());
    _suppressBottom->setChecked(plot->suppressBottom());
    _suppressRight->setChecked(plot->suppressRight());
    _suppressLeft->setChecked(plot->suppressLeft());

    //update axes transforms
    _yTransformRight->setChecked(!plot->yTransformedExp().isEmpty());
    _yTransformRightExp->setText(plot->yTransformedExp());
    _xTransformTop->setChecked(!plot->xTransformedExp().isEmpty());
    _xTransformTopExp->setText(plot->xTransformedExp());
    
    _xReversed->setChecked(plot->xReversed());
    _yReversed->setChecked(plot->yReversed());
    
    // update marker attributes
    _comboMarkerLineStyle->setCurrentItem(plot->lineStyleMarkers());
    _spinBoxMarkerLineWidth->setValue(plot->lineWidthMarkers());
    _checkBoxDefaultMarkerColor->setChecked(plot->defaultColorMarker());
    _colorMarker->setColor(plot->colorMarkers());
    _colorMarker->setEnabled(!plot->defaultColorMarker());
  } else {
    FontComboBox->setCurrentFont(KstApp::inst()->defaultFont());
  }

  updateButtons();
  updateAxesButtons();
}

void KstPlotDialogI::fillMarkerLineCombo() {
  QRect rect = _comboMarkerLineStyle->style().querySubControlMetrics(
          QStyle::CC_ComboBox, _comboMarkerLineStyle, QStyle::SC_ComboBoxEditField);
  rect.setLeft(rect.left() + 2);
  rect.setRight(rect.right() - 2);
  rect.setTop(rect.top() + 2);
  rect.setBottom(rect.bottom() - 2);

  // fill the point type dialog with point types
  QPixmap ppix(rect.width(), rect.height());
  QPainter pp(&ppix);
  QPen pen(QColor("black"), 0);

  int currentItem = _comboMarkerLineStyle->currentItem();
  _comboMarkerLineStyle->clear();

  for (int style = 0; style < (int)KSTLINESTYLE_MAXTYPE; style++) {
    pen.setStyle(KstLineStyle[style]);
    pp.setPen(pen);
    pp.fillRect( pp.window(), QColor("white"));
    pp.drawLine(1,ppix.height()/2,ppix.width()-1, ppix.height()/2);
    _comboMarkerLineStyle->insertItem(ppix);
  }

  _comboMarkerLineStyle->setCurrentItem(currentItem);
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

  // only apply label text to this plot, despite radio button values
  plot->xLabel()->setText(XAxisText->text());
  plot->yLabel()->setText(YAxisText->text());
  plot->topLabel()->setText(TopLabelText->text());
  switch (_comboBoxTopLabelJustify->currentItem()) {
    case 0:
      plot->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_LEFT, KST_JUSTIFY_V_NONE));
      break;
    case 1:
      plot->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_RIGHT, KST_JUSTIFY_V_NONE));
      break;
    case 2:
      plot->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_CENTER, KST_JUSTIFY_V_NONE));
      break;      
  }
  
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

  for (uint i = 0; i < plots.size(); i++) {
    plotExtra = plots[i];

    plotExtra->setForegroundColor(plotColors->foreground());
    plotExtra->setBackgroundColor(plotColors->background());

    // gridlines colors
    plotExtra->setGridLinesColor(_majorGridColor->color(),
                                 _minorGridColor->color(),
                                 _checkBoxDefaultMajorGridColor->isChecked(),
                                 _checkBoxDefaultMinorGridColor->isChecked());

    plotExtra->setAxisPenWidth(_axisPenWidth->value());
    plotExtra->setMajorPenWidth(_majorPenWidth->value());
    plotExtra->setMinorPenWidth(_minorPenWidth->value());

    plotExtra->xLabel()->setFontName(FontComboBox->currentText());
    plotExtra->xLabel()->setFontSize(XLabelFontSize->value());

    plotExtra->yLabel()->setFontName(FontComboBox->currentText());
    plotExtra->yLabel()->setFontSize(YLabelFontSize->value());

    plotExtra->topLabel()->setFontName(FontComboBox->currentText());
    plotExtra->topLabel()->setFontSize(TopLabelFontSize->value());
    switch (_comboBoxTopLabelJustify->currentItem()) {
      case 0:
        plotExtra->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_LEFT, KST_JUSTIFY_V_NONE));
        break;
      case 1:
        plotExtra->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_RIGHT, KST_JUSTIFY_V_NONE));
        break;
      case 2:
        plotExtra->topLabel()->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_CENTER, KST_JUSTIFY_V_NONE));
        break;
    }

    plotExtra->xTickLabel()->setFontName(FontComboBox->currentText());
    plotExtra->xTickLabel()->setFontSize(NumberFontSize->value());
    plotExtra->xTickLabel()->setRotation(_spinBoxXAngle->value());
    
    plotExtra->fullTickLabel()->setFontName(FontComboBox->currentText());
    plotExtra->fullTickLabel()->setFontSize(NumberFontSize->value());
    
    plotExtra->yTickLabel()->setFontName(FontComboBox->currentText());
    plotExtra->yTickLabel()->setFontSize(NumberFontSize->value());
    plotExtra->yTickLabel()->setRotation(_spinBoxYAngle->value());
    if (ShowLegend->isChecked()) {
      KstViewLegendPtr vl = plotExtra->getOrCreateLegend();
      vl->setTrackContents(TrackContents->isChecked());
    } else {
      KstViewLegendPtr vl = plotExtra->legend();
      if (vl) {
        plotExtra->removeChild(KstViewObjectPtr(vl));
      }
    }
    plotExtra->setDirty();
  }
}

void KstPlotDialogI::applyYAxis(Kst2DPlotPtr plot) {
  
  Kst2DPlotList plots;
  Kst2DPlotPtr  plotExtra;

  if (YAxisThisPlot->isChecked()) {
    plots += plot;
  } else if (YAxisThisWindow->isChecked()) {
    KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
    if (c) {
      plots  = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);
    }
  } else {
    fill2DPlotList(plots);
  }

  for (uint i = 0; i < plots.size(); i++) {    
    plotExtra = plots[i];
    
    plotExtra->setYOffsetMode(_checkBoxYOffsetMode->isChecked());
  
    if (_checkBoxYInterpret->isChecked()) {
      plotExtra->setYAxisInterpretation(true, 
                                        AxisInterpretations[_comboBoxYInterpret->currentItem()].type,
                                        AxisDisplays[_comboBoxYDisplay->currentItem()].type);
    } else {
      plotExtra->setYAxisInterpretation(false, 
                                        AXIS_INTERP_CTIME, 
                                        AXIS_DISPLAY_YEAR);
    }
    // minor ticks
    if (_yMinorTicksAuto->isChecked()) {
      plotExtra->setYMinorTicks(-1);
    } else {
      plotExtra->setYMinorTicks(_yMinorTicks->value());
    }
    // major ticks
    plotExtra->setYMajorTicks(
        MajorTickSpacings[_yMajorTickSpacing->currentItem()].majorTickDensity);
    
    // tick display
    plotExtra->setYTicksInPlot(_yMarksInsidePlot->isChecked() || _yMarksInsideAndOutsidePlot->isChecked());
    plotExtra->setYTicksOutPlot(_yMarksOutsidePlot->isChecked() || _yMarksInsideAndOutsidePlot->isChecked());
    
    // grid lines
    plotExtra->setYGridLines(_yMajorGrid->isChecked(), _yMinorGrid->isChecked());
    
    // axis suppression
    plotExtra->setSuppressLeft(_suppressLeft->isChecked());
    plotExtra->setSuppressRight(_suppressRight->isChecked());
    
    // transformed opposite axis
    if (_yTransformRight->isChecked()) {
      plotExtra->setYTransformedExp(_yTransformRightExp->text());
    } else {
      plotExtra->setYTransformedExp(QString::null);
    }
    
    // reversed
    plotExtra->setYReversed(_yReversed->isChecked());
    plotExtra->setDirty();
  }
}

void KstPlotDialogI::applyXAxis(Kst2DPlotPtr plot) {
  Kst2DPlotList plots;
  Kst2DPlotPtr  plotExtra;

  if (XAxisThisPlot->isChecked()) {
    plots += plot;
  } else if (XAxisThisWindow->isChecked()) {
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
    
    if (_checkBoxXInterpret->isChecked()) {      
      plotExtra->setXAxisInterpretation(true, 
                                        AxisInterpretations[_comboBoxXInterpret->currentItem()].type,
                                        AxisDisplays[_comboBoxXDisplay->currentItem()].type);
    } else {
      plotExtra->setXAxisInterpretation(false, 
                                        AXIS_INTERP_CTIME, 
                                        AXIS_DISPLAY_YEAR);
    }
    // minor tick settings.
    if (_xMinorTicksAuto->isChecked()) {
      plotExtra->setXMinorTicks(-1);
    } else {
      plotExtra->setXMinorTicks(_xMinorTicks->value());
    }
    
    // major tick settings.
    plotExtra->setXMajorTicks(
        MajorTickSpacings[_xMajorTickSpacing->currentItem()].majorTickDensity);
    
    // tick display
    plotExtra->setXTicksInPlot(_xMarksInsidePlot->isChecked() || _xMarksInsideAndOutsidePlot->isChecked());
    plotExtra->setXTicksOutPlot(_xMarksOutsidePlot->isChecked() || _xMarksInsideAndOutsidePlot->isChecked());
    
    // grid lines
    plotExtra->setXGridLines(_xMajorGrid->isChecked(), _xMinorGrid->isChecked());
    
    // axis suppression
    plotExtra->setSuppressTop(_suppressTop->isChecked());
    plotExtra->setSuppressBottom(_suppressBottom->isChecked());
    
    // transformed opposite axis
    if (_xTransformTop->isChecked()) {
      plotExtra->setXTransformedExp(_xTransformTopExp->text());
    } else {
      plotExtra->setXTransformedExp(QString::null);
    }
    
    plotExtra->setXReversed(_xReversed->isChecked());
    plotExtra->setDirty();
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
    } else if (XExpression->isChecked()) {
      plotExtra->setXScaleMode(EXPRESSION);
      if (!plotExtra->setXExpressions(XExpressionMin->text(), XExpressionMax->text()))
      {
        KMessageBox::sorry(this, i18n("There is a problem with the X range expressions."));
        return;
      }
      //if expressions are constant, just use FIXED mode
      plotExtra->optimizeXExps();
    } else if (XAutoUp->isChecked()) {   
      plotExtra->setXScaleMode(AUTOUP);
    } else if (XAuto->isChecked()) {
      plotExtra->setXScaleMode(AUTO);
    } else if (XAutoBorder->isChecked()) {
      plotExtra->setXScaleMode(AUTOBORDER);
    } else if (XNoSpikes->isChecked()) {
      plotExtra->setXScaleMode(NOSPIKE);
    } else {
      KstDebug::self()->log(i18n("Internal error: No X scale type checked in %1.").arg(Select->currentText()), KstDebug::Error);
    }

    // do Y Scale
    if (YAC->isChecked()) {
      plotExtra->setYScaleMode(AC);
      plotExtra->setYScale(0, YACRange->text().toDouble());
    } else if (YExpression->isChecked()) {
      plotExtra->setYScaleMode(EXPRESSION);
      if (!plotExtra->setYExpressions(YExpressionMin->text(), YExpressionMax->text()))
      {
        KMessageBox::sorry(this, i18n("There is a problem with the Y range expressions."));
        return;
      }
      //if expressions are constant, just use FIXED mode
      plotExtra->optimizeYExps();
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
    plotExtra->setDirty();
  }
}

void KstPlotDialogI::applySettings(KMdiChildView *c, Kst2DPlotPtr plot) {
  if (_reGrid->isChecked()) {
    static_cast<KstViewWindow*>(c)->view()->cleanup(_plotColumns->value());
  }

  applyAppearance(plot);

  // FIXME: be more efficient here.  Only remove the curves that we need, only
  //        add the curves that we need

  // add the curves
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  plot->clearCurves();
  for (unsigned i = 0; i < DisplayedCurveList->count(); i++) {
    KstBaseCurveList::Iterator it = curves.findTag(DisplayedCurveList->text(i));
    if (it != curves.end()) {
      plot->addCurve(*it);
    }
  }
  curves.clear();

  applyXAxis(plot);
  applyYAxis(plot);
  applyRange(plot);
  applyPlotMarkers(plot);

  plot->setDirty();

  // make sure we paint all the necessary windows
  if (appearanceAll->isChecked() ||
      XAxisAll->isChecked() ||
      YAxisAll->isChecked() ||
      rangeAll->isChecked() ||
      markersAll->isChecked()) {
    KMdiIterator<KMdiChildView*> *it = KstApp::inst()->createIterator();
    while (it->currentItem()) {
      KstViewWindow *win = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (win) {
        win->view()->paint(KstPainter::P_PLOT);
      }
      it->next();
    }
    KstApp::inst()->deleteIterator(it);
  } else {
    static_cast<KstViewWindow*>(c)->view()->paint(KstPainter::P_PLOT);
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
      Kst2DPlotPtr plot = static_cast<KstViewWindow*>(c)->view()->createObject<Kst2DPlot>(Name->text().stripWhiteSpace());
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

  static_cast<KstViewWindow*>(c)->view()->paint(KstPainter::P_PLOT);

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
      for (KstMarkerList::ConstIterator it = plot->plotMarkers().begin(); it != plot->plotMarkers().end(); ++it) {
        if ((*it).isRising) {
          PlotMarkerList->insertItem(i18n("%1 [rising]").arg(QString::number((*it).value, 'g', LABEL_PRECISION)));
        } else if ((*it).isFalling) {
          PlotMarkerList->insertItem(i18n("%1 [falling]").arg(QString::number((*it).value, 'g', LABEL_PRECISION)));
        } else if ((*it).isVectorValue) {
          PlotMarkerList->insertItem(i18n("%1 [value]").arg(QString::number((*it).value, 'g', LABEL_PRECISION)));        
        } else {
          PlotMarkerList->insertItem(QString::number((*it).value, 'g', LABEL_PRECISION));
        }
      }

      // update the auto-generation settings
      KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
      CurveCombo->clear();
      for (KstBaseCurveList::iterator curves_iter = curves.begin(); curves_iter != curves.end(); ++curves_iter) {
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
      } else {
        UseCurve->setChecked(false);
      }
      
      if (plot->hasVectorToMarkers()) {
        UseVector->setChecked(true);
        _vectorForMarkers->setSelection(plot->vectorToMarkers()->tagName());
      } else {
        UseVector->setChecked(false);
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
    plot->generateDefaultLabels();
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
  if (selected && !_add->isEnabled()) {
    _add->setEnabled(true);
  } else if (!selected && _add->isEnabled()) {
    _add->setEnabled(false);
  }

  selected = false;
  count = DisplayedCurveList->count();
  for (uint i = 0; i < count; i++) {
    if (DisplayedCurveList->isSelected(i)) {
      selected = true;
    }
  }
  if (selected && !_remove->isEnabled()) {
    _remove->setEnabled(true);
  } else if (!selected && _remove->isEnabled()) {
    _remove->setEnabled(false);
  }
  if (selected && !_up->isEnabled()) {
    _up->setEnabled(true);
    _down->setEnabled(true);
  } else if (!selected && _up->isEnabled()) {
    _up->setEnabled(false);
    _down->setEnabled(false);
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
  _textLabelCreateMarkersOn->setEnabled(UseCurve->isChecked());
  _vectorForMarkers->setEnabled(UseVector->isChecked());
  
  //updates for range tab
  YExpressionMin->setEnabled(YExpression->isChecked());
  YExpressionMax->setEnabled(YExpression->isChecked());
  scalarSelectorY1->setEnabled(YExpression->isChecked());
  scalarSelectorY2->setEnabled(YExpression->isChecked());
  XExpressionMin->setEnabled(XExpression->isChecked());
  XExpressionMax->setEnabled(XExpression->isChecked());
  scalarSelectorX1->setEnabled(XExpression->isChecked());
  scalarSelectorX2->setEnabled(XExpression->isChecked());
  
  _xTransformTopExp->setEnabled(_xTransformTop->isChecked());
  _yTransformRightExp->setEnabled(_yTransformRight->isChecked());
}


void KstPlotDialogI::updateCurveLists() {
  KstApp* app = KstApp::inst();
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  DisplayedCurveList->clear();
  AvailableCurveList->clear();

  KMdiChildView *c = app->findWindow(_window->currentText());
  if (Select->count() && c) {
    KstViewObjectPtr obj = static_cast<KstViewWindow*>(c)->view()->findChild(Select->currentText());
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(obj);

    if (plot) {
      // add curves while retaining the order in the plot
      for (KstBaseCurveList::iterator it = plot->Curves.begin(); it != plot->Curves.end(); ++it) {
        (*it)->readLock();
        DisplayedCurveList->insertItem((*it)->tagName());
        (*it)->readUnlock();
      }
      for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); ++it) {
        (*it)->readLock();
        if (plot->Curves.find(*it) == plot->Curves.end()) {
          AvailableCurveList->insertItem((*it)->tagName());
        }
        (*it)->readUnlock();
      }
    }
  } else {
    for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); ++it) {
      (*it)->readLock();
      AvailableCurveList->insertItem((*it)->tagName());
      (*it)->readUnlock();
    }
  }
}

void KstPlotDialogI::updateScalarCombo() {
  ScalarList->clear();
  scalarSelectorX1->clear();
  scalarSelectorX2->clear();
  scalarSelectorY1->clear();
  scalarSelectorY2->clear();
  KST::scalarList.lock().readLock();
  for (int index = 0; index < (int)KST::scalarList.count(); index++) {
    KstScalarPtr s = KST::scalarList[index];
    s->readLock();
    ScalarList->insertItem(s->tagLabel());
    scalarSelectorX1->insertItem(s->tagLabel());
    scalarSelectorX2->insertItem(s->tagLabel());
    scalarSelectorY1->insertItem(s->tagLabel());
    scalarSelectorY2->insertItem(s->tagLabel());
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
                       i18n("The text you have entered is not a valid number."),
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

  marker.isRising = false;
  marker.isFalling = false;
  marker.isVectorValue = false;
  for (i = 0; i < PlotMarkerList->count(); ++i) {
    marker.value = PlotMarkerList->text(i).toDouble();
    if (PlotMarkerList->text(i).find( i18n("rising")) == -1 &&
        PlotMarkerList->text(i).find( i18n("falling")) == -1 &&
        PlotMarkerList->text(i).find( i18n("value")) == -1) {
      newMarkers.append(marker);
    }
  }

  for (i = 0; i < plots.size(); i++) {
    plotExtra = plots[i];
    plotExtra->setPlotMarkerList(newMarkers);

    // apply the auto-generation settings
    if (UseCurve->isChecked()) {
      KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
      KstVCurveList vcurves = kstObjectSubList<KstBaseCurve, KstVCurve>(curves);
      KstVCurvePtr curve = *(vcurves.findTag(CurveCombo->currentText()));
      bool falling = Falling->isChecked();
      bool rising = Rising->isChecked();
      
      if (Both->isChecked()) {
        falling = true;
        rising = true;
      }
      plotExtra->setCurveToMarkers(curve, rising, falling);
    } else {
      plotExtra->removeCurveToMarkers();
    }

    if (UseVector->isChecked()) {
      KstVectorPtr vector = *(KST::vectorList.findTag(_vectorForMarkers->selectedVector()));
      plotExtra->setVectorToMarkers(vector);
    } else {
      plotExtra->removeVectorToMarkers();
    }
    
    plotExtra->setLineStyleMarkers(_comboMarkerLineStyle->currentItem());
    plotExtra->setLineWidthMarkers(_spinBoxMarkerLineWidth->value());
    plotExtra->setColorMarkers(_colorMarker->color());
    plotExtra->setDefaultColorMarker(_checkBoxDefaultMarkerColor->isChecked());
    plotExtra->setDirty();
  }
}

void KstPlotDialogI::newWindow() {
  QString name = KstApp::inst()->newWindow();
  if (!name.isEmpty()) {
    _window->setCurrentText(name);
    updatePlotList();
  }
}

void KstPlotDialogI::editLegend() {
  int index = Select->currentItem();

  KMdiChildView *c = KstApp::inst()->findWindow(_window->currentText());
  if (c) {
    Kst2DPlotList plots = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>(true);

    if (index < (int)plots.count()) {
      
      Kst2DPlotPtr plot = *plots.at(index);
      KstViewLegendPtr vl = plot->getOrCreateLegend();
      
      if (vl) { 
        KstViewWindow *vw = dynamic_cast<KstViewWindow*>(c);        
        if (vw) {
          KstTopLevelViewPtr tlv = vw->view();
          if (tlv) {
            vl->showDialog(tlv, false);
            
            ShowLegend->setChecked(true);
          }
        }
      }
    }
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
            QWidget *page = TabWidget->page(0);
                
            KMessageBox::sorry(this, message);
            
            if (page) {
              TabWidget->showPage(page);
            }
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
    KMessageBox::sorry(this, i18n("The plot name is empty. Enter a unique name for the plot."));
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
      _majorPenWidth->setValue(plot->majorPenWidth());
      _minorPenWidth->setValue(plot->minorPenWidth());
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
  _majorPenWidth->setEnabled(major);
  _minorPenWidth->setEnabled(minor);
  _majorPenWidthLabel->setEnabled(major);
  _minorPenWidthLabel->setEnabled(minor);
  _xMinorTicks->setEnabled(!_xMinorTicksAuto->isChecked());
  _yMinorTicks->setEnabled(!_yMinorTicksAuto->isChecked());
}


#include "kstplotdialog_i.moc"
// vim: et ts=2 sw=2
