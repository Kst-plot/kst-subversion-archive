/***************************************************************************
                               kstviewarrow.h
                             -------------------
    begin                : Jun 14, 2005
    copyright            : (C) 2005 The University of Toronto
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

#ifndef KSTVIEWARROW_H
#define KSTVIEWARROW_H

#include "kstviewline.h"
#include <qglobal.h>

class KstViewArrow;
typedef QExplicitlySharedDataPointer<KstViewArrow> KstViewArrowPtr;

class KstViewArrow : public KstViewLine {
  Q_OBJECT
  Q_PROPERTY(bool hasFromArrow READ hasFromArrow WRITE setHasFromArrow)
  Q_PROPERTY(bool hasToArrow READ hasToArrow WRITE setHasToArrow)
  Q_PROPERTY(double fromArrowScaling READ fromArrowScaling WRITE setFromArrowScaling)
  Q_PROPERTY(double toArrowScaling READ toArrowScaling WRITE setToArrowScaling)

  public:
    KstViewArrow();
    KstViewArrow(const QDomElement& e);
    KstViewArrow(const KstViewArrow& arrow);
    ~KstViewArrow();

    QMap<QString, QVariant > widgetHints(const QString& propertyName) const;

    virtual KstViewObject* copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
    virtual KstViewObject* copyObjectQuietly() const;

    void paintSelf(KstPainter& p, const QRegion& bounds);
    void paintArrow(KstPainter& p, const QPoint& to, const QPoint &from, int w, double scaling);
    bool hasArrow() const;
    bool hasFromArrow() const;
    void setHasFromArrow(bool yes);
    bool hasToArrow() const;
    void setHasToArrow(bool yes);
    double fromArrowScaling() const;
    void setFromArrowScaling(double scaling);
    double toArrowScaling() const;
    void setToArrowScaling(double scaling);
    QRegion clipRegion();
    virtual QRect surroundingGeometry() const;

  public:
    void save(QTextStream& ts, const QString& indent = QString::null);

  private:
    bool _hasFromArrow;
    bool _hasToArrow;
    double _fromArrowScaling;
    double _toArrowScaling;
};

typedef KstObjectList<KstViewArrowPtr> KstViewArrowList;

#endif
