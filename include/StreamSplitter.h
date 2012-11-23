// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STREAM_SPLITTER_H__BSS__
#define __STREAM_SPLITTER_H__BSS__

#include <sstream>
#include <memory>
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
#pragma warning(pop)
  public:
    //inline StreamSplitter(const StreamSplitter& copy) : std::basic_stringbuf<char>(copy),_targets(copy._targets) {}
#ifndef BSS_COMPILER_GCC
    inline StreamSplitter(StreamSplitter&& mov) : std::basic_stringbuf<char>(std::move(mov)), _targets(std::move(mov._targets)) {}//, _wtargets(move(copy._wtargets)) {}
#endif
    inline explicit StreamSplitter(std::ios_base::openmode _Mode = std::ios_base::out) : std::basic_stringbuf<char>(_Mode) {}
    inline void BSS_FASTCALL AddTarget(std::ostream* stream) { _targets.push_back(stream); }
    //inline void BSS_FASTCALL AddTarget(wostream* stream) { _wtargets.push_back(stream); }
    inline void ClearTargets() { sync(); _targets.clear(); } //_wtargets.clear(); }

    //inline StreamSplitter& operator =(const StreamSplitter& right) { std::basic_stringbuf<char>::operator=(right); _targets=right._targets; /*_wtargets=right._wtargets;*/ return *this;}
#ifndef BSS_COMPILER_GCC
    inline StreamSplitter& operator =(StreamSplitter&& right) { std::basic_stringbuf<char>::operator=(std::move(right)); _targets=std::move(right._targets); /*_wtargets=std::move(right._wtargets);*/ return *this;}
#endif

  protected:
	  inline virtual int sync()
    {
      size_t length = std::basic_stringbuf<char>::pptr() - std::basic_stringbuf<char>::eback();
      std::unique_ptr<char[]> hold(new char[++length]); //+1 for null terminator
      MEMCPY(hold.get(), length*sizeof(char), basic_stringbuf<char>::eback(), (length-1)*sizeof(char));
      hold.get()[length-1] = 0;

      _evaltargets(hold.get());
      //_convtargets(hold.get());

      std::basic_stringbuf<char>::seekpos(0,std::ios_base::out); //Flush the buffer (make it overwrite itself because we no longer need what's in there)
      return std::basic_stringbuf<char>::sync();
    }
    inline void BSS_FASTCALL _evaltargets(const char* str)
    {
      for(size_t i = 0; i < _targets.size(); ++i)
        (*_targets[i]) << str << std::flush;
    }
    /*inline void BSS_FASTCALL _evaltargets(const wchar_t* str)
    {
      for(size_t i = 0; i < _targets.size(); ++i)
        (*_wtargets[i]) << str << flush;
    }
    inline void BSS_FASTCALL _convtargets(const char* str)
    {
      if(_wtargets.size()>0)
      {
        cStrW whold(str);
        for(size_t i = 0; i < _wtargets.size(); ++i)
          (*_wtargets[i]) << whold.c_str() << flush;
      }
    }*/
    /*inline void BSS_FASTCALL _convtargets(const wchar_t* str)
    {
      if(_targets.size()>0)
      {
        std::string utf;
        ToUTF8(str,utf);
        for(size_t i = 0; i < _targets.size(); ++i)
          (*_targets[i]) << utf.c_str() << flush;
      }*
    }*/

	  //virtual streamsize xsputn(const char *_Ptr,	streamsize _Count);
	  //virtual streamsize xsputn(const wchar_t *_Ptr,	streamsize _Count);
    //inline virtual StreamSplitter* setbuf( wchar_t *, streamsize) { return 0; }
	  //virtual int_type overflow(int_type = _Traits::eof());

  private: // basic_stringbuf has move semantics, not copy semantics. Of course, VC++ doesn't actually warn you about this. Anywhere.
    inline StreamSplitter(const StreamSplitter& copy) { assert(false); }
    inline StreamSplitter& operator =(const StreamSplitter& right) { assert(false); return *this; }
#pragma warning(push)
#pragma warning(disable:4251)
    std::vector<std::ostream*> _targets;
    //vector<wostream*> _wtargets;
#pragma warning(pop)
  };
}

#endif
