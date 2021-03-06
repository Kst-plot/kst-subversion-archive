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

#ifndef CHANGEDATASAMPLEDIALOG_H
#define CHANGEDATASAMPLEDIALOG_H

#include <QDialog>

#include "ui_changedatasampledialog.h"

#include "kst_export.h"

namespace Kst {

class KST_EXPORT ChangeDataSampleDialog : public QDialog, Ui::ChangeDataSampleDialog
{
  Q_OBJECT
  public:
    ChangeDataSampleDialog(QWidget *parent);
    virtual ~ChangeDataSampleDialog();

    void exec();

  public slots:
    void selectAll();

  private:
    void updateCurveListDialog();

};

}

#endif

// vim: ts=2 sw=2 et
