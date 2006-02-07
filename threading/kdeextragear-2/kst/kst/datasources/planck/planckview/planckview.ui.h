/***************************************************************************
                           planckview.ui.h 
    begin                : Thu Oct 23 2003
    copyright            : (C) 2003 The University of Toronto
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

void PlanckView::fileNew()
{
    _view->clear();
    _filename = QString::null;
}

void PlanckView::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(QString::null, "*.xml", this);
    if (!fn.isEmpty()) {
	QFile f(fn);
	if (!f.open(IO_ReadOnly)) {
	    QMessageBox::warning(this, tr("PLANCK Database"), tr("Unable to open file %1.").arg(fn));
	    return;
	}
	parse(&f);
	_filename = fn;
    }
}

void PlanckView::fileSave()
{
    QFile f(_filename);
    if (!f.open(IO_WriteOnly | IO_Truncate)) {
	QMessageBox::warning(this, tr("PLANCK Database"), tr("Unable to save data to file %1.").arg(_filename));
	return;
    }

    QDomDocument doc(_docType);

    int groups = _view->childCount() + 1;

    QProgressDialog *dlg = new QProgressDialog(this);
    dlg->show();
    int cnt = 0;

    QDomElement db = doc.createElement("database");
    
    for (QListViewItem *i = _view->firstChild(); i; i = i->nextSibling()) {
	QDomElement grp = doc.createElement("group");
	GroupItem *gi = static_cast<GroupItem*>(i);
	(*gi) >> grp;

	for (QListViewItem *o = i->firstChild(); o; o = o->nextSibling()) {
	    QDomElement obj = doc.createElement("object");
	    ObjectItem *oi = static_cast<ObjectItem*>(o);
	    (*oi) >> obj;
	    grp.appendChild(obj);
	}

	db.appendChild(grp);

	dlg->setProgress(++cnt, groups);
    }

    doc.appendChild(db);

    QTextStream ts(&f);
    ts << doc.toString();
    dlg->hide();
    delete dlg;
}

void PlanckView::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(QString::null, "*.xml", this);
    if (!fn.isEmpty()) {
	_filename = fn;
	fileSave();
    }
}

void PlanckView::fileExit()
{
    close();
}

void PlanckView::init()
{
}

void PlanckView::parse( QFile *f )
{
    QDomDocument doc;
    int errline = 0, errcol = 0;
    QString err;
    if (!doc.setContent(f, &err, &errline, &errcol)) {
	QMessageBox::warning(this, tr("PLANCK Database"), tr("Error parsing database.\n%1:%2: %3").arg(errline).arg(errcol).arg(err));
	return;
    }
    
    _view->clear();

    _docType = doc.doctype();

    for (QDomNode d = doc.firstChild(); !d.isNull(); d = d.nextSibling()) {
	if (!d.isElement() || d.toElement().tagName().lower() != "database") {
	    continue;
	}

	for (QDomNode n = d.toElement().firstChild(); !n.isNull(); n = n.nextSibling()) {
	    if (n.isElement()) {
		QDomElement e = n.toElement();
		if (e.tagName().lower() != "group") {
		    continue;
		}

		GroupItem *i = new GroupItem(_view);
		(*i) << e;

		for (QDomNode o = e.firstChild(); !o.isNull(); o = o.nextSibling()) {
		    if (o.isElement()) {
			QDomElement oe = o.toElement();
			if (oe.tagName().lower() != "object") {
			    continue;
			}

			ObjectItem *oi = new ObjectItem(i);
			(*oi) << oe;
		    }
		}
	    }
	}
    }
}

void PlanckView::edit( QListViewItem *item )
{
    ObjectItem *oi = dynamic_cast<ObjectItem*>(item);
    if (oi) {
	PlanckObjectEditor *e = new PlanckObjectEditor(this);
	e->setItem(oi);
	e->exec();
	delete e;
    }
}


// vim: ts=8 sw=4 noet
