// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_ALLOC_H__
#define __BUN_ALLOC_H__

#include "defines.h"
#include <memory>
#include <malloc.h> // Must be included because GCC is weird
#include <assert.h>
#include <string.h>
#include <span>
#include <algorithm>

namespace bun {
  // Align should be a power of two for platform independence.
  inline void* aligned_realloc(void* p, size_t size, size_t align)
  {
#ifdef BUN_PLATFORM_WIN32
    return _aligned_realloc(p, size, align);
#else
    void* n = aligned_alloc(align, size);
    if(p)
    {
      size_t old = malloc_usable_size(p);
      memcpy(n, p, bun_min(old, size));
      free(p);
    }
    return n;
#endif
  }

  BUN_FORCEINLINE static constexpr size_t AlignSize(size_t sz, size_t align) { return ((sz / align) + ((sz % align) != 0))*align; }

  // An implementation of a standard allocator, with optional alignment
  template<typename T, int ALIGN = 0>
  class BUN_COMPILER_DLLEXPORT StandardAllocator {
#ifdef BUN_DEBUG
    union SIZETRACK {
      size_t sz;
      char align[!ALIGN ? 1 : ALIGN];
    };
#endif

  public:
    using value_type = T;
    template <class U> struct rebind { typedef StandardAllocator<U, ALIGN> other; };
    StandardAllocator() = default;
    template <class U> constexpr StandardAllocator(const StandardAllocator<U>&) noexcept {}

    inline T* allocate(size_t cnt) { return reallocate(cnt, nullptr, 0); }
    inline T* reallocate(size_t cnt, T* p, size_t oldsize)
    {
#ifdef BUN_DEBUG
      static_assert(sizeof(SIZETRACK) == bun_max(sizeof(size_t), ALIGN), "SIZETRACK has unexpected padding");
      if(SIZETRACK* r = reinterpret_cast<SIZETRACK*>(p))
      {
        assert(!oldsize || r[-1].sz == (oldsize * sizeof(T)));
        p = reinterpret_cast<T*>(r - 1);
#ifdef BUN_COMPILER_MSC
        if constexpr(ALIGN > alignof(void*))
          assert(oldsize > 0 && oldsize <= _aligned_msize(p, ALIGN, 0));
        else
          assert(oldsize > 0 && oldsize <= _msize(p));
#endif
      }
      SIZETRACK* r;
      if constexpr(ALIGN > alignof(void*)) // realloc/malloc always return pointer-size aligned memory anyway
        r = reinterpret_cast<SIZETRACK*>(aligned_realloc(p, (cnt * sizeof(T)) + sizeof(SIZETRACK), ALIGN));
      else
        r = reinterpret_cast<SIZETRACK*>(realloc(p, (cnt * sizeof(T)) + sizeof(SIZETRACK)));
      assert(r != 0);
      r->sz = cnt * sizeof(T);
      return reinterpret_cast<T*>(r + 1);
#else
      if constexpr(ALIGN > alignof(void*)) // realloc/malloc always return pointer-size aligned memory anyway
        return reinterpret_cast<T*>(aligned_realloc(p, cnt * sizeof(T), ALIGN));
      else
        return reinterpret_cast<T*>(realloc(p, cnt * sizeof(T)));
#endif
    }
    inline void deallocate(T* p, size_t sz) noexcept
    {
#ifdef BUN_DEBUG
      SIZETRACK* r = reinterpret_cast<SIZETRACK*>(p);
      assert(!sz || r[-1].sz == (sz * sizeof(T)));
      p = reinterpret_cast<T*>(r - 1);
#endif
      assert(p != 0);
      if constexpr(ALIGN > alignof(void*))
        ALIGNEDFREE(p);
      else
        free(p);
    }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;
  };

  template<typename T>
  struct BUN_COMPILER_DLLEXPORT AlignedAllocator : StandardAllocator<T, alignof(T)>
  { 
    AlignedAllocator() = default;
    template <class U> constexpr AlignedAllocator(const AlignedAllocator<U>&) noexcept {}
  };

  // Implementation of a null allocation policy. Doesn't free anything, fails all allocations
  template<typename T>
  struct BUN_COMPILER_DLLEXPORT NullAllocator {
    using value_type = T;
    NullAllocator() = default;
    template <class U> constexpr NullAllocator(const NullAllocator<U>&) noexcept {}

    value_type* allocate(size_t cnt) { return nullptr; }
    inline value_type* reallocate(size_t n, T* p, size_t oldsize) { return nullptr; }
    void deallocate(value_type* p, size_t) noexcept {}

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;
  }; 

  template<template <typename> class T, typename U>
  concept PolicyProvider = requires(T<U> v, U* p) {
    { v.allocate((std::size_t)1, p, (std::size_t)1) } -> std::convertible_to<U*>;
    v.deallocate(p, (std::size_t)1);
  };

  // Allocator that uses an external policy to perform allocations
  template<typename T, template <typename> class Policy> requires PolicyProvider<Policy, T>
  class PolicyAllocator
  {
  public:
    using value_type = T;
    using policy_type = Policy<T>;
    template <class U> struct rebind {typedef PolicyAllocator<U, Policy> other;};

    explicit PolicyAllocator(policy_type& policy) noexcept : _policy(&policy) {}
    template <class U> PolicyAllocator(PolicyAllocator<U, Policy> const& other) noexcept : _policy(other._policy) {}
    template <class U> PolicyAllocator(PolicyAllocator<U, Policy>&& other) noexcept : _policy(std::move(other).move_policy()) { }

    value_type* allocate(std::size_t n)
    {
      return _policy->allocate(n, nullptr, 0);
    }

    inline value_type* reallocate(size_t n, T* p, size_t oldsize)
    {
      return _policy->allocate(n, p, oldsize);
    }

    void deallocate(value_type* p, std::size_t sz) noexcept
    {
      _policy->deallocate(p, sz);
    }

    inline policy_type* move_policy() && noexcept { auto p = _policy; _policy = nullptr; return p; }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    PolicyAllocator select_on_container_copy_construction() const noexcept { return *this; }

  protected:
    policy_type* _policy;
  };

  template <class T, class U, template <typename> class PT, template <typename> class PU>
  bool operator==(PolicyAllocator<T, PT> const& a, PolicyAllocator<U, PU> const& b) noexcept
  {
    if constexpr (std::is_same_v<PT, PU>)
    {
      return a._policy == b._policy;
    }
    else 
    {
      return false;
    }
  }

  template<class A> requires std::is_trivially_copyable_v<typename A::value_type>
  inline std::span<typename A::value_type> standard_realloc(A& allocator, size_t n, std::span<typename A::value_type> old)
  {
    using T = A::value_type;
    constexpr bool has_realloc = requires(const A& a, int n, T* oldptr, size_t oldsize)
    {
      a.reallocate(n, oldptr, oldsize);
    };

    if constexpr (has_realloc)
      return std::span(allocator.reallocate(n, old.data(), old.size()), n);
    else
    {
      auto p = std::allocator_traits<A>::allocate(allocator, n);
      if (old.data() != nullptr && old.size() > 0)
      {
        MEMCPY(p, n * sizeof(T), old.data(), std::min(old.size(), n) * sizeof(T));
        std::allocator_traits<A>::deallocate(allocator, old.data(), old.size());
      }
      return std::span(p, n);
    }
  }
}

#endif
