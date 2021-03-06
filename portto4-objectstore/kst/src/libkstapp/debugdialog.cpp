/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debugdialog.h"
#include <debug.h>
#include <events.h>
#include <logevents.h>

namespace Kst {

DebugDialog::DebugDialog(QWidget *parent)
  : QDialog(parent) {
  setupUi(this);
}


DebugDialog::~DebugDialog() {
}


bool DebugDialog::event(QEvent* e) {
  if (e->type() == EventTypeLog) {
    LogEvent *le = dynamic_cast<LogEvent*>(e);
    if (le) {
      switch (le->_eventType) {
        case LogEvent::LogAdded:
          _log->append(le->_msg.msg);
          if (le->_msg.level == Debug::Error) {
            emit notifyOfError();
          }
          break;
        case LogEvent::LogCleared:
            _log->clear();
            emit notifyAllClear();
          break;
        default:
          break;
      }
    }
    return true;
  }
  return false;
}

}

// vim: ts=2 sw=2 et
