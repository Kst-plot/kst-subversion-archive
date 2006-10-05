
/***************************************************************************
                          kexifpropsplugin.h  -  description
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

#ifndef KEXIFPROPSPLUGIN_H
#define KEXIFPROPSPLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE
#include <kpropertiesdialog.h>

class KEXIFPropsPlugin : public KPropsDlgPlugin
{
	Q_OBJECT
public:
	/**
	* Constructor
	* To insert tabs into the properties dialog, use the add methods provided by
	* KDialogBase (via props->dialog() )
	*/
	KEXIFPropsPlugin( KPropertiesDialog *_props, const QString& exifinfo );

	/**
	 * Apply all changes to the file.
	 * This function is called when the user presses 'Ok'. The last plugin inserted
	 * is called first.
	 */
	virtual void applyChanges();

private:
	QString info;

protected slots:
	void copy();

};



#endif
