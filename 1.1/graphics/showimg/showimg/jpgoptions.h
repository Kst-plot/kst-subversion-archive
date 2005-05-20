
/***************************************************************************
                          jpgoptions.h  -  description
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

#ifndef JPGOPTIONS_H
#define JPGOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE 
#include <kdialogbase.h>

// QT
#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QLabel;
class QSlider;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QFrame;
class QPushButton;

class JPGOptions : public KDialogBase
{ 
	Q_OBJECT

public:
	JPGOptions( QWidget* parent = 0, const char* name = 0 );
	~JPGOptions();

	QString getOptions();
        
private:
    QGroupBox* GroupBox13;
    QLabel* TextLabel4;
    QSlider* qualitySslider1;
    QLabel* TextLabel5;
    QSpinBox* SpinBox1;
    QCheckBox* progressiveCheckBox;
    QLabel* TextLabel6;
    QComboBox* samplingComboBox;
    QLabel* TextLabel7;
    QSpinBox* smootingSpinBox;
    QFrame* Line1;
    QCheckBox* saveCheckBox;
    QPushButton* PushButton20;
    QPushButton* PushButton21;
    QPushButton* PushButton22;

    QVBoxLayout* JPGOptionsLayout;
    QSpacerItem* spacer8;
    QSpacerItem* spacer9;
    QVBoxLayout* GroupBox13Layout;
    QHBoxLayout* layout6;
    QSpacerItem* Spacer1;
    QSpacerItem* Spacer2;
    QHBoxLayout* layout7;
    QSpacerItem* spacer6;
    QSpacerItem* spacer7;
    QHBoxLayout* Layout2;
    QHBoxLayout* layout5;
    QSpacerItem* spacer5;
    QHBoxLayout* layout3;
    QSpacerItem* spacer3;

public slots:
	virtual void slotDefault();
};

#endif // JPGOPTIONS_H
