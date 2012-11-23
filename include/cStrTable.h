// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_STR_TABLE_H__BSS__
#define __C_STR_TABLE_H__BSS__

//#include "bss_alloc.h"
#include "bss_util.h"
#include "cArraySimple.h"
#include <stdlib.h>
#include <ostream>
#include <istream>

namespace bss_util {
  // helper class for template-izing
  template<typename T,typename ST_> class BSS_COMPILER_DLLEXPORT STRTABLE_FUNC {};
  template<typename ST_> class BSS_COMPILER_DLLEXPORT STRTABLE_FUNC<char,ST_> { public: inline ST_ _lstr(const char* str) { return strlen(str); } };
  template<typename ST_> class BSS_COMPILER_DLLEXPORT STRTABLE_FUNC<wchar_t,ST_> { public: inline ST_ _lstr(const wchar_t* str) { return wcslen(str); } };

  // Given a large array of strings (or a memory dump), assembles a single chunk of memory into a series of strings that can be accessed by index instantly
  template<typename T, typename ST_ = unsigned int>
  class BSS_COMPILER_DLLEXPORT cStrTable : protected STRTABLE_FUNC<T,ST_>
  {
  public:
    // Default Copy Constructor
    inline cStrTable(const cStrTable& copy) : _strings(copy._strings), _indices(copy._indices) {}
    inline cStrTable(cStrTable&& mov) : _strings(std::move(mov._strings)), _indices(std::move(mov._indices)) {}
    // Constructor for array with compile-time determined size
    template<ST_ N>
    inline cStrTable(const T* (&strings)[N]) : _strings(0), _indices(0) { _construct(strings,N); }
    // Constructor with a null-terminated array of strings.
    inline cStrTable(const T* const* strings, ST_ size) : _strings(0), _indices(0) { _construct(strings,size); }
    // Constructor from a stream that's a series of null terminated strings.
    inline cStrTable(std::istream* stream, ST_ bytes) : _strings(0), _indices(0)
    {
      if(!stream || !bytes)
        return;

      _strings.SetSize(bytes/sizeof(T));
      stream->read((char*)(T*)_strings,_strings.Size()*sizeof(T));
      _strings[(stream->gcount()/sizeof(T))-1]='\0'; //make sure the end is a null terminator

      _indices.SetSize(strccount<T>(_strings,0,_strings.Size()));
      memset((ST_*)_indices,0,_indices.Size()*sizeof(ST_));

      ST_ j=1;
      for(ST_ i=0; i<_strings.Size() && j<_indices.Size(); ++i)
      {
        ++_indices[j];
        if(!_strings[i]) { ++j; if(j<_indices.Size()) _indices[j]=_indices[j-1]; }
      }
    }
    // Destructor
    inline ~cStrTable() { }
    // Gets number of strings in table (index cannot be greater then this)
    inline ST_ Length() const { return _indices.Size(); }
    // Gets total length of all strings
    inline ST_ TotalWordSize() const { return _strings.Size(); }
    // Returns string with the corresponding index. Strings are returned null-terminated, but the index bound is not checked.
    inline const T* GetString(ST_ index) const { assert(index<_indices.Size()); return _strings+_indices[index]; }
    inline void AppendString(const char* s)
    { 
      ST_ last = STRTABLE_FUNC<T,ST_>::_lstr(_strings+_indices[_indices.Size()-1])+1+_indices[_indices.Size()-1];
      ST_ sz = STRTABLE_FUNC<T,ST_>::_lstr(s)+1;
      
      _indices.SetSize(_indices.Size()+1);
      _indices[_indices.Size()-1]=last; // Add another indice and set its value appropriately
      last=_strings.Size();
      _strings.SetSize(_strings.Size()+sz); // Allocate more space in string buffer
      memcpy(_strings+last,s,sz); // Copy over new string (including null terminator)
    };
    // Returns index array
    inline const ST_* GetIndices() const { return _indices; }
    // Dumps table to stream
    inline void DumpToStream(std::ostream* stream) { stream->write((const char*)_strings,_strings.Size()*sizeof(T)); }

    inline const T* operator[](ST_ index) { return GetString(index); }
    inline cStrTable& operator=(const cStrTable& right)
    {
      _strings=right._strings;
      _indices=right._indices;
      return *this;
    }
    inline cStrTable& operator=(cStrTable&& right)
    {
      _strings=std::move(right._strings);
      _indices=std::move(right._indices);
      return *this;
    }

    inline cStrTable& operator+=(const cStrTable& right)
    {
      ST_ byteadd = _strings.Size();
      ST_ i = _indices.Size(); //start at old value
      _strings+=right._strings;
      _indices+=right._indices;

      for(; i < _indices.Size(); ++i) _indices[i]+=byteadd; //remaps indexes

      return *this;
    }
    inline cStrTable operator+(const cStrTable& right) { return (cStrTable retval(*this))+=right; }
    inline cStrTable& operator+=(const char* right) { AppendString(right); return *this; }

  protected:
    inline void BSS_FASTCALL _construct(const T* const* strings, ST_ size)
    {
      if(!strings || !size) //special handling for empty case
        return;

      _indices.SetSize(size);
      ST_ sz=0;
      for(ST_ i = 0; i < size; ++i) //this will cause an infinite loop if someone is dumb enough to set ST_ to an unsigned type and set size equal to -1. If you actually think you could make that mistake, stop using this class.
      {
        _indices[i] = STRTABLE_FUNC<T,ST_>::_lstr(strings[i])+1; //include null terminator in length count
        sz+=_indices[i];
      }
      _strings.SetSize(sz);

      ST_ curlength=0;
      ST_ hold;
      for(ST_ i = 0; i < size; ++i) //this loop does two things - it copies the strings over, and it sets the index values to the correct ones while discarding intermediate lengths
      {
        hold=_indices[i];
        _indices[i]=curlength;
        memcpy(_strings+curlength,strings[i],hold*sizeof(T)); //we could recalculate strlen here but that's dangerous because it might overwrite memory
        curlength+=hold;
      }
    }

    cArrayWrap<cArraySimple<T,ST_>> _strings;
    cArrayWrap<cArraySimple<ST_,ST_>> _indices;
  };   
}

#endif
