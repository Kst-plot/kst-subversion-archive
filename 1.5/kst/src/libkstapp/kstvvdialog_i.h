/***************************************************************************
                       kstvvdialog_i.h  -  Dialog for KstVectorView objects.
                             -------------------
    begin                :
    copyright            : (C) 2007 Kst
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

class VectorViewDialogWidget;

class KST_EXPORT KstVvDialogI : public KstDataDialog {
  Q_OBJECT
  public:
    KstVvDialogI(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags fl = 0 );
    virtual ~KstVvDialogI();
    static KstVvDialogI *globalInstance();

  protected:
    QString objectName() { return tr("Vector View"); }
 
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
    static QGuardedPtr<KstVvDialogI> _inst;
    // the following are for the multiple edit mode

    bool _xVectorDirty;
    bool _yVectorDirty;
    bool _interpTypeDirty;

    bool _useXminDirty, _useXmaxDirty, _useYminDirty, _useYmaxDirty;
    bool _xMinScalarDirty, _xMaxScalarDirty, _yMinScalarDirty, _yMaxScalarDirty;

    bool _useFlagVectorDirty;
    bool _FlagVectorDirty;

    bool editSingleObject(KstVectorViewPtr vvPtr);

  private slots:
    void updateButtons();
    void setXVectorDirty() { _xVectorDirty = true; }
    void setYVectorDirty() { _yVectorDirty = true; }

    void realtimeClicked();
    void currentClicked();

  private:
    static const QString& defaultTag;
    void fillFieldsForEdit();
    void fillFieldsForNew();
    void cleanup();
    VectorViewDialogWidget *_w;
};

#endif
// vim: ts=2 sw=2 et
