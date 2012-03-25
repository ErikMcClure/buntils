// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COMPARE_H__
#define __BSS_COMPARE_H__

#include "bss_call.h"
#include "bss_deprecated.h"

#define SGNCOMPARE(left,right) (((left)>(right))-((left)<(right)))

namespace bss_util {
  template<typename T> // Returns -1,0,1
  inline char CompT(const T& left, const T& right) { return SGNCOMPARE(left,right); }

  template<typename T> // Returns -1,0,1
  inline char CompTInv(const T& left, const T& right) { return SGNCOMPARE(right,left); }

  template<typename T> // Returns 1 if l<r or 0 otherwise
  inline char CompT_LT(const T& left, const T& right) { return -(left<right); }

  template<typename T> // Returns 1 if l>r or 0 otherwise
  inline char CompT_GT(const T& left, const T& right) { return (left>right); }

  template<typename T> 
  inline char CompT_EQ(const T& left, const T& right) { return (left==right); }

  template<typename T> 
  inline char CompT_NEQ(const T& left, const T& right) { return (left!=right); }

  template<typename T, char (*CFunc)(const typename T::first_type&, const typename T::first_type&)>
  inline char CompTFirst(const T& left, const T& right) { return CFunc(left.first, right.first); }

  template<typename T, char (*CFunc)(const typename T::first_type&, const typename T::first_type&)>
  inline char CompTSecond(const T& left, const T& right) { return CFunc(left.second, right.second); }

  template<typename T>
  inline char CompStr(const T& left, const T& right)
  {
    int result = strcmp(left, right);
    return SGNCOMPARE(result,0);
  }
  template<typename T>
  inline char CompIStr(const T& left, const T& right)
  {
    int result = STRICMP(left, right);
    return SGNCOMPARE(result,0);
  }
  template<typename T>
  inline char CompStrW(const T& left, const T& right)
  {
    int result = wcscmp(left, right);
    return SGNCOMPARE(result,0);
  }
  template<typename T>
  inline char CompIStrW(const T& left, const T& right)
  {
    int result = WCSICMP(left, right);
    return SGNCOMPARE(result,0);
  }
}

#endif