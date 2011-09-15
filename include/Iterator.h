// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ITERATOR_H__BSS__
#define __ITERATOR_H__BSS__

#include "bss_traits.h"

namespace bss_util {
  // An implementation of a nice, clean abstract iterator class, based on the std implementation, but without all the stupidity.
  template<class T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT Iterator_Forward
  {
  protected:
    typedef typename ValueTraits<T>::const_reference const_reference;
    typedef typename ValueTraits<T>::value_type value_type;

  public:
    inline virtual ~Iterator_Forward() {}
    inline virtual const_reference operator++()=0; //prefix
    inline value_type operator++(int) { return ++(*this); } //postfix
    inline const_reference Next() { return ++(*this); }
    inline virtual const_reference Peek()=0; // Returns what would be returned if Next() was called but without incrementing the iterator
    //inline virtual const_reference Last()=0; // Returns the last value returned by Next()
    inline virtual void Remove()=0; // Removes last value returned by Next()
    inline virtual bool HasNext()=0;
  };
  
  //This iterator only gaurentees that you can progress backwards. Remove() and Peek() are not defined after a decrement call.
  template<class T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT Iterator : public Iterator_Forward<T,Traits> 
  {
  protected:
    typedef typename Iterator_Forward<T,Traits>::const_reference const_reference;
    typedef typename Iterator_Forward<T,Traits>::value_type value_type;

  public:
    inline virtual const_reference operator--()=0; //prefix
    inline value_type operator--(int) { return --(*this); }
    inline const_reference Prev() { return --(*this); }
    inline virtual bool HasPrev()=0;
  };
}

#endif