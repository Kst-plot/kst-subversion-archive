/***************************************************************************
                   pluginmanager.h
                             -------------------
    begin                : 02/28/07
    copyright            : (C) 2007 The University of Toronto
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QWidget>
#include "ui_pluginmanager4.h"

class PluginManager : public QWidget, public Ui::PluginManager {
  Q_OBJECT

public:
  PluginManager(QWidget *parent = 0);
  ~PluginManager();

private slots:
  void init();
  void selectionChanged(QListViewItem *item);
  void install();
  void remove();
  void rescan();

private:
  void reloadList();

signals:
  void rescanned();
};

#endif
// vim: ts=2 sw=2 et
