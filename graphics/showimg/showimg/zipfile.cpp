/***************************************************************************
                          zipfile.cpp  -  description
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

#include "zipfile.h"

#include <stdlib.h>

// KDE
#include <kdebug.h>

ZipFile::ZipFile (const QString& archive, const QString& file)
{
	this->archive =archive;
	this->file = file;

	mystdout = new QCString ();
}

ZipFile::~ZipFile()
{
	delete(mystdout);
}

int
ZipFile::size ()
{

	QString com= QString(" unzip -Z  \"%1\" \"%2\" | tr -s \" \" \" \" | cut -d\" \" -f 4")
	   		.arg(archive)
			.arg(file);

	KShellProcess * proc = new KShellProcess ();
	connect (proc, SIGNAL (receivedStdout (KProcess *, char *, int)), this,
	         SLOT (slotMsgRcv (KProcess *, char *, int)));

	*proc << com;
	proc->start (KShellProcess::Block, KShellProcess::Stdout);

	return mystdout->toInt ();
}


bool
ZipFile::deleteFile ()
{
	QString com=QString(" zip -d  \"%1\" \"%2\"")
			.arg(archive)
			.arg(file);
	KShellProcess * proc = new KShellProcess ();
	connect (proc, SIGNAL (receivedStdout (KProcess *, char *, int)), this,
	         SLOT (slotMsgRcv (KProcess *, char *, int)));

	*proc << com;
	proc->start (KShellProcess::Block);

	return true;
}

void
ZipFile::slotMsgRcv (KProcess * , char *buffer, int buflen)
{
	mystdout = new QCString (buffer, buflen);
}

#include "zipfile.moc"
