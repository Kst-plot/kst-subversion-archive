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


#include "kstdebug.h"
#include "plugincollection.h"
#include "pluginloader.h"
#include "pluginxmlparser.h"

#include <kdeversion.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kdebug.h>

#include <klibloader.h>
#include <klocale.h>
#include <kservicetype.h>
#include <kstdataobject.h>
#include <kparts/componentfactory.h>

#include <qdir.h>
#include <qregexp.h>

PluginCollection *PluginCollection::_self = 0L;
static KStaticDeleter<PluginCollection> _pcSelf;

PluginCollection *PluginCollection::self() {
  if (!_self) {
    _pcSelf.setObject(_self, new PluginCollection);
  }

return _self;
}


PluginCollection::PluginCollection() 
: QObject(0L, "KST Plugin Collection") {
  KGlobal::dirs()->addResourceType("kstplugins", KStandardDirs::kde_default("data") + "kst" + QDir::separator() + "plugins");
  // KDE3 provides no way to get the plugin directory
  KGlobal::dirs()->addResourceType("kstpluginlib", KStandardDirs::kde_default("lib") + QString("kde%1").arg(KDE_VERSION_MAJOR) + QDir::separator() + "kstplugins");
  _parser = new PluginXMLParser;
  scanPlugins();
}


PluginCollection::~PluginCollection() {
  unloadAllPlugins();
  delete _parser;
  _parser = 0L;
}


void PluginCollection::loadAllPlugins() {
  QStringList dirs = KGlobal::dirs()->resourceDirs("kstplugins");
  dirs += KGlobal::dirs()->resourceDirs("kstpluginlib");
  for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
    loadPluginsFor(*it);
  }
}


void PluginCollection::loadPluginsFor(const QString& path) {
  QDir d(path);

  d.setFilter(QDir::Files | QDir::NoSymLinks);
  d.setNameFilter("*.xml");

  const QFileInfoList *list = d.entryInfoList();
  if (!list) {
    return;
  }

  QFileInfoListIterator it(*list);
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
  if (!_plugins.contains(name) || _plugins[name] == 0L) {
    if (!_installedPluginNames.contains(name)) {
      rescan();
    }

    if (_installedPluginNames.contains(name)) {
      int rc = loadPlugin(_installedPluginNames[name]);
      Q_UNUSED(rc)

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

  QMap<QString,QString> backup = _installedPluginNames;
  _installedPlugins.clear();
  _installedPluginNames.clear();
  bool changed = /*scanDataObjectPlugins()*/ false;

  QStringList dirs = KGlobal::dirs()->resourceDirs("kstplugins");
  dirs += KGlobal::dirs()->resourceDirs("kstpluginlib");
  for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
    //kstdDebug() << "Scanning [" << *it << "] for plugins." << endl;
    QDir d(*it);

    d.setFilter(QDir::Files | QDir::NoSymLinks);
    d.setNameFilter("*.xml");

    const QFileInfoList *list = d.entryInfoList();
    if (!list) {
      continue;
    }

    QFileInfoListIterator fit(*list);
    QFileInfo *fi;
    int status;

    while ((fi = fit.current()) != 0L) {
      //kstdDebug() << "Parsing [" << (*it + fi->fileName()) << "]" << endl;
      status = _parser->parseFile(*it + fi->fileName());
      if (status == 0) {
        // dupe? - prefer earlier installations
        if (_installedPluginNames.contains(_parser->data()._name)) {
          ++fit;
          continue;
        }
        _installedPlugins[*it + fi->fileName()] = _parser->data();
        _installedPluginNames[_parser->data()._name] = *it + fi->fileName();
        if (!backup.contains(_parser->data()._name)) {
          emit pluginInstalled(_parser->data()._name);
          changed = true;
        } else {
          backup.remove(_parser->data()._name);
        }
      } else {
        KstDebug::self()->log(i18n("Error [%2] parsing XML file '%1'; skipping.").arg(*it + fi->fileName()).arg(status), KstDebug::Warning);
      }
      ++fit;
    }
  }

  while (!backup.isEmpty()) {
    KstDebug::self()->log(i18n("Detected disappearance of '%1'.").arg(backup.begin().key()));
    emit pluginRemoved(backup.begin().key());
    backup.remove(backup.begin());
    changed = true;
  }

  if (changed) {
    emit pluginListChanged();
  }
}

// Scans for plugins and stores the information for them in "pluginInfo"
bool PluginCollection::scanDataObjectPlugins() {

  KstDebug::self()->log(i18n("Scanning for data-object plugins."));

  QMap<QString,QString> backup = _installedPluginNames;
  bool changed = false;

  KService::List sl = KServiceType::offers("Kst Data Object");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    int err = 0;
    KService::Ptr service = ( *it );
    KstDataObject *object =
        KParts::ComponentFactory::createInstanceFromService<KstDataObject>( service, 0, "",
                                                                            QStringList(), &err );
    if ( object ) {
        int status = _parser->parseFile( object->xmlFile() );
        if (status == 0) {
        // dupe? - prefer earlier installations
        if (_installedPluginNames.contains(_parser->data()._name)) {
          continue;
        }
        _installedPlugins[object->xmlFile()] = _parser->data();
        _installedPluginNames[_parser->data()._name] = object->xmlFile();
        if (!backup.contains(_parser->data()._name)) {
          emit pluginInstalled(_parser->data()._name);
          changed = true;
        } else {
          backup.remove(_parser->data()._name);
        }
      } else {
        KstDebug::self()->log(i18n("Error [%2] parsing XML file '%1'; skipping.").arg(object->xmlFile()).arg(status), KstDebug::Warning);
      }
    }
    else
        kdDebug() << "FAILURE! " << k_funcinfo << " " << err << endl;

  }
  return changed; //changed
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
