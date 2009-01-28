/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "gradlist.h"
#include "gradient.h"

#include <qpainter.h>
#include <qdrawutil.h>

class GradientItem : public QListBoxText
{
public:
	static const int itemRtti = 1001;

	GradientItem(const QString& text, const Gradient& gradient) : QListBoxText(text),
		_gradient(gradient)
	{
	}

	int width(const QListBox* lb) const
	{
		return QListBoxText::width(lb) + 110;
	}

	int height(const QListBox* lb) const
	{
		return QListBoxText::height(lb) + 4;
	}
	
	int rtti() const
	{
		return itemRtti;
	}

	void setGradient(const Gradient& gradient)
	{
		_gradient = gradient;
	}

	const Gradient& getGradient() const
	{
		return _gradient;
	}

protected:
	void paint(QPainter* p)
	{
		int w = width(listBox());
		int h = height(listBox());

		p->drawText(110, 0, w - 110, h, Qt::AlignVCenter, text());

		p->setPen(Qt::black);
		p->setBrush(Qt::NoBrush);
		p->drawRect(4, 2, 102, h - 4);

		QRgb* buffer = new QRgb[100];
		_gradient.fillGradient(buffer, 100);

		for (int x = 0; x < 100; x++) {
			p->setPen(buffer[x]);
			p->drawLine(5 + x, 3, 5 + x, h - 4);
		}

		delete[] buffer;
	}

private:
	Gradient _gradient;
};

class SeparatorItem : public QListBoxText
{
public:
	static const int itemRtti = 1002;

	SeparatorItem(const QString& text) : QListBoxText(text)
	{
	}

	int width(const QListBox*) const
	{
		return 110;
	}

	int rtti() const
	{
		return itemRtti;
	}

protected:
	void paint(QPainter* p)
	{
		int w = listBox()->maxItemWidth();
		if (w < listBox()->width())
			w = listBox()->width();
		int h = height(listBox());

		QRect rcItem(0, 0, w, h);
		p->setBackgroundColor(listBox()->colorGroup().background());
		p->eraseRect(rcItem);

		qDrawShadeLine(p, 4, h / 2, w - 8, h / 2, listBox()->colorGroup(), false, 1, 0);

		p->setBackgroundMode(Qt::OpaqueMode);
		p->setPen(listBox()->colorGroup().buttonText());
		p->drawText(4, 0, 102, h, Qt::AlignCenter | Qt::AlignVCenter, " " + text() + " ");
	}
};

GradientList::GradientList(QWidget* parent, const char* name) : QListBox(parent, name)
{
	connect(this, SIGNAL(selectionChanged(QListBoxItem*)), SLOT(onItemSelected(QListBoxItem*)));
}

GradientList::~GradientList()
{
}

void GradientList::insertGradient(const QString& text, const Gradient& gradient)
{
	insertItem(new GradientItem(text, gradient));
}

void GradientList::insertSeparator(const QString& text)
{
	insertItem(new SeparatorItem(text));
}

void GradientList::setGradient(int index, const Gradient& gradient)
{
	QListBoxItem* listItem = item(index);
	if (listItem && listItem->rtti() == GradientItem::itemRtti) {
		GradientItem* gradItem = (GradientItem*)listItem;
		gradItem->setGradient(gradient);
		updateItem(index);
	}
}

void GradientList::onItemSelected(QListBoxItem* item)
{
	if (item->rtti() == GradientItem::itemRtti) {
		GradientItem* gradItem = (GradientItem*)item;
		emit gradientSelected(gradItem->getGradient());
	}
}

#include "gradlist.moc"
