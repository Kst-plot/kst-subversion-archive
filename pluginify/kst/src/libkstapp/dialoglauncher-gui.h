/***************************************************************************
                            dialoglauncher-gui.h
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

#ifndef DIALOGLAUNCHERGUI_H
#define DIALOGLAUNCHERGUI_H
#include "dialoglauncher.h"

class KstGuiDialogs : public KstDialogs {
  public:
    KstGuiDialogs();
    ~KstGuiDialogs();

    void showHistogramDialog(const QString& name = QString::null);

    void showPluginDialog(const QString& name = QString::null);

    void showEquationDialog(const QString& name = QString::null);

    void showCSDDialog(const QString& name = QString::null);

    void showPSDDialog(const QString& name = QString::null);

    void newMatrixDialog(QWidget *parent, const char *createdSlot = 0L, const char *selectedSlot = 0L, const char *updateSlot = 0L);
    void showMatrixDialog(const QString& name = QString::null);

    void showImageDialog(const QString& name = QString::null);

    void showCurveDialog(const QString& name = QString::null);

    void newVectorDialog(QWidget *parent, const char *createdSlot = 0L, const char *selectedSlot = 0L, const char *updateSlot = 0L);
    void showVectorDialog(const QString& name = QString::null);
};

#endif

// vim: ts=2 sw=2 et
