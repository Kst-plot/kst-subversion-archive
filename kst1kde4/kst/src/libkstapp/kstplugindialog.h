/***************************************************************************
                      kstplugindialog.h  -  Part of KST
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

#include "kstdatadialog.h"
#include "kstcplugin.h"
#include "kst_export.h"
#include "ui_plugindialogwidget.h"

class KST_EXPORT KstPluginDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstPluginDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    virtual ~KstPluginDialog();
    static KstPluginDialog *globalInstance();
    static const QString& plugin_defaultTag;

  public slots:
    void updateForm();
    void update();
    bool newObject();
    bool editObject();
    virtual void showNew(const QString &field);
    virtual void updatePluginList();

  protected slots:
    void pluginChanged(int);
    void showPluginManager();
    void fixupLayout();
    void updateScalarTooltip(const QString& n);
    void updateStringTooltip(const QString& n);

  protected:
    void fillFieldsForEdit();
    void fillFieldsForNew();
    QString objectName() { return tr("Plugin"); }
    QString editTitle();
    QString newTitle();
    void fillVectorScalarCombos(QExplicitlySharedDataPointer<Plugin> pPtr);
    virtual bool saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p);
    bool saveOutputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p);
    virtual void generateEntries(bool input, int& cnt, QWidget *parent,
        QGridLayout *grid, const QList<Plugin::Data::IOValue>& table);
    QMap<QString,QString> cacheInputs(const QLinkedList<Plugin::Data::IOValue>& table);
    void restoreInputs(const QLinkedList<Plugin::Data::IOValue>& table, const QMap<QString,QString>& v);

    QStringList _pluginList;

    // layout items
    QGridLayout* _pluginInfoGrid;
    QGridLayout* _pluginInputOutputGrid;
    QLinkedList<QWidget*> _pluginWidgets;
    Ui::PluginDialogWidget *_w;

  private:
    QString _pluginName;
    static QPointer<KstPluginDialog> _inst;
};

#endif
