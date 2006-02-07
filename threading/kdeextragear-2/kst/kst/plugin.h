/***************************************************************************
                           plugin.h  -  Part of KST
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

#ifndef _KST_PLUGIN_H
#define _KST_PLUGIN_H

#include <qmap.h>
#include <qvaluelist.h>
#include <qpair.h>
#include <qstring.h>

#include "kstsharedptr.h"

class KLibrary;

class Plugin : public KstShared {
friend class PluginLoader;
public:
  virtual ~Plugin();

  class Data;

  const Data& data() const;

  const QString& xmlFile() const;
  const QString& soFile() const;
  
  int call(const double *const inArrays[], const int inArrayLens[],
           const double inScalars[],
           double *outArrays[], int outArrayLens[],
           double outScalars[]) const;
  int filter(const double *const inArray, int inArrayLen,
             const double inScalars[], double *outArray[],
             int *outArrayLen) const;

 
  static const int CallError;

  class Data {
    public:
      Data() : _filter(false) {}

      /**
       *   Clear all values.
       */
      void clear();

      /**
       *   Types use by this class.
       */
      typedef enum { Unknown, PreAlpha, Alpha, Beta, Release } PluginState;
      typedef enum { String, Integer } ParameterTypes;

      class IOValue {
        public:
          typedef enum { UnknownType,
            TableType,
            StringType,
            MapType,
            IntegerType,
            FloatType
          } ValueType;
          typedef enum { UnknownSubType,
            AnySubType,
            FloatSubType,
            StringSubType,
            IntegerSubType
          } ValueSubType;

          QString _name;
          ValueType _type;
          ValueSubType _subType;
          QString _description;
      };

      // Intro
      bool _filter;
      QString _name;
      QString _author;
      QString _description;
      QString _version;
      PluginState _state;
      // Unimplemented:  platforms, language, documentation

      // Interface
      QValueList<IOValue> _inputs;
      QValueList<IOValue> _outputs;

      // Parameter list
      QMap<QString, QPair<ParameterTypes, QString> > _parameters;
  };


protected:
  Plugin();

  Data _data;

  KLibrary *_lib;

  void *_symbol;

  // Related files
  QString _xmlFile, _soFile;
};


#endif

// vim: ts=2 sw=2 et
