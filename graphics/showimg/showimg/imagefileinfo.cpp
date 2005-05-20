
/***************************************************************************
                          imagefileinfo.cpp  -  description
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

#include "imagefileinfo.h"

// QT
#include <qfileinfo.h> 
#include <qtextstream.h> 
#include <qglobal.h>
#include <qdir.h> 

// KDE 
#include <kio/job.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <sys/types.h>
#include <unistd.h>

ImageFileInfo::ImageFileInfo(const QString& filename, int type, bool r)
{
	this->type=type;
	this->filename=QFileInfo(filename).dir(true).absPath()+"/descriptions.txt";
	this->imageid=QFileInfo(filename).fileName();
	
	found=false;
	read(r);
}
ImageFileInfo::~ImageFileInfo()
{
}

void
ImageFileInfo::update(const QString& )
{
}

bool
ImageFileInfo::hasInfo()
{
	return found;
}

void
ImageFileInfo::read(bool r)
{
	if(!QFileInfo(filename).isFile())
	{
		return;
	}

	QFile f(filename);
	if (!f.open(IO_ReadOnly) )
	{
		return;
	}
	
	QString res, lut;
	found=false;
	QTextStream ts(&f);
	QString tag;
	if(type==IMAGE)
		tag="<name>"+imageid+"</name>";
	else
	if(type==ALBUM)
		tag="<properties>";
	while(!ts.eof() && !found)
	{
		lut=ts.readLine ();
		found=(lut.find(tag,0,false)!=-1);
	}
	if(found && r)
	{
		bool fini=false;
		QString endtag;
		
		if(type==IMAGE)
			endtag="</file>";
		else
		if(type==ALBUM)
			endtag="</properties>";
		
		while(!ts.eof() && !fini)
		{
			lut=ts.readLine ();		

			fini=(lut.find(endtag,0,false)!=-1);
			if(!fini)
				info+=lut;
		}
	}
	f.close();
}

void
ImageFileInfo::write(QString title, QString event,
			QString location, QString people,
			QString date, QString description,
			QString fileDest)
{
	if(title.isEmpty()&&event.isEmpty()&&
		location.isEmpty()&&people.isEmpty()&&
		date.isEmpty()&&description.isEmpty())	
		return;
		
	if (type!=IMAGE)
		return;
		
	if(!verif(title+event+location+people+date+description))
		return;
	
	if(fileDest.isNull())
		fileDest=filename;
	
	QFile f(fileDest);
	bool isOpen=f.open(IO_ReadOnly);
	QTextStream ts(&f);

	QString 
		lut,
		tag="<name>"+imageid+"</name>";
	bool trouve=false;
	
	QFile ftemp(locateLocal("tmp", "showimg-net/showimg_temp"+QString().setNum(getpid())));
	if (!ftemp.open(IO_WriteOnly) )
		return;
	QTextStream tstemp(&ftemp);

	while(isOpen && !ts.eof() && !trouve)
	{
		lut=ts.readLine ();
		trouve=(lut.find(tag,0,false)!=-1);
		if(!trouve)
			tstemp << lut << "\n";
	}	
	if(trouve)
	{
		bool fini=false;
		QString endtag="</file>";
		while(!ts.eof() && !fini)
		{
			lut=ts.readLine ();		

			fini=(lut.find(endtag,0,false)!=-1);
		}
	}
	
	if(!trouve) tstemp << "<file>" << "\n";	
	tstemp << "\t" << tag << "\n";
	tstemp << "\t<title>" << title << "</title>\n";
	tstemp << "\t<event>" << event << "</event>\n";
	tstemp << "\t<location>" << location << "</location>\n";
	tstemp << "\t<people>" << people << "</people>\n";
	tstemp << "\t<date>" << date << "</date>\n";
	tstemp << "\t<description>" << description << "</description>\n";
	tstemp << "</file>" << "\n";	
	
	while(isOpen && !ts.eof())
	{
		lut=ts.readLine ();
		tstemp << lut <<"\n";

	}
	ftemp.close();
	f.close();

	//
	f.open(IO_WriteOnly);
	QTextStream ts1(&f);
	ftemp.open(IO_ReadOnly);
	QTextStream tstemp1(&ftemp);
	while(isOpen && !tstemp1.eof())
	{
		lut=tstemp1.readLine ();
		ts1 << lut <<"\n";

	}
	ftemp.close();
	f.close();


	/*
	KIO::file_move(KURL("file:/"+ftemp.name() ), KURL("file:/"+fileDest), -1,
				  true, false,
				  false);
	*/

}

void
ImageFileInfo::write(QString title,
		QString shortdesc, QString longdesc,
		QString fileDest)
{
	if(title.isEmpty()&&shortdesc.isEmpty()&&
		longdesc.isEmpty())
		return;
		
	if (type!=ALBUM)
		return;
		
	if(!verif(title+shortdesc+longdesc))
		return;
		
	if(fileDest.isNull())
		fileDest=filename;	
		
	QFile f(fileDest);
	bool isOpen=f.open(IO_ReadOnly);
	QTextStream ts(&f);
	
	QString 
		lut,
		tag="<properties>";
	
	bool trouve=false;

	QFile ftemp("/tmp/showimg_temp"+QString().setNum(getpid()));
	if (!ftemp.open(IO_WriteOnly) )
		return;
	QTextStream tstemp(&ftemp);

	
	while(isOpen && !ts.eof() && !trouve)
	{
		lut=ts.readLine ();
		trouve=(lut.find(tag,0,false)!=-1);
		if(!trouve)
			tstemp << lut << "\n";
	}	
	if(trouve)
	{
		bool fini=false;
		QString endtag="</properties>";
		while(!ts.eof() && !fini)
		{
			lut=ts.readLine ();		

			fini=(lut.find(endtag,0,false)!=-1);
		}
	}
	
	
	tstemp << tag << "\n";
	tstemp << "\t<title>" << title << "</title>\n";
	tstemp << "\t<shortdesc>" << shortdesc << "</shortdesc>\n";
	tstemp << "\t<longdesc>" << longdesc << "</longdesc>\n";
	tstemp << "</properties>\n";

	while(isOpen && !ts.eof())
	{
		lut=ts.readLine ();		
		tstemp << lut <<"\n";
		
	}
	
	f.close();	
	ftemp.close();

	KIO::file_move(KURL("file:/"+ftemp.name() ), KURL("file:/"+fileDest), -1,
				  true, false,
				  false);

}


bool 
ImageFileInfo::verif(const QString& info)
{
	if(info.contains("<name>") || info.contains("</name>") ||
	info.contains("<properties>") || info.contains("</properties>") ||
	info.contains("<file>") || info.contains("</file>") ||
	info.contains("<title>") || info.contains("</title>")||
	info.contains("<event>") || info.contains("</event>") || 
	info.contains("<location>") || info.contains("</location>") ||
	info.contains("<people>") || info.contains("</people>") ||
	info.contains("<date>") || info.contains("</date>") ||
	info.contains("<description>") || info.contains("</description>") ||
	info.contains("<shortdesc>") || info.contains("</shortdesc>") ||
	info.contains("<longdesc>") || info.contains("</longdesc>") )	
		return false;
	else
		return true;
}

QString 
ImageFileInfo::getTitle() const
{
	int debut=info.find("<title>",0,false);
	int fin=info.findRev ("</title>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=7;
	return info.mid(debut, fin-debut );
}

void 
ImageFileInfo::setTitle(const QString& title)
{
	this->title=title;
}
	
QString 
ImageFileInfo::getEvent() const
{
	int debut=info.find("<event>",0,false);
	int fin=info.findRev ("</event>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=7;
	return info.mid(debut, fin-debut );
}

void 
ImageFileInfo::setEvent(const QString& event)
{
	this->event=event;
}

QString 
ImageFileInfo::getLocation() const
{
	int debut=info.find("<location>",0,false);
	int fin=info.findRev ("</location>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=10;
	return info.mid(debut, fin-debut );

}

void 
ImageFileInfo::setLocation(const QString& location)
{
	this->location=location;
}
	
QString 
ImageFileInfo::getPeople() const
{
	int debut=info.find("<people>",0,false);
	int fin=info.findRev ("</people>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=8;
	return info.mid(debut, fin-debut );
}

void 
ImageFileInfo::setPeople(const QString& people)
{
	this->people=people;
}

QString 
ImageFileInfo::getDate() const
{
	int debut=info.find("<date>",0,false);
	int fin=info.findRev ("</date>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=6;
	return info.mid(debut, fin-debut );
}

void 
ImageFileInfo::setDate(const QString& date)
{
	this->date=date;
}

QString 
ImageFileInfo::getDescription() const
{
	int debut=info.find("<description>",0,false);
	int fin=info.findRev ("</description>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=13;
	return info.mid(debut, fin-debut );
}

void 
ImageFileInfo::setDescription(const QString& description)
{
	this->description=description;
}


QString 
ImageFileInfo::getShortDescription() const
{
	int debut=info.find("<shortdesc>",0,false);
	int fin=info.findRev ("</shortdesc>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=11;
	return info.mid(debut, fin-debut );
	
}

void  
ImageFileInfo::setShortDescription(const QString& shortdesc )
{
	this->shortdesc=shortdesc;
}



QString 
ImageFileInfo::getLongDescription() const
{
	int debut=info.find("<longdesc>",0,false);
	int fin=info.findRev ("</longdesc>",-1,false);
	if(debut==-1 || fin==-1)
		return QString();
	debut+=10;
	return info.mid(debut, fin-debut );
	
}
void  
ImageFileInfo::setLongDescription(const QString& longdesc)
{
	this->longdesc=longdesc;
}



