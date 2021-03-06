/***************************************************************************
                       kstdebugdialog.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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
#ifndef KSTDEBUGI_H
#define KSTDEBUGI_H

#include <QDialog>

#include "ui_kstdebugdialog4.h"

class KstLogWidget;

class KstDebugDialogI : public QDialog, public Ui::KstDebugDialog {
  Q_OBJECT
  public:
    KstDebugDialogI(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    virtual ~KstDebugDialogI();

  public:
    KstLogWidget *logWidget() const;

  public slots:
    void email();
    void clear();
    void show_I();

  private:
    void init();

  private:
    KstLogWidget *_log;
};

#endif
// vim: ts=2 sw=2 et
