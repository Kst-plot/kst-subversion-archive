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

#include "kstcurvehint.h"
#include "kststring.h"
#include "kstvector.h"
#include "kstmatrix.h"
#include "kst_export.h"

typedef KstSharedPtr<KstDataObject> KstDataObjectPtr;
typedef KstObjectList<KstDataObjectPtr> KstDataObjectList;
typedef QMap<KstDataObjectPtr, KstDataObjectPtr> KstDataObjectDataObjectMap;

class KST_EXPORT KstDataObject : public KstObject {
  Q_OBJECT
  public:
    KstDataObject();
    KstDataObject(const QDomElement& e);
    KstDataObject(const KstDataObject& object);
    virtual ~KstDataObject();

    virtual UpdateType update(int updateCounter = -1) = 0;
    virtual const QString& typeString() const { return _typeString; }
    virtual QString propertyString() const = 0;
    virtual const QString& type() const { return _type; }

    virtual int sampleCount() const { return 0; }

    // If you use these, you must lock() and unlock() the object as long as you
    // hold the reference
    const KstVectorMap& inputVectors()  const { return _inputVectors;  }
    const KstVectorMap& outputVectors() const { return _outputVectors; }
    KstVectorMap& inputVectors() { return _inputVectors;  }
    KstVectorMap& outputVectors() { return _outputVectors; }

    const KstScalarMap& inputScalars()  const { return _inputScalars;  }
    const KstScalarMap& outputScalars() const { return _outputScalars; }
    KstScalarMap& inputScalars() { return _inputScalars;  }
    KstScalarMap& outputScalars() { return _outputScalars; }

    const KstStringMap& inputStrings()  const { return _inputStrings;  }
    const KstStringMap& outputStrings() const { return _outputStrings; }
    KstStringMap& inputStrings() { return _inputStrings;  }
    KstStringMap& outputStrings() { return _outputStrings; }
    
    const KstMatrixMap& inputMatrices() const { return _inputMatrices; }
    const KstMatrixMap& outputMatrices() const { return _outputMatrices; }
    KstMatrixMap& inputMatrices() { return _inputMatrices; }
    KstMatrixMap& outputMatrices() { return _outputMatrices; }

    virtual void save(QTextStream& ts, const QString& indent = QString::null);

    void showDialog();

    virtual bool loadInputs();

    virtual int getUsage() const;

    virtual void readLock() const;
    virtual void writeLock() const;
    virtual void unlock() const;

    virtual bool isValid() const;

    virtual const KstCurveHintList* curveHints() const;
    
    virtual bool deleteDependents();
    
    bool duplicateDependents(KstDataObjectDataObjectMap& duplicatedMap);
    
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) = 0;
    
    virtual void replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject);
    virtual void replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector);
    virtual void replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix);

    virtual bool uses(KstObjectPtr p) const;

  protected slots:
    virtual void _showDialog() = 0;

  protected:
    
    double *vectorRealloced(KstVectorPtr v, double *memptr, int newSize) const;

    KstVectorMap _inputVectors;
    KstVectorMap _outputVectors;
    KstScalarMap _inputScalars;
    KstScalarMap _outputScalars;
    KstStringMap _inputStrings;
    KstStringMap _outputStrings;
    KstMatrixMap _inputMatrices;
    KstMatrixMap _outputMatrices;

    QString _typeString, _type;

    bool _isInputLoaded;
    QValueList<QPair<QString,QString> > _inputVectorLoadQueue;
    QValueList<QPair<QString,QString> > _inputScalarLoadQueue;
    QValueList<QPair<QString,QString> > _inputStringLoadQueue;
    QValueList<QPair<QString,QString> > _inputMatrixLoadQueue;
    KstCurveHintList *_curveHints;
};


#endif
// vim: ts=2 sw=2 et
