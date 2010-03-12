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

#ifndef MATRIXSELECTOR_H
#define MATRIXSELECTOR_H
 
#include <QWidget>
#include "ui_matrixselector.h"
#include "kst_export.h"

class KST_EXPORT MatrixSelector : public QWidget, public Ui::MatrixSelector {
  Q_OBJECT
public:
  MatrixSelector(QWidget *parent = 0);
  virtual ~MatrixSelector(); 

  void allowNewMatrices( bool allowed );
  void update();
  void createNewMatrix();
  void selectMatrix();
  void editMatrix();
  void selectionWatcher( const QString & tag );
  void setSelection( const QString & tag );
  void setSelection( KstMatrixPtr s );
  QString selectedMatrix();
  void allowDirectEntry( bool allowed );

Q_SIGNALS:

private Q_SLOTS:
};
 
#endif

