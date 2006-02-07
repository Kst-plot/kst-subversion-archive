/***************************************************************************
                      pluginloader.cpp  -  Part of KST
                             -------------------
    begin                : Mon May 12 2003
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


#include "pluginloader.h"
#include "pluginxmlparser.h"

#include <klibloader.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <qfile.h>
#include <qregexp.h>


PluginLoader *PluginLoader::_self = 0L;
static KStaticDeleter<PluginLoader> _plSelf;

PluginLoader *PluginLoader::self() {
  if (!_self) {
    _self = _plSelf.setObject(new PluginLoader);
  }

return _self;
}


PluginLoader::PluginLoader() {
  _parser = new PluginXMLParser;
}


PluginLoader::~PluginLoader() {
  delete _parser;
  _parser = 0L;
}


Plugin *PluginLoader::loadPlugin(const QString& xmlfile, const QString& object) {
  // First try to load the XML file.
  if (_parser->parseFile(xmlfile) != 0) {
    kdDebug() << "Couldn't parse xml file " << xmlfile << "." << endl;
    return 0L;
  }

  // It parsed, create a new plugin and copy in the data
  Plugin *plug = new Plugin;
  plug->_data = _parser->data();

  // Load the plugin
  plug->_lib = KLibLoader::self()->library(object.local8Bit().data());
  if (!plug->_lib) {
    kdDebug() << "Couldn't find library " << object << "." << endl;
    delete plug;
    return 0L;
  }

  plug->_symbol = plug->_lib->symbol(plug->_data._name.latin1());

  if (!plug->_symbol) {
    kdDebug() << "Couldn't find symbol " << plug->_data._name << " in plugin." << endl;
    delete plug;
    return 0L;
  }

  plug->_xmlFile = xmlfile;
  plug->_soFile = object;

return plug;
}


// vim: ts=2 sw=2 et
