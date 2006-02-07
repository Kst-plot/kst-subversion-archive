/***************************************************************************
                                 kstplugin.h
                             -------------------
    begin                : May 15 2003
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

#ifndef KSTPLUGIN_H
#define KSTPLUGIN_H

#include "kstdataobject.h"
#include "plugin.h"

#include <qvaluelist.h>
#include <qstring.h>
#include <qdom.h>

/*  Usage notes:
 *   - Output vectors are created internally, but may be renamed.
 *   - Input vectors must be set before setPlugin(), else it will fail.
 */

class KstPlugin : public KstDataObject {
public:
  KstPlugin();
  KstPlugin(QDomElement &e);
  virtual ~KstPlugin();

  virtual UpdateType update(int update_counter);

  virtual void save(QTextStream &ts);

  virtual bool slaveVectorsUsed() const;
  bool isValid() const;

  virtual bool setPlugin(KstSharedPtr<Plugin> plugin);
  KstSharedPtr<Plugin> plugin() const;

  virtual QString propertyString() const;

protected:
  virtual void _showDialog();
  KstSharedPtr<Plugin> _plugin;

private:
  void commonConstructor();
  unsigned _inScalarCnt, _inArrayCnt, _outScalarCnt, _outArrayCnt;
  void *_localData;
};

typedef KstSharedPtr<KstPlugin> KstPluginPtr;
typedef KstObjectList<KstPluginPtr> KstPluginList;

#endif

// vim: ts=2 sw=2 et
