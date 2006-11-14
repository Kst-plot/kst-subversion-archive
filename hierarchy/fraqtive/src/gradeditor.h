/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef GRADEDITOR_H
#define GRADEDITOR_H

#include "gradient.h"

#include <qdialog.h>

class Presets;
class GradientWidget;
class GradientList;

/*!
* \brief The gradient editor dialog box.
*/
class GradientEditor : public QDialog
{
	Q_OBJECT
public:
	/*!
	* \brief Constructor.
	*
	* \param presets  The list of presets to use and edit.
	* \param gradient The current gradient to edit.
	*/
	GradientEditor(Presets* presets, const Gradient& gradient, QWidget* parent=0, const char* name=0);

	/*!
	* \brief Destructor.
	*/
	~GradientEditor();

signals:
	/*!
	* \brief Apply a new gradient.
	*
	* Emitted when the Apply or OK button was pressed.
	*
	* \param gradient The gradient to apply.
	*/
	void applyGradient(const Gradient& gradient);

private slots:
	// button slots
	void onSaveAs();
	void onSetDefault();
	void onDelete();
	void onApply();
	void onOK();

private:
	// fill the presets list
	void fillList();

private:
	Presets* _presets;
	Gradient _currentGradient;

	GradientWidget* _gradientWidget;
	GradientList* _gradientList;
};

#endif
