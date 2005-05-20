/***************************************************************************
                         imageentry.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "imageentry.h"

ImageEntry::ImageEntry(int id, const QString& name, int directory_id,
			const QString& comment, int note,
			QDateTime date_begin, QDateTime date_end)
{
	this->id=id;
	this->name=name;
	this->directory_id=directory_id;
	this->comment=comment;
	this->note=note;
	this->date_begin=date_begin;
	this->date_end=date_end;
}

QString
ImageEntry::getName()
{
	return name;
}
int
ImageEntry::getId()
{
	return id;
}

QString
ImageEntry::toString()
{
	return QString("%1 %2 %3 «%4» %5 %6 %7")
			.arg(id)
			.arg(name)
			.arg(directory_id)
			.arg(comment)
			.arg(note)
			.arg(date_begin.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(date_end.toString("yyyy-MM-dd hh:mm:ss"));
}

