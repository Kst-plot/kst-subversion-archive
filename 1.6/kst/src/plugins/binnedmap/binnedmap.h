/***************************************************************************
                   binnedmap.h
                             -------------------
    begin                : 04/13/07
    copyright            : (C) 2007 C. Barth Netterfield
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
#ifndef BINNEDMAP_H 
#define BINNEDMAP_H 

#include <kstdataobject.h>

class BinnedMap : public KstDataObject {
  Q_OBJECT
  public:
    BinnedMap(QObject *parent, const char *name, const QStringList &args);
    virtual ~BinnedMap();

    //algorithm
    void binnedmap();

    QString xTag() const;
    QString yTag() const;
    QString zTag() const;
    QString mapTag() const;
    QString hitsMapTag() const;

    KstVectorPtr X() const;
    KstVectorPtr Y() const;
    KstVectorPtr Z() const;
    KstMatrixPtr map() const;
    KstMatrixPtr hitsMap() const;

    void setX(KstVectorPtr new_x);
    void setY(KstVectorPtr new_y);
    void setZ(KstVectorPtr new_z);
    void setMap(const QString &name);
    void setHitsMap(const QString &name);

    //Pure virtual methods from KstDataObject
    virtual KstObject::UpdateType update(int updateCounter = -1);
    virtual QString propertyString() const;
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&);

    //Regular virtual methods from KstDataObject
    virtual void load(const QDomElement &e);
    virtual void save(QTextStream& ts, const QString& indent = QString::null);

    void setXMin(double xmin);
    void setXMax(double xmax);
    void setYMin(double ymin);
    void setYMax(double ymax);

    double xMin() const;
    double xMax() const;
    double yMin() const;
    double yMax() const;

    void setNX(int nx);
    void setNY(int ny);
    int nX() const;
    int nY() const;

    static void AutoSize(KstVectorPtr x, KstVectorPtr y, int *nx, double *minx, double *maxx, int *ny, double *miny, double *maxy);

    void setAutoBin(bool);
    bool autoBin() const;

  protected slots:
    //Pure virtual slots from KstDataObject
    virtual void showNewDialog();
    virtual void showEditDialog();
  
  private:
    double _xMin;
    double _xMax;
    double _yMin;
    double _yMax;
    int _nx;
    int _ny;
    bool _autoBin;
};

typedef KstSharedPtr<BinnedMap> BinnedMapPtr;
typedef KstObjectList<BinnedMapPtr> BinnedMapList;

#endif
