/***************************************************************************
                   kstdataobject.h: base class for data objects
                             -------------------
    begin                : May 20, 2003
    copyright            : (C) 2003 by C. Barth Netterfield
                           (C) 2003 The University of Toronto
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

#ifndef KSTDATAOBJECT_H
#define KSTDATAOBJECT_H

#include "kstobject.h"
#include "kstvector.h"

#include <qdom.h>
#include "kstsharedptr.h"
#include <qvaluelist.h>


class KstDataObject : public KstObject {
Q_OBJECT
public:
  KstDataObject();
  KstDataObject(QDomElement& e);
  virtual ~KstDataObject();

  virtual UpdateType update(int updateCounter = -1) = 0;
  virtual const QString& typeString() const { return _typeString; }
  virtual QString propertyString() const = 0;

  virtual int sampleCount() const { return 0; }

  // If you use these, you must lock() and unlock() the object as long as you
  // hold the reference
  const KstVectorMap& inputVectors()  const { return _inputVectors;  }
  const KstVectorMap& outputVectors() const { return _outputVectors; }
        KstVectorMap& inputVectors()        { return _inputVectors;  }
        KstVectorMap& outputVectors()       { return _outputVectors; }

  const KstScalarMap& inputScalars()  const { return _inputScalars;  }
  const KstScalarMap& outputScalars() const { return _outputScalars; }
        KstScalarMap& inputScalars()        { return _inputScalars;  }
        KstScalarMap& outputScalars()       { return _outputScalars; }

  virtual void save(QTextStream& ts);

  void showDialog();

  virtual bool loadInputs();

  virtual int getUsage() const;

  virtual void readLock() const;
  virtual void readUnlock() const;
  virtual void writeLock() const;
  virtual void writeUnlock() const;

protected slots:
  virtual void _showDialog() = 0;

protected:
  double *vectorRealloced(KstVectorPtr v, double *memptr, int newSize) const;

  KstVectorMap _inputVectors;
  KstVectorMap _outputVectors;
  KstScalarMap _inputScalars;
  KstScalarMap _outputScalars;
  QString _typeString;

  QValueList<QPair<QString,QString> > _inputVectorLoadQueue;
  QValueList<QPair<QString,QString> > _inputScalarLoadQueue;
};

typedef KstSharedPtr<KstDataObject> KstDataObjectPtr;
typedef KstObjectList<KstDataObjectPtr> KstDataObjectList;


#endif
// vim: ts=2 sw=2 et
