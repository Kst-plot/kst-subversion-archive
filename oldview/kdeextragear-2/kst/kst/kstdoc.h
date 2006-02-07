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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qptrlist.h>
#include <kurl.h>
#include <kapp.h>

#include "kstplotlist.h"
#include "kstobject.h"
#include "kstlistenums.h"

class KstDoc : public QObject {
  Q_OBJECT
public:
  KstDoc(QWidget *parent, const char *name=0);
  virtual ~KstDoc();

  /** sets the modified flag for the document after a modifying action
      on the view connected to the document.*/
  void setModified(bool _m=true){ modified=_m; };
  /** returns if the document is modified or not. Use this to determine
      if your document needs saving by the user on closing.*/
  bool isModified(){ return modified; };
  /** asks the user for saving if the document is modified */
  bool saveModified();
  /** deletes the document's contents */
  void deleteContents();
  /** initializes the document generally */
  bool newDocument();
  /** closes the acutal document */
  void closeDocument();
  /** loads the document by filename */
  bool openDocument(const KURL &url, const QString &o_file="|",
		    int o_n = -2, int o_f = -2,
		    int o_s = -1, bool o_ave = false);
  /** saves the document to a QTextStream */
  void saveDocument( QTextStream &ts );
  /** saves the document under filename */
  bool saveDocument(const QString &filename);
  /** returns the pathname of the current document file*/
  const QString &getAbsFilePath() const;
  /** returns the title of the document */
  const QString &getTitle() const;

  /** delete a curve from the curvelist and from the plots */
  RemoveStatus removeDataObject(const QString &tag);

  /** the update delay */
  int delay() const {return (_updateDelay);}

  virtual bool event(QEvent *e);

public slots:
  void purge();
  void wasModified();
  void forceUpdate();
  /** sets the filename of the document */
  void setTitle(const QString &_t);
  /** sets the path to the file connected with the document */
  void setAbsFilePath(const QString &filename);
  /** emit signal to update the dialogs */
  void updateDialogs() { emit dataChanged();}
  /** Increase the starting frame for all vectors by n_frames of the vector */
  void samplesUp();

  /** Decrease the starting frame for all vectors by n_frames of the vector */
  void samplesDown();

  /** set all vectors to read, counting back from the end of the file */
  void samplesEnd();

  /** cancel update */
  void cancelUpdate() { stopping = true;}

  void setDelay(int in_delay) {_updateDelay = in_delay;}
  
private:
   void createScalars() const;
  
  /** the modified flag of the current document */
  bool modified;
  bool stopping;

  int _updateDelay;

  QString title;
  QString absFilePath;
  int _lock;

signals:
  /** if something has changed the vectors */
  void dataChanged();

  /** signals a hlp message */
  void newFrameMsg(int);
};

#endif // KSTDOC_H
