/***************************************************************************
                     kstpluginmanager_i.cpp  -  Part of KST
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

// include files for Qt
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qradiobutton.h>
#include <qregexp.h>

// include files for KDE
#include <kconfig.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

// application specific includes
#include "kstpluginmanager_i.h"
#include "plugincollection.h"
#include "pluginxmlparser.h"

#define COLUMN_READABLE_NAME  0
#define COLUMN_LOADED         1
#define COLUMN_NAME           5

KstPluginManagerI::KstPluginManagerI(QWidget* parent, const char* name, bool modal, WFlags fl)
: PluginManager(parent, name, modal, fl) {
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_pluginList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));
  connect(_install, SIGNAL(clicked()), this, SLOT(install()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(remove()));
  connect(_rescan, SIGNAL(clicked()), this, SLOT(rescan()));

  _pluginList->setAllColumnsShowFocus(true);
  reloadList();
}


KstPluginManagerI::~KstPluginManagerI() {
}


void KstPluginManagerI::selectionChanged( QListViewItem *item )
{
  _remove->setEnabled(item != 0L);
}


void KstPluginManagerI::install() {
  KUrl xmlfile = KFileDialog::getOpenURL(QString::null, "*.xml", this, i18n("Select Plugin to Install"));

  if (xmlfile.isEmpty()) {
    return;
  }

  QString tmpFile;
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  if (!KIO::NetAccess::download(xmlfile, tmpFile, this)) {
#else
  if (!KIO::NetAccess::download(xmlfile, tmpFile)) {
#endif
    KMessageBox::error(this, i18n("Unable to access file %1.").arg(xmlfile.prettyURL()), i18n("KST Plugin Loader"));
    return;
  }

  PluginXMLParser parser;

  if (parser.parseFile(tmpFile)) {
    KIO::NetAccess::removeTempFile(tmpFile);
    KMessageBox::error(this, i18n("Invalid plugin file."), i18n("KST Plugin Loader"));
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
    KMessageBox::error(this, i18n("Unable to copy plugin file %1 to %2.").arg(sofile.prettyURL()).arg(pathURL.prettyURL()), i18n("KST Plugin Loader"));
    return;
  }

  tmpFileURL.setPath(tmpFile);
  pathURL.setFileName(xmlfile.fileName());

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
  if (!KIO::NetAccess::dircopy(tmpFileURL, pathURL, this)) {
#else
  if (!KIO::NetAccess::dircopy(tmpFileURL, pathURL)) {
#endif
    KMessageBox::error(this, i18n("Internal temporary file %1 could not be copied to plugin directory %2.").arg(tmpFile).arg(path), i18n("KST Plugin Loader"));
  }

  KIO::NetAccess::removeTempFile(tmpFile);
  rescan();
}


void KstPluginManagerI::remove() {
  QListViewItem *item = _pluginList->selectedItem();
  if (!item) {
    return;
  }

  int rc = KMessageBox::questionYesNo(this, i18n("Are you sure you wish to remove the plugin \"%1\" from the system?").arg(item->text(COLUMN_READABLE_NAME)), i18n("KST Plugin Loader"));

  if (rc != KMessageBox::Yes) {
    return;
  }

  if (PluginCollection::self()->isLoaded(item->text(COLUMN_NAME))) {
    PluginCollection::self()->unloadPlugin(item->text(COLUMN_NAME));
    item->setPixmap(COLUMN_LOADED, locate("data", "kst/pics/no.png"));
  }

  PluginCollection::self()->deletePlugin(PluginCollection::self()->pluginNameList()[item->text(COLUMN_NAME)]);

  delete item;
  selectionChanged(_pluginList->selectedItem());
}


void KstPluginManagerI::rescan() {
  PluginCollection::self()->rescan();
  reloadList();
  emit rescanned();
}


void KstPluginManagerI::reloadList() {
  _pluginList->clear();

  PluginCollection *pc = PluginCollection::self();
  QStringList loadedPluginList = pc->loadedPluginList();
  const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
  QMap<QString,Plugin::Data>::ConstIterator it;

  for (it = pluginList.begin(); it != pluginList.end(); ++it) {
    QString path = pc->pluginNameList()[it.data()._name];
    QListViewItem *i = new QListViewItem(_pluginList,
            it.data()._readableName,
            QString::null,
            it.data()._description,
            it.data()._version,
            it.data()._author,
            it.data()._name,
            path);
    if (loadedPluginList.contains(it.data()._name)) {
      i->setPixmap(COLUMN_LOADED, locate("data", "kst/pics/yes.png"));
      // don't use no.png - it looks bad
    }
  }
}

#include "kstpluginmanager_i.moc"

