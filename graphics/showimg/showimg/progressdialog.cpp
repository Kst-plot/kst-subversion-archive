/***************************************************************************
                          ProgressDialog.cpp  -  description
                             -------------------
    begin                : Die Mai 15 15:34:19 CEST 2001
    copyright            : (C) 2001 by Dominik Seichter
    email                : domseichter@web.de
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

#include "progressdialog.h"

// KDE 
#include <kapplication.h>
#include <klocale.h>

ProgressDialog::ProgressDialog( QWidget* parent, bool , const char* name, bool modal, WFlags fl )
    : QProgressDialog( parent, name, modal, fl )
{
        setCaption(i18n("Rename Series..."));
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::inc(  )
{
        done++;
        setProgress(done);
	kapp->processEvents();
}

void ProgressDialog::init( int numOfFiles )
{
        done=0;
	setTotalSteps(numOfFiles);
	reset();
}

void ProgressDialog::print(const QString& in, const QString& out) 
{
    setLabelText(in+(out.isEmpty()?(QString)"->\n"+out:(QString)""));
    return;
}

void ProgressDialog::quitAll()
{
    if(!preview)
        KApplication::exit(0);
    else
        delete this;
}

#include "progressdialog.moc"
