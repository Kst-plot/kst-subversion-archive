/***************************************************************************
                     kstfilterdialog_i.cpp  -  Part of KST
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2003 The University of Toronto
                           (C) 2004 The University of British Columbia
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
#include <qlineedit.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "curveappearancewidget.h"
#include "kstfilterdialog_i.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "plugincollection.h"
#include "pluginmanager.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"

QGuardedPtr<KstFilterDialogI> KstFilterDialogI::_inst;

KstFilterDialogI *KstFilterDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstFilterDialogI(KstApp::inst());
  }

  return _inst;
}


const QString newFilterPluginString = i18n("<Auto Name>");

KstFilterDialogI::KstFilterDialogI(QWidget* parent, const char* name,
                             bool modal, WFlags fl)
: KstFilterDialog(parent, name, modal, fl) {
  Init();

  connect(_OK, SIGNAL(clicked()), this, SLOT(OK()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(PluginCombo, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));
  connect(_pluginManager, SIGNAL(clicked()), this, SLOT(showPluginManager()));

  QGridLayout *grid = new QGridLayout(_pluginFrame, 1, 1);
  grid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
}


KstFilterDialogI::~KstFilterDialogI() {
}


void KstFilterDialogI::Init() {
}


void KstFilterDialogI::OK() {
  new_I();
  close();
}


void KstFilterDialogI::show_setCurve(const QString& curveName,
                                  const QString& plotName,
                                  const QString& window) {
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);

  _window   = window;
  _plotName = plotName;
  _curve    = curveName;

  // it should be impossible for the curve not to exist so this should
  // always be true.  If it is false, we do not properly take care of it, 
  // here and bad things will happen....
  KstBaseCurvePtr curve = *curves.findTag(curveName);
  if (curve) {
    curve->readLock();
    _xvector = curve->xVTag();
    _yvector = curve->yVTag();
    curve->readUnlock();
  }

  _tagName->setText(newFilterPluginString);

  updatePluginList();
  pluginChanged(PluginCombo->currentItem());

  show();
  raise();
}


void KstFilterDialogI::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();
  QString previous = _pluginList[PluginCombo->currentItem()];
  int newFocus = -1;
  int cnt = 0;

  PluginCombo->clear();
  _pluginList.clear();
  for (QMap<QString,Plugin::Data>::ConstIterator it = _pluginMap.begin(); it != _pluginMap.end(); ++it) {
    if (it.data()._isFilter) {
      _pluginList += it.data()._name;
      PluginCombo->insertItem(i18n("%1 (v%2) - %3").arg(it.data()._readableName)
                              .arg(it.data()._version)
                              .arg(it.data()._description));
      if (it.data()._name == previous) {
        newFocus = cnt;
      }
      ++cnt;
    }
  }

  if (newFocus != -1) {
    PluginCombo->setCurrentItem(newFocus);
  } else {
    PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


void KstFilterDialogI::update() {
  KstSharedPtr<Plugin> plugin = PluginCollection::self()->plugin(_pluginList[PluginCombo->currentItem()]);
  if (plugin) {
    const QValueList<Plugin::Data::IOValue>& itable = plugin->data()._inputs;
    for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
      if ((*it)._type == Plugin::Data::IOValue::TableType) {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          vs->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
        if (field) {
          StringSelector *ss = static_cast<StringSelector*>(field);
          ss->update();
        }
      } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
        // Nothing
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
        if (field) {
          ScalarSelector *ss = static_cast<ScalarSelector*>(field);
          ss->update();
        }
      }
    }
    pluginChanged(0);
  }
}


void KstFilterDialogI::fixupLayout() {
  _frameWidget->updateGeometry();
  _pluginFrame->updateGeometry();
  resize(sizeHint());
  setMinimumSize(size());
  updateGeometry();
}


bool KstFilterDialogI::saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  KST::vectorList.lock().readLock();
  KST::scalarList.lock().readLock();
  KST::stringList.lock().readLock();
  const QValueList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if ((*it)._name == p->data()._filterInputVector) {
        KstVectorPtr v = *KST::vectorList.findTag(_yvector);
        if (v) {
          v->writeLock(); // to match with plugin->writeLock()
        }
        if (plugin->inputVectors().contains((*it)._name) && plugin->inputVectors()[(*it)._name] != v) {
          plugin->inputVectors()[(*it)._name]->writeUnlock();
        }
        plugin->inputVectors().insert((*it)._name, v);
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());
          if (v) {
            v->writeLock(); // to match with plugin->writeLock()
          }
          if (plugin->inputVectors().contains((*it)._name) && plugin->inputVectors()[(*it)._name] != v) {
            plugin->inputVectors()[(*it)._name]->writeUnlock();
          }
          plugin->inputVectors().insert((*it)._name, v);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "StringSelector");
      if (field) {
        StringSelector *ss = static_cast<StringSelector*>(field);
        KstStringPtr s = *KST::stringList.findTag(ss->selectedString());
        if (s == *KST::stringList.end()) {
          QString val = ss->_string->currentText();
          KstStringPtr newString = new KstString(ss->_string->currentText(), val, true, false);
          newString->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputStrings().contains((*it)._name) && plugin->inputStrings()[(*it)._name] != newString) {
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
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else {
      QObject *field = _frameWidget->child((*it)._name.latin1(), "ScalarSelector");
      if (field) {
        ScalarSelector *ss = static_cast<ScalarSelector*>(field);
        KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());
        if (s == *KST::scalarList.end()) {
          bool ok;
          double val = ss->_scalar->currentText().toDouble(&ok);

          if (ok) {
            KstScalarPtr newScalar = new KstScalar(ss->_scalar->currentText(), val, true, false, false);
            newScalar->writeLock(); // to match with plugin->writeLock()
            if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != newScalar) {
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
      }
    }
  }
  KST::stringList.lock().readUnlock();
  KST::scalarList.lock().readUnlock();
  KST::vectorList.lock().readUnlock();

  return true;
}


bool KstFilterDialogI::saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  const QValueList<Plugin::Data::IOValue>& otable = p->data()._outputs;

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
    QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
    if (!field) {
      continue;
    }

    QLineEdit *li = static_cast<QLineEdit*>(field);

    if (li->text().isEmpty()) {
      QString tagName = _tagName->text();
      if (tagName == newFilterPluginString) {
        tagName = plugin->tagName();
      }
      li->setText(tagName); // this will be the name of the output vec
    }

    QString nt = KST::suggestVectorName(li->text());
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      KstVectorPtr v = plugin->outputVectors()[(*it)._name];
      if (!v) {
        v = new KstVector;
        v->writeLock();
        plugin->outputVectors().insert((*it)._name, v);
        KST::addVectorToList(v);
      }
      v->setTagName(nt);
      v->setProvider(plugin.data());
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      KstScalarPtr s = plugin->outputScalars()[(*it)._name];
      s->writeLock();
      s->setTagName(nt);
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
        KstStringPtr s = plugin->outputStrings()[(*it)._name];
        s->writeLock();
        s->setTagName(nt);
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    }
  }


  return true;
}


bool KstFilterDialogI::createCurve(KstPluginPtr plugin) {
  KstVectorPtr xVector;
  KstVectorPtr yVector;
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  bool rc = false;

  // Generate new fit name
  QString c_name = KST::suggestCurveName(plugin->tagName(), true);

  KstVectorList::Iterator it = KST::vectorList.findTag(_xvector);
  if (it != KST::vectorList.end()) {
    xVector = *it;
  }

  if (plugin->outputVectors().contains(plugin->plugin()->data()._filterOutputVector)) {
    yVector = plugin->outputVectors()[plugin->plugin()->data()._filterOutputVector];
  }

  if (xVector && yVector) {
    plugin->setDirty();
    KstVCurvePtr fit = new KstVCurve(c_name, KstVectorPtr(xVector), KstVectorPtr(yVector), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), _curveAppearance->color());

    fit->setHasPoints(_curveAppearance->showPoints());
    fit->setHasLines(_curveAppearance->showLines());
    fit->setHasBars(_curveAppearance->showBars());
    fit->setLineWidth(_curveAppearance->lineWidth());
    fit->setLineStyle(_curveAppearance->lineStyle());
    fit->Point.setType(_curveAppearance->pointType());
    fit->setBarStyle(_curveAppearance->barStyle());
    fit->setPointDensity(_curveAppearance->pointDensity());

    //
    // add the curve to the plot...
    //
    KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_window));
    if (w && w->view()->findChild(_plotName)) {
      Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(w->view()->findChild(_plotName));
      if (plot) {
        plot->addCurve(fit.data());
        w->view()->paint(P_PAINT);
      }
    }

    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(fit.data());
    KST::dataObjectList.lock().writeUnlock();

    rc = true;
  }

  return rc;
}


bool KstFilterDialogI::new_I() {
  QString tagName = _tagName->text();

  if (KST::dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  } else {
    int pitem = PluginCombo->currentItem();

    if (pitem >= 0 && PluginCombo->count() > 0) {
      KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

      if (pPtr) {
        KstPluginPtr plugin = new KstPlugin;
        plugin->writeLock();
        plugin->setDirty();
        if (saveInputs(plugin, pPtr)) {
          plugin->setPlugin(pPtr);

          if (tagName == newFilterPluginString) {
            tagName = KST::suggestPluginName(_pluginList[pitem], _yvector);
          }

          plugin->setTagName(tagName);
          if (saveOutputs(plugin, pPtr)) {
            if (plugin->isValid()) {
              if (!createCurve(plugin)) {
                KMessageBox::sorry(this, i18n("There is an error in the plugin you entered. Please fix it."));
              } else {
                KST::dataObjectList.lock().writeLock();
                KST::dataObjectList.append(plugin.data());
                KST::dataObjectList.lock().writeUnlock();
              }
            } else {
              KMessageBox::sorry(this, i18n("There is an error in the plugin you entered. Please fix it."));
            }
          }
        }
        plugin->writeUnlock();
      }
    }
    emit modified();
  }
  return true;
}


void KstFilterDialogI::generateEntries(const QString& fixedVector, bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table) {
  QString scalarLabelTemplate, vectorLabelTemplate, stringLabelTemplate;

  if (input) {
    stringLabelTemplate = i18n("Input String - %1:");
    scalarLabelTemplate = i18n("Input Scalar - %1:");
    vectorLabelTemplate = i18n("Input Vector - %1:");
  } else {
    stringLabelTemplate = i18n("Output String - %1:");
    scalarLabelTemplate = i18n("Output Scalar - %1:");
    vectorLabelTemplate = i18n("Output Vector - %1:");
  }

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = table.begin(); it != table.end(); ++it) {
    QString labellabel;
    bool string = false;
    bool scalar = false;
    bool fixed = false;
    switch ((*it)._type) {
      case Plugin::Data::IOValue::PidType:
        continue;
      case Plugin::Data::IOValue::FloatType:
        labellabel = scalarLabelTemplate.arg((*it)._name);
        scalar = true;
        break;
      case Plugin::Data::IOValue::StringType:
        labellabel = stringLabelTemplate.arg((*it)._name);
        string = true;
        break;
      case Plugin::Data::IOValue::TableType:
        if ((*it)._name == fixedVector) {
          fixed = true;
        }
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
          //printf("default: |%s|\n", (*it)._default.latin1());
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
        if (fixed) {
          widget = new QLabel(parent, (*it)._name.latin1());
          static_cast<QLabel*>(widget)->setText(_yvector);
        } else {
          widget = new VectorSelector(parent, (*it)._name.latin1());
          connect(widget, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
        }
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

    ++cnt;
  }
}


void KstFilterDialogI::pluginChanged(int idx) {
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
    int cnt = 0;

    // Setup the grid and the "static" entries
    int variables = pluginData._inputs.count() + pluginData._outputs.count();
    QGridLayout *grid = new QGridLayout(_frameWidget, 4+variables, 2);
    grid->setMargin(6);
    grid->setSpacing(5);
    grid->addWidget(new QLabel(i18n("Plugin name:"), _frameWidget), cnt, 0);
    grid->addWidget(new QLabel(pluginData._readableName, _frameWidget), cnt, 1);
    grid->addWidget(new QLabel(i18n("Description:"), _frameWidget), ++cnt, 0);
    QLabel *lab = new QLabel(pluginData._description, _frameWidget);
    lab->setAlignment(lab->alignment() | Qt::WordBreak);
    grid->addWidget(lab, cnt, 1);

    // Add a separator
    QFrame* line = new QFrame(_frameWidget);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    ++cnt;
    grid->addMultiCellWidget(line, cnt, cnt, 0, 1);

    // Generate the input values
    ++cnt;
    generateEntries(pluginData._filterInputVector, true, cnt, _frameWidget, grid, pluginData._inputs);

    if (!pluginData._inputs.isEmpty() && !pluginData._outputs.isEmpty()) {
      line = new QFrame(_frameWidget);
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFrameShape(QFrame::HLine);
      ++cnt;
      grid->addMultiCellWidget(line, cnt, cnt, 0, 1);
    }

    // Generate the output values
    ++cnt;
    generateEntries(pluginData._filterOutputVector, false, cnt, _frameWidget, grid, pluginData._outputs);
  }

  // show it
  _frameWidget->show();

  // resize everything
  QTimer::singleShot(0, this, SLOT(fixupLayout()));
}


void KstFilterDialogI::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;
  updatePluginList();
}


void KstFilterDialogI::updateScalarTooltip(const QString& n) {
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


void KstFilterDialogI::updateStringTooltip(const QString& n) {
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


#include "kstfilterdialog_i.moc"

// vim: ts=2 sw=2 et
