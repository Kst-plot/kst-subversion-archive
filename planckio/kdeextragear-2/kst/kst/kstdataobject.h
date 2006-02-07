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

#include "kstvector.h"
#include "kstobject.h"

#include <qdom.h>
#include <ksharedptr.h>
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

  virtual void loadInputs();

  virtual int getUsage() const;

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

typedef KSharedPtr<KstDataObject> KstDataObjectPtr;
typedef KstObjectList<KstDataObjectPtr> KstDataObjectList;

#endif
// vim: ts=2 sw=2 et
