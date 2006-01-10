/***************************************************************************
                    kstchangenptsdialog_i.cpp  -  Part of KST
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
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <kcombobox.h>
#include <knuminput.h>

#include "datarangewidget.h"
#include "kstchangenptsdialog_i.h"
#include "kstdatacollection.h"
#include "kstrvector.h"

KstChangeNptsDialogI::KstChangeNptsDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl)
: KstChangeNptsDialog(parent, name, modal, fl) {
    connect(Clear,     SIGNAL(clicked()),
            CurveList, SLOT(clearSelection()));
    connect(SelectAll, SIGNAL(clicked()),
            this,      SLOT(selectAll()));
    connect(Apply,     SIGNAL(clicked()),
            this,      SLOT(applyNptsChange()));
    connect(CurveList, SIGNAL(selected ( int )),
            this,      SLOT(updateDefaults( int )));
}


KstChangeNptsDialogI::~KstChangeNptsDialogI() {
}


void KstChangeNptsDialogI::selectAll() {
  CurveList->selectAll(true);
}


bool KstChangeNptsDialogI::updateChangeNptsDialog() {
  QStringList qsl;
  
  for (uint i_vector = 0; i_vector < CurveList->count(); i_vector++) {
    if (CurveList->isSelected(i_vector)) {
      qsl.append(CurveList->text(i_vector));
    }
  }
  CurveList->clear();
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  /* insert vectors into ChangeNptsCurveList */
  for (uint i = 0; i < rvl.count(); i++) {
    rvl[i]->readLock();
    CurveList->insertItem(rvl[i]->tagName(), -1);
    if (qsl.contains(rvl[i]->tagName())) {
      CurveList->setSelected(i, true);
    }
    rvl[i]->readUnlock();
  }
  return !qsl.isEmpty();
}


void KstChangeNptsDialogI::showChangeNptsDialog() {
  bool some_slected = updateChangeNptsDialog();
  updateDefaults(0);
  if (!some_slected) {
    CurveList->selectAll(true);
  }
  show();
  raise();
}


void KstChangeNptsDialogI::applyNptsChange() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (uint i_vector = 0; i_vector < CurveList->count(); ++i_vector) {
    if (CurveList->isSelected(i_vector)) {
      KstRVectorPtr vector = rvl[i_vector]; // FIXME: broken
      int f0, n;
      vector->readLock();
      if (_kstDataRange->isStartRelativeTime() && vector->dataSource()) {
        vector->dataSource()->readLock();
        f0 = vector->dataSource()->sampleForTime(_kstDataRange->f0Value());
        vector->dataSource()->readUnlock();
      } else if (_kstDataRange->isStartAbsoluteTime() && vector->dataSource()) {
        vector->dataSource()->readLock();
        f0 = vector->dataSource()->sampleForTime(_kstDataRange->f0DateTimeValue());
        vector->dataSource()->readUnlock();
      } else {
        f0 = int(_kstDataRange->f0Value());
      }
      
      if (_kstDataRange->isRangeRelativeTime() && vector->dataSource()) {
        KstDataSourcePtr ds = vector->dataSource();
        ds->readLock();
        double nValStored = _kstDataRange->nValue();
        if (_kstDataRange->CountFromEnd->isChecked()) {
          int frameCount = ds->frameCount(vector->field());
          double msCount = ds->relativeTimeForSample(frameCount - 1);
          n = frameCount - 1 - ds->sampleForTime(msCount - nValStored);
        } else {
          double fTime = ds->relativeTimeForSample(f0);
          n = ds->sampleForTime(fTime + nValStored) - ds->sampleForTime(fTime);
        }
        ds->readUnlock();
      } else {
        n = int(_kstDataRange->nValue());
      }
      vector->readUnlock();
      vector->writeLock();
      vector->changeFrames(
        (_kstDataRange->CountFromEnd->isChecked() ? -1 : f0),
        (_kstDataRange->ReadToEnd->isChecked() ? -1 : n),
        _kstDataRange->Skip->value(),
        _kstDataRange->DoSkip->isChecked(),
        _kstDataRange->DoFilter->isChecked());
      vector->writeUnlock();
    }
  }

  emit docChanged();
}


void KstChangeNptsDialogI::updateDefaults(int index) {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  if (rvl.isEmpty() || index >= (int)rvl.count()) {
    return;
  }

  KstRVectorPtr vector = rvl[index];
  vector->writeLock();

  _kstDataRange->_startUnits->setCurrentItem(0);
  _kstDataRange->_rangeUnits->setCurrentItem(0);

  /* fill the vector range entries */
  _kstDataRange->CountFromEnd->setChecked(vector->countFromEOF());
  _kstDataRange->setF0Value(vector->reqStartFrame());

  /* fill number of frames entries */
  _kstDataRange->ReadToEnd->setChecked(vector->readToEOF());
  _kstDataRange->setNValue(vector->reqNumFrames());

  /* fill in frames to skip box */
  _kstDataRange->Skip->setValue(vector->skip());
  _kstDataRange->DoSkip->setChecked(vector->doSkip());
  _kstDataRange->DoFilter->setChecked(vector->doAve());
  _kstDataRange->updateEnables();
  vector->writeUnlock();
}


void KstChangeNptsDialogI::updateTimeCombo() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  uint cnt = CurveList->count();
  bool supportsTime = true;
  for (uint i = 0; i < cnt; ++i) {
    if (CurveList->isSelected(i)) {
      KstRVectorPtr vector = rvl[i]; // FIXME: broken
      vector->readLock();
      KstDataSourcePtr ds = vector->dataSource();
      vector->readUnlock();
      ds->readLock();
      supportsTime = ds->supportsTimeConversions();
      ds->readUnlock();
      if (!supportsTime) {
        break;
      }
    }
  }
  _kstDataRange->setAllowTime(supportsTime);
}


#include "kstchangenptsdialog_i.moc"
// vim: ts=2 sw=2 et
