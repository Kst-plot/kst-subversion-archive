/***************************************************************************
                          kstavector.cpp - a vector with editable points.
                             -------------------
    begin                : april, 2005
    copyright            : (C) 2005 by cbn
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
// use KCodecs::base64Encode() in kmdcodecs.h
// Create QDataStream into a QByteArray
// qCompress the bytearray

#include "kstavector.h"
#include "kstdebug.h"
#include <qcstring.h>
#include <qstylesheet.h>
#include <klocale.h>
#include <kmdcodec.h>

KstAVector::KstAVector(const QDomElement &e)
: KstVector() {
  _editable = true;
  int in_n = 2;
  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "N") {
        in_n = e.text().toInt();
      }
    }
    n = n.nextSibling();
  }
  _saveable = true;
  resize(in_n, true);

  if (in_n > 0) {
    QDomNode n = e.firstChild();
    while (!n.isNull()) {
      QDomElement e = n.toElement();
      if (!e.isNull()) {
        if (e.tagName() == "data") {
          QCString qcs(e.text().latin1());
          QByteArray qbca;
          KCodecs::base64Decode(qcs, qbca);
          QByteArray qba = qUncompress(qbca);
          QDataStream qds(qba, IO_ReadOnly);
          int i;
          for (i = 0; i < in_n && !qds.atEnd(); i++) {
            qds >> _v[i];
          }
          if (i < in_n) {
            KstDebug::self()->log(i18n("Saved vector contains less data than it claims."), KstDebug::Warning);
            resize(i, false);
          }
        }
      }
      n = n.nextSibling();
    }
  }
}


KstAVector::KstAVector(int n, const QString& tag)
: KstVector(tag, n) {
  _editable = true;
  _saveable = true;
  resize(n, true);
}


void KstAVector::save(QTextStream &ts, const QString& indent, bool saveAbsolutePosition) {
  Q_UNUSED(saveAbsolutePosition)

  QByteArray qba(length()*sizeof(double));
  QDataStream qds(qba, IO_WriteOnly);

  for (int i = 0; i < length(); i++) {
    qds << _v[i];
  }

  ts << indent << "<avector>" << endl;

  ts << indent << "  <tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << indent << "  <N>" << length() << "</N>" << endl;
  ts << indent << "  <data>" << KCodecs::base64Encode(qCompress(qba)) << "</data>" << endl;
  ts << indent << "</avector>" << endl;
}


KstObject::UpdateType KstAVector::update(int update_counter) {
  bool force = dirty();

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  KstObject::UpdateType baseRC = KstVector::update(update_counter);
  if (force) {
    baseRC = UPDATE;
  }

  return baseRC;
}


// vim: ts=2 sw=2 et
