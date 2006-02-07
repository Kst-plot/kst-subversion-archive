/***************************************************************************
                                kstfilter.cpp
                             -------------------
    begin                : Dec 02 2003
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

#include "kstfilter.h"
#include "kstdatacollection.h"
#include "plugincollection.h"

#include <qstylesheet.h>

#include <kdebug.h>

#include <assert.h>

KstFilter::KstFilter() : KstPlugin() {
}


KstFilter::KstFilter(QDomElement &e) : KstPlugin(e) {
}


KstFilter::~KstFilter() {
}


void KstFilter::apply(const KstVectorPtr in, KstVectorPtr out) {
  double *inScalars = 0L;
  double *outVector = out->value();
  int outLen = out->length();
  int scalarCnt = 0;

  assert(_plugin.data());

  const QValueList<Plugin::Data::IOValue>& table = _plugin->data()._inputs;
  QValueList<Plugin::Data::IOValue>::ConstIterator it;
  // This might be condensed to _inputScalars.count() but let's be safe for now
  for (it = table.begin(); it != table.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      scalarCnt++;
    }
  }

  inScalars = new double[scalarCnt];
  scalarCnt = 0;
  for (it = table.begin(); it != table.end(); ++it) {
    if ((*it)._type == Plugin::Data::IOValue::FloatType) {
      inScalars[scalarCnt++] = _inputScalars[(*it)._name]->value();
    }
  }


  _plugin->filter(in->value(), in->length(), inScalars, &outVector, &outLen);

  vectorRealloced(out, outVector, outLen);

  delete[] inScalars;
}


bool KstFilter::setPlugin(KstSharedPtr<Plugin> plugin) {
  _plugin = plugin;
  return true;
}


KstFilterSet::KstFilterSet() : KstObjectList<KstFilterPtr>(), KstShared() {
}


KstFilterSet::KstFilterSet(QDomElement& e) : KstObjectList<KstFilterPtr>(), KstShared() {
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setName(e.text());
      } else if (e.tagName() == "filter" && e.hasAttribute("name")) {
        parseFilter(e.attribute("name"), e);
      }
    }
    n = n.nextSibling();
  }
}


KstFilterSet::~KstFilterSet() {
}


void KstFilterSet::parseFilter(const QString& nm, QDomElement& e) {
  KstSharedPtr<Plugin> pp = PluginCollection::self()->plugin(nm);
  if (!pp) {
    return;
  }

  KstFilterPtr fp = new KstFilter;
  fp->setPlugin(pp);
  fp->setTagName(nm);

  KST::scalarList.lock().writeLock();
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "input" && e.hasAttribute("name")) {
        KstScalarPtr sp;
        if (e.attribute("constant").lower() == "true") {
          sp = new KstScalar;
          sp->setValue(e.text().toDouble());
          KST::scalarList.remove(sp);
        } else {
          sp = *KST::scalarList.findTag(e.text());
        }
        fp->inputScalars().insert(e.attribute("name"), sp);
      }
    }
    n = n.nextSibling();
  }
  KST::scalarList.lock().writeUnlock();

  // would be a good idea to iterate through the plugin arguments and be sure
  // that we have all the required inputs before appending.

  append(fp);
}


void KstFilterSet::apply(const KstVectorPtr in, KstVectorPtr out) {
  int left = KstObjectList<KstFilterPtr>::count();

  if (left < 1) { // pass through unfiltered - empty filterset
    out->resize(in->length());
    memcpy(out->value(), in->value(), in->length() * sizeof(double));
    return;
  }

  KstVector *tmpVector1, *tmpVector2;

  Iterator it = begin();
  if (left == 1) {
    (*it)->apply(in, out);
    return;
  }

  tmpVector1 = new KstVector;
  tmpVector2 = new KstVector;
  (*it)->apply(in, tmpVector1);
  ++it;
  --left;

  for (; it != end(); ++it) {
    if (--left == 0) {
      (*it)->apply(tmpVector1, out);
    } else {
      (*it)->apply(tmpVector1, tmpVector2);
    }

    // Swap the vectors
    KstVector *tmp = tmpVector1;
    tmpVector1 = tmpVector2;
    tmpVector2 = tmp;
  }

  // this will delete them
  KST::vectorList.lock().writeLock();
  KST::vectorList.remove(tmpVector1);
  KST::vectorList.remove(tmpVector2);
  KST::vectorList.lock().writeUnlock();
}


void KstFilterSet::save(QTextStream& ts) {
  ts << "  <tag>" << _name << "</tag>" << endl;

  for (Iterator it = begin(); it != end(); ++it) {
    ts << "  <filter name=\"" << QStyleSheet::escape((*it)->tagName())
       << "\">" << endl;
    for (KstScalarMap::ConstIterator i = (*it)->inputScalars().begin(); i != (*it)->inputScalars().end(); ++i) {
      ts << "    <input name=\"" << QStyleSheet::escape(i.key()) << "\""
         << " constant=\"";
      if (i.data()->isGlobal()) {
        ts << "false\">" << QStyleSheet::escape(i.data()->tagName());
      } else {
        ts << "true\">" << i.data()->value();
      }
      ts << "</input>" << endl;
    }
    ts << "  </filter>" << endl;
  }
}


// vim: ts=2 sw=2 et
