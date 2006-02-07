/***************************************************************************
                      kstplugindialog_i.h  -  Part of KST
                             -------------------
    begin                : Mon May 12 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef KSTPLUGINDIALOGI_H
#define KSTPLUGINDIALOGI_H

#include <qstringlist.h>
#include "kstplugin.h"

class KstDoc;

#include "plugindialog.h"

class KstPluginDialogI : public KstPluginDialog {
  Q_OBJECT
public:
  KstPluginDialogI(QWidget* parent = 0, const char* name = 0,
                   bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstPluginDialogI();
public slots:
  /** update the entries in the plugin dialog to represent current plugins */
  void update(int new_index = -1);

  /** Calls update(), then shows/raises the dialog */
  void show_I(const QString &field = QString::null);
  void show_New();

  void new_I();
  void edit_I();
  void delete_I();

  void updatePluginList();

  static KstPluginDialogI *globalInstance();

private slots:
  void pluginChanged(int);
  void showPluginManager();
  void fixupLayout();
  void updateScalarTooltip(const QString& n);

private:
  QStringList _pluginList;
  QWidget *_frameWidget;
  static KstPluginDialogI* _inst;

  bool saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
  bool saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
  void generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table);
};

#endif
// vim: ts=2 sw=2 et
