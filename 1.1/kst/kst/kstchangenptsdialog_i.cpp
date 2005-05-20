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

void KstChangeNptsDialogI::updateChangeNptsDialog() {
  CurveList->clear();

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  /* insert vectors into ChangeNptsCurveList */
  for (uint i = 0; i < rvl.count(); i++) {
    rvl[i]->readLock();
    CurveList->insertItem(rvl[i]->tagName(), -1);
    rvl[i]->readUnlock();
  }
}

void KstChangeNptsDialogI::showChangeNptsDialog() {
  updateChangeNptsDialog();
  updateDefaults(0);
  CurveList->selectAll(true);
  show();
  raise();
}

void KstChangeNptsDialogI::applyNptsChange() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (uint i_vector = 0; i_vector < CurveList->count(); i_vector++) {
    if (CurveList->isSelected(i_vector)) {
      KstRVectorPtr vector = rvl[i_vector];
      int f0, n;
      if (_kstDataRange->isTime() && vector->dataSource()) {
        vector->dataSource()->readLock();
        f0 = vector->dataSource()->sampleForTime(_kstDataRange->f0Value());
        n = vector->dataSource()->sampleForTime(_kstDataRange->nValue());
        vector->dataSource()->readUnlock();
      } else {
        f0 = _kstDataRange->f0Value();
        n = _kstDataRange->nValue();
      }
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

  /* fill the vector range entries */
  if (vector->countFromEOF()) {
    _kstDataRange->CountFromEnd->setChecked(true);
  } else {
    _kstDataRange->CountFromEnd->setChecked(false);
  }
  _kstDataRange->setF0Value(vector->reqStartFrame());

  /* fill number of frames entries */
  if (vector->readToEOF()) {
    _kstDataRange->ReadToEnd->setChecked(true);
  } else {
    _kstDataRange->ReadToEnd->setChecked(false);
  }
  _kstDataRange->setNValue(vector->reqNumFrames());

  /* fill in frames to skip box */
  _kstDataRange->Skip->setValue(vector->skip());
  _kstDataRange->DoSkip->setChecked(vector->doSkip());
  _kstDataRange->DoFilter->setChecked(vector->doAve());
  _kstDataRange->updateEnables();
  vector->writeUnlock();
}


#include "kstchangenptsdialog_i.moc"
// vim: ts=2 sw=2 et
