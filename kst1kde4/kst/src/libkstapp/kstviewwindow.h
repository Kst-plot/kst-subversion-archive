/***************************************************************************
                               kstviewwindow.h
                             -------------------
    begin                : Apr 2004
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

#ifndef KSTVIEWWINDOW_H
#define KSTVIEWWINDOW_H

#include "kst.h"
#include "kstdatacollection.h"
#include "kstdefaultnames.h"
#include "kst_export.h"

class KST_EXPORT KstViewWindow : public QMdiSubWindow {
  Q_OBJECT
  public:
    KstViewWindow(QWidget *parent=0, const char *name=0);
    KstViewWindow(const QDomElement &e, QWidget *parent=0, const char *name=0);
    virtual ~KstViewWindow();

    void setPaused(bool paused);
    void togglePaused();
    void save(QTextStream &ts, const QString& indent = QString::null);
    void print( KstPainter &paint, const QSize &size, int pages, int lineAdjust, bool monochrome, bool enhanceReadability, bool dateTimeFooter, bool maintainAspectRatio, int pointStyleOrder, int lineStyleOrder, int lineWidthOrder, int maxLineWidth, int pointDensity );
    KstTopLevelViewPtr view() const;
    QString createPlotObject(const QString &suggestedName = QString::null, bool prompt = false);
    QString createPlot(const QString &suggestedName = QString::null, bool prompt = false);

    virtual void setCaption(const QString &szCaption);

  protected:
    /** saves the window properties for each open window during session
     * end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KMainWindow#saveProperties */
    virtual void saveProperties(QSettings *cfg);

    /** reads the session config file and restores the application's
     * state including the last opened files and documents by reading
     * the temporary files saved by saveProperties()
     * @see KMainWindow#readProperties */
    virtual void readProperties(QSettings *cfg);

    /**
    * Ignores the event and calls KMdiMainFrm::childWindowCloseRequest instead. 
    * This is because the mainframe has control over the views. 
    * Therefore the MDI view has to request the mainframe for a close.
    */
    virtual void closeEvent(QCloseEvent *e);

  private slots:
    void updateActions();
    void slotActivated(QMdiSubWindow*);

  public slots:
    void rename();
    void slotFileClose();
    void moveTabLeft();
    void moveTabRight();
    void immediatePrintToFile(const QString &filename);
    void immediatePrintToEps(const QString& filename, const QSize &size);
    void immediatePrintToPng(QDataStream *dataStream, const QSize &size, const QString &format = "PNG");
    void immediatePrintToPng(const QString &filename, const QSize &size, const QString &format = "PNG");

  private:
    void commonConstructor();

    QSettings *config;
    KstTopLevelViewPtr _view;
};

#endif
