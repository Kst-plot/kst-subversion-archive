/***************************************************************************
                       ascii.h  -  ASCII data source
                             -------------------
    begin                : Fri Oct 17 2003
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

#ifndef ASCII_H
#define ASCII_H

#include <QFile>

#include <kstdatasource.h>

class AsciiSource : public KstDataSource {
  public:
    AsciiSource(QSettings *cfg, const QString& filename, const QString& type, const QDomElement& e = QDomElement());
    ~AsciiSource();

    static int readFullLine(QFile &file, QString &str);

    bool initRowIndex();
    KstObject::UpdateType update(int = -1);
    int readField(double *v, const QString &field, int s, int n);
    virtual int readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps);
    virtual QString configuration(QString setting);
    virtual bool setConfiguration(QString setting, const QString &value);
    bool matrixDimensions(const QString& matrix, int* xDim, int* yDim);
    bool isValidField(const QString &field) const;
    bool isValidMatrix(const QString &field) const;
    int samplesPerFrame(const QString &field);
    int frameCount(const QString& field = QString::null) const;
    QString fileType() const;
    QStringList fieldList() const;
    QStringList matrixList() const;

    void save(QTextStream &ts, const QString& indent = QString::null);
    bool isEmpty() const;
    bool supportsTimeConversions() const;
    int sampleForTime(double ms, bool *ok);
    int sampleForTime(const KST::ExtDateTime& time, bool *ok);
    bool fieldListIsComplete() const;
    bool reset();

    class Config;
    static QStringList fieldListFor(const QString& filename, Config *cfg);

  private:
    int *_rowIndex;
    int _numLinesAlloc;
    int _numFrames;
    int _byteLength;
    mutable QStringList _fields;
    friend class ConfigWidgetAscii;
    mutable Config *_config;
    char *_tmpBuf;
    uint _tmpBufSize;
    bool _haveHeader;
    mutable bool _fieldListComplete;
};


#endif
