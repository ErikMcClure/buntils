// Copyright ©2011 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __LL_BASE_H__
#define __LL_BASE_H__

#include "bss_call.h"

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
		if (target->next != 0) y->next->prev = node;
		else last = node;
		target->next = node;
  }

  /* Inserts the node before the target. Does not assign values to node. */
  template<typename T>
  inline void BSS_FASTCALL LLInsertAfter(T* node, T* target)
  {
		if (target->next != 0) y->next->prev = node;
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
}



#endif