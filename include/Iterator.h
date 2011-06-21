// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __ITERATOR_H__BSS__
#define __ITERATOR_H__BSS__

#include "bss_traits.h"

namespace bss_util {
  // An implementation of a nice, clean abstract iterator class, based on the std implementation, but without all the stupidity.
  template<class T, class Traits=ValueTraits<T>>
  class __declspec(dllexport) Iterator_Forward
  {
    typedef typename ValueTraits<T>::const_reference const_reference;
    typedef typename ValueTraits<T>::value_type value_type;

  public:
    inline virtual ~Iterator_Forward() {}
    inline virtual const_reference operator++()=0; //prefix
    inline virtual value_type operator++(int)=0; //postfix
    inline const_reference Next() { return ++(*this); }
    inline virtual const_reference Peek()=0;
    inline virtual bool Remove()=0; // Removes whatever peek() would return. If peek is called again or a postfix increment is used, the return value is undefined and may vary based on the implementation
    inline virtual bool HasNext()=0;
  };

  template<class T, class Traits=ValueTraits<T>>
  class __declspec(dllexport) Iterator : public Iterator_Forward<T,Traits>
  {
    typedef typename ValueTraits<T>::const_reference const_reference;
    typedef typename ValueTraits<T>::value_type value_type;

  public:
    inline virtual const_reference operator--()=0; //prefix
    inline virtual value_type operator--(int)=0; //postfix
    inline const_reference Prev() { return --(*this); }
    inline virtual bool HasPrev()=0;
  };
}

#endif