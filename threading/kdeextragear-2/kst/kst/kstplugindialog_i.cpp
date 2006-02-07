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

#include "kstplugindialog_i.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qobjectlist.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <qlineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcolorbutton.h>
#include <kcolordialog.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include "plugincollection.h"
#include "plugin.h"
#include "kstdoc.h"
#include "scalarselector.h"
#include "vectorselector.h"
#include "kstdatacollection.h"
#include "pluginmanager.h"

#include <assert.h>
#include <stdlib.h>


KstPluginDialogI *KstPluginDialogI::_inst = 0L;
static KStaticDeleter<KstPluginDialogI> _plInst;

KstPluginDialogI *KstPluginDialogI::globalInstance() {
  if (!_inst) {
    _inst = _plInst.setObject(new KstPluginDialogI);
  }
return _inst;
}


const QString newPluginString = i18n("<New_Plugin>");

KstPluginDialogI::KstPluginDialogI(QWidget* parent, const char* name,
                                   bool modal, WFlags fl)
: KstPluginDialog(parent, name, modal, fl) {
  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(PluginCombo, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));
  connect(_pluginManager, SIGNAL(clicked()), this, SLOT(showPluginManager()));

  QGridLayout *grid = new QGridLayout(_pluginFrame, 1, 1);
  grid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
}


KstPluginDialogI::~KstPluginDialogI() {
}


void KstPluginDialogI::show_I(const QString &field) {
  int new_index = -1;

  KstPluginList plugins = kstObjectSubList<KstDataObject, KstPlugin>(KST::dataObjectList);
  new_index = plugins.findIndexTag(field);

  updatePluginList();
  update(new_index);
  show();
  raise();
}


void KstPluginDialogI::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  QString previous = _pluginList[PluginCombo->currentItem()];
  int newFocus = -1;

  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();

  PluginCombo->clear();
  _pluginList.clear();
  int cnt = 0;
  for (QMap<QString,Plugin::Data>::ConstIterator it = _pluginMap.begin();
                                                  it != _pluginMap.end();
                                                                     ++it) {
    if (it.data()._filter == false) {
      _pluginList += it.data()._name;
      PluginCombo->insertItem(i18n("%1 (v%2) - %3").arg(it.data()._name)
                              .arg(it.data()._version)
                              .arg(it.data()._description));
      if (it.data()._name == previous) {
        newFocus = cnt;
      }
      cnt++;
    }
  }

  if (newFocus != -1) {
    PluginCombo->setCurrentItem(newFocus);
  } else {
    PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


void KstPluginDialogI::show_New() {
  updatePluginList();
  update(-2);
  show();
  raise();
}


void KstPluginDialogI::update(int new_index) {
  int oldSelect = Select->currentItem();
  int oldCount = Select->count();

  KstPluginList pl = kstObjectSubList<KstDataObject, KstPlugin>(KST::dataObjectList);

  Select->clear();

  for (KstPluginList::iterator it = pl.begin(); it != pl.end(); ++it) {
    Select->insertItem((*it)->tagName());
  }

  if (new_index == -2 || Select->count() == 0) {
    Select->insertItem(i18n("P%1-%2").arg(Select->count()+1).arg(newPluginString));
    Select->setCurrentItem(Select->count()-1);
  } else if (new_index >= 0 && new_index < Select->count()) {
    Select->setCurrentItem(new_index);
  } else if (oldCount != Select->count()) {
    Select->setCurrentItem(Select->count()-1);
  } else {
    Select->setCurrentItem(oldSelect < Select->count() ? oldSelect : Select->count()-1);
  }

  if (_pluginList.isEmpty()) {
    updatePluginList();
  }

  int oldPluginIndex = PluginCombo->currentItem();

  KstPluginPtr plugin = pl[Select->currentItem()];
  if (plugin.data() && plugin->isValid()) {
    int newPluginIndex = _pluginList.findIndex(plugin->plugin()->data()._name);
    bool updateCombos = (newPluginIndex != oldPluginIndex);

    if (updateCombos) {
      PluginCombo->setCurrentItem(newPluginIndex);
      pluginChanged(PluginCombo->currentItem());
    }

    // Update input vector and scalar combos
    const QValueList<Plugin::Data::IOValue>& itable = plugin->plugin()->data()._inputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        assert(field);
        VectorSelector *vs = static_cast<VectorSelector*>(field);
        if (!updateCombos) {
          vs->update();
        }
        vs->setSelection(plugin->inputVectors()[(*it)._name]->tagName());
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
        assert(field);
        ScalarSelector *ss = static_cast<ScalarSelector*>(field);
        if (!updateCombos) {
          ss->update();
        }
        ss->setSelection(plugin->inputScalars()[(*it)._name]->tagName());
      }
    }

    // Update output vector and scalar lineedits
    const QValueList<Plugin::Data::IOValue>& otable = plugin->plugin()->data()._outputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
      assert(field);

      QLineEdit *li = static_cast<QLineEdit*>(field);
      if ((*it)._type == Plugin::Data::IOValue::TableType) { // vector
        li->setText(plugin->outputVectors()[(*it)._name]->tagName());
      } else { // scalar
        li->setText(plugin->outputScalars()[(*it)._name]->tagName());
      }
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

  _pluginFrame->resize(_pluginFrame->minimumSizeHint());
  resize(minimumSizeHint());
}


bool KstPluginDialogI::saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  KST::vectorList.lock().readLock();
  KST::scalarList.lock().readLock();
  const QValueList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
      assert(field);
      VectorSelector *vs = static_cast<VectorSelector*>(field);
      KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());
      plugin->inputVectors().insert((*it)._name, v);
    } else {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
      assert(field);
      ScalarSelector *ss = static_cast<ScalarSelector*>(field);
      KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());
      plugin->inputScalars().insert((*it)._name, s);
    }
  }
  KST::scalarList.lock().readUnlock();
  KST::vectorList.lock().readUnlock();
return true;
}


bool KstPluginDialogI::saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  const QValueList<Plugin::Data::IOValue>& otable = p->data()._outputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
    QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
    assert(field);

    QLineEdit *li = static_cast<QLineEdit*>(field);

    if (li->text().isEmpty()) {
      return false;
    }

    QString nt = li->text();
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        plugin->outputVectors()[(*it)._name]->setTagName(nt);
      } else if (plugin->outputVectors()[(*it)._name]->tagName() != nt) {
          while (KST::vectorTagNameNotUnique(nt, false)) {
            nt += "'";
          }
          plugin->outputVectors()[(*it)._name]->setTagName(nt);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        plugin->outputScalars()[(*it)._name]->setTagName(nt);
      } else if (plugin->outputScalars()[(*it)._name]->tagName() != nt) {
          while (KST::vectorTagNameNotUnique(nt, false)) {
            nt += "'";
          }
          plugin->outputScalars()[(*it)._name]->setTagName(nt);
      } 
    }
  }

return true;
}


void KstPluginDialogI::new_I() {
  KstPluginPtr plugin;
  QString tagName = Select->currentText();

  if (KST::dataTagNameNotUnique(tagName)) {
    return;
  }

  int pitem = PluginCombo->currentItem();
  if (pitem >= 0 && PluginCombo->count() > 0) {
    KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);
    plugin = new KstPlugin;
    if (!saveInputs(plugin, pPtr)) {
      return;
    }

    plugin->setPlugin(pPtr);

    if (!saveOutputs(plugin, pPtr)) {
      return;
    }

    if (tagName.endsWith(newPluginString)) {
      QString tmpTagName;
      int i = 0;
      do {
        tmpTagName = i18n("P%1-%2").arg(++i).arg(_pluginList[pitem]);
      } while (KST::dataTagNameNotUnique(tmpTagName, false));
      tagName = tmpTagName;
    }
    plugin->setTagName(tagName);
  }

  if (!plugin || !plugin->isValid()) {
    KMessageBox::sorry(0L, i18n("There is an error in the plugin you entered. Please fix it."));
    return;
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(plugin.data());
  KST::dataObjectList.lock().writeUnlock();
  plugin = 0L;
  emit modified();
}


void KstPluginDialogI::edit_I() {
  int index = Select->currentItem();

  if (Select->count() == 0 || index < 0) {
    new_I();
    return;
  }

  QString tagName = Select->currentText();
  KstPluginList pl = kstObjectSubList<KstDataObject, KstPlugin>(KST::dataObjectList);
  KstPluginPtr plugin = pl[index];

  if (!plugin.data()) {
    KMessageBox::sorry(0L, i18n("The plugin entry has disappeared. Your changes have not been saved."));
    return;
  }

  //kdDebug() << "Apply as edit [" << tagName << "]" << endl;

  if (tagName != plugin->tagName()) {
    if (KST::dataTagNameNotUnique(tagName)) {
      KMessageBox::sorry(0L, i18n("You must choose a unique name for this entry. Your changes have not been saved."));
      return;
    }
  }

  plugin->setTagName(tagName);

  plugin->inputVectors().clear();
  plugin->inputScalars().clear();

  int pitem = PluginCombo->currentItem();
  KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

  // Save the vectors and scalars
  if (!saveInputs(plugin, pPtr)) {
    return;
  }

  if (pitem >= 0 && PluginCombo->count() > 0) {
    plugin->setPlugin(pPtr);
  }

  if (!saveOutputs(plugin, pPtr)) {
    return;
  }

  if (!plugin->isValid()) {
    KMessageBox::sorry(0L, i18n("There is an error in the plugin you entered. Please fix it."));
    return;
  }

  plugin = 0L;
  emit modified();
}


void KstPluginDialogI::delete_I() {
  QString tag = Select->currentText();

  if (tag.isEmpty()) {
    KMessageBox::sorry(0L, i18n("You need to select an active plugin to delete."));
  } else {
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.removeTag(tag);
    KST::dataObjectList.lock().writeUnlock();
  }

  emit modified();
}


void KstPluginDialogI::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table) {
QString scalarLabelTemplate, vectorLabelTemplate;

  if (input) {
    scalarLabelTemplate = i18n("Input Scalar - %1:");
    vectorLabelTemplate = i18n("Input Vector - %1:");
  } else {
    scalarLabelTemplate = i18n("Output Scalar - %1:");
    vectorLabelTemplate = i18n("Output Vector - %1:");
  }

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    QString labellabel;
    bool scalar = false;
    switch ((*it)._type) {
      case Plugin::Data::IOValue::FloatType:
        labellabel = scalarLabelTemplate.arg((*it)._name);
        scalar = true;
        break;
      case Plugin::Data::IOValue::TableType:
        if ((*it)._subType == Plugin::Data::IOValue::FloatSubType) {
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
        widget = new ScalarSelector(parent, (*it)._name.latin1());
        connect(widget, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
      } else {
        widget = new VectorSelector(parent, (*it)._name.latin1());
        connect(widget, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
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
    topGrid->invalidate();
    topGrid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
  } else {
    kdError() << "Somehow we lost the grid!" << endl;
    return;
  }

  // Refill it
  if (idx >= 0 && PluginCombo->count() > 0) {
    const QString& pluginName = _pluginList[idx];
    const Plugin::Data& pluginData = PluginCollection::self()->pluginList()[PluginCollection::self()->pluginNameList()[pluginName]];

    // Setup the grid and the "static" entries
    int variables = pluginData._inputs.count() + pluginData._outputs.count();
    QGridLayout *grid = new QGridLayout(_frameWidget, 4+variables, 2);
    grid->setMargin(6);
    grid->setSpacing(5);
    grid->addWidget(new QLabel("Plugin Name:", _frameWidget), 0, 0);
    grid->addWidget(new QLabel(pluginData._name, _frameWidget), 0, 1);
    grid->addWidget(new QLabel("Description:", _frameWidget), 1, 0);
    grid->addWidget(new QLabel(pluginData._description, _frameWidget), 1, 1);

    // Add a separator
    QFrame* line = new QFrame(_frameWidget);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    grid->addMultiCellWidget(line, 2, 2, 0, 1);

    int cnt = 3;

    // Generate the input values
    generateEntries(true, cnt, _frameWidget, grid, pluginData._inputs);

    if (!pluginData._inputs.isEmpty() && !pluginData._outputs.isEmpty()) {
      // Add a separator
      line = new QFrame(_frameWidget);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFrameShape(QFrame::HLine);
      grid->addWidget(line, cnt, 0);
      cnt++;
    }

    // Generate the output values
    generateEntries(false, cnt, _frameWidget, grid, pluginData._outputs);
  }

  // show it
  _frameWidget->show();

  // resize everything
  _frameWidget->resize(_frameWidget->minimumSizeHint());
  _pluginFrame->resize(_pluginFrame->minimumSizeHint());
  resize(minimumSizeHint());
}


void KstPluginDialogI::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;
  updatePluginList();
}

#include "kstplugindialog_i.moc"

// vim: ts=2 sw=2 et
