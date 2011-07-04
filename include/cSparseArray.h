// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_SPARSE_ARRAY_H__BSS__
#define __C_SPARSE_ARRAY_H__BSS__

#include "cBitArray.h"

namespace bss_util {
  /* Sparse array implementation that allows you to specify how sparse it is */
  template<typename T, int Sparsity=4, typename _ST=unsigned int>
  class __declspec(dllexport) cSparseArray : protected cBitArray<unsigned char, _ST>
  {
  protected:
    union SPARSE_ELEM
    {
      T _elem;
      _ST _index;
    };

  public:
    cSparseArray(const cSparseArray& copy) : _array(0), _size(0), cBitArray<unsigned char, _ST>(copy) { _resize(right._size); memcpy(_array, right._array, sizeof(SPARSE_ELEM)*_size); }
    cSparseArray(__ST init=1) : _array(0), _size(0), cBitArray<unsigned char, _ST>(init) { _resize(init); }
    ~cSparseArray() { delete [] _array; } //We always have array equal something, even if its a zero length array
    /* appends something to the end of the array, pushing everything else back. This cannot fail (well, unless you run out of memory. But if you ran out of memory, boy are you fucked) */
    void Add(const T item)
    {
      if(_count==_size) _expand(_size*Sparsity);
      _insert(item,_size-1);
    }
    /* Inserts the item at the given index and shifts everything around */
    bool Insert(const T item, __ST index)
    {
      if(index<_size&&GetBit(index))
      {
        if(_count==_size) _expand(_size*Sparsity);
        _insert(item,index);
        return true;
      }
      return false;
    }
    /* Removes the index if it exists */
    bool Remove(__ST index)
    {
      __ST realindex=(index>>DIV_AMT), bitindex=(index&MOD_AMT);

      if(index<_size&&!!(_bits[realindex]&(1<<bitindex)))
      {
        __ST store=-1;

        _bits[realindex] = (_bits[realindex]&(~(1<<bitindex))); //sets the bit to false
        
        if(++index>=_size) //if this is true we are the last ones on the array
          _array[--index]._index=-1;
        else
        {
          realindex=(index>>DIV_AMT);
          bitindex=(index&MOD_AMT);

          if(!(_bits[realindex]&(1<<bitindex))) //if this is true then we just take the index pointer from the next one
            _array[index-1]._index = _array[index]._index
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
    /* Tells you if a given index is valid */
    inline bool BSS_FASTCALL IsValid(__ST index) { return (index<_size)?GetBit(index):false; }
    /* Gets the total size of the array */
    inline __ST Size() const { return _size; }
    /* Gets the number of used spots in the array */
    inline __ST Length() const { return _count; }
    /* Expands the array by the sparsity amount */
    inline void Expand(__ST size) { float a = _size*Sparsity; size+=size%Sparsity; _expand((size<a)?a:size); }
    /* Gets the next valid index */
    inline __ST BSS_FASTCALL GetNext(__ST index) const { ++index; return (index>=_size)?-1:(GetBit(index)?index:_array[index]._index); }
    /* Gets the previous valid index */
    inline __ST BSS_FASTCALL GetPrev(__ST index) const
    {
      if(index>_size) return -1; //Note that this is > instead of >= because index is about to have 1 subtracted from it, so giving this a value of _size is actually a valid thing to do
      __ST realindex, bitindex;
      while(--index>=0 && !(_bits[realindex=(index>>DIV_AMT)]&(1<<(bitindex=(index&MOD_AMT)))));
      return index;
    }

    const T operator[](__ST index) const { return _array[index]._elem; }
    cSparseArray& operator=(const cSparseArray& right) { if(right._size>_size) _resize(right._size); memcpy(_array, right._array, sizeof(SPARSE_ELEM)*_size); cBitArray<char, _ST>::operator=(right); }

  protected:
    void _expand(__ST size) //size must be at least _size*Sparsity
    {
      SPARSE_ELEM* oldarray=_array;
      _array = new SPARSE_ELEM[size];
      for(__ST i = 0; i < _size; ++i)
        _array[i*Sparsity]=oldarray[i];
    }
    void _resize(__ST size)
    {
      if(_array) delete [] _array;
      _array = new SPARSE_ELEM[size];
    }
    void _insert(const T item, __ST index)
    {
      __ST realindex=(index>>DIV_AMT), bitindex=(index&MOD_AMT);

      __ST orig = index;
      __ST rightindex;
      __ST leftindex=-1; 

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
    __ST _size;
    __ST _count; //counts the used spaces
  };
}

#endif