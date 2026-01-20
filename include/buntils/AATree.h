// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __AA_TREE_H__BUN__
  #define __AA_TREE_H__BUN__

  #include "compare.h"
  #include "BlockAllocMT.h"

namespace bun {
  template<typename T> struct AANODE
  {
    int level;
    AANODE* left;
    AANODE* right;
    T data;
  };

  // An AA tree, similar to a left leaning red-black tree, except not completely stupid.
  template<typename T, Comparison<T, T> Comp = std::compare_three_way,
           typename Alloc = PolicyAllocator<AANODE<T>, BlockPolicy>>
  class BUN_EMPTY_BASES AATree : protected Alloc, protected CompressedBase<Comp>
  {
    using CompressedBase<Comp>::_getbase;

  public:
    inline AATree()
      requires std::is_default_constructible_v<Alloc> && std::is_default_constructible_v<Comp>
      : AATree(Alloc(), Comp())
    {}
    inline explicit AATree(Alloc&& alloc)
      requires std::is_default_constructible_v<Comp>
      : AATree(std::forward<Alloc>(alloc), Comp())
    {}
    inline explicit AATree(const Comp& f)
      requires std::is_default_constructible_v<Alloc>
      : AATree(Alloc(), f)
    {}
    inline AATree(const Alloc& alloc, const Comp& f) :
      Alloc(alloc), CompressedBase<Comp>(f), _sentinel(std::allocator_traits<Alloc>::allocate(*this, 1))
    {
      _sentinel->level = 0;
      _sentinel->left = _sentinel->right = _sentinel;
      _root                              = _sentinel;
    }
    inline ~AATree()
    {
      Clear();
      std::allocator_traits<Alloc>::deallocate(*this, _sentinel, 1);
    }
    inline void Insert(const T& data) { _insert(_root, data); }
    inline bool Remove(const T& data) { return _remove(_root, data); }
    // Gets the node that belongs to the data or returns nullptr if not found.
    inline AANODE<T>* Get(const T& data)
    {
      AANODE<T>* cur = _root;

      while(cur != _sentinel)
      {
        auto r = _getbase()(data, cur->data);
        if(r < 0)
          cur = cur->left;
        else if(r > 0)
          cur = cur->right;
        else
          return cur;
      }

      return 0;
    }
    inline void Clear() { _clear(_root); }
    // Gets the root, unless the tree is empty, in which case returns nullptr
    inline AANODE<T>* GetRoot() { return _root == _sentinel ? 0 : _root; }
    inline bool IsEmpty() { return _root == _sentinel; }
    // Does an in-order traversal of the tree, applying FACTION to each node's data object.
    template<void (*FACTION)(T&)> inline void Traverse() { _traverse<FACTION>(_root); }

  protected:
    void _clear(AANODE<T>* n)
    {
      if(n == _sentinel)
        return;
      _clear(n->left);
      _clear(n->right);
      n->~AANODE<T>();
      std::allocator_traits<Alloc>::deallocate(*this, n, 1);
    }
    template<void (*FACTION)(T&)> inline void _traverse(AANODE<T>* n)
    {
      _traverse(n->left);
      FACTION(n->data);
      _traverse(n->right);
    }
    inline void _skew(AANODE<T>*& n)
    {
      if(n->level == n->left->level)
      {
        AANODE<T>* l = n->left;
        n->left      = l->right;
        l->right     = n;
        n            = l;
      }
    }
    inline void _split(AANODE<T>*& n)
    {
      if(n->right->right->level == n->level)
      {
        AANODE<T>* r = n->right;
        n->right     = r->left;
        r->left      = n;
        r->level++;
        n = r;
      }
    }
    inline void _insert(AANODE<T>*& n, const T& data)
    {
      if(n == _sentinel)
      {
        n       = std::allocator_traits<Alloc>::allocate(*this, 1);
        n->left = n->right = _sentinel;
        n->data            = data;
        n->level           = 1;
        return;
      }

      if(_getbase()(data, n->data) < 0)
        _insert(n->left, data);
      else
        _insert(n->right, data);

      _skew(n);
      _split(n);
    }
    inline bool _remove(AANODE<T>*& n, const T& data)
    {
      if(n == _sentinel)
        return false;

      _last  = n;
      bool b = false;

      if(_getbase()(data, n->data) < 0)
        b = _remove(n->left, data);
      else
      {
        _deleted = n;
        b        = _remove(n->right, data);
      }

      if(n == _last && _deleted != _sentinel && (_getbase()(data, _deleted->data) == 0))
      {
        _deleted->data = n->data;
        _deleted       = _sentinel;
        n              = n->right;
        std::allocator_traits<Alloc>::deallocate(*this, _last, 1);
        _last = _sentinel;
        b     = true;
      }
      else if(n->left->level < n->level - 1 || n->right->level < n->level - 1)
      {
        n->level--;
        if(n->right->level > n->level)
          n->right->level = n->level;
        _skew(n);
        _skew(n->right);
        _skew(n->right->right);
        _split(n);
        _split(n->right);
      }

      return b;
    }

    AANODE<T>* _deleted;
    AANODE<T>* _last;
    AANODE<T>* _sentinel;
    AANODE<T>* _root;
  };
}

#endif

/* This deletion method doesn't work because nodes get moved around after deletion, making swapping two nodes in memory
almost impossible. In order to preserve memory locations, an entirely different deletion method is required static
AANODE<T>* lastNode = 0; static AANODE<T>** deletedNode = &_sentinel; bool r = true;

if(*n != _sentinel)
{
lastNode = *n;
if(_getbase()(data, (*n)->data) < 0)
r = _remove(&(*n)->left, data);
else
{
deletedNode = n;
r = _remove(&(*n)->right, data);
}

if(*n == lastNode) // Instead of swapping values here, we swap the actual nodes, then delete the one we actually want to
delete.
{
if(deletedNode == &_sentinel || _getbase()(data, (*deletedNode)->data) != 0)
return false;   // not found
AANODE<T>* hold = *deletedNode; // store current value of deletedNode (do this before we set n, because deletedNode can
actually equal n) *n = (*n)->right; // Set n's parent to point to n->right, orphaning n. Since lastNode is equal to n,
lastNode is now pointing to the orphan. if(hold != lastNode)
{
lastNode->left = hold->left; // Copy all of the deletedNode's values over to our orphaned node
lastNode->right = hold->right;
lastNode->level = hold->level;
*deletedNode = lastNode; // Set the deletedNode's parent to point to our orphaned node.
}
deletedNode = &_sentinel; // Set the actual deletedNode variable to point to the sentinel value
_deallocate(hold); // hold is still pointing to the deletedNode, which is now removed from the tree, so we can delete it.
}
else if((*n)->left->level < (*n)->level - 1 || (*n)->right->level < (*n)->level - 1)
{
if((*n)->right->level > --(*n)->level)
(*n)->right->level = (*n)->level;
_skew(*n);
_skew((*n)->right);
_skew((*n)->right->right);
_split(*n);
_split((*n)->right);
}
}

return r;*/

//
// struct node * inOrderSuccessor(struct node *root, struct node *n)
//{
//  // step 1 of the above algorithm
//  if(n->right != nullptr)
//    return minValue(n->right);
//
//  struct node *succ = nullptr;
//
//  // Start from root and search for successor down the tree
//  while(root != nullptr)
//  {
//    if(n->data < root->data)
//    {
//      succ = root;
//      root = root->left;
//    } else if(n->data > root->data)
//      root = root->right;
//    else
//      break;
//  }
//
//  return succ;
//}
