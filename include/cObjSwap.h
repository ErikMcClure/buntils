// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_OBJSWAP_H__BSS__
#define __C_OBJSWAP_H__BSS__

#include "bss_deprecated.h"
#include "bss_dlldef.h"
#include "bss_traits.h"
#include "bss_compare.h"
#include <stdarg.h>
#include <string.h>

namespace bss_util {
  template<class T, class Traits> // Specializes to case-sensitive string matching
  struct OBJSWAPFUNC_ALL : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(constref left, constref right) { return CompT_EQ(left,right); } };
  template<class Traits> struct OBJSWAPFUNC_ALL<const char*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(const char* l, const char* r) { return !strcmp(l,r); } };
  template<class Traits> struct OBJSWAPFUNC_ALL<const wchar_t*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(const wchar_t* l, const wchar_t* r) { return !wcscmp(l,r); } };
  
  template<class T, class Traits> // Specializes to case-insensitive string matching
  struct OBJSWAPFUNC_INS : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(constref left, constref right) { return CompT_EQ(left,right); } };
  template<class Traits> struct OBJSWAPFUNC_INS<const char*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(const char* l, const char* r) { return !STRICMP(l,r); } };
  template<class Traits> struct OBJSWAPFUNC_INS<const wchar_t*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static BSS_FORCEINLINE char Comp(const wchar_t* l, const wchar_t* r) { return !WCSICMP(l,r); } };
  
  /* Generalized solution to allow dynamic switch statements */
  template<class T, class SWAP=OBJSWAPFUNC_ALL<T,ValueTraits<T>>>
  class BSS_COMPILER_DLLEXPORT cObjSwap : protected SWAP
  {
    typedef typename SWAP::constref constref;

  public:
	  inline explicit cObjSwap(constref src, int num, ...)
	  {
		  va_list vl;
		  va_start(vl, num);
		  _result = _compobj(src, num, vl);
		  va_end(vl);
	  }
    inline explicit cObjSwap(constref src, T* tarray, int num) : _result(CompareObjectsArray(src,num,tarray)) {}
    template<int N>
    inline explicit cObjSwap(constref src, const T (&tarray)[N]) : _result(CompareObjectsArray<N>(src,tarray)) {}

    inline BSS_FORCEINLINE operator int() const { return _result; }

	  inline static int CompareObjects(constref src, int num, ...)
	  {
		  va_list vl;
		  va_start(vl, num);
		  int retval = _compobj(src, num, vl);
		  va_end(vl);
		  return retval;
	  }

    template<int N>
    inline static BSS_FORCEINLINE int CompareObjectsArray(constref src, const T (&tarray)[N])
    {
      return CompareObjectsArray(src,N,tarray);
    }

    inline static BSS_FORCEINLINE int CompareObjectsArray(constref src, int num, const T* tarray)
    {
		  for(int i = 0; i < num; ++i)
			  if(SWAP::Comp(tarray[i], src))
				  return i;
		  return -1;
    }

  private:
	  inline static int _compobj(constref src, int num, va_list args)
	  {
		  for(int i = 0; i < num; ++i)
			  if(SWAP::Comp((va_arg(args, constref)), src))
				  return i;
		  return -1;
	  }

	  int _result;
  };

  typedef cObjSwap<const char*> STRSWAP;
  typedef cObjSwap<const wchar_t*> WCSSWAP;
  typedef cObjSwap<const void*> PSWAP;
}

#endif