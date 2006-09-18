/***************************************************************************
                      kstbasicdialog_i.h  -  Part of KST
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

#ifndef KSTBASICDIALOGI_H
#define KSTBASICDIALOGI_H

#include <qguardedptr.h>

#include "kst_export.h"
#include "kstdatadialog.h"
#include "kstbasicplugin.h"

class BasicDialogWidget;

class KST_EXPORT KstBasicDialogI : public KstDataDialog {
  Q_OBJECT
  public:
    KstBasicDialogI(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0);
    virtual ~KstBasicDialogI();
    static KstBasicDialogI *globalInstance();

  public slots:
    virtual void update();
    virtual bool newObject();
    virtual bool editObject();
    virtual void showNew(const QString &field);

  signals:
    void pluginChanged();

  protected:
    virtual void fillFieldsForEdit();
    virtual void fillFieldsForNew();

  private slots:
    void init();

  private:
    bool editSingleObject(KstBasicPluginPtr ptr);

    void createInputVector(const QString &name, QWidget *parent, QGridLayout *grid, int row);
    void createInputScalar(const QString &name, QWidget *parent, QGridLayout *grid, int row);
    void createInputString(const QString &name, QWidget *parent, QGridLayout *grid, int row);
    void createOutputWidget(const QString &name, QWidget *parent, QGridLayout *grid, int row);

    static const QString& defaultTag;
    static QGuardedPtr<KstBasicDialogI> _inst;

    QString _pluginName;
    BasicDialogWidget *_w;

    // layout items
    QGridLayout *_inputOutputGrid;
    QValueList<QWidget*> _widgets;
};

#endif
// vim: ts=2 sw=2 et
