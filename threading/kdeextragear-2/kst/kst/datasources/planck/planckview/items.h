/***************************************************************************
                           items.h 
    begin                : Fri Oct 24 2003
    copyright            : (C) 2003 The University of Toronto
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _ITEMS_H
#define _ITEMS_H

#include <qdom.h>
#include <qlistview.h>
#include <qmap.h>

class GroupItem : public QListViewItem {
    public:
	GroupItem(QListView *view) : QListViewItem(view) {}

	QMap<QString,QString> _attributes;

	GroupItem& operator<<(const QDomElement& e) {
	    const QDomNamedNodeMap& map = e.attributes();
	    for (uint i = 0; i < map.count(); ++i) {
		QDomAttr a = map.item(i).toAttr();
		_attributes[a.name()] = a.value();
	    }
	    setText(0, _attributes["name"]);
	    setText(1, _attributes["type"]);
	    return *this;
	}

	GroupItem& operator>>(QDomElement& e) {
	    for (QMap<QString,QString>::Iterator i = _attributes.begin(); i != _attributes.end(); ++i) {
		e.setAttribute(i.key(), i.data());
	    }
	    return *this;
	}
};

class ObjectItem : public QListViewItem {
    public:
	ObjectItem(GroupItem *grp) : QListViewItem(grp) {}

	QMap<QString,QString> _attributes;

	ObjectItem& operator<<(const QDomElement& e) {
	    const QDomNamedNodeMap& map = e.attributes();
	    for (uint i = 0; i < map.count(); ++i) {
		QDomAttr a = map.item(i).toAttr();
		_attributes[a.name()] = a.value();
	    }
	    setText(0, _attributes["name"]);
	    setText(1, _attributes["type"]);
	    setText(2, _attributes["datatype"]);
	    setText(3, _attributes["begin"]);
	    setText(4, _attributes["end"]);
	    return *this;
	}

	ObjectItem& operator>>(QDomElement& e) {
	    for (QMap<QString,QString>::Iterator i = _attributes.begin(); i != _attributes.end(); ++i) {
		e.setAttribute(i.key(), i.data());
	    }
	    return *this;
	}
};

#endif

// vim: ts=8 sw=4 noet
