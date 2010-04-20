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

#include <stdio.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QMetaProperty>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>

#include "kst.h"
#include "ksteditviewobjectdialog.h"

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
// xxx    _viewObject->setDialogLock(false);
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
// xxx    _viewObject->setDialogLock(true);
  }
  _top = top;
  updateWidgets();
  if (_viewObject) {
    if (_isNew) {
      setWindowTitle(_viewObject->newTitle());
    } else {
      setWindowTitle(_viewObject->editTitle());
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
    _editMultiple->setText(QObject::tr("Edit Multiple >>"));
    if (_viewObject && _viewObject->supportsDefaults()) {
      _buttonGroupDefaults->setEnabled(true);
      _pushButtonSetDefaults->setEnabled(true);
      _pushButtonRestoreDefaults->setEnabled(true);
    }
    updateWidgets();
  } else {
    _editMultipleWidget->show();
    _editMultiple->setText(QObject::tr("Edit Multiple <<"));
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

// xxx  windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);

    if (viewWindow) {
      if (_viewObject->type() == "TopLevelView") {
        list.append(viewWindow->view());
      } else {
        list += viewWindow->view()->findChildrenType(_viewObject->type(), true);
      }
    }
  }

  _editMultipleWidget->_objectList->addItems(list.tagNames());
}


void KstEditViewObjectDialog::populateEditMultiple() {
  QList<QWidget*>::const_iterator iter;
  QSpinBox *spinBoxWidget;
// xxx  KColorButton *colorButtonWidget;
// xxx  KURLRequester *urlRequester;
  QLineEdit *lineEditWidget;
  QCheckBox *checkBoxWidget;
  QComboBox *comboWidget;

  for (iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
    if ((spinBoxWidget = dynamic_cast<QSpinBox*>(*iter)) != 0L) {
      spinBoxWidget->setMinimum(spinBoxWidget->minimum() - 1);
      spinBoxWidget->setSpecialValueText(QString(" "));
      spinBoxWidget->setValue(spinBoxWidget->minimum());
/* xxx
    } else if ((colorButtonWidget = dynamic_cast<KColorButton*>(*iter)) != 0L) {
      colorButtonWidget->setColor(QColor());
    } else if ((urlRequester = dynamic_cast<KURLRequester*>(*iter)) != 0L) {
      urlRequester->lineEdit()->setText(QString(" "));
*/
    } else if ((lineEditWidget = dynamic_cast<QLineEdit*>(*iter)) != 0L) {
      lineEditWidget->setText(QString(" "));
    } else if ((checkBoxWidget = dynamic_cast<QCheckBox*>(*iter)) != 0L) {
      checkBoxWidget->setTristate();
      checkBoxWidget->setCheckState(Qt::PartiallyChecked);
    } else if ((comboWidget = dynamic_cast<QComboBox*>(*iter)) != 0L) {
      comboWidget->insertItem(0, QString(" "));
      comboWidget->setCurrentIndex(comboWidget->count()-1);
    }
  }
}


void KstEditViewObjectDialog::clearWidgets() {
  QList<QWidget*>::iterator i;

  //
  // clear all the current widgets from the grid...
  //

  for (i = _inputWidgets.begin(); i != _inputWidgets.end(); ++i) {
    delete *i;
  }

  _inputWidgets.clear();

  for (i = _widgets.begin(); i != _widgets.end(); ++i) {
    delete *i;
  }

  _widgets.clear();

  delete _customWidget;
  _customWidget = 0L;

  delete _grid;
  _grid = 0L;
}


void KstEditViewObjectDialog::updateDefaultWidgets() {
  int numProperties;

  numProperties = _viewObject->metaObject()->propertyCount();

  //
  // create a new grid...
  //

  _grid = new QGridLayout(_propertiesFrame);
  _grid->setContentsMargins(0, 0, 0, 0);
  _grid->setSpacing(8);
  _grid->setColumnStretch(0, 0);
  _grid->setColumnStretch(1, 1);

  //
  // get the property names and types...
  //

  for (int i = 0; i < numProperties; i++) {
    QMetaProperty property = _viewObject->metaObject()->property(i);
    QString propertyType(property.type());
    QString propertyName(property.name());

    //
    // for this property, get the meta-data map...
    //

    QMap<QString, QVariant> metaData = _viewObject->widgetHints(propertyName);

    if (!metaData.empty()) {
      QString friendlyName = metaData["_kst_label"].toString();
      QString widgetType = metaData["_kst_widgetType"].toString();
      QLabel* propertyLabel;

      metaData.remove("_kst_label");
      metaData.remove("_kst_widgetType");

      //
      // use friendly name for label...
      //

      propertyLabel = new QLabel(_propertiesFrame);
      propertyLabel->setObjectName("label-"+i);
      propertyLabel->setText(friendlyName);
      _grid->addWidget(propertyLabel, i, 0);
      propertyLabel->show();
      _widgets.append(propertyLabel);

      //
      // display different types of widgets depending on what dialogData specifies...
      //

      QWidget* propertyWidget = 0L;

      if (widgetType == "QSpinBox") {
        propertyWidget = new QSpinBox(_propertiesFrame);
        propertyWidget->setObjectName(propertyName+","+"value"); 
        propertyWidget->setProperty("value", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(valueChanged(const QString&)), this, SLOT(modified()));
// xxx          connect(propertyWidget->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KColorButton") {
/* xxx
        propertyWidget = new KColorButton(_propertiesFrame, (propertyName+","+"color").toLatin1());
        propertyWidget->setProperty("color", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(changed(const QColor&)), this, SLOT(modified()));
        }
*/
      } else if (widgetType == "QLineEdit") {
        propertyWidget = new QLineEdit(_propertiesFrame);
        propertyWidget->setObjectName(propertyName+","+"text");
        propertyWidget->setProperty("text", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KURLRequester") {
/* xxx
        propertyWidget = new KURLRequester(_propertiesFrame, (propertyName+","+"url").toLatin1());
        propertyWidget->setProperty("url", _viewObject->property(property->name()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
          connect(propertyWidget, SIGNAL(urlSelected(const QString&)), this, SLOT(modified()));
        }
*/
      } else if (widgetType == "PenStyleWidget") {
        QComboBox* combo;

        //
        // insert a combobox with QT pen styles...
        //

        combo = new QComboBox(_propertiesFrame);
        combo->setObjectName(propertyName+","+"currentItem");
        fillPenStyleWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      } else if (widgetType == "QCheckBox") {
        propertyWidget = new QCheckBox(_propertiesFrame);
        propertyWidget->setObjectName(propertyName+","+"checked");
        propertyWidget->setProperty("checked", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(pressed()), this, SLOT(modified()));
        }
      } else if (widgetType == "QDoubleSpinBox") {
        QDoubleSpinBox* input;

        input = new QDoubleSpinBox(_propertiesFrame);
        input->setObjectName(propertyName+","+"value");
        input->setDecimals(metaData["precision"].toInt());
        input->setSingleStep(metaData["lineStep"].toDouble());
        input->setMinimum(metaData["minValue"].toDouble());
        input->setMaximum(metaData["maxValue"].toDouble());

        metaData.remove("minValue");
        metaData.remove("maxValue");
        metaData.remove("lineStep");
        metaData.remove("precision");

        propertyWidget = input; 
        propertyWidget->setProperty("value", _viewObject->property(propertyName.toLatin1()));
 
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(valueChanged(double)), this, SLOT(modified()));
// xxx          connect(propertyWidget->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
        }
      } else if (widgetType == "KFontCombo") {
/* xxx
        propertyWidget = new KFontCombo(_propertiesFrame, (propertyName+","+"currentText").toLatin1());
        propertyWidget->setProperty("currentText", _viewObject->property(property->name()));  
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
*/
      } else if (widgetType == "HJustifyCombo") {
        QComboBox* combo;

        //
        // insert a combo box filled with horizontal justifications...
        //

        combo = new QComboBox(_propertiesFrame);
        combo->setObjectName(propertyName+","+"currentItem");
        fillHJustifyWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      } else if (widgetType == "VJustifyCombo") {
        QComboBox* combo;

        //
        // insert a combo box filled with vertical justifications...
        //

        combo = new QComboBox(_propertiesFrame);
        combo->setObjectName(propertyName+","+"currentItem");

        fillVJustifyWidget(combo);
        propertyWidget = combo;
        propertyWidget->setProperty("currentItem", _viewObject->property(propertyName.toLatin1()));
        if (!_isNew) {
          connect(propertyWidget, SIGNAL(activated(int)), this, SLOT(modified()));
        }
      }

      if (propertyWidget != 0L) {
        QMap<QString, QVariant>::const_iterator it;

        propertyWidget->show();

        //
        // also set any additional properties specified by metaData...
        //

        for (it = metaData.begin(); it != metaData.end(); ++it) {
          propertyWidget->setProperty(it.key().toLatin1(), (*it));
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
      _grid = new QGridLayout(_propertiesFrame);
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
  QList<Qt::PenStyle> styles;
  QRect rect;

// xxx  rect = widget->style().querySubControlMetrics(QStyle::CC_ComboBox, 
// xxx                              widget, QStyle::SC_ComboBoxEditField);

  rect.setLeft(rect.left() + 2);
  rect.setRight(rect.right() - 2);
  rect.setTop(rect.top() + 2);
  rect.setBottom(rect.bottom() - 2);

  QPixmap ppix(rect.width(), rect.height());
  QPainter pp(&ppix);
  QPen pen(Qt::black, 0);

  widget->clear();

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
// xxx    widget->insertItem(0, ppix);
    styles.pop_front();
  }
}


void KstEditViewObjectDialog::fillHJustifyWidget(QComboBox* widget) {
  widget->insertItem(0, QObject::tr("Left"));
  widget->insertItem(0, QObject::tr("Right"));
  widget->insertItem(0, QObject::tr("Center")); 
}


void KstEditViewObjectDialog::fillVJustifyWidget(QComboBox* widget) {
  widget->insertItem(0, QObject::tr("Top"));
  widget->insertItem(0,  QObject::tr("Bottom"));
  widget->insertItem(0, QObject::tr("Center")); 
}


void KstEditViewObjectDialog::modified() {
  _apply->setEnabled(true);
}


void KstEditViewObjectDialog::applySettings(KstViewObjectPtr viewObject) {
  if (viewObject && viewObject.data()) {
    if (_customWidget) {
      viewObject->readConfigWidget(_customWidget, _editMultipleMode);
    } else {
      QList<QWidget*>::const_iterator iter;

      //
      // get all the properties and set them...
      //

      for (iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
        if (_editMultipleMode) {
          QSpinBox *spinBoxWidget;
// xxx          KColorButton *colorButtonWidget;
// xxx          KURLRequester *urlRequester;
          QLineEdit *lineEditWidget;
          QCheckBox *checkBoxWidget;
          QComboBox *comboWidget;
          bool edited = false;

          if ((spinBoxWidget = dynamic_cast<QSpinBox*>(*iter)) != 0L) {
            if (spinBoxWidget->value() != spinBoxWidget->minimum()) {
              edited = true;
            }
/* xxx
          } else if ((colorButtonWidget = dynamic_cast<KColorButton*>(*iter)) != 0L) {
            if (colorButtonWidget->color() != QColor()) {
              edited = true;
            }
          } else if ((urlRequester = dynamic_cast<KURLRequester*>(*iter)) != 0L) {
            if (urlRequester->lineEdit()->text().compare(QString(" ")) != 0) {
              edited = true;
            }
*/
          } else if ((lineEditWidget = dynamic_cast<QLineEdit*>(*iter)) != 0L) {
            if (lineEditWidget->text().compare(QString(" ")) != 0 ) {
              edited = true;
            }
          } else if ((checkBoxWidget = dynamic_cast<QCheckBox*>(*iter)) != 0L) {
            if (checkBoxWidget->checkState() != Qt::PartiallyChecked) {
              edited = true;
            }
          } else if ((comboWidget = dynamic_cast<QComboBox*>(*iter)) != 0L) {
            if (comboWidget->currentText().compare(QString(" ")) != 0) {
              edited = true;
            }
          }

          if (edited) {
            QString propertyName = QString((*iter)->objectName()).section(',', 0, 0);
            QString widgetPropertyName = QString((*iter)->objectName()).section(',', 1, 1);

            viewObject->setProperty(propertyName.toLatin1(), (*iter)->property(widgetPropertyName.toLatin1()));
          }
        } else {
          QString propertyName = QString((*iter)->objectName()).section(',', 0, 0);
          QString widgetPropertyName = QString((*iter)->objectName()).section(',', 1, 1);

          viewObject->setProperty(propertyName.toLatin1(), (*iter)->property(widgetPropertyName.toLatin1()));
        }
      }
    }
  }
}


bool KstEditViewObjectDialog::apply() {
  bool applied = false;
  int i;

  if (_editMultipleMode) {
    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      QString name = _editMultipleWidget->_objectList->item(i)->text();

      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        QList<QMdiSubWindow*> windows;
        QList<QMdiSubWindow*>::const_iterator i;
      
        windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);
      
        for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
          KstViewWindow *viewWindow;

          viewWindow = dynamic_cast<KstViewWindow*>(*i);
          if (viewWindow) {
            KstViewObjectPtr viewObject;

            if (_viewObject->type() == "TopLevelView") {
              viewObject = viewWindow->view();
              if (viewObject->objectName() == name) {
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
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select one or more objects to edit."));
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
    KstViewObjectPtr viewObject;

    viewObject = _viewObject->copyObjectQuietly();
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
    QList<QWidget*>::const_iterator iter;

    for (iter = _inputWidgets.begin(); iter != _inputWidgets.end(); ++iter) {
      if ((*iter)->objectName().compare(QString("lineStyle,currentItem")) == 0) {
        QComboBox* combo;

        combo = dynamic_cast<QComboBox*>(*iter);
        if (combo != 0L) {
          int currentItem = combo->currentIndex();

          fillPenStyleWidget(combo);
          if (_editMultipleMode) {
            combo->insertItem(0, QString(" "));
          }
          combo->setCurrentIndex(currentItem);
        }
      }
    }
  }
}

#include "ksteditviewobjectdialog.moc"
