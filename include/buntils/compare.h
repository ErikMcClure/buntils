// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_COMPARE_H__
#define __BUN_COMPARE_H__

#include "defines.h"
#include <string.h>
#include <utility>
#include <compare>
#include <ranges>
#define SGNCOMPARE(left, right) (((left) > (right)) - ((left) < (right)))

namespace bun {
  template<class F, typename L, typename R = L, class Ord = std::partial_ordering>
  // We can't do the three_way_comparable_with check here because some comparisons extract comparable elements
  // from otherwise incomparable L and R types. Unfortunately this results in nonsensical invocable errors when
  // std::compare_three_way fails the three_way_comparable check.
  concept Comparison = /*std::three_way_comparable_with<L, R, Ord> &&*/ std::invocable<F&, L, R> &&
                       std::_Compares_as<std::invoke_result_t<F&, L, R>, Ord>;

  // Used to store the comparison object using EBO
  template<class T> class CompressedBase : private T
  {
  protected:
    constexpr BUN_FORCEINLINE CompressedBase()
      requires std::is_default_constructible_v<T>
    {}
    constexpr BUN_FORCEINLINE CompressedBase(const CompressedBase&) = default;
    constexpr BUN_FORCEINLINE CompressedBase(CompressedBase&&)      = default;
    constexpr BUN_FORCEINLINE CompressedBase(const T& s) : T(s) {}
    constexpr BUN_FORCEINLINE CompressedBase(T&& s) : T(std::move(s)) {}

    [[nodiscard]] constexpr BUN_FORCEINLINE T& _getbase() noexcept { return *this; }
    [[nodiscard]] constexpr BUN_FORCEINLINE const T& _getbase() const noexcept { return *this; }
  };

  // Inverts the normal three-way comparison by swapping the arguments
  struct inv_three_way
  {
    template<typename L, typename R>
      requires std::three_way_comparable_with<L, R>
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(L&& _Left, R&& _Right) const
      noexcept(noexcept(std::forward<R>(_Right) <=> std::forward<L>(_Left)))
    {
      return std::forward<R>(_Right) <=> std::forward<L>(_Left);
    }

    using is_transparent = int;
  };

  // Preserves the arguments as pointers and dereferences them before comparison
  struct indirect_three_way
  {
    template<typename L, typename R>
      requires std::three_way_comparable_with<L, R>
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const L* _Left, const R* _Right) const
      noexcept(noexcept(*_Left <=> *_Right))
    {
      return *_Left <=> *_Right;
    }

    using is_transparent = int;
  };

  // Assumes both arguments are pairs and does a comparison between the first elements
  template<typename L1, typename R1 = L1, Comparison<L1, R1> C = std::compare_three_way> struct first_three_way : protected C
  {
    first_three_way() = default;
    first_three_way(const C& c) : C(c) {}
    first_three_way(C&& c) : C(std::move(c)) {}
    first_three_way(const first_three_way&) = default;
    first_three_way(first_three_way&&)      = default;

    template<typename L2, typename R2>
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const std::pair<L1, L2>& l, const std::pair<R1, R2>& r) const
    {
      return _getf()(l.first, r.first);
    }

    [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }

    using is_transparent = int;
  };

  // Assumes both arguments are pairs and does a comparison between the second elements
  template<typename L2, typename R2 = L2, Comparison<L2, R2> C = std::compare_three_way> struct second_three_way : protected C
  {
    second_three_way() = default;
    second_three_way(const C& c) : C(c) {}
    second_three_way(C&& c) : C(std::move(c)) {}
    second_three_way(const second_three_way&) = default;
    second_three_way(second_three_way&&)      = default;

    template<typename L1, typename R1>
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const std::pair<L1, L2>& l, const std::pair<R1, R2>& r) const
    {
      return _getf()(l.second, r.second);
    }

    [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }

    using is_transparent = int;
  };

  // Assumes both arguments are pairs and compares both elements, in order
  template<typename L, typename R = L, Comparison<typename L::first_type, typename R::first_type> FC = std::compare_three_way,
           Comparison<typename L::second_type, typename R::second_type> SC = std::compare_three_way>
  struct BUN_EMPTY_BASES both_three_way : protected FC, protected SC
  {
    both_three_way()                      = default;
    both_three_way(const both_three_way&) = default;
    both_three_way(both_three_way&&)      = default;

    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const L& l, const R& r) const
    {
      auto c = _getf()(l.first, r.first);
      return (c == 0) ? _gets()(l.second, r.second) : c;
    }

    [[nodiscard]] constexpr BUN_FORCEINLINE const FC& _getf() const noexcept { return *this; }
    [[nodiscard]] constexpr BUN_FORCEINLINE const SC& _gets() const noexcept { return *this; }

    using is_transparent = int;
  };

  // Specialization to handle when LC == RC
  template<typename L, typename R,
           Comparison<typename L::first_type, typename R::first_type> C>
  struct BUN_EMPTY_BASES both_three_way<L, R, C, C> : protected C
  {
    both_three_way()                      = default;
    both_three_way(const both_three_way&) = default;
    both_three_way(both_three_way&&)      = default;

    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const L& l, const R& r) const
    {
      auto c = _getf()(l.first, r.first);
      return (c == 0) ? _gets()(l.second, r.second) : c;
    }

    [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }
    [[nodiscard]] constexpr BUN_FORCEINLINE const C& _gets() const noexcept { return *this; }

    using is_transparent = int;
  };

  // Assumes both arguments are tuples and does a comparison between the ith element
  template<typename L, typename R = L, size_t I = 0,
           Comparison<std::tuple_element_t<I, L>, std::tuple_element_t<I, R>> C = std::compare_three_way>
  struct tuple_three_way : protected C
  {
    tuple_three_way() = default;
    tuple_three_way(const C& c) : C(c) {}
    tuple_three_way(C&& c) : C(std::move(c)) {}
    tuple_three_way(const tuple_three_way&) = default;
    tuple_three_way(tuple_three_way&&)      = default;

    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(const L& l, const R& r) const
    {
      return _getf()(std::tuple_element_t<I, L>(std::get<I>(l)), std::tuple_element_t<I, R>(std::get<I>(r)));
    }

    [[nodiscard]] constexpr BUN_FORCEINLINE const C& _getf() const noexcept { return *this; }

    using is_transparent = int;
  };

  // Performs a three-way lexographic comparison between strings, either ascii or wide.
  struct string_three_way
  {
    template<typename L, typename R>
      requires(std::convertible_to<L, const char*> && std::convertible_to<R, const char*>) ||
              (std::convertible_to<L, char*> && std::convertible_to<R, char*>)
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(L&& l, R&& r) const noexcept
    {
      int result = strcmp(std::forward<L>(l), std::forward<R>(r));

      if(result > 0)
      {
        return std::strong_ordering::greater;
      }

      return (result < 0) ? std::strong_ordering::less : std::strong_ordering::equal;
    }

    template<typename L, typename R>
      requires(std::convertible_to<L, const wchar_t*> && std::convertible_to<R, const wchar_t*>) ||
              (std::convertible_to<L, wchar_t*> && std::convertible_to<R, wchar_t*>)
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(L&& l, R&& r) const noexcept
    {
      int result = wcscmp(std::forward<L>(l), std::forward<R>(r));

      if(result > 0)
      {
        return std::strong_ordering::greater;
      }

      return (result < 0) ? std::strong_ordering::less : std::strong_ordering::equal;
    }

    using is_transparent = int;
  };

  // Performs a case-insensitive three-way lexographic comparison between strings, either ascii or wide.
  struct string_three_way_insensitive
  {
    template<typename L, typename R>
      requires(std::convertible_to<L, const char*> && std::convertible_to<R, const char*>) ||
              (std::convertible_to<L, char*> && std::convertible_to<R, char*>)
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(L&& l, R&& r) const noexcept
    {
      int result = STRICMP(std::forward<L>(l), std::forward<R>(r));

      if(result > 0)
      {
        return std::strong_ordering::greater;
      }

      return (result < 0) ? std::strong_ordering::less : std::strong_ordering::equal;
    }

    template<typename L, typename R>
      requires(std::convertible_to<L, const wchar_t*> && std::convertible_to<R, const wchar_t*>) ||
              (std::convertible_to<L, wchar_t*> && std::convertible_to<R, wchar_t*>)
    [[nodiscard]] constexpr BUN_FORCEINLINE auto operator()(L&& l, R&& r) const noexcept
    {
      int result = WCSICMP(std::forward<L>(l), std::forward<R>(r));

      if(result > 0)
      {
        return std::strong_ordering::greater;
      }

      return (result < 0) ? std::strong_ordering::less : std::strong_ordering::equal;
    }

    using is_transparent = int;
  };

}

#endif
