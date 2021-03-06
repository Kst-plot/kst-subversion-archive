/***************************************************************************
                dirfile.h  -  data source plugin for dirfiles
                             -------------------
    begin                : Tue Oct 21 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef DIRFILE_H
#define DIRFILE_H

#include <datasource.h>
#include <dataplugin.h>

class DirFileSource : public Kst::DataSource {
  Q_OBJECT

  public:
    DirFileSource(Kst::ObjectStore *store, QSettings *cfg, const QString& filename, const QString& type, const QDomElement& e);

    ~DirFileSource();

    bool init();

    Kst::Object::UpdateType update(int = -1);

    int readField(double *v, const QString &field, int s, int n);

    int writeField(const double *v, const QString &field, int s, int n);

    bool isValidField(const QString &field) const;

    int samplesPerFrame(const QString &field);

    int frameCount(const QString& field = QString::null) const;

    QString fileType() const;

    void save(QXmlStreamWriter &streamWriter);

    bool isEmpty() const;

    bool reset();

    class Config;

  private:
    int _frameCount;
    int *_rowIndex;
    int _numLinesAlloc;
    int _numFrames;
    int _byteLength;
    mutable QStringList _fields;
    mutable Config *_config;
    char *_tmpBuf;
    uint _tmpBufSize;
    bool _haveHeader;
    mutable bool _fieldListComplete;
};


class DirFilePlugin : public QObject, public Kst::DataSourcePluginInterface {
    Q_OBJECT
    Q_INTERFACES(Kst::DataSourcePluginInterface)
  public:
    virtual ~DirFilePlugin() {}

    virtual QString pluginName() const;

    virtual bool hasConfigWidget() const { return false; }

    virtual Kst::DataSource *create(Kst::ObjectStore *store,
                                  QSettings *cfg,
                                  const QString &filename,
                                  const QString &type,
                                  const QDomElement &element) const;

    virtual QStringList matrixList(QSettings *cfg,
                                  const QString& filename,
                                  const QString& type,
                                  QString *typeSuggestion,
                                  bool *complete) const;

    virtual QStringList fieldList(QSettings *cfg,
                                  const QString& filename,
                                  const QString& type,
                                  QString *typeSuggestion,
                                  bool *complete) const;

    virtual int understands(QSettings *cfg, const QString& filename) const;

    virtual bool supportsTime(QSettings *cfg, const QString& filename) const;

    virtual QStringList provides() const;

    virtual Kst::DataSourceConfigWidget *configWidget(QSettings *cfg, const QString& filename) const;
};


#endif
// vim: ts=2 sw=2 et
