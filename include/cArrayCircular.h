// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_CIRCULAR_H__BSS__
#define __C_ARRAY_CIRCULAR_H__BSS__

#include "cArraySimple.h"
#include "bss_traits.h"
#include "bss_util.h"

namespace bss_util {
  /* Simple circular array implementation */
  template<class T, typename _SizeType=unsigned int, typename ArrayType=cArraySimple<T,_SizeType>>
  class BSS_COMPILER_DLLEXPORT cArrayCircular : protected ArrayType
  {
    typedef _SizeType __ST;
    typedef typename TSignPick<sizeof(__ST)>::SIGNED __ST_SIGNED;

  public:
    inline cArrayCircular(const cArrayCircular& copy) : ArrayType(copy), _cur(copy._cur), _length(copy._length) {}
    inline cArrayCircular(cArrayCircular&& mov) : ArrayType(std::move(mov)), _cur(mov._cur), _length(mov._length) {}
    inline explicit cArrayCircular(__ST size=1) : ArrayType(size), _cur((__ST)-1), _length(0) {}
    inline ~cArrayCircular() {}
    inline BSS_FORCEINLINE void Push(const T& item) { _push<const T&>(item); }
    inline BSS_FORCEINLINE void Push(T&& item) { _push<T&&>(std::move(item)); }
    inline T Pop() { assert(_length>0); --_length; __ST prev=_cur; _cur=bssmod<__ST_SIGNED>(_cur-1,_size); return _array[prev]; }
    inline __ST Size() const { return _size; }
    inline __ST Length() const { return _length; }
    inline void SetSize(__ST nsize) //Wipes out the array
    {
      ArrayType::SetSize(nsize);
      _length=0;
      _cur=(__ST)-1;
    }

    inline T& operator[](__ST_SIGNED index) { return _array[bssmod<__ST_SIGNED>(_cur-index,_size)]; } // an index of 0 is the most recent item pushed into the circular array.
    inline const T& operator[](__ST_SIGNED index) const { return _array[bssmod<__ST_SIGNED>(_cur-index,_size)]; }
    inline cArrayCircular& operator=(const cArrayCircular& right) { ArrayType::operator=(right); _cur=right._cur; _length=right._length; return *this; }
    inline cArrayCircular& operator=(cArrayCircular&& right) { ArrayType::operator=(std::move(right)); _cur=right._cur; _length=right._length; right._length=0; right._cur=0; return *this; }
  
  protected:
    template<typename U> // Note that ++_cur can never be negative so we don't need to use bssmod there
    inline void _push(U && item) { _array[_cur=((++_cur)%_size)]=std::forward<U>(item); if(_length<_size) ++_length; }

    __ST _cur;
    __ST _length;
  };
}

#endif