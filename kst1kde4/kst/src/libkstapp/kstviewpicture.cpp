/***************************************************************************
                               kstviewpicture.cpp
                             -------------------
    begin                : Jun 14, 2005
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

#include <assert.h>
#include <math.h>

#include <QBitmap>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QPainter>
#include <QTimer>

#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>

#include "kstgfxpicturemousehandler.h"
#include "kst.h"
#include "kstviewobjectfactory.h"
#include "kstviewpicture.h"

KstViewPicture::KstViewPicture()
: KstBorderedViewObject("Picture") {
  _editTitle = i18n("Edit Picture");
  _newTitle = i18n("New Picture");
  _refresh = 0;
  _timer = 0L;
  setTransparent(true);
  _maintainAspect = true;
  _standardActions |= Delete | Edit;
}


KstViewPicture::KstViewPicture(const QDomElement& e)
: KstBorderedViewObject(e) {
  _refresh = 0;
  _timer = 0L;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement();

    if (!el.isNull()) {
      if (metaObject()->indexOfProperty(el.tagName().toLatin1()) > -1) {
        setProperty(el.tagName().toLatin1(), QVariant(el.text()));
      }
    }
    n = n.nextSibling();
  }

  // always have these values
  _type = "Picture";
  _editTitle = i18n("Edit Picture");
  _newTitle = i18n("New Picture");
  setTransparent(true);
  _standardActions |= Delete | Edit;
}


KstViewPicture::KstViewPicture(const KstViewPicture& picture)
: KstBorderedViewObject(picture) {
  _timer = 0L;
  _maintainAspect = picture._maintainAspect;
  _refresh = picture._refresh;
  _url = picture._url;
  doRefresh();

  // always have these values
  _type = "Picture";
  _standardActions |= Delete | Edit;
}


KstViewPicture::~KstViewPicture() {
}


KstViewObject* KstViewPicture::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  Q_UNUSED(name)

  KstViewPicture* viewPicture = new KstViewPicture(*this);
  parent.appendChild(KstViewObjectPtr(viewPicture), true);

  return viewPicture;
}


KstViewObject* KstViewPicture::copyObjectQuietly() const {
  KstViewPicture* viewPicture = new KstViewPicture(*this);

  return viewPicture;
}


QRegion KstViewPicture::clipRegion() {
  if (_clipMask.isEmpty()) {
    QBitmap bm(_geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1);

    if (!bm.isNull()) {
      KstPainter p;

      p.setMakingMask(true);
      p.begin(&bm);
// xxx      p.setViewXForm(true);
      KstBorderedViewObject::paintSelf(p, QRegion());
      paint(p, QRegion());
// xxx      p.flush();
      _clipMask = QRegion(bm);

      p.eraseRect(0, 0, _geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1);
      paintSelf(p, QRegion());
// xxx      p.flush();
      _clipMask |= QRegion(bm);
      p.end();
    }
  }

  return _clipMask;
}


void KstViewPicture::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() != KstPainter::P_PRINT && p.type() != KstPainter::P_EXPORT) {
    if (p.makingMask()) {
      KstBorderedViewObject::paintSelf(p, bounds);
// xxx      p.setRasterOp(Qt::OrROP);
    } else {
      const QRegion clip(clipRegion());
      KstBorderedViewObject::paintSelf(p, bounds);
      p.setClipRegion(bounds & clip);
    }
  } else {
    KstBorderedViewObject::paintSelf(p, bounds);
  }

  const QRect cr(contentsRectForPainter(p));
  if (_image.isNull()) {
    // fill with X
    if (p.makingMask()) {
      p.setBrush(QBrush(Qt::color1, Qt::SolidPattern));
      p.setPen(QPen(Qt::color1, 0, Qt::SolidLine));
    } else {
      p.setBrush(QBrush(Qt::gray, Qt::SolidPattern));
      p.setPen(QPen(Qt::black, 0, Qt::SolidLine));
    }
    p.drawRect(cr);
    p.drawLine(cr.topLeft(), cr.bottomRight());
    p.drawLine(cr.topRight(), cr.bottomLeft());
  } else if (!cr.isNull()) {
    if (_iCache.isNull() || _iCache.size() != cr.size()) {
      _iCache = _image.copy();
      if (!_iCache.isNull()) {
// xxx        _iCache = _iCache.smoothScale(cr.size());
      }
    }

    if (!_iCache.isNull()) {
      if (p.makingMask()) {
        // which indicates clipping / BW mode
/* xxx
        if (_iCache.hasAlphaBuffer()) {
          p.drawImage(cr.topLeft(), _iCache.createAlphaMask());
        } else {
          p.setBrush(Qt::color1);
          p.drawRect(cr);
        }
*/
      } else {
/* xxx
        _iCache.setAlphaBuffer(false);
        p.drawImage(cr.topLeft(), _iCache);
        _iCache.setAlphaBuffer(true);
*/
      }
    }
  }

  p.restore();
}


void KstViewPicture::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstBorderedViewObject::save(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


bool KstViewPicture::setImage(const QString& source) {
  QUrl url;
  QString tmpFile;

  if (QFile::exists(source) && QFileInfo(source).isRelative()) {
    url.setPath(source);
  } else {
// xxx    url = QUrl::fromUserInput(source);
  }

  if (!KIO::NetAccess::exists(url, true, KstApp::inst())) {
    return false;
  }

  if (!KIO::NetAccess::download(url, tmpFile, KstApp::inst())) {
    return false;
  }

  QImage ti;
  bool success = true;

// xxx  ti.setAlphaBuffer(true);
  if (ti.load(tmpFile)) {
    setImage(ti);
    _url = source;

    if (_maintainAspect) { 
      restoreAspect();
    }
  } else {
    success = false;
  }

  KIO::NetAccess::removeTempFile(tmpFile);

  return success;
}


void KstViewPicture::setImage(const QImage& image) {
  _url = QString::null;
  _image = image;
  _iCache = QImage();
  setDirty();
}


const QString& KstViewPicture::url() const {
  return _url;
}


const QImage& KstViewPicture::image() const {
  return _image;
}


void KstViewPicture::doRefresh() {
  if (_url.isEmpty()) {
    setRefreshTimer(0);
  } else {
    QString u = _url;
    bool rc = setImage(_url);
    _url = u;
    // FIXME: after n failures, give up?
    if (rc) {
      KstApp::inst()->paintAll(KstPainter::P_PAINT);
    }
  }
}


void KstViewPicture::setRefreshTimer(int seconds) {
  _refresh = qMax(0, seconds);
  if (_refresh) {
    if (!_timer) {
      _timer = new QTimer(this);

      connect(_timer, SIGNAL(timeout()), this, SLOT(doRefresh()));
    }
    _timer->setSingleShot(true);
    _timer->start(_refresh * 1000);
  } else {
    delete _timer;
    _timer = 0L;
  }
}


bool KstViewPicture::transparent() const {
  bool retVal = false;

  if (!_iCache.isNull()) {
// xxx    retVal = _iCache.hasAlphaBuffer();
  } else {
// xxx    retVal = _image.hasAlphaBuffer();
  }

  return retVal;
}


int KstViewPicture::refreshTimer() const {
  return _refresh;
}


bool KstViewPicture::maintainAspect() const {
  return _maintainAspect;
}


void KstViewPicture::setMaintainAspect(bool maintain) {
  _maintainAspect = maintain;
}


void KstViewPicture::restoreSize() {
  QRect cr(contentsRect());

  cr.setSize(_image.size());
  setContentsRect(cr);
}


void KstViewPicture::restoreAspect() {
  QRect cr(contentsRect());
  QSize size = _image.size();
  
  //
  // find largest rect. which will fit inside original and still preserve aspect...
  //

  size.scale( cr.size().width(), cr.size().height(), Qt::KeepAspectRatio ); 

  cr.setSize(size);
  setContentsRect(cr);
}

QMap<QString, QVariant> KstViewPicture::widgetHints(const QString& propertyName) const {
  QMap<QString, QVariant> map = KstBorderedViewObject::widgetHints(propertyName);

  if (map.empty()) {
    if (propertyName == "path") {
      map.insert(QString("_kst_widgetType"), QString("KURLRequester"));
      map.insert(QString("_kst_label"), i18n("File path"));
    } else if (propertyName == "refreshTimer") {
      map.insert(QString("_kst_widgetType"), QString("QSpinBox"));
      map.insert(QString("_kst_label"), i18n("Refresh timer"));
    } else if (propertyName == "maintainAspect") {
      map.insert(QString("_kst_widgetType"), QString("QCheckBox"));
      map.insert(QString("_kst_label"), QString(""));    
      map.insert(QString("text"), i18n("Maintain aspect ratio"));
    }
  }

  return map;
}


namespace {
KstViewObject *create_KstViewPicture() {
  return new KstViewPicture;
}


KstGfxMouseHandler *handler_KstViewPicture() {
  return new KstGfxPictureMouseHandler;
}

KST_REGISTER_VIEW_OBJECT(Picture, create_KstViewPicture, handler_KstViewPicture)
}


#include "kstviewpicture.moc"

