// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_CIRCULAR_H__BSS__
#define __C_ARRAY_CIRCULAR_H__BSS__

#include "cArraySimple.h"
#include "bss_traits.h"
#include "bss_util.h"

namespace bss_util {
  /* Simple circular array implementation */
  template<class T, class Traits=ValueTraits<T>, typename _SizeType=unsigned int, typename ArrayType=cArraySimple<T,_SizeType>>
  class __declspec(dllexport) cArrayCircular : protected ArrayType, protected Traits
  {
    typedef _SizeType __ST;
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;
    typedef typename TSignPick<sizeof(__ST)>::SIGNED __ST_SIGNED;

  public:
    inline cArrayCircular(const cArrayCircular& copy) : ArrayType(copy), _cur(copy._cur), _length(copy._length) {}
    inline cArrayCircular(cArrayCircular&& mov) : ArrayType(std::move(mov)), _cur(mov._cur), _length(mov._length) {}
    inline explicit cArrayCircular(__ST size=1) : ArrayType(size), _cur((__ST)-1), _length(0) {}
    inline ~cArrayCircular() {}
    inline void Push(constref item) { _array[_cur=((++_cur)%_size)]=item; if(_length<_size) ++_length; }
    inline T Pop() { assert(_length>0); --_length; return _array[_cur=((_cur--)%_size)]; }
    inline __ST Size() const { return _size; }
    inline __ST Length() const { return _length; }
    inline void SetSize(__ST nsize) //Wipes out the array
    {
      ArrayType::SetSize(nsize);
      _length=0;
      _cur=(__ST)-1;
    }

    inline T& operator[](__ST_SIGNED index) { return _array[((__ST)(_cur-index))%_size]; } // an index of 0 is the most recent item pushed into the circular array.
    inline constref operator[](__ST_SIGNED index) const { return _array[((__ST)(_cur-index))%_size]; }
    inline cArrayCircular& operator=(const cArrayCircular& right) { ArrayType::operator=(right); _cur=right._cur; _length=right._length; return *this; }
    inline cArrayCircular& operator=(cArrayCircular&& right) { ArrayType::operator=(std::move(right)); _cur=right._cur; _length=right._length; right._length=0; right._cur=0; return *this; }
  
  protected:
    __ST _cur;
    __ST _length;
  };
}

#endif