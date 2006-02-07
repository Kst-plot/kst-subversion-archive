/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);


void KstDataRange::init()
{
    _time = false;
    unitsChanged(i18n("frames"));
    update();
}

void KstDataRange::clickedCountFromEnd()
{
    if (CountFromEnd->isChecked()) {
	N->setEnabled(true);
	F0->setEnabled(false);
	ReadToEnd->setChecked(false);
    } else {
	F0->setEnabled(true);
    }
}

void KstDataRange::ClickedReadToEnd()
{
    if (ReadToEnd->isChecked()) {
	F0->setEnabled(true);
	N->setEnabled(false);
	CountFromEnd->setChecked(false);
    } else {
	N->setEnabled(true);
    }
}

void KstDataRange::clickedDoSkip()
{
    if (DoSkip->isChecked()) {
	Skip->setEnabled(true);
	DoFilter->setEnabled(true);
    } else {
	Skip->setEnabled(false);
	DoFilter->setEnabled(false);
    }
}

void KstDataRange::updateEnables() {
    if (DoSkip->isChecked()) {
	Skip->setEnabled(true);
	DoFilter->setEnabled(true);
    } else {
	Skip->setEnabled(false);
	DoFilter->setEnabled(false);
    }

    if (CountFromEnd->isChecked()) {
	N->setEnabled(true);
	F0->setEnabled(false);
	ReadToEnd->setChecked(false);
    } else if (ReadToEnd->isChecked()) {
	F0->setEnabled(true);
	N->setEnabled(false);
    } else {
	N->setEnabled(true);
	F0->setEnabled(true);
    }

}

void KstDataRange::update()
{
    CountFromEnd->setChecked(KST::vectorDefaults.countFromEOF());
    ReadToEnd->setChecked(KST::vectorDefaults.readToEOF());
    F0->setText(QString::number(KST::vectorDefaults.f0(), 'g', 15));
    N->setText(QString::number(KST::vectorDefaults.n(), 'g', 15));
    Skip->setValue(KST::vectorDefaults.skip());
    DoSkip->setChecked(KST::vectorDefaults.doSkip());
    DoFilter->setChecked(KST::vectorDefaults.doAve());

    clickedCountFromEnd();
    ClickedReadToEnd();
    clickedDoSkip();
}

void KstDataRange::setAllowTime(bool allow)
{
    if (allow != _time) {
	_time = allow;
	_units->clear();
	_units->insertItem(i18n(KST::timeDefinitions[0].name, KST::timeDefinitions[0].description));
	if (_time) {
	    for (int i = 1; KST::timeDefinitions[i].name; ++i) {
		_units->insertItem(i18n(KST::timeDefinitions[i].name, KST::timeDefinitions[i].description));
	    }
	}
    }
}

void KstDataRange::unitsChanged(const QString& unit)
{
    /*
    F0->setPrefix(QString::null);
    F0->setSuffix(QString(" ") + unit);
    N->setPrefix(QString::null);
    N->setSuffix(QString(" ") + unit);
    */
}

void KstDataRange::setF0Value(double v)
{
    F0->setText(QString::number(v, 'g', 15));
}

void KstDataRange::setNValue(double v)
{
    N->setText(QString::number(v, 'g', 15));
}

double KstDataRange::interpret(const char *txt)
{
    yy_scan_string(txt);
    int rc = yyparse();
    if (rc == 0) {
	Equation::Node *eq = static_cast<Equation::Node*>(ParsedEquation);
	ParsedEquation = 0L;
	Equation::Context ctx;
	ctx.sampleCount = 2;
	ctx.noPoint = KST::NOPOINT;
	ctx.x = 0.0;
	ctx.xVector = 0L;
	Equation::FoldVisitor vis(&ctx, &eq);
	double v = eq->value(&ctx);
	delete eq;
	return v;
    } else {
	return 0.0;
    }
}

namespace {
    inline int d2i(double x) {
	return int(floor(x+0.5));
    }
}

int KstDataRange::f0Value()
{
    return ::d2i(interpret(F0->text().latin1()) * KST::timeDefinitions[_units->currentItem()].factor);
}

int KstDataRange::nValue()
{
    return ::d2i(interpret(N->text().latin1()) * KST::timeDefinitions[_units->currentItem()].factor);
}

bool KstDataRange::isTime()
{
    return _units->currentItem() > 0;
}

// vim: ts=8 sw=4 noet
