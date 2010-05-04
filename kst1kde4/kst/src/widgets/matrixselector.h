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

  void    allowNewMatrices( bool allowed );
  QString selectedMatrix();
  void    update();
  void    createNewMatrix();
  void    selectionWatcher( const QString & tag );
  void    setSelection( const QString & tag );
  void    newMatrixCreated( KstMatrixPtr v );
  void    setSelection( KstMatrixPtr v );
  void    provideNoneMatrix( bool provide );
  void    editMatrix();
  void    setEdit(const QString& tag);

Q_SIGNALS:
  void    selectionChangedLabel(const QString&);
  void    newMatrixCreated(const QString&);

private Q_SLOTS:

private:
  bool _provideNoneMatrix;
};
 
#endif

