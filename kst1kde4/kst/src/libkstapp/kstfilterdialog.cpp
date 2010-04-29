/***************************************************************************
                     kstfilterdialog.cpp  -  Part of KST
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

#include <QLineEdit>

#include "curveappearancewidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog.h"
#include "kstdataobjectcollection.h"
#include "kstfilterdialog.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "plugincollection.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"
#include "ui_plugindialogwidget.h"
#include "ui_pluginmanager.h"

QPointer<KstFilterDialog> KstFilterDialog::_inst;

KstFilterDialog *KstFilterDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstFilterDialog(KstApp::inst());
  }

  return _inst;
}


KstFilterDialog::KstFilterDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : KstPluginDialog(parent, name, modal, fl) {
  _w->_curveAppearance->show();
}


KstFilterDialog::~KstFilterDialog() {
}


void KstFilterDialog::show_setCurve(const QString& curveName, const QString& plotName, const QString& window) {
  KstBaseCurveList curves;
  KstVCurveList vcurves;
  KstVCurvePtr curve;
  
  curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  vcurves = kstObjectSubList<KstBaseCurve, KstVCurve>(curves);

  _window   = window;
  _plotName = plotName;
  _curve    = curveName;

  //
  // it should be impossible for the curve not to exist so this should
  // always be true.  If it is false, we do not properly take care of it,
  // here and bad things will happen....
  //

  curve = *vcurves.findTag(curveName);
  if (curve) {
    curve->readLock();
    _xvector = curve->xVTag().displayString();
    _yvector = curve->yVTag().displayString();
    curve->unlock();
  }
  show();
}


void KstFilterDialog::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();
  QMap<QString,Plugin::Data>::const_iterator it;
  QString previous = _pluginList[_w->PluginCombo->currentIndex()];
  int newFocus = -1;
  int cnt = 0;

  _w->PluginCombo->clear();
  _pluginList.clear();
  for (it = _pluginMap.begin(); it != _pluginMap.end(); ++it) {
    if ((*it)._isFilter) {
      _pluginList += (*it)._name;
      _w->PluginCombo->insertItem(0, i18n("%1 (v%2) - %3").arg((*it)._readableName)
                              .arg((*it)._version)
                              .arg((*it)._description));
      if ((*it)._name == previous) {
        newFocus = cnt;
      }
      ++cnt;
    }
  }

  if (newFocus != -1) {
    _w->PluginCombo->setCurrentIndex(newFocus);
  } else {
    _w->PluginCombo->setCurrentIndex(0);
    pluginChanged(0);
  }
}


bool KstFilterDialog::saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p) {
  KstReadLocker vl(&KST::vectorList.lock());
  KstWriteLocker scl(&KST::scalarList.lock());
  KstWriteLocker stl(&KST::stringList.lock());
  const QList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  QList<Plugin::Data::IOValue>::const_iterator it;

  for (it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if ((*it)._name == p->data()._filterInputVector) {
        KstVectorPtr v = *KST::vectorList.findTag(_yvector);
        if (!v) {
          return false;
        }
        plugin->inputVectors().insert((*it)._name, v);
      } else {
        VectorSelector *vs;

        vs = _w->_pluginInputOutputFrame->findChild<VectorSelector*>((*it)._name.toLatin1());
        if (vs) {
          KstVectorPtr v;

          v = *KST::vectorList.findTag(vs->selectedVector());
          if (!v) {
            return false;
          }
          plugin->inputVectors().insert((*it)._name, v);
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      StringSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<StringSelector*>((*it)._name.toLatin1());
      if (ss) {
        KstStringPtr s;

        s = *KST::stringList.findTag(ss->selectedString());
        if (s == *KST::stringList.end()) {
          QString val = ss->_string->currentText();
          KstStringPtr newString;

          //
          // create orphan string...
          //

          newString = new KstString(KstObjectTag(ss->_string->currentText(), KstObjectTag::orphanTagContext), 0L, val, true);
          plugin->inputStrings().insert((*it)._name, newString);
        } else {
          return false;
        }
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else {
      ScalarSelector *ss;

      ss = _w->_pluginInputOutputFrame->findChild<ScalarSelector*>((*it)._name.toLatin1());
      if (ss) {
        KstScalarPtr s;

        s = *KST::scalarList.findTag(ss->selectedScalar());
        if (s == *KST::scalarList.end()) {
          bool ok;
          double val = ss->_scalar->currentText().toDouble(&ok);

          if (ok) {
            KstScalarPtr newScalar;

            //
            // create orphan scalar...
            //

            newScalar = new KstScalar(KstObjectTag(ss->_scalar->currentText(), KstObjectTag::orphanTagContext), 0L, val, true, false);
            plugin->inputScalars().insert((*it)._name, newScalar);
          } else {
            return false;
          }
        } else {
          plugin->inputScalars().insert((*it)._name, s);
        }
      }
    }
  }

  return true;
}


bool KstFilterDialog::createCurve(KstCPluginPtr plugin) {
  KstViewWindow *viewWindow;
  KstVectorPtr xVector;
  KstVectorPtr yVector;
  KstVCurvePtr fit;
  QString c_name;
  QColor color;

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
  c_name = KST::suggestCurveName(plugin->tag(), true);
// xxx  color = KstApp::inst()->chooseColorDlg()->getColorForCurve(xVector, yVector);
  if (!color.isValid()) {
    color = _w->_curveAppearance->color();
  }
  fit = new KstVCurve(c_name, xVector, yVector, KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), color);

  fit->setHasPoints(_w->_curveAppearance->showPoints());
  fit->setHasLines(_w->_curveAppearance->showLines());
  fit->setHasBars(_w->_curveAppearance->showBars());
  fit->setLineWidth(_w->_curveAppearance->lineWidth());
  fit->setLineStyle(_w->_curveAppearance->lineStyle());
  fit->setPointStyle(_w->_curveAppearance->pointType());
  fit->setBarStyle(_w->_curveAppearance->barStyle());
  fit->setPointDensity(_w->_curveAppearance->pointDensity());

  viewWindow = KstApp::inst()->findWindow(_window);
  if (viewWindow && viewWindow->view()->findChild(_plotName)) {
    Kst2DPlotPtr plot;

    plot = kst_cast<Kst2DPlot>(viewWindow->view()->findChild(_plotName));
    if (plot) {
      plot->addCurve(fit);
    }
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(fit);
  KST::dataObjectList.lock().unlock();

  return true;
}


bool KstFilterDialog::newObject() {
  QString tagName = _tagName->text();

  if (KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  } else {
    int pitem = _w->PluginCombo->currentIndex();

    if (pitem >= 0 && _w->PluginCombo->count() > 0) {
      QExplicitlySharedDataPointer<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

      if (pPtr) {
        KstCPluginPtr plugin;

        plugin = new KstCPlugin;

        KstWriteLocker pl(plugin.data());

        plugin->setDirty();
        if (saveInputs(plugin, pPtr)) {
          if (tagName == plugin_defaultTag) {
            tagName = KST::suggestPluginName(_pluginList[pitem], KstObjectTag::fromString(_yvector));
          }

          plugin->setTagName(KstObjectTag(tagName, KstObjectTag::globalTagContext)); // FIXME: tag context always global?

          plugin->setPlugin(pPtr);

          if (saveOutputs(plugin, pPtr)) {
            if (plugin->isValid()) {
              if (!createCurve(plugin)) {
                QMessageBox::warning(this, i18n("Kst"), i18n("There is an error in the plugin you entered."));

                return false;
              } else {
                KST::dataObjectList.lock().writeLock();
                KST::dataObjectList.append(plugin);
                KST::dataObjectList.lock().unlock();
              }
            } else {
              QMessageBox::warning(this, i18n("Kst"), i18n("There is an error in the plugin you entered."));

              return false;
            }
          } else {
            QMessageBox::warning(this, i18n("Kst"), i18n("There is an error in the outputs you entered."));

            return false;
          }
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("There is an error in the inputs you entered."));

          return false;
        }
      }
    }

    emit modified();
  }

  return true;
}


void KstFilterDialog::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QList<Plugin::Data::IOValue>& table) {
  QString fixedVector;
  const QString& pluginName = _pluginList[_w->PluginCombo->currentIndex()];
  const Plugin::Data& pluginData = PluginCollection::self()->pluginList()[PluginCollection::self()->pluginNameList()[pluginName]];
  QList<Plugin::Data::IOValue>::const_iterator it;
  QString scalarLabelTemplate;
  QString vectorLabelTemplate;
  QString stringLabelTemplate;

  //
  // get fixed vector for filter...
  //

  if (input) {
    fixedVector = pluginData._filterInputVector;
  } else {
    fixedVector = pluginData._filterOutputVector;
  }

  if (input) {
    stringLabelTemplate = i18n("Input String - %1:");
    scalarLabelTemplate = i18n("Input Scalar - %1:");
    vectorLabelTemplate = i18n("Input Vector - %1:");
  } else {
    stringLabelTemplate = i18n("Output String - %1:");
    scalarLabelTemplate = i18n("Output Scalar - %1:");
    vectorLabelTemplate = i18n("Output Vector - %1:");
  }

  for (it = table.begin(); it != table.end(); ++it) {
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

    QLabel *label = 0L;
    QWidget *widget = 0L;

    label = new QLabel(labellabel, parent);
    label->setObjectName(input ? "Input label" : "Output label");

    if (input) {
      if (scalar) {
        ScalarSelector *w = 0L;
        KstScalarPtr p;
        
        w = new ScalarSelector(parent);
        w->setObjectName((*it)._name.toLatin1());
        widget = w;

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
					w->_scalar->QWidget::setToolTip("");
					w->_scalar->QWidget::setToolTip(QString::number(p->value()));
          p->unlock();
        }
      } else if (string) {
        StringSelector *w;
        KstStringPtr p;

        w = new StringSelector(parent);
        w->setObjectName((*it)._name.toLatin1());
        widget = w;

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
					w->_string->setToolTip("");
					w->_string->setToolTip(p->value());
          p->unlock();
        }
      } else {
        if (fixed) {
          widget = new QLabel(parent);
          widget->setObjectName((*it)._name.toLatin1());
          static_cast<QLabel*>(widget)->setText(_yvector);
        } else {
          widget = new VectorSelector(parent);
          widget->setObjectName((*it)._name.toLatin1());
          connect(widget, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
        }
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
			label->QWidget::setWhatsThis("");
			widget->QWidget::setWhatsThis("");
			label->QWidget::setWhatsThis((*it)._description);
			widget->QWidget::setWhatsThis((*it)._description);
    }

    ++cnt;
  }
}

#include "kstfilterdialog.moc"

