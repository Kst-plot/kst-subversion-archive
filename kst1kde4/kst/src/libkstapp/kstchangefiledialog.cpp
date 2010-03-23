/**************************************************************************
        kstchangefiledialog.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2001
    copyright            : (C) 2000-2003 by Barth Netterfield
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
#include <qradiobutton.h>
#include <QMessageBox>
#include <qcombobox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include <qregexp.h>

#include "kst.h"
#include "kst2dplot.h"
#include "kstchangefiledialog_i.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstrmatrix.h"
#include "kstrvector.h"
#include "kstvectordefaults.h"
#include "kstviewwindow.h"
#include "treetools.h"

KstChangeFileDialog::KstChangeFileDialog(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           Qt::WindowFlags fl)
: KstChangeFileDialog(parent, fl) {
  setupUi(this);

  connect(_clearFilter, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(_clearFilter, SIGNAL(clicked()), ChangeFileCurveList, SLOT(clearSelection()));
  connect(_filter, SIGNAL(textChanged(const QString&)), this, SLOT(updateSelection(const QString&)));
  connect(ChangeFileClear, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(ChangeFileClear, SIGNAL(clicked()), ChangeFileCurveList, SLOT(clearSelection()));
  connect(ChangeFileSelectAll, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(ChangeFileSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
  connect(ChangeFileApply, SIGNAL(clicked()), this, SLOT(applyFileChange()));
  connect(ChangeFileOK, SIGNAL(clicked()), this, SLOT(OKFileChange()));
  connect(_allFromFile, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(_allFromFile, SIGNAL(clicked()), this, SLOT(allFromFile()));
  connect(_duplicateSelected, SIGNAL(toggled(bool)), _duplicateDependents, SLOT(setEnabled(bool)));
  connect(_dataFile, SIGNAL(textChanged(const QString&)), this, SLOT(sourceChanged(const QString&)));
  connect(_configureSource, SIGNAL(clicked()), this, SLOT(configureSource()));

  _dataFile->completionObject()->setDir(QDir::currentDirPath());
  _dataFile->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);

  _clearFilter->setPixmap(BarIcon("locationbar_erase"));
  _duplicateDependents->setEnabled(_duplicateSelected->isChecked());
  _configWidget = 0L;
  _first = true;
}


KstChangeFileDialog::~KstChangeFileDialog() {
  delete _configWidget;
}


void KstChangeFileDialog::selectAll() {
  ChangeFileCurveList->selectAll(true);
}


void KstChangeFileDialog::updateChangeFileDialog() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KstRMatrixList rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  QMap<QString, QString> filesUsed;
  int i;

  // clear list
  ChangeFileCurveList->clear();

  // first add vectors
  for (i = 0; i < (int)rvl.count(); i++) {
    rvl[i]->readLock();
    ChangeFileCurveList->insertItem(rvl[i]->tag().displayString());
    filesUsed.insert(rvl[i]->filename(), rvl[i]->filename()); 
    rvl[i]->unlock();
  }

  // then add matrices
  for (i = 0; i < (int)rml.count(); i++) {
    rml[i]->readLock();
    ChangeFileCurveList->insertItem(rml[i]->tag().displayString());
    filesUsed.insert(rml[i]->filename(), rml[i]->filename()); 
    rml[i]->unlock();  
  }

  QString currentFile = _files->currentText();
  _files->clear();
  KstReadLocker ml(&KST::dataSourceList.lock());
  for (KstDataSourceList::Iterator it = KST::dataSourceList.begin(); it != KST::dataSourceList.end(); ++it) {
    if (filesUsed.contains((*it)->fileName())) {
      _files->insertItem((*it)->fileName());
    }
  }
  if (_files->contains(currentFile)) {
    _files->setCurrentText(currentFile);
  }

  _allFromFile->setEnabled(_files->count() > 0);
  _files->setEnabled(_files->count() > 0);

  if (_first) {
    _dataFile->setURL(KST::vectorDefaults.dataSource());
    _first = false;
  }
}


void KstChangeFileDialog::showChangeFileDialog() {
  updateChangeFileDialog();
  show();
  raise();
}


void KstChangeFileDialog::OKFileChange() {
  if (applyFileChange()) {
    reject();
  }
}


void KstChangeFileDialog::sourceChanged(const QString& text)
{
  delete _configWidget;
  _configWidget = 0L;
  _configureSource->setEnabled(false);
  _file = QString::null;
  if (!text.isEmpty() && text != "stdin" && text != "-") {
    KURL url;
    QString txt = _dataFile->completionObject()->replacedPath(text);
    if (QFile::exists(txt) && QFileInfo(txt).isRelative()) {
      url.setPath(txt);
    } else {
      url = KURL::fromPathOrURL(txt);
    }

    if (!url.isLocalFile() && url.protocol() != "file" && !url.protocol().isEmpty()) {
      _fileType->setText(QString::null);
      return;
    }

    if (!url.isValid()) {
      _fileType->setText(QString::null);
      return;
    }

    QString file = txt;

    KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(file);
    QStringList fl;
    QString fileType;

    if (ds) {
      ds->readLock();
      fl = ds->fieldList();
      fileType = ds->fileType();
      ds->unlock();
      ds = 0L;
    } else {
      bool complete = false;
      fl = KstDataSource::fieldListForSource(file, QString::null, &fileType, &complete);
    }

    if (!fl.isEmpty() && !fileType.isEmpty()) {
      if (ds) {
        ds->writeLock();
        _configWidget = ds->configWidget();
        ds->unlock();
      } else {
        _configWidget = KstDataSource::configWidgetForSource(file, fileType);
      }
    }

    _configureSource->setEnabled(_configWidget);
    _file = file;
    _fileType->setText(fileType.isEmpty() ? QString::null : i18n("Data source of type: %1").arg(fileType));
  } else {
    _fileType->setText(QString::null);
  }
}


void KstChangeFileDialog::configureSource()
{
  bool isNew = false;
  KST::dataSourceList.lock().readLock();
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);
  KST::dataSourceList.lock().unlock();
  if (!ds) {
    isNew = true;
    ds = KstDataSource::loadSource(_file);
    if (!ds || !ds->isValid()) {
      _configureSource->setEnabled(false);
      return;
    }
  }

  if (_configWidget) {
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
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->setInstance(ds);
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->load();
    dlg->exec();
    _configWidget->reparent(0L, QPoint(0, 0));
    dlg->setMainWidget(0L);
    delete dlg;
    sourceChanged(_dataFile->url());
  }
}


void KstChangeFileDialog::markSourceAndSave()
{
  if (_configWidget) {
    KstDataSourcePtr src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
    if (src) {
      src->disableReuse();
    }
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
  }
}


bool KstChangeFileDialog::applyFileChange() {
  KstDataSourcePtr file;
  KST::dataSourceList.lock().writeLock();
  KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(_dataFile->url());
  QString invalidSources;
  int invalid = 0;

  if (it == KST::dataSourceList.end()) {
    file = KstDataSource::loadSource(_dataFile->url());
    if (!file || !file->isValid()) {
      KST::dataSourceList.lock().unlock();
      QMessageBox::warning(this, i18n("Kst"), i18n("The file could not be loaded."));
      return false;
    }
    if (file->isEmpty()) {
      KST::dataSourceList.lock().unlock();
      QMessageBox::warning(this, i18n("Kst"), i18n("The file does not contain data."));
      return false;
    }
    KST::dataSourceList.append(file);
  } else {
    file = *it;
  }
  KST::dataSourceList.lock().unlock();

  KstApp *app = KstApp::inst();
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KstRMatrixList rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  int selected = 0;
  int handled = 0;

  int count = (int)ChangeFileCurveList->count();
  for (int i = 0; i < count; i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      ++selected;
    }
  }

  // a map to keep track of which objects have been duplicated, and mapping
  // old object -> new object
  KstDataObjectDataObjectMap duplicatedMap;
  QMap<KstVectorPtr, KstVectorPtr> duplicatedVectors;
  QMap<KstMatrixPtr, KstMatrixPtr> duplicatedMatrices;

  KstDataSourceList oldSources;

  // go through the vectors
  for (int i = 0; i < (int)rvl.count(); i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      KstRVectorPtr vector = rvl[i];
      vector->writeLock();
      file->readLock();
      bool valid = file->isValidField(vector->field());
      file->unlock();
      if (!valid) {
        if (invalid > 0) {
          invalidSources = i18n("%1, %2").arg(invalidSources).arg(vector->field());
        } else {
          invalidSources = vector->field();
        }
        ++invalid;
      } else {
        if (_duplicateSelected->isChecked()) {
          // block vector updates until vector is setup properly
          KST::vectorList.lock().writeLock();

          // create a new vector
          KstRVectorPtr newVector = vector->makeDuplicate();
          if (!oldSources.contains(newVector->dataSource())) {
            oldSources << newVector->dataSource();
          }
          newVector->changeFile(file);

          KST::vectorList.lock().unlock();

          // duplicate dependents
          if (_duplicateDependents->isChecked()) {
            duplicatedVectors.insert(KstVectorPtr(vector), KstVectorPtr(newVector));
            KST::duplicateDependents(KstVectorPtr(vector), duplicatedMap, duplicatedVectors);
          }
        } else {
          if (!oldSources.contains(vector->dataSource())) {
            oldSources << vector->dataSource();
          }
          vector->changeFile(file);
        }
      }
      vector->unlock();
      app->slotUpdateProgress(selected, ++handled, i18n("Updating vectors..."));
    }
  }

  // go through the matrices
  for (int i = (int)rvl.count(); i < (int)ChangeFileCurveList->count(); i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      KstRMatrixPtr matrix = rml[i-rvl.count()];
      matrix->writeLock();
      file->readLock();
      bool valid = file->isValidMatrix(matrix->field());
      file->unlock();
      if (!valid) {
        if (invalid > 0) {
          invalidSources = i18n("%1, %2").arg(invalidSources).arg(matrix->field());
        } else {
          invalidSources = matrix->field();
        }
        ++invalid;
      } else {
        if (_duplicateSelected->isChecked()) {
          // block matrix updates until matrix is setup properly
          KST::matrixList.lock().writeLock();

          // create a new matrix
          KstRMatrixPtr newMatrix = matrix->makeDuplicate();
          if (!oldSources.contains(newMatrix->dataSource())) {
            oldSources << newMatrix->dataSource();
          }
          newMatrix->changeFile(file);

          KST::matrixList.lock().unlock();

          // duplicate dependents
          if (_duplicateDependents->isChecked()) {
            duplicatedMatrices.insert(KstMatrixPtr(matrix), KstMatrixPtr(newMatrix));
            KST::duplicateDependents(KstMatrixPtr(matrix), duplicatedMap, duplicatedMatrices);
          }
        } else {
          if (!oldSources.contains(matrix->dataSource())) {
            oldSources << matrix->dataSource();
          }
          matrix->changeFile(file);
        }
      }
      matrix->unlock();
      app->slotUpdateProgress(selected, ++handled, i18n("Updating matrices..."));
    }
  }

  app->slotUpdateProgress(0, 0, QString::null);
  file = 0L;

  // now add any curves and images to plots if they were duplicated
  if (_duplicateSelected->isChecked() && _duplicateDependents->isChecked()) { 
    KstApp *app = KstApp::inst();
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
  
    windows = app->subWindowList( CreationOrder );

    for (i = windows.constBegin(); i != windows.constEnd(); ++i)
      KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
        KstTopLevelViewPtr view = kst_cast<KstTopLevelView>(viewWindow->view());
        if (view) {
          Kst2DPlotList plots = view->findChildrenType<Kst2DPlot>(true);

          for (Kst2DPlotList::Iterator plotIter = plots.begin(); plotIter != plots.end(); ++plotIter) {
            for (KstDataObjectDataObjectMap::ConstIterator iter = duplicatedMap.begin(); iter != duplicatedMap.end(); ++iter) {
              if (KstBaseCurvePtr curve = kst_cast<KstBaseCurve>(iter.data())) {
                if ((*plotIter)->Curves.contains(kst_cast<KstBaseCurve>(iter.key())) && !(*plotIter)->Curves.contains(kst_cast<KstBaseCurve>(curve))) {
                  (*plotIter)->addCurve(curve);
                }
              }
            }
          }
        }
      }
    }
  }

  //
  // clean up unused data sources
  //

  KST::dataSourceList.lock().writeLock();
  for (KstDataSourceList::Iterator it = oldSources.begin(); it != oldSources.end(); ++it) {
    if ((*it)->getUsage() == 1) {
      KST::dataSourceList.remove((*it).data());
    }
  }
  KST::dataSourceList.lock().unlock();

  if (!invalidSources.isEmpty()) {
    if (invalid == 1) {
      QMessageBox::warning(this, i18n("this"), i18n("The following field is not defined for the requested file:\n%1").arg(invalidSources));
    } else {
      QMessageBox::warning(this, i18n("this"), i18n("The following fields are not defined for the requested file:\n%1").arg(invalidSources));
    }
  }

  emit docChanged();

  //
  // force an update in case we're in paused mode
  //

  KstApp::inst()->forceUpdate();

  return true;
}


void KstChangeFileDialog::allFromFile() {
  if (_files->count() > 0) {
    ChangeFileCurveList->selectAll(false);
    KstReadLocker rl(&KST::vectorList.lock());
    uint i;

    for (i = 0; i < KST::vectorList.count(); ++i) {
      KstRVectorPtr v; 

      v = kst_cast<KstRVector>(*KST::vectorList.findTag(ChangeFileCurveList->text(i)));
      ChangeFileCurveList->setSelected(i, v && v->filename() == _files->currentText());
    }

    for (i = KST::vectorList.count(); i < ChangeFileCurveList->count(); i++) {
      KstRMatrixPtr m;

      m = kst_cast<KstRMatrix>(*KST::matrixList.findTag(ChangeFileCurveList->text(i)));
      ChangeFileCurveList->setSelected(i, m && m->filename() == _files->currentText());
    }
  }
}


void KstChangeFileDialog::updateSelection(const QString& txt) {
  ChangeFileCurveList->selectAll(false);
  QRegExp re(txt, true, true);
  uint i;

  for (i = 0; i < ChangeFileCurveList->count(); ++i) {
    ChangeFileCurveList->setSelected(i, re.exactMatch(ChangeFileCurveList->text(i)));
  }
}

#include "kstchangefiledialog.moc"
