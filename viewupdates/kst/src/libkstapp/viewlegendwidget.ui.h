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
void ViewLegendWidget::init() 
{
    connect(DisplayedCurveList, SIGNAL(clicked(QListBoxItem*)), 
	    this, SLOT(updateButtons()));
    connect(AvailableCurveList, SIGNAL(clicked(QListBoxItem*)),
	    this, SLOT(updateButtons()));
    connect(DisplayedCurveList, SIGNAL(doubleClicked(QListBoxItem*)),
	    this, SLOT(removeDisplayedCurve()));
    connect(AvailableCurveList, SIGNAL(doubleClicked(QListBoxItem*)),
	    this, SLOT(addDisplayedCurve()));
    connect(DisplayedCurveList, SIGNAL(selectionChanged()),
	    this, SLOT(updateButtons()));
    connect(AvailableCurveList, SIGNAL(selectionChanged()),
	    this, SLOT(updateButtons()));
    connect(CurveUnplot, SIGNAL(clicked()),
	    this, SLOT(removeDisplayedCurve()));
    connect(CurvePlot, SIGNAL(clicked()),
	    this, SLOT(addDisplayedCurve()));
}

void ViewLegendWidget::updateButtons()
{    
    bool selected = false;
    uint count = AvailableCurveList->count();

    for (uint i = 0; i < count; i++) {
        if (AvailableCurveList->isSelected(i)) {
            selected = true;
        }
    }
    if (selected && !CurvePlot->isEnabled()) {
        CurvePlot->setEnabled(true);
    } else if (!selected && CurvePlot->isEnabled()) {
        CurvePlot->setEnabled(false);
    }

    selected = false;
    count = DisplayedCurveList->count();
    for (uint i = 0; i < count; i++) {
        if (DisplayedCurveList->isSelected(i)) {
            selected = true;
        }
    }
    if (selected && !CurveUnplot->isEnabled()) {
        CurveUnplot->setEnabled(true);
    } else if (!selected && CurveUnplot->isEnabled()) {
        CurveUnplot->setEnabled(false);
    }
}


void ViewLegendWidget::removeDisplayedCurve()
{
    uint count = DisplayedCurveList->count();

    if (count > 0) {
        for (int i = count-1; i >= 0; i--) {
            if (DisplayedCurveList->isSelected(i)) {
                AvailableCurveList->insertItem(DisplayedCurveList->text(i));
                DisplayedCurveList->removeItem(i);
            }
        }
        updateButtons();
    }
}


void ViewLegendWidget::addDisplayedCurve()
{
    uint count = AvailableCurveList->count();

    if (count > 0) {
        for (int i = count-1; i >= 0; i--) {
            if (AvailableCurveList->isSelected(i)) {
                DisplayedCurveList->insertItem(AvailableCurveList->text(i));
                AvailableCurveList->removeItem(i);
            }
        }
        updateButtons();
    }
}


