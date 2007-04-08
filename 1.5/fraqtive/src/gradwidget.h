/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef GRADCONTROL_H
#define GRADCONTROL_H

#include "gradient.h"

#include <qwidget.h>
#include <qrect.h>

/*!
* \brief A widget for editing the gradient splines.
*/
class GradientWidget : public QWidget
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*/
	GradientWidget(QWidget* parent=0, const char* name=0);

	/*!
	* \brief Destructor.
	*/
	~GradientWidget();

public:
	/*!
	* \brief Return the minimum size of the widget.
	*/
	QSize minimumSizeHint() const;

	/*!
	* \brief Return the recommended size of the widget.
	*/
	QSize sizeHint() const;

	/*!
	* \brief Get the current gradient.
	*
	* \return The gradient object.
	*/
	const Gradient& getGradient() const
	{
		return _gradient;
	}

public slots:
	/*!
	* \brief Set the gradient to edit.
	*
	* \param gradient The gradient object.
	*/
	void setGradient(const Gradient& gradient);

	/*!
	* \brief Set a default RGB gradient.
	*/
	void newRgb();

	/*!
	* \brief Set a default HSV gradient.
	*/
	void newHsv();

	/*!
	* \brief Invert the current gradient.
	*/
	void invert();

private:
	// process events
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);

	// layout the area rectangles
	void calcRects();

private:
	Gradient _gradient;

	// area rectangles
	QRect _gradRect;
	QRect _splineRect[3];

	// node dragging
	int _dragSpline;
	int _dragNode;
	bool _dragRemove;
};

#endif
