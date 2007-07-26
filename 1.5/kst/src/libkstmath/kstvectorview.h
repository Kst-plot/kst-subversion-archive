/***************************************************************************
                          kstvectorview.h: Curve subsets for KST
                             -------------------
    begin                : Wed July 11 2002
    copyright            : (C) 2007 by D. Hanson
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

#ifndef KSTVECTORVIEW_H
#define KSTVECTORVIEW_H

#include "kstbasecurve.h"
#include "kst_export.h"

class KST_EXPORT KstVectorView: public KstDataObject {
  Q_OBJECT

  public:
    // InterpType order must match the order of entries of _interp in 
    // vectorviewdialogwidget.ui
    enum InterpType {
      InterpY = 0,
      InterpX = 1,
      InterpMax = 2,
      InterpMin = 3
    };

    KstVectorView(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
                  KstVectorView::InterpType itype,
                  bool useXmin, KstScalarPtr xmin,
                  bool useXmax, KstScalarPtr xmax,
                  bool useYmin, KstScalarPtr ymin,
                  bool useYmax, KstScalarPtr ymax, KstVectorPtr flag);
    KstVectorView(const QDomElement &e);
    virtual ~KstVectorView();

    KstVectorView::InterpType interp() const;
    void setInterp(KstVectorView::InterpType itype);

    virtual UpdateType update(int update_counter = -1);
    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual QString propertyString() const;

    void setXminScalar(KstScalarPtr xmin);
    void setXmaxScalar(KstScalarPtr xmax);
    void setYminScalar(KstScalarPtr ymin);
    void setYmaxScalar(KstScalarPtr ymax);

    void setUseXmin(bool useXmin);
    void setUseXmax(bool useXmax);
    void setUseYmin(bool useYmin);
    void setUseYmax(bool useYmax);

    KstScalarPtr xMinScalar() {return _xmin;}
    KstScalarPtr xMaxScalar() {return _xmax;}
    KstScalarPtr yMinScalar() {return _ymin;}
    KstScalarPtr yMaxScalar() {return _ymax;}

    bool useXmin() {return _useXmin;}
    bool useXmax() {return _useXmax;}
    bool useYmin() {return _useYmin;}
    bool useYmax() {return _useYmax;}

    void setXVector(KstVectorPtr new_v);
    void setYVector(KstVectorPtr new_v);

    void setFlagVector(KstVectorPtr Flag);
    bool hasFlag();

    virtual QString xLabel() const;
    virtual QString yLabel() const;

    virtual void showNewDialog();
    virtual void showEditDialog();

    virtual bool slaveVectorsUsed() const;

    QString in_xVTag() const;
    QString in_yVTag() const;

    QString FlagTag() const;

    virtual QString xVTag() const {return (*_cxVector)->tagName();} //?
    virtual QString yVTag() const {return (*_cyVector)->tagName();} //?

    KstVectorPtr vX() const { return *_cxVector; }
    KstVectorPtr vY() const { return *_cyVector; }

    double vMax() const;
    double vMin() const;
    int vNumSamples() const;

    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);

  protected slots:
    void scalarChanged();

  private:
    KstVectorMap::Iterator _cxVector;
    KstVectorMap::Iterator _cyVector;

    InterpType _interp;
    bool _useXmin, _useXmax, _useYmin, _useYmax;
    KstScalarPtr _xmin, _xmax, _ymin, _ymax;

    void commonConstructor(const QString &in_tag);
};

typedef KstSharedPtr<KstVectorView> KstVectorViewPtr;
typedef KstObjectList<KstVectorViewPtr> KstVectorViewList;

#endif
// vim: ts=2 sw=2 et
