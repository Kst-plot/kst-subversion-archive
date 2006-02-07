/***************************************************************************
                      kstcolorsequence.h  -  Part of KST
                             -------------------
    begin                : Mon Jul 07 2003
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

#ifndef _KST_CS_H
#define _KST_CS_H

#include <qcolor.h>
#include <kstaticdeleter.h>

class KPalette;

class KstColorSequence {
friend class KStaticDeleter<KstColorSequence>;
  public:
    static QColor next();

  private:
    KstColorSequence();
    ~KstColorSequence();
    static KstColorSequence* _self;
    int _ptr;  // pointer to the next color
    KPalette *_pal;
    int _count;
};

#endif

// vim: ts=2 sw=2 et
