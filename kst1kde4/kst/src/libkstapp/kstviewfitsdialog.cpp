/***************************************************************************
                    kstviewfitsdialog.cpp  -  Part of KST
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

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>

#include "kstcplugin.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstviewfitsdialog.h"

KstViewFitsDialog::KstViewFitsDialog(QWidget* parent, const char* name,
                                             bool modal, Qt::WindowFlags fl) 
: QDialog(parent, fl) {
  setupUi(this);

  _tableFits = new KstFitTable(this, "tableFits");
  _tableFits->setRowCount(0);
  _tableFits->setColumnCount(1);
  _tableFits->setSelectionMode(QAbstractItemView::SingleSelection);
  hboxLayout->addWidget(_tableFits, 2, 0);

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_comboBoxFits, SIGNAL(activated(const QString&)), this, SLOT(fitChanged(const QString&)));
}

KstViewFitsDialog::~KstViewFitsDialog() {
}


bool KstViewFitsDialog::hasContent() const {
  bool content = false;
  KstCPluginList fits;
  KstCPluginList::const_iterator it;
  
  fits = kstObjectSubList<KstDataObject,KstCPlugin>(KST::dataObjectList);
  it = fits.begin();
  for (; it != fits.end(); ++it) {
    (*it)->readLock();
    content = (*it)->plugin()->data()._isFit ? true : content;
    (*it)->unlock();
  }

  return content;
}


void KstViewFitsDialog::fillComboBox(const QString& str) {
  KstCPluginList::iterator it;
  KstCPluginList fits;
  QString fitName = str;
  bool changed = false;
  uint i;

  _comboBoxFits->clear();
  fits = kstObjectSubList<KstDataObject,KstCPlugin>(KST::dataObjectList);

  for (it = fits.begin(); it != fits.end(); ++it) {
    KstCPluginPtr fit;

    fit = *it;
    fit->readLock();

    if (fit->plugin()->data()._isFit) {
      _comboBoxFits->insertItem(0, fit->tagName());
      if (fitName == fit->tagName() || fitName.isEmpty()) {
        _comboBoxFits->setCurrentIndex(_comboBoxFits->count() - 1);
        if (fitName.isEmpty()) {
          fitName = fit->tagName();
        }
        changed = true;
        fitChanged(fitName);
      }
    }

    fit->unlock();
  }

  if (!changed) {
    fitChanged(_comboBoxFits->currentText());
  }
}

void KstViewFitsDialog::updateViewFitsDialog() {
/* xxx
  if (_comboBoxFits->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(updateViewFitsDialog()));
  } else {
*/
    QString old;
    if (_comboBoxFits->count() > 0) {
      int idx;

      idx = _comboBoxFits->currentIndex();
      old = _comboBoxFits->itemText(idx);
    }
    fillComboBox(old);
// xxx  }
}

void KstViewFitsDialog::showViewFitsDialog(const QString& fit) {
  fillComboBox(fit);
}

void KstViewFitsDialog::showViewFitsDialog() {
  updateViewFitsDialog();
  updateDefaults(0);

  show();
  raise();
}

void KstViewFitsDialog::fitChanged(const QString& strFit) {
  KstCPluginList fits;
  KstCPluginPtr plugin;
  double* params = 0L;
  double* covars = 0L;
  double chi2Nu = 0.0;
  int numParams = 0;
  int numCovars = 0;
  int i;

  fits = kstObjectSubList<KstDataObject,KstCPlugin>(KST::dataObjectList);
  plugin = *(fits.findTag(strFit));
  if (plugin) {
    KstScalarPtr scalarChi2Nu;
    KstVectorPtr vectorParam;
    plugin->readLock();

    const KstScalarMap& scalars = plugin->outputScalars();
    
    scalarChi2Nu = scalars["chi^2/nu"];
    if (scalarChi2Nu) {
      scalarChi2Nu->readLock();
      chi2Nu = scalarChi2Nu->value();
      scalarChi2Nu->unlock();
    }

    const KstVectorMap& vectors = plugin->outputVectors();
    vectorParam = vectors["Parameters"];

    if (vectorParam) {
      KstVectorPtr vectorCovar;

      vectorParam->readLock();
      vectorCovar = vectors["Covariance"];
      if (vectorCovar) {
        vectorCovar->readLock();
        numParams = vectorParam->length();
        numCovars = vectorCovar->length();

        if (numParams > 0 && numCovars > 0) {
          params = new double[numParams];
          covars = new double[numCovars];

          for (i = 0; i < numParams; i++) {
            params[i] = vectorParam->value(i);
          }

          for (i = 0; i < numCovars; i++) {
            covars[i] = vectorCovar->value(i);
          }
        }
        vectorCovar->unlock();
      }
      vectorParam->unlock();
    }
    plugin->unlock();
  }

  _tableFits->setParameters(params, numParams, covars, numCovars, chi2Nu);

  if (numParams > 0) {
    _tableFits->horizontalHeaderItem(0)->setText(QObject::tr("Value"));
    _tableFits->horizontalHeaderItem(1)->setText(QObject::tr("Covariance:"));

    _tableFits->verticalHeaderItem(numParams+0)->setText("---");
    _tableFits->verticalHeaderItem(numParams+1)->setText(QObject::tr("Chi^2/Nu"));

    if (plugin) {
      QExplicitlySharedDataPointer<Plugin> pluginBase;

      plugin->readLock();
      pluginBase = plugin->plugin();

      if (pluginBase) {
        textLabelFit->setText(pluginBase->data()._readableName);
        for (i = 0; i < numParams; i++) {
          QString parameterName = pluginBase->parameterName(i);
          _tableFits->horizontalHeaderItem(i+2)->setText(parameterName);
          _tableFits->verticalHeaderItem(i)->setText(parameterName);
        }
      }
      plugin->unlock();
    }
  }

  _tableFits->update();
}

void KstViewFitsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewfitsdialog.moc"
