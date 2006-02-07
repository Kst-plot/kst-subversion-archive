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

class KstDoc;

#include "kst.h"
#include "kstdatacollection.h"
#include "kstdataobject.h"
#include "kstfilteredvector.h"
#include "kstrvector.h"
#include "datamanager.h"
#include <qlistview.h>

class KstDataManagerI: public KstDataManager {
  Q_OBJECT
public:
  KstDataManagerI(KstDoc *doc, QWidget* parent=0,
                  const char* name = 0,
                  bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstDataManagerI();

public slots:
  void update();
  void show_I();
  void edit_I();
  void filter_I();
  void delete_I();

private slots:
  void doUpdates();
  void contextMenu(QListViewItem *i, const QPoint& p, int c);
  void currentChanged(QListViewItem *);

private:
  KstDoc *doc;
signals:
  void editDataVector(const QString &);
};

class KstObjectItem : public QObject, public QListViewItem {
  Q_OBJECT
  public:
    KstObjectItem(QListView *parent, KstFilteredVectorPtr x, KstDataManagerI *dm);
    KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm);
    KstObjectItem(QListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm);
    KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm);
    virtual ~KstObjectItem();

    void update(bool recursive = true);
    virtual int rtti() const { return _rtti; }
    
    const QString& tagName() const { return _name; }
    KstDataObjectPtr dataObject() { return *KST::dataObjectList.findTag(_name); }
    bool removable() const { return _removable; }
    void updateButtons();

  public slots:
    void applyFilter(int);
    void addToPlot(int);
    void removeFromPlot(int);
    void makeCurve();
    void reload();

  signals:
    void updated();

  private:
    int              _rtti;
    QString          _name;
    KstDataManagerI *_dm;
    bool             _removable;
};

#endif
