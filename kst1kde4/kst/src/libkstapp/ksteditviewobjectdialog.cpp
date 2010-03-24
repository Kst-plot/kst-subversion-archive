/***************************************************************************
                    ksteditviewobjectdialog.cpp  -  Part of KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 The University of British Columbia
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

#include "ksteditviewobjectdialog.h"

#include <qbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <Qmessagebox.h>

#include <kcolorbutton.h>
#include <kfontcombo.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "kst.h"

#include <klocale.h>
#include <stdio.h>

KstEditViewObjectDialog::KstEditViewObjectDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) 
: QDialog(parent, fl) {
  setupUi(this);
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_OK, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(_editMultiple, SIGNAL(clicked()), this, SLOT(toggleEditMultiple()));
  connect(_pushButtonSetDefaults, SIGNAL(clicked()), this, SLOT(setDefaults()));
  connect(_pushButtonRestoreDefaults, SIGNAL(clicked()), this, SLOT(restoreDefaults()));

  _grid = 0L;
  _viewObject = 0L;
  _isNew = false;

  _customWidget = 0L;
  _editMultipleMode = false;
  _editMultipleWidget->hide();

  resize(360, 200);
  setMinimumSize(360, 200);
}


KstEditViewObjectDialog::~KstEditViewObjectDialog() {
  if (_viewObject) {
    _viewObject->setDialogLock(false);
  }
}


void KstEditViewObjectDialog::setNew() {
  _isNew = true;
  _apply->setEnabled(false);
  _editMultiple->setEnabled(false);
}


void KstEditViewObjectDialog::updateEditViewObjectDialog() {
  updateWidgets();
}


void KstEditViewObjectDialog::showEditViewObjectDialog(KstViewObjectPtr viewObject, KstTopLevelViewPtr top) {
  _viewObject = viewObject;
  if (_viewObject) {
    _viewObject->setDialogLock(true);
  }
  _top = top;
  updateWidgets();
  if (_viewObject) {
    if (_isNew) {
      setCaption(_viewObject->newTitle());
    } else {
      setCaption(_viewObject->editTitle());
    }
    if (_viewObject && !_viewObject->supportsDefaults()) {
      _buttonGroupDefaults->hide();
      _pushButtonSetDefaults->hide();
      _pushButtonRestoreDefaults->hide();
    }
  }
  if (!_top) {
    _pushButtonSetDefaults->setEnabled(false);
    _pushButtonRestoreDefaults->setEnabled(false);
  }

  _apply->setEnabled(false);
  show();
  raise();
}


void KstEditViewObjectDialog::toggleEditMultiple() {
  _editMultipleWidget->_objectList->clear();

  if (_editMultipleMode) {
    _editMultipleWidget->hide();
    _editMultiple->setText(i18n("Edit Multiple >>"));
    if (_viewObject && _viewObject->supportsDefaults()) {
      _buttonGroupDefaults->setEnabled(true);
      _pushButtonSetDefaults->setEnabled(true);
      _pushButtonRestoreDefaults->setEnabled(true);
    }
    updateWidgets();
  } else {
    _editMultipleWidget->show();
    _editMultiple->setText(i18n("Edit Multiple <<"));
    _buttonGroupDefaults->setEnabled(false);
    _pushButtonSetDefaults->setEnabled(false);
    _pushButtonRestoreDefaults->setEnabled(false);

    if (_customWidget) {
      if (_viewObject) {
        fillObjectList();
        _viewObject->populateEditMultiple(_customWidget);
      }
    } else {
      fillObjectList();
      populateEditMultiple();
    }
  }

  _editMultipleMode = !_editMultipleMode;

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEditViewObjectDialog::fillObjectList() {
  KstViewObjectList list;
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i)
    KstViewWindow *view = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      if (_viewObject->type() == "TopLevelView") {
        list.append(viewWindow->view().data());
      } else {
        list += viewWindow->view()->findChildrenType(_viewObject->type(), true);
      }
    }
  }

  _editMultipleWidget->_objectList->insertStringList(list.tagNames());
}


void KstEditViewObjectDialog::populateEditMultiple() {
  QSpinBox *spinBoxWidget;
  KColorButton *colorButtonWidget;
  KURLRequester *urlRequester;
  QLineEdit *lineEditWidget;
  QCheckBox *checkBoxWidget;
  QComboBox *comboWidget;

  for (QValueList<QWidget*>::ConstIterator iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
    if ((spinBoxWidget = dynamic_cast<QSpinBox*>(*iter)) != 0L) {
      spinBoxWidget->setMinValue(spinBoxWidget->minValue() - 1);
      spinBoxWidget->setSpecialValueText(QString(" "));
      spinBoxWidget->setValue(spinBoxWidget->minValue());
    } else if ((colorButtonWidget = dynamic_cast<KColorButton*>(*iter)) != 0L) {
      colorButtonWidget->setColor(QColor());
    } else if ((urlRequester = dynamic_cast<KURLRequester*>(*iter)) != 0L) {
      urlRequester->lineEdit()->setText(QString(" "));
    } else if ((lineEditWidget = dynamic_cast<QLineEdit*>(*iter)) != 0L) {
      lineEditWidget->setText(QString(" "));
    } else if ((checkBoxWidget = dynamic_cast<QCheckBox*>(*iter)) != 0L) {
      checkBoxWidget->setTristate();
      checkBoxWidget->setNoChange();
    } else if ((comboWidget = dynamic_cast<QComboBox*>(*iter)) != 0L) {
      comboWidget->insertItem(QString(" "));
      comboWidget->setCurrentItem(comboWidget->count()-1);
    }
  }
}


void KstEditViewObjectDialog::clearWidgets() {
  // clear all the current widgets from the grid
  for (QValueList<QWidget*>::Iterator i = _inputWidgets.begin(); i != _inputWidgets.end(); ++i) {
    delete *i;
  }
  _inputWidgets.clear();

  for (QValueList<QWidget*>::Iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
    delete *i;
  }
  _widgets.clear();

  delete _customWidget;
  _customWidget = 0L;

  delete _grid;
  _grid = 0L;
}


void KstEditViewObjectDialog::updateDefaultWidgets() {
  int numProperties = _viewObject->metaObject()->numProperties(true);

  // create a new grid
  _grid = new QGridLayout(_propertiesFrame, numProperties, 2, 0, 8);
  _grid->setColStretch(0, 0);
  _grid->setColStretch(1, 1);

  // get the property names and types
  for (int i = 0; i < numProperties; i++) {
    const QMetaProperty* property = _viewObject->metaObject()->property(i, true);
    QString propertyType(property->type());
    QString propertyName(property->name());

    // for this property, get the meta-data map
    QMap<QString, QVariant> metaData = _viewObject->widgetHints(propertyName);

    if (!metaData.empty()) {
      QString friendlyName = metaData["_kst_label"].toString();
      QString widgetType = metaData["_kst_widgetType"].toString();
      metaData.erase("_kst_label");
      metaData.erase("_kst_widgetType");

      // use friendly name for label
      QLabel* propertyLabel = new QLabel(_propertiesFrame, "label-"+i);
      propertyLabel->setText(friendlyName);
      _grid->addWidget(propertyLabel, i, 0);
      propertyLabel->show();
      _widgets.append(propertyLabel);

      // display different types of widgets depending on what dialogData specifies
      QWidget* propertyWidget = 0L;

      if (widgetType == "QSpinBox") {
        propertyWidget = new QSpinBox(_propertiesFrame, (propertyName+","+"value").toLatin1()); 
        propertyWidget->setProperty("value", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(valueChanged(const QString&)), this, SLOT(modified()));
          connect(propertyWidget->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KColorButton") {
        propertyWidget = new KColorButton(_propertiesFrame, (propertyName+","+"color").toLatin1());
        propertyWidget->setProperty("color", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(changed(const QColor&)), this, SLOT(modified()));
        }
      } else if (widgetType == "QLineEdit") {
        propertyWidget = new QLineEdit(_propertiesFrame, (propertyName+","+"text").toLatin1());
        propertyWidget->setProperty("text", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KURLRequester") {
        propertyWidget = new KURLRequester(_propertiesFrame, (propertyName+","+"url").toLatin1());
        propertyWidget->setProperty("url", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
          connect(propertyWidget, SIGNAL(urlSelected(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "PenStyleWidget") {
        // insert a combobox with QT pen styles
        QComboBox* combo = new QComboBox(_propertiesFrame, (propertyName+","+"currentItem").toLatin1());
        fillPenStyleWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      } else if (widgetType == "QCheckBox") {
        propertyWidget = new QCheckBox(_propertiesFrame, (propertyName+","+"checked").toLatin1());
        propertyWidget->setProperty("checked", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(pressed()), this, SLOT(modified()));
        }
      } else if (widgetType == "KDoubleSpinBox") {
        KDoubleSpinBox* input = new KDoubleSpinBox(_propertiesFrame, (propertyName+","+"value").toLatin1());
        input->setPrecision(metaData["precision"].toInt());
        input->setMinValue(metaData["minValue"].toDouble());
        input->setMaxValue(metaData["maxValue"].toDouble());
        input->setLineStep(metaData["lineStep"].toDouble());
        metaData.erase("minValue");
        metaData.erase("maxValue");
        metaData.erase("lineStep");
        metaData.erase("precision");
        propertyWidget = input; 
        propertyWidget->setProperty("value", _viewObject->property(property->name()));
        // need the following line because of a KDE bug causing valueChanged(double) never to be emitted 
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(valueChanged(int)), this, SLOT(modified()));
          connect(propertyWidget, SIGNAL(valueChanged(double)), this, SLOT(modified()));
          connect(propertyWidget->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KFontCombo") {
        propertyWidget = new KFontCombo(_propertiesFrame, (propertyName+","+"currentText").toLatin1());
        propertyWidget->setProperty("currentText", _viewObject->property(property->name()));  
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      } else if (widgetType == "HJustifyCombo") {
        // insert a combo box filled with horizontal justifications
        QComboBox* combo = new QComboBox(_propertiesFrame, (propertyName+","+"currentItem").toLatin1());
        fillHJustifyWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      } else if (widgetType == "VJustifyCombo") {
        // insert a combo box filled with vertical justifications
        QComboBox* combo = new QComboBox(_propertiesFrame, (propertyName+","+"currentItem").toLatin1());
        fillVJustifyWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      }

      if (propertyWidget != 0L) {
        propertyWidget->show();

        // also set any additional properties specified by metaData
        for (QMap<QString, QVariant>::ConstIterator it = metaData.begin(); it != metaData.end(); ++it) {
          propertyWidget->setProperty(it.key().toLatin1(), it.data());
        }

        _grid->addWidget(propertyWidget, i, 1);
        _inputWidgets.append(propertyWidget);
      }
    }
  }
}


void KstEditViewObjectDialog::updateWidgets() {
  clearWidgets();

  if (_viewObject) {
    _customWidget = _viewObject->configWidget(_propertiesFrame);
    if (_customWidget) {
      _grid = new QGridLayout(_propertiesFrame, 1, 1);
      _customWidget->show();
      _grid->addWidget(_customWidget, 0, 0);
      _viewObject->fillConfigWidget(_customWidget, _isNew);
      if (!_isNew) {
        _viewObject->connectConfigWidget(this, _customWidget);
      }
    } else {
      updateDefaultWidgets();
    }
  }
}


void KstEditViewObjectDialog::fillPenStyleWidget(QComboBox* widget) {
  QRect rect = widget->style().querySubControlMetrics(QStyle::CC_ComboBox, 
                                                      widget, 
                                                      QStyle::SC_ComboBoxEditField);
  rect.setLeft(rect.left() + 2);
  rect.setRight(rect.right() - 2);
  rect.setTop(rect.top() + 2);
  rect.setBottom(rect.bottom() - 2);

  QPixmap ppix(rect.width(), rect.height());
  QPainter pp(&ppix);
  QPen pen(Qt::black, 0);

  widget->clear();

  QValueList<Qt::PenStyle> styles;
  styles.append(Qt::SolidLine);
  styles.append(Qt::DashLine);
  styles.append(Qt::DotLine);
  styles.append(Qt::DashDotLine);
  styles.append(Qt::DashDotDotLine);

  while (!styles.isEmpty()) {
    pen.setStyle(styles.front());
    pp.setPen(pen);
    pp.fillRect( pp.window(), QColor("white"));
    pp.drawLine(1, ppix.height()/2, ppix.width()-1, ppix.height()/2);
    widget->insertItem(ppix);
    styles.pop_front();
  }
}


void KstEditViewObjectDialog::fillHJustifyWidget(QComboBox* widget) {
  widget->insertItem(i18n("Left"));
  widget->insertItem(i18n("Right"));
  widget->insertItem(i18n("Center")); 
}


void KstEditViewObjectDialog::fillVJustifyWidget(QComboBox* widget) {
  widget->insertItem(i18n("Top"));
  widget->insertItem(i18n("Bottom"));
  widget->insertItem(i18n("Center")); 
}


void KstEditViewObjectDialog::modified() {
  _apply->setEnabled(true);
}


void KstEditViewObjectDialog::applySettings(KstViewObjectPtr viewObject) {
  if (viewObject && viewObject.data()) {
    if (_customWidget) {
      viewObject->readConfigWidget(_customWidget, _editMultipleMode);
    } else {
      // get all the properties and set them
      for (QValueList<QWidget*>::ConstIterator iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
        if (_editMultipleMode) {
          QSpinBox *spinBoxWidget;
          KColorButton *colorButtonWidget;
          KURLRequester *urlRequester;
          QLineEdit *lineEditWidget;
          QCheckBox *checkBoxWidget;
          QComboBox *comboWidget;
          bool edited = false;

          if ((spinBoxWidget = dynamic_cast<QSpinBox*>(*iter)) != 0L) {
            if (spinBoxWidget->value() != spinBoxWidget->minValue()) {
              edited = true;
            }
          } else if ((colorButtonWidget = dynamic_cast<KColorButton*>(*iter)) != 0L) {
            if (colorButtonWidget->color() != QColor()) {
              edited = true;
            }
          } else if ((urlRequester = dynamic_cast<KURLRequester*>(*iter)) != 0L) {
            if (urlRequester->lineEdit()->text().compare(QString(" ")) != 0) {
              edited = true;
            }
          } else if ((lineEditWidget = dynamic_cast<QLineEdit*>(*iter)) != 0L) {
            if (lineEditWidget->text().compare(QString(" ")) != 0 ) {
              edited = true;
            }
          } else if ((checkBoxWidget = dynamic_cast<QCheckBox*>(*iter)) != 0L) {
            if (checkBoxWidget->state() != QButton::NoChange) {
              edited = true;
            }
          } else if ((comboWidget = dynamic_cast<QComboBox*>(*iter)) != 0L) {
            if (comboWidget->currentText().compare(QString(" ")) != 0) {
              edited = true;
            }
          }

          if (edited) {
            // get the widget type and property name
            QString propertyName = QString((*iter)->name()).section(',', 0, 0);
            QString widgetPropertyName = QString((*iter)->name()).section(',', 1, 1);

            // get the widget's property and set it on the viewObject
            viewObject->setProperty(propertyName.toLatin1(), (*iter)->property(widgetPropertyName.toLatin1()));
          }
        } else {
          // get the widget type and property name
          QString propertyName = QString((*iter)->name()).section(',', 0, 0);
          QString widgetPropertyName = QString((*iter)->name()).section(',', 1, 1);

          // get the widget's property and set it on the viewObject
          viewObject->setProperty(propertyName.toLatin1(), (*iter)->property(widgetPropertyName.toLatin1()));
        }
      }
    }
  }
}


bool KstEditViewObjectDialog::apply() {
  bool applied = false;

  if (_editMultipleMode) {
    for (uint i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      QString name = _editMultipleWidget->_objectList->text(i);

      if (_editMultipleWidget->_objectList->isSelected(i)) {
        QList<QMdiSubWindow*> windows;
        QList<QMdiSubWindow*>::const_iterator i;
      
        windows = app->subWindowList(QMdiArea::CreationOrder);
      
        for (i = windows.constBegin(); i != windows.constEnd(); ++i)
          KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);
          if (viewWindow) {
            KstViewObjectPtr viewObject;

            if (_viewObject->type() == "TopLevelView") {
              viewObject = viewWindow->view();
              if (viewObject->name() == name) {
                applySettings(viewObject);

                break;
              }
            } else {
              viewObject = kst_cast<KstViewObject>(viewWindow->view()->findChild(name));
              if (viewObject) {
                applySettings(viewObject);

                break;
              }
            }
          }
        }        

        applied = true;
      }
    }

    if (!applied) {
      QMessageBox::sorry(this, i18n("Kst"), i18n("Select one or more objects to edit."));
    }
  } else {
    applied = true;
    applySettings(_viewObject);
  }

  if (applied) {
    _apply->setDisabled(true);
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }

  return applied;
}


void KstEditViewObjectDialog::setDefaults() {
  if (_top) {
    KstViewObjectPtr viewObject = _viewObject->copyObjectQuietly();
    if (viewObject) {
      applySettings(viewObject);
      _top->saveDefaults(viewObject);
    }
  }
}


void KstEditViewObjectDialog::restoreDefaults() {
  if (_top) {
    _top->restoreDefaults(_viewObject);
  }
}


void KstEditViewObjectDialog::applyClicked() {
  apply();
}


void KstEditViewObjectDialog::okClicked() {
  if (_viewObject && apply()) {
    QDialog::accept();
  }
}


void KstEditViewObjectDialog::resizeEvent(QResizeEvent *event) {
  Q_UNUSED(event)

  if (!_customWidget) {
    for (QValueList<QWidget*>::ConstIterator iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
      if (strcmp((*iter)->name(), "lineStyle,currentItem") == 0) {
        QComboBox* combo = dynamic_cast<QComboBox*>(*iter);
        if (combo != 0L) {
          int currentItem = combo->currentItem();
          fillPenStyleWidget(combo);
          if (_editMultipleMode) {
            combo->insertItem(QString(" "));
          }
          combo->setCurrentItem(currentItem);
        }
      }
    }
  }
}

#include "ksteditviewobjectdialog.moc"
