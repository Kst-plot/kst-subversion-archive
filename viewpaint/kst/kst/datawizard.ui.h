/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
const QString& DataWizard::defaultTag = KGlobal::staticQString("<Auto Name>");

void DataWizard::init()
{
    _configWidget = 0L;
    _inTest = false;
    KST::vectorDefaults.sync();
    QString default_source = KST::vectorDefaults.dataSource();
    _url->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
    setAppropriate(_pageFilters, false);
    setIcon(BarIcon("kst_datawizard"));

    _kstDataRange->update();
    _kstFFTOptions->update();

    _newWindowName->setText(defaultTag);

    setNextEnabled(_pageDataSource, false);
    setNextEnabled(_pageVectors, false);
    setNextEnabled(_pageFilters, true);
    setFinishEnabled(_pagePlot, true);

    disconnect(finishButton(), SIGNAL(clicked()), (QDialog*)this, SLOT(accept()));
    connect(finishButton(), SIGNAL(clicked()), this, SLOT(finished()));
    _vectors->addColumn(i18n("Position"));
    _vectors->setSorting(1);

    // No help button
    setHelpEnabled(_pageDataSource, false);
    setHelpEnabled(_pageVectors, false);
    setHelpEnabled(_pageFilters, false);
    setHelpEnabled(_pagePlot, false);
    _newFilter->setEnabled(false);
    _newFilter->hide(); // FIXME: implement this
    _url->setURL(default_source);
    _url->completionObject()->setDir(QDir::currentDirPath());
    _url->setFocus();
    
    // x vector selection
    connect(_xAxisCreateFromField, SIGNAL(toggled(bool)), _xVector, SLOT(setEnabled(bool)));
    connect(_xAxisUseExisting, SIGNAL(toggled(bool)), _xVectorExisting, SLOT(setEnabled(bool)));
    connect(_xAxisCreateFromField, SIGNAL(clicked()), this, SLOT(xChanged()));
    connect(_xAxisUseExisting, SIGNAL(clicked()), this, SLOT(xChanged()));
    _xAxisCreateFromField->setChecked(true);
    _xVectorExisting->setEnabled(false);
    _xVectorExisting->_newVector->hide();
    _xVectorExisting->_editVector->hide();
}


void DataWizard::destroy()
{
    delete _configWidget;
}


void DataWizard::setInput(const QString& input)
{
    _url->setURL(input);
}


void DataWizard::plotColsChanged()
{
    _reGrid->setChecked(true);
}


bool DataWizard::xVectorOk()
{
    if (!_xAxisGroup->isEnabled()) {
	return true;
    }

    if (_xAxisCreateFromField->isChecked()) {
	QString txt = _xVector->currentText();
	for (int i = 0; i < _xVector->count(); ++i) {
	    if (_xVector->text(i) == txt) {
		return true;
	    }
	}
	return false;
    } else {
	return (KST::vectorList.findTag(_xVectorExisting->selectedVector()) != KST::vectorList.end());
    }
}


bool DataWizard::yVectorsOk()
{
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


void DataWizard::xChanged()
{
  setNextEnabled(_pageVectors, xVectorOk() && yVectorsOk());
}


void DataWizard::testURL()
{
    _inTest = true;
    sourceChanged(_url->url());
    _inTest = false;
}


void DataWizard::sourceChanged(const QString& text)
{
    delete _configWidget;
    _configWidget = 0L;
    _configureSource->setEnabled(false);
    _file = QString::null;
    if (!text.isEmpty() && text != "stdin" && text != "-") {
	KURL url;
	QString txt = _url->completionObject()->replacedPath(text);
	if (QFile::exists(txt) && QFileInfo(txt).isRelative()) {
	    url.setPath(txt);
	} else {
	    url = KURL::fromPathOrURL(txt);
	}

	if (!_inTest && !url.isLocalFile() && url.protocol() != "file" && !url.protocol().isEmpty()) {
	    setNextEnabled(_pageDataSource, false);
	    _fileType->setText(QString::null);
	    _testURL->show();
	    return;
	}

	if (!_inTest) {
	    _testURL->hide();
	}

	if (!url.isValid()) {
	    setNextEnabled(_pageDataSource, false);
	    _fileType->setText(QString::null);
	    return;
	}

	QString file = txt;
	
	KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(file);
	QStringList fl;
	bool complete = false;
	QString fileType;
	int index = 0;
	int count;

	if (ds) {
	    ds->readLock();
	    fl = ds->fieldList();
	    fileType = ds->fileType();
	    complete = ds->fieldListIsComplete();
	    ds->readUnlock();
	    ds = 0L;
	} else {
	    fl = KstDataSource::fieldListForSource(file, QString::null, &fileType, &complete);
	}

	if (!fl.isEmpty() && !fileType.isEmpty()) {
	    if (ds) {
		ds->writeLock();
		_configWidget = ds->configWidget();
		ds->writeUnlock();
	    } else {
		_configWidget = KstDataSource::configWidgetForSource(file, fileType);
	    }
	}

	_configureSource->setEnabled(_configWidget);

	_fileType->setText(fileType.isEmpty() ? QString::null : i18n("Data source of type: %1").arg(fileType));

	if (fl.isEmpty()) {
	    setNextEnabled(_pageDataSource, false);
	    return;
	}

	_vectors->clear();
	_xVector->clear();

	_xVector->insertStringList(fl);
	_xVector->completionObject()->insertItems(fl);
	_xVector->setEditable(!complete);

	count = fl.count();
	if (count > 0) {
	    count = 1+(int)log10((double)count);
	} else {
	    count = 1;
	}
	for (QStringList::ConstIterator it = fl.begin(); it != fl.end(); ++it) {
	    QCheckListItem *pItem = new QCheckListItem(_vectors, *it, QCheckListItem::CheckBox);
	    QString str;
	    str.sprintf("%0*d", count, index);
	    pItem->setText(1, str);
	    ++index;
	}
	_vectors->sort();

	KST::vectorDefaults.sync();
	QString defaultX = KST::vectorDefaults.wizardXVector();
	if (fl.contains(defaultX)) {
	    _xVector->setCurrentText(defaultX);
	} else {
	    _xVector->setCurrentItem(0);
	}                
	_file = file;
    } else {
	_fileType->setText(QString::null);
	setNextEnabled(_pageDataSource, false);
	return;
    }
    setNextEnabled(_pageVectors, xVectorOk() && yVectorsOk());
    setNextEnabled(_pageDataSource, true);
    if (_inTest) {
	_testURL->hide();
    }
}


void DataWizard::fieldListChanged()
{
    bool ok = yVectorsOk();
    _uncheckAll->setEnabled(ok);
    setNextEnabled(_pageVectors, ok && xVectorOk());
}


void DataWizard::showPage( QWidget *page )
{
    if (page == _pageVectors) {
	KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);
	if (!ds) {
	    ds = KstDataSource::loadSource(_file);
	}
	ds->readLock();
	_kstDataRange->setAllowTime(ds && ds->supportsTimeConversions());
	ds->readUnlock();
	_vectorReduction->setFocus();
    } else if (page == _pageFilters) {
	QString save = _filterList->currentText();
	_filterList->clear();
    } else if (page == _pagePlot) {
	QString vectorName = _xAxisCreateFromField->isChecked() ? 
	    _xVector->currentText() : _xVectorExisting->selectedVector();
	KST::vectorDefaults.setWizardXVector(_xVector->currentText());
	KST::vectorDefaults.sync();

	// count the vectors we are about to make, so we can guess defaults
	int n_curves = 0;
	QListViewItemIterator it(_vectors);
	while (it.current()) {
	    QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	    if (i->isOn()) {
		++n_curves;
	    }
	    ++it;
	}

	_psdAxisGroup->setEnabled(_radioButtonPlotDataPSD->isChecked() || _radioButtonPlotPSD->isChecked());

	// set window and plot defaults based on some guesses.
	if (_radioButtonPlotDataPSD->isChecked()) { // plotting both psds and XY plots
	    _newWindows->setEnabled(true);
	    if (n_curves > 1) {
		_newWindows->setChecked(true);
	    } else {
		_newWindow->setChecked(true);
	    }
	} else { // just plotting XY or PSD
	    _newWindows->setEnabled(false);
	    _currentWindow->setChecked(true);
	}
	_multiplePlots->setChecked(true);

	updateWindowBox();
	updatePlotBox();
	updateColumns();
    }

    QWizard::showPage(page);
}


void DataWizard::updateWindowBox()
{
    KstApp *app = KstApp::inst();
    KMdiIterator<KMdiChildView*> *it = app->createIterator();

    _windowName->clear();
    while (it->currentItem()) {
	KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
	if (v) {
	    _windowName->insertItem(v->caption());
	}
	it->next();
    }
    app->deleteIterator(it);

    _existingWindow->setEnabled(_windowName->count() > 0);
    _currentWindow->setEnabled(_windowName->count() > 0 && KstApp::inst()->activeWindow());

    if (!_windowGroup->selected() || !_windowGroup->selected()->isEnabled()) {
	_newWindow->setChecked(true);
    }
}


void DataWizard::updateColumns()
{
    if (_newWindow->isChecked() || _newWindows->isChecked()) {
	_reGrid->setChecked(false);
	return;
    }

    KstViewWindow *v;
    if (_currentWindow->isChecked()) {
	v = static_cast<KstViewWindow*>(KstApp::inst()->activeWindow());
    } else {
	v = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(_windowName->currentText()));
    }

    if (v && v->view()->onGrid()) {
	_plotColumns->setValue(v->view()->columns());
	_reGrid->setChecked(true);
    }
}


void DataWizard::updatePlotBox()
{
    QString psave = _existingPlotName->currentText();
    KstApp *app = KstApp::inst();

    _existingPlotName->clear();

    if (_newWindow->isChecked() || _newWindows->isChecked()) {
	_onePlot->setEnabled(true);
	_multiplePlots->setEnabled(true);
	_cycleThrough->setEnabled(true);
	_plotNumber->setEnabled(_cycleThrough->isChecked());
	_cycleExisting->setEnabled(false);
	_existingPlot->setEnabled(false);
	_existingPlotName->setEnabled(false);
    } else {
	KstViewWindow *v;
	if (_currentWindow->isChecked()) {
	    v = static_cast<KstViewWindow*>(app->activeWindow());
	} else {
	    v = static_cast<KstViewWindow*>(app->findWindow(_windowName->currentText()));
	}

	Kst2DPlotList plots;
	if (v) {
	    plots += v->view()->findChildrenType<Kst2DPlot>();
	}

	for (Kst2DPlotList::ConstIterator i = plots.begin(); i != plots.end(); ++i) {
	    _existingPlotName->insertItem((*i)->tagName());
	}

	bool havePlots = _existingPlotName->count() > 0;
	_onePlot->setEnabled(true);
	_multiplePlots->setEnabled(true);
	_cycleThrough->setEnabled(true);
	_plotNumber->setEnabled(_cycleThrough->isChecked());
	_cycleExisting->setEnabled(havePlots);
	_existingPlot->setEnabled(havePlots);
	_existingPlotName->setEnabled(havePlots && _existingPlot->isChecked());
    }

    if (!_plotGroup->selected()->isEnabled()) {
	_onePlot->setChecked(true);
    }

    if (_existingPlot->isEnabled() && _existingPlotName->listBox() && _existingPlotName->listBox()->findItem(psave)) {
	_existingPlotName->setCurrentText(psave);
    }
    updateColumns();
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


void DataWizard::vectorSubset(const QString& filter)
{
    QListViewItem *after = 0L;
    _vectors->clearSelection();
    _vectors->setSorting(3, true); // Qt 3.1 compat
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
}


void DataWizard::finished()
{
    KstApp *app = KstApp::inst();
    KstVectorList l;
    QString name = KST::suggestVectorName(_xVector->currentText());
    QValueList<QColor> colors;
    QColor color;
    uint n_curves = 0;
    uint n_steps = 0;
    int ptype = 0;
    int prg = 0;
  
    KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);
    if (!ds) {
	ds = KstDataSource::loadSource(_file);
	if (!ds) {
	    KMessageBox::sorry(this, i18n("<qt>Sorry, unable to load the data file '<b>%1</b>'.").arg(_file));
	    return;
	}
	KST::dataSourceList.lock().writeLock();
	KST::dataSourceList.append(ds);
	KST::dataSourceList.lock().writeUnlock();
    }

    // check for sufficient memory
    unsigned long memoryRequested = 0, memoryAvailable = 1024*1024*1024; // 1GB
    unsigned long frames;
#ifdef HAVE_LINUX
    meminfo();
    memoryAvailable = S(kb_main_free + kb_main_buffers + kb_main_cached);
#endif

    ds->writeLock();
    int f0Value, nValue;
    if (_kstDataRange->isStartAbsoluteTime()) {
	f0Value = ds->sampleForTime(_kstDataRange->f0DateTimeValue());
    } else if (_kstDataRange->isStartRelativeTime()) {
	f0Value = ds->sampleForTime(_kstDataRange->f0Value());
    } else {
	f0Value = int(_kstDataRange->f0Value());
    }
    if (_kstDataRange->isRangeRelativeTime()) {
	double nValStored = _kstDataRange->nValue();
	if (_kstDataRange->CountFromEnd->isChecked()) {
	    int frameCount = ds->frameCount(_xVector->currentText());
	    double msCount = ds->relativeTimeForSample(frameCount - 1);
	    nValue = frameCount - 1 - ds->sampleForTime(msCount - nValStored);
	} else {
	    double fTime = ds->relativeTimeForSample(f0Value);
	    nValue = ds->sampleForTime(fTime + nValStored) - ds->sampleForTime(fTime);
	}
    } else {
	nValue = int(_kstDataRange->nValue());
    }

    // only add to memory requirement if xVector is to be created 
    if (_xAxisCreateFromField->isChecked()) {
	if (_kstDataRange->ReadToEnd->isChecked()) {
	    frames = ds->frameCount(_xVector->currentText()) - f0Value;
	} else {
	    frames = kMin(nValue, ds->frameCount(_xVector->currentText()));
	}

	if (_kstDataRange->DoSkip->isChecked()) {
	    memoryRequested += frames / _kstDataRange->Skip->value() * sizeof(double);
	} else {
	    memoryRequested += frames * ds->samplesPerFrame(_xVector->currentText())*sizeof(double);
	}
    }


    // memory estimate for the y vectors
    {
	QListViewItemIterator it(_vectors);
	int fftLen = int(pow(2.0, double(_kstFFTOptions->FFTLen->text().toInt() - 1)));
	while (it.current()) {
	    QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	    if (i->isOn()) {
		QString field = it.current()->text(0);

		if (_kstDataRange->ReadToEnd->isChecked()) {
		    frames = ds->frameCount(field) - f0Value;
		} else {
		    frames = nValue;
		    if (frames > (unsigned long)ds->frameCount(field)) {
			frames = ds->frameCount();
		    }
		}

		if (_kstDataRange->DoSkip->isChecked()) {
		    memoryRequested += frames / _kstDataRange->Skip->value()*sizeof(double);
		} else {
		    memoryRequested += frames * ds->samplesPerFrame(field)*sizeof(double);
		}
		if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
		    memoryRequested += fftLen * 6;
		}
	    }
	    ++it;
	}
    }


    ds->writeUnlock();
    if (memoryRequested > memoryAvailable) {
	KMessageBox::sorry(this, i18n("You requested to read in %1 MB of data but it seems that you only have approximately %2 MB of usable memory available.  You cannot load this much data.").arg(memoryRequested/(1024*1024)).arg(memoryAvailable/(1024*1024)));
	return;
    }

    accept();

    // Pause updates to avoid event storms
    bool wasPaused = app->paused();
    if (!wasPaused) {
	app->setPaused(true);
    }

    {
	QListViewItemIterator it(_vectors);
	while (it.current()) {
	    QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	    if (i->isOn()) {
		if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
		    n_steps++;
		}
		n_steps++;
	    }
	    ++it;
	}
    }

    KstVectorPtr xv;
    
    // only create x vector if needed
    if (_xAxisCreateFromField->isChecked()) {
	n_steps += 1; // for the creation of the x-vector
	prg = 0;
	app->slotUpdateProgress(n_steps, prg, i18n("Creating vectors..."));

	// create the x-vector
	xv = new KstRVector(ds, _xVector->currentText(),
		name, _kstDataRange->CountFromEnd->isChecked() ? -1 : f0Value,
		_kstDataRange->ReadToEnd->isChecked() ? -1 : nValue,
		_kstDataRange->DoSkip->isChecked() ? _kstDataRange->Skip->value() : 0, 
		_kstDataRange->DoSkip->isChecked(),
		_kstDataRange->DoFilter->isChecked());
	KST::addVectorToList(xv);

	app->slotUpdateProgress(n_steps, ++prg, i18n("Creating vectors..."));
    } else {
	xv = *(KST::vectorList.findTag(_xVectorExisting->selectedVector()));
	app->slotUpdateProgress(n_steps, prg, i18n("Creating vectors..."));
    }

    // create the y-vectors
    {
	QListViewItemIterator it(_vectors);
	int vectorCount;

	while (it.current()) {
	    QCheckListItem *i = static_cast<QCheckListItem*>(it.current());
	    if (i->isOn()) {
		vectorCount = KST::vectorList.count();
		name = KST::suggestVectorName(it.current()->text(0));

		KstVectorPtr v = new KstRVector(ds, it.current()->text(0), name,
			_kstDataRange->CountFromEnd->isChecked() ? -1 : f0Value,
			_kstDataRange->ReadToEnd->isChecked() ? -1 : nValue,
			_kstDataRange->DoSkip->isChecked() ? _kstDataRange->Skip->value() : 0, 
			_kstDataRange->DoSkip->isChecked(),
			_kstDataRange->DoFilter->isChecked());
		KST::addVectorToList(v);
		l.append(v);
		++n_curves;
		app->slotUpdateProgress(n_steps, ++prg, i18n("Creating vectors..."));
	    }
	    ++it;
	}
    }

    // get a pointer to the first window
    QString newName;
    KstViewWindow *w;
    if (_newWindow->isChecked() || _newWindows->isChecked()) {
	w = dynamic_cast<KstViewWindow*>(app->activeWindow());
	if (w && w->view()->children().isEmpty()) {
	    // Use the existing view
	    newName = w->caption();
	    if (newName.isEmpty()) {
		newName = w->tabCaption();
	    }
	} else {
	    newName = _newWindowName->text();
	    if (newName == defaultTag) {
		newName = KST::suggestWinName();
	    }
	    w = static_cast<KstViewWindow*>(app->findWindow(app->newWindow(newName)));
	}
    } else if (_currentWindow->isChecked()) {
	w = static_cast<KstViewWindow*>(app->activeWindow());
    } else {
	w = static_cast<KstViewWindow*>(app->findWindow(_windowName->currentText()));
    }

    if (!w) {
	if (!wasPaused) {
	    app->setPaused(false);
	}
	return;
    }

    // create the necessary plots
    app->slotUpdateProgress(n_steps, prg, i18n("Creating plots..."));
    KstViewObjectList plots;
    bool relayout = true;
    if (_onePlot->isChecked()) {
	Kst2DPlotPtr p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
	plots.append(p.data());
	if (_radioButtonPlotDataPSD->isChecked()) {
	    if (_newWindows->isChecked()) {
		newName += "-PSD";
		QString n = app->newWindow(newName);
		w = static_cast<KstViewWindow*>(app->findWindow(n));
	    }
	    Kst2DPlotPtr p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
	    plots.append(p.data());
	    p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
	    p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
	}
    } else if (_multiplePlots->isChecked()) {
	Kst2DPlotPtr p;
	for (uint i = 0; i < l.count(); ++i) {
	    p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
	    plots.append(p.data());
	}
	if (_radioButtonPlotDataPSD->isChecked()) {
	    if (_newWindows->isChecked()) {
                w->view()->cleanup(signed(sqrt(plots.count())));
		newName += "-PSD";
		QString n = app->newWindow(newName);
		w = static_cast<KstViewWindow*>(app->findWindow(n));
	    }
	    for (uint i = 0; i < l.count(); ++i) {
		p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
		plots.append(p.data());
		p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
		p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
	    }
	}
    } else if (_existingPlot->isChecked()) {
	Kst2DPlotPtr p = kst_cast<Kst2DPlot>(w->view()->findChild(_existingPlotName->currentText()));
	plots.append(p.data());
	relayout = false;
    } else if (_cycleExisting->isChecked()) {
	Kst2DPlotList pl = QDeepCopy<Kst2DPlotList>(w->view()->findChildrenType<Kst2DPlot>());
	for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
	    plots += (*i).data();
	}
	relayout = false;
    } else { /* cycle */
	Kst2DPlotPtr p;
	for (int i = 0; i < _plotNumber->value(); ++i) {
	    p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
	    plots.append(p.data());
	}
	if (_radioButtonPlotDataPSD->isChecked()) {
	    if (_newWindows->isChecked()) {
                w->view()->cleanup(signed(sqrt(plots.count())));
		QString n = app->newWindow(newName + "-PSD");
		w = static_cast<KstViewWindow*>(app->findWindow(n));
	    }
	    for (int i = 0; i < _plotNumber->value(); ++i) {
		p = kst_cast<Kst2DPlot>(w->view()->findChild(w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false)));
		plots.append(p.data());
		p->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
		p->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay());
	    }
	}
    }
    // create the data curves
    app->slotUpdateProgress(n_steps, prg, i18n("Creating curves..."));
    KstViewObjectList::Iterator pit = plots.begin();
    for (KstVectorList::Iterator it = l.begin(); it != l.end(); ++it) {
	if (_radioButtonPlotData->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
          name = KST::suggestCurveName((*it)->tagName(), false);
	    Kst2DPlotPtr plot = kst_cast<Kst2DPlot>(*pit);
	    if (plot) {
		KstVCurveList vcurves = kstObjectSubList<KstBaseCurve,KstVCurve>(plot->Curves);
		color = KstColorSequence::next(vcurves, plot->backgroundColor());
	    } else {
		color = KstColorSequence::next();
	    }
	    colors.append(color);
	    KstVCurvePtr c = new KstVCurve(name, xv, *it, 0L, 0L, 0L, 0L, color);
	    if (_drawBoth->isChecked()) {
		c->setHasPoints(true);
		c->Point.setType(ptype++ % KSTPOINT_MAXTYPE);
		c->setHasLines(true);
	    } else if (_drawLines->isChecked()) {
		c->setHasPoints(false);
		c->setHasLines(true);
	    } else {
		c->setHasPoints(true);
		c->Point.setType(ptype++ % KSTPOINT_MAXTYPE);
		c->setHasLines(false);
	    }
	    KST::dataObjectList.lock().writeLock();
	    KST::dataObjectList.append(KstDataObjectPtr(c));
	    KST::dataObjectList.lock().writeUnlock();
	    if (plot) {
		plot->addCurve(KstBaseCurvePtr(c));
	    }
	    if (!_onePlot->isChecked()) { // change plots if we are not onePlot
		if (_radioButtonPlotDataPSD->isChecked()) { // if xy and psd
		    ++pit;
		    if (plots.findIndex(*pit) >= (int)plots.count()/2) {
			pit = plots.begin();
		    }
		} else if (++pit == plots.end()) {
		    pit = plots.begin();
		}
	    }
	}
    }

    if (_onePlot->isChecked()) {
        // if we are one plot, now we can move to the psd plot
	if (++pit == plots.end()) {
	    // if _newWindows is not checked, there will not be another.
	    pit = plots.begin();
	}
    } else if (_radioButtonPlotDataPSD->isChecked()) {
	pit = plots.at(plots.count()/2);
    }

    // create the PSDs
    if (_radioButtonPlotPSD->isChecked() || _radioButtonPlotDataPSD->isChecked()) {
	KstVCurvePtr c;
	int indexColor = 0;
	ptype = 0;
	app->slotUpdateProgress(n_steps, prg, i18n("Creating PSDs..."));
	
	for (KstVectorList::Iterator it = l.begin(); it != l.end(); ++it) {
	    if ((*it)->length() > 0) {
		Kst2DPlotPtr plot;
		KstViewObjectList::Iterator startPlot = pit;
		while (!plot) {
		    plot = kst_cast<Kst2DPlot>(*pit);
		    if (!plot) {
		       	if (++pit == plots.end()) {
			    pit = plots.begin();
			}
			// If this is ever false, we have no valid 2D plots
			// which means that someone wrote some incomplete code
			// or something is really broken
			assert(pit != startPlot);
		    }
		}
	        name = KST::suggestPSDName((*it)->tagName());

		KstPSDPtr p = new KstPSD(name, *it,
			_kstFFTOptions->SampRate->text().toDouble(),
			_kstFFTOptions->Interleaved->isChecked(),
			_kstFFTOptions->FFTLen->text().toInt(),
			_kstFFTOptions->Apodize->isChecked(),
			_kstFFTOptions->RemoveMean->isChecked(),
			_kstFFTOptions->VectorUnits->text(),
			_kstFFTOptions->RateUnits->text(), 
			_kstFFTOptions->ApodizeFxn->currentItem(),
			_kstFFTOptions->Sigma->value());
		if (_radioButtonPlotPSD->isChecked() || colors.count() <= (unsigned long)indexColor) {
		    KstVCurveList vcurves = kstObjectSubList<KstBaseCurve,KstVCurve>(plot->Curves);
		    c = new KstVCurve(KST::suggestCurveName(name, true), p->vX(), p->vY(), 0L, 0L, 0L, 0L, KstColorSequence::next(vcurves, plot->backgroundColor()));
		} else {
		    c = new KstVCurve(KST::suggestCurveName(name, true), p->vX(), p->vY(), 0L, 0L, 0L, 0L, colors[indexColor]);
		    indexColor++;
		}
		if (_drawBoth->isChecked()) {
		    c->setHasPoints(true);
		    c->Point.setType(ptype++ % KSTPOINT_MAXTYPE);
		    c->setHasLines(true);
		} else if (_drawLines->isChecked()) {
		    c->setHasPoints(false);
		    c->setHasLines(true);
		} else {
		    c->setHasPoints(true);
		    c->Point.setType(ptype++ % KSTPOINT_MAXTYPE);
		    c->setHasLines(false);
		}
		KST::dataObjectList.lock().writeLock();
		KST::dataObjectList.append(KstDataObjectPtr(p));
		KST::dataObjectList.append(KstDataObjectPtr(c));
		KST::dataObjectList.lock().writeUnlock();
		plot->addCurve(c.data());
	        plot->setLog(_psdLogX->isChecked(),_psdLogY->isChecked());
		if (!_onePlot->isChecked()) { // change plots if we are not onePlot
		    if (++pit == plots.end()) {
                      if (_radioButtonPlotDataPSD->isChecked()) { // if xy and psd
                        pit = plots.at(plots.count()/2);
                      } else {
			pit = plots.begin();
                      }
		    }
		}
	    }
	}
	app->slotUpdateProgress(n_steps, ++prg, i18n("Creating PSDs..."));
    }

    // legends and labels
    bool xl = _xAxisLabels->isChecked();
    bool yl = _yAxisLabels->isChecked();
    bool tl = _plotTitles->isChecked();

    pit = plots.begin();
    while (pit != plots.end()) {
	Kst2DPlotPtr pp = kst_cast<Kst2DPlot>(*pit);
	if (!pp) {
	    ++pit;
	    continue;
	}
	pp->generateDefaultLabels(xl, yl, tl);
	if (_legendsOn->isChecked()) {
            pp->getOrCreateLegend();
	} else if (_legendsAuto->isChecked()) {
	    if (pp->Curves.count() > 1) {
                pp->getOrCreateLegend();
	    }
	}
	++pit;
    }

    if (_reGrid->isChecked()) {
	w->view()->cleanup(_plotColumns->value());
    } else if (relayout) {
	if (_radioButtonPlotDataPSD->isChecked()) {
	    w->view()->cleanup(signed(sqrt(plots.count()/2)));
	} else {
	    w->view()->cleanup(signed(sqrt(plots.count())));
	}
    } else if (w->view()->onGrid()) {
	w->view()->cleanup(-1);
    }
    w->view()->paint(KstPainter::P_PAINT);
    app->slotUpdateProgress(0, 0, QString::null);
    if (!wasPaused) {
	app->setPaused(false);
    }
}


void DataWizard::applyFiltersChecked( bool on )
{
    setAppropriate(_pageFilters, on);
}


void DataWizard::enableXEntries()
{
    _xAxisGroup->setEnabled(true);
    xChanged();
}


void DataWizard::disableXEntries()
{
    _xAxisGroup->setEnabled(false);
    xChanged();
}


void DataWizard::enablePSDEntries()
{
    _kstFFTOptions->setEnabled(true);
}


void DataWizard::disablePSDEntries()
{
    _kstFFTOptions->setEnabled(false);
}


void DataWizard::_search()
{
    QString s = _vectorReduction->text();
    if (!s.isEmpty()) {
	if (s[0] != '*') {
	    s = "*" + s;
	}
	if (s[s.length()-1] != '*') {
	    s += "*";
	}
	_vectorReduction->setText(s);
    }
}


void DataWizard::_disableWindowEntries()
{
    _windowGroup->setEnabled(false);
}


void DataWizard::_enableWindowEntries()
{
    _windowGroup->setEnabled(true);
}


void DataWizard::markSourceAndSave()
{
    assert(_configWidget);
    KstDataSourcePtr src = static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->instance();
    if (src) {
	src->disableReuse();
    }
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->save();
}


void DataWizard::configureSource()
{
    bool isNew = false;
    KST::dataSourceList.lock().readLock();
    KstDataSourcePtr ds = *KST::dataSourceList.findReusableFileName(_file);
    KST::dataSourceList.lock().readUnlock();
    if (!ds) {
	isNew = true;
	ds = KstDataSource::loadSource(_file);
	if (!ds || !ds->isValid()) {
	    _configureSource->setEnabled(false);
	    return;
	}
    }

    assert(_configWidget);
    KDialogBase *dlg = new KDialogBase(this, "Data Config Dialog", true, i18n("Configure Data Source"));
    if (isNew) {
	connect(dlg, SIGNAL(okClicked()), _configWidget, SLOT(save()));
	connect(dlg, SIGNAL(applyClicked()), _configWidget, SLOT(save()));
    } else {
	connect(dlg, SIGNAL(okClicked()), this, SLOT(markSourceAndSave()));
	connect(dlg, SIGNAL(applyClicked()), this, SLOT(markSourceAndSave()));
    }
    _configWidget->reparent(dlg, QPoint(0, 0));
    dlg->setMainWidget(_configWidget);
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->setInstance(ds);
    static_cast<KstDataSourceConfigWidget*>((QWidget*)_configWidget)->load();
    dlg->exec();
    _configWidget->reparent(0L, QPoint(0, 0));
    dlg->setMainWidget(0L);
    delete dlg;
    sourceChanged(_url->url());
}

// vim: ts=8 sw=4 noet
