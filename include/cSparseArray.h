// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SPARSE_ARRAY_H__BSS__
#define __C_SPARSE_ARRAY_H__BSS__

#include "cBitArray.h"

namespace bss_util {
  // Sparse array implementation that allows you to specify how sparse it is
  template<typename T, int Sparsity=4, typename ST_=unsigned int>
  class BSS_COMPILER_DLLEXPORT cSparseArray : protected cBitArray<unsigned char, ST_>
  {
  protected:
    using cBitArray<unsigned char, ST_>::_bits;
    using cBitArray<unsigned char, ST_>::_numbits;
    using cBitArray<unsigned char, ST_>::DIV_AMT;
    using cBitArray<unsigned char, ST_>::MOD_AMT;
    union SPARSE_ELEM
    {
      T _elem;
      ST_ _index;
    };

  public:
    cSparseArray(const cSparseArray& copy) : _array(0), _size(0), cBitArray<unsigned char, ST_>(copy) { _resize(copy._size); memcpy(_array, copy._array, sizeof(SPARSE_ELEM)*_size); }
    cSparseArray(ST_ init=1) : _array(0), _size(0), cBitArray<unsigned char, ST_>(init) { _resize(init); }
    ~cSparseArray() { delete [] _array; } //We always have array equal something, even if its a zero length array
    // appends something to the end of the array, pushing everything else back. This cannot fail (well, unless you run out of memory. But if you ran out of memory, boy are you fucked)
    void Add(const T item)
    {
      if(_count==_size) _expand(_size*Sparsity);
      _insert(item,_size-1);
    }
    // Inserts the item at the given index and shifts everything around
    bool Insert(const T item, ST_ index)
    {
      if(index<_size&&GetBit(index))
      {
        if(_count==_size) _expand(_size*Sparsity);
        _insert(item,index);
        return true;
      }
      return false;
    }
    // Removes the index if it exists
    bool Remove(ST_ index)
    {
      ST_ realindex=(index>>DIV_AMT), bitindex=(index&MOD_AMT);

      if(index<_size&&!!(_bits[realindex]&(1<<bitindex)))
      {
        ST_ store=-1;

        _bits[realindex] = (_bits[realindex]&(~(1<<bitindex))); //sets the bit to false
        
        if(++index>=_size) //if this is true we are the last ones on the array
          _array[--index]._index=-1;
        else
        {
          realindex=(index>>DIV_AMT);
          bitindex=(index&MOD_AMT);

          if(!(_bits[realindex]&(1<<bitindex))) //if this is true then we just take the index pointer from the next one
            _array[index-1]._index = _array[index]._index;
          else //otherwise we set our index to the next one
            _array[index-1]._index=index;

          store=_array[--index]._index; //store now holds whatever value we have
        }

        //now we run backwards until we hit a valid cell, updating all the references
        while(--index>=0 && !(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))))
          _array[index]._index=store;

        --_count;
        return true;
      }
      
      return false;
    }
    // Tells you if a given index is valid
    inline bool BSS_FASTCALL IsValid(ST_ index) { return (index<_size)?GetBit(index):false; }
    // Gets the total size of the array
    inline ST_ Size() const { return _size; }
    // Gets the number of used spots in the array
    inline ST_ Length() const { return _count; }
    // Expands the array by the sparsity amount
    inline void Expand(ST_ size) { float a = _size*Sparsity; size+=size%Sparsity; _expand((size<a)?a:size); }
    // Gets the next valid index
    inline ST_ BSS_FASTCALL GetNext(ST_ index) const { ++index; return (index>=_size)?-1:(GetBit(index)?index:_array[index]._index); }
    // Gets the previous valid index
    inline ST_ BSS_FASTCALL GetPrev(ST_ index) const
    {
      if(index>_size) return -1; //Note that this is > instead of >= because index is about to have 1 subtracted from it, so giving this a value of _size is actually a valid thing to do
      ST_ realindex, bitindex;
      while(--index>=0 && !(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))));
      return index;
    }

    const T operator[](ST_ index) const { return _array[index]._elem; }
    cSparseArray& operator=(const cSparseArray& right) { if(right._size>_size) _resize(right._size); memcpy(_array, right._array, sizeof(SPARSE_ELEM)*_size); cBitArray<char, ST_>::operator=(right); }

  protected:
    void _expand(ST_ size) //size must be at least _size*Sparsity
    {
      SPARSE_ELEM* oldarray=_array;
      _array = new SPARSE_ELEM[size];
      for(ST_ i = 0; i < _size; ++i)
        _array[i*Sparsity]=oldarray[i];
    }
    void _resize(ST_ size)
    {
      if(_array) delete [] _array;
      _array = new SPARSE_ELEM[size];
    }
    void _insert(const T item, ST_ index)
    {
      ST_ realindex=(index>>DIV_AMT), bitindex=(index&MOD_AMT);

      ST_ orig = index;
      ST_ rightindex;
      ST_ leftindex=-1; 

      while(--index >= 0)
        if(!(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))))
        {
          leftindex=index;
          break;
        }

      index=orig;
      rightindex=leftindex!=-1?index-leftindex+index:_size; //orig-result+orig
      
      while(++index < rightindex)
        if(!(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))))
          break;

      assert(leftindex!=-1 && index!=rightindex);

      if((index-orig)<(orig-leftindex)) //if this is true, going towards the end was the shorter path
      {
        _bits[realindex=(index>>DIV_AMT)] |= (1<<(bitindex=(index&MOD_AMT))); //This sets the end cell to full since we're about to put something in it

        while(--index>=orig)
          _array[index+1]=_array[index];
      }
      else //otherwise we go towards the beginning
      {
        _bits[realindex=(leftindex>>DIV_AMT)] |= (1<<(bitindex=(leftindex&MOD_AMT))); //This sets the end cell to full since we're about to put something in it

        index=leftindex; //Here we go backwards until we hit a valid cell and update all the references so they point to a valid cell
        while(--index>=0 && !(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))))
          _array[index]._index=leftindex;
        
        while(++leftindex>=orig)
          _array[leftindex-1]=_array[leftindex];
      }
      _array[orig]._elem =item;

      ++_count;
    }

    SPARSE_ELEM* _array;
    ST_ _size;
    ST_ _count; //counts the used spaces
  };
}

#endif
