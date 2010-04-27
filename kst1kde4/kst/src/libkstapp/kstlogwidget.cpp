/***************************************************************************
                              kstlogwidget.cpp
                             -------------------
    begin                : Fri Apr 01 2005
    copyright            : (C) 2005 The University of Toronto
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

#include <QBitmap>
#include <QPainter>
#include <QPolygon>

#include "kstlogwidget.h"
#include "kst.h"

KstLogWidget::KstLogWidget(QWidget *parent, const char *name )
: QTextBrowser(parent) {
  Q_UNUSED(name)

// xxx  setTextFormat(AutoText);
  _show = KstDebug::Warning | KstDebug::Error | KstDebug::Notice | KstDebug::Debug;
  generateImages();
// xxx  setMimeSourceFactory(&_msrc);
}


void KstLogWidget::setDebug(KstDebug *debug) {
  _debug = debug;
}


void KstLogWidget::logAdded(const KstDebug::LogMessage& msg) {
  QString sym;

  switch (msg.level) {
    case KstDebug::Warning:
      sym = "<img src=\"DebugWarning\"/> ";
      break;
    case KstDebug::Error:
      sym = "<img src=\"DebugError\"/> ";
      break;
    case KstDebug::Notice:
      sym = "<img src=\"DebugNotice\"/> ";
      break;
    case KstDebug::Debug:
      sym = "<img src=\"DebugDebug\"/> ";
      break;
    default:
      return;
  }

  if ((_show & int(msg.level)) == 0) {
    return;
  }

  QString format = "yyyy-mm-dd hh:mm";

  append(QObject::tr("%1<b>%2</b> %3").arg(sym).arg(msg.date.QDateTime::toString(format)).arg(msg.msg));
}


void KstLogWidget::setShowDebug(bool show) {
  int old = _show;

  if (show) {
    _show |= KstDebug::Debug;
  } else {
    _show &= ~KstDebug::Debug;
  }
  if (_show != old) {
    regenerate();
  }
}


void KstLogWidget::setShowNotice(bool show) {
  int old = _show;

  if (show) {
    _show |= KstDebug::Notice;
  } else {
    _show &= ~KstDebug::Notice;
  }
  if (_show != old) {
    regenerate();
  }
}


void KstLogWidget::setShowWarning(bool show) {
  int old = _show;

  if (show) {
    _show |= KstDebug::Warning;
  } else {
    _show &= ~KstDebug::Warning;
  }
  if (_show != old) {
    regenerate();
  }
}


void KstLogWidget::setShowError(bool show) {
  int old = _show;

  if (show) {
    _show |= KstDebug::Error;
  } else {
    _show &= ~KstDebug::Error;
  }
  if (_show != old) {
    regenerate();
  }
}


void KstLogWidget::clear() {
  QTextBrowser::clear();
  KstApp::inst()->destroyDebugNotifier();
}


void KstLogWidget::regenerate() {
  clear();
  QLinkedList<KstDebug::LogMessage> msgs = KstDebug::self()->messages();

  QLinkedList<KstDebug::LogMessage>::const_iterator it = msgs.begin();
  while (it != msgs.end()) {
    logAdded(*it);
    ++it;
  }

// xxx  scrollToBottom();
}


void KstLogWidget::generateImages() {
  QPolygon polygon;
  QPixmap pixmap;
  QPainter painter;

  int height = 14;
  int margin = 1;
  int step = (height - margin * 2) / 4;

  pixmap = QPixmap(height, height);
  pixmap.fill();
  painter.begin(&pixmap);
  painter.setBrush(QColor("LightSeaGreen"));
  painter.drawEllipse(margin, margin, height - margin * 2, height - margin * 2);
  painter.end();
  pixmap.setMask(pixmap.createHeuristicMask(true));
// xxx  _msrc.setIcon("DebugNotice", pixmap);

  pixmap = QPixmap(height, height);
  pixmap.fill();
  painter.begin(&pixmap);
  polygon.putPoints(0, 3, margin, margin,
      height - margin, margin,
      height / 2, height - margin);
  painter.setBrush(QColor("DarkOrange"));
  painter.drawPolygon(polygon);
  painter.end();
  pixmap.setMask(pixmap.createHeuristicMask(true));
// xxx  _msrc.setIcon("DebugWarning", pixmap);

  pixmap = QPixmap(height, height);
  pixmap.fill();
  painter.begin(&pixmap);
  painter.setBrush(QColor("Red"));
  polygon.putPoints(0, 8,
      margin + ( 0 * step ), margin + ( 1 * step ),
      margin + ( 0 * step ), margin + ( 3 * step ),
      margin + ( 1 * step ), margin + ( 4 * step ),
      margin + ( 3 * step ), margin + ( 4 * step ),
      margin + ( 4 * step ), margin + ( 3 * step ),
      margin + ( 4 * step ), margin + ( 1 * step ),
      margin + ( 3 * step ), margin + ( 0 * step ),
      margin + ( 1 * step ), margin + ( 0 * step ));
  painter.drawPolygon(polygon);
  painter.end();
  pixmap.setMask(pixmap.createHeuristicMask(true));
// xxx  _msrc.setIcon("DebugError", pixmap);

  pixmap = QPixmap(height, height);
  pixmap.fill();
  painter.begin(&pixmap);
  painter.setBrush(QColor("DeepSkyBlue"));
  painter.drawRoundRect(margin, margin,
      height - 2 * margin, height - 2 * margin,
      (height - 2 * margin) / 3, (height - 2 * margin) / 3);
  painter.end();
  pixmap.setMask(pixmap.createHeuristicMask(true));
// xxx  _msrc.setIcon("DebugDebug", pixmap);
}

#include "kstlogwidget.moc"
