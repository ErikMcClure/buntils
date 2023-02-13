// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_ALLOC_H__
#define __BSS_ALLOC_H__

#include "defines.h"
#include <memory>
#include <malloc.h> // Must be included because GCC is weird
#include <assert.h>
#include <string.h>

namespace bss {
  // Align should be a power of two for platform independence.
  inline void* aligned_realloc(void* p, size_t size, size_t align)
  {
#ifdef BSS_PLATFORM_WIN32
    return _aligned_realloc(p, size, align);
#else
    void* n = aligned_alloc(align, size);
    if(p)
    {
      size_t old = malloc_usable_size(p);
      memcpy(n, p, bssmin(old, size));
      free(p);
    }
    return n;
#endif
  }

  BSS_FORCEINLINE static constexpr size_t AlignSize(size_t sz, size_t align) { return ((sz / align) + ((sz % align) != 0))*align; }
  
  // An implementation of a standard allocator, with optional alignment
  template<typename T, int ALIGN = 0>
  class BSS_COMPILER_DLLEXPORT StandardAllocator {
    union SIZETRACK {
      size_t sz;
      char align[!ALIGN ? 1 : ALIGN];
    };

  public:
    using value_type = T;
    using policy_type = void;
    template<class U> using rebind = StandardAllocator<U, ALIGN>;
    StandardAllocator() = default;
    template <class U> constexpr StandardAllocator(const StandardAllocator<U>&) noexcept {}

    inline T* allocate(size_t cnt, T* p = nullptr, size_t old = 0) noexcept
    {
#ifdef BSS_DEBUG
      static_assert(sizeof(SIZETRACK) == bssmax(sizeof(size_t), ALIGN), "SIZETRACK has unexpected padding");
      if(SIZETRACK* r = reinterpret_cast<SIZETRACK*>(p))
      {
        assert(!old || r[-1].sz == (old * sizeof(T)));
        p = reinterpret_cast<T*>(r - 1);
#ifdef BSS_COMPILER_MSC
        if constexpr(ALIGN > alignof(void*))
          assert(old > 0 && old <= _aligned_msize(p, ALIGN, 0));
        else
          assert(old > 0 && old <= _msize(p));
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
    inline void deallocate(T* p, size_t sz = 0) noexcept
    {
#ifdef BSS_DEBUG
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
  };
  template<typename T>
  struct BSS_COMPILER_DLLEXPORT AlignedAllocator : StandardAllocator<T, alignof(T)>
  { 
    AlignedAllocator() = default;
    template <class U> constexpr AlignedAllocator(const AlignedAllocator<U>&) noexcept {}
  };

  // Implementation of a null allocation policy. Doesn't free anything, always returns 0 on all allocations.
  template<typename T>
  struct BSS_COMPILER_DLLEXPORT NullAllocator {
    using value_type = T;
    using policy_type = void;
    template<class U> using rebind = NullAllocator<U>;
    NullAllocator() = default;
    template <class U> constexpr NullAllocator(const NullAllocator<U>&) noexcept {}

    inline T* allocate(size_t cnt, T* p = nullptr, size_t old = 0) noexcept { return nullptr; }
    inline void deallocate(T* p, size_t = 0) noexcept {}
  };

  // Modified implementation of polymorphic allocator without virtual functions and with copy assignment.
  template<typename T, template <typename> class Policy>
  struct PolymorphicAllocator
  {
    using value_type = T;
    using policy_type = Policy<T>;
    template<class U> using rebind = PolymorphicAllocator<U, Policy>;
    PolymorphicAllocator() noexcept : _policy(DefaultPolicy()) {}
    PolymorphicAllocator(const PolymorphicAllocator&) = default;
    PolymorphicAllocator(PolymorphicAllocator&& mov) : _policy(mov._policy) { mov._policy = 0; }
    template <class U> constexpr PolymorphicAllocator(const PolymorphicAllocator<U, Policy>& copy) noexcept : _policy(copy._policy) {}
    explicit PolymorphicAllocator(policy_type* p) noexcept : _policy(p) {}
    inline T* allocate(size_t cnt, T* p = nullptr, size_t old = 0) noexcept { return _policy->allocate(cnt, p, old); }
    inline void deallocate(T* p, size_t sz = 0) noexcept { _policy->deallocate(p, sz); }

    PolymorphicAllocator& operator=(const PolymorphicAllocator&) = default;
    PolymorphicAllocator& operator=(PolymorphicAllocator&& mov) 
    {
      _policy = mov._policy; 
      mov._policy = 0;
      return *this;
    }

    static policy_type* DefaultPolicy() noexcept {
      static policy_type policy;
      return &policy;
    }

  protected:
    policy_type* _policy;
  };
}

#endif
