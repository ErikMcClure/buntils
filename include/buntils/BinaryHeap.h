// Copyright ©2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __HEAP_H__BUN__
#define __HEAP_H__BUN__

#include "compare.h"
#include "DynArray.h"
#include <memory>
#include <limits>

namespace bun {
  namespace internal {
    template<class T, typename CType> // This has to be a struct because the priorityheap uses it to inject a new member into the heap.
    struct MFUNC_DEFAULT { BUN_FORCEINLINE static void MFunc(const T&, CType, MFUNC_DEFAULT*) {} };
  }

  // This is a binary max-heap implemented using an array. Use CompTInv to change it into a min-heap, or to make it use pairs.
  template<class T,
    typename CType = size_t,
    char(*CFunc)(const T&, const T&) = CompT<T>,
    ARRAY_TYPE ArrayType = ARRAY_SIMPLE,
    typename Alloc = StandardAllocator<T>,
    class MFUNC = internal::MFUNC_DEFAULT<T, CType>>
  class BUN_COMPILER_DLLEXPORT BinaryHeap : protected DynArray<T, CType, ArrayType, Alloc>, protected MFUNC
  {
  protected:
    using BASE = DynArray<T, CType, ArrayType, Alloc>;
    using CT = typename BASE::CT;
    using Ty = typename BASE::Ty;
    using BASE::_array;
    using BASE::_capacity;
    using BASE::_length;
#define CBH_PARENT(i) ((i-1)/2)
#define CBH_LEFT(i) ((i<<1)+1)
#define CBH_RIGHT(i) ((i<<1)+2)

  public:
    inline BinaryHeap(const BinaryHeap& copy) = default;
    inline BinaryHeap(BinaryHeap&& mov) = default;
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline explicit BinaryHeap(typename Alloc::policy_type* policy) : BASE(0, policy) {}
    inline BinaryHeap() : BASE(0) {}
    inline BinaryHeap(const T* src, CT length) : BASE(length)
    { 
      _copy(_array, src, sizeof(T)*length);
      _length = length;
      Heapify(_array, _length);
    }
    template<CT I>
    inline explicit BinaryHeap(const T(&src)[I]) : BinaryHeap(src, I) {}
    template<CT I>
    inline explicit BinaryHeap(const std::array<T, I>& src) : BinaryHeap(src, I) {}
    inline ~BinaryHeap() {}
    inline const T& Peek() { return _array[0]; }
    inline const T& Get(CT index) { assert(index < _length); return _array[index]; }
    inline T Pop() {
      T r = std::move(_array[0]); 
      Remove(0); 
      return std::move(r);
    }
    inline bool Empty() { return !_length; }
    inline CT Length() { return _length; }
    inline void Clear() { _length = 0; }
    // Inserts a value
    inline void Insert(const T& val) { _insert(val); }
    inline void Insert(T&& val) { _insert(std::move(val)); }
    // Sets a key and percolates
    inline bool Set(CT index, const T& val) { return _set(index, val); }
    inline bool Set(CT index, T&& val) { return _set(index, std::move(val)); }
    // To remove a node, we replace it with the last item in the heap and then percolate down
    inline bool Remove(CT index)
    {
      if(index >= _length) return false; //We don't have to copy _array[_length - 1] because it stays valid during the percolation
      if(_length > 1) //We can't percolate down if there's nothing in the array! 
        PercolateDown(_array, _length - 1, index, _array[_length - 1], this);
      BASE::RemoveLast();
      return true;
    }

    // Percolate up through the heap
    template<typename U>
    static void PercolateUp(T* _array, CT _length, CT k, U && val, BinaryHeap* p = 0)
    {
      assert(k < _length);
      CT parent;

      while(k > 0)
      {
        parent = CBH_PARENT(k);
        
        if(CFunc(val, _array[parent]) < 0)
          break;

        _array[k] = std::move(_array[parent]);
        MFUNC::MFunc(_array[k], k, p);
        k = parent;
      }

      _array[k] = std::forward<U>(val);
      MFUNC::MFunc(_array[k], k, p);
    }
    // Percolate down a heap
    template<typename U>
    static void PercolateDown(T* _array, CT length, CT k, U && val, BinaryHeap* p = 0)
    {
      assert(k < length);
      assert(k < (std::numeric_limits<CT>::max() >> 1));
      CT i;

      for(i = CBH_RIGHT(k); i < length; i = CBH_RIGHT(i))
      {
        if(CFunc(_array[i - 1], _array[i]) > 0) // CFunc (left,right) and return true if left > right
          --i; //left is greater than right so pick that one

        if(CFunc(val, _array[i]) > 0)
          break;

        _array[k] = std::move(_array[i]);
        MFUNC::MFunc(_array[k], k, p);
        k = i;
        assert(k < (std::numeric_limits<CT>::max() >> 1));
      }

      if(i >= length && --i < length && CFunc(val, _array[i]) <= 0) //Check if left child is also invalid (can only happen at the very end of the array)
      {
        _array[k] = std::move(_array[i]);
        MFUNC::MFunc(_array[k], k, p);
        k = i;
      }

      _array[k] = std::forward<U>(val);
      MFUNC::MFunc(_array[k], k, p);
    }
    operator T*() { return _array; }
    operator const T*() const { return _array; }
    inline const T* begin() const { return _array; }
    inline const T* end() const { return _array + _length; }
    inline T* begin() { return _array; }
    inline T* end() { return _array + _length; }
    inline BinaryHeap& operator=(const BinaryHeap&) = default;
    inline BinaryHeap& operator=(BinaryHeap&&) = default;

    template<CT SIZE>
    inline static void Heapify(T(&src)[SIZE]) { Heapify(src, SIZE); }
    inline static void Heapify(T* src, CT length)
    {
      T store;

      for(CT i = length / 2; i > 0;)
      {
        store = src[--i];
        PercolateDown(src, length, i, store);
      }
    }
    template<CT SIZE>
    inline static void HeapSort(T(&src)[SIZE]) { HeapSort(src, SIZE); }
    inline static void HeapSort(T* src, CT length)
    {
      Heapify(src, length);
      T store;

      while(length > 1)
      {
        store = src[--length];
        src[length] = src[0];
        PercolateDown(src, length, 0, store);
      }
    }

    using BASE::SerializerArray;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { BASE::template Serialize<Engine>(s, id); }

  protected:
    template<typename U>
    inline void _insert(U && val)
    {
      BASE::_checkSize();
      new(_array + _length) T();
      CT k = _length++;
      PercolateUp(_array, _length, k, std::forward<U>(val), this);
    }

    // Sets a key and percolates
    template<typename U>
    inline bool _set(CT index, U && val)
    {
      if(index >= _length)
        return false;

      if(CFunc(_array[index], val) <= 0) //in this case we percolate up
        PercolateUp(_array, _length, index, std::forward<U>(val), this);
      else
        PercolateDown(_array, _length, index, std::forward<U>(val), this);

      return true;
    }
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
  //  CT index=0;

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
