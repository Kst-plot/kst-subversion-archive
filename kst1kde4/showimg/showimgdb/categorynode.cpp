/*****************************************************************************
                          categorynode.cpp  -  m_description
                             -------------------
    begin                : Sat Dec 1 2004
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

#include "categorynode.h"

#include <showimg/showimg_common.h>

CategoryNode::CategoryNode(unsigned int m_id, const QString& m_title, const QString& m_description, const QString& m_icon)
{
	this->m_id = m_id;
	setTitle(m_title);
	setDescription(m_description);
	setIcon(m_icon.isNull()||m_icon.isEmpty()?QString("kontact_mail"):m_icon);

	m_p_node_parent=NULL;
}

CategoryNode::~CategoryNode()
{
	CategoryNode *p=m_p_node_parent;
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
		.arg(m_id)
		.arg(m_title)
		.arg(m_description);
}

unsigned int
CategoryNode::getId() const
{
	return m_id;
}

unsigned int*
CategoryNode::getIdPtr()
{
	return &m_id;
}

QString
CategoryNode::getTitle()
{
	return m_title;
}


QString
CategoryNode::getDescription()
{
	return m_description;
}

void
CategoryNode::setTitle(const QString& a_title)
{
	m_title = a_title;
}

void
CategoryNode::setIcon(const QString& a_icon)
{
	m_icon = a_icon;
}
QString
CategoryNode::getIcon()
{
#warning CategoryNode::getIcon()
	return m_icon;
}

void
CategoryNode::setDescription(const QString& a_description)
{
	m_description=a_description;
}

QPtrList<unsigned int>
CategoryNode::getSubCategoryNumbersList()
{
	return m_subCategoryNumbersList;
}

QPtrList<CategoryNode>
CategoryNode::getSubCategoryList()
{
	return m_subCategoriesList;
}

QPtrList<CategoryNode>
CategoryNode::getChildCategoryList()
{
	return m_childCategoriesList;
}

void
CategoryNode::addChildCategory(CategoryNode *cat)
{
	m_childCategoriesList.append(cat);
	updateParentCategories(cat);
}

void
CategoryNode::updateParentCategories(CategoryNode *cat)
{
	m_subCategoriesList.append(cat);
	m_subCategoryNumbersList.append(cat->getIdPtr());
	cat->setParentCategory(this);

	CategoryNode *p=m_p_node_parent;
	while(p)
	{
		p->appendSubCategory(cat);
		p = p->getParent();
	}
}

void
CategoryNode::appendSubCategory(CategoryNode *cat)
{
	m_subCategoriesList.append(cat);
	m_subCategoryNumbersList.append(cat->getIdPtr());
}

void
CategoryNode::removeSubCategory(CategoryNode *cat)
{
	m_childCategoriesList.removeRef(cat);
	m_subCategoriesList.removeRef(cat);
	m_subCategoryNumbersList.removeRef(cat->getIdPtr());
}

void
CategoryNode::setParentCategory(CategoryNode *a_p_node_parent)
{
	m_p_node_parent = a_p_node_parent;
}

CategoryNode*
CategoryNode::getParent()
{
	return m_p_node_parent;
}

bool
CategoryNode::hasSubClasses()
{
	return m_subCategoriesList.count() != 0;
}

bool
CategoryNode::hasChildClasses()
{
	return m_childCategoriesList.count() != 0;
}

bool
CategoryNode::isLeafCategory()
{
	return !hasChildClasses();
}
