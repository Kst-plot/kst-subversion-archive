/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILEREQUESTER_H
#define FILEREQUESTER_H

#include <QWidget>
#include <QFileDialog>

#include "kst_export.h"

class QLineEdit;
class QToolButton;

namespace Kst {

class KST_EXPORT FileRequester : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QString file READ file WRITE setFile USER true)
  public:
    FileRequester(QWidget *parent = 0);
    FileRequester(const QString &file, QWidget *parent = 0);
    virtual ~FileRequester();

    QString file() const;
    void setMode(QFileDialog::FileMode mode) { _mode = mode; }

  public Q_SLOTS:
    void setFile(const QString &file);

  Q_SIGNALS:
    void changed(const QString &file);

  private Q_SLOTS:
    void chooseFile();

  private:
    void setup();

    QLineEdit *_fileEdit;
    QToolButton *_fileButton;
    QString _file;
    QFileDialog::FileMode _mode;
};

}

#endif

// vim: ts=2 sw=2 et