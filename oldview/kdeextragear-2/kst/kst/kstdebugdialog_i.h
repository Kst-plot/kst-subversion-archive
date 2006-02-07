/***************************************************************************
                       kstdebugdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 Andrew Walker
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
#ifndef KSTDEBUGI_H
#define KSTDEBUGI_H

#include "kstdebug.h"
#include "debugdialog.h"
#include "kstdoc.h"
#include <kcombobox.h>

#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qtextbrowser.h>
#include <qtable.h>

#include "kstbasecurve.h"
#include "kstlogtable.h"

class KstDebugDialogI : public DebugDialog {
    Q_OBJECT
public:
    KstDebugDialogI(QWidget* parent = 0, const char* name = 0,
                   bool modal = FALSE, WFlags fl = 0);
    virtual ~KstDebugDialogI();

public slots:
  void messagesDebug();
  void messagesWarning();
  void messagesNotice();
  void messagesOther();
  void messagesError();
  void email();
  void clear();
  void show_I();
  void logAdded();
  
signals:

private:  
  KstLogTable* _table;
  bool _bFirstNotification ;
};

#endif
