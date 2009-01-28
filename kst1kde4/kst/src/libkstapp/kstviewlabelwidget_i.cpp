/***************************************************************************
                       kstviewlabelwidget_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// include files for Qt
#include <qlabel.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kcombobox.h>
#include <klocale.h>

// application specific includes
#include "kstviewlabelwidget_i.h"
#include "scalarselector.h"
#include "stringselector.h"

KstViewLabelWidgetI::KstViewLabelWidgetI( QWidget* parent, const char* name, WFlags fl )
  : ViewLabelWidget(parent, name, fl)
{
    _horizontal->insertItem(i18n("Left"));
    _horizontal->insertItem(i18n("Right"));
    _horizontal->insertItem(i18n("Center"));

    _horizontal->hide();// FIXME: until implemented - bug 135437
    textLabel5_3->hide();

    _changedFgColor = false;
    _changedBgColor = false;

    connect(_scalars, SIGNAL(selectionChanged(const QString &)), this, SLOT(insertScalarInText(const QString &)));
    connect(_strings, SIGNAL(selectionChanged(const QString &)), this, SLOT(insertStringInText(const QString &)));

    QWhatsThis::add(_text, i18n("<qt>The syntax for labels is a derivative of a subset of LaTeX.  Supported syntax is: <b>\\[greeklettername]</b> and <b>\\[Greeklettername]</b>, <b>\\approx</b>, <b>\\cdot</b>, <b>\\ge</b>, <b>\\geq</b>, <b>\\inf</b>, <b>\\int</b>, <b>\\le</b>, <b>\\leq</b>, <b>\\ne</b>, <b>\\n</b>, <b>\\partial</b>, <b>\\prod</b>, <b>\\pm</b>, <b>\\textcolor{color name}{colored text}</b>, <b>\\textbf{bold text}</b>, <b>\\textit{italicized text}</b>, <b>\\t</b>, <b>\\sum</b>, <b>\\sqrt</b>, <b>\\underline{underlined text}</b>, <b>x^y</b>, <b>x_y</b>.  Scalars, equations, and vector elements can be embedded.  Scalar: <i>[V1/Mean]</i>.  Vector Element: <i>[V1[4]]</i>.  Equation: <i>[=[V1/Mean]^2]</i>.  A [ character can be inserted as <i>\\[</i>."));
}


KstViewLabelWidgetI::~KstViewLabelWidgetI()
{
}


void KstViewLabelWidgetI::insertScalarInText(const QString &S)
{
  _text->insert("["+S+"]");
}


void KstViewLabelWidgetI::insertStringInText(const QString &S)
{
  _text->insert("["+S+"]");
}


void KstViewLabelWidgetI::changedFgColor( )
{
  _changedFgColor = true;
}


void KstViewLabelWidgetI::changedBgColor( )
{
  _changedBgColor = true;
}

