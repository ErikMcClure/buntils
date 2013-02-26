// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_DISJOINT_SET_H__BSS__
#define __C_DISJOINT_SET_H__BSS__

#include "cArraySimple.h"
#include "cUniquePtr.h"
#include "bss_util.h"

namespace bss_util {
  // Represents a disjoint set data structure that uses path compression.
  template<typename T=unsigned int>
  class BSS_COMPILER_DLLEXPORT cDisjointSet : protected cArraySimple<typename TSignPick<sizeof(T)>::SIGNED,T>
  {
    typedef cArraySimple<typename TSignPick<sizeof(T)>::SIGNED,T> ARRAY;
    typedef typename ARRAY::T_ T_;
    using ARRAY::_array;
    using ARRAY::_size;

  public:
    // Construct a disjoint set with num initial sets
    inline cDisjointSet(const cDisjointSet& copy) : ARRAY(copy), _numsets(copy._numsets) {}
    inline cDisjointSet(cDisjointSet&& mov) : ARRAY(std::move(mov)), _numsets(mov._numsets) {}
    inline cDisjointSet(T num) : ARRAY(num) { Reset(); }

    // Union (combine) two disjoint sets into one set. Returns false if set1 or set2 aren't set names.
    inline bool BSS_FASTCALL Union(T set1, T set2)
    {
      if(_invalidindex(set1)||_invalidindex(set2))
        return false;

      T_ w1 = _array[set1];
      T_ w2 = _array[set2];
    
      if (w1 > w2) { // Weights are negated, so -w1 < -w2 can be rewritten as w1 > w2
        _array[set2] += w1;
        _array[set1] = set2;
      } else {
        _array[set1] += w2;
        _array[set2] = set1;
      }
    
      --_numsets;
      return true;
    }

    // Returns the set name that x belongs to.
    inline T_ BSS_FASTCALL Find(T x) 
    {
      assert(x < _size);
      
      T_ r = x;
      while(_array[r] >= 0) r = _array[r]; // Find set name x belongs to
    
      T_ t;
      if(x != r) { // Do path compression
        t = _array[x];
        while(t != r) {
          _array[x] = r;
          x = t;
          t = _array[t];
        }
      }
    
      return r;
    }

    inline T Length() const { return _size; }
    inline T NumSets() const { return _numsets; }
    // Returns true is x is a valid set name
    inline bool IsSetName(T x) { return x<_size && _array[x]<0; }
    // Returns the number of elements in a given set. Returns -1 on failure.
    inline T NumElements(T set) { if(_invalidindex(set)) return -1; return -_array[set]; } // Number of elements is simply the weight of the root node
    // Adds n elements to the disjoint set.
    inline void AddSets(T n)
    {
      T i=_size;
      SetSize(_size+n);
      for(;i<_size;++i) _array[i] = -1;
    }
    // Resets the disjoint set
    inline void Reset()
    {
      ARRAY::Scrub(-1); // Initialize all sets to be root nodes of trees of size 1
      _numsets=_size;
    }

    // Returns an array containing the elements in the given set.
    inline UqP_<T[]> GetElements(T set) 
    {
      T len = NumElements(set);
      if(len<0) return UqP_<T[]>();
      T* ret = new T[len];
      T j = 0;
    
      for(T i = 0; i < _size; ++i) 
      {
        if (find(i) == set) // Does this element belong to our set?
          ret[j++] = i; // If so, add it
      }
      assert(j<=len);

      return UqP_<T[]>(ret);
    }
  
    // Constructs a minimum spanning tree using Kruskal's algorithm, given a sorted list of edges (smallest first).
    template<class ITER>
    static typename DArray<std::pair<T,T>,T>::t BSS_FASTCALL MinSpanningTree(T numverts, ITER edges, ITER edgeslast)
    {
      typename DArray<std::pair<T,T>,T>::t ret(numverts-1); // A nice result in combinatorics tells us that all trees have exactly n-1 edges.
      ret.SetSize(MinSpanningTree(numverts,edges,edgeslast,ret)); // This will always be <= n-1 so the SetSize is basically free.
      return ret;
    }

    // Actual function definition that uses an out array that must be at least n-1 elements long.
    template<class ITER>
    static T BSS_FASTCALL MinSpanningTree(T numverts, ITER edges, ITER edgeslast, std::pair<T,T>* out)
    {
      cDisjointSet<T> set(numverts);
      T num=0;
      for(;edges != edgeslast; ++edges)
      {
        T_ a = set.Find((*edges).first);
        T_ b = set.Find((*edges).second);
        if(a!=b)
        {
          out[num++]=*edges;
          bool inv = set.Union(a,b);
          assert(inv);
        }
      }
      assert(num<numverts);
      return num; // If the edges are disconnected it'll return a forest of minimum spanning trees with a number of edges less than n-1.
    }

  protected:
    inline bool _invalidindex(T s) const { return s >= _size || _array[s] >= 0; }

    T _numsets;
  };
}

#endif
