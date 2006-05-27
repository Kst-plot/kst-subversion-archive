/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : TIFF file metadata loader
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

// Local includes.

#include "tiffmetaloader.h"

namespace Digikam
{

TIFFMetaLoader::TIFFMetaLoader(DMetadata* metadata)
              : DMetaLoader(metadata)
{
}

bool TIFFMetaLoader::load(const QString& filePath)
{
    return (loadWithExiv2(filePath));
}

bool TIFFMetaLoader::save(const QString& filePath)
{
    return (saveWithExiv2(filePath));
}

}  // NameSpace Digikam
