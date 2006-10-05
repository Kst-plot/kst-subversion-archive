/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef VIEW2D_H
#define VIEW2D_H

#include "generator.h"

#include <qwidget.h>

/*!
* \brief The fractal view wigdet.
*/
class ViewWidget : public QWidget
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*
	* \param progressive Use progressive updating of the fractal.
	*/
	ViewWidget(bool progressive, QWidget* parent=0, const char* name=0);

	/*!
	* \brief Destructor.
	*/
	~ViewWidget();

public slots:
	/*!
	* \brief Enable or disable displaying the fractal.
	*
	* \param visible \c true if the view is enabled, \c false otherwise.
	*/
	void setVisible(bool visible);

	/*!
	* \brief Enable or disable interactively changing the view position.
	*
	* \param transform \c true if transforming is enabled, \c false otherwise.
	*/
	void enableTransform(bool transform);

	/*!
	* \brief Enable or disable tracking the preview position.
	*
	* \param preview \c true if preview tracking is enabled, \c false otherwise.
	*/
	void enablePreview(bool preview);

	/*!
	* \brief Set the position of the fragment of the fractal to view.
	*
	* \param px    The x coordinate of the center of the fractal.
	* \param py    The y coordinate of the center of the fractal.
	* \param zoom  The zoom factor (as a decimal logarithm).
	* \param angle The rotation angle (in degrees).
	*/
	void setPosition(double px, double py, double zoom, double angle);

	/*!
	* \brief Set the position of the Julia preview fractal.
	*
	* \param px    Ignored.
	* \param py    Ignored.
	* \param zoom  The main view's zoom factor.
	* \param angle Ignored.
	*/
	void setPreviewPosition(double px, double py, double zoom, double angle);

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
	* \brief Copy image to clipboard.
	*/
	void copyImage();

signals:
	/*!
	* \brief Set the new position after user's navigation.
	*
	* \param px    The new x coordinate.
	* \param py    The new y coordinate.
	* \param zoom  The new zoom factor.
	* \param angle The new rotation angle.
	*/
	void positionChanged(double px, double py, double zoom, double angle);

	/*!
	* \brief Enable or disable the preview.
	*
	* \param visible \c true if the preview should be enabled, \c false otherwise.
	*/
	void previewVisible(bool visible);

	/*!
	* \brief Set the preview position.
	*
	* \param px The x coordinate of the mouse
	* \param py The y coordinate of the mouse.
	*/
	void previewPosition(double px, double py);

	/*!
	* \brief Notify about a double-click at the given position.
	*
	* \param px The x coordinate of the mouse.
	* \param py The y coordinate of the mouse.
	*/
	void previewDoubleClick(double px, double py);

	/*!
	* \brief Set the progress of the fractal generator.
	*
	* \param progress Current progress.
	*/
	void updateProgress(int progress);

protected:
	// process events
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent* e);
	void customEvent(QCustomEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void enterEvent(QEvent* e);
	void leaveEvent(QEvent* e);

private:
	void emitPositionChanged();

private:
	// available dragging modes
	enum DragMode {
		DRAG_NONE, DRAG_ZOOM_IN, DRAG_ZOOM, DRAG_MOVE, DRAG_ROTATE
	};

	Generator _generator;

	bool _visible;
	bool _enableTransform;
	bool _enablePreview;

	// drag information
	DragMode _dragMode;
	QPoint _startPos;

	int _lastOffset;
	int _lastOffsetX;
	int _lastOffsetY;

	// image transformation matrix
	bool _transform;
	double _transX[3];
	double _transY[3];
};

#endif
