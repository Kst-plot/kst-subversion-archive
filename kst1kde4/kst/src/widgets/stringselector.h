/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STRINGSELECTOR_H
#define STRINGSELECTOR_H
 
#include <QWidget>
#include "ui_stringselector.h"
#include "kst_export.h"

class KST_EXPORT StringSelector : public QWidget, public Ui::StringSelector {
  Q_OBJECT
public:
  StringSelector(QWidget *parent = 0);
  virtual ~StringSelector(); 

  void allowNewStrings( bool allowed );
  void update();
  void createNewString();
  void selectString();
  void editString();
  void selectionWatcher( const QString & tag );
  void setSelection( const QString & tag );
  void setSelection( KstStringPtr s );
  QString selectedString();
  void allowDirectEntry( bool allowed );

Q_SIGNALS:

private Q_SLOTS:
};
 
#endif

