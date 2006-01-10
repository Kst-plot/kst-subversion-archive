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
#include "kst_export.h"

class QWidget;

namespace KstDialogs {
  KST_EXPORT void showHistogramDialog(const QString& name);

  KST_EXPORT void showPluginDialog(const QString& name);

  KST_EXPORT void showEquationDialog(const QString& name);
  
  KST_EXPORT void showCSDDialog(const QString& name);

  KST_EXPORT void showPSDDialog(const QString& name);

  KST_EXPORT void newMatrixDialog(QWidget *parent, const char *createdSlot = 0L, const char *selectedSlot = 0L, const char *updateSlot = 0L);
  KST_EXPORT void showMatrixDialog(const QString& name);

  KST_EXPORT void showImageDialog(const QString& name);

  KST_EXPORT void showCurveDialog(const QString& name);
  
  KST_EXPORT void newVectorDialog(QWidget *parent, const char *createdSlot = 0L, const char *selectedSlot = 0L, const char *updateSlot = 0L);
  KST_EXPORT void showVectorDialog(const QString& name);
}
#endif

// vim: ts=2 sw=2 et
