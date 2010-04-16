/***************************************************************************
                     kstpluginmanager.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
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

#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QRadioButton>
#include <QRegExp>
#include <QFileDialog>

#include "kstpluginmanager.h"
#include "plugincollection.h"
#include "pluginxmlparser.h"

#define COLUMN_READABLE_NAME  0
#define COLUMN_LOADED         1
#define COLUMN_NAME           5

KstPluginManager::KstPluginManager(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl) {
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_pluginList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
  connect(_install, SIGNAL(clicked()), this, SLOT(install()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(_rescan, SIGNAL(clicked()), this, SLOT(rescan()));

// xxx  _pluginList->setAllColumnsShowFocus(true);
  reloadList();
}


KstPluginManager::~KstPluginManager() {
}


void KstPluginManager::selectionChanged(QListWidgetItem *item)
{
  _remove->setEnabled(item != 0L);
}


void KstPluginManager::install() {

  QUrl xmlfile = QFileDialog::getOpenFileName(this, QObject::tr("Select Plugin to Install"), QString::null, tr("XML files (*.xml)"));

  if (xmlfile.isEmpty()) {
    return;
  }

  QString tmpFile;
/* xxx
  if (!KIO::NetAccess::download(xmlfile, tmpFile, this)) {
    QMessageBox::critical(this, QObject::tr("Kst"),QObject::tr("Unable to access file %1.").arg(xmlfile.prettyURL()), QObject::tr("KST Plugin Loader"));
    return;
  }

  PluginXMLParser parser;

  if (parser.parseFile(tmpFile)) {
    KIO::NetAccess::removeTempFile(tmpFile);
    QMessageBox::critical(this, QObject::tr("Kst"),QObject::tr("Invalid plugin file."), QObject::tr("KST Plugin Loader"));
    return;
  }

  QString path = KGlobal::dirs()->saveLocation("kstplugins");
  KUrl pathURL;

  pathURL.setPath(path);

  // first try copying the .so file in
  KUrl sofile = xmlfile;
  KUrl tmpFileURL;
  QString tmpSoFile = sofile.path();
  tmpSoFile.replace(QRegExp(".xml$"), ".so");
  sofile.setPath(tmpSoFile);

  if (!KIO::NetAccess::dircopy(sofile, pathURL, this)) {
    KIO::NetAccess::removeTempFile(tmpFile);
    QMessageBox::critical(this, QObject::tr("Kst"), QObject::tr("Unable to copy plugin file %1 to %2.").arg(sofile.prettyURL()).arg(pathURL.prettyURL()), QObject::tr("KST Plugin Loader"));
    return;
  }

  tmpFileURL.setPath(tmpFile);
  pathURL.setFileName(xmlfile.fileName());

  if (!KIO::NetAccess::dircopy(tmpFileURL, pathURL, this)) {
    QMessageBox::critical(this, QObject::tr("Kst"), QObject::tr("Internal temporary file %1 could not be copied to plugin directory %2.").arg(tmpFile).arg(path), QObject::tr("KST Plugin Loader"));
  }

  KIO::NetAccess::removeTempFile(tmpFile);
  rescan();
*/
}


void KstPluginManager::remove() {
  if (!_pluginList->selectedItems().isEmpty()) {
    QListWidgetItem *item = 0L;

    item = _pluginList->selectedItems().front();
/* xxx
    int rc = QMessageBox::question(this, QObject::tr("Kst"),QObject::tr("Are you sure you wish to remove the plugin \"%1\" from the system?").arg(item->text(COLUMN_READABLE_NAME)), QObject::tr("KST Plugin Loader"));
  
    if (rc != QMessageBox::Yes) {
      return;
    }
  
    if (PluginCollection::self()->isLoaded(item->text(COLUMN_NAME))) {
      PluginCollection::self()->unloadPlugin(item->text(COLUMN_NAME));
    item->setIcon(COLUMN_LOADED, locate("data", "kst/pics/no.png"));
    }
  
    PluginCollection::self()->deletePlugin(PluginCollection::self()->pluginNameList()[item->text(COLUMN_NAME)]);
*/    
    delete item;
    
    if (!_pluginList->selectedItems().isEmpty()) {
      item = _pluginList->selectedItems().front();
      selectionChanged(item);
    }
  }
}


void KstPluginManager::rescan() {
  PluginCollection::self()->rescan();
  reloadList();

  emit rescanned();
}


void KstPluginManager::reloadList() {
  _pluginList->clear();

  PluginCollection *pc = PluginCollection::self();
  QStringList loadedPluginList = pc->loadedPluginList();
  const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
  QMap<QString,Plugin::Data>::const_iterator it;

  for (it = pluginList.begin(); it != pluginList.end(); ++it) {
    QString path = pc->pluginNameList()[(*it)._name];
    QListWidgetItem *i;
/* xxx
    i = new QListWidgetItem(_pluginList, (*it)._readableName, QString::null,
            (*it)._description, (*it)._version, (*it)._author, (*it)._name, path);
    if (loadedPluginList.contains((*it)._name)) {
      i->setIcon(COLUMN_LOADED, locate("data", "kst/pics/yes.png"));
    }
*/
  }
}

#include "kstpluginmanager.moc"

