/***************************************************************************
                   bin.cpp
                             -------------------
    begin                : 11/28/06
    copyright            : (C) 2006 The University of Toronto
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
#include "bin.h"

#include <kgenericfactory.h>

//in
static const QString& INPUT = KGlobal::staticQString("Input Vector");
static const QString& SIZE = KGlobal::staticQString("Bin Size");

//out
static const QString& BINS = KGlobal::staticQString("Bins");

KST_KEY_DATAOBJECT_PLUGIN( bin )

K_EXPORT_COMPONENT_FACTORY( kstobject_bin,
    KGenericFactory<Bin>( "kstobject_bin" ) )

Bin::Bin( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(SIZE, 10.0);
}


Bin::Bin( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(SIZE, 10.0);
}


Bin::~Bin() {
}


// bin the elements into the given size bins
//  additional elements at the end of the input vector are ignored
bool Bin::algorithm() {

  KstVectorPtr input = inputVector(INPUT);
  KstScalarPtr size = inputScalar(SIZE);
  KstVectorPtr bins = outputVector(BINS);
  bool rc = false;

  if (input->length() > 0) {
    int binSize = (int)size->value();

    if (binSize < 1) {
      binSize = 1;
    }
    if (binSize > input->length()) {
      binSize = input->length();
    }

    // allocate the lengths
    bins->resize(input->length() / binSize, false);

    // now bin the data
    for (int i=0; i<bins->length(); i++) {
      bins->value()[i] = 0.0;

      // add up the elements for this bin
      for (int j=0; j<binSize; j++) {
        bins->value()[i] += input->value()[i*binSize+j];
      }

      // find the mean
      bins->value()[i] /= (double)binSize;
    }

    rc = true;
  }

  return rc;
}


QStringList Bin::inputVectorList() const {
  return QStringList( INPUT );
}


QStringList Bin::inputScalarList() const {
  return QStringList( SIZE );
}


QStringList Bin::inputStringList() const {
  return QStringList();
}


QStringList Bin::outputVectorList() const {
  return QStringList( BINS );
}


QStringList Bin::outputScalarList() const {
  return QStringList();
}


QStringList Bin::outputStringList() const {
  return QStringList();
}

#include "bin.moc"
