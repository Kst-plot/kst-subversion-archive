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

// xxx #include <kiconloader.h>

#include "kstlinestyle.h"
#include "kstmonochromedialog.h"

KstMonochromeDialog::KstMonochromeDialog(QWidget* parent, const char* name,
                                           bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  availableListBox->clear();
  selectedListBox->clear();
  availableListBox->insertItem(0, QObject::tr("Point Style"));  
  availableListBox->insertItem(0, QObject::tr("Line Style"));
  availableListBox->insertItem(0, QObject::tr("Line Width"));

  connect(_Cancel, SIGNAL(clicked()), this, SLOT(accept()));
  connect(enhanceReadability, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
  connect(_add, SIGNAL(clicked()), this, SLOT(addClicked()));
  connect(_up, SIGNAL(clicked()), this, SLOT(upClicked()));
  connect(_down, SIGNAL(clicked()), this, SLOT(downClicked()));
  connect(availableListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
  connect(selectedListBox, SIGNAL(highlighted(int)), this, SLOT(updateButtons()));
/* xxx
  _up->setIcon(BarIcon("up"));
  _down->setIcon(BarIcon("down"));
  _add->setIcon(BarIcon("forward"));
  _remove->setIcon(BarIcon("back"));
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
  _remove->setEnabled(selectedListBox->currentRow() >= 0);
  _add->setEnabled(availableListBox->currentRow() >= 0);
  _up->setEnabled(selectedListBox->currentRow() > 0);
  _down->setEnabled(selectedListBox->currentRow() < (int)selectedListBox->count() - 1);
}

void KstMonochromeDialog::setOptions(const QMap<QString,QString>& opts) {
  enhanceReadability->setChecked(opts["kst-plot-monochromesettings-enhancereadability"] == "1");

  availableListBox->clear();
  selectedListBox->clear();
  if (opts["kst-plot-monochromesettings-pointstyleorder"] == "-1") {
    availableListBox->insertItem(0, QObject::tr("Point Style"));
  } else {
    selectedListBox->insertItem(opts["kst-plot-monochromesettings-pointstyleorder"].toInt(), QObject::tr("Point Style"));
  }
  if (opts["kst-plot-monochromesettings-linestyleorder"] == "-1") {
    availableListBox->insertItem(0, QObject::tr("Line Style"));
  } else {
    selectedListBox->insertItem(opts["kst-plot-monochromesettings-linestyleorder"].toInt(), QObject::tr("Line Style"));
  }
  if (opts["kst-plot-monochromesettings-linewidthorder"] == "-1") {
    availableListBox->insertItem(0, QObject::tr("Line Width"));
  } else {
    selectedListBox->insertItem(opts["kst-plot-monochromesettings-linewidthorder"].toInt(), QObject::tr("Line Width"));
  }

  maxLineWidth->setValue(opts["kst-plot-monochromesettings-maxlinewidth"].toInt());
  pointDensity->setCurrentIndex(opts["kst-plot-monochromesettings-pointdensity"].toInt());
}

void KstMonochromeDialog::getOptions(QMap<QString,QString> &opts, bool include_def) {
  QList<QListWidgetItem*> items;
  int pointStyleOrder;
  int lineStyleOrder;
  int lineWidthOrder;

  if (enhanceReadability->isChecked() || include_def) {
    opts["kst-plot-monochromesettings-enhancereadability"] = enhanceReadability->isChecked() ? "1" : "0";
  }

  items = selectedListBox->findItems(QObject::tr("Point Style"), Qt::MatchExactly);
  if (!items.isEmpty()) {
    pointStyleOrder = selectedListBox->row(items.front());
    if (pointStyleOrder != 0 || include_def) {
      opts["kst-plot-monochromesettings-pointstyleorder"] = QString::number(pointStyleOrder);
    }
  }

  items = selectedListBox->findItems(QObject::tr("Line Style"), Qt::MatchExactly);
  if (!items.isEmpty()) {
    lineStyleOrder = selectedListBox->row(items.front());
    if (lineStyleOrder != 1 || include_def) {
      opts["kst-plot-monochromesettings-linestyleorder"] = QString::number(lineStyleOrder);
    }
  }

  items = selectedListBox->findItems(QObject::tr("Line Width"), Qt::MatchExactly);
  if (!items.isEmpty()) {
    lineWidthOrder = selectedListBox->row(items.front());
    if (lineWidthOrder != 2 || include_def) {
      opts["kst-plot-monochromesettings-linewidthorder"] = QString::number(lineWidthOrder);
    }
  }

  if (maxLineWidth->value() != 3 || include_def) {
    opts["kst-plot-monochromesettings-maxlinewidth"] = QString::number(maxLineWidth->value());
  }

  if (pointDensity->currentIndex() != 2 || include_def) {
    opts["kst-plot-monochromesettings-pointdensity"] = QString::number(pointDensity->currentIndex());
  }
}


void KstMonochromeDialog::removeClicked() {
  for (int i = 0; i < selectedListBox->count(); i++) {
    if (selectedListBox->item(i)->isSelected()) {
      availableListBox->insertItem(0, selectedListBox->item(i)->text());
      selectedListBox->removeItemWidget(selectedListBox->item(i)); 
      availableListBox->item((int)availableListBox->count() - 1)->setSelected(true); 
    }
  }
  updateButtons();
}


void KstMonochromeDialog::addClicked() {
  for (int i = 0; i < availableListBox->count(); i++) {
    if (availableListBox->item(i)->isSelected()) {
      selectedListBox->insertItem(0, availableListBox->item(i)->text());
      availableListBox->removeItemWidget(availableListBox->item(i));
      selectedListBox->item((int)selectedListBox->count() - 1)->setSelected(true);
    }
  }
  updateButtons();
}


void KstMonochromeDialog::upClicked() {
  QListWidgetItem *item;
  QString text;
  int row;

  item = selectedListBox->currentItem();
  row = selectedListBox->currentRow();
  text = selectedListBox->item(row)->text();
  selectedListBox->removeItemWidget(item);
  selectedListBox->insertItem(row-1, text);
  selectedListBox->item(row-1)->setSelected(true);
  updateButtons();
}


void KstMonochromeDialog::downClicked() {
  QListWidgetItem *item;
  QString text;
  int row;

  item = selectedListBox->currentItem();
  row = selectedListBox->currentRow();
  text = selectedListBox->item(row)->text();
  selectedListBox->removeItemWidget(item);
  selectedListBox->insertItem(row+1, text);
  selectedListBox->item(row+1)->setSelected(true);
  updateButtons();
}

#include "kstmonochromedialog.moc"
