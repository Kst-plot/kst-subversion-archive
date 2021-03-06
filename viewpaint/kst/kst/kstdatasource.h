/***************************************************************************
                     kstdatasource.h  -  abstract data source
                             -------------------
    begin                : Thu Oct 16 2003
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

#ifndef KSTDATASOURCE_H
#define KSTDATASOURCE_H

#include <qdom.h>
#include <qguardedptr.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qwidget.h>

#include <kconfig.h>

#include "kstdateparser.h"
#include "kstobject.h"
#include "kst_export.h"

#define KST_CURRENT_DATASOURCE_KEY 0x00000003

#define KST_KEY_DATASOURCE_PLUGIN(x) extern "C" Q_UINT32 key_##x() { return KST_CURRENT_DATASOURCE_KEY; }

namespace KST {
  class Plugin;
}

struct KstMatrixData {
  double xMin;
  double yMin;
  double xStepSize;
  double yStepSize;
  double *z; // the data
};

class KstScalar;
class KstDataSourceConfigWidget;
typedef KstSharedPtr<KstScalar> KstScalarPtr;

class KST_EXPORT KstDataSource : public KstObject {
  protected:
    KstDataSource(KConfig *cfg, const QString& filename, const QString& type);
    friend class KstApp;
    static void setupOnStartup(KConfig*);
    static void cleanupForExit();

  public:
    virtual ~KstDataSource();

    // These six static methods are not for plugins to use
    /** Returns a list of plugins found on the system. */
    static QStringList pluginList();

    static KstSharedPtr<KstDataSource> loadSource(const QString& filename, const QString& type = QString::null);
    static KstSharedPtr<KstDataSource> loadSource(QDomElement& e);
    static QStringList fieldListForSource(const QString& filename, const QString& type = QString::null, QString *outType = 0L, bool *complete = 0L);
    static QStringList matrixListForSource(const QString& filename, const QString& type = QString::null, QString *outType = 0L, bool *complete = 0L);
    static KstDataSourceConfigWidget *configWidgetForSource(const QString& filename, const QString& type);
    static KstDataSourceConfigWidget *configWidgetForPlugin(const QString& plugin);
    // @since 1.1.0
    static bool pluginHasConfigWidget(const QString& plugin);
    // @since 1.1.0
    static bool supportsTime(const QString& plugin, const QString& type = QString::null);


    KstDataSourceConfigWidget *configWidget() const;

    // @since 1.1.0
    bool reusable() const;
    // @since 1.1.0
    void disableReuse();

    virtual bool isValid() const; // generally you don't need to change this
    // returns _valid;

    /** Updates number of samples.
      For ascii files, it also reads and writes to a temporary binary file.
      It returns 1 if there was new data. */
    virtual KstObject::UpdateType update(int = -1);

    /** Reads a field from the file.  Data is returned in the
      double Array v[]
      s is the starting frame
      n is the number of frames to read
      if n is -1, it means to read 1 -sample- from frame s.
      return the number of -samples- read.
     */
    virtual int readField(double *v, const QString& field, int s, int n);

    /** Reads a field from the file.  Data is returned in the
      double Array v[].  Will skip according to the parameter, but it may not
      be implemented.  If it returns -9999, use the non-skip version instead. */
    virtual int readField(double *v, const QString& field, int s, int n, int skip, int *lastFrameRead = 0L);
    
    /** Read the specified sub-range of the matrix, flat-packed in z in row-major order
        xStart - starting x *frame*
        yStart - starting y *frame*
        xNumSteps - number of *frames* to read in x direction; -1 to read 1 *sample* from xStart
        yNumSteps - number of *frames* to read in y direction; -1 to read 1 *sample* from yStart
        Will skip according to the parameter, but it may not be implemented.  If return value is -9999, 
        use the non-skip version instead.
        The suggested scaling and translation is returned in xMin, yMin, xStepSize, and yStepSize
        Returns the number of *samples* read **/
    virtual int readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps, int skip);
    
    /** Read the specified sub-range of the matrix, flat-packed in z in row-major order (non-skipping)
        xStart - starting x *frame*
        yStart - starting y *frame*
        xNumSteps - number of *frames* to read in x direction; -1 to read 1 *sample* from xStart
        yNumSteps - number of *frames* to read in y direction; -1 to read 1 *sample* from yStart
        The suggested scaling and translation is returned in xMin, yMin, xStepSize, and yStepSize
        Returns the number of *samples* read **/
    virtual int readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps);
    
    /** Return the current dimensions of the matrix: xDim*yDim <= total frames **/
    virtual bool matrixDimensions(const QString& matrix, int* xDim, int* yDim);
    
    /** Returns the list of fields that support readMatrix **/
    virtual QStringList matrixList() const;

    /** Returns true if the field is valid, or false if it is not */
    virtual bool isValidField(const QString& field) const;
    
    /** Returns true if the matrix is valid, or false if it is not */
    virtual bool isValidMatrix(const QString& field) const;

    /** Returns samples per frame for field <field>.  For ascii column data,
      this is always 1.  For frame data this could greater than 1. */
    virtual int samplesPerFrame(const QString& field);

    /** Returns the size of the file (in frames) as of last update.
      Field is optional, but you might not get back what you expect if you
      don't provide it as the data source may have different frame counts
      for different fields.  When implementing this, field may be ignored. */
    virtual int frameCount(const QString& field = QString::null) const;

    /** Returns the file name. It is updated each time the fn is called. */
    virtual QString fileName() const;

    virtual QStringList fieldList() const;

    /** Returns the file type or an error message in a static string
      The string is stored in a separate static variable, so changes
      to this are ignored.  It is updated each time the fn is called */
    virtual QString fileType() const;

    /** Save file description info into stream ts.
      Remember to call the base class if you reimplement this. */
    virtual void save(QTextStream &ts, const QString& indent = QString::null);

    const QString& sourceName() const { return _source; }

    virtual void *bufferMalloc(size_t size);
    virtual void bufferFree(void *ptr);
    virtual void *bufferRealloc(void *ptr, size_t size);

    /** Returns true if the field list is complete, therefore the user should
      not be able to edit the field combobox.  Default is true. */
    virtual bool fieldListIsComplete() const;

    /** Returns true if this file is empty */
    virtual bool isEmpty() const;

    /** Reset to initial state of the source, just as though no data had been
     *  read and the file had just been opened.  Return true on success.
     */
    virtual bool reset();

    virtual const QMap<QString, QString>& metaData() const;

    virtual const QString& metaData(const QString& key) const;

    virtual bool hasMetaData() const;

    virtual bool hasMetaData(const QString& key) const;

    /** Does it support time conversion of sample numbers, in general? */
    virtual bool supportsTimeConversions() const;

    virtual int sampleForTime(const KST::ExtDateTime& time, bool *ok = 0L);

    virtual int sampleForTime(double milliseconds, bool *ok = 0L);

    virtual KST::ExtDateTime timeForSample(int sample, bool *ok = 0L);

    // in (ms)
    virtual double relativeTimeForSample(int sample, bool *ok = 0L);

  protected:
    void updateNumFramesScalar();

    /** Is the object valid? */
    bool _valid;

    bool _reusable;

    /** Place to store the list of fields.  Base implementation returns this. */
    QStringList _fieldList;
    
    /** Place to store the list of matrices.  Base implementation returns this. */
    mutable QStringList _matrixList;

    /** The filename.  Populated by the base class constructor.  */
    QString _filename;

    friend class KST::Plugin;

    /** The source type name. */
    QString _source;

    QMap<QString, QString> _metaData;

    KConfig *_cfg;

    KstScalarPtr _numFramesScalar;

    // NOTE: You must bump the version key if you add new member variables
    //       or change or add virtual functions.
};


typedef KstSharedPtr<KstDataSource> KstDataSourcePtr;

class KstDataSourceList : public KstObjectList<KstDataSourcePtr> {
  public:
    KstDataSourceList() : KstObjectList<KstDataSourcePtr>() {}
    KstDataSourceList(const KstDataSourceList& x) : KstObjectList<KstDataSourcePtr>(x) {}
    virtual ~KstDataSourceList() {}

    virtual KstDataSourceList::Iterator findFileName(const QString& x) {
      for (KstDataSourceList::Iterator it = begin(); it != end(); ++it) {
        if ((*it)->fileName() == x) {
          return it;
        }
      }
      return end();
    }

    // @since 1.1.0
    KstDataSourceList::Iterator findReusableFileName(const QString& x) {
      for (KstDataSourceList::Iterator it = begin(); it != end(); ++it) {
        if ((*it)->reusable() && (*it)->fileName() == x) {
          return it;
        }
      }
      return end();
    }

    // @since 1.1.0
    QStringList fileNames() const {
      QStringList rc;
      for (KstDataSourceList::ConstIterator it = begin(); it != end(); ++it) {
        rc << (*it)->fileName();
      }
      return rc;
    }

};


// @since 1.1.0
class KstDataSourceConfigWidget : public QWidget {
  Q_OBJECT
  friend class KstDataSource;
  public:
    KstDataSourceConfigWidget(); // will be reparented later
    virtual ~KstDataSourceConfigWidget();

    virtual void setConfig(KConfig*);

    KST_EXPORT void setInstance(KstDataSourcePtr inst);
    KST_EXPORT KstDataSourcePtr instance() const;

  public slots:
    virtual void load();
    virtual void save();

  protected:
    KConfig *_cfg;
    // If _instance is nonzero, then your settings are to be saved for this
    // particular instance of the source, as opposed to globally.
    KstDataSourcePtr _instance;
};

#endif
// vim: ts=2 sw=2 et
