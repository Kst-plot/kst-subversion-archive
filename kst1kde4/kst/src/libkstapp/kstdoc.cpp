
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
#include <qmessagebox.h>

// include files for KDE
#include <dcopclient.h>
#include "ksdebug.h"
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
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
#include "kstvectorview.h"
#include "ksthistogram.h"
#include "kstimage.h"
#include "kstmatrixdefaults.h"
#include "kstobjectdefaults.h"
#include "kstcplugin.h"
#include "kstpsd.h"
#include "kstrvector.h"
#include "kstvcurve.h"
#include "kstvectordefaults.h"
#include "kstsvector.h"
#include "kstavector.h"
#include "kstrmatrix.h"
#include "kstamatrix.h"
#include "kstsmatrix.h"
#include "kstviewlabel.h"
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
  createScalars();
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
      int want_save = QMessageBox::warning( win, i18n("Question"), i18n("The current plot definition has been modified. Do you want to save it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      switch (want_save) {
        case QMessageBox::Yes:
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

        case QMessageBox::No:
          if (doDelete) {
            setModified(false);
            deleteContents();
          }
          completed = true;
          break;

        case QMessageBox::Cancel:
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
  //if (QMessageBox::Yes == QMessageBox::warningYesNo(KstApp::inst(), i18n("Are you sure you wish to erase all your data and plots?"))) {
    deleteContents();
    _modified = false;
    _absFilePath = QDir::homePath();
    _title = "Untitled";
    KstApp::inst()->newWindow(i18n("default name of first window", "W1"));
    createScalars();
    emit updateDialogs();
  //}

  return true;
}

bool KstDoc::openDocument(const QUrl& url, const QString& o_file,
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
    if (!KIO::NetAccess::exists(url, true, KstApp::inst())) {
      QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));

      return false;
    }

    if (!KIO::NetAccess::download(url, tmpFile, KstApp::inst())) {
      QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));

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
    QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("%1: There is no file with that name to open.").arg(url.prettyURL()));
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
    QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("%1: File exists, but kst could not open it.").arg(url.prettyURL()));
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
    QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("%1: Not a valid kst plot specification file.").arg(url.prettyURL()));
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
    QMessageBox::warning(KstApp::inst(), i18n("kst"), err);
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
      KIO::NetAccess::removeTempFile(tmpFile);
    }

    return false;
  }

  if (docElem.attribute("version").toDouble() > 1.3 && !docElem.attribute("version").isEmpty()) {
    QString err = i18n("While opening file %2, version %1 is too new.  Update Kst or fix the Kst file.  Attempting to load as-is.").arg(docElem.attribute("version")).arg(url.prettyURL());
    KstDebug::self()->log(err, KstDebug::Warning);
  }

  createScalars();

  QDomNode n = docElem.firstChild();
  count = docElem.childNodes().length();
  app->slotUpdateProgress(count, handled, readingDocument);

  kapp->dcopClient()->setAcceptCalls(false);

  // disable display tags for speed
  {
    KstWriteLocker ml(&KST::matrixList.lock());
    KST::matrixList.setUpdateDisplayTags(false);
  }
  {
    KstWriteLocker sl(&KST::scalarList.lock());
    KST::scalarList.setUpdateDisplayTags(false);
  }
  {
    KstWriteLocker sl(&KST::stringList.lock());
    KST::stringList.setUpdateDisplayTags(false);
  }
  {
    KstWriteLocker vl(&KST::vectorList.lock());
    KST::vectorList.setUpdateDisplayTags(false);
  }

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
        KstWriteLocker sl(&KST::scalarList.lock());
        new KstScalar(e);
      } else if (e.tagName() == "string") {
        KstWriteLocker sl(&KST::stringList.lock());
        new KstString(e);
      } else if (e.tagName() == "vector") {
        KstWriteLocker vl(&KST::vectorList.lock());
        vector = new KstRVector(e, o_file, o_n, o_f, o_s, o_ave);
        // Vectors are automatically added to the global list.
      } else if (e.tagName() == "svector") {
        KstWriteLocker vl(&KST::vectorList.lock());
        KstSVectorPtr svector = new KstSVector(e);
      } else if (e.tagName() == "avector") {
        KstWriteLocker vl(&KST::vectorList.lock());
        KstAVectorPtr avector = new KstAVector(e);
      } else if (e.tagName() == "plugin") {
        const QString name = e.attribute("name");
        KstDataObjectPtr p;
        if (name.isEmpty()) {
          p = new KstCPlugin(e);
        } else {
          if (p = KstDataObject::createPlugin(name))
            p->load(e);
        }
        if (p) {
          KstWriteLocker dowl(&KST::dataObjectList.lock());
          KST::dataObjectList.append(p);
        }
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
      } else if (e.tagName() == "vectorview") {
        KstDataObjectPtr p = new KstVectorView(e);
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
        KstWriteLocker ml(&KST::matrixList.lock());
        KstAMatrixPtr p = new KstAMatrix(e);
      } else if (e.tagName() == "smatrix") {
        KstWriteLocker ml(&KST::matrixList.lock());
        KstSMatrixPtr p = new KstSMatrix(e);
      } else if (e.tagName() == "rmatrix") {
        KstWriteLocker ml(&KST::matrixList.lock());
        KstRMatrixPtr p = new KstRMatrix(e);
      } else if (e.tagName() == "image") {
        KstDataObjectPtr p = new KstImage(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "window") {
        KstViewWindow *viewWindow = new KstViewWindow(e);

        app->addSubWindow(viewWindow, SubWindow);
        viewWindow->activate();
      } else {
        KstDebug::self()->log(i18n("Unsupported element '%1' in file %2.").arg(e.tagName()).arg(url.prettyURL()), KstDebug::Warning);
      }
    }
    handled++;
    app->slotUpdateProgress(count, handled, readingDocument);
    n = n.nextSibling();
    kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
  }

  //
  // reenable display tags
  //

  {
    KstWriteLocker ml(&KST::matrixList.lock());
    KST::matrixList.setUpdateDisplayTags(true);
  }
  {
    KstWriteLocker sl(&KST::scalarList.lock());
    KST::scalarList.setUpdateDisplayTags(true);
  }
  {
    KstWriteLocker sl(&KST::stringList.lock());
    KST::stringList.setUpdateDisplayTags(true);
  }
  {
    KstWriteLocker vl(&KST::vectorList.lock());
    KST::vectorList.setUpdateDisplayTags(true);
  }

  if (warnOldKstFile) {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(KstApp::inst(), i18n("kst"), i18n("You tried to load an old Kst file.  Curves created by equations or spectra will not be loaded."));
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  app->slotUpdateProgress(0, 0, i18n("Creating plots"));

  //
  // if we have anything left in plotHolderWhileOpeningDocument then
  //  we are most likely reading an old style document, so we create
  //  a default view and fill it with whatever is left...
  //

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

  //
  // lazy load data
  //
  KstDataObjectList bitBucket;

  {
    KST::dataObjectList.lock().readLock();
    KstDataObjectList dol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
    KST::dataObjectList.lock().unlock();
    for (KstDataObjectList::Iterator i = dol.begin(); i != dol.end(); ++i) {
      assert(*i);
      //kstdDebug() << "Load inputs for " << (*i)->tagName() << " " << (void*)*i << endl;
      (*i)->KstRWLock::writeLock();
      bool rc = (*i)->loadInputs();
      (*i)->KstRWLock::unlock();
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
  KST::dataObjectList.lock().unlock();

  if (!bitBucket.isEmpty()) {
    QString names = i18n("The Kst file could not be loaded in its entirety due to missing objects or data.\n";
    for (KstDataObjectList::Iterator i = bitBucket.begin(); i != bitBucket.end(); ++i) {
      names += (*i)->tagName();
    }

    QApplication::restoreOverrideCursor();

    QMessageBox::critical(KstApp::inst(), i18n("Kst"), names);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  app->slotUpdateProgress(0, 0, i18n("Synchronizing data"));
  kapp->dcopClient()->setAcceptCalls(true);

  _nextEventPaint = true;

  emit updateDialogs();

  if (!activeWindow.isEmpty()) {
    QMdiSubWindow *c = app->findWindow(activeWindow);
    
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
  QMdiSubWindow *viewWindow = app->activeSubWindow();

  ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  ts << "<kstdoc version=\"1.3\">" << endl;

  //
  // save window geometry for this kst file
  //

  ts << "  <windowsize>" << endl;
  ts << "    <width>" << app->width() << "</width>" << endl;
  ts << "    <height>"<< app->height()<< "</height>" << endl;
  if (viewWindow) {
    ts << "    <active name=\"" << QStyleSheet::escape(viewWindow->caption()) << "\"/>" << endl;
  }
  ts << "  </windowsize>" << endl;

  //
  // save graphics autosave settings
  //

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

  //
  // save files
  //

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
  KST::dataSourceList.lock().unlock();

  //
  // save orphan scalars
  //

  for (KstScalarList::Iterator it = KST::scalarList.begin(); it != KST::scalarList.end(); ++it) {
    if ((*it)->orphan()) {
      ts << "  <scalar>" << endl;
      (*it)->save(ts, "    ");
      ts << "  </scalar>" << endl;
    }
  }

  //
  // save orphan strings
  //

  for (KstStringList::Iterator it = KST::stringList.begin(); it != KST::stringList.end(); ++it) {
    if ((*it)->orphan()) {
      ts << "  <string>" << endl;
      (*it)->save(ts, "    ");
      ts << "  </string>" << endl;
    }
  }
  
  //
  // save vectors
  //

  for (KstVectorList::Iterator it = KST::vectorList.begin(); it != KST::vectorList.end(); ++it) {
    if ((*it)->saveable()) {
      (*it)->setSaveData(KstApp::inst()->saveData());
      (*it)->save(ts, "  ", saveAbsoluteVectorPositions);
    }
  }

  //
  // save matrices
  //

  for (KstMatrixList::Iterator it = KST::matrixList.begin(); it != KST::matrixList.end(); ++it) {
    if ((*it)->saveable()) {
      (*it)->save(ts, "  ");
    }
  }

  //
  // save data objects
  //

  KST::dataObjectList.lock().readLock();
  for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
    (*it)->save(ts, "  ");
  }
  KST::dataObjectList.lock().unlock();

  //
  // save plots
  //

  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);

    if (viewWindow) {
      KstPlotBaseList plots = viewWindow->view()->findChildrenType<KstPlotBase>(true);
      KstPlotBaseList::Iterator it;

      for (it = plots.begin(); it != plots.end(); ++it) {
        Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(*it);
        if (plot) {
          ts << "  <plot>" << endl;
          plot->saveAttributes(ts, "    ");
          ts << "  </plot>" << endl;
        }
      }
    }
  }

  app->saveTabs(ts);

  ts << "</kstdoc>" << endl;
}


bool KstDoc::saveDocument(const QString& filename, bool saveAbsoluteVectorPositions, bool prompt) {
  KUrl url;

  if (QFile::exists(filename) && QFileInfo(filename).isRelative()) {
    url.setPath(QFileInfo(filename).absFilePath());
  } else {
    url = KUrl::fromPathOrURL(filename);
  }

  if (QFile::exists(filename)) {
    //
    // only backup local files
    //

    backupFile(filename);
  }

  KTempFile tf(locateLocal("tmp", "kst"), "txt");

  QTextStream ts(tf.file());
  ts.setEncoding(QTextStream::UnicodeUTF8);
  ts.precision(14);

  _lastFilePath = url.prettyURL();

  saveDocument(ts, saveAbsoluteVectorPositions);

  tf.sync();
  tf.close();

  if (KIO::NetAccess::exists(url, false, KstApp::inst())) {
    if (prompt) {
      int rc = QMessageBox::warning(KstApp::inst(), i18n("Kst"), i18n("File %1 exists.  Overwrite?").arg(url.prettyURL()), QMessageBox::Yes | QMessageBox::No);
      if (rc == QMessageBox::No) {
        return false;
      }
    }
  }

  KIO::NetAccess::file_copy(KUrl(tf.name()), url, -1, true, false, KstApp::inst());

  _modified = false;

  return true;
}


void KstDoc::deleteContents() {
  KST::vectorDefaults.sync();
  KST::matrixDefaults.sync();
  KST::objectDefaults.sync();

  KstApp *app = KstApp::inst();
  if (app) { // Can be null on application exit
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i)
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      app->closeWindow(viewWindow);
    }
  }

  KST::dataObjectList.lock().writeLock();

  //
  // avoid deadlock in DataObject destructor
  //

  KstDataObjectList tmpDol = QDeepCopy<KstDataObjectList>(KST::dataObjectList);
  KST::dataObjectList.clear();
  KST::dataObjectList.lock().unlock();
  tmpDol.clear();

  KST::dataSourceList.lock().writeLock();
  KST::dataSourceList.clear();
  KST::dataSourceList.lock().unlock();

  KST::matrixList.lock().writeLock();
  KST::matrixList.clear();
  KST::matrixList.lock().unlock();

  KST::vectorList.lock().writeLock();
  KST::vectorList.clear();
  KST::vectorList.lock().unlock();

  KST::scalarList.lock().writeLock();
  KST::scalarList.clear();
  KST::scalarList.lock().unlock();

  KST::stringList.lock().writeLock();
  KST::stringList.clear();
  KST::stringList.lock().unlock();

  emit updateDialogs();
}


void KstDoc::samplesUp() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  bool changed = false;

  for (int i = 0; i < (int)rvl.count(); i++) {
    KstRVectorPtr v = rvl[i];
    v->writeLock();

    int f0 = v->reqStartFrame();
    int n = v->reqNumFrames();
    int skip = v->skip();
    bool doSkip = v->doSkip();
    bool doAve = v->doAve();
    int fileN = v->fileLength();
    bool didChange = false;

    if (f0 == -1) {
      didChange = true;
      f0 = fileN - n;
    } else if (f0 + 2 * n > fileN) {
      didChange = f0 != fileN - n;
      changed = changed || didChange;
      f0 = fileN - n;
    } else {
      didChange = true;
      f0 += n;
    }

    if (didChange) {
      v->changeFrames(f0, n, skip, doSkip, doAve);
    }

    v->unlock();
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
    KstRVectorPtr v = rvl[i];
    v->writeLock();
    int f0 = v->reqStartFrame();
    if (f0 == -1) {
      f0 = v->startFrame();
    }
    int n = v->reqNumFrames();
    int skip = v->skip();
    bool doSkip = v->doSkip();
    bool doAve =  v->doAve();

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
      v->changeFrames(f0, n, skip, doSkip, doAve);
    }

    v->unlock();
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
    V->unlock();
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
  KST::dataObjectList.lock().unlock();
  setModified();
  forceUpdate();
  return OBJECT_DELETED;
}


void KstDoc::purge() {
#ifdef PURGEDEBUG
  kstdDebug() << "Purging unused objects" << endl;
#endif

  QString purging = i18n("Purging unused objects");
  bool modified = false;
  bool again = true;
  KstApp *app = KstApp::inst();

  while (again) {
    KST::dataObjectList.lock().readLock();
    KST::matrixList.lock().readLock();
    KST::vectorList.lock().readLock();
    int cnt = KST::matrixList.count() + KST::vectorList.count() + KST::dataObjectList.count();
    KST::vectorList.lock().unlock();
    KST::matrixList.lock().unlock();
    KST::dataObjectList.lock().unlock();

    int prg = 0;
    app->slotUpdateProgress(cnt, prg, purging);
    again = false;

    // ASSUMPTION: this only gets called from the data manager!
    KST::dataObjectList.lock().writeLock();
    for (KstDataObjectList::Iterator it = KST::dataObjectList.begin(); it != KST::dataObjectList.end(); ++it) {
#ifdef PURGEDEBUG
      kstdDebug() << "OBJECT: " << (*it)->tag().displayString() << " (" << (void*)(*it) << ") USAGE: " << (*it)->getUsage() << endl;
#endif
      if ((*it)->getUsage() == 0 && !kst_cast<EventMonitorEntry>(*it)) {
#ifdef PURGEDEBUG
        kstdDebug() << "    -> REMOVED" << endl;
#endif
        KstDataObjectList::Iterator byebye = it;
        --it;
        KST::dataObjectList.remove(byebye);
        again = true;
        modified = true;
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
    KST::dataObjectList.lock().unlock();

    KST::vectorList.lock().readLock();
    KstVectorList vectorList = QDeepCopy<KstVectorList>(KST::vectorList.list());
    KST::vectorList.lock().unlock();

    // clear unused vectors that are editable 
    for (KstVectorList::ConstIterator it = vectorList.begin(); it != vectorList.end(); ++it) {
#ifdef PURGEDEBUG
      kstdDebug() << "VECTOR: " << (*it)->tag().displayString() << " (" << (void*)(*it) << ") USAGE: " << (*it)->getUsage() << endl;
//      if ((*it)->provider()) {
//        kstdDebug() << "  provider=" << (*it)->provider()->tag().displayString() << endl;
//      }
//      KstRVectorPtr rvp = kst_cast<KstRVector>(*it);
//      if (rvp && rvp->_file) {
//        KstDataSource *file = rvp->_file;
//        kstdDebug() << "  file=" << file->tag().displayString() << " (" << (void*)(&(*file)) << "): " << file->getUsage() << endl;
//      }
#endif
      if ((*it)->getUsage() == 1) {
#ifdef PURGEDEBUG
        kstdDebug() << "    -> REMOVED" << endl;
#endif
        KST::vectorList.lock().writeLock();
        if (KST::vectorList.removeObject(const_cast<KstVector*>((*it).data()))) {
          again = true;
          modified = true;
        }
        KST::vectorList.lock().unlock();
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }

    KST::matrixList.lock().readLock();
    KstMatrixList matrixList = QDeepCopy<KstMatrixList>(KST::matrixList.list());
    KST::matrixList.lock().unlock();

    // clear unused matrices that are editable
    for (KstMatrixList::ConstIterator it = matrixList.begin(); it != matrixList.end(); ++it) {
#ifdef PURGEDEBUG
      kstdDebug() << "MATRIX: " << (*it)->tag().displayString() << " (" << (void*)(*it) << ") USAGE: " << (*it)->getUsage() << endl;
//      if ((*it)->provider()) {
//        kstdDebug() << "  provider=" << (*it)->provider()->tag().displayString() << endl;
//      }
//      KstRMatrixPtr rmp = kst_cast<KstRMatrix>(*it);
//      if (rmp && rmp->_file) {
//        KstDataSource *file = rmp->_file;
//        kstdDebug() << "  file=" << file->tag().displayString() << " (" << (void*)(&(*file)) << "): " << file->getUsage() << endl;
//      }
#endif
      if ((*it)->getUsage() == 1) {
#ifdef PURGEDEBUG
        kstdDebug() << "    -> REMOVED" << endl;
#endif
        KST::matrixList.lock().writeLock();
        if (KST::matrixList.removeObject(const_cast<KstMatrix*>((*it).data()))) {
          again = true;
          modified = true;
        }
        KST::matrixList.lock().unlock();
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
  }

  KstDataSourceList dataList;
  KST::dataSourceList.lock().readLock();
  for (KstDataSourceList::ConstIterator it = KST::dataSourceList.begin(); it != KST::dataSourceList.end(); ++it) {
      KstDataSourcePtr ds = *it; // MUST use a reference-counted pointer to call getUsage()
#ifdef PURGEDEBUG
      kstdDebug() << "DATA SOURCE: " << ds->tag().displayString() << " (" << (void*)ds << ") USAGE: " << ds->getUsage() << endl;
#endif
      if (ds->getUsage() == 1) {
#ifdef PURGEDEBUG
        kstdDebug() << "    -> REMOVED" << endl;
#endif
        dataList.append(const_cast<KstDataSource*>((*it).data()));
        modified = true;
      }
  }
  KST::dataSourceList.lock().unlock();
  KST::dataSourceList.lock().writeLock();
  for (KstDataSourceList::ConstIterator it = dataList.begin(); it != dataList.end(); ++it) {
    KST::dataSourceList.remove(const_cast<KstDataSource*>((*it).data()));
  }
  KST::dataSourceList.lock().unlock();

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
    //
    // block update thread from sending events until we're done
    //
    _updating = true;

    ThreadEvent *te = static_cast<ThreadEvent*>(e);
    switch (te->_eventType) {
      case ThreadEvent::UpdateDataDialogs:
        {
          emit dataChanged();

          KstApp::inst()->updateDataNotifier();

          QList<QMdiSubWindow*> windows;
          QList<QMdiSubWindow*>::const_iterator i;
        
          windows = app->subWindowList(QMdiArea::CreationOrder);
        
          for (i = windows.constBegin(); i != windows.constEnd(); ++i)
            KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);
            if (viewWindow) {
              Kst2DPlotList pl = viewWindow->view()->findChildrenType<Kst2DPlot>(true);
              
              for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
                QValueList<KstBaseCurve*>::ConstIterator j;

                for (j = te->_curves.begin(); j != te->_curves.end(); ++j) {
                  // race: if ((*i)->Curves.contains(*j)) 
                  KstBaseCurveList::ConstIterator k;
                  const KstBaseCurveList& cl = (*i)->Curves;
                  bool doBreak = false;
                  
                  for (k = cl.begin(); k != cl.end(); ++k) {
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

                viewWindow->view()->recursively<int, KstViewObject>((void (KstViewObject::*)(int))&KstViewObject::update, te->_counter);
              }
            }
          }

          KstApp::inst()->paintAll(KstPainter::P_UPDATE);
        }
        break;

      case ThreadEvent::UpdateAllDialogs:
        {
          QList<QMdiSubWindow*> windows;
          QList<QMdiSubWindow*>::const_iterator i;
        
          windows = app->subWindowList(QMdiArea::CreationOrder);
        
          for (i = windows.constBegin(); i != windows.constEnd(); ++i)
            KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);
            if (viewWindow) {
              viewWindow->view()->update();
            }
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

    //
    // there are good reasons NOT to set _updating to false using a single shot timer:
    // the main thread may be waiting on a call to Kst.waitForUpdate(...),
    //  which will not return until this update is completed, which
    //  is not until _updating is set equal to false.
    // Attempting to set _updating to false through a timer will 
    //  never be processed if the main thread is waiting for 
    //  _updating to be set to false.
    //
    // QTimer::singleShot(0, this, SLOT(enableUpdates()));
    //

    _updating = false;

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
  new KstScalar(KstObjectTag("CONST_PI", KstObjectTag::constantTagContext), 0L, M_PI, false);
  new KstScalar(KstObjectTag("C_PI", KstObjectTag::constantTagContext), 0L, M_PI, false);
  new KstScalar(KstObjectTag("C_R2D", KstObjectTag::constantTagContext), 0L, 180.0/M_PI, false); // radians to degrees
  new KstScalar(KstObjectTag("C_D2R", KstObjectTag::constantTagContext), 0L, M_PI/180.0, false); // degrees to radians
}


// All code after this point is code copied from KDE libraries
static int write_all(int fd, const char *buf, size_t len) {
  while (len > 0) {
    int written = write(fd, buf, len);
    if (written < 0) {
      if (errno == EINTR) {
        continue;
      }
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
  if ( fstat( fd, &buff) < 0 ) {
    ::close( fd );
    return false;
  }

  QCString cBackup;
  if ( backupDir.isEmpty() ) {
    cBackup = cFilename;
  } else {
    QCString nameOnly;
    int slash = cFilename.findRev('/');

    if (slash < 0) {
      nameOnly = cFilename;
    } else {
      nameOnly = cFilename.mid(slash + 1);
    }
    cBackup = QFile::encodeName(backupDir);
    if ( backupDir[backupDir.length()-1] != '/' ) {
      cBackup += '/';
    }
    cBackup += nameOnly;
  }
  cBackup += QFile::encodeName(backupExtension);
  const char *backup = cBackup.data();
  int permissions = buff.st_mode & 07777;

  if ( stat( backup, &buff) == 0) {
    if ( unlink( backup ) != 0 ) {
      ::close(fd);
      return false;
    }
  }

  mode_t old_umask = umask(0);
  int fd2 = open( backup, O_WRONLY | O_CREAT | O_EXCL, permissions | S_IWUSR);
  umask(old_umask);

  if ( fd2 < 0 ) {
    ::close(fd);
    return false;
  }

  char buffer[ 32*1024 ];

  while( 1 ) {
    int n = ::read( fd, buffer, 32*1024 );
    if (n == -1) {
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

    if (write_all( fd2, buffer, n)) {
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
