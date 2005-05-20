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
#include <qradiobutton.h>
#include <qspinbox.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "datarangewidget.h"
#include "kst.h"
#include "kstdatacollection.h"
#include "kstvectordefaults.h"
#include "kstvectordialog_i.h"
#include "kstdefaultnames.h"

#define DIALOGTYPE KstVectorDialogI
#define DTYPE "Vector"
#include "dataobjectdialog.h"

QGuardedPtr<KstVectorDialogI> KstVectorDialogI::_inst = 0L;

const QString& KstVectorDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

KstVectorDialogI *KstVectorDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstVectorDialogI(KstApp::inst());
  }
  return _inst;
}


KstVectorDialogI::KstVectorDialogI(QWidget* parent, const char* name,
                                   bool modal, WFlags fl)
: KstVectorDialog(parent, name, modal, fl) {
  _inTest = false;
  FileName->completionObject()->setDir(QDir::currentDirPath());
  Init();

  FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
  connect(FileName, SIGNAL(textChanged(const QString&)),
          this, SLOT(updateCompletion()));
  connect(_configure, SIGNAL(clicked()), this, SLOT(configureSource()));
  connect(_readFromSource, SIGNAL(clicked()), this, SLOT(enableSource()));
  connect(_generateX, SIGNAL(clicked()), this, SLOT(enableGenerate()));
  connect(_connect, SIGNAL(clicked()), this, SLOT(testURL()));
  _configure->setEnabled(false);
  _fieldCompletion = Field->completionObject();
  Field->setAutoDeleteCompletionObject(true);
  setFixedHeight(height());
  _configWidget = 0L;
  Field->setEnabled(false);
  _OK->setEnabled(Field->isEnabled());
}


KstVectorDialogI::~KstVectorDialogI() {
  delete _configWidget;
  _configWidget = 0L;
  DP = 0L;
}


KstVectorPtr KstVectorDialogI::_getPtr(const QString &tagin) {
  return *KST::vectorList.findTag(tagin);
}


void KstVectorDialogI::testURL() {
  _inTest = true;
  updateCompletion();
  _inTest = false;
}


void KstVectorDialogI::enableSource() {
  _rvectorGroup->setEnabled(true);
  _svectorGroup->setEnabled(false);
  _OK->setEnabled(Field->isEnabled());
  _kstDataRange->setEnabled(true);
}


void KstVectorDialogI::enableGenerate() {
  _rvectorGroup->setEnabled(false);
  _svectorGroup->setEnabled(true);
  _OK->setEnabled(true);
  _kstDataRange->setEnabled(false);
}


void KstVectorDialogI::updateCompletion() {
  QString current_text = Field->currentText();
  Field->clear();

  /* update filename list and ll axes combo boxes */
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(FileName->url());
  KST::dataSourceList.lock().readUnlock();

  delete _configWidget;
  _configWidget = 0L;
  QStringList list;
  if (ds) {
    ds->readLock();
    list = ds->fieldList();
    Field->setEditable(!ds->fieldListIsComplete());
    _configWidget = ds->configWidget();
    ds->readUnlock();
    Field->setEnabled(true);
    _connect->hide();
    _kstDataRange->setAllowTime(ds->supportsTimeConversions());
  } else {
    QString type;
    bool complete = false;
    QString u = FileName->url();
    KURL url;
    if (QFile::exists(u) && QFileInfo(u).isRelative()) {
      url.setPath(u);
    } else {
      url = KURL::fromPathOrURL(u);
    }

    if (!_inTest && !url.isLocalFile() && url.protocol() != "file" && !url.protocol().isEmpty()) {
      _connect->show();
    } else if (url.isValid()) {
      list = KstDataSource::fieldListForSource(u, QString::null, &type, &complete);
      if (!_inTest || (_inTest && !list.isEmpty())) {
        _connect->hide();
      }
    }
    Field->setEditable(!complete);
    Field->setEnabled(!list.isEmpty());
    if (!list.isEmpty() && !type.isEmpty()) {
      _configWidget = KstDataSource::configWidgetForSource(u, type);
    }
    _kstDataRange->setAllowTime(KstDataSource::supportsTime(u, type));
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
  _OK->setEnabled(Field->isEnabled());
}


void KstVectorDialogI::_fillFieldsForRVEdit() {
  KstRVectorPtr rvp = kst_cast<KstRVector>( DP );
  rvp->readLock();

  _readFromSource->setChecked(true);
  _readFromSource->hide();
  _rvectorGroup->show();
  _kstDataRange->show();
  _kstDataRange->setEnabled(true);
  _svectorGroup->hide();
  _svectorGroup->setEnabled(false);
  _generateX->hide();

  _tagName->setText(rvp->tagName());

  /* fill the fields */
  Field->clear();
  if (_fieldCompletion) {
    _fieldCompletion->clear();
  }
  {
    KstDataSourcePtr tf;
    KST::dataSourceList.lock().readLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(rvp->filename());
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
  Field->setEnabled(Field->count() > 0);
  _OK->setEnabled(Field->isEnabled());
  Field->setCurrentText(rvp->field());

  /* select the proper file */
  FileName->setURL(rvp->filename());

  /* fill the vector range entries */
  _kstDataRange->CountFromEnd->setChecked(rvp->countFromEOF());
  _kstDataRange->setF0Value(rvp->reqStartFrame());

  /* fill number of frames entries */
  _kstDataRange->ReadToEnd->setChecked(rvp->readToEOF());
  _kstDataRange->setNValue(rvp->reqNumFrames());

  /* fill in frames to skip box */
  _kstDataRange->Skip->setValue(rvp->skip());
  _kstDataRange->DoSkip->setChecked(rvp->doSkip());
  _kstDataRange->DoFilter->setChecked(rvp->doAve());
  _kstDataRange->updateEnables();

  rvp->readUnlock();
}


void KstVectorDialogI::_fillFieldsForSVEdit() {
  KstSVectorPtr svp = kst_cast<KstSVector>( DP );
  if ( !svp ) { // shouldn't be needed
    return;
  }

  _generateX->setChecked(true);
  _readFromSource->hide();
  _rvectorGroup->hide();
  _rvectorGroup->setEnabled(false);
  _kstDataRange->hide();
  _kstDataRange->setEnabled(false);
  _svectorGroup->show();
  _svectorGroup->setEnabled(true);
  _generateX->hide();

  svp->readLock();
  _tagName->setText(svp->tagName());
  _N->setValue( svp->length() );
  _xMin->setText( QString::number( svp->min() ) );
  _xMax->setText( QString::number( svp->max() ) );
  svp->readUnlock();
  _OK->setEnabled(true);
}


void KstVectorDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  KstRVectorPtr rvp = kst_cast<KstRVector>( DP );

  if ( rvp ) {
    _fillFieldsForRVEdit();
  } else {
    _fillFieldsForSVEdit();
  }

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstVectorDialogI::_fillFieldsForNew() {
  _readFromSource->setChecked(true);
  _readFromSource->show();
  _rvectorGroup->show();
  _rvectorGroup->setEnabled(true);
  _kstDataRange->show();
  _kstDataRange->setEnabled(true);
  _svectorGroup->show();
  _svectorGroup->setEnabled(false);
  _generateX->show();

  _tagName->setText(defaultTag);

  /* set defaults with vectorDefaults */
  KST::vectorDefaults.sync();
  FileName->setURL(KST::vectorDefaults.dataSource());
  updateCompletion();
  _kstDataRange->update();
  Field->setFocus();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstVectorDialogI::update() {
  /* no longer does anything (?) */
}


bool KstVectorDialogI::new_I() {
  KstDataSourcePtr file;
  QString tag_name = _tagName->text();

  if ( _readFromSource->isChecked() ) {
    tag_name.replace(defaultTag, Field->currentText());
    tag_name = KST::suggestVectorName( tag_name);

    /* if there is not an active DataSource, create one */
    {
      KST::dataSourceList.lock().writeLock();
      KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(FileName->url());

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
  } else {
    double x0 = _xMin->text().toDouble();
    double x1 = _xMax->text().toDouble();
    int n = _N->value();
    QString tagname = _tagName->text();
    if ( tagname == defaultTag ) {
      tagname = KST::suggestVectorName(
        QString( "(%1..%2)" ).arg( x0 ).arg( x1 ) );
    }

    KstSVectorPtr svector = new KstSVector( x0, x1, n, tagname );

    KST::addVectorToList(KstVectorPtr(svector));

    emit vectorCreated(KstVectorPtr(svector));
    svector = 0L;
    emit modified();

  }

  return true;
}


bool KstVectorDialogI::edit_I() {
  KstRVectorPtr rvp = kst_cast<KstRVector>( DP );

  if ( rvp ) {
    KstDataSourcePtr file;

    KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);


    /* verify that the vector name is unique */
    rvp->readLock();
    if (_tagName->text() != rvp->tagName() && KST::vectorTagNameNotUnique(_tagName->text())) {
      rvp->readUnlock();
      return false;
    }
    rvp->readUnlock();

    /* if there is not an active KstFile, create one */
    {
      KST::dataSourceList.lock().writeLock();
      KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(FileName->url());

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
    rvp->writeLock();
    rvp->change(file, Field->currentText(),
                _tagName->text(),
                (_kstDataRange->CountFromEnd->isChecked() ?  -1 : f0),
                (_kstDataRange->ReadToEnd->isChecked() ?  -1 : n),
                _kstDataRange->Skip->value(),
                _kstDataRange->DoSkip->isChecked(),
                _kstDataRange->DoFilter->isChecked());
    rvp->writeUnlock();

    vectorList.clear();
  } else {
    KstSVectorPtr svp = kst_cast<KstSVector>( DP );
    if ( !svp ) {
      return true; // shouldn't be needed
    }
    double x0 = _xMin->text().toDouble();
    double x1 = _xMax->text().toDouble();
    int n = _N->value();
    QString tagname = _tagName->text();
    svp->writeLock();
    svp->changeRange( x0, x1, n );
    svp->setTagName( _tagName->text() ); // FIXME: doesn't verify uniqueness
    svp->writeUnlock();
  }
  emit modified();
  return true;
}


void KstVectorDialogI::markSourceAndSave() {
  assert(_configWidget);
  KstDataSourcePtr src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
  if (src) {
    src->disableReuse();
  }
  static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
}


void KstVectorDialogI::configureSource() {
  bool isNew = false;
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(FileName->url());
  KST::dataSourceList.lock().readUnlock();
  if (!ds) {
    isNew = true;
    ds = KstDataSource::loadSource(FileName->url());
    if (!ds || !ds->isValid()) {
      _configure->setEnabled(false);
      return;
    }
  }

  assert(_configWidget);
  KDialogBase *dlg = new KDialogBase(this, "Data Config Dialog", true, i18n("Configure Data Source"));
  if (isNew) {
    connect(dlg, SIGNAL(okClicked()), _configWidget, SLOT(save()));
    connect(dlg, SIGNAL(applyClicked()), _configWidget, SLOT(save()));
  } else {
    connect(dlg, SIGNAL(okClicked()), this, SLOT(markSourceAndSave()));
    connect(dlg, SIGNAL(applyClicked()), this, SLOT(markSourceAndSave()));
  }
  _configWidget->reparent(dlg, QPoint(0, 0));
  dlg->setMainWidget(_configWidget);
  _configWidget->setInstance(ds);
  _configWidget->load();
  dlg->exec();
  _configWidget->reparent(0L, QPoint(0, 0));
  dlg->setMainWidget(0L);
  delete dlg;
  updateCompletion(); // could be smarter by only running if Ok/Apply clicked
}


#include "kstvectordialog_i.moc"
// vim: sw=2 ts=2 et
