/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : A widget to display IPTC metadata
 * 
 * Copyright 2006 by Gilles Caulier
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
 * 
 * ============================================================ */

#ifndef IPTCWIDGET_H
#define IPTCWIDGET_H

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT IptcWidget : public MetadataWidget
{
    Q_OBJECT
    
public:

    IptcWidget(QWidget* parent, const char* name=0);
    ~IptcWidget();

    bool loadFromURL(const KURL& url);
    
    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle(void);

protected slots:    
    
    virtual void slotSaveMetadataToFile(void);

private:

    bool decodeMetadata(void);
    void buildView(void);

private:
    
    QStringList m_tagsfilter;    
};

}  // namespace Digikam

#endif /* IPTCWIDGET_H */
