/***************************************************************************
                       kstmatrixdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef KSTMATRIXDIALOGI_H
#define KSTMATRIXDIALOGI_H

#include "matrixdialog.h"
#include "kstmatrix.h"

class KstMatrixDialogI : public KstMatrixDialog {
  Q_OBJECT
  public:
    KstMatrixDialogI(QWidget* parent = 0, const char* name = 0,
        bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstMatrixDialogI();

    static KstMatrixDialogI *globalInstance();

  public slots:
    void update();
    void updateWindow();

    bool new_I();
    bool edit_I();

  signals:
    void modified();
    void matrixCreated(KstMatrixPtr v);

  private:
    static QGuardedPtr<KstMatrixDialogI> _inst;
    KstMatrixPtr _getPtr(const QString &tagin);

    bool _newDialog;
    KstMatrixPtr DP;
    bool checkParameters(KstVectorList::Iterator &vector_iter, double &minXDouble, double &minYDouble, double &xStepDouble, double &yStepDouble);

  private slots:
    //enable or disable fields as necessary
    void updateFields();

  /***********************************/
  /** defined in dataobjectdialog.h **/
  public slots:
    void show_Edit(const QString &field);
    void show_New();
    void OK();
    void Init();
    void close();
    void reject();
  private:
    void _fillFieldsForEdit();
    void _fillFieldsForNew();
};

#endif
// vim: ts=2 sw=2 et
