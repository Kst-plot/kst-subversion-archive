/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void DataWizard::init()
{
    setIcon(BarIcon("kst_datawizard"));
    _source->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
		    | KFile::LocalOnly);

    setNextEnabled(_pageDataSource, false);
    setNextEnabled(_pageVectors, false);
    setNextEnabled(_pageFilters, true);
    setFinishEnabled(_pagePlot, true);
    _newFilter->setEnabled(false); // FIXME - implement first
    
    connect(finishButton(), SIGNAL(clicked()), this, SLOT(finished()));
    
    // No help button
    setHelpEnabled(_pageDataSource, false);
    setHelpEnabled(_pageVectors, false);
    setHelpEnabled(_pageFilters, false);
    setHelpEnabled(_pagePlot, false);
}


bool DataWizard::xVectorOk() {
    QString txt = _xVector->currentText();

    for (int i = 0; i < _xVector->count(); ++i) {
	if (_xVector->text(i) == txt) {
	    return true;
	}
    }
    return false;
}


bool DataWizard::yVectorsOk() {
    QListViewItemIterator it(_vectors);
    while (it.current()) {
	QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	if (i->isOn()) {
	    return true;
	}
	++it;
    }

    return false;
}


void DataWizard::xChanged( const QString & txt )
{
    bool found = false;

    for (int i = 0; i < _xVector->count(); ++i) {
	if (_xVector->text(i) == txt) {
	    found = true;
	    break;
	}
    }

    setNextEnabled(_pageVectors, found && yVectorsOk());
}


void DataWizard::sourceChanged( const QString & txt )
{
    bool ok = false;
    
    if (!txt.isEmpty()) {
	KstDataSourcePtr ds = *KST::dataSourceList.findFileName(txt);
	
	if (!ds && txt != "stdin" && txt != "-") { // FIXME: deal with stdin properly
	    ds = KstDataSource::loadSource(txt);
	    if (ds) {
		KST::dataSourceList.append(ds);
	    }
	}

	if (ds && _ds == ds) {
	    return;
	}
	
	_ds = ds;
	
	_vectors->clear();
	_xVector->clear();
	_xVector->insertItem("INDEX");
	_xVector->completionObject()->addItem("INDEX");
	if (_ds) {
	    ok = true;
	    QStringList fl = _ds->fieldList();
	    _xVector->insertStringList(fl);
	    _xVector->completionObject()->insertItems(fl);
	    for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
		new QCheckListItem(_vectors, *it, QCheckListItem::CheckBox);
	    }
	}
    } else if (!_ds) {
	return;
    } else {
	_ds = 0L; // deref
    }
    
    setNextEnabled(_pageVectors, xVectorOk() && yVectorsOk());
    setNextEnabled(_pageDataSource, ok);
}


void DataWizard::fieldListChanged()
{
    bool ok = yVectorsOk();
    _uncheckAll->setEnabled(ok);
    setNextEnabled(_pageVectors, ok && xVectorOk());
}


void DataWizard::showPage( QWidget *page )
{
    if (page == _pageFilters) {
	QString save = _filterList->currentText();
	_filterList->clear();
	for (KstFilterSetList::Iterator it = KST::filterSetList.begin(); it != KST::filterSetList.end(); ++it) {
	    _filterList->insertItem((*it)->name());
	}
	
	QListBoxItem *i = _filterList->findItem(save);
	if (i) {
	    _filterList->setSelected(i, true);
	}
    } else if (page == _pagePlot) {
	QString save = _existingPlotName->currentText();
	_existingPlotName->clear();
	QStringList pl = KST::plotList.tagNames();
	_existingPlotName->insertStringList(pl);
	_existingPlot->setEnabled(!pl.isEmpty());
	if (KST::plotList.FindKstPlot(save)) {
		_existingPlotName->setCurrentText(save);
	}

	_cycleExisting->setEnabled(!pl.isEmpty());

        if (!_filterList->currentText().isEmpty() && (!_filter || _filter->name() != _filterList->currentText())) {
		_filter = KST::filterSetList.find(_filterList->currentText());
	} else {
		_filter = 0L;
	}
    }

    QWizard::showPage(page);
}


void DataWizard::uncheckAll()
{
    QListViewItemIterator it(_vectors);
    while (it.current()) {
	QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	i->setOn(false);
	++it;
    }
    
    _uncheckAll->setEnabled(false);
    setNextEnabled(_pageVectors, false);
}


void DataWizard::vectorSubset( const QString & filter )
{
    QListViewItem *after = 0L;
    _vectors->clearSelection();
    _vectors->setSorting(2, true); // Qt 3.1 compat
    QRegExp re(filter, true /* case insensitive */, true /* wildcard */);
    QListViewItemIterator it(_vectors);
    while (it.current()) {
	QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	++it;
	if (re.exactMatch(i->text(0))) {
	    if (!after) {
		_vectors->takeItem(i);
		_vectors->insertItem(i);
	    } else {
		i->moveItem(after);
	    }
	    after = i;
	    i->setSelected(true);
	}
    }
}


void DataWizard::checkSelected()
{
    bool ok = false;
    QListViewItemIterator it(_vectors);
    while (it.current()) {
	QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	if (i->isSelected()) {
	    i->setOn(true);
	    i->setSelected(false);
	    ok = true;
	}
	++it;
    }
    
    _vectorReduction->setText(QString::null);
    
    if (ok) {
	_uncheckAll->setEnabled(true);
	setNextEnabled(_pageVectors, xVectorOk());
    }
}


void DataWizard::newFilter()
{
    // FIXME
}


void DataWizard::finished()
{
    int prg = 0;
    KProgressDialog *dlg = new KProgressDialog(this, "Progress Dialog");

    dlg->setAllowCancel(false);
    dlg->progressBar()->setTotalSteps(2 * _vectors->childCount());
    dlg->progressBar()->setProgress(0);
    dlg->setLabel(i18n("Creating curves and plots..."));
    dlg->show();

    KstVectorList l;

    QString name = QString("V%1-%2").arg(KST::vectorList.count()).arg(_xVector->currentText());
    KstVectorPtr xv;
    xv = new KstRVector(_ds, _xVector->currentText(),
		    name, _fromEnd->isChecked() ? -1 : _startFrame->value(),
		    _readToEnd->isChecked() ? -1 : _numFrames->value(),
		    _framesToSkip->value(), _skipFrames->isChecked(),
		    _boxcar->isChecked());
    KST::addVectorToList(xv);

    // Create the vectors
    QListViewItemIterator it(_vectors);
    while (it.current()) {
	QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	if (i->isOn()) {
	    name = QString("V%1-%2").arg(KST::vectorList.count()).arg(it.current()->text(0));
	    KstVectorPtr v = new KstRVector(_ds, it.current()->text(0), name,
			    _fromEnd->isChecked() ? -1 : _startFrame->value(),
			    _readToEnd->isChecked() ? -1 : _numFrames->value(),
			    _framesToSkip->value(), _skipFrames->isChecked(),
			    _boxcar->isChecked());
            KST::addVectorToList(v);
	    if (_filter) {
		l.append(new KstFilteredVector(v, _filter));
	    } else {
		l.append(v);
	    }
	}
        dlg->progressBar()->setProgress(++prg);
	++it;
    }

    QPtrList<KstPlot> plots;
    bool relayout = true;

    // Generate the plots
    if (_onePlot->isChecked()) {
	KstPlot *p = new KstPlot(KST::plotList.generatePlotName());
	if (_legends->isChecked()) {
	    p->Legend->setShow(true);
	    p->Legend->setFront(true);
	}
	plots.append(p);
	KST::plotList.append(p);
    } else if (_multiplePlots->isChecked()) {
	KstPlot *p;
	for (uint i = 0; i < l.count(); ++i) {
	    p = new KstPlot(KST::plotList.generatePlotName());
	    if (_legends->isChecked()) {
		p->Legend->setShow(true);
		p->Legend->setFront(true);
	    }
	    plots.append(p);
	    KST::plotList.append(p);
	}
    } else if (_existingPlot->isChecked()) {
        plots.append(KST::plotList.FindKstPlot(_existingPlotName->currentText()));
        relayout = false;
    } else if (_cycleExisting->isChecked()) {
	plots = KST::plotList;
        relayout = false;
    } else { /* cycle */
	KstPlot *p;
	for (int i = 0; i < _plotNumber->value(); ++i) {
	    p = new KstPlot(KST::plotList.generatePlotName());
	    if (_legends->isChecked()) {
		p->Legend->setShow(true);
		p->Legend->setFront(true);
	    }
	    plots.append(p);
	    KST::plotList.append(p);
	}
    }

    QPtrListIterator<KstPlot> pit(plots);

    // Plot them
    int ptype = 0;
    for (KstVectorList::Iterator it = l.begin(); it != l.end(); ++it) {
	 name = QString("C%1-%2").arg(KST::dataObjectList.count()).arg((*it)->tagName());
	while (KST::dataObjectList.findTag(name) != KST::dataObjectList.end()) {
	    name += "'";
	}
	KstBaseCurvePtr c = new KstVCurve(name, xv, *it, 0L, 0L, KstColorSequence::next());
	if (_drawBoth->isChecked()) {
	    c->setHasPoints(true);
	    c->Point.setType(ptype++ % 8);
	    c->setHasLines(true);
	} else if (_drawLines->isChecked()) {
	    c->setHasPoints(false);
	    c->setHasLines(true);
	} else {
	    c->setHasPoints(true);
	    c->Point.setType(ptype++ % 8);
	    c->setHasLines(false);
	}
	KST::dataObjectList.append(KstDataObjectPtr(c));
	pit.current()->addCurve(c);
	if (pit.atLast()) {
		pit.toFirst();
	} else {
		++pit;
	}
        dlg->progressBar()->setProgress(++prg);
    }

    bool xl = _xAxisLabels->isChecked();
    bool yl = _yAxisLabels->isChecked();
    bool tl = _plotTitles->isChecked();

    if (xl || yl || tl) {
	pit.toFirst();
	while (pit.current()) {
	    pit.current()->GenerateDefaultLabels();
	    if (!xl) {
		pit.current()->setXLabel(QString::null);
	    } 
	    if (!yl) {
		pit.current()->setYLabel(QString::null);
	    } 
	    if (!tl) {
		pit.current()->setTopLabel(QString::null);
	    } 
	    ++pit;
	}
    }

    if (relayout) {
	KST::plotList.arrangePlots(QMAX(KST::plotList.getPlotCols(), unsigned(sqrt(plots.count()))));
    }

    delete dlg;
}
