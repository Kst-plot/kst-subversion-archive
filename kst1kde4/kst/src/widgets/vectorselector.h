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

#ifndef VECTORSELECTOR_H
#define VECTORSELECTOR_H
 
#include <QWidget>
#include "ui_vectorselector.h"
#include "kst_export.h"

class KST_EXPORT VectorSelector : public QWidget, public Ui::VectorSelector {
  Q_OBJECT
public:
  VectorSelector(QWidget *parent = 0);
  virtual ~VectorSelector(); 

  void allowNewVectors( bool allowed );
  void update();
  void createNewVector();
  void selectVector();
  void editVector();
  void selectionWatcher( const QString & tag );
  void setSelection( const QString & tag );
  void setSelection( KstVectorPtr v );
  QString selectedVector();
  void allowDirectEntry( bool allowed );

Q_SIGNALS:

private Q_SLOTS:
};
 
#endif

