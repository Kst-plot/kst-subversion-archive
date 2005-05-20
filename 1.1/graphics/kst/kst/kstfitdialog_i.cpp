/***************************************************************************
                     kstfitdialog_i.cpp  -  Part of KST
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
#include <qregexp.h>

// include files for KDE
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "curveappearancewidget.h"
#include "kstfitdialog_i.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "plugincollection.h"
#include "pluginmanager.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"

QGuardedPtr<KstFitDialogI> KstFitDialogI::_inst;

KstFitDialogI *KstFitDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstFitDialogI(KstApp::inst());
  }

  return _inst;
}


//const QString newFitPluginString = "<Auto Name>";
const QString fit_defaultTag = "<Auto Name>";

KstFitDialogI::KstFitDialogI(QWidget* parent, const char* name,
                             bool modal, WFlags fl)
: KstFitDialog(parent, name, modal, fl) {
  Init();

  connect(_OK, SIGNAL(clicked()), this, SLOT(OK()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(PluginCombo, SIGNAL(activated(int)), this, SLOT(pluginChanged(int)));
  connect(_pluginManager, SIGNAL(clicked()), this, SLOT(showPluginManager()));

  QGridLayout *grid = new QGridLayout(_pluginFrame, 1, 1);
  grid->addWidget(_frameWidget = new QWidget(_pluginFrame, "Frame Widget"), 0, 0);
}


KstFitDialogI::~KstFitDialogI() {
}


void KstFitDialogI::Init() {
}


void KstFitDialogI::OK() {
  if (new_I()) {
    close();
  }
}


void KstFitDialogI::show_setCurve(const QString& strCurve,
                                  const QString& strPlotName,
                                  const QString& strWindow) {
  KstBaseCurvePtr curve;
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  KstPluginList c = kstObjectSubList<KstDataObject, KstPlugin>(KST::dataObjectList);
  QString new_label;

  _strWindow   = strWindow;
  _strPlotName = strPlotName;
  _strCurve    = strCurve;
  curve = *curves.findTag(strCurve);
  if (curve) {
    KstReadLocker rl(curve);
    _xvector = curve->xVTag();
    _yvector = curve->yVTag();
    _evector = curve->yETag();
  }

  if (_xvector == "<None>" || _yvector == "<None>") {
    return;
  }

  _tagName->setText(fit_defaultTag);

  updatePluginList();
  pluginChanged(PluginCombo->currentItem());

  show();
  raise();
}


void KstFitDialogI::updatePluginList() {
  PluginCollection *pc = PluginCollection::self();
  const QMap<QString,Plugin::Data>& _pluginMap = pc->pluginList();
  QString previous = _pluginList[PluginCombo->currentItem()];
  QMap<QString,Plugin::Data>::ConstIterator it;
  int newFocus = -1;
  int cnt = 0;

  PluginCombo->clear();
  _pluginList.clear();
  for (it = _pluginMap.begin(); it != _pluginMap.end(); ++it) {
    if (it.data()._isFit) {
      if (!it.data()._isFitWeighted || _evector != "<None>") {
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
  }

  if (newFocus != -1) {
    PluginCombo->setCurrentItem(newFocus);
  } else {
    PluginCombo->setCurrentItem(0);
    pluginChanged(0);
  }
}


void KstFitDialogI::update() {
  KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[PluginCombo->currentItem()]);
  if (pPtr) {
    const QValueList<Plugin::Data::IOValue>& itable = pPtr->data()._inputs;
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
  }
}


void KstFitDialogI::fixupLayout() {
  _frameWidget->updateGeometry();
  _pluginFrame->updateGeometry();
  resize(sizeHint());
  setMinimumSize(size());
  updateGeometry();
}


bool KstFitDialogI::saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  int iInputVector = 0;

  KST::vectorList.lock().readLock();
  KST::scalarList.lock().readLock();
  KST::stringList.lock().readLock();
  const QValueList<Plugin::Data::IOValue>& itable = p->data()._inputs;
  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = itable.begin(); it != itable.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (iInputVector < 2 || (plugin->plugin()->data()._isFitWeighted && iInputVector < 3) ) {
        KstVectorPtr v;

        switch (iInputVector) {
          case 0:
            v = *KST::vectorList.findTag(_xvector);
            break;
          case 1:
            v = *KST::vectorList.findTag(_yvector);
            break;
          case 2:
            v = *KST::vectorList.findTag(_evector);
            break;
        }
        if (v) {
          v->writeLock(); // to match with plugin->writeLock()
        }
        if (plugin->inputVectors().contains((*it)._name)) {
          plugin->inputVectors()[(*it)._name]->writeUnlock();
        }
        plugin->inputVectors().insert((*it)._name, v);
        iInputVector++;
      } else {
        QObject *field = _frameWidget->child((*it)._name.latin1(), "VectorSelector");
        if (field) {
          VectorSelector *vs = static_cast<VectorSelector*>(field);
          KstVectorPtr v = *KST::vectorList.findTag(vs->selectedVector());
          if (v) {
            v->writeLock(); // to match with plugin->writeLock()
          }
          if (plugin->inputVectors().contains((*it)._name)) {
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
          if (plugin->inputStrings().contains((*it)._name)) {
            plugin->inputStrings()[(*it)._name]->writeUnlock();
          }
          plugin->inputStrings().insert((*it)._name, newString);
        } else {
          s->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputStrings().contains((*it)._name)) {
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
            if (plugin->inputScalars().contains((*it)._name)) {
              plugin->inputScalars()[(*it)._name]->writeUnlock();
            }
            plugin->inputScalars().insert((*it)._name, newScalar);
          } else {
            s->writeLock(); // to match with plugin->writeLock()
            if (plugin->inputScalars().contains((*it)._name)) {
              plugin->inputScalars()[(*it)._name]->writeUnlock();
            }
            plugin->inputScalars().insert((*it)._name, s);
          }
        } else {
          s->writeLock(); // to match with plugin->writeLock()
          if (plugin->inputScalars().contains((*it)._name)) {
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


bool KstFitDialogI::saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p) {
  const QValueList<Plugin::Data::IOValue>& otable = p->data()._outputs;

  for (QValueList<Plugin::Data::IOValue>::ConstIterator it = otable.begin(); it != otable.end(); ++it) {
    QObject *field = _frameWidget->child((*it)._name.latin1(), "QLineEdit");
    if (!field) {
      continue;
    }

    QLineEdit *li = static_cast<QLineEdit*>(field);

    if (li->text().isEmpty()) {
      QString tagName = _tagName->text();
      if (tagName == fit_defaultTag) {
        tagName = plugin->tagName();
      }
      li->setText(tagName+"-"+(*it)._name);
    }

    QString nt = li->text();
    if ((*it)._type == Plugin::Data::IOValue::TableType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        KstVectorPtr v = plugin->outputVectors()[(*it)._name];
        v->setTagName(nt);
        v->setProvider(plugin.data());
      } else if (plugin->outputVectors()[(*it)._name]->tagName() != nt) {
        while (KST::vectorTagNameNotUnique(nt, false)) {
          nt += "'";
        }
        KstVectorPtr v = plugin->outputVectors()[(*it)._name];
        v->setTagName(nt);
        v->setProvider(plugin.data());
      }
    } else if ((*it)._type == Plugin::Data::IOValue::StringType) {
      if (!KST::vectorTagNameNotUnique(nt, false)) {
        plugin->outputStrings()[(*it)._name]->setTagName(nt);
      } else if (plugin->outputStrings()[(*it)._name]->tagName() != nt) {
        while (KST::vectorTagNameNotUnique(nt, false)) {
          nt += "'";
        }
        plugin->outputStrings()[(*it)._name]->setTagName(nt);
      }
    } else if ((*it)._type == Plugin::Data::IOValue::PidType) {
      // Nothing
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


bool KstFitDialogI::createCurve(KstPluginPtr plugin) {
  KstVectorPtr xVector;
  KstVectorPtr yVector;
  KstVectorPtr wVector;
  KstLabel* label;
  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  KstVCurvePtr fit;
  QString strLabel;
  bool bRetVal = false;

  // Generate new fit name
  QString c_name = KST::suggestCurveName(plugin->tagName(), true);

  if (plugin->inputVectors().contains("X Array")) {
    xVector = plugin->inputVectors()["X Array"];
  }
  if (plugin->outputVectors().contains("Y Fitted")) {
    yVector = plugin->outputVectors()["Y Fitted"];
  }

  if (xVector && yVector) {
    fit = new KstVCurve(c_name, KstVectorPtr(xVector), KstVectorPtr(yVector), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), KstVectorPtr(0L), _curveAppearance->color());
    if (fit) {
      fit->setHasPoints(_curveAppearance->showPoints());
      fit->setHasLines(_curveAppearance->showLines());
      fit->setHasBars(_curveAppearance->showBars());
      fit->setLineWidth(_curveAppearance->lineWidth());
      fit->setLineStyle(_curveAppearance->lineStyle());
      fit->Point.setType(_curveAppearance->pointType());
      fit->setBarStyle(_curveAppearance->barStyle());
      fit->setPointDensity(_curveAppearance->pointDensity());

      // create the necessary label
      strLabel = i18n("curvename: pluginname\\n", "%2: %1\\n").
                 arg(_strCurve.replace( '_',  "\\_" ).remove( QRegExp( "^[C][0-9]{1,2}\\-" ) )).
                 arg(plugin->plugin()->data()._readableName);

      KstVectorPtr vectorParam = plugin->outputVectors()["Parameters"];
      if (vectorParam) {
        plugin->createFitScalars();
        for (int i = 0; i < vectorParam->length(); i++) {
          QString strParamName = plugin->plugin()->parameterName(i);
          if (!strParamName.isEmpty()) {
            if (plugin->outputScalars().contains(strParamName)) {
              KstScalarPtr scalar = plugin->outputScalars()[strParamName];

              if (scalar) {
                strLabel += i18n("parametername: [scalarname]\\n", "%1: [%2]\\n").arg(strParamName).arg(scalar->tagName());
              }
            }
          }
        }
      }

      //
      // add the label and the curve to the plot...
      //
      KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_strWindow));
      if (w && w->view()->findChild(_strPlotName)) {
        Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(w->view()->findChild(_strPlotName));
        if (plot) {
          label = new KstLabel(strLabel, LxTy, 0.0, 0.05, 0.05);
          plot->addLabel(label);
          plot->addCurve(fit.data());
        }
      }

      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.append(fit.data());
      KST::dataObjectList.lock().writeUnlock();

      bRetVal = true;
    }
  }

  return bRetVal;
}


bool KstFitDialogI::new_I() {
  QString tagName = _tagName->text();

  if (tagName != fit_defaultTag) {
    if (KST::dataTagNameNotUnique(tagName, true, this)) {
      _tagName->setFocus();
      return false;
    }
  }

  int pitem = PluginCombo->currentItem();

  if (pitem >= 0 && PluginCombo->count() > 0) {
    KstSharedPtr<Plugin> pPtr = PluginCollection::self()->plugin(_pluginList[pitem]);

    if (pPtr) {
      KstPluginPtr plugin = new KstPlugin;
      plugin->writeLock();
      plugin->setDirty();
      if (saveInputs(plugin, pPtr)) {
        plugin->setPlugin(pPtr);

        if (tagName == fit_defaultTag) {
          tagName = KST::suggestPluginName(_pluginList[pitem], _strCurve);
        }

        plugin->setTagName(tagName);
        if (saveOutputs(plugin, pPtr)) {
          if (plugin->isValid()) {
            if (!createCurve(plugin)) {
              KMessageBox::sorry(this, i18n("Could not create curve from fit, possibly due to a bad plugin."));
              plugin->writeUnlock();
              return false;
            }
            plugin->writeUnlock();
            KST::dataObjectList.lock().writeLock();
            KST::dataObjectList.append(plugin.data());
            KST::dataObjectList.lock().writeUnlock();
          } else {
            KMessageBox::sorry(this, i18n("There is something wrong (i.e, there is a bug) with the creation of the fit plugin."));
            plugin->writeUnlock();
            return false;
          }
        } else {
          plugin->writeUnlock();
        }
      } else {
        plugin->writeUnlock();
      }
    } else {
      KMessageBox::sorry(this, i18n("There is something wrong (i.e, there is a bug) with"
                                    " the selected plugin.\n"));
      return false;
    }

    emit modified();
  }
  return true;
}


void KstFitDialogI::generateEntries(bool isWeighted, bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table) {
  QString scalarLabelTemplate, vectorLabelTemplate, stringLabelTemplate;
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
        if (input && ( iInputVector < 2 || ( isWeighted && iInputVector < 3 ) ) ) {
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
        }
        KstScalarPtr p = *KST::scalarList.findTag(w->_scalar->currentText());
        w->allowDirectEntry( true );
        if (p) {
          QToolTip::add(w->_scalar, QString::number(p->value()));
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
        w->allowDirectEntry( true );
        if (p) {
          QToolTip::add(w->_string, p->value());
        }
      } else {
        if (fixed) {
          widget = new QLabel(parent, (*it)._name.latin1());
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


void KstFitDialogI::pluginChanged(int idx) {
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
    generateEntries(pluginData._isFitWeighted, true, cnt, _frameWidget, grid, pluginData._inputs);

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
    generateEntries(pluginData._isFitWeighted, false, cnt, _frameWidget, grid, pluginData._outputs);
  }

  // show it
  _frameWidget->show();

  // resize everything
  QTimer::singleShot(0, this, SLOT(fixupLayout()));
}


void KstFitDialogI::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;
  updatePluginList();
}


void KstFitDialogI::updateScalarTooltip(const QString& n) {
  KstScalarPtr s = *KST::scalarList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));
  if (s) {
    QToolTip::remove(w);
    QToolTip::add(w, QString::number(s->value()));
  } else {
    QToolTip::remove(w);
  }
}


void KstFitDialogI::updateStringTooltip(const QString& n) {
  KstStringPtr s = *KST::stringList.findTag(n);
  QWidget *w = const_cast<QWidget*>(static_cast<const QWidget*>(sender()));
  if (s) {
    QToolTip::remove(w);
    QToolTip::add(w, s->value());
  } else {
    QToolTip::remove(w);
  }
}


#include "kstfitdialog_i.moc"

// vim: ts=2 sw=2 et
