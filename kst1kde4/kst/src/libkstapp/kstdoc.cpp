
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
#include <math.h>
#include <unistd.h>

#include <QEventLoop>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextDocument>

/* xxx
#include <dcopclient.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
*/

#include "kst2dplot.h"
#include "kstcsd.h"
#include "kstdataobjectcollection.h"
#include "kstdoc.h"
#include "kstgraphfiledialog.h"
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

KstDoc::KstDoc(QWidget *parent, const char *name) : QObject(parent) {
  _lock = 0;
  _updating = false;
  _stopping = false;
  _nextEventPaint = false;
  _title = "Untitled";
  createScalars();
  setObjectName(name);
}


KstDoc::~KstDoc() {
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

    if (win->activeSubWindow()) {
      int wantSave = QMessageBox::warning( win, QObject::tr("Question"), QObject::tr("The current plot definition has been modified. Do you want to save it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

      switch (wantSave) {
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
// xxx  deleteContents();
  _modified = false;
  _absFilePath = QDir::homePath();
  _title = "Untitled";
// xxx  KstApp::inst()->newWindow(QObject::tr("default name of first window", "W1"));
  createScalars();

  emit updateDialogs();

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

  if (url.scheme() == "file" || url.isRelative()) {
    tmpFile = url.path();
  } else {
/* xxx   if (!KIO::NetAccess::exists(url, true, KstApp::inst()) || !KIO::NetAccess::download(url, tmpFile, KstApp::inst())) {
      QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("%1: There is no file with that name to open.").arg(url.toString()));

      return false;
    }

    cleanupFile = true;  */
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  opening = true;

  deleteContents();
  KstApp::inst()->setPaused(true);

  //
  // block update thread from sending events until we're done...
  //

  _updating = true;

  QFile f(tmpFile);

  if (!f.exists()) {
    QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("%1: There is no file with that name to open.").arg(url.path()));
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();

    return false;
  }

  QFileInfo fi(url.path());
  _title = fi.fileName();
  _absFilePath = url.toEncoded();
  _lastFilePath = url.toEncoded();
  if (_title.isEmpty()) {
    _title = _absFilePath;
  }

  QDomDocument doc(_title);

  if (!f.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("%1: File exists, but kst could not open it.").arg(url.path()));
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
// xxx      KIO::NetAccess::removeTempFile(tmpFile);
    }

    return false;
  }

  if (!doc.setContent(&f)) {
    QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("%1: Not a valid kst plot specification file.").arg(url.path()));
    f.close();
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
// xxx      KIO::NetAccess::removeTempFile(tmpFile);
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
  QString readingDocument = QObject::tr("Reading Kst file");

  if (docElem.tagName() != "kstdoc") {
    QString err = QObject::tr("Error opening file %1. Does not appear to be a Kst file.").arg(url.path());
    KstDebug::self()->log(err, KstDebug::Error);
    QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), err);
    opening = false;
    _updating = false;
    KstApp::inst()->setPaused(false);
    QApplication::restoreOverrideCursor();
    if (cleanupFile) {
// xxx      KIO::NetAccess::removeTempFile(tmpFile);
    }

    return false;
  }

  if (docElem.attribute("version").toDouble() > 1.3 && !docElem.attribute("version").isEmpty()) {
    QString err = QObject::tr("While opening file %2, version %1 is too new.  Update Kst or fix the Kst file.  Attempting to load as-is.").arg(docElem.attribute("version")).arg(url.path());
    KstDebug::self()->log(err, KstDebug::Warning);
  }

  createScalars();

  QDomNode n = docElem.firstChild();
  count = docElem.childNodes().length();
  app->slotUpdateProgress(count, handled, readingDocument);

// xxx  kapp->dcopClient()->setAcceptCalls(false);

  //
  // disable display tags for speed...
  //

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
/* xxx
        KstGraphFileDialog *gDlg;

        gDlg = app->graphFileDlg();
        int time = e.attribute("time").toInt();
        bool en = e.attribute("enabled") == "true";
        QString fmt = e.attribute("format");
        QString url = e.attribute("url");
        bool sq = e.attribute("square") == "true";
        int sizeX = e.attribute("xsize").toInt();
        int sizeY = e.attribute("ysize").toInt();
        int display = e.attribute("display").toInt();

        if (sizeX > 0 && sizeY > 0) {
          gDlg->setImageSize(sizeX, sizeY);
        }
        if (!url.isEmpty()) {
          gDlg->setURL(url);
        }
        if (!fmt.isEmpty()) {
          gDlg->setFormat(fmt);
        }
        if (time >= 0) {
          gDlg->setAutoSaveTimer(time);
        }
        if (sq) {
          gDlg->setDisplay(1);
        } else {
          gDlg->setDisplay(display);
        }
        gDlg->setAutoSave(en);
        gDlg->updateDialog();
        gDlg->applyAutosave();
*/
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
          KstDebug::self()->log(QObject::tr("No data in file %1.").arg(file->fileName()), KstDebug::Warning);
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
        KstSVectorPtr svector;

        KstWriteLocker vl(&KST::vectorList.lock());
        svector = new KstSVector(e);
      } else if (e.tagName() == "avector") {
        KstAVectorPtr avector;

        KstWriteLocker vl(&KST::vectorList.lock());
        avector = new KstAVector(e);
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
        KstDataObjectPtr p;

        p = new KstVCurve(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "equation") {
        KstEquationPtr p;

        p = new KstEquation(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
        warnOldKstFile = true;
      } else if (e.tagName() == "psd") {
        KstPSDPtr p;

        p = new KstPSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
        warnOldKstFile = true;
      } else if (e.tagName() == "equationobject") {
        KstDataObjectPtr p;

        p = new KstEquation(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "psdobject") {
        KstDataObjectPtr p;

        p = new KstPSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "csdobject") {
        KstDataObjectPtr p;

        p = new KstCSD(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "vectorview") {
        KstDataObjectPtr p;

        p = new KstVectorView(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "histogram") {
        KstDataObjectPtr p;
        
        p = new KstHistogram(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "event") {
        KstDataObjectPtr p;

        p = new EventMonitorEntry(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "plot") {
        KstBaseCurveList l;
        Kst2DPlotPtr plot;

        l = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
        plot = new Kst2DPlot(e);
        app->plotHolderWhileOpeningDocument()->insert(plot->tagName(), plot);
      } else if (e.tagName() == "amatrix") {
        KstAMatrixPtr p;

        KstWriteLocker ml(&KST::matrixList.lock());
        p = new KstAMatrix(e);
      } else if (e.tagName() == "smatrix") {
        KstSMatrixPtr p;

        KstWriteLocker ml(&KST::matrixList.lock());
        p = new KstSMatrix(e);
      } else if (e.tagName() == "rmatrix") {
        KstRMatrixPtr p;

        KstWriteLocker ml(&KST::matrixList.lock());
        p = new KstRMatrix(e);
      } else if (e.tagName() == "image") {
        KstDataObjectPtr p;

        p = new KstImage(e);
        KstWriteLocker dowl(&KST::dataObjectList.lock());
        KST::dataObjectList.append(p);
      } else if (e.tagName() == "window") {
        KstViewWindow *viewWindow;

        viewWindow = new KstViewWindow(e);
        if (viewWindow) {
          app->addSubWindow(viewWindow);
          viewWindow->activateWindow();
        }
      } else {
        KstDebug::self()->log(QObject::tr("Unsupported element '%1' in file %2.").arg(e.tagName()).arg(url.path()), KstDebug::Warning);
      }
    }
    handled++;
    app->slotUpdateProgress(count, handled, readingDocument);
    n = n.nextSibling();
// xxx    kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
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
    QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("You tried to load an old Kst file.  Curves created by equations or spectra will not be loaded."));
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  app->slotUpdateProgress(0, 0, QObject::tr("Creating plots"));

  //
  // if we have anything left in plotHolderWhileOpeningDocument then
  //  we are most likely reading an old style document, so we create
  //  a default view and fill it with whatever is left...
  //

  if (!app->plotHolderWhileOpeningDocument()->isEmpty() && !app->activeSubWindow()) {
    QString winName;
    KstViewWindow *win;
    
    winName = app->newWindow(QString::null);
    win = dynamic_cast<KstViewWindow*>(app->findWindow(winName));
    if (win) {
      Kst2DPlotMap::iterator it;

      if (columns != -1) {
        win->view()->setOnGrid(true);
        win->view()->setColumns(columns);
      }

      for (it = app->plotHolderWhileOpeningDocument()->begin();
          it != app->plotHolderWhileOpeningDocument()->end(); ++it ) {
        Kst2DPlotPtr plot;

        plot = *it;

        //
        // legacy 1.0 loading code...
        //

        if (plot->_width > 0.0 && plot->_height > 0.0 ) {
          win->view()->appendChild(plot);
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

  app->slotUpdateProgress(0, 0, QObject::tr("Loading data"));

  //
  // lazy load data
  //

  KstDataObjectList bitBucket;
  KstDataObjectList::iterator i;

  {
    KstDataObjectList dol;
    bool rc;

    KST::dataObjectList.lock().readLock();
    dol = KST::dataObjectList;
    KST::dataObjectList.lock().unlock();

    for (i = dol.begin(); i != dol.end(); ++i) {
      if (*i) {
        (*i)->KstRWLock::writeLock();
        rc = (*i)->loadInputs();
        (*i)->KstRWLock::unlock();

        if (!rc) {
          //
          // schedule for removal...
          //

          bitBucket.append(*i);
        }
      }

// xxx      kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
    }
  }

  KST::dataObjectList.lock().writeLock();
  for (i = bitBucket.begin(); i != bitBucket.end(); ++i) {
    KST::dataObjectList.removeOne(*i);
  }
  KST::dataObjectList.lock().unlock();

  if (!bitBucket.isEmpty()) {
    QString names = QObject::tr("The Kst file could not be loaded in its entirety due to missing objects or data.\n");

    for (i = bitBucket.begin(); i != bitBucket.end(); ++i) {
      names += (*i)->tagName();
    }

    QApplication::restoreOverrideCursor();

    QMessageBox::critical(KstApp::inst(), QObject::tr("Kst"), names);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

  app->slotUpdateProgress(0, 0, QObject::tr("Synchronizing data"));
// xxx  kapp->dcopClient()->setAcceptCalls(true);

  _nextEventPaint = true;

  emit updateDialogs();

  if (!activeWindow.isEmpty()) {
    QMdiSubWindow *mdi;

    mdi = app->findWindow(activeWindow);
    if (mdi) {
      mdi->activateWindow();
    }
  }

  app->slotUpdateProgress(0, 0, QString::null);

  _modified = false;

  QTimer::singleShot(0, this, SLOT(enableUpdates()));

  opening = false;

  KstApp::inst()->setPaused(false);
  QApplication::restoreOverrideCursor();

  if (cleanupFile) {
// xxx    KIO::NetAccess::removeTempFile(tmpFile);
  }
  return true;
}


void KstDoc::saveDocument(QTextStream& ts, bool saveAbsoluteVectorPositions) {
  KstApp *app = KstApp::inst();
  QMdiSubWindow *viewWindow = app->activeSubWindow();
  KstStringList::iterator itString; 
  KstVectorList::iterator itVector;
  KstMatrixList::iterator itMatrix;
  KstScalarList::iterator itScalar;
  KstDataObjectList::iterator itObject;

  ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  ts << "<kstdoc version=\"1.3\">" << endl;

  //
  // save window geometry for this kst file
  //

  ts << "  <windowsize>" << endl;
  ts << "    <width>" << app->width() << "</width>" << endl;
  ts << "    <height>"<< app->height()<< "</height>" << endl;
  if (viewWindow) {
    ts << "    <active name=\"" << Qt::escape(viewWindow->windowTitle()) << "\"/>" << endl;
  }
  ts << "  </windowsize>" << endl;

  //
  // save graphics autosave settings
  //
/* xxx
  ts << "  <graphicsautosave time=\""
    << app->graphFileDlg()->autoSaveTimer()
    << "\" enabled=\""
    << (app->graphFileDlg()->autoSaving() ? "true" : "false")
    << "\" format=\""
    << Qt::escape(app->graphFileDlg()->format())
    << "\" xsize=\""
    << app->graphFileDlg()->imageXSize()
    << "\" ysize=\""
    << app->graphFileDlg()->imageYSize()
    << "\" display=\""
    << app->graphFileDlg()->display()
    << "\" square=\""
    << (app->graphFileDlg()->display() == 1 ? "true" : "false");

  if (app->graphFileDlg()->autoSaving()) {
// xxx    ts << "\" url=\"" << Qt::escape(app->graphFileDlg()->url().url());
  }
  ts << "\" />" << endl;
*/
  KstRVectorList rvl;
  KstRMatrixList rml;
  KstDataSourceList::iterator dslit;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);

  //
  // save files
  //

  KST::dataSourceList.lock().readLock();
  for (dslit = KST::dataSourceList.begin(); dslit != KST::dataSourceList.end(); ++dslit) {
    KstRVectorList::iterator itRVector;
    KstRMatrixList::iterator itRMatrix;
    KstDataSourcePtr dsp;
    bool saved = false;

    dsp = *dslit;
    for (itRVector = rvl.begin(); itRVector != rvl.end() && !saved; ++itRVector) {
      if ((*itRVector)->dataSource() == dsp) {
        ts << "  <kstfile>" << endl;
        dsp->save(ts, "    ");
        ts << "  </kstfile>" << endl;
        saved = true;
      }
    }

    for (itRMatrix = rml.begin(); itRMatrix != rml.end() && !saved; ++itRMatrix) {
      if ((*itRMatrix)->dataSource() == dsp) {
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

  for (itScalar = KST::scalarList.begin(); itScalar != KST::scalarList.end(); ++itScalar) {
    if ((*itScalar)->orphan()) {
      ts << "  <scalar>" << endl;
      (*itScalar)->save(ts, "    ");
      ts << "  </scalar>" << endl;
    }
  }

  //
  // save orphan strings
  //

  for (itString = KST::stringList.begin(); itString != KST::stringList.end(); ++itString) {
    if ((*itString)->orphan()) {
      ts << "  <string>" << endl;
      (*itString)->save(ts, "    ");
      ts << "  </string>" << endl;
    }
  }
  
  //
  // save vectors
  //

  for (itVector = KST::vectorList.begin(); itVector != KST::vectorList.end(); ++itVector) {
    if ((*itVector)->saveable()) {
      (*itVector)->setSaveData(KstApp::inst()->saveData());
      (*itVector)->save(ts, "  ", saveAbsoluteVectorPositions);
    }
  }

  //
  // save matrices
  //

  for (itMatrix = KST::matrixList.begin(); itMatrix != KST::matrixList.end(); ++itMatrix) {
    if ((*itMatrix)->saveable()) {
      (*itMatrix)->save(ts, "  ");
    }
  }

  //
  // save data objects
  //

  KST::dataObjectList.lock().readLock();
  for (itObject = KST::dataObjectList.begin(); itObject != KST::dataObjectList.end(); ++itObject) {
    (*itObject)->save(ts, "  ");
  }
  KST::dataObjectList.lock().unlock();

  //
  // save plots
  //

  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      KstPlotBaseList::iterator it;
      KstPlotBaseList plots;
      Kst2DPlotPtr plot;

      plots = viewWindow->view()->findChildrenType<KstPlotBase>(true);
      for (it = plots.begin(); it != plots.end(); ++it) {
        plot = kst_cast<Kst2DPlot>(*it);
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
  if (QFile::exists(filename)) {
    //
    // only backup local files
    //

// xxx    backupFile(filename);
  }
/* xxx
  QTemporaryFile tf(locateLocal("tmp", "kst"), "txt");
  QTextStream ts(&tf);

  saveDocument(ts, saveAbsoluteVectorPositions);

  tf.close();

  if (QFile::exists(url)) {
    if (prompt) {
      int rc = QMessageBox::warning(KstApp::inst(), QObject::tr("Kst"), QObject::tr("File %1 exists.  Overwrite?").arg(url.path()), QMessageBox::Yes | QMessageBox::No);
      if (rc == QMessageBox::No) {
        return false;
      }
    }
  }

  QFile::copy(tf.fileName(), filename);
*/
  _lastFilePath = filename;

  _modified = false;

  return true;
}


void KstDoc::deleteContents() {
  KstDataObjectList tmpDol;
  KstApp *app;

  KST::vectorDefaults.sync();
  KST::matrixDefaults.sync();
  KST::objectDefaults.sync();

  app = KstApp::inst();
  if (app) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
    KstViewWindow *viewWindow;

    windows = app->subWindowList(QMdiArea::CreationOrder);

    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
        app->removeSubWindow(viewWindow);

        delete viewWindow;
      }
    }
  }

  //
  // avoid deadlock in DataObject destructor
  //

  KST::dataObjectList.lock().writeLock();
  tmpDol = KST::dataObjectList;
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
  KstRVectorList rvl;
  KstRVectorList::iterator it;
  bool changed = false;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (it = rvl.begin(); it != rvl.end(); ++it) {
    KstRVectorPtr v;

    v = *it;
    v->writeLock();

    bool doSkip = v->doSkip();
    bool doAve = v->doAve();
    int f0 = v->reqStartFrame();
    int n = v->reqNumFrames();
    int skip = v->skip();
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
  KstRVectorList rvl;
  KstRVectorList::iterator it;
  bool changed = false;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (it = rvl.begin(); it != rvl.end(); ++it) {
    KstRVectorPtr v;

    v = *it;
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
  KstRVectorPtr v;
  KstRVectorList rvl;
  KstRVectorList::iterator it;
/* xxx
  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (it = rvl.begin(); it != rvl.end(); ++it) {
    v = *it;
    v->writeLock();

    v->setFromEnd();

    v->unlock();
  }

  setModified();
  forceUpdate();
*/
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
  QString purging = QObject::tr("Purging unused objects");
  bool modified = false;
  bool again = true;
  KstApp *app = KstApp::inst();

  while (again) {
    KstVectorList vectorList;
    KstMatrixList matrixList;
    KstVectorList::const_iterator itvl;
    KstMatrixList::const_iterator itml;
    KstDataSourceList::const_iterator itdsl;
    KstDataObjectList::iterator itdol;
    int prg = 0;
    int cnt;

    KST::dataObjectList.lock().readLock();
    KST::matrixList.lock().readLock();
    KST::vectorList.lock().readLock();
    
    cnt = KST::matrixList.count();
    cnt += KST::vectorList.count();
    cnt += KST::dataObjectList.count();
    
    KST::vectorList.lock().unlock();
    KST::matrixList.lock().unlock();
    KST::dataObjectList.lock().unlock();

    app->slotUpdateProgress(cnt, prg, purging);
    again = false;

    //
    // ASSUMPTION: this only gets called from the data manager...
    //

    KST::dataObjectList.lock().writeLock();
    for (itdol = KST::dataObjectList.begin(); itdol != KST::dataObjectList.end(); ++itdol) {
      if ((*itdol)->getUsage() == 0 && !kst_cast<EventMonitorEntry>(*itdol)) {
        KstDataObjectList::iterator byebye;

        byebye = itdol;
        --itdol;
        KST::dataObjectList.removeOne(*byebye);
        again = true;
        modified = true;
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }
    KST::dataObjectList.lock().unlock();

    KST::vectorList.lock().readLock();
    vectorList = KST::vectorList.list();
    KST::vectorList.lock().unlock();

    //
    // clear unused vectors that are editable...
    //

    for (itvl = vectorList.begin(); itvl != vectorList.end(); ++itvl) {
      if ((*itvl)->getUsage() == 1) {
        KST::vectorList.lock().writeLock();
        if (KST::vectorList.removeObject(const_cast<KstVector*>((*itvl).data()))) {
          again = true;
          modified = true;
        }
        KST::vectorList.lock().unlock();
      }
      ++prg;
      app->slotUpdateProgress(cnt, prg, purging);
    }

    KST::matrixList.lock().readLock();
    matrixList = KST::matrixList.list();
    KST::matrixList.lock().unlock();

    //
    // clear unused matrices that are editable...
    //

    for (itml = matrixList.begin(); itml != matrixList.end(); ++itml) {
      if ((*itml)->getUsage() == 1) {
        KST::matrixList.lock().writeLock();
        if (KST::matrixList.removeObject(const_cast<KstMatrix*>((*itml).data()))) {
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
  KstDataSourceList::const_iterator itdsl;

  KST::dataSourceList.lock().readLock();
  for (itdsl = KST::dataSourceList.begin(); itdsl != KST::dataSourceList.end(); ++itdsl) {
    KstDataSourcePtr ds;

    ds = *itdsl; // MUST use a reference-counted pointer to call getUsage()
    if (ds->getUsage() == 1) {
      dataList.append(ds);
      modified = true;
    }
  }

  KST::dataSourceList.lock().unlock();

  KST::dataSourceList.lock().writeLock();
  for (itdsl = dataList.begin(); itdsl != dataList.end(); ++itdsl) {
    KST::dataSourceList.removeOne(*itdsl);
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
        
          windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);
        
          for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
            KstViewWindow *viewWindow;

            viewWindow = dynamic_cast<KstViewWindow*>(*i);
            if (viewWindow) {
              Kst2DPlotList pl;
              Kst2DPlotList::iterator i;

              pl = viewWindow->view()->findChildrenType<Kst2DPlot>(true);
              for (i = pl.begin(); i != pl.end(); ++i) {
                QList<KstBaseCurve*>::const_iterator j;

                for (j = te->_curves.begin(); j != te->_curves.end(); ++j) {
                  KstBaseCurveList::const_iterator k;
                  const KstBaseCurveList& cl = (*i)->_curves;
                  bool doBreak = false;
          
                  for (k = cl.begin(); k != cl.end(); ++k) {
                    if (*j == (*k).data()) {
                      (*i)->setDirty();
                      doBreak = true;

                      break;
                    }
                  }

                  if (doBreak) {
                    break;
                  }
                }

// xxx                viewWindow->view()->recursively<int, KstViewObject>((void (KstViewObject::*)(int))&KstViewObject::update, te->_counter);
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
        
          windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);
        
          for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
            KstViewWindow *viewWindow;

            viewWindow = dynamic_cast<KstViewWindow*>(*i);
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
    // there are good reasons NOT to set _updating to false using a 
    //  single shot timer: the main thread may be waiting on a call 
    //  to Kst.waitForUpdate(...), which will not return until this 
    //  update is completed, which is not until _updating is set 
    //  equal to false.
    // Attempting to set _updating to false through a timer will 
    //  never be processed if the main thread is waiting for 
    //  _updating to be set to false.
    //
    // QTimer::singleShot(0, this, SLOT(enableUpdates()));
    //

    _updating = false;

    return true;
  } else if (e->type() == KstEventTypeLog) {
    LogEvent *le;

    le = dynamic_cast<LogEvent*>(e);
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
  new KstScalar(KstObjectTag("CONST_PI", KstObjectTag::constantTagContext), 0L, M_PI, false);
  new KstScalar(KstObjectTag("C_PI", KstObjectTag::constantTagContext), 0L, M_PI, false);
  new KstScalar(KstObjectTag("C_R2D", KstObjectTag::constantTagContext), 0L, 180.0/M_PI, false); // radians to degrees
  new KstScalar(KstObjectTag("C_D2R", KstObjectTag::constantTagContext), 0L, M_PI/180.0, false); // degrees to radians
}
/* xxx
//
// all code after this point is code copied from KDE libraries...
//

static int writeAll(int fd, const char *buf, size_t len) {
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


static bool backupFile(const QString& qFilename, const QString& backupDir, const QString& backupExtension) {
  QString cFilename = QFile::encodeName(qFilename);
  const char *filename = cFilename.data();
  int fd;

  fd = open(filename, O_RDONLY);
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

    if (writeAll( fd2, buffer, n)) {
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
*/
#include "kstdoc.moc"
