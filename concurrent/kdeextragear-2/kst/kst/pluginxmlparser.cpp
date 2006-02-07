/***************************************************************************
                      pluginxmlparser.cpp  -  Part of KST
                             -------------------
    begin                : Tue May 06 2003
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


#include "pluginxmlparser.h"

#include <kdebug.h>
#include <qstring.h>
#include <qfile.h>


PluginXMLParser::PluginXMLParser() {
}


PluginXMLParser::~PluginXMLParser() {
}


int PluginXMLParser::parseFile(const QString& filename) {
  QFile qf(filename);

  if (!qf.open(IO_ReadOnly)) {
    return -1;
  }

  QDomDocument doc(filename);
  if (!doc.setContent(&qf)) {
    qf.close();
    return -2;
  }

  qf.close();

  _pluginData.clear();

  return parseDOM(doc);
}


const Plugin::Data& PluginXMLParser::data() const {
  return _pluginData;
}


static const QString QS_string = QString::fromLatin1("string");
static const QString QS_int = QString::fromLatin1("int");
static const QString QS_name = QString::fromLatin1("name");
static const QString QS_helptext = QString::fromLatin1("helptext");
static const QString QS_intro = QString::fromLatin1("intro");
static const QString QS_interface = QString::fromLatin1("interface");
static const QString QS_paralist = QString::fromLatin1("paralist");
static const QString QS_modulename = QString::fromLatin1("modulename");
static const QString QS_author = QString::fromLatin1("author");
static const QString QS_description = QString::fromLatin1("description");
static const QString QS_descr = QString::fromLatin1("descr");
static const QString QS_version = QString::fromLatin1("version");
static const QString QS_state = QString::fromLatin1("state");
static const QString QS_platforms = QString::fromLatin1("platforms");
static const QString QS_language = QString::fromLatin1("language");
static const QString QS_documentation = QString::fromLatin1("documentation");
static const QString QS_text = QString::fromLatin1("text");
static const QString QS_major = QString::fromLatin1("major");
static const QString QS_minor = QString::fromLatin1("minor");
static const QString QS_devstate = QString::fromLatin1("devstate");
static const QString QS_prealpha = QString::fromLatin1("pre-alpha");
static const QString QS_alpha = QString::fromLatin1("alpha");
static const QString QS_beta = QString::fromLatin1("beta");
static const QString QS_release = QString::fromLatin1("release");
static const QString QS_input = QString::fromLatin1("input");
static const QString QS_output = QString::fromLatin1("output");
static const QString QS_table = QString::fromLatin1("table");
static const QString QS_map = QString::fromLatin1("map");
static const QString QS_type = QString::fromLatin1("type");
static const QString QS_float = QString::fromLatin1("float");
static const QString QS_any = QString::fromLatin1("any");
static const QString QS_integer = QString::fromLatin1("integer");
static const QString QS_filter = QString::fromLatin1("filter");
static const QString QS_localdata = QString::fromLatin1("localdata");


int PluginXMLParser::parseDOM(const QDomDocument& doc) {
QDomElement topElem = doc.documentElement();

  if (topElem.tagName().lower() == QString::fromLatin1("module")) {
    QDomNode n = topElem.firstChild();

    while (!n.isNull()) {
      QDomElement e = n.toElement();
      QString tn = e.tagName().lower();
      int rc = 0;

      if (tn == QS_interface) {
        rc = parseInterface(e);
      } else if (tn == QS_intro) {
        rc = parseIntro(e);
      } else if (tn == QS_paralist) {
        rc = parseParalist(e);
      } else {
        // Unknown tag
      }

      if (rc < 0) {  // error occurred
        return rc;
      }

      n = n.nextSibling();
    }

  } else {
    return -3;  // XML parse error - no "module" at the top
  }

return 0;
}


int PluginXMLParser::parseIntro(const QDomElement& element) {
QDomNode n = element.firstChild();

  while (!n.isNull()) {
    int rc = 0;
    QDomElement e = n.toElement();

    if (e.isNull()) {
      n = n.nextSibling();
      continue;
    }

    QString tn = e.tagName().lower();
    if (tn == QS_modulename) {
      _pluginData._name = e.attribute(QS_name);
    } else if (tn == QS_localdata) {
      _pluginData._localdata = true;
    } else if (tn == QS_filter) {
      _pluginData._filter = true;
    } else if (tn == QS_author) {
      _pluginData._author = e.attribute(QS_name);
    } else if (tn == QS_description) {
      _pluginData._description = e.attribute(QS_text);
    } else if (tn == QS_version) {
      _pluginData._version = QString("%1.%2").arg(e.attribute(QS_major))
                                             .arg(e.attribute(QS_minor));
    } else if (tn == QS_state) {
      QString st = e.attribute(QS_devstate).lower();
      _pluginData._state = Plugin::Data::Unknown;

      if (st == QS_prealpha) {
        _pluginData._state = Plugin::Data::PreAlpha;
      } else if (st == QS_alpha) {
        _pluginData._state = Plugin::Data::Alpha;
      } else if (st == QS_beta) {
        _pluginData._state = Plugin::Data::Beta;
      } else if (st == QS_release) {
        _pluginData._state = Plugin::Data::Release;
      }
    } else if (tn == QS_platforms) {
      // Unimplemented
    } else if (tn == QS_language) {
      // Unimplemented
    } else if (tn == QS_documentation) {
      // Unimplemented
    } else {
      // Unknown node
    }

    if (rc < 0) {
      return rc;
    }

    n = n.nextSibling();
  }

return 0;
}


int PluginXMLParser::parseInterface(const QDomElement& element) {
QDomNode n = element.firstChild();

  while (!n.isNull()) {
    int rc = 0;
    QDomElement e = n.toElement();

    if (e.isNull()) {
      n = n.nextSibling();
      continue;
    }

    QString tn = e.tagName().lower();
    if (tn == QS_input) {
      rc = parseIO(e, _pluginData._inputs);
    } else if (tn == QS_output) {
      rc = parseIO(e, _pluginData._outputs);
    } else {
      // Unknown node
    }

    if (rc < 0) {
      return rc;
    }

    n = n.nextSibling();
  }

return 0;
}


int PluginXMLParser::parseIO(const QDomElement& element, QValueList<Plugin::Data::IOValue>& collection) {
QDomNode n = element.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();

    if (e.isNull()) {
      n = n.nextSibling();
      continue;
    }

    Plugin::Data::IOValue iov;

    QString tn = e.tagName().lower();
    if (tn == QS_table) {
      iov._type = Plugin::Data::IOValue::TableType;
//    } else if (tn == QS_integer) {
//      iov._type = Plugin::Data::IOValue::IntegerType;
    } else if (tn == QS_float) {
      iov._type = Plugin::Data::IOValue::FloatType;
//    } else if (tn == QS_string) {
//      iov._type = Plugin::Data::IOValue::StringType;
//    } else if (tn == QS_map) {
//      iov._type = Plugin::Data::IOValue::MapType;
    } else {
      // Unknown node
      iov._type = Plugin::Data::IOValue::UnknownType;
      return -1;
    }

    if (iov._type != Plugin::Data::IOValue::UnknownType) {
      iov._name = e.attribute(QS_name);
      iov._description = e.attribute(QS_descr);

      QString subtype = e.attribute(QS_type).lower();
      if (subtype == QS_float) {
        iov._subType = Plugin::Data::IOValue::FloatSubType;
//      } else if (subtype == QS_any) {
//        iov._subType = Plugin::Data::IOValue::AnySubType;
//      } else if (subtype == QS_string) {
//        iov._subType = Plugin::Data::IOValue::StringSubType;
//      } else if (subtype == QS_integer) {
//        iov._subType = Plugin::Data::IOValue::IntegerSubType;
      } else {
        iov._subType = Plugin::Data::IOValue::UnknownSubType;
      }

      if (iov._type != Plugin::Data::IOValue::TableType ||
          iov._subType != Plugin::Data::IOValue::UnknownSubType) {
        collection.append(iov);
      } else {
        return -1;
      }
    }

    n = n.nextSibling();
  }

return 0;
}

int PluginXMLParser::parseParalist(const QDomElement& element) {
QDomNode n = element.firstChild();

  while (!n.isNull()) {
    int rc = 0;
    QDomElement e = n.toElement();

    if (e.isNull()) {
      n = n.nextSibling();
      continue;
    }

    QString tn = e.tagName().lower();
    if (tn == QS_string) {
      _pluginData._parameters[e.attribute(QS_name)] = qMakePair(Plugin::Data::String, e.attribute(QS_helptext));
    } else if (tn == QS_int) {
      _pluginData._parameters[e.attribute(QS_name)] = qMakePair(Plugin::Data::Integer, e.attribute(QS_helptext));
    } else {
      // Unknown node
    }

    if (rc < 0) {
      return rc;
    }

    n = n.nextSibling();
  }

return 0;
}


// vim: ts=2 sw=2 et
