// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __STREAM_SPLITTER_H__BSS__
#define __STREAM_SPLITTER_H__BSS__

#include <sstream>
#include <vector>
#include "bss_dlldef.h"
#include "cAutoPtr.h"
#include "cStr.h"

using namespace std;

namespace bss_util {
  /* Stream buffer that can output to any number of possible external streams, and auto-converts all wchar_t* input to UTF-8 */
  template<typename _Elem=char>
  class __declspec(dllexport) StreamSplitter : public basic_stringbuf<_Elem>
  {
  public:
    inline StreamSplitter(const StreamSplitter& copy) : stringbuf(copy),_targets(copy._targets),_wtargets(copy._wtargets) {}
    inline StreamSplitter(StreamSplitter&& mov) : stringbuf(std::move(mov)),_targets(std::move(copy._targets)),
      _wtargets(std::move(copy._wtargets)) {}
    inline explicit StreamSplitter(ios_base::openmode _Mode = ios_base::out) : stringbuf(_Mode) {}
    inline void BSS_FASTCALL AddTarget(ostream* stream) { _targets.push_back(stream); }
    inline void BSS_FASTCALL AddTarget(wostream* stream) { _wtargets.push_back(stream); }
    inline void ClearTargets() { sync(); _targets.clear(); _wtargets.clear(); }

    inline StreamSplitter& operator =(const StreamSplitter& right) { stringbuf::operator=(right); _targets=right._targets; _wtargets=right._wtargets; return *this;}
    inline StreamSplitter& operator =(StreamSplitter&& right) { stringbuf::operator=(std::move(right)); _targets=std::move(right._targets); _wtargets=std::move(right._wtargets); return *this;}

  protected:
	  inline virtual int __CLR_OR_THIS_CALL sync()
    {
      size_t length = basic_stringbuf<_Elem>::pptr() - basic_stringbuf<_Elem>::eback();
      cAutoPtr<_Elem> hold(new _Elem[++length]); //+1 for null terminator
      MEMCPY(hold, length*sizeof(_Elem), basic_stringbuf<_Elem>::eback(), (length-1)*sizeof(_Elem));
      hold[length-1] = 0;

      _evaltargets(hold);
      _convtargets(hold);

      stringbuf::seekpos(0,ios_base::out); //Flush the buffer (make it overwrite itself because we no longer need what's in there)
      return stringbuf::sync();
    }
    inline void BSS_FASTCALL _evaltargets(const char* str)
    {
      for(size_t i = 0; i < _targets.size(); ++i)
        (*_targets[i]) << str << flush;
    }
    inline void BSS_FASTCALL _evaltargets(const wchar_t* str)
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
    }
    inline void BSS_FASTCALL _convtargets(const wchar_t* str)
    {
      if(_targets.size()>0)
      {
        cStr hold(str);
        for(size_t i = 0; i < _targets.size(); ++i)
          (*_targets[i]) << hold.c_str() << flush;
      }
    }

	  //virtual streamsize __CLR_OR_THIS_CALL xsputn(const char *_Ptr,	streamsize _Count);
	  //virtual streamsize __CLR_OR_THIS_CALL xsputn(const wchar_t *_Ptr,	streamsize _Count);
    //inline virtual StreamSplitter* __CLR_OR_THIS_CALL setbuf( wchar_t *, streamsize) { return 0; }
	  //virtual int_type __CLR_OR_THIS_CALL overflow(int_type = _Traits::eof());

  private:
#pragma warning(push)
#pragma warning(disable:4251)
    vector<ostream*> _targets;
    vector<wostream*> _wtargets;
#pragma warning(pop)
  };
}

#endif