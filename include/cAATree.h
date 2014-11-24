// Copyright ©2014 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AA_TREE_H__BSS__
#define __C_AA_TREE_H__BSS__

#include "bss_compare.h"
#include "bss_alloc_fixed.h"

namespace bss_util {
  template<typename T>
  struct AANODE
  {
    int level;
    AANODE* left;
    AANODE* right;
    T data;
  };

  // An AA tree, similar to a left leaning red-black tree, except not completely stupid.
  template<typename T, char(*CFunc)(const T&, const T&) = CompT<T>, typename Alloc = FixedPolicy<AANODE<T>>>
  class cAATree : cAllocTracker<Alloc>
  {
  public:
    inline cAATree(Alloc* alloc = 0) : cAllocTracker<Alloc>(alloc), _sentinel(allocate(1))
    {
      _sentinel->level = 0;
      _sentinel->left = _sentinel->right = _sentinel;
    }
    inline ~cAATree() { Clear(); }
    inline void Insert(const T& data) { _insert(_root, data); }
    inline bool Remove(const T& data) { return _remove(_root, data) != _sentinel; }
    inline void Clear() { }
    inline AANODE* GetRoot() { return _root; }
    inline bool IsEmpty() { return !_root; }
    template<void (BSS_FASTCALL *FACTION)(T&)>
    inline void Traverse() { _traverse<FACTION>(_root); }
    
  protected:
    template<void (BSS_FASTCALL *FACTION)(T&)>
    inline void _traverse(AANODE* n)
    {
      _traverse(n->left);
      FACTION(n->data);
      _traverse(n->right);
    }
    inline void _skew(AANODE*& n)
    {
      if(n->level == n->left->level) {
        AANODE* l = n->left;
        n->left = l->right;
        l->right = n;
        n = l;
      }
    }
    inline void _split(AANODE*& n)
    {
      if(n->right->right->level == n->level) {
        AANODE* r = n->right;
        n->right = r->left;
        r->left = n;
        r->level++;
        n = r;
      }
    }
    inline void _insert(AANODE*& n, const T& data)
    {
      if(n == _sentinel)
      {
        n = allocate(1);
        n->left = n->right = _sentinel;
        n->data = data;
        n->level = 1;
        return;
      }
      if(CFunc(data, n->data)<0)
        _insert(n->left, data);
      else
        _insert(n->right, data);

      _skew(n);
      _split(n);
    }
    inline void _delete(AANODE*& n, const T& data)
    {
      static AANODE* lastNode = 0;
      static AANODE** deletedNode = &_sentinel;

      if(n != _sentinel)
      {
        lastNode = n;
        if(CFunc(data, n->data) < 0)
          remove(n->left, data);
        else
        {
          deletedNode = &n;
          remove(n->right, data);
        }

        if(n == lastNode) // Instead of swapping values here, we swap the actual nodes, then delete the one we actually want to delete.
        {
          if(deletedNode == &_sentinel || CFunc(data, deletedNode->data) != 0)
            return;   // not found
          AANODE* hold = *deletedNode; // store current value of deletedNode (do this before we set n, because deletedNode can actually equal n)
          n = n->right; // Set n's parent to point to n->right, orphaning n. Since lastNode is equal to n, lastNode is now pointing to the orphan.
          if(hold != lastNode)
          {
            lastNode->left = hold->left; // Copy all of the deletedNode's values over to our orphaned node
            lastNode->right = hold->right;
            lastNode->level = hold->level;
            *deletedNode = lastNode; // Set the deletedNode's parent to point to our orphaned node.
          }
          deletedNode = &_sentinel; // Set the actual deletedNode variable to point to the sentinel value
          deallocate(hold); // hold is still pointing to the deletedNode, which is now removed from the tree, so we can delete it.
        } 
        else if(n->left->level < n->level - 1 || n->right->level < n->level - 1)
        {
          if(n->right->level > --n->level)
            n->right->level = n->level;
          _skew(n);
          _skew(n->right);
          _skew(n->right->right);
          _split(n);
          _split(n->right);
        }
      }
    }

    AANODE* _sentinel;
    AANODE* _root;
  };
}

#endif
//
//struct node * inOrderSuccessor(struct node *root, struct node *n)
//{
//  // step 1 of the above algorithm
//  if(n->right != NULL)
//    return minValue(n->right);
//
//  struct node *succ = NULL;
//
//  // Start from root and search for successor down the tree
//  while(root != NULL)
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