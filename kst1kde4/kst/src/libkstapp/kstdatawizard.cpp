/***************************************************************************
                    kstdatawizard.cpp  -  Part of KST
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 The University of British Columbia
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
#include <psversion.h>
#include <sysinfo.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qdeepcopy.h>
#include <qglobal.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qmessagebox.h>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <kurlcompletion.h>

#include "kst.h"
#include "datarangewidget.h"
#include "defaultprimitivenames.h"
#include "fftoptionswidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog.h"
#include "kstdatacollection.h"
#include "kstdataobject.h"
#include "kstdataobjectcollection.h"
#include "kstdatasource.h"
#include "kstdatawizard.h"
#include "kstplotlabel.h"
#include "kstpsd.h"
#include "kstrvector.h"
#include "kstuinames.h"
#include "kstsettings.h"
#include "kstvectordefaults.h"
#include "vectorselector.h"

const QString& KstDataWizard::defaultTag = KGlobal::staticQString("<Auto Name>");

KstDataWizard::KstDataWizard(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) :
DataWizard(parent, name, modal, fl )
{
  _configWidget = 0L;
  _inTest = false;
  _hierarchy = false;
  KST::vectorDefaults.sync();
  QString default_source = KST::vectorDefaults.dataSource();
  _url->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
  setAppropriate(_pageFilters, false);
  setIcon(BarIcon("kst_datawizard"));

  _kstDataRange->update();
  _kstFFTOptions->update();

  _newWindowName->setText(defaultTag);

  setNextEnabled(_pageDataSource, false);
  setNextEnabled(_pageVectors, false);
  setNextEnabled(_pageFilters, true);
  setFinishEnabled(_pagePlot, true);

  disconnect(finishButton(), SIGNAL(clicked()), (QDialog*)this, SLOT(accept()));
  connect(finishButton(), SIGNAL(clicked()), this, SLOT(finished()));
  _vectors->setAcceptDrops(true);
  _vectorsToPlot->setAcceptDrops(true);
  _vectors->addColumn(i18n("Position"));
  _vectors->setSorting(1);
  _vectorsToPlot->setSorting(-1);

  connect(_url, SIGNAL(openFileDialog(KURLRequester *)), this, SLOT(selectFolder()));
  connect(_cycleThrough, SIGNAL(toggled(bool)), _plotNumber, SLOT(setEnabled(bool)));
  connect(_existingPlot, SIGNAL(toggled(bool)), _existingPlotName, SLOT(setEnabled(bool)));
  connect(_existingWindow, SIGNAL(toggled(bool)), _windowName, SLOT(setEnabled(bool)));
  connect(_newWindow, SIGNAL(toggled(bool)), this, SLOT(updatePlotBox()));
  connect(_currentWindow, SIGNAL(toggled(bool)), this, SLOT(updatePlotBox()));
  connect(_existingWindow, SIGNAL(toggled(bool)), this, SLOT(updatePlotBox()));
  connect(_windowName, SIGNAL(activated(int)), this, SLOT(updatePlotBox()));
  connect(_applyFilters, SIGNAL(toggled(bool)), this, SLOT(applyFiltersChecked(bool)));
  connect(_newFilter, SIGNAL(clicked()), this, SLOT(newFilter()));
  connect(_radioButtonPlotData, SIGNAL(clicked()), this, SLOT(disablePSDEntries()));
  connect(_radioButtonPlotData, SIGNAL(clicked()), this, SLOT(enableXEntries()));
  connect(_radioButtonPlotPSD, SIGNAL(clicked()), this, SLOT(enablePSDEntries()));
  connect(_radioButtonPlotPSD, SIGNAL(clicked()), this, SLOT(disableXEntries()));
  connect(_radioButtonPlotDataPSD, SIGNAL(clicked()), this, SLOT(enablePSDEntries()));
  connect(_radioButtonPlotDataPSD, SIGNAL(clicked()), this, SLOT(enableXEntries()));
  connect(_newWindows, SIGNAL(toggled(bool)), this, SLOT(updatePlotBox()));
  connect(_plotGroup, SIGNAL(clicked(int)), this, SLOT(updateColumns()));
  connect(_windowGroup, SIGNAL(clicked(int)), this, SLOT(updateColumns()));
  connect(_url, SIGNAL(textChanged(const QString&)), this, SLOT(sourceChanged(const QString&)));
  connect(_configureSource, SIGNAL(clicked()), this, SLOT(configureSource()));
  connect(_plotColumns, SIGNAL(valueChanged(int)), this, SLOT(plotColsChanged()));
  connect(_vectorReduction, SIGNAL(textChanged(const QString&)), this, SLOT(vectorSubset(const QString&)));
  connect(_vectorSearch, SIGNAL(clicked()), this, SLOT(search()));
  connect(_vectors, SIGNAL(pressed(QListViewItem*)), this, SLOT(fieldListChanged()));
  connect(_add, SIGNAL(clicked()), this, SLOT(add()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(_up, SIGNAL(clicked()), this, SLOT(up()));
  connect(_down, SIGNAL(clicked()), this, SLOT(down()));
  connect(_vectors, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(add()));
  connect(_vectorsToPlot, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(remove()));
  connect(_vectors, SIGNAL(dropped(QDropEvent*)), this, SLOT(vectorsDroppedBack(QDropEvent*)));
  connect(_vectorsToPlot, SIGNAL(dropped(QDropEvent*)), this, SLOT(updateVectorPageButtons()));
  connect(_testURL, SIGNAL(clicked()), this, SLOT(testURL()));

  // No help button
  setHelpEnabled(_pageDataSource, false);
  setHelpEnabled(_pageVectors, false);
  setHelpEnabled(_pageFilters, false);
  setHelpEnabled(_pagePlot, false);
  _newFilter->setEnabled(false);
  _newFilter->hide(); // FIXME: implement this
  _testURL->hide();
  _url->setURL(default_source);
  _url->completionObject()->setDir(QDir::currentPath());
  _url->setFocus();

  // x vector selection
  connect(_xAxisCreateFromField, SIGNAL(toggled(bool)), _xVector, SLOT(setEnabled(bool)));
  connect(_xAxisUseExisting, SIGNAL(toggled(bool)), _xVectorExisting, SLOT(setEnabled(bool)));
  connect(_xAxisCreateFromField, SIGNAL(clicked()), this, SLOT(xChanged()));
  connect(_xAxisUseExisting, SIGNAL(clicked()), this, SLOT(xChanged()));

  _xAxisCreateFromField->setChecked(true);
  _xVectorExisting->setEnabled(false);
  _xVectorExisting->_newVector->hide();
  _xVectorExisting->_editVector->hide();

  _up->setPixmap(BarIcon("up"));
  _up->setAccel(ALT+Key_Up);

  _down->setPixmap(BarIcon("down"));
  _down->setAccel(ALT+Key_Down);

  _add->setPixmap(BarIcon("forward"));
  _add->setAccel(ALT+Key_S);

  _remove->setPixmap(BarIcon("back"));
  _remove->setAccel(ALT+Key_R);

  _plotColumns->setMinValue(0);
  _plotColumns->setMaxValue(10);
  _plotColumns->setSpecialValueText(i18n("default"));

  loadSettings();

  QToolTip::add(_up, i18n("Raise in plot order: Alt+Up"));
  QToolTip::add(_down, i18n("Lower in plot order: Alt+Down"));
  QToolTip::add(_add, i18n("Select: Alt+s"));
  QToolTip::add(_remove, i18n("Remove: Alt+r"));
}


KstDataWizard::~KstDataWizard()
{
  delete _configWidget;
}


void KstDataWizard::selectingFolder()
{
  QString strFolder = _url->url();
  KFileDialog *fileDlg = _url->fileDialog();
  QFileInfo fileInfo(strFolder);

  if (fileDlg) {
    if (fileInfo.isDir()) {
      QDir dir(strFolder);

      if (dir.cdUp()) {
        fileDlg->setURL(KUrl(dir.absPath()));
      }
    }
  }
}


void KstDataWizard::selectFolder()
{
  QTimer::singleShot(0, this, SLOT(selectingFolder()));
}


void KstDataWizard::setInput(const QString& input)
{
  _url->setURL(input);
}


void KstDataWizard::plotColsChanged()
{
  _reGrid->setChecked(true);
}


bool KstDataWizard::xVectorOk()
{
  if (!_xAxisGroup->isEnabled()) {
    return true;
  }

  if (_xAxisCreateFromField->isChecked()) {
    QString txt = _xVector->currentText();
    for (int i = 0; i < _xVector->count(); ++i) {
      if (_xVector->text(i) == txt) {
        return true;
      }
    }
    return false;
  } else {
    return (KST::vectorList.findTag(_xVectorExisting->selectedVector()) != KST::vectorList.end());
  }
}


bool KstDataWizard::yVectorsOk()
{
  return _vectorsToPlot->childCount() > 0;
}


void KstDataWizard::xChanged()
{
  setNextEnabled(_pageVectors, xVectorOk() && yVectorsOk());
}


void KstDataWizard::testURL()
{
  _inTest = true;
  sourceChanged(_url->url());
  _inTest = false;
}


void KstDataWizard::sourceChanged(const QString& text)
{
  delete _configWidget;
  _configWidget = 0L;
  _configureSource->setEnabled(false);
  _file = QString::null;
  if (!text.isEmpty() && text != "stdin" && text != "-") {
    KUrl url;

    QString txt = _url->completionObject()->replacedPath(text);
    if (QFile::exists(txt) && QFileInfo(txt).isRelative()) {
      url.setPath(txt);
    } else {
      url = KUrl::fromPathOrURL(txt);
    }

    if (!_inTest && !url.isLocalFile() && url.protocol() != "file" && !url.protocol().isEmpty()) {
      setNextEnabled(_pageDataSource, false);
      _fileType->setText(QString::null);
      _testURL->show();
      return;
    }

    if (!_inTest) {
      _testURL->hide();
    }

    if (!url.isValid()) {
      setNextEnabled(_pageDataSource, false);
      _fileType->setText(QString::null);
      return;
    }

    QString file = txt;

    KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(file);
    QStringList fl;
    bool complete = false;
    QString fileType;
    int index = 0;
    int count;

    _hierarchy = false;

    if (ds) {
      ds->readLock();
      fl = ds->fieldList();
      fileType = ds->fileType();
      complete = ds->fieldListIsComplete();
      _hierarchy = ds->supportsHierarchy();
      ds->unlock();
      ds = 0L;
    } else {
      fl = KstDataSource::fieldListForSource(file, QString::null, &fileType, &complete);
      _hierarchy = KstDataSource::supportsHierarchy(file, QString::null);
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

    _fileType->setText(fileType.isEmpty() ? QString::null : i18n("Data source of type: %1").arg(fileType));

    if (fl.isEmpty()) {
      setNextEnabled(_pageDataSource, false);
      return;
    }

    _vectors->clear();
    _vectorsToPlot->clear();
    _xVector->clear();

    _xVector->insertStringList(fl);
    _xVector->completionObject()->insertItems(fl);
    _xVector->setEditable(!complete);

    count = fl.count();
    if (count > 0) {
      count = 1 + int(log10(double(count)));
    } else {
      count = 1;
    }

    _fields.clear();

    if (_hierarchy) {
      for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
        QStringList     entries = QStringList::split(QDir::separator(), (*it), FALSE);
        QString         item;
        QListViewItem*  parent = 0L;
        QListViewItem*  parentOld = 0L;

        for (QStringList::ConstIterator itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
          item += (*itEntry);

          if (item.compare(*it) != 0) {
            parent = _fields.find(item);
            if (parent == 0L) {
              if (parentOld) {
                QListViewItem *listItem = new QListViewItem(parentOld, *itEntry);

                parentOld->setOpen(true);
                _fields.insert(item, listItem);
                parentOld = listItem;
              } else {
                QListViewItem *listItem = new QListViewItem(_vectors, *itEntry);

                _fields.insert(item, listItem);
                parentOld = listItem;
              }
            } else {
              parentOld = parent;
            }

            item += QDir::separator();
          } else if (parentOld) {
            QListViewItem *listItem = new QListViewItem(parentOld, *itEntry);

            parentOld->setOpen(true);
            _fields.insert(item, listItem);
          } else {
            QListViewItem *listItem = new QListViewItem(_vectors, *itEntry);

            _fields.insert(item, listItem);
          }
        }
      }
    } else {
      for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
        QListViewItem *item = new QListViewItem(_vectors, *it);
        QString str;

        item->setDragEnabled(true);
        str.sprintf("%0*d", count, index++);
        item->setText(1, str);
        _countMap[*it] = str;
      }
    }

    _vectors->sort();

    KST::vectorDefaults.sync();
    QString defaultX = KST::vectorDefaults.wizardXVector();
    if (fl.contains(defaultX)) {
      _xVector->setCurrentText(defaultX);
    } else {
      _xVector->setCurrentItem(0);
    }
    _file = file;
    // we probably don't want to use an
    //existing vector since the data source just changed...
    _xAxisCreateFromField->setChecked(true); 
  } else {
    _fileType->setText(QString::null);
    setNextEnabled(_pageDataSource, false);
    return;
  }

  setNextEnabled(_pageVectors, xVectorOk() && yVectorsOk());
  setNextEnabled(_pageDataSource, true);
  if (_inTest) {
    _testURL->hide();
  }
}


void KstDataWizard::fieldListChanged()
{
  bool ok = yVectorsOk();
  setNextEnabled(_pageVectors, ok && xVectorOk());
}


void KstDataWizard::showPage( QWidget *page )
{
  if (page == _pageVectors) {
    KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);
    if (!ds) {
      for (KstDataSourceList::Iterator i = _sourceCache.begin(); i != _sourceCache.end(); ++i) {
        if ((*i)->fileName() == _file) {
          ds = *i;
          break;
        }
      }

      if (!ds) {
        ds = KstDataSource::loadSource(_file);
        if (ds) {
          _sourceCache.append(ds);
        }
      }
    }
    if (ds) {
      ds->readLock();
    }
    _kstDataRange->setAllowTime(ds && ds->supportsTimeConversions());
    if (ds) {
      ds->unlock();
    }
    _vectorReduction->setFocus();
  } else if (page == _pageFilters) {
    QString save = _filterList->currentText();
    _filterList->clear();
  } else if (page == _pagePlot) {
    if (_xAxisCreateFromField->isChecked()) {
      KST::vectorDefaults.setWizardXVector(_xVector->currentText());
      KST::vectorDefaults.sync();
    }

    // count the vectors we are about to make, so we can guess defaults
    int nCurves = _vectorsToPlot->childCount();

    _psdAxisGroup->setEnabled(_radioButtonPlotDataPSD->isChecked() || _radioButtonPlotPSD->isChecked());

    // set window and plot defaults based on some guesses.
    if (_radioButtonPlotDataPSD->isChecked()) { // plotting both psds and XY plots
      _newWindows->setEnabled(true);
      if (nCurves > 1) {
        _newWindows->setChecked(true);
      } else {
        _newWindow->setChecked(true);
      }
    } else { // just plotting XY or PSD
      _newWindows->setEnabled(false);
      _currentWindow->setChecked(true);
    }
    _multiplePlots->setChecked(true);

    updateWindowBox();
    updatePlotBox();
    updateColumns();
  }

  QWizard::showPage(page);
}


void KstDataWizard::updateWindowBox()
{
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();

  _windowName->clear();
  while (it->currentItem()) {
    KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (v) {
      _windowName->insertItem(v->caption());
    }
    it->next();
  }
  app->deleteIterator(it);

  _existingWindow->setEnabled(_windowName->count() > 0);
  _currentWindow->setEnabled(_windowName->count() > 0 && KstApp::inst()->activeWindow());

  if (!_windowGroup->selected() || !_windowGroup->selected()->isEnabled()) {
    _newWindow->setChecked(true);
  }
}


void KstDataWizard::updateColumns()
{
  if (_newWindow->isChecked() || _newWindows->isChecked()) {
    _reGrid->setChecked(false);

    return;
  }

  KstViewWindow *v;

  if (_currentWindow->isChecked()) {
    v = static_cast<KstViewWindow*>(KstApp::inst()->activeWindow());
  } else {
    v = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(_windowName->currentText()));
  }

  if (v) {
    if (v->view()->onGrid()) {
      _reGrid->setChecked(true);
    } else {
      _reGrid->setChecked(false);
    }
  }
}


void KstDataWizard::updatePlotBox()
{
  QString psave = _existingPlotName->currentText();
  KstApp *app = KstApp::inst();

  _existingPlotName->clear();

  if (_newWindow->isChecked() || _newWindows->isChecked()) {
    _onePlot->setEnabled(true);
    _multiplePlots->setEnabled(true);
    _cycleThrough->setEnabled(true);
    _plotNumber->setEnabled(_cycleThrough->isChecked());
    _cycleExisting->setEnabled(false);
    _existingPlot->setEnabled(false);
    _existingPlotName->setEnabled(false);
  } else {
    KstViewWindow *v;
    if (_currentWindow->isChecked()) {
      v = static_cast<KstViewWindow*>(app->activeWindow());
    } else {
      v = static_cast<KstViewWindow*>(app->findWindow(_windowName->currentText()));
    }

    Kst2DPlotList plots;
    if (v) {
      plots += v->view()->findChildrenType<Kst2DPlot>();
    }

    for (Kst2DPlotList::ConstIterator i = plots.begin(); i != plots.end(); ++i) {
      _existingPlotName->insertItem((*i)->tagName());
    }

    bool havePlots = _existingPlotName->count() > 0;
    _onePlot->setEnabled(true);
    _multiplePlots->setEnabled(true);
    _cycleThrough->setEnabled(true);
    _plotNumber->setEnabled(_cycleThrough->isChecked());
    _cycleExisting->setEnabled(havePlots);
    _existingPlot->setEnabled(havePlots);
    _existingPlotName->setEnabled(havePlots && _existingPlot->isChecked());
  }

  if (!_plotGroup->selected()->isEnabled()) {
    _onePlot->setChecked(true);
  }

  if (_existingPlot->isEnabled() && _existingPlotName->listBox() && _existingPlotName->listBox()->findItem(psave)) {
    _existingPlotName->setCurrentText(psave);
  }
}


void KstDataWizard::vectorSubset(const QString& filter)
{
  QListViewItem *after = 0L;
  _vectors->clearSelection();
  _vectors->setSorting(3, true); // Qt 3.1 compat
  QRegExp re(filter, true /* case insensitive */, true /* wildcard */);
  QListViewItemIterator it(_vectors);

  while (it.current()) {
    QListViewItem *i = it.current();
    ++it;
    if (re.exactMatch(i->text(0))) {
      if (!_hierarchy) {
        if (!after) {
          _vectors->takeItem(i);
          _vectors->insertItem(i);
        } else {
          i->moveItem(after);
        }
        after = i;
      }
      i->setSelected(true);
    }
  }
}


void KstDataWizard::newFilter()
{
}


bool KstDataWizard::checkAvailableMemory(KstDataSourcePtr &ds, KstFrameSize f0Value, Q_INT64 nValue)
{
  unsigned long memoryRequested = 0;
  unsigned long memoryAvailable = 1024*1024*1024; // 1GB
  Q_INT64 frames;
  bool rc = true;

#ifdef HAVE_LINUX
  KST::memfree(memoryAvailable, true);
  KST::memfree(memoryAvailable, true);
#endif

  ds->writeLock();

  //
  // only add to memory requirement if xVector is to be created...
  //

  if (_xAxisCreateFromField->isChecked()) {
    if (_kstDataRange->ReadToEnd->isChecked() || nValue < 0) {
      frames = ds->frameCountLarge(_xVector->currentText()) - f0Value;
    } else {
      if (nValue < ds->frameCountLarge(_xVector->currentText())) {
        frames = nValue;
      } else {
        frames = ds->frameCountLarge(_xVector->currentText());
      }
    }

    if (_kstDataRange->DoSkip->isChecked() && _kstDataRange->Skip->value() > 0) {
      memoryRequested += frames / _kstDataRange->Skip->value() * sizeof(double);
    } else {
      memoryRequested += frames * ds->samplesPerFrame(_xVector->currentText())*sizeof(double);
    }
  }

  //
  // memory estimate for the y vectors
  //
  {
    QListViewItemIterator it(_vectorsToPlot);
    unsigned long memoryForVector = 0;
    int fftLen = -1;

    if (_kstFFTOptions->Interleaved->isChecked()) {
       fftLen = int(pow(2.0, double(_kstFFTOptions->FFTLen->text().toInt() - 1)));
    }

    while (it.current()) {
      QListViewItem *i = it.current();
      QString field = i->text(0);

      if (_kstDataRange->ReadToEnd->isChecked() || nValue < 0) {
        frames = ds->frameCountLarge(field) - f0Value;
      } else {
        frames = nValue;
        if (frames > (unsigned long)ds->frameCountLarge(field)) {
          frames = ds->frameCountLarge();
        }
      }

      if (_kstDataRange->DoSkip->isChecked() && _kstDataRange->Skip->value() > 0) {
        memoryForVector = frames / _kstDataRange->Skip->value()*sizeof(double);
      } else {
        memoryForVector = frames * ds->samplesPerFrame(field)*sizeof(double);
      }
      memoryRequested += memoryForVector;

      if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
        if (_kstFFTOptions->Interleaved->isChecked()) {
          memoryRequested += fftLen * 6;
        } else {
          memoryRequested += memoryForVector * 2;
        }
      }

      ++it;
    }
  }

  ds->unlock();

  if (memoryRequested > memoryAvailable) {
    QString strMemoryRequested;
    QString strMemoryAvailable;

    memoryRequested /= 1024;
    memoryAvailable /= 1024;
    if (memoryRequested < 10 * 1024) {
      strMemoryRequested = i18n("abbreviation for kilobytes", "%1 kB").arg(memoryRequested);
      strMemoryAvailable = i18n("abbreviation for kilobytes", "%1 kB").arg(memoryAvailable);
    } else {
      memoryRequested /= 1024;
      memoryAvailable /= 1024;
      if (memoryRequested < 10 * 1024) {
        strMemoryRequested = i18n("abbreviation for megabytes", "%1 MB").arg(memoryRequested);
        strMemoryAvailable = i18n("abbreviation for megabytes", "%1 MB").arg(memoryAvailable);
      } else {
        memoryRequested /= 1024;
        memoryAvailable /= 1024;
        if (memoryRequested < 10 * 1024) {
          strMemoryRequested = i18n("abbreviation for gigabytes", "%1 GB").arg(memoryRequested);
          strMemoryAvailable = i18n("abbreviation for gigabytes", "%1 GB").arg(memoryAvailable);
        } else {
          memoryRequested /= 1024;
          memoryAvailable /= 1024;
          strMemoryRequested = i18n("abbreviation for terabytes", "%1 TB").arg(memoryRequested);
          strMemoryAvailable = i18n("abbreviation for terabytes", "%1 TB").arg(memoryAvailable);
        }
      }
    }

    if (strMemoryRequested != strMemoryAvailable) {
      QMessageBox::warning(this, i18n("Kst"), i18n("You requested to read in over %1 of data but it seems that you have approximately only %2 of usable memory available. You cannot load this much data.").arg(strMemoryRequested).arg(strMemoryAvailable));
      rc = false;
    } else {
      if (QMessageBox::warning(this, i18n("Kst"), i18n("You requested to read in approximately %1 of data but it seems that you have slightly less usable memory than this available. Would you like to try and load the data anyway?").arg(strMemoryRequested), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        rc = true;
      } else {
        rc = false;
      }
    }
  }

  return rc;
}


void KstDataWizard::createLegendsAndLabels(KstViewObjectList &plots, bool xLabels, bool yLabels, bool titleLabel, bool legend, bool legendAuto, int fontSize)
{
  KstViewObjectList::Iterator pit = plots.begin();

  while (pit != plots.end()) {
    Kst2DPlotPtr pp = kst_cast<Kst2DPlot>(*pit);
    if (!pp) {
      ++pit;

      continue;
    }
    pp->generateDefaultLabels(xLabels, yLabels, titleLabel);

    if (legend) {
      pp->getOrCreateLegend();
    } else if (legendAuto) {
      if (pp->Curves.count() > 1) {
        pp->getOrCreateLegend();
      }
    }

    pp->setPlotLabelFontSizes(fontSize);

    ++pit;
  }
}


void KstDataWizard::cleanupWindowLayout(KstViewWindow *window)
{
  if (window) {
    //
    // we want to layout the plots in a window in a grid if:
    //  the user requested it or
    //  the window is a new one (see bug report 149740)
    //
    if (_reGrid->isChecked() || _newWindow->isChecked() || _newWindows->isChecked()) {
      int cols;

      if (_plotColumns->value() == _plotColumns->minValue()) {
        const KstViewObjectList& children(window->view()->children());
        int cnt = 0;

        for (KstViewObjectList::ConstIterator i = children.begin(); i != children.end(); ++i) {
          if ((*i)->followsFlow()) {
            ++cnt;
          }
        }
        cols = int(sqrt(cnt));
      } else {
        cols = _plotColumns->value();
      }
      window->view()->cleanup(cols);
    } else if (window->view()->onGrid()) {
      window->view()->cleanup(-1);
    }

    window->view()->paint(KstPainter::P_PAINT);
  }
}

void KstDataWizard::finished()
{
  KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);

  if (!ds) {
    for (KstDataSourceList::Iterator i = _sourceCache.begin(); i != _sourceCache.end(); ++i) {
      if ((*i)->fileName() == _file) {
        ds = *i;

        break;
      }
    }
    if (!ds) {
      ds = KstDataSource::loadSource(_file);
    }
    if (!ds) {
      QMessageBox::warning(this, i18n("Kst"), i18n("<qt>Sorry, unable to load the data file '<b>%1</b>'.").arg(_file));

      return;
    }
    KST::dataSourceList.lock().writeLock();
    KST::dataSourceList.append(ds);
    KST::dataSourceList.lock().unlock();
    _sourceCache.clear();
  }

  KstFrameSize f0Value;
  KstFrameSize nValue;

  if (_kstDataRange->isStartAbsoluteTime()) {
    f0Value = ds->sampleForTimeLarge(_kstDataRange->f0DateTimeValue());
  } else if (_kstDataRange->isStartRelativeTime()) {
    f0Value = ds->sampleForTimeLarge(_kstDataRange->f0Value());
  } else {
    f0Value = (KstFrameSize)_kstDataRange->f0Value();
  }

  if (_kstDataRange->isRangeRelativeTime()) {
    double nValStored = _kstDataRange->nValue();

    if (_kstDataRange->CountFromEnd->isChecked()) {
      KstFrameSize frameCount = ds->frameCountLarge(_xVector->currentText());
      double msCount = ds->relativeTimeForSampleLarge(frameCount - 1);

      nValue = frameCount - 1 - ds->sampleForTimeLarge(msCount - nValStored);
    } else {
      double fTime = ds->relativeTimeForSampleLarge(f0Value);

      nValue = ds->sampleForTimeLarge(fTime + nValStored) - ds->sampleForTimeLarge(fTime);
    }
  } else {
    nValue = (KstFrameSize)_kstDataRange->nValue();
  }

  if (checkAvailableMemory(ds, f0Value, nValue)) {
    KstApp *app = KstApp::inst();
    KstVectorList vectorList;
    QString name = KST::suggestVectorName(_xVector->currentText());
    QValueList<QColor> colors;
    QColor color;
    KstVectorPtr xVector;
    uint nCurves = 0;
    uint nSteps = 0;
    int progress = 0;
    int fontSize;
    int pointType = 0;

    accept();

    //
    // pause updates to avoid event storms
    //
    bool wasPaused = app->paused();
    if (!wasPaused) {
      app->setPaused(true);
    }

    nSteps += _vectorsToPlot->childCount();
    if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
      nSteps += _vectorsToPlot->childCount();
    }

    // only create x vector if needed
    if (_xAxisCreateFromField->isChecked()) {
      nSteps += 1; // for the creation of the x-vector
      progress = 0;
      app->slotUpdateProgress(nSteps, progress, i18n("Creating vectors..."));

      // create the x-vector
      xVector = new KstRVector(ds, _xVector->currentText(),
              KstObjectTag(name, ds->tag(), false),
              _kstDataRange->CountFromEnd->isChecked() ? -1 : f0Value,
              _kstDataRange->ReadToEnd->isChecked() ? -1 : nValue,
              _kstDataRange->DoSkip->isChecked() ? _kstDataRange->Skip->value() : 0, 
              _kstDataRange->DoSkip->isChecked(),
              _kstDataRange->DoFilter->isChecked());

      app->slotUpdateProgress(nSteps, ++progress, i18n("Creating vectors..."));
    } else {
      xVector = *(KST::vectorList.findTag(_xVectorExisting->selectedVector()));
      app->slotUpdateProgress(nSteps, progress, i18n("Creating vectors..."));
    }

    // next time we use the wizard this session, we probably will want to use the
    // same X vector, so... lets set that as the default.
    _xAxisUseExisting->setChecked(true);
    _xVectorExisting->update();
    _xVectorExisting->setSelection(xVector->tag().displayString());

    // create the y-vectors
    {
      QListViewItemIterator it(_vectorsToPlot);
      while (it.current()) {
        QListViewItem *i = it.current();
        name = KST::suggestVectorName(i->text(0));

        KstVectorPtr v = new KstRVector(ds, i->text(0),
                KstObjectTag(name, ds->tag(), false),
                _kstDataRange->CountFromEnd->isChecked() ? -1 : f0Value,
                _kstDataRange->ReadToEnd->isChecked() ? -1 : nValue,
                _kstDataRange->DoSkip->isChecked() ? _kstDataRange->Skip->value() : 0, 
                _kstDataRange->DoSkip->isChecked(),
                _kstDataRange->DoFilter->isChecked());
        vectorList.append(v);
        ++nCurves;
        app->slotUpdateProgress(nSteps, ++progress, i18n("Creating vectors..."));
        ++it;
      }
    }

    // get a pointer to the first window
    QString newName;
    KstViewWindow *window = 0L;
    KstViewWindow *windowPSD = 0L;

    if (_newWindow->isChecked() || _newWindows->isChecked()) {
      window = dynamic_cast<KstViewWindow*>(app->activeWindow());
      if (window && window->view()->children().isEmpty()) {
        //
        // use the existing view
        //
        newName = window->caption();
        if (newName.isEmpty()) {
          newName = window->tabCaption();
        }
      } else {
        newName = _newWindowName->text();
        if (newName == defaultTag) {
          newName = KST::suggestWinName();
        }
        window = static_cast<KstViewWindow*>(app->findWindow(app->newWindow(newName)));
      }

      if (_radioButtonPlotDataPSD->isChecked()) {
        if (_newWindows->isChecked()) {
          newName += i18n("-Spectra");
          QString n = app->newWindow(newName);
          windowPSD = static_cast<KstViewWindow*>(app->findWindow(n));
        }
      }
    } else if (_currentWindow->isChecked()) {
      window = static_cast<KstViewWindow*>(app->activeWindow());
    } else {
      window = static_cast<KstViewWindow*>(app->findWindow(_windowName->currentText()));
    }

    if (!window) {
      if (!wasPaused) {
        app->setPaused(false);
      }
      return;
    }

    fontSize = qRound(getFontSize(window));

    // create the necessary plots
    app->slotUpdateProgress(nSteps, progress, i18n("Creating plots..."));
    KstViewObjectList plots;

    if (_onePlot->isChecked()) {
      Kst2DPlotPtr p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
      plots.append(p.data());
      if (_radioButtonPlotDataPSD->isChecked()) {
        Kst2DPlotPtr p;

        if (windowPSD) {
          p = kst_cast<Kst2DPlot>(windowPSD->view()->findChild(windowPSD->createPlotObject(KST::suggestPlotName(), false)));
        } else {
           p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
        }
        plots.append(p.data());
        p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
        p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
      }
    } else if (_multiplePlots->isChecked()) {
      Kst2DPlotPtr p;

      for (uint i = 0; i < vectorList.count(); ++i) {
        p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
        plots.append(p.data());
      }
      if (_radioButtonPlotDataPSD->isChecked()) {
        for (uint i = 0; i < vectorList.count(); ++i) {
          if (windowPSD) {
            p = kst_cast<Kst2DPlot>(windowPSD->view()->findChild(windowPSD->createPlotObject(KST::suggestPlotName(), false)));
          } else {
            p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
          }
          plots.append(p.data());
          p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
          p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
        }
      }
    } else if (_existingPlot->isChecked()) {
      Kst2DPlotPtr p = kst_cast<Kst2DPlot>(window->view()->findChild(_existingPlotName->currentText()));
      plots.append(p.data());
    } else if (_cycleExisting->isChecked()) {
      Kst2DPlotList pl = QDeepCopy<Kst2DPlotList>(window->view()->findChildrenType<Kst2DPlot>());
      for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
        plots += (*i).data();
      }
    } else { /* cycle */
      Kst2DPlotPtr p;
      for (int i = 0; i < _plotNumber->value(); ++i) {
        p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
        plots.append(p.data());
      }
      if (_radioButtonPlotDataPSD->isChecked()) {
        for (int i = 0; i < _plotNumber->value(); ++i) {
          if (windowPSD) {
            p = kst_cast<Kst2DPlot>(windowPSD->view()->findChild(windowPSD->createPlotObject(KST::suggestPlotName(), false)));
          } else {
            p = kst_cast<Kst2DPlot>(window->view()->findChild(window->createPlotObject(KST::suggestPlotName(), false)));
          }
          plots.append(p.data());
          p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
          p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
        }
      }
    }

    // re-order the vectors if the user wants it
    if (_orderInColumns->isChecked()) {
      const KstVectorList lOld = vectorList;
      const int count = lOld.count();
      const int cols = signed(sqrt(plots.count()));
      const int rows = cols + (count - cols * cols) / cols;
      int overflow = count % cols;
      int row = 0, col = 0;

      for (int i = 0; i < count; ++i) {
        vectorList[row * cols + col] = lOld[i];
        ++row;
        if (row >= rows) {
          if (overflow > 0) {
            --overflow;
          } else {
            ++col;
            row = 0;
          }
        }
      }
    }

    // create the data curves
    app->slotUpdateProgress(nSteps, progress, i18n("Creating curves..."));
    KstViewObjectList::Iterator pit = plots.begin();
    for (KstVectorList::Iterator it = vectorList.begin(); it != vectorList.end(); ++it) {
      if (_radioButtonPlotData->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
        name = KST::suggestCurveName((*it)->tag(), false);
        Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(*pit);
        color = KstApp::inst()->chooseColorDlg()->getColorForCurve(xVector, *it);
        if (!color.isValid()) {
          if (plot) {
            KstVCurveList vcurves = kstObjectSubList<KstBaseCurve,KstVCurve>(plot->Curves);
            color = KstColorSequence::next(vcurves, plot->backgroundColor());
          } else {
            color = KstColorSequence::next();
          }
        }
        colors.append(color);
        KstVCurvePtr c = new KstVCurve(name, xVector, *it, 0L, 0L, 0L, 0L, color);
        c->setLineWidth(KstSettings::globalSettings()->defaultLineWeight);
        if (_drawBoth->isChecked()) {
          c->setHasPoints(true);
          c->setPointStyle(pointType++ % KSTPOINT_MAXTYPE);
          c->setHasLines(true);
        } else if (_drawLines->isChecked()) {
          c->setHasPoints(false);
          c->setHasLines(true);
        } else {
          c->setHasPoints(true);
          c->setPointStyle(pointType++ % KSTPOINT_MAXTYPE);
          c->setHasLines(false);
        }
        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(KstDataObjectPtr(c));
        KST::dataObjectList.lock().unlock();
        if (plot) {
          plot->addCurve(KstBaseCurvePtr(c));
        }
        if (!_onePlot->isChecked()) { // change plots if we are not onePlot
          if (_radioButtonPlotDataPSD->isChecked()) { // if xy and psd
            ++pit;
            if (plots.findIndex(*pit) >= (int)plots.count()/2) {
              pit = plots.begin();
            }
          } else if (++pit == plots.end()) {
            pit = plots.begin();
          }
        }
      }
    }

    if (_onePlot->isChecked()) {
      // if we are one plot, now we can move to the psd plot
      if (++pit == plots.end()) {
        // if _newWindows is not checked, there will not be another.
        pit = plots.begin();
      }
    } else if (_radioButtonPlotDataPSD->isChecked()) {
      pit = plots.at(plots.count()/2);
    }

    // create the PSDs
    if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
      KstVCurvePtr c;
      int indexColor = 0;

      pointType = 0;
      app->slotUpdateProgress(nSteps, progress, i18n("Creating spectra..."));

      for (KstVectorList::Iterator it = vectorList.begin(); it != vectorList.end(); ++it) {
        if ((*it)->length() > 0) {
          Kst2DPlotPtr plot;
          KstViewObjectList::Iterator startPlot = pit;

          while (!plot) {
            plot = kst_cast<Kst2DPlot>(*pit);
            if (!plot) {
              if (++pit == plots.end()) {
                pit = plots.begin();
              }
              // If this is ever false, we have no valid 2D plots
              // which means that someone wrote some incomplete code
              // or something is really broken
              assert(pit != startPlot);
            }
          }
          name = KST::suggestPSDName((*it)->tag());

          KstPSDPtr p = new KstPSD(name, *it,
                  _kstFFTOptions->SampRate->text().toDouble(),
                  _kstFFTOptions->Interleaved->isChecked(),
                  _kstFFTOptions->FFTLen->text().toInt(),
                  _kstFFTOptions->Apodize->isChecked(),
                  _kstFFTOptions->RemoveMean->isChecked(),
                  _kstFFTOptions->VectorUnits->text(),
                  _kstFFTOptions->RateUnits->text(), 
                  ApodizeFunction(_kstFFTOptions->ApodizeFxn->currentItem()),
                  _kstFFTOptions->Sigma->value(),
                  PSDType(_kstFFTOptions->Output->currentItem()));
          p->setInterpolateHoles(_kstFFTOptions->InterpolateHoles->isChecked());
          color = KstApp::inst()->chooseColorDlg()->getColorForCurve(p->vX(), p->vY());
          if (!color.isValid()) {
            if (_radioButtonPlotPSD->isChecked() || colors.count() <= (unsigned long)indexColor) {
              KstVCurveList vcurves = kstObjectSubList<KstBaseCurve,KstVCurve>(plot->Curves);
              color = KstColorSequence::next(vcurves, plot->backgroundColor());
            } else {
              color = colors[indexColor];
              indexColor++;
            }
          }
          c = new KstVCurve(KST::suggestCurveName(p->tag(), true), p->vX(), p->vY(), 0L, 0L, 0L, 0L, color);
          c->setLineWidth(KstSettings::globalSettings()->defaultLineWeight);
          if (_drawBoth->isChecked()) {
            c->setHasPoints(true);
            c->setPointStyle(pointType++ % KSTPOINT_MAXTYPE);
            c->setHasLines(true);
          } else if (_drawLines->isChecked()) {
            c->setHasPoints(false);
            c->setHasLines(true);
          } else {
            c->setHasPoints(true);
            c->setPointStyle(pointType++ % KSTPOINT_MAXTYPE);
            c->setHasLines(false);
          }
          KST::dataObjectList.lock().writeLock();
          KST::dataObjectList.append(KstDataObjectPtr(p));
          KST::dataObjectList.append(KstDataObjectPtr(c));
          KST::dataObjectList.lock().unlock();
          plot->addCurve(c.data());
          plot->setLog(_psdLogX->isChecked(),_psdLogY->isChecked());
          if (!_onePlot->isChecked()) { // change plots if we are not onePlot
            if (++pit == plots.end()) {
              if (_radioButtonPlotDataPSD->isChecked()) { // if xy and psd
                pit = plots.at(plots.count()/2);
              } else {
                pit = plots.begin();
              }
            }
          }
        }
      }
      app->slotUpdateProgress(nSteps, ++progress, i18n("Creating spectra..."));
    }

    createLegendsAndLabels(plots, _xAxisLabels->isChecked(), _yAxisLabels->isChecked(), _plotTitles->isChecked(), _legendsOn->isChecked(), _legendsAuto->isChecked(), fontSize);

    cleanupWindowLayout(window);
    cleanupWindowLayout(windowPSD);

    app->slotUpdateProgress(0, 0, QString::null);
    if (!wasPaused) {
      app->setPaused(false);
    }

    saveSettings();
  }
}


void KstDataWizard::applyFiltersChecked( bool on )
{
  setAppropriate(_pageFilters, on);
}


void KstDataWizard::enableXEntries()
{
  _xAxisGroup->setEnabled(true);
  xChanged();
}


void KstDataWizard::disableXEntries()
{
  _xAxisGroup->setEnabled(false);
  xChanged();
}


void KstDataWizard::enablePSDEntries()
{
  _kstFFTOptions->setEnabled(true);
}


void KstDataWizard::disablePSDEntries()
{
  _kstFFTOptions->setEnabled(false);
}


void KstDataWizard::search()
{
  QString s = _vectorReduction->text();
  if (!s.isEmpty()) {
    if (s[0] != '*') {
      s = "*" + s;
    }
    if (s[s.length()-1] != '*') {
      s += "*";
    }
    _vectorReduction->setText(s);
  }
}


void KstDataWizard::disableWindowEntries()
{
  _windowGroup->setEnabled(false);
}


void KstDataWizard::enableWindowEntries()
{
  _windowGroup->setEnabled(true);
}


void KstDataWizard::markSourceAndSave()
{
  assert(_configWidget);
  KstDataSourcePtr src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
  if (src) {
    src->disableReuse();
  }
  static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
}


void KstDataWizard::configureSource()
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
  static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->setInstance(ds);
  static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->load();
  dlg->exec();
  _configWidget->reparent(0L, QPoint(0, 0));
  dlg->setMainWidget(0L);
  delete dlg;
  sourceChanged(_url->url());
}

void KstDataWizard::saveSettings()
{
  KConfig cfg("kstrc", false, false);

  cfg.setGroup("DataWizard");

  cfg.writeEntry("PlotXY", _radioButtonPlotData->isChecked());
  cfg.writeEntry("PlotPS", _radioButtonPlotPSD->isChecked());
  cfg.writeEntry("PlotBoth", _radioButtonPlotDataPSD->isChecked());

  cfg.writeEntry("XCreate", _xAxisCreateFromField->isChecked());
  cfg.writeEntry("XFieldCreate", _xVector->currentText());
  cfg.writeEntry("XExists", _xAxisUseExisting->isChecked());
  cfg.writeEntry("XFieldExists", _xVectorExisting->selectedVector());

  cfg.writeEntry("Lines", _drawLines->isChecked());
  cfg.writeEntry("Points", _drawPoints->isChecked());
  cfg.writeEntry("Both", _drawBoth->isChecked());

  cfg.writeEntry("LogX", _psdLogX->isChecked());
  cfg.writeEntry("LogY", _psdLogY->isChecked());
  cfg.writeEntry("XAxisLabel", _xAxisLabels->isChecked());
  cfg.writeEntry("YAxisLabel", _yAxisLabels->isChecked());
  cfg.writeEntry("TitleLabel", _plotTitles->isChecked());

  cfg.writeEntry("LegendsOn", _legendsOn->isChecked());
  cfg.writeEntry("LegendsOff", _legendsOff->isChecked());
  cfg.writeEntry("LegendsAuto", _legendsAuto->isChecked());

  cfg.writeEntry("OnePlot", _onePlot->isChecked());
  cfg.writeEntry("MultiplePlots", _multiplePlots->isChecked());
  cfg.writeEntry("CycleThrough", _cycleThrough->isChecked());
  cfg.writeEntry("CycleExisting", _cycleExisting->isChecked());
  cfg.writeEntry("PlotNumber", _plotNumber->value());

  cfg.writeEntry("OrderInColumns", _orderInColumns->isChecked());
  cfg.writeEntry("PlotColumns", _plotColumns->value());
}


void KstDataWizard::loadSettings()
{
  KConfig cfg("kstrc");
  cfg.setGroup("DataWizard");

  if (cfg.readBoolEntry("PlotXY", true)) {
    _radioButtonPlotData->setChecked(true);
  } else if (cfg.readBoolEntry("PlotPS", true)) {
    _radioButtonPlotPSD->setChecked(true);
    _kstFFTOptions->setEnabled(true);
  } else if (cfg.readBoolEntry("PlotBoth", true)) {
    _radioButtonPlotDataPSD->setChecked(true);
    _kstFFTOptions->setEnabled(true);
  } else {
    _radioButtonPlotData->setChecked(true);
  }

  if (cfg.readBoolEntry("XCreate", true) || _xVectorExisting->_vector->count() == 0) {
    _xAxisCreateFromField->setChecked(true);
    QString str = cfg.readEntry("XFieldCreate", "");
    if (_xVector->listBox()) {
      QListBoxItem *item = _xVector->listBox()->findItem(str, Qt::ExactMatch);
      if (item) {
        _xVector->listBox()->setSelected(item, true);
      }
    }
  } else {
    _xAxisUseExisting->setChecked(true);
    QString str = cfg.readEntry("XFieldExists", "");
    if (_xVectorExisting->_vector->listBox() && _xVectorExisting->_vector->listBox()->findItem(str, Qt::ExactMatch)) {
      _xVectorExisting->setSelection(str);
    }
  }

  if (cfg.readBoolEntry("Lines", true)) {
    _drawLines->setChecked(true);
  } else if (cfg.readBoolEntry("Points", true)) {
    _drawPoints->setChecked(true);
  } else if (cfg.readBoolEntry("Both", true)) {
    _drawBoth->setChecked(true);
  } else {
    _drawLines->setChecked(true);
  }

  _psdLogX->setChecked(cfg.readBoolEntry("LogX", false));
  _psdLogY->setChecked(cfg.readBoolEntry("LogY", false));
  _xAxisLabels->setChecked(cfg.readBoolEntry("XAxisLabel", true));
  _yAxisLabels->setChecked(cfg.readBoolEntry("YAxisLabel", true));
  _plotTitles->setChecked(cfg.readBoolEntry("TitleLabel", true));

  if (cfg.readBoolEntry("LegendsAuto", true)) {
    _legendsAuto->setChecked(true);
  } else if (cfg.readBoolEntry("LegendsOn", true)) {
    _legendsOn->setChecked(true);
  } else { // off is default
    _legendsOff->setChecked(true);
  }

  _onePlot->setChecked(cfg.readBoolEntry("OnePlot", true));
  _multiplePlots->setChecked(cfg.readBoolEntry("MultiplePlots", false));
  _cycleThrough->setChecked(cfg.readBoolEntry("CycleThrough", false));
  _cycleExisting->setChecked(cfg.readBoolEntry("CycleExisting", false));
  _plotNumber->setValue(cfg.readNumEntry("PlotNumber", 2));
  _orderInColumns->setChecked(cfg.readBoolEntry("OrderInColumns", false));
  _plotColumns->setValue(cfg.readNumEntry("PlotColumns", 0));
}


void KstDataWizard::clear()
{
  QPtrList<QListViewItem> lst;

  QListViewItemIterator it(_vectorsToPlot);
  while (it.current()) {
    lst.append(it.current());
    ++it;
  }

  QPtrListIterator<QListViewItem> iter(lst);
  while (iter.current()) {
    _vectorsToPlot->takeItem(iter.current());
    _vectors->insertItem(iter.current());
    ++iter;
  }

  setNextEnabled(_pageVectors, yVectorsOk());
}


void KstDataWizard::down()
{
  QListViewItem *lastSelected = 0L;
  QListViewItem *lastUnselected = 0L;

  _vectorsToPlot->setSorting(10, true);

  QListViewItemIterator it(_vectorsToPlot);
  while (it.current()) {
    QListViewItem *current = it.current();
    ++it;
    if (_vectorsToPlot->isSelected(current)) {
      lastSelected = current;
    } else if (lastSelected) {
      if (lastUnselected) {
        current->moveItem(lastUnselected);
      } else {
        QListViewItem *itemAbove = current->itemAbove();
        while (itemAbove) {
          itemAbove->moveItem(current);
          itemAbove = current->itemAbove();
        }
      }

      lastUnselected = lastSelected;
      lastSelected = 0L;
    } else {
      lastUnselected = current;
      lastSelected = 0L;
    }
  }
}


void KstDataWizard::up()
{
  QListViewItem *lastSelected = 0L;
  QListViewItem *lastUnselected = 0L;

  _vectorsToPlot->setSorting(10, true);

  QListViewItemIterator it(_vectorsToPlot);
  while (it.current()) {
    if (_vectorsToPlot->isSelected(it.current())) {
      lastSelected = it.current();
    } else if (lastUnselected != 0L && lastSelected != 0L) {
      lastUnselected->moveItem(lastSelected);
      lastUnselected = it.current();
      lastSelected = 0L;
    } else {
      lastUnselected = it.current();
      lastSelected = 0L;
    }
    ++it;
  }

  if (lastUnselected && lastSelected) {
    lastUnselected->moveItem(lastSelected);
  }
}


void KstDataWizard::updateVectorPageButtons()
{
  setNextEnabled(_pageVectors, yVectorsOk());
}


void KstDataWizard::add()
{
  QPtrList<QListViewItem> lst;

  QListViewItemIterator it(_vectors);
  while (it.current()) {
    if (it.current()->isSelected()) {
      if (it.current()->childCount() == 0) {
        lst.append(it.current());
      }
    }
    ++it;
  }

  QListViewItem *last = _vectorsToPlot->lastItem();
  QPtrListIterator<QListViewItem> iter(lst);
  while (iter.current()) {
    QListViewItem* parent;
    QListViewItem *item = iter.current();
    parent = item->parent();

    while (parent) {
      item->setText(0, parent->text(0) + QDir::separator() + item->text(0));
      parent = parent->parent();
    }

    parent = item->parent();
    if (parent) {
      QListViewItem* parentNew;

      parent->takeItem(item);
      parentNew = parent;
      while (parentNew) {
        parent = parentNew;
        if (parent->childCount() == 0) {
          parentNew = parent->parent();
          parent->setSelected(false);
          parent->setVisible(false);
        } else {
          parentNew = 0L;
        }
      }
    } else {
      _vectors->takeItem(item);
    }
    _vectorsToPlot->insertItem(item);

    item->moveItem(last);
    item->setSelected(false);
    last = item;
    ++iter;
  }

  _vectors->setFocus();
  if (_vectors->currentItem()) {
    _vectors->currentItem()->setSelected(true);
  }
  updateVectorPageButtons();
}


void KstDataWizard::remove()
{
  QPtrList<QListViewItem> lst;
  QListViewItemIterator it(_vectorsToPlot);

  while (it.current()) {
    if (it.current()->isSelected()) {
      lst.append(it.current());
    }
    ++it;
  }

  QPtrListIterator<QListViewItem> iter(lst);
  while (iter.current()) {
    _vectorsToPlot->takeItem(iter.current());
    if (_hierarchy) {
      QListViewItem* parent = 0L;
      QStringList entries;
      QString text = iter.current()->text(0);
      QString item;

      entries = QStringList::split(QDir::separator(), text, FALSE);
      for (QStringList::ConstIterator itEntry = entries.begin(); itEntry != entries.end(); ++itEntry) {
        item += (*itEntry);
        if (text.compare(item) != 0) {
          parent = _fields.find(item);
          if (parent) {
            parent->setVisible(true);
          }
          item += QDir::separator();
        } else {
          iter.current()->setText(0, entries.back());

          if (parent) {
            parent->insertItem(iter.current());
          } else {
            _vectors->insertItem(iter.current());
          }
        }
      }
    } else {
      _vectors->insertItem(iter.current());
    }
    iter.current()->setSelected(false);
    ++iter;
  }

  _vectorsToPlot->setFocus();
  if (_vectorsToPlot->currentItem()) {
    _vectorsToPlot->currentItem()->setSelected(true);
  }
  updateVectorPageButtons();

  vectorsDroppedBack(0L); // abuse
}


void KstDataWizard::vectorsDroppedBack(QDropEvent *e)
{
  Q_UNUSED(e)

  // Note: e can be null
  QListViewItemIterator it(_vectors);
  while (it.current()) {
    QListViewItem *i = it.current();
    if (i->text(1).isEmpty()) {
      i->setText(1, _countMap[i->text(0)]);
    }
    ++it;
  }
  _vectors->sort();
  updateVectorPageButtons();
}

double KstDataWizard::getFontSize(KstViewWindow *w) {
  double size;

  // if there are already plots in the window, use the the first one's font size.
  Kst2DPlotList plotList = w->view()->findChildrenType<Kst2DPlot>(false);
  if (!plotList.isEmpty()) {
    size = plotList[0]->xTickLabel()->fontSize();
  } else {
    size = (double)KstSettings::globalSettings()->plotFontSize;
  }

  return size;
}


#include "kstdatawizard.moc"

