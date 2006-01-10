/* ============================================================
 * File  : imageeffect_emboss.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin to emboss 
 *               an image.
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
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_EMBOSS_H
#define IMAGEEFFECT_EMBOSS_H

// Local includes.

#include "ctrlpaneldialog.h"

class KIntNumInput;

namespace DigikamEmbossImagesPlugin
{

class ImageEffect_Emboss : public DigikamImagePlugins::CtrlPanelDialog
{
    Q_OBJECT

public:

    ImageEffect_Emboss(QWidget* parent);
    ~ImageEffect_Emboss();

private:

    KIntNumInput *m_depthInput;
    
protected:

    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* IMAGEEFFECT_EMBOSS_H */
