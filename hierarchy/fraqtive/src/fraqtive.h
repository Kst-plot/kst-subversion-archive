/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef FRAQTIVE_H
#define FRAQTIVE_H

#include "presets.h"
#include "generator.h"

#include <qvaluelist.h>
#include <kdockwidget.h>

class ViewWidget;
class View3DWidget;

class QTabWidget;
class QLineEdit;
class QSlider;
class QComboBox;
class KColorButton;
class KAction;
class KToggleAction;
class QTimer;
class QProgressDialog;

/*!
* \brief Main window of the Fraqtive application.
*/
class FraqtiveWindow : public KDockMainWindow
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*/
	FraqtiveWindow();

	/*!
	* \brief Destructor.
	*/
	~FraqtiveWindow();

public slots:
	/*!
	* \brief Change the current position of the fractal.
	*
	* Called from the main view when navigating with mouse.
	*
	* \param px    The x coordinate of the center of the fractal.
	* \param py    The y coordinate of the center of the fractal.
	* \param zoom  The zoom factor (as a decimal logarithm).
	* \param angle The rotation angle (in degrees).
	*/
	void setPosition(double px, double py, double zoom, double angle);

	/*!
	* \brief Switch to Mandelbrot fractal.
	*/
	void setMandelbrotMode();

	/*!
	* \brief Switch to Julia fractal.
	*
	* \param jx The x coordinate of the Julia parameter.
	* \param jy The y coordinate of the Julia parameter.
	*/
	void setJuliaMode(double jx, double jy);

	/*!
	* \brief Set a gradient for drawing all fractals.
	*
	* \param gradient The new gradient to use.
	*/
	void setGradient(const Gradient& gradient);

signals:
	/*!
	* \brief A new position of the fractal was set.
	*
	* \param px    The x coordinate of the center of the fractal.
	* \param py    The y coordinate of the center of the fractal.
	* \param zoom  The zoom factor (as a decimal logarithm).
	* \param angle The rotation angle (in degrees).
	*/
	void positionChanged(double px, double py, double zoom, double angle);

	/*!
	* \brief The Mandelbrot fractal was selected.
	*/
	void modeChangedMandelbrot();

	/*!
	* \brief The Julia fractal was selected.
	*
	* \param jx The x coordinate of the Julia parameter.
	* \param jy The y coordinate of the Julia parameter.
	*/
	void modeChangedJulia(double jx, double jy);

	/*!
	* \brief Maximum iterations and precision was changed.
	*
	* \param maxIter   Maximum number of iterations (as decimal logarithm).
	* \param threshold Minimum threshold for calculating fractal details.
	*/
	void precisionChanged(double maxIter, double threshold);

	/*!
	* \brief The background color was changed.
	*
	* \param color The new background color.
	*/
	void backgroundChanged(QRgb color);

	/*!
	* \brief The gradient was changed.
	*
	* \param data   The array of gradient colors.
	* \param scale  The scale of the gradient.
	* \param offset The offset of the gradient.
	*/
	void gradientChanged(const QRgb* data, double scale, double offset);

	/*!
	* \brief The grid size for the 3D view was changed.
	*
	* \param width  The horizontal size of the grid.
	* \param height The vertical size of the grid.
	*/
	void gridSizeChanged(int width, int height);

	/*!
	* \brief The height scale for the 3D view was changed.
	*
	* \param scale The new height scale.
	*/
	void heightScaleChanged(double scale);

	/*!
	* \brief Enable or disable the Julia preview.
	*
	* \param enabled Enable (true) or disable (false) the preview.
	*/
	void previewEnabled(bool enabled);

	/*!
	* \brief Enable or disable the 2D view.
	*
	* \param enabled Enable (true) or disable (false) the view.
	*/
	void view2DEnabled(bool enabled);

	/*!
	* \brief Enable or disable the 3D view.
	*
	* \param enabled Enable (true) or disable (false) the view.
	*/
	void view3DEnabled(bool enabled);

protected:
	void customEvent(QCustomEvent* e);

private slots:
	// action slots
	void onGradientEdit();
	void onGradientInvert();
	void onDefaultView();
	void onViewBack();
	void onViewForward();
	void onFileSave();
	void onEditCopy();

	void updateShowActions();

	// edit widgets update slots
	void readPosition();
	void readJulia();

	// slider/combobox slots
	void setMaxIterations(int iter);
	void setQuality(int quality);
	void setGradientScale(int scale);
	void setGradientOffset(int offset);
	void setScrollMode(int mode);
	void setScrollSpeed(int speed);
	void setGridSize(int size);
	void setHeightScale(int scale);

	// color-scrolling timer slot
	void scrollTimer();

	// button slot
	void selectBackground(const QColor& color);

	// view tab slot
	void tabChanged();

private:
	// update tab widgets
	void updatePosition();
	void updateMode();

	// create widgets and actions
	void createWidgets();
	QWidget* createParamsDock(QWidget* parent);
	QWidget* createDisplayDock(QWidget* parent);
	void createActions();

	// configuration
	void readParameters();
	void writeParameters();

private:
	struct Position
	{
		double _posX, _posY;
		double _zoom;
		double _angle;
	};

	typedef QValueList<Position> PositionHistory;

	// widgets and actions
	QTabWidget* _tabWidget;

	ViewWidget* _view2D;
	View3DWidget* _view3D;

	KDockWidget* _dockParams;
	KDockWidget* _dockDisplay;
	KDockWidget* _dockPreview;

	QLineEdit* _editPosX;
	QLineEdit* _editPosY;
	QLineEdit* _editZoom;
	QLineEdit* _editAngle;
	QLineEdit* _editJuliaX;
	QLineEdit* _editJuliaY;

	QSlider* _sliderIterations;
	QSlider* _sliderQuality;
	QSlider* _sliderScale;
	QSlider* _sliderOffset;
	QSlider* _sliderScroll;
	QSlider* _sliderHeight;

	QComboBox* _comboGrid;

	KColorButton* _colorButton;

	KAction* _actionMandelbrot;
	KAction* _actionBack;
	KAction* _actionForward;

	KToggleAction* _actionParams;
	KToggleAction* _actionDisplay;
	KToggleAction* _actionPreview;

	// fractal parameters
	bool _posValid;
	double _posX, _posY;
	double _zoom;
	double _angle;

	bool _juliaMode;
	double _juliaX, _juliaY;

	double _maxIter;
	double _threshold;

	Presets _presets;

	Gradient _gradient;

	QRgb* _gradientData;
	double _gradientScale;
	double _gradientOffset;

	QTimer* _scrollTimer;
	int _scrollDir;
	double _scrollSpeed;

	QColor _backgroundColor;

	// navigation of position history
	PositionHistory _backHistory;
	PositionHistory _forwardHistory;

	// generator for saving images
	Generator _generator;
	QProgressDialog* _progressDialog;

	QSize _imageSize;
	QString _imageMime;
	QString _imagePath;
};

#endif
