/***************************************************************************
           kstplotimagedrag.cpp: class for plot image drag objects
                             -------------------
    begin                : Mar 17, 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "kstplotimagedrag.h"
#include "kst2dplot.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kimageio.h>
#include <kprogress.h>
#include <ktempfile.h>

#include <qeventloop.h>
#include <qfile.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <stdlib.h>

KstPlotImageDrag::KstPlotImageDrag(QWidget *dragSource)
: KstDrag("image/png", dragSource) {  // mimetype here is irrelevant
  _mimeTypes = KImageIO::mimeTypes();
  // Prefer image/png, image/jpeg, image/x-eps
  if (_mimeTypes.contains("image/x-eps")) {
    _mimeTypes.remove("image/x-eps");
    _mimeTypes.prepend("image/x-eps");
  }
  if (_mimeTypes.contains("image/jpeg")) {
    _mimeTypes.remove("image/jpeg");
    _mimeTypes.prepend("image/jpeg");
  }
  if (_mimeTypes.contains("image/png")) {
    _mimeTypes.remove("image/png");
    _mimeTypes.prepend("image/png");
  }
}


KstPlotImageDrag::~KstPlotImageDrag() {
}


const char *KstPlotImageDrag::format(int i) const {
  if (i < 0 || i >= static_cast<int>(_mimeTypes.count())) {
    return 0L;
  }
  return _mimeTypes[i].latin1();
}


void KstPlotImageDrag::setPlots(const KstViewObjectList& l) {
  _plots = l;
}


QByteArray KstPlotImageDrag::encodedData(const char *mimeType) const {
  if (!_mimeTypes.contains(QString::fromLatin1(mimeType))) {
    return QByteArray();
  }

  QRect geom(0, 0, 0, 0);
  for (KstViewObjectList::ConstIterator i = _plots.begin(); i != _plots.end(); ++i) {
    geom = geom.unite((*i)->geometry());
  }

  QPixmap pm;
  pm.resize(geom.size());
  pm.fill();

  int prog = 0;
  bool cancelled = false;
  QPainter p(&pm);
  p.setClipping(true);
  KProgressDialog *dlg = new KProgressDialog(0, 0, QString::null, i18n("Generating and storing images of plots..."), true);
  dlg->setAllowCancel(true);
  dlg->progressBar()->setTotalSteps(_plots.count());
  dlg->progressBar()->setValue(prog);
  dlg->show();

  for (KstViewObjectList::Iterator i = _plots.begin(); i != _plots.end(); ++i) {
    // FIXME: make generic across view objects
    Kst2DPlotPtr pp = kst_cast<Kst2DPlot>(*i);
    if (pp) {
      p.setClipRect(pp->geometry());
      p.setViewport(pp->geometry());
      pp->draw(p, P_EXPORT);
    }
    if (dlg->wasCancelled()) {
      cancelled = true;
      break;
    }
    dlg->progressBar()->setValue(++prog);
    kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers);
  }

  delete dlg;

  if (cancelled) {
    return QByteArray();
  }

  QByteArray rc;
  QDataStream ds(rc, IO_WriteOnly);
#if QT_VERSION < 0x030200
  KTempFile tf;
  pm.save(tf.name(), KImageIO::typeForMime(mimeType).latin1());
  tf.close();
  QFile f(tf.name());
  if (f.open(IO_ReadOnly)) {
    rc = f.readAll();
    f.close();
  }
  QFile::remove(tf.name());
#else
  pm.save(ds.device(), KImageIO::typeForMime(mimeType).latin1());
#endif

  return rc;
}


bool KstPlotImageDrag::provides(const char *mimeType) const {
  return _mimeTypes.contains(QString::fromLatin1(mimeType));
}


// vim: ts=2 sw=2 et
