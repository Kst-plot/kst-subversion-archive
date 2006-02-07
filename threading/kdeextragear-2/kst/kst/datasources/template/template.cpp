/***************************************************************************
                    template.cpp  -  data source template
                             -------------------
    begin                : Fri Oct 17 2003
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

#include "template.h"

 
TemplateSource::TemplateSource(const QString& filename, const QString& type)
: KstDataSource(filename, type) {
}


TemplateSource::~TemplateSource() {
}


KstObject::UpdateType TemplateSource::update(int u) {
  Q_UNUSED(u)
  return KstObject::NO_CHANGE;
}


int TemplateSource::readField(double *v, const QString& field, int s, int n) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)
  return -1;
}


bool TemplateSource::isValidField(const QString& field) const {
  Q_UNUSED(field)
  return false;
}


int TemplateSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 0;
}


int TemplateSource::frameCount() const {
  return 0;
}


QString TemplateSource::fileType() const {
  return QString::null;
}


void TemplateSource::save(QTextStream &ts) {
  KstDataSource::save(ts);
}


extern "C" {
KstDataSource *create_template(const QString& filename, const QString& type) {
  return new TemplateSource(filename, type);
}

QStringList provides_template() {
  QStringList rc;
  // create the stringlist
  return rc;
}

bool understands_template(const QString& filename) {
  // determine if it is an X file
  Q_UNUSED(filename)
  return false;
}

}

// vim: ts=2 sw=2 et
