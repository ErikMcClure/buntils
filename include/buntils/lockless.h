// Copyright (c)2026 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __LOCKLESS_H__BUN__
#define __LOCKLESS_H__BUN__

#include "defines.h"
#include <assert.h>
#include <stddef.h>
#ifdef BUN_COMPILER_MSC
#include <intrin.h>
#endif
#include <atomic>

#ifdef BUN_CPU_x86
#define BUNASM_PREG ECX
#define BUNASM_PREGA EAX
#elif defined(BUN_CPU_x86_64)
#define BUNASM_PREG RCX
#define BUNASM_PREGA RAX
#endif

// These are all implemented using compiler intrinsics on MS compilers because the inline assembly does not behave well when inlined. GCC 
// intrinsics are not used due to inconsistent locking barriers, and the wide availability of standardized inline assembly for them.
namespace bun {
  template<typename T> struct bun_PTag { T* p; size_t tag; }; // Stores a pointer and a tag value

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
    __asm { /*lock*/ xchg Barrier, eax } //xchg locks itself if both operands are registers and blows up if you lock it anyway in a multiprocessor environment.
#elif defined(BUN_CPU_x86_64)
    __faststorefence();
#elif defined(BUN_CPU_IA_64)
    __mf();
#endif
#elif defined(BUN_COMPILER_GCC)
    int32_t Barrier = 0;
    __asm__ __volatile__("xchgl %%eax,%0 ":"=r" (Barrier));
#endif
  }
#pragma warning(pop)

  namespace internal {
    template<typename T, int size>
    struct ASMCAS_REGPICK_WRITE { };

#ifdef BUN_PLATFORM_WIN32
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 1> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange8((volatile char*)dest, *(char*)&newval, *(char*)&oldval) == *(char*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 2> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange16((volatile short*)dest, *(short*)&newval, *(short*)&oldval) == *(short*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 4> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange((volatile long*)dest, *(long*)&newval, *(long*)&oldval) == *(long*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 8> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange64((volatile int64_t*)dest, *(int64_t*)&newval, *(int64_t*)&oldval) == *(int64_t*)&oldval; }
    };
#ifdef BUN_64BIT
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 16> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval)
      {
        assert(!(((size_t)dest) % 16));
        return _InterlockedCompareExchange128((volatile int64_t*)dest, ((int64_t*)&newval)[1], ((int64_t*)&newval)[0], (int64_t*)&oldval);
      }
    };
#endif
#else
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 1> {
      BUN_FORCEINLINE static  uint8_t asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile char*)dest, *(char*)&oldval, *(char*)&newval) != 0; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 2> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile short*)dest, *(int16_t*)&oldval, *(int16_t*)&newval) != 0; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 4> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile long*)dest, *(int32_t*)&oldval, *(int32_t*)&newval) != 0; }
    };
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 8> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile long long*)dest, *(int64_t*)&oldval, *(int64_t*)&newval) != 0; }
    };
#ifdef BUN_64BIT
    template<typename T> struct ASMCAS_REGPICK_WRITE<T, 16> {
      BUN_FORCEINLINE static uint8_t asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile __int128*)dest, *(__int128*)&oldval, *(__int128*)&newval) != 0; }
    };
#endif
#endif

    template<typename T, int size>
    struct ASMCAS_REGPICK_READ { };

#ifdef BUN_PLATFORM_WIN32
    template<typename T> struct ASMCAS_REGPICK_READ<T, 1> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(char*)&retval = _InterlockedCompareExchange8((volatile char*)dest, *(char*)&newval, *(char*)&oldval); return *(char*)&retval == *(char*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 2> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(int16_t*)&retval = _InterlockedCompareExchange16((volatile short*)dest, *(int16_t*)&newval, *(int16_t*)&oldval); return *(int16_t*)&retval == *(int16_t*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 4> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(int32_t*)&retval = _InterlockedCompareExchange((volatile long*)dest, *(int32_t*)&newval, *(int32_t*)&oldval); return *(int32_t*)&retval == *(int32_t*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 8> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(int64_t*)&retval = _InterlockedCompareExchange64((volatile long long*)dest, *(int64_t*)&newval, *(int64_t*)&oldval); return *(int64_t*)&retval == *(int64_t*)&oldval; }
    };
#ifdef BUN_64BIT
    template<typename T> struct ASMCAS_REGPICK_READ<T, 16> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, [[maybe_unused]] T oldval, T& retval)
      {
        assert(!(((size_t)dest) % 16));
        char r = _InterlockedCompareExchange128((volatile int64_t*)dest, ((int64_t*)&newval)[1], ((int64_t*)&newval)[0], (int64_t*)&retval); return r != 0;
      }
    };
#endif
#else
    template<typename T> struct ASMCAS_REGPICK_READ<T, 1> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(char*)&retval = __sync_val_compare_and_swap((volatile char*)dest, *(char*)&oldval, *(char*)&newval); return *(char*)&retval == *(char*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 2> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(short*)&retval = __sync_val_compare_and_swap((volatile short*)dest, *(int16_t*)&oldval, *(int16_t*)&newval); return *(int16_t*)&retval == *(int16_t*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 4> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(long*)&retval = __sync_val_compare_and_swap((volatile long*)dest, *(int32_t*)&oldval, *(int32_t*)&newval); return *(int32_t*)&retval == *(int32_t*)&oldval; }
    };
    template<typename T> struct ASMCAS_REGPICK_READ<T, 8> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(long long*)&retval = __sync_val_compare_and_swap((volatile long long*)dest, *(int64_t*)&oldval, *(int64_t*)&newval); return *(int64_t*)&retval == *(int64_t*)&oldval; }
    };
#ifdef BUN_64BIT
    template<typename T> struct ASMCAS_REGPICK_READ<T, 16> {
      BUN_FORCEINLINE static bool asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int128*)&retval = __sync_val_compare_and_swap((volatile __int128*)dest, *(__int128*)&oldval, *(__int128*)&newval); return *(__int128*)&retval == *(__int128*)&oldval; }
    };
#endif
#endif

  }

  // Provides assembly level Compare and Exchange operation. Returns 1 if successful or 0 on failure. Implemented via
  // inline template specialization so that the proper assembly is generated for the type given. If a type is provided
  // that isn't exactly 1,2,4,or 8 bytes,the compiler will explode.
  template<typename T>
  inline uint8_t asmcas(volatile T *pval, T newval, T oldval)
  {
    return internal::ASMCAS_REGPICK_WRITE<T, sizeof(T)>::asmcas(pval, newval, oldval);
  }

  // Provides assembly level Compare and Exchange operation and sets retval to the previous value held by pval. Returns true if operation succeeded.
  template<typename T>
  inline bool asmcasr(volatile T *pval, T newval, T oldval, T& retval)
  {
    return internal::ASMCAS_REGPICK_READ<T, sizeof(T)>::asmcas(pval, newval, oldval, retval);
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
 //defined(BUN_CPU_x86_64) || defined(BUN_CPU_x86)
}

#endif
