/***************************************************************************
                   linefitplugin.h
                             -------------------
    begin                : 09/08/06
    copyright            : (C) 2006 The University of Toronto
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
#ifndef LINEFITPLUGIN_H
#define LINEFITPLUGIN_H

#include <kstdataobject.h>

class LineFit : public KstDataObject {
  Q_OBJECT
public:

  LineFit(QObject *parent, const char *name, const QStringList &args);
  virtual ~LineFit();

  //algorithm
  void linefit();

  QString xArrayTag() const;
  QString yArrayTag() const;
  QString xInterpolatedTag() const;
  QString yInterpolatedTag() const;
  QString aTag() const;
  QString bTag() const;
  QString chi2Tag() const;

  KstVectorPtr xArray() const;
  KstVectorPtr yArray() const;
  KstVectorPtr xInterpolated() const;
  KstVectorPtr yInterpolated() const;
  KstScalarPtr a() const;
  KstScalarPtr b() const;
  KstScalarPtr chi2() const;

  void setXArray(KstVectorPtr new_xArray);
  void setYArray(KstVectorPtr new_yArray);
  void setXInterpolated(KstVectorPtr new_xInterpolated);
  void setYInterpolated(KstVectorPtr new_yInterpolated);
  void setA(KstScalarPtr new_a);
  void setB(KstScalarPtr new_b);
  void setChi2(KstScalarPtr new_chi2);

  //Pure virtual methods from KstDataObject
  virtual KstObject::UpdateType update(int updateCounter = -1);
  virtual QString propertyString() const;
  virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&);

  //Regular virtual methods from KstDataObject
//   virtual const QString& typeString() const { return _typeString; }
//   virtual const QString& type() const { return _type; }
//
//   virtual int sampleCount() const { return 0; }
//
  virtual void load(const QDomElement &e);
  virtual void save(QTextStream& ts, const QString& indent = QString::null);

//   virtual bool loadInputs();
//
//   virtual int getUsage() const;
//
//   virtual void readLock() const;
//   virtual void writeLock() const;
//   virtual void unlock() const;
//
  virtual bool isValid() const;
//
//   virtual const KstCurveHintList* curveHints() const;
//
//   virtual bool deleteDependents();
//
//   virtual void replaceDependency(KstDataObjectPtr oldObject, KstDataObjectPtr newObject);
//   virtual void replaceDependency(KstVectorPtr oldVector, KstVectorPtr newVector);
//   virtual void replaceDependency(KstMatrixPtr oldMatrix, KstMatrixPtr newMatrix);
//
//   virtual bool uses(KstObjectPtr p) const;

protected slots:
  //Pure virtual slots from KstDataObject
  virtual void showNewDialog();
  virtual void showEditDialog();

private:
  bool m_init;
};

typedef KstSharedPtr<LineFit> LineFitPtr;
typedef KstObjectList<LineFitPtr> LineFitList;

#endif
