/***************************************************************************
                             kstdefaultnames.h
                             -------------------
    begin                : July 31, 2004
    copyright            : (C) 2004 C. Barth Netterfield
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

#ifndef KSTDEFAULTNAMES_H
#define KSTDEFAULTNAMES_H

#include <qstring.h>

namespace KST {
  extern QString suggestPlotName();

  extern QString suggestVectorName(const QString &field);
  extern QString suggestCurveName(const QString& vector_name, bool add_c=false);
  extern QString suggestPSDName(const QString& vector_name);
  extern QString suggestEQName(const QString& vector_name);
  extern QString suggestHistogramName(const QString& vector_name);
  extern QString suggestPluginName(const QString& pname, const QString& vname = QString::null);
  extern QString suggestMatrixName(const QString& vector_name);
  extern QString suggestImageName(const QString& matrix_name);
}

#endif
// vim: ts=2 sw=2 et
