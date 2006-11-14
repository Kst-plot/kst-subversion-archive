/* ============================================================
 * File  : imageplugin_freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-28
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_freerotation.h"
#include "imageplugin_freerotation.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_freerotation,
                            KGenericFactory<ImagePlugin_FreeRotation>("digikamimageplugin_freerotation"));

ImagePlugin_FreeRotation::ImagePlugin_FreeRotation(QObject *parent, const char*, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_FreeRotation")
{
    m_freerotationAction = new KAction(i18n("Free Rotation..."), "freerotation", 0, 
                             this, SLOT(slotFreeRotation()),
                             actionCollection(), "imageplugin_freerotation");
    
    setXMLFile("digikamimageplugin_freerotation_ui.rc");         
                                    
    DDebug() << "ImagePlugin_FreeRotation plugin loaded" << endl;
}

ImagePlugin_FreeRotation::~ImagePlugin_FreeRotation()
{
}

void ImagePlugin_FreeRotation::setEnabledActions(bool enable)
{
    m_freerotationAction->setEnabled(enable);
}

void ImagePlugin_FreeRotation::slotFreeRotation()
{
    QString title = i18n("Free Rotation");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamFreeRotationImagesPlugin::ImageEffect_FreeRotation dlg(parentWidget(),
                                title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_freerotation.moc"
