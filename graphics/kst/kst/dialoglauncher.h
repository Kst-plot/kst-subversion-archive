/***************************************************************************
                              dialoglauncher.h
                             -------------------
    begin                : Nov. 24, 2004
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

#ifndef DIALOGLAUNCHER_H
#define DIALOGLAUNCHER_H
#include <qstring.h>

namespace KstDialogs {
  void showHistogramDialog(const QString& name);

  void showPluginDialog(const QString& name);

  void showEquationDialog(const QString& name);

  void showPSDDialog(const QString& name);

  void showMatrixDialog(const QString& name);

  void showImageDialog(const QString& name);

  void showCurveDialog(const QString& name);
}
#endif

// vim: ts=2 sw=2 et
