/***************************************************************************
                   plugincollection.cpp  -  Part of KST
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


#include "plugincollection.h"
#include "pluginloader.h"
#include "pluginxmlparser.h"
#include "plugin.h"

#include <kdebug.h>
#include <klibloader.h>
#include <kstaticdeleter.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <qregexp.h>
#include <qdir.h>
#include <qfileinfo.h>

PluginCollection *PluginCollection::_self = 0L;
static KStaticDeleter<PluginCollection> _pcSelf;

PluginCollection *PluginCollection::self() {
  if (!_self) {
    _self = _pcSelf.setObject(new PluginCollection);
  }

return _self;
}


PluginCollection::PluginCollection() 
: QObject(0L, "KST Plugin Collection") {
  KGlobal::dirs()->addResourceType("kst", KStandardDirs::kde_default("data") + "kst");
  KGlobal::dirs()->addResourceType("kstplugins", KStandardDirs::kde_default("data") + "kst" + QDir::separator() + "plugins");
  _parser = new PluginXMLParser;
  scanPlugins();
}


PluginCollection::~PluginCollection() {
  unloadAllPlugins();
  delete _parser;
  _parser = 0L;
}


void PluginCollection::loadAllPlugins() {
  QString path = KGlobal::dirs()->saveLocation("kstplugins");
  QDir d(path);

  d.setFilter(QDir::Files | QDir::NoSymLinks);
  d.setNameFilter("*.xml");

  QFileInfoListIterator it(*(d.entryInfoList()));
  QFileInfo *fi;

  while ((fi = it.current()) != 0L) {
    loadPlugin(path + QDir::separator() + fi->fileName());
    ++it;
  }
}


int PluginCollection::loadPlugin(const QString& xmlfile) {
  if (!_installedPlugins.contains(xmlfile)) {
    // Try to find it
    if (_parser->parseFile(xmlfile) != 0) {
      return -1;  // can't parse
    }

    _installedPlugins[xmlfile] = _parser->data();
    _installedPluginNames[_parser->data()._name] = xmlfile;
  }

  QString name = _installedPlugins[xmlfile]._name;

  if (_plugins.contains(name)) {
    return 0;  // already loaded
  }

  QString sofile = xmlfile;
  Plugin *p = PluginLoader::self()->loadPlugin(xmlfile,
                                   sofile.replace(QRegExp(".xml$"), ".so"));
  if (p) {
    _plugins[name] = KstSharedPtr<Plugin>(p);
    emit pluginLoaded(name);
    return 0;
  }

return -2;
}


int PluginCollection::unloadPlugin(const KstSharedPtr<Plugin> p) {
  if (!p.data()) {
    return -1;
  }

  QString key = p->data()._name;
  _plugins.remove(key);

  emit pluginUnloaded(key);

  return 0;
}


int PluginCollection::unloadPlugin(const QString& name) {
  return unloadPlugin(plugin(name));
}


void PluginCollection::unloadAllPlugins() {
  for (QMap<QString, KstSharedPtr<Plugin> >::ConstIterator it = _plugins.begin();
                                                          it != _plugins.end();
                                                                         ++it) {
    emit pluginUnloaded(it.key());
  }

  _plugins.clear();
}


KstSharedPtr<Plugin> PluginCollection::plugin(const QString& name) {
  if (!_plugins.contains(name)) {
    if (!_installedPluginNames.contains(name)) {
      rescan();
    }

    if (_installedPluginNames.contains(name)) {
      loadPlugin(_installedPluginNames[name]);

      if (!_plugins.contains(name)) {
        return 0L;
      }
    }
  }

  return _plugins[name];
}


bool PluginCollection::isLoaded(const QString& name) const {
  return _plugins.contains(name);
}


int PluginCollection::count() const {
  return _plugins.count();
}


QStringList PluginCollection::loadedPluginList() const {
  return _plugins.keys();
}


const QMap<QString, Plugin::Data>& PluginCollection::pluginList() const {
  return _installedPlugins;
}


const QMap<QString, QString>& PluginCollection::pluginNameList() const {
  return _installedPluginNames;
}


void PluginCollection::rescan() {
  // Make me smarter
  scanPlugins();
}


void PluginCollection::scanPlugins() {
  QString path = KGlobal::dirs()->saveLocation("kstplugins");
  QDir d(path);

  d.setFilter(QDir::Files | QDir::NoSymLinks);
  d.setNameFilter("*.xml");

  QFileInfoListIterator it(*(d.entryInfoList()));
  QFileInfo *fi;

  QMap<QString,QString> backup = _installedPluginNames;

  _installedPlugins.clear();
  _installedPluginNames.clear();

  bool changed = false;

  while ((fi = it.current()) != 0L) {
    if (_parser->parseFile(path + QDir::separator() + fi->fileName()) == 0) {
      _installedPlugins[path + QDir::separator() + fi->fileName()] = _parser->data();
      _installedPluginNames[_parser->data()._name] = path + QDir::separator() + fi->fileName();
      if (!backup.contains(_parser->data()._name)) {
        emit pluginInstalled(_parser->data()._name);
        changed = true;
      } else {
        backup.remove(_parser->data()._name);
      }
    }
    ++it;
  }

  while (!backup.isEmpty()) {
    kdDebug() << "Detected disappearance of " << backup.begin().key() << endl;
    emit pluginRemoved(backup.begin().key());
    backup.remove(backup.begin());
    changed = true;
  }

  if (changed) {
    emit pluginListChanged();
  }
}


int PluginCollection::deletePlugin(const QString& xmlfile, const QString& object) {
  QString pname = _installedPlugins[xmlfile]._name;
  QFile::remove(xmlfile);
  if (object.isEmpty()) {
    QString f = xmlfile;
    f.replace(QRegExp(".xml$"), ".so");
    QFile::remove(f);
  } else {
    QFile::remove(object);
  }

  _installedPlugins.remove(xmlfile);
  _installedPluginNames.remove(pname);
  emit pluginRemoved(pname);

  return 0;
}

#include "plugincollection.moc"

// vim: ts=2 sw=2 et
