/***************************************************************************
                      kstvectordialog_i.h  -  Part of KST
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
#ifndef KSTVECTORDIALOGI_H
#define KSTVECTORDIALOGI_H

#include "kstrvector.h"
#include "kstsvector.h"
#include "vectordialog.h"

class KCompletion;

class KstVectorDialogI : public KstVectorDialog {
  Q_OBJECT
  public:
    KstVectorDialogI(QWidget* parent = 0, const char* name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstVectorDialogI();

  public slots:
    /** update no longer does anything */
    void update();

    bool new_I();
    bool edit_I();

    static KstVectorDialogI *globalInstance();

    void updateCompletion();

  signals:
    void modified();
    void vectorCreated(KstVectorPtr v);

  private slots:
    void configureSource();
    void enableSource();
    void enableGenerate();
    void markSourceAndSave();
    void testURL();

  private:
    static QGuardedPtr<KstVectorDialogI> _inst;
    QGuardedPtr<KCompletion> _fieldCompletion;
    KstVectorPtr _getPtr(const QString &tagin);
    QGuardedPtr<KstDataSourceConfigWidget> _configWidget;

    KstVectorPtr DP;
    bool _newDialog;
    bool _inTest;

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
    void _fillFieldsForSVEdit();
    void _fillFieldsForRVEdit();
    void _fillFieldsForNew();
    static const QString& defaultTag;
};

#endif
// vim: ts=2 sw=2 et
