/***************************************************************************
                               bind_elog.h
                             -------------------
    begin                : Nov 20 2007
    copyright            : (C) 2007 The University of British Columbia
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

#ifndef BIND_ELOG_H
#define BIND_ELOG_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

#define MAX_ATTACHMENTS  50
#define MAX_N_ATTR       50

/* @class ELOG
   @description An object that represents the interface to an ELOG server.
*/
class KstBindELOG : public KstBinding {
  public:
    /* @constructor
       @description Interface to an ELOG server.
    */
    KstBindELOG(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindELOG();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    /* @method submit
       @description Submits the ELOG entry.
    */
    KJS::Value submit(KJS::ExecState *exec, const KJS::List& args);

    /* @method addAttachment
       @arg string attachment The name of the attachment.
       @description Adds an attachment to the ELOG entry.
    */
    KJS::Value addAttachment(KJS::ExecState *exec, const KJS::List& args);

    /* @method clearAttachments
       @description Clears the list of attachments in the ELOG entry.
    */
    KJS::Value clearAttachments(KJS::ExecState *exec, const KJS::List& args);

    /* @method addAttribute
       @arg string attribute The name of the attribute to be added to the ELOG entry.
       @arg string attributeValue The value of the attribute to be added to the ELOG entry.
       @description Adds an attribute to the ELOG entry.
    */
    KJS::Value addAttribute(KJS::ExecState *exec, const KJS::List& args);

    /* @method removeAttribute
       @arg string attribute The name of the attribute to be removed from the ELOG entry.
       @description Removes an attribute from the ELOG entry.
    */
    KJS::Value removeAttribute(KJS::ExecState *exec, const KJS::List& args);

    /* @method clearAttributes
       @description Clears the list of attributes in the ELOG entry.
    */
    KJS::Value clearAttributes(KJS::ExecState *exec, const KJS::List& args);

    /* @property string hostname
       @description The hostname of the ELOG server.
    */
    KJS::Value hostname(KJS::ExecState *exec) const;
    void setHostname(KJS::ExecState *exec, const KJS::Value& value);

    /* @property number port
       @description The port number of the ELOG server.
    */
    KJS::Value port(KJS::ExecState *exec) const;
    void setPort(KJS::ExecState *exec, const KJS::Value& value);

    /* @property string logbook
       @description A weblog made available by the ELOG server.
    */
    KJS::Value logbook(KJS::ExecState *exec) const;
    void setLogbook(KJS::ExecState *exec, const KJS::Value& value);

    /* @property string username
       @description Username.
    */
    KJS::Value username(KJS::ExecState *exec) const;
    void setUsername(KJS::ExecState *exec, const KJS::Value& value);

    /* @property string password
       @description Password.
    */
    KJS::Value password(KJS::ExecState *exec) const;
    void setPassword(KJS::ExecState *exec, const KJS::Value& value);

    /* @property string writePassword
       @description The write password.
    */
    KJS::Value writePassword(KJS::ExecState *exec) const;
    void setWritePassword(KJS::ExecState *exec, const KJS::Value& value);

    /* @property string text
       @description Text.
    */
    KJS::Value text(KJS::ExecState *exec) const;
    void setText(KJS::ExecState *exec, const KJS::Value& value);

    /* @property boolean encodedHTML
       @description The message is HTML encoded.
    */
    KJS::Value encodedHTML(KJS::ExecState *exec) const;
    void setEncodedHTML(KJS::ExecState *exec, const KJS::Value& value);

    /* @property boolean suppressEmailNotification
       @description If true email notification is suppressed.
    */
    KJS::Value suppressEmailNotification(KJS::ExecState *exec) const;
    void setSuppressEmailNotification(KJS::ExecState *exec, const KJS::Value& value);

    /* @property boolean includeCapture
       @description If true an image capture of the Kst session is included in the ELOG entry.
    */
    KJS::Value includeCapture(KJS::ExecState *exec) const;
    void setIncludeCapture(KJS::ExecState *exec, const KJS::Value& value);

    /* @property number captureWidth
       @description The width of the image capture to be included in the ELOG entry.
    */
    KJS::Value captureWidth(KJS::ExecState *exec) const;
    void setCaptureWidth(KJS::ExecState *exec, const KJS::Value& value);

    /* @property number captureHeight
       @description The height of the image capture to be included in the ELOG entry.
    */
    KJS::Value captureHeight(KJS::ExecState *exec) const;
    void setCaptureHeight(KJS::ExecState *exec, const KJS::Value& value);

    /* @property boolean includeConfiguration
       @description If true a capture of the Kst session is included in the ELOG entry.
    */
    KJS::Value includeConfiguration(KJS::ExecState *exec) const;
    void setIncludeConfiguration(KJS::ExecState *exec, const KJS::Value& value);

    /* @property boolean includeDebugInfo
       @description If true a capture of the Kst debug information is included in the ELOG entry.
    */
    KJS::Value includeDebugInfo(KJS::ExecState *exec) const;
    void setIncludeDebugInfo(KJS::ExecState *exec, const KJS::Value& value);

  protected:
    KstBindELOG(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:

  private:
    QString _hostname;
    int _port;
    QString _logbook;
    QString _writePassword;
    QString _password;
    QString _username;
    QString _text;
    QStringList _attachments;
    QMap<QString, QString> _attributes;

    bool _suppressEmailNotification;
    bool _encodedHTML;
    bool _includeCapture;
    bool _includeConfiguration;
    bool _includeDebugInfo;
    int _captureWidth;
    int _captureHeight;
};

#endif
