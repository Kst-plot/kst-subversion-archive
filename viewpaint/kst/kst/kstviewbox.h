/***************************************************************************
                                kstviewbox.h
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

#ifndef KSTVIEWBOX_H
#define KSTVIEWBOX_H

#include "kstborderedviewobject.h"

#include <kglobal.h>

class KstViewBox;
typedef KstSharedPtr<KstViewBox> KstViewBoxPtr;

class KstViewBox : public KstBorderedViewObject {
  Q_OBJECT
  Q_PROPERTY(int xRound READ xRound WRITE setXRound)
  Q_PROPERTY(int yRound READ yRound WRITE setYRound)
  Q_PROPERTY(bool transparentFill READ transparentFill WRITE setTransparentFill)
  public:
    KstViewBox();
    KstViewBox(const QDomElement& e);
    ~KstViewBox();

    void setXRound(int rnd);
    int xRound() const;
    void setYRound(int rnd);
    int yRound() const;
    void setCornerStyle(Qt::PenJoinStyle style);
    Qt::PenJoinStyle cornerStyle() const;
    
    bool transparentFill() const;
    void setTransparentFill(bool yes);

    void paintSelf(KstPainter& p, const QRegion& bounds);

  public:
    void save(QTextStream& ts, const QString& indent = QString::null);
    
    QMap<QString, QVariant > widgetHints(const QString& propertyName) const;

  private:
    int _xRound, _yRound;
    Qt::PenJoinStyle _cornerStyle;
    bool _transparentFill;
};

typedef KstObjectList<KstViewBoxPtr> KstViewBoxList;


#endif
// vim: ts=2 sw=2 et
