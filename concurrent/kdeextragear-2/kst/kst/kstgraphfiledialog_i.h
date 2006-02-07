/***************************************************************************
                       kstgraphdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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
#ifndef KSTGRAPHFILEDIALOGI_H
#define KSTGRAPHFILEDIALOGI_H

#include "graphfiledialog.h"

class KstGraphFileDialogI : public KstGraphFileDialog {
  Q_OBJECT
public:
  KstGraphFileDialogI(QWidget* parent = 0, const char* name = 0,
                     bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstGraphFileDialogI();

public slots:
  /** shows/raises the dialog */
  void showGraphFileDialog();
  void reqGraphFile();

private slots:
  void xsizeChanged(int x);
  void setAutoSave();
  void square();

private:
  QTimer *autoSaveTimer;

signals:
  void graphFileReq(const QString &filename, int w, int h);
};

#endif
