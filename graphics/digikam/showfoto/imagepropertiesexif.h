/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ============================================================ */

#ifndef IMAGEPROPERTIESEXIF_H
#define IMAGEPROPERTIESEXIF_H

// Qt includes.

#include <qobject.h>
//#include <qguardedptr.h>

class QComboBox;
class QPixmap;

class KURL;
class KFileItem;

class KExifWidget;

class ImagePropertiesEXIF : public QObject
{
    Q_OBJECT

public:

    ImagePropertiesEXIF(QWidget* page);
    ~ImagePropertiesEXIF();

    void setCurrentURL(const KURL& url);

private:

    KExifWidget                  *m_exifWidget;
    QComboBox                    *m_levelCombo;
    QLabel                       *m_labelThumb;

    QString                       m_currItem;
    
private slots:

    void slotLevelChanged(int);
    void slotGotThumbnail(const KFileItem *, const QPixmap &);
    void slotFailedThumbnail(const KFileItem *);
};

#endif /* IMAGEPROPERTIESEXIF_H */
