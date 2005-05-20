/***************************************************************************
                       kstcurvedialog_i.h  -  Part of KST
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

#ifndef KSTIMAGEDIALOGI_H
#define KSTIMAGEDIALOGI_H

#include "imagedialog.h"
#include "kstimage.h"

class KstImageDialogI : public KstImageDialog {
  Q_OBJECT
  public:
    KstImageDialogI(QWidget *parent = 0, const char *name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstImageDialogI();

  public slots:
    void update();
    void updateWindow();

    bool new_I();
    bool edit_I();

    static KstImageDialogI *globalInstance();

  private slots:
    void calcAutoThreshold();
    void updateGroups();
    void updateEnables();

  signals:
    void modified();

  private:
    static QGuardedPtr<KstImageDialogI> _inst;
    KstImagePtr _getPtr(const QString& tagin);

    bool _newDialog;
    KstImagePtr DP;
    void placeInPlot(KstImagePtr image);
    bool checkParameters(double& lowerZDouble, double& upperZDouble);

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
