/***************************************************************************
                         categoryimageproperties.h  -  description
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

#ifndef CategoriesImageProperty_H
#define CategoriesImageProperty_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef WANT_LIBKEXIDB

// KDE
#include <kdialogbase.h>

#include <qvariant.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qdict.h>

class KTextEdit;
class KListView;
class KDateWidget;
class KSqueezedTextLabel;
class KListViewItem;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QCheckBox;
class KTextEdit;
class KListView;
class QListViewItem;
class QSlider;
class QSpinBox;
class KDateWidget;
class QLabel;
class KSqueezedTextLabel;

class QStringList;
class QVariant;
class CategoryNode;
class ImageEntry;
class CategoryDBManager;
class CategoriesImagePropertyCategoryItem;

class CategoriesImageProperty : public KDialogBase
{
    Q_OBJECT

public:
    CategoriesImageProperty( QWidget* parent, CategoryDBManager* categoryDBManager, ImageEntry* imageEntry);
    CategoriesImageProperty( QWidget* parent, CategoryDBManager* categoryDBManager, QPtrList<ImageEntry>& imageEntryList, int numerOfImages);
    ~CategoriesImageProperty();

    //return ID category IDs
    QStringList getCheckedCategories(bool takeIntoAccountTristate=false);
    QStringList getRemovedCategories();
    QStringList getAddedCategories();

    QString getName();
    QString getComment();
    int getNote();
    QDateTime getDate_begin();
    QDateTime getDate_end();

	QSize sizeHint () const;

protected:
    //internal
    void createUI();
    void init();
    void createCategoryView();
    void createSubCategoryView(CategoriesImagePropertyCategoryItem *root, CategoryNode *rootnode);

    void visitCategoryTree();
    bool visitCategoryTree_rec(CategoriesImagePropertyCategoryItem *root, int d);

protected:
    QGroupBox* commentsGroupBox;
    QCheckBox* commentsCheckBox;
    KTextEdit* commentTextEdit;
    QGroupBox* categoriesGroupBox;
    KListView* categoriesListView;
    QGroupBox* notesGroupBox;
    QCheckBox* noteCheckBox;
    QSlider* noteSlider;
    QSpinBox* noteSpinBox;
    QGroupBox* datesGroupBox;
    KDateWidget* endDateWidget;
    QLabel* endTextLabel;
    QLabel* beginTextLabel;
    KDateWidget* beginDateWidget;
    QCheckBox* datesCheckBox;
    KSqueezedTextLabel* imagenameTextLabel;

    QGridLayout* CategoriesImageProperpertyLayout;
    QVBoxLayout* commentsGroupBoxLayout;
    QVBoxLayout* categoriesGroupBoxLayout;
    QGridLayout* notesGroupBoxLayout;
    QGridLayout* datesGroupBoxLayout;

protected:
    ImageEntry* m_imageEntry;
    CategoryDBManager* m_categoryDBManager;
    QDict<QVariant> *numberOfImagesForCategory;
    int m_numerOfImages;

protected slots:
    virtual void languageChange();

};
#endif /* #ifdef WANT_LIBKEXIDB */
#endif // CategoriesImageProperty_H


