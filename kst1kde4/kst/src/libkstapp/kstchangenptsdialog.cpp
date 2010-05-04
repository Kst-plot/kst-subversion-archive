/***************************************************************************
                    kstchangenptsdialog.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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

#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include "datarangewidget.h"
#include "kstchangenptsdialog.h"
#include "kstdatacollection.h"
#include "kstrvector.h"

KstChangeNptsDialog::KstChangeNptsDialog(
  QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  connect(Clear, SIGNAL(clicked()), this, SLOT(selectNone()));
  connect(SelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
  connect(Apply, SIGNAL(clicked()), this, SLOT(applyNptsChange()));
  connect(OK, SIGNAL(clicked()), this, SLOT(OKNptsChange()));
  connect(CurveList, SIGNAL(selectionChanged()), this, SLOT(changedSelection()));
  connect(Cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(CurveList, SIGNAL(selectionChanged()), this, SLOT(updateTimeCombo()));
  connect(_kstDataRange, SIGNAL(changed()), this, SLOT(modifiedRange()));

  _modifiedRange = false;
}


KstChangeNptsDialog::~KstChangeNptsDialog() {
}


void KstChangeNptsDialog::selectNone() {
  CurveList->clearSelection();

  OK->setEnabled(false);
  Apply->setEnabled(false);
}


void KstChangeNptsDialog::selectAll() {
  CurveList->selectAll();

  if (CurveList->count() > 0) {
    OK->setEnabled(true);
    Apply->setEnabled(true);
  }
}


bool KstChangeNptsDialog::updateChangeNptsDialog() {
  QStringList qsl;
  int inserted = 0;
  int i;
  
  for (i = 0; i < CurveList->count(); i++) {
    if (CurveList->item(i)->isSelected()) {
      qsl.append(CurveList->item(i)->text());
    }
  }
  CurveList->clear();

  KstRVectorList rvl;
  KstRVectorList::const_iterator it;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  //
  // insert vectors into ChangeNptsCurveList...
  //

  CurveList->blockSignals(true);

  for (it = rvl.begin(); it != rvl.end(); ++it) {
    KstRVectorPtr vector;
    QString tag;

    vector = *it;
    vector->readLock();
    
    tag = vector->tag().displayString();
    CurveList->insertItem(-1, tag);
    if (qsl.contains(tag)) {
      CurveList->item(inserted)->setSelected(true);
    }
    ++inserted;
    
    vector->unlock();
  }

  CurveList->blockSignals(false);

  return !qsl.isEmpty();
}


void KstChangeNptsDialog::showChangeNptsDialog() {
  bool selected = updateChangeNptsDialog();

  updateDefaults(0);
  _modifiedRange = false;
  if (!selected) {
    CurveList->selectAll();
  }
  show();
  raise();
}


void KstChangeNptsDialog::OKNptsChange() {
  applyNptsChange();
  reject();
}


void KstChangeNptsDialog::applyNptsChange() {
  KstRVectorList rvl;
  int i;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  for (i = 0; i < CurveList->count(); ++i) {
    if (CurveList->item(i)->isSelected()) {
      KstRVectorPtr vector;

      vector = *(rvl.findTag(CurveList->item(i)->text()));
      if (vector) {
        KstDataSourcePtr ds;
        KstFrameSize f0;
        KstFrameSize n;

        vector->readLock();

        ds = vector->dataSource();
        if (_kstDataRange->isStartRelativeTime() && ds) {
          ds->readLock();
          f0 = ds->sampleForTimeLarge(_kstDataRange->f0Value());
          ds->unlock();
        } else if (_kstDataRange->isStartAbsoluteTime() && ds) {
          ds->readLock();
          f0 = ds->sampleForTimeLarge(_kstDataRange->f0DateTimeValue());
          ds->unlock();
        } else {
          f0 = (KstFrameSize)_kstDataRange->f0Value();
        }

        if (_kstDataRange->isRangeRelativeTime() && ds) {
          double nValStored;
          double msCount;
          double fTime;

          ds->readLock();

          nValStored = _kstDataRange->nValue();
          if (_kstDataRange->CountFromEnd->isChecked()) {
            KstFrameSize frameCount = ds->frameCountLarge(vector->field());
            msCount = ds->relativeTimeForSampleLarge(frameCount - 1);
            n = frameCount - 1 - ds->sampleForTimeLarge(msCount - nValStored);
          } else {
            fTime = ds->relativeTimeForSampleLarge(f0);
            n = ds->sampleForTimeLarge(fTime + nValStored) - ds->sampleForTimeLarge(fTime);
          }

          ds->unlock();
        } else {
          n = (KstFrameSize)_kstDataRange->nValue();
        }
        vector->unlock();

        vector->writeLock();

        vector->changeFrames(
          (_kstDataRange->CountFromEnd->isChecked() ? -1 : f0),
          (_kstDataRange->ReadToEnd->isChecked() ? -1 : n),
          _kstDataRange->Skip->value(),
          _kstDataRange->DoSkip->isChecked(),
          _kstDataRange->DoFilter->isChecked());

        vector->unlock();
      }
    }
  }

  _modifiedRange = false;

  //
  // avoid re-entering the dialog
  //

  QTimer::singleShot(0, this, SLOT(emitDocChanged()));
}


void KstChangeNptsDialog::emitDocChanged() {
  emit docChanged();
}


void KstChangeNptsDialog::updateDefaults(int index) {
  KstRVectorList rvl;
  KstRVectorPtr vector;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  if (rvl.isEmpty() || index >= (int)rvl.count() || index < 0) {
    return;
  }

// xxx  vector = rvl[index];

  vector->readLock();

  disconnect(_kstDataRange, SIGNAL(changed()), this, SLOT(modifiedRange()));

  _kstDataRange->_startUnits->setCurrentIndex(0);
  _kstDataRange->_rangeUnits->setCurrentIndex(0);

  //
  // fill the vector range entries
  //

  _kstDataRange->CountFromEnd->setChecked(vector->countFromEOF());
  _kstDataRange->setF0Value(vector->reqStartFrame());

  //
  // fill number of frames entries
  //

  _kstDataRange->ReadToEnd->setChecked(vector->readToEOF());
  _kstDataRange->setNValue(vector->reqNumFrames());

  //
  // fill in frames to skip box
  //

  _kstDataRange->Skip->setValue(vector->skip());
  _kstDataRange->DoSkip->setChecked(vector->doSkip());
  _kstDataRange->DoFilter->setChecked(vector->doAve());
  _kstDataRange->updateEnables();

  connect(_kstDataRange, SIGNAL(changed()), this, SLOT(modifiedRange()));

  vector->unlock();
}


void KstChangeNptsDialog::changedSelection() {
  bool selected = false;
  bool enable;
  int index = -1;
  int i;

  for (i=0; i<CurveList->count(); ++i) {
    if (CurveList->item(i)->isSelected()) {
      selected = true;
      if (index == -1) {
        index = i;
      } else {
        index = -1;
        break;
      }
    }
  }

  if (!_modifiedRange) {
    if (index != -1) {
      updateDefaults(index);
    }
  }

  if (selected) {
    enable = true;
  } else {
    enable = false;
  }

  if (OK->isEnabled() != enable) {
    Apply->setEnabled(enable);
    OK->setEnabled(enable);
  }
}


void KstChangeNptsDialog::updateTimeCombo() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  bool supportsTime = true;
  uint cnt = CurveList->count();
  uint i;

  for (i = 0; i < cnt; ++i) {
    if (CurveList->item(i)->isSelected()) {
      KstRVectorPtr vector;

      vector = *(rvl.findTag(CurveList->item(i)->text()));
      if (vector) {
        KstDataSourcePtr ds;

        vector->readLock();
        ds = vector->dataSource();
        vector->unlock();
        if (ds) {
          ds->readLock();
          supportsTime = ds->supportsTimeConversions();
          ds->unlock();
          if (!supportsTime) {
            break;
          }
        }
      }
    }
  }

  _kstDataRange->setAllowTime(supportsTime);
}


void KstChangeNptsDialog::modifiedRange() {
  _modifiedRange = true;
}

#include "kstchangenptsdialog.moc"
