/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef VIEW3D_H
#define VIEW3D_H

#include "generator.h"

#include <qgl.h>

/*!
* \brief The 3D fractal view wigdet.
*/
class View3DWidget : public QGLWidget
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*/
	View3DWidget(QWidget* parent=0, const char* name=0);

	/*!
	* \brief Destructor.
	*/
	~View3DWidget();

public slots:
	/*!
	* \brief Enable or disable rendering the fractal.
	*
	* \param visible \c true if the view is enabled, \c false otherwise.
	*/
	void setVisible(bool visible);

	/*!
	* \brief Set the position of the fragment of the fractal to render.
	*
	* \param px    The x coordinate of the center of the fractal.
	* \param py    The y coordinate of the center of the fractal.
	* \param zoom  The zoom factor (as a decimal logarithm).
	* \param angle The rotation angle (in degrees).
	*/
	void setPosition(double px, double py, double zoom, double angle);

	/*!
	* \brief View the Mandelbrot fractal.
	*/
	void setMandelbrotMode();

	/*!
	* \brief View a Julia fractal.
	*
	* \param jx The x coordinate of the Julia parameter.
	* \param jy The y coordinate of the Julia parameter.
	*/
	void setJuliaMode(double jx, double jy);

	/*!
	* \brief Set maximum iterations and precision.
	*
	* \param maxIter   Maximum number of iterations (as decimal logarithm).
	* \param threshold Minimum threshold for calculating fractal details.
	*/
	void setQuality(double maxIter, double threshold);

	/*!
	* \brief Set the background color.
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
	* \brief Set the grid size of the 3D mesh.
	*
	* \param width  The horizontal size of the grid.
	* \param height The vertical size of the grid.
	*/
	void setGridSize(int width, int height);

	/*!
	* \brief Set the height scale for the view.
	*
	* \param scale The new height scale.
	*/
	void setHeightScale(double scale);

	/*!
	* \brief Copy image to clipboard.
	*/
	void copyImage();

signals:
	/*!
	* \brief Set the progress of the fractal generator.
	*
	* \param progress Current progress.
	*/
	void updateProgress(int progress);

protected:
	// GL opetations
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	// input events
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);

	// generator event
	void customEvent(QCustomEvent* e);

private:
	Generator _generator;

	bool _visible;

	double _heightScale;

	// image aspect ratio
	double _aspect;

	// view transformation matrix
	double _matrix[16];

	QPoint _lastPos;
};

#endif
