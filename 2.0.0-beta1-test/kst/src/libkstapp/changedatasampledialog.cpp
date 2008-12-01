/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "changedatasampledialog.h"

#include "datacollection.h"
#include "datavector.h"
#include "objectstore.h"
#include "document.h"
#include "mainwindow.h"

namespace Kst {

ChangeDataSampleDialog::ChangeDataSampleDialog(QWidget *parent)
  : QDialog(parent), _modified(false) {
   setupUi(this);

  if (MainWindow *mw = qobject_cast<MainWindow*>(parent)) {
    _store = mw->document()->objectStore();
  } else {
    // FIXME: we need the object store
    qFatal("ERROR: can't construct a ChangeDataSampleDialog without the object store");
  }

  connect(_clear, SIGNAL(clicked()), _curveList, SLOT(clearSelection()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
  connect(_curveList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(updateDefaults(QListWidgetItem*)));
  connect(_curveList, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtons()));

  connect(_dataRange, SIGNAL(modified()), this, SLOT(modified()));

  connect(_buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
  connect(_buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(OKClicked()));
  connect(_buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

  updateButtons();
}


ChangeDataSampleDialog::~ChangeDataSampleDialog() {
}


void ChangeDataSampleDialog::exec() {
  updateCurveListDialog();
  QDialog::exec();
}


void ChangeDataSampleDialog::updateButtons() {
  _buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_curveList->selectedItems().count() > 0 && (_modified || _curveList->selectedItems().count() > 1));
}


void ChangeDataSampleDialog::modified() {
  _modified = true;
  updateButtons();
}


void ChangeDataSampleDialog::updateCurveListDialog() {
  _curveList->clear();
  DataVectorList dataVectors = _store->getObjects<DataVector>();

  _curveList->blockSignals(true);
  for (DataVectorList::ConstIterator i = dataVectors.begin(); i != dataVectors.end(); ++i) {
    DataVectorPtr vector = *i;
    vector->readLock();
    QListWidgetItem *wi = new QListWidgetItem(vector->Name());
    wi->setToolTip(vector->descriptionTip());
    vector->unlock();
    _curveList->addItem(wi);
  }

  _curveList->selectAll();
  _curveList->blockSignals(false);
}


void ChangeDataSampleDialog::selectAll() {
  _curveList->selectAll();
}


void ChangeDataSampleDialog::updateDefaults(QListWidgetItem* item) {
  if (!item) {
    return;
  }

  if (DataVectorPtr vector = kst_cast<DataVector>(_store->retrieveObject(item->text()))) {
    vector->readLock();

    _dataRange->setCountFromEnd(vector->countFromEOF());
    _dataRange->setStart(vector->countFromEOF() ? 0 : vector->reqStartFrame());
    _dataRange->setReadToEnd(vector->readToEOF());
    _dataRange->setRange(vector->readToEOF() ? 0 : vector->reqNumFrames());
    _dataRange->setSkip(vector->skip());
    _dataRange->setDoSkip(vector->doSkip());
    _dataRange->setDoFilter(vector->doAve());

    vector->unlock();
  }

  _modified = false;
}


void ChangeDataSampleDialog::OKClicked() {
  if (_buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
    apply();
  }
  accept();
}


void ChangeDataSampleDialog::apply() {
  QList<QListWidgetItem*> selectedItems = _curveList->selectedItems();
  for (int i = 0; i < selectedItems.size(); ++i) {
    if (DataVectorPtr vector = kst_cast<DataVector>(_store->retrieveObject(selectedItems.at(i)->text()))) {
      vector->writeLock();
      vector->changeFrames( (_dataRange->countFromEnd() ? -1 : _dataRange->start()),
                            (_dataRange->readToEnd() ? -1 : _dataRange->range()),
                            _dataRange->skip(),
                            _dataRange->doSkip(),
                            _dataRange->doFilter());
      vector->immediateUpdate(); // FIXME: cache all dependent updates until all vectors have been updated
      vector->unlock();
    }
  }
  updateCurveListDialog();
}


}

// vim: ts=2 sw=2 et