/***************************************************************************
                       kstviewlabelwidget_i.h  -  Part of KST
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

#ifndef KSTVIEWLABELWIDGETI_H
#define KSTVIEWLABELWIDGETI_H

#include "viewlabelwidget.h"

class KstViewLabelWidgetI : public ViewLabelWidget {
  Q_OBJECT
  public:
    KstViewLabelWidgetI(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~KstViewLabelWidgetI();

  protected:

  public:
    bool _changedFgColor;
    bool _changedBgColor;

  public slots:
    void insertScalarInText( const QString& S );
    void insertStringInText( const QString& S );

    virtual void changedFgColor();
    virtual void changedBgColor();

  private slots:

  signals:

  private:
};

#endif
