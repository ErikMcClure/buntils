// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKED_ARRAY_H__BSS__
#define __C_LINKED_ARRAY_H__BSS__

#include "bss_util.h"
#include "cArray.h"

namespace bss_util {
  template<class T, typename CType>
  struct BSS_COMPILER_DLLEXPORT LINKEDNODE
  {
    T val;
    CType next;
    CType prev;
  };

  // Linked list implemented as an array.
  template<class T, typename CType = size_t, ARRAY_TYPE ArrayType = CARRAY_SIMPLE, typename Alloc=StaticAllocPolicy<LINKEDNODE<T, CType>>>
  class BSS_COMPILER_DLLEXPORT cLinkedArray
  {
  public:
    typedef LINKEDNODE<T, CType> TLNODE;
    typedef CType CT_;
    typedef T value_type;

    inline cLinkedArray() : _ref(1), _length(0), _start(-1), _end(-1), _freelist(-1) { _setupchunk(0); }
    inline cLinkedArray(const cLinkedArray& copy) : _ref(copy._ref), _length(copy._length), _start(copy._start), _end(copy._end), _freelist(copy._freelist) { }
    inline cLinkedArray(cLinkedArray&& mov) : _ref(std::move(mov._ref)), _length(mov._length), _start(mov._start), _end(mov._end), _freelist(mov._freelist) { mov._ref=1; mov._length=0; mov._start=mov._end=-1=mov._freelist=-1; }
    inline explicit cLinkedArray(CT_ size) : _ref(size), _length(0), _start(-1), _end(-1), _freelist(-1) { _setupchunk(0); }
    inline ~cLinkedArray() {}
    BSS_FORCEINLINE CT_ Add(const T& item) { return InsertAfter(item, _end); }
    BSS_FORCEINLINE CT_ Add(T&& item) { return InsertAfter(std::move(item), _end); }
    BSS_FORCEINLINE CT_ InsertBefore(const T& item, CT_ index) { return _insertbefore<const T&>(item, index); }
    BSS_FORCEINLINE CT_ InsertBefore(T&& item, CT_ index) { return _insertbefore<T&&>(std::move(item), index); }
    BSS_FORCEINLINE CT_ InsertAfter(const T& item, CT_ index) { return _insertafter<const T&>(item, index); }
    BSS_FORCEINLINE CT_ InsertAfter(T&& item, CT_ index) { return _insertafter<T&&>(std::move(item), index); }
    T Remove(CT_ index)
    {
      assert(index<_ref.Capacity());
      TLNODE& pcur=_ref[index];
      if(pcur.next==(CT_)-1) _end=pcur.prev; //you're the end, reassign
      else _ref[pcur.next].prev=pcur.prev;
      if(pcur.prev==(CT_)-1) _start=pcur.next; //you're the root, reassign
      else _ref[pcur.prev].next=pcur.next;

      _addfreelist(index);
      --_length;
      return pcur.val;
    }
    inline CT_ Length() const { return _length; }
    inline CT_ Capacity() const { return _ref.Capacity(); }
    inline void Reserve(CT_ size) { if(size<_ref.Capacity()) return; CT_ hold=_ref.Capacity(); _ref.SetCapacity(size); _setupchunk(hold); }
    inline void Clear() { _freelist=_start=_end=-1; _length=0; _setupchunk(0); }
    BSS_FORCEINLINE void Next(CT_& ref) const { ref=_ref[ref].next; }
    BSS_FORCEINLINE void Prev(CT_& ref) const { ref=_ref[ref].prev; }
    BSS_FORCEINLINE CT_ Front() const { return _start; }
    BSS_FORCEINLINE CT_ Back() const { return _end; }
    //inline cLinkedArray& operator +=(const cLinkedArray& right);
    //inline cLinkedArray operator +(const cLinkedArray& right) const { cLinkedArray retval(*this); retval+=right; return retval; }
    inline cLinkedArray& operator =(const cLinkedArray& right) { _ref=right._ref; _length=right._length; _start=right._start; _end=right._end; _freelist=right._freelist; return *this; }
    inline cLinkedArray& operator =(cLinkedArray&& mov) { _ref=std::move(mov._ref); _length=mov._length; _start=mov._start; _end=mov._end; _freelist=mov._freelist; return *this; }
    BSS_FORCEINLINE T& GetItem(CT_ index) { return _ref[index].val; }
    BSS_FORCEINLINE const T& GetItem(CT_ index) const { return _ref[index].val; }
    BSS_FORCEINLINE T* GetItemPtr(CT_ index) { return &_ref[index].val; }
    BSS_FORCEINLINE T& operator [](CT_ index) { return GetItem(index); }
    BSS_FORCEINLINE const T& operator [](CT_ index) const { return GetItem(index); }

    // Iterator for cLinkedArray
    template<bool ISCONST, typename U = typename std::conditional<ISCONST, typename std::add_const<T>::type, T>::type, typename D = typename std::conditional<ISCONST, typename std::add_const<cLinkedArray>::type, cLinkedArray>::type>
    class BSS_COMPILER_DLLEXPORT cLAIter : public std::iterator<std::bidirectional_iterator_tag, typename D::value_type, ptrdiff_t, U*, U&>
    {
    public:
      inline explicit cLAIter(D& src) : _src(src), cur((CT_)-1) {}
      inline cLAIter(D& src, CT_ start) : _src(src), cur(start) {}
      inline cLAIter(const cLAIter& copy, CT_ start) : _src(copy._src), cur(start) {}
      BSS_FORCEINLINE U& operator*() const { return _src[cur]; }
      BSS_FORCEINLINE U* operator->() const { return &_src[cur]; }
      BSS_FORCEINLINE cLAIter& operator++() { _src.Next(cur); return *this; } //prefix
      BSS_FORCEINLINE cLAIter operator++(int) { cLAIter r=*this; ++*this; return r; } //postfix
      BSS_FORCEINLINE cLAIter& operator--() { _src.Prev(cur); return *this; } //prefix
      BSS_FORCEINLINE cLAIter operator--(int) { cLAIter r=*this; --*this; return r; } //postfix
      BSS_FORCEINLINE bool operator==(const cLAIter& _Right) const { return (cur == _Right.cur); }
      BSS_FORCEINLINE bool operator!=(const cLAIter& _Right) const { return (cur != _Right.cur); }
      BSS_FORCEINLINE bool operator!() const { return cur==(CT_)-1; }
      BSS_FORCEINLINE bool IsValid() { return cur!=(CT_)-1; }
      BSS_FORCEINLINE void Remove() { _src.Remove(cur); } // Only use this in the form (i++).Remove(), so you don't blow up the iterator.

      CT_ cur;

    protected:
      D& _src;
    };

    inline cLAIter<true> begin() const { return cLAIter<true>(*this, _start); } // Use these to get an iterator you can use in standard containers
    inline cLAIter<true> end() const { return cLAIter<true>(*this); }
    inline cLAIter<false> begin() { return cLAIter<false>(*this, _start); } // Use these to get an iterator you can use in standard containers
    inline cLAIter<false> end() { return cLAIter<false>(*this); }

  protected:
    template<typename U>
    CT_ _insertafter(U && item, CT_ index)
    {
      CT_ cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.val=std::forward<U>(item);
      pcur.next=(CT_)-1;
      pcur.prev=(index==(CT_)-1)?_end:index;

      if(pcur.prev==(CT_)-1) _start=cur;
      else { pcur.next=_ref[pcur.prev].next; _ref[pcur.prev].next=cur; }
      if(pcur.next==(CT_)-1) _end=cur;
      else _ref[pcur.next].prev=cur;

      ++_length;
      return cur;
    }
    template<typename U>
    CT_ _insertbefore(U && item, CT_ index)
    {
      CT_ cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.val=std::forward<U>(item);
      pcur.next=index;
      pcur.prev=(index==(CT_)-1)?_end:(CT_)-1; //This allows one to insert after the end with this function

      if(pcur.next==(CT_)-1) _end=cur;
      else { pcur.prev=_ref[pcur.next].prev; _ref[pcur.next].prev=cur; }
      if(pcur.prev==(CT_)-1) _start=cur;
      else { _ref[pcur.prev].next=cur; }

      ++_length;
      return cur;
    }
    inline void _addfreelist(CT_ index)
    {
      _ref[index].next=_freelist;
      _freelist=index;
    }
    inline void _setupchunk(CT_ start)
    {
      CT_ i=_ref.Capacity();
      while(i>start) //this trick lets us run backwards without worrying about the unsigned problem.
        _addfreelist(--i);
    }
    inline CT_ _insprep()
    {
      if(_freelist==(CT_)-1) Reserve(fbnext(_ref.Capacity()));
      CT_ cur=_freelist;
      _freelist=_ref[_freelist].next; //increment freelist
      return cur;
    }

    CT_ _length;
    CT_ _start;
    CT_ _end;
    CT_ _freelist;

  private:
    cArray<LINKEDNODE<T, CType>, CType, ArrayType, Alloc> _ref;
  };
}

#endif
