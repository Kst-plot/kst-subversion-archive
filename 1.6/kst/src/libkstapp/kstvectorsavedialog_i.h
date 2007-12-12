/***************************************************************************
                       kstvectorsavedialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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

#ifndef KSTVECTORSAVEDIALOGI_H
#define KSTVECTORSAVEDIALOGI_H

#include "vectorsavedialog.h"

class KstVectorSaveDialogI : public VectorSaveDialog {
  Q_OBJECT
  public:
    KstVectorSaveDialogI(QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = false, WFlags fl = 0 );
    virtual ~KstVectorSaveDialogI();

  public slots:
    void init();
    void show();

  private slots:
    void selectionChanged();
    void save();
};

#endif
