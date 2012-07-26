// Copyright ©2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_AVLTREE_H__BSS__
#define __C_AVLTREE_H__BSS__

#include "bss_compare.h"
#include "bss_alloc.h"
#include "functor.h"

namespace bss_util {
  template<class Key, class Data>
  struct BSS_COMPILER_DLLEXPORT AVL_Node
  {
    inline AVL_Node() : _pkey(), _data(), _left(0), _right(0), _balance(0) {}
    Data _data;
    Key _pkey;
    AVL_Node<Key,Data>* _left;
    AVL_Node<Key,Data>* _right;
    int _balance;
  };

  /* AVL Tree implementation */
  template<class Key, class Data, char (*CFunc)(const Key&, const Key&)=CompT<Key>, typename Alloc=Allocator<AVL_Node<Key,Data>>, Data INVALID = 0>
	class BSS_COMPILER_DLLEXPORT cAVLtree : cAllocTracker<Alloc>
  {
  public:
    inline cAVLtree(const cAVLtree& copy) : cAllocTracker<Alloc>(copy), _root(0) {}
    inline cAVLtree(cAVLtree&& mov) : cAllocTracker<Alloc>(copy), _root(mov._root) { mov._root=0; }
    inline cAVLtree(Alloc* allocator=0) : cAllocTracker<Alloc>(allocator), _root(0) {}
    inline ~cAVLtree() { Clear(); }
    inline void Clear() { _clear(_root); _root=0; }
    template<typename F> // std::function<void(Data)>
    inline BSS_FORCEINLINE void Traverse(F lambda) { _traverse<F>(lambda, _root); }

    inline cAVLtree& operator=(cAVLtree&& mov) { Clear(); _root=mov._root; mov._root=0; }

    inline Data BSS_FASTCALL Insert(Key key, Data data)
    {
      char change=0;
      AVL_Node<Key,Data>* cur=_insert(key,&_root,change);
      if(cur)
      {
        cur->_data=data;
        return cur->_data;
      }
      return INVALID;
    }
    inline Data BSS_FASTCALL Remove(const Key key)
    {
      char change=0;
      return _remove(key,&_root,change);
    }
    inline Data BSS_FASTCALL Get(const Key key) const
    {
      AVL_Node<Key,Data>* retval=_find(key);
      return !retval?INVALID:retval->_data;
    }
    inline Data* BSS_FASTCALL GetRef(const Key key) const
    {
      AVL_Node<Key,Data>* retval=_find(key);
      return !retval?0:&retval->_data;
    }
    inline Data BSS_FASTCALL ReplaceKey(const Key oldkey, const Key newkey)
    {
      char change=0;
      Data retval = _remove(oldkey,&_root,change);
      AVL_Node<Key,Data>* cur=_insert(newkey,&_root,change);
      if(cur)
      {
        cur->_data=retval;
        return cur->_data;
      }
      return INVALID;
    }

  protected:
    template<typename F>
    BSS_FORCEINLINE static void BSS_FASTCALL _traverse(F lambda, AVL_Node<Key,Data>* node)
    {
      if(!node) return;
      _traverse<F>(lambda,node->_left);
      lambda(node->_data);
      _traverse<F>(lambda,node->_right);
    }
    inline void BSS_FASTCALL _clear(AVL_Node<Key,Data>* node)
    {
      if(!node) return;
      _clear(node->_left);
      _clear(node->_right);
			_deallocate(node, 1);
    }

    inline static void BSS_FASTCALL _leftrotate(AVL_Node<Key,Data>** root)
    {
      AVL_Node<Key,Data>* _root = *root;
      AVL_Node<Key,Data>* _rotate;

      _rotate = _root->_right;
      _root->_right = _rotate->_left;
      _rotate->_left = _root;
      *root = _rotate;

      _rotate->_left->_balance -= (1 + bssmax(_rotate->_balance, 0));
      _rotate->_balance -= (1 - bssmin(_rotate->_left->_balance, 0));
    }

    inline static void BSS_FASTCALL _rightrotate(AVL_Node<Key,Data>** root)
    {
      AVL_Node<Key,Data>* _root = *root;
      AVL_Node<Key,Data>* _rotate;

      _rotate = _root->_left;
      _root->_left = _rotate->_right;
      _rotate->_right = _root;
      *root = _rotate;

      _rotate->_right->_balance += (1 - bssmin(_rotate->_balance, 0));
      _rotate->_balance += (1 + bssmax(_rotate->_right->_balance, 0));
    }
    inline AVL_Node<Key,Data>* BSS_FASTCALL _insert(Key key, AVL_Node<Key,Data>** proot, char& change) //recursive insertion function
    {
      AVL_Node<Key,Data>* root=*proot;
      if(!root)
      {
				*proot=_allocate(1);
        ObjectTraits<AVL_Node<Key,Data>>::construct(*proot, AVL_Node<Key,Data>());
        (*proot)->_pkey=key;
        change=1;
        return *proot;
      }

      char result=CFunc(root->_pkey,key);
      AVL_Node<Key,Data>* retval=0;
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

    inline Data BSS_FASTCALL _remove(const Key& key, AVL_Node<Key,Data>** proot, char& change) //recursive removal function
    {
      AVL_Node<Key,Data>* root=*proot;
      if(!root)
      {
        change=0;
        return INVALID;
      }
   
      char result=CFunc(root->_pkey,key);
      Data retval=0;
      if(result<0) {
        retval= _remove(key,&root->_left,change);
        result *=change;
      } else if(result>0) {
        retval= _remove(key,&root->_right,change);
        result *=change;
      } else {
        retval=root->_data;
        if(!root->_left&&!root->_right) //leaf node
        {
          _deallocate(root,1);
          *proot=0;
          change=1;
          return retval;
        } else if(!root->_left||!root->_right) { //one child
          AVL_Node<Key,Data>* hold=root;
          *proot = !root->_left?root->_right:root->_left;
          _deallocate(root,1);
          change=1;
          return retval;
				} else {
					AVL_Node<Key,Data>* successor = root->_right;
					while(successor->_left)
						successor = successor->_left;

					retval=root->_data;
					root->_pkey=successor->_pkey;
          root->_data=_remove(successor->_pkey,&root->_right,result); //this works because we're always removing something from the right side, which means we should always subtract 1 or 0.
					change=1;
				}
      }
      root->_balance -= result;

      change=(result)?((root->_balance)?_rebalance(proot):1):0;
      return retval;
    }

    inline AVL_Node<Key,Data>* BSS_FASTCALL _find(const Key& key) const
    {
      AVL_Node<Key,Data>* cur=_root;
      char result=0;
      while(cur)
      {
        switch(CFunc(cur->_pkey,key)) //This is faster then if/else statements because FUCK IF I KNOW!
        {
        case -1:
          cur=cur->_left;
          break;
        case 1:
          cur=cur->_right;
          break;
        default:
        //case 0:
          return cur;
        }
      }

      return 0;
    }
    inline static char BSS_FASTCALL _rebalance(AVL_Node<Key,Data>** root)
    {   
      AVL_Node<Key,Data>* _root=*root;
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

    AVL_Node<Key,Data>* _root;
  };
}

#endif