// Copyright �2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SORT_H__BSS__
#define __C_ARRAY_SORT_H__BSS__

#include "bss_traits.h"
#include "bss_algo.h"
#include "cArraySimple.h"

namespace bss_util {
  /* Sorted dynamic array */
  template<typename T, char (*CFunc)(const T&,const T&)=CompT<T>, typename _SizeType=unsigned int, class ArrayType=cArraySimple<T,_SizeType>, class Traits=ValueTraits<T>>
  class BSS_COMPILER_DLLEXPORT cArraySort : public Traits, protected ArrayType
  {
  public:
    typedef _SizeType __ST;
    typedef typename Traits::const_reference constref;
    typedef typename Traits::reference reference;

    inline cArraySort(const cArraySort& copy) : _length(copy._length), ArrayType(copy) {} 
    inline cArraySort(cArraySort&& mov) : _length(mov._length), ArrayType(std::move(mov)) {} 
    inline explicit cArraySort(__ST size=1) : _length(0), ArrayType(size) {}
    inline ~cArraySort() { }
    inline __ST BSS_FASTCALL Insert(constref data)
    {
      if(_length>=_size) Expand(fbnext(_size));
      if(!_length) _array[_length++]=data;
      else
      {
        __ST loc = binsearch_after<T,__ST,CFunc>(_array,_length,data);
        ArrayType::_pushback(loc,(_length++)-loc,data);
        return loc;
      }
      return 0;
    }
    inline void Clear() { _length=0; }
    inline void BSS_FASTCALL Discard(unsigned int num) { _length-=((num>_length)?_length:num); }
    inline __ST BSS_FASTCALL ReplaceData(__ST index, constref data)
    {
      T swap;
      _array[index]=data;
      while(index>0 && CFunc(_array[index-1], _array[index])>0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap=_array[index];
        _array[index]=_array[index-1];
        _array[--index]=swap;
      }
      while(index<(_length-1) && CFunc(_array[index+1], _array[index])<0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap=_array[index];
        _array[index]=_array[index+1];
        _array[++index]=swap;
      }
      return index;
    }
    inline bool BSS_FASTCALL Remove(__ST index)
    {
      if(index<0||index>=_length) return false;
      ArrayType::Remove(index);
      --_length;
      return true;
    }
    inline void BSS_FASTCALL Expand(__ST newsize)
    {
      ArrayType::SetSize(newsize);
    }
    inline __ST BSS_FASTCALL Find(constref data) const
    {
      return binsearch_exact<T,__ST,CFunc>(_array,_length,data);
      //__ST retval=_findnear(data,true);
      //return ((retval!=(__ST)(-1))&&(!CFunc(_array[retval],data)))?retval:(__ST)(-1);
    }
    /* Can actually return -1 if there isn't anything in the array */
    inline __ST BSS_FASTCALL FindNear(constref data, bool before=true) const
    {
      __ST retval=(binsearch_near<T,__ST,CFunc,before?CompT_NEQ:CompT_EQ>(_array,data,0,_length)-before);
      return (retval<_length)?retval:(__ST)(-1); // This is only needed for before=false in case it returns a value outside the range.
    }
    inline bool IsEmpty() const { return !_length; }
    inline __ST Length() const { return _length; }
    inline constref operator [](__ST index) const { return _array[index]; }
    inline reference operator [](__ST index) { return _array[index]; }
    inline cArraySort& operator=(const cArraySort& right)
    { 
      ArrayType::operator=(right);
      _length=right._length;
      return *this;
    }
    inline cArraySort& operator=(cArraySort&& mov)
    {
      ArrayType::operator=(std::move(mov));
      _length=mov._length;
      return *this;
    }
  protected:
    __ST _length; //How many slots are used
  };
}

#endif