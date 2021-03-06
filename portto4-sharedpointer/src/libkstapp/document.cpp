/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "document.h"
#include "mainwindow.h"
#include "sessionmodel.h"
#include "tabwidget.h"
#include <datasourcefactory.h>
#include <graphicsfactory.h>
#include <datacollection.h>
#include <objectfactory.h>
#include <primitivefactory.h>
#include <relationfactory.h>
#include <viewitem.h>
#include <commandlineparser.h>
#include "objectstore.h"
#include "updatemanager.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>


namespace Kst {

Document::Document(MainWindow *window)
: CoreDocument(), _win(window), _dirty(false), _isOpen(false) {
  _session = new SessionModel(objectStore());

  _fileName.clear();
  UpdateManager::self()->setStore(objectStore());
}


Document::~Document() {
  delete _session;
  _session = 0;
}


SessionModel* Document::session() const {
  return _session;
}


QString Document::fileName() const {
  return _fileName;
}


bool Document::save(const QString& to) {

  QString file = !to.isEmpty() ? to : _fileName;
  if (!file.endsWith(".kst")) {
    file.append(".kst");
  }
  QFile f(file);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    _lastError = QObject::tr("File could not be opened for writing.");
    return false;
  }

  Q_ASSERT(objectStore());

  objectStore()->cleanUpDataSourceList();

  _fileName = file;

  QXmlStreamWriter xml;
  xml.setDevice(&f);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();
  xml.writeStartElement("kst");
  xml.writeAttribute("version", "2.0");

  xml.writeStartElement("data");
  foreach (DataSourcePtr s, objectStore()->dataSourceList()) {
    s->saveSource(xml);
  }
  xml.writeEndElement();

  xml.writeStartElement("variables");

  foreach (VectorPtr s, objectStore()->getObjects<Vector>()) {
    s->save(xml);
  }
  foreach (MatrixPtr s, objectStore()->getObjects<Matrix>()) {
    s->save(xml);
  }
  foreach (ScalarPtr s, objectStore()->getObjects<Scalar>()) {
    s->save(xml);
  }
  foreach (StringPtr s, objectStore()->getObjects<String>()) {
    s->save(xml);
  }
  xml.writeEndElement();

  xml.writeStartElement("objects");
  foreach (DataObjectPtr s, objectStore()->getObjects<DataObject>()) {
    s->save(xml);
  }
  xml.writeEndElement();

  xml.writeStartElement("relations");
  foreach (RelationPtr s, objectStore()->getObjects<Relation>()) {
    s->save(xml);
  }
  xml.writeEndElement();

  xml.writeStartElement("graphics");
  for (int i = 0; i < _win->tabWidget()->count(); ++i) {
    View *v = qobject_cast<View*>(_win->tabWidget()->widget(i));
    xml.writeStartElement("view");
    xml.writeAttribute("name", _win->tabWidget()->tabText(i));

    v->save(xml);

    xml.writeEndElement();
  }
  xml.writeEndElement();

  xml.writeEndDocument();

  setChanged(false);

  return true;
}

bool Document::initFromCommandLine(CommandLineParser *P) {

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  bool ok;
  bool dataPlotted = P->processCommandLine(&ok);

  if (!dataPlotted && ok) {
    QString kstfile = P->kstFileName();
    if (!kstfile.isEmpty()) {
      dataPlotted = open(kstfile);

      if (dataPlotted) {
        UpdateManager::self()->doUpdates(true);
        setChanged(false);
      }
    }
  }
  QApplication::restoreOverrideCursor();

  return ok;
}


#define malformed() \
  return false;


bool Document::open(const QString& file) {
  _isOpen = false;
  QFile f(file);
  if (!f.open(QIODevice::ReadOnly)) {
    _lastError = QObject::tr("File could not be opened for reading.");
    return false;
  }

  _fileName = file;

  // If we move this into the <graphics> block then we could, if desired, open
  // .kst files that contained only data and basically "merge" that data into
  // the current session
  
  View *loadedView = 0;
  bool firstView = true;
  QRectF currentSceneRect;

  QXmlStreamReader xml;
  xml.setDevice(&f);

  enum State { Unknown=0, Data, Variables, Objects, Relations, Graphics, View };
  State state = Unknown;

  while (!xml.atEnd()) {
    if (xml.isStartElement()) {
      QString n = xml.name().toString();
      if (n == "kst") {
      } else if (n == "data") {
        if (state != Unknown) {
          malformed();
        }
        state = Data;
      } else if (n == "variables") {
        if (state != Unknown) {
          malformed();
        }
        state = Variables;
      } else if (n == "objects") {
        if (state != Unknown) {
          malformed();
        }
        state = Objects;
      } else if (n == "relations") {
        if (state != Unknown) {
          malformed();
        }
        state = Relations;
      } else if (n == "graphics") {
        if (state != Unknown) {
          malformed();
        }
        state = Graphics;
      } else {
        switch (state) {
          case Objects:
            {
              DataObjectPtr object = ObjectFactory::parse(objectStore(), xml);
              if (object) {
//                addDataObjectToList(object);
              } else {
                malformed();
              }
              break;
            }
          case Graphics:
            {
              if (n == "view") {
                loadedView = new Kst::View(0);
                if (firstView) {
                  _win->tabWidget()->clear();
                  firstView = false;
                }
                _win->tabWidget()->addView(loadedView);
                QXmlStreamAttributes attrs = xml.attributes();
                loadedView->setObjectName(attrs.value("name").toString());
                _win->tabWidget()->setCurrentViewName(attrs.value("name").toString());
                qreal width = 1.0, height = 1.0;
                QStringRef string = attrs.value("width");
                if (!string.isNull()) {
                   width = string.toString().toDouble();
                }
                string = attrs.value("height");
                if (!string.isNull()) {
                   height = string.toString().toDouble();
                }
                currentSceneRect = QRectF(QPointF(0, 0), QSizeF(width, height));
                state = View;
              } else {
                malformed();
              }
            }
            break;
          case View:
            {

              ViewItem *i = GraphicsFactory::parse(xml, objectStore(), loadedView);
              if (i) {
                loadedView->scene()->addItem(i);
              }
            }
            break;
          case Data:
            DataSourceFactory::parse(objectStore(), xml);
            break;
          case Variables:
            PrimitiveFactory::parse(objectStore(), xml);
            break;
          case Relations:
            RelationFactory::parse(objectStore(), xml);
            break;
          case Unknown:
            malformed();
            break;
        }
      }
    } else if (xml.isEndElement()) {
      QString n = xml.name().toString();
      if (n == "kst") {
        if (state != Unknown) {
          malformed();
        }
        break;
      } else if (n == "view") {
        if (loadedView->sceneRect() != currentSceneRect) {
          loadedView->forceChildResize(currentSceneRect, loadedView->sceneRect());
        }
        state = Graphics;
      } else if (n == "data") {
        state = Unknown;
      } else if (n == "objects") {
        state = Unknown;
      } else if (n == "variables") {
        state = Unknown;
      } else if (n == "relations") {
        state = Unknown;
      } else if (n == "graphics") {
        state = Unknown;
      }
    }
    xml.readNext();
  }

  if (xml.hasError()) {
    _lastError = QObject::tr("File is malformed and encountered an error while reading.");
    return false;
  }

  _vnum = max_vnum+1;
  _xnum = max_xnum+1;
  _pnum = max_pnum+1;
  _csdnum = max_csdnum+1;
  _cnum = max_cnum+1;
  _enum = max_enum+1;
  _hnum = max_hnum+1;
  _inum = max_inum+1;
  _psdnum = max_psdnum+1;
  _tnum = max_tnum+1;
  _mnum = max_mnum+1;


  UpdateManager::self()->doUpdates(true);
  setChanged(false);

  return _isOpen = true;
}

#undef malformed


void Document::createView() {
  _win->tabWidget()->createView();
}


QString Document::lastError() const {
  return _lastError;
}


bool Document::isChanged() const {
  return _dirty;
}


bool Document::isOpen() const {
  return _isOpen;
}


void Document::setChanged(bool dirty) {
  _dirty = dirty;
}

View* Document::currentView() const {
  return _win->tabWidget()->currentView();
}

}

// vim: ts=2 sw=2 et
