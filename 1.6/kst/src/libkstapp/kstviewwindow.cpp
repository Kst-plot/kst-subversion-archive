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
#include "kst2dplot.h"
#include "kstplotlabel.h"
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
  _view->applyDefaults();
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


void KstViewWindow::immediatePrintToEps(const QString &filename, const QSize& size) {
  if (!view()->children().isEmpty()) {
    QString filenameNew;
    QString filenameNewEps;
    int right;
    int bottom;

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
      right = ( 72 * size.height() ) / resolution;
      bottom = ( 72 * size.width() ) / resolution;

      printer.setMargins(0, 0, 0, 0);
      printer.setResolution(resolution);
      printer.setPageSize(QPrinter::Letter);
      printer.setOrientation(QPrinter::Landscape);
      printer.setOutputToFile(true);
      printer.setOutputFileName(filenameNew);
      printer.setColorMode(QPrinter::Color);

      KstPainter paint(KstPainter::P_PRINT);
      paint.begin(&printer);
      QPaintDeviceMetrics metrics(&printer);

      view()->resizeForPrint(size);
      view()->paint(paint, QRegion());
      view()->revertForPrint();
      paint.end();
    }

    //
    // now try to open the .ps file and convert it to an .eps...
    //
    QFile filePS(filenameNew);
    QFile fileEPS(filenameNewEps);
    QString line;

    if (filePS.open(IO_ReadOnly)) {
      if (fileEPS.open(IO_WriteOnly | IO_Truncate)) {
        QTextStream streamPS(&filePS);
        QTextStream streamEPS(&fileEPS);

        line = streamPS.readLine();
        if (line.startsWith("%!PS-Adobe-")) {
          //
          // we have a .ps file, so do the conversion...
          //
          streamEPS << "%!PS-Adobe-2.0 EPSF-2.0\n";

          line = streamPS.readLine();
          if (line.startsWith("%%BoundingBox:")) {
            streamEPS << "%%BoundingBox: 0 0 " << right << " " << bottom << "\n";
          } else {
            streamEPS << line << "\n";
          }

          while (!streamPS.atEnd()) {
            line = streamPS.readLine();
            streamEPS << line << "\n";
          }
        }
        fileEPS.close();
      }
      filePS.close();
      filePS.remove();
    }
  }
}


void KstViewWindow::print(KstPainter& paint, const QSize& size, int pages, int lineAdjust, bool monochrome, bool enhanceReadability, bool datetimeFooter, bool maintainAspectRatio, int pointStyleOrder, int lineStyleOrder, int lineWidthOrder, int maxLineWidth, int pointDensity) {
  KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(view());
  QSize sizeNew = size;

  if (tlv) {
    if (lineAdjust != 0) {
      tlv->forEachChild(&Kst2DPlot::pushAdjustLineWidth, lineAdjust, false);
    }
    if (monochrome) {
      tlv->forEachChild2(&Kst2DPlot::pushPlotColors);
      tlv->forEachChild<const QColor&>(&Kst2DPlot::pushCurveColor, Qt::black, false);
      // additional pushes for enhanced readability
      if (enhanceReadability) {
        Kst2DPlotList plotList = tlv->findChildrenType<Kst2DPlot>(false);
        for (Kst2DPlotList::Iterator it = plotList.begin(); it != plotList.end(); ++it ) {
          (*it)->changeToMonochrome(pointStyleOrder, lineStyleOrder, lineWidthOrder, maxLineWidth, pointDensity);
        }
      }
    }

    if (datetimeFooter) {
      QDateTime dateTime = QDateTime::currentDateTime();
      QString title = i18n("Page: %1  Name: %2  Date: %3").arg(pages).arg(caption()).arg(dateTime.toString(Qt::ISODate));
      QRect rect(0, 0, sizeNew.width(), sizeNew.height());
      QRect rectBounding;

      rectBounding = paint.boundingRect(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
      rect.setTop(sizeNew.height() - (2 * rectBounding.height()));
      paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
      sizeNew.setHeight(rect.top());
    }

    if (maintainAspectRatio) {
      const QRect geom(view()->geometry());
      const double ratioWindow = double(geom.width()) / double(geom.height());
      const double ratioPrinter = double(sizeNew.width()) / double(sizeNew.height());
      if (ratioWindow > ratioPrinter) {
        sizeNew.setHeight(int(double(sizeNew.width()) / ratioWindow));
      } else if (ratioWindow < ratioPrinter) {
        sizeNew.setWidth(int(ratioWindow * double(sizeNew.height())));
      }
    }

    view()->resizeForPrint(sizeNew);
    view()->paint(paint, QRegion());

    if (lineAdjust != 0) {
      tlv->forEachChild2(&Kst2DPlot::popLineWidth);
    }
    if (monochrome) {
      tlv->forEachChild2(&Kst2DPlot::popPlotColors);
      tlv->forEachChild2(&Kst2DPlot::popCurveColor);
      // additional pops to undo enhanced readability
      if (enhanceReadability) {
        Kst2DPlotList plotList = tlv->findChildrenType<Kst2DPlot>(false);
        for (Kst2DPlotList::Iterator it = plotList.begin(); it != plotList.end(); ++it ) {
          (*it)->undoChangeToMonochrome(pointStyleOrder, lineStyleOrder, lineWidthOrder);
        }
      }
    }

    view()->revertForPrint();
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
  _view->setTagName(KstObjectTag(caption, KstObjectTag::globalTagContext));  // FIXME: global tag context?
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


QString KstViewWindow::createPlotObject(const QString& suggestedName, bool prompt) {
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *iter;

  QString name = suggestedName;
  bool duplicate = true;
  while (duplicate) {
    duplicate = false;
    KstViewObjectPtr rc;
    //check the name
    iter = app->createIterator();
    while (iter->currentItem() && !duplicate) {
      KMdiChildView *childview = iter->currentItem();
      KstViewWindow *viewwindow = dynamic_cast<KstViewWindow*>(childview);
      if (viewwindow) {
        rc = viewwindow->view()->findChild(name);
        if (rc) {
          duplicate = true;
          name = KST::suggestPlotName();
        }
      }
      iter->next();
    }
    app->deleteIterator(iter);
  }

  if (prompt) {
    bool ok = false;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
    name = KInputDialog::getText(i18n("Kst"), i18n("Enter a name for the new plot:"), name, &ok);
#else
    name = KLineEditDlg::getText(i18n("Enter a name for the new plot:"), name, &ok, 0L);
#endif
    if (!ok) {
      return QString::null;
    }
    //check the name
    duplicate = true;
    while (duplicate) {
      duplicate = false;
      KstViewObjectPtr rc;
      //check the name
      iter = app->createIterator();
      while (iter->currentItem() && !duplicate) {
        KMdiChildView *childview = iter->currentItem();
        KstViewWindow *viewwindow = dynamic_cast<KstViewWindow*>(childview);
        if (viewwindow) {
          rc = viewwindow->view()->findChild(name);
          if (rc) {
            duplicate = true;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
            name = KInputDialog::getText(i18n("Kst"), i18n("Enter a name for the new plot:"), name, &ok);
#else
            name = KLineEditDlg::getText(i18n("Enter a name for the new plot:"), name, &ok, 0L);
#endif
            if (!ok) {
              app->deleteIterator(iter);
              return QString::null;
            }
          }
        }
        iter->next();
      }
      app->deleteIterator(iter);
    }
  }

  _view->createPlotObject(name);

  return name;
}


QString KstViewWindow::createPlot(const QString& suggestedName, bool prompt) {
  Kst2DPlotList plotList = view()->findChildrenType<Kst2DPlot>(false);
  QString name = createPlotObject(suggestedName, prompt);
  Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(view()->findChild(name));

  // if there are already plots in the window, use the the first one's font size.
  if (!plotList.isEmpty()) {
    plot->setPlotLabelFontSizes(plotList[0]->xTickLabel()->fontSize());
  }

  return (name);
}


#include "kstviewwindow.moc"

