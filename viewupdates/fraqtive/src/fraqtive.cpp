/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "fraqtive.h"
#include "view2d.h"
#include "view3d.h"
#include "gradeditor.h"
#include "genthread.h"
#include "gradbutton.h"

#include <qtimer.h>
#include <qtabwidget.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kstatusbar.h>
#include <kcolordialog.h>
#include <kcolorbutton.h>
#include <qprogressdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <qtooltip.h>

FraqtiveWindow::FraqtiveWindow() : KDockMainWindow(NULL, "mainwnd", WType_TopLevel),
	_posValid(false),
	_juliaMode(false),
	_maxIter(0.5),
	_threshold(0.5),
	_gradientData(NULL),
	_scrollTimer(NULL),
	_scrollDir(0),
	_generator(this, false),
	_progressDialog(NULL)
{
	KImageIO::registerFormats();

	generatorThread.start(QThread::LowPriority);

	createWidgets();
	createActions();
	createGUI();

	_presets.loadPresets(kapp->config());
	readParameters();

	onDefaultView();
	updateMode();

	_scrollTimer = new QTimer(this);
	connect(_scrollTimer, SIGNAL(timeout()), SLOT(scrollTimer()));

	resize(700, 520);

	setAutoSaveSettings();
	readDockConfig();
}

FraqtiveWindow::~FraqtiveWindow()
{
	_presets.savePresets(kapp->config());
	writeParameters();
	writeDockConfig();

	generatorThread.abort();

	delete[] _gradientData;
}

void FraqtiveWindow::setGradient(const Gradient& gradient)
{
	if (!_gradientData)
		_gradientData = new QRgb[GRADIENT_LENGTH];

	_gradient = gradient;
	_gradient.fillGradient(_gradientData);

	emit gradientChanged(_gradientData, _gradientScale, _gradientOffset);
}

void FraqtiveWindow::setPosition(double px, double py, double zoom, double angle)
{
	if (_posValid && _posX == px && _posY == py && _zoom == zoom && _angle == angle)
		return;

	if (_posValid) {
		Position position;
		position._posX = _posX;
		position._posY = _posY;
		position._zoom = _zoom;
		position._angle = _angle;

		if (_backHistory.size() > 100)
			_backHistory.pop_front();

		_backHistory.push_back(position);
		_forwardHistory.clear();
	}

	_posX = px;
	_posY = py;
	_zoom = zoom;
	_angle = angle;
	_posValid = true;

	updatePosition();
}

void FraqtiveWindow::setMandelbrotMode()
{
	if (!_juliaMode)
		return;

	_juliaMode = false;

	_backHistory.clear();
	_forwardHistory.clear();
	_posValid = false;

	onDefaultView();
	updateMode();
}

void FraqtiveWindow::setJuliaMode(double jx, double jy)
{
	if (_juliaMode && _juliaX == jx && _juliaY == jy)
		return;

	_juliaMode = true;
	_juliaX = jx;
	_juliaY = jy;

	_backHistory.clear();
	_forwardHistory.clear();
	_posValid = false;

	onDefaultView();
	updateMode();
}

void FraqtiveWindow::onGradientEdit()
{
	GradientEditor dlg(&_presets, _gradient, this);

	connect(&dlg, SIGNAL(applyGradient(const Gradient&)), SLOT(setGradient(const Gradient&)));

	dlg.exec();
}

void FraqtiveWindow::onGradientInvert()
{
	_gradient.invert();
	_gradient.fillGradient(_gradientData);

	emit gradientChanged(_gradientData, _gradientScale, _gradientOffset);
}

void FraqtiveWindow::onDefaultView()
{
	setPosition(_juliaMode ? 0.0 : -0.7, 0.0, 0.0, 0.0);
}

void FraqtiveWindow::onViewBack()
{
	if (!_backHistory.empty()) {
		Position posNew = _backHistory.back();
		_backHistory.pop_back();
		
		Position posOld;
		posOld._posX = _posX;
		posOld._posY = _posY;
		posOld._zoom = _zoom;
		posOld._angle = _angle;
		_forwardHistory.push_front(posOld);

		_posX = posNew._posX;
		_posY = posNew._posY;
		_zoom = posNew._zoom;
		_angle = posNew._angle;

		updatePosition();
	}
}

void FraqtiveWindow::onViewForward()
{
	if (!_forwardHistory.empty()) {
		Position posNew = _forwardHistory.front();
		_forwardHistory.pop_front();

		Position posOld;
		posOld._posX = _posX;
		posOld._posY = _posY;
		posOld._zoom = _zoom;
		posOld._angle = _angle;
		_backHistory.push_back(posOld);

		_posX = posNew._posX;
		_posY = posNew._posY;
		_zoom = posNew._zoom;
		_angle = posNew._angle;

		updatePosition();
	}
}

void FraqtiveWindow::onFileSave()
{
	QHBox* box = new QHBox(NULL);
	box->setSpacing(10);

	new QLabel(i18n("Image Size:"), box);
	QLineEdit* editResX = new QLineEdit(box);
	editResX->setValidator(new QIntValidator(1, 32768, editResX));
	box->setStretchFactor(editResX, 0);
	new QLabel(i18n("x"), box);
	QLineEdit* editResY = new QLineEdit(box);
	editResY->setValidator(new QIntValidator(1, 32768, editResY));
	box->setStretchFactor(editResY, 0);
	box->setStretchFactor(new QWidget(box), 1);

	editResX->setText(QString::number(_imageSize.width()));
	editResY->setText(QString::number(_imageSize.height()));

	KFileDialog imageDlg(_imagePath, QString::null, this, "file_dlg", true, box);
	imageDlg.setCaption(i18n("Save Image"));
	imageDlg.setOperationMode(KFileDialog::Saving);
	imageDlg.setMimeFilter(KImageIO::mimeTypes(KImageIO::Writing), _imageMime);

	if (imageDlg.exec() == KFileDialog::Accepted) {
		QString fileName = imageDlg.selectedFile();

		_imagePath = QFileInfo(fileName).dirPath();
		_imageMime = imageDlg.currentMimeFilter();

		if (editResX->hasAcceptableInput() && editResY->hasAcceptableInput()) {
			_imageSize.setWidth(editResX->text().toInt());
			_imageSize.setHeight(editResY->text().toInt());
		} else {
			KMessageBox::sorry(this, i18n("An invalid image size was entered."));
			return;
		}

		emit previewEnabled(false);

		_progressDialog = new QProgressDialog(this, NULL, true);
		_progressDialog->setCaption(i18n("Save Image"));
		_progressDialog->setLabelText(i18n("Generating Image..."));
		_progressDialog->setTotalSteps(100);
		_progressDialog->setMinimumDuration(0);

		_generator.setSize(_imageSize);
		_generator.setPosition(_posX, _posY, _zoom, _angle);
		if (_juliaMode)
			_generator.setJuliaMode(_juliaX, _juliaY);
		_generator.setQuality(_maxIter, _threshold);
		_generator.setBackground(_backgroundColor.rgb());
		_generator.setGradient(_gradientData, _gradientScale, _gradientOffset);

		_generator.addToQueue();
		_progressDialog->exec();

		if (!_progressDialog->wasCanceled()) {
			if (!_generator.isValid()) {
				KMessageBox::sorry(this, i18n("Not enough memory to generate the fractal."));
			} else {
				_progressDialog->setLabelText(i18n("Saving Image..."));
				_progressDialog->setProgress(0);
				qApp->processEvents();
				_generator.repaintImage();
				QString format = KImageIO::typeForMime(_imageMime);
				bool result = _generator.getImage().save(fileName, format.ascii());
				_progressDialog->setProgress(100);
				if (!result) {
					KMessageBox::sorry(this, i18n("Failed to save the image file."));
				}
			}
		} else
			_generator.abort();

		delete _progressDialog;
		_progressDialog = NULL;

		_generator.freeData();

		if (!_juliaMode)
			emit previewEnabled(true);
	}
}

void FraqtiveWindow::onEditCopy()
{
	if (_tabWidget->currentPageIndex() == 0)
		_view2D->copyImage();
	else
		_view3D->copyImage();
}

void FraqtiveWindow::customEvent(QCustomEvent* e)
{
	if (e->type() == (int)GeneratorEvent::EventType) {
		GeneratorEvent* ge = (GeneratorEvent*)e;
		if (_progressDialog)
			_progressDialog->setProgress(ge->getProgress());
	}
}

void FraqtiveWindow::readPosition()
{
	double posX = _posX;
	double posY = _posY;
	double zoom = _zoom;
	double angle = _angle;

	if (_editPosX->hasAcceptableInput())
		posX = _editPosX->text().toDouble();
	if (_editPosY->hasAcceptableInput())
		posY = _editPosY->text().toDouble();
	if (_editZoom->hasAcceptableInput())
		zoom = _editZoom->text().toDouble();
	if (_editAngle->hasAcceptableInput())
		angle = _editAngle->text().toDouble();

	setPosition(posX, posY, zoom, angle);
}

void FraqtiveWindow::readJulia()
{
	if (_editJuliaX->hasAcceptableInput())
		_juliaX = _editJuliaX->text().toDouble();
	if (_editJuliaY->hasAcceptableInput())
		_juliaY = _editJuliaY->text().toDouble();

	updateMode();
}

void FraqtiveWindow::setMaxIterations(int iter)
{
	_maxIter = 0.5 * iter;

	emit precisionChanged(_maxIter, _threshold);
}

void FraqtiveWindow::setQuality(int quality)
{
	_threshold = (12.8 - quality) / (2.48 * quality + 3.2);

	emit precisionChanged(_maxIter, _threshold);
}

void FraqtiveWindow::setGradientScale(int scale)
{
	_gradientScale = GRADIENT_LENGTH * (0.01 + 0.0049 * scale);

	if (_gradientData)
		emit gradientChanged(_gradientData, _gradientScale, _gradientOffset);
}

void FraqtiveWindow::setGradientOffset(int offset)
{
	_gradientOffset = GRADIENT_LENGTH * 0.01 * offset;

	if (_gradientData)
		emit gradientChanged(_gradientData, _gradientScale, _gradientOffset);
}

void FraqtiveWindow::setScrollMode(int mode)
{
	if (mode == 0) {
		_scrollTimer->stop();

		_sliderScroll->setEnabled(false);
		_sliderOffset->setEnabled(true);

		setGradientOffset(_sliderOffset->value());
	} else {
		_scrollTimer->start(50);
		_scrollDir = (mode == 1) ? 1 : -1;

		_sliderScroll->setEnabled(true);
		_sliderOffset->setEnabled(false);
	}
}

void FraqtiveWindow::setScrollSpeed(int speed)
{
	_scrollSpeed = GRADIENT_LENGTH * 0.025 * (0.05 + 0.0095 * speed);
}

void FraqtiveWindow::setGridSize(int size)
{
	int grid = 64;
	for (int i = 0; i < size; i++)
		grid *= 2;

	emit gridSizeChanged(grid, grid);
}

void FraqtiveWindow::setHeightScale(int scale)
{
	emit heightScaleChanged(0.01 + 0.0099 * scale);
}

void FraqtiveWindow::scrollTimer()
{
	_gradientOffset += _scrollDir * _scrollSpeed;

	if (_gradientOffset > GRADIENT_LENGTH)
		_gradientOffset -= GRADIENT_LENGTH;
	if (_gradientOffset < 0.0)
		_gradientOffset += GRADIENT_LENGTH;

	if (_gradientData)
		emit gradientChanged(_gradientData, _gradientScale, _gradientOffset);
}
	
void FraqtiveWindow::selectBackground(const QColor& color)
{
	_backgroundColor = color;
	emit backgroundChanged(_backgroundColor.rgb());
}

void FraqtiveWindow::tabChanged()
{
	if (_tabWidget->currentPageIndex() == 1) {
		emit view2DEnabled(false);
		emit view3DEnabled(true);
		emit previewEnabled(false);

		_comboGrid->setEnabled(true);
		_sliderHeight->setEnabled(true);
	} else {
		emit view3DEnabled(false);
		emit view2DEnabled(true);
		if (!_juliaMode)
			emit previewEnabled(true);

		_comboGrid->setEnabled(false);
		_sliderHeight->setEnabled(false);
	}
}

void FraqtiveWindow::updatePosition()
{
	QString str;
	_editPosX->setText(str.setNum(_posX, 'g', 12));
	_editPosY->setText(str.setNum(_posY, 'g', 12));
	_editZoom->setText(str.setNum(_zoom, 'g', 6));
	_editAngle->setText(str.setNum(_angle, 'g', 4));

	emit positionChanged(_posX, _posY, _zoom, _angle);

	_actionBack->setEnabled(!_backHistory.empty());
	_actionForward->setEnabled(!_forwardHistory.empty());
}

void FraqtiveWindow::updateMode()
{
	_actionMandelbrot->setEnabled(_juliaMode);
	_editJuliaX->setEnabled(_juliaMode);
	_editJuliaY->setEnabled(_juliaMode);

	if (_juliaMode) {
		QString str;
		_editJuliaX->setText(str.setNum(_juliaX, 'g', 12));
		_editJuliaY->setText(str.setNum(_juliaY, 'g', 12));
		emit previewEnabled(false);
		emit modeChangedJulia(_juliaX, _juliaY);
	} else {
		_editJuliaX->setText(QString::null);
		_editJuliaY->setText(QString::null);
		emit previewEnabled(true);
		emit modeChangedMandelbrot();
	}
}

void FraqtiveWindow::createWidgets()
{
	KDockWidget* dockMain = createDockWidget("main", NULL);
	_tabWidget = new QTabWidget(dockMain);
	dockMain->setWidget(_tabWidget);

	_view2D = new ViewWidget(true, _tabWidget);
	_tabWidget->addTab(_view2D, SmallIconSet("frtab2d"), i18n("2D View"));

	_view3D = new View3DWidget(_tabWidget);
	_tabWidget->addTab(_view3D, SmallIconSet("frtab3d"), i18n("3D View"));

	_dockParams = createDockWidget("params", SmallIcon("frparams"), NULL,
		i18n("Parameters"));
	QWidget* params = createParamsDock(_dockParams);
	_dockParams->setWidget(params);

	_dockDisplay = createDockWidget("display", SmallIcon("frdisplay"), NULL,
		i18n("Display"));
	QWidget* display = createDisplayDock(_dockParams);
	_dockDisplay->setWidget(display);

	_dockPreview = createDockWidget("preview", SmallIcon("viewmag"), NULL, i18n("Preview"));
	ViewWidget* preview = new ViewWidget(false, _dockPreview);
	_dockPreview->setWidget(preview);

	dockMain->setDockSite(KDockWidget::DockCorner);
	dockMain->setEnableDocking(KDockWidget::DockNone);
	setView(dockMain);
	setMainDockWidget(dockMain);

	KDockWidget* dock1 = _dockDisplay->manualDock(_dockParams, KDockWidget::DockCenter);
	KDockWidget* dock2 = _dockPreview->manualDock(dock1, KDockWidget::DockBottom, 65);
	dock2->manualDock(dockMain, KDockWidget::DockRight, 65);

	connect(_tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(tabChanged()));

	connect(this, SIGNAL(view2DEnabled(bool)), _view2D, SLOT(setVisible(bool)));
	connect(this, SIGNAL(view3DEnabled(bool)), _view3D, SLOT(setVisible(bool)));
	connect(this, SIGNAL(previewEnabled(bool)), _view2D, SLOT(enablePreview(bool)));

	connect(this, SIGNAL(positionChanged(double, double, double, double)), _view2D, SLOT(setPosition(double, double, double, double)));
	connect(this, SIGNAL(positionChanged(double, double, double, double)), _view3D, SLOT(setPosition(double, double, double, double)));
	connect(this, SIGNAL(positionChanged(double, double, double, double)), preview, SLOT(setPreviewPosition(double, double, double, double)));

	connect(this, SIGNAL(modeChangedMandelbrot()), _view2D, SLOT(setMandelbrotMode()));
	connect(this, SIGNAL(modeChangedMandelbrot()), _view3D, SLOT(setMandelbrotMode()));
	connect(this, SIGNAL(modeChangedJulia(double, double)), _view2D, SLOT(setJuliaMode(double, double)));
	connect(this, SIGNAL(modeChangedJulia(double, double)), _view3D, SLOT(setJuliaMode(double, double)));

	connect(this, SIGNAL(precisionChanged(double, double)), _view2D, SLOT(setQuality(double, double)));
	connect(this, SIGNAL(precisionChanged(double, double)), _view3D, SLOT(setQuality(double, double)));
	connect(this, SIGNAL(precisionChanged(double, double)), preview, SLOT(setQuality(double, double)));

	connect(this, SIGNAL(backgroundChanged(QRgb)), _view2D, SLOT(setBackground(QRgb)));
	connect(this, SIGNAL(backgroundChanged(QRgb)), _view3D, SLOT(setBackground(QRgb)));
	connect(this, SIGNAL(backgroundChanged(QRgb)), preview, SLOT(setBackground(QRgb)));

	connect(this, SIGNAL(gradientChanged(const QRgb*, double, double)), _view2D, SLOT(setGradient(const QRgb*, double, double)));
	connect(this, SIGNAL(gradientChanged(const QRgb*, double, double)), _view3D, SLOT(setGradient(const QRgb*, double, double)));
	connect(this, SIGNAL(gradientChanged(const QRgb*, double, double)), preview, SLOT(setGradient(const QRgb*, double, double)));

	connect(_view2D, SIGNAL(previewVisible(bool)), preview, SLOT(setVisible(bool)));
	connect(_view2D, SIGNAL(previewPosition(double, double)), preview, SLOT(setJuliaMode(double, double)));

	connect(_view2D, SIGNAL(positionChanged(double, double, double, double)), this, SLOT(setPosition(double, double, double, double)));
	connect(_view2D, SIGNAL(previewDoubleClick(double, double)), this, SLOT(setJuliaMode(double, double)));

	connect(this, SIGNAL(gridSizeChanged(int, int)), _view3D, SLOT(setGridSize(int, int)));
	connect(this, SIGNAL(heightScaleChanged(double)), _view3D, SLOT(setHeightScale(double)));

	_view2D->enableTransform(true);

	KStatusBar* status = statusBar();
	status->setSizeGripEnabled(false);

	QProgressBar* progress = new QProgressBar(100, status);
	progress->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	progress->setMaximumHeight(status->sizeHint().height() - 2);
	progress->setMinimumWidth(120);
	progress->setProgress(100);
	status->addWidget(progress, 0, true);

	connect(_view2D, SIGNAL(updateProgress(int)), progress, SLOT(setProgress(int)));
	connect(_view3D, SIGNAL(updateProgress(int)), progress, SLOT(setProgress(int)));
}

QWidget* FraqtiveWindow::createParamsDock(QWidget* parent)
{
	QVBox* wrapper = new QVBox(parent);
	wrapper->setSpacing(0);

	QGrid* propParams = new QGrid(2, wrapper);
	propParams->setMargin(7);
	propParams->setSpacing(5);

	new QLabel(i18n("X coordinate:"), propParams);
	_editPosX = new QLineEdit(propParams);
	_editPosX->setValidator(new QDoubleValidator(-4.0, 4.0, 15, _editPosX));
	connect(_editPosX, SIGNAL(returnPressed()), SLOT(readPosition()));
	connect(_editPosX, SIGNAL(lostFocus()), SLOT(readPosition()));
	QToolTip::add(_editPosX, i18n("X coordinate of the center of the view"));

	new QLabel(i18n("Y coordinate:"), propParams);
	_editPosY = new QLineEdit(propParams);
	_editPosY->setValidator(new QDoubleValidator(-4.0, 4.0, 15, _editPosY));
	connect(_editPosY, SIGNAL(returnPressed()), SLOT(readPosition()));
	connect(_editPosY, SIGNAL(lostFocus()), SLOT(readPosition()));
	QToolTip::add(_editPosY, i18n("Y coordinate of the center of the view"));

	new QLabel(i18n("Zoom magnitude:"), propParams);
	_editZoom = new QLineEdit(propParams);
	_editZoom->setValidator(new QDoubleValidator(-1.0, 14.0, 15, _editZoom));
	connect(_editZoom, SIGNAL(returnPressed()), SLOT(readPosition()));
	connect(_editZoom, SIGNAL(lostFocus()), SLOT(readPosition()));
	QToolTip::add(_editZoom, i18n("Zoom order of maginitude"));

	new QLabel(i18n("Rotation angle:"), propParams);
	_editAngle = new QLineEdit(propParams);
	_editAngle->setValidator(new QDoubleValidator(-360.0, 360.0, 15, _editAngle));
	connect(_editAngle, SIGNAL(returnPressed()), SLOT(readPosition()));
	connect(_editAngle, SIGNAL(lostFocus()), SLOT(readPosition()));
	QToolTip::add(_editAngle, i18n("View rotation angle in degrees"));

	new QLabel(i18n("Julia X param.:"), propParams);
	_editJuliaX = new QLineEdit(propParams);
	connect(_editJuliaX, SIGNAL(returnPressed()), SLOT(readJulia()));
	connect(_editJuliaX, SIGNAL(lostFocus()), SLOT(readJulia()));
	QToolTip::add(_editJuliaX, i18n("X coordinate of the Julia parameter"));

	new QLabel(i18n("Julia Y param.:"), propParams);
	_editJuliaY = new QLineEdit(propParams);
	connect(_editJuliaY, SIGNAL(returnPressed()), SLOT(readJulia()));
	connect(_editJuliaY, SIGNAL(lostFocus()), SLOT(readJulia()));
	QToolTip::add(_editJuliaY, i18n("Y coordinate of the Julia parameter"));

	new QLabel(i18n("Iterations limit:"), propParams);
	_sliderIterations = new QSlider(QSlider::Horizontal, propParams);
	_sliderIterations->setRange(0, 20);
	connect(_sliderIterations, SIGNAL(valueChanged(int)), SLOT(setMaxIterations(int)));
	QToolTip::add(_sliderIterations, i18n("Maximum number of iterations to calculate"));

	new QLabel(i18n("Detail level:"), propParams);
	_sliderQuality = new QSlider(QSlider::Horizontal, propParams);
	_sliderQuality->setRange(0, 10);
	connect(_sliderQuality, SIGNAL(valueChanged(int)), SLOT(setQuality(int)));
	QToolTip::add(_sliderQuality, i18n("Amount of details to calculate"));

	wrapper->setStretchFactor(new QWidget(wrapper), 1);

	return wrapper;
}

QWidget* FraqtiveWindow::createDisplayDock(QWidget* parent)
{
	QVBox* wrapper = new QVBox(parent);
	wrapper->setSpacing(0);

	QGrid* propDisplay = new QGrid(2, wrapper);
	propDisplay->setMargin(7);
	propDisplay->setSpacing(5);

	new QLabel(i18n("Color gradient:"), propDisplay);
	GradientButton* gradButton = new GradientButton(propDisplay);
	connect(this, SIGNAL(gradientChanged(const QRgb*, double, double)), gradButton, SLOT(setGradient(const QRgb*, double, double)));
	connect(gradButton, SIGNAL(clicked()), this, SLOT(onGradientEdit()));
	QToolTip::add(gradButton, i18n("Gradient used to display the fractal"));

	new QLabel(i18n("Color scale:"), propDisplay);
	_sliderScale = new QSlider(QSlider::Horizontal, propDisplay);
	_sliderScale->setRange(0, 100);
	connect(_sliderScale, SIGNAL(valueChanged(int)), SLOT(setGradientScale(int)));
	QToolTip::add(_sliderScale, i18n("Scale of the color gradient"));

	new QLabel(i18n("Color offset:"), propDisplay);
	_sliderOffset = new QSlider(QSlider::Horizontal, propDisplay);
	_sliderOffset->setRange(0, 100);
	connect(_sliderOffset, SIGNAL(valueChanged(int)), SLOT(setGradientOffset(int)));
	QToolTip::add(_sliderOffset, i18n("Offset of the color gradient"));

	new QLabel(i18n("Scroll colors:"), propDisplay);
	QHBox* box = new QHBox(propDisplay);
	box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	box->setSpacing(5);
	QComboBox* combo = new QComboBox(box);
	combo->insertItem(i18n("scroll direction", "Off"));
	combo->insertItem(i18n("scroll direction", "Left"));
	combo->insertItem(i18n("scroll direction", "Right"));
	connect(combo, SIGNAL(activated(int)), SLOT(setScrollMode(int)));
	_sliderScroll = new QSlider(QSlider::Horizontal, box);
	_sliderScroll->setRange(0, 100);
	_sliderScroll->setEnabled(false);
	connect(_sliderScroll, SIGNAL(valueChanged(int)), SLOT(setScrollSpeed(int)));
	QToolTip::add(combo, i18n("Scrolling direction"));
	QToolTip::add(_sliderScroll, i18n("Scrolling speed"));

	new QLabel(i18n("Background:"), propDisplay);
	_colorButton = new KColorButton(_backgroundColor, propDisplay);
	connect(_colorButton, SIGNAL(changed(const QColor&)), SLOT(selectBackground(const QColor&)));
	QToolTip::add(_colorButton, i18n("Color of the background area"));

	new QLabel(i18n("3D grid size:"), propDisplay);
	_comboGrid = new QComboBox(propDisplay);
	for (int i = 64; i <= 1024; i *= 2)
		_comboGrid->insertItem(i18n("%1 x %2").arg(i).arg(i));
	_comboGrid->setEnabled(false);
	connect(_comboGrid, SIGNAL(activated(int)), SLOT(setGridSize(int)));
	QToolTip::add(_comboGrid, i18n("Resolution of the 3D mesh"));

	new QLabel(i18n("Height scale:"), propDisplay);
	_sliderHeight = new QSlider(QSlider::Horizontal, propDisplay);
	_sliderHeight->setRange(0, 100);
	_sliderHeight->setEnabled(false);
	connect(_sliderHeight, SIGNAL(valueChanged(int)), SLOT(setHeightScale(int)));
	QToolTip::add(_sliderHeight, i18n("Height scale of the 3D mesh"));

	wrapper->setStretchFactor(new QWidget(wrapper), 1);

	return wrapper;
}

void FraqtiveWindow::createActions()
{
	KAction* actionSave = KStdAction::save(this, SLOT(onFileSave()), actionCollection());
	actionSave->setText(i18n("&Save Image..."));
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	KStdAction::copy(this, SLOT(onEditCopy()), actionCollection());
	new KAction(i18n("&Edit Gradient..."), "frgradedit", CTRL+Key_E,
		this, SLOT(onGradientEdit()), actionCollection(), "grad_edit");
	new KAction(i18n("&Invert Gradient"), "frgradinv", CTRL+Key_X,
		this, SLOT(onGradientInvert()), actionCollection(), "grad_inv");

	_actionMandelbrot = KStdAction::up(this, SLOT(setMandelbrotMode()), actionCollection());
	_actionMandelbrot->setText(i18n("&To Mandelbrot"));
	KStdAction::home(this, SLOT(onDefaultView()), actionCollection());
	_actionBack = KStdAction::back(this, SLOT(onViewBack()), actionCollection());
	_actionForward = KStdAction::forward(this, SLOT(onViewForward()), actionCollection());

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);

	_actionParams = new KToggleAction(i18n("Show Parameters"), SmallIconSet("frparams"), 0,
		_dockParams, SLOT(changeHideShowState()), actionCollection(), "show_params");
	_actionDisplay = new KToggleAction(i18n("Show Display"), SmallIconSet("frdisplay"), 0,
		_dockDisplay, SLOT(changeHideShowState()), actionCollection(), "show_display");
	_actionPreview = new KToggleAction(i18n("Show Preview"), SmallIconSet("viewmag"), 0,
		_dockPreview, SLOT(changeHideShowState()), actionCollection(), "show_preview");
#if KDE_IS_VERSION(3,2,90)
	_actionParams->setCheckedState(i18n("Hide Parameters"));
	_actionDisplay->setCheckedState(i18n("Hide Display"));
	_actionPreview->setCheckedState(i18n("Hide Preview"));
#endif
	KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

	connect(dockManager, SIGNAL(change()), this, SLOT(updateShowActions()));
	updateShowActions();
}

void FraqtiveWindow::updateShowActions()
{
	_actionParams->setChecked(_dockParams->mayBeHide());
	_actionDisplay->setChecked(_dockDisplay->mayBeHide());
	_actionPreview->setChecked(_dockPreview->mayBeHide());
}

void FraqtiveWindow::readParameters()
{
	KConfig* config = kapp->config();
	config->setGroup("Parameters");

	_sliderIterations->setValue(config->readNumEntry("Iterations", 2));
	_sliderQuality->setValue(config->readNumEntry("Quality", 5));
	_sliderScale->setValue(config->readNumEntry("Scale", 15));
	_sliderOffset->setValue(config->readNumEntry("Offset", 0));
	_sliderScroll->setValue(config->readNumEntry("Scroll", 25));
	_sliderHeight->setValue(config->readNumEntry("Height", 20));
	
	_comboGrid->setCurrentItem(config->readNumEntry("Grid", 1));
	setGridSize(_comboGrid->currentItem());

	QColor black(0, 0, 0);
	_colorButton->setColor(config->readColorEntry("Background", &black));

	QStringList list = config->readListEntry("Gradient");
	if (list.size() == 4) {
		Gradient gradient(list[0] == "hsv");
		gradient.getSpline(0).fromString(list[1]);
		gradient.getSpline(1).fromString(list[2]);
		gradient.getSpline(2).fromString(list[3]);
		setGradient(gradient);
	} else
		setGradient(_presets.getDefault());

	QSize defSize(800, 600);
	_imageSize = config->readSizeEntry("ImageSize", &defSize);
	_imageMime = config->readEntry("ImageMime", "image/png");
	_imagePath = config->readEntry("ImagePath");
}

void FraqtiveWindow::writeParameters()
{
	KConfig* config = kapp->config();
	config->setGroup("Parameters");

	config->writeEntry("Iterations", _sliderIterations->value());
	config->writeEntry("Quality", _sliderQuality->value());
	config->writeEntry("Scale", _sliderScale->value());
	config->writeEntry("Offset", _sliderOffset->value());
	config->writeEntry("Scroll", _sliderScroll->value());
	config->writeEntry("Height", _sliderHeight->value());
	config->writeEntry("Grid", _comboGrid->currentItem());
	config->writeEntry("Background", _backgroundColor);

	QStringList list;
	list.append(_gradient.isHsv() ? "hsv" : "rgb");
	list.append(_gradient.getSpline(0).toString());
	list.append(_gradient.getSpline(1).toString());
	list.append(_gradient.getSpline(2).toString());
	config->writeEntry("Gradient", list);

	config->writeEntry("ImageSize", _imageSize);
	config->writeEntry("ImageMime", _imageMime);
	config->writeEntry("ImagePath", _imagePath);
}

#include "fraqtive.moc"
