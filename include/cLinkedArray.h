// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKED_ARRAY_H__BSS__
#define __C_LINKED_ARRAY_H__BSS__

#include "bss_traits.h"
#include "bss_util.h"
#include "cArraySimple.h"
#include <xutility>

namespace bss_util {
  template<class T,typename SizeType>
  struct BSS_COMPILER_DLLEXPORT LINKEDNODE
  {
    T val;
    SizeType next;
    SizeType prev;
  };

  /* Linked list implemented as an array. */
  template<class T, class Traits=ValueTraits<T>, typename SizeType=unsigned int>
  class BSS_COMPILER_DLLEXPORT cLinkedArray : protected Traits
  {
  public:
    typedef LINKEDNODE<T,SizeType> TLNODE;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::value_type value_type;
    typedef SizeType __ST;
  
    inline cLinkedArray() : _ref(1),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline cLinkedArray(const cLinkedArray& copy) : _ref(copy._ref),_length(copy._length),_start(copy._start),_end(copy._end),_freelist(copy._freelist) { }
    inline cLinkedArray(cLinkedArray&& mov) : _ref(std::move(mov._ref)),_length(mov._length),_start(mov._start),_end(mov._end),_freelist(mov._freelist) { }
    inline explicit cLinkedArray(__ST size) : _ref(size),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline ~cLinkedArray() {}
    inline BSS_FORCEINLINE __ST BSS_FASTCALL Add(const_reference item) { return InsertAfter(item,_end); }
    inline __ST BSS_FASTCALL InsertAfter(const_reference item, __ST index)
    {
      __ST cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.next=(__ST)-1;
      pcur.prev=(index==(__ST)-1)?_end:index;;
      pcur.val=item;

      if(pcur.prev==(__ST)-1) _start=cur;
      else { pcur.next=_ref[pcur.prev].next; _ref[pcur.prev].next=cur; }
      if(pcur.next==(__ST)-1) _end=cur;
      else _ref[pcur.next].prev=cur;
      
      ++_length;
      return cur;
    }
  
    inline __ST BSS_FASTCALL InsertBefore(const_reference item, __ST index)
    {
      __ST cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.next=index;
      pcur.prev=(index==(__ST)-1)?_end:(__ST)-1; //This allows one to insert after the end with this function
      pcur.val=item;

      if(pcur.next==(__ST)-1) _end=cur;
      else { pcur.prev=_ref[pcur.next].prev; _ref[pcur.next].prev=cur; }
      if(pcur.prev==(__ST)-1) _start=cur;
      else { _ref[pcur.prev].next=cur; }
      
      ++_length;
      return cur;
    }
    
    inline T BSS_FASTCALL Remove(__ST index)
    {
      assert(index<_ref.Size());
      TLNODE& pcur=_ref[index];
      if(pcur.next==(__ST)-1) _end=pcur.prev; //your the end, reassign
      else _ref[pcur.next].prev=pcur.prev;
      if(pcur.prev==(__ST)-1) _start=pcur.next; //your the root, reassign
      else _ref[pcur.prev].next=pcur.next;

      _addfreelist(index);
      --_length;
      return pcur.val;
    }
    inline __ST Length() const { return _length; }
    inline void BSS_FASTCALL Reserve(__ST size) { if(size<_ref.Size()) return; __ST hold=_ref.Size(); _ref.SetSize(size); _setupchunk(hold); }
    inline void Clear() { _freelist=_start=_end=-1; _length=0; _setupchunk(0); }
    inline BSS_FORCEINLINE void Next(__ST& ref) const { ref=_ref[ref].next; }
    inline BSS_FORCEINLINE void Prev(__ST& ref) const { ref=_ref[ref].prev; }
    inline BSS_FORCEINLINE __ST Start() const { return _start; }
    inline BSS_FORCEINLINE __ST End() const { return _end; }
    //inline cLinkedArray& BSS_FASTCALL operator +=(const cLinkedArray& right);
    //inline cLinkedArray BSS_FASTCALL operator +(const cLinkedArray& right) const { cLinkedArray retval(*this); retval+=right; return retval; }
    inline cLinkedArray& BSS_FASTCALL operator =(const cLinkedArray& right) { _ref=right._ref; _length=right._length;_start=right._start;_end=right._end;_freelist=right._freelist; return *this; }
    inline cLinkedArray& BSS_FASTCALL operator =(cLinkedArray&& mov) { _ref=std::move(mov._ref); _length=mov._length;_start=mov._start;_end=mov._end;_freelist=mov._freelist; return *this; }
    inline BSS_FORCEINLINE reference BSS_FASTCALL GetItem(__ST index) { return _ref[index].val; }
    inline BSS_FORCEINLINE const_reference BSS_FASTCALL GetItem(__ST index) const { return _ref[index].val; }
    inline BSS_FORCEINLINE pointer BSS_FASTCALL GetItemPtr(__ST index) { return &_ref[index].val; }
    inline BSS_FORCEINLINE reference BSS_FASTCALL operator [](__ST index) { return _ref[index].val; }
    inline BSS_FORCEINLINE const_reference BSS_FASTCALL operator [](__ST index) const { return _ref[index].val; }

    /* Iterator for cLinkedArray */
    template<typename _PP, typename _REF, typename D=cLinkedArray<T,Traits,SizeType>>
    class BSS_COMPILER_DLLEXPORT cLAIter : public std::iterator<std::bidirectional_iterator_tag,typename D::value_type,ptrdiff_t,_PP,_REF>
	  {
    public:
      inline explicit cLAIter(D& src) : _src(src), cur((__ST)-1) {}
      inline cLAIter(D& src, __ST start) : _src(src), cur(start) {}
      inline cLAIter(const cLAIter& copy, __ST start) : _src(copy._src), cur(start) {}
      inline BSS_FORCEINLINE _REF operator*() const { return _src[cur]; }
      inline BSS_FORCEINLINE _PP operator->() const { return &_src[cur]; }
      inline BSS_FORCEINLINE cLAIter& operator++() { _src.Next(cur); return *this; } //prefix
      inline BSS_FORCEINLINE cLAIter operator++(int) { cLAIter r=*this; ++*this; return r; } //postfix
      inline BSS_FORCEINLINE cLAIter& operator--() { _src.Prev(cur); return *this; } //prefix
      inline BSS_FORCEINLINE cLAIter operator--(int) { cLAIter r=*this; --*this; return r; } //postfix
      inline BSS_FORCEINLINE bool operator==(const cLAIter& _Right) const { return (cur == _Right.cur); }
	    inline BSS_FORCEINLINE bool operator!=(const cLAIter& _Right) const { return (cur != _Right.cur); }
      inline BSS_FORCEINLINE bool operator!() const { return cur==(__ST)-1; }
      inline BSS_FORCEINLINE bool IsValid() { return cur!=(__ST)-1; }

      __ST cur;

    protected:
      D& _src;
	  };
    
    inline cLAIter<const_pointer, const_reference, const cLinkedArray<T,Traits,SizeType>> begin() const { return cLAIter<const_pointer, const_reference, const cLinkedArray<T,Traits,SizeType>>(*this,_start); } // Use these to get an iterator you can use in standard containers
    inline cLAIter<const_pointer, const_reference, const cLinkedArray<T,Traits,SizeType>> end() const { return cLAIter<const T, const cLinkedArray<T,Traits,SizeType>>(*this); }
    inline cLAIter<pointer, reference, cLinkedArray<T,Traits,SizeType>> begin() { return cLAIter<pointer, reference, cLinkedArray<T,Traits,SizeType>>(*this,_start); } // Use these to get an iterator you can use in standard containers
    inline cLAIter<pointer, reference, cLinkedArray<T,Traits,SizeType>> end() { return cLAIter<pointer, reference, cLinkedArray<T,Traits,SizeType>>(*this); }
    
    /* Nonstandard Removal Iterator for cLinkedArray to make removing elements easier */
    class BSS_COMPILER_DLLEXPORT cLAIterRM : protected cLAIter<pointer,reference,cLinkedArray<T,Traits,SizeType>>
	  {
      typedef cLAIter<pointer,reference,cLinkedArray<T,Traits,SizeType>> __BASE;

    public:
      inline cLAIterRM(const cLAIterRM& copy) : __BASE(copy), next(copy.next) { }
      inline explicit cLAIterRM(const __BASE& from) : __BASE(from,(__ST)-1), next(from.cur) { }
      inline BSS_FORCEINLINE reference operator*() const { return _src[cur]; }
      inline BSS_FORCEINLINE cLAIterRM& operator++() { cur=next; _src.Next(next); return *this; } //prefix
      inline BSS_FORCEINLINE cLAIterRM& operator--() { next=cur; _src.Prev(cur); return *this; } //prefix
      //inline bool operator==(const cLAIterRM& _Right) const { return (next == _Right.next); }
	    //inline bool operator!=(const cLAIterRM& _Right) const { return (next != _Right.next); }
      inline BSS_FORCEINLINE bool HasNext() { return next!=(__ST)-1; }
      inline BSS_FORCEINLINE void Remove() { _src.Remove(cur); }

      __ST next;
	  };

  protected:
    inline void BSS_FASTCALL _addfreelist(__ST index)
    {
      _ref[index].next=_freelist;
      _freelist=index;
    }
    inline void BSS_FASTCALL _setupchunk(__ST start)
    {
      __ST i=_ref.Size();
      while(i>start) //this trick lets us run backwards without worrying about the unsigned problem.
        _addfreelist(--i);
    }
    inline __ST _insprep()
    {
      if(_freelist==(__ST)-1) Reserve(fbnext(_ref.Size()));
      __ST cur=_freelist;
      _freelist=_ref[_freelist].next; //increment freelist
      return cur;
    }

    __ST _length;
    __ST _start;
    __ST _end;
    __ST _freelist;

  private:
    cArrayWrap<cArraySimple<LINKEDNODE<T,__ST>,__ST>> _ref;
  };
    
}

#endif