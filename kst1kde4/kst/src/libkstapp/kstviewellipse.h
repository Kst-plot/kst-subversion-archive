/***************************************************************************
                              kstviewellipse.h
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

#ifndef KSTVIEWELLIPSE_H
#define KSTVIEWELLIPSE_H

#include "kstviewobject.h"

class KstViewEllipse;
typedef QExplicitlySharedDataPointer<KstViewEllipse> KstViewEllipsePtr;

class KstViewEllipse : public KstViewObject {
  Q_OBJECT
  Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
  Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
  Q_PROPERTY(QColor fillColor READ foregroundColor WRITE setForegroundColor)
  Q_PROPERTY(bool transparentFill READ transparent WRITE setTransparent)
  public:
    KstViewEllipse();
    KstViewEllipse(const QDomElement& e);
    KstViewEllipse(const KstViewEllipse& ellipse);
    virtual ~KstViewEllipse();

    virtual void invalidateClipRegion();
    virtual QRegion clipRegion();
    virtual KstViewObject* copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
    virtual KstViewObject* copyObjectQuietly() const;
    virtual void setBorderColor(const QColor& to);
    virtual QColor borderColor() const;
    virtual void setBorderWidth(int width);
    virtual int borderWidth() const;
    virtual void drawShadow(KstPainter& p, const QPoint& pos);
    virtual void paintSelf(KstPainter& p, const QRegion& bounds);
    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    virtual QMap<QString, QVariant> widgetHints(const QString& propertyName) const; 
    virtual signed int directionFor(const QPoint& pos);
    virtual QRegion region();
    virtual bool transparent() const;
    virtual void setTransparent(bool transparent);
    virtual bool complexObject() const;

    // can't have Q_PROPERTY in KstViewObject?
    virtual void setForegroundColor(const QColor& color);
    virtual QColor foregroundColor() const;


  private:
    int _borderWidth;
    QColor _borderColor;
};

typedef KstObjectList<KstViewEllipsePtr> KstViewEllipseList;

#endif

