// Copyright ©2013 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_LINKEDLIST_H__BSS__
#define __C_LINKEDLIST_H__BSS__

#include "LLBase.h"
#include "bss_alloc.h"

namespace bss_util {
  // Default node for internal use
  template<typename T>
  struct cLLNode : LLBase<cLLNode<T>> { T item; };

  // Generic Iterator for cLinkedList
  template<typename T>
  class BSS_COMPILER_DLLEXPORT cLLIter : public std::iterator<std::bidirectional_iterator_tag,T>
  {
    typedef typename std::iterator<std::bidirectional_iterator_tag,T>::pointer pointer;
    typedef typename std::iterator<std::bidirectional_iterator_tag,T>::reference reference;

  public:
    inline cLLIter() : cur(0) {}
    inline explicit cLLIter(cLLNode<T>* node) : cur(node) { }
    inline reference operator*() const { return cur->item; }
    inline pointer operator->() const { return &cur->item; }
    inline cLLIter& operator++() { cur=cur->next; return *this; } //prefix
    inline cLLIter operator++(int) { cLLIter r=*this; ++*this; return r; } //postfix
    inline cLLIter& operator--() { cur=cur->prev; return *this; } //prefix
    inline cLLIter operator--(int) { cLLIter r=*this; --*this; return r; } //postfix
    inline bool operator==(const cLLIter& _Right) const { return (cur == _Right.cur); }
	  inline bool operator!=(const cLLIter& _Right) const { return (cur != _Right.cur); }
    inline bool operator!() const { return !cur; }
    inline bool IsValid() { return cur!=0; }

    cLLNode<T>* cur;
  }; // Remove a node by doing LinkedList.Remove(iter++);

  // Adaptive class template for Size usage
  template<bool size> struct cLList_SIZE {};
  template<> struct cLList_SIZE<true> { 
    inline unsigned int Length() const { return _size; }

  protected:    
    unsigned int _size;
    inline void _incsize() { ++_size; }
    inline void _decsize() { --_size; }
    inline void _zerosize() { _size=0; } 
    cLList_SIZE() : _size(0) {}
  };
  template<> struct cLList_SIZE<false> { inline void _incsize() { } inline void _decsize() { } inline void _zerosize() { } };
  
// Adaptive class template for _last usage
  template<typename T, bool L> 
  struct cLList_LAST
  { 
    inline cLLNode<T>* Back() const { return _last; }
    inline cLLNode<T>* GetLast() const { return _last; }
    inline cLLNode<T>* BSS_FASTCALL Item(T item, cLLNode<T>* from=0, bool backwards=false) const 
    { //if target is 0 we start from the root
      if(!from) from=_root;
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

  protected:
    inline cLList_LAST() : _last(0), _root(0) {}
    inline cLList_LAST(cLList_LAST<T,L>&& mov) : _last(mov._last), _root(mov._root) { mov._root=0; mov._last=0; }
    inline void _add(cLLNode<T>* node) { if(!_root) { node->next=node->prev=0; _last=_root=node; } else _last=LLAdd<cLLNode<T>>(node,_last); }
    inline void _remove(cLLNode<T>* node) { LLRemove<cLLNode<T>>(node,_root,_last); }
    inline cLList_LAST<T,L>& operator =(cLList_LAST<T,L>&& mov) { _last=mov._last; mov._last=0; _root=mov._root; mov._root=0; return *this; }

    cLLNode<T>* _root;
    cLLNode<T>* _last;
  };
  template<typename T>
  struct cLList_LAST<T,false>
  {
    inline cLLNode<T>* BSS_FASTCALL Item(T item, cLLNode<T>* from=0) const 
    { //if target is 0 we start from the root
      if(!from) from=_root;
      cLLNode<T>* cur=from;
      while(cur->item!=item)
      {
        cur=cur->next; //This method of finding an object is such bad practice we just let the CPU optimize this out.
        if(!cur) cur=_root; //return to the flipside value
        if(cur==from) return 0; //if we have returned to the starting node, the item doesn't exist
      }
      return cur;
    } 
  protected:
    inline cLList_LAST() : _root(0) {}
    inline cLList_LAST(cLList_LAST<T,false>&& mov) : _root(mov._root) { mov._root=0; }
    inline void _remove(cLLNode<T>* node) { LLRemove<cLLNode<T>>(node,_root); }
    inline void _add(cLLNode<T>* node) { LLInsertRoot<cLLNode<T>>(node,_root); }
    inline cLList_LAST<T,false>& operator =(cLList_LAST<T,false>&& mov) { _root=mov._root; mov._root=0; return *this; }

    cLLNode<T>* _root;
  };

  // Doubly linked list implementation with _root, optional _last and an optional _size
  template<typename T, typename Alloc=Allocator<cLLNode<T>>, bool useLast=false, bool useSize=false>
  class BSS_COMPILER_DLLEXPORT cLinkedList : protected cAllocTracker<Alloc>, public cLList_SIZE<useSize>, public cLList_LAST<T,useLast>
  {
    using cLList_LAST<T,useLast>::_root;
    using cLList_LAST<T,useLast>::_add;
    using cLList_LAST<T,useLast>::_remove;
    using cLList_SIZE<useSize>::_incsize;

  public:
    // Constructor, takes an optional allocator instance
    inline explicit cLinkedList(Alloc* allocator=0) : cAllocTracker<Alloc>(allocator) { }
    // Copy constructor
    inline cLinkedList(const cLinkedList<T,Alloc>& copy) : cAllocTracker<Alloc>(copy) { operator =(copy); }
    inline cLinkedList(cLinkedList<T,Alloc>&& mov) : cAllocTracker<Alloc>(std::move(mov)), cLList_LAST<T,useLast>(std::move(mov))
    {
      cLList_SIZE<useSize>::operator=(mov);
      mov._zerosize();
    }
    // Destructor
    inline ~cLinkedList() { Clear(); }
    // Appends the item to the end of the list
    inline cLLNode<T>* BSS_FASTCALL Add(T item)
    {
      cLLNode<T>* node = cAllocTracker<Alloc>::_allocate(1);
      node->item=item;
      _add(node);
      _incsize(); //increment size by one
      return node;
    }
    // Inserts the item in the front of the list
    inline cLLNode<T>* BSS_FASTCALL Insert(T item) { return Insert(item,_root); }
    // Inserts the item before the given node. If the given node is null, appends the item to the end of the list
    inline cLLNode<T>* BSS_FASTCALL Insert(T item, cLLNode<T>* target)
    {
      if(!target) return Add(item);

      cLLNode<T>* hold = _createnode(item, target->prev, target);
      LLInsert<cLLNode<T>>(hold,target,_root);

      _incsize(); //increment size by one
      return hold;
    }
    inline void BSS_FASTCALL Remove(cLLIter<T>& iter) { Remove(iter.cur); }
    inline void BSS_FASTCALL Remove(cLLNode<T>* node)
    {
      _remove(node);

      _delnode(node); //We don't need our linkedlist wrapper struct anymore so we destroy it
      cLList_SIZE<useSize>::_decsize(); //our size is now down one
    }
    inline cLLNode<T>* BSS_FASTCALL Remove(cLLNode<T>* node, bool backwards)
    {
      if(!node) return 0;

      cLLNode<T>* retval=backwards?node->prev:node->next;
      Remove(node);

      return retval;
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
      memset(this,0,sizeof(cLinkedList<T,Alloc,useLast,useSize>));
    }

    inline cLLNode<T>* Front() const { return _root; }
    inline cLLIter<const T> begin() const { return cLLIter<const T>(_root); } // Use these to get an iterator you can use in standard containers
    inline cLLIter<const T> end() const { return cLLIter<const T>(0); }
    inline cLLIter<T> begin() { return cLLIter<T>(_root); }
    inline cLLIter<T> end() { return cLLIter<T>(0); }

    template<typename U, bool V>
    inline cLinkedList& operator =(const cLinkedList<T,U,useLast,V>& right) { if(&right==this) return *this; Clear(); return operator +=(right); }
    template<typename U, bool V>
    inline cLinkedList& operator =(cLinkedList<T,U,useLast,V>&& mov) 
    { 
      if(&mov==this) return *this; 
      Clear(); 
      cLList_LAST<T,useLast>::operator=(std::move(mov)); 
      cLList_SIZE<useSize>::operator=(mov); 
      mov._zerosize(); 
      return *this;
    }
    template<typename U, bool L, bool V>
    inline cLinkedList& operator +=(const cLinkedList<T,U,L,V>& right) 
    { 
      for(cLLNode<T>* cur=right._root; cur!=0; cur=cur->next)
        Add(cur->item);
      return *this;
    }

  protected:
    inline cLLNode<T>* BSS_FASTCALL _createnode(T _item, cLLNode<T>* _prev=0, cLLNode<T>* _next=0)
    {
			cLLNode<T>* retval = cAllocTracker<Alloc>::_allocate(1);
			retval->item=_item;
			retval->prev=_prev;
			retval->next=_next;
			return retval;
    }

    inline void BSS_FASTCALL _delnode(cLLNode<T>* target) { cAllocTracker<Alloc>::_deallocate(target, 1); }
  };
}

#endif
