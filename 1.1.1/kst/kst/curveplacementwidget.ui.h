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

void CurvePlacementWidget::setCols( int c )
{
    _plotColumns->setValue(c);
}

void CurvePlacementWidget::setCurrentPlot( const QString & p )
{
    _plotList->setCurrentText(p);
}

void CurvePlacementWidget::newWindow()
{
    KstApp *app = KstApp::inst();
    app->slotFileNewWindow(this);
    update();
}

void CurvePlacementWidget::update()
{
    KstApp *app = KstApp::inst();

    _plotWindow->clear();
    KMdiIterator<KMdiChildView*> *it = app->createIterator();
    while (it->currentItem()) {
	_plotWindow->insertItem(it->currentItem()->caption());
	it->next();
    }
    app->deleteIterator(it);
    KMdiChildView *c = app->activeWindow();
    if (c) {
	_plotWindow->setCurrentItem(c->caption());
    }

    updatePlotList();

    updateEnabled();

    updateGrid();
}

void CurvePlacementWidget::updatePlotList()
{
    KstApp *app = KstApp::inst();
    KMdiChildView *c = app->findWindow(_plotWindow->currentText());

    QString old;
    if (_plotList->count()) {
	old = _plotList->currentText();
    }

    _plotList->clear();
    if (c) {
	Kst2DPlotList plots = static_cast<KstViewWindow*>(c)->view()->findChildrenType<Kst2DPlot>();

	Kst2DPlotList::Iterator i = plots.begin();
	for ( ; i != plots.end(); ++i) {
	    _plotList->insertItem((*i)->tagName());
	}

	if (!old.isNull() && _plotList->count() > 0) {
	    _plotList->setCurrentText(old);
	}
    }
}


void CurvePlacementWidget::updateEnabled()
{
    _plotWindow->setEnabled(_plotWindow->count() > 0);

    _inPlot->setEnabled(_plotList->count() > 0 );

    _plotList->setEnabled(_inPlot->isChecked());
    _reGrid->setEnabled(_newPlot->isChecked());
    _plotColumns->setEnabled(_newPlot->isChecked() && _reGrid->isChecked());
}


void CurvePlacementWidget::updateGrid()
{
    KstApp *app = KstApp::inst();
    KMdiChildView *c = app->findWindow(_plotWindow->currentText());
    KstViewWindow *w = dynamic_cast<KstViewWindow*>(c);
    if (w) {
	KstTopLevelViewPtr view = w->view();
	_reGrid->setChecked(view->onGrid());
	_plotColumns->setValue(view->columns());
    }
}


bool CurvePlacementWidget::reGrid()
{
    return _reGrid->isChecked();
}

// vim: ts=8 sw=4 noet
