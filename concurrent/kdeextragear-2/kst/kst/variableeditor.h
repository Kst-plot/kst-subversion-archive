/***************************************************************************
             variableditor.h  -  widget for editting lists of variables
                             -------------------
    begin                : Mon Jan 26 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef VAREDIT_H
#define VAREDIT_H

#include <qscrollview.h>
#include <qstringlist.h>

class QSpacerItem;
class QVBoxLayout;

class VariableEditor : public QScrollView {
  Q_OBJECT
  public:
    VariableEditor(QWidget *parent = 0L, const char *name = 0L);
    virtual ~VariableEditor();

    bool addVariable(const QString& var, const QString& val);
    bool removeVariable(const QString& var);
    QString variable(const QString& var) const;

    const QStringList& variables() const { return _variables; }

  private slots:
    void changed();

  signals:
    void variableModified(const QString& name);

  private:
    QStringList _variables;
    QVBoxLayout *_layout;
    QSpacerItem *_spacer;
};


#endif
// vim: ts=2 sw=2 et
