// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __HEAP_H__BUN__
#define __HEAP_H__BUN__

#include "compare.h"
#include "DynArray.h"
#include <memory>
#include <limits>

namespace bun {
  namespace internal {
    // This has to be a struct because the priorityheap uses it to inject a new member into the heap.
    template<class T, typename CType> struct MFUNC_DEFAULT
    {
      BUN_FORCEINLINE static void MFunc(const T&, CType, MFUNC_DEFAULT*) {}
    };
  }

  // This is a binary max-heap implemented using an array. Use inv_three_way to change it into a min-heap, or to make it use pairs.
  template<class T, Comparison<T, T> Comp = std::compare_three_way, typename CType = size_t,
           typename Alloc = StandardAllocator<T>,
           class MFUNC = internal::MFUNC_DEFAULT<T, CType>>
  class BUN_COMPILER_DLLEXPORT BUN_EMPTY_BASES BinaryHeap :
    private Comp,
    protected MFUNC,
    protected DynArray<T, CType, Alloc>
  {
  protected:
    using BASE = DynArray<T, CType, Alloc>;
    using CT   = typename BASE::CT;
    using Ty   = typename BASE::Ty;
    using BASE::_array;
    using BASE::_length;
#define CBH_PARENT(i) ((i - 1) / 2)
#define CBH_LEFT(i)   ((i << 1) + 1)
#define CBH_RIGHT(i)  ((i << 1) + 2)

    [[nodiscard]] constexpr BUN_FORCEINLINE const Comp& _getcomp() const noexcept { return *this; }

  public:
    inline BinaryHeap(const BinaryHeap& copy) = default;
    inline BinaryHeap(BinaryHeap&& mov)       = default;
    inline explicit BinaryHeap(const Alloc& alloc, const Comp& f, CT length = 0) : Comp(f), BASE(length, alloc) {}
    inline explicit BinaryHeap(const Alloc& alloc, CT length = 0)
      requires std::is_default_constructible_v<Comp>
      : BinaryHeap(alloc, Comp(), length)
    {}
    inline explicit BinaryHeap(const Comp& f, CT length = 0)
      requires std::is_default_constructible_v<Alloc>
      : BinaryHeap(Alloc(), f, length)
    {}
    inline BinaryHeap()
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : BinaryHeap(Alloc(), 0)
    {}
    inline explicit BinaryHeap(std::span<T> src)
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : BASE(src.size())
    {
      _copy(_array, src, sizeof(T) * src.size());
      _length = src.size();
      Heapify(src);
    }
    template<CT I> inline explicit BinaryHeap(const T (&src)[I]) : BinaryHeap(src, I) {}
    template<CT I> inline explicit BinaryHeap(const std::array<T, I>& src) : BinaryHeap(src, I) {}
    inline ~BinaryHeap() {}
    inline const T& Peek() { return _array[0]; }
    inline const T& Get(CT index)
    {
      assert(index < _length);
      return _array[index];
    }
    inline T Pop()
    {
      T r = std::move(_array[0]);
      Remove(0);
      return std::move(r);
    }
    inline bool Empty() { return !_length; }
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
      if(index >= _length)
        return false; // We don't have to copy _array[_length - 1] because it stays valid during the percolation
      if(_length > 1) // We can't percolate down if there's nothing in the array!
        PercolateDown(_array.subspan(0, _length - 1), index, _array[_length - 1], _getcomp(), this);
      BASE::RemoveLast();
      return true;
    }

    // Percolate up through the heap
    template<typename U> static void PercolateUp(std::span<T> a, CT k, U&& val, const Comp& f, BinaryHeap* p = nullptr)
    {
      assert(k < a.size());
      CT parent;

      while(k > 0)
      {
        parent = CBH_PARENT(k);

        if(f(static_cast<const T&>(val), a[parent]) < 0)
          break;

        a[k] = std::move(a[parent]);
        MFUNC::MFunc(a[k], static_cast<CType>(k), p);
        k = parent;
      }

      a[k] = std::forward<U>(val);
      MFUNC::MFunc(a[k], static_cast<CType>(k), p);
    }
    // Percolate down a heap
    template<typename U>
    static void PercolateDown(std::span<T> a, size_t k, U&& val, const Comp& f, BinaryHeap* p = nullptr)
    {
      assert(k < a.size());
      assert(k < (std::numeric_limits<CT>::max() >> 1));
      size_t i;

      for(i = CBH_RIGHT(k); i < a.size(); i = CBH_RIGHT(i))
      {
        if(f(a[i - 1], a[i]) > 0) // f (left,right) and return true if left > right
          --i;                                       // left is greater than right so pick that one

        if(f(static_cast<const T&>(val), a[i]) > 0)
          break;

        a[k] = std::move(a[i]);
        MFUNC::MFunc(a[k], static_cast<CType>(k), p);
        k = i;
        assert(k < (std::numeric_limits<CT>::max() >> 1));
      }

      if(i >= a.size() && --i < a.size() &&
         f(static_cast<const T&>(val), a[i]) <=
           0) // Check if left child is also invalid (can only happen at the very end of the array)
      {
        a[k] = std::move(a[i]);
        MFUNC::MFunc(a[k], static_cast<CType>(k), p);
        k = i;
      }

      a[k] = std::forward<U>(val);
      MFUNC::MFunc(a[k], static_cast<CType>(k), p);
    }
    inline const T* begin() const { return BASE::begin(); }
    inline const T* end() const { return BASE::end(); }
    inline T* begin() { return BASE::begin(); }
    inline T* end() { return BASE::end(); }
    inline CT size() { return _length; }
    inline T* data() { return BASE::data(); }
    inline const T* data() const { return BASE::data(); }

    inline BinaryHeap& operator=(const BinaryHeap&) = default;
    inline BinaryHeap& operator=(BinaryHeap&&)      = default;

    inline static void Heapify(std::span<T> src, const Comp& f)
    {
      for(size_t i = src.size() / 2; i > 0;)
      {
        T store = src[--i];
        PercolateDown(src, i, store, f);
      }
    }
    inline static void HeapSort(std::span<T> src, const Comp& f)
    {
      Heapify(src, f);
      auto length = src.size();

      while(length > 1)
      {
        T store       = src[--length];
        src[length] = src[0];
        PercolateDown(src.subspan(0, length), 0, store, f);
      }
    }

    using BASE::SerializerArray;
    template<typename Engine> void Serialize(Serializer<Engine>& s, const char* id)
    {
      BASE::template Serialize<Engine>(s, id);
    }

  protected:
    template<typename U> inline void _insert(U&& val)
    {
      BASE::_checkSize();
      new(_array.data() + _length) T();
      CT k = _length++;
      PercolateUp(_array.subspan(0, _length), k, std::forward<U>(val), _getcomp(), this);
    }

    // Sets a key and percolates
    template<typename U> inline bool _set(CT index, U&& val)
    {
      if(index >= _length)
        return false;

      if(_getcomp()(_array[index], static_cast<const T&>(val)) <= 0) // in this case we percolate up
        PercolateUp(_array.subspan(0, _length), index, std::forward<U>(val), _getcomp(), this);
      else
        PercolateDown(_array.subspan(0, _length), index, std::forward<U>(val), _getcomp(), this);

      return true;
    }
  };

  // This will grab the value closest to K while that value is still greater then or equal to K
  // inline size_t GetNearest(const K& key)
  //{
  //  BINHEAP_CELL& cur=_array[0];
  //  if(_array.empty() || f(cur.first,key)<0)
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
  //    if(_getcomp()(left.first,right.first)>0) //if this is true then left > right
  //    {
  //      ++index;
  //      left=right;
  //    }
  //    else
  //      index+=2;
  //
  //    //auto comp = f(key, left.first);
  //    //if (comp > 0) {
  //    //  cur=left;
  //    //  break;
  //    //} else if (comp < 0) { //if this is true we went too far
  //    //  return cur.second;
  //    //} else {
  //    //  return left.second;
  //    //}
  //  }
  //}

  /*while(k<_length)
  {
  left=CBH_LEFT(k);
  right=CBH_RIGHT(k);
  largest=k;
  if(left<_length && f(_array[left].first,store.first) <= 0) {
  largest=left;

  if(right<_length && f(_array[right].first,_array[largest].first) <= 0)
  largest=right;
  } else if(right<_length && f(_array[right].first,store.first) <= 0)
  largest=right;
  if(largest==k) break;
  _array[k]=_array[largest];
  k=largest;
  }
  _array[k]=store;*/
}

#endif
