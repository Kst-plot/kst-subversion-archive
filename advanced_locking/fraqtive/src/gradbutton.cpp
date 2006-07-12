/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "gradbutton.h"
#include "gradient.h"

#include <qpainter.h>
#include <qstyle.h>
#include <qdrawutil.h>
#include <qapplication.h>

GradientButton::GradientButton(QWidget* parent, const char* name) : QPushButton(parent, name),
	_gradientData(NULL)
{
}

GradientButton::~GradientButton()
{
}

void GradientButton::setGradient(const QRgb* data, double, double)
{
	_gradientData = data;

	update();
}

void GradientButton::drawButtonLabel(QPainter* p)
{
	int x, y, w, h;
	QRect rectContents = style().subRect(QStyle::SR_PushButtonContents, this);
	rectContents.rect(&x, &y, &w, &h);

	int margin = style().pixelMetric(QStyle::PM_ButtonMargin, this);
	x += margin;
	y += margin;
	w -= 2 * margin;
	h -= 2 * margin;

	if (isOn() || isDown()) {
		x += style().pixelMetric(QStyle::PM_ButtonShiftHorizontal, this);
		y += style().pixelMetric(QStyle::PM_ButtonShiftVertical, this);
	}

	qDrawShadePanel(p, x, y, w, h, colorGroup(), true, 1, NULL);
	if (_gradientData) {
		for (int i = 0; i < w - 2; i++) {
			p->setPen(_gradientData[i * (GRADIENT_LENGTH - 1) / (w - 2)]);
			p->drawLine(x + i + 1, y + 1, x + i + 1, y + h - 2);
		}
	}

	if (hasFocus()) {
		QRect focusRect = style().subRect(QStyle::SR_PushButtonFocusRect, this);
		style().drawPrimitive(QStyle::PE_FocusRect, p, focusRect, colorGroup());
	}
}

QSize GradientButton::sizeHint() const
{
	return style().sizeFromContents(QStyle::CT_PushButton, this, QSize(40, 15)).
		expandedTo(QApplication::globalStrut());
}

#include "gradbutton.moc"
