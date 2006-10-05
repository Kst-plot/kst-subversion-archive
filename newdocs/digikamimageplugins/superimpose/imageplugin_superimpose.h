/* ============================================================
 * File  : imageplugin_superimpose.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-04
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_SUPERIMPOSE_H
#define IMAGEPLUGIN_SUPERIMPOSE_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_SuperImpose : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_SuperImpose(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_SuperImpose();

    void setEnabledActions(bool enable);

private:

    KAction *m_superimposeAction;
   
private slots:

    void slotSuperImpose();

};
    
#endif /* IMAGEPLUGIN_SUPERIMPOSE_H */
