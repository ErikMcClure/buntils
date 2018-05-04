// Copyright ©2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STR_H__BSS__
#define __STR_H__BSS__

#include <string>
#include <string.h>
#include <stdarg.h>
#include <vector>
#include <assert.h>
#include <stddef.h>
#include "utf_conv.h"
#include "defines.h"
#ifdef BSS_COMPILER_GCC
#include <stdio.h>
#endif

namespace bss {
  namespace internal {
  /* This is a static class that holds specializations for char and wchar_t string operations */
    template<class _Ty> class STR_CT {};

    template<>
    class STR_CT<char>
    {
    public:
      typedef char CHAR;
      typedef wchar_t OTHER_C;
      typedef char32_t OTHER_C2;

      static BSS_FORCEINLINE const CHAR* SCHR(const CHAR* str, int val) { return strchr(str, val); }
      static BSS_FORCEINLINE size_t SLEN(const CHAR* str) { return strlen(str); }
      static BSS_FORCEINLINE CHAR* STOK(CHAR* str, const CHAR* delim, CHAR** context) { return STRTOK(str, delim, context); }
      static BSS_FORCEINLINE int VPF(CHAR *dest, size_t size, const CHAR *format, va_list args) { return VSNPRINTF(dest, size, format, args); }
      static BSS_FORCEINLINE int VPCF(const CHAR* str, va_list args) { return VSCPRINTF(str, args); }
      static BSS_FORCEINLINE size_t CONV(const OTHER_C* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF16toUTF8(src, srclen, dest, len); }
      static BSS_FORCEINLINE size_t CONV2(const OTHER_C2* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF32toUTF8(src, srclen, dest, len); }

      static BSS_FORCEINLINE size_t O_SLEN(const OTHER_C* str) { return wcslen(str); }
      static BSS_FORCEINLINE const CHAR* STREMPTY() { return ""; }
    };

    template<>
    class STR_CT<wchar_t>
    {
    public:
      typedef wchar_t CHAR;
      typedef char OTHER_C;
      typedef char32_t OTHER_C2;

      static BSS_FORCEINLINE const CHAR* SCHR(const CHAR* str, CHAR val) { return wcschr(str, val); }
      static BSS_FORCEINLINE size_t SLEN(const CHAR* str) { return wcslen(str); }
      static BSS_FORCEINLINE CHAR* STOK(CHAR* str, const CHAR* delim, CHAR** context) { return WCSTOK(str, delim, context); }
#ifdef BSS_COMPILER_GCC
  //template<class S> static BSS_FORCEINLINE int VPF(S* str,const CHAR *format, va_list args)
  //{
  //  str->resize(SLEN(format));
  //  while(VSNWPRINTF(str->UnsafeString(),str->capacity(),format,args)==str->capacity()) //double size until it fits.
  //    str->resize(str->capacity()*2);
  //  return str->capacity();
  //}
#else
      static BSS_FORCEINLINE int VPF(CHAR *dest, size_t size, const CHAR *format, va_list args) { return VSNWPRINTF(dest, size, format, args); }
      static BSS_FORCEINLINE int VPCF(const CHAR* str, va_list args) { return VSCWPRINTF(str, args); }
#endif
      static BSS_FORCEINLINE size_t CONV(const OTHER_C* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF8toUTF16(src, srclen, dest, len); }
      static BSS_FORCEINLINE size_t CONV2(const OTHER_C2* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF32toUTF16(src, srclen, dest, len); }

      static BSS_FORCEINLINE size_t O_SLEN(const OTHER_C* str) { return strlen(str); }
      static BSS_FORCEINLINE const CHAR* STREMPTY() { return L""; }
    };

    template<>
    class STR_CT<char32_t>
    {
    public:
      typedef char32_t CHAR;
      typedef char OTHER_C;
      typedef wchar_t OTHER_C2;

      static BSS_FORCEINLINE const CHAR* SCHR(const CHAR* str, CHAR val) { while(*str && *str != val) ++str; return str; }
      static BSS_FORCEINLINE size_t SLEN(const CHAR* str) { const CHAR* i = str; while(*i) ++i; return (size_t)(i - str); }
      static BSS_FORCEINLINE CHAR* STOK(CHAR* str, const CHAR* delim, CHAR** context)
      {
        if(str)
          *context = str;
        else
          **context = *delim;
        CHAR* r = str = *context;
        while(*str && *str != *delim) ++str;
        *str = 0;
        *context = str;
        return r;
      }
      static BSS_FORCEINLINE size_t CONV(const OTHER_C* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF8toUTF32(src, srclen, dest, len); }
      static BSS_FORCEINLINE size_t CONV2(const OTHER_C2* src, ptrdiff_t srclen, CHAR* dest, size_t len) { return UTF16toUTF32(src, srclen, dest, len); }

      static BSS_FORCEINLINE size_t O_SLEN(const OTHER_C* str) { return strlen(str); }
      static BSS_FORCEINLINE const CHAR* STREMPTY() { return (const CHAR*)"\0\0\0"; }
    };
  }

#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
  template<typename T = char, typename Alloc = std::allocator<T>>
  class BSS_COMPILER_DLLEXPORT StrT : public std::basic_string<T, std::char_traits<T>, Alloc> //If the constructors aren't inlined, it causes heap corruption errors because the basic_string constructors are inlined
  { //Note that you can take off BSS_COMPILER_DLLEXPORT but you'll pay for it with even more unnecessary 4251 warnings.
    typedef internal::STR_CT<T> STRCT;
    typedef typename STRCT::CHAR CHAR;
    typedef typename STRCT::OTHER_C OTHER_C;
    typedef typename STRCT::OTHER_C2 OTHER_C2;
    typedef std::basic_string<T, std::char_traits<T>, Alloc> BASE;

  public:
    inline StrT() : BASE() {}
    explicit inline StrT(size_t length) : BASE() { BASE::reserve(length); } //an implicit constructor here would be bad
    inline StrT(const BASE& copy) : BASE(copy) {}
    inline StrT(BASE&& mov) : BASE(std::move(mov)) {}
    inline StrT(const StrT& copy) : BASE(copy) {}
    inline StrT(StrT&& mov) : BASE(std::move(mov)) {}
    template<class U> inline StrT(const StrT<T, U>& copy) : BASE(copy) {}
    template<class U> inline StrT(const StrT<OTHER_C, U>& copy) : BASE() { _convStr(copy.c_str(), copy.size() + 1); }
    template<class U> inline StrT(const StrT<OTHER_C2, U>& copy) : BASE() { _convStr2(copy.c_str(), copy.size() + 1); }
    inline StrT(const CHAR* string) : BASE(!string ? STRCT::STREMPTY() : string) {}
    inline StrT(const CHAR* string, size_t count) : BASE(!string ? STRCT::STREMPTY() : string, count) {}
    inline StrT(const OTHER_C* text) : BASE() { if(text != 0) _convStr(text, -1); }
    inline StrT(const OTHER_C* text, size_t count) : BASE() { if(text != 0) _convStr(text, count); }
    inline StrT(const OTHER_C2* text) : BASE() { if(text != 0) _convStr2(text, -1); }
    inline StrT(const OTHER_C2* text, size_t count) : BASE() { if(text != 0) _convStr2(text, count); }
    StrT(uint16_t index, const CHAR* text, const CHAR delim) : BASE() //Creates a new string from the specified chunk
    {
      for(uint16_t i = 0; i < index; ++i)
      {
        if(text) text = STRCT::SCHR(text, (int)delim);
        if(text) text++;
      }

      if(!text)
        return;

      const CHAR* end = STRCT::SCHR(text, (int)delim);
      if(!end)
        end = &text[STRCT::SLEN(text)];

      size_t _length = end - text;
      BASE::reserve(++_length);
      BASE::insert(0, text, _length - 1);
    }

    inline operator const CHAR*() const { return _internalPtr(); }

    inline StrT operator +(const StrT& right) const { return StrT(*this) += right; }
    inline StrT operator +(StrT&& right) const { right.insert(0, *this); return (std::move(right)); }
    template<class U> inline const StrT operator +(const StrT<T, U>& right) const { return StrT(*this) += right; }
    template<class U> inline const StrT operator +(StrT<T, U>&& right) const { right.insert(0, *this); return (std::move(right)); }
    inline StrT operator +(const CHAR* right) const { return StrT(*this) += right; }
    inline StrT operator +(const CHAR right) const { return StrT(*this) += right; }

    inline StrT& operator =(const StrT& right) { BASE::operator =(right); return *this; }
    template<class U> inline StrT& operator =(const StrT<T, U>& right) { BASE::operator =(right); return *this; }
    template<class U> inline StrT& operator =(const StrT<OTHER_C, U>& right) { BASE::clear(); _convStr(right.c_str(), right.size() + 1); return *this; }
    template<class U> inline StrT& operator =(const StrT<OTHER_C2, U>& right) { BASE::clear(); _convStr2(right.c_str(), right.size() + 1); return *this; }
    inline StrT& operator =(const CHAR* right) { if(right != 0) BASE::operator =(right); else BASE::clear(); return *this; }
    inline StrT& operator =(const OTHER_C* right) { BASE::clear(); if(right != 0) _convStr(right, -1); return *this; }
    inline StrT& operator =(const OTHER_C2* right) { BASE::clear(); if(right != 0) _convStr2(right, -1); return *this; }
    //inline StrT& operator =(const CHAR right) { BASE::operator =(right); return *this; } //Removed because char is also a number, and therefore confuses the compiler whenever something could feasibly be a number of any kind. If you need it, cast to basic_string<>
    inline StrT& operator =(StrT&& right) { BASE::operator =(std::move(right)); return *this; }

    inline StrT& operator +=(const StrT& right) { BASE::operator +=(right); return *this; }
    template<class U> inline StrT& operator +=(const StrT<T, U>& right) { BASE::operator +=(right); return *this; }
    inline StrT& operator +=(const CHAR* right) { if(right != 0 && right != _internalPtr()) BASE::operator +=(right); return *this; }
    inline StrT& operator +=(const CHAR right) { BASE::operator +=(right); return *this; }

    inline CHAR* UnsafeString() { return _internalPtr(); } //This is potentially dangerous if the string is modified
                                                            //inline const CHAR* String() const { return _internalPtr(); }
    inline CHAR& GetChar(size_t index) { return BASE::operator[](index); }
    inline void RecalcSize() { BASE::resize(STRCT::SLEN(_internalPtr())); }
    inline StrT Trim() const { StrT r(*this); r = _ltrim(_rtrim(r._internalPtr(), r.size())); return r; }
    inline StrT& ReplaceChar(CHAR search, CHAR repl)
    {
      CHAR* pmod = _internalPtr();
      size_t sz = BASE::size();

      for(size_t i = 0; i < sz; ++i)
        if(pmod[i] == search)
          pmod[i] = repl;

      return *this;
    }

    static void Explode(std::vector<StrT> &dest, const CHAR delim, const CHAR* text)
    {
      if(!delim)
        return ExplodeNull(dest, text);

      StrT copy(text);
      CHAR delimhold[2] = { delim, 0 };
      CHAR* hold;
      CHAR* res = STRCT::STOK(copy.UnsafeString(), delimhold, &hold);

      while(res)
      {
        dest.push_back(res);
        res = STRCT::STOK(nullptr, delimhold, &hold);
      }
    }
    static void ExplodeNull(std::vector<StrT> &dest, const CHAR* text)
    {
      if(!text)
        return;

      while(*text)
      {
        dest.push_back(text);
        text += strlen(text) + 1;
      }
    }
    template<typename I, typename F> // F = I(const T* s)
    static BSS_FORCEINLINE size_t ParseTokens(const T* str, const T* delim, std::vector<I>& vec, F parser)
    {
      StrT<T> buf(str);
      return ParseTokens<I, F>(buf.UnsafeString(), delim, vec, parser);
    }
    template<typename I, typename F> // F = I(const T* s)
    static size_t ParseTokens(T* str, const T* delim, std::vector<I>& vec, F parser)
    {
      T* ct;
      T* cur = STRCT::STOK(str, delim, &ct);

      while(cur != 0)
      {
        vec.push_back(parser(cur));
        cur = STRCT::STOK(0, delim, &ct);
      }

      return vec.size();
    }

    static inline std::vector<StrT> Explode(const CHAR delim, const CHAR* text) { std::vector<StrT> r; Explode(r, delim, text); return r; }
    static StrT StripChar(const CHAR* text, const CHAR c)
    {
      StrT r;
      r.resize(STRCT::SLEN(text) + 1); // resize doesn't always account for null terminator
      size_t i;

      for(i = 0; *text != 0; ++text)
        if(*text != c)
          r.UnsafeString()[i++] = *text;

      r.UnsafeString()[i] = 0;
      r.RecalcSize();
      return r;
    }

    inline static StrT<T, Alloc> StrTF(const T* string, va_list vl)
    {
      if(!string)
        return StrT<T, Alloc>();

      //if(STRCT::SCHR(string, '%')==0) //Do we even need to check our va_list?
      //  return StrT<T,Alloc>(string);
      StrT<T, Alloc> r;
#ifdef BSS_COMPILER_GCC // GCC implements va_list in such a way that we actually have to copy it before using it anywhere
      va_list vltemp;
      va_copy(vltemp, vl);
      size_t _length = (size_t)STRCT::VPCF(string, vltemp); // If we didn't copy vl here, it would get modified by vsnprintf and blow up.
      va_end(vltemp);
#else
      size_t _length = (size_t)STRCT::VPCF(string, vl); // This ensures VPCF doesn't modify our original va_list
#endif
      r.resize(_length + 1);
      STRCT::VPF(r.UnsafeString(), r.capacity(), string, vl);
      r.resize(STRCT::SLEN(r.c_str()));
      return r;
    }

  private:
    void operator[](std::allocator<char>&) = delete;
    BSS_FORCEINLINE CHAR* _internalPtr() { return const_cast<CHAR*>(BASE::data()); }
    BSS_FORCEINLINE const CHAR* _internalPtr() const { return BASE::data(); }
    BSS_FORCEINLINE void _convStr(const OTHER_C* src, ptrdiff_t len)
    {
      size_t r = STRCT::CONV(src, len, 0, 0);
      if(r == (size_t)~0)
        return; // If invalid, bail

      BASE::resize(r); // resize() only adds one for null terminator if it feels like it.
      r = STRCT::CONV(src, len, _internalPtr(), BASE::capacity());
      if(r == (size_t)~0)
        return; // If somehow still invalid, bail again

      BASE::resize(r - 1); // resize to actual number of characters instead of simply the maximum (disregard null terminator)
    }
    BSS_FORCEINLINE void _convStr2(const OTHER_C2* src, ptrdiff_t len)
    {
      size_t r = STRCT::CONV2(src, len, 0, 0);
      if(r == (size_t)~0)
        return; // If invalid, bail

      BASE::resize(r); // resize() only adds one for null terminator if it feels like it.
      r = STRCT::CONV2(src, len, _internalPtr(), BASE::capacity());
      if(r == (size_t)~0)
        return; // If somehow still invalid, bail again

      BASE::resize(r - 1); // resize to actual number of characters instead of simply the maximum (disregard null terminator)
    }

    inline static T* _ltrim(T* str) { for(; *str > 0 && *str < 33; ++str); return str; }
    inline static T* _rtrim(T* str, size_t size) { T* inter = str + size; for(; inter > str && *inter < 33; --inter); *(++inter) = 0; return str; }

  };
#pragma warning(pop)

#ifdef BSS_PLATFORM_WIN32
  typedef StrT<wchar_t, std::allocator<wchar_t>> StrW;
  inline StrW StrWF(const wchar_t* string, ...) { va_list vl; va_start(vl, string); StrW r = StrW::StrTF(string, vl); va_end(vl); return r; }
  typedef wchar_t bsschar;
#else
  typedef char bsschar;
#endif
  typedef StrT<char, std::allocator<char>> Str;
  inline Str StrF(const char* string, ...) { va_list vl; va_start(vl, string); Str r = Str::StrTF(string, vl); va_end(vl); return r; }

#ifdef _UNICODE
  typedef StrW TStr;
#else
  typedef Str TStr;
#endif

  // This is a slightly more useful ToString method that performs a true no-op on actual strings

  template<typename T>
  BSS_FORCEINLINE std::conditional_t<std::is_same_v<std::decay_t<T>, char*> || std::is_same_v<std::decay_t<T>, const char*> || std::is_same_v<T, Str>, const T&, Str> ToString(const T& v)
  { 
    if constexpr(std::is_same_v<std::decay_t<T>, char*> || std::is_same_v<std::decay_t<T>, const char*> ||  std::is_base_of_v<std::string, T>)
      return v;
    else
      return std::to_string(v);
  }
}

// This allows Str to inherit all the string operations
  template<class _Elem, class _Alloc> // return NTCS + string
  inline bss::StrT<_Elem, _Alloc> operator+(const _Elem *_Left, const bss::StrT<_Elem, _Alloc>& _Right)
  {
    bss::StrT<_Elem, _Alloc> _Ans(std::char_traits<_Elem>::length(_Left) + _Right.size()); _Ans += _Left; return (_Ans += _Right);
  }
  template<class _Elem, class _Alloc> // return character + string
  inline bss::StrT<_Elem, _Alloc> operator+(const _Elem _Left, const bss::StrT<_Elem, _Alloc>& _Right)
  {
    bss::StrT<_Elem, _Alloc> _Ans(1 + _Right.size()); _Ans += _Left; return (_Ans += _Right);
  }
  template<class _Elem, class _Alloc> // return string + string
  inline bss::StrT<_Elem, _Alloc> operator+(bss::StrT<_Elem, _Alloc>&& _Left, const bss::StrT<_Elem, _Alloc>& _Right)
  {
    _Left.append(_Right); return (std::move(_Left));
  } //These operations are moved to the left because they return a basic_string, not Str
  template<class _Elem, class _Alloc> // return NTCS + string
  inline bss::StrT<_Elem, _Alloc> operator+(const _Elem *_Left, bss::StrT<_Elem, _Alloc>&& _Right)
  {
    _Right.insert(0, _Left); return (std::move(_Right));
  }
  template<class _Elem, class _Alloc> // return character + string
  inline bss::StrT<_Elem, _Alloc> operator+(const _Elem _Left, bss::StrT<_Elem, _Alloc>&& _Right)
  {
    _Right.insert(0, 1, _Left); return (std::move(_Right));
  }
  template<class _Elem, class _Alloc> // return string + NTCS
  inline bss::StrT<_Elem, _Alloc> operator+(bss::StrT<_Elem, _Alloc>&& _Left, const _Elem *_Right)
  {
    _Left.append(_Right); return (std::move(_Left));
  }
  template<class _Elem, class _Alloc> // return string + character
  inline bss::StrT<_Elem, _Alloc> operator+(bss::StrT<_Elem, _Alloc>&& _Left, const _Elem _Right)
  {
    _Left.append(1, _Right); return (std::move(_Left));
  }
  template<class _Elem, class _Alloc> // return string + string
  inline bss::StrT<_Elem, _Alloc> operator+(bss::StrT<_Elem, _Alloc>&& _Left, bss::StrT<_Elem, _Alloc>&& _Right)
  {
    if(_Right.size() <= _Left.capacity() - _Left.size() || _Right.capacity() - _Right.size() < _Left.size())
    {
      _Left.append(_Right);
      return (std::move(_Left));
    }

    _Right.insert(0, _Left);
    return (std::move(_Right));
  }
#endif
