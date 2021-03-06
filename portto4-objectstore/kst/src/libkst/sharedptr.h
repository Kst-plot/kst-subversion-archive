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
#ifndef SharedPTR_H
#define SharedPTR_H

#include <q3semaphore.h>

#include <qdebug.h>

// NOTE: In order to preserve binary compatibility with plugins, you must
//       not add, remove, or change member variables or virtual functions.
//       You must also not remove or change non-virtual functions.

// See KSharedPtr in KDE libraries for more information

namespace Kst {

class Shared {
public:
   /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0.
    */
   Shared() : sem(999999) { }

   /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0.
    */
   Shared( const Shared & ) : sem(999999) { }

   /**
    * Overloaded assignment operator.
    */
   Shared &operator=(const Shared & ) { return *this; }

   /**
    * Increases the reference count by one.
    */
   void _KShared_ref() const {
	   sem++;
//	   qDebug() << "KShared_ref: " << (void*)this << " -> " << _KShared_count() << endl;
//	   qDebug() << kstdBacktrace() << endl;
   }

   /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself.
    */
   void _KShared_unref() const {
	   sem--;
//	   qDebug() << "KShared_unref: " << (void*)this << " -> " << _KShared_count() << endl;
//	   qDebug() << kstdBacktrace() << endl;
	   if (sem.total() == sem.available()) delete this;
   }

   /**
    * Return the current number of references held.
    *
    * @return Number of references
    */
   int _KShared_count() const { return sem.total() - sem.available(); }

protected:
   virtual ~Shared() { }
private:
   mutable Q3Semaphore sem;
};

template< class T >
struct SharedPtr
{
public:
  /**
   * Creates a null pointer.
   */
  SharedPtr() : ptr(0) { }
  /**
   * Creates a new pointer.
   * @param t the pointer
   */
  SharedPtr( T* t ) : ptr(t) { if ( ptr ) ptr->_KShared_ref(); }

  /**
   * Copies a pointer.
   * @param p the pointer to copy
   */
  SharedPtr( const SharedPtr& p )
	  : ptr(p.ptr) { if ( ptr ) ptr->_KShared_ref(); }

  template<class Y> SharedPtr(SharedPtr<Y>& p)
	  : ptr(p.data()) { if (ptr) ptr->_KShared_ref(); }

  /**
   * Unreferences the object that this pointer points to. If it was
   * the last reference, the object will be deleted.
   */
  ~SharedPtr() { if ( ptr ) ptr->_KShared_unref(); }

  SharedPtr<T>& operator= ( const SharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p.ptr;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }

  template<class Y>
  SharedPtr<T>& operator=(SharedPtr<Y>& p) {
    if (ptr == p.data()) return *this;
    if (ptr) ptr->_KShared_unref();
    ptr = p.data();
    if (ptr) ptr->_KShared_ref();
    return *this;
  }

  SharedPtr<T>& operator= ( T* p ) {
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  bool operator== ( const SharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const SharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
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

}
#endif
