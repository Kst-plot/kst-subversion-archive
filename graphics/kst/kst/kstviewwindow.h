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

#include <config.h>

// include files for KDE
#include <kdeversion.h>
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif
#include <klocale.h>

// application specific includes
#include "kst.h"
#include "kstdatacollection.h"
#include "kstdefaultnames.h"

class QLabel;

/** The base class for Kst application view windows.
  */
class KstViewWindow : public KMdiChildView {
  Q_OBJECT
  public:
    KstViewWindow(QWidget *parent=0, const char *name=0);
    KstViewWindow(const QDomElement& e, QWidget *parent=0, const char *name=0);
    virtual ~KstViewWindow();

    /** pause the updating of data */
    void setPaused(bool paused);
    void togglePaused();
    void save(QTextStream& ts, const QString& indent = QString::null);
    KstTopLevelViewPtr view() const;
    virtual void setCaption(const QString& szCaption);

  protected:
    /** saves the window properties for each open window during session
     * end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KMainWindow#saveProperties */
    virtual void saveProperties(KConfig *cfg);

    /** reads the session config file and restores the application's
     * state including the last opened files and documents by reading
     * the temporary files saved by saveProperties()
     * @see KMainWindow#readProperties */
    virtual void readProperties(KConfig *cfg);

    /**
    * Ignores the event and calls KMdiMainFrm::childWindowCloseRequest instead. This is because the
    * mainframe has control over the views. Therefore the MDI view has to request the mainframe for a close.
    */
    virtual void closeEvent(QCloseEvent *e);

  private slots:
    // Hack to update KStdActions
    void updateActions();

    void slotActivated(KMdiChildView*);

  public slots:
    /** asks for saving if the file is modified, then closes the actual
      file and window*/
    void slotFileClose();

    /** print without querying */
    void immediatePrintToFile(const QString& filename);

    /** export to png without querying */
    void immediatePrintToPng(QDataStream* dataStream, const QSize& size, const QString& format = "PNG");
    void immediatePrintToPng(const QString& filename, const QSize& size, const QString& format = "PNG");

  public:
    template<class T> QString createPlot(const QString& suggestedName = QString::null, bool prompt = false);

  private:
    void commonConstructor();

    /** the configuration object of the application */
    KConfig *config;
    KstTopLevelViewPtr _view;
};


template<class T>
QString KstViewWindow::createPlot(const QString& suggestedName, bool prompt) {

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
  //create the plot now
  _view->createPlot<T>(name);
  return name;
}


#endif
// vim: ts=2 sw=2 et
