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
#include <kmessagebox.h>

// application specific includes
#include "curveappearancewidget.h"
#include "kst2dplot.h"
#include "kstdataobjectcollection.h"
#include "kstfilterdialog_i.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "plugincollection.h"
#include "plugindialogwidget.h"
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


KstFilterDialogI::KstFilterDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstPluginDialogI(parent, name, modal, fl) {
  _w->_curveAppearance->show();
}


KstFilterDialogI::~KstFilterDialogI() {
}


void KstFilterDialogI::show_setCurve(const QString& curveName,
                                  const QString& plotName,
                                  const QString& window) {

  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  KstVCurveList vcurves = kstObjectSubList<KstBaseCurve, KstVCurve>(curves);
  _window   = window;
  _plotName = plotName;
  _curve    = curveName;

  // it should be impossible for the curve not to exist so this should
  // always be true.  If it is false, we do not properly take care of it,
  // here and bad things will happen....
  KstVCurvePtr curve = *vcurves.findTag(curveName);
  if (curve) {
    curve->readLock();
    _xvector = curve->xVTag();
    _yvector = curve->yVTag();
    curve->unlock();
  }
  show();
}


void KstFilterDialogI::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();
  QString previous = _pluginList[_w->PluginCombo->currentItem()];
  int newFocus = -1;
  int cnt = 0;

  _w->PluginCombo->clear();
  _pluginList.clear();
  for (QMap<QString,Plugin::Data>::ConstIterator it = _pluginMap.begin(); it != _pluginMap.end(); ++it) {
    if (it.data()._isFilter) {
      _pluginList += it.data()._name;
      _w->PluginCombo->insertItem(i18n("%1 (v%2) - %3").arg(it.data()._readableName)
                              .arg(it.data()._version)
                              .arg(it.data()._description));
      if (it.data()._name == previous) {
        newFocus = cnt;
      }
      ++cnt;
    }
  }

  if (newFocus != -1) {
    _w->PluginCombo->setCurrentItem(newFocus);
  } else {
    _w->PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


bool KstFilterDialogI::saveInputs(KstCPluginPtr plugin, KstSharedPtr<Plugin> p) {
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
          plugin->inputVectors()[(*it)._name]->unlock();
        }
        plugin->inputVectors().insert((*it)._name, v);
      } else {
        QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.latin1(), "VectorSelector");
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());
          if (v) {
            v->writeLock(); // to match with plugin->writeLock()
          }
          if (plugin->inputVectors().contains((*it)._name) && plugin->inputVectors()[(*it)._name] != v) {
            plugin->inputVectors()[(*it)._name]->unlock();
          }
          plugin->inputVectors().insert((*it)._name, v);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.latin1(), "StringSelector");
      if (field) {
        StringSelector *ss = static_cast<StringSelector*>(field);
        KstStringPtr s = *KST::stringList.findTag(ss->selectedString());
        if (s == *KST::stringList.end()) {
          QString val = ss->_string->currentText();
          KstStringPtr newString = new KstString(ss->_string->currentText(), 0L, val, true, false);
          newString->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputStrings().contains((*it)._name) && plugin->inputStrings()[(*it)._name] != newString) {
            plugin->inputStrings()[(*it)._name]->unlock();
          }
          plugin->inputStrings().insert((*it)._name, newString);
        } else {
          s->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputStrings().contains((*it)._name) && plugin->inputStrings()[(*it)._name] != s) {
            plugin->inputStrings()[(*it)._name]->unlock();
          }
          plugin->inputStrings().insert((*it)._name, s);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else {
      QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.latin1(), "ScalarSelector");
      if (field) {
        ScalarSelector *ss = static_cast<ScalarSelector*>(field);
        KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());
        if (s == *KST::scalarList.end()) {
          bool ok;
          double val = ss->_scalar->currentText().toDouble(&ok);

          if (ok) {
            KstScalarPtr newScalar = new KstScalar(ss->_scalar->currentText(), 0L, val, true, false, false);
            newScalar->writeLock(); // to match with plugin->writeLock()
            if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != newScalar) {
              plugin->inputScalars()[(*it)._name]->unlock();
            }
            plugin->inputScalars().insert((*it)._name, newScalar);
          } else {
            s->writeLock(); // to match with plugin->writeLock()
            if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != s) {
              plugin->inputScalars()[(*it)._name]->unlock();
            }
            plugin->inputScalars().insert((*it)._name, s);
          }
        } else {
          s->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputScalars().contains((*it)._name) && plugin->inputScalars()[(*it)._name] != s) {
            plugin->inputScalars()[(*it)._name]->unlock();
          }
          plugin->inputScalars().insert((*it)._name, s);
        }
      }
    }
  }
  KST::stringList.lock().unlock();
  KST::scalarList.lock().unlock();
  KST::vectorList.lock().unlock();

  return true;
}


bool KstFilterDialogI::createCurve(KstCPluginPtr plugin) {
  KstVectorPtr xVector;
  KstVectorPtr yVector;

  KST::vectorList.lock().readLock();
  KstVectorList::Iterator it = KST::vectorList.findTag(_xvector);
  if (it != KST::vectorList.end()) {
    xVector = *it;
  }
  KST::vectorList.lock().unlock();

  if (plugin->outputVectors().contains(plugin->plugin()->data()._filterOutputVector)) {
    yVector = plugin->outputVectors()[plugin->plugin()->data()._filterOutputVector];
  }

  if (!xVector || !yVector) {
    return false;
  }
  plugin->setDirty();
  QString c_name = KST::suggestCurveName(plugin->tagName(), true);
  KstVCurvePtr fit = new KstVCurve(c_name, KstVectorPtr(xVector), KstVectorPtr(yVector), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), _w->_curveAppearance->color());

  fit->setHasPoints(_w->_curveAppearance->showPoints());
  fit->setHasLines(_w->_curveAppearance->showLines());
  fit->setHasBars(_w->_curveAppearance->showBars());
  fit->setLineWidth(_w->_curveAppearance->lineWidth());
  fit->setLineStyle(_w->_curveAppearance->lineStyle());
  fit->pointType = _w->_curveAppearance->pointType();
  fit->setBarStyle(_w->_curveAppearance->barStyle());
  fit->setPointDensity(_w->_curveAppearance->pointDensity());

  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_window));
  if (w && w->view()->findChild(_plotName)) {
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(w->view()->findChild(_plotName));
    if (plot) {
      plot->addCurve(fit.data());
    }
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(fit.data());
  KST::dataObjectList.lock().unlock();
  return true;
}


bool KstFilterDialogI::newObject() {
  QString tagName = _tagName->text();

  if (KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  } else {
    int pitem = _w->PluginCombo->currentItem();

    if (pitem >= 0 && _w->PluginCombo->count() > 0) {
      KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

      if (pPtr) {
        KstCPluginPtr plugin = new KstCPlugin;
        plugin->writeLock();
        plugin->setDirty();
        if (saveInputs(plugin, pPtr)) {
          plugin->setPlugin(pPtr);

          if (tagName == plugin_defaultTag) {
            tagName = KST::suggestPluginName(_pluginList[pitem], _yvector);
          }

          plugin->setTagName(tagName);
          if (saveOutputs(plugin, pPtr)) {
            if (plugin->isValid()) {
              if (!createCurve(plugin)) {
                KMessageBox::sorry(this, i18n("There is an error in the plugin you entered."));
              } else {
                KST::dataObjectList.lock().writeLock();
                KST::dataObjectList.append(plugin.data());
                KST::dataObjectList.lock().unlock();
              }
            } else {
              KMessageBox::sorry(this, i18n("There is an error in the plugin you entered."));
            }
          }
        }
        plugin->unlock();
      }
    }
    emit modified();
  }
  return true;
}


void KstFilterDialogI::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table) {

  // get fixed vector for filter
  QString fixedVector;
  const QString& pluginName = _pluginList[_w->PluginCombo->currentItem()];
  const Plugin::Data& pluginData = PluginCollection::self()->pluginList()[PluginCollection::self()->pluginNameList()[pluginName]];
  if (input) {
    fixedVector = pluginData._filterInputVector;
  } else {
    fixedVector = pluginData._filterOutputVector;
  }

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
          QToolTip::remove(w->_scalar);
          QToolTip::add(w->_scalar, QString::number(p->value()));
          p->unlock();
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
          QToolTip::remove(w->_string);
          QToolTip::add(w->_string, p->value());
          p->unlock();
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
    _pluginWidgets.push_back(label);
    label->show();

    grid->addWidget(widget, cnt, 1);
    _pluginWidgets.push_back(widget);
    widget->show();

    if (!(*it)._description.isEmpty()) {
      QWhatsThis::remove(label);
      QWhatsThis::remove(widget);
      QWhatsThis::add(label, (*it)._description);
      QWhatsThis::add(widget, (*it)._description);
    }

    ++cnt;
  }
}


#include "kstfilterdialog_i.moc"

// vim: ts=2 sw=2 et
