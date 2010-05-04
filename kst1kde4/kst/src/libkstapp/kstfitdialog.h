/***************************************************************************
                      kstfitdialog.h  -  Part of KST
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

#ifndef KSTFITDIALOGI_H
#define KSTFITDIALOGI_H

#include "kstplugindialog.h"

class KstFitDialog : public KstPluginDialog {
  Q_OBJECT
  public:
    KstFitDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstFitDialog();
    static KstFitDialog *globalInstance();

  protected:
    bool newObject();

  public slots:
    void show_setCurve(const QString& strCurve, const QString& strPlotName, const QString& strWindow);
    void updatePluginList();

  private:
    static QPointer<KstFitDialog> _inst;

    QString _xvector;
    QString _yvector;
    QString _evector;
    QString _strWindow;
    QString _strPlotName;
    QString _strCurve;

    bool createCurve(KstCPluginPtr plugin);

    void generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QList<Plugin::Data::IOValue>& table);

    bool saveInputs(KstCPluginPtr plugin, QExplicitlySharedDataPointer<Plugin> p);
};

#endif
