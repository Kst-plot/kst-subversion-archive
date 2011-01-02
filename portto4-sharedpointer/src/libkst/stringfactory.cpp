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

#include "stringfactory.h"

#include "debug.h"
#include "string_kst.h"
#include "objectstore.h"

namespace Kst {

StringFactory::StringFactory()
: PrimitiveFactory() {
  registerFactory(String::staticTypeTag, this);
}


StringFactory::~StringFactory() {
}


PrimitivePtr StringFactory::generatePrimitive(ObjectStore *store, QXmlStreamReader& xml) {
  QByteArray data;

  Q_ASSERT(store);

  bool orphan, editable;
  QString value, descriptiveName;

  while (!xml.atEnd()) {
      const QString n = xml.name().toString();
    if (xml.isStartElement()) {
      if (n == "string") {
        QXmlStreamAttributes attrs = xml.attributes();
        value = attrs.value("value").toString();
        orphan = attrs.value("orphan").toString() == "true" ? true : false;
        editable = attrs.value("editable").toString() == "true" ? true : false;
        if (attrs.value("descriptiveNameIsManual").toString() == "true") {
          descriptiveName = attrs.value("descriptiveName").toString();
        }
        Object::processShortNameIndexAttributes(attrs);
      } else {
        return PrimitivePtr();
      }
    } else if (xml.isEndElement()) {
      if (n == "string") {
        break;
      } else {
        Debug::self()->log(QObject::tr("Error creating string from Kst file."), Debug::Warning);
        return PrimitivePtr();
      }
    }
    xml.readNext();
  }

  if (xml.hasError()) {
    return PrimitivePtr();
  }

  StringPtr string = store->createObject<String>();
  string->setValue(value);
  string->setOrphan(true);
  string->setEditable(true);
  string->setDescriptiveName(descriptiveName);

  return string;
}


}

// vim: ts=2 sw=2 et
