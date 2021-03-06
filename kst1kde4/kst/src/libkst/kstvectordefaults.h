/***************************************************************************
                             kstvectordefaults.h
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef KSTVECTORDEFAULTS_H
#define KSTVECTORDEFAULTS_H

#include <QString>
#include <QSettings>

#include "kst_export.h"

class KST_EXPORT KstVectorDefaults {
  public:
    KstVectorDefaults();

    void sync();
    const QString& dataSource() const;
    const QString& wizardXVector() const;
    void setWizardXVector(const QString& vector);
    double f0() const;
    double n() const;
    bool countFromEOF() const;
    bool readToEOF() const;
    bool doSkip() const;
    bool doAve() const;
    int skip() const;

    void readConfig(QSettings *config);
    void writeConfig(QSettings *config);

  private:
    QString _dataSource;
    QString _wizardX;
    double _f0;
    double _n;
    bool _doSkip;
    bool _doAve;
    int _skip;
};

namespace KST {
  extern KST_EXPORT KstVectorDefaults vectorDefaults;
}

#endif
