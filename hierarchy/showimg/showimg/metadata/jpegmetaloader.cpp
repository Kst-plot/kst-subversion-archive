/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : JPEG file metadata loader
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

#include "jpegmetaloader.h"


JPEGMetaLoader::JPEGMetaLoader(DMetadata* metadata)
              : DMetaLoader(metadata)
{
}

bool JPEGMetaLoader::load(const QString& filePath)
{
    return (loadWithExiv2(filePath));
}

bool JPEGMetaLoader::save(const QString& filePath)
{
    return (saveWithExiv2(filePath));
}

