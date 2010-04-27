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
#include <QListBox>
#include <QPushButton>
#include <QTable>
#include <QTimer>

#include "kstcplugin.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstviewfitsdialog.h"

KstViewFitsDialog::KstViewFitsDialog(QWidget* parent,
                                             const char* name,
                                             bool modal,
                                             Qt::WindowFlags fl) 
: QDialog(parent, name, modal, fl) {
  setupUi(this);
  tableFits = new KstFitTable(this, "tableFits");
  tableFits->setNumRows(0);
  tableFits->setNumCols(1);
  tableFits->setReadOnly(true);
  tableFits->setSorting(false);
  tableFits->setSelectionMode(QTable::Single);
  layout2->addWidget(tableFits, 2, 0);

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_comboBoxFits, SIGNAL(activated(const QString&)), this, SLOT(fitChanged(const QString&)));

  tableFits->setReadOnly(true);
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
  KstCPluginList fits;
  QString fitName = str;
  bool changed = false;
  uint i;

  _comboBoxFits->clear();
  fits = kstObjectSubList<KstDataObject,KstCPlugin>(KST::dataObjectList);
  for (i = 0; i < fits.count(); i++) {
    KstCPluginPtr fit = fits[i];

    fit->readLock();

    if (fit->plugin()->data()._isFit) {
      _comboBoxFits->insertItem(fit->tagName());
      if (fitName == fit->tagName() || fitName.isEmpty()) {
        _comboBoxFits->setCurrentItem(_comboBoxFits->count() - 1);
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
  if (_comboBoxFits->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(updateViewFitsDialog()));
  } else {
    QString old;
    if (_comboBoxFits->count() > 0) {
      int idx = _comboBoxFits->currentItem();
      old = _comboBoxFits->text(idx);
    }
    fillComboBox(old);
  }
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

  fits = kstObjectSubList<KstDataObject,KstCPlugin>(KST::dataObjectList);
  plugin = *(fits.findTag(strFit));
  if (plugin) {
    plugin->readLock();

    const KstScalarMap& scalars = plugin->outputScalars();
    KstScalarPtr scalarChi2Nu = scalars["chi^2/nu"];
    if (scalarChi2Nu) {
      scalarChi2Nu->readLock();
      chi2Nu = scalarChi2Nu->value();
      scalarChi2Nu->unlock();
    }

    const KstVectorMap& vectors = plugin->outputVectors();
    KstVectorPtr vectorParam = vectors["Parameters"];

    if (vectorParam) {
      vectorParam->readLock();
      KstVectorPtr vectorCovar = vectors["Covariance"];
      if (vectorCovar) {
        vectorCovar->readLock();
        numParams = vectorParam->length();
        numCovars = vectorCovar->length();

        if (numParams > 0 && numCovars > 0) {
          params = new double[numParams];
          covars = new double[numCovars];

          for (int i = 0; i < numParams; i++) {
            params[i] = vectorParam->value(i);
          }

          for (int i = 0; i < numCovars; i++) {
            covars[i] = vectorCovar->value(i);
          }
        }
        vectorCovar->unlock();
      }
      vectorParam->unlock();
    }
    plugin->unlock();
  }

  tableFits->setParameters(params, numParams, covars, numCovars, chi2Nu);

  if (numParams > 0) {
    tableFits->horizontalHeader()->setLabel(0, tr("Value"));
    tableFits->horizontalHeader()->setLabel(1, tr("Covariance:"));

    tableFits->verticalHeader()->setLabel(numParams+0, "---");
    tableFits->verticalHeader()->setLabel(numParams+1, tr("Chi^2/Nu"));

    if (plugin) {
      QExplicitlySharedDataPointer<Plugin> pluginBase;

      plugin->readLock();
      pluginBase = plugin->plugin();

      if (pluginBase) {
        textLabelFit->setText(pluginBase->data()._readableName);
        for (int i = 0; i < numParams; i++) {
          QString parameterName = pluginBase->parameterName(i);
          tableFits->horizontalHeader()->setLabel(i + 2, parameterName);
          tableFits->verticalHeader()->setLabel(i, parameterName);
        }
      }
      plugin->unlock();
    }
  }

  tableFits->update();
}

void KstViewFitsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewfitsdialog.moc"
