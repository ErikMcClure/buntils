// Copyright �2018 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __LINKEDLIST_H__BUN__
#define __LINKEDLIST_H__BUN__

#include "LLBase.h"
#include "Alloc.h"
#include <string.h>

namespace bun {
  // Default node for internal use
  template<typename T>
  struct LLNode : LLBase<LLNode<T>> { T item; };

  // Generic Iterator for LinkedList
  template<typename T>
  class BUN_COMPILER_DLLEXPORT LLIter
  {
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

  public:
    inline LLIter() : cur(0) {}
    inline explicit LLIter(LLNode<T>* node) : cur(node) {}
    inline reference operator*() const { return cur->item; }
    inline pointer operator->() const { return &cur->item; }
    inline LLIter& operator++() { cur = cur->next; return *this; } //prefix
    inline LLIter operator++(int) { LLIter r = *this; ++*this; return r; } //postfix
    inline LLIter& operator--() { cur = cur->prev; return *this; } //prefix
    inline LLIter operator--(int) { LLIter r = *this; --*this; return r; } //postfix
    inline bool operator==(const LLIter& _Right) const { return (cur == _Right.cur); }
    inline bool operator!=(const LLIter& _Right) const { return (cur != _Right.cur); }
    inline bool operator!() const { return !cur; }
    inline bool IsValid() { return cur != 0; }

    LLNode<T>* cur;
  }; // Remove a node by doing LinkedList.Remove(iter++);

  namespace internal {
  // Adaptive class template for Size usage
    template<bool size> struct LList_SIZE {};
    template<> struct LList_SIZE<true> {
      inline size_t Length() const { return _length; }

    protected:
      size_t _length;
      inline void _incsize() { ++_length; }
      inline void _decsize() { --_length; }
      inline void _zerosize() { _length = 0; }
      LList_SIZE() : _length(0) {}
    };
    template<> struct LList_SIZE<false> { inline void _incsize() {} inline void _decsize() {} inline void _zerosize() {} };

  // Adaptive class template for _last usage
    template<typename T, bool L>
    struct LList_LAST
    {
      inline LLNode<T>* Back() const { return _last; }
      inline LLNode<T>* Item(T item, LLNode<T>* from = 0, bool backwards = false) const
      { //if target is 0 we start from the root
        if(!from) from = _root;
        LLNode<T>* cur = from;
        LLNode<T>* rootflip = backwards ? _last : _root;
        while(cur->item != item)
        {
          cur = backwards ? cur->prev : cur->next; //This method of finding an object is such bad practice we just let the CPU optimize this out.
          if(!cur) cur = rootflip; //return to the flipside value
          if(cur == from) return 0; //if we have returned to the starting node, the item doesn't exist
        }
        return cur;
      }

    protected:
      inline LList_LAST() : _last(0), _root(0) {}
      inline LList_LAST(LList_LAST<T, L>&& mov) : _last(mov._last), _root(mov._root) { mov._root = 0; mov._last = 0; }
      inline void _add(LLNode<T>* node) { if(!_root) { node->next = node->prev = 0; _last = _root = node; } else _last = LLAddAfter<LLNode<T>>(node, _last); }
      inline void _remove(LLNode<T>* node) { LLRemove<LLNode<T>>(node, _root, _last); }
      inline LList_LAST<T, L>& operator =(LList_LAST<T, L>&& mov) { _last = mov._last; mov._last = 0; _root = mov._root; mov._root = 0; return *this; }

      LLNode<T>* _root;
      LLNode<T>* _last;
    };
    template<typename T>
    struct LList_LAST<T, false>
    {
      inline LLNode<T>* Item(T item, LLNode<T>* from = 0) const
      { //if target is 0 we start from the root
        if(!from) from = _root;
        LLNode<T>* cur = from;
        while(cur->item != item)
        {
          cur = cur->next; //This method of finding an object is such bad practice we just let the CPU optimize this out.
          if(!cur) cur = _root; //return to the flipside value
          if(cur == from) return 0; //if we have returned to the starting node, the item doesn't exist
        }
        return cur;
      }
    protected:
      inline LList_LAST() : _root(0) {}
      inline LList_LAST(LList_LAST<T, false>&& mov) : _root(mov._root) { mov._root = 0; }
      inline void _remove(LLNode<T>* node) { LLRemove<LLNode<T>>(node, _root); }
      inline void _add(LLNode<T>* node) { LLAdd<LLNode<T>>(node, _root); }
      inline LList_LAST<T, false>& operator =(LList_LAST<T, false>&& mov) { _root = mov._root; mov._root = 0; return *this; }

      LLNode<T>* _root;
    };
  }

  // Doubly linked list implementation with _root, optional _last and an optional _length
  template<typename T, typename Alloc = StandardAllocator<LLNode<T>>, bool useLast = false, bool useSize = false>
  class BUN_COMPILER_DLLEXPORT LinkedList : protected Alloc, public internal::LList_SIZE<useSize>, public internal::LList_LAST<T, useLast>
  {
  protected:
    using internal::LList_LAST<T, useLast>::_root;
    using internal::LList_LAST<T, useLast>::_add;
    using internal::LList_LAST<T, useLast>::_remove;
    using internal::LList_SIZE<useSize>::_incsize;

  public:
    // Constructor, takes an optional allocator instance
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline explicit LinkedList(typename Alloc::policy_type* policy) : Alloc(policy) {}
    inline LinkedList() {}
    // Copy constructor
    inline LinkedList(const LinkedList<T, Alloc>& copy) : Alloc(copy) { operator =(copy); }
    inline LinkedList(LinkedList<T, Alloc>&& mov) : Alloc(std::move(mov)), internal::LList_LAST<T, useLast>(std::move(mov))
    {
      internal::LList_SIZE<useSize>::operator=(mov);
      mov._zerosize();
    }
    // Destructor
    inline ~LinkedList() { Clear(); }
    // Appends the item to the end of the list
    inline LLNode<T>* Add(T item)
    {
      LLNode<T>* node = Alloc::allocate(1);
      node->item = item;
      _add(node);
      _incsize(); //increment size by one
      return node;
    }
    // Inserts the item in the front of the list
    inline LLNode<T>* Insert(T item) { return Insert(item, _root); }
    // Inserts the item before the given node. If the given node is null, appends the item to the end of the list
    inline LLNode<T>* Insert(T item, LLNode<T>* target)
    {
      if(!target)
        return Add(item);

      LLNode<T>* hold = _createNode(item, target->prev, target);
      if(target->prev != 0) target->prev->next = hold;
      else _root = hold;
      target->prev = hold;

      _incsize(); //increment size by one
      return hold;
    }
    inline void Remove(LLIter<T>& iter) { Remove(iter.cur); }
    inline void Remove(LLNode<T>* node)
    {
      _remove(node);

      _deleteNode(node); //We don't need our linkedlist wrapper struct anymore so we destroy it
      internal::LList_SIZE<useSize>::_decsize(); //our size is now down one
    }
    inline LLNode<T>* Remove(LLNode<T>* node, bool backwards)
    {
      if(!node) return 0;

      LLNode<T>* retval = backwards ? node->prev : node->next;
      Remove(node);

      return retval;
    }
    inline void Clear()
    {
      LLNode<T>* hold = _root;
      LLNode<T>* nexthold;

      while(hold)
      {
        nexthold = hold->next;
        _deleteNode(hold);
        hold = nexthold;
      }

      bun_Fill(*this, 0);
    }

    inline LLNode<T>* Front() const { return _root; }
    inline LLIter<const T> begin() const { return LLIter<const T>(_root); } // Use these to get an iterator you can use in standard containers
    inline LLIter<const T> end() const { return LLIter<const T>(0); }
    inline LLIter<T> begin() { return LLIter<T>(_root); }
    inline LLIter<T> end() { return LLIter<T>(0); }

    template<typename U, bool V>
    inline LinkedList& operator =(const LinkedList<T, U, useLast, V>& right) { if(&right == this) return *this; Clear(); return operator +=(right); }
    template<typename U, bool V>
    inline LinkedList& operator =(LinkedList<T, U, useLast, V>&& mov)
    {
      if(&mov == this)
        return *this;

      Clear();
      internal::LList_LAST<T, useLast>::operator=(std::move(mov));
      internal::LList_SIZE<useSize>::operator=(mov);
      mov._zerosize();
      return *this;
    }
    template<typename U, bool L, bool V>
    inline LinkedList& operator +=(const LinkedList<T, U, L, V>& right)
    {
      for(LLNode<T>* cur = right._root; cur != 0; cur = cur->next)
        Add(cur->item);

      return *this;
    }
    inline const LinkedList operator +(const LinkedList& add) const
    {
      LinkedList r(*this);
      r += add;
      return r;
    }

  protected:
    inline LLNode<T>* _createNode(T _item, LLNode<T>* _prev = 0, LLNode<T>* _next = 0)
    {
      LLNode<T>* retval = Alloc::allocate(1);
      retval->item = _item;
      retval->prev = _prev;
      retval->next = _next;
      return retval;
    }

    inline void _deleteNode(LLNode<T>* target) { Alloc::deallocate(target, 1); }
  };
}

#endif
