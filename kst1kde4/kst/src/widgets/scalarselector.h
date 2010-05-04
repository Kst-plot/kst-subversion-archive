/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SCALARSELECTOR_H
#define SCALARSELECTOR_H
 
#include <QWidget>
#include <kstscalar.h>
#include "ui_scalarselector.h"
#include "kst_export.h"

class KST_EXPORT ScalarSelector : public QWidget, public Ui::ScalarSelector {
   Q_OBJECT
public:
  ScalarSelector(QWidget *parent = 0);
  virtual ~ScalarSelector(); 

  void allowNewScalars( bool allowed );
  void update();  
  void selectionWatcher( const QString & tag );
  void setSelection( const QString & tag );
  void setSelection( KstScalarPtr s );
  QString selectedScalar();
  KstScalarPtr selectedScalarPtr();
  void allowDirectEntry( bool allowed );

Q_SIGNALS:
  void selectionChangedLabel(const QString&);
  void selectionChanged(const QString&);
  void newScalarCreated();

private Q_SLOTS:
  void createNewScalar();
  void editScalar();
  void selectScalar();
};
 
#endif





