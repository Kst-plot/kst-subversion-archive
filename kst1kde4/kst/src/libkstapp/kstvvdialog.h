/***************************************************************************
                       kstvvdialog_i.h  -  Dialog for KstVectorView objects.
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by The University of British Columbia
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

#ifndef KSTVVDIALOGI_H
#define KSTVVDIALOGI_H

#include "kstdatadialog.h"
#include "kstvectorview.h"
#include "kst_export.h"
#include "ui_vectorviewdialogwidget.h"

class KST_EXPORT KstVvDialog : public KstDataDialog {
  Q_OBJECT
  public:
    KstVvDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0 );
    virtual ~KstVvDialog();
    static KstVvDialog *globalInstance();

  protected:
    QString editTitle() { return tr("Edit Vector View"); }
    QString newTitle() { return tr("New Vector View"); }

  public slots:
    void update();
    void updateWindow();
    void updatePlotList();
    bool newObject();
    bool editObject();
    void populateEditMultiple();
    void setXVector(const QString& name);
    void setYVector(const QString& name);

  private:
    static QPointer<KstVvDialog> _inst;
    Ui::VectorViewDialogWidget *_w;

    // the following are for the multiple edit mode
    bool _xVectorDirty;
    bool _yVectorDirty;
    bool _interpTypeDirty;
    bool _useXminDirty;
    bool _useXmaxDirty;
    bool _useYminDirty;
    bool _useYmaxDirty;
    bool _xMinScalarDirty;
    bool _xMaxScalarDirty;
    bool _yMinScalarDirty;
    bool _yMaxScalarDirty;
    bool _useFlagVectorDirty;
    bool _FlagVectorDirty;

    bool editSingleObject(KstVectorViewPtr vvPtr);

  private slots:
    void updateButtons();
    void setXVectorDirty() { _xVectorDirty = true; }
    void setYVectorDirty() { _yVectorDirty = true; }
    void realtimeClicked();
    void currentClicked();
    void xMinCheckboxClicked();
    void xMaxCheckboxClicked();
    void yMinCheckboxClicked();
    void yMaxCheckboxClicked();

  private:
    static const QString& defaultTag;
    void fillFieldsForEdit();
    void fillFieldsForNew();
    void cleanup();
};

#endif

