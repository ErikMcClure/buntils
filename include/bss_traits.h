// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_TRAITS_H__
#define __BSS_TRAITS_H__

#include "bss_call.h"
#include "bss_deprecated.h"

namespace bss_util {
  template<typename T>
	class BSS_COMPILER_DLLEXPORT RefTraits
  {
	public: 
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T&& move_reference;
    typedef T value_type;
  };
  
  template<typename T>
	class BSS_COMPILER_DLLEXPORT ValueTraits
  {
	public: 
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T reference;
    typedef const T const_reference;
    typedef T&& move_reference;
    typedef T value_type;
  };
}

#endif