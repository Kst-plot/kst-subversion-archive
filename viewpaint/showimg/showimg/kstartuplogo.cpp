/***************************************************************************
                           kstartuplogo.cpp  -  description
                             -------------------
    artwork              : KDevelop Project / Ralf Nolden <nolden@kde.org>
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001-2005 by Kai Heitkamp
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "kstartuplogo.h"

// KDE
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kwin.h>

KStartupLogo::KStartupLogo(QWidget * parent) : QWidget(parent, "KStartupLogo", WStyle_NoBorder | WStyle_Customize | WDestructiveClose | WType_TopLevel  ), m_bReadyToHide( false ){
	QPixmap pm;
	pm.load(locate("appdata", "pics/logo.png"));
	setBackgroundPixmap(pm);
	setGeometry(KApplication::desktop()->width()/2-pm.width()/2, KApplication::desktop()->height()/2-pm.height()/2, pm.width(),pm.height());
	KWin::setState(winId(), NET::StaysOnTop);
	setHideEnabled(true);
}

KStartupLogo::~KStartupLogo()
{
}

void KStartupLogo::mousePressEvent( QMouseEvent*)
{
	if (m_bReadyToHide) hide();
}

#include "kstartuplogo.moc"
