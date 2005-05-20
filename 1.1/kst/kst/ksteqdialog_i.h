/***************************************************************************
                       ksteqdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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

#ifndef KSTEQDIALOGI_H
#define KSTEQDIALOGI_H

#include "eqdialog.h"
#include "kstequation.h"

class KstEqDialogI : public KstEqDialog {
  Q_OBJECT
  public:
    KstEqDialogI(QWidget* parent = 0, const char* name = 0,
        bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstEqDialogI();

  public slots:
    void update();
    void updateWindow();

    bool new_I();
    bool edit_I();

    static KstEqDialogI *globalInstance();

  private:
    static QGuardedPtr<KstEqDialogI> _inst;
    KstEquationPtr _getPtr(const QString &tagin);
    bool _checkEntries();
    bool _newDialog;
    KstEquationPtr DP;

    /***********************************/
    /** defined in dataobjectdialog.h **/
  public slots:
    void show_Edit(const QString &field);
    void show_New();
    void OK();
    void Init();
    void close();
    void reject();
    void populateFunctionList();

  private:
    static const QString& defaultTag;
    void _fillFieldsForEdit();
    void _fillFieldsForNew();
};

#endif
// vim: ts=2 sw=2 et
