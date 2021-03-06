// -*- c++ -*-

/*
 *  Copyright (C) 2003, Richard J. Moore <rich@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KJSEMBED_BRUSH_PLUGIN_H
#define KJSEMBED_BRUSH_PLUGIN_H

#include <kjsembed/jsproxy_imp.h>

namespace KJSEmbed {
namespace Bindings {

class BrushImp : public JSProxyImp {

    /** Identifiers for the methods provided by this class. */
    enum MethodId { Methodstyle, MethodsetStyle, MethodColor, MethodsetColor };

public:
    BrushImp( KJS::ExecState *exec, int id );
    virtual ~BrushImp();

    static void addBindings( KJS::ExecState *exec, KJS::Object &object );

    /** Returns true iff this object implements the call function. */
    virtual bool implementsCall() const {
        return true;
    }

    /** Invokes the call function. */
    virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );


private:
    int mid;
};

} // namespace
} // namespace

#endif // KJSEMBED_IMAGE_IMP_H
