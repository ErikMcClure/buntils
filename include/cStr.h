// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __CSTRING_H__BSS__
#define __CSTRING_H__BSS__

#include <string>
#include <stdarg.h>
#include <vector>
#include "bss_deprecated.h"
//#include "cBucketAlloc.h"
#include "bss_alloc.h"
/*
namespace bss_util {
  static cBucketAlloc cstrpool_alloc[8];

	template<typename T, unsigned char index=0>
  class __declspec(dllexport) StringPoolPolicy : public AllocPolicySize<T> {
	public:
    template<typename U>
    struct rebind { typedef StringPoolPolicy<U,index> other; };

    inline explicit StringPoolPolicy(cBucketAlloc& ref) : _ref(ref) {}
    inline explicit StringPoolPolicy() : _ref(cstrpool_alloc[index]) {}
    inline ~StringPoolPolicy() {}
    inline explicit StringPoolPolicy(StringPoolPolicy const&) : _ref(cstrpool_alloc[index]) {}
    template <typename U>
    inline explicit StringPoolPolicy(StringPoolPolicy<U> const&) : _ref(cstrpool_alloc[index]) {}

    inline pointer allocate(std::size_t cnt, 
      typename std::allocator<void>::const_pointer = 0) {
        void* retval=_ref.alloc(cnt * sizeof (T));
        if(!retval)
          retval=malloc(cnt*sizeof(T));
        return reinterpret_cast<pointer>(retval); 

    }
    inline void deallocate(pointer p, std::size_t = 0) { 
      if(!_ref.dealloc(p))
        free(p);
    }

  protected:
    cBucketAlloc& _ref;
	};

	
#define DEFPOOLCOMPARE(num) template<typename T, typename T2> inline bool operator==(StringPoolPolicy<T,num> const&, StringPoolPolicy<T2,num> const&) { return true; }
	DEFPOOLCOMPARE(0)
	DEFPOOLCOMPARE(1)
	DEFPOOLCOMPARE(2)
	DEFPOOLCOMPARE(3)
	DEFPOOLCOMPARE(4)
	DEFPOOLCOMPARE(5)
	DEFPOOLCOMPARE(6)
	DEFPOOLCOMPARE(7)
	DEFPOOLCOMPARE(8)

  template<typename T, typename OtherAllocator> inline bool operator==(StringPoolPolicy<T> const&, OtherAllocator const&) { return false; }

}
*/
//#define CSTRALLOC(T) std::basic_string<T, std::char_traits<T>, bss_util::RefAllocator<Alloc,bss_util::RefHackAllocPolicy<T>, bss_util::ObjectTraits<T>>>
#define CSTRALLOC(T) std::basic_string<T, std::char_traits<T>, Alloc>

/* This uses a trick stolen from numeric_limits, where you can explicitely initialize entire classes. Thus, we create a blank slate
and then explicitely initialize it for char and wchar_t, thus allowing our cStrT class template to be pretty and easy to use */
template<class _Ty>
class CSTR_CT
{
};

template<>
class CSTR_CT<char>
{
public:
  typedef char CHAR;
  typedef wchar_t OTHER_C;

  static inline const CHAR* __cdecl SCHR(const CHAR* str, int val) { return strchr(str,val); }
  static inline size_t __cdecl SLEN(const CHAR* str) { return strlen(str); }
  static inline CHAR* __cdecl STOK(CHAR* str,const CHAR* delim, CHAR** context) { return strtok_s(str,delim,context); }
  static inline errno_t __cdecl WTOMB(size_t* outsize, CHAR* dest, size_t destsize, const OTHER_C* src, size_t maxcount) { return wcstombs_s(outsize,dest,destsize,src,maxcount); }
  static inline int __cdecl VPF(CHAR *dest, size_t size, const CHAR *format, va_list args) { return VSPRINTF(dest,size,format,args); }
  static inline int __cdecl VPCF(const CHAR* str, va_list args) { return _vscprintf(str,args); }

  static inline size_t __cdecl O_SLEN(const OTHER_C* str) { return wcslen(str); }
};

template<>
class CSTR_CT<wchar_t>
{
public:
  typedef wchar_t CHAR;
  typedef char OTHER_C;

  static inline const CHAR* __cdecl SCHR(const CHAR* str, int val) { return wcschr(str,val); }
  static inline size_t __cdecl SLEN(const CHAR* str) { return wcslen(str); }
  static inline CHAR* __cdecl STOK(CHAR* str,const CHAR* delim, CHAR** context) { return wcstok_s(str,delim,context); }
  static inline errno_t __cdecl WTOMB(size_t* outsize, CHAR* dest, size_t destsize, const OTHER_C* src, size_t maxcount) { return mbstowcs_s(outsize,dest,destsize,src,maxcount); }
  static inline int __cdecl VPF(CHAR *dest, size_t size, const CHAR *format, va_list args) { return VSWPRINTF(dest,size,format,args); }
  static inline int __cdecl VPCF(const CHAR* str, va_list args) { return _vscwprintf(str,args); }

  static inline size_t __cdecl O_SLEN(const OTHER_C* str) { return strlen(str); }
};

#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
//template<typename Alloc = bss_util::AllocatorPolicyRef<char, bss_util::StringPoolAllocPolicy<char>, bss_util::ObjectTraits<char>>>
//template<typename Alloc = bss_util::Allocator<CHAR,bss_util::StringPoolPolicy<CHAR>>>
template<typename T=char, typename Alloc=std::allocator<T>>
class __declspec(dllexport) cStrT : public CSTRALLOC(T)//If dllimport is used on this linker errors crop up. I have no idea why. Also, if the constructors aren't inlined, it causes heap corruption errors because the basic_string constructors are inlined
{ //Note that you can take off __declspec(dllexport) but you'll pay for it with even more unnecessary 4251 warnings.
  typedef typename CSTR_CT<T>::CHAR CHAR;
  typedef typename CSTR_CT<T>::OTHER_C OTHER_C;
public:
  explicit inline cStrT(size_t length = 1) : CSTRALLOC(CHAR)() { reserve(length); } //an implicit constructor here would be bad
  inline cStrT(const CSTRALLOC(CHAR)& copy) : CSTRALLOC(CHAR)(copy) {}
  inline cStrT(CSTRALLOC(CHAR)&& mov) : CSTRALLOC(CHAR)(std::move(mov)) {}
  inline cStrT(const cStrT& copy) : CSTRALLOC(CHAR)(copy) {}
  inline cStrT(cStrT&& mov) : CSTRALLOC(CHAR)(std::move(mov)) {}
  template<class U> inline cStrT(const cStrT<T,U>& copy) : CSTRALLOC(CHAR)(copy) {}
  template<class U> inline cStrT(const cStrT<OTHER_C,U>& copy) : CSTRALLOC(CHAR)() { size_t numchar=copy.size(); reserve(++numchar); CSTR_CT<T>::WTOMB(&_Mysize,_Myptr(),_Myres, copy.String(), numchar); RecalcSize(); }
  inline cStrT(const OTHER_C* text) : CSTRALLOC(CHAR)() { if(!text) return; size_t numchar=CSTR_CT<T>::O_SLEN(text); reserve(++numchar); CSTR_CT<T>::WTOMB(&_Mysize,_Myptr(),_Myres, text, numchar); RecalcSize(); }
  inline cStrT(unsigned short index, const CHAR* text, const CHAR delim) : CSTRALLOC(CHAR)() //Creates a new string from the specified chunk
  {
    for(unsigned short i = 0; i < index; ++i)
    {
      if(text) text = CSTR_CT<T>::SCHR(text,(int)delim);  
      if(text) text++;
    }
    if(!text) return;
    const CHAR* end = CSTR_CT<T>::SCHR(text,(int)delim);
    if(!end) end = &text[CSTR_CT<T>::SLEN(text)];
    
    size_t _length = end-text;
    reserve(++_length);
    insert(0, text,_length-1);
  }
  inline cStrT(const CHAR* string, ...) : CSTRALLOC(CHAR)()
  {
    if(!string);
      //do nothing
    else if(CSTR_CT<T>::SCHR(string, '%')==0) //Do we even need to check our va_list?
      CSTRALLOC(CHAR)::operator =(string);
    else
    {
      va_list vl;
      va_start(vl,string);
      size_t _length = (size_t)CSTR_CT<T>::VPCF(string,vl);
      reserve(++_length);
      CSTR_CT<T>::VPF(_Myptr(), _Myres, string, vl);
      _Mysize = CSTR_CT<T>::SLEN(_Myptr());
    }
  }

  inline operator const CHAR*() const { return _Myptr(); }
  
  //inline const cStrT operator +(const cStrT* right) const { return cStrT(*this,right); }
  inline const cStrT operator +(const cStrT& right) const { return cStrT(*this)+=right; }
  template<class U> inline const cStrT operator +(const cStrT<T,U>& right) const { return cStrT(*this)+=right; }
  inline const cStrT operator +(const CHAR* right) const { return cStrT(*this)+=right; }
  inline const cStrT operator +(const CHAR right) const { return cStrT(*this)+=right; }

  //inline cStrT& operator =(const cStrT* right) { CSTRALLOC(CHAR)::operator =(*right); return *this; } 
  inline cStrT& operator =(const cStrT& right) { CSTRALLOC(CHAR)::operator =(right); return *this; }
  template<class U> inline cStrT& operator =(const cStrT<T,U>& right) { CSTRALLOC(CHAR)::operator =(right); return *this; }
  inline cStrT& operator =(const CHAR* right) { if(right != 0) CSTRALLOC(CHAR)::operator =(right); return *this; }
  inline cStrT& operator =(const CHAR right) { CSTRALLOC(CHAR)::operator =(right); return *this; } //resize(2); _Myptr()[0] = right; _Myptr()[1] = '\0'; return *this; } 
  inline cStrT& operator =(cStrT&& right) { CSTRALLOC(CHAR)::operator =(std::move(right)); return *this; }

  //inline cStrT& operator +=(const cStrT* right) { if(!_Mysize) return CSTRALLOC(CHAR)::operator =(*right); CSTRALLOC(CHAR)::operator +=(*right); return *this; }
  inline cStrT& operator +=(const cStrT& right) { CSTRALLOC(CHAR)::operator +=(right); return *this; }
  template<class U> inline cStrT& operator +=(const cStrT<T,U>& right) { CSTRALLOC(CHAR)::operator +=(right); return *this; }
  inline cStrT& operator +=(const CHAR* right) { if(right != 0 && right != _Myptr()) CSTRALLOC(CHAR)::operator +=(right); return *this; }
  inline cStrT& operator +=(const CHAR right) { CSTRALLOC(CHAR)::operator +=(right); return *this; }
  
  inline CHAR* UnsafeString() { return _Myptr(); } //This is potentially dangerous if the string is modified
  inline const CHAR* String() const { return _Myptr(); }
  inline CHAR& GetChar(size_t index) { return CSTRALLOC(CHAR)::operator[](index); }
  inline cStrT& ReplaceChar(CHAR search, CHAR replace) { CHAR* pmod=_Myptr(); for(size_t i = 0; i < _Mysize; ++i) if(pmod[i] == search) pmod[i] = replace; return *this; }
  inline void RecalcSize() { _Mysize = CSTR_CT<T>::SLEN(_Myptr()); }
  inline void SetSize(size_t nsize) { _Mysize=nsize; }
  inline cStrT Trim() const { cStrT r(*this); r=_ltrim(_rtrim(r._Myptr(),r._Mysize)); return r; }

  static inline void Explode(std::vector<cStrT> &dest, const CHAR delim, const CHAR* text)
  {
    cStrT copy(text);
    CHAR delimhold[2] = { delim, 0 };
    CHAR* hold;
    CHAR* res = CSTR_CT<T>::STOK(copy.UnsafeString(), delimhold, &hold);

    while(res)
    {
      dest.push_back(res);
      res = CSTR_CT<T>::STOK(NULL,delimhold,&hold);
    }
  }
  static inline std::vector<cStrT> Explode(const CHAR delim, const CHAR* text) { std::vector<cStrT> r; Explode(r,delim,text); return r; }
  static inline cStrT StripChar(const CHAR* text, const CHAR c)
  { 
    cStrT r(CSTR_CT<T>::SLEN(text)+1);
    unsigned int i;
    for(i=0;*text!=0;++text)
      if(*text>32)
        r.UnsafeString()[i++]=*text;
    r.UnsafeString()[i]=0;
    r.RecalcSize();
    return r;
  }

private:
  inline static T* BSS_FASTCALL _ltrim(T* str) { for(;*str>0 && *str<33;++str); return str; }
  inline static T* BSS_FASTCALL _rtrim(T* str, size_t size) { T* inter=str+size; for(;inter>str && *inter<33;--inter); *(++inter)=0; return str; }
  //The following line of code is in such a twisted state because it must overload the operator[] inherent in the basic_string class and render it totally unusable so as to force the compiler to disregard it as a possibility, otherwise it gets confused with the CHAR* type conversion
  void __CLR_OR_THIS_CALL operator[](std::allocator<CHAR>& f) { }//return CSTRALLOC(CHAR)::operator [](_Off); }

};
#pragma warning(pop)

typedef cStrT<wchar_t,std::allocator<wchar_t>> cStrW;
typedef cStrT<char,std::allocator<char>> cStr;

#ifdef _UNICODE
typedef cStrW TStr;
#else
typedef cStr TStr;
#endif

// This allows cStr to inherit all the string operations
template<class _Elem, class _Alloc> // return NTCS + string
inline cStrT<_Elem,_Alloc> operator+(const _Elem *_Left,const cStrT<_Elem,_Alloc>& _Right)
	{	cStrT<_Elem,_Alloc> _Ans(_Traits::length(_Left) + _Right.size()); _Ans += _Left; return (_Ans += _Right); }
template<class _Elem, class _Alloc> // return character + string
inline cStrT<_Elem,_Alloc> operator+(const _Elem _Left,const cStrT<_Elem,_Alloc>& _Right)
	{	cStrT<_Elem,_Alloc> _Ans(1+_Right.size()); _Ans += _Left; return (_Ans += _Right); }
template<class _Elem, class _Alloc> // return string + string
inline cStrT<_Elem,_Alloc> operator+(const cStrT<_Elem,_Alloc>& _Left,cStrT<_Elem,_Alloc>&& _Right) 
	{	return (_STD move(_Right.insert(0, _Left))); }
template<class _Elem, class _Alloc> // return string + string
inline cStrT<_Elem,_Alloc> operator+(cStrT<_Elem,_Alloc>&& _Left,const cStrT<_Elem,_Alloc>& _Right)
	{	return (_STD move(_Left.append(_Right))); }
template<class _Elem, class _Alloc> // return NTCS + string
inline cStrT<_Elem,_Alloc> operator+(const _Elem *_Left,cStrT<_Elem,_Alloc>&& _Right)
	{	return (_STD move(_Right.insert(0, _Left))); }
template<class _Elem, class _Alloc> // return character + string
inline cStrT<_Elem,_Alloc> operator+(const _Elem _Left,cStrT<_Elem,_Alloc>&& _Right)
	{	return (_STD move(_Right.insert(0, 1, _Left))); }
template<class _Elem, class _Alloc> // return string + NTCS
inline cStrT<_Elem,_Alloc> operator+(cStrT<_Elem,_Alloc>&& _Left,const _Elem *_Right)
	{	return (_STD move(_Left.append(_Right)));	}
template<class _Elem, class _Alloc> // return string + character
inline cStrT<_Elem,_Alloc> operator+(cStrT<_Elem,_Alloc>&& _Left,const _Elem _Right)
	{	return (_STD move(_Left.append(1, _Right))); }
template<class _Elem, class _Alloc> // return string + string
inline cStrT<_Elem,_Alloc> operator+(cStrT<_Elem,_Alloc>&& _Left,cStrT<_Elem,_Alloc>&& _Right)
	{	
	if (_Right.size() <= _Left.capacity() - _Left.size()
		|| _Right.capacity() - _Right.size() < _Left.size())
		return (_STD move(_Left.append(_Right)));
	else
		return (_STD move(_Right.insert(0, _Left)));
	}

#endif