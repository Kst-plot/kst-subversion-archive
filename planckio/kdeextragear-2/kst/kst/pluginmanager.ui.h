/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void PluginManager::init()
{
    reloadList();
}


void PluginManager::selectionChanged( QListViewItem *item )
{
    bool on = (item != 0L);
    _remove->setEnabled(on);
}


void PluginManager::install()
{
    KURL xmlfile = KFileDialog::getOpenURL(QString::null, "*.xml", this, i18n("Please Select Plugin to Install"));
    
    if (xmlfile.isEmpty()) {
	return;
    }
    
    QString tmpFile;
    if (!KIO::NetAccess::download(xmlfile, tmpFile)) {
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
    KURL pathURL;
    pathURL.setPath(path);
    
    // First try copying the .so file in
    KURL sofile = xmlfile;
    QString tmpSoFile = sofile.path();
    tmpSoFile.replace(QRegExp(".xml$"), ".so");
    sofile.setPath(tmpSoFile);
    
    if (!KIO::NetAccess::dircopy(sofile, pathURL)) {
	KIO::NetAccess::removeTempFile(tmpFile);
	KMessageBox::error(this, i18n("Unable to copy plugin file %1 to %2.").arg(sofile.prettyURL()).arg(pathURL.prettyURL()), i18n("KST Plugin Loader"));
	return;
    }

    KURL tmpFileURL;
    tmpFileURL.setPath(tmpFile);
    pathURL.setFileName(xmlfile.fileName());
    
    if (!KIO::NetAccess::dircopy(tmpFileURL, pathURL)) {
	KMessageBox::error(this, i18n("Internal temporary file %1 could not be copied to plugin directory %2.").arg(tmpFile).arg(path), i18n("KST Plugin Loader"));
    }
    
    KIO::NetAccess::removeTempFile(tmpFile);
    rescan();
}


void PluginManager::remove()
{
    QListViewItem *item = _pluginList->selectedItem();
    if (!item) {
	return;
    }
    
    int rc = KMessageBox::questionYesNo(this, i18n("Are you sure you wish to remove the plugin \"%1\" from the system?").arg(item->text(0)), i18n("KST Plugin Loader"));
    
    if (rc != KMessageBox::Yes) {
	return;
    }
    
    if (PluginCollection::self()->isLoaded(item->text(0))) {
        PluginCollection::self()->unloadPlugin(item->text(0));
        item->setPixmap(0, locate("data", "kst/pics/no.png"));
    }
    
    PluginCollection::self()->deletePlugin(PluginCollection::self()->pluginNameList()[item->text(0)]);
    
    delete item;
    selectionChanged(_pluginList->selectedItem());
}


void PluginManager::rescan()
{
    PluginCollection::self()->rescan();
    reloadList();
}


void PluginManager::reloadList()
{
    _pluginList->clear();
    PluginCollection *pc = PluginCollection::self();
    QStringList loadedPluginList = pc->loadedPluginList();
    const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
    QMap<QString,Plugin::Data>::ConstIterator it;
    
    for (it = pluginList.begin(); it != pluginList.end(); ++it) {
        QListViewItem *i = new QListViewItem(_pluginList,
                                             it.data()._name,
                                             "",
                                             it.data()._description,
                                             it.data()._version,
                                             it.data()._author);
        if (loadedPluginList.contains(it.data()._name)) {
            i->setPixmap(1, locate("data", "kst/pics/yes.png"));
        /*} else {   // Don't do this for now.  It looks better without it.
            i->setPixmap(1, locate("data", "kst/pics/no.png"));*/
        }
    }
}
