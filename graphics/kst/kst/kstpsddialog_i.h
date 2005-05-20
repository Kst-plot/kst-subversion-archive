/***************************************************************************
                       kstpsddialog_i.h  -  Part of KST
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

#ifndef KSTPSDDIALOGI_H
#define KSTPSDDIALOGI_H

#include "psddialog.h"
#include "kstpsd.h"

class KstPsdDialogI : public KstPsdDialog {
  Q_OBJECT
  public:
    KstPsdDialogI(QWidget* parent = 0, const char* name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstPsdDialogI();

  public slots:
    /** update the entries in the psd dialog to represent current psds */
    void update();
    void updateWindow();

    bool new_I();
    bool edit_I();

    static KstPsdDialogI *globalInstance();
  private:
    static QGuardedPtr<KstPsdDialogI> _inst;

    KstPSDPtr _getPtr(const QString& tagin);

    bool _newDialog;
    KstPSDPtr DP;

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
    static const QString& defaultTag;
    void _fillFieldsForEdit();
    void _fillFieldsForNew();
};

#endif
// vim: ts=2 sw=2 et
