/**************************************************************************
        kstmonochromedialog.h - source file
                             -------------------
    begin                :  2005
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

#ifndef KSTMONOCHROMEDIALOGI_H
#define KSTMONOCHROMEDIALOGI_H

#include "ui_monochromedialog.h"

class KstMonochromeDialog : public QDialog, public Ui::KstMonochromeDialog {
  Q_OBJECT
  public:
    KstMonochromeDialog(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = TRUE, Qt::WindowFlags fl = 0 );
    virtual ~KstMonochromeDialog();

    void setOptions(const QMap<QString,QString>& opts);
    void getOptions(QMap<QString,QString> &opts, bool include_def = false);

  public slots:

    void updateMonochromeDialog();

    /** calls updateMonochromeDialog(), then shows and raises the dialog */
    void showMonochromeDialog();

  private slots:
    void updateButtons();
    void removeClicked();
    void addClicked();
    void upClicked();
    void downClicked();
};

#endif
