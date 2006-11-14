/*****************************************************************************
                         categorynode.h  -  m_description
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

#ifndef CATEGORYNODE_H
#define CATEGORYNODE_H

#include <qstring.h>
#include <qptrlist.h>


class CategoryNode
{
public:
	CategoryNode(unsigned int m_id, const QString& m_title, const QString& m_description=QString::null, const QString& icon=QString::null);
	virtual ~CategoryNode();

	QString toString();

	unsigned int getId() const;

	QString getTitle();
	void setTitle(const QString& m_title);

	QString getDescription();
	void setDescription(const QString& m_description);

	void setIcon(const QString& icon);
	QString getIcon();

	CategoryNode* getParent();

	void addChildCategory(CategoryNode *cat);

	bool hasSubClasses();
	bool hasChildClasses();
	bool isLeafCategory();

	QPtrList<unsigned int> getSubCategoryNumbersList();
	QPtrList<CategoryNode> getSubCategoryList();
	QPtrList<CategoryNode> getChildCategoryList();

	bool operator!= ( const CategoryNode & n ) const {return getId()!=n.getId();}
	bool operator== ( const CategoryNode & n ) const {return getId()==n.getId();}

protected:
	unsigned int* getIdPtr();
	void setParentCategory(CategoryNode *m_p_node_parent);

	void updateParentCategories(CategoryNode *cat);
	void appendSubCategory(CategoryNode *cat);
	void removeSubCategory(CategoryNode *cat);

private :
	unsigned int 
		m_id;

	CategoryNode 
		* m_p_node_parent;

	QString 
		m_title,
		m_description,
		m_icon;

	QPtrList<CategoryNode> 
		m_childCategoriesList,
		m_subCategoriesList;
		
	QPtrList<unsigned int> 
		m_subCategoryNumbersList;
};

#endif
