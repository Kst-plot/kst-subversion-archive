/***************************************************************************
                      kstfilterdialog.h  -  Part of KST
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

#include "kstplugindialog.h"

class KstFilterDialog : public KstPluginDialog {
  Q_OBJECT
  public:
    KstFilterDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    virtual ~KstFilterDialog();
    static KstFilterDialog *globalInstance();

  protected:
    bool newObject();

  public slots:
    void show_setCurve(const QString& strCurve, const QString& strPlotName, const QString& strWindow);
    void updatePluginList();

  private:
    bool createCurve(KstCPluginPtr plugin);
    bool saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p);
    void generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QList<Plugin::Data::IOValue>& table);

    QString _xvector;
    QString _yvector;
    QString _evector;
    QString _window;
    QString _plotName;
    QString _curve;
    static QPointer<KstFilterDialog> _inst;
};

#endif
