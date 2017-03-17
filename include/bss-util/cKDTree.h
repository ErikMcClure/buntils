// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_KD_TREE_H__BSS__
#define __C_KD_TREE_H__BSS__

#include "bss_compare.h"
#include "bss_alloc.h"
#include "LLBase.h"
#include <float.h> // for FLT_MAX on linux

namespace bss_util {
  // Node for the KD-tree 
  template<typename T>
  struct KDNode {
    uint32_t num;
    float div;
    union {
      struct {
        struct KDNode* right; //not needed for in-array implementation
        struct KDNode* left;
      };
      struct KDNode* _children[2];
    };
    struct KDNode* parent;
    T* items;
    short balance;
    char axis; // This could be absorbed into num to allow balance to become a full 32-bit int.
    float total[2]; //holds the true accumulations. This may cause precision problems, but calculating and updating the true average is more expensive
  };

  // KD-tree storing arbitrary rectangles. Requires a function turning T into a float[4] reference, LLBASE<T> list, and an action.
  template<typename T, typename Alloc, const float* (*FRECT)(T*), LLBase<T>& (*FLIST)(T*), KDNode<T>*& (*FNODE)(T*)>
  class cKDTree : protected cAllocTracker<Alloc>
  {
    inline cKDTree(const cKDTree&) BSS_DELETEFUNC
      inline cKDTree& operator=(const cKDTree&)BSS_DELETEFUNCOP
  public:
    inline explicit cKDTree(uint32_t rb=RBTHRESHOLD, Alloc* alloc=0) : cAllocTracker<Alloc>(alloc), _root(0), _rbthreshold(rb) {}
    inline cKDTree(cKDTree&& mov) : cAllocTracker<Alloc>(std::move(mov)), _root(mov._root), _rbthreshold(mov._rbthreshold) { mov._root=0; }
    inline ~cKDTree() { Clear(); }
    inline void Clear() { if(_root) _destroynode(_root); _root=0; }
    template<void(*F)(T*)>
    void Traverse(float(&rect)[4]) const { if(_root) _traverse<0, 1, F>(_root, rect); }
    template<typename F>
    void TraverseAction(float(&rect)[4], F && f) const { if(_root) _traverseaction<0, 1, F>(_root, rect, std::forward<F>(f)); }
    template<typename F>
    void TraverseAll(F && f) { _traverseall(_root, std::forward<F>(f)); }
    void Insert(T* item)
    {
      KDNode<T>** p=&_root;
      KDNode<T>* h;
      KDNode<T>* parent=0;
      KDNode<T>* rb=0;
      const float* r = FRECT(item);
      char axis=0;
      while(h=*p)
      {
        h->total[0]+=r[0]+r[2];
        h->total[1]+=r[1]+r[3];
        h->num++;
        if(r[axis+2]<h->div) { // Drop into left
          h->balance-=1;
          p=&h->left;
        } else if(r[axis]>h->div) { // Drop into right
          h->balance+=1;
          p=&h->right;
        } else //insert here
          break;
        axis=((axis+1)&1);
        parent=h;
        if(!rb && (abs(h->balance*100)/h->num)>_rbthreshold) rb=h;
      }
      if(!h)
      {
        h=_allocnode(parent, axis);
        h->total[0]=r[0]+r[2];
        h->total[1]=r[1]+r[3];
        h->num=1;
        h->div=h->total[axis]/2;
        *p=h;
      }
      _insertitem(h, item);
      if(rb) Rebalance(rb);
    }
    void Remove(T* item)
    { // run up tree, adjust balance and check for rebalancing
      KDNode<T>* node=FNODE(item);
      KDNode<T>* rb=0;
      KDNode<T>* prev=node;
      const float* r = FRECT(item);
      _removeitem(node, item); // If the node becomes empty, we'll only remove it after a rebalance is triggered
      ++node->balance; //we're about to subtract one so counter it here for the bottom node (whose balance won't be changed)

      while(node)
      {
        --node->num;
        node->total[0]-=r[0]+r[2];
        node->total[1]-=r[1]+r[3];
        node->balance += ((node->left==prev)?1:-1); // This is the inverse balance value
        if(node->num>0 && (abs(node->balance*100)/node->num)>_rbthreshold) rb=node; // It's possible for --node->num to leave it at 0
        node=node->parent;
      }

      if(rb) Rebalance(rb);
    }
    inline cKDTree& operator=(cKDTree&& mov) // Move assignment operator
    {
      Clear();
      cAllocTracker<Alloc>::operator=(std::move(mov));
      _root=mov._root;
      mov._root=0;
      return *this;
    }
    BSS_FORCEINLINE void Rebalance(KDNode<T>* node)
    {
      float rect[4]={ -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };
      KDNode<T>* parents[4] ={ 0 };
      float total[2];
      _rebalance(node, rect, parents, total); // We don't bother trying to do a checkdestroy here because it isn't possible. In order for this node to be rebalanced, it must have images, and those images cannot go higher than this node in the tree.
    }
    inline void InsertRoot(T* item)
    {
      if(!_root) _root=_allocnode(0, 0);
      const float* r;
      r=FRECT(item);
      _root->total[0]+=r[0]+r[2];
      _root->total[1]+=r[1]+r[3];
      ++_root->num;
      _insertitem(_root, item);
    }
    inline void Solve() { if(_root) _solve(&_root); }
    inline KDNode<T>* GetRoot() { return _root; }
    inline uint32_t GetRBThreshold() const { return _rbthreshold; }
    inline void SetRBThreshold(uint32_t rbthreshold) { _rbthreshold=rbthreshold; }

    const static uint32_t RBTHRESHOLD=20; //default threshold

  protected:
    void _solve(KDNode<T>** pnode)
    {
      KDNode<T>* node=*pnode;
      if(!node->num) { _destroynode(node); *pnode=0; return; }
      node->div=node->total[node->axis]/(node->num*2);

      KDNode<T>** p;
      KDNode<T>* h;
      const float* r;
      T* item;
      T* next=node->items;
      while((item=next))
      {
        next=FLIST(item).next;
        r=FRECT(item);
        if(r[node->axis+2]<node->div) p=&node->left; // Drop into left
        else if(r[node->axis]>node->div) p=&node->right; // Drop into right
        else continue;
        if(!(*p)) // If null create the node
          *p=_allocnode(node, ((node->axis+1)&1));
        _removeitem(node, item);
        _insertitem(h=*p, item);
        h->total[0]+=r[0]+r[2];
        h->total[1]+=r[1]+r[3];
        ++h->num;
      }

      if(node->left) _solve(&node->left);
      if(node->right) _solve(&node->right);
    }

    void _rebalance(KDNode<T>* node, const float(&r)[4], KDNode<T>* const* p, float(&total)[2])
    {
      node->div=node->total[node->axis]/(node->num*2);
      node->balance=0;
      total[0]-=node->total[0]; // Subtract our total from the parent total
      total[1]-=node->total[1];

      float rect[4] ={ r[0], r[1], r[2], r[3] }; // Run down through the tree, resetting all the divider lines to whatever the current total is.
      KDNode<T>* parents[4] ={ p[0], p[1], p[2], p[3] };
      char axis=node->axis;
      if(node->left)
      {
        node->num-=node->left->num;
        rect[axis+2]=node->div;
        parents[axis+2]=node;
        _rebalance(node->left, rect, parents, node->total);
        node->num+=node->left->num;
        _checkdestroy(node->left);
        rect[axis+2]=r[axis+2]; // Reset so we don't mess up node below
      }
      if(node->right)
      {
        node->num-=node->right->num;
        rect[axis]=node->div;
        parents[axis]=node;
        _rebalance(node->right, rect, parents, node->total);
        node->num+=node->right->num;
        _checkdestroy(node->right);
      }

      T* item;
      T* next=node->items;
      const float* itemr;
      KDNode<T>* par;
      char i;
      while(item=next)
      {
        next=FLIST(item).next;
        itemr=FRECT(item);
        if(itemr[0] < r[0]) i=0; // Check for a violation. We don't bother checking to see if there are multiple violations, because 
        if(itemr[1] < r[1]) i=1; // if that happens, we'll catch it on our way back up the stack.
        if(itemr[2] > r[2]) i=2;
        if(itemr[3] > r[3]) i=3;
        else continue;
        node->total[0]-=itemr[0]+itemr[2]; // Remove image from us
        node->total[1]-=itemr[1]+itemr[3];
        --node->num;
        _removeitem(node, item);
        par=p[i];
        par->total[0]+=itemr[0]+itemr[2]; // Add image to violated parent. We don't worry about the middle, because the rebalance will
        par->total[1]+=itemr[1]+itemr[3]; // fix all the intermediate totals and nums for us.
        ++par->num;
        _insertitem(par, item);
      }

      total[0]+=node->total[0]; // Now add our new total back to the parent
      total[1]+=node->total[1];
    }
    inline void _checkdestroy(KDNode<T>*& node)
    {
      if(!node->num)
      {
        assert(!node->items);
        _destroynode(node);
        node=0;
      }
    }
    inline void _destroynode(KDNode<T>* node)
    {
      if(node->left) _destroynode(node->left);
      if(node->right) _destroynode(node->right);
      cAllocTracker<Alloc>::_deallocate(node); //Deallocate node
    }
    template<typename F>
    static void _traverseall(KDNode<T>* node, const F& f)
    {
      if(!node) return;
      f(node->items);
      _traverseall(node->left, f);
      _traverseall(node->right, f);
    }
    template<char cur, char next, void(*FACTION)(T*)>
    static void _traverse(const KDNode<T>* node, const float(&rect)[4])
    {
      T* item=node->items;
      const float* r;
      while(item)
      {
        r=FRECT(item);
        if(r[0] <= rect[2] && r[1] <= rect[3] && r[2] >= rect[0] && r[3] >= rect[1])
          FACTION(item);
        item=FLIST(item).next;
      }

      if(node->div >= rect[cur] && node->left!=0)
        _traverse<next, cur, FACTION>(node->left, rect);
      if(node->div <= rect[cur+2] && node->right!=0)
        _traverse<next, cur, FACTION>(node->right, rect);
    }
    template<char cur, char next, typename F>
    static void _traverseaction(const KDNode<T>* node, const float(&rect)[4], F && f)
    {
      T* item=node->items;
      const float* r;
      while(item)
      {
        r=FRECT(item);
        if(r[0] <= rect[2] && r[1] <= rect[3] && r[2] >= rect[0] && r[3] >= rect[1])
          f(item);
        item=FLIST(item).next;
      }

      if(node->div >= rect[cur] && node->left!=0)
        _traverseaction<next, cur>(node->left, rect, std::forward<F>(f));
      if(node->div <= rect[cur+2] && node->right!=0)
        _traverseaction<next, cur>(node->right, rect, std::forward<F>(f));
    }
    inline KDNode<T>* _allocnode(KDNode<T>* parent, char axis)
    {
      KDNode<T>* r=cAllocTracker<Alloc>::_allocate(1);
      memset(r, 0, sizeof(KDNode<T>));
      r->parent=parent;
      r->axis=axis;
      return r;
    }
    inline void _insertitem(KDNode<T>* node, T* item)
    {
      LLBase<T>& l=FLIST(item);
      l.next=node->items;
      l.prev=0;
      if(node->items) FLIST(node->items).prev=item;
      node->items=item;
      FNODE(item)=node;
    }
    inline void _removeitem(KDNode<T>* node, T* item)
    {
      FNODE(item)=0;
      LLBase<T>& l=FLIST(item);
      if(l.prev) FLIST(l.prev).next=l.next;
      else node->items=l.next;
      if(l.next) FLIST(l.next).prev=l.prev;
    }

    KDNode<T>* _root;
    uint32_t _rbthreshold;
  };
}

#endif