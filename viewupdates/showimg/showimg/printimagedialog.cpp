/***************************************************************************
                          printImageDialog.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C)2001-2005 - Andrew Richards
    email                : ajr@users.sourceforge.net
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

#include "printimagedialog.h"

#include <stdio.h>

// Qt
#include <qpainter.h>
#include <qframe.h>
#include <qlayout.h>

// KDE
#include <kapplication.h>
#include <klocale.h>
#include <kdialog.h>


extern int min(int a, int b);
extern int max(int a, int b);


printImageDialog::printImageDialog(QWidget *parent, const QPixmap& pixmap, const QString& name, KPrinter *p)
	: KDialog(parent,"printdialog", true)
{
	setCaption(i18n("Print Image"));

	prn=p;
	imageName=name;
	pageSize=pageDimensions();
	pageScale=0.3;
	scale=1;
	pix=pixmap;

	QBoxLayout *lyMain=new QVBoxLayout(this);
	lyMain->addSpacing(int(pageSize.height()*pageScale+10));

	lyMain->addStrut(int(pageSize.width()*pageScale+10));
	lyMain->addSpacing(10);

	double maxScale = (int )min(100*pageSize.width()/pix.width(),
				100*pageSize.height()/pix.height());

	if ((scale*100) > (maxScale*0.9))
		scale=maxScale*0.9/100;

	slScale=new numSlider(1, maxScale, 1, scale*100 , QBoxLayout::LeftToRight,
				this);
	slScale->setFixedSize(200,20);
	lyMain->addWidget(slScale);
	connect(slScale, SIGNAL(valueChanged(double)), this, SLOT(newScale(double)));

	lyMain->addSpacing(10);

	QHBoxLayout *lyButtons=new QHBoxLayout;
	lyMain->addLayout(lyButtons);

	QPushButton *btBack = new QPushButton(i18n("<< Back"), this);
	btBack->setFixedSize(btBack->sizeHint());
	lyButtons->addWidget(btBack);
	connect(btBack, SIGNAL(clicked()), this, SLOT(back()));

	lyButtons->addSpacing(20);

	QPushButton *btCancel = new QPushButton(i18n("Cancel"), this);
	btCancel->setFixedSize(btCancel->sizeHint());
	lyButtons->addWidget(btCancel);
	connect(btCancel, SIGNAL(clicked()), this, SLOT(cancel()));

	lyButtons->addSpacing(10);

	QPushButton *btPrint = new QPushButton(i18n("Print"), this);
	btPrint->setFixedSize(btPrint->sizeHint());
	lyButtons->addWidget(btPrint);
	connect(btPrint, SIGNAL(clicked()), this, SLOT(printImage()));

	setMaximumSize(size());
}

printImageDialog::~printImageDialog()
{
}

void
printImageDialog::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	int xoff=int(width()-pageSize.width()*pageScale)/2;

	QSize scaledSize=pageSize.size()*pageScale;

	p.setPen(black);
	p.translate(xoff,5);
	p.fillRect(QRect(QPoint(3,3), scaledSize), black);
	p.fillRect(QRect(QPoint(0,0), scaledSize), white);
	p.scale(pageScale, pageScale);
	p.setClipRect(p.xForm(pageSize));
	paintImage(&p);
}


void
printImageDialog::paintImage(QPainter *p)
{
	QRect imagePos=QRect(QPoint(0,0), pix.size()*scale);
	imagePos.moveCenter(pageSize.center());

	p->save();
	p->translate(imagePos.x(), imagePos.y());
	p->scale(scale, scale);
	p->drawPixmap(0,0, pix);
	p->restore();
}


void
printImageDialog::printImage()
{
	done(0);
	kapp->processEvents();

	KApplication::setOverrideCursor (waitCursor); // this might take time

#ifndef Q_WS_WIN
	prn->setCreator("showimg");
	prn->setDocName(imageName);
#endif
	QPainter p;
	p.begin(prn);
	paintImage(&p);
	p.end();

	KApplication::restoreOverrideCursor ();
}


void
printImageDialog::cancel()
{
	done(1);
}


void
printImageDialog::back()
{
	done(2);
}


void
printImageDialog::newScale(double sc)
{
	scale=sc/100;
	repaint();
}


QRect
printImageDialog::pageDimensions()
{
	QSize dims;
#ifndef Q_WS_WIN
	switch (prn->pageSize()) {
	case QPrinter::A4:
		dims=QSize(210, 297);
		break;
	case QPrinter::B5:
		dims=QSize(182, 257);
		break;
	case QPrinter::Letter:
		dims=QSize(216, 279);
		break;
	case QPrinter::Legal:
		dims=QSize(216, 356);
		break;
	case QPrinter::Executive:
		dims=QSize(191, 254);
		break;
	default:
		dims=QSize(210, 297);
	}
	// Convert to points
	dims*=double(72/25.4);

#if KDE_VERSION > 220
	if (prn->orientation() != KPrinter::Portrait)
		dims.transpose();
#else
	if (prn->orientation() != QPrinter::Portrait)
		dims.transpose();
#endif
#endif//!win

	return(QRect(QPoint(0,0), dims));
}

#include "printimagedialog.moc"
