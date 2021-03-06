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

#include <q3deepcopy.h>
#include <QFile>
#include <QFileInfo>
#include <QTextDocument>
#include <QTextStream>
#include <Q3ValueList>

#include "kstdatacollection.h"
#include "kstdataplugin.h"
#include "kstdebug.h"
#include "kstscalar.h"
#include "stdinsource.h"

static QSettings *qSettingsObject = 0L;
static QMap<QString,QString> urlMap;
static KST::PluginInfoList pluginInfo;


void KstDataSource::setupOnStartup(QSettings *cfg) {
  qSettingsObject = cfg;
}


void KstDataSource::cleanupForExit() {
  pluginInfo.clear();
  qSettingsObject = 0L;
  for (QMap<QString,QString>::Iterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    KIO::NetAccess::removeTempFile(*i);
  }
  urlMap.clear();
}


static QString obtainFile(const QString& source) {
  KUrl url(source);
  QString tmpFile;

  if (url.isLocalFile() || url.protocol().isEmpty() || url.protocol().toLower() == "nad") {
    return source;
  }

  if (urlMap.contains(source)) {
    return urlMap[source];
  }

  // FIXME: come up with a way to indicate the "widget" and fill it in here so
  //        that KIO dialogs are associated with the proper window
  if (!KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, 0L)) {
    return QString::null;
  }

  if (!KIO::NetAccess::download(url, tmpFile, 0L)) {
    return QString::null;
  }

  urlMap[source] = tmpFile;

  return tmpFile;
}


// Scans for plugins and stores the information for them in "pluginInfo"
static void scanPlugins() {
  KST::PluginInfoList tmpList;

  KstDebug::self()->log(QObject::tr("Scanning for data-source plugins."));
/* xxx  KService::List sl = KServiceType::offers("Kst Data Source");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    for (KST::PluginInfoList::ConstIterator i2 = pluginInfo.begin(); i2 != pluginInfo.end(); ++i2) {
      if ((*i2)->service == *it) {
        tmpList.append(*i2);
        continue;
      }
    }

    QExplicitlySharedDataPointer<KST::Plugin> p = new KST::DataSourcePlugin(*it);
    tmpList.append(p);
  }

  // This cleans up plugins that have been uninstalled and adds in new ones.
  // Since it is a shared pointer it can't dangle anywhere.
  pluginInfo.clear();
  pluginInfo = QDeepCopy<KST::PluginInfoList>(tmpList);
*/
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
    QExplicitlySharedDataPointer<KST::DataSourcePlugin> plugin;

    int match;
    int operator<(const PluginSortContainer& x) const {
      return match > x.match; // yes, this is by design.  biggest go first
    }
    int operator==(const PluginSortContainer& x) const {
      return match == x.match;
    }
};
}


static QList<PluginSortContainer> bestPluginsForSource(const QString& filename, const QString& type) {
  QList<PluginSortContainer> bestPlugins;

  if (pluginInfo.isEmpty()) {
    scanPlugins();
  }

  KST::PluginInfoList info = Q3DeepCopy<KST::PluginInfoList>(pluginInfo);

  if (!type.isEmpty()) {
    for (KST::PluginInfoList::iterator it = info.begin(); it != info.end(); ++it) {
      if (KST::DataSourcePlugin *p = dynamic_cast<KST::DataSourcePlugin*>((*it).data())) {
        if (p && p->provides(type)) {
          PluginSortContainer psc;
          psc.match = 100;
          psc.plugin = p;
          bestPlugins.append(psc);
          
          return bestPlugins;
        }
      }
    }
  }

  for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
    PluginSortContainer psc;
    if (KST::DataSourcePlugin *p = kst_cast<KST::DataSourcePlugin>(*it)) {
      if ((psc.match = p->understands(qSettingsObject, filename)) > 0) {
        psc.plugin = p;
        bestPlugins.append(psc);
      }
    }
  }

  qSort(bestPlugins);

  return bestPlugins;
}


static KstDataSourcePtr findPluginFor(const QString& filename, const QString& type, const QDomElement& e = QDomElement()) {
  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(filename, type);
 
  for (Q3ValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    KstDataSourcePtr plugin((*i).plugin->create(qSettingsObject, filename, QString::null, e));

    if (plugin) {
      // restore tag if present
      QDomNodeList l = e.elementsByTagName("tag");
      if (l.count() > 0) {
        QDomElement e2 = l.item(0).toElement();
        if (!e2.isNull()) {
          plugin->setTag(KstObjectTag::fromString(e2.text()));
        }
      }
      return plugin;
    }
  }

  return KstDataSourcePtr();
}


KstDataSourcePtr KstDataSource::loadSource(const QString& filename, const QString& type) {
  if (filename == "stdin" || filename == "-") {
    return KstDataSourcePtr(new KstStdinSource(qSettingsObject));
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return KstDataSourcePtr( );
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

  KST::PluginInfoList info = Q3DeepCopy<KST::PluginInfoList>(pluginInfo);

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

  KST::PluginInfoList info = Q3DeepCopy<KST::PluginInfoList>(pluginInfo);

  for (KST::PluginInfoList::ConstIterator it = info.begin(); it != info.end(); ++it) {
    if (KST::DataSourcePlugin *p = kst_cast<KST::DataSourcePlugin>(*it)) {
      if (p->service->property("Name").toString() == plugin) {
        return p->configWidget(qSettingsObject, QString::null);
      }
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

  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  for (Q3ValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    KstDataSourceConfigWidget *w = (*i).plugin->configWidget(qSettingsObject, fn);
    // Don't iterate.
    return w;
  }

  KstDebug::self()->log(QObject::tr("Could not find a datasource for '%1'(%2), but we found one just prior.  Something is wrong with Kst.").arg(filename).arg(type), KstDebug::Error);
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

  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  if (bestPlugins.isEmpty()) {
    return false;
  }
  return (*bestPlugins.begin()).plugin->supportsTime(qSettingsObject, fn);
}


bool KstDataSource::supportsHierarchy(const QString& filename, const QString& type) {
  if (filename.isEmpty() || filename == "stdin" || filename == "-") {
    return false;
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return false;
  }

  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  if (bestPlugins.isEmpty()) {
    return false;
  }

  return (*bestPlugins.begin()).plugin->supportsHierarchy();
}


QString KstDataSource::units(const QString& field) {
  Q_UNUSED(field)

  QString empty;

  return empty;
}

QStringList KstDataSource::fieldListForSource(const QString& filename, const QString& type, QString *outType, bool *complete) {
  if (filename == "stdin" || filename == "-") {
    return QStringList();
  }

  QString fn = obtainFile(filename);
  if (fn.isEmpty()) {
    return QStringList();
  }

  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  QStringList rc;
  for (Q3ValueList<PluginSortContainer>::Iterator it = bestPlugins.begin(); it != bestPlugins.end(); ++it) {
    QString typeSuggestion;
    rc = (*it).plugin->fieldList(qSettingsObject, fn, QString::null, &typeSuggestion, complete);
    if (!rc.isEmpty()) {
      if (outType) {
        if (typeSuggestion.isEmpty()) {
          *outType = (*it).plugin->provides()[0];
        } else {
          *outType = typeSuggestion;
        }
      }
      break;
    }
  }

  if (!rc.isEmpty()) {
    //
    // check for duplicate field names and warn the user if necessary...
    //
    QMap<QString, QString> map;

    for (QStringList::const_iterator it = rc.begin(); it != rc.end(); ++it) {
      map.insert(*it, *it);
    }

    if (map.size() != rc.size()) {
      KstDebug::self()->log( QObject::tr("The datasource '%1' has at least one duplicate field name. As a result one or more fields will not be accessible.").arg(filename), KstDebug::Error);
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

  Q3ValueList<PluginSortContainer> bestPlugins = bestPluginsForSource(fn, type);
  QStringList rc;
  
  for (Q3ValueList<PluginSortContainer>::Iterator i = bestPlugins.begin(); i != bestPlugins.end(); ++i) {
    QString typeSuggestion;
    rc = (*i).plugin->matrixList(qSettingsObject, fn, QString::null, &typeSuggestion, complete);
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
  QString filename, type, tag;

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
    return KstDataSourcePtr();
  }

  if (filename == "stdin" || filename == "-") {
    return KstDataSourcePtr(new KstStdinSource(qSettingsObject));
  }

  return findPluginFor(filename, type, e);
}


KstDataSource::KstDataSource(QSettings *cfg, const QString& filename, const QString& type)
: KstObject(), _filename(filename), _cfg(cfg) {
  Q_UNUSED(type)
  _valid = false;
  _reusable = true;
  _writable = false;

  QString shortFilename = filename;
  while (shortFilename.at(shortFilename.length() - 1) == '/') {
    shortFilename.truncate(shortFilename.length() - 1);
  }
  shortFilename = shortFilename.section('/', -1);
  QString tn = QObject::tr("DS-%1").arg(shortFilename);
  int count = 1;

  KstObject::setTag(KstObjectTag(tn, KstObjectTag::globalTagContext));  // are DataSources always top-level?
  while (KstData::self()->dataSourceTagNameNotUnique(tagName(), false)) {
    KstObject::setTag(KstObjectTag(tn + QString("-%1").arg(count++), KstObjectTag::globalTagContext));  // are DataSources always top-level?
  }

  _numFramesScalar = new KstScalar(KstObjectTag("frames", tag()));
  // Don't set provider - this is always up-to-date
}


KstDataSource::~KstDataSource() {
  QMultiHash<QString, KstString>::iterator it;

  KST::scalarList.lock().writeLock();
  KST::scalarList.remove(_numFramesScalar.data());
  KST::scalarList.lock().unlock();

  KST::stringList.lock().writeLock();
  KST::stringList.setUpdateDisplayTags(false);

  for (it = _metaData.begin(); it != _metaData.end(); ++it) {
    KstString  str(it.value());

    KST::stringList.remove(&str);
  }

  KST::stringList.setUpdateDisplayTags(true);
  KST::stringList.lock().unlock();

  _numFramesScalar = 0L;
}


void KstDataSource::setTag(const KstObjectTag& in_tag) {
  if (in_tag == tag()) {
    return;
  }

  KstObject::setTag(in_tag);
  _numFramesScalar->setTag(KstObjectTag("frames", tag()));

  QHash<QString, KstString>::iterator it;
  
  for (it=_metaData.begin(); it != _metaData.end(); ++it) {
    KstObjectTag stag = it->tag();

    stag.setContext(tag().fullTag());
    it->setTag(stag);
  }
}


KstObject::UpdateType KstDataSource::update(int u) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  Q_UNUSED(u)
  return KstObject::NO_CHANGE;
}


void KstDataSource::updateNumFramesScalar() {
  _numFramesScalar->setValue((double)frameCountLarge());
}


int KstDataSource::readFieldSkip(double *v, const QString& field, int s, int n, int skip, int *lastFrameRead) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)
  Q_UNUSED(skip)
  Q_UNUSED(lastFrameRead)

  return -9999; // unsupported
}


KstFrameSize KstDataSource::readFieldLargeSkip(double *v, const QString& field, KstFrameSize s, KstFrameSize n, int skip, KstFrameSize *lastFrameRead) {
  KstFrameSize rc = -9999;

  if (s <= INT_MAX && s+n <= INT_MAX) {
    int iLastFrameRead;

    rc = (KstFrameSize)readFieldSkip(v, field, (int)s, (int)n, skip, &iLastFrameRead);

    *lastFrameRead = (KstFrameSize)iLastFrameRead;

    return (KstFrameSize)rc;
  }

  return rc;
}


int KstDataSource::readField(double *v, const QString& field, int s, int n) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)

  return -1;
}


KstFrameSize KstDataSource::readFieldLarge(double *v, const QString& field, KstFrameSize s, KstFrameSize n) {
  KstFrameSize rc = -1;

  if (s <= INT_MAX && n <= INT_MAX) {
    rc = (KstFrameSize)readField(v, field, (int)s, (int)n);
  }

  return rc;
}


bool KstDataSource::isWritable() const {
  return _writable;
}


int KstDataSource::writeField(const double *v, const QString& field, int s, int n) {
  Q_UNUSED(v)
  Q_UNUSED(field)
  Q_UNUSED(s)
  Q_UNUSED(n)

  return -1;
}


KstFrameSize KstDataSource::writeFieldLarge(const double *v, const QString& field, KstFrameSize s, KstFrameSize n) {
  KstFrameSize rc = -1;

  if (s <= INT_MAX && s+n <= INT_MAX) {
    rc = (KstFrameSize)writeField(v, field, (int)s, (int)n);
  }

  return rc;
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


KstFrameSize KstDataSource::frameCountLarge(const QString& field) const {
  return (KstFrameSize)frameCount(field);
}


QString KstDataSource::fileName() const {
  //
  // look to see if it was a URL and save the URL instead...
  //

  for (QMap<QString,QString>::ConstIterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    if (i.value() == _filename) {
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
  QString name = Qt::escape(_filename);

  //
  // look to see if it was a URL and save the URL instead...
  //

  for (QMap<QString,QString>::ConstIterator i = urlMap.begin(); i != urlMap.end(); ++i) {
    if (i.value() == _filename) {
      name = Qt::escape(i.key());
      break;
    }
  }

  ts << indent << "<tag>" << Qt::escape(tag().tagString()) << "</tag>" << endl;
  ts << indent << "<filename>" << name << "</filename>" << endl;
  ts << indent << "<type>" << Qt::escape(fileType()) << "</type>" << endl;
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


const QMultiHash<QString, KstString>& KstDataSource::metaData() const {
  return _metaData;
}


QString KstDataSource::metaData(const QString& key) const {
  QString str = QString::null;
  
  if (_metaData.contains(key)) {
    str = _metaData.value(key).value();
  }

  return str;
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


KstFrameSize KstDataSource::sampleForTimeLarge(const KST::ExtDateTime& time, bool *ok) {
  return sampleForTime(time, ok);
}


int KstDataSource::sampleForTime(double ms, bool *ok) {
  Q_UNUSED(ms)

  if (ok) {
    *ok = false;
  }

  return 0;
}


KstFrameSize KstDataSource::sampleForTimeLarge(double ms, bool *ok) {
  return sampleForTime(ms, ok);
}


KST::ExtDateTime KstDataSource::timeForSample(int sample, bool *ok) {
  Q_UNUSED(sample)

  if (ok) {
    *ok = false;
  }

  return KST::ExtDateTime::currentDateTime();
}


KST::ExtDateTime KstDataSource::timeForSampleLarge(KstFrameSize sample, bool *ok) {
  if (sample <= INT_MAX) {
    return timeForSample((int)sample, ok);
  }

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

  return 0.0;
}


double KstDataSource::relativeTimeForSampleLarge(KstFrameSize sample, bool *ok) {
  if (sample <= INT_MAX) {
    relativeTimeForSample((int)sample, ok);
  }

  if (ok) {
    *ok = false;
  }

  return 0.0;
}


bool KstDataSource::supportsHierarchy() const {
  return false;
}


bool KstDataSource::reusable() const {
  return _reusable;
}


void KstDataSource::disableReuse() {
  _reusable = false;
}


QString KstDataSource::configuration(QString setting) {
  Q_UNUSED(setting)

  return QString();
}


bool KstDataSource::setConfiguration(QString setting, const QString &value) {
  Q_UNUSED(setting)
  Q_UNUSED(value)

  return false;
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


void KstDataSourceConfigWidget::setConfig(QSettings *cfg) {
  _cfg = cfg;
}


void KstDataSourceConfigWidget::setInstance(KstDataSourcePtr inst) {
  _instance = inst;
}


KstDataSourcePtr KstDataSourceConfigWidget::instance() const {
  return _instance;
}

#include "kstdatasource.moc"
