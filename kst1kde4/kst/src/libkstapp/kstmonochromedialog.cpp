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

// xxx #include <qbuttongroup.h> 
#include <QCheckBox> 
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>

#include <kiconloader.h>

#include "kstlinestyle.h"
#include "kstmonochromedialog.h"

KstMonochromeDialog::KstMonochromeDialog(QWidget* parent, const char* name,
                                           bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  availableListBox->clear();
  selectedListBox->clear();
  availableListBox->insertItem(QObject::tr("Point Style"));  
  availableListBox->insertItem(QObject::tr("Line Style"));
  availableListBox->insertItem(QObject::tr("Line Width"));

  connect(_Cancel, SIGNAL(clicked()), this, SLOT(accept()));
  connect(enhanceReadability, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
  connect(_add, SIGNAL(clicked()), this, SLOT(addClicked()));
  connect(_up, SIGNAL(clicked()), this, SLOT(upClicked()));
  connect(_down, SIGNAL(clicked()), this, SLOT(downClicked()));
  connect(availableListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  connect(selectedListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
/* xxx
  _up->setPixmap(BarIcon("up"));
  _down->setPixmap(BarIcon("down"));
  _add->setPixmap(BarIcon("forward"));
  _remove->setPixmap(BarIcon("back"));
*/
  maxLineWidth->setMaximum(KSTLINESTYLE_MAXTYPE);
  maxLineWidth->setMinimum(1);

  updateMonochromeDialog();
}


KstMonochromeDialog::~KstMonochromeDialog() {
}


void KstMonochromeDialog::updateMonochromeDialog() {
  updateButtons();
}


void KstMonochromeDialog::showMonochromeDialog() {
  updateMonochromeDialog();
  show();
  raise();
}

void KstMonochromeDialog::updateButtons() {
  cycleOrderGroup->setEnabled(enhanceReadability->isChecked());
  optionsGroup->setEnabled(enhanceReadability->isChecked());
  _remove->setEnabled(selectedListBox->currentItem() >= 0);
  _add->setEnabled(availableListBox->currentItem() >= 0);
  _up->setEnabled(selectedListBox->currentItem() > 0);
  _down->setEnabled(selectedListBox->currentItem() < (int)selectedListBox->count() - 1);
}

void KstMonochromeDialog::setOptions(const QMap<QString,QString>& opts) {
  enhanceReadability->setChecked(opts["kst-plot-monochromesettings-enhancereadability"] == "1");

  availableListBox->clear();
  selectedListBox->clear();
  if (opts["kst-plot-monochromesettings-pointstyleorder"] == "-1") {
    availableListBox->insertItem(QObject::tr("Point Style"));
  } else {
    selectedListBox->insertItem(QObject::tr("Point Style"),
    opts["kst-plot-monochromesettings-pointstyleorder"].toInt());
  }
  if (opts["kst-plot-monochromesettings-linestyleorder"] == "-1") {
    availableListBox->insertItem(QObject::tr("Line Style"));
  } else {
    selectedListBox->insertItem(QObject::tr("Line Style"),
    opts["kst-plot-monochromesettings-linestyleorder"].toInt());
  }
  if (opts["kst-plot-monochromesettings-linewidthorder"] == "-1") {
    availableListBox->insertItem(QObject::tr("Line Width"));
  } else {
    selectedListBox->insertItem(QObject::tr("Line Width"),
    opts["kst-plot-monochromesettings-linewidthorder"].toInt());
  }

  maxLineWidth->setValue(opts["kst-plot-monochromesettings-maxlinewidth"].toInt());
  pointDensity->setCurrentIndex(opts["kst-plot-monochromesettings-pointdensity"].toInt());
}

void KstMonochromeDialog::getOptions(QMap<QString,QString> &opts, bool include_def) {  
  int pointStyleOrder;
  int lineStyleOrder;
  int lineWidthOrder;

  if (enhanceReadability->isChecked() || include_def) {
    opts["kst-plot-monochromesettings-enhancereadability"] = enhanceReadability->isChecked() ? "1" : "0";
  }

  pointStyleOrder = selectedListBox->index(selectedListBox->findItem(QObject::tr("Point Style"), Qt::MatchExactly));
  if (pointStyleOrder != 0 || include_def) {
    opts["kst-plot-monochromesettings-pointstyleorder"] = QString::number(pointStyleOrder);
  }

  lineStyleOrder = selectedListBox->index(selectedListBox->findItem(QObject::tr("Line Style"), Qt::MatchExactly));
  if (lineStyleOrder != 1 || include_def) {
    opts["kst-plot-monochromesettings-linestyleorder"] = QString::number(lineStyleOrder);
  }

  lineWidthOrder = selectedListBox->index(selectedListBox->findItem(QObject::tr("Line Width"), Qt::MatchExactly));
  if (lineWidthOrder != 2 || include_def) {
    opts["kst-plot-monochromesettings-linewidthorder"] = QString::number(lineWidthOrder);
  }

  if (maxLineWidth->value() != 3 || include_def) {
    opts["kst-plot-monochromesettings-maxlinewidth"] = QString::number(maxLineWidth->value());
  }

  if (pointDensity->currentIndex() != 2 || include_def) {
    opts["kst-plot-monochromesettings-pointdensity"] = QString::number(pointDensity->currentIndex());
  }
}


void KstMonochromeDialog::removeClicked() {
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


void KstMonochromeDialog::addClicked() {
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


void KstMonochromeDialog::upClicked() {
  // move item up
  int i = selectedListBox->currentItem();
  QString text = selectedListBox->currentText();
  selectedListBox->removeItem(i);
  selectedListBox->insertItem(text, i-1);
  selectedListBox->setSelected(i-1, true);
  updateButtons();
}


void KstMonochromeDialog::downClicked() {
  // move item down
  int i = selectedListBox->currentItem();
  QString text = selectedListBox->currentText();
  selectedListBox->removeItem(i);
  selectedListBox->insertItem(text, i+1);
  selectedListBox->setSelected(i+1, true);
  updateButtons();
}

#include "kstmonochromedialog.moc"
