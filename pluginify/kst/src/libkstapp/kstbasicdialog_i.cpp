/***************************************************************************
                     kstbasicdialog_i.cpp  -  Part of KST
                             -------------------
    begin                : 09/15/06
    copyright            : (C) 2006 The University of Toronto
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

// include files for Qt
#include <qvbox.h>
#include <qlayout.h>
#include <qlineedit.h>

// include files for KDE
#include <klocale.h>
#include <kmessagebox.h>

#include "kstbasicdialog_i.h"
#include "basicdialogwidget.h"

// application specific includes
#include "kst.h"
#include "kstdoc.h"
#include "scalarselector.h"
#include "stringselector.h"
#include "vectorselector.h"
#include "kstdefaultnames.h"
#include "kstdataobjectcollection.h"

const QString& KstBasicDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstBasicDialogI> KstBasicDialogI::_inst;

KstBasicDialogI *KstBasicDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstBasicDialogI(KstApp::inst());
  }
  return _inst;
}


KstBasicDialogI::KstBasicDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  setMultiple(false);
  _w = new BasicDialogWidget(_contents);
  connect(this, SIGNAL(modified()), KstApp::inst()->document(), SLOT(wasModified())); //FIXME this should be in KstDataDialog constructor...

  _pluginName = QString::null;
  _inputOutputGrid = 0L;
}


KstBasicDialogI::~KstBasicDialogI() {
}

void KstBasicDialogI::init() {

  KstBasicPluginPtr ptr;
  if (!_newDialog)
    ptr = kst_cast<KstBasicPlugin>(_dp);
  else
    ptr = kst_cast<KstBasicPlugin>(
        KstDataObject::plugin(_pluginName));

  Q_ASSERT(ptr); //shouldn't happen

  int cnt = 0;
  int numInputOutputs = ptr->inputVectors().count()
                      + ptr->inputScalars().count()
                      + ptr->inputStrings().count()
                      + ptr->outputVectors().count()
                      + ptr->outputScalars().count()
                      + ptr->outputStrings().count();

  _inputOutputGrid = new QGridLayout(_w->_inputOutputFrame, numInputOutputs + 1, 2, 0, 8);
  _inputOutputGrid->setColStretch(1,1);
  _inputOutputGrid->setColStretch(0,0);

  //create input widgets
  //First, the inputVectors...
  QStringList iv = ptr->inputVectors();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI) {
      createInputVector(*ivI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }

  //Now, the inputScalars...
  QStringList is = ptr->inputScalars();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI) {
      createInputScalar(*isI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }

  //Finally, the inputStrings...
  QStringList istr = ptr->inputStrings();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI) {
      createInputString(*istrI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }

  //create sep
  cnt++;
  QFrame* line = new QFrame(_w->_inputOutputFrame);
  line->setFrameShadow(QFrame::Sunken);
  line->setFrameShape(QFrame::HLine);
  _inputOutputGrid->addMultiCellWidget(line, cnt, cnt, 0, 1);
  line->show();
  cnt++;

  //create output widgets
  //output vectors...
  QStringList ov = ptr->outputVectors();
  QStringList::ConstIterator ovI = ov.begin();
  for (; ovI != ov.end(); ++ovI) {
      createOutputWidget(*ovI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }

  //output scalars...
  QStringList os = ptr->outputScalars();
  QStringList::ConstIterator osI = os.begin();
  for (; osI != os.end(); ++osI) {
      createOutputWidget(*osI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }

  //ouput strings...
  QStringList ostr = ptr->outputStrings();
  QStringList::ConstIterator ostrI = ostr.begin();
  for (; ostrI != ostr.end(); ++ostrI) {
      createOutputWidget(*ostrI,
                          _w->_inputOutputFrame,
                          _inputOutputGrid, ++cnt);
  }
}


void KstBasicDialogI::createInputVector(const QString &name, QWidget *parent, QGridLayout *grid, int row) {
  QLabel *label = new QLabel(name, parent);

  VectorSelector *widget = new VectorSelector(parent,
                                              name.latin1());
  connect(widget, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));

  grid->addWidget(label, row, 0);
  label->show();
  grid->addWidget(widget, row, 1);
  widget->show();
}


void KstBasicDialogI::createInputScalar(const QString &name, QWidget *parent, QGridLayout *grid, int row) {
  QLabel *label = new QLabel(name, parent);

  ScalarSelector *widget = new ScalarSelector(parent,
                                              name.latin1());
  connect(widget, SIGNAL(newScalarCreated()),
          this, SIGNAL(modified()));
  widget->allowDirectEntry(true);

  grid->addWidget(label, row, 0);
  label->show();
  grid->addWidget(widget, row, 1);
  widget->show();
}


void KstBasicDialogI::createInputString(const QString &name, QWidget *parent, QGridLayout *grid, int row) {
  QLabel *label = new QLabel(name, parent);

  StringSelector *widget = new StringSelector(parent,
                                              name.latin1());
  connect(widget, SIGNAL(newStringCreated()),
          this, SIGNAL(modified()));

  grid->addWidget(label, row, 0);
  label->show();
  grid->addWidget(widget, row, 1);
  widget->show();
}


void KstBasicDialogI::createOutputWidget(const QString &name, QWidget *parent, QGridLayout *grid, int row) {
  QLabel *label = new QLabel(name, parent);
  QLineEdit *widget = new QLineEdit(parent, name.latin1());
  grid->addWidget(label, row, 0);
  label->show();
  grid->addWidget(widget, row, 1);
  widget->show();
}


void KstBasicDialogI::update() {
  //called upon showing the dialog either in 'edit' mode or 'new' mode
}


bool KstBasicDialogI::newObject() {
  //called upon clicking 'ok' in 'new' mode
  //return false if the specified objects can't be made, otherwise true

  //Need to create a new object rather than use the one in KstDataObject pluginList
  KstBasicPluginPtr ptr = kst_cast<KstBasicPlugin>(
      KstDataObject::createPlugin(_pluginName));
  Q_ASSERT(ptr); //should never happen...

  ptr->writeLock();

  QString tagName = _tagName->text();

  if (tagName != defaultTag && KstData::self()->dataTagNameNotUnique(tagName, true, this)) {
    _tagName->setFocus();
    return false;
  }

  if (tagName == defaultTag) {
    tagName = KST::suggestPluginName(ptr->propertyString());
  }
  ptr->setTagName(tagName);

  ptr->unlock();

  // Save the vectors and scalars
  if (!editSingleObject(ptr) || !ptr->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the values you entered."));
    return false;
  }

  //set the outputs
  //output vectors...
  QStringList ov = ptr->outputVectors();
  QStringList::ConstIterator ovI = ov.begin();
  for (; ovI != ov.end(); ++ovI) {
    if (QLineEdit *w = output(*ovI)) {
      ptr->setOutputVector(*ovI, w->text());
    }
  }

  //output scalars...
  QStringList os = ptr->outputScalars();
  QStringList::ConstIterator osI = os.begin();
  for (; osI != os.end(); ++osI) {
    if (QLineEdit *w = output(*osI)) {
      ptr->setOutputScalar(*ovI, w->text());
    }
  }

  //ouput strings...
  QStringList ostr = ptr->outputStrings();
  QStringList::ConstIterator ostrI = ostr.begin();
  for (; ostrI != ostr.end(); ++ostrI) {
    if (QLineEdit *w = output(*ostrI)) {
      ptr->setOutputString(*ovI, w->text());
    }
  }

  if (!ptr || !ptr->isValid()) {
    KMessageBox::sorry(this, i18n("There is an error in the plugin you entered."));
    return false;
  }

  ptr->setDirty();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(ptr.data());
  KST::dataObjectList.lock().unlock();
  ptr = 0L; // drop the reference
  emit modified();

  return true;
}


bool KstBasicDialogI::editObject() {
  //called upon clicking 'ok' in 'edit' mode
  //return false if the specified objects can't be editted, otherwise true
  return true;
}


void KstBasicDialogI::showNew(const QString &field) {
  if (_pluginName != field) {
    _pluginName = field;
    KstDataDialog::showNew(field);
    init();
  } else {
    KstDataDialog::showNew(field);
  }
}


void KstBasicDialogI::showEdit(const QString &field) {
  if (_pluginName != field) {
    _pluginName = field;
    KstDataDialog::showEdit(field);
    init();
  } else {
    KstDataDialog::showEdit(field);
  }
}


bool KstBasicDialogI::editSingleObject(KstBasicPluginPtr ptr) {
  Q_UNUSED(ptr)
  return true;
}


void KstBasicDialogI::fillFieldsForEdit() {

  KstBasicPluginPtr ptr = kst_cast<KstBasicPlugin>(_dp);
  if (!ptr)
    return; //shouldn't happen

  ptr->readLock();

  _tagName->setText(ptr->tagName());
  _legendText->setText(defaultTag); //FIXME?

  //Update the various widgets...

  //input vectors...
  QStringList iv = ptr->inputVectors();
  QStringList::ConstIterator ivI = iv.begin();
  for (; ivI != iv.end(); ++ivI) {
    KstVectorPtr p = ptr->inputVector(*ivI);
    QString t = p ? p->tagName() : QString::null;
    if (VectorSelector *w = vector(*ivI)) {
      w->setSelection(t);
    }
  }

  //input scalars...
  QStringList is = ptr->inputScalars();
  QStringList::ConstIterator isI = is.begin();
  for (; isI != is.end(); ++isI) {
    KstScalarPtr p = ptr->inputScalar(*isI);
    QString t = p ? p->tagName() : QString::null;
    if (ScalarSelector *w = scalar(*isI)) {
      w->setSelection(t);
    }
  }

  //input strings...
  QStringList istr = ptr->inputStrings();
  QStringList::ConstIterator istrI = istr.begin();
  for (; istrI != istr.end(); ++istrI) {
    KstStringPtr p = ptr->inputString(*istrI);
    QString t = p ? p->tagName() : QString::null;
    if (StringSelector *w = string(*istrI)) {
      w->setSelection(t);
    }
  }

  //output vectors...
  QStringList ov = ptr->outputVectors();
  QStringList::ConstIterator ovI = ov.begin();
  for (; ovI != ov.end(); ++ovI) {
    KstVectorPtr p = ptr->outputVector(*ovI);
    QString t = p ? p->tagName() : QString::null;
    if (QLineEdit *w = output(*ovI)) {
      w->setText(t);
      w->setEnabled(false);
    }
  }

  //output scalars...
  QStringList os = ptr->outputScalars();
  QStringList::ConstIterator osI = os.begin();
  for (; osI != os.end(); ++osI) {
    KstScalarPtr p = ptr->outputScalar(*osI);
    QString t = p ? p->tagName() : QString::null;
    if (QLineEdit *w = output(*osI)) {
      w->setText(t);
      w->setEnabled(false);
    }
  }

  //ouput strings...
  QStringList ostr = ptr->outputStrings();
  QStringList::ConstIterator ostrI = ostr.begin();
  for (; ostrI != ostr.end(); ++ostrI) {
    KstStringPtr p = ptr->outputString(*ostrI);
    QString t = p ? p->tagName() : QString::null;
    if (QLineEdit *w = output(*ostrI)) {
      w->setText(t);
      w->setEnabled(false);
    }
  }

  ptr->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstBasicDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


VectorSelector *KstBasicDialogI::vector(const QString &name) const {
  return ::qt_cast<VectorSelector*>(_w->_inputOutputFrame->child(name.latin1()));
}


ScalarSelector *KstBasicDialogI::scalar(const QString &name) const {
  return ::qt_cast<ScalarSelector*>(_w->_inputOutputFrame->child(name.latin1()));
}


StringSelector *KstBasicDialogI::string(const QString &name) const {
  return ::qt_cast<StringSelector*>(_w->_inputOutputFrame->child(name.latin1()));
}


QLineEdit *KstBasicDialogI::output(const QString &name) const {
  return ::qt_cast<QLineEdit*>(_w->_inputOutputFrame->child(name.latin1()));
}

#include "kstbasicdialog_i.moc"

// vim: ts=2 sw=2 et
