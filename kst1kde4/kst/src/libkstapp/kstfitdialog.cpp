/***************************************************************************
                     kstfitdialog.cpp  -  Part of KST
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
#include <stdio.h>

#include <QLineEdit>
#include <QMessageBox>
#include <QRegExp>
#include <QToolTip>
#include <QWhatsThis>

#include "curveappearancewidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog.h"
#include "kstdataobjectcollection.h"
#include "kstfitdialog.h"
#include "kstvcurve.h"
#include "kstviewlabel.h"
#include "kstviewwindow.h"
#include "plugincollection.h"
#include "plugindialogwidget.h"
#include "pluginmanager.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"

QPointer<KstFitDialog> KstFitDialog::_inst;

KstFitDialog *KstFitDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstFitDialog(KstApp::inst());
  }
  return _inst;
}


KstFitDialog::KstFitDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstPluginDialog(parent, name, modal, fl) {
  _w->_curveAppearance->show();
}


KstFitDialog::~KstFitDialog() {
}


void KstFitDialog::show_setCurve(const QString& strCurve,
                                 const QString& strPlotName,
                                 const QString& strWindow) {

  KstVCurvePtr curve;
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  KstVCurveList vcurves = kstObjectSubList<KstBaseCurve, KstVCurve>(curves);
  KstCPluginList c = kstObjectSubList<KstDataObject, KstCPlugin>(KST::dataObjectList);
  QString new_label;

  _strWindow   = strWindow;
  _strPlotName = strPlotName;
  _strCurve    = strCurve;
  curve = *vcurves.findTag(strCurve);
  if (curve) {
    KstReadLocker rl(curve);
    _yvector = curve->yVTag().displayString();
    _xvector = curve->xVTag().displayString();
    _evector = curve->yETag().displayString();
  }

  if (_xvector == "<None>" || _yvector == "<None>") {
    return;
  }

  show();
}


void KstFitDialog::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();
  QString previous = _pluginList[_w->PluginCombo->currentItem()];
  QMap<QString,Plugin::Data>::ConstIterator it;
  int newFocus = -1;
  int cnt = 0;

  _w->PluginCombo->clear();
  _pluginList.clear();
  for (it = _pluginMap.begin(); it != _pluginMap.end(); ++it) {
    if (it.data()._isFit) {
      if (!it.data()._isFitWeighted || _evector != "<None>") {
        _pluginList += it.data()._name;
        _w->PluginCombo->insertItem(i18n("%1 (v%2)").arg(it.data()._readableName).arg(it.data()._version));
        if (it.data()._name == previous) {
          newFocus = cnt;
        }
        ++cnt;
      }
    }
  }

  if (newFocus != -1) {
    _w->PluginCombo->setCurrentItem(newFocus);
  } else {
    _w->PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


bool KstFitDialog::createCurve(KstCPluginPtr plugin) {
  KstVectorPtr xVector;
  KstVectorPtr yVector;

  if (plugin->inputVectors().contains("X Array")) {
    xVector = plugin->inputVectors()["X Array"];
  }
  if (plugin->outputVectors().contains("Y Fitted")) {
    yVector = plugin->outputVectors()["Y Fitted"];
  }

  if (!xVector || !yVector) {
    return false;
  }

  QString c_name = KST::suggestCurveName(plugin->tag(), true);
  QColor color = KstApp::inst()->chooseColorDlg()->getColorForCurve(KstVectorPtr(xVector), KstVectorPtr(yVector));
  if (!color.isValid()) {
    color = _w->_curveAppearance->color();
  }
  KstVCurvePtr fit = new KstVCurve(c_name, KstVectorPtr(xVector), KstVectorPtr(yVector), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), color);
  fit->setHasPoints(_w->_curveAppearance->showPoints());
  fit->setHasLines(_w->_curveAppearance->showLines());
  fit->setHasBars(_w->_curveAppearance->showBars());
  fit->setLineWidth(_w->_curveAppearance->lineWidth());
  fit->setLineStyle(_w->_curveAppearance->lineStyle());
  fit->setPointStyle(_w->_curveAppearance->pointType());
  fit->setBarStyle(_w->_curveAppearance->barStyle());
  fit->setPointDensity(_w->_curveAppearance->pointDensity());

  // add the label and the curve to the plot
  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_strWindow));
  if (w && w->view()->findChild(_strPlotName)) {
    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(w->view()->findChild(_strPlotName));
    if (plot) {
      QString strLabel = QString("[%1]").arg(plugin->tagName());
      KstViewLabelPtr label = new KstViewLabel();

      label->setTransparent(true);
      label->resizeFromAspect(0.1, 0.1, 0.05, 0.05);
      label->setJustification(SET_KST_JUSTIFY(KST_JUSTIFY_H_CENTER, KST_JUSTIFY_V_CENTER));

      plot->appendChild(KstViewObjectPtr(label), true);
      label->setText(strLabel);

      plot->addCurve(KstBaseCurvePtr(fit));
    }
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(fit.data());
  KST::dataObjectList.lock().unlock();

  return true;
}


bool KstFitDialog::newObject() {
  QString tagName = _tagName->text();

  if (tagName != plugin_defaultTag) {
    if (KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
      _tagName->setFocus();
      return false;
    }
  }

  int pitem = _w->PluginCombo->currentItem();

  if (pitem >= 0 && _w->PluginCombo->count() > 0) {
    QExplicitlySharedDataPointer<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

    if (pPtr) {
      KstCPluginPtr plugin = new KstCPlugin;
      KstWriteLocker pl(plugin);
      plugin->setDirty();
      if (saveInputs(plugin, pPtr)) {
        if (tagName == plugin_defaultTag) {
          tagName = KST::suggestPluginName(_pluginList[pitem], KstObjectTag::fromString(_strCurve));
        }

        plugin->setTagName(KstObjectTag(tagName, KstObjectTag::globalTagContext)); // FIXME: tag context always global?

        plugin->setPlugin(pPtr);

        if (saveOutputs(plugin, pPtr)) {
          if (plugin->isValid()) {
            if (!createCurve(plugin)) {
              QMessageBox::warning(this, i18n("Kst"), i18n("Could not create curve from fit, possibly due to a bad plugin."));
              return false;
            }
            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(plugin.data());
            KST::dataObjectList.lock().unlock();
          } else {
            QMessageBox::warning(this, i18n("Kst"), i18n("There is something wrong (i.e, there is a bug) with the creation of the fit plugin."));
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
    } else {
      QMessageBox::warning(this, i18n("Kst"), i18n("There is something wrong (i.e, there is a bug) with the selected plugin.\n"));
      return false;
    }

    emit modified();
  }
  return true;
}


void KstFitDialog::generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QList<Plugin::Data::IOValue>& table) {
  QList<Plugin::Data::IOValue>::const_iterator it;
  QString scalarLabelTemplate;
  QString vectorLabelTemplate;
  QString stringLabelTemplate;
  int iInputVector = 0;

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
        if (input && ( iInputVector == 0 || iInputVector == 1 ) ) {
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
        ScalarSelector *w = new ScalarSelector(parent, (*it)._name.toLatin1());
        widget = w;
        connect(w->_scalar, SIGNAL(activated(const QString&)), this, SLOT(updateScalarTooltip(const QString&)));
        connect(widget, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_scalar->insertItem((*it)._default);
          w->_scalar->setCurrentText((*it)._default);
        }
        KstScalarPtr p = *KST::scalarList.findTag(w->_scalar->currentText());
        w->allowDirectEntry( true );
        if (p) {
          QToolTip::add(w->_scalar, QString::number(p->value()));
        }
      } else if (string) {
        StringSelector *w = new StringSelector(parent, (*it)._name.toLatin1());
        widget = w;
        connect(w->_string, SIGNAL(activated(const QString&)), this, SLOT(updateStringTooltip(const QString&)));
        connect(widget, SIGNAL(newStringCreated()), this, SIGNAL(modified()));
        if (!(*it)._default.isEmpty()) {
          w->_string->insertItem((*it)._default);
          w->_string->setCurrentText((*it)._default);
        }
        KstStringPtr p = *KST::stringList.findTag(w->_string->currentText());
        w->allowDirectEntry( true );
        if (p) {
          QToolTip::add(w->_string, p->value());
        }
      } else {
        if (fixed) {
          widget = new QLabel(parent, (*it)._name.toLatin1());
          switch (iInputVector) {
            case 0:
              static_cast<QLabel*>(widget)->setText(_xvector);
              break;
            case 1:
              static_cast<QLabel*>(widget)->setText(_yvector);
              break;
            case 2:
              static_cast<QLabel*>(widget)->setText(_evector);
              break;
          }
          iInputVector++;
        } else {
          VectorSelector *vectorSelector = new VectorSelector(parent, (*it)._name.toLatin1());
          widget = dynamic_cast<QWidget*>(vectorSelector);
          if ((*it)._optional) {
            KstVectorPtr vector;

            vectorSelector->provideNoneVector(true);
            vectorSelector->setSelection(vector);
          }
          connect(widget, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
        }
      }
    } else {
      widget = new QLineEdit(parent, (*it)._name.toLatin1());
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


bool KstFitDialog::saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p) {
  bool rc = true;

  const QValueList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "VectorSelector");
      KstVectorPtr v;
      KstReadLocker vl(&KST::vectorList.lock());

      if (!field) {
        field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "QLabel");
        assert(field);
        v = *KST::vectorList.findTag(static_cast<QLabel*>(field)->text());
      } else {
        VectorSelector *vs = static_cast<VectorSelector*>(field);
        v = *KST::vectorList.findTag(vs->selectedVector());
      }

      if (!v && (*it)._optional) {
        plugin->inputVectors().insert((*it)._name, v);
      } else if (v) {
        plugin->inputVectors().insert((*it)._name, v);
      } else if (plugin->inputVectors().contains((*it)._name)) {
        plugin->inputVectors().erase((*it)._name);
        rc = false;
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "StringSelector");
      assert(field);
      StringSelector *ss = static_cast<StringSelector*>(field);
      KstWriteLocker sl(&KST::stringList.lock());
      KstStringPtr s = *KST::stringList.findTag(ss->selectedString());

      if (s == *KST::stringList.end()) {
        QString val = ss->_string->currentText();
        // create orphan string
        KstStringPtr newString = new KstString(KstObjectTag(ss->_string->currentText(), KstObjectTag::orphanTagContext), 0L, val, true);
        plugin->inputStrings().insert((*it)._name, newString);
      } else {
        plugin->inputStrings().insert((*it)._name, s);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
    } else if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      QObject *field = _w->_pluginInputOutputFrame->child((*it)._name.toLatin1(), "ScalarSelector");
      assert(field);
      ScalarSelector *ss = static_cast<ScalarSelector*>(field);
      KstWriteLocker sl(&KST::scalarList.lock());
      KstScalarPtr s = *KST::scalarList.findTag(ss->selectedScalar());

      if (s == *KST::scalarList.end()) {
        bool ok;
        double val = ss->_scalar->currentText().toDouble(&ok);

        if (ok) {
          // create orphan scalar
          KstScalarPtr newScalar = new KstScalar(KstObjectTag(ss->_scalar->currentText(), KstObjectTag::orphanTagContext), 0L, val, true, false);
          plugin->inputScalars().insert((*it)._name, newScalar);
        } else {
          rc = false;
        }
      } else {
        plugin->inputScalars().insert((*it)._name, s);
      }
    } else {
    }
  }

  return rc;
}

#include "kstfitdialog.moc"

