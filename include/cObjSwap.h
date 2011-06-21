// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_OBJSWAP_H__BSS__
#define __C_OBJSWAP_H__BSS__

#include "bss_deprecated.h"
#include "bss_dlldef.h"
#include "bss_traits.h"
#include <stdarg.h>
#include <string.h>

namespace bss_util {
  template<typename T, class Traits=ValueTraits<T>>
  class __declspec(dllexport) CompBoolTraits : public Traits
  {
  public:
    typedef typename Traits::const_reference constref;
    static inline char Compare(constref keyleft, constref keyright) { return (keyleft == keyright)?0:1; }
  };

  /* Default ignores case */
  template<>
  class __declspec(dllexport) CompBoolTraits<char const*,ValueTraits<char const*>> : public ValueTraits<char const*>
  {
  public:
    typedef ValueTraits<char const*>::const_reference constref;
    static inline char Compare(constref keyleft, constref keyright) { return (STRICMP(keyleft,keyright)==0)?0:1; }
  };
  template<>
  class __declspec(dllexport) CompBoolTraits<wchar_t const*,ValueTraits<wchar_t const*>> : public ValueTraits<wchar_t const*>
  {
  public:
    typedef ValueTraits<wchar_t const*>::const_reference constref;
    static inline char Compare(constref keyleft, constref keyright) { return (WCSICMP(keyleft,keyright)==0)?0:1;  }
  };
  template<typename T> class __declspec(dllexport) CompCaseTraits {}; //causes compile error if an invalid typename is chosen
  template<> class __declspec(dllexport) CompCaseTraits<char const*> { public: static inline char Compare(char const* keyleft, char const* keyright) { return (strcmp(keyleft,keyright)==0)?0:1; } };
  template<> class __declspec(dllexport) CompCaseTraits<wchar_t const*> { public: static inline char Compare(wchar_t const* keyleft, wchar_t const* keyright) { return (wcscmp(keyleft,keyright)==0)?0:1; } };

  /* Generalized solution to allow dynamic switch statements */
  template<class T, class Comp=CompBoolTraits<T>>
  class __declspec(dllexport) cObjSwap 
  {
    typedef typename Comp::constref constref;
  public:
	  inline explicit cObjSwap(constref src, int num, ...)
	  {
		  va_list vl;
		  va_start(vl, num);
		  _result = _compobj(src, num, vl);
		  va_end(vl);
	  }

    inline operator int() const { return _result; }

	  inline static int CompareObjects(constref src, int num, ...)
	  {
		  va_list vl;
		  va_start(vl, num);
		  int retval = _compobj(src, num, vl);
		  va_end(vl);
		  return retval;
	  }

  private:
	  inline static int _compobj(constref src, int num, va_list args)
	  {
		  for(int i = 0; i < num; ++i)
			  if(Comp::Compare((va_arg(args, constref)), src)==0)
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
class __declspec(dllexport) cObjectSwap // Using the same idea as the class above, we can extend it into a template comparison function to compare any object in a switch statement
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