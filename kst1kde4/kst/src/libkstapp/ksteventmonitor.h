/***************************************************************************
                       ksteventmonitor.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef KSTEVENTMONITORI_H
#define KSTEVENTMONITORI_H

#include "kstdatadialog.h"
#include "ksteventmonitorentry.h"
#include "ui_eventmonitorwidget.h"

class EventMonitorWidget;

class KstEventMonitor : public KstDataDialog, public Ui::EventMonitorWidget {
  Q_OBJECT
  public:
    KstEventMonitor(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstEventMonitor();
    static KstEventMonitor *globalInstance();

  protected:
    QString editTitle() { return tr("Edit Event Monitor"); }
    QString newTitle() { return tr("New Event Monitor"); }
    bool newObject();
    bool editObject();

  public slots:
    void update();
    void enableELOG();
    void disableELOG();
    void populateEditMultiple();

  private:
    static QPointer<KstEventMonitor> _inst;

    // the following are for the multiple edit mode
    bool _changed : 1;
    bool _setWidths : 1;
    bool _lineEditEquationDirty : 1;
    bool _lineEditDescriptionDirty : 1;
    bool _checkBoxDebugDirty : 1;
    bool _radioButtonLogNoticeDirty : 1;
    bool _radioButtonLogWarningDirty : 1;
    bool _radioButtonLogErrorDirty : 1;
    bool _checkBoxEMailNotifyDirty : 1;
    bool _lineEditEMailRecipientsDirty : 1;
    bool _checkBoxELOGNotifyDirty : 1;
    bool _scriptDirty : 1;
    bool editSingleObject(EventMonitorEntryPtr emPtr);

  private slots:
    void setcheckBoxDebugDirty();
    void setcheckBoxEMailNotifyDirty();
    void setcheckBoxELOGNotifyDirty();
    void setScriptDirty();

  private:
    void fillEvent(EventMonitorEntryPtr& event);
    void fillFieldsForEdit();
    void fillFieldsForNew();

    EventMonitorWidget *_w;
    QString _vector;
};

#endif
