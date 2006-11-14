
/***************************************************************************
                          kexifpropsplugin.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#include "kexifpropsplugin.h"

// Local
#include "misc/exif.h"

// Qt
#include <qheader.h>
#include <qlayout.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qfile.h>

// KDE
#include <klocale.h>
#include <klistview.h>
#include <kapplication.h>

KEXIFPropsPlugin::KEXIFPropsPlugin( KPropertiesDialog *_props, const QString& fileName )
:KPropsDlgPlugin(_props)
{
	#if  KDE_VERSION < 0x30200
	KDialogBase *dialog=_props->dialog();
	#else
	KPropertiesDialog *dialog=_props;
	#endif
	QFrame *page=dialog->addPage( i18n("&Metadata"));
	QString info=ProcessFile(QFile::encodeName(fileName));
	this->info=info;

	QBoxLayout *lyMain=new QVBoxLayout(page);
	KListView *listView = new KListView(page);
	listView->setFullWidth(true);
	lyMain->addWidget(listView);

	listView->addColumn(i18n("Title"));
        listView->header()->setClickEnabled( true, listView->header()->count() - 1 );
	listView->addColumn(i18n("Data"));
        listView->header()->setClickEnabled( true, listView->header()->count() - 1 );
	listView->setAllColumnsShowFocus ( true );

	QPushButton *butCopy=new QPushButton(i18n("Copy"), page);
	lyMain->addWidget(butCopy);
	connect(butCopy, SIGNAL(clicked()), this, SLOT(copy()));

	int pos = info.find ("\n");
	int i=0;

	QString line, debut, fin, order;
	int dp;
	while (pos != -1)
	{
		line = info.left(pos);
		dp=line.find (":");
		debut=line.left(dp).stripWhiteSpace();
		fin=line.mid(dp+1, line.length()).stripWhiteSpace();
		order.sprintf ("%010d",i);
		if(dp!=-1)
			(void)new KListViewItem(listView, debut, fin, order);

		info = info.right (info.length () - pos - 1);
		pos = info.find ("\n");
	}
	listView->setSorting(3, true);
	listView->sort();
}


void
KEXIFPropsPlugin::applyChanges()
{
}

void
KEXIFPropsPlugin::copy()
{
	KApplication::clipboard()->setText(info);
}


#include "kexifpropsplugin.moc"
