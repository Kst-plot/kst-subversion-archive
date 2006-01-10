/***************************************************************************
                       kstdatamanger_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of British Columbia
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
#ifndef KSTVIEWMANAGERI_H
#define KSTVIEWMANAGERI_H

class KstDoc;

#include "viewmanager.h"
#include "kst2dplot.h"
#include "kstplotgroup.h"
#include "kstviewobject.h"
#include "kstviewwindow.h"

class KstViewManagerI: public KstViewManager {
  Q_OBJECT
  public:
    KstViewManagerI(KstDoc *doc, QWidget* parent = 0,
        const char *name = 0,
        bool modal = false, WFlags fl = 0);
    virtual ~KstViewManagerI();

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

  private:
    KstDoc *doc;

  protected:

  signals:
    void docChanged();
};

class KstViewObjectItem : public QObject, public QListViewItem {
  Q_OBJECT
  public:
    KstViewObjectItem(QListView *parent, KstTopLevelViewPtr x, KstViewManagerI *dm, int localUseCount = 0);
    KstViewObjectItem(QListViewItem *parent, Kst2DPlotPtr x, KstViewManagerI *dm, int localUseCount = 0);
    KstViewObjectItem(QListViewItem *parent, KstPlotGroupPtr x, KstViewManagerI *dm, int localUseCount = 0);
    KstViewObjectItem(QListViewItem *parent, KstViewObjectPtr x, KstViewManagerI *dm, int localUseCount = 0);
    virtual ~KstViewObjectItem();

    void update(KstViewObjectPtr x, bool recursive = true, int localUseCount = 0);
    virtual int rtti() const { return _rtti; }

    const QString& tagName() const { return _name; }
//    KstViewObjectPtr viewObject() { return *KST::dataObjectList.findTag(_name); }
    bool removable() const { return _removable; }
    void updateButtons();

  public slots:

  signals:
    void updated();

  private:
    int _rtti;
    QString _name;
    KstViewManagerI *_vm;
    bool _removable;
    bool _inUse;
};

#endif
// vim: ts=2 sw=2 et
