/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTWIDGETS_H
#define KSTWIDGETS_H

#include <QObject>
#include <QDesignerCustomWidgetInterface>

//FIXME Remove this eventually...
#include <kcomponentdata.h>

#include <QtPlugin>

class KstWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface {
public:
  KstWidgetPlugin(QObject *parent = 0) : QObject(parent), _initialized(false) {}

  QString group() const {
    return tr("Kst Widgets");
  }
  QString toolTip() const {
    return QString::null;
  }
  QString whatsThis() const {
    return QString::null;
  }

  QString instanceName() const {
    QChar camel = name().at(0).toLower();
    return name().replace(0,1,camel.toLower());
  }

  QString includeFile() const {
    return name().toLower() + ".h";
  }

  QString domXml() const {
    return QString::fromUtf8("<widget class=\"%1\" name=\"%2\"/>")
           .arg(name()).arg(instanceName().toLower());
  }

  bool isContainer() const {
    return false;
  }
  bool isInitialized() const {
    return _initialized;
  }
  QIcon icon() const {
    return QIcon();
  }

  void initialize(QDesignerFormEditorInterface *) {
    if (_initialized)
      return;

    _initialized = true;
  }

private:
  bool _initialized;
};


#include "colorbutton.h"
class ColorButtonPlugin : public KstWidgetPlugin {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
  ColorButtonPlugin(QObject *parent = 0) : KstWidgetPlugin(parent) {}
  QString name() const {
    return QLatin1String("Kst::ColorButton");
  } //do not translate
  QWidget *createWidget(QWidget *parent) {
    return new Kst::ColorButton(parent);
  }
};


class KstWidgets : public QObject, public QDesignerCustomWidgetCollectionInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
  KstWidgets(QObject *parent = 0);
  virtual ~KstWidgets() {}
  QList<QDesignerCustomWidgetInterface*> customWidgets() const {
    return _plugins;
  }

private:
  QList<QDesignerCustomWidgetInterface*> _plugins;
};
Q_EXPORT_PLUGIN2(kstwidgets, KstWidgets)


KstWidgets::KstWidgets(QObject *parent)
    : QObject(parent) {
  (void) new KComponentData("kstwidgets");
  _plugins.append(new ColorButtonPlugin(this));
}

#endif
// vim: ts=2 sw=2 et
