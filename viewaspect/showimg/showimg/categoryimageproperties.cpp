/***************************************************************************
                         categoryimageproperties.cpp  -  description
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

#include "categoryimageproperties.h"

#ifdef WANT_LIBKEXIDB

#include "categorydbmanager.h"
#include <showimgdb/categorynode.h>
#include <showimgdb/imageentry.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qptrdict.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qstyle.h>
#include <qpen.h>

#include <klistview.h>
#include <ktextedit.h>
#include <kdatewidget.h>
#include <kdebug.h>
#include <kapplication.h>
#include <ksqueezedtextlabel.h>
#include <kiconloader.h>
#include <klocale.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

class CategoriesImagePropertyCategoryItem : public QCheckListItem
{
public:
	CategoriesImagePropertyCategoryItem(CategoriesImagePropertyCategoryItem* root, CategoryNode* node)
	: QCheckListItem(root, node->getTitle(), QCheckListItem::CheckBox)
	{
		m_id = QString::number(node->getId());
		setSubCategoryIsSelected(false);
		setSeen(false);
	}

	CategoriesImagePropertyCategoryItem(QListViewItem* root, CategoryNode* node)
	: QCheckListItem(root, node->getTitle(), QCheckListItem::CheckBox)
	{
		m_id = QString::number(node->getId());
	}

	QString getId(){return m_id;}

	void
	paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
	{
		QColorGroup myCg(cg);
		if(getSubCategoryIsSelected() || state()==QCheckListItem::NoChange || state()==QCheckListItem::On )
			myCg.setColor(QColorGroup::Text, QColor("steelblue"));
		QCheckListItem::paintCell(p, myCg, column, width, alignment);
	}

	void setSubCategoryIsSelected(bool s){m_hasSubCategoryIsSelected=s;};
	bool getSubCategoryIsSelected(){return m_hasSubCategoryIsSelected;};

	void setSeen(bool s=true){m_seen=s;}
	bool isSeen(){return m_seen;}

protected:
	QString m_id;
	bool m_hasSubCategoryIsSelected;
	bool m_seen;
};


/*
 *  Constructs a CategoriesImageProperty as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CategoriesImageProperty::CategoriesImageProperty( QWidget* parent, CategoryDBManager* categoryDBManager, ImageEntry* imageEntry)
    : KDialogBase( parent, "CategoriesImageProperty", true, "Describe" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
{
    m_imageEntry = imageEntry;
    m_categoryDBManager = categoryDBManager;
    m_numerOfImages = 1;

    createUI();
    init();

    imagenameTextLabel->setText(imageEntry->getName());
}

CategoriesImageProperty::CategoriesImageProperty( QWidget* parent, CategoryDBManager* categoryDBManager, QPtrList<ImageEntry>& imageEntryList, int numerOfImages)
    : KDialogBase( parent, "CategoriesImageProperty", true, "Describe" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true ),

 		    m_imageEntry(NULL),
 		    m_categoryDBManager(categoryDBManager),
 		    m_numerOfImages(numerOfImages)
{
	numberOfImagesForCategory = new QDict<QVariant>(200);
	QStringList image_id_list;
	for(ImageEntry *image = imageEntryList.first(); image; image=imageEntryList.next())
		image_id_list.append(QString::number(image->getId()));
	QStringList* cat_id_list = m_categoryDBManager->getCategoryIdListImage(image_id_list, false);
	for ( QStringList::Iterator it = cat_id_list->begin(); it != cat_id_list->end(); ++it )
	{
			QVariant* num = (*numberOfImagesForCategory)[*it];
			if(!num)
			{
				numberOfImagesForCategory->insert(*it, new QVariant(1));
			}
			else
			{
				numberOfImagesForCategory->replace(*it, new QVariant(num->toInt()+1));
			}
	}



	if(!imageEntryList.isEmpty())
		m_imageEntry = imageEntryList.first();
	createUI();
	init();
	
	if(m_numerOfImages == 1)
		imagenameTextLabel->setText(i18n("File: ").arg(imageEntryList.first()->getName()));
	else
		imagenameTextLabel->setText(i18n("Files: %1 images").arg(imageEntryList.count()));



}

void
CategoriesImageProperty::createUI()
{
    QWidget *page = new QWidget( this );
    setMainWidget(page);

//////////////////////////
//    if ( !name )
	setName( "Categories Image Properperty" );
    CategoriesImageProperpertyLayout = new QGridLayout( page, 1, 1, 11, 6, "CategoriesImageProperpertyLayout");

    commentsGroupBox = new QGroupBox( page, "commentsGroupBox" );
    commentsGroupBox->setColumnLayout(0, Qt::Vertical );
    commentsGroupBox->layout()->setSpacing( 6 );
    commentsGroupBox->layout()->setMargin( 11 );
    commentsGroupBoxLayout = new QVBoxLayout( commentsGroupBox->layout() );
    commentsGroupBoxLayout->setAlignment( Qt::AlignTop );

    commentsCheckBox = new QCheckBox( commentsGroupBox, "commentsCheckBox" );
    commentsCheckBox->setChecked( true );
    commentsGroupBoxLayout->addWidget( commentsCheckBox );

    commentTextEdit = new KTextEdit( commentsGroupBox, "commentTextEdit" );
    commentsGroupBoxLayout->addWidget( commentTextEdit );

    CategoriesImageProperpertyLayout->addWidget( commentsGroupBox, 1, 1 );

    categoriesGroupBox = new QGroupBox( page, "categoriesGroupBox" );
    categoriesGroupBox->setColumnLayout(0, Qt::Vertical );
    categoriesGroupBox->layout()->setSpacing( 6 );
    categoriesGroupBox->layout()->setMargin( 11 );
    categoriesGroupBoxLayout = new QVBoxLayout( categoriesGroupBox->layout() );
    categoriesGroupBoxLayout->setAlignment( Qt::AlignTop );

    categoriesListView = new KListView( categoriesGroupBox, "categoriesListView" );
    categoriesListView->setAllColumnsShowFocus( true );
    categoriesListView->setTreeStepSize( 10 );
    categoriesListView->setFullWidth( true );
    categoriesListView->setItemsMovable( false );
    categoriesGroupBoxLayout->addWidget( categoriesListView );

    CategoriesImageProperpertyLayout->addMultiCellWidget( categoriesGroupBox, 1, 3, 0, 0 );

    notesGroupBox = new QGroupBox( page, "notesGroupBox" );
    notesGroupBox->setColumnLayout(0, Qt::Vertical );
    notesGroupBox->layout()->setSpacing( 6 );
    notesGroupBox->layout()->setMargin( 11 );
    notesGroupBoxLayout = new QGridLayout( notesGroupBox->layout() );
    notesGroupBoxLayout->setAlignment( Qt::AlignTop );

    noteCheckBox = new QCheckBox( notesGroupBox, "noteCheckBox" );
    noteCheckBox->setChecked( true );

    notesGroupBoxLayout->addWidget( noteCheckBox, 0, 0 );

    noteSlider = new QSlider( notesGroupBox, "noteSlider" );
    noteSlider->setMaxValue( 10 );
    noteSlider->setValue( 0 );
    noteSlider->setOrientation( QSlider::Horizontal );
    noteSlider->setTickmarks( QSlider::Below );
    noteSlider->setTickInterval( 1 );

    notesGroupBoxLayout->addWidget( noteSlider, 1, 0 );

    noteSpinBox = new QSpinBox( notesGroupBox, "noteSpinBox" );
    noteSpinBox->setMaxValue( 10 );

    notesGroupBoxLayout->addWidget( noteSpinBox, 1, 1 );

    CategoriesImageProperpertyLayout->addWidget( notesGroupBox, 2, 1 );

    datesGroupBox = new QGroupBox( page, "datesGroupBox" );
    datesGroupBox->setColumnLayout(0, Qt::Vertical );
    datesGroupBox->layout()->setSpacing( 6 );
    datesGroupBox->layout()->setMargin( 11 );
    datesGroupBoxLayout = new QGridLayout( datesGroupBox->layout() );
    datesGroupBoxLayout->setAlignment( Qt::AlignTop );

    endDateWidget = new KDateWidget( datesGroupBox, "endDateWidget" );

    datesGroupBoxLayout->addWidget( endDateWidget, 2, 1 );

    endTextLabel = new QLabel( datesGroupBox, "endTextLabel" );

    datesGroupBoxLayout->addWidget( endTextLabel, 2, 0 );

    beginTextLabel = new QLabel( datesGroupBox, "beginTextLabel" );

    datesGroupBoxLayout->addWidget( beginTextLabel, 1, 0 );

    beginDateWidget = new KDateWidget( datesGroupBox, "beginDateWidget" );

    datesGroupBoxLayout->addWidget( beginDateWidget, 1, 1 );

    datesCheckBox = new QCheckBox( datesGroupBox, "datesCheckBox" );
    datesCheckBox->setChecked( true );

    datesGroupBoxLayout->addMultiCellWidget( datesCheckBox, 0, 0, 0, 1 );

    CategoriesImageProperpertyLayout->addWidget( datesGroupBox, 3, 1 );

    imagenameTextLabel = new KSqueezedTextLabel( page, "imagenameTextLabel" );

    CategoriesImageProperpertyLayout->addMultiCellWidget( imagenameTextLabel, 0, 0, 0, 1 );
    languageChange();
    clearWState( WState_Polished );

    // signals and slots connections
    connect( noteSlider, SIGNAL( valueChanged(int) ), noteSpinBox, SLOT( setValue(int) ) );
    connect( noteSpinBox, SIGNAL( valueChanged(int) ), noteSlider, SLOT( setValue(int) ) );
    connect( commentsCheckBox, SIGNAL( toggled(bool) ), commentTextEdit, SLOT( setEnabled(bool) ) );
    connect( noteCheckBox, SIGNAL( toggled(bool) ), noteSlider, SLOT( setEnabled(bool) ) );
    connect( noteCheckBox, SIGNAL( toggled(bool) ), noteSpinBox, SLOT( setEnabled(bool) ) );
    connect( datesCheckBox, SIGNAL( toggled(bool) ), beginDateWidget, SLOT( setEnabled(bool) ) );
    connect( datesCheckBox, SIGNAL( toggled(bool) ), endDateWidget, SLOT( setEnabled(bool) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CategoriesImageProperty::~CategoriesImageProperty()
{
    // no need to delete child widgets, Qt does it all for us
}

void
CategoriesImageProperty::init()
{
	if(m_numerOfImages == 1)
	{
		commentsCheckBox->hide();
		noteCheckBox->hide();
		datesCheckBox->hide();
	}
	else
	{
		commentsCheckBox->setChecked(false);
		noteCheckBox->setChecked(false);
		datesCheckBox->setChecked(false);
	}
	categoriesListView->addColumn("Name");
    createCategoryView();
    visitCategoryTree();

	if(!m_imageEntry) return;
	beginDateWidget->setDate(m_imageEntry->getDate_begin().date());
	endDateWidget->setDate(m_imageEntry->getDate_end().date());
	imagenameTextLabel->setText( m_imageEntry->getName() );
	commentTextEdit->setText( m_imageEntry->getComment() );
	noteSlider->setValue(m_imageEntry->getNote());
}

void
CategoriesImageProperty::visitCategoryTree()
{
	CategoriesImagePropertyCategoryItem *root =
		(CategoriesImagePropertyCategoryItem *)(categoriesListView->firstChild()->firstChild());
	visitCategoryTree_rec(root, 0);

	while(root)
	{
		root->setOpen(root->getSubCategoryIsSelected());
		root = (CategoriesImagePropertyCategoryItem *)root->nextSibling ();
	}
}

bool
CategoriesImageProperty::visitCategoryTree_rec(CategoriesImagePropertyCategoryItem *root, int d)
{
	QString espace;
	for (int i=0; i<d; i++) espace+="  ";
	bool rootHasSubCategoryIsSelected = root->state() == QCheckListItem::On || root->state() == QCheckListItem::NoChange ;
	while(root)
	{
		CategoriesImagePropertyCategoryItem *item = (CategoriesImagePropertyCategoryItem *)(root->firstChild());
		bool itemHasSubCategoryIsSelected = false;
		while(item)
		{
			if(!item->isSeen())
			{
				itemHasSubCategoryIsSelected = itemHasSubCategoryIsSelected || visitCategoryTree_rec(item, d+1);
				item->setSeen();
			}
			item = (CategoriesImagePropertyCategoryItem *)item->nextSibling ();
		}
		root->setSubCategoryIsSelected(itemHasSubCategoryIsSelected);
		rootHasSubCategoryIsSelected = rootHasSubCategoryIsSelected || itemHasSubCategoryIsSelected;

		root = (CategoriesImagePropertyCategoryItem *)root->nextSibling();
	}
	return rootHasSubCategoryIsSelected;
}

void
CategoriesImageProperty::createCategoryView()
{
	QPtrList<CategoryNode> cat_list = m_categoryDBManager->getRootCategories();
	QListViewItem *root = new QListViewItem(categoriesListView, i18n("Categories"));
	root->setOpen(true);
	for(CategoryNode *node = cat_list.first(); node; node=cat_list.next())
	{
		CategoriesImagePropertyCategoryItem *item = new CategoriesImagePropertyCategoryItem(root, node);
		item->setOpen(true);item->setOpen(false);
		item->setPixmap(0, BarIcon(node->getIcon(), 16 ));

		QVariant* num = (*numberOfImagesForCategory)[item->getId()];
		if(num)
		{
			if(num->toInt() != m_numerOfImages)
			{
				item->setState(QCheckListItem::NoChange);
				item->setTristate(true);
			}
			else
			{
				item->setOn(true);
			}
		}

		createSubCategoryView(item, node);
	}
}

void
CategoriesImageProperty::createSubCategoryView(CategoriesImagePropertyCategoryItem *root, CategoryNode *rootnode)
{
	QPtrList<CategoryNode> cat_list = rootnode->getChildCategoryList();
	for(CategoryNode *node = cat_list.first(); node; node=cat_list.next())
	{
		CategoriesImagePropertyCategoryItem *item = new CategoriesImagePropertyCategoryItem(root, node);
		item->setOpen(true);item->setOpen(false);
		item->setPixmap(0, BarIcon(node->getIcon(), 16 ));

		QVariant* num = (*numberOfImagesForCategory)[item->getId()];
		if(num)
		{
			if(num->toInt() != m_numerOfImages)
			{
				item->setState(QCheckListItem::NoChange);
				item->setTristate(true);
			}
			else
			{
				item->setOn(true);
			}
		}

		createSubCategoryView(item, node);
	}
}


QStringList
CategoriesImageProperty::getCheckedCategories(bool takeIntoAccountTristate)
{
	QStringList checkedCategories;
	CategoriesImagePropertyCategoryItem *item =
		(CategoriesImagePropertyCategoryItem *)(categoriesListView->firstChild()->itemBelow());
	while(item)
	{
		item->setOpen(true);
		if(
			item->state()==QCheckListItem::On ||
			(takeIntoAccountTristate && item->state()==QCheckListItem::QCheckListItem::NoChange)
		)
			checkedCategories.append(item->getId());
		item = (CategoriesImagePropertyCategoryItem *)(item->itemBelow());
	}
	return checkedCategories;
}

QStringList
CategoriesImageProperty::getAddedCategories()
{
	QStringList checkedCategories = getCheckedCategories();
	QStringList addedCategories;
	for ( QStringList::Iterator it = checkedCategories.begin(); it != checkedCategories.end(); ++it )
	{
		QVariant *num = (*numberOfImagesForCategory)[*it];
		if(!num)
			addedCategories.append(*it);
		else
		if(num->toInt()<m_numerOfImages)
			addedCategories.append(*it);
	}
	return addedCategories;
}

QStringList
CategoriesImageProperty::getRemovedCategories()
{
	QStringList checkedCategories = getCheckedCategories(true);
	QStringList removedCategories;
	QDictIterator<QVariant> it (*numberOfImagesForCategory);
	for ( ; it.current(); ++it )
	{
		if(!checkedCategories.contains(it.currentKey()))
			removedCategories.append(it.currentKey());
	}
	return removedCategories;
}

QString
CategoriesImageProperty::getName()
{
	return imagenameTextLabel->text();
}
QString
CategoriesImageProperty::getComment()
{
	if(commentsCheckBox->isChecked())
		return commentTextEdit->text();
	else
		return QString::null;
}
int
CategoriesImageProperty::getNote()
{
	if(noteCheckBox->isChecked())
		return noteSlider->value();
	else
		return -1;
}
QDateTime
CategoriesImageProperty::getDate_begin()
{
	if(datesCheckBox->isChecked())
		return beginDateWidget->date();
	else
		return QDateTime();
}
QDateTime
CategoriesImageProperty::getDate_end()
{
	if(datesCheckBox->isChecked())
		return endDateWidget->date();
	else
		return QDateTime();
}



/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CategoriesImageProperty::languageChange()
{
    commentsGroupBox->setTitle( i18n( "Comments" ) );
    commentsCheckBox->setText( i18n( "Change all comments" ) );
    categoriesGroupBox->setTitle( i18n( "Categories" ) );
    notesGroupBox->setTitle( i18n( "Note" ) );
    noteCheckBox->setText( i18n( "Change all notes" ) );
    datesGroupBox->setTitle( i18n( "Dates" ) );
    endTextLabel->setText( i18n( "End:" ) );
    beginTextLabel->setText( i18n( "Begin:" ) );
    datesCheckBox->setText( i18n( "Change all dates" ) );
    imagenameTextLabel->setText( i18n( "Imagename" ) );
}

QSize
CategoriesImageProperty::sizeHint () const
{
	return  QSize(492, 441);
}


#include "categoryimageproperties.moc"

#endif /* #ifdef WANT_LIBKEXIDB */
