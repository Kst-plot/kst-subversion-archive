/***************************************************************************
                          numSlider.cpp  -  description
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

#include <stdio.h>
#include <stdlib.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,3,0)
#include <fixx11h.h>
#endif

// Local
#include "numslider.h"

#include <qslider.h>
#include <qlineedit.h>

extern int max(int a, int b);

numSlider::numSlider( QWidget *parent)
	: QWidget(parent, "numSlider")
{
	init(0, 10, 0, 0, QBoxLayout::LeftToRight);
}


numSlider::numSlider( QBoxLayout::Direction aDir, QWidget *parent, const char *name )
	: QWidget(parent, name)
{
	init(0, 10, 0, 0, aDir);
}


numSlider::numSlider(double minValue, double maxValue, int decimals,
										 double value, QBoxLayout::Direction aDir,
										 QWidget *parent, const char *name )
	: QWidget(parent, name)
{
	init(minValue, maxValue, decimals, value, aDir);
}


void
numSlider::init(double minValue, double maxValue, int decimals, double value,
								QBoxLayout::Direction aDir)
{
	QSlider::Orientation orient;
	QBoxLayout *bl;

	theDecimals=decimals;
	dir=aDir;
	bl=new QBoxLayout(this, dir, 2);

	if ((dir==QBoxLayout::TopToBottom) || (dir==QBoxLayout::BottomToTop))
		orient=QSlider::Vertical;
	else
		orient=QSlider::Horizontal;

	slider=new QSlider(0, SLIDERMAX, SLIDERMAX/20, 0, orient, this);
	if (orient==QSlider::Vertical)
		slider->setFixedWidth(20);
	else
		slider->setFixedHeight(20);
	bl->addWidget(slider);

	num=new QLineEdit(this);
	num->setFixedHeight(20);
	bl->addWidget(num);

	//	QDoubleValidator *dv=new QDoubleValidator(theMin,theMax,0,this);
	//	num->setValidator(dv);

	bl->activate();

	theValue=value;
	theMin=minValue;
	theMax=maxValue;

	char str[16];
	int numWidth=max(20,num->fontMetrics().width(str));
	numWidth=max(numWidth,num->fontMetrics().width(str));
	num->setFixedWidth(numWidth);

 	range=theMax-theMin;

	connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
	connect(num, SIGNAL(returnPressed()), this, SLOT(numberChanged()));

	updateValue();
	numberChanged();
}


void
numSlider::sliderChanged(int value)
{
	theValue=range*value/SLIDERMAX+theMin;
	updateValue();
}


void
numSlider::updateValue()
{
	QString numText;
	numText.sprintf("%.*f",theDecimals,theValue);
	num->setText(numText);

	emit valueChanged(theValue);
}


void
numSlider::numberChanged()
{
	theValue=num->text().toInt();
	if (theValue>theMax) { theValue=theMax; }
	if (theValue<theMin) { theValue=theMin; }
	int sliderValue=int(SLIDERMAX*(theValue-theMin)/range);
	slider->setValue(sliderValue);
	updateValue();
}


double
numSlider::value()
{
	return(theValue);
}

#include "numslider.moc"
