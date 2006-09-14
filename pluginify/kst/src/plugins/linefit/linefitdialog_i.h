/***************************************************************************
                      kstlinefitdialog_i.h  -  Part of KST
                             -------------------
    begin                : 09/12/06
    copyright            : (C) 2006 The University of Toronto
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

#ifndef LINEFITDIALOGI_H
#define LINEFITDIALOGI_H

#include "linefitplugin.h"

#include "kstdatadialog.h"
#include "kst_export.h"

class LineFitDialogWidget;

class KST_EXPORT LineFitDialogI : public KstDataDialog {
  Q_OBJECT
  public:
    LineFitDialogI(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0);
    virtual ~LineFitDialogI();

  public slots:
//     virtual void ok();
//     virtual void close();
//     virtual void reject();
//     virtual void init();
//     virtual void show();
    virtual void update();
//     virtual void showEdit( const QString & field );
    virtual bool newObject();
    virtual bool editObject();
//     virtual bool multiple();
//     virtual void toggleEditMultiple();

  protected:
    virtual void fillFieldsForEdit();
    virtual void fillFieldsForNew();

  private:
    bool editSingleObject(LineFitPtr lf);
    static const QString& defaultTag;
    static QGuardedPtr<LineFitDialogI> _inst;
    LineFitDialogWidget *_w;
};

#endif
// vim: ts=2 sw=2 et
