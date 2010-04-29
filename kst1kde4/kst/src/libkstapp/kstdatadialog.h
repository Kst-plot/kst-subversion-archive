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

#ifndef KSTDATADIALOG_H
#define KSTDATADIALOG_H

#include "kst.h"
#include "kstdoc.h"
#include "ui_kstdatadialog.h"

class KST_EXPORT KstDataDialog : public QDialog, public Ui::KstDataDialog
{
   Q_OBJECT
public:
  KstDataDialog(QWidget *parent = 0);
  virtual ~KstDataDialog(); 

  QString editTitle();
  QString newTitle();
  void fillFieldsForEdit();
  void fillFieldsForNew();
  KstObjectPtr findObject( const QString & name );
  void populateEditMultiple();
  void setMultiple(bool multiple);
  void closeEvent(QCloseEvent *e);
  void cleanup();

public slots:
  bool multiple();
  bool newObject();
  bool editObject();
  void update();
  void show();
  void wasModifiedApply();
  void toggleEditMultiple();
  void showNew(const QString& field);
  void showEdit(const QString& field);

private slots:
  void ok();
  void apply();
  void close();
  void reject();

signals:
  void modified();

protected:
  KstObjectPtr _dp;
  bool _editMultipleMode;
  bool _multiple;
  bool _newDialog;

private:

};

#endif

