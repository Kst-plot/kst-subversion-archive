/***************************************************************************
                         categoryproperties.h  -  description
                             -------------------
    begin                : fri 10 jun 2005
    copyright            : (C) 2005 by Richard Groult
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

#ifndef CATEGORYPROPERTIES_H
#define CATEGORYPROPERTIES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WANT_LIBKEXIDB

// KDE
#include <kdialogbase.h>
// Qt
#include <qvariant.h>
#include <qwidget.h>

class CategoryListItemTag;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class KIconButton;
class KTextEdit;
class QFrame;
class KLineEdit;
class QLabel;

class CategoryProperties : public KDialogBase
{
    Q_OBJECT

public:
    CategoryProperties( QWidget* parent, CategoryListItemTag* categoryListItemTag );
    ~CategoryProperties();

    const QString getName() const;
    const QString getDescription() const;
    const QString getIcon() const;

	QSize sizeHint () const;

protected:
    KIconButton* iconButton;
    KTextEdit* commentTextEdit;
    QFrame* line1;
    KLineEdit* nameLineEdit;
    QLabel* nametextLabel;
    QLabel* describeTextLabel;

    QGridLayout* CategoryPropertiesLayout;
    QSpacerItem* spacer2;



    CategoryListItemTag* m_categoryListItemTag;



protected slots:
    virtual void languageChange();

};

#endif /* #ifdef WANT_LIBKEXIDB */

#endif // CATEGORYPROPERTIES_H
