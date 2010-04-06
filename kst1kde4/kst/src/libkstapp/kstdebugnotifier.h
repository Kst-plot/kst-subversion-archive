/***************************************************************************
                             kstdebugnotifier.h
                             -------------------
    begin                : Sep 13 2005
    copyright            : (C) 2005 The University of Toronto
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

#ifndef KSTDEBUGNOTIFIER_H
#define KSTDEBUGNOTIFIER_H

#include <QLabel>
#include <QPixmap>
#include <QVector>

class KstDebugNotifier : public QLabel {
  Q_OBJECT
  public:
    KstDebugNotifier(QWidget *parent);
    ~KstDebugNotifier();

  public slots:
    void close();
    void showDebugLog();
    void reanimate();

  private slots:
    void animate();

  protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

  private:
    int _animationStage;
    bool _gotPress;
    QVector<QPixmap> _pm;
};

#endif
