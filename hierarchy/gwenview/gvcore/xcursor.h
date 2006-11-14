/*  This file is part of the KDE libraries
    Copyright (C) 2003 Fredrik H�glund <fredrik@kde.org>
    Copyright (C) 2005 Lubos Lunak <l.lunak@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef gvxcursor_h
#define gvxcursor_h

#include <qasyncimageio.h>

namespace Gwenview {

class XCursorFormatType : public QImageFormatType {
public:
	QImageFormat* decoderFor(const uchar* buffer, int length);
	const char* formatName() const;
};

// -----------------------------------------------------------------------------
} // namespace

#endif // gvxcursor_h
