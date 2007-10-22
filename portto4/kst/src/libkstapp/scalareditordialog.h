/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SCALAREDITORDIALOG_H
#define SCALAREDITORDIALOG_H

#include <QDialog>

#include "ui_scalareditordialog.h"

#include "kst_export.h"

namespace Kst {

class Document;
class ScalarTableModel;

class KST_EXPORT ScalarEditorDialog : public QDialog, Ui::ScalarEditorDialog
{
  Q_OBJECT
  public:
    ScalarEditorDialog(QWidget *parent, Document *doc);
    virtual ~ScalarEditorDialog();

    virtual void show();

  private:
    void refreshScalars();

    Document *_doc;
    ScalarTableModel *_model;
};

}

#endif

// vim: ts=2 sw=2 et
