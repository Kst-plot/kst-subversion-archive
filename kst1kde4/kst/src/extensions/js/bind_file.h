/***************************************************************************
                               bind_file.h
                             -------------------
    begin                : Oct 18 2007
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

#ifndef BIND_FILE_H
#define BIND_FILE_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class File
   @description An object that represents a file.
*/
class KstBindFile : public KstBinding {
  public:
    /* @constructor
       @arg string name Name of the file to be opened.
       @description Constructs a file with the given name.
    */
    KstBindFile(KJS::ExecState *exec, QFile *f);
    KstBindFile(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindFile();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    /* @method open
       @description Opens the file.
    */
    KJS::Value open(KJS::ExecState *exec, const KJS::List& args);

    /* @method close
       @description Closes the file.
    */
    KJS::Value close(KJS::ExecState *exec, const KJS::List& args);

    /* @method readLine
       @returns string
       @optarg number length Maximum length of string to return
       @description Reads a line from the file.
    */
    KJS::Value readLine(KJS::ExecState *exec, const KJS::List& args);

    /* @property string name
       @description The name of the file.
    */
    KJS::Value name(KJS::ExecState *exec) const;

    /* @property number size
       @description The size of the file.
    */
    KJS::Value size(KJS::ExecState *exec) const;

    /* @property boolean exists
       @description True if the underlying file exists.
    */
    KJS::Value exists(KJS::ExecState *exec) const;

  protected:
    KstBindFile(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    QFile *_f;
};


#endif
