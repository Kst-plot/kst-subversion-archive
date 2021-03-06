/* This file is derived from the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2004 The University of Toronto

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KstSharedPTR_H
#define KstSharedPTR_H

#include <qsemaphore.h>

// NOTE: In order to preserve binary compatibility with plugins, you must
//       not add, remove, or change member variables or virtual functions.
//       You must also not remove or change non-virtual functions.

// See KSharedPtr in KDE libraries for more information

class KstShared {
public:
   /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0.
    */
   KstShared() : sem(999999) { }

   /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0.
    */
   KstShared( const KstShared & ) : sem(999999) { }

   /**
    * Overloaded assignment operator.
    */
   KstShared &operator=(const KstShared & ) { return *this; }

   /**
    * Increases the reference count by one.
    */
   void _KShared_ref() const { sem++; }

   /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself.
    */
   void _KShared_unref() const { sem--;  if (sem.total() == sem.available()) delete this; }

   /**
    * Return the current number of references held.
    *
    * @return Number of references
    */
   int _KShared_count() const { return sem.total() - sem.available(); }

protected:
   virtual ~KstShared() { }
private:
   mutable QSemaphore sem;
};

template< class T >
struct KstSharedPtr
{
public:
  /**
   * Creates a null pointer.
   */
  KstSharedPtr() : ptr(0) { }
  /**
   * Creates a new pointer.
   * @param t the pointer
   */
  KstSharedPtr( T* t ) : ptr(t) { if ( ptr ) ptr->_KShared_ref(); }

  /**
   * Copies a pointer.
   * @param p the pointer to copy
   */
  KstSharedPtr( const KstSharedPtr& p )
    : ptr(p.ptr) { if ( ptr ) ptr->_KShared_ref(); }

  /**
   * Unreferences the object that this pointer points to. If it was
   * the last reference, the object will be deleted.
   */
  ~KstSharedPtr() { if ( ptr ) ptr->_KShared_unref(); }

  KstSharedPtr<T>& operator= ( const KstSharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p.ptr;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  KstSharedPtr<T>& operator= ( T* p ) {
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  bool operator== ( const KstSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const KstSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
  bool operator== ( const T* p ) const { return ( ptr == p ); }
  bool operator!= ( const T* p ) const { return ( ptr != p ); }
  bool operator!() const { return ( ptr == 0 ); }
  operator T*() const { return ptr; }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  T* data() { return ptr; }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  const T* data() const { return ptr; }

  const T& operator*() const { return *ptr; }
  T& operator*() { return *ptr; }
  const T* operator->() const { return ptr; }
  T* operator->() { return ptr; }

  /**
   * Returns the number of references.
   * @return the number of references
   */
  int count() const { return ptr->_KShared_count(); } // for debugging purposes
private:
  T* ptr;
};

#endif
