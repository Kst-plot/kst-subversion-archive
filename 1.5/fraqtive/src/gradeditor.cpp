/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "gradeditor.h"
#include "gradwidget.h"
#include "gradlist.h"
#include "presets.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>

GradientEditor::GradientEditor(Presets* presets, const Gradient& gradient, QWidget* parent, const char* name) : QDialog(parent, name, true),
	_presets(presets),
	_currentGradient(gradient)
{
	setCaption(i18n("Gradient Editor"));
	setIcon(SmallIcon("frgradedit"));

	QGridLayout* layout = new QGridLayout(this, 2, 2, 10);

	_gradientList = new GradientList(this);
	layout->addWidget(_gradientList, 0, 0);

	_gradientWidget = new GradientWidget(this);
	layout->addWidget(_gradientWidget, 0, 1);
	_gradientWidget->setGradient(_currentGradient);

	fillList();

	connect(_gradientList, SIGNAL(gradientSelected(const Gradient&)), _gradientWidget, SLOT(setGradient(const Gradient&)));
	connect(_gradientList, SIGNAL(doubleClicked(QListBoxItem*)), SLOT(onOK()));
	connect(_gradientList, SIGNAL(returnPressed(QListBoxItem*)), SLOT(onOK()));

	QHBoxLayout* box1 = new QHBoxLayout(layout, 10);

	QPushButton* button = new QPushButton(this);
	button->setText(i18n("&Save As"));
	connect(button, SIGNAL(clicked()), SLOT(onSaveAs()));
	box1->addWidget(button);
	button = new QPushButton(this);
	button->setText(i18n("Set &Default"));
	connect(button, SIGNAL(clicked()), SLOT(onSetDefault()));
	box1->addWidget(button);
	button = new QPushButton(this);
	button->setText(i18n("Delete"));
	connect(button, SIGNAL(clicked()), SLOT(onDelete()));
	box1->addWidget(button);

	QHBoxLayout* box2 = new QHBoxLayout(layout, 10);

	button = new QPushButton(this);
	button->setText(i18n("&New"));
	QPopupMenu* menu = new QPopupMenu(this);
	menu->insertItem(i18n("&RGB Gradient"), _gradientWidget, SLOT(newRgb()));
	menu->insertItem(i18n("&HSV Gradient"), _gradientWidget, SLOT(newHsv()));
	button->setPopup(menu);
	box2->addWidget(button);
	button = new QPushButton(this);
	button->setText(i18n("&Invert"));
	connect(button, SIGNAL(clicked()), _gradientWidget, SLOT(invert()));
	box2->addWidget(button);
	box2->addStretch(1);
	button = new QPushButton(this);
	button->setText(i18n("&Apply"));
	connect(button, SIGNAL(clicked()), SLOT(onApply()));
	box2->addWidget(button);
	button = new QPushButton(this);
	button->setText(i18n("&OK"));
	button->setDefault(true);
	connect(button, SIGNAL(clicked()), SLOT(onOK()));
	box2->addWidget(button);
	button = new QPushButton(this);
	button->setText(i18n("&Cancel"));
	connect(button, SIGNAL(clicked()), SLOT(reject()));
	box2->addWidget(button);

	layout->setColStretch(0, 0);
	layout->setColStretch(1, 1);
}

GradientEditor::~GradientEditor()
{
}

void GradientEditor::onSaveAs()
{
	bool ok = false;
	QString name = KInputDialog::getText(i18n("Save Preset"), i18n("Enter Preset Name:"),
		QString::null, &ok, this);
	if (ok) {
		_presets->setPreset(name, _gradientWidget->getGradient());
		_gradientList->clear();
		fillList();
		_gradientList->setSelected(_gradientList->findItem(name, Qt::ExactMatch | Qt::CaseSensitive), true);
	}
}

void GradientEditor::onSetDefault()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to set current gradient"
			" as the default gradient?")) == KMessageBox::Yes) {
		_presets->setDefault(_gradientWidget->getGradient());
		_gradientList->setGradient(1, _gradientWidget->getGradient());
	}
}

void GradientEditor::onDelete()
{
	int item = _gradientList->currentItem();
	if (item >= 3) {
		_presets->removePreset(_gradientList->currentText());
		_gradientList->removeItem(item);
	}
}

void GradientEditor::onApply()
{
	_currentGradient = _gradientWidget->getGradient();
	emit applyGradient(_currentGradient);
	_gradientList->setGradient(0, _currentGradient);
}

void GradientEditor::onOK()
{
	onApply();
	accept();
}

void GradientEditor::fillList()
{
	_gradientList->insertGradient(i18n("Current"), _currentGradient);
	_gradientList->insertGradient(i18n("Default"), _presets->getDefault());
	_gradientList->insertSeparator(i18n("Presets"));
	for (int i = 0; i < _presets->getPresetsCnt(); i++)
		_gradientList->insertGradient(_presets->getPresetName(i), _presets->getPresetGradient(i));
}

#include "gradeditor.moc"
