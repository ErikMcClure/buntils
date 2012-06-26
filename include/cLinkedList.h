// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKEDLIST_H__BSS__
#define __C_LINKEDLIST_H__BSS__

#include "LLBase.h"
#include "bss_alloc.h"

namespace bss_util {
  /* Default node for internal use */
  template<typename T>
  struct cLLNode : LLBase<cLLNode<T>> 
  {
    T item;
  };

  /* Public node prohibiting manipulation of pointers, has the same bit layout as the internal node */
  template<typename T>
  struct cLLPNode : private LLBase<cLLPNode<T>> 
  {
    cLLPNode(const cLLNode<T>& copy) : item(copy.item) { next=(cLLPNode<T>*)copy.next; prev=(cLLPNode<T>*)copy.prev; }
    T item;
    inline BSS_FORCEINLINE cLLPNode<T>* GetNext() { return next; }
    inline BSS_FORCEINLINE cLLPNode<T>* GetPrev() { return prev; }
  };
  
  /* Const node prohibiting manipulation of everything, has the same bit layout as the internal node */
  template<typename T>
  struct cLLCNode : LLBase<cLLCNode<const T>> 
  {
    T item;
  };

  /* Generic Iterator for cLinkedList */
  template<typename T, typename _Nd=cLLNode<T>>
  class BSS_COMPILER_DLLEXPORT cLLIter : public LLIterator<T,_Nd>
  {
  public:
    inline cLLIter() {}
    inline explicit cLLIter(_Nd* node) : LLIterator<T,_Nd>(node) { }
    inline reference operator*() const { return cur->item; }
    inline pointer operator->() const { return &cur->item; }
    inline cLLIter& operator++() { cur=cur->next; return *this; } //prefix
    inline cLLIter operator++(int) { cLLIter r=*this; ++*this; return r; } //postfix
    inline cLLIter& operator--() { cur=cur->prev; return *this; } //prefix
    inline cLLIter operator--(int) { cLLIter r=*this; --*this; return r; } //postfix
    inline bool operator==(const cLLIter& _Right) const { return (cur == _Right.cur); }
	  inline bool operator!=(const cLLIter& _Right) const { return (cur != _Right.cur); }
    inline bool operator!() const { return !cur; }
  };

  /* Adaptive class template for Size usage */
  template<bool size> struct cLinkedList_FuncSize {};
  template<> struct cLinkedList_FuncSize<true> { 
    inline unsigned int Length() const { return _size; }

  protected:    
    unsigned int _size;
    inline void _incsize() { ++_size; }
    inline void _decsize() { --_size; }
    inline void _zerosize() { _size=0; } 
    cLinkedList_FuncSize() : _size(0) {}
  };
  template<> struct cLinkedList_FuncSize<false> { inline void _incsize() { } inline void _decsize() { } inline void _zerosize() { } };

  /* Doubly linked list implementation with _root, _last and an optional _size */
  template<typename T, typename Alloc=Allocator<cLLNode<T>>, bool useSize=false>
  class BSS_COMPILER_DLLEXPORT cLinkedList : protected cAllocTracker<Alloc>, public cLinkedList_FuncSize<useSize>
  {
  public:
    /* Constructor, takes an optional allocator instance */
    inline explicit cLinkedList(Alloc* allocator=0) : cAllocTracker<Alloc>(allocator), _last(0), _root(0)
    {
    }
    /* Copy constructor */
    inline cLinkedList(const cLinkedList<T,Alloc>& copy) : cAllocTracker<Alloc>(copy), _last(0), _root(0)
    {
      operator =(copy);
    }
    inline cLinkedList(cLinkedList<T,Alloc>&& mov) : cAllocTracker<Alloc>(std::move(mov)), _last(mov._last), _root(mov._root)
    {
      mov._root=0;
      mov._last=0;
      cLinkedList_FuncSize<useSize>::operator=(mov);
      mov._zerosize();
    }
    /* Destructor */
    inline ~cLinkedList()
    {
      Clear();
    }
    /* Appends the item to the end of the list */
    inline cLLNode<T>* BSS_FASTCALL Add(T item)
    {
      cLLNode<T>* node = _createnode(item,_last);
      if(_last!=0) _last->next=node;
      _last = node;

      _incsize(); //increment size by one
      if(_root == 0) _root = node; //If root is 0 it means our list hasn't gotten anything assigned to it, so this node is both the root and the last
      return node;
    }
    /* Inserts the item before the given node. If the given node is null, appends the item to the end of the list */
    inline cLLNode<T>* BSS_FASTCALL Insert(T item, cLLNode<T>* target)
    {
      if(!target) return Add(item);

      cLLNode<T>* hold = _createnode(item, target->prev, target);
      LLInsert<cLLNode<T>>(hold,target,_root);

      _incsize(); //increment size by one
      return hold;
    }
    inline void BSS_FASTCALL Remove(cLLNode<T>* node)
    {
      LLRemove<cLLNode<T>>(node,_root,_last);

      _delnode(node); //We don't need our linkedlist wrapper struct anymore so we destroy it
      _decsize(); //our size is now down one
    }
    inline cLLNode<T>* BSS_FASTCALL Remove(cLLNode<T>* node, bool backwards)
    {
      if(node == 0) return 0;

      cLLNode<T>* retval=backwards?node->prev:node->next;
      Remove(node);

      return retval;
    }
    inline cLLNode<T>* BSS_FASTCALL Item(T item, cLLNode<T>* from=0, bool backwards=false) const 
    { //if target is 0 we start from the root
      if(from == 0) from=_root;
      cLLNode<T>* cur=from;
      cLLNode<T>* rootflip=backwards?_last:_root;
      while(cur->item!=item)
      {
        cur=backwards?cur->prev:cur->next; //This method of finding an object is such bad practice we just let the CPU optimize this out.
        if(!cur) cur=rootflip; //return to the flipside value
        if(cur==from) return 0; //if we have returned to the starting node, the item doesn't exist
      }
      return cur;
    } 
    inline void Clear()
    {
      cLLNode<T>* hold = _root;
      cLLNode<T>* nexthold;
      while(hold)
      {
        nexthold = hold->next;
        _delnode(hold);
        hold = nexthold;
      }
      _root = 0;
      _last=0;
      _zerosize();
    }

    inline cLLNode<T>* GetRoot() const { return _root; }
    inline cLLNode<T>* GetLast() const { return _last; }
    inline cLLIter<T> begin() const { return cLLIter<T>(_root); } // Use these to get an iterator you can use in standard containers
    inline cLLIter<T> end() const { return cLLIter<T>(0); }

    template<typename U, bool V>
    inline cLinkedList<T,Alloc,useSize>& operator =(const cLinkedList<T,U,V>& right) { if(&right==this) return *this; Clear(); return operator +=(right); }
    template<typename U, bool V>
    inline cLinkedList<T,Alloc,useSize>& operator =(cLinkedList<T,U,V>&& mov) { if(&mov==this) return *this;
      Clear(); _root=mov._root; _last=mov._last; mov._root=0; mov._last=0; cLinkedList_FuncSize<useSize>::operator=(mov); mov._zerosize();
    }
    template<typename U, bool V>
    inline cLinkedList<T,Alloc,useSize>& operator +=(const cLinkedList<T,U,V>& right) 
    { 
      cLLNode<T>* cur=right._root;
      while(cur!=0)
      {
        Add(cur->item);
        cur=cur->next;
      }
      return *this;
    }

  protected:
    inline cLLNode<T>* BSS_FASTCALL _createnode(T _item, cLLNode<T>* _prev=0, cLLNode<T>* _next=0)
    {
			cLLNode<T>* retval = _alloc->allocate(1);
			retval->item=_item;
			retval->prev=_prev;
			retval->next=_next;
			return retval;
    }

    inline void BSS_FASTCALL _delnode(cLLNode<T>* target)
    {
			_alloc->deallocate(target, 1);
    }

    cLLNode<T>* _root;
    cLLNode<T>* _last;
  };
}

/* Adaptive class templitization for _last usage */
  /*template<typename T, typename Alloc, bool useSize, bool last> class cLinkedList_FuncLast {};
  template<typename T, typename Alloc, bool useSize> struct cLinkedList_FuncLast<T,Alloc,useSize,true>
  { 
    inline cLLNode<T>* GetLast() const { return _last; }
  protected:
    cLLNode<T>* _last;
    template<cLLNode<T>* (BSS_FASTCALL cLinkedList<T,Alloc,useSize,true>::* Insert)(T, cLLNode<T>*),cLLNode<T>* (BSS_FASTCALL cLinkedList<T,Alloc,useSize,true>::* _add)(T,cLLNode<T>*&)>
    static inline cLLNode<T>* BSS_FASTCALL _addfunc(cLinkedList<T,Alloc,useSize,true>* p, T item) { return (p->*_add)(item, p->_last); }
    inline void BSS_FASTCALL _removefunc(cLLNode<T>* node, cLLNode<T>* _root) { LLRemove<cLLNode<T>>(node,_root,_last); }
    inline void _zerolast() { _last=0; } 
    cLinkedList_FuncLast() : _last(0) {}
  };
  template<typename T, typename Alloc, bool useSize> struct cLinkedList_FuncLast<T,Alloc,useSize,false>
  {
  protected:
    template<cLLNode<T>* (BSS_FASTCALL cLinkedList<T,Alloc,useSize,false>::* Insert)(T, cLLNode<T>*),cLLNode<T>* (BSS_FASTCALL cLinkedList<T,Alloc,useSize,false>::* _add)(T,cLLNode<T>*&)>
    static inline cLLNode<T>* BSS_FASTCALL _addfunc(cLinkedList<T,Alloc,useSize,false>* p, T item) { if(_root==0) return _root=_createnode(item,0,0); return (p->*Insert)(p,item,p->_root); }
    inline void BSS_FASTCALL _removefunc(cLLNode<T>* node, cLLNode<T>* _root) { LLRemove<cLLNode<T>>(node,_root); }
    inline void _zerolast() { } 
  };*/

#endif