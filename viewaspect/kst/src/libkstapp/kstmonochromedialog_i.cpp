/**************************************************************************
        kstmonochromedialog_i.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2005
    copyright            : (C) 2005 by University of British Columbia
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

#include <qbuttongroup.h> 
#include <qcheckbox.h> 
#include <qcombobox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <klocale.h>

#include "kstlinestyle.h"
#include "kstmonochromedialog_i.h"

KstMonochromeDialogI::KstMonochromeDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl)
: KstMonochromeDialog(parent, name, modal, fl) {

  availableListBox->clear();
  selectedListBox->clear();
  availableListBox->insertItem(i18n("Point Style"));  
  availableListBox->insertItem(i18n("Line Style"));
  availableListBox->insertItem(i18n("Line Width"));
  
  connect(_Cancel, SIGNAL(clicked()), this, SLOT(accept()));
  connect(enhanceReadability, SIGNAL(clicked()), this, SLOT(updateButtons()));
  
  // more connections to emulate kactionselector behaviour
  connect(leftButton, SIGNAL(clicked()), this, SLOT(leftButtonClicked()));
  connect(rightButton, SIGNAL(clicked()), this, SLOT(rightButtonClicked()));
  connect(upButton, SIGNAL(clicked()), this, SLOT(upButtonClicked()));
  connect(downButton, SIGNAL(clicked()), this, SLOT(downButtonClicked()));
  connect(availableListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  connect(selectedListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  
  maxLineWidth->setMaxValue(KSTLINESTYLE_MAXTYPE);
  maxLineWidth->setMinValue(1);
  updateMonochromeDialog();
}


KstMonochromeDialogI::~KstMonochromeDialogI() {
  
}

void KstMonochromeDialogI::updateMonochromeDialog() {
  updateButtons();
}


void KstMonochromeDialogI::showMonochromeDialog() {
  updateMonochromeDialog();
  show();
  raise();
}

void KstMonochromeDialogI::updateButtons() {
  cycleOrderGroup->setEnabled(enhanceReadability->isChecked());
  optionsGroup->setEnabled(enhanceReadability->isChecked());
  leftButton->setEnabled(selectedListBox->currentItem() >= 0);
  rightButton->setEnabled(availableListBox->currentItem() >= 0);
  upButton->setEnabled(selectedListBox->currentItem() > 0);
  downButton->setEnabled(selectedListBox->currentItem() < (int)selectedListBox->count() - 1);
}

void KstMonochromeDialogI::setOptions(const QMap<QString,QString>& opts) {
  
  enhanceReadability->setChecked(opts["kst-plot-monochromesettings-enhancereadability"] == "1");
  
  availableListBox->clear();
  selectedListBox->clear();
  if (opts["kst-plot-monochromesettings-pointstyleorder"] == "-1") {
    availableListBox->insertItem(i18n("Point Style"));
  } else {
    selectedListBox->insertItem(i18n("Point Style"),
    opts["kst-plot-monochromesettings-pointstyleorder"].toInt());
  }
  if (opts["kst-plot-monochromesettings-linestyleorder"] == "-1") {
    availableListBox->insertItem(i18n("Line Style"));
  } else {
    selectedListBox->insertItem(i18n("Line Style"),
    opts["kst-plot-monochromesettings-linestyleorder"].toInt());
  }
  if (opts["kst-plot-monochromesettings-linewidthorder"] == "-1") {
    availableListBox->insertItem(i18n("Line Width"));
  } else {
    selectedListBox->insertItem(i18n("Line Width"),
    opts["kst-plot-monochromesettings-linewidthorder"].toInt());
  }
  
  maxLineWidth->setValue(opts["kst-plot-monochromesettings-maxlinewidth"].toInt());
  pointDensity->setCurrentItem(opts["kst-plot-monochromesettings-pointdensity"].toInt());
}

void KstMonochromeDialogI::getOptions(QMap<QString,QString> &opts, bool include_def) {  
  // enhance readability - default is false
  if (enhanceReadability->isChecked() || include_def) {
    opts["kst-plot-monochromesettings-enhancereadability"] = enhanceReadability->isChecked() ? "1" : "0";
  }
  // point style order - default is 0
  int pointStyleOrder = selectedListBox->index(selectedListBox->findItem(i18n("Point Style"), ExactMatch));
  if (pointStyleOrder != 0 || include_def) {
    opts["kst-plot-monochromesettings-pointstyleorder"] = QString::number(pointStyleOrder);
  }
  // line style order - default is 1
  int lineStyleOrder = selectedListBox->index(selectedListBox->findItem(i18n("Line Style"), ExactMatch));
  if (lineStyleOrder != 1 || include_def) {
    opts["kst-plot-monochromesettings-linestyleorder"] = QString::number(lineStyleOrder);
  }
  // line width order - default is 2
  int lineWidthOrder = selectedListBox->index(selectedListBox->findItem(i18n("Line Width"), ExactMatch));
  if (lineWidthOrder != 2 || include_def) {
    opts["kst-plot-monochromesettings-linewidthorder"] = QString::number(lineWidthOrder);
  }
  // maximum line width - default is 3
  if (maxLineWidth->value() != 3 || include_def) {
    opts["kst-plot-monochromesettings-maxlinewidth"] = QString::number(maxLineWidth->value());
  }
  // point density - default is 2
  if (pointDensity->currentItem() != 2 || include_def) {
    opts["kst-plot-monochromesettings-pointdensity"] = QString::number(pointDensity->currentItem());
  }
}


void KstMonochromeDialogI::leftButtonClicked() {
  // move from selected to available
  for (uint i = 0; i < selectedListBox->count(); i++) {
    if (selectedListBox->isSelected(i)) {
      availableListBox->insertItem(selectedListBox->text(i));
      selectedListBox->removeItem(i); 
      availableListBox->setSelected((int)availableListBox->count() - 1, true); 
    }  
  }
  updateButtons();
}


void KstMonochromeDialogI::rightButtonClicked() {
  // move from available to selected
  for (uint i = 0; i < availableListBox->count(); i++) {
    if (availableListBox->isSelected(i)) {
      selectedListBox->insertItem(availableListBox->text(i));
      availableListBox->removeItem(i);  
      selectedListBox->setSelected((int)selectedListBox->count() - 1, true);
    }  
  }
  updateButtons();
}


void KstMonochromeDialogI::upButtonClicked() {
  // move item up
  int i = selectedListBox->currentItem();
  QString text = selectedListBox->currentText();
  selectedListBox->removeItem(i);
  selectedListBox->insertItem(text, i-1);
  selectedListBox->setSelected(i-1, true);
  updateButtons();
}


void KstMonochromeDialogI::downButtonClicked() {
  // move item down
  int i = selectedListBox->currentItem();
  QString text = selectedListBox->currentText();
  selectedListBox->removeItem(i);
  selectedListBox->insertItem(text, i+1);
  selectedListBox->setSelected(i+1, true);
  updateButtons();
}


#include "kstmonochromedialog_i.moc"
// vim: ts=2 sw=2 et
