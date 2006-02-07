/***************************************************************************
              kstsettings.h: a collection of settings for kst
                             -------------------
    begin                : Nov 23, 2003
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

#ifndef KSTSETTINGS_H
#define KSTSETTINGS_H

#include <qcolor.h>

class KstSettings {
  public:
    KstSettings();
    KstSettings(const KstSettings&);
    KstSettings& operator=(const KstSettings&);

    // do not delete this object
    static KstSettings *globalSettings();
    static void setGlobalSettings(const KstSettings *settings);
    void save();
    void reload();

    long psdSampleRate;
    long plotUpdateTimer;
    QColor backgroundColor;

  private:
    static KstSettings *_self;
};

#endif
// vim: ts=2 sw=2 et
