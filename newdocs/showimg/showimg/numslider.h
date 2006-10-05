/***************************************************************************
                          numSlider.h  -  description
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

#ifndef NUMSLIDER_H
#define NUMSLIDER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qlayout.h>
#include <qvalidator.h>

class QLineEdit;
class QSlider;

#define SLIDERMAX 1000000

class numSlider : public QWidget
{
	Q_OBJECT

 public:
	/**
		a slider with a number
	*/
	numSlider(QWidget *parent=0 );
	numSlider(QBoxLayout::Direction aDir, QWidget *parent=0, const char *name=0);
	numSlider(double minValue, double maxValue, int decimals, double value,
						QBoxLayout::Direction aDir, QWidget *parent=0, const char *name=0);
	double value();

 protected slots:
	void sliderChanged(int value);
	void numberChanged();

 signals:
	 void valueChanged(double value);

 private:
	void init(double minValue, double maxValue, int decimals, double value,
						QBoxLayout::Direction aDir);
	void updateValue();

	QSlider *slider;
	QLineEdit *num;
	QBoxLayout::Direction dir;
	double theValue,theMax,theMin,range;
	int theDecimals;
};

#endif
