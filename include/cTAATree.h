// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_T_AA_TREE_H__BSS__
#define __C_T_AA_TREE_H__BSS__

#include "bss_alloc_fixed.h"
#include "bss_compare.h"

namespace bss_util {
  template<class TKey, class TVal, typename TLEVEL>
  struct BSS_COMPILER_DLLEXPORT TAATreeNode
  {
   TAATreeNode* next;
   TAATreeNode* prev;
   TAATreeNode* left;
   TAATreeNode* right;
   TLEVEL level;
   TKey key;
   TVal value;
  };

  /* Single-Threaded AA Tree (A variant of the RBT tree). */ //BROKEN
  template<class TKey, class TVal, char (*CFunc)(const TKey& keyleft, const TKey& keyright)=CompT<TKey>, typename TLEVEL=unsigned short, typename TSIZE=unsigned int, class Alloc=bss_util::Allocator<TAATreeNode<TKey, TVal, TLEVEL>,bss_util::FixedChunkPolicy<TAATreeNode<TKey, TVal, TLEVEL>>>>
  class BSS_COMPILER_DLLEXPORT cTAATree : cAllocTracker<Alloc>
  {
  public:
    typedef TAATreeNode<TKey,TVal,TLEVEL> TNODE;

    inline cTAATree() : _root(0), _first(0), _last(0) {}
    inline ~cTAATree() {}
    inline void Clear()
    {
      TNODE* hold;
      while(_root)
      { 
        hold=_root->next;
        _alloc->deallocate(_root);
        _root=hold;
      }
    }

    inline void BSS_FASTCALL Insert(const TKey key, const TVal data)
    {
      if(!_root)
      {
        _root=_allocnew(key,data);
        _root->next=0;
        _root->prev=0;
        _first=_root;
        _last=_root;
      }
      else
        _root=_insert(_root,key,data);
    }
    inline const TNODE* GetRoot() const { return _root; }
    inline const TNODE* GetFirst() const { return _first; }
    inline const TNODE* GetLast() const { return _last; }
    //inline bool BSS_FASTCALL Remove(const TKey key)
    //{

    //}

  protected:
    inline TNODE* BSS_FASTCALL _insert(TNODE* node, const TKey key, const TVal data)
    {
      assert(node!=&_nil);
      char cmp = CFunc(key,node->key);
      TNODE* retval=0;
      if(cmp<0) {
        retval=(node->left=(node->left==&_nil)?_nullcheckprev(node,key,data):_insert(node->left,key,data));
      } else if(cmp>0) {
        retval=(node->right=(node->right==&_nil)?_nullcheck(node,key,data):_insert(node->right,key,data));
      } else {
        //_nullcheck(node,key,data)->level=(TLEVEL)-1;
        return node;
      }
      
      retval=_skew(retval);
      return _split(retval);
    }
    //Inserts after
    inline TNODE* _nullcheck(TNODE* parent, const TKey key, const TVal data)
    {
      TNODE* retval=_allocnew(key,data);
      retval->next=parent->next;
      retval->prev=parent;
      parent->next=retval;
      if(!retval->next) _last=retval;
      else retval->next->prev=retval;
      return retval;
    }
    //Inserts before
    inline TNODE* _nullcheckprev(TNODE* parent, const TKey key, const TVal data)
    {
      TNODE* retval=_allocnew(key,data);
      retval->next=parent;
      retval->prev=parent->prev;
      parent->prev=retval;
      if(!retval->prev) _first=retval;
      else retval->prev->next=retval;
      return retval;
    }
    inline static TNODE* BSS_FASTCALL _skew(TNODE* node)
    {
      if (node!=&_nil && node->left->level == node->level)
      {
        TNODE* L = node->left;
        node->left = L->right;
        L->right=node;
        return L;
      }
      return node;
    }
    inline static TNODE* BSS_FASTCALL _split(TNODE* node)
    {
      if (node!=&_nil && node->level == node->right->right->level)
      {
        TNODE* R = node->right;
        node->right = R->left;
        R->left = node;
        ++R->level;
        return R;
      }
      return node;
    }

    inline TNODE* BSS_FASTCALL _allocnew(const TKey key, const TVal value)
    {
      TNODE* retval=_alloc->allocate(1);
      //retval->next=parent->next;
      //parent->next=retval;
      retval->left=&_nil;
      retval->right=&_nil;
      retval->level=1;
      retval->key=key;
      retval->value=value;
      return retval;
    }
    TNODE* _root;
    TNODE* _first;
    TNODE* _last;
    static TNODE _nil;
    //TSIZE _size;
  };

  template<class TKey, class TVal, char (*CFunc)(const TKey& keyleft, const TKey& keyright), typename TLEVEL, typename TSIZE, class Alloc>
  TAATreeNode<TKey,TVal,TLEVEL> cTAATree<TKey,TVal,CFunc,TLEVEL,TSIZE,Alloc>::_nil = { 0, 0, &cTAATree<TKey,TVal,CFunc,TLEVEL,TSIZE,Alloc>::_nil, &cTAATree<TKey,TVal,CFunc,TLEVEL,TSIZE,Alloc>::_nil, 0, TKey(), TVal() };
}

#endif