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

#include "ksdebug.h"
#include <kio/netaccess.h>
#include <klibloader.h>
#include <klocale.h>
#include <kservicetype.h>

#include <qdeepcopy.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstylesheet.h>

#include "kst.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstscalar.h"
#include "stdinsource.h"


// Eventually this will move to another file but I leave it here until then
// to avoid confusion between the function plugins and Kst applicaton plugins.
namespace KST {
  class Plugin : public KstShared {
    public:
      Plugin(KService::Ptr svc) : KstShared(), service(svc), _lib(0L) {
        assert(service);
        _plugLib = service->property("X-Kst-Plugin-Library").toString();
        //kstdDebug() << "Create plugin " << (void*)this << " " << service->property("Name").toString() << endl;
      }

      virtual ~Plugin() {
        //kstdDebug() << "Destroy plugin " << (void*)this << " " << service->property("Name").toString() << endl;
        if (_lib) {
          _lib->unload();
        }
      }

      KstDataSource *create(KConfig *cfg, const QString& filename, const QString& type = QString::null) const {
        KstDataSource *(*sym)(KConfig*, const QString&, const QString&) = (KstDataSource*(*)(KConfig*, const QString&, const QString&))symbol("create");
        if (sym) {
          //kstdDebug() << "Trying to create " << filename << " type=" << type << " with " << service->property("Name").toString() << endl;
          KstDataSource *ds = (sym)(cfg, filename, type);
          if (ds) {
            ds->_source = service->property("Name").toString();
          }
          //kstdDebug() << (ds ? "SUCCESS" : "FAILED") << endl;
          return ds;
        }

        return 0L;
      }

      KstDataSource *create(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e) const {
        KstDataSource *(*sym)(KConfig*, const QString&, const QString&, const QDomElement&) = (KstDataSource*(*)(KConfig*, const QString&, const QString&, const QDomElement&))symbol("load");
        if (sym) {
          //kstdDebug() << "Trying to create " << filename << " type=" << type << " with " << service->property("Name").toString() << endl;
          KstDataSource *ds = (sym)(cfg, filename, type, e);
          if (ds) {
            ds->_source = service->property("Name").toString();
          }
          //kstdDebug() << (ds ? "SUCCESS" : "FAILED") << endl;
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
        // fallback incase the helper isn't implemented
        //  (note: less efficient now)
        KstDataSourcePtr ds = create(cfg, filename, type);
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

        // fallback incase the helper isn't implemented
        //  (note: less efficient now)
        KstDataSourcePtr ds = create(cfg, filename, type);
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
          //kstdDebug() << "Checking if " << service->property("Name").toString() << " understands " << filename << endl;
          int rc = (sym)(cfg, filename);
          //kstdDebug() << "result: " << rc << endl;
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

      bool provides(const QString& type) const {
        return provides().contains(type);
      }

      QStringList provides() const {
        QStringList (*sym)() = (QStringList(*)())symbol("provides");
        if (sym) {
          //kstdDebug() << "Checking if " << service->property("Name").toString() << " provides " << type << endl;
          return (sym)();
        }

        return QStringList();
      }

      Q_UINT32 key() const {
        Q_UINT32 (*sym)() = (Q_UINT32(*)())symbol("key");
        if (sym) {
          return (sym)();
        }

        return Q_UINT32();
      }

      bool hasConfigWidget() const {
        return 0L != symbol("widget");
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
          KstDebug::self()->log(i18n("Error in plugin %1: Configuration widget is of the wrong type.").arg(service->property("Name").toString()), KstDebug::Error);
          delete rc;
        }

        return 0L;
      }

      KService::Ptr service;

    private:
      void *symbol(const QString& sym) const {
        if (!loadLibrary()) {
          return 0L;
        }

        QCString s = QFile::encodeName(sym + "_" + _plugLib);
        if (_lib->hasSymbol(s)) {
          return _lib->symbol(s);
        }
        return 0L;
      }

      bool loadLibrary() const {
        assert(service);
        if (_lib) {
          return true;
        }

        QCString libname = QFile::encodeName(QString("kstdata_") + _plugLib);
        _lib = KLibLoader::self()->library(libname);
        if (!_lib) {
          KstDebug::self()->log(i18n("Error loading data-source plugin [%1]: %2").arg(libname).arg(KLibLoader::self()->lastErrorMessage()), KstDebug::Error);
          return false;
        }

        if (key() != KST_CURRENT_DATASOURCE_KEY) {
          KstDebug::self()->log(i18n("Error loading data-source plugin [%1]: %2").arg(libname).arg(i18n("Plugin is too old and needs to be recompiled.")), KstDebug::Error);
          return false;
        }
        return true;
      }

      QString _plugLib;
      // mutable so we can lazy load the library, but at the same time
      // use const iterators and provide a nice const interface
      mutable KLibrary *_lib;
  };

  typedef QValueList<KstSharedPtr<KST::Plugin> > PluginInfoList;
}


static KConfig *kConfigObject = 0L;
static QMap<QString,QString> urlMap;
void KstDataSource::setupOnStartup(KConfig *cfg) {
  kConfigObject = cfg;
}


static KST::PluginInfoList pluginInfo;
void KstDataSource::cleanupForExit() {
  pluginInfo.clear();
  kConfigObject = 0L;
  for (QMap<QString,QString>::Iterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    QFile::remove(i.data());
  }
  urlMap.clear();
}


static QString obtainFile(const QString& source) {
  KURL url;
  
  if (QFile::exists(source) && QFileInfo(source).isRelative()) {
    url.setPath(source);
  } else {
    url = KURL::fromPathOrURL(source);
  }

  if (url.isLocalFile() || url.protocol().isEmpty()) {
    return source;
  }

  if (urlMap.contains(source)) {
    return urlMap[source];
  }

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  // FIXME: come up with a way to indicate the "widget" and fill it in here so
  //        that KIO dialogs are associated with the proper window
  if (!KIO::NetAccess::exists(url, true, 0L)) {
#else
  if (!KIO::NetAccess::exists(url, true)) {
#endif
    return QString::null;
  }

  QString tmpFile;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  // FIXME: come up with a way to indicate the "widget" and fill it in here so
  //        that KIO dialogs are associated with the proper window
  if (!KIO::NetAccess::download(url, tmpFile, 0L)) {
#else
  if (!KIO::NetAccess::download(url, tmpFile)) {
#endif
    return QString::null;
  }

  urlMap[source] = tmpFile;

  return tmpFile;
}


// Scans for plugins and stores the information for them in "pluginInfo"
static void scanPlugins() {
  KST::PluginInfoList tmpList;

  KstDebug::self()->log(i18n("Scanning for data-source plugins."));

  KService::List sl = KServiceType::offers("Kst Data Source");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    for (KST::PluginInfoList::ConstIterator i2 = pluginInfo.begin(); i2 != pluginInfo.end(); ++i2) {
      if ((*i2)->service == *it) {
        tmpList.append(*i2);
        continue;
      }
    }

    KstSharedPtr<KST::Plugin> p = new KST::Plugin(*it);
    tmpList.append(p);
  }

  // This cleans up plugins that have been uninstalled and adds in new ones.
  // Since it is a shared pointer it can't dangle anywhere.
  pluginInfo.clear();
  pluginInfo = QDeepCopy<KST::PluginInfoList>(tmpList);
}


QStringList KstDataSource::pluginList() {
  QStringList rc;

  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }

  for (KST::PluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
    rc += (*it)->service->property("Name").toString();
  }

  return rc;
}


namespace {
class PluginSortContainer {
  public:
    KstSharedPtr<KST::Plugin> plugin;
    int match;
    int operator<(const PluginSortContainer& x) const {
      return match > x.match; // yes, this is by design.  biggest go first
    }
    int operator==(const PluginSortContainer& x) const {
      return match == x.match;
    }
};
}


static QValueList<PluginSortContainer> bestPluginsForSource(const QString& filename, const QString& type) {
  QValueList<PluginSortContainer> bestPlugins;
  scanPlugins();

  KST::PluginInfoList info = QDeepCopy<KST::PluginInfoList>(pluginInfo);

  if (!type.isEmpty()) {
    for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
      if ((*it)->provides(type)) {
        PluginSortContainer psc;
        psc.match = 100;
        psc.plugin = *it;
        bestPlugins.append(psc);
        return bestPlugins;
      }
    }
  }

  for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
    PluginSortContainer psc;
    if ((psc.match = (*it)->understands(kConfigObject, filename)) > 0) {
      psc.plugin = *it;
      bestPlugins.append(psc);
    }
  }

  qHeapSort(bestPlugins);
  return bestPlugins;
}


static KstDataSourcePtr findPluginFor(const QString& filename, const QString& type, const QDomElement& e = QDomElement()) {

  QValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(filename, type);

  for (QValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    KstDataSourcePtr plugin = (*i).plugin->create(kConfigObject, filename, QString::null, e);
    if (plugin) {
      return plugin;
    }
  }

  return 0L;
}


KstDataSourcePtr KstDataSource::loadSource(const QString& filename, const QString& type) {
  if (filename == "stdin" || filename == "-") {
    return new KstStdinSource(kConfigObject);
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return 0L;
  }

  return findPluginFor(fn, type);
}


KstDataSourceConfigWidget* KstDataSource::configWidget() const {
  KstDataSourceConfigWidget *w = configWidgetForSource(_filename, fileType());
  if (w) {
    // FIXME: what to do here?  This is ugly, but the method is const and we
    //        can't put a const shared pointer in the config widget.  Fix for
    //        Kst 2.0 by making this non-const?
    w->_instance = const_cast<KstDataSource*>(this);
  }
  return w;
}


bool KstDataSource::pluginHasConfigWidget(const QString& plugin) {
  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }

  KST::PluginInfoList info = QDeepCopy<KST::PluginInfoList>(pluginInfo);

  for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
    if ((*it)->service->property("Name").toString() == plugin) {
      return (*it)->hasConfigWidget();
    }
  }

  return false;
}


KstDataSourceConfigWidget* KstDataSource::configWidgetForPlugin(const QString& plugin) {
  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }

  KST::PluginInfoList info = QDeepCopy<KST::PluginInfoList>(pluginInfo);

  for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
    if ((*it)->service->property("Name").toString() == plugin) {
      return (*it)->configWidget(kConfigObject, QString::null);
    }
  }

  return 0L;
}


KstDataSourceConfigWidget* KstDataSource::configWidgetForSource(const QString& filename, const QString& type) {
  if (filename == "stdin" || filename == "-") {
    return 0L;
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return 0L;
  }

  QValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  for (QValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    KstDataSourceConfigWidget *w = (*i).plugin->configWidget(kConfigObject, fn);
    // Don't iterate.
    return w;
  }

  KstDebug::self()->log(i18n("Could not find a datasource for '%1'(%2), but we found one just prior.  Something is wrong with Kst.").arg(filename).arg(type), KstDebug::Error);
  return 0L;
}


bool KstDataSource::supportsTime(const QString& filename, const QString& type) {
  if (filename.isEmpty() || filename == "stdin" || filename == "-") {
    return false;
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return false;
  }

  QValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  if (bestPlugins.isEmpty()) {
    return false;
  }
  return (*bestPlugins.begin()).plugin->supportsTime(kConfigObject, fn);
}


QStringList KstDataSource::fieldListForSource(const QString& filename, const QString& type, QString *outType, bool *complete) {
  if (filename == "stdin" || filename == "-") {
    return QStringList();
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return QStringList();
  }

  QValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  QStringList rc;
  for (QValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    QString typeSuggestion;
    rc = (*i).plugin->fieldList(kConfigObject, fn, QString::null, &typeSuggestion, complete);
    if (!rc.isEmpty()) {
      if (outType) {
        if (typeSuggestion.isEmpty()) {
          *outType = (*i).plugin->provides()[0];
        } else {
          *outType = typeSuggestion;
        }
      }
      break;
    }
  }

  return rc;
}


QStringList KstDataSource::matrixListForSource(const QString& filename, const QString& type, QString *outType, bool *complete) {
  if (filename == "stdin" || filename == "-") {
    return QStringList();
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return QStringList();
  }

  QValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  QStringList rc;
  for (QValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    QString typeSuggestion;
    rc = (*i).plugin->matrixList(kConfigObject, fn, QString::null, &typeSuggestion, complete);
    if (!rc.isEmpty()) {
      if (outType) {
        if (typeSuggestion.isEmpty()) {
          *outType = (*i).plugin->provides()[0];
        } else {
          *outType = typeSuggestion;
        }
      }
      break;
    }
  }

  return rc;
}


KstDataSourcePtr KstDataSource::loadSource(QDomElement& e) {
  QString filename, type;

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "filename") {
        filename = obtainFile(e.text());
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
    return new KstStdinSource(kConfigObject);
  }

  return findPluginFor(filename, type, e);
}


KstDataSource::KstDataSource(KConfig *cfg, const QString& filename, const QString& type)
: KstObject(), _filename(filename), _cfg(cfg) {
  Q_UNUSED(type)
  _valid = false;
  _reusable = true;
  _numFramesScalar = new KstScalar(filename + i18n("-frames"));
  // Don't set provider - this is always up-to-date
}


KstDataSource::~KstDataSource() {
  KST::scalarList.lock().writeLock();
  KST::scalarList.remove(_numFramesScalar);
  KST::scalarList.lock().writeUnlock();
  _numFramesScalar = 0L;
}


KstObject::UpdateType KstDataSource::update(int u) {
  Q_UNUSED(u)
  return KstObject::NO_CHANGE;
}


void KstDataSource::updateNumFramesScalar() {
  _numFramesScalar->setValue(frameCount());
}


int KstDataSource::readField(double *v, const QString& field, int s, int n, int skip, int *lastFrameRead) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)
  Q_UNUSED(skip)
  Q_UNUSED(lastFrameRead)
  return -9999; // unsupported
}


int KstDataSource::readField(double *v, const QString& field, int s, int n) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)
  return -1;
}


int KstDataSource::readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps, int skip) {
  Q_UNUSED(data)
  Q_UNUSED(matrix)
  Q_UNUSED(xStart)
  Q_UNUSED(yStart)
  Q_UNUSED(xNumSteps)
  Q_UNUSED(yNumSteps)
  Q_UNUSED(skip)
  return -9999;
}
 
  
int KstDataSource::readMatrix(KstMatrixData* data, const QString& matrix, int xStart, int yStart, int xNumSteps, int yNumSteps) {
  Q_UNUSED(data)
  Q_UNUSED(matrix)
  Q_UNUSED(xStart)
  Q_UNUSED(yStart)
  Q_UNUSED(xNumSteps)
  Q_UNUSED(yNumSteps)
  return -1;
}


bool KstDataSource::matrixDimensions(const QString& matrix, int* xDim, int* yDim) {
  Q_UNUSED(matrix)
  Q_UNUSED(xDim)
  Q_UNUSED(yDim)
  return false;  
}


bool KstDataSource::isValid() const {
  return _valid;
}


bool KstDataSource::isValidField(const QString& field) const {
  Q_UNUSED(field)
  return false;
}


bool KstDataSource::isValidMatrix(const QString& field) const {
  Q_UNUSED(field)
  return false;  
}


int KstDataSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 0;
}


int KstDataSource::frameCount(const QString& field) const {
  Q_UNUSED(field)
  return 0;
}


QString KstDataSource::fileName() const {
  // Look to see if it was a URL and save the URL instead
  for (QMap<QString,QString>::ConstIterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    if (i.data() == _filename) {
      return i.key();
    }
  }
  return _filename;
}


QStringList KstDataSource::fieldList() const {
  return _fieldList;
}


QStringList KstDataSource::matrixList() const { 
  return _matrixList;  
}


QString KstDataSource::fileType() const {
  return QString::null;
}


void KstDataSource::save(QTextStream &ts, const QString& indent) {
  QString name = QStyleSheet::escape(_filename);
  // Look to see if it was a URL and save the URL instead
  for (QMap<QString,QString>::ConstIterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    if (i.data() == _filename) {
      name = QStyleSheet::escape(i.key());
      break;
    }
  }
  ts << indent << "<filename>" << name << "</filename>" << endl;
  ts << indent << "<type>" << QStyleSheet::escape(fileType()) << "</type>" << endl;
}


void *KstDataSource::bufferMalloc(size_t size) {
  return KST::malloc(size);
}


void KstDataSource::bufferFree(void *ptr) {
  return ::free(ptr);
}


void *KstDataSource::bufferRealloc(void *ptr, size_t size) {
  return KST::realloc(ptr, size);
}


bool KstDataSource::fieldListIsComplete() const {
  return true;
}


bool KstDataSource::isEmpty() const {
  return true;
}


bool KstDataSource::reset() {
  return false;
}


const QMap<QString, QString>& KstDataSource::metaData() const {
  return _metaData;
}


const QString& KstDataSource::metaData(const QString& key) const {
  if (_metaData.contains(key)) {
    return _metaData[key];
  } else {
    return QString::null;
  }
}


bool KstDataSource::hasMetaData() const {
  return !_metaData.isEmpty();
}


bool KstDataSource::hasMetaData(const QString& key) const {
  return _metaData.contains(key);
}


bool KstDataSource::supportsTimeConversions() const {
  return false;
}


int KstDataSource::sampleForTime(const KST::ExtDateTime& time, bool *ok) {
  Q_UNUSED(time)
  if (ok) {
    *ok = false;
  }
  return 0;
}



int KstDataSource::sampleForTime(double ms, bool *ok) {
  Q_UNUSED(ms)
  if (ok) {
    *ok = false;
  }
  return 0;
}



KST::ExtDateTime KstDataSource::timeForSample(int sample, bool *ok) {
  Q_UNUSED(sample)
  if (ok) {
    *ok = false;
  }
  return KST::ExtDateTime::currentDateTime();
}



double KstDataSource::relativeTimeForSample(int sample, bool *ok) {
  Q_UNUSED(sample)
  if (ok) {
    *ok = false;
  }
  return 0;
}


bool KstDataSource::reusable() const {
  return _reusable;
}


void KstDataSource::disableReuse() {
  _reusable = false;
}


/////////////////////////////////////////////////////////////////////////////
KstDataSourceConfigWidget::KstDataSourceConfigWidget()
: QWidget(0L), _cfg(0L) {
}


KstDataSourceConfigWidget::~KstDataSourceConfigWidget() {
}


void KstDataSourceConfigWidget::save() {
}


void KstDataSourceConfigWidget::load() {
}


void KstDataSourceConfigWidget::setConfig(KConfig *cfg) {
  _cfg = cfg;
}


void KstDataSourceConfigWidget::setInstance(KstDataSourcePtr inst) {
  _instance = inst;
}


KstDataSourcePtr KstDataSourceConfigWidget::instance() const {
  return _instance;
}



#include "kstdatasource.moc"
// vim: ts=2 sw=2 et
