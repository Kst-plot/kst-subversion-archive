/***************************************************************************
                                    js.h
                             -------------------
    begin                : Feb 09 2004
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

#ifndef JS_H
#define JS_H

#include <kstextension.h>
#include <kxmlguiclient.h>

namespace KJSEmbed {
  class KJSEmbedPart;
}

class KstJS : public KstExtension, public KXMLGUIClient {
  Q_OBJECT
  public:
    KstJS(QObject *parent, const char *name, const QStringList&);
    virtual ~KstJS();

    // To save state
    virtual void load(QDomElement& e);
    virtual void save(QTextStream& ts);

    KJSEmbed::KJSEmbedPart *part() const { return _jsPart; }

  public slots:
    void doShow();
    void loadScript();
    void createRegistry();
    void destroyRegistry();

  private:
    QStringList _scripts;
    KJSEmbed::KJSEmbedPart *_jsPart;
};


class KstJSUIBuilder : public QObject, public KXMLGUIClient {
  Q_OBJECT
  public:
    KstJSUIBuilder(const QString& ui, KstJS *js);
    virtual ~KstJSUIBuilder();

  public slots:
    void merge();

  private:
    KstJS *_js;
    QString _ui;
};


class KstUIMerge : public QObject {
  Q_OBJECT
  public:
    KstUIMerge(KstJS *parent, const char *name);
    virtual ~KstUIMerge();

  public slots:
    KXMLGUIClient *mergeUI(const QString& ui);

  private:
    KstJS *_parent;
};

#endif

// vim: ts=2 sw=2 et
