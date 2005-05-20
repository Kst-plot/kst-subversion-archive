
/***************************************************************************
                          imagefileinfo.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#ifndef IMAGEFILEINFO_H
#define IMAGEFILEINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// QT
#include <qstring.h>

#define IMAGE	0
#define ALBUM	1

class QString;

class ImageFileInfo
{
public:
	ImageFileInfo(const QString& filename, int type, bool r=true);
	~ImageFileInfo();

	bool hasInfo();

	void write(QString title, QString event,
			QString location, QString people,
			QString date, QString description,
			QString fileDest=QString());

	void write(QString title,
		QString shortDescription, QString longDescription,
		QString fileDest=QString());

	void update(const QString& destDir);

	QString getTitle() const;
	void setTitle(const QString& title);
	QString getEvent() const;
	void setEvent(const QString& event);
	QString getLocation() const;
	void setLocation(const QString& location);
	QString getPeople() const;
	void setPeople(const QString& people);
	QString getDate() const;
	void setDate(const QString& date);
	QString getDescription() const;
	void setDescription(const QString& description);
	QString getShortDescription() const;
	void setShortDescription(const QString& shortdesc);
	QString getLongDescription() const;
	void setLongDescription(const QString& longdesc);

protected:
	bool verif(const QString& info);
	void read(bool r=true);
	int type;

	QString filename,imageid;
	QString info;

	QString
		title, event,
		location, people,
		date, description,
		shortdesc, longdesc;

private:
	bool found;

};
#endif
