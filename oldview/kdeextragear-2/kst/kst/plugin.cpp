/***************************************************************************
                          plugin.cpp  -  Part of KST
                             -------------------
    begin                : Tue May 06 2003
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


#include "plugin.h"

#include <klibloader.h>
#include <kdebug.h>


const int Plugin::CallError = -424242;

Plugin::Plugin() : KstShared() {
  _lib = 0L;
  _symbol = 0L;
  //kdDebug() << "Creating Plugin: " << long(this) << endl;
}


Plugin::~Plugin() {
  _symbol = 0L;

  if (_lib) {
    _lib->unload();  // this deletes it too
    _lib = 0L;
  }

  //kdDebug() << "Destroying Plugin: " << long(this) << endl;
}


const QString& Plugin::xmlFile() const {
  return _xmlFile;
}


const QString& Plugin::soFile() const {
  return _soFile;
}


int Plugin::call(const double *const inArrays[], const int inArrayLens[],
                 const double inScalars[], double *outArrays[],
                 int outArrayLens[], double outScalars[], void** local) const {
  if (!_symbol || _data._filter) {
    return CallError;
  }

  return ((int(*)(const double *const[], const int[],
          const double[], double *[], int[],
          double[], void**))_symbol)
    (inArrays, inArrayLens, inScalars, outArrays, outArrayLens, outScalars, local);
}


int Plugin::call(const double *const inArrays[], const int inArrayLens[],
                 const double inScalars[], double *outArrays[],
                 int outArrayLens[], double outScalars[]) const {
  if (!_symbol || _data._filter) {
    return CallError;
  }

  return ((int(*)(const double *const[], const int[],
          const double[], double *[], int[],
          double[]))_symbol)
    (inArrays, inArrayLens, inScalars, outArrays, outArrayLens, outScalars);
}


int Plugin::filter(const double *const inArray, int inArrayLen,
                 const double inScalars[], double *outArray[],
                 int *outArrayLen) const {
  if (!_symbol || !_data._filter) {
    return CallError;
  }

  return ((int(*)(const double *const, int, const double[], double*[], int*))_symbol) (inArray, inArrayLen, inScalars, outArray, outArrayLen);
}


int Plugin::filter(const double *const inArray, int inArrayLen,
                 const double inScalars[], double *outArray[],
                 int *outArrayLen, void **local) const {
  if (!_symbol || !_data._filter) {
    return CallError;
  }

  return ((int(*)(const double *const, int, const double[], double*[], int*, void**))_symbol) (inArray, inArrayLen, inScalars, outArray, outArrayLen, local);
}


const Plugin::Data& Plugin::data() const {
  return _data;
}



void Plugin::Data::clear() {
  _filter = false;
  _localdata = false;
  _name = QString::null;
  _author = QString::null;
  _description = QString::null;
  _version = QString::null;
  _state = Unknown;
  _isFit = false;
  
  _inputs.clear();
  _outputs.clear();
  _parameters.clear();
}

// vim: ts=2 sw=2 et
