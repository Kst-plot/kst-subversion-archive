/***************************************************************************
                       ksteventmonitor_i.h  -  Part of KST
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

#include "eventmonitor.h"
#include "ksteventmonitorentry.h"

class KstEventMonitorI : public EventMonitor {
  Q_OBJECT
  public:
    KstEventMonitorI(QWidget* parent = 0, const char* name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstEventMonitorI();

  public slots:
    void update();
    bool new_I();
    bool edit_I();
    void enableELOG();
    void disableELOG();

    static KstEventMonitorI *globalInstance();

  signals:
    void modified();

  private:
    void fillEvent(EventMonitorEntryPtr& event);

    QString _vector;
    bool _changed;
    bool _setWidths;

    static QGuardedPtr<KstEventMonitorI> _inst;
    EventMonitorEntryPtr _getPtr(const QString &tagin);

    bool _newDialog;
    EventMonitorEntryPtr DP;

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
