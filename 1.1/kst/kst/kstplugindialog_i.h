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

#include "kglobal.h"

#include "kstplugin.h"

#include "plugindialog.h"

static const QString& plugin_defaultTag = KGlobal::staticQString("<Auto Name>");

class KstPluginDialogI : public KstPluginDialog {
  Q_OBJECT
  public:
    KstPluginDialogI(QWidget* parent = 0, const char* name = 0,
        bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstPluginDialogI();
  public slots:
    /** update the entries in the plugin dialog to represent current plugins */
    void updateForm();
    virtual void update();

    virtual bool new_I();
    bool edit_I();

    virtual void updatePluginList();

    static KstPluginDialogI *globalInstance();

  protected slots:
    void pluginChanged(int);
    void showPluginManager();
    void fixupLayout();
    void updateScalarTooltip(const QString& n);
    void updateStringTooltip(const QString& n);

  protected:
    QStringList _pluginList;
    
    
    void fillVectorScalarCombos(KstSharedPtr<Plugin> pPtr);
    virtual bool saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
    bool saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
    virtual void generateEntries(bool input, int& cnt, QWidget *parent,
        QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table);

    KstPluginPtr _getPtr(const QString &tagin);

    QMap<QString,QString> cacheInputs(const QValueList<Plugin::Data::IOValue>& table);
    void restoreInputs(const QValueList<Plugin::Data::IOValue>& table, const QMap<QString,QString>& v);

    bool _newDialog;
    KstPluginPtr DP;
    
    // layout items
    QGridLayout* _pluginInfoGrid;
    QGridLayout* _pluginInputOutputGrid;
    
    QValueList<QWidget*> _pluginWidgets;
    
  private:
    static QGuardedPtr<KstPluginDialogI> _inst;

    /***********************************/
    /** defined in dataobjectdialog.h **/
  public slots:
    void show_Edit(const QString &field);
    void show_New();
    void OK();
    void Init();
    void close();
    void reject();
  protected:
    void _fillFieldsForEdit();
    void _fillFieldsForNew();
};

#endif
// vim: ts=2 sw=2 et
