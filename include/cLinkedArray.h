// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKED_ARRAY_H__BSS__
#define __C_LINKED_ARRAY_H__BSS__

#include "bss_traits.h"
#include "bss_util.h"
#include "cArraySimple.h"

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
  
  public:
    inline cLinkedArray() : _ref(1),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline cLinkedArray(const cLinkedArray& copy) : _ref(copy._ref),_length(copy._length),_start(copy._start),_end(copy._end),_freelist(copy._freelist) { }
    inline explicit cLinkedArray(SizeType size) : _ref(size),_length(0),_start(-1),_end(-1),_freelist(-1) { _setupchunk(0); }
    inline ~cLinkedArray() {}
    inline SizeType BSS_FASTCALL Add(const_reference item) { return InsertAfter(item,_end); }
    inline SizeType BSS_FASTCALL InsertAfter(const_reference item, SizeType index)
    {
      SizeType cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.next=(SizeType)-1;
      pcur.prev=(index==(SizeType)-1)?_end:index;;
      pcur.val=item;

      if(pcur.prev==(SizeType)-1) _start=cur;
      else { pcur.next=_ref[pcur.prev].next; _ref[pcur.prev].next=cur; }
      if(pcur.next==(SizeType)-1) _end=cur;
      else _ref[pcur.next].prev=cur;
      
      ++_length;
      return cur;
    }
  
    inline SizeType BSS_FASTCALL InsertBefore(const_reference item, SizeType index)
    {
      SizeType cur=_insprep();

      TLNODE& pcur=_ref[cur];
      pcur.next=index;
      pcur.prev=(index==(SizeType)-1)?_end:(SizeType)-1; //This allows one to insert after the end with this function
      pcur.val=item;

      if(pcur.next==(SizeType)-1) _end=cur;
      else { pcur.prev=_ref[pcur.next].prev; _ref[pcur.next].prev=cur; }
      if(pcur.prev==(SizeType)-1) _start=cur;
      else { _ref[pcur.prev].next=cur; }
      
      ++_length;
      return cur;
    }
    
    inline T BSS_FASTCALL Remove(SizeType index)
    {
      assert(index<_ref.Size());
      TLNODE& pcur=_ref[index];
      if(pcur.next==(SizeType)-1) _end=pcur.prev; //your the end, reassign
      else _ref[pcur.next].prev=pcur.prev;
      if(pcur.prev==(SizeType)-1) _start=pcur.next; //your the root, reassign
      else _ref[pcur.prev].next=pcur.next;

      _addfreelist(index);
      --_length;
      return pcur.val;
    }
    inline SizeType Size() const { return _length; }
    inline void BSS_FASTCALL Reserve(SizeType size) { if(size<_ref.Size()) return; SizeType hold=_ref.Size(); _ref.SetSize(size); _setupchunk(hold); }
    inline void Clear() { _start=_end=-1; _length=_freelist=0; }
    inline void Next(SizeType& ref) const { ref=_ref[ref].next; }
    inline SizeType Start() const { return _start; }
    inline SizeType End() const { return _end; }
    //inline cLinkedArray& BSS_FASTCALL operator +=(const cLinkedArray& right);
    //inline cLinkedArray BSS_FASTCALL operator +(const cLinkedArray& right) const { cLinkedArray retval(*this); retval+=right; return retval; }
    inline cLinkedArray& BSS_FASTCALL operator =(const cLinkedArray& right) { _ref=right._ref; _length=right._length;_start=right._start;_end=right._end;_freelist=right._freelist; return *this; }
    inline reference BSS_FASTCALL GetItem(SizeType index) { return _ref[index].val; }
    inline const_reference BSS_FASTCALL GetItem(SizeType index) const { return _ref[index].val; }
    inline pointer BSS_FASTCALL GetItemPtr(SizeType index) { return &_ref[index].val; }
    inline reference BSS_FASTCALL operator [](SizeType index) { return _ref[index].val; }
    inline const_reference BSS_FASTCALL operator [](SizeType index) const { return _ref[index].val; }

  protected:
    inline void BSS_FASTCALL _addfreelist(SizeType index)
    {
      _ref[index].next=_freelist;
      _freelist=index;
    }
    inline void BSS_FASTCALL _setupchunk(SizeType start)
    {
      SizeType i=_ref.Size();
      while(i>start) //this trick lets us run backwards without worrying about the unsigned problem.
        _addfreelist(--i);
    }
    inline SizeType _insprep()
    {
      if(_freelist==(SizeType)-1) Reserve(fbnext(_ref.Size()));
      SizeType cur=_freelist;
      _freelist=_ref[_freelist].next; //increment freelist
      return cur;
    }

    SizeType _length;
    SizeType _start;
    SizeType _end;
    SizeType _freelist;

  private:
    cArraySimple<LINKEDNODE<T,SizeType>,SizeType> _ref;
  };
}

#endif