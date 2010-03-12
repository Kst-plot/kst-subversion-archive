/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <QMessageBox>
#include <QTimer>

#include "enodes.h"
#include "scalareditor.h"
#include "scalarselector.h"
#include "dialoglauncher.h"
#include "kstdatacollection.h"

ScalarSelector::ScalarSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  update();
// xxx _newScalar->setIcon(QPixmap(BarIcon("kst_scalarnew"));
// xxx _editScalar->setPixmap(BarIcon("kst_scalaredit"));

  connect(_selectScalar, SIGNAL(clicked()), this, SLOT(selectScalar()));
  connect(_newScalar, SIGNAL(clicked()), this, SLOT(createNewScalar()));
  connect(_editScalar, SIGNAL(clicked()), this, SLOT(editScalar()));
  connect(_scalar, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
  connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
}

ScalarSelector::~ScalarSelector() {
}

void ScalarSelector::allowNewScalars( bool allowed )
{
  _newScalar->setEnabled(allowed);
}

void ScalarSelector::update()
{
/* xxx
  if (_scalar->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));
    return;
  }
*/
  blockSignals(true);

  QStringList scalars;
  QString prev = _scalar->currentText();
  bool found = false;
  bool ok;

  _scalar->clear();

  KST::scalarList.lock().readLock();
  for (KstScalarList::Iterator i = KST::scalarList.begin(); i != KST::scalarList.end(); ++i) {
    (*i)->readLock();
    QString tag = (*i)->tag().displayString();
    if ((*i)->displayable()) {
      scalars << tag;
    }
    (*i)->unlock();

    if (tag == prev) {
      found = true;
    }
  }
  KST::scalarList.lock().unlock();

// xxx  qHeapSort(scalars);
  _scalar->insertItems(0, scalars);

  prev.toDouble(&ok);
  ok = ok && _scalar->isEditable(); 

  //
  // if directentry is allowed, and if prev is a number, 
  //  then we can put it back into the combobox...
  //

  if (found || ok) {
    int index;

    index = _scalar->findText(prev);
    if (index != -1) {
      _scalar->setCurrentIndex(index);
    }
  } else {
    _scalar->insertItem(0, "0");
    _scalar->setCurrentIndex(0);
    _editScalar->setEnabled(false);
  }

  blockSignals(false);
}

void ScalarSelector::createNewScalar()
{
  ScalarEditor *se = new ScalarEditor(this);
  se->setWindowTitle(tr("New Scalar"));

  int rc = se->exec();
  if (rc == QDialog::Accepted) {
    bool ok = false;
    double val = se->_value->text().toFloat(&ok);

    if (!ok) {
      val = Equation::interpret(se->_value->text().toLatin1(), &ok);
    }

    if (ok) {
      KstScalarPtr scalar;
      QString name = se->_name->text();

      name.remove('[');
      name.remove(']');
      scalar = new KstScalar(KstObjectTag(name, KstObjectTag::globalTagContext), 0L, val);

      if (scalar) {
        scalar->setOrphan(true);
        scalar->setEditable(true);
// xxx        emit newScalarCreated();
        update();
        setSelection(scalar);
      }
      _editScalar->setEnabled(true);
    } else {
      QMessageBox::warning(this, tr("Kst"), tr("Error creating the scalar. Please enter a valid value."));
    }
  }

  delete se;
}

void ScalarSelector::selectScalar()
{
/* xxx
  ComboBoxSelectionI *selection = new ComboBoxSelectionI(this, "scalar selector");
  int i;

  selection->reset();
  for (i=0; i<_scalar->count(); i++) {
    selection->addString(_scalar->text(i));
  }
  selection->sort();
  int rc = selection->exec();
  if (rc == QDialog::Accepted) {
    _scalar->setCurrentText(selection->selected());
  }

  delete selection;
*/
}

void ScalarSelector::editScalar()
{
  ScalarEditor *se = new ScalarEditor(this);
  KstScalarPtr scalarOld = *KST::scalarList.findTag(_scalar->currentText());

  if (scalarOld && scalarOld->editable()) {
    se->_value->setText(QString::number(scalarOld->value()));
    se->_name->setText(scalarOld->tagName());
    se->_value->selectAll();
    se->_value->setFocus();
  }

  se->setWindowTitle(tr("Edit Scalar"));

  int rc = se->exec();
  if (rc == QDialog::Accepted) {
    bool ok = false;
    double val = se->_value->text().toFloat(&ok);

    if (!ok) {
      val = Equation::interpret(se->_value->text().toLatin1(), &ok);
    }

    if (ok) {
      KstScalarPtr scalar;
      QString name = se->_name->text();

      scalar = *KST::scalarList.findTag(name);
      if (scalar) {
        scalar->setValue(val);
        setSelection(scalar);
      } else {
        name.remove('[');
        name.remove(']');
        scalar = new KstScalar(KstObjectTag(name, KstObjectTag::globalTagContext), 0L, val);
        if (scalar) {
          scalar->setOrphan(true);
          scalar->setEditable(true);
// xxx          emit newScalarCreated();
          update();
          setSelection(scalar);
        }
        _editScalar->setEnabled(true);
      }
    } else {
      QMessageBox::warning(this, tr("Kst"), tr("Error changing the scalar. Please enter a valid value."));
    }
  }

  delete se;
}

void ScalarSelector::selectionWatcher( const QString & tag )
{
  bool editable = false;

  QString label = "["+tag+"]";
// xxx  emit selectionChangedLabel(label);
  KST::scalarList.lock().readLock();
  KstScalarPtr p = *KST::scalarList.findTag(tag);
  if (p && p->editable()) {
    editable = true;
  }
  KST::scalarList.lock().unlock();
  _editScalar->setEnabled(editable);
}

void ScalarSelector::setSelection( const QString & tag )
{
  if (!tag.isEmpty()) {
    blockSignals(true);
// xxx    _scalar->setCurrentText(tag);
    selectionWatcher(tag);
    blockSignals(false);
  }
}

void ScalarSelector::setSelection( KstScalarPtr s )
{
  setSelection(s->tagName());
}

QString ScalarSelector::selectedScalar()
{
  KstScalarPtr ptr = *KST::scalarList.findTag(_scalar->currentText());
  if (ptr) {
    return _scalar->currentText();
  } else {
    return QString::null;
  }
}

//
// returns a pointer to the selected scalar.
// If the scalar does not exist, but the text entered is a 
//  number and the selector is editable, then a new scalar 
//  is created with the given value and returned.
//

KstScalarPtr ScalarSelector::selectedScalarPtr()
{
  KstScalarPtr ptr = *KST::scalarList.findTag(_scalar->currentText());

  if (!ptr) {
    if (_scalar->isEditable()) {
      KstWriteLocker sl(&KST::scalarList.lock());
      bool ok;
      double val = _scalar->currentText().toDouble(&ok);

      if (ok) {
        ptr = new KstScalar(KstObjectTag::fromString(_scalar->currentText()), 0L, val, true, false);
      }
    }
  }

  return ptr;
}

void ScalarSelector::allowDirectEntry( bool allowed )
{
  _scalar->setEditable(allowed);
}
