/* This file is derived from the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2004 The University of Toronto <netterfield@astro.utoronto.ca>

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

#include <QSharedPointer>
#include <QWeakPointer>
#include <QDebug>

#include "kst_export.h"

// NOTE: In order to preserve binary compatibility with plugins, you must
//       not add, remove, or change member variables or virtual functions.
//       You must also not remove or change non-virtual functions.

namespace Kst {

template< class T >
class SharedPtr;

template<class T>
class Shared
{

public:
  Shared(T* ptr) : obj(ptr) {}

  virtual ~Shared() {}


  QSharedPointer<T> toSharedPointer() const
  {
    QSharedPointer<T> ptr = _weakpointer.toStrongRef();
    if (ptr.isNull()) {
      QSharedPointer<T> tmp = QSharedPointer<T>(obj);
      ptr = tmp;
      _weakpointer = ptr.toWeakRef();
    }
    return ptr;
  }


  template<class Y>
  QSharedPointer<Y> toSharedPointer() const
  {
    QSharedPointer<Y> ptr = toSharedPointer().template objectCast<Y>();
    //QSharedPointer<Y> ptr = toSharedPointer().template dynamicCast<Y>();    
    Q_ASSERT(!ptr.isNull());
    return ptr;
  }


private:
  Shared();
  Shared(const Shared & rhs);
  Shared& operator=(const Shared & rhs);
  mutable QWeakPointer<T> _weakpointer;
  T* obj;
};



template< class T >
class SharedPtr
{
public:
  /**
   * Creates a null pointer.
   */
  SharedPtr() {}
  
  /**
   * Creates a new pointer.
   * @param t the pointer
   */
  SharedPtr( T* p ) 
  { 
    if (p)
      ptr = p->template toSharedPointer<T>();
  }

  /**
   * Copies a pointer.
   * @param p the pointer to copy
   */
  SharedPtr( const SharedPtr& p ) 
  { 
    if (p.isPtrValid()) 
      ptr = p->template toSharedPointer<T>();
  }

  template<class Y> 
  SharedPtr( const SharedPtr<Y>& p ) 
  { 
    if (p.isPtrValid()) 
      ptr = p->template toSharedPointer<T>();
  }

  /**
   * Unreferences the object that this pointer points to. If it was
   * the last reference, the object will be deleted.
   */
  ~SharedPtr() {}

  SharedPtr<T>& operator= ( const SharedPtr<T>& p ) 
  {
    if ( ptr == p.ptr ) 
      return *this;
    ptr = p.ptr;
    return *this;
  }


  template<class Y>
  SharedPtr<T>& operator=( const SharedPtr<Y>& p ) 
  {
    if (ptr == p.data()) 
      return *this;
    ptr = p->template toSharedPointer<T>();
    return *this;
  }

  SharedPtr<T>& operator= ( T* p ) 
  {
    if (ptr == p) 
      return *this;
    ptr = p->template toSharedPointer<T>();
    return *this;
  }

  bool operator== ( const SharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const SharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
  bool operator== ( const T* p ) const            { return ( ptr == p ); }
  bool operator!= ( const T* p ) const            { return ( ptr != p ); }
  bool operator!() const                          { return ( ptr == 0 ); }
  operator T*() const                             { return ptr.data(); }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  T* data() { return ptr.data(); }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  const T* data() const { return ptr.data(); }

  const T& operator*() const  { Q_ASSERT(isPtrValid()); return *ptr.data(); }
  T&       operator*()        { Q_ASSERT(isPtrValid()); return *ptr.data(); }
  const T* operator->() const { Q_ASSERT(isPtrValid()); return ptr.data(); }
  T*       operator->()       { Q_ASSERT(isPtrValid()); return ptr.data(); }

  bool isPtrValid() const { return !ptr.isNull(); } 

private:
  QSharedPointer<T> ptr;
    
};


template <typename T>
inline SharedPtr<T> kst_cast(QObject *object) 
{
  return SharedPtr<T>(qobject_cast<T*>(object));
}


template <typename T, typename U>
inline SharedPtr<T> kst_cast(SharedPtr<U>& object) 
{
  return kst_cast<T>(object.data());
}



}
#endif
