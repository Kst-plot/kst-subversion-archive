/***************************************************************************
                          khexeditpropsplugin  -  description
                             -------------------
    begin                : mer oct 29 2003
    copyright            : (C) 2003 by Richard Groult
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

#ifndef KHEXEDITPROPSPLUGIN_H
#define KHEXEDITPROPSPLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE 
#include <kpropertiesdialog.h>

// Local 
#include "khexedit/hexbuffer.h"

class CHexBuffer;
class CHexViewWidget;
class CProgress;
class CHexValidator;

class QFile;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;

class KComboBox;
class KLineEdit;
class KPushButton;

/**
@author Richard Groult
*/
class KHexeditPropsPlugin : public KPropsDlgPlugin
{
	Q_OBJECT
public:
	/**
	* Constructor
	* To insert tabs into the properties dialog, use the add methods provided by
	* KDialogBase (via props->dialog() )
	*/
	KHexeditPropsPlugin(KPropertiesDialog *_props, const QString& exifinfo);

	/**
	 * Apply all changes to the file.
	 * This function is called when the user presses 'Ok'. The last plugin inserted
	 * is called first.
	 */
	~KHexeditPropsPlugin();

public slots:
	void slotFind();
	void slotTextChanged(const QString&);

protected:
	QGridLayout* Form1Layout;
	QVBoxLayout* layout2;
	QHBoxLayout* layout1;

	void languageChange();

private:
	KLineEdit* stringToFind;
	KComboBox* kComboBox1;
	KPushButton* findButton;

	CHexBuffer *hb;
	CHexViewWidget *hv;
	CProgress *p;
	
	SSearchControl sc;
	CHexValidator *mValidator;
	bool findFirst;
	
	QFile *myFile;
	
	QFrame *page;
	QString *myKey;
};

#endif
