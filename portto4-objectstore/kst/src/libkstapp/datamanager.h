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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QDialog>

#include "ui_datamanager.h"

#include "kst_export.h"

class QToolBar;

namespace Kst {

class Document;

class KST_EXPORT DataManager : public QDialog, Ui::DataManager
{
  Q_OBJECT
  public:
    DataManager(QWidget *parent, Document *doc);
    virtual ~DataManager();

  private:
    Document *_doc;

    QToolBar *_primitives;
    QToolBar *_dataObjects;
    QToolBar *_fits;
    QToolBar *_filters;
};

}

#endif

// vim: ts=2 sw=2 et
