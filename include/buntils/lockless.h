// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __LOCKLESS_H__BUN__
#define __LOCKLESS_H__BUN__

#ifdef BUN_COMPILER_MSC
  #include <intrin.h>
  #include <intrin0.inl.h>
#endif

#include "compiler.h"
#include <cassert>
#include <cstdint>

#ifdef BUN_CPU_x86
  #define BUNASM_PREG  ECX
  #define BUNASM_PREGA EAX
#elif defined(BUN_CPU_x86_64)
  #define BUNASM_PREG  RCX
  #define BUNASM_PREGA RAX
#endif

// These are all implemented using compiler intrinsics on MS compilers because the inline assembly does not behave well when
// inlined. GCC intrinsics are not used due to inconsistent locking barriers, and the wide availability of standardized
// inline assembly for them.
namespace bun {
  template<typename T> union bun_PTag
  {
    struct
    {
      T* p;
      size_t tag;
    };
#ifdef BUN_64BIT
  #ifdef BUN_PLATFORM_WIN32
    long long i[2];
  #else
    __int128 i;
  #endif
#else
    int64_t i;
#endif
  }; // Stores a pointer and a tag value

#if defined(BUN_CPU_x86_64) || defined(BUN_CPU_x86)
  #pragma warning(push)
  #pragma warning(disable : 4793)

  // Enforces a CPU barrier to prevent any reordering attempts
  BUN_FORCEINLINE void CPU_Barrier()
  {
  #ifdef BUN_COMPILER_CLANG
    __sync_synchronize();
  #elif defined(BUN_COMPILER_MSC)
    #ifdef BUN_CPU_x86
    int32_t Barrier;
    __asm { /*lock*/ xchg Barrier, eax }
    // xchg locks itself if both operands are registers and blows up if you lock it anyway in a multiprocessor environment.
    #elif defined(BUN_CPU_x86_64)
    __faststorefence();
    #elif defined(BUN_CPU_IA_64)
    __mf();
    #endif
  #elif defined(BUN_COMPILER_GCC)
    int32_t Barrier = 0;
    __asm__ __volatile__("xchgl %%eax,%0 " : "=r"(Barrier));
  #endif
  }
  #pragma warning(pop)

  template<typename T>
  BUN_FORCEINLINE bool asmcasr(volatile bun_PTag<T>* dest, bun_PTag<T> newval, bun_PTag<T> oldval, bun_PTag<T>& retval)
  {
  #ifdef BUN_PLATFORM_WIN32
    #ifdef BUN_64BIT
    char r = _InterlockedCompareExchange128((volatile long long*)dest, newval.i[1], newval.i[0], (long long*)&retval);
    return r != 0;
    #else
    retval.i = _InterlockedCompareExchange64((volatile long long*)dest, newval.i, retval.i);
    return retval.i == oldval.i;
    #endif
  #else
    retval.i = __sync_val_compare_and_swap(&dest->i, oldval.i, newval.i);
    return retval.i == oldval.i;
  #endif
  }

#endif
}

#endif
