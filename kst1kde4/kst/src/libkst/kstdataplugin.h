/***************************************************************************
                   kstdataplugin.cpp  -  data plugin
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

#include <assert.h>

#include <qfile.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <klibloader.h>
#include <klocale.h>
#include <kservice.h>
#include <kservicetype.h>

#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstscalar.h"
#include "stdinsource.h"

// Eventually this will move to another file but I leave it here until then
// to avoid confusion between the function plugins and Kst applicaton plugins.
namespace KST {
  class Plugin : public QSharedData {
    public:
      Plugin(KService::Ptr svc) : QSharedData(), service(svc), _lib(0L) {
        assert(service);
        _plugLib = service->library();
      }

      virtual ~Plugin() {
        if (_lib) {
          _lib->unload();
        }
      }

  public:
      quint32 key() const {
        quint32 (*sym)() = (quint32(*)())symbol("key");
        if (sym) {
          return (sym)();
        }

        return quint32();
      }

      bool hasConfigWidget() const {
        return 0L != symbol("widget");
      }

      KService::Ptr service;

    protected:
      void *symbol(const QString& sym) const {
        if (!loadLibrary()) {
          return 0L;
        }

        QString libname = _plugLib;
        QByteArray s = QFile::encodeName(sym + "_" + libname.remove(QString("kstobject_")));
        
	return _lib->resolveSymbol(s);
      }

      bool loadLibrary() const {
        assert(service);
        if (_lib) {
          return true;
        }

        bool isDataObject = _plugLib.contains(QString("kstobject_"));

        QByteArray libname = QFile::encodeName((!isDataObject ? QString("kstdata_") : QString()) + _plugLib);
        _lib = KLibLoader::self()->library(libname);
        if (!_lib) {
          KstDebug::self()->log(i18n("Error loading data plugin [%1]: %2").arg(libname.data()).arg(KLibLoader::self()->lastErrorMessage()), KstDebug::Error);
          return false;
        }

        if (key() != (isDataObject ? KST_CURRENT_DATAOBJECT_KEY : KST_CURRENT_DATASOURCE_KEY)) {
          KstDebug::self()->log(i18n("Error loading data plugin [%1]: %2").arg(libname.data()).arg(i18n("Plugin is too old and needs to be recompiled.")), KstDebug::Error);
          KstDebug::self()->log(i18n("Error loading data plugin key = [%1]: %2").arg(key()).arg(QFile::encodeName("key_" + _plugLib).data()), KstDebug::Error);
          return false;
        }
        return true;
      }

      QString _plugLib;
      // mutable so we can lazy load the library, but at the same time
      // use const iterators and provide a nice const interface
      mutable KLibrary *_lib;
  };

  class DataSourcePlugin : public Plugin {
    public:
      DataSourcePlugin(KService::Ptr svc) : Plugin(svc) {
      }

      virtual ~DataSourcePlugin() {
      }

      KstDataSource *create(KConfig *cfg, const QString& filename, const QString& type = QString::null) const {
        KstDataSource *(*sym)(KConfig*, const QString&, const QString&) = (KstDataSource*(*)(KConfig*, const QString&, const QString&))symbol("create");
        if (sym) {
          KstDataSource *ds = (sym)(cfg, filename, type);
          if (ds) {
            ds->_source = service->property("Name").toString();
          }

          return ds;
        }

        return 0L;
      }

      KstDataSource *create(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e) const {
        KstDataSource *(*sym)(KConfig*, const QString&, const QString&, const QDomElement&) = (KstDataSource*(*)(KConfig*, const QString&, const QString&, const QDomElement&))symbol("load");
        if (sym) {
          KstDataSource *ds = (sym)(cfg, filename, type, e);
          if (ds) {
            ds->_source = service->property("Name").toString();
          }

          return ds;
        } else {
          KstDataSource *(*sym)(KConfig*, const QString&, const QString&) = (KstDataSource*(*)(KConfig*, const QString&, const QString&))symbol("create");
          if (sym) {
            KstDataSource *ds = (sym)(cfg, filename, type);
            if (ds) {
              ds->_source = service->property("Name").toString();
            }

            return ds;
          }
        }

        return 0L;
      }

      QStringList matrixList(KConfig *cfg, const QString& filename, const QString& type = QString::null, QString *typeSuggestion = 0L, bool *complete = 0L) const {
        QStringList (*sym)(KConfig*, const QString&, const QString&, QString*, bool*) = (QStringList(*)(KConfig*, const QString&, const QString&, QString*, bool*))symbol("matrixList");
        if (sym) {
          return (sym)(cfg, filename, type, typeSuggestion, complete);  
        }
	
	//
        // fallback in case the helper isn't implemented
        //  (note: less efficient now)
	//
	
        KstDataSourcePtr ds;
	
	ds = create(cfg, filename, type);
        if (ds) {
          QStringList rc = ds->matrixList();
          if (typeSuggestion) {
            *typeSuggestion = ds->fileType();
          }
          if (complete) {
            *complete = ds->fieldListIsComplete();
          }
          return rc;
        }
        return QStringList();
      }

      QStringList fieldList(KConfig *cfg, const QString& filename, const QString& type = QString::null, QString *typeSuggestion = 0L, bool *complete = 0L) const {
        QStringList (*sym)(KConfig*, const QString&, const QString&, QString*, bool*) = (QStringList(*)(KConfig*, const QString&, const QString&, QString*, bool*))symbol("fieldList");
        if (sym) {
          return (sym)(cfg, filename, type, typeSuggestion, complete);
        }

	//
        // fallback in case the helper isn't implemented
        //  (note: less efficient now)
	//
	
        KstDataSourcePtr ds;
	
	ds = create(cfg, filename, type);
        if (ds) {
          QStringList rc = ds->fieldList();
          if (typeSuggestion) {
            *typeSuggestion = ds->fileType();
          }
          if (complete) {
            *complete = ds->fieldListIsComplete();
          }
          return rc;
        }

        return QStringList();
      }

      int understands(KConfig *cfg, const QString& filename) const {
        int (*sym)(KConfig*, const QString&) = (int(*)(KConfig*, const QString&))symbol("understands");
        if (sym) {
          int rc = (sym)(cfg, filename);

          return rc;
        }

        return 0;
      }

      bool supportsTime(KConfig *cfg, const QString& filename) const {
        bool (*sym)(KConfig*, const QString&) = (bool(*)(KConfig*, const QString&))symbol("supportsTime");
        if (sym) {
          bool rc = (sym)(cfg, filename);
          return rc;
        }

        return false;
      }

      bool supportsHierarchy() const {
        bool (*sym)() = (bool(*)())symbol("supportsHierarchy");
        if (sym) {
          bool rc = (sym)();

          return rc;
        }

        return false;
      }

      bool provides(const QString& type) const {
        return provides().contains(type);
      }

      QStringList provides() const {
        QStringList (*sym)() = (QStringList(*)())symbol("provides");
        if (sym) {
          return (sym)();
        }

        return QStringList();
      }

      KstDataSourceConfigWidget *configWidget(KConfig *cfg, const QString& filename) const {
        QWidget *(*sym)(const QString&) = (QWidget *(*)(const QString&))symbol("widget");
        if (sym) {
          QWidget *rc = (sym)(filename);
          KstDataSourceConfigWidget *cw = dynamic_cast<KstDataSourceConfigWidget*>(rc);
          if (cw) {
            cw->setConfig(cfg);
            return cw;
          }
          if (rc) {
            KstDebug::self()->log(i18n("Error in plugin %1: Configuration widget is of the wrong type.").arg(service->property("Name").toString()), KstDebug::Error);
            delete rc;
          }
        }

        return 0L;
      }
  };

  class DataObjectPlugin : public Plugin {
    public:
      DataObjectPlugin(KService::Ptr svc) : Plugin(svc) {
      }

      virtual ~DataObjectPlugin() {
      }

      QWidget *configWidget(const QString& name) const {
        Q_UNUSED(name);
        return 0L;
      }
  };

  typedef QLinkedList<QExplicitlySharedDataPointer<KST::Plugin> > PluginInfoList;
}
