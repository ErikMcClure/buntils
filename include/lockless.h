// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LOCKLESS_H__BSS__
#define __LOCKLESS_H__BSS__

#include "bss_call.h"
#include <WinNT.h>

#ifdef BSS_CPU_x86
#define BSSASM_PREG ECX
#elif defined(BSS_CPU_x86_64)
#define BSSASM_PREG RCX
#endif

namespace bss_util {

#if defined(BSS_CPU_x86_64) || defined(BSS_CPU_x86)
#pragma warning(push)
#pragma warning(disable : 4793)
	BSS_COMPILER_FORCEINLINE void CPU_Barrier()
	{
		__int32 Barrier;
		__asm {
			lock xchg Barrier, eax
		}
	}
#pragma warning(pop)

  template<typename T>
  inline void BSS_COMPILER_FORCEINLINE BSS_FASTCALL atomic_inc(volatile T* p)
  {
      __asm
      {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov BSSASM_PREG, p
#endif
          lock inc ptr [BSSASM_PREG]
          ret
      }
  }

  template<typename T, int size>
  class ASMCAS_REGPICK_WRITE
  {
  };

  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,1>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      unsigned char rval;
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov DL, newval
        mov BSSASM_PREG, pval
#endif
        mov AL, oldval
        lock cmpxchg [BSSASM_PREG], DL
        sete rval // Note that sete sets a 'byte' not the word
      }
      return rval;
    }
  };

  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,2>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      unsigned char rval;
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov DX, newval
        mov BSSASM_PREG, pval
#endif
        mov AX, oldval
        lock cmpxchg [BSSASM_PREG], DX
        sete rval // Note that sete sets a 'byte' not the word
      }
      return rval;
    }
  };

  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,4>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      unsigned char rval;
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov EDX, newval
        mov BSSASM_PREG, pval
#endif
        mov EAX, oldval
        lock cmpxchg [BSSASM_PREG], EDX
        sete rval // Note that sete sets a 'byte' not the word
      }
      return rval;
    }
  };

#ifdef BSS_CPU_x86
  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,8>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
    {
      unsigned char rval;
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
        sete rval;
      } 
      return rval;
    }
  };
#endif
  
#ifdef BSS_CPU_x86_64
  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,8>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      unsigned char rval;
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov RDX, newval
        mov RCX, pval
#endif
        mov RAX, oldval
        lock cmpxchg [RCX], RDX
        sete rval // Note that sete sets a 'byte' not the word
      }
      return rval;
    }
  };
  
  template<typename T>
  class ASMCAS_REGPICK_WRITE<T,16>
  {
  public:
    inline static unsigned char BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
    {
      unsigned char rval;
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

  /* Provides assembly level Compare and Exchange operation. Returns 1 if successful or 0 on failure. Implemented via
  inline template specialization so that the proper assembly is generated for the type given. If a type is provided
  that isn't exactly 1,2,4,or 8 bytes,the compiler will explode. */
  template<typename T>
  inline unsigned char BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
  {
    return ASMCAS_REGPICK_WRITE<T,sizeof(T)>::asmcas(pval,newval,oldval);
  }

  template<typename T, int size>
  class ASMCAS_REGPICK_READ
  {
  };

  template<typename T>
  class ASMCAS_REGPICK_READ<T,1>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov DL, newval
        mov BSSASM_PREG, pval
#endif
        mov AL, oldval
        lock cmpxchg [BSSASM_PREG], DL
      }
    }
  };

  template<typename T>
  class ASMCAS_REGPICK_READ<T,2>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov DX, newval
        mov BSSASM_PREG, pval
#endif
        mov AX, oldval
        lock cmpxchg [BSSASM_PREG], DX
      }
    }
  };

  template<typename T>
  class ASMCAS_REGPICK_READ<T,4>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov EDX, newval
        mov BSSASM_PREG, pval
#endif
        mov EAX, oldval
        lock cmpxchg [BSSASM_PREG], EDX
      }
    }
  };

#ifdef BSS_CPU_x86
  template<typename T>
  class ASMCAS_REGPICK_READ<T,8>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
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

#ifdef BSS_CPU_x86_64
  template<typename T>
  class ASMCAS_REGPICK_READ<T,8>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *pval, T newval, T oldval)
    {
      __asm {
#ifdef BSS_NO_FASTCALL //if we are using fastcall we don't need these instructions
        mov RDX, newval
        mov RCX, pval
#endif
        mov RAX, oldval
        lock cmpxchg [RCX], RDX
      }
    }
  };
  
  template<typename T>
  class ASMCAS_REGPICK_READ<T,16>
  {
  public:
    inline static T BSS_FASTCALL asmcas(volatile T *dest, T newval, T oldval)
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

  /* Provides assembly level Compare and Exchange operation. Always returns the old value. */
  template<typename T>
  inline T BSS_FASTCALL rasmcas(volatile T *pval, T newval, T oldval)
  {
    return ASMCAS_REGPICK_READ<T,sizeof(T)>::asmcas(pval,newval,oldval);
  }

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
#endif //defined(BSS_CPU_x86_64) || defined(BSS_CPU_x86)

  /* This is a flip-flop that allows lockless two thread communication using the exchange property of CAS */
  template<typename T>
  class BSS_COMPILER_DLLEXPORT cLocklessFlipper
  {
  public:
    inline cLocklessFlipper(T* ptr1, T* ptr2) : _ptr1(ptr1), _ptr2(ptr2), _readnum(0), _curwrite(-1), _flag(0)
    {
    }
    /* Call this before you do any reading. Use the returned pointer. */
    inline T* StartRead()
    { 
      asmcas<unsigned char>(&_flag,1,_flag);
      return (T*)_ptr1;
    }
    /* Call this after you finish all reading - do not use the pointer again */
    inline void EndRead()
    {
      asmcas<unsigned char>(&_flag,0,_flag);
      asmcas<unsigned __int32>(&_readnum,_readnum+1,_readnum);
    }
    /* Call this before you do any writing. Use only the returned pointer */
    inline T* StartWrite()
    {
      if(_flag!=0 && _curwrite==_readnum) return 0; //We haven't finished a read since the last write
      return (T*)_ptr2;
    }
    /* Call this after you finish writing - do not use the pointer again */
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
  };
}

#endif