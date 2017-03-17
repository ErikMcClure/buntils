// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_STR_TABLE_H__BSS__
#define __C_STR_TABLE_H__BSS__

//#include "bss_alloc.h"
#include "bss_util.h"
#include "cArray.h"
#include "cStr.h"
#include <stdlib.h>
#include <ostream>
#include <istream>

namespace bss_util {
  // Given a large array of strings (or a memory dump), assembles a single chunk of memory into a series of strings that can be accessed by index instantly
  template<typename T, typename CT_ = size_t>
  class BSS_COMPILER_DLLEXPORT cStrTable
  {
  public:
    // Default Copy Constructor
    inline cStrTable(const cStrTable& copy) : _strings(copy._strings), _indices(copy._indices) {}
    inline cStrTable(cStrTable&& mov) : _strings(std::move(mov._strings)), _indices(std::move(mov._indices)) {}
    // Constructor for array with compile-time determined size
    template<CT_ N>
    inline cStrTable(const T* (&strings)[N]) : _strings(0), _indices(0) { _construct(strings, N); }
    // Constructor with a null-terminated array of strings.
    inline cStrTable(const T* const* strings, CT_ size) : _strings(0), _indices(0) { _construct(strings, size); }
    // Constructor from a stream that's a series of null terminated strings.
    cStrTable(std::istream* stream, CT_ bytes) : _strings(0), _indices(0)
    {
      if(!stream || !bytes)
        return;

      _strings.SetCapacity(bytes/sizeof(T));
      stream->read((char*)(T*)_strings, _strings.Capacity()*sizeof(T));
      _strings[(stream->gcount()/sizeof(T))-1]='\0'; //make sure the end is a null terminator

      _indices.SetCapacity(strccount<T>(_strings, 0, _strings.Capacity()));
      memset((CT_*)_indices, 0, _indices.Capacity()*sizeof(CT_));

      CT_ j=1;
      for(CT_ i=0; i<_strings.Capacity() && j<_indices.Capacity(); ++i)
      {
        ++_indices[j];
        if(!_strings[i]) { ++j; if(j<_indices.Capacity()) _indices[j]=_indices[j-1]; }
      }
    }
    // Destructor
    inline ~cStrTable() { }
    // Gets number of strings in table (index cannot be greater then this)
    inline CT_ Length() const { return _indices.Capacity(); }
    // Gets total length of all strings
    inline CT_ TotalWordCapacity() const { return _strings.Capacity(); }
    // Returns string with the corresponding index. Strings are returned null-terminated, but the index bound is not checked.
    inline const T* GetString(CT_ index) const { assert(index<_indices.Capacity()); return _strings+_indices[index]; }
    inline void AppendString(const char* s)
    {
      CT_ last = CSTR_CT<T>::SLEN(_strings+_indices[_indices.Capacity()-1])+1+_indices[_indices.Capacity()-1];
      CT_ sz = CSTR_CT<T>::SLEN(s)+1;

      _indices.SetCapacity(_indices.Capacity()+1);
      _indices[_indices.Capacity()-1]=last; // Add another indice and set its value appropriately
      last=_strings.Capacity();
      _strings.SetCapacity(_strings.Capacity()+sz); // Allocate more space in string buffer
      memcpy(_strings+last, s, sz); // Copy over new string (including null terminator)
    };
    // Returns index array
    inline const CT_* GetIndices() const { return _indices; }
    // Dumps table to stream
    inline void DumpToStream(std::ostream* stream) { stream->write((const char*)_strings, _strings.Capacity()*sizeof(T)); }

    inline const T* operator[](CT_ index) { return GetString(index); }
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
      CT_ byteadd = _strings.Capacity();
      CT_ i = _indices.Capacity(); //start at old value
      _strings+=right._strings;
      _indices+=right._indices;

      for(; i < _indices.Capacity(); ++i) _indices[i]+=byteadd; //remaps indexes

      return *this;
    }
    inline cStrTable operator+(const cStrTable& right) { cStrTable retval(*this); return retval+=right; }
    inline cStrTable& operator+=(const char* right) { AppendString(right); return *this; }

  protected:
    void _construct(const T* const* strings, CT_ size)
    {
      if(!strings || !size) //special handling for empty case
        return;

      _indices.SetCapacity(size);
      CT_ sz=0;
      for(CT_ i = 0; i < size; ++i) //this will cause an infinite loop if someone is dumb enough to set CT_ to an unsigned type and set size equal to -1. If you actually think you could make that mistake, stop using this class.
      {
        _indices[i] = CSTR_CT<T>::SLEN(strings[i])+1; //include null terminator in length count
        sz+=_indices[i];
      }
      _strings.SetCapacity(sz);

      CT_ curlength=0;
      CT_ hold;
      for(CT_ i = 0; i < size; ++i) //this loop does two things - it copies the strings over, and it sets the index values to the correct ones while discarding intermediate lengths
      {
        hold=_indices[i];
        _indices[i]=curlength;
        memcpy(_strings+curlength, strings[i], hold*sizeof(T)); //we could recalculate strlen here but that's dangerous because it might overwrite memory
        curlength+=hold;
      }
    }

    cArray<T, CT_> _strings;
    cArray<CT_, CT_> _indices;
  };
}

#endif
