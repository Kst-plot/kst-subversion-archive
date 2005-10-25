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

#include "datamanager.h"
#include "kstrvector.h"
#include "kstsvector.h"

class KstDataManagerI: public KstDataManager {
  Q_OBJECT
  public:
    KstDataManagerI(KstDoc *doc, QWidget* parent = 0,
        const char *name = 0,
        bool modal = false, WFlags fl = 0);
    virtual ~KstDataManagerI();

  public slots:
    void update();
    void updateContents();
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
    void editStaticVector(const QString &);
    void docChanged();
};

class KstObjectItem : public QObject, public QListViewItem {
  Q_OBJECT
  public:
    KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstSVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm, int localUseCount = 0);
    KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm, int localUseCount = 0);
    virtual ~KstObjectItem();

    void update(bool recursive = true, int localUseCount = 0);
    virtual int rtti() const { return _rtti; }

    const QString& tagName() const { return _name; }
    KstDataObjectPtr dataObject() { return *KST::dataObjectList.findTag(_name); }
    bool removable() const { return _removable; }
    void updateButtons();

  public slots:
    void applyFilter(int);
    void addToPlot(int);
    void activateHint(int);
    void addImageToPlot(int);
    void removeFromPlot(int);
    void removeImageFromPlot(int);
    void makeCurve();
    void makeHistogram();
    void makePSD();
    void makeMatrix();
    void makeImage();
    void reload();

  signals:
    void updated();

  private:
    int              _rtti;
    QString          _name;
    KstDataManagerI *_dm;
    bool             _removable;
    void paintPlot(Kst2DPlotPtr p); //used by add to/remove from plot slots
};

#endif
