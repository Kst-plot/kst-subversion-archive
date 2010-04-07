/***************************************************************************
                       kstpluginmanager.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
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

#ifndef KSTPLUGINMANAGERI_H
#define KSTPLUGINMANAGERI_H

#include <QListWidgetItem>

#include "ui_pluginmanager.h"

class KstPluginManager : public QDialog, public Ui::PluginManager {
  Q_OBJECT
  public:
    KstPluginManager(QWidget* parent = 0, const char* name = 0,
        bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstPluginManager();

  public slots:
    void selectionChanged(QListWidgetItem *item);
    void install();
    void remove();
    void rescan();

  private:
    void reloadList();

  signals:
    void rescanned();
};

#endif
