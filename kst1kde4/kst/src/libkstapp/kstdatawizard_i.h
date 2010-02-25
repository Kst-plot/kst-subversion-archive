/***************************************************************************
                       kstdatawizard_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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
#ifndef KSTDATAWIZARDI_H
#define KSTDATAWIZARDI_H

#include <kurlrequester.h>
#include <klineedit.h>
#include <kpushbutton.h>

#include "datawizard.h"
#include "kstcombobox.h"
#include "kstdatasource.h"
#include "vectorlistview.h"

class KstDataWizard: public DataWizard {
  Q_OBJECT

  public:
    KstDataWizard(QWidget* parent = 0, const char *name = 0, bool modal = false, WFlags fl = 0);
    virtual ~KstDataWizard();

  public:
    bool xVectorOk();
    bool yVectorsOk();
    double getFontSize( KstViewWindow *w );
    void showPage( QWidget *page );
    void saveSettings();
    void loadSettings();

  public slots:
    void selectFolder();
    void selectingFolder();
    void setInput( const QString &input );
    void plotColsChanged();
    void xChanged();
    void testURL();
    void sourceChanged( const QString &txt );
    void fieldListChanged();
    void updateWindowBox();
    void updateColumns();
    void updatePlotBox();
    void vectorSubset( const QString &filter );
    void newFilter();
    void finished();
    void applyFiltersChecked(bool on);
    void enableXEntries();
    void disableXEntries();
    void enablePSDEntries();
    void disablePSDEntries();
    void search();
    void disableWindowEntries();
    void enableWindowEntries();
    void markSourceAndSave();
    void configureSource();
    void clear();
    void down();
    void up();
    void updateVectorPageButtons();
    void add();
    void remove();
    void vectorsDroppedBack(QDropEvent *e);

  private:
    bool checkAvailableMemory(KstDataSourcePtr &ds, KstFrameSize f0Value, Q_INT64 nValue);
    void createLegendsAndLabels(KstViewObjectList &plots, bool xLabels, bool yLabels, bool titleLabel, bool legend, bool legendAuto, int fontSize);
    void cleanupWindowLayout(KstViewWindow *window);

    static const QString &defaultTag;
    QGuardedPtr<QWidget> _configWidget;
    KstDataSourceList _sourceCache;
    QMap<QString,QString> _countMap;
    QDict<QListViewItem> _fields;
    QString _file;
    bool _hierarchy;
    bool _inTest;

  protected:
};

#endif
