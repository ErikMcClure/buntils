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
    typedef T value_type;
  };

  template<typename T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT CompareKeysTraits : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_pointer constptr;
    typedef typename Traits::pointer pointer;

    static inline char Compare(constref keyleft, constref keyright)
    {
      return (keyleft < keyright)?-1:((keyleft == keyright)?0:1);
    }
  };

  template<typename T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT CompareShortsTraits : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_pointer constptr;
    typedef typename Traits::pointer pointer;

    static inline char Compare(constref keyleft, constref keyright)
    {
      return ((unsigned short)keyleft < (unsigned short)keyright)?-1:(((unsigned short)keyleft == (unsigned short)keyright)?0:1);
    }
  };

  template<typename T, class Traits, class Inner>
  class BSS_COMPILER_DLLEXPORT ComparePairTraits_first : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_pointer constptr;
    typedef typename Traits::pointer pointer;

    static inline char Compare(constref keyleft, constref keyright)
    {
      return Inner::Compare(keyleft.first, keyright.first);
    }
  };

  template<typename T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT CompareStringsTraits : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_pointer constptr;
    typedef typename Traits::pointer pointer;

    static inline char Compare(constref keyleft, constref keyright)
    {
      int result = strcmp(keyleft, keyright);
      return (result < 0)?-1:((result == 0)?0:1);
    }
  };

  template<typename T, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT CompareStringsTraitsNoCase : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_pointer constptr;
    typedef typename Traits::pointer pointer;

    static inline char Compare(constref keyleft, constref keyright)
    {
    int result = STRICMP(keyleft, keyright);
    return (result < 0)?-1:((result == 0)?0:1);
    }
  };
}

#endif