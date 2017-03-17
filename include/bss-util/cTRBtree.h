// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// Insert/delete implementations modified from Arjan van den Boogaard (2004)
// http://archive.gamedev.net/archive/reference/programming/features/TStorage/TStorage.h

#ifndef __C_TRB_TREE_H__BSS__
#define __C_TRB_TREE_H__BSS__

#include "bss_compare.h"
#include "bss_alloc.h"
#include "LLBase.h"

namespace bss_util {
  // Threaded Red-black tree node
  template<class T>
  struct BSS_COMPILER_DLLEXPORT TRB_Node : LLBase<TRB_Node<T>>
  {
    using LLBase<TRB_Node<T>>::next;
    using LLBase<TRB_Node<T>>::prev;

    inline explicit TRB_Node(TRB_Node<T>* pNIL) : left(pNIL), right(pNIL), color(0), parent(0) { next = 0; prev = 0; }
    inline TRB_Node(T v, TRB_Node<T>* pNIL) : value(v), left(pNIL), right(pNIL), color(1), parent(0) { next = 0; prev = 0; }
    TRB_Node<T>* parent;
    union {
      struct {
        TRB_Node<T>* left;
        TRB_Node<T>* right;
      };
      TRB_Node<T>* children[2];
    };
    T value;
    char color; // 0 - black, 1 - red, -1 - duplicate
  };

  // Threaded Red-black tree implementation
  template<typename T, char(*CFunc)(const T&, const T&) = CompT<T>, typename Alloc = StandardAllocPolicy<TRB_Node<T>>>
  class BSS_COMPILER_DLLEXPORT cTRBtree : protected cAllocTracker<Alloc>
  {
    inline cTRBtree(const cTRBtree&) BSS_DELETEFUNC
      inline cTRBtree& operator=(const cTRBtree&) BSS_DELETEFUNCOP
  public:
    inline explicit cTRBtree(Alloc* allocator = 0) : cAllocTracker<Alloc>(allocator), _first(0), _last(0), _root(pNIL) {}
    inline cTRBtree(cTRBtree&& mov) : cAllocTracker<Alloc>(std::move(mov)), _first(mov.first), _last(mov._last), _root(mov._root)
    {
      mov._first = 0;
      mov._last = 0;
      mov._root = pNIL; // Note that this move works because pNIL is *static* so everyone can point to it.
    }
    // Destructor
    inline ~cTRBtree() { Clear(); }
    // Clears the tree
    inline void Clear()
    {
      for(auto i = begin(); i.IsValid();) // Walk through the tree using the linked list and deallocate everything
      {
        (*i)->~TRB_Node();
        cAllocTracker<Alloc>::_deallocate(*(i++), 1);
      }

      _first = 0;
      _last = 0;
      _root = pNIL;
    }
    // Retrieves a given node by key if it exists
    BSS_FORCEINLINE TRB_Node<T>* Get(const T& value) const { return GetNode(value, _root); }
    // Retrieves the node closest to the given key.
    BSS_FORCEINLINE TRB_Node<T>* GetNear(const T& value, bool before = true) const { return GetNodeNear(value, before, _root); }
    // Inserts a key with the associated data
    BSS_FORCEINLINE TRB_Node<T>* Insert(const T& value)
    {
      TRB_Node<T>* node = cAllocTracker<Alloc>::_allocate(1);
      new(node) TRB_Node<T>(value, pNIL);
      InsertNode(node, _root, _first, _last);
      return node;
    }
    // Searches for a node with the given key and removes it if found, otherwise returns false.
    BSS_FORCEINLINE bool Remove(const T& value) { return Remove(GetNode(value, _root)); }
    // Removes the given node. Returns false if node is null
    BSS_FORCEINLINE bool Remove(TRB_Node<T>* node)
    {
      if(!node) return false;
      RemoveNode(node, _root, _first, _last);
      node->~TRB_Node();
      cAllocTracker<Alloc>::_deallocate(node, 1);
      return true;
    }
    // Returns first element
    BSS_FORCEINLINE TRB_Node<T>* Front() const { return _first; }
    // Returns last element
    BSS_FORCEINLINE TRB_Node<T>* Back() const { return _last; }
    // Iteration functions
    inline LLIterator<const TRB_Node<T>> begin() const { return LLIterator<const TRB_Node<T>>(_first); }
    inline LLIterator<const TRB_Node<T>> end() const { return LLIterator<const TRB_Node<T>>(0); }
    inline LLIterator<TRB_Node<T>> begin() { return LLIterator<TRB_Node<T>>(_first); }
    inline LLIterator<TRB_Node<T>> end() { return LLIterator<TRB_Node<T>>(0); }

    inline cTRBtree& operator=(cTRBtree&& mov) // Move assignment operator
    {
      Clear();
      cAllocTracker<Alloc>::operator=(std::move(mov));
      _first = mov._first;
      _last = mov._last;
      _root = mov._root;
      mov._first = 0;
      mov._last = 0;
      mov._root = pNIL;
      return *this;
    }


    static void RemoveNode(TRB_Node<T>* node, TRB_Node<T>*& root, TRB_Node<T>*& first, TRB_Node<T>*& last)
    {
      if(node->color == -1) { LLRemove(node, first, last); return; }
      if(node->next && node->next->color == -1) { _replacenode(node, node->next, root); LLRemove(node, first, last); return; }

      LLRemove(node, first, last);
      TRB_Node<T>*  y;
      TRB_Node<T>*  z;

      if(node->left != pNIL && node->right != pNIL)
        y = _findmin(node->right);
      else
        y = node;

      z = y->children[y->left == pNIL];
      z->parent = y->parent;

      if(y->parent != 0)
        y->parent->children[y == y->parent->right] = z;
      else
        root = z;

      bool balance = (y->color == 0);

      if(y != node) _replacenode(node, y, root);
      if(balance) _fixdelete(z, root);
    }

    static void InsertNode(TRB_Node<T>* node, TRB_Node<T>*& root, TRB_Node<T>*& first, TRB_Node<T>*& last)
    {
      TRB_Node<T>* cur = root;
      TRB_Node<T>* parent = 0;
      int c;

      while(cur != pNIL)
      {
        parent = cur;
        switch(c = CFunc(node->value, cur->value))
        {
        case -1: cur = cur->left; break;
        case 1: cur = cur->right; break;
        default: // duplicate
          LLInsertAfter(node, cur, last);
          node->color = -1; //set color to duplicate
          return; //terminate, we have nothing else to do since this node isn't actually in the tree
        }
      }

      if(parent != 0)
      {
        node->parent = parent; //set parent

        if(c < 0) //depending on if its less than or greater than the parent, set the appropriate child variable
        {
          parent->left = node;
          LLInsert(node, parent, first); //Then insert into the appropriate side of the list
        }
        else
        {
          parent->right = node;
          if(parent->next != 0 && parent->next->color == -1) // This is required to support duplicate nodes
          { // What can happen is you get here with a value greater than the parent, then try to insert after the parent... but any duplicate values would then be ahead of you.
            if((parent = _treenextsub(parent)) == 0) last = LLAddAfter(node, last);
            else LLInsert(node, parent, first);
          }
          else
            LLInsertAfter(node, parent, last); // If there aren't any duplicate values in front of you, it doesn't matter.
        }
        _fixinsert(node, root);
      }
      else //this is the root node so re-assign
      {
        first = last = root = node; //assign to first and last
        root->color = 0; //root is always black (done below)
      }
    }
    static TRB_Node<T>* GetNode(const T& x, TRB_Node<T>* const& root)
    {
      TRB_Node<T>* cur = root;

      while(cur != pNIL)
      {
        switch(CFunc(x, cur->value)) //This is faster then if/else statements
        {
        case -1: cur = cur->left; break;
        case 1: cur = cur->right; break;
        default: return cur;
        }
      }

      return 0;
    }
    static TRB_Node<T>* GetNodeNear(const T& x, bool before, TRB_Node<T>* const& root)
    {
      TRB_Node<T>* cur = root;
      TRB_Node<T>* parent = pNIL;
      char res = 0;

      while(cur != pNIL)
      {
        parent = cur;
        switch(res = CFunc(x, cur->value)) //This is faster then if/else statements
        {
        case -1: cur = cur->left; break;
        case 1: cur = cur->right; break;
        default: return cur;
        }
      }

      if(before)
        return (res < 0 && parent->prev) ? parent->prev : parent;
      else
        return (res > 0 && parent->next) ? parent->next : parent;
    }
  protected:
    static void _leftrotate(TRB_Node<T>* node, TRB_Node<T>*& root)
    {
      TRB_Node<T>* r = node->right;

      node->right = r->left;
      if(node->right != pNIL) node->right->parent = node;
      if(r != pNIL) r->parent = node->parent;

      if(node->parent)
        node->parent->children[node->parent->right == node] = r;
      else
        root = r;

      r->left = node;
      if(node != pNIL) node->parent = r;
    }
    static void _rightrotate(TRB_Node<T>* node, TRB_Node<T>*& root)
    {
      TRB_Node<T>* r = node->left;

      node->left = r->right;
      if(node->left != pNIL) node->left->parent = node;
      if(r != pNIL) r->parent = node->parent;

      if(node->parent)
        node->parent->children[node->parent->right == node] = r;
      else
        root = r;

      r->right = node;
      if(node != pNIL) node->parent = r;
    }
    inline static void _fixinsert(TRB_Node<T>* node, TRB_Node<T>*& root)
    {
      while(node != root && node->parent->color == 1)
      {
        if(node->parent == node->parent->parent->left)
        {
          TRB_Node<T>* y = node->parent->parent->right;
          if(y->color == 1)
          {
            node->parent->color = 0;
            y->color = 0;
            node->parent->parent->color = 1;
            node = node->parent->parent;
          }
          else
          {
            if(node == node->parent->right)
            {
              node = node->parent;
              _leftrotate(node, root);
            }

            node->parent->color = 0;
            node->parent->parent->color = 1;
            _rightrotate(node->parent->parent, root);
          }
        }
        else
        {
          TRB_Node<T>* y = node->parent->parent->left;
          if(y->color == 1)
          {
            node->parent->color = 0;
            y->color = 0;
            node->parent->parent->color = 1;
            node = node->parent->parent;
          }
          else
          {
            if(node == node->parent->left)
            {
              node = node->parent;
              _rightrotate(node, root);
            }
            node->parent->color = 0;
            node->parent->parent->color = 1;
            _leftrotate(node->parent->parent, root);
          }
        }
      }

      root->color = 0;
    }
    inline static void _fixdelete(TRB_Node<T>* node, TRB_Node<T>*& root)
    {
      while(node != root && node->color == 0)
      {
        if(node == node->parent->left)
        {
          TRB_Node<T>* w = node->parent->right;
          if(w->color == 1)
          {
            w->color = 0;
            node->parent->color = 1;
            _leftrotate(node->parent, root);
            w = node->parent->right;
          }
          if(w->left->color == 0 && w->right->color == 0)
          {
            w->color = 1;
            node = node->parent;
          }
          else
          {
            if(w->right->color == 0)
            {
              w->left->color = 0;
              w->color = 1;
              _rightrotate(w, root);
              w = node->parent->right;
            }
            w->color = node->parent->color;
            node->parent->color = 0;
            w->right->color = 0;
            _leftrotate(node->parent, root);
            node = root;
          }
        }
        else
        {
          TRB_Node<T>* w = node->parent->left;
          if(w->color == 1)
          {
            w->color = 0;
            node->parent->color = 1;
            _rightrotate(node->parent, root);
            w = node->parent->left;
          }
          if(w->right->color == 0 && w->left->color == 0)
          {
            w->color = 1;
            node = node->parent;
          }
          else
          {
            if(w->left->color == 0)
            {
              w->right->color = 0;
              w->color = 1;
              _leftrotate(w, root);
              w = node->parent->left;
            }
            w->color = node->parent->color;
            node->parent->color = 0;
            w->left->color = 0;
            _rightrotate(node->parent, root);
            node = root;
          }
        }
      }
      node->color = 0;
    }
    inline static void _replacenode(TRB_Node<T>* node, TRB_Node<T>* y, TRB_Node<T>*& root)
    {
      y->color = node->color;
      y->left = node->left;
      y->right = node->right;
      y->parent = node->parent;

      if(y->parent != 0)
        y->parent->children[y->parent->right == node] = y;
      else
        root = y;

      y->left->parent = y;
      y->right->parent = y;
    }
    BSS_FORCEINLINE static TRB_Node<T>* _findmin(TRB_Node<T>* node)
    {
      while(node->left != pNIL) node = node->left;
      return node;
    }
    inline static TRB_Node<T>* _treenextsub(TRB_Node<T>* node)
    {
      while(node->parent && node != node->parent->left)
        node = node->parent;
      return node->parent;
    }

    TRB_Node<T>*  _first;
    TRB_Node<T>*  _last;
    TRB_Node<T>*  _root;
    static TRB_Node<T> NIL;
    static TRB_Node<T>* pNIL;
  };

  template<class T, char(*C)(const T&, const T&), typename A>
  TRB_Node<T> cTRBtree<T, C, A>::NIL(&cTRBtree<T, C, A>::NIL);
  template<class T, char(*C)(const T&, const T&), typename A>
  TRB_Node<T>* cTRBtree<T, C, A>::pNIL = &cTRBtree<T, C, A>::NIL;
}

#endif
