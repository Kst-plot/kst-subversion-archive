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

#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QTextDocument>
#include <QVBoxLayout>

#include "kst2dplot.h"
#include "kstplotlabel.h"
#include "kstdoc.h"
#include "kstsettings.h"
#include "kstviewwindow.h"

#define KST_STATUSBAR_FRAME 0
#define KST_STATUSBAR_DATA 1
#define KST_STATUSBAR_STATUS 2

KstViewWindow::KstViewWindow(QWidget *parent, const char* name)
: QMdiSubWindow(parent) {
  commonConstructor();
  _view = new KstTopLevelView(this, name);
  _view->applyDefaults();
}


KstViewWindow::KstViewWindow(const QDomElement& e, QWidget* parent, const char* name)
: QMdiSubWindow(parent) {
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
// xxx        setTabCaption(in_tag);
      } else if (e.tagName() == "restore") {
        rectRestore.setX( e.attribute( "x", "0" ).toInt() );
        rectRestore.setY( e.attribute( "y", "0" ).toInt() );
        rectRestore.setWidth( e.attribute( "w", "100" ).toInt() );
        rectRestore.setHeight( e.attribute( "h", "100" ).toInt() );
// xxx        setRestoreGeometry( rectRestore );
      } else if (e.tagName() == "internal") {
        rectInternal.setX( e.attribute( "x", "0" ).toInt() );
        rectInternal.setY( e.attribute( "y", "0" ).toInt() );
        rectInternal.setWidth( e.attribute( "w", "100" ).toInt() );
        rectInternal.setHeight( e.attribute( "h", "100" ).toInt() );
// xxx        setInternalGeometry( rectInternal );
      } else if (e.tagName() == "minimize") {
// xxx        minimize(false);
      } else if (e.tagName() == "maximize") {
// xxx        maximize(false);
      }
    }
    n = n.nextSibling();
  }
}


void KstViewWindow::commonConstructor() {
// xxx  config = kapp->config();

// xxx  connect(this, SIGNAL(focusInEventOccurs( QMdiSubWindow*)), this, SLOT(slotActivated(QMdiSubWindow*)));

// xxx  QTimer::singleShot(0, this, SLOT(updateActions()));

// xxx  QVBoxLayout *vb = new QVBoxLayout(this);
// xxx  vb->setAutoAdd(true);
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
  KstApp *app;

  _view->release();

  app = KstApp::inst();
  if (app) {
    app->updateDialogsForWindow();
  }
}


void KstViewWindow::saveProperties(QSettings *config) {
  Q_UNUSED(config)
}


void KstViewWindow::readProperties(QSettings* config) {
  Q_UNUSED(config)
}


void KstViewWindow::slotActivated(QMdiSubWindow*) {
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

  QPrinter printer(QPrinter::HighResolution);
  printer.setPageSize(QPrinter::Letter);
  printer.setOrientation(QPrinter::Landscape);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(filename);

  KstPainter p(KstPainter::P_PRINT);
  p.begin(&printer);

  const QSize size(printer.width(), printer.height());

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
      pixmap.save(dataStream->device(), format.toLatin1());
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
      const int pos = filename.lastIndexOf(dotFormat, -1, Qt::CaseInsensitive);

      if (pos != -1 && pos == int(filename.length() - dotFormat.length())) {
        filenameNew = filename;
      } else {
        filenameNew = filename + "." + format.toLower();
      }

      view()->resizeForPrint(size);
      view()->paint(paint, QRegion());
      if (!pixmap.save(filenameNew, format.toLatin1())) {
        KstDebug::self()->log(QObject::tr("Failed to save image to %1").arg(filename), KstDebug::Warning);
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
      const int pos = filename.lastIndexOf(dotFormat, -1, Qt::CaseInsensitive);

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

      printer.setPageMargins(0.0, 0.0, 0.0, 0.0, QPrinter::Point);
      printer.setResolution(resolution);
      printer.setPageSize(QPrinter::Letter);
      printer.setOrientation(QPrinter::Landscape);
      printer.setOutputFileName(filenameNew);
      printer.setColorMode(QPrinter::Color);

      KstPainter paint(KstPainter::P_PRINT);
      paint.begin(&printer);

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

    if (filePS.open(QIODevice::ReadOnly)) {
      if (fileEPS.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
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
  KstTopLevelViewPtr tlv;
  QSize sizeNew = size;

  tlv = kst_cast<KstTopLevelView>(view());
  if (tlv) {
    if (lineAdjust != 0) {
      tlv->forEachChild(&Kst2DPlot::pushAdjustLineWidth, lineAdjust, false);
    }
    if (monochrome) {
      tlv->forEachChild2(&Kst2DPlot::pushPlotColors);
      tlv->forEachChild<const QColor&>(&Kst2DPlot::pushCurveColor, Qt::black, false);
      // additional pushes for enhanced readability
      if (enhanceReadability) {
        Kst2DPlotList plotList;

// xxx        plotList = tlv->findChildrenType<Kst2DPlot>(false);
        for (Kst2DPlotList::Iterator it = plotList.begin(); it != plotList.end(); ++it ) {
          (*it)->changeToMonochrome(pointStyleOrder, lineStyleOrder, lineWidthOrder, maxLineWidth, pointDensity);
        }
      }
    }

    if (datetimeFooter) {
      QDateTime dateTime = QDateTime::currentDateTime();
      QString title = QObject::tr("Page: %1  Name: %2  Date: %3").arg(pages).arg(windowTitle()).arg(dateTime.toString(Qt::ISODate));
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
        Kst2DPlotList plotList;
        Kst2DPlotList::iterator it;

// xxx        plostList = tlv->findChildrenType<Kst2DPlot>(false);
        for (it = plotList.begin(); it != plotList.end(); ++it ) {
          (*it)->undoChangeToMonochrome(pointStyleOrder, lineStyleOrder, lineWidthOrder);
        }
      }
    }

    view()->revertForPrint();
  }
}


void KstViewWindow::save(QTextStream& ts, const QString& indent) {
  const QRect restoreGeom;// xxx restoreGeometry());
  const QRect internalGeom;// xxx internalGeometry());

  ts << indent << "<tag>" << Qt::escape(windowTitle()) << "</tag>" << endl;

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
  QMdiSubWindow::setWindowTitle(caption);
  _view->setTagName(KstObjectTag(caption, KstObjectTag::globalTagContext));  // FIXME: global tag context?
}


void KstViewWindow::closeEvent(QCloseEvent *e) {
  if (KstSettings::globalSettings()->promptWindowClose && !view()->children().isEmpty()) {
    if (QMessageBox::warning(this, tr("Kst"), tr("Are you sure you want to close window '%1'?\nClosing a window deletes all plots in the window.").arg(windowTitle()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
      e->ignore();

      return;
    }
  }

  QMdiSubWindow::closeEvent(e);
}


QString KstViewWindow::createPlotObject(const QString& suggestedName, bool prompt) {
  KstApp *app = KstApp::inst();
  QString name = suggestedName;
  bool duplicate = true;

  while (duplicate) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
    KstViewObjectPtr rc;

    //
    // check the name
    //

    duplicate = false;
// xxx    windows = app->subWindowList(QMdiArea::CreationOrder); 
    for (i = windows.constBegin(); i != windows.constEnd() && !duplicate; ++i) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
        rc = viewWindow->view()->findChild(name);
        if (rc) {
          duplicate = true;
          name = KST::suggestPlotName();
        }
      }
    }
  }

  if (prompt) {
    bool ok = false;
    name = QInputDialog::getText(widget(), QObject::tr("Kst"), QObject::tr("Enter a name for the new plot:"), QLineEdit::Normal, name, &ok);

    if (ok) {
      //
      //check the name
      //
  
      duplicate = true;
      while (duplicate) {
        QList<QMdiSubWindow*> windows;
        QList<QMdiSubWindow*>::const_iterator i;
        KstViewObjectPtr rc;

        duplicate = false;
      
// xxx        windows = app->subWindowList(QMdiArea::CreationOrder);
      
        for (i = windows.constBegin(); i != windows.constEnd() && !duplicate; ++i) {
          KstViewWindow *viewWindow;

          viewWindow = dynamic_cast<KstViewWindow*>(*i);
          if (viewWindow) {
            rc = viewWindow->view()->findChild(name);
            if (rc) {
              duplicate = true;

              name = QInputDialog::getText(widget(), QObject::tr("Kst"), QObject::tr("Enter a name for the new plot:"), QLineEdit::Normal, name, &ok);

              if (!ok) {
                name = QString::null;

                break;
              }
            }
          }
        }
      }
    } else {
      name = QString::null;
    }
  }

  if (name != QString::null ) {
    _view->createPlotObject(name);
  }

  return name;
}


QString KstViewWindow::createPlot(const QString& suggestedName, bool prompt) {
  Kst2DPlotList plotList;
  Kst2DPlotPtr plot;
  QString name;

// xxx  plotList = view()->findChildrenType<Kst2DPlot>(false);
  name = createPlotObject(suggestedName, prompt);
  plot = kst_cast<Kst2DPlot>(view()->findChild(name));

  //
  // if there are already plots in the window, use the the first one's font size...
  //

  if (!plotList.isEmpty()) {
    plot->setPlotLabelFontSizes(plotList.first()->xTickLabel()->fontSize());
  }

  return (name);
}


#include "kstviewwindow.moc"
