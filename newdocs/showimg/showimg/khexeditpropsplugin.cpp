/***************************************************************************
                          khexeditpropsplugin  -  description
                             -------------------
    begin                : mer oct 29 2003
    copyright            : (C) 2003-2005 by Richard Groult
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

#include "khexeditpropsplugin.h"

// Local
#include "khexedit/hexviewwidget.h"
#include "khexedit/hexbuffer.h"
#include "khexedit/progress.h"
#include "khexedit/hexeditstate.h"
#include "khexedit/hexvalidator.h"

// KDE
#include <klocale.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

// Qt
#include <qfile.h>
#include <qfont.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qframe.h>
#include <qlayout.h>
#include <qcstring.h>

KHexeditPropsPlugin::KHexeditPropsPlugin(KPropertiesDialog *_props, const QString& exifinfo)
 : KPropsDlgPlugin(_props)
{
	#if  KDE_VERSION < 0x30200
	KDialogBase *dialog=_props->dialog();
	#else
	KPropertiesDialog *dialog=_props;
	#endif
	page=dialog->addPage( i18n("&Hexa"));
	QGridLayout *lyMain=new QGridLayout(page, 1, 1);
	//
	hb=new CHexBuffer();
	hv=new CHexViewWidget( page, "CHexViewWidget hv", hb );

	SDisplayFontInfo mfntInfo;
	mfntInfo.init();
	mfntInfo.font.setPointSize( 10 );
    	hb->setFont(mfntInfo);

	SDisplayLayout mySDisplayLayout;
	mySDisplayLayout.offsetVisible=false;
	hv->setLayout(mySDisplayLayout);

	SDisplayInputMode myDisplayInputMode;
	myDisplayInputMode.inputLock=true;
	hb->setInputMode(myDisplayInputMode);

	p=new CProgress();

	myFile=new QFile(exifinfo);
	myFile->open( IO_ReadOnly );
	hb->readFile(*myFile, "file:"+exifinfo, *p );
	//
	layout2 = new QVBoxLayout( 0, 0, 6, "layout2");

	hv->setFrameShape( QFrame::StyledPanel );
	hv->setFrameShadow( QFrame::Raised );
	layout2->addWidget(hv);

	layout1 = new QHBoxLayout( 0, 0, 6, "layout1");

	stringToFind = new KLineEdit( page, "stringToFind" );
	stringToFind->setMinimumSize( QSize( 150, 0 ) );
	layout1->addWidget( stringToFind );

	kComboBox1 = new KComboBox( false, page, "kComboBox1" );
	kComboBox1->setMinimumSize( QSize( 110, 0 ) );
	layout1->addWidget( kComboBox1 );

	findButton = new KPushButton( page, "findButton" );
	findButton->setMinimumSize( QSize( 70, 0 ) );
	layout1->addWidget( findButton );
	layout2->addLayout( layout1 );

	lyMain->addLayout( layout2, 0, 0 );
	languageChange();
	page->setMinimumWidth(hv->dataWidth()+2*mfntInfo.font.pointSize());

	//
	connect( stringToFind, SIGNAL( returnPressed() ), this, SLOT( slotFind() ) );
	connect( findButton, SIGNAL( clicked() ), this, SLOT( slotFind() ) );
	connect( stringToFind, SIGNAL(textChanged(const QString&)), this, SLOT( slotTextChanged(const QString&)));
	connect( kComboBox1, SIGNAL(activated(const QString&)), this, SLOT( slotTextChanged(const QString&)));

	//
	myKey=NULL;
	mValidator = new CHexValidator(page, (CHexValidator::EState)kComboBox1->currentItem());
	findFirst=true;
}


KHexeditPropsPlugin::~KHexeditPropsPlugin()
{
	myFile->close();
	delete(hb);
	delete(hv);
	delete(p);
	delete(myFile);
	delete(myKey);
	delete(mValidator);
}


void
KHexeditPropsPlugin::languageChange()
{
    kComboBox1->clear();
    kComboBox1->insertItem( i18n( "Hexadecimal" ) );
    kComboBox1->insertItem( i18n( "Decimal" ) );
    kComboBox1->insertItem( i18n( "Octal" ) );
    kComboBox1->insertItem( i18n( "Binary" ) );
    kComboBox1->insertItem( i18n( "Simple Text" ) );
    findButton->setText( i18n( "F&ind" ) );
}

void
KHexeditPropsPlugin::slotFind()
{
	if(findFirst && myKey)
	{
		sc.fromCursor=false;
		sc.inSelection=false;
		sc.forward=true;
		sc.ignoreCase=false;
		sc.wrapActive=true;

		mValidator->setState((CHexValidator::EState)kComboBox1->currentItem());
		mValidator->convert(sc.key, *myKey);
		sc.keyType=kComboBox1->currentItem();

		findFirst = false;
		int res=hv->findFirst(sc);
		if(res==Err_NoMatch||res==Err_WrapBuffer||res==Err_NoData)
		{
			KMessageBox::sorry(page, "<qt>"+i18n("The string \"%1\" was not found.").arg(*myKey)+"</qt>");
		}
		else
		if (res == Err_EmptyArgument)
			KMessageBox::error(page,  "<qt>"+i18n("The argument is not valid.")+"</qt>");
		else if (res!=0)
		{

		}
	}
	else
	{
		int res=hv->findNext(sc);
		if(res==Err_WrapBuffer)
		{
			findFirst = true;
			slotFind();
		}
	}

}



void
KHexeditPropsPlugin::slotTextChanged(const QString&)
{
	delete(myKey);
	if(!stringToFind->text().isEmpty())
		myKey = new QString(stringToFind->text());
	else
		myKey = NULL;
	findFirst=true;
}






#include "khexeditpropsplugin.moc"
