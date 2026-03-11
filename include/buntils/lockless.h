// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __LOCKLESS_H__BUN__
#define __LOCKLESS_H__BUN__

#ifdef BUN_COMPILER_MSC
  #include <intrin.h>
#endif

#include "compiler.h"
#include <cassert>
#include <cstdint>
#include <intrin0.inl.h>

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

    /*
  #ifdef BUN_CPU_x86_64
    template<typename T>
    struct ASMCAS_REGPICK_WRITE<T,16>
    {
      inline static uint8_t BUN_FORCEINLINE asmcas(volatile T *dest, T newval, T oldval)
      {
        uint8_t rval;
        __asm {
          lea rsi,oldval;
          lea rdi,newval;
          mov rax,[rsi];
          mov rdx,8[rsi];
          mov rbx,[rdi];
          mov rcx,8[rdi];
          mov rsi,dest;
          //lock CMPXCHG16B [rsi] is equivalent to the following except that it's atomic:
          //ZeroFlag = (rdx:rax == *rsi);
          //if (ZeroFlag) *rsi = rcx:rbx;
          //else rdx:rax = *rsi;
          lock CMPXCHG16B [rsi];
          sete rval;
        }
        return rval;
      }
    };
  #endif
  #define ASMCAS_REGPICK_READ_MACRO(SIZE,REG_D,REG_A) template<typename T> \
    struct ASMCAS_REGPICK_READ<T,SIZE> \
    { \
      inline static uint8_t BUN_FORCEINLINE asmcas(volatile T *pval, T newval, T oldval) \
      { \
        ASMCAS_FSTMOV(REG_D) \
        __asm mov REG_A, oldval \
        __asm lock cmpxchg [BUNASM_PREG], REG_D \
      }; \
    }

    ASMCAS_REGPICK_READ_MACRO(1,DL,AL);
    ASMCAS_REGPICK_READ_MACRO(2,DX,AX);
    ASMCAS_REGPICK_READ_MACRO(4,EDX,EAX);
  #ifdef BUN_CPU_x86_64
    ASMCAS_REGPICK_READ_MACRO(8,RDX,RAX);
  #endif

  #ifdef BUN_CPU_x86
    template<typename T>
    struct ASMCAS_REGPICK_READ<T,8>
    {
    public:
      inline static BUN_FORCEINLINE T asmcas(volatile T *dest, T newval, T oldval)
      {
        __asm {
          lea esi,oldval;
          lea edi,newval;
          mov eax,[esi];
          mov edx,4[esi];
          mov ebx,[edi];
          mov ecx,4[edi];
          mov esi,dest;
          //lock CMPXCHG8B [esi] is equivalent to the following except that it's atomic:
          //ZeroFlag = (edx:eax == *esi);
          //if (ZeroFlag) *esi = ecx:ebx;
          //else edx:eax = *esi;
          lock CMPXCHG8B [esi];
        }
      }
    };
  #endif

  #ifdef BUN_CPU_x86_64
    template<typename T>
    struct ASMCAS_REGPICK_READ<T,16>
    {
    public:
      inline static T asmcas(volatile T *dest, T newval, T oldval)
      {
        __asm {
          lea rsi,oldval;
          lea rdi,newval;
          mov rax,[rsi];
          mov rdx,8[rsi];
          mov rbx,[rdi];
          mov rcx,8[rdi];
          mov rsi,dest;
          //lock CMPXCHG16B [rsi] is equivalent to the following except that it's atomic:
          //ZeroFlag = (rdx:rax == *rsi);
          //if (ZeroFlag) *rsi = rcx:rbx;
          //else rdx:rax = *rsi;
          lock CMPXCHG16B [rsi];
        }
      }
    };
  #endif

    // Assembly level 8-byte compare exchange operation. Compare with old value if you want to know if it succeeded
    inline int64_t asmcas8b(volatile int64_t *dest,int64_t newval,int64_t oldval)
    {
      //value returned in eax::edx
      __asm {
        lea esi,oldval;
        lea edi,newval;
        mov eax,[esi];
        mov edx,4[esi];
        mov ebx,[edi];
        mov ecx,4[edi];
        mov esi,dest;
        //lock CMPXCHG8B [esi] is equivalent to the following except
        //that it's atomic:
        //ZeroFlag = (edx:eax == *esi);
        //if (ZeroFlag) *esi = ecx:ebx;
        //else edx:eax = *esi;
        lock CMPXCHG8B [esi];
      }
    }*/
#endif
  // defined(BUN_CPU_x86_64) || defined(BUN_CPU_x86)
}

#endif
