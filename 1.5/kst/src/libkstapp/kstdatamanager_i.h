/***************************************************************************
                       kstdatamanger_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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
#ifndef KSTDATAMANAGERI_H
#define KSTDATAMANAGERI_H

#include <qaction.h>

class KstDoc;
class QToolBar;
class KListViewSearchLineWidget;

#include "datamanager.h"
#include "kstrvector.h"
#include "kstsvector.h"
#include "kstrmatrix.h"
#include "kstsmatrix.h"

class KstDataAction : public QAction {
  Q_OBJECT
  public:
    KstDataAction(const QString &menuText, QKeySequence accel, QObject *parent, const char *name = 0)
      : QAction(menuText, accel, parent, name) {}

  protected:
    void addedTo(QWidget *actionWidget, QWidget *container);
};

class KstDataManagerI : public KstDataManager {
  Q_OBJECT
  public:
    KstDataManagerI(KstDoc *doc, QWidget* parent = 0,
        const char *name = 0,
        bool modal = false, WFlags fl = 0);
    virtual ~KstDataManagerI();

    const QPixmap& yesPixmap() const;

  public slots:
    void update();
    void updateContents();
    void show_I();
    void edit_I();
    void delete_I();

  private slots:
    void doUpdates();
    void contextMenu(QListViewItem *i, const QPoint& p, int c);
    void currentChanged(QListViewItem *);
    void selectionChanged();
    void doubleClicked(QListViewItem *);
    void showOldPlugin();

  private:
    void createObjectAction(const QString &txt, QToolBar *bar,
                            QObject *receiver = 0, const char *slot = 0);
    void setupPluginActions();

  private:
    KstDoc *doc;
    QToolBar *_primitive;
    QToolBar *_data;
    QToolBar *_plugins;
    QToolBar *_fits;
    QToolBar *_filters;
    KListViewSearchLineWidget *_searchWidget;

  protected:
    QPixmap _yesPixmap;

  signals:
    void editDataVector(const QString &);
    void editStaticVector(const QString &);
    void editDataMatrix(const QString &);
    void editStaticMatrix(const QString &);
    void docChanged();
};

class KstObjectItem : public QObject, public KListViewItem {
  Q_OBJECT
  public:
    KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstSVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(KListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstRMatrixPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstSMatrixPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(KListViewItem *parent, KstMatrixPtr x, KstDataManagerI *dm, int localUseCount = 0);
    virtual ~KstObjectItem();

    void update(bool recursive = true, int localUseCount = 0);
    virtual int rtti() const { return _rtti; }

    const KstObjectTag& tag() const { return _tag; }
    KstDataObjectPtr dataObject();
    bool removable() const { return _removable; }
    void updateButtons();

  public slots:
    void addToPlot(int);
    void activateHint(int);
    void removeFromPlot(int);
    void makeCurve();
    void makeCSD();
    void makeHistogram();
    void makePSD();
    void makeImage();
    void reload();
    void showMetadata();

  signals:
    void updated();

  private:
    int _rtti;
    KstObjectTag _tag;
    KstDataManagerI *_dm;
    bool _removable;
    bool _inUse;
    void paintPlot(Kst2DPlotPtr p); //used by add to/remove from plot slots
};

#endif
// vim: ts=2 sw=2 et
