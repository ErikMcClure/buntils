// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_HEAP_H__
#define __C_HEAP_H__

#include "bss_compare.h"

namespace bss_util {
  /* This is a binary min-heap implemented using an array. Use CompareKeysInverse to change it into a max-heap */
  template<class K, class D, char (*Compare)(const K& keyleft, const K& keyright)=CompareKeys<K>>
  class __declspec(dllexport) cBinaryHeap
  {
  protected:
    typedef std::pair<K,D> BINHEAP_CELL;
#define CBH_PARENT(i) ((i-1)/2)
#define CBH_LEFT(i) ((i<<1)+1)
#define CBH_RIGHT(i) ((i<<1)+2)

  public:
    inline cBinaryHeap() : _array((BINHEAP_CELL*)malloc(0)), _totalsize(0), _length(0) {}
    inline ~cBinaryHeap()
    {
      free(_array);
    }
    inline void Insert(const K& key, D data)
    {
      int k = _length;
      if(_length >= _totalsize) Expand(!_totalsize?2:(_totalsize<<1));
      ++_length;
      _array[k].first=key;
      _array[k].second=data;
      PercolateUp(k);
    }

    inline const BINHEAP_CELL& GetRoot()
    {
      return _array[0];
    }

    inline bool Empty()
    {
      return !_length;
    }

    /* Sets a key and percolates */
    inline bool SetKey(size_t index, const K& key)
    {
      if(index>=_length) return false;
      if(Compare(_array[index].first,key) >= 0) //in this case we percolate up
      {
        _array[index].first=key;
        PercolateUp(index);
      }
      else
      {
        _array[index].first=key;
        PercolateDown(index);
      }
      return true;
    }
    /* Sets data on a node */
    inline bool SetData(size_t index, D data)
    {
      if(index>=_length) return false;
      _array[index].second=data;
      return true;
    }

    /* To remove a node, we replace it with the last item in the heap and then percolate down */
    inline bool Remove(size_t index)
    {
      if(index>=_length) return false;
      _array[index]=_array[--_length];
      if(_length>0) PercolateDown(index);
      return true;
    }

    /* This allows you to check if a given value exists */
    inline const BINHEAP_CELL* GetValue(size_t index)
    {
      return index<_length?&_array[index]:0;
    }

    /* Percolate up through the heap */
    inline void PercolateUp(unsigned int k)
    {
      assert(k<_length);
      unsigned int parent;
      BINHEAP_CELL store=_array[k];

      while (k > 0) {
        parent = CBH_PARENT(k);
        if(Compare(_array[parent].first,store.first) <= 0) break;
        _array[k] = _array[parent];
        k = parent;
      }
      _array[k]=store;
    }
    /* Percolate down a heap */
    inline void PercolateDown(unsigned int k)
    {
      assert(k<_length);
      unsigned int largest;
      unsigned int left;
      unsigned int right;
      BINHEAP_CELL store=_array[k];

      while(k<_length)
      {
        left=CBH_LEFT(k);
        right=CBH_RIGHT(k);
        largest=k;
        if(left<_length && Compare(_array[left].first,store.first) <= 0) {
          largest=left;

          if(right<_length && Compare(_array[right].first,_array[largest].first) <= 0)
            largest=right;
        } else if(right<_length && Compare(_array[right].first,store.first) <= 0)
          largest=right;
        if(largest==k) break;
        _array[k]=_array[largest];
        k=largest;
      }
      _array[k]=store;
    }
    /* Expands to the given size. Size must be >= _totalsize */
    inline void Expand(unsigned int size)
    {
      assert(size>=_totalsize);
      BINHEAP_CELL* narray=(BINHEAP_CELL*)malloc(size*sizeof(BINHEAP_CELL));
      memcpy(narray,_array,sizeof(BINHEAP_CELL)*_length);
      free(_array);
      _array=narray;
      _totalsize=size;
    }
    operator BINHEAP_CELL*() { return _array; }
    operator const BINHEAP_CELL*() const { return _array; }

  protected:
    BINHEAP_CELL* _array;
    unsigned int _totalsize;
    unsigned int _length; //amount of used cells
  };

    /* This will grab the value closest to K while that value is still greater then or equal to K */
    //inline size_t GetNearest(const K& key) 
    //{
    //  BINHEAP_CELL& cur=_array[0];
    //  if(_array.empty() || Compare(cur.first,key)<0)
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
    //    if(Compare(left.first,right.first)>0) //if this is true then left > right
    //    {
    //      ++index;
    //      left=right;
    //    }
    //    else
    //      index+=2;

    //    //switch(Compare(key,left.first))
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
}

#endif