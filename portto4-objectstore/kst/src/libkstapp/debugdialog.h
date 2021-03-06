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

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>

#include "ui_debugdialog.h"

#include "kst_export.h"

namespace Kst {

class KST_EXPORT DebugDialog : public QDialog, Ui::DebugDialog
{
  Q_OBJECT
  public:
    DebugDialog(QWidget *parent);
    virtual ~DebugDialog();

  Q_SIGNALS:
    void notifyOfError();
    void notifyAllClear();

  protected:
    bool event(QEvent *e);
};

}

#endif

// vim: ts=2 sw=2 et
