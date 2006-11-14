/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kexidb/schemadata.h>
#include <kexidb/connection.h>

#include <kdebug.h>

using namespace KexiDB;

SchemaData::SchemaData(int obj_type)
	: m_type(obj_type)
	, m_id(-1)
	, m_native(false)
{
}

SchemaData::~SchemaData()
{
}

void SchemaData::clear()
{
	m_id = -1;
	m_name = QString::null;
	m_caption = QString::null;
	m_desc = QString::null;
}

QString SchemaData::schemaDataDebugString() const
{
	QString desc = m_desc;
	if (desc.length()>40) {
		desc.truncate(40);
		desc+="...";
	}
	return QString("id=%1 name='%2' caption='%3' desc='%4'")
		.arg(m_id).arg(m_name).arg(m_caption).arg(desc);
}
