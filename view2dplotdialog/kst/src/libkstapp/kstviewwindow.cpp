/***************************************************************************
                              kstviewwindow.cpp
                             -------------------
    begin                : April 2004
    copyright            : (C) 2004 by The University of Toronto
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

// include files for Qt
#include <qfile.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>

// include files for KDE
#include <kmessagebox.h>
#include <kprinter.h>

// application specific includes
#include "kstdoc.h"
#include "kstsettings.h"
#include "kstviewwindow.h"

#define KST_STATUSBAR_FRAME 0
#define KST_STATUSBAR_DATA 1
#define KST_STATUSBAR_STATUS 2

KstViewWindow::KstViewWindow(QWidget *parent, const char* name)
: KMdiChildView(QString::null, parent, name) {
  commonConstructor();
  _view = new KstTopLevelView(this, name);
}


KstViewWindow::KstViewWindow(const QDomElement& e, QWidget* parent, const char* name)
: KMdiChildView(QString::null, parent, name) {
  QString in_tag;
  QRect rectRestore;
  QRect rectInternal;

  commonConstructor();
  _view = new KstTopLevelView(e, this);

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull()) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
        setCaption(in_tag);
        setTabCaption(in_tag);
      } else if (e.tagName() == "restore") {
        rectRestore.setX( e.attribute( "x", "0" ).toInt() );
        rectRestore.setY( e.attribute( "y", "0" ).toInt() );
        rectRestore.setWidth( e.attribute( "w", "100" ).toInt() );
        rectRestore.setHeight( e.attribute( "h", "100" ).toInt() );
        setRestoreGeometry( rectRestore );
      } else if (e.tagName() == "internal") {
        rectInternal.setX( e.attribute( "x", "0" ).toInt() );
        rectInternal.setY( e.attribute( "y", "0" ).toInt() );
        rectInternal.setWidth( e.attribute( "w", "100" ).toInt() );
        rectInternal.setHeight( e.attribute( "h", "100" ).toInt() );
        setInternalGeometry( rectInternal );
      } else if (e.tagName() == "minimize") {
        minimize(false);
      } else if (e.tagName() == "maximize") {
        maximize(false);
      }
    }
    n = n.nextSibling();
  }
}


void KstViewWindow::commonConstructor() {
  config = kapp->config();

  connect(this, SIGNAL(focusInEventOccurs( KMdiChildView*)), this, SLOT(slotActivated(KMdiChildView*)));

  QTimer::singleShot(0, this, SLOT(updateActions()));

  QVBoxLayout *vb = new QVBoxLayout(this);
  vb->setAutoAdd(true);
}


void KstViewWindow::rename() {
  KstApp::inst()->renameWindow(this);
}


void KstViewWindow::moveTabLeft() {
  KstApp::inst()->moveTabLeft(this);
}


void KstViewWindow::moveTabRight() {
  KstApp::inst()->moveTabRight(this);
}


void KstViewWindow::updateActions() {
}


KstViewWindow::~KstViewWindow() {
  _view->release();
  KstApp *app = KstApp::inst(); // Can be null on exit - but then why do
                                // we even need to be calling this function
                                // here?  It seems wrong to me.
  if (app) {
    app->updateDialogsForWindow();
  }
}


void KstViewWindow::saveProperties(KConfig *config) {
  Q_UNUSED(config)
}


void KstViewWindow::readProperties(KConfig* config) {
  Q_UNUSED(config)
}


void KstViewWindow::slotActivated(KMdiChildView*) {
  // KDE bug: KMDIMainFrm close -> activate -> loops back from the destructor
  if (KstApp::inst()) {
    if (KstApp::inst()->getZoomRadio() == KstApp::LAYOUT) {
      if( view()->viewMode() == KstTopLevelView::DisplayMode ) {
        view()->setViewMode( KstTopLevelView::LayoutMode );
      }
    } else {
      if( view()->viewMode() == KstTopLevelView::LayoutMode ) {
        view()->setViewMode( KstTopLevelView::DisplayMode );
      }
    }
  }
}


void KstViewWindow::slotFileClose() {
}


void KstViewWindow::immediatePrintToFile(const QString &filename) {
  KPrinter printer(true, QPrinter::HighResolution);
  printer.setPageSize(KPrinter::Letter);
  printer.setOrientation(KPrinter::Landscape);
  printer.setOutputToFile(true);
  printer.setOutputFileName(filename);

  KstPainter p(KstPainter::P_PRINT);
  p.begin(&printer);
  QPaintDeviceMetrics metrics(&printer);
  const QSize size(metrics.width(), metrics.height());

  view()->resizeForPrint(size);
  view()->paint(p, QRegion());
  view()->revertForPrint();
  p.end();
}


void KstViewWindow::immediatePrintToPng(QDataStream* dataStream, const QSize& size, const QString &format) {
  if (!view()->children().isEmpty()) {
    QPixmap pixmap(size);
    KstPainter paint(KstPainter::P_EXPORT);
    if (paint.begin(&pixmap)) {
      const QSize sizeOld(view()->size());
      view()->resizeForPrint(size);
      view()->paint(paint, QRegion());
#if QT_VERSION >= 0x030200
      pixmap.save(dataStream->device(), format.latin1());
#else
      Q_UNUSED(format)

      QByteArray bytes;
      QDataStream tempStream(bytes, IO_ReadWrite);

      tempStream << pixmap;
      if (bytes.count() > 4) {
        dataStream->writeRawBytes(bytes.data()+4, bytes.count()-4);
      }
#endif
      view()->revertForPrint();
      paint.end();
    }
  }
}


void KstViewWindow::immediatePrintToPng(const QString &filename, const QSize& size, const QString &format) {
  if (!view()->children().isEmpty()) {
    KstPainter paint(KstPainter::P_EXPORT);
    QPixmap pixmap(size);

    if (paint.begin(&pixmap)) {
      QString dotFormat = QString(".") + format;
      QString filenameNew;
      const int pos = filename.findRev(dotFormat, -1, false);

      if (pos != -1 && pos == int(filename.length() - dotFormat.length())) {
        filenameNew = filename;
      } else {
        filenameNew = filename + "." + format.lower();
      }

      view()->resizeForPrint(size);
      view()->paint(paint, QRegion());
      if (!pixmap.save(filenameNew, format.latin1())) {
        KstDebug::self()->log(i18n("Failed to save image to %1").arg(filename), KstDebug::Warning);
      }
      view()->revertForPrint();
      paint.end();
    }
  }
}


// FIXME: don't write to disk like this.  write to a pipe or at least use
// KTempFile
void KstViewWindow::immediatePrintToEps(const QString &filename, const QSize& size) {
  if (!view()->children().isEmpty()) {
    QString filenameNew;
    QString filenameNewEps;

    {
      QPrinter printer(QPrinter::HighResolution);
      QString dotFormat = QString(".eps");
      const int pos = filename.findRev(dotFormat, -1, false);
      
      if (pos != -1 && pos == int(filename.length() - dotFormat.length())) {
        filenameNewEps = filename;
      } else {
        filenameNewEps = filename + dotFormat;
      }
      filenameNew = filenameNewEps + ".ps";

      int resolution = size.width() / 11;
      if ( size.height() / 8 > resolution ) {
        resolution = size.height() / 8;
      }
      
      printer.setResolution(resolution);
      printer.setPageSize(QPrinter::Letter);
      printer.setOrientation(QPrinter::Landscape);
      printer.setOutputToFile(true);
      printer.setOutputFileName(filenameNew);
      
      KstPainter paint(KstPainter::P_PRINT);
      paint.begin(&printer);
      QPaintDeviceMetrics metrics(&printer);

      view()->resizeForPrint(size);
      view()->paint(paint, QRegion());
      view()->revertForPrint();
      paint.end();
    }

    //
    // now try to open the ps file and convert it to an eps...
    //
    QFile filePS(filenameNew);
    QFile fileEPS(filenameNewEps);
    QString line;

    if (filePS.open(IO_ReadOnly)) {
      if (fileEPS.open(IO_WriteOnly | IO_Truncate)) {
        QTextStream streamPS(&filePS);
        QTextStream streamEPS(&fileEPS);

        line = streamPS.readLine();
        if (!line.isNull()) {
          if (line.left(11) == "%!PS-Adobe-") {
            //
            // we have a ps file, so do the conversion...
            //
            streamEPS << "%!PS-Adobe-2.0 EPSF-2.0\n";
            while (!streamPS.atEnd()) {
              line = streamPS.readLine();
              streamEPS << line << "\n";
            }
          }
        }
        fileEPS.close();
      }
      filePS.close();
      filePS.remove();
    }
  }
}


void KstViewWindow::save(QTextStream& ts, const QString& indent) {
  const QRect restoreGeom(restoreGeometry());
  const QRect internalGeom(internalGeometry());

  ts << indent << "<tag>" << QStyleSheet::escape(caption()) << "</tag>" << endl;

  ts << indent << "<restore"  << " x=\"" << restoreGeom.x()
    << "\" y=\"" << restoreGeom.y()
    << "\" w=\"" << restoreGeom.width()
    << "\" h=\"" << restoreGeom.height() << "\" />" << endl;

  ts << indent << "<internal" << " x=\"" << internalGeom.x()
    << "\" y=\"" << internalGeom.y()
    << "\" w=\"" << internalGeom.width()
    << "\" h=\"" << internalGeom.height() << "\" />" << endl;

  if (isMinimized()) {
    ts << indent << "<minimized/>" << endl;
  }
  if (isMaximized()) {
    ts << indent << "<maximized/>" << endl;
  }

  view()->save(ts, indent);
}


KstTopLevelViewPtr KstViewWindow::view() const {
  return _view;
}


void KstViewWindow::setCaption(const QString& caption) {
  KMdiChildView::setCaption(caption);
  _view->setTagName(caption);
}


void KstViewWindow::closeEvent(QCloseEvent *e) {
  if (KstSettings::globalSettings()->promptWindowClose && !view()->children().isEmpty()) {
    if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to close window '%1'?\nClosing a window deletes all plots in the window.").arg(caption())) != KMessageBox::Yes) {
      e->ignore();
      return;
    }
  }
  KMdiChildView::closeEvent(e);
}

#include "kstviewwindow.moc"
// vim: ts=2 sw=2 et
