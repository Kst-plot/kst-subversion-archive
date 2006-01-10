/***************************************************************************
                         imageentry.h  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2005 by Richard Groult
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include <qstring.h>
#include <qdatetime.h>


class ImageEntry
{
public:
	ImageEntry(int id, const QString& name, int directory_id,
			const QString& comment="", int note=-1,
			QDateTime date_begin=QDateTime(), QDateTime date_end=QDateTime());

	int getId() const {return id;};
	QString getName(){return name;};
	int getDirectory(){return directory_id;};
	QString getComment(){return comment;};
	int getNote(){return note;};
	QDateTime getDate_begin(){return date_begin;};
	QDateTime getDate_end(){return date_end;};

	QString toString();

protected:
	int id;
	QString name;
	int directory_id;
	QString comment;
	int note;
	QDateTime date_begin;
	QDateTime date_end;

};

#endif
