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

#include <qdatetime.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qwidget.h>

#include <kconfig.h>

#include "kstobject.h"
#include "kst_export.h"

namespace KST {
  class Plugin;
}

class KstScalar;
class KstDataSourcePrivate;
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

    /** Returns true if the field is valid, or false if it is not */
    virtual bool isValidField(const QString& field) const;

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

    virtual int sampleForTime(const QDateTime& time, bool *ok = 0L);

    virtual int sampleForTime(int seconds, bool *ok = 0L);

    virtual QDateTime timeForSample(int sample, bool *ok = 0L);

    // in (s)
    virtual int relativeTimeForSample(int sample, bool *ok = 0L);

  protected:
    virtual void virtual_hook(int id, void *data);

    void updateNumFramesScalar();

    /** Is the object valid? */
    bool _valid;

    /** Place to store the list of fields.  Base implementation returns this. */
    QStringList _fieldList;

    /** The filename.  Populated by the base class constructor.  */
    QString _filename;

    friend class KST::Plugin;

    /** The source type name. */
    QString _source;

    QMap<QString, QString> _metaData;

    KConfig *_cfg;

    KstScalarPtr _numFramesScalar;

  private:
    KstDataSourcePrivate *d;

    // NOTE: In order to preserve binary compatibility with plugins, you must
    //       not add, remove, or change member variables or virtual functions.
    //       You must also not remove or change non-virtual functions.  You can
    //       add new variables to the KstDataSourcePrivate pointer if you need
    //       them.  You can simulate virtual functions with the virtual_hook().
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

    void setInstance(KstDataSourcePtr inst);
    KstDataSourcePtr instance() const;

  public slots:
    virtual void load();
    virtual void save();

  protected:
    KConfig *_cfg;
    // If _instance is nonzero, then your settings are to be saved for this
    // particular instance of the source, as opposed to globally.
    KstDataSourcePtr _instance;
    class Private;
    Private *d;
};

#endif
// vim: ts=2 sw=2 et
