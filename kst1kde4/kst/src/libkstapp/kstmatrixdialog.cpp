/***************************************************************************
                      kstmatrixdialog.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>

#include "kstmatrixdefaults.h"
#include "kstmatrixdialog.h"
#include "kstviewwindow.h"
#include <defaultprimitivenames.h>
#include "vectorselector.h"

QPointer<KstMatrixDialog> KstMatrixDialog::_inst;

KstMatrixDialog *KstMatrixDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstMatrixDialog(KstApp::inst());
  }
  return _inst;
}


KstMatrixDialog::KstMatrixDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : KstDataDialog(parent) {
  _w = new Ui::MatrixDialogWidget();
  _w->setupUi(_contents);

  setMultiple(true);
  _inTest = false;
// xxx  _w->_fileName->completionObject()->setDir(QDir::currentPath());

  connect(_w->_readFromSource, SIGNAL(clicked()), this, SLOT(updateEnables()));
  connect(_w->_generateGradient, SIGNAL(clicked()), this, SLOT(updateEnables()));
  connect(_w->_xStartCountFromEnd, SIGNAL(clicked()), this, SLOT(xStartCountFromEndClicked()));
  connect(_w->_yStartCountFromEnd, SIGNAL(clicked()), this, SLOT(yStartCountFromEndClicked()));
  connect(_w->_xNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(xNumStepsReadToEndClicked()));
  connect(_w->_yNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(yNumStepsReadToEndClicked()));
  connect(_w->_doSkip, SIGNAL(clicked()), this, SLOT(updateEnables()));

// xxx  _w->_fileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
// xxx  connect(_w->_fileName, SIGNAL(openFileDialog(KURLRequester *)), this, SLOT(selectFolder()));
// xxx  connect(_w->_fileName, SIGNAL(textChanged(const QString&)), this, SLOT(updateCompletion()));
  connect(_w->_configure, SIGNAL(clicked()), this, SLOT(configureSource()));
  connect(_w->_readFromSource, SIGNAL(clicked()), this, SLOT(enableSource()));
  connect(_w->_generateGradient, SIGNAL(clicked()), this, SLOT(updateEnables()));
  connect(_w->_connect, SIGNAL(clicked()), this, SLOT(testURL()));

  _w->_configure->setEnabled(false);
// xxx  _fieldCompletion = _w->_field->completionObject();
// xxx  _w->_field->setAutoDeleteCompletionObject(true);
  setFixedHeight(height());
  _configWidget = 0L;
  _w->_field->setEnabled(false);
  _ok->setEnabled(_w->_field->isEnabled());

  //
  // connections for multiple edit mode...
  //

  connect(_w->_xStartCountFromEnd, SIGNAL(clicked()), this, SLOT(setXStartCountFromEndDirty()));
  connect(_w->_yStartCountFromEnd, SIGNAL(clicked()), this, SLOT(setYStartCountFromEndDirty()));
  connect(_w->_xNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(setXNumStepsReadToEndDirty()));
  connect(_w->_yNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(setYNumStepsReadToEndDirty()));
  connect(_w->_doSkip, SIGNAL(clicked()), this, SLOT(setDoSkipDirty()));
  connect(_w->_doAve, SIGNAL(clicked()), this, SLOT(setDoAveDirty()));

  //
  // connections for apply button...
  //

// xxx  connect(_w->_fileName, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_field, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_configure, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_xStart, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_yStart, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_xNumSteps, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_yNumSteps, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_xStart->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_yStart->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_xNumSteps->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_yNumSteps->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xStartCountFromEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_yStartCountFromEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_xNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_yNumStepsReadToEnd, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_doSkip, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_skip, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_skip->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_doAve, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_gradientX, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_gradientY, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_gradientZAtMin, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_gradientZAtMax, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_nX, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_nY, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_minX, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_minY, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xStep, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yStep, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


KstMatrixDialog::~KstMatrixDialog() {
}


void KstMatrixDialog::selectingFolder()
{
/* xxx
  QString strFolder = _w->_fileName->url();
  QFileDialog *fileDlg = _w->_fileName->fileDialog();
  QFileInfo fileInfo(strFolder);

  if (fileDlg) {
    if (fileInfo.isDir()) {
      QDir dir(strFolder);

      if (dir.cdUp()) {
        fileDlg->setDirectory(dir);
      }
    }
  }
*/
}


void KstMatrixDialog::selectFolder() {
  QTimer::singleShot(0, this, SLOT(selectingFolder()));
}


void KstMatrixDialog::updateWindow() {
}


void KstMatrixDialog::fillFieldsForEdit() {
  KstMatrixPtr mp;

  mp = kst_cast<KstMatrix>(_dp);
  if (mp) {
    KstRMatrixPtr rmp;

    mp->readLock();
    _tagName->setText(mp->tagName());
    _w->_minX->setText(QString::number(mp->minX()));
    _w->_minY->setText(QString::number(mp->minY()));
    _w->_xStep->setText(QString::number(mp->xStepSize()));
    _w->_yStep->setText(QString::number(mp->yStepSize()));
    mp->unlock();
  
    _w->_sourceGroup->hide();
  
    rmp = kst_cast<KstRMatrix>(mp);
    if (rmp) {
      fillFieldsForRMatrixEdit();
    } else {
      fillFieldsForSMatrixEdit();
    }
  
    updateEnables();
  
    adjustSize();
    resize(minimumSizeHint());
    setFixedHeight(height());
  }
}


void KstMatrixDialog::fillFieldsForRMatrixEdit() {
  KstRMatrixPtr rmp;

  //
  // first hide/show the correct widgets...
  //

  _w->_readFromSource->setChecked(true);
  _w->_generateGradient->setChecked(false);
  _w->_dataSourceGroup->show();
  _w->_dataRangeGroup->show();
  _w->_gradientGroup->hide();
  _w->_scalingGroup->hide();

  rmp = kst_cast<KstRMatrix>(_dp);
  if (rmp) {
    rmp->readLock();
  
    //
    // fill in the list of fields...
    //

    _w->_field->clear();
/* xxx
    if (_fieldCompletion) {
      _fieldCompletion->clear();
    }
*/
    //
    // scope for iterator...
    //

    {
      KstDataSourcePtr tf;
      KstDataSourceList::iterator it;

      KST::dataSourceList.lock().readLock();
      it = KST::dataSourceList.findReusableFileName(rmp->filename());
      if (it != KST::dataSourceList.end()) {
        tf = *it;
        tf->readLock();
        _w->_field->insertItems(0, tf->matrixList());
/* xxx
        if (_fieldCompletion) {
          _fieldCompletion->insertItems(tf->matrixList());
        }
*/
        tf->unlock();
      } else {
        QStringList list;

// xxx        list = KstDataSource::matrixListForSource(_w->_fileName->url());
        _w->_field->insertItems(0, list);
/* xxx
        if (_fieldCompletion) {
          _fieldCompletion->insertItems(list);
        }
*/
      }
      KST::dataSourceList.lock().unlock();
    }

    _w->_field->setEnabled(_w->_field->count() > 0);
    _ok->setEnabled(_w->_field->isEnabled());
    _w->_field->setItemText(_w->_field->currentIndex(), rmp->field());
  
    //
    // fill in the other parameters...
    //

// xxx    _w->_fileName->setURL(rmp->filename());
  
    _w->_xStart->setValue(rmp->reqXStart());
    _w->_yStart->setValue(rmp->reqYStart());
    _w->_xNumSteps->setValue(rmp->reqXNumSteps());
    _w->_yNumSteps->setValue(rmp->reqYNumSteps());
  
    _w->_xStartCountFromEnd->setChecked(rmp->xCountFromEnd());
    _w->_yStartCountFromEnd->setChecked(rmp->yCountFromEnd());
    _w->_xNumStepsReadToEnd->setChecked(rmp->xReadToEnd());
    _w->_yNumStepsReadToEnd->setChecked(rmp->yReadToEnd());
  
    _w->_doSkip->setChecked(rmp->doSkip());
    _w->_skip->setValue(rmp->skip());
    _w->_doAve->setChecked(rmp->doAverage());
  
    rmp->unlock();
  }
}


void KstMatrixDialog::fillFieldsForSMatrixEdit() {
  KstSMatrixPtr smp;

  //
  // first hide/show the correct widgets...
  //

  _w->_readFromSource->setChecked(false);
  _w->_generateGradient->setChecked(true);
  _w->_dataSourceGroup->hide();
  _w->_dataRangeGroup->hide();
  _w->_gradientGroup->show();
  _w->_scalingGroup->show();

  smp = kst_cast<KstSMatrix>(_dp);
  if (smp) {
    smp->readLock();

    _w->_gradientX->setChecked(smp->xDirection());
    _w->_gradientY->setChecked(!smp->xDirection());
    _w->_gradientZAtMin->setText(QString::number(smp->gradZMin()));
    _w->_gradientZAtMax->setText(QString::number(smp->gradZMax()));
    _w->_nX->setValue(smp->xNumSteps());
    _w->_nY->setValue(smp->yNumSteps());

    smp->unlock();
  }
}


void KstMatrixDialog::fillFieldsForNew() {
  _tagName->setText("<New_Matrix>");

  //
  // set defaults using KstMatrixDefaults...
  //

  KST::matrixDefaults.sync();

// xxx  _w->_fileName->setURL(KST::matrixDefaults.dataSource());
  _w->_minX->setText("0");
  _w->_minY->setText("1");
  _w->_xStep->setText("1");
  _w->_yStep->setText("1");
  _w->_nX->setValue(100);
  _w->_nY->setValue(100);
  _w->_xStart->setValue(KST::matrixDefaults.xStart());
  _w->_yStart->setValue(KST::matrixDefaults.yStart());
  _w->_xNumSteps->setValue(KST::matrixDefaults.xNumSteps());
  _w->_yNumSteps->setValue(KST::matrixDefaults.yNumSteps());
  _w->_gradientZAtMin->setText("0");
  _w->_gradientZAtMax->setText("100");

  _w->_xStartCountFromEnd->setChecked(KST::matrixDefaults.xCountFromEnd());
  _w->_yStartCountFromEnd->setChecked(KST::matrixDefaults.yCountFromEnd());
  _w->_xNumStepsReadToEnd->setChecked(KST::matrixDefaults.xReadToEnd());
  _w->_yNumStepsReadToEnd->setChecked(KST::matrixDefaults.yReadToEnd());
  _w->_doSkip->setChecked(KST::matrixDefaults.doSkip());
  _w->_doAve->setChecked(KST::matrixDefaults.doAverage());
  _w->_skip->setValue(KST::matrixDefaults.skip());

  _w->_gradientX->setChecked(true);
  _w->_gradientY->setChecked(false);

  _w->_sourceGroup->show();
  _w->_dataSourceGroup->show();
  _w->_dataRangeGroup->show();
  _w->_gradientGroup->show();
  _w->_scalingGroup->show();
  _w->_readFromSource->setChecked(true);

  updateEnables();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstMatrixDialog::update() {
  // nothing to do
}


bool KstMatrixDialog::new_IRMatrix() {
  KstDataSourcePtr file;
  KstRMatrixPtr matrix;
  KstDataSourceList::iterator it;
  QString pField;
  QString tagName;
  bool doSkip;
  bool doAve;
  int xStart = _w->_xStartCountFromEnd->isChecked() ? -1 : _w->_xStart->value();
  int yStart = _w->_yStartCountFromEnd->isChecked() ? -1 : _w->_yStart->value();
  int xNumSteps = _w->_xNumStepsReadToEnd->isChecked() ? -1 : _w->_xNumSteps->value();
  int yNumSteps = _w->_yNumStepsReadToEnd->isChecked() ? -1 : _w->_yNumSteps->value();
  int skip;

  //
  // create a unique name...
  //

  tagName = (_tagName->text() == "<New_Matrix>") ? KST::suggestMatrixName(_w->_field->currentText()) : _tagName->text();
  if (KstData::self()->matrixTagNameNotUnique(tagName)) {
    _tagName->setFocus();

    return false;
  }

  //
  // if there is not an active KstFile, create one...
  //

  KST::dataSourceList.lock().writeLock();
// xxx  it = KST::dataSourceList.findReusableFileName(_w->_fileName->url());

  if (it == KST::dataSourceList.end()) {
// xxx    file = KstDataSource::loadSource(_w->_fileName->url());
    if (!file || !file->isValid()) {
      KST::dataSourceList.lock().unlock();
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The file could not be opened."));

      return false;
    }

    if (file->isEmpty()) {
      KST::dataSourceList.lock().unlock();
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The file does not contain data."));

      return false;
    }
    KST::dataSourceList.append(file);
  } else {
    file = *it;
  }
  KST::dataSourceList.lock().unlock();

  pField = _w->_field->currentText();

  if (!file->isValidMatrix(pField)) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The requested matrix is not defined for the requested file."));
    file->unlock();

    return false;
  }

  doSkip = _w->_doSkip->isChecked();
  doAve = _w->_doAve->isChecked();
  skip = _w->_skip->value();

  matrix = new KstRMatrix(file, pField, KstObjectTag(tagName, file->tag(), false),
                          xStart, yStart, xNumSteps, yNumSteps, doAve, doSkip, skip);

  emit matrixCreated(KstMatrixPtr(matrix));

  matrix = 0L; // drop the reference

  emit modified();

  return true;
}


bool KstMatrixDialog::new_ISMatrix() {
  KstSMatrixPtr matrix;
  QString tagPart = _w->_gradientZAtMin->text() + "-" + _w->_gradientZAtMax->text();
  QString tagName = (_tagName->text() == "<New_Matrix>") ? KST::suggestMatrixName(tagPart) : _tagName->text();
  double zMin, zMax, xStep, yStep, minX, minY;
  bool xDirection, ok1, ok2, ok3, ok4, ok5, ok6;
  int nX, nY;

  if (KstData::self()->matrixTagNameNotUnique(tagName)) {
    _tagName->setFocus();

    return false;
  }

  xDirection = _w->_gradientX->isChecked();
  zMin = _w->_gradientZAtMin->text().toDouble(&ok1);
  zMax = _w->_gradientZAtMax->text().toDouble(&ok2);
  nX = _w->_nX->value();
  nY = _w->_nY->value();

  xStep = _w->_xStep->text().toDouble(&ok3);
  yStep = _w->_yStep->text().toDouble(&ok4);
  minX = _w->_minX->text().toDouble(&ok5);
  minY = _w->_minY->text().toDouble(&ok6);

  if (!checkParameters(ok3, ok4, ok5, ok6, xStep, yStep)) {
    return false;
  }

  if (!ok1 || !ok2) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Invalid gradient bounds.  Ensure only decimal values are entered."));

    return false;
  }

  matrix = new KstSMatrix(KstObjectTag(tagName, KstObjectTag::globalTagContext),
      nX, nY, minX, minY, xStep, yStep, zMin, zMax, xDirection);

  emit matrixCreated(KstMatrixPtr(matrix));

  matrix = 0L; // drop the reference

  emit modified();

  return true;
}


bool KstMatrixDialog::newObject() {
  if (_w->_readFromSource->isChecked()) {
    return new_IRMatrix();
  }

  return new_ISMatrix();
}


bool KstMatrixDialog::editSingleRMatrix(KstRMatrixPtr rmp) {
  KstDataSourcePtr file;
  QString pField;
  bool doSkip, doAve;
  int xStart;
  int yStart;
  int xNumSteps;
  int yNumSteps;
  int skip;

  if (_fileNameDirty) {
    KstDataSourceList::iterator it;

    //
    // if there is not an active KstFile, create one...
    //

    KST::dataSourceList.lock().writeLock();
// xxx    it = KST::dataSourceList.findReusableFileName(_w->_fileName->url());

    if (it == KST::dataSourceList.end()) {
// xxx      file = KstDataSource::loadSource(_w->_fileName->url());
      if (!file || !file->isValid()) {
        KST::dataSourceList.lock().unlock();
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The file could not be opened."));

        return false;
      }

      if (file->isEmpty()) {
        KST::dataSourceList.lock().unlock();
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The file does not contain data."));

        return false;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }
    KST::dataSourceList.lock().unlock();

    pField = _w->_field->currentText();
    if (!file->isValidMatrix(pField)) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The requested field is not defined for the requested file."));
      file->unlock();

      return false;
    }
  } else {
    rmp->readLock();
    file = rmp->dataSource();
    pField = rmp->field();
    rmp->unlock();
  }

  rmp->readLock();

  if (_xStartDirty || _xStartCountFromEndDirty) {
    xStart = _w->_xStartCountFromEnd->isChecked() ? -1 : _w->_xStart->value();
  } else {
    xStart = rmp->reqXStart();
  }

  if (_yStartDirty || _yStartCountFromEndDirty) {
    yStart = _w->_yStartCountFromEnd->isChecked() ? -1 : _w->_yStart->value();
  } else {
    yStart = rmp->reqYStart();
  }

  if (_xNumStepsDirty || _xNumStepsReadToEndDirty) {
    xNumSteps = _w->_xNumStepsReadToEnd->isChecked() ? -1 : _w->_xNumSteps->value();
  } else {
    xNumSteps = rmp->reqXNumSteps();
  }

  if (_yNumStepsDirty || _yNumStepsReadToEndDirty) {
    yNumSteps = _w->_yNumStepsReadToEnd->isChecked() ? -1 : _w->_yNumSteps->value();
  } else {
    yNumSteps = rmp->reqYNumSteps();
  }

  if (_doSkipDirty) {
    doSkip = _w->_doSkip->isChecked();
  } else {
    doSkip = rmp->doSkip();
  }

  if (_doAveDirty) {
    doAve = _w->_doAve->isChecked();
  } else {
    doAve = rmp->doAverage();
  }

  if (_skipDirty) {
    skip = _w->_skip->value();
  } else {
    skip = rmp->skip();
  }

  rmp->unlock();

  rmp->writeLock();
  rmp->change(file, pField, KstObjectTag(rmp->tag().tag(), rmp->tag().context()), xStart, yStart, xNumSteps, yNumSteps, doAve, doSkip, skip);
  rmp->unlock();

  return true;
}


bool KstMatrixDialog::editSingleSMatrix(KstSMatrixPtr smp) {
  double gradientZAtMin, gradientZAtMax;
  double xMin, yMin, xStepSize, yStepSize;
  bool ok1 = true, ok2 = true, ok3 = true, ok4 = true;
  bool xDirection, ok5 = true, ok6 = true;
  int nX, nY;

  smp->readLock();

  if (_xStepDirty) {
    xStepSize = _w->_xStep->text().toDouble(&ok1);
  } else {
    xStepSize = smp->xStepSize();
  }

  if (_yStepDirty) {
    yStepSize = _w->_yStep->text().toDouble(&ok2);
  } else {
    yStepSize = smp->yStepSize();
  }

  if (_minXDirty) {
    xMin = _w->_minX->text().toDouble(&ok3);
  } else {
    xMin = smp->minX();
  }

  if (_minYDirty) {
    yMin = _w->_minY->text().toDouble(&ok4);
  } else {
    yMin = smp->minY();
  }

  if (_gradientXDirty || _gradientYDirty) {
    xDirection = _w->_gradientX->isChecked();
  } else {
    xDirection = smp->xDirection();
  }

  if (_gradientZAtMinDirty) {
    gradientZAtMin = _w->_gradientZAtMin->text().toDouble(&ok5);
  } else {
    gradientZAtMin = smp->gradZMin();
  }

  if (_gradientZAtMaxDirty) {
    gradientZAtMax = _w->_gradientZAtMax->text().toDouble(&ok6);
  } else {
    gradientZAtMax = smp->gradZMax();
  }

  if (_nXDirty) {
    nX = _w->_nX->value();
  } else {
    nX = smp->xNumSteps();
  }

  if (_nYDirty) {
    nY = _w->_nY->value();
  } else {
    nY = smp->yNumSteps();
  }

  smp->unlock();

  if (!ok5 || !ok6) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Gradient values are invalid.  Ensure only decimal values are entered."));

    return false;
  }

  if (!checkParameters(ok1, ok2, ok3, ok4, xStepSize, yStepSize)) {
    return false;
  }

  smp->writeLock();
  smp->change(KstObjectTag(smp->tag().tag(), smp->tag().context()), nX, nY, xMin, yMin, xStepSize, yStepSize, gradientZAtMin, gradientZAtMax, xDirection);
  smp->unlock();

  return true;
}


bool KstMatrixDialog::editSingleObject(KstMatrixPtr mxPtr) {
  KstRMatrixPtr rmp;
  
  rmp = kst_cast<KstRMatrix>(mxPtr);
  if (rmp) {
    return editSingleRMatrix(rmp);
  } else {
    KstSMatrixPtr smp;

    smp = kst_cast<KstSMatrix>(mxPtr);
    if (smp) {
      return editSingleSMatrix(smp);
    }
  }

  return false;
}


bool KstMatrixDialog::editObject() {
  //
  // if editing multiple objects, edit each one...
  //

  if (_editMultipleMode) {
// xxx    _fileNameDirty = !_w->_fileName->url().isEmpty();
    _gradientZAtMinDirty = !_w->_gradientZAtMin->text().isEmpty();
    _gradientZAtMaxDirty = !_w->_gradientZAtMax->text().isEmpty();
    _minXDirty = !_w->_minX->text().isEmpty();
    _minYDirty = !_w->_minY->text().isEmpty();
    _yStepDirty = !_w->_yStep->text().isEmpty();
    _xStepDirty = !_w->_xStep->text().isEmpty();

    _xStartDirty = _w->_xStart->text() != " ";
    _yStartDirty = _w->_yStart->text() != " ";
    _xNumStepsDirty = _w->_xNumSteps->text() != " ";
    _yNumStepsDirty = _w->_yNumSteps->text() != " ";

    _skipDirty = _w->_skip->text() != " ";
    _nXDirty = _w->_nX->text() != " ";
    _nYDirty = _w->_nY->text() != " ";

    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstMatrixPtr mxPtr;

        //
        // get the pointer to the object...
        //

        KST::matrixList.lock().readLock();
        mxPtr = *KST::matrixList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        KST::matrixList.lock().unlock();
        
        if (!mxPtr) {
          return false;
        }

        if (!editSingleObject(mxPtr)) {
          return false;
        }

        didEdit = true;
      }
    }

    if (!didEdit) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select one or more objects to edit."));

      return false;
    }
  } else {
    KstMatrixPtr mp;
    QString tagName;

    mp = kst_cast<KstMatrix>(_dp);
    tagName = _tagName->text();
    if (!mp || (tagName != mp->tagName() && KstData::self()->dataTagNameNotUnique(tagName))) {
      _tagName->setFocus();

      return false;
    }

    mp->writeLock();
    mp->setTag(KstObjectTag(tagName, mp->tag().context())); // FIXME: can't change tag context
    mp->unlock();

    //
    // then edit the object...
    //

    _fileNameDirty = true;
    _fieldDirty = true;
    _xStartDirty = true;
    _xStartCountFromEndDirty = true;
    _yStartDirty = true;
    _yStartCountFromEndDirty = true;
    _xNumStepsDirty = true;
    _xNumStepsReadToEndDirty = true;
    _yNumStepsDirty = true;
    _yNumStepsReadToEndDirty = true;
    _gradientXDirty = true;
    _gradientYDirty = true;
    _gradientZAtMinDirty = true;
    _gradientZAtMaxDirty = true;
    _minXDirty = true;
    _minYDirty = true;
    _xStepDirty = true;
    _yStepDirty = true;
    _doSkipDirty = true;
    _skipDirty = true;
    _doAveDirty = true;
    _nXDirty = true;
    _nYDirty = true;

    if (!editSingleObject(mp)) {
      return false;
    }
  }

  emit modified();

  return true;
}


bool KstMatrixDialog::checkParameters(bool ok1, bool ok2, bool ok3, bool ok4, double xStep, double yStep) {
  if (!(ok1 && ok2 && ok3 && ok4)) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("One or more grid parameters have invalid values.  Ensure that only decimal values are entered."));

    return false;
  }

  if (xStep <= 0 || yStep <= 0) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Invalid step size entered.  Ensure the step sizes are positive."));

    return false;
  }

  return true;
}


void KstMatrixDialog::populateEditMultiple() {
  if (kst_cast<KstRMatrix>(_dp)) {
    populateEditMultipleRMatrix();
  } else {
    populateEditMultipleSMatrix();
  }

  //
  // also intermediate state for multiple edit...
  //

  _w->_minX->setText("");
  _w->_minY->setText("");
  _w->_xStep->setText("");
  _w->_yStep->setText("");
  _tagName->setText("");
  _tagName->setEnabled(false);

  //
  // and clean all the fields...
  //

  _fileNameDirty = false;
  _fieldDirty = false;
  _xStartDirty = false;
  _xStartCountFromEndDirty = false;
  _yStartDirty = false;
  _yStartCountFromEndDirty = false;
  _xNumStepsDirty = false;
  _xNumStepsReadToEndDirty = false;
  _yNumStepsDirty = false;
  _yNumStepsReadToEndDirty = false;
  _gradientXDirty = false;
  _gradientYDirty = false;
  _gradientZAtMinDirty = false;
  _gradientZAtMaxDirty = false;
  _minXDirty = false;
  _minYDirty = false;
  _xStepDirty = false;
  _yStepDirty = false;
  _doSkipDirty = false;
  _skipDirty = false;
  _doAveDirty = false;
  _nXDirty = false;
  _nYDirty = false;
}


void KstMatrixDialog::populateEditMultipleRMatrix() {
  KstRMatrixList mxList;

  mxList = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  _editMultipleWidget->_objectList->insertItems(0, mxList.tagNames());

  //
  // intermediate state for multiple edit...
  //

// xxx  _w->_fileName->clear();
  _w->_xStart->setSpecialValueText(" ");
  _w->_xStart->setMinimum(_w->_xStart->minimum() - 1);
  _w->_xStart->setValue(_w->_xStart->minimum());

  _w->_yStart->setSpecialValueText(" ");
  _w->_yStart->setMinimum(_w->_yStart->minimum() - 1);
  _w->_yStart->setValue(_w->_yStart->minimum());

  _w->_xNumSteps->setSpecialValueText(" ");
  _w->_xNumSteps->setMinimum(_w->_xNumSteps->minimum() - 1);
  _w->_xNumSteps->setValue(_w->_xNumSteps->minimum());

  _w->_yNumSteps->setSpecialValueText(" ");
  _w->_yNumSteps->setMinimum(_w->_yNumSteps->minimum() - 1);
  _w->_yNumSteps->setValue(_w->_yNumSteps->minimum());

  _w->_skip->setSpecialValueText(" ");
  _w->_skip->setMinimum(_w->_skip->minimum() - 1);
  _w->_skip->setValue(_w->_skip->minimum());

  _w->_doSkip->setTristate(true);
  _w->_doSkip->setChecked(Qt::PartiallyChecked);
  _w->_doAve->setTristate(true);
  _w->_doAve->setChecked(Qt::PartiallyChecked);
  _w->_xStartCountFromEnd->setTristate(true);
  _w->_xStartCountFromEnd->setChecked(Qt::PartiallyChecked);
  _w->_yStartCountFromEnd->setTristate(true);
  _w->_yStartCountFromEnd->setChecked(Qt::PartiallyChecked);
  _w->_xNumStepsReadToEnd->setTristate(true);
  _w->_xNumStepsReadToEnd->setChecked(Qt::PartiallyChecked);
  _w->_yNumStepsReadToEnd->setTristate(true);
  _w->_yNumStepsReadToEnd->setChecked(Qt::PartiallyChecked);
  _w->_xStart->setEnabled(true);
  _w->_xNumSteps->setEnabled(true);
  _w->_yStart->setEnabled(true);
  _w->_yNumSteps->setEnabled(true);
}


void KstMatrixDialog::populateEditMultipleSMatrix() {
  KstSMatrixList mxList;

  mxList = kstObjectSubList<KstMatrix,KstSMatrix>(KST::matrixList);
  _editMultipleWidget->_objectList->insertItems(0, mxList.tagNames());

  //
  // intermediate state for multiple edit...
  //

  _w->_gradientZAtMin->setText("");
  _w->_gradientZAtMax->setText("");
  _w->_nX->setSpecialValueText(" ");
  _w->_nX->setMinimum(_w->_nX->minimum() - 1);
  _w->_nX->setValue(_w->_nX->minimum());
  _w->_nY->setSpecialValueText(" ");
  _w->_nY->setMinimum(_w->_nY->minimum() - 1);
  _w->_nY->setValue(_w->_nY->minimum());
}


void KstMatrixDialog::cleanup() {
  if (_editMultipleMode) {
    _w->_xStart->setSpecialValueText(QString::null);
    _w->_xStart->setMinimum(_w->_xStart->minimum() + 1);
    _w->_yStart->setSpecialValueText(QString::null);
    _w->_yStart->setMinimum(_w->_yStart->minimum() + 1);
    _w->_xNumSteps->setSpecialValueText(QString::null);
    _w->_xNumSteps->setMinimum(_w->_xNumSteps->minimum() + 1);
    _w->_yNumSteps->setSpecialValueText(QString::null);
    _w->_yNumSteps->setMinimum(_w->_yNumSteps->minimum() + 1);
    _w->_skip->setSpecialValueText(QString::null);
    _w->_skip->setMinimum(_w->_skip->minimum() + 1);
    _w->_nX->setSpecialValueText(QString::null);
    _w->_nY->setSpecialValueText(QString::null);
  }
}


void KstMatrixDialog::updateCompletion() {
  KstDataSourcePtr ds;
  QString current_text = _w->_field->currentText();
  QStringList list;

  _w->_field->clear();

  //
  // update filename list and ll axes combo boxes...
  //

  KST::dataSourceList.lock().readLock();
// xxx  ds = *KST::dataSourceList.findReusableFileName(_w->_fileName->url());
  KST::dataSourceList.lock().unlock();

  delete _configWidget;
  _configWidget = 0L;

  if (ds) {
    ds->readLock();
    list = ds->matrixList();
    _w->_field->setEditable(!ds->fieldListIsComplete());
    _configWidget = ds->configWidget();
    ds->unlock();
    _w->_field->setEnabled(true);
    _w->_connect->hide();
  //  _kstDataRange->setAllowTime(ds->supportsTimeConversions());
  } else {
    bool complete = false;
    QString u;
    QString type;

/* xxx
    u = _w->_fileName->url();
    
    if (QFile::exists(u) && QFileInfo(u).isRelative()) {
      url.setPath(u);
    } else {
      url = QUrl(u);
    }

    if (!_inTest && !url.scheme()=="file" && url.scheme() != "file" && !url.isRelative()) {
      _w->_connect->show();
    } else if (url.isValid()) {
      list = KstDataSource::matrixListForSource(u, QString::null, &type, &complete);

      //
      // pretend we're getting the full field list...
      //

      if (list.isEmpty()) {
        QStringList fullList = KstDataSource::fieldListForSource(u, QString::null, &type, &complete);
      }

      if (!_inTest || (_inTest && !list.isEmpty())) {
        _w->_connect->hide();
      }
    }
*/
    _w->_field->setEditable(!complete);
    _w->_field->setEnabled(!list.isEmpty());
    if (!type.isEmpty()) {
      _configWidget = KstDataSource::configWidgetForSource(u, type);
    }
//    _kstDataRange->setAllowTime(KstDataSource::supportsTime(u, type));
  }

  _w->_configure->setEnabled(_configWidget);

// xxx  _fieldCompletion = _w->_field->completionObject();

  _w->_field->insertItems(0, list);
/* xxx
  if (_fieldCompletion) {
    _fieldCompletion->clear();
    _fieldCompletion->insertItems(list);
  }
*/

  if (!current_text.isEmpty() && (list.contains(current_text) || _w->_field->isEditable())) {
    _w->_field->setItemText(_w->_field->currentIndex(), current_text);
  }

  _ok->setEnabled(_w->_field->isEnabled() || _editMultipleMode);
}


void KstMatrixDialog::markSourceAndSave() {
  if (_configWidget) {
    KstDataSourcePtr src;

    src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
    if (src) {
      src->disableReuse();
    }
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
  }
}


void KstMatrixDialog::setXStartCountFromEndDirty() {
  _w->_xStartCountFromEnd->setTristate(false);
  _xStartCountFromEndDirty = true;
}


void KstMatrixDialog::setYStartCountFromEndDirty() {
  _w->_yStartCountFromEnd->setTristate(false);
  _yStartCountFromEndDirty = true;
}


void KstMatrixDialog::setXNumStepsReadToEndDirty() {
  _w->_xNumStepsReadToEnd->setTristate(false);
  _xNumStepsReadToEndDirty = true;
}


void KstMatrixDialog::setYNumStepsReadToEndDirty() {
  _w->_yNumStepsReadToEnd->setTristate(false);
  _yNumStepsReadToEndDirty = true;
}


void KstMatrixDialog::updateEnables() {
  _w->_dataSourceGroup->setEnabled(_w->_readFromSource->isChecked());
  _w->_dataRangeGroup->setEnabled(_w->_readFromSource->isChecked());
  _w->_gradientGroup->setEnabled(_w->_generateGradient->isChecked());
  _w->_scalingGroup->setEnabled(_w->_generateGradient->isChecked());
  _ok->setEnabled(_ok->isEnabled() || !_w->_readFromSource->isChecked());

  //
  // also some enables for the checkboxes and spinboxes...
  //

  if (_w->_dataRangeGroup->isEnabled()) {
    _w->_skip->setEnabled(_w->_doSkip->isChecked());
    _w->_doAve->setEnabled(_w->_doSkip->isChecked());
    xStartCountFromEndClicked();
    xNumStepsReadToEndClicked();
    yStartCountFromEndClicked();
    yNumStepsReadToEndClicked();
  }
}


void KstMatrixDialog::xStartCountFromEndClicked() {
  _w->_xNumStepsReadToEnd->setChecked(_w->_xNumStepsReadToEnd->isChecked() && !_w->_xStartCountFromEnd->isChecked());
  _w->_xStart->setEnabled(!_w->_xStartCountFromEnd->isChecked());
  _w->_xNumSteps->setEnabled(!_w->_xNumStepsReadToEnd->isChecked());
}


void KstMatrixDialog::xNumStepsReadToEndClicked() {
  _w->_xStartCountFromEnd->setChecked(_w->_xStartCountFromEnd->isChecked() && !_w->_xNumStepsReadToEnd->isChecked());
  _w->_xNumSteps->setEnabled(!_w->_xNumStepsReadToEnd->isChecked());
  _w->_xStart->setEnabled(!_w->_xStartCountFromEnd->isChecked());
}


void KstMatrixDialog::yStartCountFromEndClicked() {
  _w->_yNumStepsReadToEnd->setChecked(_w->_yNumStepsReadToEnd->isChecked() && !_w->_yStartCountFromEnd->isChecked());
  _w->_yStart->setEnabled(!_w->_yStartCountFromEnd->isChecked());
  _w->_yNumSteps->setEnabled(!_w->_yNumStepsReadToEnd->isChecked());
}


void KstMatrixDialog::yNumStepsReadToEndClicked() {
  _w->_yStartCountFromEnd->setChecked(_w->_yStartCountFromEnd->isChecked() && !_w->_yNumStepsReadToEnd->isChecked());
  _w->_yNumSteps->setEnabled(!_w->_yNumStepsReadToEnd->isChecked());
  _w->_yStart->setEnabled(!_w->_yStartCountFromEnd->isChecked());
}


void KstMatrixDialog::setDoSkipDirty() {
  _w->_doSkip->setTristate(false);
  _doSkipDirty = true;
}


void KstMatrixDialog::setDoAveDirty() {
  _w->_doAve->setTristate(false);
  _doAveDirty = true;
}


void KstMatrixDialog::configureSource() {
  KstDataSourcePtr ds;
  bool isNew = false;

  KST::dataSourceList.lock().readLock();
// xxx  ds = *KST::dataSourceList.findReusableFileName(_w->_fileName->url());
  KST::dataSourceList.lock().unlock();

  if (!ds) {
    isNew = true;
// xxx    ds = KstDataSource::loadSource(_w->_fileName->url());
    if (!ds || !ds->isValid()) {
      _w->_configure->setEnabled(false);

      return;
    }
  }

  if (_configWidget) {
    QDialog *dlg;
  
    dlg = new QDialog(this);
    dlg->setWindowTitle(QObject::tr("Configure Data Source"));

    if (isNew) {
      connect(dlg, SIGNAL(okClicked()), _configWidget, SLOT(save()));
      connect(dlg, SIGNAL(applyClicked()), _configWidget, SLOT(save()));
    } else {
      connect(dlg, SIGNAL(okClicked()), this, SLOT(markSourceAndSave()));
      connect(dlg, SIGNAL(applyClicked()), this, SLOT(markSourceAndSave()));
    }

    _configWidget->setParent(dlg);
// xxx    dlg->setMainWidget(_configWidget);
    _configWidget->setInstance(ds);
    _configWidget->load();
    dlg->exec();
// xxx    _configWidget->reparent(0L, QPoint(0, 0));
// xxx    dlg->setMainWidget(0L);

    delete dlg;
  }

  updateCompletion(); // could be smarter by only running if Ok/Apply clicked
}


void KstMatrixDialog::enableSource() {
  _w->_dataSourceGroup->setEnabled(true);
  _w->_gradientGroup->setEnabled(false);
  _ok->setEnabled(_w->_field->isEnabled());
  _w->_dataRangeGroup->setEnabled(true);
}


void KstMatrixDialog::testURL() {
  _inTest = true;
  updateCompletion();
  _inTest = false;
}


KstObjectPtr KstMatrixDialog::findObject(const QString& name) {
  KstObjectPtr obj;

  KST::matrixList.lock().readLock();
  obj = (*KST::matrixList.findTag(name));
  KST::matrixList.lock().unlock();

  return obj;
}

#include "kstmatrixdialog.moc"
