/***************************************************************************
                           kstborderedviewobject.h
                             -------------------
    begin                : Mar 11, 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef KSTBORDEREDVIEWOBJECT_H
#define KSTBORDEREDVIEWOBJECT_H

#include "kstviewobject.h"


/***************************************************************************

 (position().x(), position().y())
      +-------------------------------------------------------+  ---
      |                   Margin                              |   ^
      |   +---------------------------- Border -----------+   |   |
      |   |            Padding                            |   |   |
      |   |                                               |   |   |
      |   |     +-----------------------------------+     |   |   |
      |   |     |        Object contents            |     |   |   |
      |   |     |                                   |     |   |  Height
      |   |     |                                   |     |   |   |
      |   |     |                                   |     |   |   |
      |   |     |                                   |     |   |   |
      |   |     +-----------------------------------+     |   |   |
      |   |                                               |   |   |
      |   |                                               |   |   |
      |   +-----------------------------------------------+   |   |
      |                                                       |   V
      +-------------------------------------------------------+  ---

      |<----------------------- Width ----------------------->|


Contents Rect
-------------
object.width = width() - 2*(margin() + padding() + borderWidth());
object.height = height() - 2*(margin() + padding() + borderWidth());
object.x = position().x() + margin() + padding() + borderWidth();
object.y = position().y() + margin() + padding() + borderWidth();

***************************************************************************/

class KstBorderedViewObject;
typedef KstSharedPtr<KstBorderedViewObject> KstBorderedViewObjectPtr;

class KstBorderedViewObject : public KstViewObject {
  Q_OBJECT
  protected:
    KstBorderedViewObject(const QString& type);
  public:
    KstBorderedViewObject(const QDomElement& e);
    KstBorderedViewObject(const KstBorderedViewObject& borderedViewObject);
    virtual ~KstBorderedViewObject();

    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    virtual void saveTag(QTextStream& ts, const QString& indent = QString::null);

    void setBorderColor(const QColor& c);
    const QColor& borderColor() const;

    void setBorderWidth(int w);
    int borderWidth() const;

    void setMargin(int w);
    int margin() const;

    void setPadding(int p);
    int padding() const;

    // See above for gross details
    virtual QRect contentsRect() const;
    virtual void setContentsRect(QRect& rect);

  public slots:
    virtual void paint(KstPaintType type, QPainter& p);

  protected:
    virtual void readBinary(QDataStream& str);
    virtual void writeBinary(QDataStream& str);

  private:
    QColor _borderColor;
    int _borderWidth;
    int _padding, _margin;
};

typedef KstObjectList<KstBorderedViewObjectPtr> KstBorderedViewObjectList;


#endif
// vim: ts=2 sw=2 et
