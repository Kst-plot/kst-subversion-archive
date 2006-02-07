/***************************************************************************
                       kstvectordialog_i.cpp  -  Part of KST
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

#include <assert.h>

#include <qcheckbox.h>
#include <qspinbox.h>

#include <kcombobox.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstaticdeleter.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "datarangewidget.h"
#include "kstdatacollection.h"
#include "kstvectordefaults.h"
#include "kstvectordialog_i.h"

#define DIALOGTYPE KstVectorDialogI
#define DTYPE "Vector"
#include "dataobjectdialog.h"

KstVectorDialogI *KstVectorDialogI::_inst = 0L;
static KStaticDeleter<KstVectorDialogI> _vInst;

KstVectorDialogI *KstVectorDialogI::globalInstance() {
  if (!_inst) {
    _inst = _vInst.setObject(new KstVectorDialogI);
  }
  return _inst;
}


KstVectorDialogI::KstVectorDialogI(QWidget* parent, const char* name,
                                   bool modal, WFlags fl)
: KstVectorDialog(parent, name, modal, fl) {
  FileName->completionObject()->setDir(QDir::currentDirPath());
  Init();

  FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
                    | KFile::LocalOnly);
  connect(FileName, SIGNAL(textChanged(const QString&)),
          this, SLOT(updateCompletion()));
  connect(_configure, SIGNAL(clicked()), this, SLOT(configureSource()));
  _configure->setEnabled(false);
  _fieldCompletion = Field->completionObject();
  Field->setAutoDeleteCompletionObject(true);
  setFixedHeight(height());
  _configWidget = 0L;
}


KstVectorDialogI::~KstVectorDialogI() {
  delete _configWidget;
  _configWidget = 0L;
  DP = 0L;
}


KstRVectorPtr KstVectorDialogI::_getPtr(const QString &tagin) {
  KstRVectorList v = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  return *v.findTag(tagin);
}


void KstVectorDialogI::updateCompletion() {
  QString current_text = Field->currentText();
  Field->clear();

  /* update filename list and ll axes combo boxes */
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findFileName(FileName->url());
  KST::dataSourceList.lock().readUnlock();

  delete _configWidget; // FIXME: very inefficient!!!!
  _configWidget = 0L;
  QStringList list;
  if (ds) {
    ds->readLock();
    list = ds->fieldList();
    Field->setEditable(!ds->fieldListIsComplete());
    _configWidget = ds->configWidget();
    ds->readUnlock();
  } else {
    QString type;
    bool complete = false;
    list = KstDataSource::fieldListForSource(FileName->url(), QString::null, &type, &complete);
    Field->setEditable(!complete);
    if (!list.isEmpty() && !type.isEmpty()) {
      _configWidget = KstDataSource::configWidgetForSource(FileName->url(), type);
    }
  }

  _configure->setEnabled(_configWidget);

  _fieldCompletion = Field->completionObject();

  Field->insertStringList(list);
  if (_fieldCompletion) {
    _fieldCompletion->clear();
    _fieldCompletion->insertItems(list);
  }
  if (!current_text.isEmpty() && (list.contains(current_text) || Field->editable())) {
    Field->setCurrentText(current_text);
  }
  _kstDataRange->setAllowTime(ds && ds->supportsTimeConversions());
}


void KstVectorDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  DP->readLock();

  _tagName->setText(DP->tagName());

  /* fill the fields */
  Field->clear();
  if (_fieldCompletion) {
    _fieldCompletion->clear();
  }
  {
    KstDataSourcePtr tf;
    KST::dataSourceList.lock().readLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(DP->filename());
    if (it != KST::dataSourceList.end()) {
      tf = *it;
      tf->readLock();
      Field->insertStringList(tf->fieldList());
      if (_fieldCompletion) {
        _fieldCompletion->insertItems(tf->fieldList());
      }
      tf->readUnlock();
    } else {
      QStringList list = KstDataSource::fieldListForSource(FileName->url());
      Field->insertStringList(list);
      if (_fieldCompletion) {
        _fieldCompletion->insertItems(list);
      }
    }
    KST::dataSourceList.lock().readUnlock();
  }
  Field->setCurrentText(DP->field());

  /* select the proper file */
  FileName->setURL(DP->filename());

  /* fill the vector range entries */
  if (DP->countFromEOF()) {
    _kstDataRange->CountFromEnd->setChecked(true);
  } else {
    _kstDataRange->CountFromEnd->setChecked(false);
  }
  _kstDataRange->setF0Value(DP->reqStartFrame());

  /* fill number of frames entries */
  if (DP->readToEOF()) {
    _kstDataRange->ReadToEnd->setChecked(true);
  } else {
    _kstDataRange->ReadToEnd->setChecked(false);
  }
  _kstDataRange->setNValue(DP->reqNumFrames());

  /* fill in frames to skip box */
  _kstDataRange->Skip->setValue(DP->skip());
  _kstDataRange->DoSkip->setChecked(DP->doSkip());
  _kstDataRange->DoFilter->setChecked(DP->doAve());
  _kstDataRange->updateEnables();

  DP->readUnlock();
}


void KstVectorDialogI::_fillFieldsForNew() {
  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  int n_v = vectorList.count();

  if (n_v < 1) {
    Field->insertItem("INDEX");
  } else {
    Field->setCurrentText(QString::null);
  }
  QString new_label;
  new_label.sprintf("V%d-", n_v+1);
  new_label += "<New_Vector>";
  _tagName->setText(new_label);

  /* set defaults with vectorDefaults */
  KST::vectorDefaults.sync();
  FileName->setURL(KST::vectorDefaults.dataSource());
  updateCompletion();
  _kstDataRange->update();
  Field->setFocus();
}


void KstVectorDialogI::update() {
  /* no longer does anything (?) */
}


bool KstVectorDialogI::new_I() {
  KstDataSourcePtr file;
  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  QString tag_name = _tagName->text();

  tag_name.replace("<New_Vector>", Field->currentText());

  KST::vectorList.lock().readLock();
  int i_c = KST::vectorList.count() + 1;
  KST::vectorList.lock().readUnlock();
  while (KST::vectorTagNameNotUnique(tag_name, false)) {
    tag_name.sprintf("V%d-", i_c);
    tag_name += Field->currentText();
    i_c++;
  }
  vectorList.clear();

  /* if there is not an active DataSource, create one */
  {
    KST::dataSourceList.lock().writeLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(FileName->url());

    if (it == KST::dataSourceList.end()) {
      file = KstDataSource::loadSource(FileName->url());
      if (!file || !file->isValid()) {
        KST::dataSourceList.lock().writeUnlock();
        KMessageBox::sorry(this, i18n("The file could not be loaded."));
        return false;
      }
      if (file->isEmpty()) {
        KST::dataSourceList.lock().writeUnlock();
        KMessageBox::sorry(this, i18n("The file does not contain data."));
        return false;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }
    KST::dataSourceList.lock().writeUnlock();
  }
  file->readLock();
  if (!file->isValidField(Field->currentText())) {
    file->readUnlock();
    KMessageBox::sorry(this, i18n("The requested field is not defined for the requested file."));
    return false;
  }
  file->readUnlock();

  int f0, n;
  if (_kstDataRange->isTime()) {
    file->readLock();
    f0 = file->sampleForTime(_kstDataRange->f0Value());
    n = file->sampleForTime(_kstDataRange->nValue());
    file->readUnlock();
  } else {
    f0 = _kstDataRange->f0Value();
    n = _kstDataRange->nValue();
  }
  /* create the vector */
  KstRVectorPtr vector = new KstRVector(
    file, Field->currentText(),
    tag_name,
    (_kstDataRange->CountFromEnd->isChecked() ? -1 : f0),
    (_kstDataRange->ReadToEnd->isChecked() ? -1 : n),
    _kstDataRange->Skip->value(),
    _kstDataRange->DoSkip->isChecked(),
    _kstDataRange->DoFilter->isChecked());

  KST::addVectorToList(KstVectorPtr(vector));

  emit vectorCreated(KstVectorPtr(vector));
  vector = 0L;
  emit modified();

  return true;
}


bool KstVectorDialogI::edit_I() {
  KstDataSourcePtr file;

  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  /* verify that the vector name is unique */
  DP->readLock();
  if (_tagName->text() != DP->tagName() && KST::vectorTagNameNotUnique(_tagName->text())) {
    DP->readUnlock();
    return false;
  }
  DP->readUnlock();

  /* if there is not an active KstFile, create one */
  {
    KST::dataSourceList.lock().writeLock();
    KstDataSourceList::Iterator it =
      KST::dataSourceList.findFileName(FileName->url());

    if (it == KST::dataSourceList.end()) {
      file = KstDataSource::loadSource(FileName->url());
      if (!file || !file->isValid()) {
        KST::dataSourceList.lock().writeUnlock();
        KMessageBox::sorry(this, i18n("The file could not be opened."));
        return false;
      }
      if (file->isEmpty()) {
        KST::dataSourceList.lock().writeUnlock();
        KMessageBox::sorry(this, i18n("The file does not contain data."));
        return false;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }
    KST::dataSourceList.lock().writeUnlock();
  }
  file->readLock();
  if (!file->isValidField(Field->currentText())) {
    file->readUnlock();
    KMessageBox::sorry(this, i18n("The requested field is not defined for "
                                "the requested file\n"));
    return false;
  }
  file->readUnlock();

  int f0, n;
  if (_kstDataRange->isTime()) {
    file->readLock();
    f0 = file->sampleForTime(_kstDataRange->f0Value());
    n = file->sampleForTime(_kstDataRange->nValue());
    file->readUnlock();
  } else {
    f0 = _kstDataRange->f0Value();
    n = _kstDataRange->nValue();
  }
  /* change the vector */
  DP->writeLock();
  DP->change(file, Field->currentText(),
             _tagName->text(),
             (_kstDataRange->CountFromEnd->isChecked() ?  -1 : f0),
             (_kstDataRange->ReadToEnd->isChecked() ?  -1 : n),
             _kstDataRange->Skip->value(),
             _kstDataRange->DoSkip->isChecked(),
             _kstDataRange->DoFilter->isChecked());
  DP->writeUnlock();

  vectorList.clear();
  emit modified();
  return true;
}


void KstVectorDialogI::configureSource() {
  assert(_configWidget);
  KDialogBase *dlg = new KDialogBase(this, "Data Config Dialog", true, i18n("Configure Data Source"));
  connect(dlg, SIGNAL(okClicked()), _configWidget, SLOT(save()));
  connect(dlg, SIGNAL(applyClicked()), _configWidget, SLOT(save()));
  _configWidget->reparent(dlg, QPoint(0, 0));
  dlg->setMainWidget(_configWidget);
  dlg->exec();
  _configWidget->reparent(0L, QPoint(0, 0));
  dlg->setMainWidget(0L);
  delete dlg;
}


#include "kstvectordialog_i.moc"
// vim: sw=2 ts=2 et
