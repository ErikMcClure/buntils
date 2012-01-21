// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LL_BASE_H__BSS__
#define __LL_BASE_H__BSS__

#include "bss_call.h"
#include "Iterator.h"

namespace bss_util {
  /* A base node for a doubly-linked list. The given parameter T may be any class that publically inherits cLLBase */
  template<typename T>
  struct LLBase
  {
    T* next;
    T* prev;
  };

  /* Inserts the node before the target and re-assigns root if necessary. Does not assign values to node. */
  template<typename T>
  inline void BSS_FASTCALL LLInsert(T* node, T* target, T*& root)
  {
		if(target->prev != 0) target->prev->next = node;
		else root = node;
		target->prev = node;
  }

  /* Inserts the node before the target. Does not assign values to node or re-assign root. */
  template<typename T>
  inline void BSS_FASTCALL LLInsert(T* node, T* target)
  {
		if(target->prev != 0) target->prev->next = node;
		target->prev = node;
  }

  /* Assigns node's values for a before insertion (used before LLInsert) */
  template<typename T>
  inline void BSS_FASTCALL LLInsertAssign(T* node, T* target)
  {
		node->prev = target->prev;
		node->next = target;
  }

  /* Inserts the node before the target and re-assigns last if necessary. Does not assign values to node. */
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfter(T* node, T* target, T*& last)
  {
		if (target->next != 0) target->next->prev = node;
		else last = node;
		target->next = node;
  }

  /* Inserts the node before the target. Does not assign values to node. */
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfter(T* node, T* target)
  {
		if (target->next != 0) target->next->prev = node;
		target->next = node;
  }

  /* Assigns node's values for an after insertion (used before LLInsertAfter) */
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfterAssign(T* node, T* target)
  {
		node->next = target->next;
		node->prev = target;
  }

  /* Removes a node from a list, re-assigning root and last as necessary. Does not zero values on node */
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node, T*& root, T*& last)
  {
		if(node->prev != 0) node->prev->next = node->next;
		else root = node->next;
		if(node->next != 0) node->next->prev = node->prev;
		else last = node->prev;
  }

  /* Removes a node from a list, re-assigning root as necessary. Does not zero values on node */
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node, T*& root)
  {
		if(node->prev != 0) node->prev->next = node->next;
		else root = node->next;
		if(node->next != 0) node->next->prev = node->prev;
  }

  /* Removes a node from a list. Does not zero values on node */
  template<typename T>
  inline void BSS_FASTCALL LLRemove(T* node)
  {
		if(node->prev != 0) node->prev->next = node->next;
		if(node->next != 0) node->next->prev = node->prev;
  }

  /* Iterator for doubly linked list. By default it does not initialize the previous node. */
  template<typename T>
  class BSS_COMPILER_DLLEXPORT LLIterator : public Iterator<T*>
  {
    typedef typename Iterator<T*>::const_reference const_reference;
    typedef typename Iterator<T*>::value_type value_type;

  public:
    inline explicit LLIterator(T* start) : cur(0), next(start) { }
    inline explicit LLIterator(T* start, char initPrev) : cur(start), next(start) { if(start!=0) cur=start->prev; }
    inline virtual const_reference operator++() { cur=next; next=next->next; return cur; } //prefix
    inline virtual const_reference operator--() { next=cur; cur=cur->prev; return cur;} //prefix
    inline virtual const_reference Peek() { return next; }
    //inline virtual const_reference Last() { return cur; }
    inline virtual void Remove() { LLRemove<T>(cur); cur=0; }
    inline virtual bool HasNext() { return next!=0; }
    inline virtual bool HasPrev() { return cur!=0 && cur->prev!=0; }

  protected:
    T* cur;
    T* next;
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT LLIteratorR : public LLIterator<T>
  {
  public:
    inline explicit LLIteratorR(T* start, T*& root) : LLIterator<T>(start), _root(root) { }
    inline explicit LLIteratorR(T* start, T*& root, char initPrev) : LLIterator<T>(start,0), _root(root) { }
    inline virtual void Remove() { LLRemove<T>(cur,_root); cur=0; }

  protected:
    T*& _root;
  };

  template<typename T>
  class BSS_COMPILER_DLLEXPORT LLIteratorRL : public LLIteratorR<T>
  {
  public:
    inline explicit LLIteratorRL(T* start, T*& root, T*& last) : LLIteratorR<T>(start,root), _last(last) { }
    inline explicit LLIteratorRL(T* start, T*& root, T*& last, char initPrev) : LLIteratorR<T>(start,root,0), _last(last) { }
    inline virtual void Remove() { LLRemove<T>(cur,_root,_last); cur=0; }

  protected:
    T*& _last;
  };
}



#endif