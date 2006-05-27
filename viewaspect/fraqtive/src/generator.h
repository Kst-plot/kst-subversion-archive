/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qobject.h>
#include <qmutex.h>
#include <qevent.h>
#include <qimage.h>

/*!
* \brief The fractal generator.
*
* This class is thread-safe. It can be used with QGeneratorThread for processing
* data asynchronously.
*/
class Generator : public QObject
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*
	* \param receiver    The object to send events to.
	* \param progressive Partially update buffer and image during generation.
	*/
	Generator(QObject* receiver = NULL, bool progressive = false);

	/*!
	* \brief Destructor.
	*/
	~Generator();

public:
	/*!
	* \brief The fractal generation routine.
	*
	* This procedure is called asynchronously by the generator thread.
	*/
	void generate();

	/*!
	* \brief Add the generator to the generator thread's queue.
	*/
	void addToQueue();

	/*!
	* \brief Abort processing the fractal.
	*/
	void abort();

	/*!
	* \brief Check if the generator data was changed since current generation.
	*
	* \return \c true if the data was changed, \c false otherwise.
	*/
	bool isChanged() const
	{
		return _dataChanged;
	}

	/*!
	* \brief Get the current x coordinate of the fractal's position.
	*
	* \return The value of the x coordinate.
	*/
	double getPosX() const
	{
		return _posX;
	}

	/*!
	* \brief Get the current y coordinate of the fractal's position.
	*
	* \return The value of the y coordinate.
	*/
	double getPosY() const
	{
		return _posY;
	}

	/*!
	* \brief Get the current zoom factor of the fractal.
	*
	* \return The value of the zoom factor.
	*/
	double getZoom() const
	{
		return _zoom;
	}

	/*!
	* \brief Get the current rotation angle of the fractal.
	*
	* \return The value of the rotation angle.
	*/
	double getAngle() const
	{
		return _angle;
	}

	/*!
	* \brief Set the position of the fragment of the fractal to generate.
	*
	* \param px    The x coordinate of the center of the fractal.
	* \param py    The y coordinate of the center of the fractal.
	* \param zoom  The zoom factor (as a decimal logarithm).
	* \param angle The rotation angle (in degrees).
	*/
	bool setPosition(double px, double py, double zoom, double angle);

	/*!
	* \brief Generate the Mandelbrot fractal.
	*/
	bool setMandelbrotMode();

	/*!
	* \brief Generate a Julia fractal.
	*
	* \param jx The x coordinate of the Julia parameter.
	* \param jy The y coordinate of the Julia parameter.
	*/
	bool setJuliaMode(double jx, double jy);

	/*!
	* \brief Set maximum iterations and precision.
	*
	* \param maxIter   Maximum number of iterations (as decimal logarithm).
	* \param threshold Minimum threshold for calculating fractal details.
	*/
	bool setQuality(double maxIter, double threshold);

	/*!
	* \brief Set the size of the fractal to generate.
	*
	* \param size New size of the fractal.
	*/
	bool setSize(const QSize& size);

	/*!
	* \brief Zoom the fragment of the fractal in or out.
	*
	* \param centerX The x coordinate of the reference point.
	* \param centerY The y coordinate of the reference point.
	* \param zoom    The zoom factor (as a decimal logarithm).
	*/
	void zoom(double centerX, double centerY, double zoom);

	/*!
	* \brief Move the position of the fragment of the fractal.
	*
	* \param moveX The x offset.
	* \param moveY The y offset.
	*/
	void move(double moveX, double moveY);

	/*!
	* \brief Rotate the fragment of the fractal.
	*
	* \param angle The rotation angle (in degrees).
	*/
	void rotate(double angle);

	/*!
	* \brief Calculate the zoomed and rotated offset of the fractal's position.
	*
	* \param ox           The x offset of the position.
	* \param oy           The y offset of the position.
	* \param[out] offsetX The transformed x offset.
	* \param[out] offsetY The transformed y offset.
	*/
	void calcOffset(double ox, double oy, double& offsetX, double& offsetY) const;

	/*!
	* \brief Calculate the zoomed and rotated coordinate of the fractal.
	*
	* \param px          The x coordinate.
	* \param py          The y coordinate.
	* \param[out] coordX The transformed x coordinate.
	* \param[out] coordY The transformed y coordinate.
	*/
	void calcCoords(double px, double py, double& coordX, double& coordY) const;

	/*!
	* \brief Set the background color.
	*
	* The background color is used for drawing areas inside the fractal.
	*
	* \param color The new background color.
	*/
	void setBackground(QRgb color);

	/*!
	* \brief Set the gradient of colors.
	*
	* \param data   The array of gradient colors.
	* \param scale  The scale of the gradient.
	* \param offset The offset of the gradient.
	*/
	void setGradient(const QRgb* data, double scale, double offset);

	/*!
	* \brief Check if the fractal data is valid and up-to-date.
	*
	* \return \c true if the data is valid, \c false otherwise.
	*/
	bool isValid();

	/*!
	* \brief Check if the fractal image has to be updated.
	*
	* \param[out] rect The area that needs to be updated.
	* \return \c true if the update is needed, \c false otherwise.
	*/
	bool getUpdateRect(QRect& rect);

	/*!
	* \brief Repaint the area of the image that needs to be updated.
	*/
	void repaintImage();

	/*!
	* \brief Get the fractal image.
	*
	* \return Reference to the image object.
	*/
	const QImage& getImage() const
	{
		return _image;
	}

	/*!
	* \brief Paint the 3D view of the fractal in the current GL context.
	*/
	void paint3D();

	/*!
	* \brief Free buffer and image data.
	*/
	void freeData();

private:
	void setBuffer(double* data, int width, int height, int stride);
	void setFinished(bool finished);
	void invalidateImage(int top, int bottom, bool texture, bool dispList);

private:
	QObject* _receiver;
	bool _progressive;

	// input data for the generator
	QMutex _dataMutex;
	bool _dataChanged;
	bool _aborted;
	bool _validPos;
	double _posX, _posY;
	double _zoom;
	double _angle;
	bool _juliaMode;
	double _juliaX, _juliaY;
	double _maxIterations;
	double _threshold;
	QSize _size;

	// generated data and gradient
	QMutex _bufferMutex;
	double* _bufferData;
	int _bufferStride;
	QSize _bufferSize;
	QRgb _backgroundColor;
	const QRgb* _gradientData;
	double _gradientScale;
	double _gradientOffset;
	bool _finished;

	// image/display list state
	QMutex _invalidMutex;
	int _invalidTop;
	int _invalidBottom;
	bool _invalidTexture;
	bool _invalidDispList;

	// image and GL data
	QImage _image;
	float* _vertexArray;
	float* _textureArray;
	unsigned int _texture;
	unsigned int _displayList;
};

/*!
* \brief The generator event.
*
* This event is sent to notify about generation progress and image updates.
*/
class GeneratorEvent : public QCustomEvent
{
public:
	enum {
		EventType = QEvent::User
	};

	/*!
	* \brief Constructor.
	*
	* \param fullUpdate \c true if image update is required, \c false otherwise.
	* \param progress   the progress of the generation (0 - 100).
	*/
	GeneratorEvent(bool fullUpdate, int progress) : QCustomEvent(EventType),
		_fullUpdate(fullUpdate),
		_progress(progress)
	{
	}

	/*!
	* \brief Check if full image update is required.
	*
	* Full image update is required if new data was generated.
	*
	* \return \c true if full image update is required, \c false otherwise.
	*/
	bool isFullUpdate() const
	{
		return _fullUpdate;
	}

	/*!
	* \brief Get the current progress of the fractal generation.
	*
	* \return The current progress (0 - 100).
	*/
	int getProgress() const
	{
		return _progress;
	}

private:
	bool _fullUpdate;
	int _progress;
};

#endif
