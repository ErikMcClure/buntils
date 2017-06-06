// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"
// Insert/delete implementations modified from Arjan van den Boogaard (2004)
// http://archive.gamedev.net/archive/reference/programming/features/TStorage/TStorage.h

#ifndef __TRB_TREE_H__BSS__
#define __TRB_TREE_H__BSS__

#include "compare.h"
#include "Alloc.h"
#include "LLBase.h"

namespace bss {
  namespace internal {
  // Generic Threaded Red-black tree node
    template<class T>
    struct BSS_COMPILER_DLLEXPORT TRB_NodeBase : LLBase<T>
    {
      using LLBase<T>::next;
      using LLBase<T>::prev;

      inline explicit TRB_NodeBase(T* pNIL, char c = 1) : left(pNIL), right(pNIL), color(c), parent(0) { next = 0; prev = 0; }
      inline TRB_NodeBase(TRB_NodeBase&& mov, T*& root, T*& first, T*& last, T* pNIL)
      {
        parent = mov.parent;

        if(parent)
        {
          if(parent->left == &mov) parent->left = this;
          else if(parent->right == &mov) parent->right = this;
          else assert(false);
        }
        else
          root = this;

        left = mov.left;
        if(left != pNIL) { assert(left->parent == &mov); left->parent = this; }
        right = mov.right;
        if(right != pNIL) { assert(right->parent == &mov); right->parent = this; }
        next = mov.next;
        if(next) { assert(next->prev == &mov); next->prev = this; }
        else { assert(last == &mov); last = this; }
        prev = mov.prev;
        if(prev) { assert(prev->next == &mov); prev->next = this; }
        else { assert(root == &mov); root = this; }

        color = mov.color;
        mov.left = pNIL;
        mov.right = pNIL;
        mov.next = 0;
        mov.prev = 0;
        mov.parent = 0;
      }
      T* parent;
      union {
        struct {
          T* left;
          T* right;
        };
        T* children[2];
      };
      char color; // 0 - black, 1 - red, -1 - duplicate

      static void RemoveNode(T* node, T*& root, T*& first, T*& last, T* pNIL)
      {
        if(node->color == -1) { LLRemove(node, first, last); return; }
        if(node->next && node->next->color == -1)
        {
          _replaceNode(node, node->next, root);
          LLRemove(node, first, last);
          pNIL->parent = 0;
          return;
        }

        LLRemove(node, first, last);
        T*  y;
        T*  z;

        if(node->left != pNIL && node->right != pNIL)
          y = _findMin(node->right, pNIL);
        else
          y = node;

        z = y->children[y->left == pNIL];
        z->parent = y->parent;

        if(y->parent != 0)
          y->parent->children[y == y->parent->right] = z;
        else
          root = z;

        bool balance = (y->color == 0);

        if(y != node) _replaceNode(node, y, root);
        if(balance) _fixDelete(z, root, pNIL);
        pNIL->parent = 0;
        assert(pNIL->color == 0);
      }

      template<char(*CFunc)(const T&, const T&)>
      static void InsertNode(T* node, T*& root, T*& first, T*& last, T* pNIL)
      {
        assert(node != pNIL);
        T* cur = root;
        T* parent = 0;
        int c;

        while(cur != pNIL)
        {
          parent = cur;
          switch(c = CFunc(*node, *cur))
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
              if((parent = _treeNextSub(parent)) == 0) last = LLAddAfter(node, last);
              else LLInsert(node, parent, first);
            }
            else
              LLInsertAfter(node, parent, last); // If there aren't any duplicate values in front of you, it doesn't matter.
          }
          _fixInsert(node, root, pNIL);
        }
        else //this is the root node so re-assign
        {
          first = last = root = node; //assign to first and last
          root->color = 0; //root is always black (done below)
        }
      }

    protected:
      static void _leftRotate(T* node, T*& root, T* pNIL)
      {
        T* r = node->right;

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
      static void _rightRotate(T* node, T*& root, T* pNIL)
      {
        T* r = node->left;

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
      inline static void _fixInsert(T* node, T*& root, T* pNIL)
      {
        while(node != root && node->parent->color == 1)
        {
          if(node->parent == node->parent->parent->left)
          {
            T* y = node->parent->parent->right;

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
                _leftRotate(node, root, pNIL);
              }

              node->parent->color = 0;
              node->parent->parent->color = 1;
              _rightRotate(node->parent->parent, root, pNIL);
            }
          }
          else
          {
            T* y = node->parent->parent->left;

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
                _rightRotate(node, root, pNIL);
              }

              node->parent->color = 0;
              node->parent->parent->color = 1;
              _leftRotate(node->parent->parent, root, pNIL);
            }
          }
        }

        root->color = 0;
      }
      inline static void _fixDelete(T* node, T*& root, T* pNIL)
      {
        while(node != root && node->color == 0)
        {
          if(node == node->parent->left)
          {
            T* w = node->parent->right;
            assert(w != pNIL);
            if(w->color == 1)
            {
              w->color = 0;
              node->parent->color = 1;
              _leftRotate(node->parent, root, pNIL);
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
                _rightRotate(w, root, pNIL);
                w = node->parent->right;
              }

              w->color = node->parent->color;
              node->parent->color = 0;
              w->right->color = 0;
              _leftRotate(node->parent, root, pNIL);
              node = root;
            }
          }
          else
          {
            T* w = node->parent->left;
            assert(w != pNIL);
            if(w->color == 1)
            {
              w->color = 0;
              node->parent->color = 1;
              _rightRotate(node->parent, root, pNIL);
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
                _leftRotate(w, root, pNIL);
                w = node->parent->left;
              }

              w->color = node->parent->color;
              node->parent->color = 0;
              w->left->color = 0;
              _rightRotate(node->parent, root, pNIL);
              node = root;
            }
          }
        }
        node->color = 0;
      }
      inline static void _replaceNode(T* node, T* y, T*& root)
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
      BSS_FORCEINLINE static T* _findMin(T* node, T* pNIL)
      {
        while(node->left != pNIL) node = node->left;
        return node;
      }
      inline static T* _treeNextSub(T* node)
      {
        while(node->parent && node != node->parent->left)
          node = node->parent;
        return node->parent;
      }
    };
  }

  // Threaded Red-black tree node with a value
  template<class T>
  struct BSS_COMPILER_DLLEXPORT TRB_Node : internal::TRB_NodeBase<TRB_Node<T>>
  {
    inline explicit TRB_Node(TRB_Node* pNIL) : internal::TRB_NodeBase<TRB_Node<T>>(pNIL, 0) {}
    inline TRB_Node(T v, TRB_Node* pNIL) : value(v), internal::TRB_NodeBase<TRB_Node<T>>(pNIL) {}
    T value;
  };

  // Threaded Red-black tree implementation
  template<typename T, char(*CFunc)(const T&, const T&) = CompT<T>, typename Alloc = StandardAllocPolicy<TRB_Node<T>>>
  class BSS_COMPILER_DLLEXPORT TRBtree : protected AllocTracker<Alloc>
  {
    inline TRBtree(const TRBtree&) BSS_DELETEFUNC
      inline TRBtree& operator=(const TRBtree&) BSS_DELETEFUNCOP

      BSS_FORCEINLINE static char CompNode(const TRB_Node<T>& l, const TRB_Node<T>& r) { return CFunc(l.value, r.value); }

  public:
    inline explicit TRBtree(Alloc* allocator = 0) : AllocTracker<Alloc>(allocator), _first(0), _last(0), _root(&NIL), NIL(&NIL) {}
    // Destructor
    inline ~TRBtree() { Clear(); }
    // Clears the tree
    inline void Clear()
    {
      for(auto i = begin(); i.IsValid();) // Walk through the tree using the linked list and deallocate everything
      {
        (*i)->~TRB_Node();
        AllocTracker<Alloc>::_deallocate(*(i++), 1);
      }

      _first = 0;
      _last = 0;
      _root = &NIL;
    }
    // Retrieves a given node by key if it exists
    BSS_FORCEINLINE TRB_Node<T>* Get(const T& value) const { return GetNode(value, _root, &NIL); }
    // Retrieves the node closest to the given key.
    BSS_FORCEINLINE TRB_Node<T>* GetNear(const T& value, bool before = true) const { return GetNodeNear(value, before, _root, &NIL); }
    // Inserts a key with the associated data
    BSS_FORCEINLINE TRB_Node<T>* Insert(const T& value)
    {
      TRB_Node<T>* node = AllocTracker<Alloc>::_allocate(1);
      new(node) TRB_Node<T>(value, &NIL);
      TRB_Node<T>::template InsertNode<CompNode>(node, _root, _first, _last, &NIL);
      return node;
    }
    // Searches for a node with the given key and removes it if found, otherwise returns false.
    BSS_FORCEINLINE bool Remove(const T& value) { return Remove(GetNode(value, _root, &NIL)); }
    // Removes the given node. Returns false if node is null
    BSS_FORCEINLINE bool Remove(TRB_Node<T>* node)
    {
      if(!node) 
        return false;

      TRB_Node<T>::RemoveNode(node, _root, _first, _last, &NIL);
      node->~TRB_Node();
      AllocTracker<Alloc>::_deallocate(node, 1);
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

    static TRB_Node<T>* GetNode(const T& x, TRB_Node<T>* const& root, TRB_Node<T>* pNIL)
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
    static TRB_Node<T>* GetNodeNear(const T& x, bool before, TRB_Node<T>* const& root, TRB_Node<T>* pNIL)
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
    TRB_Node<T>*  _first;
    TRB_Node<T>*  _last;
    TRB_Node<T>*  _root;
    mutable TRB_Node<T> NIL;
  };
}

#endif