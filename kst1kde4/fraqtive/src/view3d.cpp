/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "view3d.h"

#include <qapplication.h>
#include <qclipboard.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

View3DWidget::View3DWidget(QWidget* parent, const char* name) : QGLWidget(parent, name),
	_generator(this, true),
	_visible(false),
	_heightScale(1.0)
{
	_generator.setSize(QSize(128, 128));
}

View3DWidget::~View3DWidget()
{
	_generator.abort();
}

void View3DWidget::setVisible(bool visible)
{
	if (_visible != visible) {
		_visible = visible;
		if (_visible)
			_generator.addToQueue();
		else
			_generator.abort();
	}
}

void View3DWidget::setPosition(double px, double py, double zoom, double angle)
{
	if (_generator.setPosition(px, py, zoom - std::log10(2.0), angle) && _visible)
		_generator.addToQueue();
}

void View3DWidget::setMandelbrotMode()
{
	if (_generator.setMandelbrotMode() && _visible)
		_generator.addToQueue();
}

void View3DWidget::setJuliaMode(double jx, double jy)
{
	if (_generator.setJuliaMode(jx, jy) && _visible)
		_generator.addToQueue();
}

void View3DWidget::setQuality(double maxIter, double threshold)
{
	if (_generator.setQuality(maxIter, threshold) && _visible)
		_generator.addToQueue();
}

void View3DWidget::setBackground(QRgb color)
{
	_generator.setBackground(color);
	 if (_visible)
	 	updateGL();
}

void View3DWidget::setGradient(const QRgb* data, double scale, double offset)
{
	_generator.setGradient(data, scale, offset);
	 if (_visible)
	 	updateGL();
}

void View3DWidget::setGridSize(int width, int height)
{
	if (_generator.setSize(QSize(width, height)) && _visible)
		_generator.addToQueue();
}

void View3DWidget::setHeightScale(double scale)
{
	if (_heightScale != scale) {
		_heightScale = scale;
		if (_visible)
			updateGL();
	}
}

void View3DWidget::copyImage()
{
	QImage image = grabFrameBuffer();

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setImage(image, QClipboard::Clipboard);
}

void View3DWidget::initializeGL()
{
	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetDoublev(GL_MODELVIEW_MATRIX, _matrix);
}

void View3DWidget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	_aspect = (double)width / (double)height;
}

void View3DWidget::paintGL()
{
	double clipY = 5.0;
	double dist = 15.0;
	double nearZ = 10.0;
	double farZ = 20.0;

	double fy = clipY - (nearZ * clipY) / dist;
	double fx = _aspect * fy;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-fx, fx, -fy, fy, dist - nearZ, dist + farZ);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslated(0.0, 0.0, -dist);
	glMultMatrixd(_matrix);
	glScaled(1.0, 1.0, _heightScale);

	_generator.paint3D();
}

void View3DWidget::mousePressEvent(QMouseEvent* e)
{
	_lastPos = e->pos();
}

void View3DWidget::mouseMoveEvent(QMouseEvent* e)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (e->state() & Qt::RightButton) {
		QPoint center(width() / 2, height() / 2);
		QPoint oldVec = _lastPos - center;
		QPoint newVec = e->pos() - center;
		double oldAngle = std::atan2((double)oldVec.x(), (double)oldVec.y());
		double newAngle = std::atan2((double)newVec.x(), (double)newVec.y());
		double rotZ = (newAngle - oldAngle) * 180.0 / M_PI;
		glRotated(rotZ, 0.0, 0.0, 1.0);
	} else {
		double rotX = 0.4 * (e->y() - _lastPos.y());
		double rotY = 0.4 * (e->x() - _lastPos.x());
		glRotated(rotX, 1.0, 0.0, 0.0);
		glRotated(rotY, 0.0, 1.0, 0.0);
	}

	glMultMatrixd(_matrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, _matrix);

	_lastPos = e->pos();

	updateGL();
}

void View3DWidget::customEvent(QCustomEvent* e)
{
	if (e->type() == (int)GeneratorEvent::EventType) {
		GeneratorEvent* ge = (GeneratorEvent*)e;

		if (_visible && _generator.isValid())
			updateGL();

		emit updateProgress(ge->getProgress());
	}
}

#include "view3d.moc"
