/***************************************************************************
                          kstartuplogo.h  -  description
                             -------------------
    artwork              : KDevelop Project / Ralf Nolden <nolden@kde.org>
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001 by Kai Heitkamp
    email                : koncd@kai-heitkamp.de
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

#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include "showimg_export.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt 
#include <qwidget.h>

/**
  *@author 
  */

class SHOWIMGCORE_EXPORT KStartupLogo : public QWidget  {
   Q_OBJECT

public:
	KStartupLogo(QWidget *parent=0, const char *name=0);
	~KStartupLogo();

	void setHideEnabled(bool bEnabled) { m_bReadyToHide = bEnabled; };

protected:
	virtual void mousePressEvent( QMouseEvent*);
	bool m_bReadyToHide;
};

#endif
