/*****************************************************************************
                          cdarchivecreatordialog.h  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2006 by Richard Groult
    email                : rgroult@jalix.org
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the Free Software             *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301 *
 *   USA.                                                                    *
 *                                                                           *
 *   For license exceptions see LICENSE.EXC file, attached with the source   *
 *   code.                                                                   *
 *                                                                           *
 *****************************************************************************/

#ifndef CDARCHIVECREATORDIALOG_H
#define CDARCHIVECREATORDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qvariant.h>
#include <qdialog.h>

// KDE
#include <kdialogbase.h>

class CDArchiveCreator;

class KProgressDialog;
class KLineEdit;
class KPushButton;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QLabel;
class QTime;


class CDArchiveCreatorDialog : public KDialogBase
{
  Q_OBJECT
public:
	CDArchiveCreatorDialog( const QString & cdromPath = "", QWidget* parent = 0, const char* name = 0 );
	virtual ~CDArchiveCreatorDialog();

protected slots:
	void parseDirectoryDone();
	void chooseDir();
	void textChanged ( const QString& );

	void customEvent(QCustomEvent *event);
	virtual void languageChange();

public slots:
	void accept();
	void slotCancel();


protected:
    QGroupBox* groupBox1;
    QLabel* textLabel1;
    QLabel* textLabel2;
    KLineEdit* cdRomPathLineEdit;
    KLineEdit* archiveNameLineEdit;
    KPushButton* browseButton;

    QGridLayout* CDArchiveCreatorDialogLayout;
    QGridLayout* groupBox1Layout;
    QSpacerItem* spacer6;
    QSpacerItem* spacer5;
    QSpacerItem* spacer4;

private:
	CDArchiveCreator *m_p_ar;
 	KProgressDialog *m_p_progressDlg;
	int m_current;
	int m_total;
};


#endif
