/***************************************************************************
                          kstdoc.h  -  description
                             -------------------
    begin                : Tue Aug 22 13:46:13 CST 2000
    copyright            : (C) 2000 by Barth Netterfield
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

#ifndef KSTDOC_H
#define KSTDOC_H

#include <config.h>
#include <QApplication>
#include <QUrl>
#include "kstdebug.h"
#include "kstlistenums.h"

class KstDoc : public QObject {
  Q_OBJECT
public:
  KstDoc(QWidget *parent, const char *name=0);
  virtual ~KstDoc();

  void setModified(bool m = true) { _modified = m; }
  bool isModified() const { return _modified; }
  bool saveModified(bool doDelete = true);
  bool newDocument();
  void closeDocument();
  bool openDocument(const QUrl& url, const QString& o_file="|", int o_n = -2, int o_f = -2, int o_s = -1, bool o_ave = false);
  void saveDocument(QTextStream& ts, bool saveAbsoluteVectorPositions = false);
  bool saveDocument(const QString& filename, bool saveAbsoluteVectorPositions = false, bool prompt = true);
  const QString& absFilePath() const;
  const QString& title() const;
  const QString& lastFilePath() const;

  /** delete a curve from the curvelist and from the plots */
  RemoveStatus removeDataObject(const QString& tag);

  virtual bool event(QEvent *e);

  bool updating() const { return _updating; }

public slots:
  void deleteContents();
  void purge();
  void wasModified();
  void forceUpdate();
  void setTitle(const QString& t);
  void setAbsFilePath(const QString& filename);
  void setLastFilePath(const QString& filename);

  void samplesUp();
  void samplesDown();
  void samplesEnd();

  void cancelUpdate() { _stopping = true;}

private slots:
  void enableUpdates();

private:
  void createScalars() const;

  bool _modified;
  bool _stopping;
  bool _updating;
  bool _nextEventPaint;

  QString _title;
  QString _absFilePath;
  QString _lastFilePath;
  int _lock;

signals:
  /** if something has changed the vectors */
  void dataChanged();
  void updateDialogs();
  void logAdded(const KstDebug::LogMessage& msg);
  void logCleared();
};

#endif
