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

#ifndef KSTBASICDIALOGI_H
#define KSTBASICDIALOGI_H

#include <QLinkedList>
#include <QPointer>

#include "kst_export.h"
#include "kstdatadialog.h"
#include "kstbasicplugin.h"
#include "ui_basicdialogwidget.h"

class QLineEdit;
class VectorSelector;
class ScalarSelector;
class StringSelector;

class KST_EXPORT KstBasicDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstBasicDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    virtual ~KstBasicDialog();
    static KstBasicDialog *globalInstance();

  public slots:
    void updateForm();
    virtual void update();
    virtual bool newObject();
    virtual bool editObject();
    virtual void showNew(const QString &field);
    virtual void showEdit(const QString &field);

  protected:
    virtual void fillFieldsForEdit();
    virtual void fillFieldsForNew();

    QString editTitle();
    QString newTitle();

  private slots:
    void init();

  private:
    bool editSingleObject(KstBasicPluginPtr ptr, QString &error);

    void createInputVector(const QString &name, int row);
    void createInputScalar(const QString &name, int row, double value);
    void createInputString(const QString &name, int row);
    void createOutputWidget(const QString &name, int row);

    VectorSelector *vector(const QString &name) const;
    ScalarSelector *scalar(const QString &name) const;
    StringSelector *string(const QString &name) const;
    QLabel *label(const QString &name) const;
    QLineEdit *output(const QString &name) const;

    static const QString& defaultTag;
    static QPointer<KstBasicDialog> _inst;

    QString _pluginName;
    Ui::BasicDialogWidget *_w;

    // layout items
    QGridLayout *_grid;
    QLinkedList<QWidget*> _widgets;
};

#endif
