/***************************************************************************
                                bind_matrix.h
                             -------------------
    begin                : Jul 27 2005
    copyright            : (C) 2005 The University of Toronto
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

#ifndef BIND_MATRIX_H
#define BIND_MATRIX_H

#include "bind_object.h"

#include <kstmatrix.h>

/* @class Matrix
   @inherits Object
   @collection MatrixCollection Kst.matrices
   @description Represents a matrix of any type in Kst.  Some matrices are
                editable, while others are not.
*/
class KstBindMatrix : public KstBindObject {
  public:
    /* @constructor
       @description Default constructor creates an empty matrix.
    */
    KstBindMatrix(KJS::ExecState *exec, KstMatrixPtr d, const char *name = 0L);
    KstBindMatrix(KJS::ExecState *exec, KJS::Object *globalObject = 0L, const char *name = 0L);
    ~KstBindMatrix();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    int methodCount() const;
    int propertyCount() const;

    /* @method resize
       @description Resizes the matrix.
       @arg number columns The new number of rows in the matrix.  Should be &gt;= 1.
       @arg number rows The new number of columns in the matrix.  Should be &gt;= 1.
       @exception GeneralError Throws this exception if the matrix is not
                               editable.
     */
    KJS::Value resize(KJS::ExecState *exec, const KJS::List& args);

    /* @method zero
       @description Sets all values in the matrix to zero.
       @exception GeneralError Throws this exception if the matrix is not editable.
     */
    KJS::Value zero(KJS::ExecState *exec, const KJS::List& args);

    /* @method value
       @returns number
       @description Gets the value of a matrix at the given column and row indices.
       @arg number column The column index of the matrix position to be read.
       @arg number row The row index of the matrix position to be read.
       @exception GeneralError Throws this exception if the indices are out of range.
     */
    KJS::Value value(KJS::ExecState *exec, const KJS::List& args);

    /* @method setValue
       @description Sets the value of a matrix at the given column and row indices.
       @arg number column The column index of the matrix position to be set.
       @arg number row The row index of the matrix position to be set.
       @arg number value The value to set the specified matrix position to.
       @exception GeneralError Throws this exception if the matrix is not editable or the indices are out of range.
     */
    KJS::Value setValue(KJS::ExecState *exec, const KJS::List& args);

    /* @method update
       @description Updates the statistical values associated with a matrix.
       @exception GeneralError Throws this exception if the matrix is not editable.
     */
    KJS::Value update(KJS::ExecState *exec, const KJS::List& args);

    /* @property number editable
       @readonly
       @description True if the matrix is editable by the user.  This should be
       checked before attempting to modify a matrix in any way.
     */
    KJS::Value editable(KJS::ExecState *exec) const;

    /* @property number min
       @readonly
       @description The value of the smallest sample in the matrix.
     */
    KJS::Value min(KJS::ExecState *exec) const;

    /* @property number max
       @readonly
       @description The value of the largest sample in the matrix.
     */
    KJS::Value max(KJS::ExecState *exec) const;

    /* @property number mean
       @readonly
       @description The mean value of all samples in the matrix.
     */
    KJS::Value mean(KJS::ExecState *exec) const;

    /* @property number numNew
       @readonly
       @description The number of new samples.
     */
    KJS::Value numNew(KJS::ExecState *exec) const;

    /* @property number rows
       @readonly
       @description The number of rows in the matrix.
     */
    KJS::Value rows(KJS::ExecState *exec) const;

    /* @property number columns
       @readonly
       @description The number of coluns in the matrix.
     */
    KJS::Value columns(KJS::ExecState *exec) const;

  protected:
    KstBindMatrix(int id, const char *name = 0L);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
};

#endif
