// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __LL_BASE_H__BUN__
#define __LL_BASE_H__BUN__

#include "defines.h"
#include <iterator>
#include <stddef.h>

namespace bun {
  template<typename T>
  concept IntrusiveLinkedList = std::same_as<std::remove_cvref_t<decltype(std::declval<T>().next)>, T*> &&
                                std::same_as<std::remove_cvref_t<decltype(std::declval<T>().prev)>, T*>;

  // A base node for a doubly-linked list. The given parameter T may be any class that publically inherits LLBase
  template<typename T> struct BUN_COMPILER_DLLEXPORT LLBase
  {
    T* next;
    T* prev;
  };

  // Does a full insert, calling LLInsertAssign and LLInsert
  template<IntrusiveLinkedList T> inline void LLInsert(T* node, T* target, T*& root) noexcept
  {
    node->prev = target->prev;
    node->next = target;
    if(target->prev != 0)
      target->prev->next = node;
    else
      root = node;
    target->prev = node;
  }

  // Does a full insert, calling LLInsertAssign and LLInsert, but doesn't re-assign root
  template<IntrusiveLinkedList T> inline void LLInsert(T* node, T* target) noexcept
  {
    node->prev = target->prev;
    node->next = target;
    if(target->prev != 0)
      target->prev->next = node;
    target->prev = node;
  }

  // Does a full insert before the root
  template<IntrusiveLinkedList T> inline void LLAdd(T* node, T*& root) noexcept
  {
    node->prev = 0;
    node->next = root;
    if(root)
      root->prev = node;
    root = node;
  }

  template<IntrusiveLinkedList T> inline void LLAdd(T* node, T*& root, T*& last) noexcept
  {
    node->prev = 0;
    node->next = root;
    if(root)
      root->prev = node;
    else
      last = node;
    root = node;
  }

  // Does a full insert, calling LLInsertAssign and LLInsert
  template<IntrusiveLinkedList T> inline void LLInsertAfter(T* node, T* target, T*& last) noexcept
  {
    node->next = target->next;
    node->prev = target;
    if(target->next != 0)
      target->next->prev = node;
    else
      last = node;
    target->next = node;
  }

  // Does a full insert, calling LLInsertAssign and LLInsert, but doesn't re-assign last
  template<IntrusiveLinkedList T> inline void LLInsertAfter(T* node, T* target) noexcept
  {
    node->next = target->next;
    node->prev = target;
    if(target->next != 0)
      target->next->prev = node;
    target->next = node;
  }

  // Used in the form last = LLAddAfter(node,last); where last must be the last element of a linked list, defined by
  // last->next = 0.
  template<IntrusiveLinkedList T> inline T* LLAddAfter(T* node, T* last) noexcept
  {
    node->next = 0;
    node->prev = last;
    last->next = node;
    return node;
  }

  template<IntrusiveLinkedList T> inline void LLAddAfter(T* node, T*& root, T*& last) noexcept
  {
    node->next = 0;
    node->prev = last;
    if(last)
      last->next = node;
    else
      root = node;
    last = node;
  }

  // Removes a node from a list, re-assigning root and last as necessary. Does not zero values on node
  template<IntrusiveLinkedList T> inline void LLRemove(T* node, T*& root, T*& last) noexcept
  {
    if(node->prev != 0)
      node->prev->next = node->next;
    else if(root == node)
      root = node->next;
    if(node->next != 0)
      node->next->prev = node->prev;
    else if(last == node)
      last = node->prev;
  }

  // Removes a node from a list, re-assigning root as necessary. Does not zero values on node
  template<IntrusiveLinkedList T> inline void LLRemove(T* node, T*& root) noexcept
  {
    if(node->prev != 0)
      node->prev->next = node->next;
    else if(root == node)
      root = node->next;
    if(node->next != 0)
      node->next->prev = node->prev;
  }

  // Removes a node from a list. Does not zero values on node
  template<IntrusiveLinkedList T> inline void LLRemove(T* node) noexcept
  {
    if(node->prev != 0)
      node->prev->next = node->next;
    if(node->next != 0)
      node->next->prev = node->prev;
  }

  // Reimplementations of LLAdd and LLRemove generalized to any data structure instead of requiring it to be inherited.
  template<typename T, LLBase<T>& (*GETNODES)(T* n)> BUN_FORCEINLINE void AltLLAdd(T* node, T*& root) noexcept
  {
    GETNODES(node).prev = 0;
    GETNODES(node).next = root;
    if(root)
      GETNODES(root).prev = node;
    root = node;
  }
  template<typename T, LLBase<T>& (*GETNODES)(T* n)> inline void AltLLRemove(T* node, T*& root) noexcept
  {
    if(GETNODES(node).prev != 0)
      GETNODES(GETNODES(node).prev).next = GETNODES(node).next;
    else if(root == node)
      root = static_cast<T*>(GETNODES(node).next);
    if(GETNODES(node).next != 0)
      GETNODES(GETNODES(node).next).prev = GETNODES(node).prev;
  }

  // Iterator for doubly linked list where the item is itself. Does not support remove; use postfix-- or the equivelent
  template<IntrusiveLinkedList T> class BUN_COMPILER_DLLEXPORT LLIterator
  {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T*;

    inline LLIterator() : cur(0) {}
    inline explicit LLIterator(T* node) : cur(node) {}
    inline T* operator*() const { return cur; }
    inline T* operator->() const { return cur; }
    inline LLIterator& operator++()
    {
      cur = cur->next;
      return *this;
    } // prefix
    inline LLIterator operator++(int)
    {
      LLIterator r = *this;
      ++*this;
      return r;
    } // postfix
    inline LLIterator& operator--()
    {
      cur = cur->prev;
      return *this;
    } // prefix
    inline LLIterator operator--(int)
    {
      LLIterator r = *this;
      --*this;
      return r;
    } // postfix
    inline bool operator==(const LLIterator& _Right) const { return (cur == _Right.cur); }
    inline bool operator!=(const LLIterator& _Right) const { return (cur != _Right.cur); }
    inline bool operator!() const { return !cur; }
    inline bool IsValid() const { return cur != 0; }

    T* cur;
  };
}

#endif
