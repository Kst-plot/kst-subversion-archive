/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void ViewLabelWidget::init()
{
    connect(_scalars, SIGNAL(selectionChanged(const QString &)),
	    this, SLOT(insertScalarInText(const QString &)));
    _horizontal->insertItem(i18n("Left"));
    _horizontal->insertItem(i18n("Right"));
    _horizontal->insertItem(i18n("Center"));

    _horizontal->hide();// FIXME: until implemented - bug 135437
    textLabel5_3->hide();

    _changedFgColor = false;
    _changedBgColor = false;

    connect(_strings, SIGNAL(selectionChanged(const QString &)),
	    this, SLOT(insertStringInText(const QString &)));

    QWhatsThis::add(_text, i18n("<qt>The syntax for labels is a derivative of a subset of LaTeX.  Supported syntax is: <b>\\[greeklettername]</b> and <b>\\[Greeklettername]</b>, <b>\\approx</b>, <b>\\cdot</b>, <b>\\ge</b>, <b>\\geq</b>, <b>\\inf</b>, <b>\\int</b>, <b>\\le</b>, <b>\\leq</b>, <b>\\ne</b>, <b>\\n</b>, <b>\\partial</b>, <b>\\prod</b>, <b>\\pm</b>, <b>\\textcolor{color name}{colored text}</b>, <b>\\textbf{bold text}</b>, <b>\\textit{italicized text}</b>, <b>\\t</b>, <b>\\sum</b>, <b>\\sqrt</b>, <b>\\underline{underlined text}</b>, <b>x^y</b>, <b>x_y</b>.  Scalars, equations, and vector elements can be embedded.  Scalar: <i>[V1/Mean]</i>.  Vector Element: <i>[V1[4]]</i>.  Equation: <i>[=[V1/Mean]^2]</i>.  A [ character can be inserted as <i>\\[</i>."));
}


void ViewLabelWidget::insertScalarInText(const QString &S)
{
    _text->insert("["+S+"]");
}

void ViewLabelWidget::insertStringInText(const QString &S)
{
    _text->insert("["+S+"]");
}

void ViewLabelWidget::changedFgColor( )
{
  _changedFgColor = true;
}

void ViewLabelWidget::changedBgColor( )
{
  _changedBgColor = true;
}

// vim: ts=8 sw=4 noet
