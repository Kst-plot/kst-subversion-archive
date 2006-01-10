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

#include <qimage.h>

class QTimer;

class KstViewPicture;
typedef KstSharedPtr<KstViewPicture> KstViewPicturePtr;

class KstViewPicture : public KstBorderedViewObject {
  Q_OBJECT
  Q_PROPERTY(QString path READ url WRITE setImage)
  Q_PROPERTY(int refreshTimer READ refreshTimer WRITE setRefreshTimer)
  public:
    KstViewPicture();
    KstViewPicture(const QDomElement& e);
    ~KstViewPicture();

    bool setImage(const QString& url);
    void setImage(const QImage& image);
    const QString& url() const;
    const QImage& image() const;
    
    // resize picture to the image size
    void restoreSize();

    // 0 == no refresh (default)
    void setRefreshTimer(int seconds);
    int refreshTimer() const;
    
    virtual QMap<QString, QVariant> widgetHints(const QString& propertyName) const;

  public slots:
    void paint(KstPainter& p, const QRegion& bounds);

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
// vim: ts=2 sw=2 et
