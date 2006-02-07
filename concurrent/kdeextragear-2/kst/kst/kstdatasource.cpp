/***************************************************************************
                   kstdatasource.cpp  -  abstract data source
                             -------------------
    begin                : Thu Oct 16 2003
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

#include "kstdatasource.h"

#include <assert.h>

#include <kdebug.h>
#include <klibloader.h>
#include <kservice.h>
#include <kservicetype.h>

#include <qfile.h>
#include <qstylesheet.h>

#include "stdinsource.h"


// Eventually this will move to another file but I leave it here until then
// to avoid confusion between the function plugins and Kst applicaton plugins.
namespace KST {
  class Plugin : public KstShared {
    public:
      Plugin(KService::Ptr svc) : KstShared(), service(svc), _lib(0L) {
        assert(service);
        _plugLib = service->property("X-Kst-Plugin-Library").toString();
      }

      virtual ~Plugin() { if (_lib) { _lib->unload(); } }

      KstDataSource *create(const QString& filename, const QString& type = QString::null) const {
        KstDataSource *(*sym)(const QString&, const QString&) = (KstDataSource*(*)(const QString&, const QString&))symbol("create");
        if (sym) {
          return (sym)(filename, type);
        }

        return 0L;
      }

      bool understands(const QString& filename) const {
        bool (*sym)(const QString&) = (bool(*)(const QString&))symbol("understands");
        if (sym) {
          return (sym)(filename);
        }

        return false;
      }

      bool provides(const QString& type) const {
        QStringList (*sym)() = (QStringList(*)())symbol("provides");
        if (sym) {
          return ((sym)()).contains(type);
        }

        return false;
      }

      KService::Ptr service;

    private:
      void *symbol(const QString& sym) const {
        if (!loadLibrary()) {
          return 0L;
        }

        // FIXME: might be a good idea to cache this per-symbol

        return _lib->symbol(QFile::encodeName(sym + "_" + _plugLib));
      }

      bool loadLibrary() const {
        assert(service);
        if (_lib) {
          return true;
        }

        QCString libname = QFile::encodeName(QString("kstdata_") + _plugLib);
        _lib = KLibLoader::self()->library(libname);
        if (!_lib) {
          kdDebug() << "Error datasource plugin [" << libname << "]" << endl;
          kdDebug() << "Reason: " << KLibLoader::self()->lastErrorMessage() << endl;
        }
        return _lib != 0L;
      }

      QString _plugLib;
      // mutable so we can lazy load the library, but at the same time
      // use const iterators and provide a nice const interface
      mutable KLibrary *_lib;
  };

  typedef QValueList<KstSharedPtr<KST::Plugin> > PluginInfoList;
}


static KST::PluginInfoList pluginInfo;


// Scans for plugins and stores the information for them in "pluginInfo"
static void scanPlugins() {
  KST::PluginInfoList tmpList;

  kdDebug() << "Scanning for plugins..." << endl;

  KService::List sl = KServiceType::offers("Kst Data Source");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    for (KST::PluginInfoList::ConstIterator i2 = pluginInfo.begin(); i2 != pluginInfo.end(); ++i2) {
      if ((*i2)->service == *it) {
        tmpList.append(*i2);
        continue;
      }
    }

    KstSharedPtr<KST::Plugin> p = new KST::Plugin(*it);
    kdDebug() << "Added a new plugin " << (*it)->property("Name").toString() << endl;
    tmpList.append(p);
  }

  // This cleans up plugins that have been uninstalled and adds in new ones.
  // Since it is a shared pointer it can't dangle anywhere.
  pluginInfo = tmpList;

  kdDebug() << "Done scanning for plugins!" << endl;
}


static KstDataSourcePtr findPluginFor(const QString& filename, const QString& type) {
  KstDataSourcePtr plugin;

  scanPlugins();

  if (!type.isEmpty()) {
    for (KST::PluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
      if ((*it)->provides(type)) {
        plugin = (*it)->create(filename, type);
        break;
      }
    }

    return plugin;
  }

  for (KST::PluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
    if ((*it)->understands(filename)) {
      plugin = (*it)->create(filename);
      if (plugin) {
        break;
      }
    }
  }

  return plugin;
}


KstDataSourcePtr KstDataSource::loadSource(const QString& filename, const QString& type) {
  if (filename == "stdin" || filename == "-") {
    return new KstStdinSource;
  }

  if (filename.isEmpty()) {
    return 0L;
  }

  return findPluginFor(filename, type);
}


KstDataSourcePtr KstDataSource::loadSource(QDomElement& e) {
  QString filename, type;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "filename") {
        filename = e.text();
      } else if (e.tagName() == "type") {
        type = e.text();
      }
    }
    n = n.nextSibling();
  }

  if (filename.isEmpty()) {
    return 0L;
  }

  if (filename == "stdin" || filename == "-") {
    return new KstStdinSource;
  }

  return findPluginFor(filename, type);
}

 
KstDataSource::KstDataSource(const QString& filename, const QString& type)
: KstObject(), _filename(filename) {
  Q_UNUSED(type)
  _valid = false;
}


KstDataSource::~KstDataSource() {
}


KstObject::UpdateType KstDataSource::update(int u) {
  Q_UNUSED(u)
  return KstObject::NO_CHANGE;
}


int KstDataSource::readField(double *v, const QString& field, int s, int n) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)
  return -1;
}


bool KstDataSource::isValid() const {
  return _valid;
}


bool KstDataSource::isValidField(const QString& field) const {
  Q_UNUSED(field)
  return false;
}


int KstDataSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 0;
}


int KstDataSource::frameCount() const {
  return 0;
}


QString KstDataSource::fileName() const {
  return _filename;
}


QStringList KstDataSource::fieldList() const {
  return _fieldList;
}


QString KstDataSource::fileType() const {
  return QString::null;
}


void KstDataSource::save(QTextStream &ts) {
  ts << " <filename>" << QStyleSheet::escape(_filename) << "</filename>" << endl;
  ts << " <type>" << QStyleSheet::escape(fileType()) << "</type>" << endl;
}


int KstDataSource::readIFile() {
  // FIXME
  return -1;
}


void KstDataSource::virtual_hook(int id, void *data) {
  Q_UNUSED(id)
  Q_UNUSED(data)
  //KstObject::virtual_hook(id, data);
}

// vim: ts=2 sw=2 et
