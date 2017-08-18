// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STREAM_H__BSS__
#define __STREAM_H__BSS__

#include <sstream>
#include <streambuf>
#include <vector>
#include "Str.h"
#include "DynArray.h"

/* modifies the basic_ostream so it takes wchar_t and converts it into UTF */
template<class _Traits>
inline std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const wchar_t *_Val)
{
  _Ostr << bss::Str(_Val).c_str();
  return _Ostr;
}

namespace bss {
  /* Stream buffer that can output to any number of possible external streams, and auto-converts all wchar_t* input to UTF-8 */
#pragma warning(push)
#pragma warning(disable:4251)
  class BSS_COMPILER_DLLEXPORT StreamSplitter : public std::basic_stringbuf<char>
  {
    inline StreamSplitter(const StreamSplitter& copy) = delete;
      inline StreamSplitter& operator =(const StreamSplitter& right) = delete;
  public:
#ifndef BSS_COMPILER_GCC
    inline StreamSplitter(StreamSplitter&& mov) : std::basic_stringbuf<char>(std::move(mov)), _targets(std::move(mov._targets)) {}//, _wtargets(move(copy._wtargets)) {}
#endif
    inline explicit StreamSplitter(std::ios_base::openmode _Mode = std::ios_base::out) : std::basic_stringbuf<char>(_Mode) {}
    inline void AddTarget(std::ostream* stream) { sync(); _targets.push_back(stream); }
    inline void ClearTargets() { sync(); _targets.clear(); } //_wtargets.clear(); }

#ifndef BSS_COMPILER_GCC
    inline StreamSplitter& operator =(StreamSplitter&& right) 
    { 
      std::basic_stringbuf<char>::operator=(std::move(right));
      _targets = std::move(right._targets); 
      /*_wtargets=std::move(right._wtargets);*/
      return *this; 
    }
#endif

  protected:
    virtual int sync()
    {
      size_t length = std::basic_stringbuf<char>::pptr() - std::basic_stringbuf<char>::pbase();

      for(size_t i = 0; i < _targets.size(); ++i)
        _targets[i]->write(basic_stringbuf<char>::pbase(), length).flush();

      std::basic_stringbuf<char>::seekpos(0, std::ios_base::out); //Flush the buffer (make it overwrite itself because we no longer need what's in there)
      return std::basic_stringbuf<char>::sync();
    }

  protected: // basic_stringbuf has move semantics, not copy semantics. Of course, VC++ doesn't actually warn you about this. Anywhere.
    std::vector<std::ostream*> _targets;
  };
    
  // Implementation of a read-only memory stream buffer
  template<class T>
  class BSS_COMPILER_DLLEXPORT StreamBufArray : public std::streambuf
  {
    inline StreamBufArray(const StreamBufArray& copy) = delete;
    inline StreamBufArray& operator =(const StreamBufArray& right) = delete;

  public:
    StreamBufArray(StreamBufArray&& mov) : std::streambuf(std::move(mov)), _begin(mov._begin), _cur(mov._cur), _end(mov._end)
    { 
      mov._begin = 0;
      mov._cur = 0;
      mov._end = 0;
    }
    StreamBufArray(const T* src, size_t sz) : _begin(reinterpret_cast<const char*>(src)), _cur(reinterpret_cast<const char*>(src)), _end(reinterpret_cast<const char*>(src + sz)) {}

    inline StreamBufArray& operator =(StreamBufArray&& right)
    {
      std::streambuf::operator=(std::move(right));
      _begin = right._begin;
      _cur = right._cur;
      _end = right._end;
      right._begin = 0;
      right._cur = 0;
      right._end = 0;
      return *this;
    }

  protected:
    inline virtual int_type uflow() override { return (_cur == _end) ? traits_type::eof() : traits_type::to_int_type(*_cur++); }
    inline virtual int_type underflow() override { return (_cur == _end) ? traits_type::eof() : traits_type::to_int_type(*_cur); }
    inline virtual int_type pbackfail(int_type ch) override
    {
      if(_cur == _begin || (ch != traits_type::eof() && ch != _cur[-1]))
        return traits_type::eof();
      return traits_type::to_int_type(*--_cur);
    }
    inline virtual std::streamsize showmanyc() override { assert(_cur <= _end); return _end - _cur; }
    inline virtual std::streamsize xsgetn(char* s, std::streamsize n) override
    { 
      std::streamsize r = _end - _cur;
      if(r > n)
        r = n;
      if(r > 0)
        MEMCPY(s, n, _cur, r);
      _cur += r;
      return r;
    }
    virtual pos_type seekpos(pos_type sp_, std::ios_base::openmode which_) override { return seekoff(off_type(sp_), std::ios_base::beg, which_); }
    virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which_) override
    {	// change position by offset, according to way and mode
      switch(way)
      {
      case std::ios_base::cur:
        _cur += off;
        break;
      case std::ios_base::end:
        _cur = _end + off;
        break;
      case std::ios_base::beg:
        _cur = _begin + off;
        break;
      }

      return _cur - _begin;
    }

    const char* _begin;
    const char* _cur;
    const char* _end;
  };

  // Read and write stream for DynArray
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = ARRAY_SIMPLE, typename Alloc = StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT StreamBufDynArray : public std::streambuf
  {
    typedef DynArray<T, CType, ArrayType, Alloc> D;
    inline StreamBufDynArray(const StreamBufDynArray& copy) = delete;
    inline StreamBufDynArray& operator =(const StreamBufDynArray& right) = delete;

  public:
    StreamBufDynArray(StreamBufDynArray&& mov) : std::streambuf(std::move(mov)), _ref(mov._ref){}
    StreamBufDynArray(D& ref, CType begin = 0) : _ref(ref)
    {
      setg(reinterpret_cast<char*>(_ref.begin() + begin), reinterpret_cast<char*>(_ref.begin() + begin), reinterpret_cast<char*>(_ref.end()));
      setp(reinterpret_cast<char*>(_ref.end()), reinterpret_cast<char*>(_ref.begin() + _ref.Capacity()));
    }

  protected:
    inline virtual pos_type seekpos(pos_type sp_, std::ios_base::openmode which_) override { return seekoff(off_type(sp_), std::ios_base::beg, which_); }
    inline virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which_) override
    {	// change position by offset, according to way and mode
      switch(way)
      {
      case std::ios_base::cur:
        setg(eback(), gptr() + off, egptr());
        break;
      case std::ios_base::end:
        setg(eback(), egptr() + off, egptr());
        break;
      case std::ios_base::beg:
        setg(eback(), eback() + off, egptr());
        break;
      }

      return gptr() - eback();
    }
    inline virtual int_type underflow() override { return gptr() == egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr()); }
    inline virtual std::streamsize xsgetn(char* s, std::streamsize n) override
    {
      std::streamsize r = egptr() - gptr();
      if(r > n)
        r = n;
      if(r > 0)
        MEMCPY(s, n, gptr(), r);
      setg(eback(), gptr() + r, egptr());
      return r;
    }

    void _fixlength()
    {
      _ref._length = (pptr() - eback()) / sizeof(T); // Only set length to the number of fully written indexes
      setg(eback(), gptr(), reinterpret_cast<char*>(_ref.end())); // We cast back from end() because we might not have finished writing through all the bytes in T
    }
    void _expand(CType sz)
    {
      ptrdiff_t goff = gptr() - eback();
      _ref.SetCapacity(std::max<CType>(fbnext(_ref._length), (sz / sizeof(T)) + _ref._length + 1));
      setg(reinterpret_cast<char*>(_ref.begin()), reinterpret_cast<char*>(_ref.begin() + goff), reinterpret_cast<char*>(_ref.end()));
      setp(reinterpret_cast<char*>(_ref.end()), reinterpret_cast<char*>(_ref.begin() + _ref.Capacity()));
    }
    inline virtual int_type overflow(int_type ch) override
    {
      if(ch != traits_type::eof())
      {
        if(pptr() == epptr())
          _expand(0);
        assert(pptr() < epptr());
        *pptr() = ch;
        pbump(1);
        _fixlength();
        return ch;
      }
      return traits_type::eof();
    }
    inline virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
      if(pptr() == epptr())
        _expand(n);
      std::streamsize r = epptr() - pptr();
      if(r > n)
        r = n;
      if(r > 0)
        MEMCPY(pptr(), r, s, r);
      pbump(r);
      _fixlength();
      n -= r;
      s += r;
      if(n)
        return xsputn(s, n);
      return r;
    }

    D& _ref;
  };
  
  // Generic read/write streambuffer that performs an operation on the input and output.
  class StreamBufFunction : public std::streambuf
  {
    inline StreamBufFunction(const StreamBufFunction& copy) = delete;
    inline StreamBufFunction& operator =(const StreamBufFunction& right) = delete;

  public:
    StreamBufFunction(StreamBufFunction&& mov) : std::streambuf(std::move(mov)) {}
    StreamBufFunction(std::istream& in, size_t bufsize = DEFAULTBUFSIZE) : _ibuf(new char[bufsize]), _obuf(new char[bufsize]), _sz(bufsize), _in(&in), _out(0)
    {
      setg(_obuf + _sz, _obuf + _sz, _obuf + _sz);
    }
    StreamBufFunction(std::ostream& out, size_t bufsize = DEFAULTBUFSIZE) : _ibuf(new char[bufsize]), _obuf(new char[bufsize]), _sz(bufsize), _in(0), _out(&out)
    {
      setp(_obuf + _sz, _obuf + _sz);
    }
    ~StreamBufFunction()
    {
      delete[] _ibuf;
      delete[] _obuf;
    }

    static const size_t DEFAULTBUFSIZE = (std::size_t)1 << 18;

  protected:
    inline virtual int_type sync() override 
    { 
      assert(_out != 0); 
      if(_out) 
        _out->flush();
      return 0;
    }
    inline virtual int_type underflow() override
    {
      if(gptr() == egptr())
        _onread();
      return gptr() == egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr());
    }
    inline virtual std::streamsize xsgetn(char* s, std::streamsize n) override
    {
      if(gptr() == egptr())
        _onread();
      std::streamsize r = egptr() - gptr();
      if(r > n)
        r = n;
      if(r > 0)
        MEMCPY(s, n, gptr(), r);
      setg(eback(), gptr() + r, egptr());
      return r;
    }
    inline virtual int_type overflow(int_type ch) override
    {
      if(ch != traits_type::eof())
      {
        if(pptr() == epptr())
          _onwrite();
        if(pptr() < epptr())
        {
          *pptr() = ch;
          pbump(1);
          return ch;
        }
      }
      return traits_type::eof();
    }
    inline virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
      if(pptr() == epptr())
        _onwrite();
      std::streamsize r = epptr() - pptr();
      if(r > n)
        r = n;
      if(r > 0)
        MEMCPY(pptr(), r, s, r);
      pbump(r);
      n -= r;
      s += r;
      if(n)
        return xsputn(s, n);
      return r;
    }
    virtual void _onwrite() = 0; // should read input from _in when appropriate.
    virtual void _onread() = 0; // should write output to _out when appropriate.

    char* _ibuf;
    char* _obuf;
    size_t _sz;
    std::istream* _in;
    std::ostream* _out;
  };

#pragma warning(pop)
}

#endif
