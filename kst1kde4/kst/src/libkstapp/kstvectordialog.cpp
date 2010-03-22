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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "datarangewidget.h"
#include "defaultprimitivenames.h"
#include "editmultiplewidget.h"
#include "kst.h"
#include "kstdatacollection.h"
#include "kstfieldselect_i.h"
#include "kstrvector.h"
#include "kstsvector.h"
#include "kstvectordefaults.h"
#include "kstvectordialog_i.h"
#include "kstdefaultnames.h"
#include "kstcombobox.h"
#include "vectordialogwidget.h"


QPointer<KstVectorDialog> KstVectorDialog::_inst = 0L;

const QString& KstVectorDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

KstVectorDialogI *KstVectorDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstVectorDialog(KstApp::inst());
  }
  return _inst;
}


KstVectorDialog::KstVectorDialog(QWidget* parent, const char* name,
                                   bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new VectorDialogWidget(_contents);
  _w->setupUi(this);

  setMultiple(true);
  _inTest = false;
  _w->FileName->completionObject()->setDir(QDir::currentDirPath());

  _w->FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
  connect(_w->FileName, SIGNAL(openFileDialog(KURLRequester *)), this, SLOT(selectFolder()));
  connect(_w->FileName, SIGNAL(textChanged(const QString&)), this, SLOT(updateCompletion()));
  connect(_w->_configure, SIGNAL(clicked()), this, SLOT(configureSource()));
  connect(_w->_readFromSource, SIGNAL(clicked()), this, SLOT(enableSource()));
  connect(_w->_generateX, SIGNAL(clicked()), this, SLOT(enableGenerate()));
  connect(_w->_connect, SIGNAL(clicked()), this, SLOT(testURL()));
  connect(_w->_pushButtonHierarchy, SIGNAL(clicked()), this, SLOT(showFields()));

  // connections for multiple edit mode
  connect(_w->_kstDataRange->CountFromEnd, SIGNAL(clicked()), this, SLOT(setCountFromEndDirty()));
  connect(_w->_kstDataRange->ReadToEnd, SIGNAL(clicked()), this, SLOT(setReadToEndDirty()));
  connect(_w->_kstDataRange->DoFilter, SIGNAL(clicked()), this, SLOT(setDoFilterDirty()));
  connect(_w->_kstDataRange->DoSkip, SIGNAL(clicked()), this, SLOT(setDoSkipDirty()));

  // for apply button
  connect(_w->_readFromSource, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_generateX, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_configure, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->FileName, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->Field, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->Field, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_N, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_N->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMin, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMax, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->F0, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->_startUnits, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->N, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->_rangeUnits, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->CountFromEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->ReadToEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->DoSkip, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->Skip, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstDataRange->DoFilter, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));

  _w->_configure->setEnabled(false);
  _w->_connect->setHidden(true);
  _fieldCompletion = _w->Field->completionObject();
  _w->Field->setAutoDeleteCompletionObject(true);
  setFixedHeight(height());
  _configWidget = 0L;
  _hierarchy = false;
  _w->Field->setEnabled(false);
  _ok->setEnabled(_w->Field->isEnabled());
  _legendLabel->hide();
  _legendText->hide();
}


KstVectorDialog::~KstVectorDialogI() {
  delete _configWidget;
  _configWidget = 0L;
}


void KstVectorDialog::selectingFolder()
{
  QString strFolder = _w->FileName->url();
  KFileDialog *fileDlg = _w->FileName->fileDialog();
  QFileInfo fileInfo(strFolder);

  if (fileDlg) {
    if (fileInfo.isDir()) {
      QDir dir(strFolder);

      if (dir.cdUp()) {
        fileDlg->setURL(KURL(dir.absPath()));
      }
    }
  }
}


void KstVectorDialog::selectFolder()
{
  QTimer::singleShot(0, this, SLOT(selectingFolder()));
}


void KstVectorDialog::testURL() {
  _inTest = true;
  updateCompletion();
  _inTest = false;
}


void KstVectorDialog::showFields() {
  KstFieldSelectI *dlg = new KstFieldSelectI(this, "Field Select", true);
  if (dlg) {
    dlg->setURL(_w->FileName->url());
    dlg->exec();

    if (dlg->result() == QDialog::Accepted) {
      if (!dlg->selection().isEmpty()) {
        _w->Field->setCurrentText(dlg->selection());
      }
    }

    delete dlg;
  }
}


void KstVectorDialog::enableSource() {
  _w->_rvectorGroup->setEnabled(true);
  _w->_svectorGroup->setEnabled(false);
  _ok->setEnabled(_w->Field->isEnabled());
  _w->_kstDataRange->setEnabled(true);
}


void KstVectorDialog::enableGenerate() {
  _w->_rvectorGroup->setEnabled(false);
  _w->_svectorGroup->setEnabled(true);
  _ok->setEnabled(true);
  _w->_kstDataRange->setEnabled(false);
}


void KstVectorDialog::updateCompletion() {
  QStringList list;
  QString currentText = _w->Field->currentText();

  _w->Field->clear();

  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_w->FileName->url());
  KST::dataSourceList.lock().unlock();

  delete _configWidget;
  _configWidget = 0L;
  _hierarchy = false;

  if (ds) {
    ds->readLock();
    list = ds->fieldList();
    _w->Field->setEditable(!ds->fieldListIsComplete());
    _configWidget = ds->configWidget();
    _hierarchy = ds->supportsHierarchy();
    ds->unlock();
    _w->Field->setEnabled(true);
    _w->_connect->hide();
    _w->_kstDataRange->setAllowTime(ds->supportsTimeConversions());
  } else {
    QString type;
    bool complete = false;
    QString u = _w->FileName->url();
    KURL url;
    if (QFile::exists(u) && QFileInfo(u).isRelative()) {
      url.setPath(u);
    } else {
      url = KURL::fromPathOrURL(u);
    }

    if (!_inTest && !url.isLocalFile() && url.protocol() != "file" && !url.protocol().isEmpty()) {
      _w->_connect->show();
    } else if (url.isValid()) {
      list = KstDataSource::fieldListForSource(u, QString::null, &type, &complete);
      if (!_inTest || (_inTest && !list.isEmpty())) {
        _w->_connect->hide();
      }
    }
    _w->Field->setEditable(!complete);
    _w->Field->setEnabled(!list.isEmpty());
    if (!list.isEmpty() && !type.isEmpty()) {
      _hierarchy = KstDataSource::supportsHierarchy(u, type);
      _configWidget = KstDataSource::configWidgetForSource(u, type);
    }
    _w->_kstDataRange->setAllowTime(KstDataSource::supportsTime(u, type));
  }

  _w->_configure->setEnabled(_configWidget);
  _w->_pushButtonHierarchy->setEnabled(_hierarchy);
  _fieldCompletion = _w->Field->completionObject();

  _w->Field->insertStringList(list);
  if (_fieldCompletion) {
    _fieldCompletion->clear();
    _fieldCompletion->insertItems(list);
  }
  if (!currentText.isEmpty() && (list.contains(currentText) || _w->Field->editable())) {
    _w->Field->setCurrentText(currentText);
  }
  _ok->setEnabled(_w->Field->isEnabled() || _editMultipleMode);
}


void KstVectorDialog::fillFieldsForRVEdit() {
  KstRVectorPtr rvp = kst_cast<KstRVector>(_dp);
  rvp->readLock();

  _w->_readFromSource->setChecked(true);
  _w->_rvectorGroup->show();
  _w->_kstDataRange->show();
  _w->_kstDataRange->setEnabled(true);
  _w->_svectorGroup->hide();
  _w->_svectorGroup->setEnabled(false);
  _w->sourceGroup->hide();

  _tagName->setText(rvp->tagName());

  _w->Field->clear();
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
      _w->Field->insertStringList(tf->fieldList());
      if (_fieldCompletion) {
        _fieldCompletion->insertItems(tf->fieldList());
      }
      tf->unlock();
    } else {
      QStringList list = KstDataSource::fieldListForSource(_w->FileName->url());
      _w->Field->insertStringList(list);
      if (_fieldCompletion) {
        _fieldCompletion->insertItems(list);
      }
    }
    KST::dataSourceList.lock().unlock();
  }

  _w->Field->setEnabled(_w->Field->count() > 0);
  _ok->setEnabled(_w->Field->isEnabled());
  _w->Field->setCurrentText(rvp->field());

  // select the proper file
  _w->FileName->setURL(rvp->filename());

  // fill the vector range entries
  _w->_kstDataRange->CountFromEnd->setChecked(rvp->countFromEOF());
  _w->_kstDataRange->setF0Value(rvp->reqStartFrame());

  // fill number of frames entries
  _w->_kstDataRange->ReadToEnd->setChecked(rvp->readToEOF());
  _w->_kstDataRange->setNValue(rvp->reqNumFrames());

  // fill in frames to skip box
  _w->_kstDataRange->Skip->setValue(rvp->skip());
  _w->_kstDataRange->DoSkip->setChecked(rvp->doSkip());
  _w->_kstDataRange->DoFilter->setChecked(rvp->doAve());
  _w->_kstDataRange->updateEnables();

  rvp->unlock();
}


void KstVectorDialog::fillFieldsForSVEdit() {
  KstSVectorPtr svp = kst_cast<KstSVector>(_dp);
  if (!svp) { // shouldn't be needed
    return;
  }

  _w->_generateX->setChecked(true);
  _w->_rvectorGroup->hide();
  _w->_rvectorGroup->setEnabled(false);
  _w->_kstDataRange->hide();
  _w->_kstDataRange->setEnabled(false);
  _w->_svectorGroup->show();
  _w->_svectorGroup->setEnabled(true);
  _w->sourceGroup->hide();

  svp->readLock();
  _tagName->setText(svp->tagName());
  _w->_N->setValue( svp->length() );
  _w->_xMin->setText(QString::number(svp->min()));
  _w->_xMax->setText(QString::number(svp->max()));
  svp->unlock();
  _ok->setEnabled(true);
}


void KstVectorDialog::fillFieldsForEdit() {
  KstRVectorPtr rvp = kst_cast<KstRVector>(_dp);

  if (rvp) {
    fillFieldsForRVEdit();
  } else {
    fillFieldsForSVEdit();
  }
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstVectorDialog::fillFieldsForNew() {
  _w->_readFromSource->setChecked(true);
  _w->_rvectorGroup->show();
  _w->_rvectorGroup->setEnabled(true);
  _w->_kstDataRange->show();
  _w->_kstDataRange->setEnabled(true);
  _w->_svectorGroup->show();
  _w->_svectorGroup->setEnabled(false);
  _w->sourceGroup->show();

  _tagName->setText(defaultTag);

  // set defaults with vectorDefaults
  KST::vectorDefaults.sync();
  _w->FileName->setURL(KST::vectorDefaults.dataSource());
  updateCompletion();
  _w->_kstDataRange->update();
  _w->Field->setFocus();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


bool KstVectorDialog::newObject() {
  KstDataSourcePtr file;
  QString tagName = _tagName->text();

  if (_w->_readFromSource->isChecked()) {
    tagName.replace(defaultTag, _w->Field->currentText());
    tagName = KST::suggestVectorName(tagName);
    tagName.remove('[');
    tagName.remove(']');

    // if there is not an active DataSource, create one
    {
      KST::dataSourceList.lock().writeLock();
      KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(_w->FileName->url());

      if (it == KST::dataSourceList.end()) {
        file = KstDataSource::loadSource(_w->FileName->url());
        if (!file || !file->isValid()) {
          KST::dataSourceList.lock().unlock();
          KMessageBox::sorry(this, i18n("The file could not be loaded."));
          return false;
        }
        if (file->isEmpty()) {
          KST::dataSourceList.lock().unlock();
          KMessageBox::sorry(this, i18n("The file does not contain data."));
          return false;
        }
        KST::dataSourceList.append(file);
      } else {
        file = *it;
      }
      KST::dataSourceList.lock().unlock();
    }
    file->readLock();
    if (!file->isValidField(_w->Field->currentText())) {
      file->unlock();
      KMessageBox::sorry(this, i18n("The requested field is not defined for the requested file."));
      return false;
    }

    KstFrameSize f0;
    KstFrameSize n;

    if (_w->_kstDataRange->isStartRelativeTime()) {
      f0 = file->sampleForTimeLarge(_w->_kstDataRange->f0Value());
    } else if (_w->_kstDataRange->isStartAbsoluteTime()) {
      bool ok = false;
      f0 = file->sampleForTimeLarge(_w->_kstDataRange->f0DateTimeValue(), &ok);
      if (!ok) {
        file->unlock();
        KMessageBox::sorry(this, i18n("The requested field or file could not use the specified date."));
        return false;
      }
    } else {
      f0 = (KstFrameSize)_w->_kstDataRange->f0Value();
    }

    if (_w->_kstDataRange->isRangeRelativeTime()) {
      double nValStored = _w->_kstDataRange->nValue();
      if (_w->_kstDataRange->CountFromEnd->isChecked()) {
        KstFrameSize frameCount = file->frameCountLarge(_w->Field->currentText());
        double msCount = file->relativeTimeForSampleLarge(frameCount - 1);
        n = frameCount - 1 - file->sampleForTimeLarge(msCount - nValStored);
      } else {
        double fTime = file->relativeTimeForSampleLarge(f0);
        n = file->sampleForTimeLarge(fTime + nValStored) - file->sampleForTimeLarge(fTime);
      }
    } else {
      n = (KstFrameSize)_w->_kstDataRange->nValue();
    }
    file->unlock();

    //
    // create the vector
    //

    KstRVectorPtr vector = new KstRVector(
        file, _w->Field->currentText(),
        KstObjectTag(tagName, file->tag(), false),
        _w->_kstDataRange->CountFromEnd->isChecked() ? -1 : f0,
        _w->_kstDataRange->ReadToEnd->isChecked() ? -1 : n,
        _w->_kstDataRange->Skip->value(),
        _w->_kstDataRange->DoSkip->isChecked(),
        _w->_kstDataRange->DoFilter->isChecked());

    emit vectorCreated(KstVectorPtr(vector));
    vector = 0L;
    emit modified();
  } else {
    double x0 = _w->_xMin->text().toDouble();
    double x1 = _w->_xMax->text().toDouble();
    int n = _w->_N->value();
    QString tagName = _tagName->text();

    if (tagName == defaultTag) {
      tagName = KST::suggestVectorName(QString("(%1..%2)").arg(x0).arg(x1));
    } else {
      tagName.remove('[');
      tagName.remove(']');
    }

    KstSVectorPtr svector = new KstSVector(x0, x1, n, KstObjectTag(tagName, KstObjectTag::globalTagContext));
    emit vectorCreated(KstVectorPtr(svector));
    svector = 0L;
    emit modified();
  }

  return true;
}


bool KstVectorDialog::editSingleObject(KstVectorPtr vcPtr) {
  if (kst_cast<KstRVector>(vcPtr)) {
    return editSingleObjectRV(vcPtr);
  } else {
    return editSingleObjectSV(vcPtr);
  }
}


bool KstVectorDialog::editSingleObjectSV(KstVectorPtr vcPtr) {
  KstSVectorPtr svp = kst_cast<KstSVector>(vcPtr);

  svp->readLock();

  int p_N;
  double p_xMin, p_xMax;
  p_N = _NDirty ? _w->_N->value() : svp->length();
  p_xMin = _xMinDirty ? _w->_xMin->text().toDouble() : svp->min();
  p_xMax = _xMaxDirty ? _w->_xMax->text().toDouble() : svp->max();

  svp->unlock();
  svp->writeLock();
  svp->changeRange(p_xMin, p_xMax, p_N);
  svp->unlock();

  return true;
}


bool KstVectorDialog::editSingleObjectRV(KstVectorPtr vcPtr) {
  KstRVectorPtr rvp = kst_cast<KstRVector>(vcPtr);
  KstDataSourcePtr file;

  if (_fileNameDirty) {
    // if there is not an active KstFile, create one
    KST::dataSourceList.lock().writeLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(_w->FileName->url());

    if (it == KST::dataSourceList.end()) {
      file = KstDataSource::loadSource(_w->FileName->url());
      if (!file || !file->isValid()) {
        KST::dataSourceList.lock().unlock();
        KMessageBox::sorry(this, i18n("The file could not be opened."));
        return false;
      }
      if (file->isEmpty()) {
        KST::dataSourceList.lock().unlock();
        KMessageBox::sorry(this, i18n("The file does not contain data."));
        return false;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }
    KST::dataSourceList.lock().unlock();
  } else {
    KstRVectorList vcList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
    for (uint i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->isSelected(i)) {
        // get the pointer to the object
        KstRVectorList::Iterator vcIter = vcList.findTag(_editMultipleWidget->_objectList->text(i));
        if (vcIter == vcList.end()) {
          return false;
        }

        KstRVectorPtr rvp = *vcIter;
        rvp->readLock();
        file = rvp->dataSource();
        rvp->unlock();
      }
    }
  }

  file->writeLock();
  if (rvp) {
    QString pField;
    if (_fileNameDirty) {
      pField = _w->Field->currentText();
      if (!file->isValidField(pField)) {
        KMessageBox::sorry(this, i18n("The requested field is not defined for the requested file."));
        file->unlock();
        return false;
      }
    } else {
      pField = rvp->field();
    }

    KstFrameSize f0 = 0;
    KstFrameSize n = 0;

    if (_f0Dirty) {
      if (_w->_kstDataRange->isStartRelativeTime()) {
        f0 = file->sampleForTimeLarge(_w->_kstDataRange->f0Value());
      } else if (_w->_kstDataRange->isStartAbsoluteTime()) {
        bool ok = false;
        f0 = file->sampleForTime(_w->_kstDataRange->f0DateTimeValue(), &ok);
        if (!ok) {
          file->unlock();
          KMessageBox::sorry(this, i18n("The requested field or file could not use the specified date."));
          return false;
        }
      } else {
        f0 = (KstFrameSize)_w->_kstDataRange->f0Value();
      }
    }

    if (_nDirty) {
      if (_w->_kstDataRange->isRangeRelativeTime()) {
        double nValStored = _w->_kstDataRange->nValue();
        if (_w->_kstDataRange->CountFromEnd->isChecked()) {
          KstFrameSize frameCount = file->frameCountLarge(_w->Field->currentText());
          double msCount = file->relativeTimeForSampleLarge(frameCount - 1);
          n = frameCount - 1 - file->sampleForTimeLarge(msCount - nValStored);
        } else {
          double fTime = file->relativeTimeForSampleLarge(f0);
          n = file->sampleForTimeLarge(fTime + nValStored) - file->sampleForTimeLarge(fTime);
        }
      } else {
        n = (KstFrameSize)_w->_kstDataRange->nValue();
      }
    }

    // use existing requested start and number of frames if not dirty
    rvp->readLock();
    if (!_f0Dirty) {
      f0 = rvp->reqStartFrame();
    }
    if (!_nDirty) {
      n = rvp->reqNumFrames();
    }

    //
    // other parameters for multiple edit
    //

    bool pCountFromEnd;
    bool pReadToEnd;
    bool pDoSkip;
    bool pDoFilter;
    int pSkip;

    if (_countFromEndDirty) {
      pCountFromEnd = _w->_kstDataRange->CountFromEnd->isChecked();
    } else {
      pCountFromEnd = rvp->countFromEOF();
    }

    if (_readToEndDirty) {
      pReadToEnd = _w->_kstDataRange->ReadToEnd->isChecked();
    } else {
      pReadToEnd = rvp->readToEOF();
    }

    if (_skipDirty) {
      pSkip = _w->_kstDataRange->Skip->value();
    } else {
      pSkip = rvp->skip();
    }

    if (_doSkipDirty) {
      pDoSkip = _w->_kstDataRange->DoSkip->isChecked();
    } else {
      pDoSkip = rvp->doSkip();
    }

    if (_doFilterDirty) {
      pDoFilter = _w->_kstDataRange->DoFilter->isChecked();
    } else {
      pDoFilter = rvp->doAve();
    }

    rvp->unlock();

    // change the vector
    rvp->writeLock();
    rvp->change(file, pField, rvp->tag(), pCountFromEnd ?  -1 : f0, pReadToEnd ?  -1 : n, pSkip, pDoSkip, pDoFilter);
    rvp->unlock();
  } else {
    KstSVectorPtr svp = kst_cast<KstSVector>(_dp);
    if (!svp) {
      file->unlock();
      return true; // shouldn't be needed
    }

    double x0 = _w->_xMin->text().toDouble();
    double x1 = _w->_xMax->text().toDouble();
    int n = _w->_N->value();

    svp->writeLock();
    svp->changeRange(x0, x1, n);
    svp->setTagName(KstObjectTag(_tagName->text(), svp->tag().context())); // FIXME: doesn't verify uniqueness, doesn't allow changing tag context
    svp->unlock();
  }
  file->unlock();

  return true;
}


bool KstVectorDialog::editObject() {
  // if editing multiple objects, edit each one
  if (_editMultipleMode) {
    // get dirties first
    _fileNameDirty = !_w->FileName->url().isEmpty();
    _skipDirty = _w->_kstDataRange->Skip->text() != " ";
    _f0Dirty = !_w->_kstDataRange->F0->text().isEmpty();
    _nDirty = !_w->_kstDataRange->N->text().isEmpty();
    _NDirty = _w->_N->text() != " ";
    _xMinDirty = !_w->_xMin->text().isEmpty();
    _xMaxDirty = !_w->_xMax->text().isEmpty();

    // now edit the vectors
    bool didEdit = false;
    for (uint i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->isSelected(i)) {
        // get the pointer to the object
        KST::vectorList.lock().readLock();
        KstVectorList::Iterator vcIter = KST::vectorList.findTag(_editMultipleWidget->_objectList->text(i));
        if (vcIter == KST::vectorList.end()) {
          KST::vectorList.lock().unlock();
          return false;
        }
        KstVectorPtr vcPtr = *vcIter;
        KST::vectorList.lock().unlock();

        if (!editSingleObject(vcPtr)) {
          return false;
        }
        didEdit = true;
      }
    }
    if (!didEdit) {
      KMessageBox::sorry(this, i18n("Select one or more objects to edit."));
      return false;
    }
  } else {
    // verify that the curve name is unique
    QString tag_name = _tagName->text();
    _dp->writeLock();
    if (tag_name != _dp->tagName() && KstData::self()->vectorTagNameNotUnique(tag_name)) {
      _dp->unlock();
      _tagName->setFocus();
      return false;
    }

    _dp->setTagName(KstObjectTag(tag_name, _dp->tag().context())); // FIXME: doesn't allow changing tag context
    _dp->unlock();

    // then edit the object
    _fileNameDirty = true;
    _f0Dirty = true;
    _nDirty = true;
    _NDirty = true;
    _countFromEndDirty = true;
    _readToEndDirty = true;
    _doFilterDirty = true;
    _doSkipDirty = true;
    _skipDirty = true;
    _xMinDirty = true;
    _xMaxDirty = true;
    KstVectorPtr vp = kst_cast<KstVector>(_dp);
    if (!vp || !editSingleObject(vp)) {
      return false;
    }
  }
  emit modified();
  return true;
}


void KstVectorDialog::markSourceAndSave() {
  assert(_configWidget);
  KstDataSourcePtr src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
  if (src) {
    src->disableReuse();
  }
  static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
}


void KstVectorDialog::configureSource() {
  bool isNew = false;
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_w->FileName->url());
  KST::dataSourceList.lock().unlock();
  if (!ds) {
    isNew = true;
    ds = KstDataSource::loadSource(_w->FileName->url());
    if (!ds || !ds->isValid()) {
      _w->_configure->setEnabled(false);
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


void KstVectorDialog::populateEditMultipleRV() {
  KstRVectorList vclist = kstObjectSubList<KstVector, KstRVector>(KST::vectorList);
  _editMultipleWidget->_objectList->insertStringList(vclist.tagNames());

  // also intermediate state for multiple edit
  _w->FileName->clear();
  _w->_kstDataRange->F0->setText("");
  _w->_kstDataRange->N->setText("");
  _w->_kstDataRange->Skip->setMinValue(_w->_kstDataRange->Skip->minValue() - 1);
  _w->_kstDataRange->Skip->setSpecialValueText(" ");
  _w->_kstDataRange->Skip->setValue(_w->_kstDataRange->Skip->minValue());
  _w->_kstDataRange->CountFromEnd->setTristate(true);
  _w->_kstDataRange->CountFromEnd->setNoChange();
  _w->_kstDataRange->ReadToEnd->setTristate(true);
  _w->_kstDataRange->ReadToEnd->setNoChange();
  _w->_kstDataRange->DoFilter->setTristate(true);
  _w->_kstDataRange->DoFilter->setNoChange();
  _w->_kstDataRange->DoSkip->setTristate(true);
  _w->_kstDataRange->DoSkip->setNoChange();

  _w->_kstDataRange->Skip->setEnabled(true);
  _w->_kstDataRange->N->setEnabled(true);
  _w->_kstDataRange->F0->setEnabled(true);

  // and clean all the fields
  _fileNameDirty = false;
  _f0Dirty = false;
  _nDirty = false;
  _countFromEndDirty = false;
  _readToEndDirty = false;
  _doFilterDirty = false;
  _doSkipDirty = false;
  _skipDirty = false;
}


void KstVectorDialog::populateEditMultipleSV() {
  KstSVectorList vclist = kstObjectSubList<KstVector, KstSVector>(KST::vectorList);
  _editMultipleWidget->_objectList->insertStringList(vclist.tagNames());

  _w->_N->setMinValue(_w->_N->minValue() - 1);
  _w->_N->setSpecialValueText(" ");
  _w->_N->setValue(_w->_N->minValue());
  _w->_xMin->setText("");
  _w->_xMax->setText("");

  // clean all the fields
  _NDirty = false;
  _xMinDirty = false;
  _xMaxDirty = false;
}


void KstVectorDialog::populateEditMultiple() {
  _tagName->setText("");
  _tagName->setEnabled(false);

  if (kst_cast<KstRVector>(_dp)) {
    populateEditMultipleRV();
  } else {
    populateEditMultipleSV();
  }
}


void KstVectorDialog::setCountFromEndDirty() {
  _w->_kstDataRange->CountFromEnd->setTristate(false);
  _countFromEndDirty = true;
}


void KstVectorDialog::setReadToEndDirty() {
  _w->_kstDataRange->ReadToEnd->setTristate(false);
  _readToEndDirty = true;
}


void KstVectorDialog::setDoFilterDirty() {
  _w->_kstDataRange->DoFilter->setTristate(false);
  _doFilterDirty = true;
}


void KstVectorDialog::setDoSkipDirty() {
  _w->_kstDataRange->DoSkip->setTristate(false);
  _doSkipDirty = true;
  // also assume the number of frames to skip is dirty too
  _skipDirty = true;
}


void KstVectorDialog::cleanup() {
  if (_editMultipleMode) {
    if (_w->_kstDataRange->Skip->specialValueText() == " ") {
      _w->_kstDataRange->Skip->setSpecialValueText(QString::null);
      _w->_kstDataRange->Skip->setMinValue(_w->_kstDataRange->Skip->minValue() + 1);
    }
    if (_w->_N->specialValueText() == " ") {
      _w->_N->setSpecialValueText(QString::null);
      _w->_N->setMinValue(_w->_N->minValue() + 1);
    }
  }
}


KstObjectPtr KstVectorDialog::findObject(const QString& name) {
  KST::vectorList.lock().readLock();
  KstObjectPtr o = (*KST::vectorList.findTag(name)).data();
  KST::vectorList.lock().unlock();
  return o;
}

#include "kstvectordialog.moc"
