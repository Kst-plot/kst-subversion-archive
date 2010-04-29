/***************************************************************************
                       kstviewlegendwidget.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
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

#ifndef KSTVIEWLEGENDWIDGETI_H
#define KSTVIEWLEGENDWIDGETI_H

#include <QDialog>

#include "kcolorbutton.h"
#include "klineedit.h"

#include "plotlistbox.h"
#include "ui_viewlegendwidget.h"

class KstViewLegendWidget : public QDialog, public Ui::ViewLegendWidget {
  Q_OBJECT
  public:
    KstViewLegendWidget(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0 );
    virtual ~KstViewLegendWidget();

  protected:

  public:
    bool _changedFgColor;
    bool _changedBgColor;

  public slots:
    void updateButtons();
    void removeDisplayedCurve();
    void addDisplayedCurve();

    virtual void changedFgColor();
    virtual void changedBgColor();

  private slots:

  signals:
    void changed();

  private:
};

#endif
