// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COMPARE_H__
#define __BSS_COMPARE_H__

#include "defines.h"
#include <string.h>
#include <utility>

#define SGNCOMPARE(left,right) (((left)>(right))-((left)<(right)))
#define PRICOMPARE(left,right,p) (SGNCOMPARE(left,right)<<p)

namespace bss {
  template<typename T> // Returns -1,0,1 if l<r,l==r,l>r, respectively
  BSS_FORCEINLINE char CompT(const T& left, const T& right) noexcept { return SGNCOMPARE(left, right); }

  template<typename T> // Returns -1,0,1 if l<r,l==r,l>r, respectively
  BSS_FORCEINLINE char CompTInv(const T& left, const T& right) noexcept { return SGNCOMPARE(right, left); }

  template<typename T> // Returns -1 if l<r or 0 otherwise
  BSS_FORCEINLINE char CompT_LT(const T& left, const T& right) noexcept { return -(left < right); }

  template<typename T> // Returns 1 if l>r or 0 otherwise
  BSS_FORCEINLINE char CompT_GT(const T& left, const T& right) noexcept { return (left > right); }

  template<typename T>
  BSS_FORCEINLINE bool CompT_EQ(const T& left, const T& right) noexcept { return (left == right); }

  template<typename T>
  BSS_FORCEINLINE bool CompT_NEQ(const T& left, const T& right) noexcept { return (left != right); }

  template<typename T1, typename T2, char(*CompF)(const T1&, const T1&) = &CompT<T1>>
  BSS_FORCEINLINE char CompTFirst(const std::pair<T1,T2>& left, const std::pair<T1, T2>& right) noexcept { return CompF(left.first, right.first); }

  template<typename T1, typename T2, char(*CompS)(const T2&, const T2&) = &CompT<T2>>
  BSS_FORCEINLINE char CompTSecond(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept { return CompS(left.second, right.second); }

  template<typename T1, typename T2, char(*CompF)(const T1&, const T1&) = &CompT<T1>, char(*CompS)(const T2&, const T2&) = &CompT<T2>>
  BSS_FORCEINLINE char CompTBoth(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept { char c = CompF(left.first, right.first); return !c ? CompS(left.second, right.second) : c; }

  template<typename T, int I = 0, char(*CompF)(const std::tuple_element_t<I, T>&, const std::tuple_element_t<I, T>&) = &CompT<std::tuple_element_t<I, T>>>
  BSS_FORCEINLINE char CompTuple(const T& left, const T& right) { return CompF(std::get<I>(left), std::get<I>(right)); }

  template<typename I, typename... Args>
  BSS_FORCEINLINE char CompAllIndice(const std::tuple<Args...>& left, const std::tuple<Args...>& right)
  {
    char c = CompT(std::get<sizeof...(Args) - I - 1>(left), std::get<sizeof...(Args)-I - 1>(right));
    if constexpr(I > 0)
    {
      if(!c)
        return CompAllIndice<I - 1, Args...>(left, right);
    }
    return c;
  }

  template<typename... Args>
  BSS_FORCEINLINE char CompAll(const std::tuple<Args...>& left, const std::tuple<Args...>& right) { return CompAllIndice(left, right); }
  
  template<typename T>
  BSS_FORCEINLINE char CompStr(const T& left, const T& right)
  {
    int result = strcmp(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BSS_FORCEINLINE char CompIStr(const T& left, const T& right)
  {
    int result = STRICMP(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BSS_FORCEINLINE bool CompStrLT(const T& left, const T& right) { return strcmp(left, right) < 0; }
  template<typename T>
  BSS_FORCEINLINE bool CompIStrLT(const T& left, const T& right) { return STRICMP(left, right) < 0; }
  template<typename T>
  BSS_FORCEINLINE char CompStrW(const T& left, const T& right)
  {
    int result = wcscmp(left, right);
    return SGNCOMPARE(result, 0);
  }
  template<typename T>
  BSS_FORCEINLINE char CompIStrW(const T& left, const T& right)
  {
    int result = WCSICMP(left, right);
    return SGNCOMPARE(result, 0);
  }
}

#endif
