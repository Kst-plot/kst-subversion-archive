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

#include "ksidebar.h"

#include <qwidgetstack.h>
#include <qlayout.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kmultitabbar.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

KSideBar::KSideBar(QWidget *parent, const char *name, Qt::Orientation /*orientation = Vertical*/, bool alwaysShowLabels /*= false*/) :
	QFrame(parent, name),
	//m_symbolTab(0L),
	m_nTabs(0),
	m_nCurrent(0),
	m_bMinimized(false),
	m_nMinSize(0),
	m_nMaxSize(1000),
	m_nSize(400)
{
	setFrameStyle(QFrame::Box|QFrame::Plain);
	setLineWidth(1);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	QLayout *layout=0L;
	layout = new QHBoxLayout(this);

	m_tabStack = new QWidgetStack(this);
	m_tabStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);


	KMultiTabBar::KMultiTabBarMode tabbarori = KMultiTabBar::Horizontal;
	KMultiTabBar::KMultiTabBarPosition tabbarpos = KMultiTabBar::Top;
	tabbarori = KMultiTabBar::Vertical;
	tabbarpos = KMultiTabBar::Right;
	m_tabBar = new KMultiTabBar(tabbarori, this);
	m_tabBar->setPosition(tabbarpos);
	m_tabBar->setStyle(alwaysShowLabels ? KMultiTabBar::KDEV3ICON : KMultiTabBar::VSNET);


	++m_nTabs;
	setMinimumWidth(m_tabBar->width());
	m_nMinSize = m_tabBar->width();
	m_nMaxSize = m_tabBar->maximumWidth();

	layout->add(m_tabBar);
	layout->add(m_tabStack);
}

KSideBar::~KSideBar()
{
}

int KSideBar::addTab(QWidget *tab, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	//m_isSymbolView[m_nTabs] = false;
	m_widgetToIndex[tab] = m_nTabs;

	m_tabBar->appendTab(pic, m_nTabs, text);
	m_tabStack->addWidget(tab, m_nTabs);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));

	showTab(m_nTabs);

	return m_nTabs++;
}

int KSideBar::addSymbolTab(int page, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	//m_widgetToIndex[m_symbolTab] = m_nTabs;
	//m_isSymbolView[m_nTabs] = true;
	m_indexToPage[m_nTabs] = page;

	m_tabBar->appendTab(pic, m_nTabs, text);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));

	return m_nTabs++;
}

void KSideBar::setVisible(bool show)
{
	kdDebug(0) << "==KSideBar::setVisible(" << show << ")===========" << endl;
	//if (show) expand();
	//else shrink();
}

void KSideBar::shrink()
{
	kdDebug(0) << "==KSideBar::shrink ===========" << endl;
	if ( !isVisible() ) return;
	kdDebug(0) << "\t==KSideBar::shrink ===========" << endl;

	m_bMinimized = true;

	m_nSize = width();
	m_nMinSize = minimumWidth();
	m_nMaxSize = maximumWidth();

	m_tabStack->hide();
	resize(m_tabBar->width(), height());
	setFixedWidth(m_tabBar->width());

	emit visibilityChanged(false);
}

void KSideBar::expand()
{
	kdDebug(0) << "==KSideBar::expand ===========" << endl;
	if ( isVisible() ) return;

	m_bMinimized = false;

	m_tabStack->show();

// 	resize(m_nSize, height());
	setMinimumWidth(m_nMinSize);
	setMaximumWidth(m_nMaxSize);

	emit visibilityChanged(true);
}

QWidget* KSideBar::currentPage()
{
	return m_tabStack->visibleWidget();
}

void KSideBar::showPage(QWidget *widget)
{
	if ( m_widgetToIndex.contains(widget) )
		switchToTab(m_widgetToIndex[widget]);
}

void KSideBar::showTab(int id)
{
	if ( id != m_nCurrent)
	{
		switchToTab(id);
		if (m_bMinimized) expand();
	}
	else
		toggleTab();
}

void KSideBar::removeTab(int id)
{
	m_tabBar->removeTab(id);
}

void KSideBar::toggleTab()
{
	//if (m_bMinimized) expand();
	//else shrink();
}

void KSideBar::switchToTab(int id)
{
	m_tabBar->setTab(m_nCurrent, false);
	m_tabBar->setTab(id, true);

	//int tabId = isSymbolView(id) ? 0 : id;
	//if ( isSymbolView(id) ) m_symbolTab->showPage(m_indexToPage[id]);

	//m_tabStack->raiseWidget(tabId);
	m_tabStack->raiseWidget(id);

	m_nCurrent = id;
}

KBottomBar::KBottomBar(QWidget *parent, const char *name) :
	KSideBar(parent, name, Qt::Horizontal, true)
{}

void KBottomBar::shrink()
{
	m_bMinimized = true;

	m_nSize = height();
	m_nMinSize = minimumHeight();
	m_nMaxSize = maximumHeight();

	m_tabStack->hide();
	setFixedHeight(m_tabBar->height());

	emit visibilityChanged(false);
}

void KBottomBar::expand()
{
	m_bMinimized = false;

	m_tabStack->show();

// 	resize(width(), m_nSize);
	setMinimumHeight(m_nMinSize);
	setMaximumHeight(m_nMaxSize);

	emit visibilityChanged(true);
}

#include "ksidebar.moc"
