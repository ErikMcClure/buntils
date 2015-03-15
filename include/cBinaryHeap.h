// Copyright ©2015 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HEAP_H__BSS__
#define __C_HEAP_H__BSS__

#include "bss_compare.h"
#include "cArray.h"
#include <memory>
#include <limits>

namespace bss_util {
  // This is a binary max-heap implemented using an array. Use CompTInv to change it into a min-heap, or to make it use pairs.
  template<class T, typename ST_=unsigned int, char(*CFunc)(const T&, const T&)=CompT<T>, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<T>>
  class BSS_COMPILER_DLLEXPORT cBinaryHeap : protected cArrayBase<T, ST_, ArrayType, Alloc>
  {
  protected:
    typedef cArrayBase<T, ST_, ArrayType, Alloc> AT_;
    using AT_::_array;
    using AT_::_size;
#define CBH_PARENT(i) ((i-1)/2)
#define CBH_LEFT(i) ((i<<1)+1)
#define CBH_RIGHT(i) ((i<<1)+2)

  public:
    inline cBinaryHeap(const cBinaryHeap& copy) : AT_(copy), _length(copy._length) {}
    inline cBinaryHeap(cBinaryHeap&& mov) : AT_(std::move(mov)), _length(mov._length) { mov._length=0; }
    inline cBinaryHeap() : AT_(0), _length(0) {}
    inline cBinaryHeap(const T* src, ST_ length) : AT_(length), _length(length) { memcpy(_array, src, sizeof(T)*_length); Heapify(_array, _length); }
    template<ST_ SIZE>
    inline cBinaryHeap(const T(&src)[SIZE]) : AT_(SIZE), _length(SIZE) { memcpy(_array, src, sizeof(T)*_length); Heapify(_array, _length); }
    inline ~cBinaryHeap() {}
    inline const T& Peek() { return _array[0]; }
    inline const T& Get(ST_ index) { assert(index<_length); return _array[index]; }
    inline T Pop() { T r=std::move(_array[0]); Remove(0); return std::move(r); }
    inline bool Empty() { return !_length; }
    inline ST_ Length() { return _length; }
    inline void Clear() { _length=0; }
    // Inserts a value
    inline void Insert(const T& val) { _insert(val); }
    inline void Insert(T&& val) { _insert(std::move(val)); }
    // Sets a key and percolates
    inline bool Set(ST_ index, const T& val) { return _set(index, val); }
    inline bool Set(ST_ index, T&& val) { return _set(index, std::move(val)); }
    // To remove a node, we replace it with the last item in the heap and then percolate down
    inline bool Remove(ST_ index)
    {
      if(index>=_length) return false;
      --_length; //We don't have to copy _array[--_length] because it stays valid during the percolation
      if(_length>0) //We can't percolate down if there's nothing in the array! 
        PercolateDown(_array, _length, index, _array[_length]);
      return true;
    }

    // Percolate up through the heap
    template<typename U>
    static void PercolateUp(T* _array, ST_ _length, ST_ k, U && val)
    {
      assert(k<_length);
      unsigned int parent;

      while(k > 0) {
        parent = CBH_PARENT(k);
        if(CFunc(_array[parent], std::forward<U>(val)) > 0) break;
        _array[k] = _array[parent];
        k = parent;
      }
      _array[k]=std::forward<U>(val);
    }
    // Percolate down a heap
    template<typename U>
    static void PercolateDown(T* _array, ST_ _length, ST_ k, U && val)
    {
      assert(k<_length);
      assert(k<(std::numeric_limits<ST_>::max()>>1));
      ST_ i;

      for(i = CBH_RIGHT(k); i < _length; i = CBH_RIGHT(i))
      {
        if(CFunc(_array[i-1], _array[i]) > 0) // CFunc (left,right) and return true if left > right
          --i; //left is greater than right so pick that one
        if(CFunc(std::forward<U>(val), _array[i]) > 0)
          break;
        _array[k]=std::move(_array[i]);
        k=i;
        assert(k<(std::numeric_limits<ST_>::max()>>1));
      }
      if(i >= _length && --i < _length && CFunc(std::forward<U>(val), _array[i])<=0) //Check if left child is also invalid (can only happen at the very end of the array)
      {
        _array[k]=std::move(_array[i]);
        k=i;
      }
      _array[k]=std::forward<U>(val);
    }
    operator T*() { return _array; }
    operator const T*() const { return _array; }
    inline const T* begin() const { return _array; }
    inline const T* end() const { return _array+_length; }
    inline T* begin() { return _array; }
    inline T* end() { return _array+_length; }
    inline cBinaryHeap& operator=(const cBinaryHeap& copy) { _length=copy._length; AT_::operator=(copy); return *this; }
    inline cBinaryHeap& operator=(cBinaryHeap&& mov) { _length=mov._length; AT_::operator=(std::move(mov)); mov._length=0; return *this; }

    template<ST_ SIZE>
    inline static void Heapify(T(&src)[SIZE]) { Heapify(src, SIZE); }
    inline static void Heapify(T* src, ST_ length)
    {
      T store;
      for(ST_ i = length/2; i>0;)
      {
        store=src[--i];
        PercolateDown(src, length, i, store);
      }
    }
    template<ST_ SIZE>
    inline static void HeapSort(T(&src)[SIZE]) { HeapSort(src, SIZE); }
    inline static void HeapSort(T* src, ST_ length)
    {
      Heapify(src, length);
      T store;
      while(length>1)
      {
        store=src[--length];
        src[length]=src[0];
        PercolateDown(src, length, 0, store);
      }
    }

  protected:
    template<typename U>
    inline void _insert(U && val)
    {
      int k = _length;
      if(_length >= _size) AT_::SetSize(fbnext(_size));
      PercolateUp(_array, _length, _length++, std::forward<U>(val));
    }

    // Sets a key and percolates
    template<typename U>
    inline bool _set(ST_ index, U && val)
    {
      if(index>=_length) return false;
      if(CFunc(_array[index], std::forward<U>(val)) <= 0) //in this case we percolate up
        PercolateUp(_array, _length, index, std::forward<U>(val));
      else
        PercolateDown(_array, _length, index, std::forward<U>(val));
      return true;
    }

    ST_ _length; //amount of used cells
  };

  // This will grab the value closest to K while that value is still greater then or equal to K
  //inline size_t GetNearest(const K& key) 
  //{
  //  BINHEAP_CELL& cur=_array[0];
  //  if(_array.empty() || CFunc(cur.first,key)<0)
  //    return 0;
  //  //const BINHEAP_CELL& last=BINHEAP_CELL(key(),0);
  //  BINHEAP_CELL& right=cur;
  //  BINHEAP_CELL& left=cur;
  //  int index=0;

  //  while(true)
  //  {
  //    //last=cur;
  //    left=_array[index*=2 + 1];
  //    right=_array[index + 2];
  //    if(CFunc(left.first,right.first)>0) //if this is true then left > right
  //    {
  //      ++index;
  //      left=right;
  //    }
  //    else
  //      index+=2;

  //    //switch(CFunc(key,left.first))
  //    //{
  //    //case 1: //this is a valid choice
  //    //  cur=left;
  //    //  break;
  //    //case 0: //we have a winner
  //    //  return left.second;
  //    //case -1: //if this is true we went too far
  //    //  return cur.second;
  //    //}
  //  }
  //}

  /*while(k<_length)
  {
  left=CBH_LEFT(k);
  right=CBH_RIGHT(k);
  largest=k;
  if(left<_length && CFunc(_array[left].first,store.first) <= 0) {
  largest=left;

  if(right<_length && CFunc(_array[right].first,_array[largest].first) <= 0)
  largest=right;
  } else if(right<_length && CFunc(_array[right].first,store.first) <= 0)
  largest=right;
  if(largest==k) break;
  _array[k]=_array[largest];
  k=largest;
  }
  _array[k]=store;*/
}

#endif
