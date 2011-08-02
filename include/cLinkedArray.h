// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKED_ARRAY_H__BSS__
#define __C_LINKED_ARRAY_H__BSS__

#include "bss_traits.h"
#include "bss_util.h"
#include "cArraySimple.h"
#include "Iterator.h"

namespace bss_util {
  template<class T,typename SizeType>
  struct __declspec(dllexport) LINKEDNODE
  {
    T val;
    SizeType next;
    SizeType prev;
  };

  /* Linked list implemented as an array. */
  template<class T, class Traits=ValueTraits<T>, typename SizeType=unsigned int>
  class __declspec(dllexport) cLinkedArray : protected Traits
  {
    typedef LINKEDNODE<T,SizeType> TLNODE;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::const_pointer const_pointer;
    typedef typename Traits::reference reference;
    typedef typename Traits::const_reference const_reference;
    typedef typename Traits::value_type value_type;
    typedef SizeType __ST;
  
  public:
    inline cLinkedArray() : _ref(1),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline cLinkedArray(const cLinkedArray& copy) : _ref(copy._ref),_length(copy._length),_start(copy._start),_end(copy._end),_freelist(copy._freelist) { }
    inline cLinkedArray(cLinkedArray&& mov) : _ref(std::move(mov._ref)),_length(mov._length),_start(mov._start),_end(mov._end),_freelist(mov._freelist) { }
    inline explicit cLinkedArray(__ST size) : _ref(size),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline ~cLinkedArray() {}
    inline __ST BSS_FASTCALL Add(const_reference item) { return InsertAfter(item,_end); }
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
    inline __ST Size() const { return _length; }
    inline void BSS_FASTCALL Reserve(__ST size) { if(size<_ref.Size()) return; __ST hold=_ref.Size(); _ref.SetSize(size); _setupchunk(hold); }
    inline void Clear() { _start=_end=-1; _length=_freelist=0; }
    inline void Next(__ST& ref) const { ref=_ref[ref].next; }
    inline void Prev(__ST& ref) const { ref=_ref[ref].prev; }
    inline __ST Start() const { return _start; }
    inline __ST End() const { return _end; }
    //inline cLinkedArray& BSS_FASTCALL operator +=(const cLinkedArray& right);
    //inline cLinkedArray BSS_FASTCALL operator +(const cLinkedArray& right) const { cLinkedArray retval(*this); retval+=right; return retval; }
    inline cLinkedArray& BSS_FASTCALL operator =(const cLinkedArray& right) { _ref=right._ref; _length=right._length;_start=right._start;_end=right._end;_freelist=right._freelist; return *this; }
    inline cLinkedArray& BSS_FASTCALL operator =(cLinkedArray&& mov) { _ref=std::move(mov._ref); _length=mov._length;_start=mov._start;_end=mov._end;_freelist=mov._freelist; return *this; }
    inline reference BSS_FASTCALL GetItem(__ST index) { return _ref[index].val; }
    inline const_reference BSS_FASTCALL GetItem(__ST index) const { return _ref[index].val; }
    inline pointer BSS_FASTCALL GetItemPtr(__ST index) { return &_ref[index].val; }
    inline reference BSS_FASTCALL operator [](__ST index) { return _ref[index].val; }
    inline const_reference BSS_FASTCALL operator [](__ST index) const { return _ref[index].val; }

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
  
  /* Iterator for doubly linked list */
  template<class T, class Traits=ValueTraits<T>, typename SizeType=unsigned int>
  class __declspec(dllexport) LinkedArrayIterator : public Iterator<SizeType>
  {
    typedef typename Iterator<SizeType>::const_reference const_reference;
    typedef typename Iterator<SizeType>::value_type value_type;

  public:
    inline explicit LinkedArrayIterator(cLinkedArray<T,Traits,SizeType>& Src) : src(Src), cur((SizeType)-1), next(src.Start()) {}
    inline LinkedArrayIterator(cLinkedArray<T,Traits,SizeType>& Src, SizeType start) : src(Src), cur(start), next(start) { if(start!=((SizeType)-1)) Src.Prev(cur); }
    inline virtual const_reference operator++() { cur=next; src.Next(next); return cur; } //prefix
    inline virtual const_reference operator--() { next=cur; src.Prev(cur); return cur;} //prefix
    inline virtual const_reference Peek() { return next; }
    //inline virtual const_reference Last() { return cur; }
    inline virtual void Remove() { src.Remove(cur); cur=((SizeType)-1); }
    inline virtual bool HasNext() { return next!=((SizeType)-1); }
    inline virtual bool HasPrev() { if(cur!=((SizeType)-1)) return false; value_type r=cur; src.Prev(r); return r!=((SizeType)-1); }
    //inline const_reference operator*() const { return cur; } // extra operator to make it easy to use with cLinkedArray

  protected:
    cLinkedArray<T,Traits,SizeType>& src;
    SizeType cur;
    SizeType next;
  };
}

#endif