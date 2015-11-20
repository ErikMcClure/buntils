// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STREAM_H__BSS__
#define __STREAM_H__BSS__

#include <sstream>
#include <streambuf>
#include "cStr.h"
#include "cDynArray.h"

/* modifies the basic_ostream so it takes wchar_t and converts it into UTF */
template<class _Traits>
inline std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const wchar_t *_Val)
{
  _Ostr << cStr(_Val).c_str();
  return _Ostr;
}

namespace bss_util {
  /* Stream buffer that can output to any number of possible external streams, and auto-converts all wchar_t* input to UTF-8 */
#pragma warning(push)
#pragma warning(disable:4251)
  class BSS_COMPILER_DLLEXPORT StreamSplitter : public std::basic_stringbuf<char>
  {
    inline StreamSplitter(const StreamSplitter& copy) BSS_DELETEFUNC
    inline StreamSplitter& operator =(const StreamSplitter& right) BSS_DELETEFUNCOP
  public:
#ifndef BSS_COMPILER_GCC
    inline StreamSplitter(StreamSplitter&& mov) : std::basic_stringbuf<char>(std::move(mov)), _targets(std::move(mov._targets)) {}//, _wtargets(move(copy._wtargets)) {}
#endif
    inline explicit StreamSplitter(std::ios_base::openmode _Mode = std::ios_base::out) : std::basic_stringbuf<char>(_Mode) { }
    inline void BSS_FASTCALL AddTarget(std::ostream* stream) { sync(); _targets.push_back(stream); }
    inline void ClearTargets() { sync(); _targets.clear(); } //_wtargets.clear(); }

#ifndef BSS_COMPILER_GCC
    inline StreamSplitter& operator =(StreamSplitter&& right) { std::basic_stringbuf<char>::operator=(std::move(right)); _targets=std::move(right._targets); /*_wtargets=std::move(right._wtargets);*/ return *this;}
#endif

  protected:
	  virtual int sync()
    {
      size_t length = std::basic_stringbuf<char>::pptr() - std::basic_stringbuf<char>::pbase();

      for(size_t i = 0; i < _targets.size(); ++i)
        _targets[i]->write(basic_stringbuf<char>::pbase(),length).flush();

      std::basic_stringbuf<char>::seekpos(0,std::ios_base::out); //Flush the buffer (make it overwrite itself because we no longer need what's in there)
      return std::basic_stringbuf<char>::sync();
    }

  protected: // basic_stringbuf has move semantics, not copy semantics. Of course, VC++ doesn't actually warn you about this. Anywhere.
    std::vector<std::ostream*> _targets;
  };

  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT DynArrayIBuf : public std::streambuf
  {
    inline DynArrayIBuf(const DynArrayIBuf& copy) BSS_DELETEFUNC
    inline DynArrayIBuf& operator =(const DynArrayIBuf& right) BSS_DELETEFUNCOP

  public:
    DynArrayIBuf(DynArrayIBuf&& mov) : std::streambuf(std::move(mov)), _ref(mov._ref), _read(mov._read) { mov._read = (CType)-1; }
    DynArrayIBuf(const cDynArray<T, CType, ArrayType, Alloc>& ref) : _ref(ref), _read(0) {}

    inline DynArrayIBuf& operator =(DynArrayIBuf&& right) { 
      std::streambuf::operator=(std::move(right));
      _ref = right._ref;
      _read = right._read;
      right._read = (CType)-1;
      return *this;
    }

  protected:
    inline virtual int_type uflow() { return (_read == _ref.Length()) ? traits_type::eof() : traits_type::to_int_type(_ref[_read++]); }
    inline virtual int_type underflow() { return (_read == _ref.Length()) ? traits_type::eof() : traits_type::to_int_type(_ref[_read]); }
    inline virtual int_type pbackfail(int_type ch) {
      if(_read == 0 || (ch != traits_type::eof() && ch != _ref[_read-1]))
        return traits_type::eof();
      return traits_type::to_int_type(_ref[--_read]);
    }
    inline virtual std::streamsize showmanyc() { assert(_read <= _ref.Length()); return _read - _ref.Length(); }

    virtual pos_type __CLR_OR_THIS_CALL seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which_)
    {	// change position by offset, according to way and mode
      switch(way)
      {
      case std::ios_base::cur:
        _read += off;
        break;
      case std::ios_base::end:
        _read = _ref.Length() + off;
        break;
      case std::ios_base::beg:
        _read = off;
        break;
      }

      return _read;
    }

    virtual pos_type __CLR_OR_THIS_CALL seekpos(pos_type sp_, std::ios_base::openmode which_)
    {	// change to specified position, according to mode
      return seekoff(off_type(sp_), std::ios_base::beg, which_);
    }

    const cDynArray<T, CType, ArrayType, Alloc>& _ref;
    CType _read;
  };


  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT DynArrayBuf : public DynArrayIBuf<T, CType, ArrayType, Alloc>
  {
    typedef DynArrayIBuf<T, CType, ArrayType, Alloc> BASE;
    inline DynArrayBuf(const DynArrayBuf& copy) BSS_DELETEFUNC
    inline DynArrayBuf& operator =(const DynArrayBuf& right) BSS_DELETEFUNCOP

  public:
    DynArrayBuf(DynArrayBuf&& mov) : BASE(std::move(mov)), _write(mov._write) { mov._write = (CType)-1; }
    DynArrayBuf(cDynArray<T, CType, ArrayType, Alloc>& ref, CType begin = (CType)-1) : BASE(ref), _write((begin == (CType)-1) ? ref.Length() : begin) {}

    inline DynArrayBuf& operator =(DynArrayIBuf&& right) { BASE::operator=(std::move(right)); _write = right._write; right._write = (CType)-1; return *this; }

  protected:
    int_type overflow(int_type ch) { const_cast<cDynArray<T, CType, ArrayType, Alloc>&>(BASE::_ref).Insert(ch, _write++); return ch; }

    CType _write;
  };

#pragma warning(pop)
}

#endif
