/*****************************************************************************
    begin                : Fri 18-06-2004
    copyright            : (C) 2004-2005 by Jeroen Wijnhout
                               2005-2006 Richard Groult <rgroult@jalix.org>
    email                :  Jeroen.Wijnhout@kdemail.net
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

#include "ksidebar.h"

#include "showimg_common.h"

#include <qwidgetstack.h>
#include <qlayout.h>

#include <kdeversion.h>
#include <kdebug.h>

#if KDE_IS_VERSION(3,3,0)
#include <kmultitabbar.h>
#endif


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


#if KDE_IS_VERSION(3,3,0)
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
#endif
	layout->add(m_tabStack);
}

KSideBar::~KSideBar()
{
}

int KSideBar::addTab(QWidget *tab, const QPixmap &pic, const QString &text /* = QString::null*/)
{
#if KDE_IS_VERSION(3,3,0)
	//m_isSymbolView[m_nTabs] = false;
	m_widgetToIndex[tab] = m_nTabs;

	m_tabBar->appendTab(pic, m_nTabs, text);
	m_tabStack->addWidget(tab, m_nTabs);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));

	showTab(m_nTabs);
#endif
	return m_nTabs++;
}

int KSideBar::addSymbolTab(int page, const QPixmap &pic, const QString &text /* = QString::null*/)
{
	m_indexToPage[m_nTabs] = page;

#if KDE_IS_VERSION(3,3,0)
	m_tabBar->appendTab(pic, m_nTabs, text);
	connect(m_tabBar->tab(m_nTabs), SIGNAL(clicked(int)), this, SLOT(showTab(int)));
#endif

	return m_nTabs++;
}

void KSideBar::setVisible(bool show)
{
	MYDEBUG << "==KSideBar::setVisible(" << show << ")===========" << endl;
}

void KSideBar::shrink()
{
	MYDEBUG << "==KSideBar::shrink ===========" << endl;
	if ( !isVisible() ) return;
	MYDEBUG << "\t==KSideBar::shrink ===========" << endl;

	m_bMinimized = true;

	m_nSize = width();
	m_nMinSize = minimumWidth();
	m_nMaxSize = maximumWidth();

	m_tabStack->hide();
#if KDE_IS_VERSION(3,3,0)
	resize(m_tabBar->width(), height());
	setFixedWidth(m_tabBar->width());
#endif

	emit visibilityChanged(false);
}

void KSideBar::expand()
{
	MYDEBUG << "==KSideBar::expand ===========" << endl;
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
}

void KSideBar::removeTab(int id)
{
#if KDE_IS_VERSION(3,3,0)
	m_tabBar->removeTab(id);
#endif
}

void KSideBar::toggleTab()
{
}

void KSideBar::switchToTab(int id)
{
#if KDE_IS_VERSION(3,3,0)
	m_tabBar->setTab(m_nCurrent, false);
	m_tabBar->setTab(id, true);
#endif

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

#if KDE_IS_VERSION(3,3,0)
	m_tabStack->hide();
	setFixedHeight(m_tabBar->height());
#endif

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
