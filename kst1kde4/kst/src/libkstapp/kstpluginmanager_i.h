/***************************************************************************
                       kstpluginmanager_i.h  -  Part of KST
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

#include "pluginmanager.h"

class KstPluginManagerI : public PluginManager {
  Q_OBJECT
  public:
    KstPluginManagerI(QWidget* parent = 0, const char* name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstPluginManagerI();

  public slots:
    void selectionChanged( QListViewItem *item );
    void install();
    void remove();
    void rescan();

  private:
    void reloadList();

  signals:
    void rescanned();
};

#endif
