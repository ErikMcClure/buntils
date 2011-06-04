// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COMPARE_H__
#define __BSS_COMPARE_H__

#include "bss_call.h"

namespace bss_util {
  template<class Key>
  char CompareKeys(const Key& keyleft, const Key& keyright)
  {
    return (keyleft < keyright)?-1:((keyleft == keyright)?0:1);
  }

  template<class Key>
  char CompareKeysInverse(const Key& keyleft, const Key& keyright)
  {
    return (keyleft < keyright)?1:((keyleft == keyright)?0:-1);
  }

  template<class Key>
  char CompareShorts(const Key& keyleft, const Key& keyright)
  {
    return ((unsigned short)keyleft < (unsigned short)keyright)?-1:(((unsigned short)keyleft == (unsigned short)keyright)?0:1);
  }

  template<class Key, class Inner, char (*CompareFunc)(const Inner& keyleft, const Inner& keyright)>
  char ComparePair_first(const Key& keyleft, const Key& keyright)
  {
    return CompareFunc(keyleft.first, keyright.first);
  }

  template<class Key, class Inner, char (*CompareFunc)(const Inner& keyleft, const Inner& keyright)>
  char ComparePair_second(const Key& keyleft, const Key& keyright)
  {
    return CompareFunc(keyleft.second, keyright.second);
  }

  template<class Key>
  char CompareStrings(const Key& keyleft, const Key& keyright)
  {
    int result = strcmp(keyleft, keyright);
    return (result < 0)?-1:((result == 0)?0:1);
  }
  template<class Key>
  char CompareStringsNoCase(const Key& keyleft, const Key& keyright)
  {
    int result = _stricmp(keyleft, keyright);
    return (result < 0)?-1:((result == 0)?0:1);
  }
}

#endif