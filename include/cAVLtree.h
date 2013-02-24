// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AVLTREE_H__BSS__
#define __C_AVLTREE_H__BSS__

#include "bss_compare.h"
#include "bss_alloc.h"

namespace bss_util {
  template<class Key, class Data>
  struct BSS_COMPILER_DLLEXPORT AVL_Node
  {
    inline explicit AVL_Node(Key key) : _key(key), _data(), _left(0), _right(0), _balance(0) {}
    Data _data;
    Key _key;
    AVL_Node<Key,Data>* _left;
    AVL_Node<Key,Data>* _right;
    int _balance;
  };

  template<class Key>
  struct BSS_COMPILER_DLLEXPORT AVL_Node<Key,void>
  {
    inline explicit AVL_Node(Key key) : _key(key), _left(0), _right(0), _balance(0) {}
    Key _key;
    AVL_Node<Key,void>* _left;
    AVL_Node<Key,void>* _right;
    int _balance;
  };

  // Internal deep functions
  template<class Key, class Data, char (*CFunc)(const Key&, const Key&), typename Alloc>
  class BSS_COMPILER_DLLEXPORT _AVL_TREE_INTERNAL : protected cAllocTracker<Alloc>
  {
    typedef AVL_Node<Key,Data> AVLNode;

  protected:
    inline _AVL_TREE_INTERNAL(const _AVL_TREE_INTERNAL& copy) : cAllocTracker<Alloc>(copy), _root(0) {}
    inline _AVL_TREE_INTERNAL(_AVL_TREE_INTERNAL&& mov) : cAllocTracker<Alloc>(std::move(mov)), _root(mov._root) { mov._root=0; }
    inline _AVL_TREE_INTERNAL(Alloc* allocator=0) : cAllocTracker<Alloc>(allocator), _root(0) {}

    inline void BSS_FASTCALL _clear(AVLNode* node)
    {
      if(!node) return;
      _clear(node->_left);
      _clear(node->_right);
      node->~AVLNode();
			_deallocate(node, 1);
    }

    inline static void BSS_FASTCALL _leftrotate(AVLNode** pnode)
    {
      AVLNode* node = *pnode;
      AVLNode* r = node->_right;

      node->_right = r->_left;
      r->_left = node;
      *pnode = r;

      r->_left->_balance -= (1 + bssmax(r->_balance, 0));
      r->_balance -= (1 - bssmin(r->_left->_balance, 0));
    }

    inline static void BSS_FASTCALL _rightrotate(AVLNode** pnode)
    {
      AVLNode* node = *pnode;
      AVLNode* r = node->_left;

      node->_left = r->_right;
      r->_right = node;
      *pnode = r;

      r->_right->_balance += (1 - bssmin(r->_balance, 0));
      r->_balance += (1 + bssmax(r->_right->_balance, 0));
    }
    inline AVLNode* BSS_FASTCALL _insert(Key key, AVLNode** proot, char& change) //recursive insertion function
    {
      AVLNode* root=*proot;
      if(!root)
      {
				*proot=cAllocTracker<Alloc>::_allocate(1);
        new(*proot) AVLNode(key);
        change=1;
        return *proot;
      }

      char result=CFunc(root->_key,key);
      AVLNode* retval=0;
      if(result<0)
        retval= _insert(key,&root->_left,change);
      else if(result>0)
        retval= _insert(key,&root->_right,change);
      else
      {
        change=0;
        return 0;
      }
      result *=change;
      root->_balance+=result;
      change = (result && root->_balance)?(1 - _rebalance(proot)):0;
      return retval;
    };

    inline AVLNode* BSS_FASTCALL _find(const Key& key) const
    {
      AVLNode* cur=_root;
      while(cur)
      {
        switch(CFunc(cur->_key,key)) //This is faster then if/else statements because FUCK IF I KNOW!
        {
        case -1: cur=cur->_left; break;
        case 1: cur=cur->_right; break;
        default: return cur;
        }
      }

      return 0;
    }
    inline static char BSS_FASTCALL _rebalance(AVLNode** root)
    {   
      AVLNode* _root=*root;
      char retval=0;
      if(_root->_balance<-1)
      {
        if(_root->_left->_balance == 1) // LR
        {
          _leftrotate(&_root->_left);
          _rightrotate(root);
          retval = 1;
        } else { // LL 
          retval = !_root->_left->_balance?0:1;
          //retval = (_root->_left->_balance<0)+(_root->_left->_balance>0); //Does the same thing as above without branching (useless)
          _rightrotate(root);
        }
      } else if(_root->_balance>1) {
        if (_root->_right->_balance == -1) { //RL
          _rightrotate(&_root->_right);
          _leftrotate(root);
          retval = 1;
        } else { // RR
          retval = !_root->_right->_balance?0:1;
          //retval = (_root->_right->_balance<0)+(_root->_right->_balance>0); //Does the same thing as above without branching (useless)
          _leftrotate(root);
        }
      }
      return retval;
    }

    AVLNode* _root;
  };

  // Adaptive function definitions to allow for an optional Data field
  template<class Key, class Data, char (*CFunc)(const Key&, const Key&), typename Alloc>
  class BSS_COMPILER_DLLEXPORT _AVL_TREE_DATAFIELD : public _AVL_TREE_INTERNAL<Key,Data,CFunc,Alloc>
  {
  public:
    typedef AVL_Node<Key,Data> AVLNode;
    using _AVL_TREE_INTERNAL<Key,Data,CFunc,Alloc>::_root;

    inline bool BSS_FASTCALL Insert(Key key, Data data)
    {
      char change=0;
      AVLNode* cur=_insert(key,&_root,change);
      if(cur)
      {
        cur->_data=std::move(data);
        return true;
      }
      return false;
    }
    inline Data BSS_FASTCALL Get(const Key key, const Data& INVALID=0) const
    {
      AVLNode* retval=_find(key);
      return !retval?INVALID:retval->_data;
    }
    inline Data* BSS_FASTCALL GetRef(const Key key) const
    {
      AVLNode* retval=_find(key);
      return !retval?0:&retval->_data;
    }
    
  protected:
    inline _AVL_TREE_DATAFIELD(const _AVL_TREE_DATAFIELD& copy) : _AVL_TREE_INTERNAL<Key,Data,CFunc,Alloc>(copy) {}
    inline _AVL_TREE_DATAFIELD(_AVL_TREE_DATAFIELD&& mov) : _AVL_TREE_INTERNAL<Key,Data,CFunc,Alloc>(std::move(mov)) { }
    inline _AVL_TREE_DATAFIELD(Alloc* allocator=0) : _AVL_TREE_INTERNAL<Key,Data,CFunc,Alloc>(allocator) {}
    template<typename F>
    BSS_FORCEINLINE static void BSS_FASTCALL _traverse(F lambda, AVLNode* node)
    {
      if(!node) return;
      _traverse<F>(lambda,node->_left);
      lambda(node->_data);
      _traverse<F>(lambda,node->_right);
    }
    BSS_FORCEINLINE static void BSS_FASTCALL _setdata(AVLNode* old,AVLNode* cur) { if(cur!=0) cur->_data=std::move(old->_data); }
    BSS_FORCEINLINE static void BSS_FASTCALL _swapdata(AVLNode* retval,AVLNode* root) 
    {
      if(retval!=0) // We have to actually swap the data so that retval returns the correct data so it can be used in ReplaceKey
      {
        Data h(std::move(retval->_data));
        retval->_data=std::move(root->_data);
        root->_data=std::move(h);
      }
    }
  };

  template<class Key, char (*CFunc)(const Key&, const Key&), typename Alloc>
  class BSS_COMPILER_DLLEXPORT _AVL_TREE_DATAFIELD<Key,void,CFunc,Alloc> : public _AVL_TREE_INTERNAL<Key,void,CFunc,Alloc>
  {
  public:
    typedef AVL_Node<Key,void> AVLNode;
    using _AVL_TREE_INTERNAL<Key,void,CFunc,Alloc>::_root;

    inline bool BSS_FASTCALL Insert(Key key)
    {
      char change=0;
      AVLNode* cur=_insert(key,&_root,change);
      return cur!=0;
    }
    inline Key BSS_FASTCALL Get(const Key key, const Key& INVALID=0) const
    {
      AVLNode* retval=_find(key);
      return !retval?INVALID:retval->_key;
    }
    inline Key* BSS_FASTCALL GetRef(const Key key) const
    {
      AVLNode* retval=_find(key);
      return !retval?0:&retval->_key;
    }
    
  protected:
    inline _AVL_TREE_DATAFIELD(const _AVL_TREE_DATAFIELD& copy) : _AVL_TREE_INTERNAL<Key,void,CFunc,Alloc>(copy) {}
    inline _AVL_TREE_DATAFIELD(_AVL_TREE_DATAFIELD&& mov) : _AVL_TREE_INTERNAL<Key,void,CFunc,Alloc>(std::move(mov)) { }
    inline _AVL_TREE_DATAFIELD(Alloc* allocator=0) : _AVL_TREE_INTERNAL<Key,void,CFunc,Alloc>(allocator) {}
    template<typename F>
    BSS_FORCEINLINE static void BSS_FASTCALL _traverse(F lambda, AVLNode* node)
    {
      if(!node) return;
      _traverse<F>(lambda,node->_left);
      lambda(node->_key);
      _traverse<F>(lambda,node->_right);
    }
    BSS_FORCEINLINE static void BSS_FASTCALL _setdata(AVLNode* old,AVLNode* cur) { }
    BSS_FORCEINLINE static void BSS_FASTCALL _swapdata(AVLNode* retval,AVLNode* root) { }
  };

  // AVL Tree implementation
  template<class Key, class Data, char (*CFunc)(const Key&, const Key&)=CompT<Key>, typename Alloc=Allocator<AVL_Node<Key,Data>>>
	class BSS_COMPILER_DLLEXPORT cAVLtree : public _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>
  {
    typedef AVL_Node<Key,Data> AVLNode;
    using _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>::_root;

  public:
    inline cAVLtree(const cAVLtree& copy) : _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>(copy) {}
    inline cAVLtree(cAVLtree&& mov) : _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>(std::move(mov)) { }
    inline cAVLtree(Alloc* allocator=0) : _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>(allocator) {}
    inline ~cAVLtree() { Clear(); }
    inline void Clear() { _clear(_root); _root=0; }
    template<typename F> // std::function<void(Data)> (we infer _traverse's template argument here because GCC explodes otherwise)
    inline BSS_FORCEINLINE void Traverse(F lambda) { _AVL_TREE_DATAFIELD<Key,Data,CFunc,Alloc>::_traverse(lambda, _root); }

    inline cAVLtree& operator=(cAVLtree&& mov) { Clear(); _root=mov._root; mov._root=0; }
    inline bool BSS_FASTCALL Remove(const Key key)
    {
      char change=0;
      AVLNode* node = _remove(key,&_root,change);
      if(node!=0)
      {
        node->~AVLNode();
        _deallocate(node,1);
        return true;
      }
      return false;
    }
    inline bool BSS_FASTCALL ReplaceKey(const Key oldkey, const Key newkey)
    {
      char change=0;
      AVLNode* old = _remove(oldkey,&_root,change);
      AVLNode* cur = _insert(newkey,&_root,change);
      if(old!=0)
      {
        _setdata(old,cur);
        old->~AVLNode();
			  _deallocate(old, 1);
      }
      return (cur!=0);
    }

  protected:
    inline AVLNode* BSS_FASTCALL _remove(const Key& key, AVLNode** proot, char& change) //recursive removal function
    {
      AVLNode* root=*proot;
      if(!root)
      {
        change=0;
        return 0;
      }
   
      char result=CFunc(root->_key,key);
      AVLNode* retval=0;
      if(result<0) {
        retval= _remove(key,&root->_left,change);
        result *=change;
      } else if(result>0) {
        retval= _remove(key,&root->_right,change);
        result *=change;
      } else {
        if(!root->_left&&!root->_right) //leaf node
        {
          *proot=0;
          change=1;
          return root;
        } else if(!root->_left||!root->_right) { //one child
          *proot = !root->_left?root->_right:root->_left;
          change=1;
          return root;
				} else {
					AVLNode* successor = root->_right;
					while(successor->_left)
						successor = successor->_left;

					root->_key=successor->_key;
          retval=_remove(successor->_key,&root->_right,result); //this works because we're always removing something from the right side, which means we should always subtract 1 or 0.
          _swapdata(retval,root);

					change=1;
				}
      }
      root->_balance -= result;

      change=(result)?((root->_balance)?_rebalance(proot):1):0;
      return retval;
    }
  };
}

#endif
