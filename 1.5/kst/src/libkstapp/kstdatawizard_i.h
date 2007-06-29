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

#include "datawizard.h"
#include "kurlrequester.h"
#include "klineedit.h"
#include "kpushbutton.h"
#include "vectorlistview.h"
#include "kstcombobox.h"

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
    static const QString &defaultTag;
    QString _file;
    bool _inTest;
    QGuardedPtr<QWidget> _configWidget;
    KstDataSourceList _sourceCache;
    QMap<QString,QString> _countMap;

  protected:
};

#endif
// vim: ts=2 sw=2 et
