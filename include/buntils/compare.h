// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_COMPARE_H__
#define __BUN_COMPARE_H__

#include "defines.h"
#include <string.h>
#include <utility>

#define SGNCOMPARE(left,right) (((left)>(right))-((left)<(right)))
#define PRICOMPARE(left,right,p) (SGNCOMPARE(left,right)<<p)

namespace bun {
  template<typename T> // Returns -1,0,1 if l<r,l==r,l>r, respectively
  BUN_FORCEINLINE char CompT(const T& left, const T& right) noexcept { return SGNCOMPARE(left, right); }

  template<typename T> // Returns -1,0,1 if l<r,l==r,l>r, respectively
  BUN_FORCEINLINE char CompTInv(const T& left, const T& right) noexcept { return SGNCOMPARE(right, left); }

  template<typename T> // Returns -1 if l<r or 0 otherwise
  BUN_FORCEINLINE char CompT_LT(const T& left, const T& right) noexcept { return -(left < right); }

  template<typename T> // Returns 1 if l>r or 0 otherwise
  BUN_FORCEINLINE char CompT_GT(const T& left, const T& right) noexcept { return (left > right); }

  template<typename T>
  BUN_FORCEINLINE bool CompT_EQ(const T& left, const T& right) noexcept { return (left == right); }

  template<typename T>
  BUN_FORCEINLINE bool CompT_NEQ(const T& left, const T& right) noexcept { return (left != right); }

  template<typename T1, typename T2, char(*CompF)(const T1&, const T1&) = &CompT<T1>>
  BUN_FORCEINLINE char CompTFirst(const std::pair<T1,T2>& left, const std::pair<T1, T2>& right) noexcept { return CompF(left.first, right.first); }

  template<typename T1, typename T2, char(*CompS)(const T2&, const T2&) = &CompT<T2>>
  BUN_FORCEINLINE char CompTSecond(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept { return CompS(left.second, right.second); }

  template<typename T1, typename T2, char(*CompF)(const T1&, const T1&) = &CompT<T1>, char(*CompS)(const T2&, const T2&) = &CompT<T2>>
  BUN_FORCEINLINE char CompTBoth(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept { char c = CompF(left.first, right.first); return !c ? CompS(left.second, right.second) : c; }

  template<typename T, int I = 0, char(*CompF)(const std::tuple_element_t<I, T>&, const std::tuple_element_t<I, T>&) = &CompT<std::tuple_element_t<I, T>>>
  BUN_FORCEINLINE char CompTuple(const T& left, const T& right) { return CompF(std::get<I>(left), std::get<I>(right)); }

  template<typename T>
  BUN_FORCEINLINE char CompStr(const T& left, const T& right)
  {
    int result = strcmp(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BUN_FORCEINLINE char CompIStr(const T& left, const T& right)
  {
    int result = STRICMP(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BUN_FORCEINLINE bool CompStrLT(const T& left, const T& right) { return strcmp(left, right) < 0; }
  template<typename T>
  BUN_FORCEINLINE bool CompIStrLT(const T& left, const T& right) { return STRICMP(left, right) < 0; }
  template<typename T>
  BUN_FORCEINLINE char CompStrW(const T& left, const T& right)
  {
    int result = wcscmp(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BUN_FORCEINLINE char CompIStrW(const T& left, const T& right)
  {
    int result = WCSICMP(left, right);
    return SGNCOMPARE(result, 0);
  }
}

#endif
