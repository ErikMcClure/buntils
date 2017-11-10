// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __AVLTREE_H__BSS__
#define __AVLTREE_H__BSS__

#include "compare.h"
#include "BlockAlloc.h"

namespace bss {
  template<class KeyData> // If data is non-void, KeyData is std::pair<Key,Data>, otherwise it's just Key
  struct BSS_COMPILER_DLLEXPORT AVLNode
  {
    inline AVLNode() : _key(), _left(0), _right(0), _balance(0) {} // Empty constructor is important so we initialize the data as empty if it exists.
#pragma warning(push)
#pragma warning(disable:4251)
    KeyData _key;
#pragma warning(pop)
    AVLNode<KeyData>* _left;
    AVLNode<KeyData>* _right;
    int _balance;

  private:
    inline AVLNode(const AVLNode&) = delete; // This is to keep the compiler from defining a copy or assignment constructor that could conflict with move-only structures, like unique_ptr
    AVLNode& operator =(const AVLNode&) = delete;
  };

  namespace internal {
  // Adaptive function definitions to allow for an optional Data field
    template<class Key, class Data>
    class BSS_COMPILER_DLLEXPORT AVLTreeDataField
    {
    public:
      typedef std::pair<Key, Data> KeyData;
      typedef AVLNode<KeyData> Node;
      typedef Data KeyGet;
      typedef Data DataGet;

    protected:
      BSS_FORCEINLINE static void _setData(Node* BSS_RESTRICT old, Node* BSS_RESTRICT cur)
      { 
        assert(old != cur);
        if(cur != 0)
          cur->_key.second = std::move(old->_key.second);
      }
      template<typename U>
      BSS_FORCEINLINE static void _setRaw(U && data, Node* cur) { if(cur != 0) cur->_key.second = std::forward<U>(data); }
      BSS_FORCEINLINE static KeyGet& _getData(KeyData& cur) { return cur.second; }
      BSS_FORCEINLINE static Key& _getKey(KeyData& cur) { return cur.first; }
      BSS_FORCEINLINE static void _swapData(Node* BSS_RESTRICT retval, Node* BSS_RESTRICT root)
      {
        assert(retval != root);
        if(retval != 0) // We have to actually swap the data so that retval returns the correct data so it can be used in ReplaceKey
        {
          Data h(std::move(retval->_key.second));
          retval->_key.second = std::move(root->_key.second);
          root->_key.second = std::move(h);
        }
      }
    };

    template<class Key>
    class BSS_COMPILER_DLLEXPORT AVLTreeDataField<Key, void>
    {
    public:
      typedef Key KeyData;
      typedef AVLNode<KeyData> Node;
      typedef Key KeyGet;
      typedef char DataGet;

    protected:
      BSS_FORCEINLINE static void _setData(Node* BSS_RESTRICT old, Node* BSS_RESTRICT cur) {}
      template<typename U>
      BSS_FORCEINLINE static void _setRaw(U && data, Node* cur) {}
      BSS_FORCEINLINE static KeyGet& _getData(KeyData& cur) { return cur; }
      BSS_FORCEINLINE static Key& _getKey(KeyData& cur) { return cur; }
      BSS_FORCEINLINE static void _swapData(Node* BSS_RESTRICT retval, Node* BSS_RESTRICT root) {}
    };
  }

  // AVL Tree implementation
  template<class Key, class Data, char(*CFunc)(const Key&, const Key&) = CompT<Key>, typename Alloc = PolymorphicAllocator<typename internal::AVLTreeDataField<Key, Data>::Node, BlockPolicy>>
  class BSS_COMPILER_DLLEXPORT AVLTree : protected Alloc, public internal::AVLTreeDataField<Key, Data>
  {
    AVLTree(const AVLTree& copy) = delete;
    AVLTree& operator=(const AVLTree& copy) = delete;
  protected:
    typedef internal::AVLTreeDataField<Key, Data> Base;
    typedef typename Base::KeyData KeyData;
    typedef typename Base::Node Node;
    typedef typename Base::KeyGet KeyGet;
    typedef typename Base::DataGet DataGet;

  public:
    inline AVLTree(AVLTree&& mov) : Alloc(std::move(mov)), _root(mov._root) { mov._root = 0; }
    template<bool U = std::is_void_v<typename Alloc::policy_type>, std::enable_if_t<!U, int> = 0>
    inline explicit AVLTree(typename Alloc::policy_type* policy) : Alloc(policy), _root(0) {}
    inline AVLTree() : _root(0) {}
    inline ~AVLTree() { Clear(); }
    inline void Clear() { _clear(_root); _root = 0; }
    inline Node* GetRoot() { return _root; }
    template<typename F> // std::function<void(std::pair<key,data>)> (we infer _traverse's template argument here, otherwise GCC explodes)
    BSS_FORCEINLINE void Traverse(F lambda) { _traverse(lambda, _root); }
    BSS_FORCEINLINE Node* Insert(Key key, const DataGet& data)
    {
      char change = 0;
      Node* cur = _insert(key, &_root, change);
      Base::template _setRaw<const DataGet&>(data, cur); // WHO COMES UP WITH THIS SYNTAX?!
      return cur;
    }
    BSS_FORCEINLINE Node* Insert(Key key, DataGet&& data)
    {
      char change = 0;
      Node* cur = _insert(key, &_root, change);
      Base::template _setRaw<DataGet&&>(std::move(data), cur);
      return cur;
    }
    BSS_FORCEINLINE Node* Insert(Key key)
    {
      char change = 0;
      return _insert(key, &_root, change);
    }
    BSS_FORCEINLINE Node* Near(const Key key) const { return _near(key); }
    BSS_FORCEINLINE KeyGet Get(const Key key, const KeyGet& INVALID) const
    {
      Node* retval = _find(key);
      return !retval ? INVALID : Base::_getData(retval->_key);
    }
    BSS_FORCEINLINE KeyGet* GetRef(const Key key) const
    {
      Node* retval = _find(key);
      return !retval ? 0 : &Base::_getData(retval->_key);
    }
    inline bool Remove(const Key key)
    {
      char change = 0;
      Node* node = _remove(key, &_root, change);

      if(node != 0)
      {
        node->~Node();
        Alloc::deallocate(node, 1);
        return true;
      }

      return false;
    }
    inline bool ReplaceKey(const Key oldkey, const Key newkey)
    {
      char change = 0;
      Node* old = _remove(oldkey, &_root, change);
      Node* cur = _insert(newkey, &_root, change);

      if(old != 0)
      {
        Base::_setData(old, cur);
        old->~Node();
        Alloc::deallocate(old, 1);
      }

      return (cur != 0);
    }

    inline AVLTree& operator=(AVLTree&& mov) { Clear(); _root = mov._root; mov._root = 0; return *this; }

  protected:
    template<typename F>
    BSS_FORCEINLINE static void _traverse(F lambda, Node* node)
    {
      if(!node)
        return;

      _traverse<F>(lambda, node->_left);
      lambda(node->_key);
      _traverse<F>(lambda, node->_right);
    }

    inline void _clear(Node* node)
    {
      if(!node)
        return;

      _clear(node->_left);
      _clear(node->_right);
      node->~Node();
      Alloc::deallocate(node, 1);
    }

    static void _leftRotate(Node** pnode)
    {
      Node* node = *pnode;
      Node* r = node->_right;

      node->_right = r->_left;
      r->_left = node;
      *pnode = r;

      r->_left->_balance -= (1 + bssmax(r->_balance, 0));
      r->_balance -= (1 - bssmin(r->_left->_balance, 0));
    }

    static void _rightRotate(Node** pnode)
    {
      Node* node = *pnode;
      Node* r = node->_left;

      node->_left = r->_right;
      r->_right = node;
      *pnode = r;

      r->_right->_balance += (1 - bssmin(r->_balance, 0));
      r->_balance += (1 + bssmax(r->_right->_balance, 0));
    }
    Node* _insert(Key key, Node** proot, char& change) //recursive insertion function
    {
      Node* root = *proot;

      if(!root)
      {
        root = Alloc::allocate(1);
        *proot = root;
        new(root) Node();
        Base::_getKey(root->_key) = key;
        change = 1;
        return root;
      }

      char result = CFunc(Base::_getKey(root->_key), key);
      Node* retval = 0;

      if(result < 0)
        retval = _insert(key, &root->_left, change);
      else if(result > 0)
        retval = _insert(key, &root->_right, change);
      else
      {
        change = 0;
        return 0;
      }

      result *= change;
      root->_balance += result;
      change = (result && root->_balance) ? (1 - _rebalance(proot)) : 0;
      return retval;
    }

    inline Node* _find(const Key& key) const
    {
      Node* cur = _root;

      while(cur)
      {
        switch(CFunc(Base::_getKey(cur->_key), key)) //This is faster then if/else statements because FUCK IF I KNOW!
        {
        case -1: cur = cur->_left; break;
        case 1: cur = cur->_right; break;
        default: return cur;
        }
      }

      return 0;
    }

    inline Node* _near(const Key& key) const
    {
      Node* prev = 0;
      Node* cur = _root;

      while(cur)
      {
        prev = cur;
        switch(CFunc(Base::_getKey(cur->_key), key)) //This is faster then if/else statements because FUCK IF I KNOW!
        {
        case -1: cur = cur->_left; break;
        case 1: cur = cur->_right; break;
        default: return cur;
        }
      }

      return prev;
    }
    inline static char _rebalance(Node** root)
    {
      Node* _root = *root;
      char retval = 0;

      if(_root->_balance < -1)
      {
        if(_root->_left->_balance == 1) // LR
        {
          _leftRotate(&_root->_left);
          _rightRotate(root);
          retval = 1;
        }
        else
        { // LL 
          retval = !_root->_left->_balance ? 0 : 1;
          //retval = (_root->_left->_balance<0)+(_root->_left->_balance>0); //Does the same thing as above without branching (useless)
          _rightRotate(root);
        }
      }
      else if(_root->_balance > 1)
      {
        if(_root->_right->_balance == -1)
        { //RL
          _rightRotate(&_root->_right);
          _leftRotate(root);
          retval = 1;
        }
        else
        { // RR
          retval = !_root->_right->_balance ? 0 : 1;
          //retval = (_root->_right->_balance<0)+(_root->_right->_balance>0); //Does the same thing as above without branching (useless)
          _leftRotate(root);
        }
      }

      return retval;
    }

    Node* _remove(const Key& key, Node** proot, char& change) //recursive removal function
    {
      Node* root = *proot;

      if(!root)
      {
        change = 0;
        return 0;
      }

      char result = CFunc(Base::_getKey(root->_key), key);
      Node* retval = 0;

      if(result < 0)
      {
        retval = _remove(key, &root->_left, change);
        result *= change;
      }
      else if(result > 0)
      {
        retval = _remove(key, &root->_right, change);
        result *= change;
      }
      else
      {
        if(!root->_left && !root->_right) //leaf node
        {
          *proot = 0;
          change = 1;
          return root;
        }
        else if(!root->_left || !root->_right)
        { //one child
          *proot = !root->_left ? root->_right : root->_left;
          change = 1;
          return root;
        }
        else
        {
          Node* successor = root->_right;
          while(successor->_left)
            successor = successor->_left;

          Base::_getKey(root->_key) = Base::_getKey(successor->_key);
          retval = _remove(Base::_getKey(successor->_key), &root->_right, result); //this works because we're always removing something from the right side, which means we should always subtract 1 or 0.
          Base::_swapData(retval, root);

          change = 1;
        }
      }
      root->_balance -= result;

      change = (result) ? ((root->_balance) ? _rebalance(proot) : 1) : 0;
      return retval;
    }

    Node* _root;
  };
}

#endif
