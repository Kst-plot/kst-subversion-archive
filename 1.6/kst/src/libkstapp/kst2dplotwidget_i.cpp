/***************************************************************************
                    Kst2dPlotWidget_i.cpp  -  Part of KST
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 The University of British Columbia
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

#include <qbutton.h>
#include <qcheckbox.h>
#include <qdeepcopy.h>
#include <qfontdatabase.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qstyle.h>
#include <qtooltip.h>

#include <kcombobox.h>
#include <kcolorbutton.h>
#include <kdualcolorbutton.h>
#include <kfontcombo.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "plotlistbox.h"
#include "vectorselector.h"
#include "kst2dplotwidget_i.h"
#include "kstcombobox.h"
#include "kstdatacollection.h"
#include "kstdataobject.h"
#include "kstdataobjectcollection.h"
#include "kstdebug.h"
#include "kstdefaultnames.h"
#include "kstlinestyle.h"
#include "kstplotlabel.h"
#include "kstsettings.h"
#include "ksttoplevelview.h"

Kst2dPlotWidget::Kst2dPlotWidget(QWidget* parent, const char* name, WFlags fl) : View2DPlotWidget(parent, name, fl )
{
  QFontDatabase qfd;

  _editMultipleMode = false;
  _plot = 0L;

  _up->setPixmap(BarIcon("up"));
  _up->setEnabled(false);
  _up->setAccel(ALT+Key_Up);
  _down->setPixmap(BarIcon("down"));
  _down->setEnabled(false);
  _down->setAccel(ALT+Key_Down);
  _add->setPixmap(BarIcon("forward"));
  _add->setEnabled(false);
  _add->setAccel(ALT+Key_S);
  _remove->setPixmap(BarIcon("back"));
  _remove->setEnabled(false);
  _remove->setAccel(ALT+Key_R);

  QToolTip::add(_up, i18n("Shortcut: Alt+Up"));
  QToolTip::add(_down, i18n("Shortcut: Alt+Down"));
  QToolTip::add(_add, i18n("Shortcut: Alt+s"));
  QToolTip::add(_remove, i18n("Shortcut: Alt+r"));

  FontComboBox->setEditable(false);

  // axes range
  connect(XAC, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));
  connect(YAC, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));
  connect(XExpression, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));
  connect(YExpression, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));

  connect(scalarSelectorX1, SIGNAL(activated(const QString&)), this, SLOT(insertXExpressionMin(const QString&)));
  connect(scalarSelectorY1, SIGNAL(activated(const QString&)), this, SLOT(insertYExpressionMin(const QString&)));
  connect(scalarSelectorX2, SIGNAL(activated(const QString&)), this, SLOT(insertXExpressionMax(const QString&)));
  connect(scalarSelectorY2, SIGNAL(activated(const QString&)), this, SLOT(insertYExpressionMax(const QString&)));

  // adding/removing curves
  connect(DisplayedCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(DisplayedCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(removeDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(addDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeDisplayedCurve()));
  connect(_add, SIGNAL(clicked()), this, SLOT(addDisplayedCurve()));
  connect(_up, SIGNAL(clicked()), this, SLOT(upDisplayedCurve()));
  connect(_down, SIGNAL(clicked()), this, SLOT(downDisplayedCurve()));

  connect(AutoLabel, SIGNAL(clicked()), this, SLOT(generateDefaultLabels()));

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
  connect(_checkBoxDefaultMarkerColor, SIGNAL(toggled(bool)), this, SLOT(updateButtons()));

  _scalarDest = TopLabelText;

  connect(_pushButtonEditLegend, SIGNAL(clicked()), this, SLOT(editLegend()));

  FontComboBox->setFonts(qfd.families());

  for (unsigned i = 0; i < numMajorTickSpacings; ++i) {
    _xMajorTickSpacing->insertItem(i18n(MajorTickSpacings[i].label));
    _yMajorTickSpacing->insertItem(i18n(MajorTickSpacings[i].label));
  }

  for (unsigned i = 0; i < numAxisInterpretations; ++i) {
    _comboBoxXInterpret->insertItem(i18n(AxisInterpretations[i].label));
    _comboBoxYInterpret->insertItem(i18n(AxisInterpretations[i].label));
  }
  for (unsigned i = 0; i < numAxisDisplays; ++i) {
    _comboBoxXDisplay->insertItem(i18n(AxisDisplays[i].label));
    _comboBoxYDisplay->insertItem(i18n(AxisDisplays[i].label));
  }

  _comboBoxXInterpret->setEnabled(false);
  _comboBoxXDisplay->setEnabled(false);
  textLabelXDisplayAs->setEnabled(false);

  _comboBoxYInterpret->setEnabled(false);
  _comboBoxYDisplay->setEnabled(false);
  textLabelYDisplayAs->setEnabled(false);

  // plot markers
  connect(AddPlotMarker, SIGNAL(clicked()), this, SLOT(addPlotMarker()));
  connect(RemovePlotMarker, SIGNAL(clicked()), this, SLOT(removePlotMarker()));
  connect(RemoveAllPlotMarkers, SIGNAL(clicked()), this, SLOT(removeAllPlotMarkers()));
  connect(PlotMarkerList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(updateButtons()));
  connect(PlotMarkerList, SIGNAL(selectionChanged()), this, SLOT(updateButtons()));
  connect(NewPlotMarker, SIGNAL(returnPressed()), this, SLOT(addPlotMarker()));
  connect(NewPlotMarker, SIGNAL(textChanged(const QString &)), this, SLOT(updateButtons()));
  connect(UseCurve, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(UseVector, SIGNAL(clicked()), this, SLOT(updateButtons()));

  // grid lines
  connect(_xMajorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_yMajorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_xMinorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_yMinorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));

  connect(_xMinorTicksAuto, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_yMinorTicksAuto, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMajorGridColor, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMinorGridColor, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));

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

  _colorMarker->setColor(QColor("black"));
  _spinBoxMarkerLineWidth->setValue(0);
  _checkBoxDefaultMarkerColor->setChecked(true);
  fillMarkerLineCombo();
}

Kst2dPlotWidget::~Kst2dPlotWidget() {
}

void Kst2dPlotWidget::generateDefaultLabels() {
  if (_plot) {
    _plot->generateDefaultLabels();
    XAxisText->setText(_plot->xLabel()->text());
    YAxisText->setText(_plot->yLabel()->text());
    TopLabelText->setText(_plot->topLabel()->text());
  }
}

void Kst2dPlotWidget::updateButtons() {
  bool selected = false;
  uint count = AvailableCurveList->count();

  for (uint i = 0; i < count; ++i) {
    if (AvailableCurveList->isSelected(i)) {
      selected = true;
      break;
    }
  }
  if (selected && !_add->isEnabled()) {
    _add->setEnabled(true);
  } else if (!selected && _add->isEnabled()) {
    _add->setEnabled(false);
  }

  selected = false;
  count = DisplayedCurveList->count();
  for (uint i = 0; i < count; ++i) {
    if (DisplayedCurveList->isSelected(i)) {
      selected = true;
      break;
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

  // updates for range tab X...
  XACRange->setEnabled(_editMultipleMode || XAC->isChecked());
  XExpressionMin->setEnabled(_editMultipleMode || XExpression->isChecked());
  XExpressionMax->setEnabled(_editMultipleMode || XExpression->isChecked());
  scalarSelectorX1->setEnabled(_editMultipleMode || XExpression->isChecked());
  scalarSelectorX2->setEnabled(_editMultipleMode || XExpression->isChecked());
  _xTransformTopExp->setEnabled(_xTransformTop->isChecked());

  // updates for range tab Y...
  YACRange->setEnabled(_editMultipleMode || YAC->isChecked());
  YExpressionMin->setEnabled(_editMultipleMode || YExpression->isChecked());
  YExpressionMax->setEnabled(_editMultipleMode || YExpression->isChecked());
  scalarSelectorY1->setEnabled(_editMultipleMode || YExpression->isChecked());
  scalarSelectorY2->setEnabled(_editMultipleMode || YExpression->isChecked());
  _yTransformRightExp->setEnabled(_yTransformRight->isChecked());

  // updates for Plot Markers tab
  if (!_editMultipleMode) {
    CurveCombo->setEnabled(UseCurve->isChecked());
    Rising->setEnabled(UseCurve->isChecked());
    Falling->setEnabled(UseCurve->isChecked());
    Both->setEnabled(UseCurve->isChecked());
    _textLabelCreateMarkersOn->setEnabled(UseCurve->isChecked());
    _vectorForMarkers->setEnabled(UseVector->isChecked());
    AddPlotMarker->setEnabled(!NewPlotMarker->text().isEmpty());

    selected = false;
    count = PlotMarkerList->count();
    for (uint i = 0; i < count; ++i) {
      if (PlotMarkerList->isSelected(i)) {
        selected = true;
        break;
      }
    }
    RemovePlotMarker->setEnabled(selected);
    RemoveAllPlotMarkers->setEnabled(count > 0);
  }
  _colorMarker->setEnabled(_checkBoxDefaultMarkerColor->state() != QButton::On);
}


void Kst2dPlotWidget::addDisplayedCurve() {
  uint count = AvailableCurveList->count();
  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (AvailableCurveList->isSelected(i)) {
        DisplayedCurveList->insertItem(AvailableCurveList->text(i));
        AvailableCurveList->removeItem(i);
      }
    }
    updateButtons();
    emit changed();
  }
  TrackContents->setChecked(false);
}


void Kst2dPlotWidget::removeDisplayedCurve() {
  uint count = DisplayedCurveList->count();
  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      if (DisplayedCurveList->isSelected(i)) {
        AvailableCurveList->insertItem(DisplayedCurveList->text(i));
        DisplayedCurveList->removeItem(i);
      }
    }
    updateButtons();
    emit changed();
  }
  TrackContents->setChecked(false);
}


void Kst2dPlotWidget::upDisplayedCurve() {
  if (DisplayedCurveList->up()) {
    emit changed();
  }
}


void Kst2dPlotWidget::downDisplayedCurve() {
  if (DisplayedCurveList->down()) {
    emit changed();
  }
}


void Kst2dPlotWidget::fillMarkerLineCombo() {
  QRect rect = _comboMarkerLineStyle->style().querySubControlMetrics(QStyle::CC_ComboBox, _comboMarkerLineStyle, QStyle::SC_ComboBoxEditField);
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

  for (int style = 0; style < (int)KSTLINESTYLE_MAXTYPE; ++style) {
    pen.setStyle(KstLineStyle[style]);
    pp.setPen(pen);
    pp.fillRect(pp.window(), QColor("white"));
    pp.drawLine(1,ppix.height()/2,ppix.width()-1, ppix.height()/2);
    _comboMarkerLineStyle->insertItem(ppix);
  }

  _comboMarkerLineStyle->setCurrentItem(currentItem);
}


void Kst2dPlotWidget::updateAxesButtons() {
  bool major = _xMajorGrid->isChecked() || _yMajorGrid->isChecked();
  bool minor = _xMinorGrid->isChecked() || _yMinorGrid->isChecked();

  _checkBoxDefaultMajorGridColor->setEnabled(major);
  _checkBoxDefaultMinorGridColor->setEnabled(minor);
  _majorGridColor->setEnabled(major && _checkBoxDefaultMajorGridColor->state() != QButton::On);
  _minorGridColor->setEnabled(minor && _checkBoxDefaultMinorGridColor->state() != QButton::On);

  _majorPenWidth->setEnabled(major);
  _minorPenWidth->setEnabled(minor);
  _majorPenWidthLabel->setEnabled(major);
  _minorPenWidthLabel->setEnabled(minor);

  _xMinorTicks->setEnabled(_xMinorTicksAuto->state() != QButton::On);
  _yMinorTicks->setEnabled(_yMinorTicksAuto->state() != QButton::On);
}


void Kst2dPlotWidget::updateScalarCombo() {
  ScalarList->clear();
  scalarSelectorX1->clear();
  scalarSelectorX2->clear();
  scalarSelectorY1->clear();
  scalarSelectorY2->clear();
  KST::scalarList.lock().readLock();
  KstScalarList sl = QDeepCopy<KstScalarList>(KST::scalarList.list());
  KST::scalarList.lock().unlock();
  qHeapSort(sl);
  for (KstScalarList::ConstIterator i = sl.begin(); i != sl.end(); ++i) {
    (*i)->readLock();
    QString n = (*i)->tag().displayString();
    (*i)->unlock();
    ScalarList->insertItem(n);
    scalarSelectorX1->insertItem(n);
    scalarSelectorX2->insertItem(n);
    scalarSelectorY1->insertItem(n);
    scalarSelectorY2->insertItem(n);
  }
}

void Kst2dPlotWidget::updatePlotMarkers(const Kst2DPlot *plot) {
  for (KstMarkerList::ConstIterator it = plot->plotMarkers().begin(); it != plot->plotMarkers().end(); ++it) {
    if ((*it).isRising) {
      PlotMarkerList->insertItem(i18n("%1 [rising]").arg(QString::number((*it).value, 'g', MARKER_LABEL_PRECISION)));
    } else if ((*it).isFalling) {
      PlotMarkerList->insertItem(i18n("%1 [falling]").arg(QString::number((*it).value, 'g', MARKER_LABEL_PRECISION)));
    } else if ((*it).isVectorValue) {
      PlotMarkerList->insertItem(i18n("%1 [value]").arg(QString::number((*it).value, 'g', MARKER_LABEL_PRECISION)));
    } else {
      PlotMarkerList->insertItem(QString::number((*it).value, 'g', MARKER_LABEL_PRECISION));
    }
  }

  // update the auto-generation settings
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  CurveCombo->clear();
  for (KstBaseCurveList::ConstIterator curves_iter = curves.begin(); curves_iter != curves.end(); ++curves_iter) {
    (*curves_iter)->readLock();
    CurveCombo->insertItem((*curves_iter)->tagName());
    (*curves_iter)->unlock();
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
    for (int curveComboIndex = 0; curveComboIndex < CurveCombo->count(); curveComboIndex++) {
      if (CurveCombo->text(curveComboIndex) == plot->curveToMarkers()->tagName()) {
        CurveCombo->setCurrentItem(curveComboIndex);
        break;
      }
    }
  } else {
    UseCurve->setChecked(false);
  }

  if (plot->hasVectorToMarkers()) {
    UseVector->setChecked(true);
    _vectorForMarkers->setSelection(plot->vectorToMarkers()->tag().displayString());
  } else {
    UseVector->setChecked(false);
  }
}

void Kst2dPlotWidget::populateEditMultiple(QRadioButton *radioButtonWidget) {
  radioButtonWidget->setChecked(false);
}

void Kst2dPlotWidget::populateEditMultiple(QComboBox *comboWidget) {
  comboWidget->insertItem(QString(" "));
  comboWidget->setCurrentItem(comboWidget->count()-1);
}

void Kst2dPlotWidget::populateEditMultiple(KColorButton *colorButton) {
  colorButton->setColor(QColor());
}

void Kst2dPlotWidget::populateEditMultiple(QLineEdit *lineEditWidget) {
  lineEditWidget->setText(QString(" "));
}

void Kst2dPlotWidget::populateEditMultiple(QSpinBox *spinBoxWidget) {
  spinBoxWidget->setMinValue(spinBoxWidget->minValue() - 1);
  spinBoxWidget->setSpecialValueText(QString(" "));
  spinBoxWidget->setValue(spinBoxWidget->minValue());
}

void Kst2dPlotWidget::populateEditMultiple(QCheckBox *checkBoxWidget) {
  checkBoxWidget->setTristate();
  checkBoxWidget->setNoChange();
}

void Kst2dPlotWidget::populateEditMultiple(const Kst2DPlot *plot) {
  Q_UNUSED(plot)

  _editMultipleMode = true;

  //
  // Content tab...
  //
  _title->setText(QString(""));
  _title->setEnabled(false);
  _up->setEnabled(false);
  _down->setEnabled(false);
  _add->setEnabled(false);
  _remove->setEnabled(false);
  AvailableCurveList->clear();
  DisplayedCurveList->clear();
  AvailableCurveList->setEnabled(false);
  DisplayedCurveList->setEnabled(false);

  //
  // Appearance tab...
  //
  populateEditMultiple(_checkBoxDefaultMajorGridColor);
  populateEditMultiple(_checkBoxDefaultMinorGridColor);
  populateEditMultiple(ShowLegend);
  populateEditMultiple(TrackContents);
  populateEditMultiple(_axisPenWidth);
  populateEditMultiple(_majorPenWidth);
  populateEditMultiple(_minorPenWidth);
  populateEditMultiple(TopLabelFontSize);
  populateEditMultiple(YLabelFontSize);
  populateEditMultiple(XLabelFontSize);
  populateEditMultiple(NumberFontSize);
  populateEditMultiple(_spinBoxXAngle);
  populateEditMultiple(_spinBoxYAngle);
  populateEditMultiple(TopLabelText);
  populateEditMultiple(YAxisText);
  populateEditMultiple(XAxisText);
  populateEditMultiple(TopLabelText);
  populateEditMultiple(_majorGridColor);
  populateEditMultiple(_minorGridColor);
  populateEditMultiple(_comboBoxTopLabelJustify);
  populateEditMultiple(FontComboBox);

  //
  // X Axis tab...
  //
  populateEditMultiple(_suppressTop);
  populateEditMultiple(_suppressBottom);
  populateEditMultiple(XIsLog);
  populateEditMultiple(_checkBoxXOffsetMode);
  populateEditMultiple(_xReversed);
  populateEditMultiple(_checkBoxXInterpret);
  populateEditMultiple(_xTransformTop);
  populateEditMultiple(_xMinorTicksAuto);
  populateEditMultiple(_xMajorGrid);
  populateEditMultiple(_xMinorGrid);
  populateEditMultiple(_xMinorTicks);
  populateEditMultiple(_xTransformTopExp);
  populateEditMultiple(_comboBoxXInterpret);
  populateEditMultiple(_comboBoxXDisplay);
  populateEditMultiple(_xMajorTickSpacing);
  populateEditMultiple(_xMarksInsidePlot);
  populateEditMultiple(_xMarksOutsidePlot);
  populateEditMultiple(_xMarksInsideAndOutsidePlot);

  //
  // Y Axis tab...
  //
  populateEditMultiple(_suppressLeft);
  populateEditMultiple(_suppressRight);
  populateEditMultiple(YIsLog);
  populateEditMultiple(_checkBoxYOffsetMode);
  populateEditMultiple(_yReversed);
  populateEditMultiple(_checkBoxYInterpret);
  populateEditMultiple(_yTransformRight);
  populateEditMultiple(_yMinorTicksAuto);
  populateEditMultiple(_yMajorGrid);
  populateEditMultiple(_yMinorGrid);
  populateEditMultiple(_yMinorTicks);
  populateEditMultiple(_yTransformRightExp);
  populateEditMultiple(_comboBoxYInterpret);
  populateEditMultiple(_comboBoxYDisplay);
  populateEditMultiple(_yMajorTickSpacing);
  populateEditMultiple(_yMarksInsidePlot);
  populateEditMultiple(_yMarksOutsidePlot);
  populateEditMultiple(_yMarksInsideAndOutsidePlot);

  //
  // Range tab...
  //
  populateEditMultiple(XExpressionMin);
  populateEditMultiple(XExpressionMax);
  populateEditMultiple(YExpressionMin);
  populateEditMultiple(YExpressionMax);
  populateEditMultiple(XAuto);
  populateEditMultiple(XAutoBorder);
  populateEditMultiple(XAutoUp);
  populateEditMultiple(XNoSpikes);
  populateEditMultiple(XAC);
  populateEditMultiple(XExpression);
  populateEditMultiple(YAuto);
  populateEditMultiple(YAutoBorder);
  populateEditMultiple(YAutoUp);
  populateEditMultiple(YNoSpikes);
  populateEditMultiple(YAC);
  populateEditMultiple(YExpression);

  //
  // Markers tab...
  //
  populateEditMultiple(_checkBoxDefaultMarkerColor);
  populateEditMultiple(_spinBoxMarkerLineWidth);
  populateEditMultiple(_colorMarker);
  populateEditMultiple(_comboMarkerLineStyle);
  UseCurve->setEnabled(false);
  UseVector->setEnabled(false);
  CurveCombo->setEnabled(false);
  _vectorForMarkers->setEnabled(false);
  Rising->setEnabled(false);
  Falling->setEnabled(false);
  Both->setEnabled(false);
  AddPlotMarker->setEnabled(false);
  RemovePlotMarker->setEnabled(false);
  RemoveAllPlotMarkers->setEnabled(false);
  NewPlotMarker->setEnabled(false);
  PlotMarkerList->setEnabled(false);

  updateAxesButtons();
}

void Kst2dPlotWidget::fillWidget(const Kst2DPlot *plot) {
  _plot = Kst2DPlot::findPlotByName(plot->tagName());
  _vectorForMarkers->update();
  scalarSelectorX1->update();
  scalarSelectorY1->update();
  scalarSelectorX2->update();
  scalarSelectorY2->update();

  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  DisplayedCurveList->clear();
  AvailableCurveList->clear();

  // add curves while retaining the order in the plot
  for (KstBaseCurveList::ConstIterator it = plot->Curves.begin(); it != plot->Curves.end(); ++it) {
    (*it)->readLock();
    DisplayedCurveList->insertItem((*it)->tagName());
    (*it)->unlock();
  }
  for (KstBaseCurveList::ConstIterator it = curves.begin(); it != curves.end(); ++it) {
    (*it)->readLock();
    if (plot->Curves.find(*it) == plot->Curves.end()) {
      AvailableCurveList->insertItem((*it)->tagName());
    }
    (*it)->unlock();
  }

  updateScalarCombo();
  updatePlotMarkers(plot);

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

  // insert the current plot name in the plot name edit box
  _title->setText(plot->tagName());

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

  //void KstPlotDialogI::setMajorSpacing(QComboBox* majorTicks, int spacing) {

  for (int i = 0; i < (int)numMajorTickSpacings; ++i) {
    if (MajorTickSpacings[i].majorTickDensity <= plot->xMajorTicks()) {
      _xMajorTickSpacing->setCurrentItem(i);
    }
    if (MajorTickSpacings[i].majorTickDensity <= plot->yMajorTicks()) {
      _yMajorTickSpacing->setCurrentItem(i);
    }
  }

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
    for (unsigned i = 0; i < numAxisInterpretations; ++i) {
      if (AxisInterpretations[i].type == xAxisInterpretation) {
        _comboBoxXInterpret->setCurrentItem(i);
        break;
      }
    }
    for (unsigned i = 0; i < numAxisDisplays; ++i) {
      if (AxisDisplays[i].type == xAxisDisplay) {
        _comboBoxXDisplay->setCurrentItem(i);
        break;
      }
    }
  } else {
    _comboBoxXInterpret->setCurrentItem(KstSettings::globalSettings()->xAxisInterpretation);
    _comboBoxXDisplay->setCurrentItem(KstSettings::globalSettings()->xAxisDisplay);
  }

  plot->getYAxisInterpretation(yAxisInterpret, yAxisInterpretation, yAxisDisplay);
  _checkBoxYInterpret->setChecked(yAxisInterpret);
  _comboBoxYInterpret->setEnabled(yAxisInterpret);
  _comboBoxYDisplay->setEnabled(yAxisInterpret);
  textLabelYDisplayAs->setEnabled(yAxisInterpret);

  if (yAxisInterpret) {
    for (unsigned i = 0; i < numAxisInterpretations; ++i) {
      if (AxisInterpretations[i].type == yAxisInterpretation) {
        _comboBoxYInterpret->setCurrentItem(i);
        break;
      }
    }
    for (unsigned i = 0; i < numAxisDisplays; ++i) {
      if (AxisDisplays[i].type == yAxisDisplay) {
        _comboBoxYDisplay->setCurrentItem(i);
        break;
      }
    }
  } else {
    // FIXME: these should use kstsettings defaults
    _comboBoxYInterpret->setCurrentItem(KstSettings::globalSettings()->xAxisInterpretation);
    _comboBoxYDisplay->setCurrentItem(KstSettings::globalSettings()->xAxisDisplay);
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

  // update the tick display options
  _xMarksInsidePlot->setChecked(plot->xTicksInPlot() && !plot->xTicksOutPlot());
  _xMarksOutsidePlot->setChecked(plot->xTicksOutPlot() && !plot->xTicksInPlot());
  _xMarksInsideAndOutsidePlot->setChecked(plot->xTicksOutPlot() && plot->xTicksInPlot());

  _yMarksInsidePlot->setChecked(plot->yTicksInPlot() && !plot->yTicksOutPlot());
  _yMarksOutsidePlot->setChecked(plot->yTicksOutPlot() && !plot->yTicksInPlot());
  _yMarksInsideAndOutsidePlot->setChecked(plot->yTicksOutPlot() && plot->yTicksInPlot());

  // update axis suppression
  _suppressTop->setChecked(plot->suppressTop());
  _suppressBottom->setChecked(plot->suppressBottom());
  _suppressRight->setChecked(plot->suppressRight());
  _suppressLeft->setChecked(plot->suppressLeft());

  // update axes transforms
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

  updateButtons();
  updateAxesButtons();
  _scalarDest = TopLabelText;
}

void Kst2dPlotWidget::applyContents(Kst2DPlotPtr plot) {
  if (!_editMultipleMode) {
    plot->clearCurves();
    KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
    for (unsigned i = 0; i < DisplayedCurveList->count(); ++i) {
      KstBaseCurveList::Iterator it = curves.findTag(DisplayedCurveList->text(i));
      if (it != curves.end()) {
        plot->addCurve(*it);
      }
    }
    curves.clear();
  }
}

void Kst2dPlotWidget::applyAppearance(Kst2DPlotPtr plot) {
  if (!_editMultipleMode || XAxisText->text().compare(QString(" ")) != 0) {
    plot->xLabel()->setText(XAxisText->text());
  }
  if (!_editMultipleMode || YAxisText->text().compare(QString(" ")) != 0) {
    plot->yLabel()->setText(YAxisText->text());
  }
  if (!_editMultipleMode || TopLabelText->text().compare(QString(" ")) != 0) {
    plot->topLabel()->setText(TopLabelText->text());
  }

  if (!_editMultipleMode || _comboBoxTopLabelJustify->currentText().compare(QString(" ")) != 0) {
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
  }

  plot->setForegroundColor(plotColors->foreground());
  plot->setBackgroundColor(plotColors->background());

  QColor majorGridColor = plot->majorGridColor();
  QColor minorGridColor = plot->minorGridColor();
  bool defaultMajorGridColor = plot->defaultMajorGridColor();
  bool defaultMinorGridColor = plot->defaultMinorGridColor();

  if (_checkBoxDefaultMajorGridColor->state() == QButton::On) {
    defaultMajorGridColor = true;
  } else if (_checkBoxDefaultMajorGridColor->state() == QButton::Off) {
    defaultMajorGridColor = false;
  }

  if (_checkBoxDefaultMinorGridColor->state() == QButton::On) {
    defaultMinorGridColor = true;
  } else if (_checkBoxDefaultMinorGridColor->state() == QButton::Off) {
    defaultMinorGridColor = false;
  }

  if (_majorGridColor->color() != QColor()) {
    majorGridColor = _majorGridColor->color();
  }
  if (_minorGridColor->color() != QColor()) {
    minorGridColor = _minorGridColor->color();
  }

  plot->setGridLinesColor(majorGridColor, minorGridColor, defaultMajorGridColor, defaultMinorGridColor);

  if (!_editMultipleMode || _axisPenWidth->value() != _axisPenWidth->minValue()) {
    plot->setAxisPenWidth(_axisPenWidth->value());
  }
  if (!_editMultipleMode || _majorPenWidth->value() != _majorPenWidth->minValue()) {
    plot->setMajorPenWidth(_majorPenWidth->value());
  }
  if (!_editMultipleMode || _minorPenWidth->value() != _minorPenWidth->minValue()) {
    plot->setMinorPenWidth(_minorPenWidth->value());
  }

  if(FontComboBox->currentText().compare(QString(" ")) != 0) {
    plot->xLabel()->setFontName(FontComboBox->currentText());
    plot->yLabel()->setFontName(FontComboBox->currentText());
    plot->topLabel()->setFontName(FontComboBox->currentText());
  }

  if (!_editMultipleMode || XLabelFontSize->value() != XLabelFontSize->minValue()) {
    plot->xLabel()->setFontSize(XLabelFontSize->value());
  }

  if (!_editMultipleMode || YLabelFontSize->value() != YLabelFontSize->minValue()) {
    plot->yLabel()->setFontSize(YLabelFontSize->value());
  }

  if (!_editMultipleMode || TopLabelFontSize->value() != TopLabelFontSize->minValue()) {
    plot->topLabel()->setFontSize(TopLabelFontSize->value());
  }

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

  plot->fullTickLabel()->setFontName(FontComboBox->currentText());
  if (!_editMultipleMode || NumberFontSize->value() != NumberFontSize->minValue()) {
    plot->fullTickLabel()->setFontSize(NumberFontSize->value());
    plot->xTickLabel()->setFontSize(NumberFontSize->value());
    plot->yTickLabel()->setFontSize(NumberFontSize->value());
  }

  plot->xTickLabel()->setFontName(FontComboBox->currentText());
  if (!_editMultipleMode || _spinBoxXAngle->value() != _spinBoxXAngle->minValue()) {
    plot->xTickLabel()->setRotation(_spinBoxXAngle->value());
  }

  plot->yTickLabel()->setFontName(FontComboBox->currentText());
  if (!_editMultipleMode || _spinBoxYAngle->value() != _spinBoxYAngle->minValue()) {
    plot->yTickLabel()->setRotation(_spinBoxYAngle->value());
  }

  if (ShowLegend->state() == QButton::On) {
    KstViewLegendPtr vl = plot->getOrCreateLegend();
    if (vl) {
      if (TrackContents->state() == QButton::On) {
        vl->setTrackContents(true);
      } else if (TrackContents->state() == QButton::Off) {
        vl->setTrackContents(false);
      }
    }
  } else if (ShowLegend->state() == QButton::Off) {
    KstViewLegendPtr vl = plot->legend();
    if (vl) {
      plot->removeChild(KstViewObjectPtr(vl));
    }
  }

  plot->setDirty();
}

void Kst2dPlotWidget::applyXAxis(Kst2DPlotPtr plot) {
  if (_checkBoxXOffsetMode->state() == QButton::On) {
    plot->setXOffsetMode(true);
  } else if (_checkBoxXOffsetMode->state() == QButton::Off) {
    plot->setXOffsetMode(false);
  }

  KstAxisInterpretation xAxisInterpretation;
  KstAxisDisplay xAxisDisplay;
  bool isXAxisInterpreted;

  plot->getXAxisInterpretation(isXAxisInterpreted, xAxisInterpretation, xAxisDisplay);
  if (_checkBoxXInterpret->state() == QButton::On || 
      (_checkBoxXInterpret->state() == QButton::NoChange && isXAxisInterpreted)) {
    if (_comboBoxXInterpret->currentText().compare(QString(" ")) != 0) {
      xAxisInterpretation = AxisInterpretations[_comboBoxXInterpret->currentItem()].type;
    }
    if (_comboBoxXDisplay->currentText().compare(QString(" ")) != 0) {
      xAxisDisplay = AxisDisplays[_comboBoxXDisplay->currentItem()].type;
    }
    plot->setXAxisInterpretation(true, xAxisInterpretation, xAxisDisplay);
  } else if (_checkBoxXInterpret->state() == QButton::Off) {
    plot->setXAxisInterpretation(false, AXIS_INTERP_CTIME, AXIS_DISPLAY_YEAR);
  }

  if (_xMinorTicksAuto->state() == QButton::On) {
    plot->setXMinorTicks(-1);
  } else if (_xMinorTicksAuto->state() == QButton::Off) {
    plot->setXMinorTicks(_xMinorTicks->value());
  } else if (_xMinorTicksAuto->state() == QButton::NoChange && !plot->xMinorTicksAuto()) {
    plot->setXMinorTicks(_xMinorTicks->value());
  }

  if (!_editMultipleMode || _xMajorTickSpacing->currentText().compare(QString(" ")) != 0) {
    plot->setXMajorTicks(MajorTickSpacings[_xMajorTickSpacing->currentItem()].majorTickDensity);
  }
  if (!_editMultipleMode || _xMarksInsidePlot->isChecked() || _xMarksOutsidePlot->isChecked() || _xMarksInsideAndOutsidePlot->isChecked()) {
    plot->setXTicksInPlot(_xMarksInsidePlot->isChecked() || _xMarksInsideAndOutsidePlot->isChecked());
    plot->setXTicksOutPlot(_xMarksOutsidePlot->isChecked() || _xMarksInsideAndOutsidePlot->isChecked());
  }

  bool xmajorgrid = plot->hasXMajorGrid();
  bool xminorgrid = plot->hasXMinorGrid();

  if (_xMajorGrid->state() == QButton::On) {
    xmajorgrid = true;
  } else if (_xMajorGrid->state() == QButton::Off) {
    xmajorgrid = false;
  }
  if (_xMinorGrid->state() == QButton::On) {
    xminorgrid = true;
  } else if (_xMinorGrid->state() == QButton::Off) {
    xminorgrid = false;
  }
  plot->setXGridLines(xmajorgrid, xminorgrid);

  if (_suppressTop->state() == QButton::On) {
    plot->setSuppressTop(true);
  } else if (_suppressTop->state() == QButton::Off) {
    plot->setSuppressTop(false);
  }

  if (_suppressBottom->state() == QButton::On) {
    plot->setSuppressBottom(true);
  } else if (_suppressBottom->state() == QButton::Off) {
    plot->setSuppressBottom(false);
  }

  if (_xTransformTop->state() == QButton::On || 
      (_xTransformTop->state() == QButton::NoChange && !plot->xTransformedExp().isNull())) {
    if (_xTransformTopExp->text() != QString(" ")) {
      plot->setXTransformedExp(_xTransformTopExp->text());
    }
  } else if (_xTransformTop->state() == QButton::Off) {
    plot->setXTransformedExp(QString::null);
  }

  if (_xReversed->state() == QButton::On) {
    plot->setXReversed(true);
  } else if (_xReversed->state() == QButton::Off) {
    plot->setXReversed(false);
  }

  plot->setDirty();
}

void Kst2dPlotWidget::applyYAxis(Kst2DPlotPtr plot) {
  if (_checkBoxYOffsetMode->state() == QButton::On) {
    plot->setYOffsetMode(true);
  } else if (_checkBoxYOffsetMode->state() == QButton::Off) {
    plot->setYOffsetMode(false);
  }

  KstAxisInterpretation yAxisInterpretation;
  KstAxisDisplay yAxisDisplay;
  bool isYAxisInterpreted;

  plot->getYAxisInterpretation(isYAxisInterpreted, yAxisInterpretation, yAxisDisplay);
  if (_checkBoxYInterpret->state() == QButton::On ||
      (_checkBoxYInterpret->state() == QButton::NoChange && isYAxisInterpreted)) {
    if (_comboBoxYInterpret->currentText().compare(QString(" ")) != 0) {
      yAxisInterpretation = AxisInterpretations[_comboBoxYInterpret->currentItem()].type;
    }
    if (_comboBoxYDisplay->currentText().compare(QString(" ")) != 0) {
      yAxisDisplay = AxisDisplays[_comboBoxYDisplay->currentItem()].type;
    }
    plot->setYAxisInterpretation(true, yAxisInterpretation, yAxisDisplay);
  } else if (_checkBoxYInterpret->state() == QButton::Off) {
    plot->setYAxisInterpretation(false, AXIS_INTERP_CTIME, AXIS_DISPLAY_YEAR);
  }

  if (_yMinorTicksAuto->state() == QButton::On) {
    plot->setYMinorTicks(-1);
  } else if (_yMinorTicksAuto->state() == QButton::Off) {
    plot->setYMinorTicks(_yMinorTicks->value());
  } else if (_yMinorTicksAuto->state() == QButton::NoChange && !plot->yMinorTicksAuto()) {
    plot->setYMinorTicks(_yMinorTicks->value());
  }

  if (!_editMultipleMode || _yMajorTickSpacing->currentText().compare(QString(" ")) != 0) {
    plot->setYMajorTicks(MajorTickSpacings[_yMajorTickSpacing->currentItem()].majorTickDensity);
  }

  if (!_editMultipleMode || _yMarksInsidePlot->isChecked() || _yMarksOutsidePlot->isChecked() || _yMarksInsideAndOutsidePlot->isChecked()) {
    plot->setYTicksInPlot(_yMarksInsidePlot->isChecked() || _yMarksInsideAndOutsidePlot->isChecked());
    plot->setYTicksOutPlot(_yMarksOutsidePlot->isChecked() || _yMarksInsideAndOutsidePlot->isChecked());
  }

  bool ymajorgrid = plot->hasYMajorGrid();
  bool yminorgrid = plot->hasYMinorGrid();

  if (_yMajorGrid->state() == QButton::On) {
    ymajorgrid = true;
  } else if (_yMajorGrid->state() == QButton::Off) {
    ymajorgrid = false;
  }
  if (_yMinorGrid->state() == QButton::On) {
    yminorgrid = true;
  } else if (_yMinorGrid->state() == QButton::Off) {
    yminorgrid = false;
  }
  plot->setYGridLines(ymajorgrid, yminorgrid);

  if (_suppressLeft->state() == QButton::On) {
    plot->setSuppressLeft(true);
  } else if (_suppressLeft->state() == QButton::Off) {
    plot->setSuppressLeft(false);
  }

  if (_suppressRight->state() == QButton::On) {
    plot->setSuppressRight(true);
  } else if (_suppressRight->state() == QButton::Off) {
    plot->setSuppressRight(false);
  }

  if (_yTransformRight->state() == QButton::On || 
      (_yTransformRight->state() == QButton::NoChange && !plot->yTransformedExp().isNull())) {
    if (_yTransformRightExp->text() != QString(" ")) {
      plot->setYTransformedExp(_yTransformRightExp->text());
    }
  } else if (_yTransformRight->state() == QButton::Off) {
    plot->setYTransformedExp(QString::null);
  }

  if (_yReversed->state() == QButton::On) {
    plot->setYReversed(true);
  } else if (_yReversed->state() == QButton::Off) {
    plot->setYReversed(false);
  }

  plot->setDirty();
}

void Kst2dPlotWidget::applyRange(Kst2DPlotPtr plot) {
  if (XAC->isChecked() || plot->xScaleMode() == AC) {
    plot->setXScaleMode(AC);
    if (!_editMultipleMode || XACRange->text() != QString(" ")) {
      plot->setXScale(0, XACRange->text().toDouble());
    }
  } else if (XExpression->isChecked() || plot->xScaleMode() == EXPRESSION) {
    plot->setXScaleMode(EXPRESSION);
    if (!_editMultipleMode || (XExpressionMin->text() != QString(" ") && XExpressionMax->text() != QString(" "))) {
      if (!plot->setXExpressions(XExpressionMin->text(), XExpressionMax->text())) {
        if (!_editMultipleMode) {
          KMessageBox::sorry(this, i18n("There is a problem with the X range expressions."));
          return;
        }
      }
    }
    // if expressions are constant, just use FIXED mode
    plot->optimizeXExps();
  } else if (XAutoUp->isChecked()) {
    plot->setXScaleMode(AUTOUP);
  } else if (XAuto->isChecked()) {
    plot->setXScaleMode(AUTO);
  } else if (XAutoBorder->isChecked()) {
    plot->setXScaleMode(AUTOBORDER);
  } else if (XNoSpikes->isChecked()) {
    plot->setXScaleMode(NOSPIKE);
  } else if (!_editMultipleMode) {
    KstDebug::self()->log(i18n("Internal error: No X scale type checked in %1.").arg(_title->text()), KstDebug::Error);
  }

  if (YAC->isChecked() || plot->yScaleMode() == AC) {
    plot->setYScaleMode(AC);
    if (!_editMultipleMode || YACRange->text() != QString(" ")) {
      plot->setYScale(0, YACRange->text().toDouble());
    }
  } else if (YExpression->isChecked() || plot->yScaleMode() == EXPRESSION) {
    plot->setYScaleMode(EXPRESSION);
    if (!_editMultipleMode || (YExpressionMin->text() != QString(" ") && YExpressionMax->text() != QString(" "))) {
      if (!plot->setYExpressions(YExpressionMin->text(), YExpressionMax->text())) {
        if (!_editMultipleMode) {
          KMessageBox::sorry(this, i18n("There is a problem with the Y range expressions."));
          return;
        }
      }
    }
    // if expressions are constant, just use FIXED mode
    plot->optimizeYExps();
  } else if (YAutoUp->isChecked()) {
    plot->setYScaleMode(AUTOUP);
  } else if (YAuto->isChecked()) {
    plot->setYScaleMode(AUTO);
  } else if (YAutoBorder->isChecked()) {
    plot->setYScaleMode(AUTOBORDER);
  } else if (YNoSpikes->isChecked()) {
    plot->setYScaleMode(NOSPIKE);
  } else if (!_editMultipleMode) {
    KstDebug::self()->log(i18n( "Internal error: No Y scale type checked in %1." ).arg(_title->text()), KstDebug::Error);
  }

  plot->setDirty();
}

void Kst2dPlotWidget::addPlotMarker() {
  // silently do nothing if there is no text, to
  // be consistent with "Add" button disabled
  if (!NewPlotMarker->text().isEmpty()) {
    bool ok;
    double newMarkerVal = NewPlotMarker->text().toDouble(&ok);

    if (ok) {
      uint i = 0;
      QString stringnum;

      stringnum.setNum(newMarkerVal, 'g', MARKER_LABEL_PRECISION);
      while (i < PlotMarkerList->count() && PlotMarkerList->text(i).toDouble() < newMarkerVal) {
        ++i;
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

void Kst2dPlotWidget::removePlotMarker() {
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

void Kst2dPlotWidget::removeAllPlotMarkers() {
  uint count = PlotMarkerList->count();
  if (count > 0) {
    for (int i = count-1; i >= 0; i--) {
      PlotMarkerList->removeItem(i);
    }
    updateButtons();
  }
}

void Kst2dPlotWidget::applyPlotMarkers(Kst2DPlotPtr plot) {
  if (!_editMultipleMode) {
    KstMarker marker;
    KstMarkerList newMarkers;

    marker.isRising = false;
    marker.isFalling = false;
    marker.isVectorValue = false;
    for (unsigned i = 0; i < PlotMarkerList->count(); ++i) {
      marker.value = PlotMarkerList->text(i).toDouble();
      // FIXME: this is very broken.  you can't search for i18n() substrings!
      if (PlotMarkerList->text(i).find( i18n("rising")) == -1 &&
              PlotMarkerList->text(i).find( i18n("falling")) == -1 &&
              PlotMarkerList->text(i).find( i18n("value")) == -1) {
        newMarkers.append(marker);
      }
    }

    plot->setPlotMarkerList(newMarkers);

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
      plot->setCurveToMarkers(curve, rising, falling);
    } else {
      plot->removeCurveToMarkers();
    }
  }

  if (UseVector->isChecked()) {
    KstVectorPtr vector = *(KST::vectorList.findTag(_vectorForMarkers->selectedVector()));
    plot->setVectorToMarkers(vector);
  } else {
    plot->removeVectorToMarkers();
  }

  plot->setLineStyleMarkers(_comboMarkerLineStyle->currentItem());
  if (!_editMultipleMode || _spinBoxMarkerLineWidth->value() != _spinBoxMarkerLineWidth->minValue()) {
    plot->setLineWidthMarkers(_spinBoxMarkerLineWidth->value());
  }

  if (_colorMarker->color() != QColor()) {
    plot->setColorMarkers(_colorMarker->color());
  }

  if (_checkBoxDefaultMarkerColor->state() == QButton::On) {
    plot->setDefaultColorMarker(true);
  } else if (_checkBoxDefaultMarkerColor->state() == QButton::Off) {
    plot->setDefaultColorMarker(false);
  }

  plot->setDirty();
}

void Kst2dPlotWidget::fillPlot( Kst2DPlotPtr plot ) {
  applyContents(plot);
  applyAppearance(plot);

  bool xlog = plot->isXLog();
  bool ylog = plot->isYLog();

  if (XIsLog->state() == QButton::On) {
    xlog = true;
  } else if (XIsLog->state() == QButton::Off) {
    xlog = false;
  }
  if (YIsLog->state() == QButton::On) {
    ylog = true;
  } else if (YIsLog->state() == QButton::Off) {
    ylog = false;
  }
  plot->setLog(xlog, ylog);

  applyXAxis(plot);
  applyYAxis(plot);
  applyRange(plot);
  applyPlotMarkers(plot);

  if (!_editMultipleMode) {
    QString tag = _title->text().stripWhiteSpace();
    if (tag.isEmpty()) {
      plot->setTagName(KstObjectTag(KST::suggestPlotName(), KstObjectTag::globalTagContext));
    } else {
      plot->setTagName(KstObjectTag(tag+"randomtextasholdingpattern", KstObjectTag::globalTagContext));
      if (KstData::self()->viewObjectNameNotUnique(tag)) {
        int j = 1;

        while (KstData::self()->viewObjectNameNotUnique(tag+"-"+QString::number(j))) {
          j++;
        }
        tag = tag + "-" + QString::number(j);
      }
      plot->setTagName(KstObjectTag(tag, KstObjectTag::globalTagContext));
    }
  }

  plot->setDirty();
}

void Kst2dPlotWidget::insertCurrentScalar() {
  _scalarDest->insert(ScalarList->currentText());
}

void Kst2dPlotWidget::setScalarDestXLabel() {
  _scalarDest = XAxisText;
}

void Kst2dPlotWidget::setScalarDestYLabel() {
  _scalarDest = YAxisText;
}

void Kst2dPlotWidget::setScalarDestTopLabel() {
  _scalarDest = TopLabelText;
}

void Kst2dPlotWidget::editLegend() {
  KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(_plot->topLevelParent());
  _plot->getOrCreateLegend()->showDialog(tlv, false);
  ShowLegend->setChecked(true);
}

void Kst2dPlotWidget::insertXExpressionMin(const QString& strIn) {
  QString str;

  str = "["+strIn+"]";

  XExpressionMin->insert(str);
}

void Kst2dPlotWidget::insertYExpressionMin(const QString& strIn) {
  QString str;

  str = "["+strIn+"]";

  YExpressionMin->insert(str);
}

void Kst2dPlotWidget::insertXExpressionMax(const QString& strIn) {
  QString str;

  str = "["+strIn+"]";

  XExpressionMax->insert(str);
}

void Kst2dPlotWidget::insertYExpressionMax(const QString& strIn) {
  QString str;

  str = "["+strIn+"]";

  YExpressionMax->insert(str);
}


#include "kst2dplotwidget_i.moc"
