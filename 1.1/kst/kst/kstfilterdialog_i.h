/***************************************************************************
                      kstfilterdialog_i.h  -  Part of KST
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef KSTFILTERDIALOGI_H
#define KSTFILTERDIALOGI_H

#include "filterdialog.h"
#include "kstplugin.h"

class KstDoc;

class KstFilterDialogI : public KstFilterDialog {
  Q_OBJECT
  public:
    KstFilterDialogI(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstFilterDialogI();

    static KstFilterDialogI *globalInstance();

  public slots:
    void update();
    void show_setCurve(const QString& strCurve, const QString& strPlotName, const QString& strWindow);
    bool new_I();
    void updatePluginList();
    void OK();
    void Init();

  private slots:
    void pluginChanged(int);
    void showPluginManager();
    void fixupLayout();
    void updateScalarTooltip(const QString& n);
    void updateStringTooltip(const QString& n);

  private:
    QStringList _pluginList;
    QWidget *_frameWidget;
    QString _xvector;
    QString _yvector;
    QString _evector;
    QString _window;
    QString _plotName;
    QString _curve;
    static QGuardedPtr<KstFilterDialogI> _inst;

    bool createCurve(KstPluginPtr plugin);
    bool saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
    bool saveOutputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
    void generateEntries(const QString& fixedVector, bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table);
};

#endif
// vim: ts=2 sw=2 et
