/***************************************************************************
                       kstsettingsdlg.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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
#ifndef KSTSETTINGSDLGI_H
#define KSTSETTINGSDLGI_H

#include "ui_kstsettingsdlg.h"

class KstSettingsDlg: public QDialog, public Ui::KstSettingsDlg {
  Q_OBJECT
  public:
    KstSettingsDlg(QWidget* parent = 0, const char *name = 0, Qt::WindowFlags fl = 0);
    virtual ~KstSettingsDlg();

  signals:
    void settingsChanged();

  public slots:
    void setSettings( const KstSettings *settings );
    void updateAxesButtons();
    void updateAxesSettings();
    void updateEMailSettings();
    void updateCurveColorSettings();
    void fillAxesSettings();
    void configureSource();

  private slots:
    void defaults();
    void setDirty();
    void setClean();
    void save();
    void updateUTCOffset();
    void setUTCOffset( const QString &timezone );
    void updateTimezone( const QString &strHours );
    void updateTimezone( double hours );
    void sourceChanged( const QString &name );

  private:
    int utcOffset( const QString &timezone );
    QString timezoneFromUTCOffset( double hours );

    bool _dirty;
};

#endif
