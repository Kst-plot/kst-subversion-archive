/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2004-08-25
 * Description : a digiKam image editor plugin to simulate 
 *               an oil painting.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#ifndef IMAGEEFFECT_OILPAINT_H
#define IMAGEEFFECT_OILPAINT_H

// Digikam includes.

#include "ctrlpaneldlg.h"

class KIntNumInput;

namespace DigikamOilPaintImagesPlugin
{

class ImageEffect_OilPaint : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_OilPaint(QWidget* parent);
    ~ImageEffect_OilPaint();

private slots:

    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();    
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    KIntNumInput *m_brushSizeInput;
    KIntNumInput *m_smoothInput;
};

}  // NameSpace DigikamOilPaintImagesPlugin

#endif /* IMAGEEFFECT_OILPAINT_H */
