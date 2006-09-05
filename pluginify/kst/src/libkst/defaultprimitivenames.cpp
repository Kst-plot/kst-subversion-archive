/***************************************************************************
                          defaultprimitivenames.cpp
                             -------------------
    begin                : July 31, 2004
    copyright            : (C) 2003 C. Barth Netterfield
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

#include "kstdatacollection.h"
#include "defaultprimitivenames.h"

#include <qregexp.h>
#include <klocale.h>
#include <stdio.h>

// takes a field name and returns a unique tag name, which will be
// the field if that is unique, or field-N if there are already N
// vectors of that name
QString KST::suggestVectorName(const QString& field) {
  int i = 0;
  QString name(field);

  while (KstData::self()->vectorTagNameNotUnique(name, false)) {
    name = QString("%1-%2").arg(field).arg(++i);
  }

  return name;
}


QString KST::suggestMatrixName(const QString& vectorName) {
  int i = 1;
  QString name(vectorName);

  while (KST::matrixList.findTag(name) != KST::matrixList.end()) {
    name = QString("%1-%2").arg(vectorName).arg(++i);
  }

  return name;
}

// vim: ts=2 sw=2 et
