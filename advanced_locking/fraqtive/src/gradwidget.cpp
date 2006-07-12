/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "gradwidget.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qimage.h>

GradientWidget::GradientWidget(QWidget* parent, const char* name) : QWidget(parent, name, Qt::WNoAutoErase)
{
}

GradientWidget::~GradientWidget()
{
}

QSize GradientWidget::minimumSizeHint() const
{
	return QSize(200, 200);
}

QSize GradientWidget::sizeHint() const
{
	return QSize(400, 300);
}

void GradientWidget::setGradient(const Gradient& gradient)
{
	_gradient = gradient;
	calcRects();
	update();
}

void GradientWidget::newRgb()
{
	_gradient = Gradient(false);

	for (int i=0; i<3; i++) {
		_gradient.getSpline(i).addNode(0.0, 0.0);
		_gradient.getSpline(i).addNode(1.0, 1.0);
	}

	calcRects();
	update();
}

void GradientWidget::newHsv()
{
	_gradient = Gradient(true);

	_gradient.getSpline(0).addNode(0.0, 0.0);
	_gradient.getSpline(0).addNode(1.0, 1.0);
	_gradient.getSpline(1).addNode(0.5, 1.0);
	_gradient.getSpline(2).addNode(0.5, 0.5);

	calcRects();
	update();
}

void GradientWidget::invert()
{
	_gradient.invert();
	update();
}

void GradientWidget::paintEvent(QPaintEvent*)
{
	QPixmap pm(size());
	QPainter p;
	p.begin(&pm, this);

	p.eraseRect(rect());

	for (int i = 1; i <= 3; i++) {
		int lx = _gradRect.left() + _gradRect.width() * i / 4;
		qDrawShadeLine(&p, lx, 0, lx, height(), colorGroup(), true, 1, 1);
	}

	qDrawShadePanel(&p, _gradRect.left() - 1, _gradRect.top() - 1,
		_gradRect.width() + 2, _gradRect.height() + 2,
		colorGroup(), true, 1, NULL);

	for (int i = 0; i < 3; i++) {
		qDrawShadePanel(&p, _splineRect[i].left() - 1, _splineRect[i].top() - 1,
			_splineRect[i].width() + 2, _splineRect[i].height() + 2,
			colorGroup(), true, 1, NULL);
	}
	
	int length = _gradRect.width();
	QRgb* buffer = new QRgb[length];
	double* data = new double[length*3];

	_gradient.fillGradient(buffer, length, data);

	int x1 = _gradRect.left();
	int y1 = _gradRect.top();
	int y2 = _gradRect.bottom();

	for (int x = 0; x < length; x++) {
		p.setPen(buffer[x]);
		p.drawLine(x1 + x, y1, x1 + x, y2);
	}

	if (!_gradient.isHsv()) {
		y1 = _splineRect[0].top();
		y2 = _splineRect[0].bottom();
		for (int x = 0; x < length; x++) {
			p.setPen(qRgb(qRed(buffer[x]), 0, 0));
			p.drawLine(x1 + x, y1, x1 + x, y2);
		}
		y1 = _splineRect[1].top();
		y2 = _splineRect[1].bottom();
		for (int x = 0; x < length; x++) {
			p.setPen(qRgb(0, qGreen(buffer[x]), 0));
			p.drawLine(x1 + x, y1, x1 + x, y2);
		}
		y1 = _splineRect[2].top();
		y2 = _splineRect[2].bottom();
		for (int x = 0; x < length; x++) {
			p.setPen(qRgb(0, 0, qBlue(buffer[x])));
			p.drawLine(x1 + x, y1, x1 + x, y2);
		}
	} else {
		QImage image(_splineRect[0].size(), 32);
		y1 = _splineRect[0].top();
		y2 = _splineRect[0].bottom();
		int h = _splineRect[0].height();
		for (int x = 0; x < length; x++) {
			QRgb color = Gradient::hsvToRgb(data[x], 1.0, 0.5);
			for (int y = 0; y < h; y++)
				((QRgb*)image.scanLine(y))[x] = color;
		}
		p.drawImage(x1, y1, image);

		image.create(_splineRect[1].size(), 32);
		y1 = _splineRect[1].top();
		h = _splineRect[1].height() - 1;
		for (int x = 0; x < length; x++) {
			int y;
			for (y = 0; y < h; y += 2) {
				QRgb color = Gradient::hsvToRgb(data[x], 1.0 - (double)y / (double)h, 0.5);
				((QRgb*)image.scanLine(y))[x] = color;
				((QRgb*)image.scanLine(y + 1))[x] = color;
			}
			if (y == h) {
				QRgb color = Gradient::hsvToRgb(data[x], 1.0 - (double)y / (double)h, 0.5);
				((QRgb*)image.scanLine(y))[x] = color;
			}
		}
		p.drawImage(x1, y1, image);

		image.create(_splineRect[2].size(), 32);
		y1 = _splineRect[2].top();
		h = _splineRect[2].height();
		for (int x = 0; x < length; x++) {
			for (int y = 0; y < h; y++) {
				QRgb color = Gradient::hsvToRgb(data[x], data[x + length], 1.0 - (double)y / (double)h);
				((QRgb*)image.scanLine(y))[x] = color;
			}
		}
		p.drawImage(x1, y1, image);
	}

	QPen dottedPen(Qt::white, 0, Qt::DotLine);
	p.setPen(dottedPen);

	p.setBackgroundMode(Qt::OpaqueMode);
	p.setBackgroundColor(Qt::black);

	QPointArray arr(length);
	for (int i = 0; i < 3; i++) {
		y1 = _splineRect[i].top();
		int h = _splineRect[i].height() - 1;
		for (int x = 0; x < length; x++) {
			arr.setPoint(x, x1+x, y1 + (int)((1.0 - data[i*length + x]) * h));
		}
		p.drawPolyline(arr);
	}

	p.setPen(Qt::black);
	p.setBrush(Qt::white);

	for (int i = 0; i < 3; i++) {
		y1 = _splineRect[i].top();
		int h = _splineRect[i].height() - 1;
		const Spline& spline = _gradient.getSpline(i);
		for (int node = 0; node < spline.getNodesCnt(); node++) {
			int x = (int)(spline.getNodeX(node) * (length-1));
			int y = (int)((1.0 - spline.getNodeY(node)) * h);
			p.drawRect(x1 + x - 4, y1 + y - 4, 9, 9);
		}
	}

	delete[] buffer;
	delete[] data;

	p.end();
	bitBlt(this, 0, 0, &pm);
}

void GradientWidget::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);
	calcRects();
}

void GradientWidget::mousePressEvent(QMouseEvent* e)
{
	for (int i = 0; i < 3; i++) {
		QRect rectCheck(_splineRect[i].left() - 5, _splineRect[i].top() - 5,
			_splineRect[i].width() + 10, _splineRect[i].height() + 10);
		if (rectCheck.contains(e->pos())) {
			_dragSpline = i;
			break;
		}
	}

	if (_dragSpline < 0)
		return;

	Spline& spline = _gradient.getSpline(_dragSpline);

	QPoint pt(e->x() - _splineRect[_dragSpline].left(), e->y() - _splineRect[_dragSpline].top());
	QSize size(_splineRect[_dragSpline].size());

	_dragNode = spline.findNode(size, pt, QSize(5, 5));

	if (_dragNode < 0 && _splineRect[_dragSpline].contains(e->pos())) {
		_dragNode = spline.insertNode((double)pt.x() / (double)size.width(),
			1.0 - (double)pt.y() / (double)size.height());
	}

	if (_dragNode < 0)
		_dragSpline = -1;
	else {
		_dragRemove = false;
		mouseMoveEvent(e);
	}
}

void GradientWidget::mouseReleaseEvent(QMouseEvent*)
{
	_dragSpline = -1;
}

void GradientWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (_dragSpline < 0)
		return;

	QRect rectCheck(_splineRect[_dragSpline].left() - 40, _splineRect[_dragSpline].top() - 40,
		_splineRect[_dragSpline].width() + 80, _splineRect[_dragSpline].height() + 80);

	if (!_dragRemove) {
		if (_gradient.getSpline(_dragSpline).getNodesCnt() > 1 && !rectCheck.contains(e->pos())) {
			_dragRemove = true;
			_gradient.getSpline(_dragSpline).removeNode(_dragNode);
		}
	} else {
		if (rectCheck.contains(e->pos())) {
			_dragRemove = false;
			_gradient.getSpline(_dragSpline).insertAt(_dragNode);
		} else
			return;
	}

	if (!_dragRemove) {
		double x = (double)(e->x() - _splineRect[_dragSpline].left()) / (double)_splineRect[_dragSpline].width();
		double y = 1.0 - (double)(e->y() - _splineRect[_dragSpline].top()) / (double)_splineRect[_dragSpline].height();

		double minX = _gradient.getSpline(_dragSpline).getLimitMin(_dragNode);
		double maxX = _gradient.getSpline(_dragSpline).getLimitMax(_dragNode);

		if (x < minX)
			x = minX;
		else if (x > maxX)
			x = maxX;

		if (y < 0.0)
			y = 0.0;
		else if (y > 1.0)
			y = 1.0;

		_gradient.getSpline(_dragSpline).setNode(_dragNode, x, y);
	}

	update();
}

void GradientWidget::calcRects()
{
	QRect baseRect(5, 5, width() - 10, height() - 10);

	_gradRect = baseRect;
	_gradRect.setHeight(50);

	for (int i=0; i<3; i++)
		_splineRect[i] = baseRect;

	if (!_gradient.isHsv()) {
		int h = (baseRect.height() - 50 - 30) / 3;
		for (int i=0; i<3; i++) {
			_splineRect[i].setTop(_gradRect.bottom() + i * (h + 10) + 10);
			_splineRect[i].setHeight(h);
		}
	} else {
		int h = (baseRect.height() - 50 - 30) / 4;
		_splineRect[0].setTop(_gradRect.bottom() + 10);
		_splineRect[0].setHeight(2*h);
		_splineRect[1].setTop(_splineRect[0].bottom() + 10);
		_splineRect[1].setHeight(h);
		_splineRect[2].setTop(_splineRect[1].bottom() + 10);
		_splineRect[2].setHeight(h);
	}
}

#include "gradwidget.moc"
