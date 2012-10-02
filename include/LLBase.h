// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LL_BASE_H__BSS__
#define __LL_BASE_H__BSS__

#include "bss_call.h"
#include <xutility>
#include <list>

namespace bss_util {
  // A base node for a doubly-linked list. The given parameter T may be any class that publically inherits LLBase
  template<typename T>
  struct LLBase
  {
    T* next;
    T* prev;
  };

  // Inserts the node before the target and re-assigns root if necessary. Does not assign values to node.
  template<typename T>
  inline void BSS_FASTCALL LLInsert(T* node, T* target, T*& root)
  {
		if(target->prev != 0) target->prev->next = node;
		else root = node;
		target->prev = node;
  }

  // Inserts the node before the target. Does not assign values to node or re-assign root.
  template<typename T>
  inline void BSS_FASTCALL LLInsert(T* node, T* target)
  {
		if(target->prev != 0) target->prev->next = node;
		target->prev = node;
  }

  // Assigns node's values for a before insertion (used before LLInsert)
  template<typename T>
  inline void BSS_FASTCALL LLInsertAssign(T* node, T* target)
  {
		node->prev = target->prev;
		node->next = target;
  }

  // Does a full insert, calling LLInsertAssign and LLInsert
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL LLInsertFull(T* node, T* target, T*& root)
  {
    LLInsertAssign(node,target);
    LLInsert(node,target,root);
  }

  // Does a full insert, calling LLInsertAssign and LLInsert, but doesn't re-assign root
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL LLInsertFull(T* node, T* target)
  {
    LLInsertAssign(node,target);
    LLInsert(node,target);
  }

  // Inserts the node before the target and re-assigns last if necessary. Does not assign values to node.
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfter(T* node, T* target, T*& last)
  {
		if (target->next != 0) target->next->prev = node;
		else last = node;
		target->next = node;
  }

  // Inserts the node before the target. Does not assign values to node.
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfter(T* node, T* target)
  {
		if (target->next != 0) target->next->prev = node;
		target->next = node;
  }

  // Assigns node's values for an after insertion (used before LLInsertAfter)
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfterAssign(T* node, T* target)
  {
		node->next = target->next;
		node->prev = target;
  }

  // Does a full insert, calling LLInsertAssign and LLInsert
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL LLInsertAfterFull(T* node, T* target, T*& last)
  {
    LLInsertAfterAssign(node,target);
    LLInsertAfter(node,target,last);
  }

  // Does a full insert, calling LLInsertAssign and LLInsert, but doesn't re-assign last
  template<typename T>
  BSS_FORCEINLINE void BSS_FASTCALL LLInsertAfterFull(T* node, T* target)
  {
    LLInsertAfterAssign(node,target);
    LLInsertAfter(node,target);
  }

  // Used in the form last = LLAdd(node,last); where last must be the last element of a linked list, defined by last->next = 0.
  template<typename T>
  inline T* BSS_FASTCALL LLAdd(T* node, T* last)
  {
    LLInsertAfterAssign(node,last);
    last->next = node;
    return node;
  }

  // Removes a node from a list, re-assigning root and last as necessary. Does not zero values on node
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node, T*& root, T*& last)
  {
		if(node->prev != 0) node->prev->next = node->next;
		else root = node->next;
		if(node->next != 0) node->next->prev = node->prev;
		else last = node->prev;
  }

  // Removes a node from a list, re-assigning root as necessary. Does not zero values on node
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node, T*& root)
  {
		if(node->prev != 0) node->prev->next = node->next;
		else root = node->next;
		if(node->next != 0) node->next->prev = node->prev;
  }

  // Removes a node from a list. Does not zero values on node
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node)
  {
		if(node->prev != 0) node->prev->next = node->next;
		if(node->next != 0) node->next->prev = node->prev;
  }

  // Iterator for doubly linked list. Does not support remove; use postfix-- or the equivelent
  template<typename T, typename _Nd>
  class BSS_COMPILER_DLLEXPORT LLIterator : public std::iterator<std::bidirectional_iterator_tag,T>
	{
public:
    inline LLIterator() : cur(0) {}
    inline explicit LLIterator(_Nd* node) : cur(node) { }
    //inline reference operator*() const { } //Inherited iterators must define this based on where they store T
    //inline pointer operator->() const { }
    inline LLIterator& operator++() { cur=cur->next; return *this; } //prefix
    inline LLIterator operator++(int) { LLIterator r=*this; ++*this; return r; } //postfix
    inline LLIterator& operator--() { cur=cur->prev; return *this; } //prefix
    inline LLIterator operator--(int) { LLIterator r=*this; --*this; return r; } //postfix
    inline bool operator==(const LLIterator& _Right) const { return (cur == _Right.cur); }
	  inline bool operator!=(const LLIterator& _Right) const { return (cur != _Right.cur); }
    inline bool operator!() const { return !cur; }
    inline bool IsValid() { return cur!=0; }

    _Nd* cur;
	};
}



#endif