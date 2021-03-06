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

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

// include files for Qt
#include <qdeepcopy.h>
#include <qeventloop.h>
#include <qstylesheet.h>

// include files for KDE
#include <dcopclient.h>
#include "ksdebug.h"
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kmdimainfrm.h>
#include <kprogress.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

// application specific includes
#include "kst2dplot.h"
#include "kstcsd.h"
#include "kstdataobjectcollection.h"
#include "kstdoc.h"
#include "kstgraphfiledialog_i.h"
#include "kstequation.h"
#include "ksteventmonitorentry.h"
#include "ksthistogram.h"
#include "kstimage.h"
#include "kstmatrixdefaults.h"
#include "kstplugin.h"
#include "kstpsd.h"
#include "kstrvector.h"
#include "kstvcurve.h"
#include "kstvectordefaults.h"
#include "kstsvector.h"
#include "kstavector.h"
#include "kstrmatrix.h"
#include "kstamatrix.h"
#include "kstsmatrix.h"
#include "kstviewwindow.h"
#include "logevents.h"
#include "threadevents.h"

static bool backupFile(const QString& qFilename, const QString& backupDir = QString::null, const QString& backupExtension = "~");

KstDoc::KstDoc(QWidget *parent, const char *name)
: QObject(parent, name) {
  _lock = 0;
  _updating = false;
  _stopping = false;
  _nextEventPaint = false;
  _title = "Untitled";
}

KstDoc::~KstDoc() {
  // We -have- to clear this here because static destruction order is not
  // guaranteed by any means and KstDoc::vectorList is static and has a
  // pointer to KstDoc::scalarList, also static, which gets used during
  // vector destruction.
  deleteContents();
}

void KstDoc::setAbsFilePath(const QString& filename) {
  _absFilePath = filename;
}

const QString& KstDoc::absFilePath() const {
  return _absFilePath;
}

void KstDoc::setLastFilePath(const QString& filename) {
  _lastFilePath = filename;
}

const QString& KstDoc::lastFilePath() const {
  return _lastFilePath;
}

void KstDoc::setTitle(const QString& t) {
  _title = t;
}

const QString& KstDoc::title() const {
  return _title;
}

bool KstDoc::saveModified(bool doDelete) {
  bool completed = true;

  if (_modified) {
    KstApp *win = KstApp::inst();
    if (win->activeWindow()) {
      int want_save = KMessageBox::warningYesNoCancel( win, i18n("The current plot definition has been modified. Do you want to save it?"), i18n("Question"));
      switch (want_save) {
        case KMessageBox::Yes:
          if (_title == "Untitled") {
            if (!win->slotFileSaveAs()) {
              return false;
            }
          } else {
            saveDocument(absFilePath());
          }

          if (doDelete) {
            deleteContents();
          }
          completed = true;
          break;

        case KMessageBox::No:
          if (doDelete) {
            setModified(false);
            deleteContents();
          }
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
  }

  return completed;
}

void KstDoc::closeDocument() {
  deleteContents();
}

bool KstDoc::newDocument() {
  // FIXME: implement this properly
  //if (KMessageBox::Yes == KMessageBox::warningYesNo(KstApp::inst(), i18n("Are you sure you wish to erase all your data and plots?"))) {
    deleteContents();
    _modified = false;
    _absFilePath = QDir::homeDirPath();
    _title = "Untitled";
    KstApp::inst()->newWindow(i18n("default name of first window", "W1"));
    createScalars();
    emit updateDialogs();
  //}

  return true;
}

bool KstDoc::openDocument(const KURL& url, const QString& o_file,
                          int o_n, int o_f, int o_s, bool o_ave) {
  static bool opening = false;
  bool cleanupFile = false;

  if (opening) {
    return false;
  }

  QString tmpFile;
  if (url.isLocalFile() || url.protocol().isEmpty()) {
    tmpFile = url.path();
  } else {
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
    if (!KIO::NetAccess::exists(url, true, KstApp::inst())) {
#else
    if (!KIO::NetAccess::exists(url, true)) {
#endif
      KMessageBox::sorry(KstApp::inst(), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));
      return false;
    }

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
    if (!KIO::NetAccess::download(url, tmpFile, KstApp::inst())) {
#else
    if (!KIO::NetAccess::download(url, tmpFile)) {
#endif
      KMessageBox::sorry(KstApp::inst(), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));
      return false;
    }
    cleanupFile = true;
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  opening = true;

  deleteContents();
  KstApp::inst()->setPaused(true);
  _updating = true; // block update thread from sending events until we're done

  QFile f(tmpFile);
  if (!f.exists()) {
    KMessageBox::sorry(KstApp::inst(), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    return false;
  }

  _title = url.fileName(false);
  _absFilePath = url.url();
  _lastFilePath = url.url();
  if (_title.isEmpty()) {
    _title = _absFilePath;
  }
  QDomDocument doc(_title);

  if (!f.open(IO_ReadOnly)) {
    KMessageBox::sorry(KstApp::inst(), i18n("%1: File exists, but kst could not open it.").arg(url.prettyURL()));
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
      KIO::NetAccess::removeTempFile(tmpFile);
    }
    return false;
  }

  if (!doc.setContent(&f)) {
    KMessageBox::sorry(KstApp::inst(), i18n("%1: Not a valid kst plot specification file.").arg(url.prettyURL()));
    f.close();
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
      KIO::NetAccess::removeTempFile(tmpFile);
    }
    return false;
  }

  f.close();

  QDomElement docElem = doc.documentElement();
  int columns = -1;
  int count;
  int handled = 0;
  bool warnOldKstFile = false;
  QString activeWindow;
  KstRVectorPtr vector;
  KstApp *app = KstApp::inst();
  QString readingDocument = i18n("Reading Kst file");

  if (docElem.tagName() != "kstdoc") {
    QString err = i18n("Error opening file %1.  Does not appear to be a Kst file.").arg(url.prettyURL());
    KstDebug::self()->log(err, KstDebug::Error);
    KMessageBox::sorry(KstApp::inst(), err);
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
      KIO::NetAccess::removeTempFile(tmpFile);
    }
    return false;
  }

  if (docElem.attribute("version") != "1.2" && !docElem.attribute("version").isEmpty()) {
    QString err = i18n("Error opening file %2.  Version %1 is too new.  Update Kst or fix the Kst file.").arg(docElem.attribute("version")).arg(url.prettyURL());
    KstDebug::self()->log(err, KstDebug::Error);
    KMessageBox::sorry(KstApp::inst(), err);
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
      KIO::NetAccess::removeTempFile(tmpFile);
    }
    return false;
  }

  QDomNode n = docElem.firstChild();
  count = docElem.childNodes().length();
  app->slotUpdateProgress(count, handled, readingDocument);

  kapp->dcopClient()->setAcceptCalls(false);
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull()) { // the node was really an element.
      if (e.tagName() == "windowsize") {
        QDomNode nn = e.firstChild();
        QSize size = app->size();
        while (!nn.isNull()) {
          QDomElement ee = nn.toElement(); // convert child to element
          if (ee.tagName() == "width") {
            size.setWidth(ee.text().toInt());
          } else if (ee.tagName() == "height") {
            size.setHeight(ee.text().toInt());
          } else if (ee.tagName() == "active") {
            activeWindow = ee.attribute("name");
          }
          nn = nn.nextSibling();
        }
        app->resize(size);
      } else if (e.tagName() == "graphicsautosave") {
        KstGraphFileDialogI *gdlg = app->graphFileDlg();
        int time = e.attribute("time").toInt();
        bool en = e.attribute("enabled") == "true";
        QString fmt = e.attribute("format");
        QString url = e.attribute("url");
        bool sq = e.attribute("square") == "true";
        int sizeX = e.attribute("xsize").toInt();
        int sizeY = e.attribute("ysize").toInt();
        int display = e.attribute("display").toInt();

        if (sizeX > 0 && sizeY > 0) {
          gdlg->setImageSize(sizeX, sizeY);
        }
        if (!url.isEmpty()) {
          gdlg->setURL(url);
        }
        if (!fmt.isEmpty()) {
          gdlg->setFormat(fmt);
        }
        if (time >= 0) {
          gdlg->setAutoSaveTimer(time);
        }
        if (sq) {
          gdlg->setDisplay(1);
        } else {
          gdlg->setDisplay(display);
        }
        gdlg->setAutoSave(en);
        gdlg->updateDialog();
        gdlg->applyAutosave();
      } else if (e.tagName() == "plotcols") {
        columns = e.text().toInt();
      } else if (e.tagName() == "kstfile") {
        KstWriteLocker dswl(&KST::dataSourceList.lock());
        KstDataSourcePtr file;
        if (o_file == "|") {
          file = KstDataSource::loadSource(e);
        } else {
          file = KstDataSource::loadSource(o_file);
        }
        if (file && (!file->isValid() || file->isEmpty())) {
          KstDebug::self()->log(i18n("No data in file %1.").arg(file->fileName()), KstDebug::Warning);
        }
        if (file) {
          KST::dataSourceList.append(file);
        }
      } else if (e.tagName() == "scalar") {
        new KstScalar(e);
      } else if (e.tagName() == "string") {
        new KstString(e);
      } else if (e.tagName() == "vector") {
        vector = new KstRVector(e, o_file, o_n, o_f, o_s, o_ave);
        KST::addVectorToList(KstVectorPtr(vector));
        // Vectors are automatically added to the global list.
      } else if (e.tagName() == "svector") {
        KstSVectorPtr svector = new KstSVector(e);
        KST::addVectorToList(KstVectorPtr(svector));
      } else if (e.tagName() == "avector") {
        KstAVectorPtr avector = new KstAVector(e);
        avector = new KstAVector(e);
        KST::addVectorToList(KstVectorPtr(avector));
      } else if (e.tagName() == "plugin") {
        KstDataObjectPtr p = new KstPlugin(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "curve") {
        KstDataObjectPtr p = new KstVCurve(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "equation") {
        KstEquationPtr p = new KstEquation(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p.data());
        warnOldKstFile = true;
      } else if (e.tagName() == "psd") {
        KstPSDPtr p = new KstPSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p.data());
        warnOldKstFile = true;
      } else if (e.tagName() == "equationobject") {
        KstDataObjectPtr p = new KstEquation(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "psdobject") {
        KstDataObjectPtr p = new KstPSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "csdobject") {
        KstDataObjectPtr p = new KstCSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "histogram") {
        KstDataObjectPtr p = new KstHistogram(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "event") {
        KstDataObjectPtr p = new EventMonitorEntry(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "plot") {
        KstBaseCurveList l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
        Kst2DPlotPtr plot = new Kst2DPlot(e);
        app->plotHolderWhileOpeningDocument()->insert(plot->tagName(), plot);
      } else if (e.tagName() == "amatrix") {
        KstAMatrixPtr p = new KstAMatrix(e);
        KST::addMatrixToList(KstMatrixPtr(p));
      } else if (e.tagName() == "smatrix") {
        KstSMatrixPtr p = new KstSMatrix(e);
        KST::addMatrixToList(KstMatrixPtr(p));
      } else if (e.tagName() == "rmatrix") {
        KstRMatrixPtr p = new KstRMatrix(e);
        KST::addMatrixToList(KstMatrixPtr(p));
      } else if (e.tagName() == "image") {
        KstDataObjectPtr p = new KstImage(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "window") {
        KstViewWindow *p = dynamic_cast<KstViewWindow*>(app->activeWindow());
        KstViewWindow *toDelete = 0L;

        if (!p) {
          KMdiIterator<KMdiChildView*> *it = app->createIterator();
          p = dynamic_cast<KstViewWindow*>(it->currentItem());
          app->deleteIterator(it);
        }

        if (p && p->view()->children().count() == 0) {
          toDelete = p;
        }

        p = new KstViewWindow(e);
        app->addWindow(p, KMdi::StandardAdd | KMdi::UseKMdiSizeHint);
        p->activate();
        if (toDelete) {
          app->closeWindow(toDelete);
        }
      } else {
        KstDebug::self()->log(i18n("Unsupported element '%1' in file %2.").arg(e.tagName()).arg(url.prettyURL()), KstDebug::Warning);
      }
    }
    handled++;
    app->slotUpdateProgress(count, handled, readingDocument);
    n = n.nextSibling();
    kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
  }

  if (warnOldKstFile) {
    QApplication::restoreOverrideCursor();
    KMessageBox::sorry(KstApp::inst(), i18n("You tried to load an old Kst file.  Curves created by equations or PSDs will not be loaded."));
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  app->slotUpdateProgress(0, 0, i18n("Creating plots"));
  // if we have anything left in plotHolderWhileOpeningDocument then
  //  we are most likely reading an old style document, so we create
  //  a default view and fill it with whatever is left...
  if (!app->plotHolderWhileOpeningDocument()->isEmpty() && !app->activeWindow()) {

    QString winName = app->newWindow(QString::null);
    KstViewWindow *win = dynamic_cast<KstViewWindow*>(app->findWindow(winName));

    if (win) {
      if (columns != -1) {
        win->view()->setOnGrid(true);
        win->view()->setColumns(columns);
      }

      for (Kst2DPlotMap::Iterator it = app->plotHolderWhileOpeningDocument()->begin();
          it != app->plotHolderWhileOpeningDocument()->end(); ++it ) {
        Kst2DPlotPtr plot = *it;

        // Legacy 1.0 loading code
        if (plot->_width > 0.0 && plot->_height > 0.0 ) {
          win->view()->appendChild(plot.data(), true);
          if (plot->_width > 1.0 && plot->_height > 1.0) {
            plot->resizeFromAspect(double(plot->_pos_x) / double(plot->_width),
                double(plot->_pos_y) / double(plot->_height),
                1.0 / double(plot->_width),
                1.0 / double(plot->_height));
          } else {
            plot->resizeFromAspect(plot->_pos_x, plot->_pos_y,
                plot->_width, plot->_height);
          }
        }
      }
    }
  }

  app->plotHolderWhileOpeningDocument()->clear();

  app->slotUpdateProgress(0, 0, i18n("Loading data"));

  // lazy load data
  KstDataObjectList bitBucket;
  {
    KST::dataObjectList.lock().readLock();
    KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
    KST::dataObjectList.lock().readUnlock();
    for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
      assert(*i);
      //kstdDebug() << "Load inputs for " << (*i)->tagName() << " " << (void*)*i << endl;
      (*i)->KstRWLock::writeLock();
      bool rc = (*i)->loadInputs();
      (*i)->KstRWLock::writeUnlock();
      if (!rc) {
        // schedule for removal
        bitBucket.append(*i);
      }
      kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
    }
  }

  KST::dataObjectList.lock().writeLock();
  for (KstDataObjectList::Iterator i = bitBucket.begin(); i != bitBucket.end(); ++i) {
    KST::dataObjectList.remove(*i);
  }
  KST::dataObjectList.lock().writeUnlock();

  if (!bitBucket.isEmpty()) {
    QStringList names;
    for (KstDataObjectList::Iterator i = bitBucket.begin(); i != bitBucket.end(); ++i) {
      names += (*i)->tagName();
    }

    QApplication::restoreOverrideCursor();
#if KDE_VERSION < KDE_MAKE_VERSION(3,3,90)
    KMessageBox::informationList(KstApp::inst(), i18n("The Kst file could not be loaded in its entirety due to missing objects or data."), names);
#else
    KMessageBox::errorList(KstApp::inst(), i18n("The Kst file could not be loaded in its entirety due to missing objects or data."), names);
#endif
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  createScalars();

  app->slotUpdateProgress(0, 0, i18n("Synchronizing data"));
  kapp->dcopClient()->setAcceptCalls(true);

  _nextEventPaint = true;

  emit updateDialogs();

  if (!activeWindow.isEmpty()) {
    KMdiChildView *c = app->findWindow(activeWindow);
    if (c) {
      c->activate();
    }
  }

  app->slotUpdateProgress(0, 0, QString::null);

  _modified = false;

  QTimer::singleShot(0, this, SLOT(enableUpdates()));

  opening = false;

  KstApp::inst()->setPaused(false);
  QApplication::restoreOverrideCursor();
  
  if (cleanupFile) {
    KIO::NetAccess::removeTempFile(tmpFile);
  }
  return true;
}


void KstDoc::saveDocument(QTextStream& ts, bool saveAbsoluteVectorPositions) {
  KstApp *app = KstApp::inst();
  ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  ts << "<kstdoc version=\"1.2\">" << endl;
  // save window geometry for this kst file
  ts << "  <windowsize>" << endl;
  ts << "    <width>" << app->width() << "</width>" << endl;
  ts << "    <height>"<< app->height()<< "</height>" << endl;
  KMdiChildView *c = app->activeWindow();
  if (c) {
    ts << "    <active name=\"" << QStyleSheet::escape(c->caption()) << "\"/>" << endl;
  }
  ts << "  </windowsize>" << endl;

  // save graphics autosave settings
  ts << "  <graphicsautosave time=\""
    << app->graphFileDlg()->autoSaveTimer()
    << "\" enabled=\""
    << (app->graphFileDlg()->autoSaving() ? "true" : "false")
    << "\" format=\""
    << QStyleSheet::escape(app->graphFileDlg()->format())
    << "\" xsize=\""
    << app->graphFileDlg()->imageXSize()
    << "\" ysize=\""
    << app->graphFileDlg()->imageYSize()
    << "\" display=\""
    << app->graphFileDlg()->display()
    // #### Kst2 Remove me - compatibility
    << "\" square=\""
    << (app->graphFileDlg()->display() == 1 ? "true" : "false");

  if (app->graphFileDlg()->autoSaving()) {
    ts << "\" url=\"" << QStyleSheet::escape(app->graphFileDlg()->url().url());
  }
  ts << "\" />" << endl;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KstRMatrixList rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);

  // save files
  KST::dataSourceList.lock().readLock();
  for (uint i = 0; i < KST::dataSourceList.count(); i++) {
    KstDataSourcePtr dsp = KST::dataSourceList[i];
    bool saved = false;
    for (KstRVectorList::Iterator it = rvl.begin(); it != rvl.end() && !saved; ++it) {
      if ((*it)->dataSource() == dsp) {
        ts << "  <kstfile>" << endl;
        dsp->save(ts, "    ");
        ts << "  </kstfile>" << endl;
        saved = true;
      } 
    }
    for (KstRMatrixList::Iterator it = rml.begin(); it != rml.end() && !saved; ++it) {
      if ((*it)->dataSource() == dsp) {
        ts << "  <kstfile>" << endl;
        dsp->save(ts, "    ");
        ts << "  </kstfile>" << endl;
        saved = true;
      }
    }
  }
  KST::dataSourceList.lock().readUnlock();

  // save orphan scalars
  for (KstScalarList::Iterator it = KST::scalarList.begin(); it != KST::scalarList.end(); ++it) {
    if ((*it)->orphan()) {
      ts << "  <scalar>" << endl;
      (*it)->save(ts, "    ");
      ts << "  </scalar>" << endl;
    }
  }

  // save orphan strings
  for (KstStringList::Iterator it = KST::stringList.begin(); it != KST::stringList.end(); ++it) {
    if ((*it)->orphan()) {
      ts << "  <string>" << endl;
      (*it)->save(ts, "    ");
      ts << "  </string>" << endl;
    }
  }

  // save vectors
  for (KstVectorList::Iterator it = KST::vectorList.begin(); it != KST::vectorList.end(); ++it) {
    if ((*it)->saveable()) {
      (*it)->save(ts, "  ", saveAbsoluteVectorPositions);
    }
  }
  
  // save matrices
  for (KstMatrixList::Iterator it = KST::matrixList.begin(); it != KST::matrixList.end(); ++it) {
    if ((*it)->saveable()) {
      (*it)->save(ts, "  ");  
    }  
  }

  // save data objects
  KST::dataObjectList.lock().readLock();
  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    (*it)->save(ts, "  ");
  }
  KST::dataObjectList.lock().readUnlock();
  
  // save plots
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  while (it->currentItem()) {
    KstViewWindow *c = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (c) {
      KstPlotBaseList plots = c->view()->findChildrenType<KstPlotBase>(true);
      for (KstPlotBaseList::Iterator i = plots.begin(); i != plots.end(); ++i) {
        Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(*i);
        if (plot) {
          ts << "  <plot>" << endl;
          plot->saveAttributes(ts, "    ");
          ts << "  </plot>" << endl;
        }
      }
    }
    it->next();
  }
  app->deleteIterator(it);

  // save windows
  it = app->createIterator();
  if (it) {
    while (it->currentItem()) {
      KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (v) {
        ts << "  <window>" << endl;
        v->save(ts, "    ");
        ts << "  </window>" << endl;
      }
      it->next();
    }
    app->deleteIterator(it);
  }

  ts << "</kstdoc>" << endl;
}


bool KstDoc::saveDocument(const QString& filename, bool saveAbsoluteVectorPositions, bool prompt) {
  KURL url;
  if (QFile::exists(filename) && QFileInfo(filename).isRelative()) {
    url.setPath(QFileInfo(filename).absFilePath());
  } else {
    url = KURL::fromPathOrURL(filename);
  }

  if (QFile::exists(filename)) {
    // Only backup local files
    backupFile(filename);
  }
  KTempFile tf(locateLocal("tmp", "kst"), "txt");

  QTextStream ts(tf.file());
  ts.setEncoding(QTextStream::UnicodeUTF8);
  ts.precision(14);

  _lastFilePath = url.prettyURL();

  saveDocument(ts, saveAbsoluteVectorPositions);

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  tf.sync();
#else
  tf.close();
#endif

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  if (KIO::NetAccess::exists(url, false, KstApp::inst())) {
#else
  if (KIO::NetAccess::exists(url)) {
#endif
    if (prompt) {
      int rc = KMessageBox::warningYesNo(KstApp::inst(), i18n("File %1 exists.  Overwrite?").arg(url.prettyURL()), i18n("Kst"));
      if (rc == KMessageBox::No) {
        return false;
      }
    }
  }

#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,0)
  KIO::NetAccess::file_copy(KURL(tf.name()), url, -1, true, false, KstApp::inst());
#else
  KIO::NetAccess::upload(tf.name(), url);
#endif

  _modified = false;
  return true;
}


void KstDoc::deleteContents() {
  KST::vectorDefaults.sync();
  KST::matrixDefaults.sync();

  KstApp *app = KstApp::inst();
  if (app) { // Can be null on application exit
    KMdiIterator<KMdiChildView*> *it = app->createIterator();
    if (it) {
      while (it->currentItem()) {
        KMdiChildView *view = it->currentItem();
        it->next();
        app->closeWindow(view);
      }
      app->deleteIterator(it);
    }
  }

  KST::vectorList.lock().writeLock();
  KST::vectorList.clear();
  KST::vectorList.lock().writeUnlock();
  
  KST::matrixList.lock().writeLock();
  KST::matrixList.clear();
  KST::matrixList.lock().writeUnlock();

  KST::scalarList.lock().writeLock();
  KST::scalarList.clear();
  KST::scalarList.lock().writeUnlock();

  KST::stringList.lock().writeLock();
  KST::stringList.clear();
  KST::stringList.lock().writeUnlock();

  KST::dataSourceList.lock().writeLock();
  KST::dataSourceList.clear();
  KST::dataSourceList.lock().writeUnlock();

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.clear();
  KST::dataObjectList.lock().writeUnlock();

  emit updateDialogs();
}


void KstDoc::samplesUp() {
  bool changed = false;
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (int i = 0; i < (int)rvl.count(); i++) {
    KstRVectorPtr V = rvl[i];
    V->writeLock();
    int f0 = V->reqStartFrame();
    int n = V->reqNumFrames();
    int skip = V->skip();
    bool doSkip = V->doSkip();
    bool doAve = V->doAve();
    int fileN = V->fileLength();

    bool didChange = false;
    if (f0 + 2 * n > fileN) {
      didChange = f0 != fileN - n;
      changed = changed || didChange;
      f0 = fileN - n;
    } else {
      didChange = true;
      f0 += n;
    }

    if (didChange) {
      V->changeFrames(f0, n, skip, doSkip, doAve);
    }
    V->writeUnlock();
  }

  if (changed) {
    setModified();
    forceUpdate();
    emit dataChanged();
  }
}


void KstDoc::samplesDown() {
  bool changed = false;
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (int i = 0; i < (int)rvl.count(); i++) {
    KstRVectorPtr V = rvl[i];
    V->writeLock();
    int f0 = V->reqStartFrame();
    if (f0 == -1) {
      f0 = V->startFrame();
    }
    int n = V->reqNumFrames();
    int skip = V->skip();
    bool doSkip = V->doSkip();
    bool doAve =  V->doAve();

    bool didChange = false;
    if (f0 - n < 0) {
      didChange = f0 != 0;
      changed = changed || didChange;
      f0 = 0;
    } else {
      didChange = true;
      f0 -= n;
    }

    if (didChange) {
      V->changeFrames(f0, n, skip, doSkip, doAve);
    }
    V->writeUnlock();
  }

  if (changed) {
    setModified();
    forceUpdate();
    emit dataChanged();
  }
}


void KstDoc::samplesEnd() {
  KstRVectorPtr V;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (int i = 0; i < (int)rvl.count(); i++) {
    V = rvl[i];
    V->writeLock();
    V->setFromEnd();
    V->writeUnlock();
  }

  setModified();
  forceUpdate();

  emit dataChanged();
}


void KstDoc::wasModified() {
  _modified = true;
  forceUpdate();
  QTimer::singleShot(0, this, SIGNAL(updateDialogs()));
}


RemoveStatus KstDoc::removeDataObject(const QString& tag) {
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.removeTag(tag);
  KST::dataObjectList.lock().writeUnlock();
  setModified();
  forceUpdate();
  return OBJECT_DELETED;
}


void KstDoc::purge() {
  QString purging = i18n("Purging unused objects");
  bool modified = false;
  bool again = true;
  KstApp *app = KstApp::inst();

  while (again) {
    KST::dataObjectList.lock().readLock();
    KST::matrixList.lock().readLock();
    KST::vectorList.lock().readLock();
    int cnt = KST::matrixList.count() + KST::vectorList.count() + KST::dataObjectList.count();
    KST::vectorList.lock().readUnlock();
    KST::matrixList.lock().readUnlock();
    KST::dataObjectList.lock().readUnlock();
    
    int prg = 0;
    app->slotUpdateProgress(cnt, prg, purging);
    again = false;
  
    // ASSUMPTION: this only gets called from the data manager!
    KST::dataObjectList.lock().writeLock();
    for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
      //kstdDebug() << "OBJECT: " << (*it)->tagName() << " USAGE: " << (*it)->getUsage() << endl;
      if ((*it)->getUsage() == 0) {
        //kstdDebug() << "    -> REMOVED" << endl;
        KstDataObjectList::Iterator byebye = it;
        --it;
        KST::dataObjectList.remove(byebye);
        again = true;
        modified = true;
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
    KST::dataObjectList.lock().writeUnlock();
  
    KST::vectorList.lock().readLock();
    KstVectorList vectorList = QDeepCopy<KstVectorList>(KST::vectorList);
    KST::vectorList.lock().readUnlock();
    
    // clear unused vectors that are editable 
    for (KstVectorList::ConstIterator it = vectorList.begin(); it != vectorList.end(); ++it) {
      //kstdDebug() << "VECTOR: " << (*it)->tagName() << " USAGE: " << (*it)->getUsage() << endl;
      if ((*it)->getUsage() == 1) {
        //kstdDebug() << "    -> REMOVED" << endl;
        KST::vectorList.lock().writeLock();
        KST::vectorList.remove(const_cast<KstVector*>((*it).data()));
        KST::vectorList.lock().writeUnlock();
        again = true;
        modified = true;
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
    
    KST::matrixList.lock().readLock();
    KstMatrixList matrixList = QDeepCopy<KstMatrixList>(KST::matrixList);
    KST::matrixList.lock().readUnlock();

    // clear unused matrices that are editable
    for (KstMatrixList::ConstIterator it = matrixList.begin(); it != matrixList.end(); ++it) {
      if ((*it)->getUsage() == 1) {
        //kstdDebug() << "    -> REMOVED" << endl;
        KST::matrixList.lock().writeLock();
        KST::matrixList.remove(const_cast<KstMatrix*>((*it).data()));
        KST::matrixList.lock().writeUnlock();
        again = true;
        modified = true;
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
  }

  KstDataSourceList dataList;
  KST::dataSourceList.lock().readLock();
  for (KstDataSourceList::ConstIterator it = KST::dataSourceList.begin(); it != KST::dataSourceList.end(); ++it) {
      if ((*it)->getUsage() == 1) {
        //kstdDebug() << "    -> REMOVED" << endl;
        dataList.append(const_cast<KstDataSource*>((*it).data()));
        modified = true;
      }
  }
  KST::dataSourceList.lock().readUnlock();
  KST::dataSourceList.lock().writeLock();
  for (KstDataSourceList::ConstIterator it = dataList.begin(); it != dataList.end(); ++it) {
    KST::dataSourceList.remove(const_cast<KstDataSource*>((*it).data()));
  }
  KST::dataSourceList.lock().writeUnlock();
 
  setModified(modified);
  emit updateDialogs();
  
  app->slotUpdateProgress(0, 0, QString::null);
}


void KstDoc::forceUpdate() {
  _nextEventPaint = false;
  KstApp::inst()->forceUpdate();
}


void KstDoc::enableUpdates() {
  _updating = false;
}


bool KstDoc::event(QEvent *e) {
  if (e->type() == KstEventTypeThread) {
    _updating = true; // block update thread from sending events 'till we're done

    ThreadEvent *te = static_cast<ThreadEvent*>(e);
    switch (te->_eventType) {
      case ThreadEvent::UpdateDataDialogs:
        {
          emit dataChanged();
          KstApp::inst()->updateDataNotifier();
          // HACK: remove me later
          KMdiIterator<KMdiChildView*> *it = KstApp::inst()->createIterator();
          if (it) {
            while (it->currentItem()) {
              KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
              if (!view) {
                it->next();
                continue;
              }

              Kst2DPlotList pl = view->view()->findChildrenType<Kst2DPlot>(true);
              for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
                for (QValueList<KstBaseCurve*>::ConstIterator j = te->_curves.begin(); j != te->_curves.end(); ++j) {
                  // race: if ((*i)->Curves.contains(*j)) {
                  const KstBaseCurveList& cl = (*i)->Curves;
                  bool doBreak = false;
                  for (KstBaseCurveList::ConstIterator k = cl.begin(); k != cl.end(); ++k) {
                    if (*j == *k) {
                      (*i)->setDirty();
                      doBreak = true;
                      break;
                    }
                  }
                  if (doBreak) {
                    break;
                  }
                }
              }
              it->next();
            }
            KstApp::inst()->deleteIterator(it);
          }
          KstApp::inst()->paintAll(KstPainter::P_UPDATE);
        }
        break;
      case ThreadEvent::UpdateAllDialogs:
        //kstdDebug() << "Update ALL dialogs" << endl;
        {
          KMdiIterator<KMdiChildView*>* it = KstApp::inst()->createIterator();
          if (it) {
            while (it->currentItem()) {
              KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
              if (view) {
                view->view()->update();
              }
              it->next();
            }
            KstApp::inst()->deleteIterator(it);
          }

          emit updateDialogs();
        }
        break;
      case ThreadEvent::Done:
        break;
      case ThreadEvent::Repaint:
        KstApp::inst()->paintAll();
        break;
      case ThreadEvent::NoUpdate:
        if (_nextEventPaint) {
          KstApp::inst()->paintAll();
        }
        break;
      default:
        break;
    }

    _nextEventPaint = false;
    QTimer::singleShot(0, this, SLOT(enableUpdates()));

    return true;
  } else if (e->type() == KstEventTypeLog) {
    LogEvent *le = dynamic_cast<LogEvent*>(e);
    if (le) {
      switch (le->_eventType) {
        case LogEvent::LogAdded:
          emit logAdded(le->_msg);
          if (le->_msg.level == KstDebug::Error) {
            QTimer::singleShot(0, KstApp::inst(), SLOT(createDebugNotifier()));
          }
          break;
        case LogEvent::LogCleared:
          emit logCleared();
          break;
        default:
          break;
      }
    }
    return true;
  }

  return false;
}


void KstDoc::createScalars() const {
#if 0
  // Should not be orphans, just unparented so they are not saved
  new KstScalar("CONST_MKSA_SPEED_OF_LIGHT", 2.99792458e8 , false);
  new KstScalar("CONST_MKSA_GRAVITATIONAL_CONSTANT", 6.673e-11, false);
  new KstScalar("CONST_MKSA_PLANCKS_CONSTANT_H", 6.62606876e-34, false);
  new KstScalar("CONST_MKSA_PLANCKS_CONSTANT_HBAR", 1.05457159642e-34, false);
  new KstScalar("CONST_MKSA_ASTRONOMICAL_UNIT", 1.49597870691e11, false);
  new KstScalar("CONST_MKSA_VACUUM_PERMITTIVITY", 8.854187817e-12, false);
  new KstScalar("CONST_MKSA_VACUUM_PERMEABILITY", 1.25663706144e-6, false);
  new KstScalar("CONST_MKSA_GRAV_ACCEL", 9.80665e0, false);
  new KstScalar("CONST_MKSA_MASS_MUON", 1.88353109e-28, false);
  new KstScalar("CONST_MKSA_MASS_PROTON", 1.67262158e-27, false);
  new KstScalar("CONST_MKSA_MASS_NEUTRON", 1.67492716e-27, false);
  new KstScalar("CONST_MKSA_RYDBERG", 2.17987190389e-18, false);
  new KstScalar("CONST_MKSA_BOLTZMANN", 1.3806503e-23, false);
  new KstScalar("CONST_MKSA_SOLAR_MASS", 1.98892e30, false);
  new KstScalar("CONST_MKSA_BOHR_RADIUS", 5.291772083e-11, false);
  new KstScalar("CONST_MKSA_ELECTRON_CHARGE", 1.602176462e-19, false);
  new KstScalar("CONST_MKSA_MOLAR_GAS", 8.314472e0, false);
  new KstScalar("CONST_MKSA_STANDARD_GAS_VOLUME", 2.2710981e-2, false);
#endif
  new KstScalar("CONST_PI", M_PI, false);
  new KstScalar("C_PI", M_PI, false);
  new KstScalar("C_R2D", 180.0/M_PI, false); // radians to degrees
  new KstScalar("C_D2R", M_PI/180.0, false); // degrees to radians
}


// All code after this point is code copied from KDE libraries
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

   int fd = open(filename, O_RDONLY);
   if (fd < 0) {
      return false;
   }

   struct stat buff;
   if ( fstat( fd, &buff) < 0 )
   {
      ::close( fd );
      return false;
   }

   QCString cBackup;
   if ( backupDir.isEmpty() ) {
       cBackup = cFilename;
   }
   else
   {
       QCString nameOnly;
       int slash = cFilename.findRev('/');
       if (slash < 0)
         nameOnly = cFilename;
       else
         nameOnly = cFilename.mid(slash + 1);
       cBackup = QFile::encodeName(backupDir);
       if ( backupDir[backupDir.length()-1] != '/' ) {
           cBackup += '/';
       }
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
          if (errno == EINTR) {
              continue;
          }
          ::close(fd);
          ::close(fd2);
          return false;
       }
       if (n == 0) {
          break; // Finished
       }

       if (write_all( fd2, buffer, n))
       {
          ::close(fd);
          ::close(fd2);
          return false;
       }
    }

    ::close( fd );

    if (::close(fd2)) {
      return false;
    }
    return true;
}

#include "kstdoc.moc"

// vim: ts=2 sw=2 et
