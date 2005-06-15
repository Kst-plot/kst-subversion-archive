/***************************************************************************
                     kstplugindialog_I.h  -  Part of KST
                             -------------------
    begin                : Mon May 12 2003
    copyright            : (C) 2003 The University of Toronto
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

// include files for Qt
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// application specific includes
#include "kst.h"
#include "kstplugindialog_i.h"
#include "kstdefaultnames.h"
#include "plugincollection.h"
#include "pluginmanager.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"

#define DIALOGTYPE KstPluginDialogI
#define DTYPE "Plugin"
#include "dataobjectdialog.h"


QGuardedPtr<KstPluginDialogI> KstPluginDialogI::_inst;

KstPluginDialogI *KstPluginDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstPluginDialogI(KstApp::inst());
  }
  return _inst;
}


static const QString& plugin_defaultTag = KGlobal::staticQString("<Auto Name>");

KstPluginDialogI::KstPluginDialogI(QWidget* parent, const char* name,
                                   bool modal, WFlags fl)
: KstPluginDialog(parent, name, modal, fl) {
  Init();
  connect(PluginCombo, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));
  connect(_pluginManager, SIGNAL(clicked()), this, SLOT(showPluginManager()));

  QGridLayout *grid = new QGridLayout(_pluginFrame, 1, 1);
  grid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
}


KstPluginDialogI::~KstPluginDialogI() {
  DP = 0L;
}


KstPluginPtr KstPluginDialogI::_getPtr(const QString &tagin) {
  KstPluginList c = kstObjectSubList<KstDataObject, KstPlugin>(KST::dataObjectList);

  return *c.findTag(tagin);
}


void KstPluginDialogI::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  QString previous = _pluginList[PluginCombo->currentItem()];
  int newFocus = -1;

  const QMap<QString,Plugin::Data>& pluginMap = pc->pluginList();

  QMap<QString,QString> oldIEntries, oldOEntries;

  PluginCombo->clear();
  _pluginList.clear();
  int cnt = 0;
  Plugin::Data restoreEntry;
  for (QMap<QString,Plugin::Data>::ConstIterator it = pluginMap.begin();
                                                  it != pluginMap.end();
                                                                   ++it) {
    _pluginList += it.data()._name;
    PluginCombo->insertItem(i18n("%1 (v%2) - %3").arg(it.data()._readableName).arg(it.data()._version).arg(it.data()._description));
    if (it.data()._name == previous) {
      newFocus = cnt;
      oldIEntries = cacheInputs(it.data()._inputs);
      oldOEntries = cacheInputs(it.data()._outputs);
      restoreEntry = it.data();
    }
    ++cnt;
  }

  if (newFocus != -1) {
    PluginCombo->setCurrentItem(newFocus);
    pluginChanged(PluginCombo->currentItem());
    restoreInputs(restoreEntry._inputs, oldIEntries);
    restoreInputs(restoreEntry._outputs, oldOEntries);
  } else {
    PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


void KstPluginDialogI::updateForm() {
  KstSharedPtr<Plugin> plugin = PluginCollection::self()->plugin(_pluginList[PluginCombo->currentItem()]);
  if (plugin) {
    const QValueList<Plugin::Data::IOValue>& itable = plugin->data()._inputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        assert(field);
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          vs->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) { // string
        QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
        assert(field);
        if (field) {
          StringSelector *ss = static_cast<StringSelector*>(field);
          ss->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) { 
        // Nothing
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
        assert(field);
        if (field) {
          ScalarSelector *ss = static_cast<ScalarSelector*>(field);
          ss->update();
        }
      }
    }
  }
}


void KstPluginDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }
  DP->readLock();
  _tagName->setText(DP->tagName());

  updatePluginList();

  int i = _pluginList.findIndex(DP->plugin()->data()._name);
  PluginCombo->setCurrentItem(i);
  pluginChanged(PluginCombo->currentItem());

  fillVectorScalarCombos(DP->plugin());
  PluginCombo->setEnabled(DP->getUsage() < 2);
  DP->readUnlock();

  show();
  fixupLayout();
}


void KstPluginDialogI::_fillFieldsForNew() { 
  updatePluginList();
  PluginCombo->setCurrentItem(0);
  pluginChanged(PluginCombo->currentItem());
  _tagName->setText(plugin_defaultTag); 
}


void KstPluginDialogI::fillVectorScalarCombos(KstSharedPtr<Plugin> plugin) {
  bool DPvalid = false;

  if (DP && DP->isValid()) {
    DPvalid = true;
  }

  if (plugin) {
    if (DPvalid) {
      DP->readLock();
    }
    // Update input vector and scalar combos
    const QValueList<Plugin::Data::IOValue>& itable = plugin->data()._inputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin();
         it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        assert(field);
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          QString selectedVector = vs->selectedVector();
          vs->update();
          if (DPvalid) {
            vs->setSelection(DP->inputVectors()[(*it)._name]->tagName());
          } else {
            vs->setSelection(selectedVector);
          }
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
        assert(field);
        if (field) {
          StringSelector *ss = static_cast<StringSelector*>(field);
          QString selectedString = ss->selectedString();
          ss->update();
          if (DPvalid) {
            ss->setSelection(DP->inputStrings()[(*it)._name]->tagName());
          } else {
            ss->setSelection(selectedString);
          }
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) { 
        // Nothing
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
        assert(field);
        if (field) {
          ScalarSelector *ss = static_cast<ScalarSelector*>(field);
          QString selectedScalar = ss->selectedScalar();
          ss->update();
          if (DPvalid) {
            ss->setSelection(DP->inputScalars()[(*it)._name]->tagName());
          } else {
            ss->setSelection(selectedScalar);
          }
        }
      }
    }

    // Update output vector and scalar lineedits
    if (DPvalid) {
      const QValueList<Plugin::Data::IOValue>& otable = plugin->data()._outputs;
      for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        assert(field);
        if (field) {
          QLineEdit *li = static_cast<QLineEdit*>(field);
          QString ts;
          if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
            ts = DP->outputVectors()[(*it)._name]->tagName();
          } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
          } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
            ts = DP->outputStrings()[(*it)._name]->tagName();
          } else { // scalar
            ts = DP->outputScalars()[(*it)._name]->tagName();
          }
          li->setText(ts);
        }
      }
      DP->readUnlock();
    }
  } else { // invalid plugin
    PluginCollection *pc = PluginCollection::self();
    QString cur = _pluginList[PluginCombo->currentItem()];
    Plugin::Data pdata = pc->pluginList()[pc->pluginNameList()[cur]];
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = pdata._outputs.begin(); it != pdata._outputs.end(); ++it) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
      if (field) {
        static_cast<QLineEdit*>(field)->setText(QString::null);
      }
    }
  }
}


void KstPluginDialogI::update() {
  updatePluginList();
}


void KstPluginDialogI::fixupLayout() {
  _frameWidget->updateGeometry();
  _pluginFrame->updateGeometry();
  resize(sizeHint());
  setMinimumSize(size());
  updateGeometry();
}


QMap<QString,QString> KstPluginDialogI::cacheInputs(const QValueList<Plugin::Data::IOValue>& table) {
  QMap<QString,QString> rc;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
      if (field) {
        rc[(*it)._name] = static_cast<VectorSelector*>(field)->selectedVector();
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          rc[(*it)._name] = static_cast<QLineEdit*>(field)->text();
        }
      }

    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
      if (field) {
        rc[(*it)._name] = static_cast<StringSelector*>(field)->selectedString();
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          rc[(*it)._name] = static_cast<QLineEdit*>(field)->text();
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
      if (field) {
        rc[(*it)._name] = static_cast<ScalarSelector*>(field)->selectedScalar();
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          rc[(*it)._name] = static_cast<QLineEdit*>(field)->text();
        }
      }
    }
  }
  return rc;
}


void KstPluginDialogI::restoreInputs(const QValueList<Plugin::Data::IOValue>& table, const QMap<QString,QString>& v) {
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    if (!v.contains((*it)._name)) {
      continue;
    }
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
      if (field) {
        static_cast<VectorSelector*>(field)->setSelection(v[(*it)._name]);
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          static_cast<QLineEdit*>(field)->setText(v[(*it)._name]);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
      if (field) {
        static_cast<StringSelector*>(field)->setSelection(v[(*it)._name]);
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          static_cast<QLineEdit*>(field)->setText(v[(*it)._name]);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
      if (field) {
        static_cast<ScalarSelector*>(field)->setSelection(v[(*it)._name]);
      } else {
        field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
        if (field) {
          static_cast<QLineEdit*>(field)->setText(v[(*it)._name]);
        }
      }
    }
  }
}


bool KstPluginDialogI::saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  bool rc = true;
  
  KST::vectorList.lock().readLock();
  KST::scalarList.lock().readLock();
  KST::stringList.lock().readLock();
  const QValueList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
      assert(field);
      VectorSelector *vs = static_cast<VectorSelector*>(field);
      KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());
      if (v) {
        v->writeLock(); // to match with plugin->writeLock()
        if (plugin->inputVectors().contains((*it)._name) && plugin->inputVectors()[(*it)._name] != v) {
          plugin->inputVectors()[(*it)._name]->writeUnlock();
        }
        plugin->inputVectors().insert((*it)._name, v);
      } else if (plugin->inputVectors().contains((*it)._name)) {
        plugin->inputVectors().erase((*it)._name);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
      assert(field);
      StringSelector *ss = static_cast<StringSelector*>(field);
      KstStringPtr s = *KST::stringList.findTag(ss->selectedString());
      if (s == *KST::stringList.end()) {
        QString val = ss->_string->currentText();
        KstStringPtr newString = new KstString(ss->_string->currentText(), val, true, false);
        newString->writeLock(); // to match with plugin->writeLock()
        if (plugin->inputStrings().contains((*it)._name) && plugin->inputStrings()[(*it)._name] != s) {
          plugin->inputStrings()[(*it)._name]->writeUnlock();
        }
        plugin->inputStrings().insert((*it)._name, newString);
      } else {
        s->writeLock(); // to match with plugin->writeLock()
        if (plugin->inputStrings().contains((*it)._name) && plugin->inputStrings()[(*it)._name] != s) {
          plugin->inputStrings()[(*it)._name]->writeUnlock();
        }
        plugin->inputStrings().insert((*it)._name, s);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
      assert(field);
      ScalarSelector *ss = static_cast<ScalarSelector*>(field);
      KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());
      if (s == *KST::scalarList.end()) {
        bool ok;
        double val = ss->_scalar->currentText().toDouble(&ok);

        if (ok) {
          KstScalarPtr newScalar = new KstScalar(ss->_scalar->currentText(), val, true, false, false);
          newScalar->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != s) {
            plugin->inputScalars()[(*it)._name]->writeUnlock();
          }
          plugin->inputScalars().insert((*it)._name, newScalar);
        } else {
          s->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != s) {
            plugin->inputScalars()[(*it)._name]->writeUnlock();
          }
          plugin->inputScalars().insert((*it)._name, s);
        }
      } else {
        s->writeLock(); // to match with plugin->writeLock()
        if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != s) {
          plugin->inputScalars()[(*it)._name]->writeUnlock();
        }
        plugin->inputScalars().insert((*it)._name, s);
      }
    } else {
    }
  }
  KST::stringList.lock().readUnlock();
  KST::scalarList.lock().readUnlock();
  KST::vectorList.lock().readUnlock();
  
  return rc;
}


bool KstPluginDialogI::saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  const QValueList<Plugin::Data::IOValue>& otable = p->data()._outputs;

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
    QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
    if (!field) {
      continue; // Some are unsupported
    }

    QLineEdit *li = static_cast<QLineEdit*>(field);

    if (li->text().isEmpty()) {
      QString tagName = _tagName->text();
      if (tagName==plugin_defaultTag) {
        tagName = plugin->tagName();
      }
      li->setText(tagName + "-" + (*it)._name);
    }

    QString nt = li->text();
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        // Implicitly creates it if it doesn't exist
        KstVectorPtr v = plugin->outputVectors()[(*it)._name];
        if (!v) {
          v = new KstVector;
          v->writeLock();
          plugin->outputVectors().insert((*it)._name, v);
          KST::addVectorToList(v);
        }
        v->setTagName(nt);
        v->setProvider(plugin.data()); // in case it's implicitly created
      } else if (plugin->outputVectors()[(*it)._name]->tagName() != nt) {
        while (KST::vectorTagNameNotUnique(nt, false)) {
          nt += "'";
        }
        KstVectorPtr v;
        if (plugin->outputVectors().contains((*it)._name)) {
          v = plugin->outputVectors()[(*it)._name];
        } else {
          v = new KstVector;
          v->writeLock();
          v->setProvider(plugin.data());
          plugin->outputVectors().insert((*it)._name, v);
          KST::addVectorToList(v);
        }
        v->setTagName(nt);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        KstStringPtr s;
        if (plugin->outputStrings().contains((*it)._name)) {
          s = plugin->outputStrings()[(*it)._name];
        } else {
          s = new KstString;
          s->writeLock();
          plugin->outputStrings().insert((*it)._name, s);
        }
        s->setTagName(nt);
        s->setProvider(plugin.data());
      } else if (plugin->outputStrings()[(*it)._name]->tagName() != nt) {
        while (KST::vectorTagNameNotUnique(nt, false)) {
          nt += "'";
        }
        KstStringPtr s;
        if (plugin->outputStrings().contains((*it)._name)) {
          s = plugin->outputStrings()[(*it)._name];
        } else {
          s = new KstString;
          s->writeLock();
          plugin->outputStrings().insert((*it)._name, s);
        }
        s->setTagName(nt);
        s->setProvider(plugin.data());
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        KstScalarPtr s;
        if (plugin->outputScalars().contains((*it)._name)) {
          s = plugin->outputScalars()[(*it)._name];
        } else {
          s = new KstScalar;
          s->writeLock();
          plugin->outputScalars().insert((*it)._name, s);
        }
        s->setTagName(nt);
        s->setProvider(plugin.data());
      } else if (plugin->outputScalars()[(*it)._name]->tagName() != nt) {
        while (KST::vectorTagNameNotUnique(nt, false)) {
          nt += "'";
        }
        KstScalarPtr s;
        if (plugin->outputScalars().contains((*it)._name)) {
          s = plugin->outputScalars()[(*it)._name];
        } else {
          s = new KstScalar;
          s->writeLock();
          plugin->outputScalars().insert((*it)._name, s);
        }
        s->setTagName(nt);
        s->setProvider(plugin.data());
      }
    }
  }

  return true;
}


bool KstPluginDialogI::new_I() {
  QString tagName = _tagName->text();

  if (tagName != plugin_defaultTag && KST::dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  }
  KstPluginPtr plugin;
  int pitem = PluginCombo->currentItem();
  if (pitem >= 0 && PluginCombo->count() > 0) {
    KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);
    if (pPtr) {
      plugin = new KstPlugin;
      plugin->writeLock();
      if (!saveInputs(plugin, pPtr)) {
        KMessageBox::sorry(this, i18n("One or more of the inputs was undefined."));
        plugin->writeUnlock();
        plugin = 0L;
        return false;
      }

      plugin->setPlugin(pPtr);

      if (tagName == plugin_defaultTag) {
        tagName = KST::suggestPluginName(_pluginList[pitem]);
      }
      plugin->setTagName(tagName);
      if (!saveOutputs(plugin, pPtr)) {
        KMessageBox::sorry(this, i18n("One or more of the outputs was undefined."));
        plugin->writeUnlock();
        plugin = 0L;
        return false;
      }
      plugin->writeUnlock();
    }
  }

  if (!plugin || !plugin->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the plugin you entered.  Please fix the plugin."));
    return false;
  }

  plugin->setDirty();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(plugin.data());
  KST::dataObjectList.lock().writeUnlock();
  plugin = 0L;
  emit modified();

  return true;
}


bool KstPluginDialogI::edit_I() {
  if (!DP) { // something is dreadfully wrong - this should never happen
    return false;
  }

  DP->writeLock();
  if (_tagName->text() != DP->tagName() && KST::dataTagNameNotUnique(_tagName->text())) {
    _tagName->setFocus();
    DP->writeUnlock();
    return false;
  }

  DP->setTagName(_tagName->text());

  int pitem = PluginCombo->currentItem();
  KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

  // Must unlock before clear()
  for (KstVectorMap::Iterator i = DP->inputVectors().begin(); i != DP->inputVectors().end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstScalarMap::Iterator i = DP->inputScalars().begin(); i != DP->inputScalars().end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstStringMap::Iterator i = DP->inputStrings().begin(); i != DP->inputStrings().end(); ++i) {
    (*i)->writeUnlock();
  }
  DP->inputVectors().clear();
  DP->inputScalars().clear();
  DP->inputStrings().clear();

  // Save the vectors and scalars
  if (!saveInputs(DP, pPtr)) {
    KMessageBox::sorry(this, i18n("There is an error in the plugin you "
                                  "entered. Please fix it."));
    DP->writeUnlock();
    return false;
  }

  if (pitem >= 0 && PluginCombo->count() > 0) {
    DP->setPlugin(pPtr);
  }

  if (!saveOutputs(DP, pPtr)) {
    KMessageBox::sorry(this, i18n("There is an error in the plugin you "
                                  "entered. Please fix it."));
    DP->writeUnlock();
    return false;
  }

  if (!DP->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the plugin you entered. Please fix it."));
    DP->writeUnlock();
    return false;
  }
  DP->setDirty();
  DP->writeUnlock();

  emit modified();
  return true;
}

void KstPluginDialogI::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table) {
  QString scalarLabelTemplate, vectorLabelTemplate, stringLabelTemplate;

  if (input) {
    stringLabelTemplate = i18n("Input string - %1:");
    scalarLabelTemplate = i18n("Input scalar - %1:");
    vectorLabelTemplate = i18n("Input vector - %1:");
  } else {
    stringLabelTemplate = i18n("Output string - %1:");
    scalarLabelTemplate = i18n("Output scalar - %1:");
    vectorLabelTemplate = i18n("Output vector - %1:");
  }

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    QString labellabel;
    bool scalar = false;
    bool string = false;
    switch ((*it)._type) {
      case Plugin::Data::IOValue::PidType:
        continue;
      case Plugin::Data::IOValue::StringType:
        labellabel = stringLabelTemplate.arg((*it)._name);
        string = true;
        break;
      case Plugin::Data::IOValue::FloatType:
        labellabel = scalarLabelTemplate.arg((*it)._name);
        scalar = true;
        break;
      case Plugin::Data::IOValue::TableType:
        if ((*it)._subType == Plugin::Data::IOValue::FloatSubType ||
            (*it)._subType == Plugin::Data::IOValue::FloatNonVectorSubType) {
          labellabel = vectorLabelTemplate.arg((*it)._name);
        } else {
          // unsupported
          continue;
        }
        break;
      default:
        // unsupported
        continue;
    }

    QLabel *label = new QLabel(labellabel, parent, input ? "Input label" : "Output label");

    QWidget *widget = 0L;

    if (input) {
      if (scalar) {
        ScalarSelector *w = new ScalarSelector(parent, (*it)._name.latin1());
        widget = w;
        connect(w->_scalar, SIGNAL(activated(const QString&)), this, SLOT(updateScalarTooltip(const QString&)));
        connect(widget, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_scalar->insertItem((*it)._default);
          w->_scalar->setCurrentText((*it)._default);
        }
        KstScalarPtr p = *KST::scalarList.findTag(w->_scalar->currentText());
        w->allowDirectEntry(true);
        if (p) {
          p->readLock();
          QToolTip::add(w->_scalar, QString::number(p->value()));
          p->readUnlock();
        }
      } else if (string) {
        StringSelector *w = new StringSelector(parent, (*it)._name.latin1());
        widget = w;
        connect(w->_string, SIGNAL(activated(const QString&)), this, SLOT(updateStringTooltip(const QString&)));
        connect(widget, SIGNAL(newStringCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_string->insertItem((*it)._default);
          w->_string->setCurrentText((*it)._default);
        }
        KstStringPtr p = *KST::stringList.findTag(w->_string->currentText());
        w->allowDirectEntry(true);
        if (p) {
          p->readLock();
          QToolTip::add(w->_string, p->value());
          p->readUnlock();
        }
      } else {
        widget = new VectorSelector(parent, (*it)._name.latin1());
        connect(widget, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
      }
    } else {
      widget = new QLineEdit(parent, (*it)._name.latin1());
    }

    grid->addWidget(label, cnt, 0);
    grid->addWidget(widget, cnt, 1);

    if (!(*it)._description.isEmpty()) {
      QWhatsThis::add(label, (*it)._description);
      QWhatsThis::add(widget, (*it)._description);
    }

    cnt++;
  }
}


void KstPluginDialogI::pluginChanged(int idx) {
  // Get rid of the old widget
  delete _frameWidget;
  _frameWidget = 0L;

  // Create a new one
  QGridLayout *topGrid = dynamic_cast<QGridLayout*>(_pluginFrame->layout());
  if (topGrid) {
    topGrid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
  } else {
    kdError() << "Somehow we lost the grid!" << endl;
    return;
  }

  // Refill it
  if (idx >= 0 && PluginCombo->count() > 0) {
    const QString& pluginName = _pluginList[idx];
    const Plugin::Data& pluginData = PluginCollection::self()->pluginList()[PluginCollection::self()->pluginNameList()[pluginName]];
    int cnt = 3;

    // Setup the grid and the "static" entries
    int variables = pluginData._inputs.count() + pluginData._outputs.count();
    QGridLayout *grid = new QGridLayout(_frameWidget, 4+variables, 2);
    grid->setMargin(6);
    grid->setSpacing(5);
    grid->addWidget(new QLabel(i18n("Plugin name:"), _frameWidget), 0, 0);
    grid->addWidget(new QLabel(pluginData._readableName, _frameWidget), 0, 1);
    grid->addWidget(new QLabel(i18n("Description:"), _frameWidget), 1, 0);
    QLabel *lab = new QLabel(pluginData._description, _frameWidget);
    lab->setAlignment(lab->alignment() | Qt::WordBreak);
    grid->addWidget(lab, 1, 1);

    // Add a separator
    QFrame* line = new QFrame(_frameWidget);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    grid->addMultiCellWidget(line, 2, 2, 0, 1);

    // Generate the input values
    generateEntries(true, cnt, _frameWidget, grid, pluginData._inputs);

    if (!pluginData._inputs.isEmpty() && !pluginData._outputs.isEmpty()) {
      // Add a separator
      line = new QFrame(_frameWidget);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFrameShape(QFrame::HLine);
      grid->addMultiCellWidget(line, cnt, cnt, 0, 1);
      cnt++;
    }

    // Generate the output values
    generateEntries(false, cnt, _frameWidget, grid, pluginData._outputs);
  }

  // show it
  _frameWidget->show();

  // resize everything
  QTimer::singleShot(0, this, SLOT(fixupLayout()));
}


void KstPluginDialogI::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;
  updatePluginList();
}


void KstPluginDialogI::updateScalarTooltip(const QString& n) {
  KstScalarPtr s = *KST::scalarList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));
  if (s) {
    s->readLock();
    QToolTip::remove(w);
    QToolTip::add(w, QString::number(s->value()));
    s->readUnlock();
  } else {
    QToolTip::remove(w);
  }
}


void KstPluginDialogI::updateStringTooltip(const QString& n) {
  KstStringPtr s = *KST::stringList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));
  if (s) {
    s->readLock();
    QToolTip::remove(w);
    QToolTip::add(w, s->value());
    s->readUnlock();
  } else {
    QToolTip::remove(w);
  }
}


#include "kstplugindialog_i.moc"

// vim: ts=2 sw=2 et
