/***************************************************************************
                          categorynode.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "categorynode.h"

// KDE
#include <kdebug.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

CategoryNode::CategoryNode(unsigned int id, const QString& title, const QString& description, const QString& icon)
{
	this->id = id;
	setTitle(title);
	setDescription(description);
	setIcon(icon.isNull()||icon.isEmpty()?QString("kontact_mail"):icon);

	parent=NULL;
}

CategoryNode::~CategoryNode()
{
	CategoryNode *p=parent;
	while(p)
	{
		p->removeSubCategory(this);
		p = p->getParent();
	}
}

QString
CategoryNode::toString()
{
	return QString("%1 %2 %3")
		.arg(id)
		.arg(title)
		.arg(description);
}

unsigned int
CategoryNode::getId() const
{
	return id;
}

unsigned int*
CategoryNode::getIdPtr()
{
	return &id;
}

QString
CategoryNode::getTitle()
{
	return title;
}


QString
CategoryNode::getDescription()
{
	return description;
}

void
CategoryNode::setTitle(const QString& title)
{
	this->title = title;
}

void
CategoryNode::setIcon(const QString& icon)
{
	this->icon = icon;
}
QString
CategoryNode::getIcon()
{
	return icon;
}

void
CategoryNode::setDescription(const QString& description)
{
	this->description=description;
}

QPtrList<unsigned int>
CategoryNode::getSubCategoryNumbersList()
{
	return subCategoryNumbersList;
}

QPtrList<CategoryNode>
CategoryNode::getSubCategoryList()
{
	return subCategoriesList;
}

QPtrList<CategoryNode>
CategoryNode::getChildCategoryList()
{
	return childCategoriesList;
}

void
CategoryNode::addChildCategory(CategoryNode *cat)
{
	childCategoriesList.append(cat);
	updateParentCategories(cat);
}

void
CategoryNode::updateParentCategories(CategoryNode *cat)
{
	subCategoriesList.append(cat);
	subCategoryNumbersList.append(cat->getIdPtr());
	cat->setParentCategory(this);

	CategoryNode *p=parent;
	while(p)
	{
		p->appendSubCategory(cat);
		p = p->getParent();
	}
}

void
CategoryNode::appendSubCategory(CategoryNode *cat)
{
	subCategoriesList.append(cat);
	subCategoryNumbersList.append(cat->getIdPtr());
}

void
CategoryNode::removeSubCategory(CategoryNode *cat)
{
	childCategoriesList.removeRef(cat);
	subCategoriesList.removeRef(cat);
	subCategoryNumbersList.removeRef(cat->getIdPtr());
}

void
CategoryNode::setParentCategory(CategoryNode *parent)
{
	this->parent = parent;
}

CategoryNode*
CategoryNode::getParent()
{
	return parent;
}

bool
CategoryNode::hasSubClasses()
{
	return subCategoriesList.count() != 0;
}

bool
CategoryNode::hasChildClasses()
{
	return childCategoriesList.count() != 0;
}

bool
CategoryNode::isLeafCategory()
{
	return !hasChildClasses();
}
