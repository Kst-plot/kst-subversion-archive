/***************************************************************************
                          kstdoc.cpp  -  description
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

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <unistd.h>
#include <fcntl.h>

// include files for Qt
#include <qdir.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfontdatabase.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstring.h>
#include <qlistbox.h>

// include files for KDE
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kcolordialog.h>
#include <ksavefile.h>
#include <kapplication.h>

// application specific includes
#include "kstdoc.h"
#include "kstpoint.h"
#include "kstplugin.h"
#include "kst.h"
#include "kstvcurve.h"
#include "kstpsdcurve.h"
#include "kstequationcurve.h"
#include "ksthistogram.h"
#include "kstdatacollection.h"

static bool backupFile(const QString& qFilename, const QString& backupDir = QString::null, const QString& backupExtension = "~");

KstDoc::KstDoc(QWidget *parent, const char *name)
: QObject(parent, name) {
    stopping = false;
    UpdateCounter = 0;
    _updateDelay = 200;
}

KstDoc::~KstDoc() {
    // We -have- to clear this here because static destruction order is not
    // guaranteed by any means and KstDoc::vectorList is static and has a
    // pointer to KstDoc::scalarList, also static, which gets used during
    // vector destruction.
    KST::plotList.clear();
    KST::vectorList.clear();
    KST::scalarList.clear();
}

void KstDoc::setAbsFilePath(const QString &filename) {
  absFilePath = filename;
}

const QString &KstDoc::getAbsFilePath() const {
  return absFilePath;
}

void KstDoc::setTitle(const QString &_t) {
  title = _t;
}

const QString &KstDoc::getTitle() const {
  return title;
}

bool KstDoc::saveModified() {
  bool completed = true;

  if (modified) {
    KstApp *win = static_cast<KstApp*>(parent());
    int want_save = KMessageBox::warningYesNoCancel( win, i18n("The current plot definition has been modified. Do you want to save it?"), i18n("Question"));
    switch (want_save) {
      case KMessageBox::Yes:
           if (title == i18n("Untitled")) {
             win->slotFileSaveAs();
           } else {
             saveDocument(getAbsFilePath());
       	   }

       	   deleteContents();
           completed = true;
           break;

      case KMessageBox::No:
           setModified(false);
           deleteContents();
           completed = true;
           break;

      case KMessageBox::Cancel:
           completed = false;
           break;

      default:
           completed = false;
           break;
    }
  }

  return completed;
}

void KstDoc::closeDocument() {
  deleteContents();
}

bool KstDoc::newDocument() {
  deleteContents();
  modified = false;
  absFilePath = QDir::homeDirPath();
  title = i18n("Untitled");

  return true;
}

bool KstDoc::openDocument(const KURL &url, const QString &o_file,
                          int o_n, int o_f, int o_s, bool o_ave) {
  // Temp variables
  KstFilePtr file;
  KstRVectorPtr vector;

  deleteContents();
  QFile f(url.path());
  if (!f.exists()) {
    KMessageBox::sorry(0L, i18n("%1: There is no file with that name to open.").arg(url.path()));
    return false;
  }

  title = url.fileName(false);
  absFilePath = url.path();
  if (title == 0L)
    title = absFilePath;
  QDomDocument doc(title);

  if ( !f.open( IO_ReadOnly ) ) {
    KMessageBox::sorry(0L, i18n("%1: File exists, but kst could not open it.").arg(url.path()));
    return false;
  }
  if (!doc.setContent(&f)) {
    KMessageBox::sorry(0L, i18n("%1: Not a valid kst plot specification file.").arg(url.path()));
    f.close();
    return false;
  }
  f.close();

  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "windowsize") {
        QDomNode nn = e.firstChild();
        QSize size = static_cast<KstApp*>(parent())->size();
        while (!nn.isNull()) {
          QDomElement ee = nn.toElement(); // convert child to element
          if (ee.tagName() == "width") {
            size.setWidth(ee.text().toInt());
          } else if (ee.tagName() == "height") {
            size.setHeight(ee.text().toInt());
          }
          nn = nn.nextSibling();
        }
        static_cast<KstApp*>(parent())->resize(size);
      } else if (e.tagName() == "plotcols") {
        KST::plotList.setPlotCols(e.text().toInt());
      } else if (e.tagName() == "updateDelay") {
        setDelay(e.text().toInt());
      } else if (e.tagName() == "kstfile") {
        if (o_file == "|") {
          file = new KstFile(e);
        } else {
          file = new KstFile(o_file);
        }
        if (file->numFrames() < 1) { // No data in file
          kdWarning() << I18N_NOOP("No data in file: ") << file->fileName()
                      << endl;
        }
        KST::fileList.append(file);
      } else if (e.tagName() == "vector") {
        vector = new KstRVector(e, o_file, o_n, o_f, o_s, o_ave);
        // Vectors are automatically added to the global list.
      } else if (e.tagName() == "plugin") {
        KST::dataObjectList.append(new KstPlugin(e));
      } else if (e.tagName() == "curve") {
        KST::dataObjectList.append(new KstVCurve(e));
      } else if (e.tagName() == "equation") {
        KST::dataObjectList.append(new KstEquationCurve(e));
      } else if (e.tagName() == "psd") {
        KST::dataObjectList.append(new KstPSDCurve(e));
      } else if (e.tagName() == "histogram") {
        KST::dataObjectList.append(new KstHistogram(e));
      } else if (e.tagName() == "plot") {
        KstBaseCurveList l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
        KST::plotList.append(new KstPlot(e, l));
      } else {
        kdWarning() << I18N_NOOP("unsupported element: ") << e.tagName() << endl;
      }
    }
    n = n.nextSibling();
  }

  for (KstDataObjectList::Iterator i = KST::dataObjectList.begin();
       i != KST::dataObjectList.end();
       ++i) {
    (*i)->loadInputs();
  }

  modified = false;
  return true;
}

bool KstDoc::saveDocument(const QString &filename) {
  int i;
  backupFile(filename);
  QFile f(filename);

  if (f.exists() && (filename != getAbsFilePath())) {
    if (KMessageBox::warningYesNo(0L, i18n("%1: A file of this name already exists.").arg(filename),
                                  i18n("Warning"),i18n("&Overwrite"),
                                  i18n("&Cancel")) == KMessageBox::No) {
      return false;
    }
  }

  if (!f.open(IO_WriteOnly|IO_Truncate)) {
    KMessageBox::sorry(0L, i18n("%1: Could not open file for saving. The plot description has not been saved. Try a different filename or directory.").arg(filename));
    return false;
  }

  QTextStream ts(&f);

  ts << "<kstdoc>" << endl;
  // save window geometry for this kst file
  ts << " <windowsize>" << endl;
  ts << "  <width>" << ((KstApp *)parent())->width() << "</width>" << endl;
  ts << "  <height>"<< ((KstApp *)parent())->height()<< "</height>" << endl;
  ts << " </windowsize>" << endl;

  // save plot cols
  ts << " <plotcols>" << KST::plotList.getPlotCols() << "</plotcols>" << endl;

  // save update delay
  ts << " <updateDelay>" << delay() << "</updateDelay>" << endl;

  // save files
  for (i = 0; i < (int)KST::fileList.count(); i++) {
    ts << " <kstfile>" << endl;
    KST::fileList[i]->save(ts);
    ts << " </kstfile>" << endl;
  }

  // save vectors
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (KstRVectorList::Iterator it = rvl.begin(); it != rvl.end(); ++it) {
    ts << " <vector>" << endl;
    (*it)->save(ts);
    ts << " </vector>" << endl;
  }

  // save data objects
  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    (*it)->save(ts);
  }

  // save plots
  for (i = 0; i < (int)KST::plotList.count(); i++) {
    ts << " <plot>" << endl;
    KST::plotList.at(i)->save(ts);
    ts << " </plot>" << endl;
  }

  ts << "</kstdoc>" << endl;

  f.close();

  modified = false;
  return true;
}

void KstDoc::deleteContents()
{
  /* delete the plots from plotList */
  KST::plotList.clear();

  /* delete the dataobjects */
  KST::dataObjectList.clear();

  /* delete the vectors from the vectorList */
  KST::vectorList.clear();

  /* delete the files from the filelist */
  KST::fileList.clear();
}

/*****************
    Return values:
    NO_CHANGE: Nothing changed and so no view updates are required
    UPDATE:    There was a change, and we are done now - go do a view update
******************/
/** Update Strategy **/
/* Update kstfiles, always, even if they are unused */
/* updata data vectors, even if they are unused, so stats stay correct */
/*     If reading is happening, don't go on until done (stay responsive) */
/* update all curves (curves, psd's, hs, eqs) */
/*     Each curve calls update on any vector it depends on */
/*     Slave vectors call update on masters */
/*     Masters update their slaves at the end */
/*     Multiple re-updates are stopped by looking for new update_counter */
/* update all plugins - same rule as curves */
/* We need to create a KstDataObject, which is inherited by vectors, curves */
/*  and plugins.  update, tags, save, usage so slaves can know about any */
/*  masters (inc. plugins) */
/****/
KstObject::UpdateType KstDoc::update(bool force) {
  unsigned i;
  int lastFrame=-1;
  QString message;
  KstObject::UpdateType tU, U = KstObject::NO_CHANGE;

  UpdateCounter++;
  if (UpdateCounter < 1) {
    UpdateCounter = 1; // check for wrap around
  }

  // Update the files
  for (i = 0; i < KST::fileList.count(); i++) {
    KST::fileList[i]->update();
  }

  kapp->processEvents();
  if (stopping) {
    return KstObject::NO_CHANGE;
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  // Update data vectors
  for (i = 0; i < rvl.count(); i++) {
    tU = rvl[i]->update(UpdateCounter);
    if (tU != KstObject::NO_CHANGE) {
      kapp->processEvents();
      if (stopping) {
        return KstObject::NO_CHANGE;
      }
      lastFrame = rvl[i]->numFrames() + rvl[i]->startFrame();
      if (tU == KstObject::READ) {
        U = KstObject::READ;
      } else if (U == KstObject::NO_CHANGE) {
        U = KstObject::UPDATE;
      }
    }
  }

  if (lastFrame > 0) {
    emit newFrameMsg(lastFrame);
  }

  /* update the data objects - except curves! */
  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    if (!dynamic_cast<KstBaseCurve*>((*it).data())) {
      (*it)->update(UpdateCounter);
    }
  }

  // update curves next
  if ((U == KstObject::UPDATE) || force) {
    for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
      KstBaseCurve *bc = dynamic_cast<KstBaseCurve*>((*it).data());
      if (bc) {
        bc->update(UpdateCounter);
        kapp->processEvents();
        if (stopping) {
          return KstObject::NO_CHANGE;
        }
      }
    }

    /* all updated: time to reset new and shift */
    for (i = 0; i < KST::vectorList.count(); i++) {
      KST::vectorList[i]->newSync();
    }
  }
  return U;
}

void KstDoc::samplesUp() {
  KstRVectorPtr V;
  int i;
  int f0, n, skip;
  bool doSkip, doAve;
  int fileN;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (i = 0; i < (int)rvl.count(); i++) {
    V = rvl[i];
    f0 = V->reqStartFrame();
    n = V->reqNumFrames();
    skip = V->skip();
    doSkip = V->doSkip();
    doAve =  V->doAve();
    fileN = V->getFileLength();

    if (f0 + n > fileN) {
      f0 = fileN - n;
    } else {
      f0 += n;
    }
    V->changeFrames(f0, n, skip, doSkip, doAve);
  }

  setModified();
  update();

  emit dataChanged();
}

void KstDoc::samplesDown() {
  KstRVectorPtr V;
  int i;
  int f0, n, skip;
  bool doSkip, doAve;
  int fileN;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (i = 0; i < (int)rvl.count(); i++) {
    V = rvl[i];
    f0 = V->reqStartFrame();
    if (f0 == -1)
      f0 = V->startFrame();
    n = V->reqNumFrames();
    skip = V->skip();
    doSkip = V->doSkip();
    doAve =  V->doAve();
    fileN = V->getFileLength();

    if (f0 - n < 0) {
      f0 = 0;
    } else {
      f0 -= n;
    }

    V->changeFrames(f0, n, skip, doSkip, doAve);
  }

  setModified();
  update();

  emit dataChanged();
}

void KstDoc::samplesEnd() {
  KstRVectorPtr V;
  int i;
  int f0, n, skip;
  bool doSkip, doAve;
  int fileN;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (i = 0; i < (int)rvl.count(); i++) {
    V = rvl[i];
    f0 = -1;
    n = V->numFrames();
    skip = V->skip();
    doSkip = V->doSkip();
    doAve =  V->doAve();
    fileN = V->getFileLength();

    V->changeFrames(f0, n, skip, doSkip, doAve);
  }

  setModified();
  update();

  emit dataChanged();
}

void KstDoc::wasModified() {
  modified = true;
  update();
  updateDialogs();
}

RemoveStatus KstDoc::removeDataObject(const QString &tag) {
  // FIXME: necessary?  We don't allow removal if it's in a plot...
  /* delete the curve from all plots */
  for (unsigned i_plot = 0; i_plot < KST::plotList.count(); i_plot++) {
    KstPlot *plot = KST::plotList.at(i_plot);
    for (KstBaseCurveList::iterator it = plot->Curves.begin(); it != plot->Curves.end(); ++it) {
      if ((*it)->tagName() == tag) {
        if ((*it)->slaveVectorsUsed()) {
          return SLAVE_USED;
        } else {
          KstBaseCurvePtr tmp = *it;
          --it;
          plot->Curves.remove(tmp);
        }
      }
    }
  }

  KST::dataObjectList.removeTag(tag);
  setModified();
  update();
  updateDialogs();
  return OBJECT_DELETED;
}


void KstDoc::purge() {
  bool modified = false;
  // ASSUMPTION: this only gets called from the data manager!
  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    //kdDebug() << "OBJECT: " << (*it)->tagName() << " USAGE: " << (*it)->getUsage() << endl;
    if ((*it)->getUsage() == 0) {
      //kdDebug() << "    -> REMOVED" << endl;
      KstDataObjectList::Iterator byebye = it;
      --it;
      KST::dataObjectList.remove(byebye);
      modified = true;
    }
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (KstRVectorList::Iterator it = rvl.begin(); it != rvl.end(); ++it) {
    //kdDebug() << "VECTOR: " << (*it)->tagName() << " USAGE: " << (*it)->getUsage() << endl;
    if ((*it)->getUsage() == 1) {
      //kdDebug() << "    -> REMOVED" << endl;
      KST::vectorList.remove((*it).data());
      modified = true;
    }
  }

  rvl.clear();
  setModified(modified);
  updateDialogs();
}

static int write_all(int fd, const char *buf, size_t len) {
   while (len > 0)
   {
      int written = write(fd, buf, len);
      if (written < 0)
      {
          if (errno == EINTR)
             continue;
          return -1;
      }
      buf += written;
      len -= written;
   }
   return 0;
}

static bool backupFile(const QString& qFilename, const QString& backupDir,
                       const QString& backupExtension) {
   QCString cFilename = QFile::encodeName(qFilename);
   const char *filename = cFilename.data();

   int fd = open( filename, O_RDONLY);
   if (fd < 0)
      return false;

   struct stat buff;
   if ( fstat( fd, &buff) < 0 )
   {
      ::close( fd );
      return false;
   }

   QCString cBackup;
   if ( backupDir.isEmpty() )
       cBackup = cFilename;
   else
   {
       QCString nameOnly;
       int slash = cFilename.findRev('/');
       if (slash < 0)
	   nameOnly = cFilename;
       else
	   nameOnly = cFilename.mid(slash + 1);
       cBackup = QFile::encodeName(backupDir);
       if ( backupDir[backupDir.length()-1] != '/' )
           cBackup += '/';
       cBackup += nameOnly;
   }
   cBackup += QFile::encodeName(backupExtension);
   const char *backup = cBackup.data();
   int permissions = buff.st_mode & 07777;

   if ( stat( backup, &buff) == 0)
   {
      if ( unlink( backup ) != 0 )
      {
         ::close(fd);
         return false;
      }
   }

   mode_t old_umask = umask(0);
   int fd2 = open( backup, O_WRONLY | O_CREAT | O_EXCL, permissions | S_IWUSR);
   umask(old_umask);

   if ( fd2 < 0 )
   {
      ::close(fd);
      return false;
   }

    char buffer[ 32*1024 ];

    while( 1 )
    {
       int n = ::read( fd, buffer, 32*1024 );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          ::close(fd);
          ::close(fd2);
          return false;
       }
       if (n == 0)
          break; // Finished

       if (write_all( fd2, buffer, n))
       {
          ::close(fd);
          ::close(fd2);
          return false;
       }
    }

    ::close( fd );

    if (::close(fd2))
        return false;
    return true;
}

#include "kstdoc.moc"

// vim: ts=2 sw=2 et
