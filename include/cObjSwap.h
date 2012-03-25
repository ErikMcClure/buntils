// Copyright ©2011 Black Sphere Studios
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
    inline static char Comp(constref left, constref right) { return CompT_EQ(left,right); } };
  template<class Traits> struct OBJSWAPFUNC_ALL<const char*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static char Comp(const char* l, const char* r) { return strcmp(l,r)!=0; } };
  template<class Traits> struct OBJSWAPFUNC_ALL<const wchar_t*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static char Comp(const wchar_t* l, const wchar_t* r) { return wcscmp(l,r)!=0; } };
  
  template<class T, class Traits> // Specializes to case-insensitive string matching
  struct OBJSWAPFUNC_INS : Traits { typedef typename Traits::const_reference constref;
    inline static char Comp(constref left, constref right) { return CompT_EQ(left,right); } };
  template<class Traits> struct OBJSWAPFUNC_INS<const char*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static char Comp(const char* l, const char* r) { return STRICMP(l,r)!=0; } };
  template<class Traits> struct OBJSWAPFUNC_INS<const wchar_t*,Traits> : Traits { typedef typename Traits::const_reference constref;
    inline static char Comp(const wchar_t* l, const wchar_t* r) { return WCSICMP(l,r)!=0; } };
  
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

    inline operator int() const { return _result; }

	  inline static int CompareObjects(constref src, int num, ...)
	  {
		  va_list vl;
		  va_start(vl, num);
		  int retval = _compobj(src, num, vl);
		  va_end(vl);
		  return retval;
	  }

    template<int N>
    inline static int CompareObjectsArray(constref src, const T (&tarray)[N])
    {
      return CompareObjectsArray(src,N,tarray);
    }

    inline static int CompareObjectsArray(constref src, int num, const T* tarray)
    {
		  for(int i = 0; i < num; ++i)
			  if(SWAP::Comp(tarray[i], src)==0)
				  return i;
		  return -1;
    }

  private:
	  inline static int _compobj(constref src, int num, va_list args)
	  {
		  for(int i = 0; i < num; ++i)
			  if(SWAP::Comp((va_arg(args, constref)), src)==0)
				  return i;
		  return -1;
	  }

	  int _result;
  };

  typedef cObjSwap<const char*> STRSWAP;
  typedef cObjSwap<const wchar_t*> WCSSWAP;
  typedef cObjSwap<const void*> PSWAP;
}

/*
template<int (*compfunc)(const char*,int,va_list)=&cStringSwap::_compstr>
class cStringSwap //I absolutely freaking hate not being able to do a switch statement between strings in C++, so this allows you to pass in an array of strings and will return the array index of whichever one it corresponds to, allowing a switch statement
{
public:
  cStringSwap(const char* src, int num, ...)
  {
    va_list vl;
    va_start(vl, num);
    _result = (*compfunc)(src, num, vl);
    va_end(vl);
  }

  operator int() const { return _result; }

  template<int (*compfunc)(const char*,int,va_list)>
  static int CompareStrings(const char* src,int num, ...)
  {
    va_list vl;
    va_start(vl, num);
    int retval = (*compfunc)(src, num, vl);
    va_end(vl);
    return retval;
  }

  static int _compstr(const char* src, int num, va_list args)
  {
    for(int i = 0; i < num; ++i)
      if(!STRICMP(va_arg(args, const char*), src))
        return i;
    return -1;
  }

  static int _compstrcase(const char* src, int num, va_list args)
  {
    for(int i = 0; i < num; ++i)
      if(!strcmp(va_arg(args, const char*), src))
        return i;
    return -1;
  }

protected:
  int _result;
}; //This is declared as a class for ease of use. The functionality can also be accessed via pure static function

typedef cStringSwap<&cStringSwap<0>::_compstr> SWAPSTR;
typedef cStringSwap<&cStringSwap<0>::_compstr> STRSWAP;
typedef cStringSwap<&cStringSwap<0>::_compstr> cStrSwap; 
typedef cStringSwap<&cStringSwap<0>::_compstr> SWITCHSTR;
typedef cStringSwap<&cStringSwap<0>::_compstrcase> SWAPSTR_CASE;
typedef cStringSwap<&cStringSwap<0>::_compstrcase> STRSWAP_CASE;
typedef cStringSwap<&cStringSwap<0>::_compstrcase> SWITCHSTR_CASE;

template<int (*compfunc)(const wchar_t*,int,va_list)=&cStringSwapW::_compstr>
class cStringSwapW //I absolutely freaking hate not being able to do a switch statement between strings in C++, so this allows you to pass in an array of strings and will return the array index of whichever one it corresponds to, allowing a switch statement
{
public:
  cStringSwapW(const wchar_t* src, int num, ...)
  {
    va_list vl;
    va_start(vl, num);
    _result = (*compfunc)(src, num, vl);
    va_end(vl);
  }

  operator int() const { return _result; }

  template<int (*compfunc)(const wchar_t*,int,va_list)>
  static int CompareStrings(const wchar_t* src,int num, ...)
  {
    va_list vl;
    va_start(vl, num);
    int retval = (*compfunc)(src, num, vl);
    va_end(vl);
    return retval;
  }

  static int _compstr(const wchar_t* src, int num, va_list args)
  {
    for(int i = 0; i < num; ++i)
      if(!WCSICMP(va_arg(args, const wchar_t*), src))
        return i;
    return -1;
  }
  static int _compstrcase(const wchar_t* src, int num, va_list args)
  {
    for(int i = 0; i < num; ++i)
      if(!wcscmp(va_arg(args, const wchar_t*), src))
        return i;
    return -1;
  }

protected:
  int _result;
};

typedef cStringSwapW<&cStringSwapW<0>::_compstr> SWAPSTRW;
typedef cStringSwapW<&cStringSwapW<0>::_compstr> cStrSwapW; 
typedef cStringSwapW<&cStringSwapW<0>::_compstr> SWITCHSTRW;
typedef cStringSwapW<&cStringSwapW<0>::_compstrcase> SWAPSTRW_CASE;
typedef cStringSwapW<&cStringSwapW<0>::_compstrcase> cStrSwapW_CASE; 
typedef cStringSwapW<&cStringSwapW<0>::_compstrcase> SWITCHSTRW_CASE;


template<class T, bool (*Compare)(const T& left, const T& right)=CompFunc<T>, class Traits=ValueTraits<T>>
class BSS_COMPILER_DLLEXPORT cObjectSwap // Using the same idea as the class above, we can extend it into a template comparison function to compare any object in a switch statement
{
public:
	cObjectSwap(T* src, int num, ...) //These are all assumed to be T* pointers. If you are comparing a series of pointers, that means you'll have pointers to pointers
	{
		va_list vl;
		va_start(vl, num);
		_result = _compobj(src, num, vl);
		va_end(vl);
	}

  operator int() const { return _result; }

	static int CompareObjects(T* src, int num, ...)
	{
		va_list vl;
		va_start(vl, num);
		int retval = _compobj(src, num, vl);
		va_end(vl);
		return retval;
	}

private:
	static int _compobj(T* src, int num, va_list args)
	{
		for(int i = 0; i < num; ++i)
			if(Compare(*(va_arg(args, T*)), *src))
				return i;
		return -1;
	}

	int _result;
};
*/

#endif