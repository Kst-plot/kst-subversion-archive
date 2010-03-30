/***************************************************************************
        kstviewobjectimagedrag.cpp: class for view object image drag objects
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

#include <stdlib.h>

#include <QEventLoop>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QProgressDialog>
#include <QTemporaryFile>

#include <kapplication.h>
#include <kimageio.h>
#include <klocale.h>

#include "kstviewobjectimagedrag.h"

KstViewObjectImageDrag::KstViewObjectImageDrag(QWidget *dragSource)
: KstDrag("image/png", dragSource) {  // mimetype here is irrelevant
  _mimeTypes = KImageIO::mimeTypes();
  // Prefer image/png, image/jpeg, image/x-eps
  if (_mimeTypes.contains("image/x-eps")) {
    _mimeTypes.removeAll("image/x-eps");
    _mimeTypes.prepend("image/x-eps");
  }
  if (_mimeTypes.contains("image/jpeg")) {
    _mimeTypes.removeAll("image/jpeg");
    _mimeTypes.prepend("image/jpeg");
  }
  if (_mimeTypes.contains("image/png")) {
    _mimeTypes.removeAll("image/png");
    _mimeTypes.prepend("image/png");
  }
}


KstViewObjectImageDrag::~KstViewObjectImageDrag() {
}


const char *KstViewObjectImageDrag::format(int i) const {
  if (i < 0 || i >= static_cast<int>(_mimeTypes.count())) {
    return 0L;
  }
  return _mimeTypes[i].toLatin1();
}


void KstViewObjectImageDrag::setObjects(const KstViewObjectList& l) {
  _objects = l;
}


QByteArray KstViewObjectImageDrag::encodedData(const char *mimeType) const {
  if (!_mimeTypes.contains(QString::fromLatin1(mimeType))) {
    return QByteArray();
  }

  QRect geom(0, 0, 0, 0);
  for (KstViewObjectList::ConstIterator i = _objects.begin(); i != _objects.end(); ++i) {
    geom = geom.unite((*i)->geometry());
  }

  QPixmap pm;
// xxx  pm.resize(geom.size());
  pm.fill();

  int prog = 0;
  bool cancelled = false;
  KstPainter p(KstPainter::P_EXPORT);

  p.begin(&pm);
  p.setClipping(true);

  QProgressDialog *dlg = new QProgressDialog(0L);

// xxx  dlg->setAllowCancel(true);
  dlg->setRange(0, _objects.count());
  dlg->setValue(prog);
  dlg->show();

  for (KstViewObjectList::Iterator i = _objects.begin(); i != _objects.end(); ++i) {
    p.setClipRect((*i)->geometry());
    p.setViewport((*i)->geometry());
    (*i)->paint(p, QRegion());
    if (dlg->wasCanceled()) {
      cancelled = true;
      break;
    }
    dlg->setValue(++prog);
// xxx    kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers);
  }
  p.end();

  delete dlg;

  if (cancelled) {
    return QByteArray();
  }

  QByteArray rc;
  QDataStream ds(&rc, QIODevice::WriteOnly);

  pm.save(ds.device(), KImageIO::typeForMime(mimeType).first().toLatin1());

  return rc;
}


bool KstViewObjectImageDrag::provides(const char *mimeType) const {
  return _mimeTypes.count(QString::fromLatin1(mimeType));
}
