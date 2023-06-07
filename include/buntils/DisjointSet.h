// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __DISJOINT_SET_H__BUN__
#define __DISJOINT_SET_H__BUN__

#include "Array.h"
#include "buntils.h"

namespace bun {
  // Represents a disjoint set data structure that uses path compression.
  template<typename T = size_t, typename Alloc = StandardAllocator<typename std::make_signed<T>::type>>
  class BUN_COMPILER_DLLEXPORT DisjointSet : protected ArrayBase<typename std::make_signed<T>::type, T, Alloc>
  {
  protected:
    typedef ArrayBase<typename std::make_signed<T>::type, T, Alloc> ARRAY;
    typedef typename ARRAY::Ty Ty;
    using ARRAY::_array;
    using ARRAY::_capacity;

  public:
    // Construct a disjoint set with num initial sets
    inline DisjointSet(const DisjointSet& copy) : ARRAY(copy.Capacity(), copy), _numsets(copy._numsets) 
    { 
      memcpy(_array, copy._array, _capacity); 
    }
    inline DisjointSet(DisjointSet&& mov) : ARRAY(std::move(mov)), _numsets(mov._numsets) { mov._numsets = 0; }
    inline DisjointSet(T num, const Alloc& alloc) : ARRAY(num, alloc) { Reset(); }
    inline explicit DisjointSet(T num) requires std::is_default_constructible_v<Alloc> : ARRAY(num) { Reset(); }
    inline DisjointSet(Ty* overload, T num) requires std::is_same_v<Alloc, NullAllocator<Ty>> : ARRAY(0)
    {
      _array = overload;
      _capacity = num;
      Reset();
    }
    // Union (combine) two disjoint sets into one set. Returns false if set1 or set2 aren't set names.
    inline bool Union(T set1, T set2)
    {
      if(_invalidIndex(set1) || _invalidIndex(set2))
        return false;

      Ty w1 = _array[set1];
      Ty w2 = _array[set2];

      if(w1 > w2)
      { // Weights are negated, so -w1 < -w2 can be rewritten as w1 > w2
        _array[set2] += w1;
        _array[set1] = set2;
      }
      else
      {
        _array[set1] += w2;
        _array[set2] = set1;
      }

      --_numsets;
      return true;
    }

    // Returns the set name that x belongs to.
    inline Ty Find(T x)
    {
      assert(x < _capacity);

      Ty r = x;
      while(_array[r] >= 0)
        r = _array[r]; // Find set name x belongs to

      Ty t;
      if(x != r)
      { // Do path compression
        t = _array[x];

        while(t != r)
        {
          _array[x] = r;
          x = t;
          t = _array[t];
        }
      }

      return r;
    }

    inline T Length() const { return _capacity; }
    inline T NumSets() const { return _numsets; }
    // Returns true if x is a valid set name
    inline bool IsSetName(T x) { return x < _capacity && _array[x] < 0; }
    // Returns the number of elements in a given set. Returns -1 on failure.
    inline Ty NumElements(T set) 
    { 
      if(set >= _capacity) 
        return -1; 
      return -_array[Find(set)]; // Number of elements is simply the weight of the root node
    } 
    // Adds n elements to the disjoint set.
    inline void AddSets(T n)
    {
      T i = _capacity;
      SetCapacity(_capacity + n);

      for(; i < _capacity; ++i)
        _array[i] = -1;
    }
    // Resets the disjoint set
    inline void Reset()
    {
      bun_FillN(_array, _capacity, -1); // Initialize all sets to be root nodes of trees of size 1
      _numsets = _capacity;
    }

    // Returns an array containing the elements in the given set.
    inline std::unique_ptr<T[]> GetElements(T set)
    {
      Ty len = NumElements(set);

      if(len < 0)
        return std::unique_ptr<T[]>();

      T* ret = new T[len];
      T j = GetElements(set, ret);
      assert(j <= len);
      return std::unique_ptr<T[]>(ret);
    }

    // Fills target with the elements of the given set. target must be at least NumElements(set) long. If target is null, returns NumElements(set)
    inline Ty GetElements(T set, T* target)
    {
      if(!target)
        return NumElements(set);
      set = Find(set); // Get the root element of our set
      T j = 0;

      for(T i = 0; i < _capacity; ++i)
      {
        if(Find(i) == set) // Does this element belong to our set?
          target[j++] = i; // If so, add it
      }
      return j;
    }

    // Constructs a minimum spanning tree using Kruskal's algorithm, given a sorted list of edges (smallest first).
    template<class ITER>
    inline static Array<std::pair<T, T>, T> MinSpanningTree(T numverts, ITER edges, ITER edgeslast)
    {
      Array<std::pair<T, T>, T> ret(numverts - 1); // A nice result in combinatorics tells us that all trees have exactly n-1 edges.
      ret.SetCapacity(MinSpanningTree(numverts, edges, edgeslast, ret)); // This will always be <= n-1 so the SetCapacity is basically free.
      return ret;
    }

    // Actual function definition that uses an out array that must be at least n-1 elements long.
    template<class ITER>
    static T MinSpanningTree(T numverts, ITER edges, ITER edgeslast, std::pair<T, T>* out)
    {
      VARARRAY(Ty, arr, numverts); // Allocate everything on the stack
      DisjointSet<T, NullAllocator<Ty>> set(arr, numverts);
      T num = 0;

      for(; edges != edgeslast; ++edges)
      {
        Ty a = set.Find((*edges).first);
        Ty b = set.Find((*edges).second);

        if(a != b)
        {
          out[num++] = *edges;
          bool inv = set.Union(a, b);
          assert(inv);
        }
      }

      assert(num < numverts);
      return num; // If the edges are disconnected it'll return a forest of minimum spanning trees with a number of edges less than n-1.
    }

    inline DisjointSet& operator=(const DisjointSet& copy) 
    {
      SetCapacityDiscard(copy._capacity);
      memcpy(_array, copy._array, _capacity * sizeof(T));
      _numsets = copy._numsets;
      return *this;
    }
    inline DisjointSet& operator=(DisjointSet&& mov) 
    {
      ARRAY::operator=(std::move(mov)); 
      _numsets = mov._numsets;
      return *this;
    }

  protected:
    BUN_FORCEINLINE bool _invalidIndex(T s) const { return s >= _capacity || _array[s] >= 0; }

    T _numsets;
  };
}

#endif
