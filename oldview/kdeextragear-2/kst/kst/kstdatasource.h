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

#include <qstring.h>
#include <qtextstream.h>
#include <qdom.h>

#include "kstobject.h"


namespace KST {
  class Plugin;
}

class KstDataSourcePrivate;

// BINARY COMPATIBILITY IS NOT YET GUARANTEED
class KstDataSource : public KstObject {
protected:
  KstDataSource(const QString& filename, const QString& type);

public:
  virtual ~KstDataSource();

  /** Returns a list of plugins found on the system. */
  static QStringList pluginList();

  static KstSharedPtr<KstDataSource> loadSource(const QString& filename, const QString& type = QString::null);
  static KstSharedPtr<KstDataSource> loadSource(QDomElement& e);
 
  virtual bool isValid() const; // generally you don't need to change this
                                // returns _valid;

  /** Updates number of samples.
      For ascii files, it also reads and writes to a temporary binary file.
      It returns 1 if there was new data. */
  virtual KstObject::UpdateType update(int = -1);

  /** Reads a field from the file.  Data is returned in the
      double Array v[] */
  virtual int readField(double *v, const QString& field, int s, int n);

  /** Returns true if the field is valid, or false if it is not */
  virtual bool isValidField(const QString& field) const;

  /** Returns samples per frame for field <field>.  For ascii column data,
      this is always 1.  For frame data this could greater than 1. */
  virtual int samplesPerFrame(const QString& field);

  /** Returns the size of the file (in frames) as of last update */
  virtual int frameCount() const;

  /** Returns the file name. It is updated each time the fn is called. */
  virtual QString fileName() const;

  virtual QStringList fieldList() const;

  /** Returns the file type or an error message in a static string
      The string is stored in a separate static variable, so changes
      to this are ignored.  It is updated each time the fn is called */
  virtual QString fileType() const;

  /** Save file description info into stream ts.
      Remember to call the base class if you reimplement this. */
  virtual void save(QTextStream &ts);

  const QString& sourceName() const { return _source; }

  virtual void *bufferMalloc(size_t size);
  virtual void bufferFree(void *ptr);
  virtual void *bufferRealloc(void *ptr, size_t size);

protected:
  virtual void virtual_hook(int id, void *data);

  /** Is the object valid? */
  bool _valid;

  /** Place to store the list of fields.  Base implementation returns this. */
  QStringList _fieldList;

  /** The filename.  Populated by the base class constructor.  */
  QString _filename;

  friend class KST::Plugin;

  /** The source type name. */
  QString _source;

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
};


#endif
// vim: ts=2 sw=2 et
