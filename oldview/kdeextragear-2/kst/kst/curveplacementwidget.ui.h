/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


bool CurvePlacementWidget::existingPlot()
{
    return _inPlot->isChecked();
}

bool CurvePlacementWidget::newPlot()
{
    return _newPlot->isChecked();
}

void CurvePlacementWidget::setExistingPlot( bool existingPlot )
{
    _inPlot->setChecked(existingPlot);
}

void CurvePlacementWidget::setNewPlot( bool newPlot )
{
    _newPlot->setChecked(newPlot);
}

QString CurvePlacementWidget::plotName()
{
    return _plotList->currentText();
}

int CurvePlacementWidget::columns()
{
    return _plotColumns->value();
}

void CurvePlacementWidget::setPlotList( const QStringList & plots, bool saveSelection )
{
    QString old = QString::null;
    if (saveSelection && _plotList->count()) {
	old = _plotList->currentText();
    }
    _plotList->clear();
    if (!plots.isEmpty()) {
        _plotList->insertStringList(plots);
        if (saveSelection && old != QString::null) {
	    _plotList->setCurrentText(old);
        }
    }
}

void CurvePlacementWidget::setColumns( int c )
{
    _plotColumns->setValue(c);
}

void CurvePlacementWidget::appendPlot( const QString & plot, bool activate )
{
    _plotList->insertItem(plot);
    if (activate) {
	setCurrentPlot(plot);
    }
}

void CurvePlacementWidget::setCurrentPlot( const QString & p )
{
    _plotList->setCurrentText(p);
}
