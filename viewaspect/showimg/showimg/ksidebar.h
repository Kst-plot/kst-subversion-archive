/***************************************************************************
    begin                : Fri 18-06-2004
    copyright            : (C) 2004-2005 by Jeroen Wijnhout
                               2005-2005 Richard Groult <rgroult@jalix.org>
    email                :  Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSIDEBAR_H
#define KSIDEBAR_H

#include <qframe.h>
#include <qmap.h>

class QWidgetStack;
class KMultiTabBar;
//class SymbolView;

/**
@author Jeroen Wijnhout
*/
class KSideBar : public QFrame
{
	Q_OBJECT

public:
	KSideBar(QWidget *parent = 0, const char *name = 0, Qt::Orientation orientation = Qt::Vertical, bool alwaysShowLabels = false);
	~KSideBar();

	int addTab(QWidget *tab, const QPixmap &pic, const QString &text = QString::null);
	int addSymbolTab(int page, const QPixmap &pic, const QString &text = QString::null);

	int currentTab() { return m_nCurrent; }
	//SymbolView* symbolView() { return m_symbolTab; }

	bool isVisible() { return !m_bMinimized; }

	QWidget* currentPage();

signals:
	void visibilityChanged(bool );

public slots:
	void setVisible(bool );

	virtual void shrink();
	virtual void expand();

	void showTab(int);
	void removeTab(int);
	void showPage(QWidget *);
	void toggleTab();
	void switchToTab(int id);
	//bool isSymbolView(int id) { return m_isSymbolView[id]; }

protected:
	QWidgetStack		*m_tabStack;
	KMultiTabBar		*m_tabBar;
	//SymbolView		*m_symbolTab;
	int			m_nTabs;
	int			m_nCurrent;
	QMap<int,int>		m_indexToPage;
	QMap<QWidget*,int>	m_widgetToIndex;
	QMap<int,bool>		m_isSymbolView;
	bool			m_bMinimized;
	int			m_nMinSize, m_nMaxSize, m_nSize;
};

class KBottomBar : public KSideBar
{
	Q_OBJECT

public:
	KBottomBar(QWidget *parent = 0, const char *name = 0);

	void shrink();
	void expand();
};

#endif
