// Copyright �2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LOCKLESS_H__BSS__
#define __LOCKLESS_H__BSS__

#include "bss_defines.h"
#ifdef BSS_COMPILER_MSC
#include <intrin.h>
#endif

#ifdef BSS_CPU_x86
#define BSSASM_PREG ECX
#define BSSASM_PREGA EAX
#elif defined(BSS_CPU_x86_64)
#define BSSASM_PREG RCX
#define BSSASM_PREGA RAX
#endif

// These are all implemented using compiler intrinsics on MS compilers because the inline assembly does not behave well when inlined. GCC 
// intrinsics are not used due to inconsistent locking barriers, and the wide availability of standardized inline assembly for them.
namespace bss_util {
  template<typename T> struct bss_PTag { T* p; size_t tag; }; // Stores a pointer and a tag value
  
#if defined(BSS_CPU_x86_64) || defined(BSS_CPU_x86)
#pragma warning(push)
#pragma warning(disable : 4793)

  // Enforces a CPU barrier to prevent any reordering attempts
	BSS_FORCEINLINE static void CPU_Barrier()
	{
#ifdef BSS_COMPILER_MSC
#ifdef BSS_CPU_x86
		__int32 Barrier;
		__asm { /*lock*/ xchg Barrier, eax } //xchg locks itself if both operands are registers and blows up if you lock it anyway in a multiprocessor environment.
#elif defined(BSS_CPU_x86_64)
    __faststorefence();
#elif defined(BSS_CPU_IA_64)
    __mf();
#endif
#elif defined(BSS_COMPILER_GCC)
    __int32 Barrier = 0;
    __asm__ __volatile__("xchgl %%eax,%0 ":"=r" (Barrier));
#endif
	}
#pragma warning(pop)

  template<typename T, int size>
  struct ATOMIC_XADDPICK { };
  
#ifdef BSS_COMPILER_GCC
#define ATOMIC_XADDPICK_MACRO(INST,SIZE)  template<typename T> \
  struct ATOMIC_XADDPICK<T,SIZE> \
  { \
    static BSS_FORCEINLINE T BSS_FASTCALL atomic_xadd(volatile T* p, T val) \
    { \
        __asm__ __volatile__("lock " INST " %0,%1" :"+r" (val),"+m" (*p): : "memory"); \
        return val; \
    } \
  }

  ATOMIC_XADDPICK_MACRO("xaddb",1);
  ATOMIC_XADDPICK_MACRO("xaddw",2);
  ATOMIC_XADDPICK_MACRO("xaddl",4);
#ifdef BSS_CPU_x86_64
  ATOMIC_XADDPICK_MACRO("xaddq",8);
#endif
#elif defined(BSS_COMPILER_MSC)
  template<typename T> struct ATOMIC_XADDPICK<T,1> { BSS_FORCEINLINE static T BSS_FASTCALL atomic_xadd(volatile T* p, T v) { return (T)_InterlockedExchangeAdd8((volatile char*)p,v); } };
  template<typename T> struct ATOMIC_XADDPICK<T,2> { BSS_FORCEINLINE static T BSS_FASTCALL atomic_xadd(volatile T* p, T v) { return (T)_InterlockedExchangeAdd16((volatile short*)p,v); } };
  template<typename T> struct ATOMIC_XADDPICK<T,4> { BSS_FORCEINLINE static T BSS_FASTCALL atomic_xadd(volatile T* p, T v) { return (T)_InterlockedExchangeAdd((volatile long*)p,v); } };
  template<typename T> struct ATOMIC_XADDPICK<T,8> { BSS_FORCEINLINE static T BSS_FASTCALL atomic_xadd(volatile T* p, T v) { return (T)_InterlockedExchangeAdd64((volatile __int64*)p,v); } };
#endif
 //#ifdef BSS_COMPILER_GCC
  
  template<typename T> // This performs an atomic addition, and returns the value of the variable BEFORE the addition. This is faster if p is 32-bit aligned.
  BSS_FORCEINLINE T BSS_FASTCALL atomic_xadd(volatile T* p, T val=1)
  { 
    return ATOMIC_XADDPICK<T,sizeof(T)>::atomic_xadd(p,val);
  }

  template<typename T, int size>
  struct ATOMIC_XCHGPICK { };
  
#ifdef BSS_COMPILER_GCC
#define ATOMIC_XCHGPICK_MACRO(INST,SIZE,TYPE)  template<typename T> \
  struct ATOMIC_XCHGPICK<T,SIZE> \
  { \
    static BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* ptr, T x) \
    { \
		__asm__ __volatile__(INST " %0,%1" :"=r" ((TYPE)x) :"m" (*(volatile TYPE*)ptr), "0" ((TYPE)x) :"memory"); \
		return x; \
    } \
  }

  ATOMIC_XCHGPICK_MACRO("xchgb",1,unsigned char);
  ATOMIC_XCHGPICK_MACRO("xchgw",2,unsigned short);
  ATOMIC_XCHGPICK_MACRO("xchgl",4,unsigned int);
#ifdef BSS_CPU_x86_64
  ATOMIC_XCHGPICK_MACRO("xchgq",8,unsigned long long);
#endif
#elif defined(BSS_COMPILER_MSC)
  template<typename T> struct ATOMIC_XCHGPICK<T,1> { static BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* p, T val) { return (T)_InterlockedExchange8((volatile char*)p,(char)val); } };
  template<typename T> struct ATOMIC_XCHGPICK<T,2> { static BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* p, T val) { return (T)_InterlockedExchange16((volatile short*)p,(short)val); } };
  template<typename T> struct ATOMIC_XCHGPICK<T,4> { static BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* p, T val) { return (T)_InterlockedExchange((volatile long*)p,(long)val); } };
  template<typename T> struct ATOMIC_XCHGPICK<T,8> { static BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* p, T val) { return (T)_InterlockedExchange64((volatile __int64*)p,(__int64)val); } };
#endif
 //#ifdef BSS_COMPILER_GCC

  template<typename T> // This performs an atomic exchange, and returns the old value of *p. This is faster if p is 32-bit aligned.
  BSS_FORCEINLINE T BSS_FASTCALL atomic_xchg(volatile T* p, T val)
  { 
    return ATOMIC_XCHGPICK<T,sizeof(T)>::atomic_xchg(p,val);
  }

  template<typename T, int size>
  struct ASMCAS_REGPICK_WRITE { };
  
#ifdef BSS_COMPILER_GCC
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,1> { 
    BSS_FORCEINLINE static  unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile char*)dest,*(__int8*)&oldval, *(__int8*)&newval)!=0; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,2> { 
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile short*)dest,*(__int16*)&oldval, *(__int16*)&newval)!=0; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,4> { 
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile long*)dest,*(__int32*)&oldval, *(__int32*)&newval)!=0; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,8> { 
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile long long*)dest,*(__int64*)&oldval, *(__int64*)&newval)!=0; } 
  };
#ifdef BSS_64BIT
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,16> { 
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return __sync_bool_compare_and_swap((volatile __int128*)dest,*(__int128*)&oldval, *(__int128*)&newval)!=0; } 
  };
#endif
#else
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,1> {
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange8((volatile char*)dest,*(char*)&newval, *(char*)&oldval)==*(char*)&oldval; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,2> {
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange16((volatile short*)dest,*(short*)&newval, *(short*)&oldval)==*(short*)&oldval; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,4> {
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange((volatile long*)dest,*(long*)&newval, *(long*)&oldval)==*(long*)&oldval; } 
  };
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,8> {
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { return _InterlockedCompareExchange64((volatile __int64*)dest,*(__int64*)&newval, *(__int64*)&oldval)==*(__int64*)&oldval; } 
  };
#ifdef BSS_64BIT
  template<typename T> struct ASMCAS_REGPICK_WRITE<T,16> {
    BSS_FORCEINLINE static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval) { assert(!(((size_t)dest)%16));
    return _InterlockedCompareExchange128((volatile __int64*)dest,((__int64*)&newval)[1],((__int64*)&newval)[0], (__int64*)&oldval); } 
  };
#endif
#endif

  // Provides assembly level Compare and Exchange operation. Returns 1 if successful or 0 on failure. Implemented via
  // inline template specialization so that the proper assembly is generated for the type given. If a type is provided
  // that isn't exactly 1,2,4,or 8 bytes,the compiler will explode.
  template<typename T>
  inline unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
  {
    return ASMCAS_REGPICK_WRITE<T,sizeof(T)>::asmcas(pval,newval,oldval);
  }

  template<typename T, int size>
  struct ASMCAS_REGPICK_READ { };

#ifdef BSS_COMPILER_GCC
  template<typename T> struct ASMCAS_REGPICK_READ<T,1> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(char*)&retval = __sync_val_compare_and_swap((volatile char*)dest, *(__int8*)&oldval, *(__int8*)&newval); return *(__int8*)&retval==*(__int8*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,2> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(short*)&retval = __sync_val_compare_and_swap((volatile short*)dest, *(__int16*)&oldval, *(__int16*)&newval); return *(__int16*)&retval==*(__int16*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,4> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(long*)&retval = __sync_val_compare_and_swap((volatile long*)dest, *(__int32*)&oldval, *(__int32*)&newval); return *(__int32*)&retval==*(__int32*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,8> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(long long*)&retval = __sync_val_compare_and_swap((volatile long long*)dest, *(__int64*)&oldval, *(__int64*)&newval); return *(__int64*)&retval==*(__int64*)&oldval; }
  };
#ifdef BSS_64BIT
  template<typename T> struct ASMCAS_REGPICK_READ<T,16> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int128*)&retval = __sync_val_compare_and_swap((volatile __int128*)dest, *(__int128*)&oldval, *(__int128*)&newval); return *(__int128*)&retval==*(__int128*)&oldval; }
  };
#endif
#else
  template<typename T> struct ASMCAS_REGPICK_READ<T,1> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int8*)&retval = _InterlockedCompareExchange8((volatile char*)dest, *(__int8*)&newval, *(__int8*)&oldval); return *(__int8*)&retval==*(__int8*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,2> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int16*)&retval = _InterlockedCompareExchange16((volatile short*)dest, *(__int16*)&newval, *(__int16*)&oldval); return *(__int16*)&retval==*(__int16*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,4> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int32*)&retval = _InterlockedCompareExchange((volatile long*)dest, *(__int32*)&newval, *(__int32*)&oldval); return *(__int32*)&retval==*(__int32*)&oldval; }
  };
  template<typename T> struct ASMCAS_REGPICK_READ<T,8> { 
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) { *(__int64*)&retval = _InterlockedCompareExchange64((volatile long long*)dest, *(__int64*)&newval, *(__int64*)&oldval); return *(__int64*)&retval==*(__int64*)&oldval; }
  };
#ifdef BSS_64BIT
  template<typename T> struct ASMCAS_REGPICK_READ<T,16> {
    BSS_FORCEINLINE static bool BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval, T& retval) {
      assert(!(((size_t)dest)%16));
      char r = _InterlockedCompareExchange128((volatile __int64*)dest, ((__int64*)&newval)[1], ((__int64*)&newval)[0], (__int64*)&retval); return r!=0;
    } 
  };
#endif
#endif

  // Provides assembly level Compare and Exchange operation and sets retval to the previous value held by pval. Returns true if operation succeeded.
  template<typename T>
  inline bool BSS_FASTCALL asmcasr(volatile T *pval, T newval, T oldval, T& retval)
  {
    return ASMCAS_REGPICK_READ<T,sizeof(T)>::asmcas(pval,newval,oldval,retval);
  }
    
//  
//#ifdef BSS_CPU_x86_64
//  template<typename T>
//  struct ASMCAS_REGPICK_WRITE<T,16>
//  {
//    inline static unsigned char BSS_FORCEINLINE BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
//    {
//      unsigned char rval;
//      __asm { 
//        lea rsi,oldval; 
//        lea rdi,newval; 
//        mov rax,[rsi]; 
//        mov rdx,8[rsi]; 
//        mov rbx,[rdi]; 
//        mov rcx,8[rdi]; 
//        mov rsi,dest; 
//        //lock CMPXCHG16B [rsi] is equivalent to the following except that it's atomic:
//        //ZeroFlag = (rdx:rax == *rsi); 
//        //if (ZeroFlag) *rsi = rcx:rbx; 
//        //else rdx:rax = *rsi; 
//        lock CMPXCHG16B [rsi];
//        sete rval;
//      } 
//      return rval;
//    }
//  };
//#endif
//#define ASMCAS_REGPICK_READ_MACRO(SIZE,REG_D,REG_A) template<typename T> \
//  struct ASMCAS_REGPICK_READ<T,SIZE> \
//  { \
//    inline static unsigned char BSS_FORCEINLINE BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval) \
//    { \
//      ASMCAS_FSTMOV(REG_D) \
//      __asm mov REG_A, oldval \
//      __asm lock cmpxchg [BSSASM_PREG], REG_D \
//    }; \
//  }
//  
//  ASMCAS_REGPICK_READ_MACRO(1,DL,AL);
//  ASMCAS_REGPICK_READ_MACRO(2,DX,AX);
//  ASMCAS_REGPICK_READ_MACRO(4,EDX,EAX);
//#ifdef BSS_CPU_x86_64
//  ASMCAS_REGPICK_READ_MACRO(8,RDX,RAX);
//#endif
//
//#ifdef BSS_CPU_x86
//  template<typename T>
//  struct ASMCAS_REGPICK_READ<T,8>
//  {
//  public:
//    inline static BSS_FORCEINLINE T BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
//    {
//      __asm { 
//        lea esi,oldval; 
//        lea edi,newval; 
//        mov eax,[esi]; 
//        mov edx,4[esi]; 
//        mov ebx,[edi]; 
//        mov ecx,4[edi]; 
//        mov esi,dest; 
//        //lock CMPXCHG8B [esi] is equivalent to the following except that it's atomic: 
//        //ZeroFlag = (edx:eax == *esi); 
//        //if (ZeroFlag) *esi = ecx:ebx; 
//        //else edx:eax = *esi; 
//        lock CMPXCHG8B [esi];
//      } 
//    }
//  };
//#endif
//
//#ifdef BSS_CPU_x86_64
//  template<typename T>
//  struct ASMCAS_REGPICK_READ<T,16>
//  {
//  public:
//    inline static T BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
//    {
//      __asm { 
//        lea rsi,oldval; 
//        lea rdi,newval; 
//        mov rax,[rsi]; 
//        mov rdx,8[rsi]; 
//        mov rbx,[rdi]; 
//        mov rcx,8[rdi]; 
//        mov rsi,dest; 
//        //lock CMPXCHG16B [rsi] is equivalent to the following except that it's atomic: 
//        //ZeroFlag = (rdx:rax == *rsi); 
//        //if (ZeroFlag) *rsi = rcx:rbx; 
//        //else rdx:rax = *rsi; 
//        lock CMPXCHG16B [rsi];
//      } 
//    }
//  };
//#endif

  /* Assembly level 8-byte compare exchange operation. Compare with old value if you want to know if it succeeded */
  //inline __int64 asmcas8b(volatile __int64 *dest,__int64 newval,__int64 oldval) 
  //{ 
  //  //value returned in eax::edx 
  //  __asm { 
  //    lea esi,oldval; 
  //    lea edi,newval; 
  //    mov eax,[esi]; 
  //    mov edx,4[esi]; 
  //    mov ebx,[edi]; 
  //    mov ecx,4[edi]; 
  //    mov esi,dest; 
  //    //lock CMPXCHG8B [esi] is equivalent to the following except 
  //    //that it's atomic: 
  //    //ZeroFlag = (edx:eax == *esi); 
  //    //if (ZeroFlag) *esi = ecx:ebx; 
  //    //else edx:eax = *esi; 
  //    lock CMPXCHG8B [esi]; 
  //  } 
  //}
#endif
 //defined(BSS_CPU_x86_64) || defined(BSS_CPU_x86)

  /* This is a flip-flop that allows lockless two thread communication using the exchange property of CAS */
  /*template<typename T>
  struct BSS_COMPILER_DLLEXPORT cLocklessFlipper
  {
  public:
    inline cLocklessFlipper(T* ptr1, T* ptr2) : _ptr1(ptr1), _ptr2(ptr2), _readnum(0), _curwrite(-1), _flag(0)
    {
    }
    // Call this before you do any reading. Use the returned pointer.
    inline T* StartRead()
    { 
      asmcas<unsigned char>(&_flag,1,_flag);
      return (T*)_ptr1;
    }
    // Call this after you finish all reading - do not use the pointer again
    inline void EndRead()
    {
      asmcas<unsigned char>(&_flag,0,_flag);
      asmcas<unsigned __int32>(&_readnum,_readnum+1,_readnum);
    }
    // Call this before you do any writing. Use only the returned pointer
    inline T* StartWrite()
    {
      if(_flag!=0 && _curwrite==_readnum) return 0; //We haven't finished a read since the last write
      return (T*)_ptr2;
    }
    // Call this after you finish writing - do not use the pointer again
    inline void EndWrite()
    {
      //asmcas<unsigned __int32>(&_curwrite,_readnum,_curwrite);
      volatile unsigned __int32 vread = _readnum; //this allows us to read the _readnum BEFORE we check the flag
      if(_flag!=0) _curwrite=vread;
      volatile T* swap = _ptr1;
      asmcas<volatile T*>(&_ptr1,_ptr2,_ptr1);
      _ptr2=swap;
    }

  protected:
    volatile T* _ptr1;
    volatile T* _ptr2;
    volatile unsigned __int32 _curwrite;
    volatile unsigned __int32 _readnum;
    volatile unsigned char _flag;
  };*/
}

#endif
