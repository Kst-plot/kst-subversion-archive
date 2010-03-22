/***************************************************************************
                       ksteditviewobjectdialog.h  -  Part of KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 The University of British Columbia
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

#ifndef KSTEDITVIEWOBJECTDIALOGI_H
#define KSTEDITVIEWOBJECTDIALOGI_H

#include <QPointer>

#include "editmultiplewidget.h"
#include "ui_editviewobjectdialog.h"
#include "kstviewobject.h"
#include "kstviewwindow.h"
#include "ksttoplevelview.h"
#include "kst_export.h"

class QComboBox;
class QGridLayout;

class KST_EXPORT KstEditViewObjectDialog : public QDialog, public Ui::KstEditViewObjectDialog {
  Q_OBJECT
  public:
    KstEditViewObjectDialog(QWidget* parent = 0,
                             const char* name = 0,
                             bool modal = false, WFlags fl = 0 );
    virtual ~KstEditViewObjectDialog();

  public slots:
    void updateEditViewObjectDialog();
    void showEditViewObjectDialog(KstViewObjectPtr viewObject, KstTopLevelViewPtr top);
    void setNew();
    void toggleEditMultiple();
    void setDefaults();
    void restoreDefaults();

  protected:
   virtual void resizeEvent(QResizeEvent *e);

  private:
    void updateDefaultWidgets();
    void updateWidgets();
    void clearWidgets();
    void fillObjectList();
    void populateEditMultiple();
    void applySettings(KstViewObjectPtr viewObject);
    bool apply();

    void fillPenStyleWidget(QComboBox* widget);
    void fillHJustifyWidget(QComboBox* widget);
    void fillVJustifyWidget(QComboBox* widget);

    KstViewObjectPtr _viewObject; // the view object we are currently editing
    KstTopLevelViewPtr _top; // the top level view that invoked this dialog

    // for layout purposes
    QValueList<QWidget*> _inputWidgets; // the widgets used to change properties
    QValueList<QWidget*> _widgets; // all other widgets
    QGridLayout* _grid;
    QPointer<QWidget> _customWidget;
    bool _isNew;
    bool _editMultipleMode;

  private slots:
    void applyClicked();
    void okClicked();
    void modified();
};

#endif
