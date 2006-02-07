/***************************************************************************
                            filterintermediate.h
                             -------------------
    begin                : Dec 11 2003
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

#ifndef FILTERINTERMEDIATE_H
#define FILTERINTERMEDIATE_H

#include "kstscalar.h"

#include <qlistbox.h>
#include <qstring.h>

class FilterListBoxItem : public QListBoxText {
  public:
    FilterListBoxItem(QListBox *p, const QString& filter) : QListBoxText(p, filter) {}
    virtual ~FilterListBoxItem() {}

    KstScalarMap arguments;
};

#endif

// vim: ts=2 sw=2 et
