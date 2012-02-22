// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COMPARE_H__
#define __BSS_COMPARE_H__

#include "bss_call.h"

#define SGNCOMPARE(left,right) (((left)>(right))-((left)<(right)))

namespace bss_util {
  template<class T>
  struct COMPARE
  {
    static inline char sgn(const T& l, const T& r) { return SGNCOMPARE(l,r); }
    static inline bool lt(const T& l, const T& r) { return (l)<(r); }
    static inline bool gt(const T& l, const T& r) { return (l)>(r); }
    static inline bool lte(const T& l, const T& r) { return (l)<=(r); }
    static inline bool gte(const T& l, const T& r) { return (l)>=(r); }
  };
  template<class T>
  struct COMPAREINV
  {
    static inline char sgn(const T& l, const T& r) { return SGNCOMPARE(r,l); }
    static inline bool lt(const T& l, const T& r) { return (r)<(l); }
    static inline bool gt(const T& l, const T& r) { return (r)>(l); }
    static inline bool lte(const T& l, const T& r) { return (r)<=(l); }
    static inline bool gte(const T& l, const T& r) { return (r)>=(l); }
  };

  template<class Key>
  inline char CompareKeys(const Key& keyleft, const Key& keyright)
  {
    return SGNCOMPARE(keyleft,keyright);
    //return (keyleft < keyright)?-1:((keyleft == keyright)?0:1);
  }

  template<class Key>
  inline char CompareKeysInverse(const Key& keyleft, const Key& keyright)
  {
    return SGNCOMPARE(keyright,keyleft);
    //return (keyleft < keyright)?1:((keyleft == keyright)?0:-1);
  }

  template<class Key>
  inline char CompareShorts(const Key& keyleft, const Key& keyright)
  {
    return SGNCOMPARE((unsigned short)keyleft,(unsigned short)keyright);
    //return ((unsigned short)keyleft < (unsigned short)keyright)?-1:(((unsigned short)keyleft == (unsigned short)keyright)?0:1);
  }

  template<class Key, class Inner, char (*CompareFunc)(const Inner& keyleft, const Inner& keyright)>
  inline char ComparePair_first(const Key& keyleft, const Key& keyright)
  {
    return CompareFunc(keyleft.first, keyright.first);
  }

  template<class Key, class Inner, char (*CompareFunc)(const Inner& keyleft, const Inner& keyright)>
  inline char ComparePair_second(const Key& keyleft, const Key& keyright)
  {
    return CompareFunc(keyleft.second, keyright.second);
  }

  template<class Key>
  inline char CompareStrings(const Key& keyleft, const Key& keyright)
  {
    int result = strcmp(keyleft, keyright);
    return SGNCOMPARE(result,0);
    //return (result < 0)?-1:((result == 0)?0:1);
  }
  template<class Key>
  inline char CompareStringsNoCase(const Key& keyleft, const Key& keyright)
  {
    int result = _stricmp(keyleft, keyright);
    return SGNCOMPARE(result,0);
    //return (result < 0)?-1:((result == 0)?0:1);
  }
}

#endif