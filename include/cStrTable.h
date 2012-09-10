// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_STR_TABLE_H__BSS__
#define __C_STR_TABLE_H__BSS__

//#include "bss_alloc.h"
#include "bss_util.h"
#include <malloc.h>
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
    inline cStrTable(const cStrTable& copy) : _sasize(copy._sasize), _numindices(copy._numindices)
    { 
      _strings=(T*)malloc(_sasize*sizeof(T));
      memcpy(_strings,copy._strings,_sasize*sizeof(T));
      _indexarray=(ST_*)malloc(_numindices*sizeof(ST_));
      memcpy(_indexarray,copy._indexarray,_numindices*sizeof(ST_));
    }
    inline cStrTable(cStrTable&& mov) : _sasize(mov._sasize), _numindices(mov._numindices),_strings(mov._strings),_indexarray(mov._indexarray)
    {
      mov._sasize=0;
      mov._numindices=0;
      mov._strings=0;
      mov._indexarray=0;
    }
    // Generic Copy Constructor (for conversions)
    template<class U> inline cStrTable(const cStrTable<T,U>& copy) : _sasize((ST_)copy.TotalWordSize()), _numindices((ST_)copy.Length())
    {
      _strings=(T*)malloc(_sasize*sizeof(T));
      memcpy(_strings,copy.GetString(0),_sasize*sizeof(T));
      _indexarray=(ST_*)malloc(_numindices*sizeof(ST_));
      for(ST_ i = 0; i < _numindices; ++i) _indexarray[i]=(ST_)copy.GetIndices()[i];
    }
    // Constructor from a stream that's a series of null terminated strings.
    inline cStrTable(std::istream* stream, ST_ size) : _sasize(size/sizeof(T)), _numindices(0)
    {
      if(!stream || !size) 
      {
        _indexarray=(ST_*)malloc(sizeof(ST_));
        _indexarray[0]=0;
        _strings=(T*)malloc(1); //malloc(0) is undefined
        return;
      }

      _strings=(T*)malloc(_sasize*sizeof(T));
      stream->read((char*)_strings,_sasize*sizeof(T));
      _strings[(stream->gcount()/sizeof(T))-1]='\0'; //make sure the end is a null terminator

      _numindices=strccount<T>(_strings,0,_sasize);
      _indexarray=(ST_*)malloc(_numindices*sizeof(ST_));
      memset(_indexarray,0,_numindices*sizeof(ST_));

      ST_ j=1;
      for(ST_ i=0; i<_sasize && j<_numindices; ++i)
      {
        ++_indexarray[j];
        if(!_strings[i]) { ++j; if(j<_numindices) _indexarray[j]=_indexarray[j-1]; }
      }
    }
    
    // Constructor for array with compile-time determined size
    template<ST_ N>
    inline cStrTable(const T* (&strings)[N]) : _sasize(0), _numindices(N) { _construct(strings,N); }
    // Constructor with a null-terminated array of strings.
    inline cStrTable(const T* const* strings, ST_ size) : _sasize(0), _numindices(size) { _construct(strings,size); }
    // Destructor
    inline ~cStrTable() { if(_strings!=0) free(_strings); if(_indexarray!=0) free(_indexarray); }
    // Gets number of strings in table (index cannot be greater then this)
    inline ST_ Length() const { return _numindices; }
    // Gets total length of all strings
    inline ST_ TotalWordSize() const { return _sasize; }
    // Returns string with the corresponding index. Strings are returned null-terminated, but the index bound is not checked.
    inline const T* GetString(ST_ index) const { return _strings+_indexarray[index]; }
    // Returns index array
    inline const ST_* GetIndices() const { return _indexarray; }
    // Dumps table to stream
    inline void DumpToStream(std::ostream* stream) { stream->write((char*)_strings,_sasize*sizeof(T)); }
    inline const T* operator[](ST_ index) { return GetString(index); }
    inline cStrTable& operator=(const cStrTable& right)
    { 
      if(_strings!=0) free(_strings);
      if(_indexarray!=0) free(_indexarray);
      _sasize=right._sasize;
      _numindices=right._numindices;
      _strings=(T*)malloc(_sasize*sizeof(T)); 
      _indexarray=(ST_*)malloc(_numindices*sizeof(ST_));
      memcpy(_strings,right._strings,_sasize*sizeof(T));
      memcpy(_indexarray,right._indexarray,_numindices*sizeof(ST_));
      return *this;
    }
    inline cStrTable& operator=(cStrTable&& right)
    {
      if(_strings!=0) free(_strings);
      if(_indexarray!=0) free(_indexarray);
      _sasize=right._sasize;
      _numindices=right._numindices;
      _strings=right._strings;
      _indexarray=right._indexarray;
      right._sasize=0;
      right._numindices=0;
      right._strings=0;
      right._indexarray=0;
      return *this;
    }

    inline cStrTable& operator+=(const cStrTable& right)
    {
      T* nstrings=(T*)malloc((_sasize+right._sasize)*sizeof(T));
      memcpy(nstrings,_strings,_sasize*sizeof(T));
      memcpy(nstrings+_sasize,right._strings,right._sasize*sizeof(T));

      ST_* nindices=(ST_*)malloc((_numindices+right._numindices)*sizeof(ST_));
      memcpy(nindices,_indexarray,_numindices*sizeof(ST_));
      memcpy(nindices+_numindices,right._indexarray,right._numindices*sizeof(ST_));

      free(_strings);
      free(_indexarray);
      _strings=nstrings;
      _indexarray=nindices;
      ST_ i = _numindices; //start at old value
      ST_ byteadd = _sasize;
      _numindices+=right._numindices;
      _sasize+=right._sasize;

      for(; i < _numindices; ++i) _indexarray[i]+=byteadd; //remaps indexes

      return *this;
    }
    inline cStrTable operator+(const cStrTable& right) { return (cStrTable retval(*this))+=right; }

  protected:
    inline void BSS_FASTCALL _construct(const T* const* strings, ST_ size)
    {
      if(!strings || !size) //special handling for empty case
      {
        _numindices=0;
        _indexarray=(ST_*)malloc(sizeof(ST_));
        _indexarray[0]=0;
        _strings=(T*)malloc(1); //malloc(0) is undefined
        return;
      }

      _indexarray=(ST_*)malloc(_numindices*sizeof(ST_));
      for(ST_ i = 0; i < size; ++i) //this will cause an infinite loop if someone is dumb enough to set ST_ to an unsigned type and set size equal to -1. If you actually think you could make that mistake, stop using this class.
      {
        _indexarray[i] = STRTABLE_FUNC<T,ST_>::_lstr(strings[i])+1; //include null terminator in length count
        _sasize+=_indexarray[i];
      }
      _strings=(T*)malloc(_sasize*sizeof(T));

      ST_ curlength=0;
      ST_ hold;
      for(ST_ i = 0; i < size; ++i) //this loop does two things - it copies the strings over, and it sets the index values to the correct ones while discarding intermediate lengths
      {
        hold=_indexarray[i];
        _indexarray[i]=curlength;
        memcpy(_strings+curlength,strings[i],hold*sizeof(T)); //we could recalculate strlen here but that's dangerous because it might overwrite memory
        curlength+=hold;
      }
    }

    T* _strings;
    ST_* _indexarray;
    ST_ _sasize;
    ST_ _numindices;
  };

  //template<typename ST_>
  //inline template<class U> cStrTable<char,ST_>::cStrTable(const cStrTable<U>& copy)
  //{
  //}
    

   
}

#endif