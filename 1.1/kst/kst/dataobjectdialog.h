/***************************************************************************
                      dataobjectdialog.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of Toronto
                           (C) 2004 C. Barth Netterfield
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
// Include methods common to all datadialogs.
// usage example: kstvectordialog_I
// In qtdesigner, define the buttons _OK, _cancel, and
//    the line edit _tagName
// In kstvectordialog_i.h declare the public slots (defined here)
//    public:
//    void show_Edit(const QString &field);
//    void show_New();
//    void OK();
//    void Init();
//    void close();
//    void reject();
//    private:
//    void _fillFieldsForEdit();
//    void _fillFieldsForNew();
//
//      KstRVectorPtr getPtr(const QString &tagin) {
//          return kstptr to tagin, or 0L 
// In kstvectordialog_i.h declare the private variable
//    bool _newDialog
//    KstRVectorPtr DP
// In kstvectordialog_i.cpp:
//   #define DIALOGTYPE KstVectorDialogI
//   #define DTYPE "Vector"
//   #include "dataobjectdialog.h"
//   Call Init() in the constructor, and DP = 0L in the destructor
// Additionally, new_I, edit_I, and update(int new_index)
//   must be delcared and defined.
void DIALOGTYPE::Init() {
  connect(_OK, SIGNAL(clicked()), this, SLOT(OK()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  _newDialog = false;
  DP = 0L;
}

void DIALOGTYPE::show_Edit(const QString &field) {
  DP = _getPtr(field);
  _newDialog = false;

  update();
  if (DP==0L) show_New(); // shouldn't ever happen.
  _fillFieldsForEdit();

  setCaption(i18n("Edit %1").arg(DTYPE));
  show();
  raise();
  _OK->setEnabled(true);
  _cancel->setEnabled(true);
}

/** show_new:
    show as a newVector dialog: disable/hide editing related things and show ok/cancel */
void DIALOGTYPE::show_New() {
  _newDialog = true;
  DP = 0L;

  update();
  _fillFieldsForNew();

  setCaption(i18n("New %1").arg(DTYPE));
  show();
  raise();
  _OK->setEnabled(true);
  _cancel->setEnabled(true);
}

void DIALOGTYPE::OK() {
  _OK->setEnabled(false);
  _cancel->setEnabled(false);
  if (_newDialog || (DP == 0L)) {
    if (new_I()) {
      close();
    } else {
      _OK->setEnabled(true);
      _cancel->setEnabled(true);
    }
  } else {
    if (edit_I()) {
      close();
    } else {
      _OK->setEnabled(true);
      _cancel->setEnabled(true);
    }
  }
}

void DIALOGTYPE::close() {
  DP = 0L;
  QDialog::close();
}

void DIALOGTYPE::reject() {
  DP = 0L;
  QDialog::reject();
}

#undef DIALOGTYPE
#undef DTYPE
