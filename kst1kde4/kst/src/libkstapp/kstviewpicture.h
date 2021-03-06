/***************************************************************************
                              kstviewpicture.h
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

#ifndef KSTVIEWPICTURE_H
#define KSTVIEWPICTURE_H

#include "kstborderedviewobject.h"

#include <QImage>

class QTimer;

class KstViewPicture;
typedef QExplicitlySharedDataPointer<KstViewPicture> KstViewPicturePtr;

class KstViewPicture : public KstBorderedViewObject {
  Q_OBJECT
  Q_PROPERTY(bool maintainAspect READ maintainAspect WRITE setMaintainAspect)
  Q_PROPERTY(QString path READ url WRITE setImage)
  Q_PROPERTY(int refreshTimer READ refreshTimer WRITE setRefreshTimer)

  public:
    KstViewPicture();
    KstViewPicture(const QDomElement& e);
    KstViewPicture(const KstViewPicture& picture);
    ~KstViewPicture();

    virtual KstViewObject* copyObjectQuietly(KstViewObject& parent, const QString& name = QString::null) const;
    virtual KstViewObject* copyObjectQuietly() const;
    bool setImage(const QString& url);
    void setImage(const QImage& image);
    const QString& url() const;
    const QImage& image() const;

    // resize picture to the image size
    void restoreSize();
    void restoreAspect();

    bool maintainAspect() const;
    void setMaintainAspect(bool maintain);

    // 0 == no refresh (default)
    void setRefreshTimer(int seconds);
    int refreshTimer() const;

    virtual QMap<QString, QVariant> widgetHints(const QString& propertyName) const;
    QRegion clipRegion();

    void paintSelf(KstPainter& p, const QRegion& bounds);
    bool transparent() const;

  protected slots:
    void doRefresh();

  public:
    void save(QTextStream& ts, const QString& indent = QString::null);

  private:
    QImage _image, _iCache;
    QString _url;
    int _refresh;
    QTimer *_timer;
};

typedef KstObjectList<KstViewPicturePtr> KstViewPictureList;

#endif
