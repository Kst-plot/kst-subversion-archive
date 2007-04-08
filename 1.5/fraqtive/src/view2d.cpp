/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "view2d.h"

#include <qimage.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ViewWidget::ViewWidget(bool progressive, QWidget* parent, const char* name) : QWidget(parent, name, Qt::WNoAutoErase),
	_generator(this, progressive),
	_visible(false),
	_enableTransform(false),
	_enablePreview(false),
	_dragMode(DRAG_NONE),
	_transform(false)
{
}

ViewWidget::~ViewWidget()
{
	_generator.abort();
}

void ViewWidget::enableTransform(bool transform)
{
	_enableTransform = transform;
}

void ViewWidget::enablePreview(bool preview)
{
	_enablePreview = preview;
	setMouseTracking(preview);
	if (!_enablePreview)
		emit previewVisible(false);
}

void ViewWidget::setVisible(bool visible)
{
	if (_visible != visible) {
		_visible = visible;
		if (_visible)
			_generator.addToQueue();
		else
			_generator.abort();
		update();
	}
}

void ViewWidget::setPosition(double px, double py, double zoom, double angle)
{
	if (_generator.setPosition(px, py, zoom, angle) && _visible)
		_generator.addToQueue();
}

void ViewWidget::setPreviewPosition(double, double, double zoom, double)
{
	if (_generator.setPosition(0.0, 0.0, zoom / 2.0, 0.0) && _visible)
		_generator.addToQueue();
}

void ViewWidget::setMandelbrotMode()
{
	if (_generator.setMandelbrotMode() && _visible)
		_generator.addToQueue();
}

void ViewWidget::setJuliaMode(double jx, double jy)
{
	if (_generator.setJuliaMode(jx, jy) && _visible)
		_generator.addToQueue();
}
	
void ViewWidget::setQuality(double maxIter, double threshold)
{
	if (_generator.setQuality(maxIter, threshold) && _visible)
		_generator.addToQueue();
}

void ViewWidget::setBackground(QRgb color)
{
	_generator.setBackground(color);
	 if (_visible)
	 	update();
}

void ViewWidget::setGradient(const QRgb* data, double scale, double offset)
{
	_generator.setGradient(data, scale, offset);
	 if (_visible)
	 	update();
}

void ViewWidget::copyImage()
{
	_generator.repaintImage();
	const QImage& image = _generator.getImage();

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setImage(image, QClipboard::Clipboard);
}

void ViewWidget::paintEvent(QPaintEvent* e)
{
	QPainter p(this);

	if (!_visible) {
		p.eraseRect(e->rect());
		return;
	}

	_generator.repaintImage();
	const QImage& image = _generator.getImage();
	int ix = image.width();
	int iy = image.height();
	int wx = width();
	int wy = height();
	if (_transform) {
		QImage transImage(wx, wy, 32);
		QRgb bg = p.backgroundColor().rgb();
		for (int y = 0; y < wy; y++) {
			QRgb* line = (QRgb*)transImage.scanLine(y);
			for (int x = 0; x < wx; x++) {
				int sx = (int)(_transX[0] * x + _transX[1] * y + _transX[2]);
				int sy = (int)(_transY[0] * x + _transY[1] * y + _transY[2]);
				if (sx >= 0 && sx < ix && sy >= 0 && sy < iy)
					line[x] = image.pixel(sx, sy);
				else
					line[x] = bg;
			}
		}
		p.drawImage(e->rect().topLeft(), transImage, e->rect());
	} else {
		p.drawImage(e->rect().topLeft(), image, e->rect());
		if (wx > ix)
			p.eraseRect(ix, 0, wx - ix, iy);
		if (wy > iy)
			p.eraseRect(0, iy, wx, wy - iy);
	}
}

void ViewWidget::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	_generator.setSize(size());
	if (_visible)
		_generator.addToQueue();
}

void ViewWidget::customEvent(QCustomEvent* e)
{
	if (e->type() == (int)GeneratorEvent::EventType) {
		GeneratorEvent* ge = (GeneratorEvent*)e;

		if (_visible) {
			if (ge->isFullUpdate()) {
				if (_dragMode == DRAG_NONE)
					_transform = false;
				repaint(false);
			} else {
				QRect rect;
				if (_generator.getUpdateRect(rect)) {
					if (_dragMode == DRAG_NONE)
						update(rect);
					else
						update();
				}
			}
		}

		emit updateProgress(ge->getProgress());
	}
}

void ViewWidget::mousePressEvent(QMouseEvent* e)
{
	if (!_enableTransform || !_visible || !_generator.isValid())
		return;

	if (_dragMode != DRAG_NONE) {
		_dragMode = DRAG_NONE;
		_transform = false;
		update();
		return;
	}

	if (e->button() == Qt::LeftButton) {
		if (e->state() & Qt::ShiftButton)
			_dragMode = DRAG_ROTATE;
		else if (e->state() & Qt::ControlButton)
			_dragMode = DRAG_ZOOM;
		else
			_dragMode = DRAG_ZOOM_IN;
	} else if (e->button() == Qt::RightButton)
		_dragMode = DRAG_MOVE;

	_startPos = e->pos();

	_lastOffset = 0;
	_lastOffsetX = 0;
	_lastOffsetY = 0;
}

void ViewWidget::mouseMoveEvent(QMouseEvent* e)
{
	switch (_dragMode) {
	case DRAG_NONE: {
		if (_enablePreview && _visible && _generator.isValid()) {
			double jx, jy;
			_generator.calcCoords(e->x(), e->y(), jx, jy);
			emit previewVisible(true);
			emit previewPosition(jx, jy);
		}
		break;
	}
	case DRAG_ZOOM_IN: {
		int offset = (e->pos() - _startPos).manhattanLength();
		if (offset != _lastOffset) {
			double zoom = std::pow(10.0, -offset / 256.0);
			_transX[0] = zoom; _transX[1] = 0.0; _transX[2] = (1.0 - zoom) * _startPos.x();
			_transY[0] = 0.0; _transY[1] = zoom; _transY[2] = (1.0 - zoom) * _startPos.y();
			_transform = true;
			repaint(false);
			_lastOffset = offset;
		}
		break;
	}
	case DRAG_ZOOM: {
		int offset = e->y() - _startPos.y();
		if (offset != _lastOffset) {
			double zoom = std::pow(10.0, -offset / 256.0);
			_transX[0] = zoom; _transX[1] = 0.0; _transX[2] = (1.0 - zoom) * width() / 2.0;
			_transY[0] = 0.0; _transY[1] = zoom; _transY[2] = (1.0 - zoom) * height() / 2.0;
			_transform = true;
			repaint(false);
			_lastOffset = offset;
		}
		break;
	}
	case DRAG_MOVE: {
		int offsetX = _startPos.x() - e->x();
		int offsetY = _startPos.y() - e->y();
		if (offsetX != _lastOffsetX || offsetY != _lastOffsetY) {
			_transX[0] = 1.0; _transX[1] = 0.0; _transX[2] = offsetX;
			_transY[0] = 0.0; _transY[1] = 1.0; _transY[2] = offsetY;
			_transform = true;
			repaint(false);
			_lastOffsetX = offsetX;
			_lastOffsetY = offsetY;
		}
		break;
	}
	case DRAG_ROTATE: {
		double px = width() / 2.0;
		double py = height() / 2.0;
		double startAngle = std::atan2(_startPos.x() - px, _startPos.y() - py);
		double currentAngle = std::atan2(e->x() - px, e->y() - py);
		double sa = std::sin(currentAngle - startAngle);
		double ca = std::cos(currentAngle - startAngle);
		_transX[0] = ca; _transX[1] = -sa; _transX[2] = (1.0 - ca) * px + sa * py;
		_transY[0] = sa; _transY[1] = ca; _transY[2] = (1.0 - ca) * py - sa * px;
		_transform = true;
		repaint(false);
		break;
	}
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
	switch (_dragMode) {
	case DRAG_NONE:
		break;
	case DRAG_ZOOM_IN: {
		int offset = (e->pos() - _startPos).manhattanLength();
		if (offset != 0) {
			double zoom = std::pow(10.0, -offset / 256.0);
			int bx = width();
			int by = height();
			double px = (bx / 2.0 - _startPos.x()) * zoom + _startPos.x();
			double py = (by / 2.0 - _startPos.y()) * zoom + _startPos.y();
			_generator.zoom(px, py, offset / 256.0);
			_generator.addToQueue();
			emitPositionChanged();
		} else {
			_transform = false;
			update();
		}
		break;
	}
	case DRAG_ZOOM: {
		int offset = e->y() - _startPos.y();
		if (offset != 0) {
			_generator.zoom(width() / 2.0, height() / 2.0, offset / 256.0);
			_generator.addToQueue();
			emitPositionChanged();
		} else {
			_transform = false;
			update();
		}
		break;
	}
	case DRAG_MOVE: {
		int offsetX = _startPos.x() - e->x();
		int offsetY = _startPos.y() - e->y();
		if (offsetX != 0 || offsetY != 0) {
			_generator.move(offsetX, offsetY);
			_generator.addToQueue();
			emitPositionChanged();
		} else {
			_transform = false;
			update();
		}
		break;
	}
	case DRAG_ROTATE: {
		double px = width() / 2.0;
		double py = height() / 2.0;
		double startAngle = std::atan2(_startPos.x() - px, _startPos.y() - py);
		double currentAngle = std::atan2(e->x() - px, e->y() - py);
		if (currentAngle != startAngle) {
			_generator.rotate((currentAngle - startAngle) * 180.0 / M_PI);
			_generator.addToQueue();
			emitPositionChanged();
		} else {
			_transform = false;
			update();
		}
		break;
	}
	}
	_dragMode = DRAG_NONE;
}

void ViewWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_enablePreview && _visible && _generator.isValid()) {
		double jx, jy;
		_generator.calcCoords(e->x(), e->y(), jx, jy);
		emit previewDoubleClick(jx, jy);
	}
}

void ViewWidget::enterEvent(QEvent*)
{
	if (_enablePreview && _visible)
		emit previewVisible(true);
}

void ViewWidget::leaveEvent(QEvent*)
{
	if (_enablePreview && _visible)
		emit previewVisible(false);
}

void ViewWidget::emitPositionChanged()
{
	emit positionChanged(_generator.getPosX(), _generator.getPosY(),
		_generator.getZoom(), _generator.getAngle());
}

#include "view2d.moc"
