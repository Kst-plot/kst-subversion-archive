/***************************************************************************
                      kstbasicdialog.h  -  Part of KST
                             -------------------
    begin                : 09/15/06
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

#ifndef KSTBASICDIALOG_H
#define KSTBASICDIALOG_H

#include "kst_export.h"
#include "kstdatadialog.h"
#include "kstbasicplugin.h"

class KST_EXPORT KstBasicDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstBasicDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0);
    virtual ~KstBasicDialog();

  public slots:
    virtual void update();
    virtual bool newObject();
    virtual bool editObject();

  protected:
    virtual void fillFieldsForEdit();
    virtual void fillFieldsForNew();

  private:
    bool editSingleObject(KstBasicPluginPtr ptr);
    static const QString& defaultTag;
    QWidget *_w;
};

#endif
// vim: ts=2 sw=2 et
