/**************************************************************************
        kstchangefiledialog_i.h - source file: inherits designer dialog
                             -------------------
    begin                :  2001
    copyright            : (C) 2000-2003 by Barth Netterfield
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

#ifndef KSTCHANGEFILEDIALOGI_H
#define KSTCHANGEFILEDIALOGI_H

#include "changefiledialog.h"

class KstChangeFileDialogI : public KstChangeFileDialog {
  Q_OBJECT
  public:
    KstChangeFileDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstChangeFileDialogI();

  public slots:
    void updateChangeFileDialog();
    void showChangeFileDialog();
    void selectAll();
    void allFromFile();

  private:
    QPointer<QWidget> _configWidget;
    QString _file;
    int _lastVectorIndex;
    bool _first;

  private slots:
    bool applyFileChange();
    void OKFileChange();
    void updateSelection(const QString&);
    void sourceChanged(const QString& text);
    void configureSource();
    void markSourceAndSave();

  signals:
    void docChanged();
};

#endif
