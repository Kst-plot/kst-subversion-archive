/***************************************************************************
                             kstlegenddefaults.h
                             -------------------
    begin                : 2007
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

#ifndef KSTLEGENDDEFAULTS_H
#define KSTLEGENDDEFAULTS_H

#include <qstring.h>
#include <qcolor.h>
#include "kst_export.h"

class KConfig;

class KST_EXPORT KstLegendDefaults {
  public:

    KstLegendDefaults();

    void readConfig(KConfig *config);
    void writeConfig(KConfig *config);

    const QColor& fontColor() const;
    const QColor& foregroundColor() const;
    const QColor& backgroundColor() const;
    const QString& font() const;
    int fontSize() const;
    bool vertical() const;
    bool transparent() const;
    bool trackContents() const;
    int border() const;
    int margin() const;

    void setFontColor(const QColor& color);
    void setForegroundColor(const QColor& color);
    void setBackgroundColor(const QColor& color);
    void setFont(const QString& font);
    void setFontSize(int size);
    void setVertical(bool vertical);
    void setTransparent(bool transparent);
    void setTrackContents(bool trackContents);
    void setBorder(int border);
    void setMargin(int margin);

  private:
    QColor _fontColor;
    QColor _foregroundColor;
    QColor _backgroundColor;
    QString _font;
    int _fontSize;
    bool _vertical;
    bool _transparent;
    bool _trackContents;
    int _border;
    int _margin;
};

namespace KST {
  extern KST_EXPORT KstLegendDefaults legendDefaults;
}

#endif
