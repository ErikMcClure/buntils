// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_CIRCULAR_H__BSS__
#define __C_ARRAY_CIRCULAR_H__BSS__

#include "cArraySimple.h"
#include "bss_util.h"

namespace bss_util {
  // Simple circular array implementation
  template<class T, typename SizeType=unsigned int, typename ArrayType=cArraySimple<T,SizeType>>
  class BSS_COMPILER_DLLEXPORT cArrayCircular : protected ArrayType
  {
    typedef SizeType ST_;
    typedef typename TSignPick<sizeof(ST_)>::SIGNED __ST_SIGNED;

  public:
    // Constructors
    inline cArrayCircular(const cArrayCircular& copy) : ArrayType(copy), _cur(copy._cur), _length(copy._length) {}
    inline cArrayCircular(cArrayCircular&& mov) : ArrayType(std::move(mov)), _cur(mov._cur), _length(mov._length) {}
    inline explicit cArrayCircular(ST_ size=0) : ArrayType(size), _cur((ST_)-1), _length(0) {}
    
    inline ~cArrayCircular() {}
    inline BSS_FORCEINLINE void Push(const T& item) { _push<const T&>(item); }
    inline BSS_FORCEINLINE void Push(T&& item) { _push<T&&>(std::move(item)); }
    inline T Pop() { assert(_length>0); --_length; ST_ prev=_cur; _cur=bssmod<__ST_SIGNED>(_cur-1,_size); return std::move(_array[prev]); }
    inline T PopBack() { assert(_length>0); --_length; return std::move(_array[_modindex(_length)]); }
    inline const T& Front() const { assert(_length>0); return _array[_cur]; }
    inline T& Front() { assert(_length>0); return _array[_cur]; }
    inline const T& Back() const { assert(_length>0); return _array[bssmod<__ST_SIGNED>(_length-1)]; }
    inline T& Back() { assert(_length>0); return _array[bssmod<__ST_SIGNED>(_length-1)]; }
    inline ST_ Size() const { return _size; }
    inline ST_ Length() const { return _length; }
    inline void SetSize(ST_ nsize) // Will preserve the array but only if it got larger
    {
      ST_ sz=_size;
      ArrayType::SetSize(nsize);
      __ST_SIGNED c=_cur+1; // We don't want to normalize this becuase its important it equals sz if _cur=sz-1
      if(sz<nsize)
        ArrayType::_mvarray(c,c+_size-sz,sz-c);
    }
    inline T& operator[](__ST_SIGNED index) { return _array[_modindex(index)]; } // an index of 0 is the most recent item pushed into the circular array.
    inline const T& operator[](__ST_SIGNED index) const { return _array[_modindex(index)]; }
    inline cArrayCircular& operator=(const cArrayCircular& right) { ArrayType::operator=(right); _cur=right._cur; _length=right._length; return *this; }
    inline cArrayCircular& operator=(cArrayCircular&& right) { ArrayType::operator=(std::move(right)); _cur=right._cur; _length=right._length; right._length=0; right._cur=0; return *this; }
  
  protected:
    template<typename U> // Note that ++_cur can never be negative so we don't need to use bssmod there
    inline void _push(U && item) { _array[_cur=((++_cur)%_size)]=std::forward<U>(item); if(_length<_size) ++_length; }
    inline __ST_SIGNED _modindex(__ST_SIGNED index) { return bssmod<__ST_SIGNED>(_cur-index,_size); }

    ST_ _cur;
    ST_ _length;
  };
}

#endif
