// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_ARRAY_SORT_H__BSS__
#define __C_ARRAY_SORT_H__BSS__

#include "bss_traits.h"
#include "bss_util.h"
#include <memory.h>
#include "cArraySimple.h"

namespace bss_util {
  /* Sorted dynamic array */
  template<typename T, typename CompareTraits=CompareKeysTraits<T>, typename _SizeType=unsigned int, typename ArrayType=cArraySimple<T,_SizeType>>
  class __declspec(dllexport) cArraySort : public CompareTraits, protected ArrayType
  {
  public:
    typedef _SizeType __ST;
    typedef typename CompareTraits::constref constref;
    typedef typename CompareTraits::reference reference;

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
        __ST loc = _findnear(data,false);
        ArrayType::_pushback(loc,(_length++)-loc,data);
        //(_array+_length)->~T();
        //memmove(_array+(loc+1),_array+loc,(_length-loc)*sizeof(T));
        //++_length;
        //new (_array+loc) T(data);
        //for(__ST i=_length++; i>loc; --i)
        //  _array[i]=_array[i-1];
        //_array[loc] = data;
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
      while(index>0 && Compare(_array[index-1], _array[index])>0)
      { //we do a swap here and hope that the compiler is smart enough to optimize it
        swap=_array[index];
        _array[index]=_array[index-1];
        _array[--index]=swap;
      }
      while(index<(_length-1) && Compare(_array[index+1], _array[index])<0)
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
      //(_array+index)->~T(); //call destructor on item about to be destroyed
      //memmove(_array+index,_array+(index+1),((--_length)-index)*sizeof(T));
      //new (_array+_length) T(); //call constructor on now empty last chunk
      ////while(++index<_length)
      ////  _array[index-1]=_array[index];
      --_length;
      return true;
    }
    inline void BSS_FASTCALL Expand(__ST newsize)
    {
      ArrayType::SetSize(newsize);
      //T* narray = new T[newsize];
      //memcpy(narray,_array,(_length<newsize?_length:newsize)*sizeof(T));
      //delete [] _array;
      //_array=narray;
      //_size=newsize;
    }
    inline __ST BSS_FASTCALL Find(constref data) const
    {
      __ST retval=_findnear(data,true);
      return ((retval!=(__ST)(-1))&&(!Compare(_array[retval],data)))?retval:(__ST)(-1);
    }
    /* Can actually return -1 if there isn't anything in the array */
    inline __ST BSS_FASTCALL FindNearest(constref data, bool before=true) const
    {
      __ST retval=_findnear(data,before);
      return (retval<_length)?retval:(__ST)(-1);
    }
    inline bool IsEmpty() const { return !_length; }
    inline __ST Length() const { return _length; }
    inline constref operator [](__ST index) const { return _array[index]; }
    inline reference operator [](__ST index) { return _array[index]; }
    inline cArraySort& operator=(const cArraySort& right)
    { 
      ArrayType::operator=(right);
      //if(_size<right._size)
      //{
      //  delete [] _array;
      //  T* _array = new T[right._size];
      //  _size=right._size;
      //}
      _length=right._length;
      //memcpy(_array,right._array,_length*sizeof(T));
      return *this;
    }
    inline cArraySort& operator=(cArraySort&& mov)
    {
      ArrayType::operator=(std::move(mov));
      _length=mov._length;
      return *this;
    }
  protected:
    inline __ST BSS_FASTCALL _findnear(constref data, bool before) const
    {
      if(!_length) return (__ST)(-1);
      __ST last=_length;
      __ST first=0;
      __ST retval=last>>1;
      char compres;
      for(;;) //we do not preform an equality check here because an equality will only occur once.
      {
        if((compres=Compare(data,_array[retval]))<0)
        {
          last=retval;
          retval=first+((last-first)>>1); //R = F+((L-F)/2)
          if(last==retval)
            return before?--retval:retval; //interestingly, if retval is 0, we end up with -1 which is exactly what we'd want anyway
        }
        else if(compres>0)
        {
          first=retval;
          //retval=first+((last-first)>>1); //R = F+((L-F)/2)
          retval+=(last-first)>>1;
          if(first==retval)
            return before?retval:++retval;
        }
        else //otherwise they are equal
          return retval;
      }
      return retval;
    }

    __ST _length; //How many slots are used
  };
}

#endif