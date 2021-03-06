/***************************************************************************
                      kstfitdialog_i.h  -  Part of KST
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

#include "kstplugindialog_i.h"

class KstFitDialogI : public KstPluginDialogI {
  Q_OBJECT
  public:
    KstFitDialogI(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0 );
    virtual ~KstFitDialogI();
    static KstFitDialogI *globalInstance();

  public slots:
    void show_setCurve(const QString& strCurve, const QString& strPlotName, const QString& strWindow);
    bool newObject();
    void updatePluginList();

  private:
    static QGuardedPtr<KstFitDialogI> _inst;
    
    QString _xvector;
    QString _yvector;
    QString _evector;
    QString _strWindow;
    QString _strPlotName;
    QString _strCurve;
    
    bool createCurve(KstPluginPtr plugin);
    
    void generateEntries(bool input, int& cnt, QWidget *parent, QGridLayout *grid, const QValueList<Plugin::Data::IOValue>& table);
    
    bool saveInputs(KstPluginPtr plugin, KstSharedPtr<Plugin> p);
};

#endif
// vim: ts=2 sw=2 et
