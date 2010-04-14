/***************************************************************************
                     kstplugindialog.cpp  -  Part of KST
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

#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QTextEdit>
#include <QTimer>
#include <QToolTip>
#include <QWhatsThis>

#include "curveappearancewidget.h"
#include "kst.h"
#include "kstplugindialog.h"
#include "kstpluginmanager.h"
#include "kstdataobjectcollection.h"
#include "kstdefaultnames.h"
#include "plugincollection.h"
#include "ui_plugindialogwidget.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"

const QString& KstPluginDialog::plugin_defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstPluginDialog> KstPluginDialog::_inst;

KstPluginDialog *KstPluginDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstPluginDialog(KstApp::inst());
  }

  return _inst;
}


KstPluginDialog::KstPluginDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent) {
  _w = new Ui::PluginDialogWidget();
  _w->setupUi(this);

  setMultiple(false);
  connect(_w->PluginCombo, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->PluginCombo, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));
  connect(_w->_pluginManager, SIGNAL(clicked()), this, SLOT(showPluginManager()));
  connect(_w->_curveAppearance, SIGNAL(changed()), this, SLOT(wasModifiedApply()));

  _pluginInfoGrid = 0L;
  _pluginInputOutputGrid = 0L;
  _pluginName = QString::null;
}


KstPluginDialog::~KstPluginDialog() {
}


QString KstPluginDialog::editTitle() {
  return QObject::tr("Edit Plugin");
}


QString KstPluginDialog::newTitle() {
  return QObject::tr("New Plugin");
}


void KstPluginDialog::updatePluginList() {
  QMap<QString,Plugin::Data>::const_iterator it;
  PluginCollection *pc = PluginCollection::self();
  Plugin::Data restoreEntry;
  QString previous = _pluginList[_w->PluginCombo->currentIndex()];
  QMap<QString,QString> oldIEntries;
  QMap<QString,QString> oldOEntries;
  int newFocus = -1;
  int cnt = 0;
  const QMap<QString,Plugin::Data>& pluginMap = pc->pluginList();

  _w->PluginCombo->clear();
  _pluginList.clear();

  for (it = pluginMap.begin(); it != pluginMap.end(); ++it) {
    _pluginList += (*it)._name;
    _w->PluginCombo->insertItem(0, QObject::tr("%1 (v%2)").arg((*it)._readableName).arg((*it)._version));
    if ((*it)._name == previous) {
      newFocus = cnt;
// xxx      oldIEntries = cacheInputs((*it)._inputs);
// xxx      oldOEntries = cacheInputs((*it)._outputs);
      restoreEntry = (*it);
    }
    ++cnt;
  }

  if (newFocus != -1) {
    _w->PluginCombo->setCurrentIndex(newFocus);
    pluginChanged(_w->PluginCombo->currentIndex());
// xxx    restoreInputs(restoreEntry._inputs, oldIEntries);
// xxx    restoreInputs(restoreEntry._outputs, oldOEntries);
  } else {
    _w->PluginCombo->setCurrentIndex(0);
    pluginChanged(0);
  }
}


void KstPluginDialog::updateForm() {
  QExplicitlySharedDataPointer<Plugin> plugin = PluginCollection::self()->plugin(_pluginList[_w->PluginCombo->currentIndex()]);
  if (plugin) {
    QList<Plugin::Data::IOValue>::const_iterator it;
    const QList<Plugin::Data::IOValue>& itable = plugin->data()._inputs;
/* xxx
    for (it = itable.begin(); it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
        QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "VectorSelector");

        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);

          vs->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) { // string
        QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "StringSelector");

        if (field) {
          StringSelector *ss = static_cast<StringSelector*>(field);

          ss->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
        // Nothing
      } else {
        QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "ScalarSelector");

        if (field) {
          ScalarSelector *ss = static_cast<ScalarSelector*>(field);

          ss->update();
        }
      }
    }
*/
  }
}


void KstPluginDialog::fillFieldsForEdit() {
  KstCPluginPtr pp;

  pp = kst_cast<KstCPlugin>(_dp);
  if (!pp) {
    return;
  }

  pp->readLock();
  if (!pp->plugin()) { // plugin() can be null if the kst file is invalid
    pp->unlock();
    return;
  }

  const QString pluginName(pp->tagName());
  const QString pluginObjectName(pp->plugin()->data()._name);
  const int usage = pp->getUsage();
  QExplicitlySharedDataPointer<Plugin> plug = pp->plugin();
  int index;

  pp->unlock();

  _tagName->setText(pluginName);
  updatePluginList();

  index = _pluginList.indexOf(pluginObjectName);
  _w->PluginCombo->setCurrentIndex(index);
  pluginChanged(_w->PluginCombo->currentIndex());

  fillVectorScalarCombos(plug);
  _w->PluginCombo->setEnabled(usage < 3);

  fixupLayout();
}


void KstPluginDialog::fillFieldsForNew() {
  int index;

  updatePluginList();
  index = _pluginList.indexOf(_pluginName);
  _w->PluginCombo->setCurrentIndex(index);
  pluginChanged(_w->PluginCombo->currentIndex());
  _tagName->setText(plugin_defaultTag);
}


void KstPluginDialog::fillVectorScalarCombos(QExplicitlySharedDataPointer<Plugin> plugin) {
  bool dpValid = false;
  KstCPluginPtr pp;

  pp = kst_cast<KstCPlugin>(_dp);
  if (pp) {
    pp->readLock();
    dpValid = pp->isValid();
    pp->unlock();
  }

  if (plugin) {
    if (dpValid) {
      pp->readLock();
    }

    //
    // Update input vector and scalar combos...
    //

    const QList<Plugin::Data::IOValue>& itable = plugin->data()._inputs;
    QList<Plugin::Data::IOValue>::const_iterator it;
    
    for (it = itable.begin(); it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        VectorSelector *vs = 0L;
        
        vs =  _w->_pluginInputOutputFrame->findChild<VectorSelector*>((*it)._name.toLatin1());
        if (vs) {
          QString selectedVector = vs->selectedVector();

          vs->update();
          if (dpValid) {
            vs->setSelection(pp->inputVectors()[(*it)._name]->tag().displayString());
          } else {
            vs->setSelection(selectedVector);
          }
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        StringSelector *ss = 0L;

        ss = _w->_pluginInputOutputFrame->findChild<StringSelector*>((*it)._name.toLatin1());
        if (ss) {
          QString selectedString = ss->selectedString();

          ss->update();
          if (dpValid) {
            ss->setSelection(pp->inputStrings()[(*it)._name]->tag().displayString());
          } else {
            ss->setSelection(selectedString);
          }
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
        // Nothing
      } else {
        ScalarSelector *ss = 0L;

        ss = _w->_pluginInputOutputFrame->findChild<ScalarSelector*>((*it)._name.toLatin1());
        if (ss) {
          QString selectedScalar = ss->selectedScalar();

          ss->update();
          if (dpValid) {
            ss->setSelection(pp->inputScalars()[(*it)._name]->tag().displayString());
          } else {
            ss->setSelection(selectedScalar);
          }
        }
      }
    }

    //
    // update output vector and scalar lineedits...
    //

    if (dpValid) {
      const QList<Plugin::Data::IOValue>& otable = plugin->data()._outputs;
      QList<Plugin::Data::IOValue>::const_iterator it;

      for (it = otable.begin(); it != otable.end(); ++it) {
        QLineEdit *li;

        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          QString ts;

          if ((*it)._type == Plugin::Data::IOValue::TableType) {
            ts = pp->outputVectors()[(*it)._name]->tagName();
          } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
          } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
            ts = pp->outputStrings()[(*it)._name]->tagName();
          } else { // scalar
            ts = pp->outputScalars()[(*it)._name]->tagName();
          }
          li->setText(ts);
        }
      }
      pp->unlock();
    }
  } else { // invalid plugin
    PluginCollection *pc = PluginCollection::self();
    QString cur = _pluginList[_w->PluginCombo->currentIndex()];
    Plugin::Data pdata = pc->pluginList()[pc->pluginNameList()[cur]];
    QList<Plugin::Data::IOValue>::const_iterator it;

    for (it = pdata._outputs.begin(); it != pdata._outputs.end(); ++it) {
      QLineEdit *li;

      li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
      if (li) {
        li->setText(QString::null);
      }
    }
  }
}


void KstPluginDialog::update() {
  updatePluginList();
}


void KstPluginDialog::fixupLayout() {
  adjustSize();
  resize(650, minimumSizeHint().height());
  setFixedHeight(height());
}


QMap<QString,QString> KstPluginDialog::cacheInputs(const QLinkedList<Plugin::Data::IOValue>& table) {
  QMap<QString,QString> rc;
  QLinkedList<Plugin::Data::IOValue>::const_iterator it;
  
  for (it = table.begin(); it != table.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      VectorSelector *vs;

      vs = _w->_pluginInputOutputFrame->findChild<VectorSelector*>((*it)._name.toLatin1());
      if (vs) {
        rc[(*it)._name] = vs->selectedVector();
      } else {
        QLineEdit *li;

        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          rc[(*it)._name] = li->text();
        }
      }

    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      StringSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<StringSelector*>((*it)._name.toLatin1());
      if (ss) {
        rc[(*it)._name] = ss->selectedString();
      } else {
        QLineEdit *li;
        
        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          rc[(*it)._name] = li->text();
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      ScalarSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<ScalarSelector*>((*it)._name.toLatin1());
      if (ss) {
        rc[(*it)._name] = ss->selectedScalar();
      } else {
        QLineEdit *li;
        
        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          rc[(*it)._name] = li->text();
        }
      }
    }
  }

  return rc;
}


void KstPluginDialog::restoreInputs(const QLinkedList<Plugin::Data::IOValue>& table, const QMap<QString,QString>& v) {
  QLinkedList<Plugin::Data::IOValue>::const_iterator it;

  for (it = table.begin(); it != table.end(); ++it) {
    if (!v.contains((*it)._name)) {
      continue;
    }
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      VectorSelector *vs;

      vs = _w->_pluginInputOutputFrame->findChild<VectorSelector*>((*it)._name.toLatin1());
      if (vs) {
        vs->setSelection(v[(*it)._name]);
      } else {
        QLineEdit *li;
        
        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          li->setText(v[(*it)._name]);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      StringSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<StringSelector*>((*it)._name.toLatin1());
      if (ss) {
        ss->setSelection(v[(*it)._name]);
      } else {
        QLineEdit *li;
        
        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          li->setText(v[(*it)._name]);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      ScalarSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<ScalarSelector*>((*it)._name.toLatin1());
      if (ss) {
        ss->setSelection(v[(*it)._name]);
      } else {
        QLineEdit *li;
        
        li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
        if (li) {
          li->setText(v[(*it)._name]);
        }
      }
    }
  }
}


bool KstPluginDialog::saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p) {
  bool rc = true;
  const QList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  QList<Plugin::Data::IOValue>::const_iterator it;

  for (it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      VectorSelector *vs;

      vs = _w->_pluginInputOutputFrame->findChild<VectorSelector*>((*it)._name.toLatin1());
      if (vs) {
        KstReadLocker vl(&KST::vectorList.lock());
        KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());

        if (v) {
          plugin->inputVectors().insert((*it)._name, v);
        } else if (plugin->inputVectors().contains((*it)._name)) {
// xxx          plugin->inputVectors().erase((*it)._name);
          rc = false;
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      StringSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<StringSelector*>((*it)._name.toLatin1());
      if (ss) {
        KstWriteLocker sl(&KST::stringList.lock());
        KstStringPtr s = *KST::stringList.findTag(ss->selectedString());

        if (s == *KST::stringList.end()) {
          QString val = ss->_string->currentText();
          KstStringPtr newString;

          newString = new KstString(KstObjectTag::fromString(ss->_string->currentText()), 0L, val, true);
          if (!newString) {
            rc = false;
          }
          plugin->inputStrings().insert((*it)._name, newString);
        } else {
          plugin->inputStrings().insert((*it)._name, s);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      ScalarSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<ScalarSelector*>((*it)._name.toLatin1());
      if (ss) {
        KstWriteLocker sl(&KST::scalarList.lock());
        KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());

        if (s == *KST::scalarList.end()) {
          bool ok;
          double val = ss->_scalar->currentText().toDouble(&ok);
  
          if (ok) {
            KstScalarPtr newScalar;

            newScalar = new KstScalar(KstObjectTag::fromString(ss->_scalar->currentText()), 0L, val, true, false);
            plugin->inputScalars().insert((*it)._name, newScalar);
          } else {
            rc = false;
          }
        } else {
          plugin->inputScalars().insert((*it)._name, s);
        }
      }
    } else {
    }
  }

  return rc;
}


bool KstPluginDialog::saveOutputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p) {
  const QList<Plugin::Data::IOValue>& otable = p->data()._outputs;
  QList<Plugin::Data::IOValue>::const_iterator it;

  for (it = otable.begin(); it != otable.end(); ++it) {
    QLineEdit *li;
    QString nt;

    li = _w->_pluginInputOutputFrame->findChild<QLineEdit*>((*it)._name.toLatin1());
    if (li) {
      if (li->text().isEmpty()) {
        li->setText((*it)._name);
      }
  
      nt = li->text();
  
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        if (!KstData::self()->vectorTagNameNotUnique(nt, false)) {
          KstVectorPtr v;
  
          //
          // implicitly creates it if it doesn't exist...
          //
  
          v = plugin->outputVectors()[(*it)._name];
          if (!v) {
            KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
  
            v = new KstVector(KstObjectTag(nt, plugin->tag()), 0, plugin.data());
            plugin->outputVectors().insert((*it)._name, v);
          }
          v->setTagName(KstObjectTag(nt, plugin->tag()));
        } else if (plugin->outputVectors()[(*it)._name]->tagName() != nt) {
          KstVectorPtr v;
  
          while (KstData::self()->vectorTagNameNotUnique(nt, false)) {
            nt += "'";
          }
  
          if (plugin->outputVectors().contains((*it)._name)) {
            v = plugin->outputVectors()[(*it)._name];
          } else {
            KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
  
            v = new KstVector(KstObjectTag(nt, plugin->tag()), 0, plugin.data());
            plugin->outputVectors().insert((*it)._name, v);
          }
          v->setTagName(KstObjectTag(nt, plugin->tag()));
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        if (!KstData::self()->vectorTagNameNotUnique(nt, false)) {
          KstStringPtr s;
  
          if (plugin->outputStrings().contains((*it)._name)) {
            s = plugin->outputStrings()[(*it)._name];
          } else {
            KstWriteLocker blockStringUpdates(&KST::stringList.lock());
  
            s = new KstString(KstObjectTag(nt, plugin->tag()), plugin.data());
            plugin->outputStrings().insert((*it)._name, s);
          }
          s->setTagName(KstObjectTag(nt, plugin->tag()));
  
        } else if (plugin->outputStrings()[(*it)._name]->tagName() != nt) {
          KstStringPtr s;
  
          while (KstData::self()->vectorTagNameNotUnique(nt, false)) {
            nt += "'";
          }
  
          if (plugin->outputStrings().contains((*it)._name)) {
            s = plugin->outputStrings()[(*it)._name];
          } else {
            KstWriteLocker blockStringUpdates(&KST::stringList.lock());
  
            s = new KstString(KstObjectTag(nt, plugin->tag()), plugin.data());
            plugin->outputStrings().insert((*it)._name, s);
          }
          s->setTagName(KstObjectTag(nt, plugin->tag()));
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
        // Nothing
      } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
        if (!KstData::self()->vectorTagNameNotUnique(nt, false)) {
          KstScalarPtr s;
  
          if (plugin->outputScalars().contains((*it)._name)) {
            s = plugin->outputScalars()[(*it)._name];
          } else {
            KstWriteLocker blockScalarUpdates(&KST::scalarList.lock());
  
            s = new KstScalar(KstObjectTag(nt, plugin->tag()), plugin.data());
            plugin->outputScalars().insert((*it)._name, s);
          }
          s->setTagName(KstObjectTag(nt, plugin->tag()));
        } else if (plugin->outputScalars()[(*it)._name]->tagName() != nt) {
          KstScalarPtr s;
  
          while (KstData::self()->vectorTagNameNotUnique(nt, false)) {
            nt += "'";
          }
  
          if (plugin->outputScalars().contains((*it)._name)) {
            s = plugin->outputScalars()[(*it)._name];
          } else {
            KstWriteLocker blockScalarUpdates(&KST::scalarList.lock());
  
            s = new KstScalar(KstObjectTag(nt, plugin->tag()), plugin.data());
            plugin->outputScalars().insert((*it)._name, s);
          }
          s->setTagName(KstObjectTag(nt, plugin->tag()));
        }
      }
    }
  }

  return true;
}


bool KstPluginDialog::newObject() {
  KstCPluginPtr plugin;
  QString tagName = _tagName->text();
  int pitem;

  if (tagName != plugin_defaultTag && KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  }

  pitem = _w->PluginCombo->currentIndex();
  if (pitem >= 0 && _w->PluginCombo->count() > 0) {
    QExplicitlySharedDataPointer<Plugin> pPtr;
    
    pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);
    if (pPtr) {
      plugin = new KstCPlugin;
      KstWriteLocker pl(plugin.data());

      //
      // set the tag name before any dependents are created...
      //

      if (tagName == plugin_defaultTag) {
        tagName = KST::suggestPluginName(_pluginList[pitem]);
      }
      plugin->setTagName(KstObjectTag(tagName, KstObjectTag::globalTagContext));

      if (!saveInputs(plugin, pPtr)) {
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the inputs you entered."));
        plugin = 0L;

        return false;
      }

      plugin->setPlugin(pPtr);

      if (!saveOutputs(plugin, pPtr)) {
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the outputs you entered."));
        plugin = 0L;

        return false;
      }
    }
  }

  if (!plugin || !plugin->isValid()) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("There is an error in the plugin you entered."));

    return false;
  }

  plugin->setDirty();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(plugin);
  KST::dataObjectList.lock().unlock();
  plugin = 0L;

// xxx  emit modified();

  return true;
}


bool KstPluginDialog::editObject() {
  KstCPluginPtr pp;
  bool rc = false;

  pp = kst_cast<KstCPlugin>(_dp);
  if (pp) {
    QExplicitlySharedDataPointer<Plugin> pPtr;
    KstWriteLocker pl(pp.data());
    int pitem;

    if (_tagName->text() != pp->tagName() && KstData::self()->dataTagNameNotUnique(_tagName->text())) {
      _tagName->setFocus();
      return false;
    }
  
    pp->setTagName(_tagName->text());
  
    pitem = _w->PluginCombo->currentIndex();
    pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);
  
    pp->setRecursed(false);
    pp->inputVectors().clear();
    pp->inputScalars().clear();
    pp->inputStrings().clear();
  
    //
    // save the vectors and scalars...
    //
  
    if (!saveInputs(pp, pPtr)) {
      QMessageBox::warning(this, QObject::tr("Kst"),QObject::tr("There is an error in the inputs you entered."));
  
      return false;
    }
  
    if (pitem >= 0 && _w->PluginCombo->count() > 0) {
      pp->setPlugin(pPtr);
    }
  
    if (!saveOutputs(pp, pPtr)) {
      QMessageBox::warning(this, QObject::tr("Kst"),QObject::tr("There is an error in the outputs you entered."));
  
      return false;
    }
  
    if (!pp->isValid()) {
      QMessageBox::warning(this, QObject::tr("Kst"),QObject::tr("There is an error in the plugin you entered."));
  
      return false;
    }
  
    pp->setRecursed(false);
    if (pp->recursion()) {
      pp->setRecursed(true);
      QMessageBox::warning(this, QObject::tr("Kst"),QObject::tr("There is a recursion resulting from the plugin you entered."));
  
      return false;
    }
  
    pp->setDirty();
  
// xxx    emit modified();
  }

  return rc;
}


void KstPluginDialog::showNew(const QString &field) {
  _pluginName = field;
  _newDialog = true;
// xxx  init();
  KstDataDialog::showNew(field);
}


void KstPluginDialog::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QList<Plugin::Data::IOValue>& table) {
  QList<Plugin::Data::IOValue>::const_iterator it;
  QString scalarLabelTemplate;
  QString vectorLabelTemplate;
  QString stringLabelTemplate;

  if (input) {
    stringLabelTemplate = QObject::tr("Input string - %1:");
    scalarLabelTemplate = QObject::tr("Input scalar - %1:");
    vectorLabelTemplate = QObject::tr("Input vector - %1:");
  } else {
    stringLabelTemplate = QObject::tr("Output string - %1:");
    scalarLabelTemplate = QObject::tr("Output scalar - %1:");
    vectorLabelTemplate = QObject::tr("Output vector - %1:");
  }

  for (it = table.begin(); it != table.end(); ++it) {
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

    QLabel *label = 0L;
    QWidget *widget = 0L;
    
    label = new QLabel(labellabel, parent);
    label->setObjectName(input ? "Input label" : "Output label");

    if (input) {
      if (scalar) {
        KstScalarPtr p;
        ScalarSelector *w;

        w = new ScalarSelector(parent);
        w->setObjectName((*it)._name.toLatin1());
        widget = w;
        connect(w->_scalar, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
        connect(w->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
        connect(w->_scalar, SIGNAL(activated(const QString&)), this, SLOT(updateScalarTooltip(const QString&)));
        connect(widget, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_scalar->insertItem(0, (*it)._default);
          w->_scalar->setEditText((*it)._default);
        }
        p = *KST::scalarList.findTag(w->_scalar->currentText());
        w->allowDirectEntry(true);
        if (p) {
          p->readLock();
// xxx          QToolTip::remove(w->_scalar);
// xxx          QToolTip::add(w->_scalar, QString::number(p->value()));
          p->unlock();
        }
      } else if (string) {
        KstStringPtr p;
        StringSelector *w;

        w = new StringSelector(parent);
        w->setObjectName((*it)._name.toLatin1());
        widget = w;
        connect(w->_string, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
        connect(w->_string, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
        connect(w->_string, SIGNAL(activated(const QString&)), this, SLOT(updateStringTooltip(const QString&)));
        connect(widget, SIGNAL(newStringCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_string->insertItem(0, (*it)._default);
          w->_string->setEditText((*it)._default);
        }
        p = *KST::stringList.findTag(w->_string->currentText());
        w->allowDirectEntry(true);
        if (p) {
          p->readLock();
// xxx          QToolTip::remove(w->_string);
// xxx          QToolTip::add(w->_string, p->value());
          p->unlock();
        }
      } else {
        VectorSelector *vectorSelector = new VectorSelector(parent);

        vectorSelector->setObjectName((*it)._name.toLatin1());
        widget = dynamic_cast<QWidget*>(vectorSelector);
        if ((*it)._optional) {
          KstVectorPtr vector;

          vectorSelector->provideNoneVector(true);
          vectorSelector->setSelection(vector);
        }
        connect(vectorSelector->_vector, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
        connect(vectorSelector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
        connect(widget, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
      }
    } else {
      widget = new QLineEdit(parent);
      widget->setObjectName((*it)._name.toLatin1());
    }

    grid->addWidget(label, cnt, 0);
    _pluginWidgets.push_back(label);
    label->show();

    grid->addWidget(widget, cnt, 1);
    _pluginWidgets.push_back(widget);
    widget->show();

    if (!(*it)._description.isEmpty()) {
/* xxx
      QWhatsThis::remove(label);
      QWhatsThis::remove(widget);
      QWhatsThis::add(label, (*it)._description);
      QWhatsThis::add(widget, (*it)._description);
*/
    }

    ++cnt;
  }
}


void KstPluginDialog::pluginChanged(int idx) {
  //
  // find all children and delete them and the grids...
  //

  while (!_pluginWidgets.isEmpty()) {
    QWidget* tempWidget = _pluginWidgets.back();
    _pluginWidgets.pop_back();

    delete tempWidget;
  }

  delete _pluginInfoGrid;
  delete _pluginInputOutputGrid;

  //
  // create new info grid...
  //

  _pluginInfoGrid = new QGridLayout(_w->_pluginInfoFrame);
  _pluginInfoGrid->setContentsMargins(0, 0, 0, 0);
  _pluginInfoGrid->setSpacing(8);
  _pluginInfoGrid->setColumnStretch(1, 1); // stretch the right column
  _pluginInfoGrid->setColumnStretch(0, 0); // don't stretch the left column

  if (idx >= 0 && _w->PluginCombo->count() > 0) {
    const QString& pluginName = _pluginList[idx];
    const Plugin::Data& pluginData = PluginCollection::self()->pluginList()[PluginCollection::self()->pluginNameList()[pluginName]];

    //
    // setup the grid and the "static" entries...
    //

    QLabel* infoLabel;

    infoLabel = new QLabel(QObject::tr("Plugin name:"), _w->_pluginInfoFrame);
    _pluginInfoGrid->addWidget(infoLabel, 0, 0);
    _pluginWidgets.append(infoLabel);
    infoLabel->show();

    infoLabel = new QLabel(pluginData._readableName, _w->_pluginInfoFrame);
    _pluginInfoGrid->addWidget(infoLabel, 0, 1);
    _pluginWidgets.append(infoLabel);
    infoLabel->show();

    infoLabel = new QLabel(QObject::tr("Description:"), _w->_pluginInfoFrame);
    infoLabel->setAlignment(Qt::AlignTop);
    _pluginInfoGrid->addWidget(infoLabel, 1, 0);
    _pluginWidgets.append(infoLabel);
    infoLabel->show();

    infoLabel = new QLabel(pluginData._description,  _w->_pluginInfoFrame);
    _pluginInfoGrid->addWidget(infoLabel, 1, 1);
    _pluginWidgets.append(infoLabel);
    infoLabel->show();

    int cnt = 0;
    int numInputOutputs = pluginData._inputs.count() + pluginData._outputs.count();

    //
    // generate inputs...
    //

    _pluginInputOutputGrid = new QGridLayout(_w->_pluginInputOutputFrame);
    _pluginInfoGrid->setContentsMargins(0, 0, 0, 0);
    _pluginInfoGrid->setSpacing(8);
    _pluginInputOutputGrid->setColumnStretch(1, 1);
    _pluginInputOutputGrid->setColumnStretch(0, 0);
    generateEntries(true, cnt, _w->_pluginInputOutputFrame, _pluginInputOutputGrid, pluginData._inputs);

    //
    // insert separator...
    //

    cnt++;
    QFrame* line = new QFrame(_w->_pluginInputOutputFrame);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
// xxx    _pluginInputOutputGrid->addMultiCellWidget(line, cnt, cnt, 0, 1);
    _pluginWidgets.push_back(line);
    line->show();
    cnt++;

    //
    // generate outputs...
    //

    _pluginInputOutputGrid->setColumnStretch(1, 1);
    _pluginInputOutputGrid->setColumnStretch(0, 0);
    generateEntries(false, cnt, _w->_pluginInputOutputFrame, _pluginInputOutputGrid, pluginData._outputs);
  }

  fixupLayout();
}


void KstPluginDialog::showPluginManager() {
  KstPluginManager *pm = new KstPluginManager(this, "Plugin Manager");

  pm->exec();
  delete pm;
  updatePluginList();
}


void KstPluginDialog::updateScalarTooltip(const QString& n) {
  KstScalarPtr s = *KST::scalarList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));

  if (s) {
    s->readLock();
// xxx    QToolTip::remove(w);
// xxx    QToolTip::add(w, QString::number(s->value()));
    s->unlock();
  } else {
// xxx    QToolTip::remove(w);
  }
}


void KstPluginDialog::updateStringTooltip(const QString& n) {
  KstStringPtr s = *KST::stringList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));

  if (s) {
    s->readLock();
// xxx    QToolTip::remove(w);
// xxx    QToolTip::add(w, s->value());
    s->unlock();
  } else {
// xxx    QToolTip::remove(w);
  }
}

#include "kstplugindialog.moc"
