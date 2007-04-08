/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "generator.h"
#include "genthread.h"
#include "gradient.h"

#include <qapplication.h>
#include <qgl.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <new>
#include <cmath>
#include <climits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Generator::Generator(QObject* receiver, bool progressive) :
	_receiver(receiver),
	_progressive(progressive),
	_dataChanged(false),
	_aborted(false),
	_validPos(false),
	_juliaMode(false),
	_maxIterations(0.0),
	_threshold(0.5),
	_size(0, 0),
	_bufferData(NULL),
	_backgroundColor(qRgb(0, 0, 0)),
	_gradientData(NULL),
	_finished(false),
	_invalidTop(0),
	_invalidBottom(0),
	_invalidTexture(false),
	_invalidDispList(false),
	_vertexArray(NULL),
	_textureArray(NULL),
	_texture(0),
	_displayList(0)
{
}

Generator::~Generator()
{
	delete[] _bufferData;
	delete[] _vertexArray;
	delete[] _textureArray;
	if (_texture)
		glDeleteTextures(1, &_texture);
	if (_displayList)
		glDeleteLists(_displayList, 1);
}

inline static double calcMandelbrot(double zx, double zy, int iter)
{
	double rx = zx;
	double ry = zy;
	double rad;

	double rx2 = rx * rx;
	double ry2 = ry * ry;

	int k = iter;
	do
	{
		double tx = rx2 - ry2 + zx;
		double rxy = rx * ry;
		ry = rxy + rxy + zy;
		rx = tx;
		rx2 = rx * rx;
		ry2 = ry * ry;
		rad = rx2 + ry2;
	} while (--k > 0 && rad < 64.0);

	if (k == 0)
		return 0.0;

	double n = (iter - k) - std::log(std::log(std::sqrt(rad))) / std::log(2.0);

	if (n < 0.0)
		return 0.0;

	return std::sqrt(n);
}

inline static double calcJulia(double zx, double zy, double jx, double jy, int iter)
{
	double rx = zx;
	double ry = zy;
	double rad;

	double rx2 = rx * rx;
	double ry2 = ry * ry;

	int k = iter;
	do
	{
		double tx = rx2 - ry2 + jx;
		double rxy = rx * ry;
		ry = rxy + rxy + jy;
		rx = tx;
		rx2 = rx * rx;
		ry2 = ry * ry;
		rad = rx2 + ry2;
	} while (--k > 0 && rad < 64.0);

	if (k == 0)
		return 0.0;

	double n = (iter - k) - std::log(std::log(std::sqrt(rad))) / std::log(2.0);

	if (n < 0.0)
		return 0.0;

	return std::sqrt(n);
}

void Generator::generate()
{
	_dataMutex.lock();

	if (!_dataChanged || _size.isEmpty() || !_validPos || _aborted) {
		_dataMutex.unlock();
		return;
	}

	int nx = _size.width();
	int ny = _size.height();

	double scale = std::pow(10.0, 0.4 - _zoom) / ny;

	double sa = scale * std::sin(_angle * M_PI / 180.0);
	double ca = scale * std::cos(_angle * M_PI / 180.0);

	double vx = (nx - 1) / 2.0;
	double vy = (ny - 1) / 2.0;

	double px = _posX - ca * vx + sa * vy;
	double py = _posY + ca * vy + sa * vx;

	bool juliaMode = _juliaMode;
	double jx = _juliaX;
	double jy = _juliaY;

	int iter = (int)(100.0 * (_maxIterations + _zoom));
	if (iter < 20)
		iter = 20;

	double threshold = _threshold;

	_dataChanged = false;

	bool mirrorMode = _juliaMode && !_progressive && _posX == 0.0 && _posY == 0.0;

	setFinished(false);

	if (_progressive)
		setBuffer(NULL, 0, 0, 0);
	if (_receiver)
		QApplication::postEvent(_receiver, new GeneratorEvent(false, 0));

	_dataMutex.unlock();

	int cx = ((nx + 3) & ~3) + 1;
	int cy = ((ny + 3) & ~3) + 1;
	double* buffer = new(std::nothrow) double[cx * cy];

	if (!buffer) {
		if (_receiver)
			QApplication::postEvent(_receiver, new GeneratorEvent(false, 100));
		return;
	}

	if (mirrorMode)
		cy = ((ny / 2 + 3) & ~3) + 1;

	for (int y = 0; y < cy; y += 4) {
		double* row = buffer + y * cx;
		for (int x = 0; x < cx; x += 4) {
			if (juliaMode)
				row[x] = calcJulia(px + ca * x - sa * y, py - sa * x - ca * y, jx, jy, iter);
			else
				row[x] = calcMandelbrot(px + ca * x - sa * y, py - sa * x - ca * y, iter);
		}

		for (int x = 0; x < cx - 4; x += 4) {
			double* cell = buffer + x + y * cx;
			double pl = cell[0];
			double pr = cell[4];
			if (pl == 0.0 || pr == 0.0) {
				cell[1] = cell[2] = pl; cell[3] = pr;
			} else {
				cell[1] = 0.75 * pl + 0.25 * pr;
				cell[2] = 0.5 * (pl + pr);
				cell[3] = 0.25 * pl + 0.75 * pr;
			}
		}

		if (y > 0) {
			for (int x = 0; x < cx; x++) {
				double* cell = buffer + x + (y - 4) * cx;
				double pt = cell[0];
				double pb = cell[4 * cx];
				if (pt == 0.0 || pb == 0.0) {
					cell[cx] = cell[2 * cx] = pt; cell[3 * cx] = pb;
				} else {
					cell[cx] = 0.75 * pt + 0.25 * pb;
					cell[2 * cx] = 0.5 * (pt + pb);
					cell[3 * cx] = 0.25 * pt + 0.75 * pb;
				}
			}

			if (_aborted) {
				delete[] buffer;
				return;
			}

			if ((y % 128) == 0 && y > 0 && _receiver) {
				QApplication::postEvent(_receiver,
					new GeneratorEvent(false, (y + 4) * 25 / cy));
			}
		}
	}

	if (_progressive)
		setBuffer(buffer, nx, ny, cx);
	if (_receiver)
		QApplication::postEvent(_receiver, new GeneratorEvent(_progressive, 25));

	for (int y = 0; y < cy - 4; y += 4) {
		for (int x = 0; x < cx - 4; x += 4) {
			double* cell = buffer + x + y * cx;
			double ptl = cell[0];
			double ptr = cell[4];
			double pbl = cell[4 * cx];
			double pbr = cell[4 * cx + 4];
			double mint, maxt, minb, maxb;
			if (ptl < ptr) {
				mint = ptl; maxt = ptr;
			} else {
				mint = ptr; maxt = ptl;
			}
			if (pbl < pbr) {
				minb = pbl; maxb = pbr;
			} else {
				minb = pbr; maxb = pbl;
			}
			double minCell = (mint < minb) ? mint : minb;
			double maxCell = (maxt > maxb) ? maxt : maxb;
			if (minCell == 0.0 && maxCell != 0.0 || (maxCell - minCell) > threshold) {
				for (int sy = 0; sy < 4; sy++) {
					for (int sx = 0; sx < 4; sx++) {
						if (sx != 0 || sy != 0) {
							if (juliaMode)
								cell[sy * cx + sx] = calcJulia(px + ca * (x + sx) - sa * (y + sy), py - sa * (x + sx) - ca * (y + sy), jx, jy, iter);
							else
								cell[sy * cx + sx] = calcMandelbrot(px + ca * (x + sx) - sa * (y + sy), py - sa * (x + sx) - ca * (y + sy), iter);
						}
					}
				}
			}
		}

		if (_aborted) {
			if (!_progressive)
				delete[] buffer;
			return;
		}

		if ((y % 64) == 0 && y > 0) {
			if (_progressive)
				invalidateImage(y - 64, y, false, true);
			if (_receiver) {
				QApplication::postEvent(_receiver,
					new GeneratorEvent(false, 25 + (y + 4) * 75 / cy));
			}
		}
	}

	if (mirrorMode) {
		for (int y = (ny + 1) / 2; y < ny; y++) {
			double* destRow = buffer + y * cx;
			double* srcRow = buffer + (ny - y - 1) * cx;
			for (int x = 0; x < nx; x++)
				destRow[x] = srcRow[nx - x - 1];
		}
	}

	if (_progressive)
		invalidateImage((cy - 4) & ~63, ny, false, true);

	if (!_progressive)
		setBuffer(buffer, nx, ny, cx);

	if (_receiver)
		QApplication::postEvent(_receiver, new GeneratorEvent(!_progressive, 100));

	setFinished(true);
}

void Generator::addToQueue()
{
	QMutexLocker locker(&_dataMutex);

	if (_aborted) {
		if (!_finished)
			_dataChanged = true;
		_aborted = false;
	}

	generatorThread.addToQueue(this, _progressive);
}

void Generator::abort()
{
	QMutexLocker locker(&_dataMutex);

	_aborted = true;
}

bool Generator::setPosition(double px, double py, double zoom, double angle)
{
	QMutexLocker locker(&_dataMutex);

	if (_validPos && _posX == px && _posY == py && _zoom == zoom && _angle == angle)
		return false;
	
	_validPos = true;
	_posX = px;
	_posY = py;
	_zoom = zoom;
	_angle = angle;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;

	return true;
}

bool Generator::setMandelbrotMode()
{
	QMutexLocker locker(&_dataMutex);

	if (!_juliaMode)
		return false;

	_juliaMode = false;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;

	return true;
}

bool Generator::setJuliaMode(double jx, double jy)
{
	QMutexLocker locker(&_dataMutex);

	if (_juliaMode && _juliaX == jx && _juliaY == jy)
		return false;

	_juliaMode = true;
	_juliaX = jx;
	_juliaY = jy;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;

	return true;
}

bool Generator::setQuality(double maxIter, double threshold)
{
	QMutexLocker locker(&_dataMutex);

	if (_maxIterations == maxIter && _threshold == threshold)
		return false;

	_maxIterations = maxIter;
	_threshold = threshold;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;

	return true;
}

bool Generator::setSize(const QSize& size)
{
	QMutexLocker locker(&_dataMutex);

	if (_size == size)
		return false;
	
	_size = size;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;

	return true;
}

void Generator::zoom(double centerX, double centerY, double zoom)
{
	QMutexLocker locker(&_dataMutex);

	double cx, cy;
	calcCoords(centerX, centerY, cx, cy);

	_posX = cx;
	_posY = cy;
	_zoom += zoom;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;
}

void Generator::move(double moveX, double moveY)
{
	QMutexLocker locker(&_dataMutex);

	double ox, oy;
	calcOffset(moveX, moveY, ox, oy);

	_posX = ox;
	_posY = oy;
	_dataChanged = true;
	if (_progressive)
		_aborted = true;
}

void Generator::rotate(double angle)
{
	QMutexLocker locker(&_dataMutex);

	_angle = std::fmod(_angle + angle, 360.0);
	if (_angle < 0.0)
		_angle += 360.0;

	_dataChanged = true;
	if (_progressive)
		_aborted = true;
}

void Generator::calcOffset(double ox, double oy, double& offsetX, double& offsetY) const
{
	double scale = std::pow(10.0, 0.4 - _zoom) / _size.height();
	double sa = scale * std::sin(_angle * M_PI / 180.0);
	double ca = scale * std::cos(_angle * M_PI / 180.0);

	offsetX = ca * ox - sa * oy + _posX;
	offsetY = -sa * ox - ca * oy + _posY;
}

void Generator::calcCoords(double px, double py, double& coordX, double& coordY) const
{
	calcOffset(px - _size.width() / 2.0, py - _size.height() / 2.0, coordX, coordY);
}

void Generator::setBackground(QRgb color)
{
	_backgroundColor = color;

	QMutexLocker locker(&_bufferMutex);
	invalidateImage(0, _bufferSize.height(), false, false);
}

void Generator::setGradient(const QRgb* data, double scale, double offset)
{
	_gradientData = data;
	_gradientScale = scale;
	_gradientOffset = offset;

	QMutexLocker locker(&_bufferMutex);
	invalidateImage(0, _bufferSize.height(), true, false);
}

bool Generator::isValid()
{
	QMutexLocker locker(&_bufferMutex);

	if (_progressive && _dataChanged)
		return false;

	if (!_bufferData || !_gradientData)
		return false;

	return true;
}

bool Generator::getUpdateRect(QRect& rect)
{
	QMutexLocker locker(&_bufferMutex);

	if (_progressive && _dataChanged)
		return false;

	if (!_bufferData || !_gradientData || _invalidTop >= _invalidBottom)
		return false;

	rect.setTop(_invalidTop);
	rect.setBottom(_invalidBottom);
	rect.setLeft(0);
	rect.setRight(_bufferSize.width());

	return true;
}

void Generator::repaintImage()
{
	QMutexLocker locker(&_bufferMutex);

	if (_progressive && _dataChanged)
		return;

	if (!_bufferData || !_gradientData)
		return;

	int top, bottom;
	{
		QMutexLocker invLocker(&_invalidMutex);

		if (_invalidTop >= _invalidBottom)
			return;

		top = _invalidTop;
		bottom = _invalidBottom;
		_invalidTop = 0;
		_invalidBottom = 0;
	}

	int bx = _bufferSize.width();
	int stride = _bufferStride;

	if (_image.isNull() || _image.size() != _bufferSize)
		_image.create(_bufferSize, 32);

	for (int y = top; y < bottom; y++) {
		QRgb* line = (QRgb*)_image.scanLine(y);
		for (int x = 0; x < bx; x++) {
			double col = _bufferData[x + y * stride];
			if (col == 0.0)
				line[x] = _backgroundColor;
			else
				line[x] = _gradientData[(int)(col * _gradientScale + _gradientOffset) % GRADIENT_LENGTH];
		}
	}
}

void Generator::paint3D()
{
	_dataMutex.lock();

	int iter = (int)(100.0 * (_maxIterations + _zoom));
	if (iter < 20)
		iter = 20;
	double maxD = std::sqrt((double)iter);

	_dataMutex.unlock();

	QMutexLocker locker(&_bufferMutex);

	glClearColor(qRed(_backgroundColor) / 255.0f, qGreen(_backgroundColor) / 255.0f, qBlue(_backgroundColor) / 255.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!_bufferData || !_gradientData)
		return;

	if (_progressive && _dataChanged)
		return;

	_invalidMutex.lock();

	if (_invalidTexture || _texture == 0) {
		if (_texture == 0)	
			glGenTextures(1, &_texture);

		QImage image(1024, 1, 32);
		QRgb* line = (QRgb*)image.scanLine(0);
		for (int x = 0; x < 1024; x++)
			line[x] = _gradientData[x * GRADIENT_LENGTH / 1024];
		QImage texImage = QGLWidget::convertToGLFormat(image);

		glBindTexture(GL_TEXTURE_1D, _texture);
		gluBuild1DMipmaps(GL_TEXTURE_1D, 4, 1024, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		_invalidTexture = false;
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslated(_gradientOffset / (double)GRADIENT_LENGTH, 0.0, 0.0);
	glScaled(_gradientScale, 1.0, 1.0);

	if (_invalidDispList || _displayList == 0) {
		_invalidDispList = false;
		_invalidMutex.unlock();

		int bx = _bufferSize.width();
		int by = _bufferSize.height();
		int stride = _bufferStride;
	
		double minD = 100.0;
		for (int y = 0; y < by; y++) {
			for (int x = 0; x < bx; x++) {
				double d = _bufferData[x + y * stride];
				if (d > 0.0 && d < minD)
					minD = d;
			}
		}

		delete[] _vertexArray;
		_vertexArray = new(std::nothrow) float[3 * by * bx];
		if (!_vertexArray)
			return;
		
		delete[] _textureArray;
		_textureArray = new(std::nothrow) float[by * bx];
		if (!_textureArray)
			return;

		int iv = 0, it = 0;

		for (int y = 0; y < by; y++) {
			float vy = 10.0f - (20.0f * y) / (by - 1);
			for (int x = 0; x < bx; x++) {
				double d = _bufferData[x + y * stride];
				if (d == 0.0)
					d = maxD;
				float vx = (20.0f * x) / (bx - 1) - 10.0f;
				float vz = (float)(minD - d);
				float tex = (float)d / (float)GRADIENT_LENGTH;
				_vertexArray[iv++] = vx;
				_vertexArray[iv++] = vy;
				_vertexArray[iv++] = vz;
				_textureArray[it++] = tex;
			}
		}

		if (_displayList == 0)
			_displayList = glGenLists(1);

		glNewList(_displayList, GL_COMPILE_AND_EXECUTE);

		glBindTexture(GL_TEXTURE_1D, _texture);
		glEnable(GL_TEXTURE_1D);

		double plane[4] = { 0.0, 0.0, 1.0, -(minD - maxD) - 0.001 };
	
		glClipPlane(GL_CLIP_PLANE0, plane);
		glEnable(GL_CLIP_PLANE0);

		glVertexPointer(3, GL_FLOAT, 0, _vertexArray);
		glEnableClientState(GL_VERTEX_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, _textureArray);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		int* indexArray = new int[2 * bx];
	
		for (int y = 0; y < by - 1; y++) {
			int ii = 0;
			for (int x = 0; x < bx; x++) {
				indexArray[ii++] = x + y * bx;
				indexArray[ii++] = x + (y + 1) * bx;
			}
			glDrawElements(GL_TRIANGLE_STRIP, 2 * bx, GL_UNSIGNED_INT, indexArray);
		}

		delete[] indexArray;

		glEndList();
	} else {
		_invalidMutex.unlock();

		glVertexPointer(3, GL_FLOAT, 0, _vertexArray);
		glEnableClientState(GL_VERTEX_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, _textureArray);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glCallList(_displayList);
	}
}

void Generator::freeData()
{
	if (!_progressive) {
		setBuffer(NULL, 0, 0, 0);
		_dataChanged = true;
	}

	_image.reset();

	delete[] _vertexArray;
	delete[] _textureArray;
	_vertexArray = NULL;
	_textureArray = NULL;
}

void Generator::setBuffer(double* data, int width, int height, int stride)
{
	QMutexLocker locker(&_bufferMutex);

	delete[] _bufferData;
	_bufferData = data;
	_bufferStride = stride;

	_bufferSize = QSize(width, height);

	QMutexLocker invLocker(&_invalidMutex);
	_invalidTop = 0;
	_invalidBottom = height;
}

void Generator::setFinished(bool finished)
{
	QMutexLocker locker(&_bufferMutex);

	_finished = finished;
}

void Generator::invalidateImage(int top, int bottom, bool texture, bool dispList)
{
	QMutexLocker locker(&_invalidMutex);

	if (_invalidTop >= _invalidBottom) {
		_invalidTop = top;
		_invalidBottom = bottom;
	} else {
		if (_invalidTop > top)
			_invalidTop = top;
		if (_invalidBottom < bottom)
			_invalidBottom = bottom;
	}

	if (texture)
		_invalidTexture = true;
	if (dispList)
		_invalidDispList = true;
}

#include "generator.moc"
