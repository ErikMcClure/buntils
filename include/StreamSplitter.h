// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STREAM_SPLITTER_H__BSS__
#define __STREAM_SPLITTER_H__BSS__

#include <sstream>
#include "cStr.h"

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

  private: // basic_stringbuf has move semantics, not copy semantics. Of course, VC++ doesn't actually warn you about this. Anywhere.
    inline StreamSplitter(const StreamSplitter& copy) { assert(false); }
    inline StreamSplitter& operator =(const StreamSplitter& right) { assert(false); return *this; }
    std::vector<std::ostream*> _targets;
  };
#pragma warning(pop)
}

#endif
