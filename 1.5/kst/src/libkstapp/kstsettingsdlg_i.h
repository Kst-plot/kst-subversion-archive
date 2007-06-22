/***************************************************************************
                       kstsettingsdlg_i.h  -  Part of KST
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

#include "kstsettingsdlg.h"

class KstSettingsDlgI: public KstSettingsDlg {
  Q_OBJECT
  public:
    KstSettingsDlgI(QWidget* parent = 0, const char *name = 0, WFlags fl = 0);
    virtual ~KstSettingsDlgI();

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
